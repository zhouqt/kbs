#include "bbs.h"


int settop(struct boardheader* bh,void* arg)
{
#ifdef DEBUG_NEW_READ
  char buf[MAXPATH];
  int i;
  setbdir(DIR_MODE_ZHIDING,buf,bh->filename);
   i=get_num_records(buf, sizeof(struct fileheader));
   board_update_toptitle(bh,i);
#endif
//   printf("%s %d\n",bh->filename,i);
}

main()
{
  chdir(BBSHOME);
  resolve_boards();
  apply_boards(settop,NULL);
}

