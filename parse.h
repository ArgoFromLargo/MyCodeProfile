/* 
 * File:   parse.h
 * Author: lkledzik
 */

#ifndef PARSE_H
#define PARSE_H
#define MAXARGS 32                 /* structure to hold input data */

struct PARAM {
    char *inputRedirect;           /* file name or NULL */
    char *outputRedirect;          /* file name or NULL */
    int  argumentCount;            /* number of tokens in argument vector */
    char *argumentVector[MAXARGS]; /* array of strings */
};

typedef struct PARAM Param_t;

void printParams(Param_t* param);
void tokenize(char command[], const char delimiters[], Param_t *param);

#endif

