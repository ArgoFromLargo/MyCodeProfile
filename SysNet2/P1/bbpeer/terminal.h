/** 
 * File:   terminal.h
 *
 * terminal.h handles user input such as commands writing messages, and preparing 
 * information for the server thread. User commands are tokenized and error checked.
 * The following commands are supported:
 *
 * ACTION:				DESCRIPTION:
 * write				Appends a new message to the end of the message board.
 * read <message id>	Reads the message <message id> from the message board to stdout
 * list 				Displays the range of valid sequence
 * exit					Close the message board
 *
 * To initiate each of the above actions, the user must have the token to lock the message 
 * log. Because the user operates at a different rate than the token ring messages, the terminal
 * launches in its own independent thread. The proper command handler is stored as a function
 * for the token thread to handle in its own time.
 *
 * @author Luke Kledzik
 * @author Adam Mooers
 * @date 2/18/2017
 * @info Course COP4635
 */
 
 #ifndef TERMINAL_H
 #define TERMINAL_H
 
 #include <pthread.h>
 
 // Mutex lock used whenever the terminal is updating information for the 
 // token ring. A global lock is used for simplicity.
 pthread_mutex_t terminalLock;
 
 // Code to end a multi-line message when placed on a new line and followed
 // immediately by another new line.
 #define MSG_STOP_CODE ".."
 
/**
 * Runs the main terminal thread, prompting for user commands. User commands
 * are parsed and error-checked. Permissable user commands are documented in
 * the header of the h file as well as in the project description. After a user
 * command is entered, the terminal thread waits until the network thread unlocks
 * the tokenLock. It then locks tokenLock, updates the userCmd and the message log,
 * (if needed) and then possibly repeats.
 * 
 * @param n Not used. This is needed for the thread pattern
 * @return Returns the thread exit status to joined threads via pthread_exit
 */
 
 void* terminalLoop(void* n);
 
 /**
  * Tokenizes the user input and determines whether the input matches any commands
  * described in the header of this document. If not, an error message describing 
  * the problem is printed to stderr. Possible errors can include the wrong number
  * of arguments, an unknown command, or an argument of the wrong type.
  * 
  * @param inputStr the input string from the user
  */
  
 void parseCmdString(char* inputStr);
 
 /**
  * Handles the user input (stdin) while the user is typing a multi-line message.
  * To exit input mode, the user should type MSG_STOP_CODE on a new line. The 
  * MSG_STOP_CODE will not be inlcuded in the message. If the user types MSG_STOP_CODE
  * on the first line the message will be an empty string. After the message is 
  * typed completely, it will be copied into the msgBuffer while there is a lock
  * on tokenLock.
  */
  
 void handleMessageInput();
  
 /**
  * Starts the terminal thread, saving a reference to it as a singleton. Only one terminal
  * thread can be created at a time because there is only one terminal. Any attempt to make
  * a two or more concurrent terminal thread will result in undocumented behaviour.
  * The mutex lock for controlling access to the message buffer is initialized here.
  */
 
 void runTerminalThread();
 
 /**
  * Handles the network thread side of terminal command processing. This function is
  * run by the terminal thread while the network thread has the token (and therefore 
  * a lock on the message log). First it checks userCmd to see if there are any new 
  * commands to process. If so, they are handled immediately.runTerminalThread must
  * be called first to ensure the buffers and locks are properly initialized.
  */
  
 void handleCommandsWithToken();
 
 /**
  * Determines if a given string can be converted to a valid integer.
  * The integer may start with a plus or minus sign. The length of the
  * integer is not accounted for.
  * 
  * @param str the null-terminated string to test
  * @return 0 if str is not a string, !0 otherwise
  */
 int isInt(const char* str);
 
 /**
  * @return returns whether the terminal is exiting.
  */
 int exitPending();

#endif