#include <sys/types.h>
#include <sys/wait.h>

#include "bbs.h"

static char genbuf1[255];
int killdir(char *basedir,char *filename)
{
    int fd ;
    struct stat st;
    struct fileheader *files,*afile;
    int i;
    int now = (time(NULL) / 86400) % 100;
    int count=0;
    int deleted = 0;
    strcpy(genbuf1,basedir);
    strcat(genbuf1,"/");
    strcat(genbuf1,filename);
    fd = open(genbuf1,O_RDWR);
    if (fd < 0) return 0;
    if (flock(fd,LOCK_EX) < 0) { close(fd); return 0; }
    if (fstat(fd,&st)<0) {flock(fd,LOCK_UN) ; close(fd); return 0; }
    if ((files = (struct fileheader *)malloc(st.st_size)) == NULL) {
        flock(fd,LOCK_UN) ; close (fd); return 0; 
    }
    if (read(fd,files,st.st_size) <0) {
        free(files);
        flock(fd,LOCK_UN) ; close (fd); return 0; 
    }
    lseek(fd,0,0);
    for (i=0,afile = files;i<st.st_size/sizeof(struct fileheader);i++,afile++) {
        if (now - afile->accessed[11] > DAY_DELETED_CLEAN ||(now<afile->accessed[11] && now+100 - afile->accessed[11] > DAY_DELETED_CLEAN ) ) {
                strcpy(genbuf1,basedir);
                strcat(genbuf1,"/");
                strcat(genbuf1,afile->filename);
                unlink(genbuf1);
                deleted++;
        }else{
                write(fd,afile,sizeof(struct fileheader));
                count += sizeof(struct fileheader);
        }               
    }
    ftruncate(fd,count);
    flock(fd,LOCK_EX);
    free(files);
    close(fd);
    return deleted;
}

int dokilldir(char *board)
{
     char hehe[255];
     int killed;
     if (!board[0]) return 0;
     strcpy(hehe,"boards");
     strcat(hehe,"/");
     strcat(hehe,board);
     killed = killdir(hehe,".DELETED") + killdir(hehe,".JUNK");
     log("0miscdaemon","deleted %d files in %s board",killed,board);
     return killed;
}

int doaboard(struct boardheader *brd)
{
     if (!brd) return 0;
     return dokilldir(brd->filename);
}
int dokillalldir ()
{
     resolve_boards();
     apply_boards(doaboard);
}
static char tmpbuf[255];
static char genbuf1[255];
char *
setmailpath( buf, userid )  /* 取 某用户 的mail */
char    *buf, *userid;
{
    if (isalpha(userid[0]))  /* 加入错误判断,提高容错性, alex 1997.1.6*/
        sprintf( buf, "mail/%c/%s", toupper(userid[0]), userid );
    else
        sprintf( buf, "mail/wrong/%s", userid);
    return buf;
}
char *
sethomepath( buf, userid )  /* 取 某用户 的home */
char    *buf, *userid;
{
    if (isalpha(userid[0]))  /* 加入错误判断,提高容错性, alex 1997.1.6*/
        sprintf( buf, "home/%c/%s", toupper(userid[0]), userid );
    else
        sprintf( buf, "home/wrong/%s", userid);
    return buf;
}
int killauser(struct userec *theuser,char *data)
{
       int a;
       struct userec *ft;
       if (!theuser || theuser->userid[0]==0) return 0;
       a = compute_user_value(theuser);
       if (a<0) {
       log("0miscdaemon","kill user %s",theuser->userid); 
       a = getuser(theuser->userid,&ft);
       setmailpath(tmpbuf,theuser->userid);
       sprintf(genbuf1,"/bin/mv %s mail/mailbak",tmpbuf);
       system(genbuf1) ;
       sethomepath(tmpbuf, theuser->userid);
       sprintf(genbuf1,"/bin/mv %s home/homebak",tmpbuf);
       system(genbuf1) ;
       sprintf(genbuf1,"/bin/rm -fr tmp/email/%s", theuser->userid) ;
       system(genbuf1) ; 
       setuserid(a,"");
       theuser->userlevel=0;
       strcpy(theuser->address, "");
       strcpy(theuser->username, "");
       strcpy(theuser->realname, "");
     } 
}
int dokilluser()
{
    resolve_ucache();
    log("0miscdaemon","Started kill users\n");
    apply_users(killauser,NULL);
    log("0miscdaemon","kill users done\n");
}
int getnextday4am()
{
     time_t now = time( 0 );
     struct tm *tm = localtime(&now);
     if (tm -> tm_hour >= 4) {
         now += 86400;
         tm = localtime(&now);
     } 
     tm -> tm_hour = 4;
     tm -> tm_sec = 0;
     tm -> tm_min = 4;
     return mktime(tm);
}
int ismonday()
{
     time_t now = time( 0 );
     struct tm *tm = localtime(&now);
     return tm -> tm_wday == 1;
}

static void
reaper()
{
    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}


int dodaemon()
{
    struct sigaction act;

     switch (fork()) {
         case -1: printf("faint, i can't fork.\n"); exit(0); break;
         case 0: break;
         default : exit(0); break;
     }
     setsid();
     setpgrp();
#ifdef AIX
    act.sa_handler = NULL;
    act.sa_flags = SA_RESTART|SA_NOCLDWAIT;
    sigaction(SIGCHLD, &act, NULL);
#else
    act.sa_handler = reaper;
    act.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &act, NULL);
#endif

     while (1) {
     	sleep(getnextday4am() - time( 0 ));
    	 switch(fork()) {
       	     case -1: 
       	        log("0miscdaemon","fork failed\n");
       	        break;
       	     case 0 : 
       	        dokilluser();
                exit(0);
       	        break;
       	     default:
       	        break;   
    	 }
    	 if (ismonday()) {
	    switch(fork()) {
       		case -1: 
       	      	   log("0miscdaemon","fork failed\n");
       	           break;
       	    	case 0 : 
       	       	   dokillalldir();
                   exit(0);
       	       	   break;
       	     	default:
       	           break;   
    	    } 
    	 }
     }
}
int main (int argc,char *argv[])
{
     chdir(BBSHOME);
     if (argc>1) {
         if (strcasecmp(argv[1],"killuser") == 0)  return dokilluser();
         if (strcasecmp(argv[1],"allboards") == 0) return dokillalldir();
         if (strcasecmp(argv[1],"daemon") == 0) return dodaemon();
         return dokilldir(argv[1]);
     }
     printf("Usage : %s killuser to kill old users\n",argv[0]);
     printf("        %s allboards to delete all old files\n",argv[0]);
     printf("        %s daemon to run as a daemon\n",argv[0]);
     printf("        %s BOARDNAME to delete old file in BOARDNAME\n",argv[0]);
     printf("That's all, folks\n");
}       
