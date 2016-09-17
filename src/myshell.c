/* 
 * File:   myshell.c
 * Authors: Luke Kledzik, Adam Mooers
 *
 * Created on August 24, 2016, 4:04 PM
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "parse.h"

#define CMD_BUFFER_LEN 500
#define SHELL_USAGE "Usage: command count [child_argument]*"

/*
 * Processes a tokenized shell command. If the input is properly-formatted,
 * the shell will create and manage an arbitrary number of specified child
 * processes. In the case that the input is not correctly formatted, an error
 * message will describe the problem. Input piping (<) and output piping (>)
 * of child processes are supported and optional.
 * 
 * Command format:
 *   - child_process n [child_argument]*
 *
 * Note(0): * indicates 0 or more of the item in brackets
 * Note(1): child_process indicates the child process to run
 * Note(2): n is an integer which indicates the number of child_process to create
 *
 * See execCmd(...) for information on how the child processes recieve the formatted
 * information.
 *
 * Note(4): i is the index of the child, in the order that they are executed
 *
 * @param inputCmd the tokenized input command from myshell. 
 */
void processCmd(const Param_t* inputCmd);

/*
 * Executes the given child process a given number of times with the given arguments.
 * The command should be properly formatted by the time this stage is reached. The actual
 * fork-exec occurs at this point, so mal-formatted input could forkbomb to the host OS.
 *
 * Each child process recieves the presented arguments in the following manner:
 *   - child_process n i [child_argument]*
 *
 * Note (1): i is the index of the process, starting at zero, ending at n-1
 *
 * @param n The number of instances of child_process to create (correctly formatted)
 * @param inputCmd original, user-defined, tokenized arguments
 */
void execCmd(int n, Param_t* inputCmd);

/*
 * Waits for any open child process to finish and accepts their exit codes. This should
 * be run after execCmd(..) to prevent zombie processes and the grader's wrath.
 *
 * @param n The number of child processes that need to be closed
 */
void waitChildren(int n);

/*
 * The entrance point for the shell. The user is prompted for
 * a myshell command. If they enter the exit command, the session
 * terminates. Otherwise, they can use the shell to start and manage
 * a arbitrary number of child processes (see attemptExec for command
 * formatting and implementation) The -Debug flag allows the user
 * to view their tokenized input.
 * 
 * @param argc number of arguments from shell
 * @param argv arguments from the shell (not the same as myshell arguments)
 * @return 0, if no errors occur, otherwise a non-zero value
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
    if(argc > 1 && strcmp(argv[1], "-Debug") == 0) {
        // -debug is set, so print arguments
        printParams(&inputCommand);
    }
 
    // Check if the input is correctly-formatted
    // Run the command if it is formatted correctly.
    processCmd(&inputCommand);
    return 0;
}

void processCmd(const Param_t* inputCmd)
{
    // Make sure the minimum number of arguments have been added
    if (inputCmd->argumentCount < 2) {
        printf("myshell: missing operand\n%s\n", SHELL_USAGE);
        return;
    }

    // Check if the program in arg[0] exists
    if (access(inputCmd->argumentVector[0], X_OK) != 0) {
        // File does not exist, or user lacks exe permissions
        printf("myshell: \"%s\" cannot execute.\n%s\n", 
            inputCmd->argumentVector[0], 
            SHELL_USAGE);
        return;
    }
    
    // Determine if n can be formatted correctly
    if (!isInt(inputCmd->argumentVector[1])) {
        printf("That's not an int!\n");
    }
    
    
    // Fork the process n times and exec
    
}

void execCmd(int n, Param_t* inputCmd)
{

}
