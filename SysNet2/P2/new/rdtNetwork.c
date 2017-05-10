/**
 * File: rdtNetwork.c
 *
 * the rdtNetwork manages the connection between the sender and the reciever.
 * Its goal is to create a simulation of real-world network conditions in order
 * to test how the sender and receiver state machines behave under non-ideal
 * network conditions. The network is started from the terminal with the
 * following arguments:
 *
 * network port lostPercent delayedPercent errorPercent
 * 	port: the local port of the network
 * 	lostPercent: the percentage of lost packets
 *	delayedPercent: percent of delayed packets
 *	errorPercent: percentage of corrupted packets
 *
 * The final three arguments are nonnegative integers less than 100.
 *
 * @author Luke Kledzik
 * @author Adam Mooers
 * @date 4/6/2017
 * @info Course COP4635
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_PERCENT 100
#define MAX_PERCENT_CHARS 3
#define STATS_INTERVAL 3
#define TIMEOUT_INTERVAL 3000

#include "rdtIPConverter.h"

/**
 * Confirms the arguments that the user entered for the sender.
 * The following checks are run in the following order:
 *
 *   - Confirms that the number of arguments is correct (expecting 4)
 *   - Confirms that the port is valid
 *   - Confirms that the lostPercent is a positive integer <= 100
 *   - Confirms that the delayedPercent is a positive integer <= 100
 *   - Confirms that the errorPercent is a positive integer <= 100
 *
 * If any the checks fail, an error message will be printed to std_err and the program will terminate.
 * If the program returns, lostPercent, delayedPercent, and errorPercent will all contain the correct port values.
 *
 * @param argc the argument count passed from the system to main
 * @param argv the argument vector passed from the system to main
 * @param ownPort a pointer to an integer to store the converted ownPort in
 * @param lostPercent a pointer to an integer to store the converted ownPort in
 * @param delayedPercent a pointer to an integer to store the converted destPort in
 * @param errorPercent a pointer to an integer to store the converted netPort in
 */
void confirmArgs(int argc, char **argv, int* ownPort, int *lostPercent, int *delayedPercent, int *errorPercent);

/**
 * Randomly decides an event will occur based on the percent probability of the event.
 * For example:
 * Input: 25 --> Output = 1, 25% chance  0, 75% chance
 * Input: 50 --> Output = 1, 50% chance  0, 50% chance
 *
 * @param trueChance the percent chance that the output will be true Range: [0, 100]
 * @return whether or not the output is true
 */
int randomChance (int trueChance);

/**
 * Recieves and processes incoming packets potentially adding dropping them, corrupting them,
 * or adding delay. This function additionally keeps track of the sender and receiver stats
 * and prints them to stdout every STATS_INTERVAL packet. The loop is infinite: only serious
 * network errors will cause it to return.
 *
 * @param rSocket the RDP socket over which to send and recieve data
 * @param lostPercent the percentage of lost packets
 * @param delayedPercent the percentage of delayed packets
 * @param errorPercent the percentage of corrupted packets
 * @return -1 if an error occurred, 0 otherwise
 */
int processLoop(rdpSocket *rSocket, int lostPercent, int delayedPercent, int errorPercent);

/**
 * Updates the sender and receiver stats. The first time the function is called, the port
 * and ip address of the packet are captured and stored. This IP+packet combination is the
 * "sender". Each time afterwards, the ip address and port of the incoming packet are compared
 * with the stored sender's host info. If they do not match, the host is considered the receiver.
 * If the receiver is detected, the receiverCount is incremented. Otherwise the senderCount is
 * incremented.
 *
 * @param sourcePacket the packet to compare
 * @param senderCount the count statistic of the sender
 * @param receiverCount the count statistic of the receiver
 */
void determinePacketSource(rdpPacket *sourcePacket, int *senderCount, int *receiverCount);

int main(int argc, char **argv)
{
	srand(time(NULL));	// Give the random number generator a random start seed

	// User-defined ports given as command-line arguments
	int ownPort, lostPercent, delayedPercent, errorPercent;

	confirmArgs(argc, argv, &ownPort, &lostPercent, &delayedPercent, &errorPercent);

	rdpSocket rSocket;

	// Configure and bind the new RDP socket
	if (initRDPSocket(ownPort, &rSocket) == 0)
	{
		fprintf(stderr, "Unable to bind socket to port %d\n", ownPort);
		close(rSocket.netSock);
		return -1;
	}

	processLoop(&rSocket, lostPercent, delayedPercent, errorPercent);
	
	close(rSocket.netSock);

	return 0;
}

void confirmArgs(int argc, char **argv, int* ownPort, int *lostPercent, int *delayedPercent, int *errorPercent)
{
	int printUsage = 0;

	if (argc == 5)
	{
		// Convert the ports to integers if they are valid.
		*ownPort = confirmIntString(argv[1], 0, MAX_PORT, MAX_PORT_CHARS);
		*lostPercent = confirmIntString(argv[2], 0, MAX_PERCENT, MAX_PERCENT_CHARS);
		*delayedPercent = confirmIntString(argv[3], 0, MAX_PERCENT, MAX_PERCENT_CHARS);
		*errorPercent = confirmIntString(argv[4], 0, MAX_PERCENT, MAX_PERCENT_CHARS);

		printUsage = (*ownPort == -1) || (*lostPercent == -1) || (*delayedPercent == -1) || (*errorPercent == -1);
	}
	else
	{
		printf("Incorrect number of arguments.\n");
		printUsage = 1;
	}

	if (printUsage)
	{
		printf("Usage: ./network <port> <lostPercent> <delayedPercent> <errorPercent>\n");
		exit(1);
	}
}

int randomChance(int trueChance)
{
	return (random()%MAX_PERCENT < trueChance);
}

int processLoop(rdpSocket *rSocket, int lostPercent, int delayedPercent, int errorPercent)
{
	rdpPacket pktBuffer;
	int byteCount; 	// Status of received or transmitted data

	int senderCount = 0;
	int receiverCount = 0;
	int numPacketsCorrupted = 0;
	int numPacketsDelayed = 0;
	int numPacketsDropped = 0;

	for (;;) // Wait indefinitely for packet
	{
		printf("Sender Count: %d\tReceiver Count: %d\t\n", senderCount, receiverCount);
		printf("Corrupted: %d\tDelayed: %d\tDropped: %d\n\n", numPacketsCorrupted, numPacketsDelayed, numPacketsDropped);

		byteCount = getNextRDPPacket(&pktBuffer, rSocket, -1);

		if (byteCount == -1) // An error occurred: no need to consider timeouts
		{
			fprintf(stderr, "Error reading from the socket.\n");
			return -1;
		}

		determinePacketSource(&pktBuffer, &senderCount, &receiverCount);
		
		pktBuffer.corrupt = randomChance(errorPercent);	// Randomly corrupt the packet
		numPacketsCorrupted += pktBuffer.corrupt;

		// Update the sender source ip address (s)
		strncpy((char *)&pktBuffer.srcIP, inet_ntoa(rSocket->rcvdFrom.sin_addr), sizeof(pktBuffer.destIP));

		// If the packet is not lost in the network
		if (!randomChance(lostPercent))
		{

			// Update the nextNode with packet's destination
			if (configurePacketNode(pktBuffer.destIP, atoi(pktBuffer.destPort), &rSocket->nextDest) == NULL)
			{
				fprintf(stderr, "Unable to forward the packet destined to %s\n", pktBuffer.destIP);
			}
			else
			{
				if (randomChance(delayedPercent)) // Randomly delay packet
				{
					sendPacketDelayed(&pktBuffer, TIMEOUT_INTERVAL, rSocket);
					numPacketsDelayed++;
				}
				else
				{
					sendPacket(&pktBuffer, rSocket);
				}
			}
		}
		else
		{
			numPacketsDropped++;
		}
	}
	return 0;
}

void determinePacketSource(rdpPacket *sourcePacket, int *senderCount, int *receiverCount)
{
	static char senderIP[sizeof(sourcePacket->srcIP)] = "";
	static char senderPort[sizeof(sourcePacket->srcPort)] = "";

	// If this is the first time running
	if (senderIP[0] == '\0')
	{
		strcpy(senderIP, sourcePacket->srcIP);
		strcpy(senderPort, sourcePacket->srcPort);
	}


	if (strcmp(senderIP, sourcePacket->srcIP) == 0)
	{
		(*senderCount)++;
	}
	else
	{
		(*receiverCount)++;
	}
}
