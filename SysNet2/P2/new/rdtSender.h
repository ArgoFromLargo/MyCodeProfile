/**
 * This file describes the function(s) to be implemented by an RDT sender.
 *
 * A 4-state RDP 3.0 transmitter protocol is implemented on top of the
 * RDP socket described in rdpIPConverter.h. The algorithm is described
 * in depth in the included protocol document (protocol.docx).
 * 
 * The sender is started from the terminal with the following arguments:
 * Command Line Arguments:
 * 	sender <port> <rcvHost> <rcvPort> <networkHost> <networkPort>
 * 		port: the local port of the sender
 *		rcvHost: the hostname of the receiver
 *		rcvPort: the port of the receiver
 *		networkHost: the hostname of the network
 * 		networkPort: the port of the network
 *
 * @author Adam Mooers
 * @author Luke Kledzik
 * @author Thomas Reichherzer
 * @date 4/6/2017
 * @info Systems and Networks II
 * @info Project 2
 */

#ifndef _RDT_SENDER_H
#define _RDT_SENDER_H

#define MAX_FINAL_TIMEOUTS 3

// States of the sender finite state machine
enum senderStates {
	SEND_0 = 0,
	ACK_0 = 1,
	SEND_1 = 2,
	ACK_1 = 3
};

/**
 * Sends a message to an RDT receiver on a specified host and port using a network simulator
 * program for transporting messages to the receiver.
 *
 * @param localPort - the local port to bind the socket
 * @param netwHost  - the name of the host that runs the network simulator
 * @param netwPort  - the number of the port on which the network simulator listens
 * @param destHost  - the name of the host that runs the receiver
 * @param destPort  - the number of the port on which the receiver listens
 * @param message   - the entire text message to be sent to the receiver; the message is \0 terminated
 *
 * @return 0, if no error; otherwise, a negative number indicating the error
 */
int sendMessage(int localPort, char* netwhost, int netwPort, char* desthost, int destPort, char* message);

/**
 * Confirms the arguments that the user entered for the sender.
 * The following checks are run in the following order:
 *
 *   - Confirms that the number of arguments is correct (expecting 5)
 *   - Confirms port is an int and in the range [0, MAX_PORT]
 *   - Confirms rcvPort is an int and in the range [0, MAX_PORT]
 *   - Confirms networkPort is an int and in the range [0, MAX_PORT]
 *
 * If any the checks fail, an error message will be printed to std_err and the program will terminate.
 * If the program returns, ownPort, rcvPort, and netPort will all contain the correct port values.
 *
 * @param argc the argument count passed from the system to main
 * @param argv the argument vector passed from the system to main
 * @param ownPort a pointer to an integer to store the converted ownPort in
 * @param destPort a pointer to an integer to store the converted destPort in
 * @param netPort a pointer to an integer to store the converted netPort in
 */
void confirmArgs(int argc, char **argv, int *ownPort, int *destPort, int *netPort);

#endif
