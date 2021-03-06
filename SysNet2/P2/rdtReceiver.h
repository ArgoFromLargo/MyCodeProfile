/**
 * This file describes the functions to be implemented by an RDT receiver.
 *
 * @author Thomas Reichherzer
 * @date 2/22/2017
 * @info Systems and Networks II
 * @info Project 2
 *
 */

#ifndef _RDT_RECEIVER_H
#define _RDT_RECEIVER_H

/**
 * Receives a message from an RDT sender on a specified port.
 *
 * @param port - the number of the port on which the receiver listens to receive messages
 *
 * @return the complete text message received by a sender or NULL if an error occurred
 */
char* receiveMessage(int port);

// 2 arguments expected. Notify user if incorrect.
void confirmArgs(int argc);

#endif