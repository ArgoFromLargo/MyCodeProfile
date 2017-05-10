/**
 * File:   node.h
 *
 * node.h describes the networking interface for building forwarding tables. It defines
 * the functions required to create and configure UDP sockets, create link-state packets,
 * listen for incoming link-state packets, handle link-state packets, and forward them
 * to the set neighbors.
 *
 * Each function is designed from the perspective of a node: that is, a node should build
 * its own forwarding table. From a global perspective, all nodes build their forwarding
 * tables at the same time. Each node should be able to build the same graph from the given
 * information, and compute the correct forwarding table of every node including itself.
 *
 * node.h also handles the case where one or more links in the network has a dynamically 
 * changing weight. In this case, it enforces that functions support recalculation.
 *
 * @author Luke Kledzik
 * @author Adam Mooers
 * @date 4/15/2017
 * @info Course COP4635
 */

#ifndef NODE_H
#define NODE_H

#include <netinet/in.h>
#include <unistd.h>

#define MAX_NUM_NEIGHBORS 12
#define LS_PACKET_BUFFER_SIZE 2000	// Maximum length of incoming packet
#define MAX_PORT_CHARS 5
#define MAX_PORT 0xFFFF
#define MAX_NUM_ROUTERS_CHARS 2
#define MAX_NUM_ROUTERS 99
#define MAX_HOST_NAME_LEN 253
#define TAG_DYNAMIC "-dynamic"
#define LISTEN_TIMEOUT 10000

#define STR_HELPER(VAL) #VAL
#define STR(VAL) STR_HELPER(VAL)	// Converts a non-string constant into a string literal

typedef struct lsHeader lsHeader;
typedef struct lsEdge lsEdge;
typedef struct node node;

/**
 * Link-State packet that is distributed throughout the network when the routers
 * are building their forwarding tables. Each link state packet contains the info
 * about the source router and information about its connections to other routers.
 * Each packet consists of a header containing information about the source node,
 * followed by and array of lSEdges containing information about its neighbors.
 *
 * The packet format follows:
 * [ls Header][lsEdge 1][lsEdge 2][...][lsEdge N]
 *
 * Note, the number of neighbors is explicitly given in the header, but also implicitly given
 * by the size of the received packet. NumNeighbors = (packetLen-headerLen)/sizeof(lsEdge)
 */
struct lsHeader 
{
	uint16_t hopCount;		// Decremented at each node, packet is dropped after hopCount = 0. Prevents loops.
	uint16_t seqNum;		// The current state of the node
	uint16_t numNeighbors;	// Number of neighboring routers (array of edges follows headers)
	char sourceLabel;		// The label of the source node
}__attribute__((packed));

struct lsEdge 
{
	uint16_t cost;			// Cost of the link
	char destLabel;			// Vertex connected to the end link
}__attribute__((packed));

/*
 * Contains info about the node for the purpose of mapping to and from the adjacency matrix.
 * The name is not given because it is implicit to the position within the array.
 */
struct node
{
	int seqNum;
	struct sockaddr_in* addr; // Contact info, null if not a neighbor
};

/**
 * A constructor for the header of the link-state packet. The constructor is mainly for clarity
 * in the packet-filling process. See the lsHeader struct definition for more information about
 * each field.
 *
 * @param bufferLoc the memory to fill with the struct info
 * @return a pointer to the first byte after the newly-filled struct
 */
void* fillLSHeader(void* bufferLoc, 
		   unsigned int hopCount, 
		   unsigned int seqNum, 
		   unsigned int numNeighbors, 
		   char sourceLabel);

/**
 * A constructor for the lsEdge packet. The constructor is for simplifying the packet-filling
 * process. See the lsEdge struct for more information about the fields.
 *
 * @param bufferLoc the memory to fill with the struct info
 * @return a pointer to the first byte after the newly-filled struct
 */
void* fillLSEdge(void* bufferLoc, 
		 unsigned int cost, 
		 char destLabel);

/**
 * Adds the neighbors to the node list, adds their weights to the adjacency matrix,
 * sets their sequence numbers to -1, and finds their network info. If a line has a formatting error,
 * the loading process stops and -1 is returned. The nodeList is assumed empty when the parser is called.
 * The first node in the known node list is filled in with the current router's info.
 * 
 *
 * @param fileName name of the neighbor file
 * @param adjMatrix the matrix to place the neighbor edge weights in
 * @param totalNumRouters the total number of routers in the network
 * @param lsPktBuffer the name of the link state packet buffer for the local node
 * @param nodeList the array in which to store actual network contact info about the neighbors
 * @param ownName the current router's own name
 * @return the number of neighbors read from the file and stored in the link state packet. -1 if file error.
 */
int parseNeighborFile(char* fileName,
					int** adjMatrix, 
					int totalNumRouters, 
					void* lsPktBuffer, 
					node* nodeList, 
					char ownName);

/**
 * Sends the link state packet stored in the given buffer to the given neighbor. Be certain that 
 * a link-state packet (as defined in the struct definition region of this header) is actually stored.
 * If not, the resulting operation is undefined. The link state packet is sent as a single UDP packet
 * and no attempt is made to verify that the receiver actually receives that packet.
 *
 * @param socketNum the identifier of the UDP socket to send over
 * @param lsPktBuffer a pointer to the buffer containing the link-state packet
 * @param destNeighbor the neighbor to send the link-state packet to
 * @return -1 if not successful, the number of bytes send otherwise
 */
ssize_t sendToNeighbor(int socketNum, void* lsPktBuffer, struct sockaddr_in* destNeighbor);

/**
 * Attempts to send the given link state packet to all of the node's neighbors. Neighbors are contacted
 * in the order listed in the array, but the order in which the packets are received is not guaranteed.
 * If at any time the socket becomes unavailable, the remaining packets are dropped, the function returns.
 *
 * @param socketNum the identifier of the UDP socket to send over
 * @param lsPktBuffer a pointer to the buffer containing the link-state packet
 * @param hostName name of the node causing the flood
 * @param nodeList the array which contains all the nodes including the neighbors
 * @param numNodes the number of neighbors in nodeList
 * @return -1 if not successful, 0 otherwise
 */
int floodNeighbors(int socketNum, void* lsPktBuffer, char hostName,  node* nodeList, int numNodes);

/**
 * Listens on the given socket for a link-state packet to arrive. When the packet arrives,
 * the given buffer is filled with the new packet. No testing occurs to verify that the 
 * arriving packet is a genuine link-state packet. If no packet arrives within a specified
 * time window, the function returns with an empty buffer. In any case (even if a socket error
 * occurs), the buffer is zeroed.
 *
 * @param socketNum the identifier of the UDP socket to send over
 * @param lsPktBuffer the buffer to fill with the received link-state packet
 * @param timeout the waiting timeout in milliseconds
 * @param rcvdFromLbl the label of the node that sent the packet
 * @param nodeList the array which contains all the nodes including the neighbors
 * @param numNodes the number of neighbors in nodeList
 * @return -1 if socket failure, -2 if timeout occured, 0 if buffer correctly filled
 */
int listenPacket(int socketNum, void* lsPktBuffer, int timeout, char* rcvdFromLbl, node* nodeList, int numNodes);

/**
 * Searches the node list for the neighbor which matches the ip and port of the received
 * packet.
 *
 * @param nodeList the array which contains all the nodes including the neighbors
 * @param numNodes the number of neighbors in nodeList
 * @param rcvdFrom the address info about the receieved packet
 * @return the label of the node associated with the received packet, 0 otherwise
 */
char portToNodeLabel(node* nodeList, int numNodes, struct sockaddr_in* rcvdFrom);

/**
 * Builds the packet to send from the adjacency matrix and the node list. The neighbor file
 * parser must be called before a node's own link state packet can be built.
 *
 * @param lsPktBuffer the buffer to clear and fill
 * @param nodeList a list to the node list (populated with neighbors)
 * @param adjMatrix the adjacency matrix containing the edge weights for the neighbors
 * @param numNodes the number of items in nodeList
 * @param hopCount the number of hops before the link state packet dies out
 * @param numNeighbors number of neighbors to the host node
 * @param sourceLabel the label of the current node
 */
void buildLSPacket(	void* lsPktBuffer, 
			node* nodeList, 
			int** adjMatrix, 
			int numNodes, 
			int hopCount, 
			int numNeighbors, 
			char sourceLabel);

/**
 * Prints the link-state packet contained in the given buffer for debugging.
 *
 * @param lsPktBuffer a pointer to the buffer containing the link-state packet
 */
void printLSPacketDebug(void* lsPktBuffer);

/**
 * Handles the link-state packet. The packet will be dropped if it has already been received
 * and not forwarded if its hopCount is 0. hopCount will be decremented. If the packet contains
 * new information, the adjacency matrix and sequence number will be updated. If the node was 
 * previously unknown, it will be added to the node list. Lastly, if the packet is forwarded,
 * it will be sent to all of the node's neighbors. 
 *
 * @param lsPktBuffer the received link-state packet
 * @param nodeList the array which contains all the nodes including the neighbors
 * @param numNodes the number of neighbors in neighborList (may increase)
 */
void handleLinkStatePacket(void* lsPktBuffer, node* nodeList, int* numNodes);

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
 * Confirms the string is an actual int within the given range. If the int is not in the
 * given range, a message will be printed detailing the error.
 *
 * @param pStr the int string to check
 * @param lowerBound the lower inclusive bound of the integer
 * @param upperBound the upper inclusive bound of the integer
 * @param maxChars the maximum number of chars in the string (as given by strlen)
 * @return the converted int if all the criteria were met, -1 otherwise
 */
int confirmIntString(const char* pStr, unsigned int lowerBound, unsigned int upperBound, unsigned int maxChars);

/**
 * Confirms that the correct number of arguments are provided and that each
 * meets the formatting requirements. The following checks are run in the following
 * order.
 * 
 * Usage:
 * 	node routerLabel portNum totalNumRouters discoverFile [-dynamic]
 *
 * Test Cases:
 *	- Confirm that the number of arguments is in the range [5 6]
 *	- If the number of arguments is 6, confirm that the 6th argument is "-dynamic".
 *	- Confirm that routerLabel is a single char
 *	- Confirm portNum is an int and in the range [0, MAX_PORT]
 *	- Confirm totalNumRouters is an int > 0
 *
 * If any test fails, an error message is printed to the terminal, and the program exits.
 * @param argc the number of arguments from the command line
 * @param argv a pointer to the command-line arguments
 * @param ownPort a pointer to an integer to store the converted ownPort in
 * @param totalNumRouters a pointer to an integer to store the converted totalNumRouters in
 * @param isDynamic a pointer to a flag storing whether or not the dynamic flag was set
 */
void confirmArgs(int argc, char* argv[], int *ownPort, int* totalNumRouters, int* isDynamic);

/**
 * Creates an array of nodes, initializes the fields to default values,
 * and returns a reference to the newly-created array. The default address
 * is null.
 *
 * @param numNodes the number of nodes in the list
 * @param seqNum the filler sequence number
 * @return the newly-allocated node array
 */
node* allocNodeArray(int numNodes, int seqNum);

/**
 * Replaces each occurence of matchChar in the given string with the given replacement char.
 *
 * @param str the string to search and modify
 * @param matchChar the char to replace
 * @param replacement the replacement char
 */
void replaceSubStr(char* str, char matchChar, char replacement);

/**
 * Configures a given  neighbor's ip information after running a DNS lookup of the host name.
 * The input should be either a dotted-quad formatted IPv4 address or a hostname. 
 *
 * @param nodeHostName the hostname or IPv4 address of the next packet destination
 * @param nodePort the port of the neighbor node.
 * @param node the address info to set/overwrite
 * @return null if the host name was not resolvable, a pointer to node otherwise
 */
struct sockaddr_in *configureNeighbor(const char *nodeHostName, int nodePort, struct sockaddr_in *node);

/**
 * Configures the host info and attempts to bind the socket to a given port.
 * 
 * @param port the port to bind to
 * @param hostNode the node whose addr to fill out
 * @return -1 if an error occurred, or the socket number otherwise
 */
int configureHostInfo(int port, node* hostNode);

/**
 * Calculates and returns the length of a link state packet.
 *
 * @param lsPktBuffer the packet whose length to find
 * @return the length of the packet
 */
int findLSPacketLen(void* lsPktBuffer);
#endif