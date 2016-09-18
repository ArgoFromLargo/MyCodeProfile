/*
 * File: parse.c
 * Author: lkledzik
 * Last Update: 9/12/2016 3:29 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parse.h"

/*
 * Defines the maximum number of base 10 digits required to represent
 * a 32-bit unsigned integer: MAX = floor(log10(2^32-1))+2
 */
#define INT_MAX_CHARS 11

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

    char *token = strtok(command, delimiters);
    
    while(token != NULL) {
        if(*token == '<') {
            token++;
            
            // sets inputRedirect if < is read before argument
            param->inputRedirect = token;
        }
        else if(*token == '>') {
            token++;

            // sets outputRedirect if > is read before argument
            param->outputRedirect = token;
        }
        else {
            // adds arg to array if not input or output redirect
            param->argumentVector[param->argumentCount] = token;

            param->argumentCount++;
        }
        
        token = strtok(NULL, delimiters); // set token to next delimiter
    }
}

/*
 * Determines if a given string can be converted to a valid integer.
 * The integer may start with a plus or minus sign. The length of the
 * integer is not accounted for.
 *
 * @param str the string to validate
 * @return whether str can be converted to a valid integer
 */
int isInt(const char* str) {
    // Account for sign
    if (*str == '+' || *str == '-') str++;   
 
    // Check each digit
    do {
        if (!isdigit(*str)) return 0;
    } while (*(++str) != '\0');

   return 1;  // All characters met the criteria
}

/*
 * Formats the argument vector (argv) for new execv processes. Each new process 
 * has a new index, so this value needs to be computed dynamically.
 *
 * @param inputCmd the validated, tokenized input from myshell. 
 * @param curI the current index of the process (see execCmd description)
 * @return A reference to the newly-concatenated string
 */
char* formatChildArgV(const Param_t* inputCmd, int curI) {
    // Allocate memory for the int string (plus the null pointer)
    char curIStr[INT_MAX_CHARS+1];
    sprintf(curIStr, "%d", curI);

    // Calculate the size of the output
    int argBufferLen = strlen(curIStr);

    int j,k;

    // Add the lengths of the argument vector members
    for (j=1; j<inputCmd->argumentCount; j++) {
        argBufferLen += strlen(inputCmd->argumentVector[j]);
    }

    // Allocate memory for the concatenated array
    // Adding argumentCount is for spaces
    char* argv = (char*) malloc(argBufferLen+inputCmd->argumentCount+1);
    
    // Get a reference to the first char position
    char* cpPos = argv;
    
    // Copy all of the tokens into the new array
    for (j=1; j<inputCmd->argumentCount; j++) {
    
        // insert curI into the second position
        if (j == 2) {
            strcpy(cpPos, curIStr);
            cpPos += strlen(curIStr);
            *(cpPos++) = ' ';
        }
    
        // For each character k in the jth arg vector
        for(k=0; inputCmd->argumentVector[j][k] != '\0'; k++) {
            *(cpPos++) = inputCmd->argumentVector[j][k]; // Copy char to argv
        }
                
        // Add space to separate tokens
        *(cpPos++) = ' ';
    }
    
    *(--cpPos) = '\0';
    printf("argv:%s\n", argv);
    return argv;
}
