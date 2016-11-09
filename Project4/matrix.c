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

    for(i = 0; i < 10; i++) {
        start = clock();
        writeRow();
        finish = clock();

        total = (finish - start); // AVERAGE THE TIMES
        milliSecs = total * 1000 / CLOCKS_PER_SEC;
        secs = milliSecs / 1000;
        milliSecs = milliSecs % 1000;

        printf("WR, %d.%d\n", secs, milliSecs);
    }

    // WRITE BY COLUMN

    for(i = 0; i < 10; i++) {
        start = clock();
        writeColumn();
        finish = clock();

        total = (finish - start); // AVERAGE THE TIMES
        milliSecs = total * 1000 / CLOCKS_PER_SEC;
        secs = milliSecs / 1000;
        milliSecs = milliSecs % 1000;

        printf("WC, %d.%d\n", secs, milliSecs);
    }

    // READ BY ROW

    for(i = 0; i < 10; i++) {
        start = clock();
        readRow();
        finish = clock();

        total = (finish - start); // AVERAGE THE TIMES
        milliSecs = total * 1000 / CLOCKS_PER_SEC;
        secs = milliSecs / 1000;
        milliSecs = milliSecs % 1000;

        printf("RR, %d.%d\n", secs, milliSecs);
    }

    // READ BY COLUMN

    for(i = 0; i < 10; i++) {
        start = clock();
        readColumn();
        finish = clock();

        total = (finish - start); // AVERAGE THE TIMES
        milliSecs = total * 1000 / CLOCKS_PER_SEC;
        secs = milliSecs / 1000;
        milliSecs = milliSecs % 1000;

        printf("RC, %d.%d\n", secs, milliSecs);
    }

    return 0;

}

// READING AND WRITING FUNCTIONS

/*
Writes the char 'R' to every index in the global
2-dimensional array row-by-row
*/
void writeRow() {
    int i, j;
    for(i = 0; i < 20480; i++) {
        for(j = 0; j < 4096; j++) {
            matrix[i][j] = 'R';
        }
    }
}

/*
Writes the char 'C' to every index in the global
2-dimensional array column-by-column
*/
void writeColumn() {
    int i, j;
    for(i = 0; i < 4096; i++) {
        for(j = 0; j < 20480; j++) {
            matrix[j][i] = 'C';
        }
    }
}

/*
Reads the char in every index in the global
2-dimensional array row-by-row
*/
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

/*
Reads the char in every index in the global
2-dimensional array column-by-column
*/
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
