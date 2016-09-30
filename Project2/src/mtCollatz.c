/* This assignment will explore the use of threads in a computational setting
and attempt to quantify their usefulness. It will also introduce a
common problem encountered in multi-threaded and multi-process
environments: a race condition. Although you will not be required to
solve the concurrency problems, after this assignment, you should be aware
of their existence and the difficulties they present. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h> 

#define HIST_SIZE 1000

int* stoppingTimes;
int nCount = 1;

void* collatz(void* n) {
   while(nCount <= *(int *)n) {
      int i = 1;
      int prev = ++nCount;
   
      while(prev > 1) {
         if(prev % 2 == 0) // prev is EVEN
            prev /= 2;
         else              // prev is ODD
            prev = prev * 3 + 1;
         i++;
      }
      stoppingTimes[i]++;
   }
   pthread_exit(0);
}

void getStoppingTimes(int* arr, int len) {
   int i;
   for(i = 2; i <= len; i++) {
      printf("Index: %d Freq: %d\n", i, arr[i]);
   }
}

int main(int argc, char** argv) {

   int argN = 0, argT = 0;

   if(argc < 3) {
      printf("Missing arguments. Format: ./mtCollatz [range] [number of threads]\n");
      exit(0);
   }

   if(argv[1] != NULL)
      argN = atoi(argv[1]); // the range of numbers for which a collatz sequence must be completed.
   if(argv[2] != NULL)
      argT = atoi(argv[2]); // the number of threads to create to compute the results in parallel.
	
   
   stoppingTimes = calloc(HIST_SIZE, sizeof(int));
   pthread_t* threads = malloc(argT * sizeof(pthread_t));
   
   printf("Range of numbers: %d\n", argN);
   printf("Number of threads: %d\n", argT);
   
   int i;
   for(i = 0; i < argT; i++) {
      pthread_create(&threads[i], NULL, collatz, (void *)&argN);
   }
   for(i = 0; i < argT; i++) {
      pthread_join(threads[i], NULL);
   }
   getStoppingTimes(stoppingTimes, HIST_SIZE);

   return 0;
}
