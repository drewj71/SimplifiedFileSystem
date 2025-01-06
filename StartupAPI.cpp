#include "StartupAPI.h"

int StartupAPI::FS_Boot(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, string& osErrMsg, string& osSuccessMsg)
{
    if (!doesDiskExist) {
        osSuccessMsg = S_FILE_BOOT;
        return DiskAPI::Disk_Init(externalDisk, workingDisk, doesDiskExist, osErrMsg, osSuccessMsg);
    }
    osSuccessMsg = S_FILE_BOOT;
    return DiskAPI::Disk_Load(externalDisk, workingDisk, doesDiskExist, true, osErrMsg);
}

int StartupAPI::FS_Sync(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg)
{
    osSuccessMsg = S_FILE_SYNC;
    return DiskAPI::Disk_Save(externalDisk, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg);
}

int StartupAPI::FS_Reset(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg)
{
    if (!isFileSystemAccessible) {
        osErrMsg = E_INVALID_ACCESS_ATTEMPT;
        return -1;
    }
    else if (!doesDiskExist) {
        osErrMsg = E_FILE_BOOT;
        return -1;
    }
    else
        FS_Sync(externalDisk, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg);

    osSuccessMsg = S_FILE_RESET;
    return 0;
}