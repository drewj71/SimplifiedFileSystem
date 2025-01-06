#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

#include "StartupAPI.h"
#include "FileAPI.h"
#include "DirectoryAPI.h"
#include "Structures.h"

using namespace std;

string trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\n\r\f\v");
	if (start == std::string::npos) {
		return "";
	}
	size_t end = str.find_last_not_of(" \t\n\r\f\v");
	return str.substr(start, end - start + 1);
}

void logMessages(string msg, ofstream& out1, ofstream& out2) {
	cout << msg << endl;
	out1 << msg << endl;
	out2 << msg << endl;
}

int main() {
	string manualOrFile, inputFile, outputFile1, outputFile2;
	ifstream checkFileExist;
	bool isManualEntry = false;

	Disk externalDisk;
	Disk workingDisk; 
	vector<FileBlock> openedFiles;

	bool doesDiskExist = false;
	bool isFileSystemAccessible = false;
	string osErrMsg; 
	string diskErrMsg;

	string osSuccessMsg;
	string diskSuccessMsg;

	cout << "Do you want to enter commands manually or enter an input file?\n";
	cout << "(Enter M for manual / Enter I for input file): ";
	while (true) {
		getline(cin, manualOrFile);
		if (manualOrFile == "M" || manualOrFile == "m") {
			isManualEntry = true;
			break;
		}
		else if (manualOrFile == "I" || manualOrFile == "i") {
			isManualEntry = false;
			break;
		}
	}

	cout << "Enter the name of the first output file: ";
	while (true) {
		getline(cin, outputFile1);
		checkFileExist.open(outputFile1);
		if (checkFileExist.is_open()) {
			cout << "\nInput file with the name: " << outputFile1 << " already exists. Please enter a new output file name: ";
			checkFileExist.close();
		}
		else {
			break;
		}
	}

	cout << "Enter the name of the second output file: ";
	while (true) {
		getline(cin, outputFile2);
		checkFileExist.open(outputFile2);
		if (checkFileExist.is_open()) {
			cout << "\nInput file with the name: " << outputFile2 << " already exists. Please enter a new output file name: ";
			checkFileExist.close();
		}
		else {
			break;
		}
	}

	ofstream out1(outputFile1);
	ofstream out2(outputFile2);

	if (!isManualEntry) {
		cout << "Enter the name of the input file: ";
		while (true) {
			getline(cin, inputFile);
			checkFileExist.open(inputFile);
			if (!checkFileExist.is_open()) {
				cout << "\nInput file with the name: " << inputFile << " wasn't found. Please enter the correct input file name: ";
			}
			else {
				checkFileExist.close();
				break;
			}
		}
		ifstream input(inputFile);
		string line;
		while (getline(input, line)) { 
			string buffer;
			istringstream iss(line);
			string functionName;
			iss >> functionName;
			vector<string> parameters;
			string parameter;
			while (iss >> parameter) {
				parameters.push_back(parameter);
			}

			if (functionName == "BOOT") {
				if (StartupAPI::FS_Boot(externalDisk, workingDisk, doesDiskExist, osErrMsg, osSuccessMsg) == 0) {
					isFileSystemAccessible = true;
					logMessages(osSuccessMsg, out1, out2);
				}
				else
					logMessages(osErrMsg, out1, out2);
			}
			else if (functionName == "SYNC") {
				if (StartupAPI::FS_Sync(externalDisk, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg) == 0)
					logMessages(osSuccessMsg, out1, out2);
				else
					logMessages(osErrMsg, out1, out2);
			}
			else if (functionName == "RESET") {
				if (StartupAPI::FS_Reset(externalDisk, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg) == 0) {
					isFileSystemAccessible = false;
					logMessages(osSuccessMsg, out1, out2);
				}
				else
					logMessages(osErrMsg, out1, out2);
			}
			else if (functionName == "DIR_CREATE") {
				if (parameters.size() == 1) {
					if (DirectoryAPI::Dir_Create(parameters[0], workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) == 0)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - PATH - FOR DIR_CREATE COMMAND!!!";
				}
			}
			else if (functionName == "DIR_SIZE") {
				if (parameters.size() == 1) {
					int size = DirectoryAPI::Dir_Size(parameters[0], workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg);
					if (size > -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - PATH - FOR DIR_SIZE COMMAND!!!";
				}
			}
			else if (functionName == "DIR_READ") {
				if (parameters.size() == 2) {
					int numEntries = DirectoryAPI::Dir_Read(parameters[0], buffer, stoi(parameters[1]), workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg);
					if (numEntries > -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - PATH & SIZE - FOR DIR_READ COMMAND!!!";
				}
			}
			else if (functionName == "DIR_UNLINK") {
				if (parameters.size() == 1) {
					if (DirectoryAPI::Dir_Unlink(parameters[0], workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) == 0)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - PATH - FOR DIR_UNLINK COMMAND!!!";
				}
			}
			else if (functionName == "FILE_CREATE") {
				if (parameters.size() == 1) {
					if (FileAPI::File_Create(parameters[0], workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) == 0)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FILE - FOR FILE_CREATE COMMAND!!!";
				}
			}
			else if (functionName == "FILE_OPEN") {
				if (parameters.size() == 1) {
					if (FileAPI::File_Open(parameters[0], workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) != -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FILE - FOR FILE_OPEN COMMAND!!!";
				}
			}
			else if (functionName == "FILE_READ") {
				if (parameters.size() == 2) {
					if (FileAPI::File_Read(stoi(parameters[0]), buffer, stoi(parameters[1]), workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) != -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FD & SIZE - FOR FILE_READ COMMAND!!!";
				}
			}
			else if (functionName == "FILE_WRITE") {
				if (parameters.size() >= 3) {
					int paramsSize = parameters.size();
					ostringstream combinedWrite;
					for (int i = 1; i < parameters.size() - 1; i++) {
						combinedWrite << parameters[i] << " ";
					}
					parameters[1] = trim(combinedWrite.str());
					parameters.erase(parameters.begin() + 2, parameters.end() - 1);
					if (FileAPI::File_Write(stoi(parameters[0]), parameters[1], parameters[1].size(), workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) != -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FD & BUFFER & SIZE - FOR FILE_WRITE COMMAND!!!";
				}
			}
			else if (functionName == "FILE_SEEK") {
				if (parameters.size() == 2) {
					if (FileAPI::File_Seek(stoi(parameters[0]), stoi(parameters[1]), workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) != -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FD & OFFSET - FOR FILE_SEEK COMMAND!!!";
				}
			}
			else if (functionName == "FILE_CLOSE") {
				if (parameters.size() == 1) {
					if (FileAPI::File_Close(stoi(parameters[0]), workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) == 0)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FD - FOR FILE_CLOSE COMMAND!!!";
				}
			}
			else if (functionName == "FILE_UNLINK") {
				if (parameters.size() == 1) {
					if (FileAPI::File_Unlink(parameters[0], workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) == 0)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FILE - FOR FILE_UNLINK COMMAND!!!";
				}
			}
			else if (functionName == "DUMP_EXTERNAL") {
				out2 << "-----------------------------------------\n";
				out2 << "EXTERNAL DISK DUMP\n\n";
				out2 << "SUPER BLOCK: " + to_string(externalDisk.superBlock) + "\n\n";
				out2 << "INODES: \n";
				for (int i = 0; i < externalDisk.inodes.size(); i++) {
					for (int j = 0; j < externalDisk.inodes[i].size(); j++) {
						out2 << "INDEX: " + to_string(i) + to_string(j) + " -- TYPE: " + externalDisk.inodes[i][j].type + " -- SIZE: " + to_string(externalDisk.inodes[i][j].fileSize) + " -- NAME: " + externalDisk.inodes[i][j].fileName + " -- DATA BLOCKS: ";
						for (int k = 0; k < externalDisk.inodes[i][j].dataBlocks.size(); k++) {
							out2 << to_string(externalDisk.inodes[i][j].dataBlocks[k]) << " ";
						}
						out2 << "\n";
					}
					out2 << "\n";
				}
				out2 << "IBITMAP: \n";
				for (int i = 0; i < externalDisk.ibitmap.size(); i++) {
					out2 << "INDEX: " + to_string(i) + " " + to_string(externalDisk.ibitmap[i]) + " -- ";
				}
				out2 << "DBITMAP: \n";
				for (int i = 0; i < externalDisk.dbitmap.size(); i++) {
					out2 << "INDEX: " + to_string(i) + " " + to_string(externalDisk.dbitmap[i]) + " -- ";
				}
				out2 << "DATA: \n";
				for (int i = 0; i < externalDisk.data.size(); i++) {
					for (int j = 0; j < externalDisk.data[i].size(); j++) {
						out2 << externalDisk.data[i][j] << " ";
					}
					out2 << "\n";
				}
			}
			else if (functionName == "DUMP_WORKING") {
				out2 << "-----------------------------------------\n";
				out2 << "WORKING DISK DUMP\n\n";
				out2 << "SUPER BLOCK: " + to_string(workingDisk.superBlock) + "\n\n";
				out2 << "INODES: \n";
				for (int i = 0; i < workingDisk.inodes.size(); i++) {
					for (int j = 0; j < workingDisk.inodes[i].size(); j++) {
						out2 << "INDEX: " + to_string(i) + to_string(j) + " -- TYPE: " + workingDisk.inodes[i][j].type + " -- SIZE: " + to_string(workingDisk.inodes[i][j].fileSize) + " -- NAME: " + workingDisk.inodes[i][j].fileName + " -- DATA BLOCKS: ";
						for (int k = 0; k < workingDisk.inodes[i][j].dataBlocks.size(); k++) {
							out2 << to_string(externalDisk.inodes[i][j].dataBlocks[k]) << " ";
						}
						out2 << "\n";
					}
					out2 << "\n";
				}
				out2 << "\n\n\nIBITMAP: \n";
				for (int i = 0; i < workingDisk.ibitmap.size(); i++) {
					out2 << "INDEX: " + to_string(i) + " " + to_string(workingDisk.ibitmap[i]) + " -- ";
				}
				out2 << "\n\n\nDBITMAP: \n";
				for (int i = 0; i < workingDisk.dbitmap.size(); i++) {
					out2 << "INDEX: " + to_string(i) + " " + to_string(workingDisk.dbitmap[i]) + " -- ";
				}
				out2 << "\n\n\nDATA: \n";
				for (int i = 0; i < workingDisk.data.size(); i++) {
					for (int j = 0; j < workingDisk.data[i].size(); j++) {
						out2 << workingDisk.data[i][j] << " ";
					}
					out2 << "\n";
				}
			}
			else {
				cout << "UNKNOWN COMMAND! PLEASE TRY AGAIN: ";
			}
		}
		input.close();
	}
	else if (isManualEntry) {
		string userInput;
		while (userInput != "QUIT") {
			cout << "\nEnter HELP to see the commands!\nEnter QUIT to stop the program!\nPlease enter a command: ";
			getline(cin, userInput);
			string buffer;
			istringstream iss(userInput);
			string functionName;
			iss >> functionName;
			vector<string> parameters;
			string parameter;
			while (iss >> parameter) {
				parameters.push_back(parameter);
			}

			if (functionName == "HELP") {
				cout << "BOOT\nSYNC\nRESET\nDIR_CREATE {path}\nDIR_SIZE {path}\nDIR_READ {path}\nDIR_UNLINK {path}\nFILE_CREATE {file}\n";
				cout << "FILE_OPEN {file}\nFILE_READ {file descriptor} {size}\nFILE_WRITE {file descriptor} {buffer}\n";
				cout << "FILE_SEEK {file descriptor} {offset}\nFILE_CLOSE {file descriptor}\nFILE_UNLINK {file}\n\n";
			}
			else if (functionName == "QUIT") {
				break;
			}
			else if (functionName == "BOOT") {
				if (StartupAPI::FS_Boot(externalDisk, workingDisk, doesDiskExist, osErrMsg, osSuccessMsg) == 0) {
					isFileSystemAccessible = true;
					logMessages(osSuccessMsg, out1, out2);
				}
				else
					logMessages(osErrMsg, out1, out2);
			}
			else if (functionName == "SYNC") {
				if (StartupAPI::FS_Sync(externalDisk, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg) == 0)
					logMessages(osSuccessMsg, out1, out2);
				else
					logMessages(osErrMsg, out1, out2);
			}
			else if (functionName == "RESET") {
				if (StartupAPI::FS_Reset(externalDisk, workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg) == 0) {
					isFileSystemAccessible = false;
					logMessages(osSuccessMsg, out1, out2);
				}
				else
					logMessages(osErrMsg, out1, out2);
			}
			else if (functionName == "DIR_CREATE") {
				if (parameters.size() == 1) {
					if (DirectoryAPI::Dir_Create(parameters[0], workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) == 0)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - PATH - FOR DIR_CREATE COMMAND!!!";
				}
			}
			else if (functionName == "DIR_SIZE") {
				if (parameters.size() == 1) {
					int size = DirectoryAPI::Dir_Size(parameters[0], workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg);
					if (size > -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - PATH - FOR DIR_SIZE COMMAND!!!";
				}
			}
			else if (functionName == "DIR_READ") { 
				if (parameters.size() == 2) {
					int numEntries = DirectoryAPI::Dir_Read(parameters[0], buffer, stoi(parameters[1]), workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg);
					if (numEntries > -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - PATH & SIZE - FOR DIR_READ COMMAND!!!";
				}
			}
			else if (functionName == "DIR_UNLINK") {
				if (parameters.size() == 1) {
					if (DirectoryAPI::Dir_Unlink(parameters[0], workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) == 0)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - PATH - FOR DIR_UNLINK COMMAND!!!";
				}
			}
			else if (functionName == "FILE_CREATE") {
				if (parameters.size() == 1) {
					if (FileAPI::File_Create(parameters[0], workingDisk, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) == 0)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FILE - FOR FILE_CREATE COMMAND!!!";
				}
			}
			else if (functionName == "FILE_OPEN") {
				if (parameters.size() == 1) {
					if (FileAPI::File_Open(parameters[0], workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) != -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FILE - FOR FILE_OPEN COMMAND!!!";
				}
			}
			else if (functionName == "FILE_READ") {
				if (parameters.size() == 2) {
					if (FileAPI::File_Read(stoi(parameters[0]), buffer, stoi(parameters[1]), workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) != -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FD & SIZE - FOR FILE_READ COMMAND!!!";
				}
			}
			else if (functionName == "FILE_WRITE") {
				if (parameters.size() > 1) {
					int paramsSize = parameters.size();
					ostringstream combinedWrite;
					for (int i = 1; i < parameters.size() - 1; i++) {
						combinedWrite << parameters[i] << " ";
					}
					parameters[1] = trim(combinedWrite.str());
					parameters.erase(parameters.begin() + 2, parameters.end());
					if (FileAPI::File_Write(stoi(parameters[0]), parameters[1], parameters[1].size(), workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) != -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FD & BUFFER & SIZE - FOR FILE_WRITE COMMAND!!!";
				}
			}
			else if (functionName == "FILE_SEEK") {
				if (parameters.size() == 2) {
					if (FileAPI::File_Seek(stoi(parameters[0]), stoi(parameters[1]), workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) != -1)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FD & OFFSET - FOR FILE_SEEK COMMAND!!!";
				}
			}
			else if (functionName == "FILE_CLOSE") {
				if (parameters.size() == 1) {
					if (FileAPI::File_Close(stoi(parameters[0]), workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) == 0)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FD - FOR FILE_CLOSE COMMAND!!!";
				}
			}
			else if (functionName == "FILE_UNLINK") {
				if (parameters.size() == 1) {
					if (FileAPI::File_Unlink(parameters[0], workingDisk, openedFiles, doesDiskExist, isFileSystemAccessible, osErrMsg, osSuccessMsg, diskErrMsg, diskSuccessMsg) == 0)
						logMessages(osSuccessMsg, out1, out2);
					else
						logMessages(osErrMsg, out1, out2);
				}
				else {
					cout << "INVALID PARAMETERS - FILE - FOR FILE_UNLINK COMMAND!!!";
				}
			}
			else if (functionName == "DUMP_EXTERNAL") {
				out2 << "-----------------------------------------\n";
				out2 << "EXTERNAL DISK DUMP\n\n";
				out2 << "SUPER BLOCK: " + to_string(externalDisk.superBlock) + "\n\n";
				out2 << "INODES: \n";
				for (int i = 0; i < externalDisk.inodes.size(); i++) {
					for (int j = 0; j < externalDisk.inodes[i].size(); j++) {
						out2 << "INDEX: " + to_string(i) + to_string(j) + " -- TYPE: " + externalDisk.inodes[i][j].type + " -- SIZE: " + to_string(externalDisk.inodes[i][j].fileSize) + " -- NAME: " + externalDisk.inodes[i][j].fileName + " -- DATA BLOCKS: ";
						for (int k = 0; k < externalDisk.inodes[i][j].dataBlocks.size(); k++) {
							out2 << to_string(externalDisk.inodes[i][j].dataBlocks[k]) << " ";
						}
						out2 << "\n";
					}
					out2 << "\n";
				}
				out2 << "IBITMAP: \n";
				for (int i = 0; i < externalDisk.ibitmap.size(); i++) {
					out2 << "INDEX: " + to_string(i) + " " + to_string(externalDisk.ibitmap[i]) + " -- ";
				}
				out2 << "DBITMAP: \n";
				for (int i = 0; i < externalDisk.dbitmap.size(); i++) {
					out2 << "INDEX: " + to_string(i) + " " + to_string(externalDisk.dbitmap[i]) + " -- ";
				}
				out2 << "DATA: \n";
				for (int i = 0; i < externalDisk.data.size(); i++) {
					for (int j = 0; j < externalDisk.data[i].size(); j++) {
						out2 << externalDisk.data[i][j] << " ";
					}
					out2 << "\n";
				}
			}
			else if (functionName == "DUMP_WORKING") {
				out2 << "-----------------------------------------\n";
				out2 << "WORKING DISK DUMP\n\n";
				out2 << "SUPER BLOCK: " + to_string(workingDisk.superBlock) + "\n\n";
				out2 << "INODES: \n";
				for (int i = 0; i < workingDisk.inodes.size(); i++) {
					for (int j = 0; j < workingDisk.inodes[i].size(); j++) {
						out2 << "INDEX: " + to_string(i) + to_string(j) + " -- TYPE: " + workingDisk.inodes[i][j].type + " -- SIZE: " + to_string(workingDisk.inodes[i][j].fileSize) + " -- NAME: " + workingDisk.inodes[i][j].fileName + " -- DATA BLOCKS: ";
						for (int k = 0; k < workingDisk.inodes[i][j].dataBlocks.size(); k++) {
							out2 << to_string(externalDisk.inodes[i][j].dataBlocks[k]) << " ";
						}
						out2 << "\n";
					}
					out2 << "\n";
				}
				out2 << "\nIBITMAP: \n";
				for (int i = 0; i < workingDisk.ibitmap.size(); i++) {
					out2 << "INDEX: " + to_string(i) + " " + to_string(workingDisk.ibitmap[i]) + " -- ";
				}
				out2 << "\nDBITMAP: \n";
				for (int i = 0; i < workingDisk.dbitmap.size(); i++) {
					out2 << "INDEX: " + to_string(i) + " " + to_string(workingDisk.dbitmap[i]) + " -- ";
				}
				out2 << "\nDATA: \n";
				for (int i = 0; i < workingDisk.data.size(); i++) {
					for (int j = 0; j < workingDisk.data[i].size(); j++) {
						out2 << workingDisk.data[i][j] << " ";
					}
					out2 << "\n";
				}
			}
			else {
				cout << "UNKNOWN COMMAND! PLEASE TRY AGAIN: ";
			}
		}
	}
	out1.close();
	out2.close();
	return 0;
}