#include "FileAPI.h"

int FileAPI::File_Create(string path, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg) {
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }

    if (path.empty() || path[0] != '/') {
        osErrMsg = E_FILE_CREATE;
        return -1; // Invalid path
    }

    vector<string> directories;
    stringstream ss(path);
    string segment;
    while (getline(ss, segment, '/')) {
        if (!segment.empty())
            directories.push_back(segment);
    }

    string currentPath = "/";
    INode* parentInode = nullptr;
    if (currentPath == "/")
        parentInode = &workingDisk.inodes[0][0];
    for (int i = 0; i < directories.size() - 1; ++i) {
        if (directories[i] != "/")
            currentPath += directories[i];
        bool parentFound = false;
        for (int row = 0; row < workingDisk.inodes.size(); ++row) {
            for (int col = 0; col < workingDisk.inodes[row].size(); ++col) {
                INode& inode = workingDisk.inodes[row][col];
                if (inode.fileName == currentPath && inode.type == "directory") {
                    parentFound = true;
                    parentInode = &inode; // Found the parent inode
                    break;
                }
            }
            if (parentFound) break;
        }

        if (!parentFound) {
            osErrMsg = E_DIR_NOT_FOUND;
            return -1; // Parent directory does not exist
        }

        currentPath += "/";
    }

    string finalFile = directories.back();
    currentPath += finalFile;
    for (int row = 0; row < workingDisk.inodes.size(); ++row) {
        for (int col = 0; col < workingDisk.inodes[row].size(); ++col) {
            INode& inode = workingDisk.inodes[row][col];
            if (inode.fileName == currentPath) {
                osErrMsg = E_FILE_CREATE;
                return -1; // File already exists
            }
        }
    }

    int newInodeIdx = -1;
    for (int row = 0; row < workingDisk.ibitmap.size(); ++row) {
        if (!workingDisk.ibitmap[row]) {
            newInodeIdx = row;
            workingDisk.ibitmap[row] = true;
            break;
        }
    }

    if (newInodeIdx == -1) {
        osErrMsg = E_FILE_CREATE;
        return -1; // No free inode available
    }

    INode& newInode = workingDisk.inodes[newInodeIdx / 10][newInodeIdx % 10];
    newInode.fileName = currentPath;
    newInode.type = "file";
    newInode.fileSize = 0;
    newInode.isFileOpen = false;

    int newDataBlockIdx = -1;
    for (int i = 0; i < workingDisk.dbitmap.size(); ++i) {
        if (!workingDisk.dbitmap[i]) {
            newDataBlockIdx = i;
            workingDisk.dbitmap[i] = true; // Mark the data block as used
            break;
        }
    }

    if (newDataBlockIdx == -1) {
        osErrMsg = E_FILE_CREATE;
        return -1; // No free data block available
    }
    newInode.dataBlocks.push_back(newDataBlockIdx);

    if (parentInode != nullptr) {
        if (parentInode->fileSize < 32) {
            if (finalFile.length() > 15)
                finalFile.resize(15);
            else if (finalFile.length() < 15)
                finalFile.resize(15, '*');

            string inodeStr = to_string(newInodeIdx);
            inodeStr.insert(0, 4 - inodeStr.length(), '*');
            finalFile += inodeStr;

            if (finalFile.length() < 19)
                finalFile.resize(19, '*');
            finalFile += '\n';
            int currDataBlock = parentInode->dataBlocks.back();
            string blockData;
            if (DiskAPI::Disk_Read(currDataBlock, blockData, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
                osErrMsg = diskErrMsg;
                return -1;
            }

            size_t insertPos = blockData.find('\0');
            if (insertPos == string::npos) {
                osErrMsg = "Disk block is full!";
                return -1;
            }

            for (size_t i = 0; i < finalFile.size(); i++) {
                blockData[insertPos + i] = finalFile[i];
            }

            if (DiskAPI::Disk_Write(currDataBlock, blockData, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
                osErrMsg = diskErrMsg;
                return -1;
            }
            parentInode->fileSize++;
            osSuccessMsg = S_FILE_CREATE;
        }
        else {
            osErrMsg = E_FILE_CREATE;
            return -1;
        }
    }
    return 0;
}

int FileAPI::File_Open(string file, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg) {
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }

    if (openedFiles.size() >= 10) {
        osErrMsg = E_TOO_MANY_OPEN_FILES;
        return -1;
    }

    if (file.empty() || file[0] != '/') {
        osErrMsg = E_NO_SUCH_FILE;
        return -1;
    }

    vector<string> pathComponents;
    stringstream ss(file);
    string segment;
    while (getline(ss, segment, '/')) {
        if (!segment.empty()) {
            pathComponents.push_back(segment);
        }
    }

    int currINodeRow = 0, currINodeCol = 0; // Root inode
    string buildComponent;
    INode* currINode = &workingDisk.inodes[currINodeRow][currINodeCol];
    for (size_t i = 0; i < pathComponents.size(); ++i) {
        const string& component = pathComponents[i];
        bool isLastComponent = (i == pathComponents.size() - 1);
        bool found = false;
        buildComponent += "/" + component;
        for (int dataBlock : currINode->dataBlocks) {
            string blockData;
            if (DiskAPI::Disk_Read(dataBlock, blockData, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
                osErrMsg = diskErrMsg;
                return -1;
            }

            if (blockData.find(component) != string::npos) {
                found = true;

                if (isLastComponent) {
                    tie(currINodeRow, currINodeCol) = DiskAPI::GetINode(file, workingDisk, false);
                    INode& fileINode = workingDisk.inodes[currINodeRow][currINodeCol];
                    if (fileINode.type != "file") {
                        osErrMsg = E_NO_SUCH_FILE;
                        return -1; // Component is not a file
                    }
                }
                else {
                    tie(currINodeRow, currINodeCol) = DiskAPI::GetINode(buildComponent, workingDisk, true);
                    currINode = &workingDisk.inodes[currINodeRow][currINodeCol];
                }
                break;
            }
        }

        if (!found) {
            osErrMsg = E_NO_SUCH_FILE;
            return -1; // Component not found
        }
    }

    for (size_t i = 0; i < openedFiles.size(); ++i) {
        if (openedFiles[i].fileName == file) {
            osSuccessMsg = E_FILE_ALREADY_OPEN + " FILE DESCRIPTOR: " + to_string(i);
            return i; // Return the existing file descriptor
        }
    }

    string inodeData;
    int inodeIndex = currINodeRow * workingDisk.inodes[currINodeRow].size() + currINodeCol;
    if (DiskAPI::Disk_Read(inodeIndex, inodeData, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
        osErrMsg = diskErrMsg;
        return -1; // Read failed
    }

    FileBlock fileBlock;
    fileBlock.inodeIdx = inodeIndex;
    fileBlock.fileName = file;
    fileBlock.filePointer = 0;
    openedFiles.push_back(fileBlock);
    int openFileDescriptor = openedFiles.size() - 1;
    workingDisk.inodes[currINodeRow][currINodeCol].isFileOpen = true;
    osSuccessMsg = S_FILE_OPEN + " FILE DESCRIPTOR: " + to_string(openFileDescriptor);
    return openFileDescriptor; // Return the file descriptor
}

int FileAPI::File_Read(int fd, string& buffer, int size, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }

    if (fd < 0 || fd >= openedFiles.size()) {
        osErrMsg = E_READ_BAD_FD;
        return -1; // Invalid file descriptor
    }

    FileBlock& fileBlock = openedFiles[fd];
    int inodeIdx = fileBlock.inodeIdx;
    int inodeRow = inodeIdx / workingDisk.inodes[0].size();
    int inodeCol = inodeIdx % workingDisk.inodes[0].size();
    INode& inode = workingDisk.inodes[inodeRow][inodeCol];

    if (!inode.isFileOpen || inode.type != "file") {
        osErrMsg = E_READ_BAD_FD;
        return -1; // File is not open
    }

    int& filePointer = fileBlock.filePointer;

    if (filePointer >= inode.fileSize) {
        osSuccessMsg = "File pointer is at end of data!";
        return 0;
    }

    int bytesToRead = std::min(size, inode.fileSize - filePointer);

    if (bytesToRead <= 0) {
        osSuccessMsg = "EOF, no more data to read!";
        return 0;
    }

    buffer.clear();
    buffer.reserve(bytesToRead);
    int bytesRead = 0;

    int blockStart = filePointer / SECTOR_SIZE;
    int blockOffset = filePointer % SECTOR_SIZE;

    for (size_t i = blockStart; i < inode.dataBlocks.size() && bytesRead < bytesToRead; ++i) {
        int block = inode.dataBlocks[i];
        string blockData;
        if (DiskAPI::Disk_Read(block, blockData, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
            osErrMsg = diskErrMsg;
            return -1; // Disk read error
        }

        int remainingBytes = bytesToRead - bytesRead;
        int readThisTime = std::min((int)blockData.length() - blockOffset, remainingBytes);

        buffer += blockData.substr(blockOffset, readThisTime);
        bytesRead += readThisTime;
        filePointer += readThisTime;

        // Reset blockOffset for subsequent blocks
        blockOffset = 0;

        if (bytesRead >= bytesToRead) {
            break; // Done reading
        }
    }

    osSuccessMsg = S_FILE_READ + " BYTES READ: " + to_string(bytesRead) + " - IN THE BUFFER: " + buffer;
    return bytesRead;
}

int FileAPI::File_Write(int fd, string buffer, int size, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }

    if (fd < 0 || fd >= openedFiles.size()) {
        osErrMsg = E_BAD_FD;
        return -1; // Invalid file descriptor
    }

    FileBlock& fileBlock = openedFiles[fd];
    int inodeIdx = fileBlock.inodeIdx;
    int inodeRow = inodeIdx / workingDisk.inodes[0].size();
    int inodeCol = inodeIdx % workingDisk.inodes[0].size();
    INode& inode = workingDisk.inodes[inodeRow][inodeCol];

    if (!inode.isFileOpen || inode.type != "file") {
        osErrMsg = E_BAD_FD;
        return -1; // File is not open
    }

    int& filePointer = fileBlock.filePointer;

    if (filePointer + size > MAX_FILE_SIZE) {
        osErrMsg = E_FILE_TOO_BIG;
        return -1;
    }

    if (size > buffer.size())
        size = buffer.size();

    int spaceNeeded = size - (inode.fileSize - filePointer);
    if (spaceNeeded > 0) {
        int freeBlocks = 0;
        for (bool blockFree : workingDisk.dbitmap) {
            if (!blockFree) freeBlocks++;
        }
        if (freeBlocks < (spaceNeeded + SECTOR_SIZE - 1) / SECTOR_SIZE) {
            osErrMsg = E_NO_SPACE;
            return -1;
        }
    }

    int bytesWritten = 0;
    for (int& block : inode.dataBlocks) {
        if (block == -1) {
            for (int i = 0; i < workingDisk.dbitmap.size(); ++i) {
                if (!workingDisk.dbitmap[i]) {
                    block = i;
                    workingDisk.dbitmap[i] = true;
                    break;
                }
            }
            if (block == -1) {
                osErrMsg = E_NO_SPACE;
                return -1; // No space left
            }
        }

        int blockOffset = filePointer % SECTOR_SIZE;
        string blockData(SECTOR_SIZE, '\0');

        // Prevent out-of-range access
        for (int i = blockOffset; i < SECTOR_SIZE && bytesWritten < size; ++i) {
            blockData[i] = buffer[bytesWritten++];
            filePointer++;
        }

        blockData[bytesWritten] = '\n';
        bytesWritten++;
        if (DiskAPI::Disk_Write(block, blockData, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
            osErrMsg = diskErrMsg;
            return -1;
        }

        if (bytesWritten >= size) break;
    }

    inode.fileSize = std::max(inode.fileSize, filePointer);
    osSuccessMsg = S_FILE_WRITE + " BYTES WRITTEN: " + to_string(bytesWritten);
    return bytesWritten;
}

int FileAPI::File_Seek(int fd, int offset, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }

    if (fd < 0 || fd >= openedFiles.size()) {
        osErrMsg = E_SEEK_BAD_FD;
        return -1; // Invalid file descriptor
    }

    FileBlock& fileBlock = openedFiles[fd];
    int inodeIdx = fileBlock.inodeIdx;
    int inodeRow = inodeIdx / workingDisk.inodes[0].size();
    int inodeCol = inodeIdx % workingDisk.inodes[0].size();
    INode& inode = workingDisk.inodes[inodeRow][inodeCol];

    if (!inode.isFileOpen || inode.type != "file") {
        osErrMsg = E_SEEK_BAD_FD;
        return -1; // File is not open
    }

    if (offset < 0 || offset > inode.fileSize) {
        osErrMsg = E_SEEK_OUT_OF_BOUNDS;
        return -1; // Offset out of bounds
    }

    fileBlock.filePointer = offset;
    osSuccessMsg = S_FILE_SEEK + " NEW FILE POINTER LOCATION: " + to_string(fileBlock.filePointer);
    return fileBlock.filePointer;
}

int FileAPI::File_Close(int fd, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }

    if (fd < 0 || fd >= openedFiles.size()) {
        osErrMsg = E_CLOSE_BAD_FD;
        return -1; // Invalid file descriptor
    }

    FileBlock& fileBlock = openedFiles[fd];
    int inodeIdx = fileBlock.inodeIdx;
    int inodeRow = inodeIdx / workingDisk.inodes[0].size();
    int inodeCol = inodeIdx % workingDisk.inodes[0].size();
    INode& inode = workingDisk.inodes[inodeRow][inodeCol];

    if (!inode.isFileOpen || inode.type != "file") {
        osErrMsg = E_CLOSE_BAD_FD;
        return -1; // File is not open
    }

    inode.isFileOpen = false;
    openedFiles.erase(openedFiles.begin() + fd);
    osSuccessMsg = S_FILE_CLOSE;
    return 0;
}

int FileAPI::File_Unlink(string file, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }

    auto inodePair = DiskAPI::GetINode(file, workingDisk, false);
    int inodeRow = inodePair.first, inodeCol = inodePair.second;
    if (inodeRow == -1 || inodeCol == -1) {
        osErrMsg = E_NO_SUCH_FILE; // File not found
        return -1;
    }

    INode& inode = workingDisk.inodes[inodeRow][inodeCol];

    if (inode.isFileOpen) {
        osErrMsg = E_FILE_IN_USE;
        return -1;
    }

    for (int block : inode.dataBlocks) {
        if (block >= 0 && block < workingDisk.data.size()) {
            fill(workingDisk.data[block].begin(), workingDisk.data[block].end(), '\0');
            workingDisk.dbitmap[block] = false;
            string blockBuffer(workingDisk.data[block].begin(), workingDisk.data[block].end());
            if (DiskAPI::Disk_Write(block, blockBuffer, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
                osErrMsg = diskErrMsg;
                return -1;
            }
        }
    }

    inode.fileName.clear();
    inode.fileSize = 0;
    inode.type.clear();
    inode.isFileOpen = false;
    inode.dataBlocks.clear();
    workingDisk.ibitmap[inodeRow * workingDisk.inodes[inodeRow].size() + inodeCol] = false;
    string ibitmapBuffer(workingDisk.ibitmap.begin(), workingDisk.ibitmap.end());
    if (DiskAPI::Disk_Write(0, ibitmapBuffer, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
        osErrMsg = diskErrMsg;
        return -1;
    }

    osSuccessMsg = S_FILE_UNLINK;
    return 0;
}
