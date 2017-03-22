#Compiler
CC = gcc

#Compiler flags for object files
CFLAGS = -g -Wall -pthread

#Program name
PNAME = mtCollatz

# Link the program
all: mtCollatz.c
	$(CC) $(CFLAGS) mtCollatz.c -o $(PNAME)

clean:
	rm -f $(PNAME)
