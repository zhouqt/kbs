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
struct UCACHE   *uidshm;

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
        strncpy((char*)uidshm->users[*number],uentp->userid,IDLEN+1) ;
        uidshm->users[*number][IDLEN] = '\0' ;
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
    strncpy(userid,(char*)uidshm->users[uid-1], IDLEN+1);
    return uid;
}

void
setuserid( int     num,const char    *userid) /* 设置user num的id为user id*/
{
    if( num > 0 && num <= MAXUSERS ) {
    	int oldkey,newkey,find;
        if( num > uidshm->number )
            uidshm->number = num;
        oldkey=ucache_hash((char*)uidshm->users[ num - 1 ]);
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
		          	log("3system","UCACHE:can't find %s in hash table",uidshm->users[ num - 1 ]);
		          	exit(0);
			}
	          }
	          else uidshm->next[find-1] = uidshm->next[num-1];
	        }

	        uidshm->next[num-1]=uidshm->hashhead[newkey];
	        uidshm->hashhead[newkey]=num;
		    ucache_unlock(lockfd);
        }	        
        strncpy( (char*)uidshm->users[ num - 1 ], userid, IDLEN+1 );
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
		if (!strcasecmp(userid,uidshm->users[i-1]))
			return i;
		else
			i=uidshm->next[i-1];
    return 0 ;
}

int
getuser(const char *userid) /* 取用户信息 */
{
    int uid = searchuser(userid) ;

    if(uid == 0) return 0 ;
    get_record(PASSFILE,&lookupuser,sizeof(lookupuser),uid) ;
    return uid ;
}

char* getuserid2(int uid)
{
	return uidshm->users[uid-1];
}

char *u_namearray( char    buf[][ IDLEN+1 ],int     *pnum, char * tag)
/* 根据tag ,生成 匹配的user id 列表 (针对所有注册用户)*/
{
    register struct UCACHE *reg_ushm = uidshm;
    register int n, num, i;
    int hash, len;
    char tagv[IDLEN+1];

    *pnum=0;

    len=strlen(tag);
    if(!len){
        *pnum=MAXUSERS;
        return (char*)reg_ushm->users;
    }

    strcpy(tagv,tag);
   
    if(len>=UCACHE_HASHKCHAR){
        tagv[UCACHE_HASHKCHAR]=0;
        hash = ucache_hash(tagv);
        for( n = hash; n < hash+UCACHE_HASHBSIZE; n++ ) {
            num=reg_ushm->hashhead[n];
            while(num){
                if(! strncasecmp(reg_ushm->users[num-1],tag,len)){
                    strcpy( buf[ (*pnum)++ ], reg_ushm->users[num-1] ); /*如果匹配, add into buf */
                } 
                num=reg_ushm->next[num-1];
            }
        }
    } else {
        for(i=len;i<UCACHE_HASHKCHAR;i++)tagv[i]='0';
        tagv[UCACHE_HASHKCHAR]=0;

        while(1){
            hash = ucache_hash(tagv);
            hash-=hash%UCACHE_HASHBSIZE;
            for( n = hash; n < hash+UCACHE_HASHBSIZE; n++ ) {
                num=reg_ushm->hashhead[n];
                while(num){
                    if(! strncasecmp(reg_ushm->users[num-1],tag,len)){
                        strcpy( buf[ (*pnum)++ ], reg_ushm->users[num-1] ); /*如果匹配, add into buf */
                    } 
                    num=reg_ushm->next[num-1];
                }
            }

            i=UCACHE_HASHKCHAR-1;
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


