#include <sys/types.h>
#include <sys/wait.h>
#include "sys/socket.h"
#include "netinet/in.h"
#include <signal.h>


#include "bbs.h"

void do_exit()
{
   flush_ucache();
}

void do_exit_sig(int sig)
{
    exit(0);
}

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
       log("1user","kill user %s",theuser->userid); 
       a = getuser(theuser->userid,&ft);
       setmailpath(tmpbuf,theuser->userid);
       sprintf(genbuf1,"/bin/rm -rf %s",tmpbuf);
       system(genbuf1) ;
       sethomepath(tmpbuf, theuser->userid);
       sprintf(genbuf1,"/bin/rm -rf %s",tmpbuf);
       system(genbuf1) ;
       sprintf(genbuf1,"/bin/rm -fr tmp/email/%s", theuser->userid) ;
       system(genbuf1) ; 
       setuserid2(a,"");
       theuser->userlevel=0;
       strcpy(theuser->address, "");
       strcpy(theuser->username, "");
       strcpy(theuser->realname, "");
     } 
     
     return 0;
}
int dokilluser()
{
/*    if (load_ucache()!=0) return -1;*/
    log("1user","Started kill users\n");
    apply_users(killauser,NULL);
    log("1user","kill users done\n");
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

static char* username;
static char* cmd;
static int num;
int getrequest(int m_socket)
{
        int len;
        struct sockaddr_in sin;
        int s;
	char* pnum;
        len = sizeof(sin);

        for (s = accept(m_socket,&sin,&len);;s = accept(m_socket,&sin,&len)) {
                if ((s<=0)&&errno!=EINTR){
    	            log("3system","userd:accept %s",strerror(errno));
                    exit(-1);
                }
                if (s<=0) continue;
                memset(tmpbuf,0,255);
                len = read(s,tmpbuf,255);
                if (len<=0) {close (s) ;continue;}
                strtok(tmpbuf," ");
		cmd=tmpbuf;
                username = strtok(NULL," ");
                pnum = strtok(NULL," ");
		num = atoi(pnum);
                if (strcmp(tmpbuf,"QUIT")==0) exit(0);
                if (strcmp(tmpbuf,"NEW") == 0) break;
                if (strcmp(tmpbuf,"SET") == 0) break;
                if (strcmp(tmpbuf,"DEL") == 0) {
                    pnum=username;
		    num = atoi(pnum);
                    break;
                }
                close(s);
        }
        return s;
}

void putrequest(int sock,int id)
{
        write(sock,&id,sizeof(id));
        close(sock);
}

void userd()
{
    int m_socket;

    struct sockaddr_in sin;
    int sinlen = sizeof(sin);
    int opt=1;
    if (( m_socket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
    	log("3system","userd:socket %s",strerror(errno));
    	exit(-1);
    }
    setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,&opt,4);
    memset(&sin,0,sinlen);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(60001);
    inet_aton("127.0.0.1",&sin.sin_addr);
    if  (0!=bind(m_socket,(struct sockaddr *)&sin,sizeof(sin))) {
    	log("3system","userd:bind %s",strerror(errno));
    	exit(-1);
    }
    if (0!=listen(m_socket,5)) {
    	log("3system","userd:listen %s",strerror(errno));
    	exit(-1);
    }
    while (1) {
        int sock,id;
     
        sock = getrequest(m_socket);
	if (!strcmp(cmd,"NEW"))
		id = getnewuserid(username);
	if (!strcmp(cmd,"SET")) {
		setuserid2(num,username);
		id = 0;
        }
	if (!strcmp(cmd,"DEL")) {
		setuserid2(num,"");
		id = 0;
        }
        putrequest(sock,id);
    }
    return;
}

void flushd()
{
    struct sigaction act;

    atexit(do_exit);
    bzero(&act,sizeof(act));
    act.sa_handler = do_exit_sig;
    sigaction(SIGTERM,&act,NULL);
    sigaction(SIGHUP,&act,NULL);
    sigaction(SIGABRT,&act,NULL);

    while (1) {
        sleep(2*60*60);
      	 flush_ucache();
        log("4miscdaemon","flush passwd file");
    };
}

static void
reaper()
{
    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}


int dodaemon(char* argv1,char* daemon)
{
    struct sigaction act;
    
    if (load_ucache()!=0) {
           printf("ft,load ucache error!");
           exit(-1);
    }

     switch (fork()) {
         case -1: printf("faint, i can't fork.\n"); exit(0); break;
         case 0: break;
         default : exit(0); break;
     }
     setsid();
#ifdef AIX
     setpgrp();
#else
     // by zixia setpgrp(0, 0);
     setpgrp();
#endif
#ifdef AIX
    act.sa_handler = NULL;
    act.sa_flags = SA_RESTART|SA_NOCLDWAIT;
    sigaction(SIGCHLD, &act, NULL);
#else
    act.sa_handler = reaper;
    act.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &act, NULL);
#endif

    if (((daemon==NULL)||(!strcmp(daemon,"killd")))&&fork()) {
     strcpy(argv1,"killd");
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
     };
    }
    if (((daemon==NULL)||(!strcmp(daemon,"userd")))&&fork()) {
          strcpy(argv1,"userd");
          userd();
    }
    if ((daemon==NULL)||(!strcmp(daemon,"flushd"))) {
          strcpy(argv1,"flushd");
          flushd();
    }
}
int main (int argc,char *argv[])
{
     chdir(BBSHOME);
     setuid(BBSUID);
     setgid(BBSGID);
     setreuid(BBSUID,BBSUID);
     setregid(BBSGID,BBSGID);

     if (argc>1) {
         if (strcasecmp(argv[1],"killuser") == 0)  return dokilluser();
         if (strcasecmp(argv[1],"allboards") == 0) return dokillalldir();
         if (strcasecmp(argv[1],"daemon") == 0) return dodaemon(argv[1],argv[2]);
         return dokilldir(argv[1]);
     }
     printf("Usage : %s killuser to kill old users\n",argv[0]);
     printf("        %s allboards to delete all old files\n",argv[0]);
     printf("        %s daemon to run as a daemon\n",argv[0]);
     printf("        %s BOARDNAME to delete old file in BOARDNAME\n",argv[0]);
     printf("That's all, folks\n");

     return 0;
}       
