/* 
 * File:   parse.h
 * Author: lkledzik
 */

#ifndef PARSE_H
#define PARSE_H
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

void printParams(Param_t* param);

void tokenize(char command[], const char delimiters[], Param_t *param);

int isInt(const char* str);

char* formatChildArgV(const Param_t* inputCmd, int curI);

#endif
