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

/*
 * 
 */
int main(int argc, char** argv) {

    char command[500];
    const char delimiters[] = " \t\n";
    Param_t *inputCommand = (Param_t *)malloc(sizeof(Param_t));

    printf("Enter a command with no more than 32 arguments.\n");
    fgets(command, 500, stdin);
    if(strcmp(command, "exit\n") == 0) {
        printf("Program terminated.\n");
        return 0;
    }
    else {
        tokenize(command, delimiters, inputCommand);
    }
        
    return 0;
}
