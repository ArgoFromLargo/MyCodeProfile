/*
 * File: parse.c
 * Author: lkledzik
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

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

void tokenize(char command[], const char delimiters[], Param_t *param) {
    param->inputRedirect = NULL;
    param->outputRedirect = NULL;
    char *token;
    int debugMode = 0;
    token = strtok(command, delimiters);
    
    if(strcmp(token, "-Debug") == 0 || strcmp(token, "-debug") == 0)
        debugMode = 1;
    
    while(token != NULL) {
        if(*token == '<') {
            token++;
            param->inputRedirect = token;
        }
        else if(*token == '>') {
            token++;
            param->outputRedirect = token;
        }
        else {
            param->argumentVector[param->argumentCount] = token;
            param->argumentCount++;
        }
        
        token = strtok(NULL, delimiters);
    }
    
    if(debugMode)
        printParams(param);
}
