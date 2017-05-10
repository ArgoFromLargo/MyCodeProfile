#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "rdtReceiver.h"

#define MESSAGE_SIZE 10

int main(int argc, char **argv) {
    confirmArgs(argc);

	char* message = receiveMessage(atoi(argv[1]));
	printf("Complete message: %s\n", message);
	free(message);

    return 0;
}

void confirmArgs(int argc) {
    if(argc != 2) {
        printf("Incorrect number of arguments.\n");
        printf("Usage: ./receiver <port>\n");
        exit(1);
    }
}

char* receiveMessage(int port) {
	int receiverSocket;
    struct sockaddr_in networkAddr; // Where we're receiving from
    struct sockaddr_in receiverAddr; // Own address
    socklen_t networkLen = sizeof(networkAddr);
    char packet[54] = {0};
    char recvAddr[16];
    char recvPort[6];
    char sendAddr[16];
    char sendPort[6];
	char* message = malloc(2048);

	if((receiverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket creation failed.\n");
        exit(1);
    }

    memset((char *)&receiverAddr, 0, sizeof(receiverAddr));
    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    receiverAddr.sin_port = htons(port);

    // Bind the socket.
    if(bind(receiverSocket, (struct sockaddr *)&receiverAddr, sizeof(receiverAddr)) < 0) {
        printf("Binding of socket failed.\n");
        exit(1);
    }

    // RECEIVE MESSAGE FROM SENDER HERE

    printf("Ready to receive messages...\n");

    // LOOP UNTIL BROKEN OUT OF BY RECEIVING THE LAST SEGMENT
	while(1) {

        // LOOP UNTIL SEQUENCE NUM = 0
        do{
    		recvfrom(receiverSocket, packet, 54, 0, (struct sockaddr *)&networkAddr, &networkLen);

    		printf("Received message: ");
    		printf("%s ", packet);
            printf("%s ", packet + 16);
            printf("%s ", packet + 22);
            printf("%s ", packet + 38);
            printf("%c ", packet[44]); // SEQUENCE NUMBER
            printf("%c ", packet[45]); // VALID/CORRUPT
            printf("%c ", packet[46]); // LAST SEGMENT FLAG
    		printf("%s\n", packet + 47);

            // STORE SENDER AND RECV INFO FROM PACKET
            memcpy(sendAddr, packet, 16);
            memcpy(sendPort, packet + 16, 6);
            memcpy(recvAddr, packet + 22, 16);
            memcpy(recvPort, packet + 38, 6);

            // SWAP SOURCE AND DEST INFO FROM DATA THAT WAS JUST CAPTURED
            memcpy(packet, recvAddr, 16);
            memcpy(packet + 16, recvPort, 6);
            memcpy(packet + 22, sendAddr, 16);
            memcpy(packet + 38, sendPort, 6);

            if(sendto(receiverSocket, packet, 54, 0, (struct sockaddr *)&networkAddr, sizeof(networkAddr)) < 0) {
    			printf("Failed sendto()\n");
    			exit(1);
    		}

        }while(packet[44] != '0');

        strcat(message, packet + 47); // ADD RECEIVED SEGMENT TO MESSAGE

        // IF LAST SEGMENT, BREAK OUT OF RECEIVING LOOP
		if(packet[46] == '1') {
			break;
		}

        // LOOP UNTIL SEQUENCE NUM = 1
        do{
    		recvfrom(receiverSocket, packet, 54, 0, (struct sockaddr *)&networkAddr, &networkLen);

    		printf("Received message: ");
    		printf("%s ", packet);
            printf("%s ", packet + 16);
            printf("%s ", packet + 22);
            printf("%s ", packet + 38);
            printf("%c ", packet[44]); // SEQUENCE NUMBER
            printf("%c ", packet[45]); // VALID/CORRUPT
            printf("%c ", packet[46]); // LAST SEGMENT FLAG
    		printf("%s\n", packet + 47);

            // STORE SENDER AND RECV INFO FROM PACKET
            memcpy(sendAddr, packet, 16);
            memcpy(sendPort, packet + 16, 6);
            memcpy(recvAddr, packet + 22, 16);
            memcpy(recvPort, packet + 38, 6);

            // SWAP SOURCE AND DEST INFO FROM DATA THAT WAS JUST CAPTURED
            memcpy(packet, recvAddr, 16);
            memcpy(packet + 16, recvPort, 6);
            memcpy(packet + 22, sendAddr, 16);
            memcpy(packet + 38, sendPort, 6);

            if(sendto(receiverSocket, packet, 54, 0, (struct sockaddr *)&networkAddr, sizeof(networkAddr)) < 0) {
    			printf("Failed sendto()\n");
    			exit(1);
    		}

        }while(packet[44] != '1');

        strcat(message, packet + 47); // ADD RECEIVED SEGMENT TO MESSAGE

        // IF LAST SEGMENT, BREAK OUT OF RECEIVING LOOP
		if(packet[46] == '1') {
			break;
		}
	}

	return message;
}
