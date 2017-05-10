#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "terminal.h"
#include <arpa/inet.h>

#define BUFSIZE 1024

int main(int argc, char **argv) {
    int sockFD, ownPort;
    int serverLen, destLen;
    unsigned short hostPort;
    struct sockaddr_in serverAddr, clientAddr, destClientAddr, sourceClientAddr;
    struct hostent *server;
    char *hostname;
    char ownHostName[BUFSIZE];
    char buffer[BUFSIZE];
    int token;
    const char delim[2] = " ";
    char *tokenize;
    char tokenArgs[6][BUFSIZE];


    ownPort = atoi(argv[1]);
    hostname = argv[2];
    hostPort = atoi(argv[3]);

    gethostname(ownHostName, BUFSIZE - 1);

    // Create socket
    sockFD = socket(AF_INET, SOCK_DGRAM, 0);

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    // Set attributes for connecting to a server (Already bound)
    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
    serverAddr.sin_port = htons(hostPort);

    // Set this client's attributes
    bzero((char *) &clientAddr, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons((unsigned short)ownPort);

    if(bind(sockFD, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0) {
        printf("Binding of socket failed.\n");
        return -1;
    }

    // Enter a message
    bzero(buffer, BUFSIZE);
    printf("Press ENTER to connect to server...");
    getchar();

    // Send message to server
    serverLen = sizeof(serverAddr);
    sendto(sockFD, buffer, strlen(buffer), 0, (const struct sockaddr *)&serverAddr, serverLen);

    // Display the IP the client will now try to contact
    recvfrom(sockFD, buffer, 18, 0, (struct sockaddr *)&serverAddr, (socklen_t *)&serverLen);
    printf("Pairing to this hostname: %s\n", buffer);

    // Display the port on which the client will contact as well
    recvfrom(sockFD, &hostPort, sizeof(unsigned short), 0, (struct sockaddr *)&serverAddr, (socklen_t *)&serverLen);
    printf("On this port: %hu\n", (hostPort));

    // Display if the client initially has the token. 1 = yes, 0 = no.
    recvfrom(sockFD, &token, sizeof(token), 0, (struct sockaddr *)&serverAddr, (socklen_t *)&serverLen);

    // Initalize info about where the client now needs to send their data to create a ring
    server = gethostbyname(buffer);
    bzero((char *)&destClientAddr, sizeof(destClientAddr));
    destClientAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&destClientAddr.sin_addr.s_addr, server->h_length);
    destClientAddr.sin_port = htons(hostPort);
    destLen = sizeof(destClientAddr);

    // If token is equal to 1, that means the client is the initiator and needs to pass along their message (token)
    // The initiator sends "TOKEN" to its destination
    if(token == 1) {
        sendto(sockFD, "TOKEN", sizeof "TOKEN", 0, (const struct sockaddr *)&destClientAddr, destLen);
    }
    else {
        memset(buffer, '\0', sizeof buffer);
    }

    printf("Now connected to the bulletin board!\n");
    printf("Acceptable commands: write, list, read <msgNum>, exit.\n");

	runTerminalThread();

    while(1) {
		// Lock access to the files
		pthread_mutex_lock(&terminalLock);

        recvfrom(sockFD, buffer, sizeof buffer, 0, (struct sockaddr *)&sourceClientAddr, (socklen_t *)&sourceClientAddr);

        // Get the first token
        tokenize = strtok(buffer, delim);
        int i = 0;

        while( tokenize != NULL )
        {
            strcpy(tokenArgs[i], tokenize);
            i++;
            tokenize = strtok(NULL, delim);
        }

        // See if this is a token
        if(tokenArgs[0][0] == 'T') {
            if(i == 1) {
                // Just a regular token
            }
            else if(i == 2) {
                // This is a request to join
            }
            else if(i == 6) {
                // This is a leave request
                if(tokenArgs[2] == ownHostName && atoi(tokenArgs[3]) == ownPort) {
                    server = gethostbyname(tokenArgs[4]);
                    bzero((char *)&destClientAddr, sizeof(destClientAddr));
                    destClientAddr.sin_family = AF_INET;
                    bcopy((char *)server->h_addr, (char *)&destClientAddr.sin_addr.s_addr, server->h_length);
                    destClientAddr.sin_port = htons(atoi(tokenArgs[5]));
                    destLen = sizeof(destClientAddr);
                }
            }
        }

        // Used for initialization
        else {

        }

        // Check if the peer is wishing to leave
		if (exitPending())
		{
            /*** ATTEMPTED CODE FOR ORGANIZING THE LEAVING OF A PEER ***/

            /*
			strcpy(buffer, "T L ");
			prevClient = gethostbyaddr((const char *)&sourceClientAddr.sin_addr.s_addr,
				sizeof(sourceClientAddr.sin_addr.s_addr), AF_INET);
			strcat(buffer, prevClient->h_name);
			strcat(buffer, " ");
			prevIP = inet_ntoa(sourceClientAddr.sin_addr);
			strcat(buffer, prevIP);
			strcat(buffer, " ");

			destClient = gethostbyaddr((const char *)&destClientAddr.sin_addr.s_addr,
				sizeof(destClientAddr.sin_addr.s_addr), AF_INET);
			strcat(buffer, destClient->h_name);
			strcat(buffer, " ");
			destIP = inet_ntoa(destClientAddr.sin_addr);
			strcat(buffer, destIP);
			printf("Modified token: %s\n", buffer);
            */
			break;
		}


        sendto(sockFD, buffer, sizeof buffer, 0, (const struct sockaddr *)&destClientAddr, destLen);

		// Allow terminal thread to access the file
		pthread_mutex_unlock(&terminalLock);
    }
	// No other thread has access to this lock at this point
	pthread_mutex_destroy(&terminalLock);

    return 0;
}
