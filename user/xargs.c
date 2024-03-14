#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

#define BUFFER_SIZE 1024
int main(int argc, char*argv[])
{

    char *argvs[MAXARG];
    char buf[BUFFER_SIZE];
    int j = 1;
    while(j < argc)
    {
        argvs[j-1] = argv[j];
        j++;
    }
    j--;
    int i = 0;
    read(0, &buf[i], 1);
    while(buf[i] != '\n' && i < BUFFER_SIZE)
    {
        i++;
        read(0, &buf[i], 1);
    }
    
    int p = 0, q = 0;
    int len = strlen(buf);
    while(q < len)
    {
        while(buf[q] != ' ' && buf[q] != '\n')
            q++;
        buf[q] = 0;
        argvs[j++] = buf+p;
        q++;
        p=q;
    }
    
    int pid = fork();
    if(pid == 0)
    {
        exec(argvs[0], argvs);
        exit(0);
    }
    wait(0);
    exit(0);
}