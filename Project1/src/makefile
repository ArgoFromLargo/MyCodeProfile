#Compiler
CC = gcc

#Compiler flags for object files
# - Use –DDEBUG=1 to enable debug messages
CFLAGS = -c -g -Wall

#Program name
PNAME = myshell

# Link the program
myshell: myshell.o parse.o
	$(CC) -g myshell.o parse.o -o $(PNAME)

#Link objects
myshell.o: myshell.c
	$(CC) $(CFLAGS) myshell.c

parse.o: parse.c
	$(CC) $(CFLAGS) parse.c

clean:
	rm -f *.o
	rm -f $(PNAME)
