#Compiler
CC = gcc

#Compiler flags for object files
# - Use –DDEBUG=1 to enable debug messages
CFLAGS = -c -g -Wall

#Program name
PNAME = node

# Link the program
node: node.o dijkstra.o
	$(CC) -g -Wall node.o dijkstra.o -o $(PNAME)

#Compile object files	
node.o: node.h node.c dijkstra.h
	$(CC) $(CFLAGS) node.c

dijkstra.o: dijkstra.h dijkstra.c
	$(CC) $(CFLAGS) dijkstra.c
	
.PHONY: clean
clean:
	rm -f *.o
	rm -f $(PNAME)
