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
#include <sys/ipc.h>
#include <sys/shm.h>

struct UTMPFILE *utmpshm;

void resolve_utmp()
{
	int iscreate;
    if( utmpshm == NULL ) {
        utmpshm = (struct UTMPFILE*)attach_shm( "UTMP_SHMKEY", 3699, sizeof( *utmpshm ),&iscreate );/*attach user tmp cache */
        if (iscreate) {
        	int i;
        	bzero(utmpshm,sizeof(struct UTMPFILE));
			utmpshm->number=0;
        	utmpshm->hashhead[0]=1;
        	for (i=0;i<USHM_SIZE-1;i++) 
        		utmpshm->next[i]=i+2;
        	utmpshm->next[USHM_SIZE-1]=0;
        }
    }
}

static int utmp_lock()
{
    int          utmpfd=0;
    utmpfd = open( ULIST, O_RDWR|O_CREAT, 0600 );
    if( utmpfd < 0 ) {
        exit(-1);
    }
    if (flock( utmpfd, LOCK_EX ) ==-1)  {
        exit(-1);
    }
    return utmpfd;
}

static void utmp_unlock(int fd)
{
	flock(fd,LOCK_UN);
	close(fd);
}

static int utmp_hash(const char* userid)
{
	char ch;
	int i;
	if (!userid[0]||!userid[1]) return 0;
	i=(toupper(userid[0])-'A')*26;
	if ((userid[1]>='0')&&(userid[1]<='9'))
		i+=userid[1]-'0'+26;
	else i+=(toupper(userid[1])-'A');
	return i*20 + ucache_hash(userid)%20; 
}

int getnewutmpent(struct user_info *up)
{
    struct user_info    *uentp;
    time_t      now;
    int         pos, n, i,prev;
    int utmpfd,hashkey;

	utmpfd = utmp_lock();
	pos = utmpshm->hashhead[0]-1;
    if( pos==-1 ) {
    	utmp_unlock(utmpfd);
        return -1;
    }
    utmpshm->hashhead[0]=utmpshm->next[pos];
    utmpshm->uinfo[pos] = *up;
    hashkey=utmp_hash(up->userid);

    i = utmpshm->hashhead[hashkey];
    if ((!i)||(strcasecmp(utmpshm->uinfo[i-1].userid,up->userid)>=0)) {
    	utmpshm->next[pos]=i;
	    utmpshm->hashhead[hashkey]=pos+1;
    } else {
    	prev = i;
    	i=utmpshm->next[i-1];
        while (i && strcasecmp(utmpshm->uinfo[i-1].userid,up->userid) <0) {
        	prev=i;
    	    i = utmpshm->next[i-1];
        }
        utmpshm->next[pos]=utmpshm->next[prev-1];
        utmpshm->next[prev-1]=pos+1;
    }
	utmpshm->number++;
    now = time( NULL );
    if(( now > utmpshm->uptime + 120 )||(now < utmpshm->uptime-120)) {
        utmpshm->uptime = now;
        log( "3system", "UTMP:Clean user utmp cache");
        for( n = 0; n < USHM_SIZE; n++ ) {
            utmpshm->uptime = now;
            uentp = &(utmpshm->uinfo[ n ]);
            if( uentp->active && uentp->pid && kill( uentp->pid, 0 ) == -1 ) /*uentp检查*/
            {
                char buf[STRLEN];
                strncpy(buf, uentp->userid, IDLEN+2);
                clear_utmp(n+1);
                RemoveMsgCountFile2(buf);
            }
        }
    }
    utmp_unlock(utmpfd);
    return pos+1 ;
}

int
apply_ulist( APPLY_UTMP_FUNC fptr,char* arg) /* apply func on user list */
{
    struct user_info    *uentp, utmp;
    int         i, max;

    max = USHM_SIZE - 1;
    while( max > 0 && utmpshm->uinfo[ max ].active == 0 ) /*跳过后段 非active的user*/
        max--;
    for( i = 0; i <= max; i++ ) {
        uentp = &(utmpshm->uinfo[ i ]);
        utmp = *uentp;
        if( (*fptr)( &utmp,arg,i ) == QUIT )
            return QUIT;
    }
    return 0;
}

int apply_ulist_addr( APPLY_UTMP_FUNC fptr,char* arg,struct UTMP_POS *pos) /* apply func on user list */
{
    struct user_info    *uentp, utmp;
    int         i, max;
    int			num;

	num = 0;
	if (pos->key==0) {
		pos->key=1;
		pos->curpos=utmpshm->hashhead[1];
	}

	for (;pos->key<UTMP_HASHSIZE;pos->key++) {
		while (pos->curpos) {
			int ret;
	        ret = (*fptr)( &utmpshm->uinfo[ pos->curpos-1 ],arg,pos->curpos ) ;
	        if (ret == QUIT )
	            return QUIT;
	        if (ret ==COUNT) num++;
			pos->curpos=utmpshm->next[pos->curpos-1];
		}
		pos->curpos=utmpshm->hashhead[pos->key+1];
	}
    return num;
}

int apply_utmpuid(APPLY_UTMP_FUNC fptr,int uid,char* arg)
{
	int i,num;
	char userid[IDLEN+1];
	num=0;

    if (uid<=0) return 0;
	strcpy(userid,getuserid2(uid));
    return apply_utmp(fptr,0,userid,arg);
}

int apply_utmp(APPLY_UTMP_FUNC fptr,int maxcount,char* userid,char* arg)
{
	int i,num;
	int hashkey;
	
	num=0;
	hashkey = utmp_hash(userid);
	i = utmpshm->hashhead[hashkey];
	while (i) {
		if ((utmpshm->uinfo[i-1].active) &&
		  (!strcasecmp(utmpshm->uinfo[i-1].userid,userid)))
        {
        	int ret;
        	if (fptr) {
                ret = (*fptr)(&utmpshm->uinfo[ i-1 ],arg,i);
        	    if (ret==QUIT)
        	        break;
        	    if (ret==COUNT) num++;
        	    if (maxcount&&(num>maxcount)) break;
        	} else
        		num++;
        }
		i = utmpshm->next[i-1];
	}
    return num;
}

int
search_ulist( uentp, fptr, farg ) /* ulist 中 search 符合fptr函数的 user */
struct user_info *uentp;
int (*fptr)();
int farg;
{
    int         i;

    for( i = 0; i < USHM_SIZE; i++ ) {
        *uentp = utmpshm->uinfo[ i ];
        if( (*fptr)( farg, uentp ) )
            return i+1;
    }
    return 0;
}

void clear_utmp2(struct user_info* uentp)
{
	clear_utmp((uentp-utmpshm->uinfo)/sizeof(struct user_info));
}

void clear_utmp(int uent)
{
 	int lockfd , hashkey, find;
   	struct user_info zeroinfo;
	if (!uent) {
		if (!CHECK_UENT(uinfo.uid))
			return;
		uent=utmpent;
	}
   	lockfd=utmp_lock();

	hashkey=utmp_hash(utmpshm->uinfo[uent-1].userid);
	find = utmpshm->hashhead[hashkey];

	if (find == uent) 
		utmpshm->hashhead[hashkey] = utmpshm->next[uent-1];
	else {
		while (utmpshm->next[find-1]&&utmpshm->next[find-1]!=uent)
			find = utmpshm->next[find-1];
		if (!utmpshm->next[find-1])
			log("3system","UTMP:Can't find %s [%d]",uinfo.userid,uent);
		else
			utmpshm->next[find-1]=utmpshm->next[uent-1];
	}

  	log("1system","UTMP:clean %s",utmpshm->uinfo[ uent - 1 ].userid);
	utmpshm->next[uent-1]=utmpshm->hashhead[0];
	utmpshm->hashhead[0]=uent;
    zeroinfo.active = NA ;
    zeroinfo.pid = 0 ;
    zeroinfo.invisible = YEA ;
    zeroinfo.sockactive = NA ;
    zeroinfo.sockaddr = 0 ;
    zeroinfo.destuid = 0 ;

    utmpshm->uinfo[ uent - 1 ] = zeroinfo;
  	utmpshm->number--;
    utmp_unlock(lockfd);
}


