/*
 * Constructed by flyriver from SMTH's original source codes,
 * for integrating with NJUWWWBBS-0.9.
 * Built and run on a X86 machine running FreeBSD-4.4 RELEASE.
 *
 * For building a clean and general library, a lot of global variables
 * had been removed, all calls to terminal I/O functions had been gotten rid
 * of, and some function arguments list had been modified
 * to match the new library rules.
 *
 *                                    flyriver
 *                                    2001.12.16
 * vim: ts=4, sw=4
*/

#include "bbs.h"
#include "types.h"
#include "smthlib.h"
#include "bbslib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define BBS_PAGESIZE    (20) /* 19->20, modified by dong, 1999.1.26 */
#define refreshtime     (30)
extern time_t   login_start_time;
extern char     BoardName[];

int   (*func_list_show)();
time_t update_time=0;
int showexplain=0,freshmode=0;
int mailmode,numf;
int friendmode=0;
int usercounter,real_user_names=0;
int range,page,readplan,num;

struct user_info *user_record[USHM_SIZE];
struct userec *user_data;
extern char MsgDesUid[14]; /* 保存所发msg的目的uid 1998.7.5 by dong */

char seccode[SECNUM][5]={
	"0", "1", "3", "4", "5", "6", "7", "8", "9"
};

char secname[SECNUM][2][20]={
	"BBS 系统", "[站内]",
	"清华大学", "[本校]",
	"电脑技术", "[电脑/系统]",
	"休闲娱乐", "[休闲/音乐]",
	"文化人文", "[文化/人文]",
	"社会信息", "[社会/信息]",
	"学术科学", "[学科/语言]",
	"体育健身", "[运动/健身]",
	"知性感性", "[谈天/感性]",
};

int loginok=0;
friends_t bbb[MAXREJECTS];
int badnum=0;

struct userec currentuser;
struct user_info *u_info;
struct UTMPFILE *shm_utmp;
struct BCACHE *shm_bcache;
struct UCACHE *shm_ucache;
char fromhost[256];
char parm_name[256][80], *parm_val[256];
int parm_num=0;

friends_t fff[200];

int friendnum=0;

/* from ucache.c */
int myfriend(int uid,char* fexp)
{
    extern int  nf;
    int i,found=NA;
    int cmp;
    /*char buf[IDLEN+3];*/

    if(nf<=0)
    {
        return NA;
    }
    for (i=0;i<nf;i++) {
    	if (topfriend[i].uid==uid) {
    		found=YEA;
    		break;
    	}
    }
    if((found)&&fexp)
        strcpy(fexp,topfriend[i].exp);
    return found;
}

void swap_user_record(a,b)
int a,b;
{
    struct user_info *c;

    c=user_record[a];
    user_record[a]=user_record[b];
    user_record[b]=c;
}

void
sort_user_record(left,right)
int left, right;
{
    int i,last;

    if(left>=right)
        return;
    swap_user_record(left,(left+right)/2);
    last=left;
    for(i=left+1;i<=right;i++)
        if(strcasecmp(user_record[i]->userid,user_record[left]->userid)<0)
            swap_user_record(++last,i);
    swap_user_record(left,last);
    sort_user_record(left,last-1);
    sort_user_record(last+1,right);
}

int full_utmp(struct user_info* uentp,int* count)
{
    if( !uentp->active || !uentp->pid )
    {
        return 0;
    }
    if(!has_perm(PERM_SEECLOAK) && uentp->invisible 
		&& strcmp(uentp->userid,getcurruserid()))/*Haohmaru.99.4.24.让隐身者能看见自己*/
    {
        return 0;
    }
    if(friendmode&&!myfriend(uentp->uid,NULL))
    {
        return 0;
    }
    user_record[*count]=uentp;
    (*count)++;
    return COUNT;
}

int
fill_userlist()
{
    static int i,i2;

    i2=0;
    if(!friendmode)
    {
	    apply_ulist_addr((APPLY_UTMP_FUNC)full_utmp,(char*)&i2);
    }
	else
	{
    	for (i=0;i<nf;i++)
		{
		if (topfriend[i].uid)
			apply_utmpuid((APPLY_UTMP_FUNC)full_utmp,topfriend[i].uid,(char*)&i2);
    	}
    }
    range=i2;
    return i2==0?-1:1;
}

int
countusers(struct userec *uentp ,char* arg)
{
    char permstr[10];

    if(uentp->numlogins != 0&&uleveltochar( permstr, uentp ) != 0)
		return COUNT;
    return 0;
}

int
allusers()
{
	int count;
    if((count=apply_users(countusers,0)) <= 0) {
        return 0;
    }
    return count;
}

static inline int ucache_lock()
{
    int lockfd;
    lockfd = open( ULIST, O_RDWR|O_CREAT, 0600 );
    if( lockfd < 0 ) {
        log( "3system", "CACHE:lock ucache:%s", strerror(errno) );
        return -1;
    }
    flock(lockfd,LOCK_EX);
    return lockfd;
}

static inline int ucache_unlock(int fd)
{
    flock(fd,LOCK_UN);
    close(fd);
}


/*
   CaseInsensitive ucache_hash, assume 
   isalpha(userid[0]) 
   isahpha(userid[n]) || isnumber(userid[n]) n>0
   01/5/5 wwj
 */
unsigned int ucache_hash(const char* userid)
{
    int n1,n2,n,len;
    ucache_hashtable * hash;
    ucache_hashtable * usage;

    if(!*userid)return 0;
    hash=&uidshm->hashtable;
    usage=&uidshm->hashusage;

    n1=*userid++;
    if(n1>='a' && n1<='z')n1&=0xdf;
    n1-='A';
    if(n1<0 || n1>=26)return 0;

    n1=hash->hash0[n1];

    while(n1<0){
        n1=-n1-1;
        if(!*userid){
            n1=hash->hash[n1][0];
        } else {
            n2=*userid++;
            if(n2>='a' && n2<='z'){
               n2-='a'-10;
            } else if(n2>='A' && n2<='Z'){
               n2-='A'-10;
            } else {
               n2-='0';
            }
            if(n2<0 || n2>=36)return 0;
            n1=hash->hash[n1][n2];
        }
    }
    n1=(n1*UCACHE_HASHBSIZE)%UCACHE_HASHSIZE+1;
    if(!*userid)return n1;

    n2=0;
    len=strlen(userid);
    while(*userid){
        n=*userid++;
        if(n>='a' && n<='z'){
           n-=32;
        }
        n2+=(n-47)*len;
        len--;
    }
    n1=(n1+n2%UCACHE_HASHBSIZE)%UCACHE_HASHSIZE+1;
    return n1;
}

void
resolve_ucache()
{
    int iscreate;

    iscreate = 0;
    if( uidshm == NULL ) {
        uidshm = (struct UCACHE*)attach_shm1( "UCACHE_SHMKEY", 3696, sizeof( *uidshm ) ,&iscreate , 1, NULL); 
            /*attach to user shm,readonly */
        if (iscreate) { /* shouldn't load passwd file in this place */
        	log("4system","passwd daemon havn't startup");
        	exit(-1);
        }
    }

    if (passwd==NULL) { 
        passwd = (struct userec*)attach_shm( "PASSWDCACHE_SHMKEY", 3697, MAXUSERS*sizeof(struct userec) ,&iscreate); /*attach to user shm */
        if (iscreate) { /* shouldn't load passwd file in this place */
        	log("4system","passwd daemon havn't startup");
        	exit(-1);
        }
    }        
}

/*---	period	2000-10-20	---*/
int getuserid(char * userid, int uid)
{
    if( uid > uidshm->number || uid <= 0 ) return 0;
    strncpy(userid,(char*)passwd[uid-1].userid, IDLEN+1);
    userid[IDLEN]=0;
    return uid;
}

void
setuserid_internal( int num, const char *userid) /* 设置user num的id为user id*/
{
    if( num > 0 && num <= MAXUSERS )
	{
    	int oldkey,newkey,find;
        if( num > uidshm->number )
            uidshm->number = num;
        oldkey=ucache_hash((char*)passwd[ num - 1 ].userid);
        newkey=ucache_hash(userid);
		find=uidshm->hashhead[oldkey];

		if (find==num)
			uidshm->hashhead[oldkey]=uidshm->next[find-1];
		else
		{
			/* find and remove the hash node */
			int i=0;
			while (uidshm->next[find-1]&&uidshm->next[find-1]!=num)
			{
				find=uidshm->next[find-1];
				i++;
				if (i>MAXUSERS)
				{
					log("3system",
						"UCACHE:uhash loop???! find %d,%s",num,userid);
					exit(0);
				}
			}
			if (!uidshm->next[find-1])
			{
				if (oldkey!=0)
					log("3system",
						"UCACHE:can't find %s in hash table",
						passwd[ num - 1 ].userid);
			}
			else uidshm->next[find-1] = uidshm->next[num-1];
		}

		uidshm->next[num-1]=uidshm->hashhead[newkey];
		uidshm->hashhead[newkey]=num;
        strncpy( passwd[ num - 1 ].userid, userid, IDLEN+1 );
    }
}

void setuserid( int num,const char * userid)
{
    int lockfd;
    lockfd=ucache_lock();
    setuserid_internal(num,userid);
    ucache_unlock(lockfd);
}

int
searchnewuser() /* 找cache中 空闲的 user num */
{
    register int num, i;
    if (uidshm->hashhead[0]) return uidshm->hashhead[0];
    if (uidshm->number<MAXUSERS) return uidshm->number+1;
    return 0;
}

int
searchuser(const char *userid )
{
    register int i ;

	i = uidshm->hashhead[ucache_hash(userid)];
	while (i)
	{
		if (!strcasecmp(userid,passwd[ i - 1 ].userid))
			return i;
		else
			i=uidshm->next[i-1];
	}
    return 0 ;
}

int getuser(const char *userid, struct userec** user) /* 取用户信息 */
{
    int uid = searchuser(userid) ;

    if(uid == 0) return 0 ;
    if (user)
	    *user=&passwd[uid-1];
    return uid ;
}

char* getuserid2(int uid)
{
    if( uid > uidshm->number || uid <= 0 ) return NULL;
    return passwd[uid-1].userid;
}

int getnewuserid2(char * userid)
{
	int result = getnewuserid3(userid);
#ifdef BBSMAIN
	log( "1system", "APPLY: uid %d from %s", result, fromhost );
#endif
	if (result>=0) return result;
	return -1;
}

int getnewuserid3(char * userid)
{
	int m_socket;
	char cmdbuf[255];
	struct sockaddr_in sin;
	fd_set rfds;
	int result;
	struct  timeval tv;

	bzero(&sin, sizeof(sin));
	m_socket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (m_socket<0) return -1;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(60001);
	inet_aton("127.0.0.1",&sin.sin_addr);
	if (connect(m_socket,(struct sockaddr*)&sin,sizeof(sin))!=0)
	{
		close(m_socket);
		return -1;
	}
	sprintf(cmdbuf,"NEW %s",userid);
	write(m_socket,cmdbuf,strlen(cmdbuf));
	FD_ZERO(&rfds);
	FD_SET(m_socket,&rfds);
	tv.tv_sec=5;
	tv.tv_usec=0;
	result = select(m_socket+1,&rfds,NULL,NULL,&tv);
	if (result)
	{
		int len=read(m_socket,&result,sizeof(result));
		close(m_socket);
		if (len!=sizeof(result)) return -1;
		return result;
	}
	close(m_socket);
	return -1;
}

int getnewuserid(char* userid)
{
    struct userec utmp, zerorec;
    struct stat st;
    int         fd, size, val, i;
    char    tmpstr[30];
    time_t system_time;

    system_time = time( NULL );
    fd=ucache_lock();

    i = searchnewuser();
#ifdef BBSMAIN
    log( "1system", "APPLY: uid %d from %s", i, fromhost);
#endif

    if( i <= 0 || i > MAXUSERS )
	{
        ucache_unlock(fd);
        return -1;
    }
    memset( &utmp, 0, sizeof( utmp ) );
    strcpy( utmp.userid, userid );
    utmp.lastlogin = time( NULL );
    setuserid_internal( i, userid ); /* added by dong, 1998.12.2 */
    update_user(&utmp,i,0);
    ucache_unlock(fd);
    return i;
}

int update_user(struct userec* user,int num,int all)
{
	struct userec tmpuser;

	if (!all)
	{
		if (strncasecmp(user->userid,passwd[num-1].userid,IDLEN)) 
			return -1;
		tmpuser=*user;
		memcpy(tmpuser.userid,passwd[num-1].userid,IDLEN+2);
		memcpy(tmpuser.passwd,passwd[num-1].passwd,IDLEN+2);
		memcpy(tmpuser.md5passwd,passwd[num-1].md5passwd,IDLEN+2);
	}
	else
	{
		tmpuser=*user;
		memcpy(tmpuser.userid,passwd[num-1].userid,IDLEN+2);
	}
	memcpy(&passwd[num-1],&tmpuser,sizeof(struct userec));
	return 0;
}

int apply_users(int (*fptr)(struct userec* ,char*),char* arg)
{
	int i;
	int count;
	count=0;

	for (i=0;i<uidshm->number;i++)
	{
		if (fptr)
		{
			int ret;
			ret = (*fptr)(&passwd[i],arg);
			if (ret==QUIT) break;
			if (ret==COUNT) count++;
		}
		else
			count++;
	}
	return count;
}

/* from log.c */
/*

  优先级
  
  0   message
  1   notice
  2   warning
  3   error
  4   cirtical error
*/

typedef struct _logconfig{
    int  compare;  /* -1 小于等于, 0 相等,  1 大于等于 */
    int  prio;     /* 优先级 */
    char * name;   /* 确定的来源，NULL是全匹配 */
    char * file;   /* 文件名，相对于 BBSHOME/reclog ，如果是NULL，丢弃 */
    int  bufsize;  /* 缓存大小，如果是 0，不缓存 */
    int  searchnext; /* 如果这个匹配了，还查找后面的config吗？*/
    
    /* 运行时参数 */
    int  bufptr;   /* 使用缓存位置 */
    char * buf;    /* 缓存 */
    int  fd;       /* 文件句柄 */
}logconfig;

static logconfig logconf[]= {
    {1, 3, NULL,"error.log", 0 , 1, 0, NULL, 0},   /* 错误记录 */
    {1, 0, "connect","connect.log", 0, 0, 0, NULL, 0},     /* 连接记录 */
    {1, 0, "user","user.log", 0, 0, 0, NULL, 0},/*所有用户的使用记录，使用缓冲*/
    {1, 0, "usermsg","msg.log", 0 , 0, 0, NULL, 0},/* 所有用户的message记录 */
    {1, 0, "boardusage", "boardusage.log",0,0, 0, NULL, 0}, /* 版面使用记录 */
    {1, 0, "chatd","trace.chatd",1024, 0, 0, NULL, 0},  /* 聊天室使用记录，使用缓冲 */
    {1, 0, "system","usies", 0 , 0, 0, NULL, 0},   /* 最后所有的记录都在这里 */
    {1, 0, NULL,"trace", 0 , 0, 0, NULL, 0}   /* 最后所有的记录都在这里 */
};

static int bdoatexit=0;


static void getheader(char * header,const char *from,int prio)
{
    struct tm *pt;
    time_t tt;
    time(&tt);
    pt=localtime(&tt);
     
    sprintf(header,"[%02u/%02u %02u:%02u:%02u %5d %d.%s] %s ",
        pt->tm_mon+1,pt->tm_mday,pt->tm_hour,pt->tm_min,pt->tm_sec,getpid(),
        prio,from,
        getcurruserid()
        );
}

/* 写入log, 如果buf==NULL那么flush。否则根据大小决定是否缓存 */
static void writelog(logconfig * pconf,const char * from,int prio, const char * buf)
{
    char header[64];
    
    if(!from)from=pconf->name;
    if(!from)from="unknown";
    
    getheader(header,from,prio);
    
    if(buf && pconf->buf){
        if((int)(pconf->bufptr+strlen(header)+strlen(buf)+2)<=pconf->bufsize){
             strcpy(&pconf->buf[pconf->bufptr],header);
             pconf->bufptr+=strlen(header);
             strcpy(&pconf->buf[pconf->bufptr],buf);
             pconf->bufptr+=strlen(buf);
             strcpy(&pconf->buf[pconf->bufptr],"\r\n");
             pconf->bufptr+=2;
             return;
        }
    }
    
    flock(pconf->fd,LOCK_SH);
    lseek(pconf->fd,0,SEEK_END);
	                
    if(pconf->buf && pconf->bufptr){
        write(pconf->fd,pconf->buf,pconf->bufptr);
        pconf->bufptr=0;
    }
    if(buf){
        write(pconf->fd,header,strlen(header));
        write(pconf->fd,buf,strlen(buf));
        write(pconf->fd,"\r\n",2);
    }
    flock(pconf->fd,LOCK_UN);
}

static void logatexit()
{
    logconfig * pconf;
    pconf=logconf;
    
    while(pconf-logconf<(int)(sizeof(logconf)/sizeof(logconfig))){
        if(pconf->buf && pconf->bufptr) writelog(pconf,NULL,0,NULL);
        if(pconf->buf)free(pconf->buf);
        if(pconf->fd>0)close(pconf->fd);
        pconf++;   
    }
}

int log(const char * from,const char *fmt,...)
{
    logconfig * pconf;
    int  prio;
    char buf[512];
	va_list v;
    
    prio=0;
    
    if(from[0]>='0' && from[0]<='9'){
        prio=from[0]-'0';
        from++;
    }
    if(strlen(from)>16)return -3;
    if(!fmt || !*fmt)return 0;
    
	va_start( v,fmt );
    vsprintf(buf,fmt,v);
	
    
    pconf=logconf;
    
    while(pconf-logconf<(int)(sizeof(logconf)/sizeof(logconfig))){
        if( (pconf->compare== 1 && prio>=pconf->prio) ||
            (pconf->compare== 0 && prio==pconf->prio) ||
            (pconf->compare==-1 && prio<=pconf->prio) ){
            if(!pconf->name || !strcasecmp(pconf->name,from)) {
                if(pconf->fd<=0){  /* init it! */
                    if(!pconf->file)return 0; /* discard it */
                    pconf->fd=open(pconf->file,O_WRONLY);
	                if(pconf->fd<0)pconf->fd=creat(pconf->file,0644);
	                if(pconf->fd<0)return -1;
	                
	                if(pconf->bufsize){
	                    if(!bdoatexit){ atexit(logatexit); bdoatexit=1; }
	                    pconf->buf=(char *)malloc(pconf->bufsize);
	                    pconf->bufptr=0;
	                }
                }
                writelog(pconf,from,prio,buf);
                if(!pconf->searchnext)break;
            }
        }
        pconf++;
    }
        if (pconf->fd>=0 && !pconf->bufsize) {        
        close(pconf->fd);         
        pconf->fd=-1;             
    }                             

	return 0;
}

#include "md5.h"

void igenpass(const char *passwd,const char *userid,unsigned char md5passwd[])
{
    static const char passmagic[]="wwj&kcn4SMTHBBS MD5 p9w2d gen2rat8, //grin~~, 2001/5/7";
    MD5_CTX md5;
    MD5Init(&md5);
    
    /* update size > 128 */
    MD5Update(&md5,(unsigned char *)passmagic,strlen(passmagic));
    MD5Update(&md5,(unsigned char *)passwd,strlen(passwd));
    MD5Update(&md5,(unsigned char *)passmagic,strlen(passmagic));
    MD5Update(&md5,(unsigned char *)userid,strlen(userid));
    
    MD5Final(md5passwd,&md5);
}

int setpasswd(const char *passwd,struct userec *user)
{
    igenpass(passwd,user->userid,user->md5passwd);
    user->passwd[0]=0;
    return 1;
}

int checkpasswd2(const char * passwd,const struct userec *user)
{
    if(user->passwd[0]){
        return checkpasswd(user->passwd,passwd);
    } else {
        unsigned char md5passwd[MD5_DIGEST_LENGTH];
        igenpass(passwd,user->userid,md5passwd);
        return !(memcmp(md5passwd,user->md5passwd,MD5_DIGEST_LENGTH));
    }
}

int checkpasswd(const char * passwd,const char * test)
{
    char *crypt1(char *buf, char *salt);

    static char pwbuf[14] ;
    char *pw ;

    strncpy(pwbuf,test,14) ;
    pw = crypt1(pwbuf, (char *)passwd) ;
    return (!strcmp(pw, passwd)) ;
}

/* from bcache.c */

static int bcache_lock()
{
    int lockfd;
    lockfd = creat( "bcache.lock", 0600 );
    if( lockfd < 0 ) {
        log( "3system", "CACHE:lock bcache:%s", strerror(errno) );
        return -1;
    }
    flock(lockfd,LOCK_EX);
    return lockfd;
}

static int bcache_unlock(int fd)
{
    flock(fd,LOCK_UN);
    close(fd);
}

void
reload_boards()
{
}

void resolve_boards()
{
	int boardfd;
	struct stat st;
	int iscreate;
	
    if( brdshm == NULL ) {
        brdshm = attach_shm( "BCACHE_SHMKEY", 3693, sizeof( *brdshm ) ,&iscreate); /* attach board share memory*/
    }
    
	if ((boardfd=open(BOARDS,O_RDWR|O_CREAT,0644)) == -1) {
		log("3system","Can't open " BOARDS "file %s",strerror(errno));
       	exit(-1);
	}
   	bcache = (struct boardheader*) mmap(NULL,
   			MAXBOARD*sizeof(struct boardheader),
   			PROT_READ|PROT_WRITE,MAP_SHARED,boardfd,0);
   	if (bcache==(struct boardheader*)-1) {
		log("4system","Can't map " BOARDS "file %s",strerror(errno));
		close(boardfd);
       	exit(-1);
   	}
	if (iscreate) {
		int i;
		int fd;
		fd = bcache_lock();
		ftruncate(boardfd,MAXBOARD*sizeof(struct boardheader));
		for (i=0;i<MAXBOARD;i++)
			if (bcache[i].filename[0])
				brdshm->numboards=i+1;
		bcache_unlock(fd);
	}
   	close(boardfd);
}

int apply_boards(int (*func)()) /* 对所有版 应用 func函数*/
{
    register int i ;

    for(i=0;i<brdshm->numboards;i++)
        if( bcache[i].level & PERM_POSTMASK || 
#ifdef BBSMAIN 
             has_perm( bcache[i].level ) || 
#endif
             (bcache[i].level&PERM_NOZAP))
            if (bcache[i].filename[0])
            if((*func)(&bcache[i]) == QUIT)
                return QUIT;
    return 0;
}

int
getbnum( bname ) /* board name --> board No. */
char    *bname;
{
    register int i;

    for(i=0;i<brdshm->numboards;i++)
        if( bcache[i].level & PERM_POSTMASK || has_perm( bcache[i].level )|| (bcache[i].level&PERM_NOZAP))
            if(!strncasecmp( bname, bcache[i].filename, STRLEN ) )
                return i+1 ;
    return 0 ;
}

/*---	added by period		2000-11-07	to be used in postfile	---*/
/* getboardnum() 的返回值是 1 based 的 */
int getboardnum(char*  bname ,struct boardheader* bh) /* board name --> board No. & not check level */
{
    register int i;

    for(i=0;i<brdshm->numboards;i++)
        if(!strncasecmp( bname, bcache[i].filename, STRLEN ) ) {
        	if (bh)
        		*bh=bcache[i];
            return i+1 ;
        }
    return 0 ;
} /*---	---*/

/* corresponding to has_post_perm() in NJU */
int haspostperm(char *bname) /* 判断在 bname版 是否有post权 */
{
    register int i;

    if ((i = getbnum(bname)) == 0) return 0;
    if (has_perm(PERM_DENYPOST))
    {
		if (!strcmp(bname, "Complain"))
			return 1;/* added by stephen 2000.10.27*/
        else if(!strcmp(bname, "sysop"))
            return 1;
	} /* stephen 2000.10.27 */
    if (!has_perm(PERM_POST)) return 0;
    return (has_perm((bcache[i-1].level&~PERM_NOZAP) & ~PERM_POSTMASK));
}

int normal_board(char *bname)
{
    register int i;

    if( strcmp( bname, DEFAULTBOARD ) == 0 )  return 1;
    if ((i = getbnum(bname)) == 0) return 0;
    return (bcache[i-1].level==0);
}

bcache_t *getbcache(char* bname)
{
	int i;
	i = getbnum(bname);
	if (i==0) return NULL;
	return &bcache[i-1];
}

int get_boardcount()
{
	return brdshm->numboards;
}

struct boardheader const* getboard(int num)
{
	if (num>0&&num<=MAXBOARD) {
		return &bcache[num-1];
	}
	return NULL;
}

int set_board(int bid,struct boardheader* board)
{
    memcpy(&bcache[bid-1], board, sizeof(struct boardheader));
}

/* from bbs.c */

char *
sethomefile( buf, userid , filename) /* added by Luzi 1997.11.30 */
char    *buf, *userid, *filename;
{
    if (isalpha(userid[0]))
        sprintf( buf, "home/%c/%s/%s", toupper(userid[0]), userid ,filename);
    else sprintf( buf, "home/wrong/%s/%s", userid, filename);
    return buf;
}

char *
setuserfile( buf, filename )    /* 取当前用户文件 路径*/
char    *buf, *filename;
{
	struct userec *user = getcurrusr();

    if (isalpha(user->userid[0]))  /* 加入错误判断,提高容错性, alex 1997.1.6*/
        sprintf( buf, "home/%c/%s/%s", toupper(user->userid[0]), user->userid, filename );
    else
        sprintf( buf, "home/wrong/%s/%s", user->userid, filename);
    return buf;
}

char *
setmailfile( buf, userid, filename )    /* 取某用户mail文件 路径*/
char    *buf, *userid, *filename;
{
    if (isalpha(userid[0]))  /* 加入错误判断,提高容错性, alex 1997.1.6*/
        sprintf( buf, "mail/%c/%s/%s", toupper(userid[0]), userid, filename );
    else
        sprintf( buf, "mail/wrong/%s/%s", userid, filename);
    return buf;
}

char *
setbpath( buf, boardname )   /* 取某版 路径 */
char *buf, *boardname;
{
    strcpy( buf, "boards/" );
    strcat( buf, boardname );
    return buf;
}

char *
setbfile( buf, boardname, filename )  /* 取某版下文件 */
char *buf, *boardname, *filename;
{
    sprintf( buf, "boards/%s/%s", boardname, filename );
    return buf;
}

void record_exit_time()   /* 记录离线时间  Luzi 1998/10/23 */
{
    char path[80];
    FILE *fp;
    time_t now;
    sethomefile( path, getcurruserid() , "exit");
    fp=fopen(path, "wb");
    if (fp!=NULL)
    {
        now=time(NULL);
        fwrite(&now,sizeof(time_t),1,fp);
        fclose(fp);
    }
}

/* from stuff.c */
/* Case Independent strncmp */

void RemoveMsgCountFile2(userID)
char *userID;
{
  char fname[STRLEN];
  sethomefile(fname,userID,"msgcount");
  unlink(fname);
 }

int
ci_strncmp(s1,s2,n)
register char *s1,*s2 ;
register int n ;
{
    char        c1, c2;

    while( n-- > 0 ) {
        c1 = *s1++;
        c2 = *s2++;
        if( c1 >= 'a' && c1 <= 'z' )
            c1 &= 0xdf;
        if( c2 >= 'a' && c2 <= 'z' )
            c2 &= 0xdf;
        if( c1 != c2 )
            return (c1 - c2);
        if( c1 == 0 )
            return 0;
    }
    return 0;
}

int
ci_strcmp( s1, s2 )
register char   *s1, *s2;
{
    char        c1, c2;

    while( 1 ) {
        c1 = *s1++;
        c2 = *s2++;
        if( c1 >= 'a' && c1 <= 'z' )
            c1 &= 0xdf;
        if( c2 >= 'a' && c2 <= 'z' )
            c2 &= 0xdf;
        if( c1 != c2 )
            return (c1 - c2);
        if( c1 == 0 )
            return 0;
    }
}

void attach_err( int     shmkey,char    *name)
{
    /*prints( "Error! %s error! key = %x.\n", name, shmkey );
    oflush();*/
    exit( 1 );
}

void *attach_shm( char    *shmstr,int     defaultkey, int shmsize,int* iscreate)
{
	return attach_shm1(shmstr,defaultkey, shmsize,iscreate,0, NULL);
}

void *attach_shm1( char    *shmstr,int     defaultkey, int shmsize,int* iscreate,int readonly, void* shmaddr)
{
    void        *shmptr;
    int         shmkey, shmid;

    shmkey = get_shmkey( shmstr );
    if( shmkey < 1024 )
        shmkey = defaultkey;
    shmid = shmget( shmkey, shmsize, 0 );
    if( shmid < 0 ) {
        shmid = shmget( shmkey, shmsize, IPC_CREAT | 0660 ); /* modified by dong , for web application , 1998.12.1 */
        *iscreate=YEA;
        if( shmid < 0 ) {
            attach_err( shmkey, "shmget" );
            exit(0);
        }
        if (readonly)
            shmptr = (void *) shmat( shmid, shmaddr, SHM_RDONLY);
        else
            shmptr = (void *) shmat( shmid, shmaddr, 0 );
        if( shmptr == (void *)-1 ) {
            attach_err( shmkey, "shmat" );
            exit(0);
    	} else
        if (!readonly)
            memset( shmptr, 0, shmsize );
    } else {
    	*iscreate=0;
        if (readonly)
            shmptr = (void *) shmat( shmid, shmaddr, SHM_RDONLY);
        else
            shmptr = (void *) shmat( shmid, shmaddr, 0 );
        if( shmptr == (void *)-1 ) {
            attach_err( shmkey, "shmat" );
            exit(0);
        }
    }
    return shmptr;
}

int my_system(const char *cmdstring)
{
        pid_t pid;
        int status;
        if (!cmdstring) return 1;
        switch (pid = fork())
        {
                case -1:
                   status = -1;
                   break;
                case 0:
                   {
                        int fd = open("/dev/zero",O_RDWR);
                        dup2(fd,2);
                        dup2(fd,1);
                        if (fd !=1 && fd !=2) close(fd);
                        execl("/bin/sh","sh","-c",cmdstring,NULL);
                        _exit(127);
                   }
                default:
                        while (waitpid(pid,&status,0)<0)
                            if (errno!=EINTR) {
                                status = -1;
                                break;
                        }
                break;
        }
        return status;
}

int countexp(struct userec *udata)
{
    int exp;

    if(!strcmp(udata->userid,"guest"))
        return -9999;
    exp=udata->numposts +/*post_in_tin( udata->userid )*/+udata->numlogins/5+(time(0)-udata->firstlogin)/86400+udata->stay/3600;
    return exp>0?exp:0;
}

int countperf(struct userec *udata)
{
    int perf;
    int reg_days;

    if(!strcmp(udata->userid,"guest"))
        return -9999;
    reg_days=(time(0)-udata->firstlogin)/86400+1;
    perf=((float)(udata->numposts/*+post_in_tin( udata->userid )*/)/(float)udata->numlogins+
          (float)udata->numlogins/(float)reg_days)*10;
    return perf>0?perf:0;
}


int compute_user_value( struct userec *urec)
{
    int         value;

    /* if (urec) has CHATCLOAK permission, don't kick it */
	/* 元老和荣誉帐号 在不自杀的情况下， 生命力999 Bigman 2001.6.23 */
    /* 
    * zixia 2001-11-20 所有的生命力都使用宏替换，
    * 在 smth.h/zixia.h 中定义 
    * */
    
    if( ((urec->userlevel & PERM_HORNOR)||(urec->userlevel & PERM_CHATCLOAK )) && (!(urec->userlevel & PERM_SUICIDE)))
        return LIFE_DAY_NODIE;

    if ( urec->userlevel & PERM_SYSOP) 
	return LIFE_DAY_SYSOP;
	/* 站务人员生命力不变 Bigman 2001.6.23 */
	


    value = (time(0) - urec->lastlogin) / 60;    /* min */
    if (0 == value) value = 1; /* Leeward 98.03.30 */

    /* 修改: 将永久帐号转为长期帐号, Bigman 2000.8.11 */
    if ((urec->userlevel & PERM_XEMPT) && (!(urec->userlevel & PERM_SUICIDE)) )
    {	if (urec->lastlogin < 988610030)
        return LIFE_DAY_LONG; /* 如果没有登录过的 */
        else
            return (LIFE_DAY_LONG * 24 * 60 - value)/(60*24);
    }
    /* new user should register in 30 mins */
    if( strcmp( urec->userid, "new" ) == 0 ) {
        return (LIFE_DAY_NEW - value) / 60; /* *->/ modified by dong, 1998.12.3 */
    }

    /* 自杀功能,Luzi 1998.10.10 */
    if (urec->userlevel & PERM_SUICIDE)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value)/(60*24);
    /**********************/
    if(urec->numlogins <= 3)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value)/(60*24);
    if( !(urec->userlevel & PERM_LOGINOK) )
        return (LIFE_DAY_NEW * 24 * 60 - value)/(60*24);
    /* if (urec->userlevel & PERM_LONGID)
         return (667 * 24 * 60 - value)/(60*24); */
    return (LIFE_DAY_USER * 24 * 60 - value)/(60*24);
}

char *modestring(int mode,int towho,int complete,char *chatid)
{
    static char modestr[STRLEN];
    struct userec urec;

    /* Leeward: 97.12.18: Below removing ' characters for more display width */
    if (chatid) {
        if (complete) sprintf(modestr, "%s %s", ModeType(mode), chatid);
        else return ((char*)ModeType(mode));
        return (modestr);
    }
    if (mode != TALK && mode != PAGE && mode != QUERY)
        return ((char*)ModeType(mode));
    /*---	modified by period	2000-10-20	---*
        if (get_record(PASSFILE, &urec, sizeof(urec), towho) == -1)
            return (ModeType(mode));
    ---*/
    if (complete) {
	    if(getuserid(urec.userid, towho) != towho) return (char*)ModeType(mode);
        sprintf(modestr, "%s %s", ModeType(mode), urec.userid);
    }
    else
        return ((char*)ModeType(mode));
    return (modestr);
}

int id_invalid(char* userid)
{
    char *s;
    if (!isalpha(userid[0]))
        return 1;
    for(s=userid;*s != '\0'; s++) {
        if(*s<1 || !isalnum(*s)) {
            return 1;
        }
    }
    return 0;
}

int Rename(char* srcPath,char* destPath)
{
   int ret;

   ret = rename(srcPath,destPath);

   if (ret == 0) return 0;

   if (errno == EXDEV) {
     int in,out;
     char data[4096];
     int readsize;

     if ((in = open(srcPath,O_RDONLY)) == -1) return -1;
     if ((out = open(destPath,O_WRONLY|O_CREAT| O_TRUNC)) == -1) { close(in);return -1;};
     
     while ( (readsize = read(in,data,4096)) >0) write(out,data,readsize);

     close(out);
     close(in);
     return 0;
   }
   return -1;
}


/* from mail.c */
int
check_query_mail(qry_mail_dir)
char qry_mail_dir[STRLEN];
{
    struct fileheader fh ;
    struct stat st ;
    int fd ;
    register int  offset ;
    register long numfiles ;
    unsigned char ch ;

    offset = (int)((char *)&(fh.accessed[0]) - (char *)&(fh)) ;
    if((fd = open(qry_mail_dir,O_RDONLY)) < 0)
        return 0 ;
    fstat(fd,&st) ;
    numfiles = st.st_size ;
    numfiles = numfiles/sizeof(fh) ;
    if(numfiles <= 0) {
        close(fd) ;
        return 0 ;
    }
    lseek(fd,(st.st_size-(sizeof(fh)-offset)),SEEK_SET) ;
    /*离线查询新信只要查询最後一封是否为新信，其他并不重要*/
    /*Modify by SmallPig*/
    read(fd,&ch,1) ;
    if(!(ch & FILE_READ)) {
        close(fd) ;
        return YEA ;
    }
    close(fd) ;
    return NA ;
}

/* from talk.c */
int
cmpfuid( a,b )
struct friends   *a,*b;
{
    return strcasecmp(a->id,b->id);
}

int
getfriendstr()
{
    extern int nf;
    int i;
    struct friends* friendsdata;
	char filename[STRLEN];

    if(topfriend!=NULL)
        free(topfriend);
    setuserfile( filename, "friends" );
    nf=get_num_records(filename,sizeof(struct friends));
    if(nf<=0)
        return 0;
    if(!has_perm(PERM_ACCOUNTS) && !has_perm(PERM_SYSOP))/*Haohmaru.98.11.16*/
        nf=(nf>=MAXFRIENDS)?MAXFRIENDS:nf;
    friendsdata=(struct friends *)calloc(sizeof(struct friends),nf);
    get_records(filename,friendsdata,sizeof(struct friends),1,nf);
    
    qsort( friendsdata, nf, sizeof( friendsdata[0] ), cmpfuid );/*For Bi_Search*/
    topfriend=(struct friends_info *)calloc(sizeof(struct friends_info),nf);
    for (i=0;i<nf;i++) {
    	topfriend[i].uid=searchuser(friendsdata[i].id);
    	strcpy(topfriend[i].exp,friendsdata[i].exp);
    }
    free(friendsdata);
    return 0;
}

int addtooverride2(char *uident, char *exp)
{
    friends_t tmp;
    int  n;
    char buf[STRLEN];

    memset(&tmp,0,sizeof(tmp));
    setuserfile( buf, "friends" );
    if((!has_perm(PERM_ACCOUNTS) && !has_perm(PERM_SYSOP)) &&
            (get_num_records(buf,sizeof(struct friends))>=MAXFRIENDS) )
    {
        return -1;
    }
    if( myfriend( searchuser(uident) , NULL) )
        return -2;
	if (exp == NULL || exp[0] == '\0')
		tmp.exp[0] = '\0';
	else
	{
		strncpy(tmp.exp, exp, sizeof(tmp.exp)-1);
		tmp.exp[sizeof(tmp.exp)-1] = '\0';
	}
    n=append_record(buf,&tmp,sizeof(struct friends));
    if(n!=-1)
        getfriendstr();
    else
        return -3;
    return n;
}

void save_userfile(char * fname, int numblk, char * buf)
{
    char        fbuf[ 256 ];
    int         fd, size;

    setuserfile( fbuf, fname );
    if( (fd = open( fbuf, O_WRONLY | O_CREAT, 0600 )) != -1 ) {
        size = numblk * sizeof( int );
        write( fd, buf, size );
        close( fd );
    }
}

void save_favboard()
{
    save_userfile("favboard", (FAVBOARDNUM+1), (char *)favbrd_list);
}

int DelFavBoard(int i)
{
    int lnum;
    if(i > *favbrd_list)
		return *favbrd_list;
    lnum = --(*favbrd_list);
    for(;i<=lnum;i++)
		favbrd_list[i] = favbrd_list[i+1];
    if(!lnum)
	{
        *favbrd_list = 1;       /*  favorate board count    */
        *(favbrd_list+1) = 0;   /*  default sysop board     */
    }
    return 0;
}

void load_favboard()
{
    char fname[STRLEN];
    int  fd, size, idx;

    setuserfile(fname, "favboard");
    if( (fd = open( fname, O_RDONLY, 0600 )) != -1 )
	{
        size = (FAVBOARDNUM+1) * sizeof( int );
        read( fd, favbrd_list, size );
        close( fd );
    }
	//else if(dohelp)
	//{
        //int savmode;
        //savmode = uinfo.mode;
        //modify_user_mode(CSIE_ANNOUNCE);	/* 没合适的mode.就先用"汲取精华"吧. */
        //show_help("help/favboardhelp");
        //modify_user_mode(savmode);
    //}
    if(*favbrd_list<= 0)
	{
        *favbrd_list = 1;       /*  favorate board count    */
        *(favbrd_list+1) = 0;   /*  default sysop board     */
    }
    else
	{
        int num = *favbrd_list;
        if(*favbrd_list > FAVBOARDNUM)	/*	maybe file corrupted	*/
            *favbrd_list = FAVBOARDNUM;
        idx = 0;
        while(++idx <= *favbrd_list)
		{
        	struct boardheader* bh;
            fd = favbrd_list[idx];
            bh = (struct boardheader*) getboard(fd+1);
            if((fd >= 0 && fd <= get_boardcount())
				&& ( bh && bh->filename[0]
                        && ( (bh->level & PERM_POSTMASK)
                             || has_perm(bh->level)
                             || (bh->level&PERM_NOZAP) )
                   )
              )
                continue;
            DelFavBoard(idx);   /*  error correction    */
        }
        if(num != *favbrd_list)
			save_favboard();
    }
}

int IsFavBoard(int idx)
{
    int i;
    for(i=1;i<=*favbrd_list;i++)
	{
		if(idx == favbrd_list[i])
			return i;
	}
    return 0;
}

int get_favboard(int num)
{
    if(num > 0 && num <= FAVBOARDNUM)
		return favbrd_list[num];
	else
		return -1;
}

int get_favboard_count()
{
	return favbrd_list[0];
}

int add_favboard(char *brdname)
{
	int i;

	if(brdname != NULL && *brdname)
		i = getbnum(brdname);
	else
		return -3; // err brdname
	if (*favbrd_list > FAVBOARDNUM)
		return -2; // favboard had reach max limit
	if( i > 0 && !IsFavBoard(i-1) )
	{
		int llen;
		llen = ++(*favbrd_list);
		favbrd_list[llen] = i-1;
		//save_favboard();

		return llen; //return current favorite boards count
	}

	return -1; // brdname not found or brdname already in favbrd_list
}

/* by flyriver, 2001.12.19 */
int count_online() /* ugly */
{
	struct UTMPFILE *u;

	u = get_utmpshm_addr();
	return u == NULL ? 0 : u->number;
}

int get_ulist_length()
{
	return sizeof(user_record)/sizeof(user_record[0]);
}

struct user_info **get_ulist_addr()
{
	return user_record;
}

uinfo_t *get_user_info(int utmpnum)
{
	struct UTMPFILE *utmpshm_ptr;

	if (utmpnum < 1 || utmpnum > USHM_SIZE)
		return NULL;
	utmpshm_ptr = get_utmpshm_addr();
	return &(utmpshm_ptr->uinfo[utmpnum-1]);
}

int set_friends_num(int num)
{
	nf = num;
	return nf;
}

int get_friends_num()
{
	return nf;
}

struct friends_info *init_finfo_addr()
{
	topfriend = NULL;
	return topfriend;
}

struct boardheader *getbcache_addr()
{
	return bcache;
}

int set_friendmode(int mode)
{
	friendmode = mode;

	return friendmode;
}

int get_friendmode()
{
	return friendmode;
}

