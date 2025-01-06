#include "DiskAPI.h"

unsigned int DiskAPI::GenerateUniqueID()
{
	srand(time(0));
	return rand();
}

int DiskAPI::Disk_Init(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, string& osErrMsg, string& osSuccessMsg)
{
    if (doesDiskExist) {
        if (workingDisk.superBlock != externalDisk.superBlock) {
            osErrMsg = E_FILE_BOOT;
            return -1;
        }

        StartupAPI::FS_Sync(externalDisk, workingDisk, doesDiskExist, true, osErrMsg, osSuccessMsg);
    }
    else if (!doesDiskExist) {
        unsigned int uniqueID = GenerateUniqueID();
        externalDisk.superBlock = uniqueID;
        workingDisk.superBlock = uniqueID;
        doesDiskExist = true;
        for (int i = 0; i < workingDisk.ibitmap.size(); i++) {
            workingDisk.ibitmap[i] = false;
            workingDisk.dbitmap[i] = false;
        }
        workingDisk.inodes[0][0] = INode(0, "directory", "/", { 0 }, false);
        workingDisk.ibitmap[0] = true;
        workingDisk.dbitmap[0] = true;
    }
    return 0;
}

int DiskAPI::Disk_Load(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    else if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }
    else {
        if (externalDisk.superBlock == workingDisk.superBlock)
            workingDisk = externalDisk;
        else {
            osErrMsg = E_FILE_BOOT;
            return -1;
        }
    }
	return 0;
}

int DiskAPI::Disk_Save(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    else if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }
    else {
        if (externalDisk.superBlock == workingDisk.superBlock)
            externalDisk = workingDisk;
        else {
            osErrMsg = E_FILE_RESET;
            return -1;
        }
    }
    return 0;
}

int DiskAPI::Disk_Write(int sector, string buffer, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& diskErrMsg, string& diskSuccessMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    else if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }

    if (sector < 0 || sector >= NUM_SECTORS || buffer == "" || buffer.length() > 512 || !doesDiskExist || !isFileSystemAccessible) {
        diskErrMsg = E_WRITE_INVALID_PARAM;
        return -1;
    }

    copy(buffer.begin(), buffer.end(), workingDisk.data[sector].begin());
    diskSuccessMsg = S_DISK_WRITE;
	return 0;
}

int DiskAPI::Disk_Read(int sector, string& buffer, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& diskErrMsg, string& diskSuccessMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    else if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }
    if (sector < 0 || sector >= NUM_SECTORS || !doesDiskExist || !isFileSystemAccessible) {
        diskErrMsg = E_READ_INVALID_PARAM;
        return -1;
    }

    buffer = string(workingDisk.data[sector].begin(), workingDisk.data[sector].end());
    diskSuccessMsg = S_DISK_READ;
    return 0;
}

int DiskAPI::AllocateINode(Disk& workingDisk)
{
    for (size_t i = 0; i < workingDisk.ibitmap.size(); ++i) {
        if (!workingDisk.ibitmap[i]) {
            workingDisk.ibitmap[i] = true; // Mark inode as allocated
            return static_cast<int>(i);
        }
    }
    return -1; // No free inodes
}

int DiskAPI::AllocateDataBlock(Disk& workingDisk)
{
    for (size_t i = 0; i < workingDisk.dbitmap.size(); ++i) {
        if (!workingDisk.dbitmap[i]) {
            workingDisk.dbitmap[i] = true; // Mark data block as allocated
            return static_cast<int>(i);
        }
    }
    return -1; // No free data blocks
}

pair<int, int> DiskAPI::GetINode(const string fileName, Disk workingDisk, bool IsDirectory)
{
    for (int i = 0; i < workingDisk.inodes.size(); ++i) {
        for (int j = 0; j < workingDisk.inodes[i].size(); ++j) {
            INode& inode = workingDisk.inodes[i][j];
            if ((IsDirectory && inode.type == "directory" && inode.fileName == fileName) || (!IsDirectory && inode.type == "file" && inode.fileName == fileName)) {
                return { i, j };
            }
        }
    }
    return { -1, -1 };
}
