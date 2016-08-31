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
        char *inputRedirect;            /* file name or NULL */
        char *outputRedirect;           /* file name or NULL */
        int  argumentCount;             /* number of tokens in argument vector */
        char *argumentVector[MAXARGS];  /* array of strings */
    };
    /* a typedef so we don’t need to use "struct PARAM" all the time */
    typedef struct PARAM Param_t;

Notice that the first three components of the struct are special. The first two describe the file name for either input redirection or output redirection, if desired. The third describes the number of tokens in the argument vector entered at the command-line. Consider this line of input shown with the prompt $$$.

    $$$ one two three <four >five
    
When the line is parsed, the first three tokens are not special because they do not start with a beginning character such as '<' or '>', so they should be placed in argumentVector[0], argumentVector[1], and argumentVector[2] respectively. The argument counter named argumentCount should be set to three. When the fourth token is extracted, it is identified as an input redirection because of the beginning character (’<’). The characters following immediately the redirection indicator form the name of the file from which input should be read. The name of the input file (“four”) should be stored in inputRedirect. Similarly, the beginning character (‘>’) of the fifth token identifies output redirection and the characters following the redirect character specifies the name of the file to which output should be sent. The name of the output file (“four”) should be stored in outputRedirect. Please do not allow for spaces between the beginning character for redirection ('<' or '>') and the file name. If input or output redirection is not specified on the command line, your program needs to set the corresponding fields in the structure to NULL. Overall, an acceptable input is a single text line ending by a new line character that follows the syntax as shown below:

    [token [' '|'\t']+]* [token] [' '|'\t']+ [<input [' '|'\t']+] [>output [' '| '\t']+]

In this notation, [ ] indicates an optional parameter, * indicates 0 or more times the value in the brackets, + indicates1ormoretimesthevalueinthebrackets,and| indicatesalternatives.
Once the input line is parsed and the structure elements are properly set, you must print the structure with the following function when the shell is in debug mode.

    void printParams(Param_t * param)
    {
        int i;
        printf ("InputRedirect: [%s]\n",
          (param->inputRedirect != NULL) ? param->inputRedirect:"NULL");
        printf ("OutputRedirect: [%s]\n",
          (param->outputRedirect != NULL) ? param->outputRedirect:"NULL");
        printf ("ArgumentCount: [%d]\n", param->argumentCount);
        for (i = 0; i < param->argumentCount; i++)
           printf("ArgumentVector[%2d]: [%s]\n", i, param->argumentVector[i]);
    }

# PART 2:
You must extend the previous program to create a specific shell program that runs distributed computing solutions. The functionality of your shell program is different from that of a production shell such as a c-shell or a bourne shell found in many operating systems because the shell will launch several instances of a program. Your shell reads input from a command line and interprets it. Your shell must handle input/output redirection and interpret an argument vector to run a specified program with its own parameter as follows:
- argumentVector[0]: this element will define the name of a program that must be executed.
- argumentVector[1]: this element will define a counter that specifies the number of times the program must be launched as a separate process/instance.
- All other elements in the argument vector must be passed to the specified program.

Upon interpreting the values in the argument vector, your shell must launch kinstancesof the specified program using  system  call fork() where kis  the  number  specified  in argumentVector[1].It  must  generate  a  new argument vector for each processto become the command lineargumentsread in main(). However, in order for the  processes  to  work cooperatively  on  solving  a  problem the  shell  must  generate  a  separate  index  for  each process and passesitto the process in the new argument vector. The program will use this index to apply a divide and conquer strategy on the data for solving aproblem. The new index should be inserted as a separate value into the previously generated argument vector to create a new argument vector. You must decide for yourself whether you  can  use  the  existing  argument  vector  in  the  structure  or  create  a  new  argument  vectorto  store  the  new values.

Here is an example of how the shell should workassuming that $$$ is the prompt:

    $$$ ./collatz 410000

In  this  example,the  shell must  launcha local program  called  collatz  4  times.  It  provides  each  collatz  process the command line arguments that includethe following:

    collatz 4 i 10000

Here,iis the index of the instancethat the shell forks. It should range from 0 to 3 for the corresponding instance of collatz.Each  instance  of  collatz  may  use  i  and  4  to  decide  what  range  of  numbers among  the  specified  10,000 it must process.

Your  shell  must handle  the  following  error  cases.  Users  may  enter  an  incorrect  program  name  referringto  a program  that does  notexist on  your  file  system or  enter  a  character  string as  the  counter  value thatis  not  an integer, or enter nothing. Your job should be to alert the user if the program you attempt to execute does not exist or if the second argument is  not an integer. Take advantage of the appropriate system calls mentioned below to determine if a program exists or not. Furthermore, your shell must handle empty inputs gracefully and not crash.
