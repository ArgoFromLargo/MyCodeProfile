/**
 * File: rdtReceiver.c
 *
 * Implements rdtReceiver.h. See rdtReceiver.h for details.
 *
 * @author Luke Kledzik
 * @author Adam Mooers
 * @date 4/10/2017
 * @info Course COP4635
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rdtIPConverter.h"
#include "rdtReceiver.h"

int main(int argc, char **argv)
{
	char* rcvdMsg;

	// User-defined ports given as command-line arguments
	int ownPort;

	confirmArgs(argc, argv, &ownPort);

	rcvdMsg = receiveMessage(ownPort);

    if (rcvdMsg != NULL)
    {
        printf("Final Message: %s\n", rcvdMsg);
    }

	free(rcvdMsg);

	return 0;
}

char* receiveMessage(int port)
{
	char* message = NULL;
	rdpSocket rSocket;
	rdpPacket pktBuffer;

	// Convert the port to a string
	char portString[sizeof(pktBuffer.destPort)];
	sprintf((char *)portString, "%d", port);

	int byteCount;	// Status of send/receive

	// Configure and bind the new RDP socket
	if (initRDPSocket(port, &rSocket) == 0)
	{
		fprintf(stderr, "Unable to bind socket to port %d\n", port);
		return NULL;
	}

	message = malloc(MAX_MSG_SIZE);

	int msgError = 0;			// Assume no errors to begin with
	int messageComplete = 0;	// Was the terminating packet received?

	while (!messageComplete)
	{
		// Get a message fragment from the user, wait indefinitely
		byteCount = getNextRDPPacket(&pktBuffer, &rSocket, -1);

		// Check for socket errors
		if (byteCount == -1)
		{
			fprintf(stderr, "Error reading socket...\n");
			msgError = 1;
			break;
		}

		// Update the network node
		memcpy(&rSocket.nextDest, &rSocket.rcvdFrom, sizeof(rSocket.nextDest));

		// Update the destination node
		memcpy(&pktBuffer.destIP, &pktBuffer.srcIP, sizeof(pktBuffer.destIP));
		memcpy(&pktBuffer.destPort, &pktBuffer.srcPort, sizeof(pktBuffer.destPort));

		// Update the sender info
		memcpy(&pktBuffer.srcPort, portString, sizeof(pktBuffer.destPort));

		switch(rSocket.state)
		{
			case ACK_0:
			case ACK_1:
                if (!pktBuffer.corrupt)
                {
                    if (pktBuffer.ack == rSocket.state) // Is this the expected ack?
    				{
    					// If not the last packet and if the buffer still has space available
    					if (pktBuffer.msgFrag[0] != '\0' && strlen(message) + sizeof(pktBuffer.msgFrag) < MAX_MSG_SIZE)
    					{
    						strncat(message, (char *)&pktBuffer.msgFrag, sizeof(pktBuffer.msgFrag));
                            printf("Message: %s\n", message);
    					}
    					else
    					{
    						messageComplete = 1; // Stop receiving message
    					}

    					rSocket.state = (rSocket.state+1)%(ACK_1+1); // Go to the next state
    				}
    				else // Drop the packet
    				{
    					printf("Packet dropped: Received: %d \tExpecting: %d\n", pktBuffer.ack, rSocket.state);
    				}

                    // Send an ack in reply
                    sendPacket(&pktBuffer, &rSocket);
                }
                else
                {
                    printf("Dropped corrupt packet.\n");
                }
		}
	}

	// Close the socket
	close(rSocket.netSock);
	
	if (msgError)
	{
		free(message);
	}

	return message;	// Return null if an error occurred
}

void confirmArgs(int argc, char **argv, int* ownPort)
{
	int printUsage = 0;

	if (argc == 2)
	{
		// Convert the ports to integers if they are valid.
		*ownPort = confirmIntString(argv[1], 0, MAX_PORT, MAX_PORT_CHARS);

		printUsage = (*ownPort == -1);
	}
	else
	{
		printf("Incorrect number of arguments.\n");
		printUsage = 1;
	}

	if (printUsage)
	{
		printf("Usage: ./receiver <port>\n");
		exit(1);
	}
}
