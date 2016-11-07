#include <stdio.h>
#include <stdlib.h>

char matrix[20480][4096]; // GLOBAL ARRAY

int main(int argc, char** argv) {



    return 0;

}

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
            matrix[i][j] = 'C';
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
}

void readColumn() {
    char readVal;
    int i, j;
    for(i = 0; i < 4096; i++) {
        for(j = 0; j < 20480; j++) {
            readVal = matrix[i][j];
        }
    }
}
