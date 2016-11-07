/********************************************************
Project 4
Authors: Luke Kledzik & Adam Mooers
Date: Nov. 6, 2016
Filename: matrix.c

Description: this program simulates the time it takes to
access data that is adjacent in memory compared to data
that is separated by 4KB. This separation of 4KB
can cause a page fault due to its low proximity to the
previously accessed data.
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// FUNCTION PROTOTYPES
void writeRow();
void writeColumn();
void readRow();
void readColumn();

char matrix[20480][4096]; // GLOBAL ARRAY

int main(int argc, char** argv) {

    int i, secs, milliSecs;
    clock_t start, finish, total; // TIMER VARIABLES

    // WRITE BY ROW

    start = clock();
    for(i = 0; i < 10; i++) {
        writeRow();
    }
    finish = clock();

    total = (finish - start) / 10; // AVERAGE THE TIMES
    milliSecs = total * 1000 / CLOCKS_PER_SEC;
    secs = milliSecs / 1000;
    milliSecs = milliSecs % 1000;

    printf("Time to fill array row-by-row: %d.%d\n", secs, milliSecs);

    // WRITE BY COLUMN

    start = clock();
    for(i = 0; i < 10; i++) {
        writeColumn();
    }
    finish = clock();

    total = (finish - start) / 10; // AVERAGE THE TIMES
    milliSecs = total * 1000 / CLOCKS_PER_SEC;
    secs = milliSecs / 1000;
    milliSecs = milliSecs % 1000;

    printf("Time to fill array column-by-column: %d.%d\n", secs, milliSecs);

    // READ BY ROW

    start = clock();
    for(i = 0; i < 10; i++) {
        readRow();
    }
    finish = clock();

    total = (finish - start) / 10; // AVERAGE THE TIMES
    milliSecs = total * 1000 / CLOCKS_PER_SEC;
    secs = milliSecs / 1000;
    milliSecs = milliSecs % 1000;

    printf("Time to read array row-by-row: %d.%d\n", secs, milliSecs);

    // READ BY COLUMN

    start = clock();
    for(i = 0; i < 10; i++) {
        readColumn();
    }
    finish = clock();

    total = (finish - start) / 10; // AVERAGE THE TIMES
    milliSecs = total * 1000 / CLOCKS_PER_SEC;
    secs = milliSecs / 1000;
    milliSecs = milliSecs % 1000;

    printf("Time to read array column-by-column: %d.%d\n", secs, milliSecs);

    return 0;

}

// READING AND WRITING FUNCTIONS

void writeRow() {
    int i, j;
    for(i = 0; i < 20480; i++) {
        for(j = 0; j < 4096; j++) {
            matrix[i][j] = 'R';
        }
    }
}

void writeColumn() {
    int i, j;
    for(i = 0; i < 4096; i++) {
        for(j = 0; j < 20480; j++) {
            matrix[j][i] = 'C';
        }
    }
}

void readRow() {
    char readVal;
    int i, j;
    for(i = 0; i < 20480; i++) {
        for(j = 0; j < 4096; j++) {
            readVal = matrix[i][j];
        }
    }
    readVal = readVal; // SHUTS UP COMPILER'S "SET BUT NOT USED"
}

void readColumn() {
    char readVal;
    int i, j;
    for(i = 0; i < 4096; i++) {
        for(j = 0; j < 20480; j++) {
            readVal = matrix[j][i];
        }
    }
    readVal = readVal; // SHUTS UP COMPILER'S "SET BUT NOT USED"
}
