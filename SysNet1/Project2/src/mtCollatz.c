/**
 * mtCollatz.c is a multithreaded simulation for computing the collatz
 * sequences of number series. The output to stdout contains the sequence
 * lengths compiled into a csv-style histogram. Additional info in stderr
 * contains timing information for benchmark purposes.
 *
 * Usage: mtCollatz [range] [number of threads]
 *
 * @author Adam Mooers
 * @author Luke Kledzik
 * @date 10/2/2016
 * @info Course COP4634
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h> 

#define HIST_SIZE 1000

/**
 * Global variables to store the stopping time frequencies from 1-N
 * and to keep track of index in the array when running threads
 */
int* stoppingTimes;
int nCount = 1;

/**
 * This is the function that will be called by pthread_create.
 * This function calculates the stopping times from 2-N and increments
 * that corresponding stopping time's index in the stoppingTimes array.
 *
 * @param n The number to calculate the collatz sequence length for 
 * @return length of the collatz sequence
 */
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

/**
 * This function traverses through the stoppingTimes array
 * and prints the histogram data to stdout, in .csv style,
 * one index per line.
 * Example: 132, 328
 *
 * @param arr histogram array containing sequence length counts
 * @param len length of the array
 */
void getStoppingTimes(int* arr, int len) {
   int i;
   for(i = 1; i <= len; i++) {
      printf("%d, %d\n", i, arr[i]);
   }
}

/**
 * The main function receives command line arguments and
 * runs its processes according to the input commands.
 * Example: ./mtCollatz 10000 4
 * In the previous example the mtCollatz program is being
 * run (this .c file's executable), it is getting passed 1000 and 4.
 * 1000 corresponds to the range of numbers the user wants to calculate
 * collatz stopping times for, and 4 corresponds to the number of threads
 * the user wishes to use.
 * 
 * @param argc number of command-line arguments
 * @param argv command line arguments
 * @return exit status of main thread
 */
int main(int argc, char** argv) {

   int argN = 0; // first command line arg (range of numbers to test the collatz sequence on)
   int argT = 0; // second command line arg (number of threads desired to run the collatz sequence)
   
   struct timespec tStart, tEnd; // time variables to keep track of elapsed time during program execution
   
   // Get the start time
   clock_gettime(CLOCK_REALTIME, &tStart);
     
   if(argc < 3) {
      fprintf(stderr, "Missing arguments. Format: ./mtCollatz [range] [number of threads]\n");
      exit(0);
   }

   if(argv[1] != NULL)
      argN = atoi(argv[1]); // the range of numbers for which a collatz sequence must be completed
   if(argv[2] != NULL)
      argT = atoi(argv[2]); // the number of threads to create to compute the results in parallel
	
   
   stoppingTimes = calloc(HIST_SIZE, sizeof(int)); // allocate space for the stoppingTimes array, init to zeroes
   pthread_t* threads = malloc(argT * sizeof(pthread_t));   // allocate space for pthread variables depending on
                                                            // second command line argument
   int i;
   // creating threads
   for(i = 0; i < argT; i++) {
      pthread_create(&threads[i], NULL, collatz, (void *)&argN);
   }
   // joining threads
   for(i = 0; i < argT; i++) {
      pthread_join(threads[i], NULL);
   }
   
   // Get the end time
   clock_gettime(CLOCK_REALTIME, &tEnd);
   fprintf(stderr, "%d, %d, %.9lf\n", argN, argT, (tEnd.tv_sec-tStart.tv_sec)+(tEnd.tv_nsec-tStart.tv_nsec)*(1E-9));

   getStoppingTimes(stoppingTimes, HIST_SIZE); // traverse through the stoppingTimes array and print data to stdout
   
   // free the dynamically allocated memory
   free(stoppingTimes);
   free(threads);
      
   return 0;
}
