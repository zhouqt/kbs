#include "bbs.h"

char* curuserid;
int
chk_BM_instr(BMstr,bmname)
char BMstr[STRLEN-1];
char    bmname[IDLEN+2];
{
    char *ptr;
    char BMstrbuf[STRLEN-1];

    strcpy(BMstrbuf,BMstr);
    ptr=strtok(BMstrbuf,",: ;|&()\0\n");
    while(1)
    {
        if(ptr==NULL)
            return NA;
        if(!strcmp(ptr,bmname/*,strlen(currentuser->userid)*/))
            return YEA;
        ptr=strtok(NULL,",: ;|&()\0\n");
    }
}

int check_BM(struct boardheader* bptr)
{
  if ((bptr->level!=0)&&!(bptr->level & PERM_POSTMASK)) return 0; 
  if (chk_BM_instr(bptr->BM,curuserid) == YEA) printf("%s(%s) ",bptr->filename,bptr->title+12);
  return 0;
}

int query_BM(struct userec* user,char* arg)
{
  if (!(user->userlevel& PERM_BOARDS)) return 0;
  printf("%s: ",user->userid);
  curuserid=user->userid;
  apply_boards(check_BM);
  printf("\n");
}

main()
{
  chdir(BBSHOME);
  resolve_ucache();
  resolve_boards();
  apply_users(query_BM,NULL);
}
