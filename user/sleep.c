#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if(argc <= 1 || argc >= 3)
    {
        printf("Usage: sleep ticks...\n");
        exit(1);
    }
    
    for(int i = 1; i < argc; i++)
    {
        if(*argv[i] < '0' || *argv[i] > '9')
        {
            printf("Unvalidated input, except ticks number...\n");
            exit(0);
        }
    }

    int ticks = atoi(argv[1]);
    sleep(ticks);
    exit(0);
}