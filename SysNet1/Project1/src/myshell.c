/**
 * myshell.c runs the shell. This includes prompting the user for input then processing, 
 * interpreting, and validating that input. Algorithms for managing processes are integrated
 * as well. Because the implementation is dependent on forking, this code will only run on Unix-
 * compatible machines.
 *
 * @author Adam Mooers
 * @author Luke Kledzik
 * @date 9/18/2016
 * @info Course COP4634
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "parse.h"

#define CMD_BUFFER_LEN 500
#define SHELL_USAGE "Usage: command count [child_argument]*"

/**
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

/**
 * Executes the given child process a given number of times with the given arguments.
 * The command should be properly formatted by the time this stage is reached. The actual
 * fork-exec occurs at this point, so mal-formatted input could forkbomb to the host OS.
 * n forks will be attempted, but in the case one fails to launch, the unlaunched processes
 * will be canceled. execCmd automatically waits for started processes to prevent zombies.
 * When all have run, the function returns.
 *
 * Each child process recieves the presented arguments in the following manner:
 *   - child_process n i [child_argument]*
 *
 * Note (1): i is the index of the process, starting at zero, ending at n-1
 *
 * @param n The number of instances of child_process to create (correctly formatted)
 * @param inputCmd original, user-defined, tokenized arguments
 */
void execCmd(int n, const Param_t* inputCmd);

/**
 * Attempts to redirect a file to another gracefully. If this fails, an error
 * is printed to the terminal.
 *
 * @param redirect the name of the new file to redirect to
 * @param mode the mode of the permissions
 * @param source the file to redirect
 *
 * @return: 0 if failure to redirect input or output, !0 otherwise.
 */
int redirFile(const char* redirect, const char * mode, FILE* source);

/**
 * Waits for any open child process to finish and accepts their exit codes. This should
 * be run after execCmd(..) to prevent zombie processes and the grader's wrath.
 *
 * @param n The number of child processes that need to be closed
 */
void waitChildren(int n);

/**
 * The entrance point for the shell. The user is prompted for
 * a myshell command. If they enter the exit command, the session
 * terminates. Otherwise, they can use the shell to start and manage
 * a arbitrary number of child processes (see attemptExec for command
 * formatting and implementation) The -Debug flag allows the user
 * to view their tokenized input.
 * 
 * @param argc number of arguments from shell
 * @param argv arguments from the shell (not the same as myshell arguments)
 *
 * @return 0, if no errors occur, otherwise a non-zero value
 */
int main(int argc, char** argv) {

    char command[CMD_BUFFER_LEN];
    const char delimiters[] = " \t\n";
    
    // Enter the terminal loop
    while(1) {
        printf("$$$ ");
        fgets(command, CMD_BUFFER_LEN, stdin);
        
        // Check to see if the exit command is issued
        if(!strcmp(command, "exit\n")) {
            printf("Program terminated.\n");
            break;
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
    }
    
    return 0;
}

void processCmd(const Param_t* inputCmd) {
    // Make sure the minimum number of arguments have been added
    if (inputCmd->argumentCount < 2) {
        printf("myshell: missing operand\n%s\n", SHELL_USAGE);
        return;
    }

    // Check if the program in arg[0] exists
    if (access(inputCmd->argumentVector[0], X_OK) != 0) {
        // File does not exist, or user lacks exe permissions
        printf("myshell: \"%s\" is not a recognized command.\n%s\n", 
            inputCmd->argumentVector[0], 
            SHELL_USAGE);
        return;
    }
    
    // Determine if n can be formatted correctly
    if (!isInt(inputCmd->argumentVector[1])) {
        printf("myshell: the second argument must"
               " be an integer.\n%s\n", SHELL_USAGE);
        return;
    }
    
    // Get the number of child processes to open
    int n = atoi(inputCmd->argumentVector[1]);
    
    if (n < 1) {
        printf("myshell: count must be > 0\n%s\n", SHELL_USAGE);
        return;
    }
    
    // Check if input redirect file is valid
    if (inputCmd->inputRedirect != NULL &&
        access(inputCmd->inputRedirect, R_OK) != 0) {
        // File does not exist, or shell lacks read permissions
        printf("myshell: %s: Not available for reading.\n%s\n", 
            inputCmd->inputRedirect, 
            SHELL_USAGE);
        return;
    }
    
    // Check if output redirect matches input redirect
    if (inputCmd->outputRedirect != NULL &&
        inputCmd->inputRedirect != NULL &&
        strcmp(inputCmd->outputRedirect, inputCmd->inputRedirect) == 0) {
        // Input and output files match, but should not
        printf("myshell: inputRedirect should not match outputRedirect.\n%s\n", 
            SHELL_USAGE);
        return;
    }
    
    // Check if output redirect file already is exists, delete if so
    if (inputCmd->outputRedirect != NULL &&
        access(inputCmd->outputRedirect, F_OK) == 0) {
        remove(inputCmd->outputRedirect);
    }

    // Fork the process n times and exec
    execCmd(n, inputCmd);
}

void execCmd(int n, const Param_t* inputCmd) {
    int forkCount = 0; // The number of forks that were actually successful.
    int i;
    
    // Fork n times
    for (i=0; i<n; i++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            // The current fork failed.
            printf("Unable to launch the %d\n process. Cancelling queue.", forkCount);
            break;
        }
        
        if (pid == 0) {
            // If in child process
            
            // Format the user command for the child process
            // Note the exec frees childArgv
            char** childArgv = formatChildArgV(inputCmd, i);
            
            if (redirFile(inputCmd->inputRedirect, "rb", stdin) &&
                redirFile(inputCmd->outputRedirect, "a", stdout)) {
                // Launch the new exec
                execv(*childArgv, childArgv);
            }
            
            //inputCmd->inputRedirect
            printf("Exec has failed to launch a new process.\n");
            
            // Stop-gap fork bomb stopper, in case of a exec error
            exit(0);
        }
        
        forkCount++;
    }
    
    // Wait for all successful children to finish before returning
    waitChildren(forkCount);
}

void waitChildren(int n) {
    int status;
    int i;
    
    // Wait for all n processes to close
    // Note that the ith index may not match the ith process created
    // Use ps-axu | grep "Z" in terminal to view potential zombies
    for (i=0; i<n; i++) {
        wait(&status);
        //printf("In Wait: PID = %d\n", pid);
    }
}

int redirFile(const char* redirect, const char * mode, FILE* source) {
    FILE* redirStatus;
    
    // Redirect output to a file if specified
    if (redirect != NULL) {
        // Open with append permissions to prevent overwriting
        redirStatus = freopen(redirect, mode, source); 
        
        if (redirStatus == NULL) {
            // outputRedirect failed
            printf("Redirecting to %s has failed.\n", redirect);
            return 0;
        }
    }
    
    return 1;
}
