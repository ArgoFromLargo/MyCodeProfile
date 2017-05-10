#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXCLIENTSALLOWED 12
#define BUFSIZE 1024

int main(int argc, char **argv) {
  int sockfd, port, clientsExp, clientsConnected = 0;
  socklen_t clientlen;
  struct sockaddr_in serveraddr, clientAddr;
  struct sockaddr_in clientAddrList[MAXCLIENTSALLOWED];
  struct hostent *hostptr;
  char buffer[BUFSIZE];
  char *hostaddrp;
  char serverName[BUFSIZE];
  unsigned short ports[MAXCLIENTSALLOWED];
  char clientIPs[MAXCLIENTSALLOWED][BUFSIZE];
  int token = 1, noToken = 0;

  // Check for command line args
  if (argc != 3) {
    fprintf(stderr, "3 arguments expected, %d given.", argc);
    exit(1);
  }
  port = atoi(argv[1]);
  clientsExp = atoi(argv[2]);

  // Create socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    printf("Socket creation failed.\n");
  }

  // Set serveraddr attributes
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)port);

  // Bind the socket to serveraddr
  if(bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
    printf("Binding of socket failed.\n");
    return -1;
  }

  // Get and display info about this server
  gethostname(serverName, BUFSIZE - 1);
  printf("Server address: %s\n", serverName);
  printf("Server port: %hu\n", port);

  clientlen = sizeof(clientAddr);

  while(clientsConnected < clientsExp) {
    // Receive contact from clients
    bzero(buffer, BUFSIZE);
    recvfrom(sockfd, buffer, BUFSIZE, 0, (struct sockaddr *)&clientAddr, &clientlen);

    // Get info about clients that contacted
    hostptr = gethostbyaddr((const char *)&clientAddr.sin_addr.s_addr, sizeof(clientAddr.sin_addr.s_addr), AF_INET);
    hostaddrp = inet_ntoa(clientAddr.sin_addr);
    printf("Contacted by a client from: %s (%s)\n", hostptr->h_name, hostaddrp);

    // Store client address info and ports in arrays
    clientAddrList[clientsConnected] = clientAddr;
    strcpy(clientIPs[clientsConnected], hostptr->h_name);
    ports[clientsConnected] = ntohs(clientAddr.sin_port);
    clientsConnected++;
  }

  int i;
  for(i = 0; i < clientsExp; i++) {
    bzero(buffer, BUFSIZE);
    strcpy(buffer, clientIPs[(i + 1) % clientsExp]);
    // Display what is being sent to the client (The IP of the client they need to contact)
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&clientAddrList[i], clientlen);
    // Display what is being sent to the client (The port of the client they need to contact)
    sendto(sockfd, &ports[(i + 1) % clientsExp], sizeof(ports[(i + 1) % clientsExp]), 0, (struct sockaddr *)&clientAddrList[i], clientlen);

    // SETTING THE FIRST CLIENT AS THE TOKEN INITIATOR
    // THIS CAN BE ALTERED LATER
    if(i == 0) {
        // printf("Sent token value: %d\n", token);
        sendto(sockfd, &token, sizeof(token), 0, (struct sockaddr *)&clientAddrList[i], clientlen);
    }
    else {
        // printf("Sent token value: %d\n", noToken);
        sendto(sockfd, &noToken, sizeof(noToken), 0, (struct sockaddr *)&clientAddrList[i], clientlen);
    }

  }
  close(sockfd);

  return 0;
}
