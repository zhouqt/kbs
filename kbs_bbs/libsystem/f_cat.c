#include <fcntl.h>
#include "system.h"

#define BLK_SIZE 10240
void f_cat(fpath, msg)
    char *fpath;
    char *msg;
{
    int fd;

    if ((fd = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0600)) >= 0) {
        write(fd, msg, strlen(msg));
        close(fd);
    }
}

void f_catfile(char* srcpath,char* dstpath)
{
    int fsrc,fdst;
    if ((fsrc = open(srcpath, O_RDONLY)) != NULL) {
        if ((fdst = open(dstpath, O_WRONLY , 0600)) >= 0) {
            char* src=(char*)malloc(10240);
            long ret;
            lseek(fsrc,*pattachpos-1,SEEK_SET);
            do {
                ret = read(fsrc, src, 10240);
                if (ret <= 0)
                break;
            } while (write(fdst, src, ret) > 0);
            close(fdst);
            free(src);
        }
        close(fsrc);
    }
}
