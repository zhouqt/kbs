#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif
f_touch(char* filename)
{
    int fd;
    if ((fd=open(filename, O_WRONLY|O_NONBLOCK|O_CREAT|O_NOCTTY|O_LARGEFILE, 0666))!=-1)
    {
	close(fd);
	utime(filename,0);
	return 0;
    }
    return -1;
}
