/**
 * This file describes the functions to be implemented by an RDT receiver.
 * A 2-state RDP 3.0 receiver protocol is implemented on top of the
 * RDP socket described in rdpIPConverter.h. The algorithm is described
 * in depth in the included protocol document (protocol.docx).
 * 
 * The receiver is started from the terminal with the following arguments:
 * ./receiver <port>
 * 	port: the local port of the network
 *
 * The local port of the network and sender are inferred from the received RDP packets
 * each time a new packet is received.
 *
 * @author Thomas Reichherzer
 * @date 2/22/2017
 * @info Systems and Networks II
 * @info Project 2
 *
 */

#ifndef _RDT_RECEIVER_H
#define _RDT_RECEIVER_H

// States of the receiver finite state machine
enum receiverStates {
	ACK_0 = 0,
	ACK_1 = 1,
};

/**
 * Receives a message from an RDT sender on a specified port.
 *
 * @param port - the number of the port on which the receiver listens to receive messages
 *
 * @return the complete text message received by a sender or NULL if an error occurred
 */
char* receiveMessage(int port);

/**
 * Confirms the arguments that the user entered for the sender.
 * The following checks are run in the following order:
 *
 *   - Confirms that the number of arguments is correct (expecting 1)
 *   - Confirms port is an int and in the range [0, MAX_PORT]
 *
 * If any the checks fail, an error message will be printed to std_err and the program will terminate.
 *
 * @param argc the argument count passed from the system to main
 * @param argv the argument vector passed from the system to main
 * @param ownPort a pointer to an integer to store the converted ownPort in
 */
void confirmArgs(int argc, char **argv, int* ownPort);

#endif
