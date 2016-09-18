#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    printf("PID: %d Args: ", getpid());
    
    int i;
    for (i=0;i<argc;i++)
    {
        printf("%s ", argv[i]);
    }
    
    putchar('\n');
}
