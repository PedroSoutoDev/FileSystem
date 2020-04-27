/**************************************************************
* Class: CSC-415-0# Spring 2020
* Group Name: Last Minute
* Name: Pedro Souto
* Student ID: 918412864
* Name:
* Student ID:
* Name:
* Student ID:
* Name:
* Student ID:
*
* Project: Assignment 3 – File System
* File: fsImplementation.h
*
* Description: Header file containing all header functions implemented in the file sytem.
* **************************************************************/

#ifndef fsImplementation_h
#define fsImplementation_h
#include <stdio.h>
#endif /* fsImplementation_h */

// Default file extension name for a directory (NOT a file)
#define DIRECTORY_EXTENSION_NAME "DIRECTORY"

// Print menu of valid commands with a proper format
void printCommands(void);

// This will get the BLOCK NUMBERS of all directories
// Returns a pointer to an array of blocks containing directory entries
void* getBlockNumbersOfAllDirectories(uint16_t blockSize);

// This will get the DIRECTORY ENTRY struct of all directories
// Returns a pointer to an array of directoryEntry structs
void* getAllDirectoriesStructs(uint16_t blockSize);

// This will get A SINGLE DIRECTORY struct entry at a specific block
// Returns a pointer to the directory entry struct
void* getDirectoryEntryFromBlock(uint64_t directoryBlockNumber, uint16_t blockSize);

// * DO NOT CALL THIS *
// This is a helper functions, and should not be called directly. Use listDirectories() to list directories!
void listDirectoriesHelper(uint64_t parentDirectoryBlockNumber, uint16_t blockSize);

// This will print the sub directories of the given parent
// It takes in a parentDirectoryBlockNumber, which is the block of the directory you want to list FROM.
void listDirectories(uint64_t parentDirectoryBlockNumber, uint16_t blockSize);

// * DO NOT CALL THIS *
// This is a helper functions, and should not be called directly. Use listTree() to list directories in a tree format!
void listTreeHelper(uint64_t parentDirectoryBlockNumber, int directoryLevel, uint16_t blockSize);

// This will RECURSIVELY print directories. This ensure it will have a tree like format, which can be human readable
// It takes in a parentDirectoryBlockNumber, which is the block of the directory you want to list FROM.
// Always call this function with directoryLevel = 0
void listTree(uint64_t parentDirectoryBlockNumber, uint16_t blockSize);

// Prints the name of the current directory, used for pwd command
void displayCurrentDirectory(uint16_t blockSize);

// Takes a childDirectoryBlockNumber, and adds that block number to the indexLocation array of the parent directory, found in block parentDirectoryBlockNumber
// * Essentially, this links the parent to the child *
// Returns 1 on success
// Returns 0 on failure (not enough room in parent)
int addChildDirectoryIndexLocationToParent(uint64_t parentDirectoryBlockNumber, uint64_t childDirectoryBlockNumber, int16_t blockSize);

// * OLD FUNCTION. NO LONGER IN USE, I LEFT IT IN CASE WE NEED IT LATER *
// Create directory
// Returns the block where the directory struct was placed
uint64_t createDirectoryOLD(uint16_t blockSize);

// Create directory
// Returns the block where the directory struct was placed
uint64_t createDirectory(char* directoryName, uint64_t parentDirectoryBlockNumber, uint16_t blockSize);

// * DO NOT CALL THIS *
// Create the root directory, it is auto called when volume control block is created
void createRootDirectory(uint16_t blockSize);

// Add file
uint64_t createFileDirectory(char* fileName, char* fileExtension, uint64_t fileSize, uint64_t parentDirectoryBlockNumber, uint16_t blockSize);

// Remove file

// Copy file

// Move file

// Set metadata
void setMetaData(char* directoryPath, uint16_t newPermissions, uint16_t blockSize);

// Print metadata
void printDirectoryInfo(char* directoryPath, uint16_t blockSize);

// Copy from the normal filesystem to your filesystem

// Copy from your filesystem to the normal filesystem

// Initialize control block (block 0)
void initializeVolumeControlBlock(uint64_t volumeSize, char *volumeName, uint16_t blockSize);

// Returns the block in which the root directory structe is at
uint64_t getVCBRootDirectory(uint16_t blockSize);

// Returns the block of where our current directory is at
uint64_t getVCBCurrentDirectory(uint16_t blockSize);

// * DO NOT CALL THIS *
// Set the current directory block to this new directory. It will be auto called in changeDirectory()
void setVCBCurrentDirectory(uint64_t newDirectoryBlock, uint16_t blockSize);

// Use this function to change the current directory
// Can pass in a path followning the LINUX syntax (ex: "../Documents/Identification")
// elevated will either be 0 or 1. 0 mean not elevated, which means you CANNOT cd into a file. 1 means elevated, which means you CAN cd into a file
// Returns 1 if change was successfull, -1 is there directory does not exist, and -2 if there was an attemp to cd into a file
int changeDirectory(char* directoryPath, uint16_t elevated, uint16_t blockSize);

// * DO NOT CALL THIS *
// This function is called by change directory. This function expects a SINGLE child to change to. changeDirectory() is the one to call, which will parse the path and call this function as needed to change directories
// Returns 1 if change was successfull, -1 is there directory does not exist, and -2 if there was an attemp to cd into a file
int changeDirectoryHelper(char* directoryName, uint16_t elevated, uint16_t blockSize);

// Increase the number of directories counter by 1
void increaseVCBDirectoryCount(uint16_t blockSize);

// Decrease the number of directories counter by 1
void decreaseVCBDirectoryCount(uint16_t blockSize);

// Returns the number of directories of the file system
uint64_t getVCBDirectoryCount(uint16_t blockSize);

// Check if volume control block has been initialized before
// Returns 0 if control block has not been initialized
// Returns 1 if control block has been initialized
int hasVolumeControlBlock(uint16_t blockSize);

// Initialize free space information blocks (blocks 1-49)
void intializeFreeSpaceInformation(uint64_t volumeSize, int16_t blockSize);

// Get free space information struct
// Returns a pointer to the FreeSpaceInformation struct
void* getFreeSpaceInformation(int16_t blockSize);

// Sets the Kth bit in array A - Sets to 1 (Free)
// Source: http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
void setBit(int A[], uint64_t bit);

// Clears the Kth bit in array A -  sets to 0 (Used)
// Source: http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
void clearBit(int A[], uint64_t bit);

// Returns the Kth bit in array A
// Source: http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
int getBit(int A[], uint64_t bit);

// Finds free blocks in a given range. This is used when you want MULTIPLE blocks
// Returns a pointer to an array of ints, which represent the blocks that are free
void* findFreeLBABlocksInRange(uint64_t startIndex, uint64_t endIndex, int numBlocksNeeded, int16_t blockSize);

// Finds a single free blocks in a given range. This is used when you want ONLY ONE blocks
// Returns a single int to the free block
uint64_t findSingleFreeLBABlockInRange(uint64_t startIndex, uint64_t endIndex, int16_t blockSize);

// Set a single block as used. This is used when you want to set ONLY ONE block to used
void setBlockAsUsed(uint64_t blockNumber, int16_t blockSize);

// Returns the highest useable block in the LBA. Block may be USED or NOT USED
uint64_t getHighestUseableBlock(int16_t blockSize);

// Create DUMMY DATA for testing (adding, printing, removing, etc...)
void sampleCreateDirectories(int16_t blockSize);

// Properly closes and exits the file system
void exitFileSystem(int16_t blockSize);
