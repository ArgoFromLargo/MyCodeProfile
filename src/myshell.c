/* 
 * File:   myshell.c
 * Author: lkledzik
 *
 * Created on August 24, 2016, 4:04 PM
 */

#include <stdio.h>
#include <string.h>
#include "parse.h"

#define CMD_BUFFER_LEN 500

/*
 * 
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
