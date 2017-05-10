/**
 * File: rdtSender.c
 *
 * @author Luke Kledzik
 * @author Adam Mooers
 * @date 4/6/2017
 * @info Course COP4635
 */

#include <stdio.h>
#include <stdlib.h>

#include "rdtIPConverter.h"
#include "rdtSender.h"

int main(int argc, char **argv)
{
	char typedMsg[MAX_MSG_SIZE] = {0};

	// User-defined ports given as command-line arguments
	int ownPort, destPort, netPort;

	confirmArgs(argc, argv, &ownPort, &destPort, &netPort);

	printf("Enter message: ");
	fgets(typedMsg, MAX_MSG_SIZE, stdin);

	sendMessage(ownPort, argv[4], netPort, argv[2], destPort, typedMsg);

	return 0;
}

void confirmArgs(int argc, char **argv, int *ownPort, int *destPort, int *netPort)
{
	int printUsage = 0;

	if (argc == 6)
	{
		// Convert the ports to integers if they are valid.
		*ownPort = confirmIntString(argv[1], 0, MAX_PORT, MAX_PORT_CHARS);
		*destPort = confirmIntString(argv[3], 0, MAX_PORT, MAX_PORT_CHARS);
		*netPort = confirmIntString(argv[5], 0, MAX_PORT, MAX_PORT_CHARS);

		printUsage = (*ownPort == -1) || (*destPort == -1) || (*netPort == -1);
	}
	else
	{
		printf("Incorrect number of arguments.\n");
		printUsage = 1;
	}

	if (printUsage)
	{
		printf("Usage: ./sender <port> <rcvHost> <rcvPort> <networkHost> <networkPort>\n");
		exit(1);
	}
}

int sendMessage(int localPort, char* netwhost, int netwPort, char* desthost, int destPort, char* message)
{
	rdpSocket rSocket;
	rdpPacket pktBuffer;

	int bytesRcvd, bytesSent; // Status of inputs and outputs

	// Configure and bind the new RDP socket
	if (initRDPSocket(localPort, &rSocket) == 0)
	{
		fprintf(stderr, "Unable to bind socket to port %d\n", localPort);
		return -1;
	}

	// Set the network node
	if (configurePacketNode(netwhost, netwPort, &rSocket.nextDest) == NULL)
	{
		fprintf(stderr, "Unable to resolve the network hostname: %s\n", netwhost);
		close(rSocket.netSock);
		return -1; // Network device does not exist
	}

	// Set the destination node
	if (configurePacketNode(desthost, destPort, &rSocket.finalDest) == NULL)
	{
		fprintf(stderr, "Unable to resolve the destination hostname: %s\n", desthost);
		close(rSocket.netSock);
		return -1; // Destination device does not exist
	}

	char* msgPtr = message; 		// Start of current message fragment
	char* msgPtrAckdTo = msgPtr;	// Number of bytes recieved from the sender

	int lastPacketSent = 0;	// Is the current packet the last one sent
	int lastAckRcvd = 0; // Has the last ack been recieved?
	int numFinalTimeouts = 0; // Number of duplicate acks on the final packet

	while((!lastAckRcvd) && (numFinalTimeouts < MAX_FINAL_TIMEOUTS)) 	// Until the whole message is ackd
	{
		switch(rSocket.state)
		{
			case SEND_0:
			case SEND_1:
				// In sendmessage, there is no reason to wait from above, so send immediately
				bytesSent = sendFragment(&msgPtr, rSocket.state/SEND_1, &rSocket);

				// Send an empty message for the last packet
				if (msgPtr == msgPtrAckdTo)
				{
					lastPacketSent = 1;
					printf("Last packet has been sent.\n");
				}

				if (bytesSent == -1)
				{
					fprintf(stderr, "Error sending to network...\n");
					close(rSocket.netSock);
					return -1;
				}
				rSocket.state++;
				break;
			case ACK_0:
			case ACK_1:
				bytesRcvd = getNextRDPPacket(&pktBuffer, &rSocket, SENDER_TIMEOUT_MS);
				if (bytesRcvd == -2) // Code for a timeout
				{
					if (lastPacketSent)
					{
						printf("Timeouts: %d\n", numFinalTimeouts);
						numFinalTimeouts++;
					}

					msgPtr = msgPtrAckdTo; 	// Roll-back sent bytes
					rSocket.state--; 		// Resend on a timeout

					fprintf(stderr, "Timeout on expected ACK...\n");
				}
				else if (bytesRcvd == -1) 	// Check for a socket error
				{
					fprintf(stderr, "Error reading socket...\n");
					close(rSocket.netSock);
					return -1;
				}
				else if (pktBuffer.corrupt)
				{
					fprintf(stderr, "Corrupt ACK received and dropped.\n");
				}
				else if (pktBuffer.ack == rSocket.state/ACK_1) // Expected ack recieved
				{
					msgPtrAckdTo = msgPtr;	// The bytes have been ackd
					rSocket.state = (rSocket.state+1)%(ACK_1+1); // Roll back to zero after 3rd state

					if (lastPacketSent)
					{
						printf("Last ACK received...\n");
						lastAckRcvd = 1;
					}
				}
				else
				{
					fprintf(stderr, "Recieved ack %d while expecting ack %d.\n", pktBuffer.ack, 1-pktBuffer.ack);
				}
		}
	}

	close(rSocket.netSock);
	return 0; // Message sent successfully
}
