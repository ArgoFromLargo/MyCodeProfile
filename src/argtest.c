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
    
    printf("\nInput: \n");
    
    char buffer[32];
    
    while (fgets(buffer, sizeof buffer, stdin) != NULL)
    {
        printf("%s", buffer);
    }
    
    putchar('\n');
}
