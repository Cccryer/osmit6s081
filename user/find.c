#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char* fmtname(char *path)
{
    //get filename;
    char *p;

    for(p = path + strlen(path); p >= path  && *p != '/'; p--);
    p++;
    return p;
}

void finddir(char *path, char *filename)
{
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: connot open %s\n", path);
        return;
    }
    if(fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    switch(st.type){
        case T_DEVICE:
        case T_FILE:
            if(!strcmp(fmtname(path), filename))
                printf("%s\n", path);
            break;
        case T_DIR:
            char buf[512];//用buf存path，path空间大小已固定，不好扩展
            strcpy(buf, path);
            char *p = buf + strlen(buf);
            *p = '/';
            p++;
            while (read(fd, &de, sizeof(de)) == sizeof(de))
            {
                
                if(de.inum == 0)
                    continue;
                memmove(p, de.name, sizeof(de.name));
                p[sizeof(de.name)] = '\0';
                struct stat _st;
                if(stat(buf, &_st) < 0)
                {
                    printf("find: cannot stat %s\n", buf);
                    continue;
                }
                if(_st.type == T_DIR)
                {
                    // int pid = fork();
                    // if(pid == 0 )
                    // {
                    //     finddir(path, filename);
                    //     exit(0);
                    // }
                    if(!strcmp(de.name, ".") || !strcmp(de.name, ".."))
                        continue;
                    finddir(buf, filename);
                }
                else if(!strcmp(de.name, filename))
                    printf("%s\n", buf);
            }
            break;
    }
    close(fd);
    return;
}



int  main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Usage fnid [path] [filename]\n");
        exit(1);
    }

    finddir(argv[1], argv[2]);
    exit(0);
}
