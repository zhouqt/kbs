#include <fcntl.h>
#include "system.h"

#define BLK_SIZ 10240

int
f_cp(src, dst, mode)
  char *src, *dst;
  int mode;			/* O_EXCL / O_APPEND / O_TRUNC */
{
  int fsrc, fdst, ret;

  ret = 0;

  if ((fsrc = open(src, O_RDONLY)) >= 0)
  {
    ret = -1;

    if ((fdst = open(dst, O_WRONLY | O_CREAT | mode, 0600)) >= 0)
    {
      char* pool=(char*)malloc(BLK_SIZ);
      src = pool;
      do
      {
	ret = read(fsrc, src, BLK_SIZ);
	if (ret <= 0)
	  break;
      } while (write(fdst, src, ret) > 0);
      close(fdst);
      free(pool);
    }
    close(fsrc);
  }
  return ret;
}
