#pragma once

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include "DiskAPI.h"
#include "Structures.h"

using namespace std;

class StartupAPI {
public:
	static int FS_Boot(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, string& osErrMsg, string& osSuccessMsg);
	static int FS_Sync(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg);
	static int FS_Reset(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg);

private:
};

