/* 
 * File:   parse.h
 * Authors: Luke Kledzik, Adam Mooers
 */

#ifndef PARSE_H
#define PARSE_H

/**
 * MAXARGS defined to represent the upper bound allowed for arguments read.
 */
#define MAXARGS 32

/**
 * This struct holds the data gathered from stdin after running myshell.
 * 
 * inputRedirect is the location where data is read from if desired.
 * outputRedirect is the location where data is written to if desired.
 * argumentCount is the number of arguments entered.
 * argumentVector is an array holding the arguments entered. The size of the array is stored in argumentCount.
 */
struct PARAM {
    char *inputRedirect;           /* file name or NULL */
    char *outputRedirect;          /* file name or NULL */
    int  argumentCount;            /* number of tokens in argument vector */
    char *argumentVector[MAXARGS]; /* array of strings */
};

/**
 * Typedef for the PARAM struct. Param_t is now usable instead of struct PARAM.
 */
typedef struct PARAM Param_t;

/**
 * Prints the data currently stored in the Param_t structure.
 * Used when the shell is run in -Debug mode.
 */
void printParams(Param_t* param);

/**
 * Breaks down the commands entered within the myshell program.
 * Stores input and output redirects (if included).
 * Stores arguments into an array.
 */
void tokenize(char command[], const char delimiters[], Param_t *param);

/**
 * Determines if a given string can be converted to a valid integer.
 * The integer may start with a plus or minus sign. The length of the
 * integer is not accounted for.
 */
int isInt(const char* str);

/**
 * Formats the argument vector (argv) for new execv processes. Each new process 
 * has a new index, so this value needs to be computed dynamically.
 */
char** formatChildArgV(const Param_t* inputCmd, int curI);

#endif
