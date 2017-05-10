/**
 * File:   terminal.c
 *
 * Implements terminal.h. See terminal.h for details.
 *
 * @author Luke Kledzik
 * @author Adam Mooers
 * @date 2/18/2017
 * @info Course COP4635
 */

 #include <stdio.h>
 #include <ctype.h>
 #include <string.h>
 #include <stdlib.h>
 #include "msgparser.h"
 #include "terminal.h"

 // Delimiters for tokenizing user commands
 #define CMD_LINE_DELIMITERS " \t\n"

 // The error code to print to stdout when the wrong number of argument
 // are supplied by the user. The proper usage should follow.
 #define ERR_WRONG_NUM_ARGUMENTS "Error:\tWrong number of arguments. \n\tUsage: "

 // The maximum number of arguments to tokenize from the terminal.
 // This value is one greater than the maximum number of expected input arguments
 #define MAX_INPUT_ARGUMENTS 3
 
  // Codes for user commands
 enum commandCode
 {
	NONE,
	WRITE,
	READ,
	LIST,
	EXIT
 };
 
 // The flag for the command entered by the user. See commandCodes for details.
 // userCmd is locked by the terminal lock before it is used.
 enum commandCode userCmd = NONE;

 // Message buffer for the user input. The message (including the null-terminating
 // character) must be shorter than msgBuffer. The buffer is protected by tokenLock
 // while it is being updated by the terminal.
 char msgTempBuffer[5000];

 // Local copy of the current user command. This command is copied to userCmd when
 // the terminal thread has a lock on terminalLock.
 enum commandCode tempUserCmd = NONE;

 // The message id to read when the command code is READ
 int msgIdToRead;

 // Reference to the terminal thread singleton
 pthread_t terminalThread;

 /*int main()
 {
	runTerminalThread();

	// No other thread has access to this lock at this point
	pthread_mutex_destroy(&terminalLock);

	return(0);
}*/

/**
 * Runs the main terminal thread, prompting for user commands. User commands
 * are parsed and error-checked. Permissable user commands are documented in
 * the header of the h file as well as in the project description.
 *
 * @param n Not used. This is needed for the thread pattern
 * @return Returns the thread exit status to joined threads via pthread_exit
 */

 void* terminalLoop(void* n)
 {
	 while (userCmd != EXIT)
	 {
		printf("> ");

		// Get a line of user input
		fgets(msgTempBuffer, sizeof(msgTempBuffer), stdin);

		// Parse the command from the user
		parseCmdString(msgTempBuffer);

		printf("Entering Lock in Terminal thread\n");
		
		// Wait for the token to come to the network thread
		pthread_mutex_lock(&terminalLock);
		
		printf("Lock Acquired in Terminal thread\n");

		// Update the userCmd
		userCmd = tempUserCmd;

		// Process the user command
		handleCommandsWithToken();

		// Return the lock to the server
		pthread_mutex_unlock(&terminalLock);
	 }
	printf("Exiting terminal thread.\n");
	 pthread_exit(0);
 }

 void parseCmdString(char* inputStr)
 {
	 // The maximum number of argument to scan for
	 char* tokenArr[MAX_INPUT_ARGUMENTS];

	 // Assume the input is faulty by default
	 tempUserCmd = NONE;

	 int i;
	 int numInputArgs;

	 // Tokenize the user command
	 for (i=0; i<MAX_INPUT_ARGUMENTS; i++)
	 {
		 if (i==0)
		 {
			// Get the first token
			tokenArr[i] = strtok(inputStr, CMD_LINE_DELIMITERS);
		 }
		 else
		 {
			// Get the nth token
			tokenArr[i] = strtok(NULL, CMD_LINE_DELIMITERS);
		 }

		 // If the end of the user input is reached
		 if (tokenArr[i] == NULL)
		 {
			 break;
		 }
	 }

	 numInputArgs = i;

	 if (numInputArgs == 0)
	 {
		 return;
	 }
	 else if (strcmp(tokenArr[0], "write") == 0)
	 {
		 if (numInputArgs == 1)
		 {
			 handleMessageInput();
			 tempUserCmd = WRITE;
		 }
		 else
		 {
			 printf(ERR_WRONG_NUM_ARGUMENTS"write\n");
		 }
	 }
	 else if (strcmp(tokenArr[0], "read") == 0)
	 {
		 if (numInputArgs == 2)
		 {
			 // Check if the argument is valid
			 if (isInt(tokenArr[1]))
			 {
				tempUserCmd = READ;
				msgIdToRead = atoi(tokenArr[1]);
			 }
			 else
			 {
				printf("The message id must be an integer.\n");
			 }
		 }
		 else
		 {
			 printf(ERR_WRONG_NUM_ARGUMENTS"read <message id>\n");
		 }
	 }
	 else if (strcmp(tokenArr[0], "list") == 0)
	 {
		 if (numInputArgs == 1)
		 {
			 tempUserCmd = LIST;
		 }
		 else
		 {
			 printf(ERR_WRONG_NUM_ARGUMENTS"list\n");
		 }
	 }
	 else if (strcmp(tokenArr[0], "exit") == 0)
	 {
		 if (numInputArgs == 1)
		 {
			tempUserCmd = EXIT;
		 }
		 else
		 {
			 printf(ERR_WRONG_NUM_ARGUMENTS"exit\n");
		 }
	 }
	 else
	 {
		 printf("%s: command not found\n", tokenArr[0]);
	 }

	 return;
 }

 /**
  * Handles the user input (stdin) while the user is typing a multi-line message.
  * To exit input mode, the user should type MSG_STOP_CODE on a new line. The
  * MSG_STOP_CODE will not be inlcuded in the message. If the user types MSG_STOP_CODE
  * on the first line the message will be an empty string. After the message is
  * typed completely, it will be copied into the msgBuffer while there is a lock
  * on terminalLock.
  */

 void handleMessageInput()
 {
	 printf("\nPlease enter your message: \nType "MSG_STOP_CODE" on a new line to finish.\n\n");

	 int curMessageLen = 0;

	 // The position of the first character of the most-recent line
	 char* lastLineStart = msgTempBuffer;

	 // Clear the message buffer
	 msgTempBuffer[0] = '\0';

	 while(1)
	 {
		 // Get a line of user input (at most, fill the buffer)
		 fgets(lastLineStart, sizeof(msgTempBuffer)-strlen(msgTempBuffer), stdin);

		 // Update the current message length
		 curMessageLen = strlen(msgTempBuffer);

		 // Don't let user put closing tags into the message
		 if (strstr(lastLineStart, CLOSING_XML_TAG_FORMAT) != NULL)
		 {
			 fprintf(stdout, "\nThe closing tag "CLOSING_XML_TAG_FORMAT" cannot be contained within the message body.\n");
			 fprintf(stdout, "Please correct the line:\n\n");

			 continue;
		 }

		 // Is the latest line the stop code?
		 if (strcmp(lastLineStart, MSG_STOP_CODE"\n") == 0)
		 {
			 // Erase the stop command line
			 *lastLineStart = '\0';

			 break; // Stop accepting input
		 }

		 // Is the buffer full?
		 if (curMessageLen == sizeof(msgTempBuffer)-1)
		 {
			 printf("\nThe buffer is full. No more text will be accepted.\n");
			 break; // Stop accepting input
		 }

		 // Update lastLineStart
		 lastLineStart = msgTempBuffer+curMessageLen;
	 }
 }

 /**
  * Starts the terminal thread, saving a reference to it as a singleton. Only one terminal
  * thread can be created at a time because there is only one terminal. Any attempt to make
  * a two or more concurrent terminal thread will result in undocumented behaviour.
  * The mutex lock for controlling access to the message buffer is initialized here.
  */

 void runTerminalThread()
 {
	// Set up the terminal thread
	pthread_mutex_init(&terminalLock, NULL);

	// Create a new terminal thread
	//(void *)&argN is the pattern for inputs
	pthread_create(&terminalThread, NULL, terminalLoop, NULL);

	//pthread_join(terminalThread, NULL);
 }

 /**
  * Handles the network thread side of terminal command processing. This function is
  * run by the terminal thread while the network thread has the token (and therefore
  * a lock on the message log). First it checks userCmd to see if there are any new
  * commands to process. If so, they are handled immediately.runTerminalThread must
  * be called first to ensure the buffers and locks are properly initialized.
  */

 void handleCommandsWithToken()
 {
	 int msgCount;

	 switch (userCmd)
	 {
		 case WRITE:
			appendMsg(msgTempBuffer);
			break;
		 case READ:
			readMessageByID(msgIdToRead, stdout);
			break;
		 case LIST:
			msgCount = getMessageCount();
			if (msgCount != FIRST_MESSAGE_INDEX-1)
			{
				printf("Valid Message Range: %d - %d\n", FIRST_MESSAGE_INDEX, msgCount);
			}
			break;
		 default:
			break; // Nothing to do if no valid command.
	 }
 }

 int isInt(const char* str) {
    // Account for sign
    if (*str == '+' || *str == '-') str++;

    // Check each digit
    do {
        if (!isdigit(*str)) return 0;
    } while (*(++str) != '\0');

   return 1;  // All characters met the criteria
 }
 
 int exitPending()
 {
	 return (userCmd == EXIT);
 }
