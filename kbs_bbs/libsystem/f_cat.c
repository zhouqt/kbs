#include "system.h"

/**
 *  cat string to file
 *  Param:
 *    file: file name
 *    str: string for cat
 *  Return: 0 - success,other - failed
 **/

int f_cat(char* file,char* str)
{
    int fd;
    int ret=0;
    
    if ((fd = open(file,O_WRONLY|O_CREAT|O_APPEND,0600)) >=0 ) {
        write(fd , str ,strlen(str));
        close(fd);
    } else ret=-1;
    return ret;
}

/**
 *  append src file to file
 *  Param:
 *    src: source file name for cat
 *    dst: destination file name
 *  Return: 0 - success,other - failed
 **/
int f_catfile(char* src,char* dst)
{
    int sfd,dfd;
    int ret=0;

    if ((sfd = open(src,O_RDONLY)) >= 0 ) {
        if ((dfd = open(dst,O_WRONLY|O_CREAT|O_APPEND,0600)) >= 0 ) {
#ifdef BUFFER_IN_STACK
            char buffer[READ_BUFFER_SIZE];
#else
            char* buffer = (char*)malloc(READ_BUFFER_SIZE);
#endif
            do {
                if ((ret = read(sfd, buffer, READ_BUFFER_SIZE)) <= 0)
                    break;
            } while (write(dfd, buffer, ret) > 0);
#ifndef BUFFER_IN_STACK
            free(buffer);
#endif
            close(dfd);
        } else ret=-1;
        close(sfd);
    } else ret=-1;
    return ret;
}
