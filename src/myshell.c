/* 
 * File:   myshell.c
 * Author: Luke Ledzik, Adam Mooers
 *
 * Created on August 24, 2016, 4:04 PM
 */

#include <stdio.h>
#include <string.h>
#include "parse.h"

#define CMD_BUFFER_LEN 500

/*
 * Processes a tokenized shell command. If the input is properly-formatted,
 * the shell will create and manage an arbitrary number of specified child
 * processes. In the case that the input is not correctly formatted, an error
 * message will describe the problem. Input piping (<) and output piping (>)
 * of child processes are supported and optional.
 *
 * @param inputCmd the tokenized input command from myshell. 
 */
void processCmd(const Param_t* inputCmd);

/*
 * The entrance point for the shell. The user is prompted for
 * a myshell command. If they enter the exit command, the session
 * terminates. Otherwise, they can use the shell to start and manage
 * a arbitrary number of child processes (see attemptExec for command
 * formatting and implementation) The -Debug flag allows the user
 * to view their tokenized input.
 * 
 * @param argc number of arguments from shell
 * @param argv arguments from the shell (not the same as myshell arguments)
 * @return 0, if no errors occur, otherwise a non-zero value
 */
int main(int argc, char** argv) {

    char command[CMD_BUFFER_LEN];
    const char delimiters[] = " \t\n";

    printf("Enter a command with no more than %d arguments.\n", MAXARGS);
    fgets(command, CMD_BUFFER_LEN, stdin);
    
    // Check to see if the exit command is issued
    if(!strcmp(command, "exit\n")) {
        printf("Program terminated.\n");
        return 0;
    }
    
    Param_t inputCommand;
    tokenize(command, delimiters, &inputCommand);

    // Check if the debug flag is set
    if(strcmp(argv[1], "-Debug") == 0) {
        // -debug is set, so print arguments
        printParams(&inputCommand);
    }
   
    return 0;
}
