/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Copyright (C) 2001, Zhou Lin, KCN@cic.tsinghua.edu.cn
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/* user cache 处理
   用hask table 保存所有ID的id名,加快查找速度
   2001.5.4 KCN
*/
   

#include "bbs.h"

struct userec lookupuser;
struct userec* currentuser;
static int passwdfd=-1;
static struct userec* passwd;
static struct UCACHE   *uidshm;

static int ucache_lock()
{
    int lockfd;
    lockfd = creat( BBSHOME "/ucache.lock", 0600 );
    if( lockfd < 0 ) {
        log( "3system", "CACHE:lock ucache:%s", strerror(errno) );
        return -1;
    }
    flock(lockfd,LOCK_EX);
    return lockfd;
}

static int ucache_unlock(int fd)
{
    flock(fd,LOCK_UN);
    close(fd);
}

/* do init hashtable , read from uhashgen.dat -- wwj*/
static void ucache_hashinit()
{
    FILE *fp;
    char line[256];
    int  i,j,ptr,data;


    fp=fopen("uhashgen.dat","rt");
    if(!fp){
        log("3system","UCACHE:load uhashgen.dat fail");
        exit(0);
    }
    i=0;
    while(fgets(line,sizeof(line),fp)){
        if(line[0]=='#')continue;
        j=0;
        ptr=0;
        while(line[ptr]>='0' && line[ptr]<='9' || line[ptr]=='-'){
            data=ptr;
            while(line[ptr]>='0' && line[ptr]<='9' || line[ptr]=='-')ptr++;
            line[ptr++]=0;
            if(i==0){
                if(j>=26){
        			log("3system","UCACHE:hash0>26");
                    exit(0);
                }
                uidshm->hashtable.hash0[j++]=atoi(&line[data]);
            } else {
                if(j>=36){
        			log("3system","UCACHE:hash0>26");
                    exit(0);
                }
                uidshm->hashtable.hash[i-1][j++]=atoi(&line[data]);
            }
        }
        i++;
        if(i>sizeof(uidshm->hashtable.hash)/36){
            log("3system","hashline %d exceed",i);
            exit(0);
        }
    }
    fclose(fp);

    log( "1system","HASH load" );
}

/*
   CaseInsensitive ucache_hash, assume
   isalpha(userid[0])
   isahpha(userid[n]) || isnumber(userid[n]) n>0
   01/5/5 wwj
 */
unsigned int ucache_hash_deep(const char* userid)
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

    n=1;
    while(n1<0){
        n1=-n1-1;
        if(!*userid){
            usage->hash[n1][0]++;
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
        n++;
    }
    return n;
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

    usage->hash0[n1]++;
    n1=hash->hash0[n1];

    while(n1<0){
        n1=-n1-1;
        if(!*userid){
            usage->hash[n1][0]++;
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
            usage->hash[n1][n2]++;
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

static int
fillucache(struct userec *uentp ,int* number)
{
    if(*number < MAXUSERS) {
    	int hashkey;
        strncpy((char*)passwd[*number].userid,uentp->userid,IDLEN+1) ;
        hashkey = ucache_hash(uentp->userid);
	if (hashkey<0||hashkey>UCACHE_HASHSIZE) {
		log("3system","UCACHE:hash(%s) %d error",uentp->userid, hashkey);
		exit(0);
	}
        uidshm->next[*number] = uidshm->hashhead[hashkey];
        uidshm->hashhead[hashkey] = ++(*number);
    }
    return 0 ;
}


void
resolve_ucache()
{
    struct stat st ;
    int         ftime;
    time_t      now;
    int iscreate;

    if( uidshm == NULL ) {
        uidshm = (struct UCACHE*)attach_shm( "UCACHE_SHMKEY", 3696, sizeof( *uidshm ) ,&iscreate); /*attach to user shm */
    }

/*  This need to do by using other way 
	if (!iscreate) {
	    if( stat( FLUSH,&st ) == 0 ) {
		    ftime = st.st_mtime;
		    if( uidshm->uptime < ftime )  {
		    	iscreate=1;
		        uidshm->uptime = ftime;
		    }
	    }
	}*/
	if (passwdfd==-1) {
		if ((passwdfd=open(PASSFILE,O_RDWR|O_CREAT,0644)) == -1) {
			log("4system","Can't open " PASSFILE "file %s",strerror(errno));
        	exit(-1);
		}
    	passwd = (struct userec*) mmap(NULL,
    			MAXUSERS*sizeof(struct userec),
    			PROT_READ|PROT_WRITE,MAP_SHARED,passwdfd,0);
    	if (passwd==(struct userec*)-1) {
			log("4system","Can't map " PASSFILE "file %s",strerror(errno));
        	exit(-1);
    	}
	}
	if (iscreate) {
    	int lockfd = ucache_lock();
		int     usernumber;
    	if (lockfd==-1) {
    		log("3system","UCACHE:can't lock ucache");
    		exit(0);
    	}
    	bzero(uidshm->hashhead,UCACHE_HASHSIZE*sizeof(int));
        usernumber = 0;

    	ucache_hashinit();

        apply_record( PASSFILE, fillucache, sizeof(struct userec),&usernumber ); /*刷新user cache */

        log("1system", "CACHE:reload ucache for %d users", usernumber);
        uidshm->number = usernumber;
        ucache_unlock(lockfd);
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
setuserid( int     num,const char    *userid) /* 设置user num的id为user id*/
{
    if( num > 0 && num <= MAXUSERS ) {
    	int oldkey,newkey,find;
        if( num > uidshm->number )
            uidshm->number = num;
        oldkey=ucache_hash((char*)passwd[ num - 1 ].userid);
        newkey=ucache_hash(userid);
        if (oldkey!=newkey) {
		int lockfd = ucache_lock();
	        find=uidshm->hashhead[oldkey];

	        if (find==num) uidshm->hashhead[oldkey]=uidshm->next[find-1];
	        else { /* find and remove the hash node */
	          while (uidshm->next[find-1]&&uidshm->next[find-1]!=num)
	      			find=uidshm->next[find-1];
	          if (!uidshm->next[find-1]) {
			if (oldkey!=0) {
		          	log("3system","UCACHE:can't find %s in hash table",passwd[ num - 1 ].userid);
		          	exit(0);
			}
	          }
	          else uidshm->next[find-1] = uidshm->next[num-1];
	        }

	        uidshm->next[num-1]=uidshm->hashhead[newkey];
	        uidshm->hashhead[newkey]=num;
		    ucache_unlock(lockfd);
        }	        
        strncpy( passwd[ num - 1 ].userid, userid, IDLEN+1 );
    }
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
		if (!strcasecmp(userid,passwd[ i - 1 ].userid))
			return i;
		else
			i=uidshm->next[i-1];
    return 0 ;
}

int getuser(const char *userid,struct userec** user) /* 取用户信息 */
{
    int uid = searchuser(userid) ;

    if(uid == 0) return 0 ;
    if (user)
	    *user=&passwd[uid-1];
    return uid ;
}

char* getuserid2(int uid)
{
	return passwd[uid-1].userid;
}

char *u_namearray( char    buf[][ IDLEN+1 ],int     *pnum, char * tag)
/* 根据tag ,生成 匹配的user id 列表 (针对所有注册用户)*/
{
    register struct UCACHE *reg_ushm = uidshm;
    register int n, num, i;
    int hash, len, ksz;
    char tagv[IDLEN+1];

    *pnum=0;

    len=strlen(tag);
    if(!len){
        return NULL;
    }
    ksz=ucache_hash_deep(tag);

    strcpy(tagv,tag);


    if(len>=ksz){
        tagv[ksz]=0;
        hash = ucache_hash(tagv)-1;
        for( n = 0; n < UCACHE_HASHBSIZE; n++ ) {
            num=reg_ushm->hashhead[(hash+n%UCACHE_HASHBSIZE)%UCACHE_HASHSIZE+1];
            while(num){
                if(! strncasecmp(passwd[ num - 1 ].userid,tag,len)){
                    strcpy( buf[ (*pnum)++ ], passwd[ num - 1 ].userid ); /*如果匹配, add into buf */
                }
                num=reg_ushm->next[num-1];
            }
        }
    } else {
        for(i=len;i<ksz;i++)tagv[i]='0';
        tagv[ksz]=0;

        while(1){
            hash = ucache_hash(tagv)-1;

            for( n = 0; n < UCACHE_HASHBSIZE; n++ ) {
                num=reg_ushm->hashhead[(hash+n%UCACHE_HASHBSIZE)%UCACHE_HASHSIZE+1]; /* see hash() */
                while(num){
                    if(! strncasecmp(passwd[ num - 1 ].userid,tagv,ksz)){
                        strcpy( buf[ (*pnum)++ ], passwd[ num - 1 ].userid ); /*如果匹配, add into buf */
                    }
                    num=reg_ushm->next[num-1];
                }
            }

            i=ksz-1;
            while(i>=len){
                if(tagv[i]=='Z'){
                    tagv[i]='0';
                    i--;
                } else if(tagv[i]=='9'){
                    tagv[i]='A';
                    break;
                } else {
                    tagv[i]++;
                    break;
                }
            }
            if(i<len)break;
        }

    }
    return buf[0];
}

/* disable by KCN 
int
set_safe_record()
{
    struct userec tmp;
    extern int ERROR_READ_SYSTEM_FILE;

    if(get_record(PASSFILE,&tmp,sizeof(currentuser),usernum)==-1)
    {
        char buf[STRLEN];

        sprintf(buf,"Error:Read Passfile %4d %12.12s",usernum,currentuser->userid);
        report(buf);
        ERROR_READ_SYSTEM_FILE=YEA;
        abort_bbs();
        return -1;
    }
    currentuser->numposts=tmp.numposts;
    currentuser->numlogins=tmp.numlogins;
    currentuser->stay=tmp.stay;
    currentuser->userlevel=tmp.userlevel;
}
*/

int getnewuserid(char* userid)
{
    struct userec utmp, zerorec;
    struct stat st;
    int         fd, size, val, i;
    char    tmpstr[30];
    time_t system_time;

    system_time = time( NULL );
    /*
    if( stat( "tmp/killuser", &st )== -1 || st.st_mtime < system_time-3600 ) {
        if( (fd = open( "tmp/killuser", O_RDWR|O_CREAT, 0600 )) == -1 )
            return -1;
        write( fd, ctime( &system_time ), 25 );
        close( fd );
        log_usies( "CLEAN", "dated users." );
        prints( "寻找新帐号中, 请稍待片刻...\n\r" );
        oflush();
        memset( &zerorec, 0, sizeof( zerorec ) );
        if( (fd = open( PASSFILE, O_RDWR|O_CREAT, 0600 )) == -1 )
            return -1;
        size = sizeof( utmp );
        for( i = 0; i < MAXUSERS; i++ ) {
            if( read( fd, &utmp, size ) != size )
                break;
            val = compute_user_value( &utmp );
            if( utmp.userid[0] != '\0' && val <= 0 ) {
                sprintf( genbuf, "#%d %-12s %15.15s %d %d %d",
                         i+1, utmp.userid, ctime( &(utmp.lastlogin) )+4,
                         utmp.numlogins, utmp.numposts, val );
                log_usies( "KILL ", genbuf );
                if( !bad_user_id( utmp.userid ) ) {
                    setmailpath(tmpstr, utmp.userid);
                    sprintf( genbuf, "/bin/rm -fr %s", tmpstr);
                    system( genbuf );
                    sethomepath(tmpstr, utmp.userid);
                    sprintf( genbuf, "/bin/rm -fr %s", tmpstr);
                    system( genbuf );
                    sprintf( genbuf, "/bin/rm -f tmp/email_%s", utmp.userid );
                    system( genbuf );
                }
                lseek( fd, -size, SEEK_CUR );
                write( fd, &zerorec, sizeof( utmp ) );
            }
        }
        close( fd );
    }
*/
    if( (fd = open( PASSFILE, O_RDWR|O_CREAT, 0600 )) == -1 )
        return -1;
    flock( fd, LOCK_EX );

    i = searchnewuser();
    log( "1system", "APPLY: uid %d from %s", i, fromhost );

    if( i <= 0 || i > MAXUSERS ) {
        flock(fd,LOCK_UN) ;
        close(fd) ;
#ifdef BBSMAIN
        if( dashf( "etc/user_full" ) ) {
            ansimore( "etc/user_full", NA );
            oflush();
        } else {
            prints( "抱歉, 使用者帐号已经满了, 无法注册新的帐号.\n\r" );
            oflush();
        }
        val = (st.st_mtime - system_time + 3660) / 60;
        prints( "请等待 %d 分钟後再试一次, 祝你好运.\n\r", val );
        oflush();
        sleep( 2 );
#endif
        return -1;
    }
    memset( &utmp, 0, sizeof( utmp ) );
    strcpy( utmp.userid, userid );
    utmp.lastlogin = time( NULL );
    update_user(&utmp,i,1);
    setuserid( i, userid ); /* added by dong, 1998.12.2 */
    flock( fd, LOCK_UN );
    close( fd );
    return i;
}

int update_user(struct userec* user,int num,int all)
{
	struct userec tmpuser;
	if (!all) {
		if (strncasecmp(user->userid,passwd[num-1].userid,IDLEN)) 
			return -1;
		tmpuser=*user;
		memcpy(tmpuser.userid,passwd[num-1].userid,IDLEN+2);
		memcpy(tmpuser.passwd,passwd[num-1].passwd,IDLEN+2);
		memcpy(tmpuser.md5passwd,passwd[num-1].passwd,IDLEN+2);
		memcpy(&passwd[num-1],&tmpuser,sizeof(struct userec));
	} else
		memcpy(&passwd[num-1],user,sizeof(struct userec));
	return 0;
}

int apply_users(int (*fptr)(struct userec* ,char*),char* arg)
{
	int i;
	int count;
	count=0;
	for (i=0;i<uidshm->number;i++)
		if (fptr) {
			int ret;
			ret = (*fptr)(&passwd[i],arg);
			if (ret==QUIT) break;
			if (ret==COUNT) count++;
		} else count++;
	return count;
}
