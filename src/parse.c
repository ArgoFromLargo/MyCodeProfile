/*
 * File: parse.c
 * Author: lkledzik
 * Last Update: 9/12/2016 3:29 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

/*
 * Prints the contents of the Param_t* that is passed
 *
 * Parameters: Param_t*
 * Return: void
 */
void printParams(Param_t *param) {
    int i;
    printf ("InputRedirect: [%s]\n",
      (param->inputRedirect != NULL) ? param->inputRedirect:"NULL");
    printf ("OutputRedirect: [%s]\n",
      (param->outputRedirect != NULL) ? param->outputRedirect:"NULL");
    printf ("ArgumentCount: [%d]\n", param->argumentCount);
    for (i = 0; i < param->argumentCount; i++)
           printf("ArgumentVector[%2d]: [%s]\n", i, param->argumentVector[i]);
}

/*
 * Tokenizes the string of chars the user types into the shell
 *
 * Parameters: char[], const char[], Param_t*
 * Return: void
 */
void tokenize(char command[], const char delimiters[], Param_t *param) {
    // set the Param_t* to default values before tokenizing
    param->inputRedirect = NULL;
    param->outputRedirect = NULL;
    param->argumentCount = 0;
    char *token;
    int debugMode = 0;
    token = strtok(command, delimiters);
    
    if(strcmp(token, "-Debug") == 0 || strcmp(token, "-debug") == 0) { // tests for -Debug flag
        debugMode = 1;
        token = strtok(NULL, delimiters); // skips this argument before reading into argumentVector
    }
    
    while(token != NULL) {
        if(*token == '<') {
            token++;
            param->inputRedirect = token; // sets inputRedirect if < is read before argument
        }
        else if(*token == '>') {
            token++;
            param->outputRedirect = token; // sets outputRedirect if > is read before argument
        }
        else {
            param->argumentVector[param->argumentCount] = token; // adds arg to array if not input or output redirect
            param->argumentCount++;
        }
        
        token = strtok(NULL, delimiters); // set token to next delimiter
    }
    
    if(debugMode)
        printParams(param); // print parameters if -Debug mode is on
}
