myshell: myshell.o parse.o
	gcc -o myshell myshell.o parse.o

myshell.o: myshell.c
	gcc -c -Wall myshell.c

parse.o: parse.c
	gcc -c -Wall parse.c

clean:
	rm myshell.o parse.o output.exe
