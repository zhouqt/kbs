#include "bbs.h"

struct oldfriend {
    char id[13];
    char exp[40];
};

int newfd;
int conver(struct oldfriend* fr,int ent,void* arg)
{
    struct friends newf;
    memcpy(&newf,fr,sizeof(newf));
    write(newfd,&newf,sizeof(newf));
    return 0;
}

int refriend(struct userec* user,char* arg)
{
   char buf[256],buf2[256];
   char cmd[1024];
   sethomefile(buf,user->userid,"friends");
   sethomefile(buf2,user->userid,"friends.sav");
   f_cp(buf,buf2,O_TRUNC);
   sethomefile(buf2,user->userid,"newfriends");
   newfd=open(buf2,O_WRONLY|O_TRUNC|O_CREAT,0644);
   apply_record(buf,(APPLY_FUNC_ARG)conver,sizeof(struct oldfriend),NULL,0,false);
   close(newfd);
   f_rm(buf);
   f_mv(buf2,buf);
}

main()
{
   struct userec ur;
   chdir(BBSHOME);
   setuid(BBSUID);
   setgid(BBSGID);
   resolve_ucache();
   strcpy(ur.userid,"KCN");
//   refriend(&ur,NULL);
   apply_users(refriend,NULL);
}
