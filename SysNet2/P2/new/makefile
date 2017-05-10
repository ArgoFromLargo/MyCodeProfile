#Compiler
CC = gcc

#Compiler flags for object files
# - Use –DDEBUG=1 to enable debug messages
CFLAGS = -c -g -Wall

#Program name
PNAME_SENDER = sender
PNAME_RECEIVER = receiver
PNAME_NETWORK = network

.PHONY: all
all: sender receiver network

# Link the programs
sender: rdtIPConverter.o rdtSender.o
	$(CC) -g -Wall -pthread rdtIPConverter.o rdtSender.o -o $(PNAME_SENDER)

receiver: rdtIPConverter.o rdtReceiver.o
	$(CC) -g -Wall -pthread rdtIPConverter.o rdtReceiver.o -o $(PNAME_RECEIVER)

network: rdtIPConverter.o rdtNetwork.o
	$(CC) -g -Wall -pthread rdtIPConverter.o rdtNetwork.o -o $(PNAME_NETWORK)

#Compile object files
rdtSender.o: rdtIPConverter.h rdtSender.h rdtSender.c
	$(CC) $(CFLAGS) rdtSender.c
	
rdtReceiver.o: rdtIPConverter.h rdtReceiver.h rdtReceiver.c
	$(CC) $(CFLAGS) rdtReceiver.c
	
rdtNetwork.o: rdtIPConverter.h rdtNetwork.c
	$(CC) $(CFLAGS) rdtNetwork.c

rdtIPConverter.o: rdtIPConverter.h rdtIPConverter.c
	$(CC) $(CFLAGS) rdtIPConverter.c

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(PNAME_SENDER)
	rm -f $(PNAME_RECEIVER)
	rm -f $(PNAME_NETWORK)
