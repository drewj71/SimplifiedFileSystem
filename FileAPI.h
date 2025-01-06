#pragma once

#include <iostream>
#include <vector>
#include <tuple>
#include <string>
#include <sstream>

#include "Structures.h"
#include "DiskAPI.h"
#include "DirectoryAPI.h"

using namespace std;

class FileAPI {
public:
	static int File_Create(string file, Disk& workingDisk, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);
	static int File_Open(string file, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);
	static int File_Read(int fd, string& buffer, int size, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);
	static int File_Write(int fd, string buffer, int size, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);
	static int File_Seek(int fd, int offset, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);
	static int File_Close(int fd, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);
	static int File_Unlink(string file, Disk& workingDisk, vector<FileBlock>& openedFiles, bool doesDiskExist, bool isFileSystemAccessible, string& osErrMsg, string& osSuccessMsg, string& diskErrMsg, string& diskSuccessMsg);

private:
};

