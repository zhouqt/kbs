#include "system.h"

/**
 *  copy file src to file dst
 *  Param:
 *    src: source file name
 *    dst: destination file name
 *    mode: destination file open mode, can be O_EXCL ...
 *  Return: 0 - success,other - faled
 **/
#define READ_BUFFER_SIZE 10240

int f_cp(char* src,char* dst,int mode)
{
    int sfd,dfd;
    int ret=0;
    
    if ((sfd = open(src,O_RDONLY)) >= 0 ) {
        if ((dfd = open(dst,O_WRONLY|O_CREAT|O_TRUNC|mode,0600)) >= 0 ) {
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
