#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv)
{
    
    int p[2];
    pipe(p);
    char ch;
    int pid = fork();
    if(pid == 0)
    {
        read(p[0],&ch,1);
        printf("%d: received ping\n", getpid());
        write(p[1],&ch, 1);
    }
    else
    {
        ch = 'x';
        write(p[1],&ch,1);
        wait(0);
        read(p[0], "", 1);
        printf("%d: received pong\n", getpid());
    }
    exit(0);
}