/**************************************************************
* Class: CSC-415-0# Spring 2020
* Group Name: Last Minute
* Name: Pedro Souto
* Student ID: 918412864
* Name: Cassie Sherman
* Student ID: 918192878
* Name:
* Student ID:
* Name:
* Student ID:
*
* Project: Assignment 3 – File System
* File: inputParser.h
*
* Description: Header file containing all header functions implemented needed to parse user input.
* **************************************************************/

#ifndef inputParser_h
#define inputParser_h
#include <stdio.h>
#endif /* inputParser_h */

// Takes in user input, and validates if it is a valid input or not
// Returns 1 if inout is valid
// Return 0 if input is invalid
int userInputIsValid (int argc, char *argv[]);

// Takes in a command list, and executes the command
// This function should only be called when the command/arguments have been validated by userInputIsValid()
void executeCommand (int argc, char *argv[], uint64_t blockSize);
