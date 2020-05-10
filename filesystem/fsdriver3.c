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
* File: driver.c
*
* Description: Main driver for the file sytem.
* **************************************************************/

// Required C libraries
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>

// Bierman Files
#include "fsLow.h"

// Group Files
#include "fsStructures.h"
#include "inputParser.h"
#include "fsImplementation.h"

// Size of file system partition (5MB default)
#define VOLUME_SIZE 5000000

// Size of LBA block (512 default)
#define LBA_BLOCK_SIZE 512

// Default name for the partition
#define PARTITION_NAME "Pedro's HDD"

// Maximum size of user command input
#define INPUT_BUFFER_SIZE 128

int main (int argc, char *argv[]) {
    // Ensure the correct number of arguments
    // argv[0]: Program Name
    // argv[1]: File Name
    if (argc != 2) {
        printf("Incorrect number of parameters. Please use only ONE parameters: File Path\n");
        exit(1);
    }
    
    // Start Partition System
    printf("---------------------------------------------------------------------------------------------\n");
    printf("OPENING FILE SYSTEM...\n");
    char * filename = argv[1];
    uint64_t volumeSize = VOLUME_SIZE;
    uint64_t blockSize = LBA_BLOCK_SIZE;
    int startPartitionReturn = startPartitionSystem (filename, &volumeSize, &blockSize);

    // Make sure partition was successfully create/opened
    if (startPartitionReturn == 0) {
        printf("FILE SYSTEM OPENED SUCSESSFULLY!\n");
    } else {
        printf("ERROR OPENING FILE SYSTEM!\n");
        exit(-1);
    }
    printf("---------------------------------------------------------------------------------------------\n\n");

    // If a Volume Control Block has not been created before, create it now
    if (!hasVolumeControlBlock(blockSize)) {
        // This will create the Volume Control Block AND it will also initialize the Free Space Information blocks AND it will create the root directory
        initializeVolumeControlBlock(volumeSize, filename, PARTITION_NAME, blockSize);
        
        // This creates some sample directories, thus starting the file system with some basic directories (Pictures, Movies, Documents, ...etc)
        createDefaultDirectories(blockSize);
    }
    
    // Set the current directory to the root directory at launch
    setVCBCurrentDirectory(getVCBRootDirectory(blockSize), blockSize);
    
    // Create array for all open files
    struct openFileDirectory * openFileList = (struct openFileDirectory *)malloc(sizeof(struct openFileDirectory) * FDOPENMAX);
    
    // Check if memory allocated correctly
    if (openFileList == NULL) {
        printf("Unable to allocate memory space. Program terminated.\n");
        exit(-1);
    }
    
    // Set flag to free for the files in openFileList, since there are on open files on launch
    for (int i = 0; i < FDOPENMAX; i++) {
        openFileList[i].flags = FDOPENFREE;
    }
    
    // On launch, label and print commands
    printf("*** COMMANDS ***\n");
    printCommands();
    
    // Main loop which takes user input, then executes the correct file system functionality
    char userInput[INPUT_BUFFER_SIZE];
    char *argList[INPUT_BUFFER_SIZE];
    char *token;
    while (1) {
        // Print prompt
        printf("> ");
        
        // Get user input
        fgets(userInput, INPUT_BUFFER_SIZE, stdin);
        
        // Override the last char of input to be a null-terminator
        userInput[strlen(userInput) - 1] = '\0';
        
        // Tokenize user input into arguments list
        int argc = 0;
        token = strtok(userInput, " ");
        while (token != NULL) {
            argList[argc] = token;
            argc++;
            token = strtok(NULL, " ");
        }
        
        // If the user did not enter anything or they entered too many things, consider input invalid
        if (argc <= 0 || argc > 5) {
            printf("Invalid Command.\n");
            continue;
        }
        
        // Check if the command is valid. If it is, execute the command
        if (userInputIsValid(argc, argList)) {
            executeCommand(argc, argList , blockSize, openFileList);
        }
    }
}
