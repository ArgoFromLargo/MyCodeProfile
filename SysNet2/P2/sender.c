#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "rdtSender.h"

#define MAX_MSG_SIZE 500

int main(int argc, char **argv) {

    /***** VARIABLE DECLARATIONS *****/
    char typedMsg[MAX_MSG_SIZE] = {0};

    confirmArgs(argc);

    printf("Enter message: ");
    fgets(typedMsg, MAX_MSG_SIZE, stdin);

    sendMessage(atoi(argv[1]), argv[4], atoi(argv[5]), argv[2], atoi(argv[3]), typedMsg);

    return 0;
}

void confirmArgs(int argc) {
    if(argc != 6) {
        printf("Incorrect number of arguments.\n");
        printf("Usage: ./sender <port> <rcvHost> <rcvPort> <networkHost> <networkPort>\n");
        exit(1);
    }
}

int sendMessage(int localPort, char* netwhost, int netwPort, char* desthost, int destPort, char* message) {

    int senderSocket;
    struct sockaddr_in senderAddr; // Own address
    struct sockaddr_in networkAddr; // Where to sendto
    socklen_t networkLen = sizeof(networkAddr);
    struct hostent *hostptr;
    char seqNum;
    fd_set readset;
    int result;
    struct timeval timeout;
    int success = 0; 		// Used for verifying ACK
    char segment[9] = {0}; 	// The msg segment that will be sent in each packet
    char ack[54] = {0}; 	// Used for packet sent back from receiver
    char packet[54] = {0}; 	// SrcIP 0-15 (localIP)
							// SrcPort 16-21 (localPort)
							// DestIP 22-37 (desthost)
                     // DestPort 38-43 (destPort)
                     // Segment 44-53 (message) Byte 44 used for seqNum/ack

    // Create socket
    if((senderSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket creation failed.\n");
        exit(1);
    }

    memset((char *)&senderAddr, 0, sizeof(senderAddr));
    senderAddr.sin_family = AF_INET;
    senderAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    senderAddr.sin_port = htons(localPort);

    // Bind the socket.
    if(bind(senderSocket, (struct sockaddr *)&senderAddr, sizeof(senderAddr)) < 0) {
        printf("Binding of socket failed.\n");
        exit(1);
    }

    // Set network address attributes

    memset((char*)&networkAddr, 0, sizeof(networkAddr));
    networkAddr.sin_family = AF_INET;
    networkAddr.sin_port = htons(netwPort);

    hostptr = gethostbyname(netwhost);
    if(!hostptr) {
        printf("Failure to get host by name for: %s\n", netwhost);
        exit(1);
    }

    // Place the network address into the address structure
    memcpy((void *)&networkAddr.sin_addr, hostptr->h_addr_list[0], hostptr->h_length);

    // Fill out packet info
    char *sourceIP = inet_ntoa(senderAddr.sin_addr); // Return the IP

    // Ready to format messages that need to be sent
    // message parameter needs to be segmented into portions to send

    memcpy(packet, sourceIP, strlen(sourceIP)); // Fill in source IP
    sprintf(packet + 16, "%d", localPort); // Fill in source port
    memcpy(packet + 22, desthost, strlen(desthost)); // Fill in dest IP
    sprintf(packet + 38, "%d", destPort); // Fill in dest port

    // Set seqNum, valid/corrupt bit, and input portion of message

    int i, j, k, l;
    // Traverse through the message 7 chars at a time
    for(i = 0; i < strlen(message); i += 7) {
        // Clean out segment from previous use
        for(l = 0; l < 7; l++) {
            segment[l] = 0;
        }
        // Stop after 7 chars have been transferred orafter it reaches the end of message
        for(j = i, k = 0; (j < i + 7) && (j < strlen(message)); j++, k++) {
            segment[k] = message[j]; // Copy the chars 1-by-1
        }

        // SET SEQUENCE NUMBER
        if((i / 7) % 2 == 0) {
            packet[44] = '0';
            seqNum = '0';
        }
        else {
            packet[44] = '1';
            seqNum = '1';
		}

		// SET CORRUPT BIT TO 0, THIS IS MODIFIED BY NETWORK
        packet[45] = '0';

		// IF THIS IS THE LAST SEGMENT OF THE MESSAGE, BYTE 46 = 1
		if(i + 7 >= strlen(message)) {
			packet[46] = '1';
		}
		else {
			packet[46] = '0';
		}

        // Clean out previous message and replace with null characters
        for(l = 47; l < 54; l++) {
            packet[l] = 0;
        }

		// COPY THE MESSAGE SEGMENT INTO THE PACKET
        if(packet[46] == '1') {
            memcpy(packet + 47, segment, strlen(segment) - 1); // Fill in segment without newline
        }
        else {
            memcpy(packet + 47, segment, strlen(segment)); // Fill in segment
        }

        do {
    		// DISPLAY THE PACKET BEING SENT
            printf("Message being sent: ");
            printf("%s ", packet);
            printf("%s ", packet + 16);
            printf("%s ", packet + 22);
            printf("%s ", packet + 38);
            printf("%c ", packet[44]);
            printf("%c ", packet[45]);
            printf("%c ", packet[46]);
    		printf("%s\n", packet + 47);

            // Now ready to send message to the network
            if(sendto(senderSocket, packet, 54, 0, (struct sockaddr *)&networkAddr, sizeof(networkAddr)) < 0) {
                printf("Failed sendto()\n");
                exit(1);
            }

            // TIMER STARTS HERE

            // Initialize the set
            FD_ZERO(&readset);
            FD_SET(senderSocket, &readset);

            // TIMEOUT BEING SET TO 1 SECOND
            timeout.tv_sec = 1; // SECONDS
            timeout.tv_usec = 0; // MICROSECONDS
            // select()
            result = select(senderSocket+1, &readset, NULL, NULL, &timeout);

            // Check status
            if (result < 1)
                printf("Timeout on expected ACK...\n");
            else {
                // Receive ACK
                recvfrom(senderSocket, ack, 54, 0, (struct sockaddr *)&networkAddr, &networkLen);

                // Check valid/corrupt
                if(ack[45] == '1') {
                    printf("Received packet is corrupt...\n");
                    success = 0;
                }

                // Check ACK
                if(ack[44] == seqNum) {
                    printf("Received message: ");
            		printf("%s ", ack);
                    printf("%s ", ack + 16);
                    printf("%s ", ack + 22);
                    printf("%s ", ack + 38);
                    printf("%c ", ack[44]); // SEQUENCE NUMBER
                    printf("%c ", ack[45]); // VALID/CORRUPT
                    printf("%c ", ack[46]); // LAST SEGMENT FLAG
            		printf("%s\n", ack + 47);

                    success = 1;
                }
                // If incorrect ACK
                else {
                    printf("Unexpected ACK received...\n");

                    success = 0;
                }
            }
        }while(!success);
    }

    close(senderSocket);

    return 0;
}
