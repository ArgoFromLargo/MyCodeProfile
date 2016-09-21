/* This assignment will explore the use of threads in a computational setting
and attempt to quantify their usefulness. It will also introduce a
common problem encountered in multi-threaded and multi-process
environments: a race condition. Although you will not be required to
solve the concurrency problems, after this assignment, you should be aware
of their existence and the difficulties they present. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {

	int argn = 0, argt = 0;

	if(argc < 3) {
		printf("Missing arguments. Format: ./mtCollatz [range] [number of threads]\n");
		exit(0);
	}

	if(argv[1] != NULL)
		argn = atoi(argv[1]); // the range of numbers for which a collatz sequence must be completed.
	if(argv[2] != NULL)
		argt = atoi(argv[2]); // the number of threads to create to compute the results in parallel.
	printf("Range of numbers: %d\n", argn);
	printf("Number of threads: %d\n", argt);

	return 0;
}
