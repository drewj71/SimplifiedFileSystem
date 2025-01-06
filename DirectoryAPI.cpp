#include "DirectoryAPI.h"

int DirectoryAPI::Dir_Create(string path, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    else if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }
    if (path.empty() || path[0] != '/') {
        osErrMsg = E_DIR_CREATE;
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
    // Handle root directory case first
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

    string finalDirectory = directories.back();
    currentPath += finalDirectory;
    for (int row = 0; row < workingDisk.inodes.size(); ++row) {
        for (int col = 0; col < workingDisk.inodes[row].size(); ++col) {
            INode& inode = workingDisk.inodes[row][col];
            if (inode.fileName == currentPath) {
                osErrMsg = E_DIR_CREATE;
                return -1; // Directory already exists
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
        osErrMsg = E_DIR_CREATE;
        return -1; // No free inode available
    }

    INode& newInode = workingDisk.inodes[newInodeIdx / 10][newInodeIdx % 10];
    newInode.fileName = currentPath;
    newInode.type = "directory";
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
        osErrMsg = E_DIR_CREATE;
        return -1; // No free data block available
    }
    newInode.dataBlocks.push_back(newDataBlockIdx);

    if (parentInode != nullptr) {
        if (parentInode->fileSize < 32) {
            if (finalDirectory.length() > 15)
                finalDirectory.resize(15);
            else if (finalDirectory.length() < 15)
                finalDirectory.resize(15, '*');

            string inodeStr = to_string(newInodeIdx);
            inodeStr.insert(0, 4 - inodeStr.length(), '*');
            finalDirectory += inodeStr;

            if (finalDirectory.length() < 19)
                finalDirectory.resize(19, '*');
            finalDirectory += '\n';
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

            for (size_t i = 0; i < finalDirectory.size(); i++) {
                blockData[insertPos + i] = finalDirectory[i];
            }

            if (DiskAPI::Disk_Write(currDataBlock, blockData, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
                osErrMsg = diskErrMsg;
                return -1;
            }
            parentInode->fileSize++;
            osSuccessMsg = S_DIR_CREATE;
        }
        else {
            osErrMsg = E_DIR_CREATE;
            return -1;
        }
    }
    return 0;
}

int DirectoryAPI::Dir_Size(string path, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    else if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }

    auto pair = FindINodeByPath(path, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg);
    if (pair.first == -1 || pair.second == -1) {
        osErrMsg = E_DIR_SIZE;
        return -1;
    }
    INode& targetINode = workingDisk.inodes[pair.first][pair.second];
    osSuccessMsg = S_DIR_SIZE + " NUM OF BYTES: " + to_string(targetINode.fileSize);
    return targetINode.fileSize;
}

int DirectoryAPI::Dir_Read(string path, string& buffer, int size, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    else if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }

    if (path.empty() || path[0] != '/') {
        osErrMsg = E_INVALID_PATH;
        return -1; // Invalid path format
    }

    auto pair = FindINodeByPath(path, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg);
    if (pair.first == -1 || pair.second == -1) {
        osErrMsg = E_DIR_NOT_FOUND;
        return -1; // Directory not found
    }

    INode& dirINode = workingDisk.inodes[pair.first][pair.second];
    if (dirINode.type != "directory") {
        osErrMsg = E_NOT_DIRECTORY;
        return -1; // Not a directory
    }

    buffer.clear();
    for (int dataBlock : dirINode.dataBlocks) {
        string blockData;
        string diskErrMsg, diskSuccessMsg;

        if (DiskAPI::Disk_Read(dataBlock, blockData, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
            osErrMsg = diskErrMsg;
            return -1;
        }

        for (size_t i = 0; i < blockData.size(); i += 20) {
            if (i + 20 > blockData.size()) break;
            char entryName[16] = { 0 };
            int inodeNumber = 0;
            memcpy(entryName, blockData.c_str() + i, 16);
            memcpy(&inodeNumber, blockData.c_str() + i + 16, 4);
            if (buffer.size() + 20 > size) {
                osErrMsg = E_BUFFER_TOO_SMALL;
                return -1;
            }
            buffer.append(entryName, 16);
            buffer.append(reinterpret_cast<char*>(&inodeNumber), 4);
        }
    }

    osSuccessMsg = S_DIR_READ + " NUM OF ENTRIES: " + to_string(dirINode.fileSize) + " IN THE BUFFER: " + buffer;
    return dirINode.fileSize;
}

int DirectoryAPI::Dir_Unlink(string path, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg) {
    if (path == "/") {
        osErrMsg = E_DEL_ROOT_DIR;
        return -1;
    }

    auto inodePair = FindINodeByPath(path, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg);
    int inodeRow = inodePair.first;
    int inodeCol = inodePair.second;
    if (inodeRow == -1 || inodeCol == -1) {
        osErrMsg = E_DIR_UNLINK;
        return -1;
    }

    INode& targetINode = workingDisk.inodes[inodeRow][inodeCol];

    if (targetINode.type != "directory") {
        osErrMsg = E_DIR_UNLINK;
        return -1;
    }

    string buffer;
    for (int dataBlock : targetINode.dataBlocks) {
        if (DiskAPI::Disk_Read(dataBlock, buffer, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
            osErrMsg = diskErrMsg;
            return -1;
        }

        for (char c : buffer) {
            if (c != '\0') {  // Directory contains files or subdirectories
                osErrMsg = E_DIR_NOT_EMPTY;
                return -1;
            }
        }
    }

    string emptyBuffer(SECTOR_SIZE, '\0');
    for (int dataBlock : targetINode.dataBlocks) {
        if (DiskAPI::Disk_Write(dataBlock, emptyBuffer, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
            osErrMsg = diskErrMsg;
            return -1;
        }
        workingDisk.dbitmap[dataBlock] = false;
    }

    workingDisk.ibitmap[inodeRow * 10 + inodeCol] = false;
    targetINode = INode();

    size_t lastSlash = path.find_last_of('/');
    string parentPath = path.substr(0, lastSlash);
    string dirName = path.substr(lastSlash + 1);

    auto parentPair = FindINodeByPath(parentPath.empty() ? "/" : parentPath, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg);
    int parentRow = parentPair.first;
    int parentCol = parentPair.second;
    if (parentRow == -1 || parentCol == -1) {
        osErrMsg = "Parent directory not found";
        return -1;
    }

    INode& parentINode = workingDisk.inodes[parentRow][parentCol];
    for (int dataBlock : parentINode.dataBlocks) {
        if (DiskAPI::Disk_Read(dataBlock, buffer, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
            osErrMsg = diskErrMsg;
            return -1;
        }

        size_t entryPos = buffer.find(dirName);
        if (entryPos != string::npos) { 
            size_t newlinePos = buffer.find('\n', entryPos);
            if (newlinePos != string::npos) { 
                size_t lengthToReplace = newlinePos - entryPos + 1;
                buffer.replace(entryPos, lengthToReplace, string(lengthToReplace, '\0'));

                if (DiskAPI::Disk_Write(dataBlock, buffer, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, diskErrMsg, diskSuccessMsg) == -1) {
                    osErrMsg = diskErrMsg;
                    return -1;
                }
                parentINode.fileSize--;
                break;
            }
        }
    }
    osSuccessMsg = S_DIR_UNLINK;
    return 0;
}

pair<int, int> DirectoryAPI::FindINodeByPath(const string& path, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg)
{
    if (path.empty() || path[0] != '/') {
        osErrMsg = E_DIR_NOT_FOUND;
        return { -1, -1 };
    }

    vector<string> directories;
    stringstream ss(path);
    string segment;
    while (getline(ss, segment, '/')) {
        if (!segment.empty()) {
            directories.push_back(segment);
        }
    }

    int currentINodeRow = 0, currentINodeCol = 0;
    if (directories.empty()) {
        return { currentINodeRow, currentINodeCol };
    }

    string currentPath = "/";
    for (const string& dirName : directories) {
        bool found = false;
        for (int row = 0; row < workingDisk.inodes.size(); ++row) {
            for (int col = 0; col < workingDisk.inodes[row].size(); ++col) {
                INode& currentINode = workingDisk.inodes[row][col];
                if (currentINode.fileName == currentPath + dirName && currentINode.type == "directory") {
                    currentINodeRow = row;
                    currentINodeCol = col;
                    currentPath += dirName + "/";
                    found = true;
                    break;
                }
            }
            if (found) break;
        }

        if (!found) {
            osErrMsg = E_DIR_NOT_FOUND;
            return { -1, -1 };
        }
    }

    return { currentINodeRow, currentINodeCol };
}
