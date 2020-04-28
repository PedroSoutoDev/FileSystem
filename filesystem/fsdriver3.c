/**************************************************************
* Class: CSC-415-0# Spring 2020
* Group Name: Last Minute
* Name: Pedro Souto
* Student ID: 918412864
* Name: Aaron Schlichting
* Student ID: 917930213
* Name:
* Student ID:
* Name:
* Student ID:
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
#include "fsImplementation.h"
#include "inputParser.h"

// Maximum size of user command input
#define BUFFERSIZE 128

// Default name for the partition
#define PARTITION_NAME "Pedro's HDD"

int main (int argc, char *argv[]) {
    // Ensure the correct number of arguments
    // argv[0]: Program Name
    // argv[1]: File Name
    // argv[2]: Volume Size
    // argv[3]: Block Size
    if (argc != 4) {
        printf("In correct number of parameters. Please use the following parameters:\n");
        printf("argv[1]: File Name\n");
        printf("argv[2]: Volume Size\n");
        printf("argv[3]: Block Size\n");
        exit(1);
    }
    
    // For DEVELOPMENT. Will be removed later. This deletes the text file, thus starting a fresh partition on each run
    // TODO: Remove before turning in!
    remove(argv[1]);
    
    // Start Partition System
    printf("-------------------------------------------------------\n");
    printf("OPENING FILE SYSTEM...\n");
    char * filename = argv[1];
    uint64_t volumeSize = atoll (argv[2]);
    uint64_t blockSize = atoll (argv[3]);
    int startPartitionReturn = startPartitionSystem (filename, &volumeSize, &blockSize);

    // Make sure partition was successfully create/opened
    if (startPartitionReturn == 0) {
        printf("FILE SYSTEM OPENED SUCSESSFULLY!\n");
    } else {
        printf("ERROR OPENING FILE SYSTEM!\n");
        exit(1);
    }
    printf("-------------------------------------------------------\n\n");
    
    // If a Volume Control Block has not been created before, create it now
    if (!hasVolumeControlBlock(blockSize)) {
        // This will create the Volume Control Block AND it will also initialize the Free Space Information blocks AND it will create the root directory
        initializeVolumeControlBlock(volumeSize, PARTITION_NAME, blockSize);
    }
    
    // Set the current directory back to the root at launch
    setVCBCurrentDirectory(getVCBRootDirectory(blockSize), blockSize);
    
    // For DEVELOPMENT. Will be removed later. This creates some sample directories, thus starting the file system with some basic directories (Pictures, Videos, Documents, ...etc)
    // TODO: Remove before turning in!
    sampleCreateDirectories(blockSize);
    
    // Main loop of program, where we ask for user input then execute t that functionality
    char userInput[BUFFERSIZE];
    char *argList[BUFFERSIZE];
    char *token;
    printCommands();
    while (1) {
        // Print prompt
        printf("> ");
        
        // Get user input
        fgets(userInput, BUFFERSIZE, stdin);
        
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
        
        // Set the element after the last element to null, so we know when to stop looking for more arguments
        argList[argc] = NULL;
        
        // If the user did not enter anything or they entered too many things, consider input invalid
        if (argc <= 0 || argc > 5) {
            printf("Invalid Command.\n");
            continue;
        }
        
        // Check if the command is valid. If it is, execute the command
        if (userInputIsValid(argc, argList)) {
            executeCommand(argc, argList , blockSize);
        }
    }
}
