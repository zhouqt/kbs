#include <sys/stat.h>
#include <dirent.h>
#include "system.h"


static int rm_dir();


int
f_rm(fpath)
  char *fpath;
{
  struct stat st;

  if (stat(fpath, &st))
    return -1;

  if (!S_ISDIR(st.st_mode))
    return unlink(fpath);

  return rm_dir(fpath);
}


static int
rm_dir(fpath)
  char *fpath;
{
  struct stat st;
  DIR *dirp;
  struct dirent *de;
  char buf[256], *fname;

  if (!(dirp = opendir(fpath)))
    return -1;

  for (fname = buf; *fname = *fpath; fname++, fpath++)
    ;

  *fname++ = '/';

  readdir(dirp);
  readdir(dirp);

  while (de = readdir(dirp))
  {
    fpath = de->d_name;
    if (*fpath)
    {
      strcpy(fname, fpath);
      if (!stat(buf, &st))
      {
	if (S_ISDIR(st.st_mode))
	  rm_dir(buf);
	else
	  unlink(buf);
      }
    }
  }
  closedir(dirp);

  *--fname = '\0';
  return rmdir(buf);
}
