/*-------------------------------------------------------*/
/* util/buildir.c       ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : Maple/Phoenix/Secret_Lover .DIR ­««Ø         */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "bbs.h"


int alphasort();


int
dirselect(dir)
  struct direct *dir;
{
  register char *name = dir->d_name;
  return (name[0] == 'M' && name[1] == '.');
}


main(argc, argv)
  int argc;
  char **argv;
{
  int fdir, filetime, i, j, wrong;
  struct stat st;
  char genbuf[512], path[256], *ptr, *name;
  FILE *fp;
  int total, count;
  fileheader fhdr;
  struct direct **dirlist;
  struct tm *ptime;

  if (argc != 2)
  {
    printf("Usage:\t%s <path>\n", argv[0]);
    exit(-1);
  }

  ptr = argv[1];
  sprintf(path, "%s/.DIR", ptr);
  fdir = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fdir == -1)
  {
    printf(".DIR create error\n");
    exit(-1);
  }

  total = scandir(ptr, &dirlist, dirselect, alphasort);
  ptr = strrchr(path, '.');

  for (count = 0; count < total; count++)
  {
    name = dirlist[count]->d_name;
    strcpy(ptr, name);
    wrong = 0;
    if (!stat(path, &st))
    {
      wrong = 1;

      if (st.st_size && (fp = fopen(path, "r")))
      {
        fgets(genbuf, 256, fp);
        if (!strncmp(genbuf, "