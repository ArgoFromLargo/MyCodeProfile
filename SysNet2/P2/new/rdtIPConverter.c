/**
 * File:   rdtIPConverter.c
 *
 * Implements rdtIPConverter.h. See rdtIPConverter.h for details.
 *
 * @author Luke Kledzik
 * @author Adam Mooers
 * @date 4/6/2017
 * @info Course COP4635
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "rdtIPConverter.h"

typedef struct delayPktInfo delayPktInfo;

struct delayPktInfo
{
	rdpPacket *cpPacket;
	rdpSocket *cpSocket;
	struct timeval timeoutSys;
};

int initRDPSocket(int port, rdpSocket *rSocket)
{
	rSocket->state = 0; // Reset the protocol state
	rSocket->netSock = socket(AF_INET, SOCK_DGRAM, 0);

	// Was the socket created successfully?
	if(rSocket->netSock < 0)
	{
		return 0; // Socket creation failed
	}

	// Clear and configure the the socket
	memset((char *)&rSocket->addr, 0, sizeof(rSocket->addr));
	rSocket->addr.sin_family = AF_INET;
	rSocket->addr.sin_addr.s_addr = htonl(INADDR_ANY);
	rSocket->addr.sin_port = htons(port);

	// Bind the socket.
	if(bind(rSocket->netSock, (struct sockaddr *)&rSocket->addr, sizeof(rSocket->addr)) < 0)
	{
		return 0; // Binding to local socket failed
	}

	printf("Port successfully bound!\n");

	return 1; // No errors occurred binding the socket
}

ssize_t sendPacket(const rdpPacket *pkt, rdpSocket *rSocket)
{
	// Attempt to send the packet
	int bytesSent = sendto(	rSocket->netSock,
				pkt,
				sizeof(rdpPacket),
				0,
				(struct sockaddr *)&rSocket->nextDest,
				sizeof(rSocket->nextDest));

	return bytesSent;
}

void *pktDelayThread(void *pktInfo)
{
   delayPktInfo *forwardTo = (delayPktInfo *) pktInfo;

   fd_set readset;

   // Delay x seconds
   // Initialize the set
   FD_ZERO(&readset);

   select(0, NULL, NULL, NULL, &(forwardTo->timeoutSys));

   // Send over socket
   sendPacket(forwardTo->cpPacket, forwardTo->cpSocket);

   free(forwardTo->cpSocket);
   free(forwardTo->cpPacket);
   free(forwardTo);

   pthread_exit(NULL);
}

int sendPacketDelayed(const rdpPacket *pkt, const int delay, rdpSocket *rSocket)
{
	delayPktInfo *pktInfo;

	// Duplicate packet info for new thread
	pktInfo = (delayPktInfo *)malloc(sizeof(delayPktInfo));
	
	// Make copies of the destination structs
	pktInfo->cpPacket = (rdpPacket *)malloc(sizeof(rdpPacket));
	pktInfo->cpSocket = (rdpSocket *)malloc(sizeof(rdpSocket));
	memcpy(pktInfo->cpPacket, pkt, sizeof(rdpPacket));
	memcpy(pktInfo->cpSocket, rSocket, sizeof(rdpSocket));

	// Calculate and set the select timeout
    pktInfo->timeoutSys.tv_sec = delay/1000; 			// SECONDS
    pktInfo->timeoutSys.tv_usec = (delay%1000)*1000; 	// +MICROSECONDS

	pthread_t delayThread;
	
	return pthread_create(&delayThread, NULL, pktDelayThread, (void *)pktInfo);
}

ssize_t sendFragment(char **msgPtr, const char ack, rdpSocket *rSocket)
{
	rdpPacket pkt;

	//Sanitize the outgoing packet
	memset((char *)&pkt, 0, sizeof(pkt));

	pkt.ack = ack;		// Set the ack flag of the outgoing packet
	pkt.corrupt = 0; 	// Set the corrupt flag of the outgoing packet

	// Set the source ip address
	strncpy((char *)&pkt.srcIP, inet_ntoa(rSocket->addr.sin_addr), sizeof(pkt.destIP));

	// Set the source port
	sprintf((char *)&pkt.srcPort, "%d", ntohs(rSocket->addr.sin_port));

	// Set the destination ip address
	strncpy((char *)&pkt.destIP, inet_ntoa(rSocket->finalDest.sin_addr), sizeof(pkt.destIP));

	// Set the destination port
	sprintf((char *)&pkt.destPort, "%d", ntohs(rSocket->finalDest.sin_port));

	int curLen = 0;

	// Until the end of the message reached or the end of fragment reached
	while (**msgPtr != '\0' && curLen < sizeof(pkt.msgFrag))
	{
		pkt.msgFrag[curLen] = **msgPtr; // Copy current byte over
		*msgPtr = *msgPtr+1;
		curLen++;
	}

	return sendPacket(&pkt, rSocket); // Attempt to send the packet, and return any errors
}

struct sockaddr_in *configurePacketNode(const char *nodeHostName, int nodePort, struct sockaddr_in *node)
{
	struct hostent *hostptr;

	// Set node address attributes
	memset((char*)node, 0, sizeof(node));
    	node->sin_family = AF_INET;
	node->sin_port = htons(nodePort);

	// Run a DNS lookup
	hostptr = gethostbyname(nodeHostName);

	if(hostptr == NULL) {
		return NULL; // Failed to resolve hostname
    	}

	// Place the node address into the address structure
	memcpy((void *)&node->sin_addr, hostptr->h_addr_list[0], hostptr->h_length);

	return node;
}

int getNextRDPPacket(rdpPacket *pktBuffer, rdpSocket *rSocket, int timeout)
{
	int selectResult;
	int bytesReceived;

	struct timeval timeoutSys;
	fd_set readset;

	socklen_t networkLen = sizeof(rSocket->addr);

	if (timeout >= 0)
	{
		// Initialize the set
		FD_ZERO(&readset);
		FD_SET(rSocket->netSock, &readset);

		// Calculate and set the select timeout
		timeoutSys.tv_sec = timeout/1000; 		// SECONDS
		timeoutSys.tv_usec = (timeout%1000)*1000; 	// +MICROSECONDS

		selectResult = select(rSocket->netSock+1, &readset, NULL, NULL, &timeoutSys);

		if (selectResult == 0) // No changes to the file descriptors implies a timeout
		{
			return -2;
		}
		else if (selectResult == -1) // -1 implies an error in the code
		{
			return -1;
		}
	}

	// No timeout or select errors, so the read can occur.
	bytesReceived = recvfrom(
					rSocket->netSock,
					pktBuffer,
					sizeof(rdpPacket),
					0,
					(struct sockaddr *)&rSocket->rcvdFrom,
					&networkLen);

	return bytesReceived;
}

void printPacketDebug(const rdpPacket *pkt)
{
	printf("\nSrc Host: %s %s\n", pkt->srcIP, pkt->srcPort);
	printf("Dest Host: %s %s\n", pkt->destIP, pkt->destPort);
	printf("Ack: %d | corrupt: %d\n", pkt->ack, pkt->corrupt);
	printf("Message Fragment:%.*s\n\n", (int)sizeof(pkt->msgFrag), pkt->msgFrag);
}

int isInt(const char* str)
{
	// Account for sign
	if (*str == '+' || *str == '-') str++;

	// Check each digit
	do {
		if (!isdigit(*str)) return 0;
	} while (*(++str) != '\0');

	return 1;  // All characters met the criteria
}

int confirmIntString(const char* pStr, unsigned int lowerBound, unsigned int upperBound, unsigned int maxChars)
{
	// Can the port string be converted to a valid port number?
	int canConvert = isInt(pStr) && strlen(pStr) <= maxChars;

	// The converted port
	int converted = -1;

	if (canConvert)
	{
		converted = atoi(pStr);
	}

	if (!canConvert || converted < lowerBound || converted > upperBound)
	{
		fprintf(stderr, "The argument %s should be an integer in the range [%i, %i].\n", pStr, lowerBound, upperBound);
		converted = -1;
	}

	return converted;
}
