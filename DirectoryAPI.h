#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <tuple>

#include "Structures.h"
#include "DiskAPI.h"

using namespace std;

class DirectoryAPI {
public:
	static int Dir_Create(string path, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);
	static int Dir_Size(string path, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);
	static int Dir_Read(string path, string& buffer, int size, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);
	static int Dir_Unlink(string path, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);

private:
	static pair<int, int> FindINodeByPath(const string& path, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);
};

