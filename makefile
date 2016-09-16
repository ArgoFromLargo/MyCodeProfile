myshell: myshell.o parse.o
	gcc -o -g myshell myshell.o parse.o

myshell.o: myshell.c
	gcc -c -g -Wall myshell.c

parse.o: parse.c
	gcc -c -g -Wall parse.c

clean:
	rm myshell.o parse.o output.exe
