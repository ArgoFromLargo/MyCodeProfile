# SysNet1Proj1

# OVERVIEW
This assignment will require you to implement a new shell that parses a command entered by the user and creates a set of new processes to run a distributed computing application. The shell requires redirection of input and output and prevention of zombie processes. Your shell will not behave like a typical shell to process commands at the command line; rather it is designed to help users launch multiple processes simultaneously to solve computing problems cooperatively.

# THE PROGRAM
This is a two-part project to implement the program myshell. In the first part, your program must parse an input string into a structure. The structure must then be used by the second part to 1) create a set of new processes based on the input on the shell's command line and 2) handle input and output redirection for the new processes.

# PART 1:
Your program must perform the following actions:
1. Display a prompt on stdout.
2. Accept a command as a string from the user (input string will terminate with a newline character). The
program must terminate when the command exit is entered.
3. Parse the input string into tokens, which are single words delimited by one or more spaces, tabs, or
newline characters.
4. Store the tokens in a provided structure shown below.
5. Print the contents of the structure using the printParams() function, which is provided and explained
below, but only when the shell is started with the debug option –Debug. Otherwise, the contents of the
string will not be printed.
6. Return back to step 1.
The structure used to store the parsed input is shown below. I have included a defined value to indicate the maximum number of tokens you will find on the command line.

/* don’t test program with more than this many tokens for input */
#define MAXARGS 32
/* structure to hold input data */
struct PARAM
{
char *inputRedirect;
char *outputRedirect;
int  argumentCount;
char *argumentVector[MAXARGS]; /* array of strings */
DISTRIBUTED COMPUTING
/* file name or NULL */
/* file name or NULL */
/* number of tokens in argument vector */
};
/* a typedef so we don’t need to use "struct PARAM" all the time */
typedef struct PARAM Param_t;
