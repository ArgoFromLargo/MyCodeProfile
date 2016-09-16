/* 
 * File:   myshell.c
 * Author: lkledzik
 *
 * Created on August 24, 2016, 4:04 PM
 */

#include <stdio.h>
#include <stdlib.h>
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
    
    if(!strcmp(command, "exit\n")) {
        printf("Program terminated.\n");
        return 0;
    }
    
    Param_t *inputCommand = (Param_t *) malloc(sizeof(Param_t));
    tokenize(command, delimiters, inputCommand);

    if(strcmp(argv[1], "-Debug") == 0 || strcmp(argv[1], "-debug") == 0) {
        printParams(inputCommand);
    }

        
    return 0;
}
