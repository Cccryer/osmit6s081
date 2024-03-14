#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define NUM_MAX 35



int main(int argc, char *argv[])
{
    int p[2];
    pipe(p);

    for(int i = 2; i < NUM_MAX; i++)
        write(p[1], &i, sizeof(int));

    int n;

    int pid = fork();
    while(pid == 0)
    {
        close(p[1]);
        int pp[2];
        pipe(pp);
        int m;
        if(!read(p[0], &n, sizeof(int)))
            exit(0);
        printf("prime %d\n", n);
        while(read(p[0], &m, sizeof(int)))
        {
            if(m % n != 0)
                write(pp[1], &m, sizeof(int));
        }
        p[0] = pp[0];
        p[1] = pp[1];
        pid = fork();
    }
    close(p[0]);
    close(p[1]);
    wait(0);
    exit(0);
}

/*
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define NUM_MAX 35


void get_prime(int p[2])
{

    close(p[1]);
    int n;
    if(!read(p[0], &n, sizeof(int)))
        exit(0);

    printf("prime %d\n", n);

    int pp[2];
    pipe(pp);
    int pid = fork();

    if(pid > 0)
    {
        close(pp[0]);
        int m;
        while(read(p[0], &m, sizeof(int)))
        {
            if(m % n != 0)
                write(pp[1], &m, sizeof(int));
        }
        close(pp[1]);
        close(p[0]);
        wait(0);
    }
    else if(!pid)
        get_prime(pp);
    else
    {
        printf("fork error\n");
        exit(0);
    }
    exit(0);
}


int main(int argc, char *argv[])
{
    int p[2];
    pipe(p);

    for(int i = 2; i < NUM_MAX; i++)
        write(p[1], &i, sizeof(int));
    get_prime(p);
    exit(0);
}

*/