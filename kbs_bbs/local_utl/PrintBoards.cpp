#include <stdio.h>
#include <unistd.h>
#include "RecordFile.h"
#include "bbs.h"

#ifndef AIX
#define CONV_LONG(x) ( ((x)>>24) | (((x)&0x00ff0000)>>8) | (((x)&0x0000ff00)<<8) | (((x)&0x000000ff)<<24) )
#else
#define CONV_LONG(x) (x)
#endif
int printboard(struct boardheader* bh)
{
  if (bh->filename[0]) {
    unsigned long level = CONV_LONG(bh->level);
    if ((level&PERM_POSTMASK)||(level==0))
      printf("%s\n",bh->filename);
  }
  return 0;
}

main(int argc,char** argv)
{
  char* filename;
  if (argc<=1) filename=".BOARDS";
  else filename=argv[1];

  CRecordFile file(filename);

  if (file.ApplyRecord(sizeof(struct boardheader),(APPLY_FUNC)printboard)==-1)
	perror("open boards");
}
