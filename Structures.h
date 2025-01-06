#pragma once

#include <iostream>
#include <string>
#include <vector>

using namespace std;

const static int SECTOR_SIZE = 512;
const static int NUM_SECTORS = 1000;
const static int DISK_SIZE = SECTOR_SIZE * NUM_SECTORS;

const static int MAX_FILE_SIZE = SECTOR_SIZE * 10;

const string S_FILE_BOOT = "Success: [S_FILE_BOOT] - File system booted successfully!";
const string S_FILE_SYNC = "Success: [S_FILE_SYNC] - File system synced successfully!";
const string S_FILE_RESET = "Success: [S_FILE_RESET] - File system reset successfully!";
const string S_DISK_INIT = "Success: [S_DISK_INIT] - Disk initialized successfully!";
const string S_DISK_LOAD = "Success: [S_DISK_LOAD] - Disk loaded successfully!";
const string S_DISK_SAVE = "Success: [S_DISK_SAVE] - Disk saved successfully!";
const string S_DISK_WRITE = "Success: [S_DISK_WRITE] - Disk wrote successfully!";
const string S_DISK_READ = "Success: [S_DISK_READ] - Disk read successfully!";
const string S_DIR_CREATE = "Success: [S_DIR_CREATE] - Directory created successfully!";
const string S_DIR_SIZE = "Success: [S_DIR_CREATE] - Directory size returned successfully: ";
const string S_DIR_READ = "Success: [S_DIR_READ] - Directory read successfully: ";
const string S_DIR_UNLINK = "Success: [S_DIR_UNLINK] - Directory unlink successfully!";
const string S_FILE_CREATE = "Success: [S_FILE_CREATE] - File created successfully!";
const string S_FILE_OPEN = "Sucess: [S_FILE_OPEN] - File opened successfully!";
const string S_FILE_READ = "Success [S_FILE_READ] - File read successfully!";
const string S_FILE_WRITE = "Success [S_FILE_WRITE] - File wrote successfully!";
const string S_FILE_SEEK = "Success [S_FILE_SEEK] - File seeked successfully!";
const string S_FILE_CLOSE = "Success [S_FILE_CLOSE] - File closed successfully!";
const string S_FILE_UNLINK = "Success [S_FILE_UNLINK] - File deleted successfully!";

const string E_FILE_BOOT = "Error: [E_FILE_BOOT] - File system boot failed.";
const string E_FILE_SYNC = "Error: [E_FILE_SYNC] - File system sync failed.";
const string E_FILE_RESET = "Error: [E_FILE_RESET] - File system reset failed.";
const string E_INVALID_ACCESS_ATTEMPT = "Error [E_INVALID_ACCESS_ATTEMPT] - File system not available.";
const string E_DISK_INIT = "Error: [E_DISK_INIT] - Disk initialization failed.";
const string E_DISK_LOAD = "Error: [E_DISK_LOAD] - Disk load failed.";
const string E_DISK_SAVE = "Error: [E_DISK_SAVE] - Disk save failed.";
const string E_WRITE_INVALID_PARAM = "Error: [E_DISK_WRITE] - Disk write failed.";
const string E_READ_INVALID_PARAM = "Error: [E_DISK_READ] - Disk read failed.";
const string E_DIR_CREATE = "Error: [E_DIR_CREATE] - Directory create failed.";
const string E_DIR_SIZE = "Error: [E_DIR_SIZE] - Directory not found.";
const string E_BUFFER_TOO_SMALL = "Error: [E_BUFFER_TOO_SMALL] - Directory read failed. Buffer too small.";
const string E_DIR_UNLINK = "Error: [E_DIR_UNLINK] - Directory unlink failed.";
const string E_DIR_NOT_EMPTY = "Error: [E_DIR_UNLINK] - Directory unlink failed. Directory not empty.";
const string E_DEL_ROOT_DIR = "Error: [E_DEL_ROOT_DIR] - Directory unlink failed. Cannot delete root.";
const string E_FILE_CREATE = "Error: [E_FILE_CREATE] - File creation failed.";
const string E_NO_SUCH_FILE = "Error: [E_NO_SUCH_FILE] - No file with this path to open.";
const string E_TOO_MANY_OPEN_FILES = "Error: [E_TOO_MANY_OPEN_FILES] - Cannot open file, to many open files.";
const string E_FILE_ALREADY_OPEN = "Error: [E_FILE_ALREADY_OPEN] - File is already open!";
const string E_READ_BAD_FD = "Error: [E_READ_BAD_FD] - File not open, cannot read.";
const string E_BAD_FD = "Error: [E_BAD_FD] - File not open, cannot write.";
const string E_NO_SPACE = "Error: [E_NO_SPACE] - File write cannot complete, no space.";
const string E_FILE_TOO_BIG = "Error: [E_FILE_TOO_BIG] - File exceeds maximum file size.";
const string E_SEEK_OUT_OF_BOUNDS = "Error: [E_SEEK_OUT_OF_BOUNDS] - File seek offset value is out of bounds.";
const string E_SEEK_BAD_FD = "Error: [E_SEEK_BAD_FD] - File not open, cannot seek.";
const string E_CLOSE_BAD_FD = "Error: [E_CLOSE_BAD_FD] - File not open, cannot close.";
const string E_FILE_IN_USE = "Error: [E_FILE_IN_USE] - File cannot be unlinked, file is in use.";
const string E_INVALID_PATH = "Error: [E_INVALID_PATH] - Invalid file path format.";
const string E_DIR_NOT_FOUND = "Error: [E_DIR_NOT_FOUND] - Directory not found.";
const string E_NOT_DIRECTORY = "Error: [E_NOT_DIRECTORY] - Not a directory.";
const string E_FILE_OPEN_READ_FAILED = "Error: [E_FILE_OPEN_READ_FAILED] - File open failed.";

struct INode {
	int fileSize;
	string type;
	string fileName;
	vector<int> dataBlocks;
	bool isFileOpen;

	INode() : fileSize(0), type(""), dataBlocks(), isFileOpen(false) {}
	INode(int f, string t, string n, vector<int> d, bool o) : fileSize(f), type(t), fileName(n), dataBlocks(d), isFileOpen(o) {}
};

struct FileBlock {
	int inodeIdx;
	string fileName;
	int filePointer;

	FileBlock() : inodeIdx(-1), fileName(""), filePointer(0) {}
	FileBlock(int i, string n, int p) : inodeIdx(i), fileName(n), filePointer(p) {}
};

struct Disk {
	int superBlock;
	vector<bool> ibitmap;
	vector<bool> dbitmap;
	vector<vector<INode>> inodes;
	vector<vector<char>> data;

	Disk() {
		superBlock = NULL;
		ibitmap = vector<bool>(100, false); // Only 100 INodes possible
		dbitmap = vector<bool>(NUM_SECTORS - 13, false);
		inodes = vector<vector<INode>>(10, vector<INode>(10)); // 10 by 10 INodes allowed
		data = vector<vector<char>>(NUM_SECTORS - 13, vector<char>(SECTOR_SIZE));
	}
};