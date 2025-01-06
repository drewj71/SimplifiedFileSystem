#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

#include "StartupAPI.h"
#include "Structures.h"

using namespace std;

class DiskAPI {
public:
	static int Disk_Init(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, string& osErrMsg, string& osSuccessMsg);
	static int Disk_Load(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg);
	static int Disk_Save(Disk& externalDisk, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg);
	static int Disk_Write(int sector, string buffer, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& diskErrMsg, string& diskSuccessMsg);
	static int Disk_Read(int sector, string& buffer, Disk& workingDisk, bool& doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& diskErrMsg, string& diskSuccessMsg);
	static int AllocateINode(Disk& workingDisk);
	static int AllocateDataBlock(Disk& workingDisk);
	static pair<int, int> GetINode(const string fileName, Disk workingDisk, bool IsDirectory);

	static unsigned int GenerateUniqueID();

private:
};