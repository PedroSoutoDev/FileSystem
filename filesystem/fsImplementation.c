/**************************************************************
* Class: CSC-415-0# Spring 2020
* Group Name: Last Minute
* Name: Pedro Souto
* Student ID: 918412864
* Name: Cassie Sherman
* Student ID: 918192878
* Name: Aaron Schlichting
* Student ID: 917930213
* Name: Troy Turner
* Student ID: 918053544
*
* Project: Assignment 3 – File System
* File: fsImplementation.c
*
* Description: Implementation of all functionalities the file system uses.
* **************************************************************/

// Required C libraries
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Bierman Files
#include "fsLow.h"

// Group Files
#include "fsStructures.h"
#include "fsImplementation.h"

void printCommands(){
    printf("-------------------------------------------------------------------------------\n");
    printf("%-30s","List Directories:"); printf("'ls'\n");
    printf("%-30s","Display Tree:"); printf("'tree'\n");
    printf("%-30s","Change Directories:"); printf("'cd <directory path>''\n");
    printf("%-30s","Display Working Directory:"); printf("'pwd'\n");
    printf("%-30s","Display Directory Info:"); printf("'info <directory path>'\n");
    printf("%-30s","Create Directoty:"); printf("'makedir <directory name>'\n");
    printf("%-30s","Move Directory:"); printf("'movedir <source directory path> <destination directory path>'\n");
    printf("\n");
    printf("%-30s","Create File:"); printf("'makefile <file name> <file extension>\n");
    printf("%-30s","Remove File:"); printf("'removefile <file path>\n");
    printf("%-30s","Copy File:"); printf("'copyfile <source file path> <destination directory path>'\n");
    printf("%-30s","Move File:"); printf("'movefile <source file path> <destination directory path>'\n");
    printf("%-30s","Set File Permission:"); printf("'chmod <file path> <file permission>'\n");
    printf("\n");
    printf("%-30s","Copy From Linux Filesystem:"); printf("'copyfrom <source file path> <destination file path>'\n");
    printf("%-30s","Copy To Linux Filesystem:"); printf("'copyto <source file path> <destination file path>'\n");
    printf("\n");
    printf("%-30s","See Commands Again:"); printf("'commands' or 'c'\n");
    printf("%-30s","Clear Console:"); printf("'clear'\n");
    printf("%-30s","Exit:"); printf("'exit' or 'e'\n");
    printf("-------------------------------------------------------------------------------\n\n");
}

void initializeVolumeControlBlock(uint64_t volumeSize, char *volumeName, uint16_t blockSize) {
    // Allocate memory for struct
    struct volumeControlBlock *vcb = malloc(blockSize);
    
    // Set correct parameters
    vcb->volumeSize = volumeSize;
    vcb->rootDirectory = 50;
    vcb->currentDirectory = vcb->rootDirectory;
    vcb->numberOfDirectories = 1;
    strcpy(vcb->volumeName, volumeName);
    vcb->blockSize = blockSize;
    vcb-> numberOfBlocks = (volumeSize / blockSize);

    // Generate and set a random volumeID
    srand((unsigned int)time(0));
    uint32_t ID = rand();
    vcb->volumeID = ID;
    
    // Print information about the newly created VCB
    printf("-------------------------------------------------------------------------------\n");
    printf("CREATING VOLUME CONTROL BLOCK...\n");
    printf("Volume Name: %s\n", vcb->volumeName);
    printf("Volume ID: %u\n", vcb->volumeID);
    printf("Volume Size: %llu\n", vcb->volumeSize);
    printf("Number of Directories: %llu\n", vcb->numberOfDirectories);
    printf("Volume LBA Block Size: %llu\n", vcb->blockSize);
    printf("Number of LBA Blocks: %llu\n", vcb->numberOfBlocks);
    printf("Root Directory Block: %llu\n", vcb->rootDirectory);
    printf("VOLUME CONTROL BLOCK CREATED SUCCESSFULLY!\n");
    printf("-------------------------------------------------------------------------------\n\n");

    // Write VCB to LBA block 0
    LBAwrite(vcb, 1, 0);
    
    // Since this is the first time this partition is being created, we must also intialize the free space information blocks
    intializeFreeSpaceInformation(volumeSize, blockSize);
    
    // Since this is the first time this partition is being created, we must also intialize a root directory
    createRootDirectory(blockSize);
    
    // Cleanup
    free(vcb);
}

int hasVolumeControlBlock(uint16_t blockSize) {
    // Create a temp volumeControlBlock to gather back information from block 0
    struct volumeControlBlock *vcb = malloc(blockSize);

    // Read from LBA block 0
    LBAread(vcb, 1, 0);
    
    // If the VCB variables have a default of 0, the VCB has not been created yet
    if (vcb->volumeID == 0 && vcb->volumeSize == 0 && vcb->blockSize == 0) {
        //Cleanup
        free (vcb);
        
        //Since the VCB does not exist, return 0
        return 0;
    } else {
        // Since the VCB does exist, print information about it
        printf("-------------------------------------------------------------------------------\n");
        printf("VOLUME CONTROL BLOCK ALREADY CREATED.\n");
        printf("Volume Name: %s\n", vcb->volumeName);
        printf("Volume ID: %u\n", vcb->volumeID);
        printf("Volume Size: %llu\n", vcb->volumeSize);
        printf("Number of Directories: %llu\n", vcb->numberOfDirectories);
        printf("Volume LBA Block Size: %llu\n", vcb->blockSize);
        printf("Number of LBA Blocks: %llu\n", vcb->numberOfBlocks);
        printf("Root Directory Block: %llu\n", vcb->rootDirectory);
        printf("-------------------------------------------------------------------------------\n");

        // Since the FSI exists as well, print information about it
        struct freeSpaceInformation *fsi = getFreeSpaceInformation(blockSize);
        printf("-------------------------------------------------------------------------------\n");
        printf("FREE SPACE INFORMATION ALREADY CREATED.\n");
        printf("Free Space Available: %llu\n", fsi->freeSpace);
        printf("Lowest Block Accessible: %u\n", fsi->lowestBlockAccessible);
        printf("Highest Block Accessible: %llu\n", fsi->highestBlockAccessible);
        printf("-------------------------------------------------------------------------------\n");
        
        //Cleanup
        free(vcb);
        free(fsi);
        
        // Since the VCB exists, return 1
        return 1;
    }
}

void intializeFreeSpaceInformation(uint64_t volumeSize, int16_t blockSize) {
    // LBA Block 0: Volume Control Block
    // LBA Block 1-49: Free Space Information Blocks
    // Total Blocks used by system: 50
    uint32_t numUsedBytes = (50 * blockSize);
    
    // Actual volume usuable, after VCB and FSI blocks are taken into account
    uint64_t actualVolumeSize = volumeSize - numUsedBytes;
    
    // Create struct in order to later write it to the file system
    struct freeSpaceInformation *fsi = malloc(blockSize * 49);
    
    // Set all bits to 1, since at the beggining, they are all free
    memset(fsi, 0xFF, (blockSize * 49));
    
    // Set other parameters
    fsi->freeSpace = actualVolumeSize;
    fsi->lowestBlockAccessible = 100;
    fsi->highestBlockAccessible = ((volumeSize/blockSize) - 1);
    
    // Clear bits for blocks 0-49, since they are used by VCB and FSI blocks
    for (int i = 0; i <= 50; i++) {
        clearBit(fsi->freeBlockBitArray, i);
    }
    
    // Print info
    printf("-------------------------------------------------------------------------------\n");
    printf("CREATING FREE SPACE INFORMATION...\n");
    printf("Free Space Available: %llu\n", fsi->freeSpace);
    printf("Lowest Block Accessible: %u\n", fsi->lowestBlockAccessible);
    printf("Highest Block Accessible Size: %llu\n", fsi->highestBlockAccessible);
    printf("-------------------------------------------------------------------------------\n\n");

    // Write blocks to file system
    LBAwrite(fsi, 49, 1);
    
    // Cleanup
    free(fsi);
}

void* getBlockNumbersOfAllDirectories(uint16_t blockSize) {
    // Get all directories entries from the LBA
    struct directoryEntry *dirs = malloc(blockSize * 49);
    LBAread(dirs, 49, 50);
    
    // Number of directories on the file system
    uint64_t numberOfDirectories = getVCBDirectoryCount(blockSize);
    
    // Create an array to store the block numbers. We know the size by looking at the total number of directories on the file system
    uint64_t *blockArray = malloc(sizeof(uint64_t) * numberOfDirectories);
    
    // Keeps track of the number of directories we have found. This is useful to know which element in the above array we should add the block to once we have found it
    int directoriesFound = 0;

    // Iterate through all the directories
    for (int i = 0; i <= numberOfDirectories; i++) {
        // A directory has a file extension string set to the global const DIRECTORY_EXTENSION_NAME
        // If the current element we are checking is not a directory, skip it
        if (strcmp(dirs[i].fileExtension, DIRECTORY_EXTENSION_NAME) != 0) {
            continue;
        }
        
        // If the current element we are checking IS A valid directory, add it to the directory list
        else {
            blockArray[directoriesFound] = dirs[i].blockLocation;
            directoriesFound = directoriesFound + 1;
        }
        
        // If we have found all the directories, do not keep searching. Instead, jump out of loop to return the array
        if (directoriesFound == numberOfDirectories) {
            break;
        }
    }
    
    //Clean up
    free(dirs);
    
    // After we go find all the directory block locations, return the pointer to the array of blocks
    // ** THE CALLER OF THIS FUNCTION MUST FREE() THE POINTER AT SOME POINT **
    return blockArray;
}

void* getAllDirectoriesStructs(uint16_t blockSize) {
    // Get all directories entries from the LBA
    struct directoryEntry *dirs = malloc(blockSize * 49);
    LBAread(dirs, 49, 50);
    
    // Number of directories on the file system
    uint64_t numberOfDirectories = getVCBDirectoryCount(blockSize);
    
    // Create an array to store the directoryEntry structs. We know the size by looking at the total number of directories on the file system
    struct directoryEntry *dirList= malloc(sizeof(struct directoryEntry) * numberOfDirectories);
    
    // Keeps track of the number of directories we have found. This is useful to know which element in the above array we should add the struct to once we have found it
    int directoriesFound = 0;
    
    // Iterate through all the directories
    for (int i = 0; i <= numberOfDirectories; i++) {
        // A directory has a file extension string set to the global const DIRECTORY_EXTENSION_NAME
        // If the current element we are checking is not a directory, skip it
        if (strcmp(dirs[i].fileExtension, DIRECTORY_EXTENSION_NAME) != 0) {
            continue;
        }
        
        // If the current element we are checking IS A valid directory, add it to the directory list
        else {
            //dirList[directoriesFound] = dirs[i];
            memcpy(&dirList[directoriesFound], &dirs[i], sizeof(struct directoryEntry));
            directoriesFound = directoriesFound + 1;
        }
        
        // If we have found all the directories, do not keep searching. Instead, jump out of loop to return the array
        if (directoriesFound == numberOfDirectories) {
            break;
        }
    }
    
    // Cleanup
    free (dirs);
    
    // After we go find all the directory entries, return the pointer to the array of structs
    // ** THE CALLER OF THIS FUNCTION MUST FREE() THE POINTER AT SOME POINT **
    return dirList;
}

void* getDirectoryEntryFromBlock(uint64_t directoryBlockNumber, uint16_t blockSize) {
    // Since we know the location of the directory, we just need to read a single block
    struct directoryEntry *dir = malloc(blockSize);
    LBAread(dir, 1, directoryBlockNumber);
    
    // Ensure that this is an actual valid directory
    // A valid directory has a block that is not equal to 0
    if (dir->blockLocation != 0) {
        // Return the pointer to the struct
        // * THE CALLER IS RESPONSIBLE FOR CALLING FREE() ON THIS STRUCT *
        return dir;
    }
    
    // If it is not valid, throw an erroe
    else {
        // Cleanup
        free (dir);
        printf("ERROR AT getDirectoryEntryFromBlock(): TRIED TO ACCESS AN INVALID DIRECTORY BLOCK!\n");
        return NULL;
    }
}

void listTree (uint64_t parentDirectoryBlockNumber, uint16_t blockSize) {
    listTreeHelper(parentDirectoryBlockNumber, 0, blockSize);
    printf("\n");
}

void listDirectories(uint64_t parentDirectoryBlockNumber, uint16_t blockSize) {
    // Since we know the location of the directory, we just need to read a single block
    struct directoryEntry *dir = malloc(blockSize);
    LBAread(dir, 1, parentDirectoryBlockNumber);
    
    // Go to every child and print it if it is a directory
    for (int i = 0; dir->indexLocations[i]  != 0; i++) {
        // Create a temp directory to read name from
        struct directoryEntry *tempDir = malloc(blockSize);
        tempDir = getDirectoryEntryFromBlock(dir->indexLocations[i], blockSize);
        
        // If the directory is a file, print the file name and extension
        if (strcmp(tempDir->fileExtension, DIRECTORY_EXTENSION_NAME) == 0) {
            printf("○ %s\n", tempDir->name);
        }
        // Cleanup temp directory
        free(tempDir);
    }
    
    // Go to every child and print it if it is a file
    for (int i = 0; dir->indexLocations[i]  != 0; i++) {
        // Create a temp directory to read name from
        struct directoryEntry *tempDir = malloc(blockSize);
        tempDir = getDirectoryEntryFromBlock(dir->indexLocations[i], blockSize);
        
        // If the directory is a file, print the file name and extension
        if (strcmp(tempDir->fileExtension, DIRECTORY_EXTENSION_NAME) != 0) {
            printf("◉ %s.%s\n", tempDir->name, tempDir->fileExtension);
        }
        
        // Cleanup temp directory
        free(tempDir);
    }
    
    // Print spacer to have space before next command
    printf("\n");
    
    // Cleanup
    free(dir);
    return;
}

void listTreeHelper(uint64_t parentDirectoryBlockNumber, int directoryLevel, uint16_t blockSize) {
    // Since we know the location of the directory, we just need to read a single block
    struct directoryEntry *dirs = malloc(blockSize);
    LBAread(dirs, 1, parentDirectoryBlockNumber);
    
    // In order to make it look like a tree, we need to print spaces for deeper directories
    // These spaces are a function of the level we start are on. For instance, the root is at level 0, so it have no spaces.
    for (int i = 0; i < directoryLevel; i++) {
        printf("    ");
    }
    
    // The root should have a different denominator
    if (directoryLevel == 0)
        printf("● ");
    else if (strcmp(dirs->fileExtension, DIRECTORY_EXTENSION_NAME) == 0)
        printf("○ ");
    else
        printf("◉ ");
    
    // If the directory is a file, print the file name and extension
    if (strcmp(dirs->fileExtension, DIRECTORY_EXTENSION_NAME) == 0) {
        printf("%s\n", dirs->name);
    }
    // If the directory is an actual directory, simply print its name
    else {
        printf("%s.%s\n", dirs->name, dirs->fileExtension);
        
        // We also do not want to print it's children, since it does not have any sub directories (its a file)
        // Cleanup
        free(dirs);
        return;
    }
    
    // We have to use recursion to print out the children, which in turn will print out its children
    // This will ensure that we keep the correct tree structure
    // If the current directory we are printing DOES HAVE children, we print them
    if (dirs->indexLocations[0]  != 0) {
        // Go to every child and print its children. Stop this process when you come across an empty child block
        for (int i = 0; dirs->indexLocations[i]  != 0; i++) {
            int childreLevel = (directoryLevel + 1);
            listTreeHelper(dirs->indexLocations[i], childreLevel, blockSize);
        }
    }
    
    // Cleanup
    free(dirs);
}

void setMetaData(char* directoryPath, uint16_t newPermissions, uint16_t blockSize) {
    // Print info
    printf("Changing Metadata...\n");
    
    uint64_t originalDirectory = getVCBCurrentDirectory(blockSize);
    
    // Change directory, so that we can have direct access to change permission
    int changeStatus = changeDirectory(directoryPath, 1, blockSize);
    
    // If return was -1, this path is not valid. We want to reset back to the starting directory
    if (changeStatus == -1) {
        // If there was an error, return to the orignal directory user was at
        setVCBCurrentDirectory(originalDirectory, blockSize);
        
        // Print error, and exit the function
        printf("No metadata change. Directory Not Found!\n\n");
        return;
    }
    
    // Get the directory
    struct directoryEntry *dir = malloc(blockSize);
    LBAread(dir, 1, getVCBCurrentDirectory(blockSize));
    
    // Set permission
    dir->permissions = newPermissions;
    
    // Update modified date to now
    dir->dateModified =  (unsigned int)time(NULL);
    
    printf("Metadata changed. Permission set to %d\n\n",  newPermissions);
    
    // Write back directory to file system
    LBAwrite(dir, 1, getVCBCurrentDirectory(blockSize));
    
    // Return to the orignal directory user was at
    setVCBCurrentDirectory(originalDirectory, blockSize);

    // Cleanup
    free(dir);
}

void printDirectoryInfo(char* directoryPath, uint16_t blockSize) {
    // Print info
    printf("Printing Directory Information...\n");
    
    uint64_t originalDirectory = getVCBCurrentDirectory(blockSize);
    
    // Change directory, so that we can have direct access to change permission
    int changeStatus = changeDirectory(directoryPath, 1, blockSize);
    
    // If return was -1, this path is not valid. We want to reset back to the starting directory
    if (changeStatus == -1) {
        // If there was an error, return to the orignal directory user was at
        setVCBCurrentDirectory(originalDirectory, blockSize);
        
        // Print error, and exit the function
        printf("Directory Not Found!\n\n");
        return;
    }
    
    // Get the directory
    struct directoryEntry *dir = malloc(blockSize);
    LBAread(dir, 1, getVCBCurrentDirectory(blockSize));
    
    // Print info
    if (strcmp(dir->fileExtension, DIRECTORY_EXTENSION_NAME) == 0) {
        printf("Directory Name: %s\n", dir->name);
        printf("Directory Permissions: %hu\n", dir->permissions);
        printf("Directory Creation Date: %u\n", dir->dateCreated);
        printf("Directory Modification Date: %u\n", dir->dateModified);
    } else {
        printf("File Name: %s.%s\n", dir->name, dir->fileExtension);
        printf("File Size: %llu\n", dir->fileSize);
        printf("File Permissions: %hu\n", dir->permissions);
        printf("File Creation Date: %u\n", dir->dateCreated);
        printf("File Modification Date: %u\n", dir->dateModified);
    }
    
    printf("Data Printed Successfully.\n\n");

    
    // Return to the orignal directory user was at
    setVCBCurrentDirectory(originalDirectory, blockSize);

    // Cleanup
    free(dir);
}

uint64_t createDirectoryOLD(uint16_t blockSize) {
    // Print Info
    printf("Creating Directory...\n");
    printf("Please Enter a Directory Name. No Spaces Allowed: ");
    
    // Get name for new directory. See directoryEntry struct for size of directory/file name (its 29...)
    char userInput[29];
    char *argList[29];
    char *token;
    fgets(userInput, 29, stdin);
    
    // Ensure that there was no spaces in the name
    int numberOfWords = 0;
    token = strtok(userInput, " ");
    while (token != NULL) {
        argList[numberOfWords] = token;
        numberOfWords++;
        token = strtok(NULL, " ");
    }
    
    // If there was one word only, then the name was valid and we can procced to create a new directory
    if (numberOfWords == 1) {
        // Get the block of the directory we are in
        uint64_t currentDirectoryBlock = getVCBCurrentDirectory(blockSize);
        
        // Get the directory struct of the directory we are in
        struct directoryEntry *currentDir = getDirectoryEntryFromBlock(currentDirectoryBlock, blockSize);
        
        // Get the block of the parent directory. We need this for createDirectoryHelper()
        uint64_t parentDirectoryBlock = currentDir->parentDirectory;
        
        // Create the directory
        uint64_t locationOfNewDirectory = createDirectory(argList[0], parentDirectoryBlock, blockSize);
        
        //Cleanup
        free (currentDir);
        return locationOfNewDirectory;
    }
    
    // If there was more than one word, then the name was invalid
    else {
        printf("Invalid Directoy Name. No Spaces Allowed!\n");
        printf("Failed to Create New Directory.\n");
        
        // Return block 0, since the creation was unsuccessfull
        return 0;
    }
}

uint64_t createDirectory(char* directoryName, uint64_t parentDirectoryBlockNumber, uint16_t blockSize) {
    // Create temp directory, which will be written to file system
    struct directoryEntry *tempDir = malloc(blockSize);
    
    // Set variables for the root directory
    strcpy(tempDir->name, directoryName);
    strcpy(tempDir->fileExtension, DIRECTORY_EXTENSION_NAME);
    tempDir->permissions = 755; // Default directory permission
    tempDir->dateCreated = (unsigned int)time(NULL);
    tempDir->dateModified = (unsigned int)time(NULL);
    tempDir->fileSize = 0;
    tempDir->parentDirectory = parentDirectoryBlockNumber;
    
    // Find open block to write this directory to
    uint64_t dirBlockLocation = findSingleFreeLBABlockInRange(51, 99, blockSize);
    
    // Set this directory block location to the newly found free block
    tempDir->blockLocation = dirBlockLocation;
    
    // Since the root has no files/children directories when created, set these pointers to 0
    memset(tempDir->indexLocations, 0x00, (sizeof(tempDir->indexLocations)/sizeof(tempDir->indexLocations[0])));
    
    // Print info
    printf("CREATING NEW DIRECTORY...\n");
    printf("Directory Name: %s\n", tempDir->name);
    printf("Directory Location: %llu\n", tempDir->blockLocation);
    printf("Directory Permissions: %hu\n", tempDir->permissions);
    printf("Directory Creation Date: %u\n", tempDir->dateCreated);
    printf("Parent Directory location: %llu\n", tempDir->parentDirectory);
    printf("Child Directory locations: * No Children Directories *\n\n");
    
    // Write to open block
    LBAwrite(tempDir, 1, dirBlockLocation);
    
    // Update block to used
    setBlockAsUsed(dirBlockLocation, blockSize);
    
    // Update parent node's 'fileIndexLocation' to point to this directory
    addChildDirectoryIndexLocationToParent(parentDirectoryBlockNumber, dirBlockLocation, blockSize);
    
    // Since we created a directory, we must update the directory count
    increaseVCBDirectoryCount(blockSize);
    
    // Cleanup
    free(tempDir);
    
    // Return the block location of this new directory
    return dirBlockLocation;
}

uint64_t createFileDirectory(char* fileName, char* fileExtension, uint64_t fileSize, uint64_t parentDirectoryBlockNumber, uint16_t blockSize) {
    // Create temp directory, which will be written to file system
    struct directoryEntry *tempDir = malloc(blockSize);
    
    // Set variables for the root directory
    strcpy(tempDir->name, fileName);
    strcpy(tempDir->fileExtension, fileExtension);
    tempDir->permissions = 644; // Default permission
    tempDir->dateCreated = (unsigned int)time(NULL);
    tempDir->dateModified = (unsigned int)time(NULL);
    tempDir->fileSize = fileSize;
    tempDir->parentDirectory = parentDirectoryBlockNumber;
    
    // Calculate number of blocks needed for this file
    int numberOfBlocksNeeded = (int)((fileSize / blockSize) + 1);
    
    // Find open block to write this directory to
    uint64_t dirBlockLocation = findSingleFreeLBABlockInRange(51, 99, blockSize);
    
    // Set this file directory block location to the newly found free block
    tempDir->blockLocation = dirBlockLocation;
    
    // Create array to store free blocks. These blocks will be used to store the actual 'contents' of the file
    // Make sure to look starting at block 100, since everything before that is used by the file system
    uint64_t *freeBlocks = findFreeLBABlocksInRange(100, getHighestUseableBlock(blockSize), numberOfBlocksNeeded, blockSize);
    
    // Set the indexLocations to 0, so we know that this are has been wiped clean
    memset(tempDir->indexLocations, 0x00, (sizeof(tempDir->indexLocations)/sizeof(tempDir->indexLocations[0])));
    
    // Assign the block we set aside to these index locations
    // Also set all the index blocks that this file will need to used

    for (int i = 0; i < numberOfBlocksNeeded; i++) {
        tempDir->indexLocations[i] = freeBlocks[i];
        setBlockAsUsed(freeBlocks[i], blockSize);
    }
     
    // Print info
    printf("CREATING NEW FILE...\n");
    printf("File Name: %s\n", tempDir->name);
    printf("File Extension: %s\n", tempDir->fileExtension);
    printf("File Size: %llu bytes\n", tempDir->fileSize);
    printf("File Permissions: %hu\n", tempDir->permissions);
    printf("File Creation Date: %u\n", tempDir->dateCreated);
    printf("Directory Location: %llu\n", tempDir->blockLocation);
    printf("Parent Directory location: %llu\n\n", tempDir->parentDirectory);
    
    // Write this file directory to dirBlockLocation
    LBAwrite(tempDir, 1, dirBlockLocation);
    
    // Update file directory block to used
    setBlockAsUsed(dirBlockLocation, blockSize);
    
    // Update parent node's 'fileIndexLocation' to point to this directory
    addChildDirectoryIndexLocationToParent(parentDirectoryBlockNumber, dirBlockLocation, blockSize);
    
    // Since we created a directory, we must update the directory count
    increaseVCBDirectoryCount(blockSize);
    
    // Cleanup
    free(tempDir);
    free(freeBlocks);
    
    // Return the block location of this new file directory
    return dirBlockLocation;
}

int removeFile(char * filePath, uint16_t blockSize){
    uint64_t originalDirectory = getVCBCurrentDirectory(blockSize); // saves original spot

    // Attempt to move to the file directory
    int returnStat = changeDirectory(filePath, 1, blockSize);
    
    // If the path does not exist, report it and exit function
    if(returnStat < 0)
    {
        printf("File Path Not Valid\n\n");
        return -1;
    }
    
    // Get block of file directory
    uint64_t srcFileBlock = getVCBCurrentDirectory(blockSize);
    
    // Get actual file directory entry from block location
    struct directoryEntry *srcFile = getDirectoryEntryFromBlock(srcFileBlock, blockSize);

    // Jump back to original directory
    setVCBCurrentDirectory(originalDirectory, blockSize);
    
    // Iterate through all blocks that create this file, and set them as free
    for (int i = 0; srcFile->indexLocations[i]  != 0; i++) {
        setBlockAsFree(srcFile->indexLocations[i], blockSize);
    }

    // Update actual file directory block to free
    setBlockAsFree(srcFileBlock, blockSize);

    // Remove file reference from its parent
    removeChildFromParent(srcFile->parentDirectory, srcFile->blockLocation, blockSize);

    // Cleanup
    free(srcFile);
    
    // Print spacer for next command
    printf("\n");

    // Return 1 on success
    return 1;
}

void createRootDirectory(uint16_t blockSize) {
    // Create temp directory, which will be written to file system
    struct directoryEntry *tempRootDir = malloc(blockSize);
    
    // Set variables for the root directory
    strcpy(tempRootDir->name, "ROOT");
    strcpy(tempRootDir->fileExtension, DIRECTORY_EXTENSION_NAME);
    tempRootDir->blockLocation = getVCBRootDirectory(blockSize);
    tempRootDir->permissions = 755; // Permission for root directory
    tempRootDir->dateCreated = (unsigned int)time(NULL);
    tempRootDir->dateModified = (unsigned int)time(NULL);
    tempRootDir->fileSize = 0;
    tempRootDir->parentDirectory = getVCBRootDirectory(blockSize);
    
    // Since the root has no files/children directories when created, set these pointers to 0
    memset(tempRootDir->indexLocations, 0x00, (sizeof(tempRootDir->indexLocations)/sizeof(tempRootDir->indexLocations[0])));
    
    // Print info
    printf("-------------------------------------------------------------------------------\n");
    printf("CREATING *ROOT* DIRECTORY...\n");
    printf("Directory Name: %s\n", tempRootDir->name);
    printf("Directory Location: %llu\n", tempRootDir->blockLocation);
    printf("Directory Permissions: %hu\n", tempRootDir->permissions);
    printf("Directory Creation Date: %u\n", tempRootDir->dateCreated);
    printf("Child Directory locations: None\n");
    printf("-------------------------------------------------------------------------------\n\n");

    // Write back the block
    LBAwrite(tempRootDir, 1, 50);
    
    // Since we created a directory, we must update the directory count
    increaseVCBDirectoryCount(blockSize);
    
    //Cleanup
    free(tempRootDir);
}

void increaseVCBDirectoryCount(uint16_t blockSize) {
    // Create a temp volumeControlBlock to gather back information from block 0
    struct volumeControlBlock *vcb = malloc(blockSize);

    // Read from LBA block 0
    LBAread(vcb, 1, 0);
    
    // Update the number of directories counter
    vcb->numberOfDirectories = (vcb->numberOfDirectories + 1);
    
    // Write VCB back to file system
    LBAwrite(vcb, 1, 0);
    
    //Clean up
    free(vcb);
}

void setVCBCurrentDirectory(uint64_t newDirectoryBlock, uint16_t blockSize) {
    // Create a temp volumeControlBlock to gather back information from block 0
    struct volumeControlBlock *vcb = malloc(blockSize);

    // Read from LBA block 0
    LBAread(vcb, 1, 0);
    
    // Update the current dirctory block
    vcb->currentDirectory = newDirectoryBlock;
    
    // Write VCB back to file system
    LBAwrite(vcb, 1, 0);
    
    //Clean up
    free(vcb);
}

void cdCommand(char* directoryPath, uint16_t blockSize) {
    // Change directories
    int changeStatus = changeDirectory(directoryPath, 0, blockSize);
    
    // If there directory does not exist, print that info
    if (changeStatus != 1) {
        printf("Error Changing Directories. Directory does not exist.\n\n");
        return;
    }
    printf("\n");
}

int changeDirectory(char* directoryPath, uint16_t elevated, uint16_t blockSize) {
    // Save the current directory, in case there is an error changing directories when we call the helper
    uint64_t originalDirectory = getVCBCurrentDirectory(blockSize);
    
    // Keeps track of the path of directories we want to take. Can only change a maximum of 25 directories deep
    char *directoryList[25];
    
    // Parse directoryPath by splitting it with the "/", which will give us the path of directories to traverse
    char *token;
    token = strtok(directoryPath, "/");
    int argc = 0;
    while (token != NULL) {
        directoryList[argc] = token;
        argc++;
        token = strtok(NULL, "/");
    }
    
    // If the last argument is a file, the user might enter FILENAME.EXTENSION (for instance: Pictures/Hawaii/sunset.jpg)
    // Since the extension is not part of the directory name, we want to make sure to chop off anything after the '.'
    // Only do this if the diretory is not '..' however, since '..' is in fact a valid path
    if (strcmp(directoryList[argc - 1], "..") != 0) {
        token = strtok(directoryList[argc - 1], ".");
        directoryList[argc - 1] = token;
    }
    
    for (int i = 0 ; i < argc; i++) {
        int changeStatus = changeDirectoryHelper(directoryList[i], elevated, blockSize);
        
        // If changeStatus was -1, this path is not valid. We want to reset back to the starting directory
        if (changeStatus == -1) {
            setVCBCurrentDirectory(originalDirectory, blockSize);
            return -1;
        }
        // If changeStatus was -2, there was an attempt to cd into a file, without an elevated call. We want to reset back to the starting directory
        if (changeStatus == -2) {
            setVCBCurrentDirectory(originalDirectory, blockSize);
            return -2;
        }
        
    }
    return 1;
}

int changeDirectoryHelper(char* directoryName, uint16_t elevated, uint16_t blockSize) {
    // Get block of the current directory
    uint64_t currentDirectory = getVCBCurrentDirectory(blockSize);
    
    // Get the directory structure of the current directory
    struct directoryEntry *dir = getDirectoryEntryFromBlock(currentDirectory, blockSize);
    
    // If command was directory name was '.', then we are not changind directorie at all, so simply return a success
    if (strcmp(directoryName, ".") == 0) {
        // Cleanup
        free (dir);
        
        // Return 1, since we technically changed successfully (we did not change, but stayed where we were at)
        return 1;
    }
    
    // If command was directory name was '..', then we are changing directory to the parent
    else if (strcmp(directoryName, "..") == 0) {
        // Save parent block
        uint64_t parentBlock = dir->parentDirectory;
        
        // Get directory struct of the parent
        struct directoryEntry *parentDir = getDirectoryEntryFromBlock(parentBlock, blockSize);
        
        // Set current directory to parent block
        setVCBCurrentDirectory(parentBlock, blockSize);
        
        // Cleanup
        free (parentDir);
        free (dir);
        
        // Return 1, since we found and changed the directory successfully
        return 1;
    }
    
    // If command was 'cd NAME', then we have to ensure that NAME is a valid child before changing the directory
    else {
        // Iterate through all children directories to find a name that matches
        for (int i = 0; dir->indexLocations[i] != 0; i++) {
            // Save block of the child we are going to look at
            uint64_t childBlock = dir->indexLocations[i];
            
            // Pull the child directory
            struct directoryEntry *childDir = getDirectoryEntryFromBlock(childBlock, blockSize);
            
            // If we find the name that matches, then change the current directory to that directorie's block, and break out of loop
            if (strcmp(childDir->name, directoryName) == 0) {
                // If we are trying to cd into a file AND this call has not been elevated, return -2 since cd'in into a file is not allowed unless the call has been elevated
                if ((strcmp(childDir->fileExtension, DIRECTORY_EXTENSION_NAME) !=0) && elevated == 0) {
                    // Cleanup
                    free (childDir);
                    free (dir);
                    
                    // Return -2, which means failed attempt to cd into a file without an elevated call
                    return -2;
                }
                
                // Set new directory to the correct child
                setVCBCurrentDirectory(childBlock, blockSize);
                
                // Cleanup
                free (childDir);
                free (dir);
                
                // Return 1, since we found and changed the directory successfully
                return 1;
            }
        }
        // If we iterated through all the children and did not find a match, then we cannot change directories since it does not exist
        // Return -1, which means directory not found
        return -1;
    }
}

void displayCurrentDirectory(uint16_t blockSize) {
    // Get block of the current directory
    uint64_t currentDirectory = getVCBCurrentDirectory(blockSize);
    
    // Get the directory structure of the current directory
    struct directoryEntry *dir = getDirectoryEntryFromBlock(currentDirectory, blockSize);
    
    printf("CURRENT DIRECTORY: %s\n\n", dir->name);
    
    // Cleanup
    free(dir);
}


void decreaseVCBDirectoryCount(uint16_t blockSize) {
    // Create a temp volumeControlBlock to gather back information from block 0
    struct volumeControlBlock *vcb = malloc(blockSize);

    // Read from LBA block 0
    LBAread(vcb, 1, 0);
    
    // Update the number of directories counter
    // Ensure that we are not going into a negative number
    if ( (vcb->numberOfDirectories - 1) >= 0) {
        vcb->numberOfDirectories = (vcb->numberOfDirectories - 1);
    } else {
        printf("ERROR IN decreaseVCBDirectoryCount(). Attempting to reduce directory count below 0!\n");
    }
    
    // Write VCB back to file system
    LBAwrite(vcb, 1, 0);
    
    //Clean up
    free(vcb);
}

uint64_t getVCBDirectoryCount(uint16_t blockSize) {
    // Create a temp volumeControlBlock to gather back information from block 0
    struct volumeControlBlock *vcb = malloc(blockSize);

    // Read from LBA block 0
    LBAread(vcb, 1, 0);
    
    uint64_t numberOfDirectories = vcb->numberOfDirectories;
    
    //Clean up
    free(vcb);
    
    return numberOfDirectories;
}

uint64_t getVCBRootDirectory(uint16_t blockSize) {
    // Create a temp volumeControlBlock to gather back information from block 0
    struct volumeControlBlock *vcb = malloc(blockSize);

    // Read from LBA block 0
    LBAread(vcb, 1, 0);
    
    uint64_t rootDirectory = vcb->rootDirectory;
    
    //Clean up
    free(vcb);
    
    return rootDirectory;
}

uint64_t getVCBCurrentDirectory(uint16_t blockSize) {
    // Create a temp volumeControlBlock to gather back information from block 0
    struct volumeControlBlock *vcb = malloc(blockSize);

    // Read from LBA block 0
    LBAread(vcb, 1, 0);
    
    uint64_t rootDirectory = vcb->currentDirectory;
    
    //Clean up
    free(vcb);
    
    return rootDirectory;
}

void setBit(int A[], uint64_t bit) {
    uint64_t i = bit / 32;          // i = array index (use: A[i])
    uint64_t pos = bit % 32;        // pos = bit position in A[i]

    unsigned int flag = 1;          // flag = 0000.....00001

    flag = flag << pos;             // flag = 0000...010...000   (shifted k positions)

    A[i] = A[i] | flag;             // Set the bit at the k-th position in A[i]
}

void clearBit(int A[], uint64_t bit) {
    uint64_t i = bit / 32;
    uint64_t pos = bit % 32;

    unsigned int flag = 1;          // flag = 0000.....00001

    flag = flag << pos;             // flag = 0000...010...000   (shifted k positions)
    flag = ~flag;                   // flag = 1111...101..111

    A[i] = A[i] & flag;             // RESET the bit at the k-th position in A[i]
}

int getBit(int A[], uint64_t bit) {
    uint64_t i = bit / 32;
    uint64_t pos = bit % 32;

    unsigned int flag = 1;          // flag = 0000.....00001

    flag = flag << pos;             // flag = 0000...010...000   (shifted k positions)

    if ( A[i] & flag )              // Test the bit at the k-th position in A[i]
        return 1;
    else
        return 0;
}

void* getFreeSpaceInformation(int16_t blockSize) {
    struct freeSpaceInformation *fsi = malloc(49 * blockSize);
    LBAread(fsi, 49, 1);
    return fsi;
}

void* findFreeLBABlocksInRange(uint64_t startIndex, uint64_t endIndex, int numBlocksNeeded, int16_t blockSize) {
    // Get FSI struct
    struct freeSpaceInformation *fsi = malloc(49 * blockSize);
    LBAread(fsi, 49, 1);
    
    // Pointer to array of free blocks
    uint64_t *blocks = malloc(numBlocksNeeded * sizeof(uint64_t));
    
    // Set number of blocks found to 0
    int numBlocksFound = 0;
    
    // Visit every block and see its status
    for (uint64_t i = startIndex; i <= endIndex; i++) {
        // If we have found all the free blocks we need, then return those blocks as an array pointer
        if (numBlocksFound == numBlocksNeeded) {
            free(fsi);
            return blocks;
        }
        else {
            // If the current block we are checking is free, then add it to the free list
            if (getBit(fsi->freeBlockBitArray, i) == 1) {
                blocks[numBlocksFound] = i;
                numBlocksFound++;
            }
        }
    }
    
    // If we did not find enough blocks, then cleanup and return a null pointer
    free(fsi);
    free(blocks);
    return NULL;
}

uint64_t findSingleFreeLBABlockInRange(uint64_t startIndex, uint64_t endIndex, int16_t blockSize) {
    // Get FSI struct
    struct freeSpaceInformation *fsi = malloc(49 * blockSize);
    LBAread(fsi, 49, 1);
    
    // Visit every block and see its status
    for (uint64_t i = startIndex; i <= endIndex; i++) {
        // If the current block we are checking is free, then return its block number
        if (getBit(fsi->freeBlockBitArray, i) == 1) {
            free(fsi);
            return i;
        }
    }
    
    // If no free block was found, cleanup and return 0
    free(fsi);
    return 0;
}
void setBlockAsFree(uint64_t blockNumber, int16_t blockSize){
     // Get FSI struct
    struct freeSpaceInformation *fsi = malloc(49 * blockSize);
    LBAread(fsi, 49, 1);

    // Free the bit ->1 (Free)
    setBit(fsi->freeBlockBitArray, blockNumber);

    // Write back to LBA
    LBAwrite(fsi, 49, 1);

    //Cleanup
    free(fsi);
}
void setBlockAsUsed(uint64_t blockNumber, int16_t blockSize) {
    // Get FSI struct
    struct freeSpaceInformation *fsi = malloc(49 * blockSize);
    LBAread(fsi, 49, 1);
    
    // Clear the bit to 0 (Used)
    clearBit(fsi->freeBlockBitArray, blockNumber);
    
    // Write back to LBA
    LBAwrite(fsi, 49, 1);
    
    //Cleanup
    free(fsi);
}

uint64_t getHighestUseableBlock(int16_t blockSize) {
    // Get FSI struct
    struct freeSpaceInformation *fsi = malloc(49 * blockSize);
    LBAread(fsi, 49, 1);
    
    // Save highest block to return after cleanup
    uint64_t highestBlock = fsi->highestBlockAccessible;
    
    // Cleanup
    free(fsi);
    
    return highestBlock;
}

int addChildDirectoryIndexLocationToParent(uint64_t parentDirectoryBlockNumber, uint64_t childDirectoryBlockNumber, int16_t blockSize) {
    // Get directory from LBA block
    struct directoryEntry *tempDir = malloc(blockSize);
    LBAread(tempDir, 1, parentDirectoryBlockNumber);
    
    // Find first free IndexLocation slot
    for(int i = 0; i < (sizeof(tempDir->indexLocations) / sizeof(tempDir->indexLocations[0])); i++) {
        // If the current slot we are checking is empty, update it
        if (tempDir->indexLocations[i] == 0) {
            // Update it
            tempDir->indexLocations[i] = childDirectoryBlockNumber;
            
            // Write directory back into file system
            LBAwrite(tempDir, 1, parentDirectoryBlockNumber);
            
            // Cleanup
            free (tempDir);
            
            // Return success
            return 1;
        }
    }
    
    // If you go through every element, and do not find a slot, then the parent direcory cannot have any more children
    // Cleanup
    free(tempDir);
    
    // Return failure
    return 0;
}

void sampleCreateDirectories(int16_t blockSize) {
    // Get root directory as a base point for all our files
    uint64_t rootDirectory = getVCBRootDirectory(blockSize);
    
    // Create pictures directoy
    uint64_t picturesLocation = createDirectory("Pictures", rootDirectory, blockSize);
        // Create sub pictures directories
        uint64_t hawaiiLocation = createDirectory("Hawaii", picturesLocation, blockSize);
            // Create sub hawaii files
            createFileDirectory("familyPic", "png", 343, hawaiiLocation, blockSize);
            createFileDirectory("sunset", "png", 667, hawaiiLocation, blockSize);
            createDirectory("HotelPictures", hawaiiLocation, blockSize);
            createFileDirectory("oncean", "jpg", 536, hawaiiLocation, blockSize);
        createDirectory("NewYears2019", picturesLocation, blockSize);
    
    // Create documents directory
    uint64_t documentsLocation = createDirectory("Documents", rootDirectory, blockSize);
        // Create sub document directories
        uint64_t identificationLocation = createDirectory("Identifications", documentsLocation, blockSize);
            // Create sub identification files
            createFileDirectory("socialSecurityCard", "pdf", 534, identificationLocation, blockSize);
            createFileDirectory("driversLicense", "png", 993, identificationLocation, blockSize);
        createDirectory("LegalPaperwork", documentsLocation, blockSize);
    
    // Create videos directory
    uint64_t videosLocation = createDirectory("Videos", rootDirectory, blockSize);
        // Create sub videos directories
        createDirectory("Animations", videosLocation, blockSize);
}

void exitFileSystem(int16_t blockSize) {
    // Extends visibility from main function. This allows us to access it without having to pass it in every time
    extern struct openFileDirectory *openFileList;
    
    // Print Info
    printf("CLOSING AND EXITING FILE SYSTEM...\n");
    
    // Set the current directory back to the root at close
    setVCBCurrentDirectory(getVCBRootDirectory(blockSize), blockSize);
    
    // Close the partition
    closePartitionSystem();

    // Print success messages
    printf("FILE SYSTEM CLOSED SUCCESSFULLY!\n\n");
    
    // Exit
    exit (0);
}

uint64_t copyFile(char* srcFilePath, char* tarFilePath, uint16_t blockSize)
{
    uint16_t admin = 1; // to have access to change into a directory
    
    uint64_t originalDirectory = getVCBCurrentDirectory(blockSize); // saves original spot
    
    // get info on the source
    int returnStat = changeDirectory(srcFilePath, admin, blockSize);
    if(returnStat < 0)
    {
        printf("Source Path Not Valid.\n\n");
        return -1;
    }
    uint64_t srcFileBlock = getVCBCurrentDirectory(blockSize);
    struct directoryEntry *srcFile = getDirectoryEntryFromBlock(srcFileBlock, blockSize); //get info from srcEntry
    
    setVCBCurrentDirectory(originalDirectory, blockSize); // back to original directory
    
    //get info on the target
    returnStat = changeDirectory(tarFilePath, 0, blockSize);
    if(returnStat < 0)
    {
        printf("Target Path Not Valid.\n\n");
        free(srcFile);
        return -1;
    }
    uint64_t tarFileBlock = getVCBCurrentDirectory(blockSize);
    struct directoryEntry *tarFile = getDirectoryEntryFromBlock(tarFileBlock, blockSize); //get info from target
    
    // Create temp file, which will be written to file system
    struct directoryEntry *tempFile = malloc(blockSize);
    
    // Set variables for the root directory
    strcpy(tempFile->name, srcFile->name);
    strcpy(tempFile->fileExtension, srcFile->fileExtension);
    tempFile->permissions = srcFile->permissions; // Default directory permission
    tempFile->dateCreated = (unsigned int)time(NULL);
    tempFile->dateModified = (unsigned int)time(NULL);
    tempFile->fileSize = srcFile->fileSize;
    tempFile->parentDirectory = tarFile->blockLocation;
    
    // Find open block to write this directory to
    uint64_t dirBlockLocation = findSingleFreeLBABlockInRange(51, 99, blockSize);
    
    // Set this directory block location to the newly found free block
    tempFile->blockLocation = dirBlockLocation;
    
    // Since the root has no files/children directories when created, set these pointers to 0
    memset(tempFile->indexLocations, 0x00, (sizeof(tempFile->indexLocations)/sizeof(tempFile->indexLocations[0])));
    int count = 0;
    
    for(int i = 0; srcFile->indexLocations[i] != 0; i++)
    {
        count++;
    }
    
    for(int i = 0; i < count; i++)
    {
        
        uint64_t fileblockindex = findSingleFreeLBABlockInRange(100,getHighestUseableBlock(blockSize), blockSize);
        tempFile->indexLocations[i] = fileblockindex;
        void *data = malloc(blockSize);
        LBAread(data, 1, srcFile->indexLocations[i]);
        LBAwrite(data, 1, fileblockindex);
        setBlockAsUsed(fileblockindex, blockSize);
        free(data);
        
    }
    // Write to open block
    LBAwrite(tempFile, 1, dirBlockLocation);
    
    // Update block to used
    setBlockAsUsed(dirBlockLocation, blockSize);
    
    // Update parent node's 'fileIndexLocation' to point to this directory
    addChildDirectoryIndexLocationToParent(tarFile->blockLocation, dirBlockLocation, blockSize);
    
    // Since we created a directory, we must update the directory count
    increaseVCBDirectoryCount(blockSize);
    
    setVCBCurrentDirectory(originalDirectory, blockSize);
    // Cleanup
    
    free(tempFile);
    free(tarFile);
    free(srcFile);
    
    // Return the block location of this new directory
    return dirBlockLocation;
}

int moveDirectory(char * srcPath, char * tarPath, uint16_t blockSize)
{
    uint64_t originalDirectory = getVCBCurrentDirectory(blockSize); // saves original spot
    
    int returnStat =  changeDirectory(srcPath, 1, blockSize);
    if(returnStat < 0)
    {
        printf("Source Path Not Valid.\n\n");
        return -1;
    }
    uint64_t srcPathBlock = getVCBCurrentDirectory(blockSize);
    struct directoryEntry *src = getDirectoryEntryFromBlock(srcPathBlock, blockSize); //get info from srcEntry
    
    setVCBCurrentDirectory(originalDirectory, blockSize); // back to original directory
    
    returnStat = changeDirectory(tarPath, 0, blockSize);
    if(returnStat < 0)
    {
        printf("Target Path Not Valid.\n\n");
        free(src);
        return -1;
    }
    
    uint64_t tarPathBlock = getVCBCurrentDirectory(blockSize);
    struct directoryEntry *tar = getDirectoryEntryFromBlock(tarPathBlock, blockSize); //get info from target
    
    removeChildFromParent(src->parentDirectory, src->blockLocation, blockSize);
    
    src->parentDirectory = tar->blockLocation;
    addChildDirectoryIndexLocationToParent(tar->blockLocation, src->blockLocation, blockSize);
    
    LBAwrite(src, 1, src->blockLocation);
    
    setVCBCurrentDirectory(originalDirectory, blockSize);
    
    free(src);
    free(tar);
    
    // Print space before next command
    printf("\n");
    
    return 0;
}

uint64_t removeChildFromParent(uint64_t parentDirectoryBlockNumber, uint64_t childBlockNumber, uint16_t blockSize)
{
    int temp = 0;
    
    struct directoryEntry *tempDir = malloc(blockSize);
    LBAread(tempDir, 1, parentDirectoryBlockNumber);

    for(int i = 0; i < (sizeof(tempDir->indexLocations) / sizeof(tempDir->indexLocations[0])); i++)
    {
        // If the current slot we are checking is empty, update it
        if (tempDir->indexLocations[i] == childBlockNumber)
        {
            temp = i;
        }
        if (tempDir->indexLocations[i] == 0)
        {
            
            // Update it
            tempDir->indexLocations[temp] = tempDir->indexLocations[i-1];
            tempDir->indexLocations[i-1] = 0;
            
            // Write directory back into file system
            LBAwrite(tempDir, 1, parentDirectoryBlockNumber);
            
            // Cleanup
            free (tempDir);
            
            // Return success
            return 1;
        }
    }
    free(tempDir);
    
    // Return failure
    return 0;
}


int myFsOpen (uint64_t fileBlockLocation, int method, uint16_t blockSize, struct openFileDirectory *openFileList) {
    // Keeps track of which element in the open file array we are going to use for this file
    int fd = 0;
    
    // Iterate through open file list until we find a slot to open a new file
    for (int i = 0; i < FDOPENMAX; i++) {
        if (openFileList[i].flags == FDOPENFREE) {
            fd = i;
            break;
        }
        // If we check all the openFileDirectories and none are free, then file cannot be opened
        else if (i == (FDOPENMAX - 1)) {
            printf("Cannot Open File. Too Many Files Open Currently\n\n");
            return -1;
        }
    }
    
    // Save file directory entry
    struct directoryEntry *fileDir = getDirectoryEntryFromBlock(fileBlockLocation, blockSize);
    
    // Set directory block of this file
    openFileList[fd].directoryEntryLocation = fileDir->blockLocation;
    
    // Set file size of this file
    openFileList[fd].size = fileDir->fileSize;
    
    // Sets the file to be in use, for both read and write
    openFileList[fd].flags = FDOPENINUSE | FDOPENFORREAD | FDOPENFORWRITE;
    
    // Set the buffer to have 1 block size's worth
    openFileList[fd].fileBuffer = malloc(blockSize);
    
    // Set the pointer to start of the file
    openFileList[fd].pointer = 0;
    
    // Cleanup
    free (fileDir);
    
    return fd;
}

int myFsClose(int fd, struct openFileDirectory *openFileList) {
    // Make sure fd is a valid number
    if ( fd >= FDOPENMAX) {
        return -1;
    }
    
    // Make sure file is in use
    if ((openFileList[fd].flags & FDOPENINUSE) != FDOPENINUSE) {
        return -1;
    }
    
    // Free buffer
    free(openFileList[fd].fileBuffer);
    
    // Set back values to default
    openFileList[fd].pointer = 0;
    openFileList[fd].size = 0;
    openFileList[fd].directoryEntryLocation = 0;
    openFileList[fd].flags = FDOPENFREE;
    
    return 1;
}

uint64_t myFsSeek(int fd, uint64_t position, int method, struct openFileDirectory *openFileList) {
    // Make sure fd is a valid number
    if ( fd >= FDOPENMAX) {
        return -1;
    }
    
    // Make sure file is in use
    if ((openFileList[fd].flags & FDOPENINUSE) != FDOPENINUSE) {
        return -1;
    }
    
    switch (method) {
        case MYSEEK_CUR:
            openFileList[fd].pointer += position;
            break;
            
        case MYSEEK_POS:
            openFileList[fd].pointer = position;
            break;
            
        case MYSEEK_END:
            openFileList[fd].pointer = openFileList[fd].size - position;
            break;
        
        default:
            break;
    }
    
    return (openFileList[fd].pointer);
}

uint64_t myFsWrite(int fd, char * src, uint64_t length, uint16_t blockSize, struct openFileDirectory *openFileList) {
    // Make sure fd is a valid number
    if ( fd >= FDOPENMAX) {
        return -1;
    }
    
    // Make sure file is in use
    if ((openFileList[fd].flags & FDOPENINUSE) != FDOPENINUSE) {
        return -1;
    }
    
    return myFsWriteHelper(fd, src, length, 0, blockSize, openFileList);
}

uint64_t myFsWriteHelper(int fd, char * src, uint64_t length, uint64_t srcPosition, uint16_t blockSize, struct openFileDirectory *openFileList) {
    // Find out which LBA block we are are reffering too
    uint64_t currentBlock = openFileList[fd].pointer / blockSize;
    
    // Find the offset of that block
    uint64_t currentOffset = openFileList[fd].pointer % blockSize;
    
    // If the buffer still has space for what were writting, simply add src to buffer
    uint64_t totalBytesToWrite = length - srcPosition;
    if (currentOffset + totalBytesToWrite <= blockSize) {
        // Add src to buffer
        memcpy(openFileList[fd].fileBuffer + currentOffset, (src + srcPosition), totalBytesToWrite);
        
        // Update pointer location
        openFileList[fd].pointer += totalBytesToWrite;
        
        // Return new pointer location
        return openFileList[fd].pointer;
    }
    // When the buffer does not have space, FILL as much as we can, then write it. Then call this function recursivly until we write everything we need to
    else {
        
        // Write AS MUCH AS WE CAN to the buffer
        uint64_t numBytesToWrite = (int)(blockSize - currentOffset);
        memcpy(openFileList[fd].fileBuffer + currentOffset, (src + srcPosition), numBytesToWrite);
        
        // Update pointer location
        openFileList[fd].pointer += numBytesToWrite;
        
        // Get directory entry of the file we are writing to
        struct directoryEntry *dir = getDirectoryEntryFromBlock(openFileList[fd].directoryEntryLocation, blockSize);
        
        // LBAwrite the buffer to the correct index LBA block
        LBAwrite(openFileList[fd].fileBuffer, 1, dir->indexLocations[currentBlock]);
        
        // Clear the buffer
        free(openFileList[fd].fileBuffer);
        
        // Cleanup
        free(dir);
        
        // Call fsWriteHelper again, this time with an update scrPosition, since we are further along in src
        return myFsWriteHelper(fd, src, length, (srcPosition + numBytesToWrite), blockSize, openFileList);
    }
}

uint64_t myFsRead(int fd, char * src, uint64_t length, uint64_t bytesAlreadyRead, uint16_t blockSize, struct openFileDirectory *openFileList) {
    // Make sure fd is a valid number
    if ( fd >= FDOPENMAX) {
        return -1;
    }
    
    // Make sure file is in use
    if ((openFileList[fd].flags & FDOPENINUSE) != FDOPENINUSE) {
        return -1;
    }
    
    return myFsWriteHelper(fd, src, length, 0, blockSize, openFileList);
}

uint64_t myFsReadHelper(int fd, char * src, uint64_t length, uint64_t bytesAlreadyRead, uint16_t blockSize, struct openFileDirectory *openFileList) {
    // Find out which LBA block we are are reffering too
    uint64_t currentBlock = openFileList[fd].pointer / blockSize;
    
    // Find the offset of that block
    uint64_t currentOffset = openFileList[fd].pointer % blockSize;
    
    // If we can read what we need, WITHOUT going to the next block, simply read and return
    uint64_t totalBytesToRead = length - bytesAlreadyRead;
    if (currentOffset + totalBytesToRead <= blockSize) {
        // Pull entire LBA block
        void *block = malloc(blockSize);
        LBAread(block, 1, currentBlock);
        
        // Copy what we need from LBA block into src
        memcpy((src + bytesAlreadyRead), (block + currentOffset), totalBytesToRead);
        
        // Update pointer location
        openFileList[fd].pointer += totalBytesToRead;
        
        // Cleanup
        free (block);
        
        // Return new pointer location
        return openFileList[fd].pointer;
    }
    // When we need to read something between blocks, we will do it recursively
    else {
        // Calculate number of bytes we will read from this block
        uint64_t numBytesToRead = (int)(blockSize - currentOffset);
        
        // Pull entire LBA block
        void *block = malloc(blockSize);
        LBAread(block, 1, currentBlock);
        
        // Copy what we need from LBA block into src
        memcpy((src + bytesAlreadyRead), (block + currentOffset), numBytesToRead);
        
        // Cleanup
        free (block);
        
        // Update pointer location
        openFileList[fd].pointer += numBytesToRead;
        
        // Call fsReadHelper again, this time with an update bytesAlreadyRead, since we are further along in src
        return myFsWriteHelper(fd, src, length, (bytesAlreadyRead + numBytesToRead), blockSize, openFileList);
    }
}

int copyFromLinux(char * sourcePath, char * destinationPath, uint16_t blockSize, struct openFileDirectory *openFileList) {
    // Open linux file
    FILE *sourceFile = fopen(sourcePath, "r");
    if (sourceFile == NULL) {
        printf("Unable To Copy From Linux. Source File Does Not Exist.\n");
        return -1;
    }
    
    // Save linux file size
    fseek(sourceFile, 0L, SEEK_END);
    uint64_t linuxFileSize = ftell(sourceFile);
    fseek(sourceFile, 0L, SEEK_SET);
    
    // Save original directory
    uint64_t originalDirectory = getVCBCurrentDirectory(blockSize);
    
    // CD to directory
    int returnStat = changeDirectory(destinationPath, 1, blockSize);
    if (returnStat < 0)
    {
        printf("Destination Path Not Valid.\n\n");
        return -1;
    }
    
    // Save parent location
    uint64_t parentLocation = getVCBCurrentDirectory(blockSize);
    
    // Change back to original directory
    setVCBCurrentDirectory(originalDirectory, blockSize);
    
    // Create a new file
    uint64_t newFileBlockLocation = createFileDirectory("TESTCOPY", "txt", linuxFileSize, parentLocation, blockSize);
    
    // Open our system file
    int destinationFileFD = myFsOpen(newFileBlockLocation, -1, blockSize, openFileList);
    
    // TODO: FINISH THIS FUNCTION
    
    return 0;
}
