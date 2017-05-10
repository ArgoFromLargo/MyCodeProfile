/**
 * This program (node.c) is used for implementing the network router.
 * This program is designed to run on a single host. See node.h for
 * function definitions.
 *
 * Execution command:
 * node <routerLabel> <portNum> <totalNumRouters> <discoverFile> [-dynamic]
 *
 * routerLabel: 	a single char representing the starting node's label
 * portNum: 		the port number on which the router receives and sends data.
 * totalNumRouters: 	an integer for the total number of routers in the network.
 * discoverFile: 	the text file containing neighbor information
 * -dynamic: 		an optional parameter indicating if the router runs a dynamic
 * 			network whose cost on a link may change.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include "node.h"
#include "dijkstra.h"

#define NEIGHBOR_LINE_BUFFER_LEN 500	// Maximum length of a line in the neighbor file

int main(int argc, char* argv[]) {
	int ownPort, totalNumRouters, isDynamic;
	confirmArgs(argc, argv, &ownPort, &totalNumRouters, &isDynamic);
	
	char ownLabel = argv[1][0];
	char ownPktBuffer[LS_PACKET_BUFFER_SIZE];	// For the host's link state packet
	char rcvPktBuffer[LS_PACKET_BUFFER_SIZE];	// For a received link state packet

	int** adjMatrix = allocAdjMatrix(totalNumRouters, INF);

	int hopCount = totalNumRouters; // The longest possible path between two routers is the total router count
	
	// Node list keeps track about the status of nodes in the network
	node* nodeList = allocNodeArray(totalNumRouters, -1);
	node* hostNode = nodeList + labelToInd(ownLabel);

	// Load neighbors from the file
	int numNeighbors = parseNeighborFile(argv[4], adjMatrix, totalNumRouters, ownPktBuffer, nodeList, ownLabel);

	if (numNeighbors != -1)
	{			
		// Build the host's linkstate packet
		buildLSPacket(ownPktBuffer, nodeList, adjMatrix, totalNumRouters, hopCount, numNeighbors, ownLabel);

		// Print out the host's linkstate packet
		printLSPacketDebug(ownPktBuffer);
		
		// Configure socket
		int socket = configureHostInfo(ownPort, hostNode);
		
		// Was the socket configured correctly?
		if (socket > 0)
		{
			char rcvdFrom;
			int rcvdBytes = listenPacket(socket, rcvPktBuffer, LISTEN_TIMEOUT, &rcvdFrom, nodeList, totalNumRouters);
			
			if (rcvdBytes != -1) // If no errors reading the socket
			{
				if (rcvdBytes > 0) printLSPacketDebug(rcvPktBuffer);
				
				// Send link state packet to all neighbors
				floodNeighbors(socket, ownPktBuffer, ownLabel, nodeList, totalNumRouters);
				
				debugPrintMatrix(adjMatrix, totalNumRouters);
			}
		}
	}

	// Clean up memory
	freeAdjMatrix(adjMatrix, totalNumRouters);
	free(nodeList);

	return 0;
}

void* fillLSHeader(void* bufferLoc, 
		   unsigned int hopCount, 
		   unsigned int seqNum, 
		   unsigned int numNeighbors, 
		   char sourceLabel)
{
	lsHeader* header = (lsHeader*) bufferLoc;
	header->hopCount = hopCount;
	header->seqNum = seqNum;
	header->numNeighbors = numNeighbors;
	header->sourceLabel = sourceLabel;
	
	return bufferLoc+sizeof(lsHeader);
}

/**
 * A constructor for the lsEdge packet. The constructor is for simplifying the packet-filling
 * process. See the lsEdge struct for more information about the fields.
 *
 * @param bufferLoc the memory to fill with the struct info
 * @return a pointer to the first byte after the newly-filled struct
 */
void* fillLSEdge(void* bufferLoc, 
		 unsigned int cost, 
		 char destLabel)
{
	lsEdge* edge = (lsEdge*) bufferLoc;
	edge->cost = cost;
	edge->destLabel = destLabel;

	return bufferLoc+sizeof(lsEdge);
}

int parseNeighborFile(char* fileName,
					int** adjMatrix, 
					int totalNumRouters, 
					void* lsPktBuffer, 
					node* nodeList, 
					char ownName)
{	
	FILE* neighborFile = fopen(fileName, "r");
	
	if (neighborFile == NULL)
	{
		fprintf(stderr, "Unable to open %s for reading.\n", fileName);
		return -1;
	}
	
	char lineBuf[NEIGHBOR_LINE_BUFFER_LEN];

	int curLine = 1; // Current line in the open file
	
	// While there is still more to read
	while(fgets(lineBuf, sizeof(lineBuf), neighborFile) != NULL)
	{
		char neighborHostname[MAX_HOST_NAME_LEN+1] = {'\0'};
		int neighborWeight = INF;
		unsigned int neighborPort = -1;
		char neighborLabel;
		
		// Commas are not needed for parsing
		replaceSubStr(lineBuf, ',', ' ');

		// Parse the current line
		int argsScanned = sscanf(lineBuf, 
					" %c %"STR(MAX_HOST_NAME_LEN)"s %"STR(MAX_PORT_CHARS)"d %d \n", 
					&neighborLabel, (char *)neighborHostname, &neighborPort, &neighborWeight);

		unsigned int nodeIndex = labelToInd(neighborLabel);

		// Check if the index is in bound (can the given neighbor node exist?)
		if (nodeIndex < 0 || nodeIndex >= totalNumRouters)
		{
			fprintf(stderr, "Error at line %d of %s: Router index %c is not valid.\n", curLine, fileName, neighborLabel);
			curLine = 0;	// Reset neighbor count (as if no neighbors were imported)
			break;
		}

		node* curNode = nodeList+nodeIndex;

		// Run some more bound checks on the parsed line
		if (argsScanned != 4 || neighborPort < 0 || neighborPort > MAX_PORT)
		{
			fprintf(stderr, "Error parsing %s at line %d.\n", fileName, curLine);
			curLine = 0;	// Reset neighbor count (as if no neighbors were imported)
			break;
		}

		// Create and fill out the address structure of the neighbor
		curNode->addr = malloc(sizeof(struct sockaddr_in));
		if (configureNeighbor(neighborHostname, neighborPort, curNode->addr) == NULL)
		{
			fprintf(stderr, "Line %d of %s: Unable to resolve host info for %s\n", curLine, fileName, neighborHostname);
			curLine = 0;	// Reset neighbor count (as if no neighbors were imported)
			break;
		}

		adjMatrix[labelToInd(ownName)][nodeIndex] = neighborWeight;
		curLine++;
	}

	fclose(neighborFile);
	return curLine-1;	
}

void buildLSPacket(	void* lsPktBuffer, 
					node* nodeList, 
					int** adjMatrix, 
					int numNodes, 
					int hopCount, 
					int numNeighbors, 
					char sourceLabel)
{
	int hostIndex = labelToInd(sourceLabel);

	// Fill out the header
	node* curNode = nodeList + hostIndex; // Find the host node's info
	curNode->seqNum++; // A new packet is built, so update the sequence number
	void* fillPtr = fillLSHeader(lsPktBuffer, hopCount, curNode->seqNum, numNeighbors, sourceLabel);

	int* neighborWeights = adjMatrix[hostIndex];

	int i;
	for(i=0; i<numNodes; i++) // Determine which nodes are the neighbors
	{
		if (neighborWeights[i] != INF)
		{
			fillPtr = fillLSEdge(fillPtr, neighborWeights[i], indToLabel(i));
		}
	}
}

ssize_t sendToNeighbor(int socketNum, void* lsPktBuffer, struct sockaddr_in* destNeighbor)
{
	// Attempt to send the packet
	ssize_t bytesSent = sendto(	
				socketNum,
				lsPktBuffer,
				findLSPacketLen(lsPktBuffer),
				0,
				(struct sockaddr *)destNeighbor,
				sizeof(struct sockaddr_in));

	return bytesSent;
}

int floodNeighbors(int socketNum, void* lsPktBuffer, char hostName,  node* nodeList, int numNodes)
{
	int i;
	
	node* destNode = nodeList;
	
	for (i=0; i<numNodes; i++)
	{
		// Only send to neighbors (discard non-neighbors and sending host)
		if (destNode->addr != NULL && i != labelToInd(hostName))
		{
			printf("Sending to %c.\n", indToLabel(i));
			
			// Test for any sending errors
			if (sendToNeighbor(socketNum, lsPktBuffer, destNode->addr) == -1)
			{
				return -1;
			}
		}
		destNode++;
	}
	
	return 0;
}

int listenPacket(int socketNum, void* lsPktBuffer, int timeout, char* rcvdFromLbl, node* nodeList, int numNodes)
{
	int selectResult;
	int bytesReceived;

	struct timeval timeoutSys;
	fd_set readset;

	if (timeout >= 0)
	{
		// Initialize the set
		FD_ZERO(&readset);
		FD_SET(socketNum, &readset);

		// Calculate and set the select timeout
		timeoutSys.tv_sec = timeout/1000; 		// SECONDS
		timeoutSys.tv_usec = (timeout%1000)*1000; 	// +MICROSECONDS

		selectResult = select(socketNum+1, &readset, NULL, NULL, &timeoutSys);

		if (selectResult == 0) // No changes to the file descriptors implies a timeout
		{
			return -2;
		}
		else if (selectResult == -1) // -1 implies an error in the code
		{
			return -1;
		}
	}
	
	struct sockaddr_in rcvdFrom;
	socklen_t networkLen = sizeof(rcvdFrom);

	// No timeout or select errors, so the read can occur.
	bytesReceived = recvfrom(
					socketNum,
					lsPktBuffer,
					LS_PACKET_BUFFER_SIZE,
					0,
					(struct sockaddr *)&rcvdFrom,
					&networkLen);
					
	printf("Recieved packet from: %c\n", portToNodeLabel(nodeList, numNodes, &rcvdFrom));

	return bytesReceived;
}

char portToNodeLabel(node* nodeList, int numNodes, struct sockaddr_in* rcvdFrom)
{
	int i;
	
	// For each item in the node list
	for (i=0; i<numNodes; i++)
	{
		// If port and address match (just leave everything in network order)
		if (((nodeList+i)->addr != NULL) &&
			((nodeList+i)->addr->sin_port == rcvdFrom->sin_port) &&
			((nodeList+i)->addr->sin_addr.s_addr == rcvdFrom->sin_addr.s_addr))
			{
				printf("Match found.\n");
				return indToLabel(i);
			}
	}
	
	return 0;
}

void printLSPacketDebug(void* lsPktBuffer)
{
	// Print Header
	lsHeader* h = (lsHeader*) lsPktBuffer;
	printf("Header: hopCount:%d seqNum:%d numNeighbors:%d sourceLabel:%c\n", 
			h->hopCount, h->seqNum, h->numNeighbors, h->sourceLabel);
	
	lsEdge* curEdge = lsPktBuffer+sizeof(lsHeader);

	// Print neighbors
	int i;
	for (i = 0; i < h->numNeighbors; i++)
	{
		printf("\t%c at cost %d\n", curEdge->destLabel, curEdge->cost);
		curEdge++;
	}
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

void confirmArgs(int argc, char* argv[], int *ownPort, int* totalNumRouters, int* isDynamic)
{
	int printUsage = 0;
	
	switch (argc)
	{
		case 6:
			// Test for dynamic tag
			*isDynamic = (strcmp(argv[5], TAG_DYNAMIC) == 0); 
			if (!(*isDynamic))
			{
				printf("The optional 6th argument must be \""TAG_DYNAMIC"\".\n");
				printUsage = 1;
				break;
			}
		case 5:
			if (strlen(argv[1]) != 1)
			{
				printf("The router label should be a single character.\n");
				printUsage = 1;
				break;
			}
			
			// Attempt to convert the port and totalNumRouters to integers
			*ownPort = confirmIntString(argv[2], 0, MAX_PORT, MAX_PORT_CHARS);
			*totalNumRouters = confirmIntString(argv[3], 0, MAX_NUM_ROUTERS, MAX_NUM_ROUTERS_CHARS);
			
			printUsage = (*ownPort == -1) || (*totalNumRouters == -1);
			break;
		default:
			printf("Incorrect number of arguments.\n");
			printUsage = 1;
	}

	if (printUsage)
	{
		printf("node <routerLabel> <portNum> <totalNumRouters> <discoverFile> [-dynamic]\n");
		exit(1);
	}
}

node* allocNodeArray(int numNodes, int seqNum)
{
	// Allocate memory for the new node array
	node* nodeArray = malloc(sizeof(node)*numNodes);

	int i;
	// Initialize each node
	for (i = 0; i < numNodes; i++)
	{
		node* curNode = nodeArray+i;
		curNode->seqNum = seqNum;
		curNode->addr = NULL;
	}

	return nodeArray;
}

void replaceSubStr(char* str, char matchChar, char replacement)
{
	char* strPtr = str;
	while(*(strPtr++) != '\0') // Iterate through each character
	{
		if (*strPtr == matchChar)
		{
			*strPtr = replacement;
		}
	}
}

struct sockaddr_in *configureNeighbor(const char *nodeHostName, int nodePort, struct sockaddr_in *node)
{
	struct hostent *hostptr;

	// Set node address attributes
	memset((char*)node, 0, sizeof(struct sockaddr_in));
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

int configureHostInfo(int port, node* hostNode)
{
	hostNode->addr = malloc(sizeof(struct sockaddr_in));
	
	int netSock = socket(AF_INET, SOCK_DGRAM, 0);

	// Was the socket created successfully?
	if(netSock < 0)
	{
		return -1; // Socket creation failed
	}
	
	// Clear and configure the the socket
	memset((char *)hostNode->addr, 0, sizeof(struct sockaddr_in));
	hostNode->addr->sin_family = AF_INET;
	hostNode->addr->sin_addr.s_addr = htonl(INADDR_ANY);
	hostNode->addr->sin_port = htons(port);
	
	// Bind the socket.
	if(bind(netSock, (struct sockaddr *)hostNode->addr, sizeof(struct sockaddr_in)) < 0)
	{
		return -1; // Binding to local socket failed
	}

	printf("Port successfully bound!\n");

	return netSock; // No errors occurred binding the socket
}

int findLSPacketLen(void* lsPktBuffer)
{
	lsHeader* h = (lsHeader*) lsPktBuffer;
	return (sizeof(lsHeader)+sizeof(lsEdge)*h->numNeighbors);
}