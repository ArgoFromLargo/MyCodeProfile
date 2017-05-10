#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

/***** FUNCTION PROTOTYPES *****/

void confirmArgs(int argc);

/*******************************/

int main(int argc, char **argv) {

    /***** VARIABLE DECLARATIONS *****/
    int networkSocket;
    struct sockaddr_in networkAddr; // Own address
    struct sockaddr_in senderAddr; // Where to recvfrom
    struct sockaddr_in receiverAddr; // Where to sendto
    struct hostent *hostptr;
    socklen_t senderLen = sizeof(senderAddr);
    char packet[54] = {0};
    char* sourceIP;

    confirmArgs(argc);

    if((networkSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket creation failed.\n");
        exit(1);
    }

    memset((char *)&networkAddr, 0, sizeof(networkAddr));
    networkAddr.sin_family = AF_INET;
    networkAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    networkAddr.sin_port = htons(atoi(argv[1]));

    // Bind the socket.
    if(bind(networkSocket, (struct sockaddr *)&networkAddr, sizeof(networkAddr)) < 0) {
        printf("Binding of socket failed.\n");
        exit(1);
    }

    // RECEIVE MESSAGE FROM SENDER HERE

    printf("Ready to receive messages...\n");

    while(1) {
        recvfrom(networkSocket, packet, 54, 0, (struct sockaddr *)&senderAddr, &senderLen);

        printf("Received message: ");

        sourceIP = inet_ntoa(senderAddr.sin_addr); // Get the IP from sender
        memcpy(packet, sourceIP, strlen(sourceIP)); // Fill in source IP

        printf("%s ", packet);
        printf("%s ", packet + 16);
        printf("%s ", packet + 22);
        printf("%s ", packet + 38);
        printf("%c ", packet[44]);
        printf("%c ", packet[45]);
        printf("%c ", packet[46]);
		printf("%s\n", packet + 47);

        // FILL OUT ADDRESS STRUCTURE FOR RECEIVER AFTERWARDS

        memset((char*)&receiverAddr, 0, sizeof(receiverAddr));
        receiverAddr.sin_family = AF_INET;
        receiverAddr.sin_port = htons(atoi(packet + 38)); // Retreived from sender's message

        hostptr = gethostbyname(packet + 22); // Retreived from sender's message
        if(!hostptr) {
            printf("Failure to get host by name for: %s\n", packet + 22);
            exit(1);
        }

        // Place the receiver address into the address structure
        memcpy((void *)&receiverAddr.sin_addr, hostptr->h_addr_list[0], hostptr->h_length);

        // Now capable of sending messages to the receiver

		if(sendto(networkSocket, packet, 54, 0, (struct sockaddr *)&receiverAddr, sizeof(receiverAddr)) < 0) {
            printf("Failed sendto()\n");
            exit(1);
        }
    }

    close(networkSocket);

    return 0;
}

void confirmArgs(int argc) {
    if(argc != 5) {
        printf("Incorrect number of arguments.\n");
        printf("Usage: ./network <port> <lostPercent> <delayedPercent> <errorPercent>\n");
        exit(1);
    }
}
