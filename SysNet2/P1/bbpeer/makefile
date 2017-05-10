#Compiler
CC = gcc

#Compiler flags for object files
# - Use –DDEBUG=1 to enable debug messages
CFLAGS = -c -g -Wall

#Program name
PNAME = bbpeer

# Link the program
all: terminal.o msgparser.o bbpeer.o
	touch messages.txt
	$(CC) -g -pthread bbpeer.o terminal.o msgparser.o -o $(PNAME)

bbpeer.o: bbpeer.c terminal.h
	$(CC) $(CFLAGS) bbpeer.c

terminal.o: terminal.c terminal.h msgparser.h
	$(CC) $(CFLAGS) terminal.c

msgparser.o: msgparser.c msgparser.h
	$(CC) $(CFLAGS) msgparser.c

clean:
	rm -f *.o
	rm -f $(PNAME)
	rm -f messages.txt
	touch messages.txt
