/**
 * File:   rdtIPConverter.h
 *
 * rdtIPConverter allows the user to implement the RDP 3.0 protocol
 * over a UDP connection. The UDP portion is abstracted away wherever
 * possible so that the sender and reciever programs are not overcomplicated
 * by boilerplate IP code. It allows RDP to be abstracted so that is behaves
 * as a connection-based protocol in the sender, network, and receiver 
 * programs. 
 *
 * @author Luke Kledzik
 * @author Adam Mooers
 * @date 4/4/2017
 * @info Course COP4635
 */

#ifndef RDTIPCONVERTER_H
#define RDTIPCONVERTER_H

#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_PORT_CHARS 5
#define MAX_PORT 0xFFFF
#define SENDER_TIMEOUT_MS 2000
#define MAX_MSG_SIZE 2000

typedef struct rdpPacket rdpPacket;
typedef struct rdpSocket rdpSocket;

/*
 * Defines the network and transport layer components of the RDP packet
 * as described on page 2 of the project description. The byte layout of
 * the 54-byte package is shown in the following diagram:
 *
 * [ src IP ] [src Port] [ dest IP ] [dest Port] [flags] [ msg ]
 * [0     15] [16    21] [22     37] [38     43] [44 45] [45 53]
 *
 * Flags masking: [ack | isCorrupt | isLast | 00000]
 *  - ack: The expected boolean ack value (from sender) or ack value (from reciever)
 *  - corrupt: Indicates whether or not packet corrupted the network (set by network)
 */
struct rdpPacket
{
	char srcIP[16];			// The dotted-quad IPv4 address of the sender UDP socket
	char srcPort[6];		// The port number of the sender UDP socket
	char destIP[16];		// The dotted-quad IPv4 of the reciever UDP socket
	char destPort[6];		// The port number of the reciever UDP socket
	char ack;				// The expected boolean ack value (from sender) or ack value (from reciever)
	char corrupt;			// Indicates whether or not packet corrupted the network (set by network)
	char msgFrag[8];		// The message fragment to send to the user
}__attribute__((packed));

/*
 * Holds all of the structures needed to implement and RDP protocol over a UDP connection.
 * The goal is to have as little UDP/IP code in the sender and reciever state machines
 * as possible.
 */
struct rdpSocket
{
	struct sockaddr_in addr;		// Contains the socket configuration
	int netSock;					// The network socket that is bound
	struct sockaddr_in nextDest;	// The first recieving node of the packet, this is either the network or the destination
	struct sockaddr_in finalDest;	// The final destination of the packet
	struct sockaddr_in rcvdFrom;	// The information of the last node received from
	int state;						// The state of the RDP state machine (sender or reciever)
};

/**
 * Initializes an RDP socket for the current node. The state is cleared to 0 and the socket is bound
 * to the specified port. The next and final destinations are left unconfigured for maximum control.
 * Neither of these need to be configured to recieve a packet. To send a packet, the next destination
 * must be known. If the next destination is the network, the final destination must also be known.
 *
 * See configurePacketNode(...) for more information on configuring the destinations.
 *
 * Any error messages will be printed to perror.
 *
 * @param ownPort the port of the new RDP socket
 * @param rSocket the socket to initialize
 * @return 0 if an error occurred, !0 otherwise.
 */
int initRDPSocket(int port, rdpSocket *rSocket);

/**
 * Sends an RDP 3.0 packet over the previously configured UDP socket described by rSockt.
 * The message will be sent to the nextDest described by rSocket, with the assumption that
 * nextDest will attempt to deliver it to the destIP in pkt. No attempt is made to
 * handle or detect lost, delayed, or corrupted packages.
 *
 * @param pkt the pkt to send
 * @param rSocket the socket to send packet over
 * @return If successful, the number of sent bytes is returned. On an error, -1 is returned.
 */
ssize_t sendPacket(const rdpPacket *pkt, rdpSocket *rSocket);

/**
 * Sends an RDP 3.0 packet as described by sendpacket except with a delay. The call starts
 * a new thread an is non-blocking. pkt will be be copied for the new thread. The new thread
 * will also free the pkt when it is finished.
 *
 * @param pkt the packet to send
 * @param delay the delay in milliseconds
 * @param rSocket the socket to send the packet over
 * @return 0 on success, the error number otherwise
 */
int sendPacketDelayed(const rdpPacket *pkt, const int delay, rdpSocket *rSocket);

/**
 * Creates and sends an RDP 3.0 message fragment.
 *
 * The message length will be the minimum of either the remaining chars of msgPtr or
 * sizeof(rdpPacket.msgFrag). If there are more than or exactly sizeof(rdpPacket.msgFrag)
 * packets remaining, then msgFrag will be filled starting from msgFrag. It will not be
 * null-terminated. Otherwise, if there are not enough bytes left in the msgPtr string to
 * fill msgFrag, the  remaining bytes (except the the null-terminator) will be copied into
 * msgFrag. Beware of any newlines that may be appended to the end of the string.
 *
 * The message pointer will not remain constant. When the function returns, it will point
 * to the last byte that was copied into msgFrag. If there is nothing more to copy, msgFrag
 * will point to the string's null-terminated character.
 *
 * The corrupt bit will be set to 0 by default, indicating a non-corrupt packet.
 *
 * The sender and reciever ports and ip addresses will be determined from the rdpSocket, so
 * the rdpSocket must be initialized and have a valid destination.
 *
 * @param msgPtr the pointer to pointer of the first char of the message to send, must be null-terminated
 * @param ack the ack sequence number of the new packet
 * @param rSocket the socket to send the new packet over
 * @return If successful, the number of sent bytes is returned. On an error, -1 is set.
 */
ssize_t sendFragment(char **msgPtr, const char ack, rdpSocket *rSocket);

/**
 * Configures a given  destination of the rdp socket after running a DNS lookup of the host name.
 * The input should be either a dotted-quad formatted IPv4 address or a hostname. Do not reconfigure
 * the addr (self) node. Only configure nodes that are recievers from the self node (next dest and
 * final dest).
 *
 * @param nodeHostName the hostname or IPv4 address of the next packet destination
 * @param nodePort the port of the node destination.
 * @param node the node whose nextDest will be set (or overidden)
 * @return null if the host name was not resolvable, a pointer to node otherwise
 */
struct sockaddr_in *configurePacketNode(const char *nodeHostName, int nodePort, struct sockaddr_in *node);

/**
 * Listens for the next RDP packet on the given RDP socket. The socket must be initialized (bound
 * to a port) in order for this function to work correctly. The call is blocking. If a timeout occurs
 * the function returns. The rcvdFrom field of rSocket is set with the information of the sender.
 *
 * @param pktBuffer the buffer in which to store the incoming packet
 * @param rSocket the socket to listen for packets on
 * @param timeout the timeout in milliseconds, no timeout if negative
 * @return the number of bytes recieved, -2 if a timeout occurred, or -1 if an error occurred.
 */
int getNextRDPPacket(rdpPacket *pktBuffer, rdpSocket *rSocket, int timeout);


/**
 * Prints a packet's contents to stderr for debugging purposes.
 *
 * @param pkt the packet whose contents to display
 */
void printPacketDebug(const rdpPacket *pkt);

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

#endif
