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
    char *token;
    do {
        token = strtok(command, delimiters);
    } while(token != NULL);
}
