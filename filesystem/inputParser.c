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
* File: inputParser.c
*
* Description: Implementation of all functionalities the user parser  uses.
* **************************************************************/

#include <string.h>
#include <stdlib.h>

#include "inputParser.h"
#include "fsImplementation.h"

void executeCommand (int argc, char *argv[], uint64_t blockSize) {
    // Find the corresponding command, and call its proper execute function
    if(strcmp(argv[0],"ls") == 0) {
        listDirectories(getVCBCurrentDirectory(blockSize), blockSize);
    }
    else if (strcmp(argv[0],"cd") == 0) {
        changeDirectory(argv[1], blockSize);
    }
    else if (strcmp(argv[0],"mkdir") == 0) {
        createDirectory(argv[1], getVCBCurrentDirectory(blockSize), blockSize);
    }
}

int userInputIsValid (int argc, char *argv[]) {
    // Array of valid commands
    const char * validCommands[] = {
        "ls",
        "cd",
        "mkdir",
        "mkfile",
        "rmfile",
        "cpyfile",
        "mvfile",
        "setdata",
        "TODO: Copy from the normal filesystem to this filesystem",
        "TODO: Copy from this filesystem to the normal filesystem",
        "menu",
        "m",
        "exit",
        "e",
    };
    
    // Array listing the number of arguments each of the above commands should have. They are ordered, so think of this as a key value pair
    const int numberOfArgumentsPerCommand[] = {
        0,
        1,
        1,
        2,
        1,
        2,
        2,
        2,
        99,
        99,
        0,
        0,
        0,
        0,
    };
    
    // Number of possible commands. Comes from the above array
    int numberOfCommands = 14;
    
    // Check that the first argument is a valid command from the list
    for (int i = 0; i < numberOfCommands; i++) {
        // Check to make sure that the command is valid
        if (strcmp(validCommands[i], argv[0]) == 0) {
            // Check to make sure that the command has the correct number of arguments
            if ((argc - 1) == numberOfArgumentsPerCommand[i]) {
                return 1;
            }
            // If the command is valid, but the arguments are not, report that as a failure
            else {
                printf("Invalid Arguments.\n\n");
                return 0;
            }
        }
        // If we checked all valid commands, and none matched, then this is not a valid command
        else if (i == (numberOfCommands - 1)){
            printf("Invalid Command.\n\n");
            return 0;
        }
    }
    
    // Failsafe catch, return 0
    return 0;
}
