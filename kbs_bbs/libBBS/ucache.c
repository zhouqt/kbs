/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

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
   

struct UCACHE   *uidshm;

int ucache_lock()
{
	int lockfd;
    lockfd = open( BBSHOME "ucache.lock", O_RDWR|O_CREAT, 0600 );
    if( lockfd < 0 ) {
        log_usies( "CACHE", "reload ucache lock error!!!!" );
        return -1;
    }
    flock(lockfd,LOCK_EX);
    return lockfd;
}

int ucache_unlock(int fd)
{
    flock(lockfd,LOCK_UN);
    close(lockfd);
}


int ucache_hash(char* userid)
{
	int * i=(int*) userid;
	int key=0;
	key+=*i++;
	key+=*i++;
	key+=*i++;
	key%=UCACHE_HASHSIZE;
	return key;
}

int
fillucache(uentp) /* user cache中 添加user */
struct userec *uentp ;
{
    if(usernumber < MAXUSERS) {
        strncpy(uidshm->userid[usernumber],uentp->userid,IDLEN+1) ;
        uidshm->userid[usernumber++][IDLEN] = '\0' ;
    }
    return 0 ;
}


void
resolve_ucache()
{
    struct stat st ;
    int         ftime;
    time_t      now;
    char   log_buf[256]; /* Leeward 99.10.24 */
	int lockfd;
	
	lockfd = ucache_lock();
    if( uidshm == NULL ) {
        uidshm = attach_shm( "UCACHE_SHMKEY", 3696, sizeof( *uidshm ) ); /*attach to user shm */
    }

	if (lockfd==-1) return -1;
	
    if( stat( PASSFILE,&st ) < 0 ) {
        st.st_mtime++ ;
    }
    ftime = st.st_mtime;
    if( uidshm->uptime < ftime ) {
        usernumber = 0;
        apply_record( PASSFILE, fillucache, sizeof(struct userec) ); /*刷新user cache */
        sprintf(log_buf, "reload ucache for %d users", usernumber);
        log_usies( "CACHE", log_buf );
        uidshm->number = usernumber;
        uidshm->uptime = ftime+100000; /* change by dong ? */
    }
    ucache_unlock(lockfd);
}

/*---	period	2000-10-20	---*/
int getuserid(char * userid, int uid)
{
    if( uid > uidshm->number || uid <= 0 ) return 0;
    strncpy(userid,uidshm->userid[uid-1], IDLEN+1);
    return uid;
}

void
setuserid( num, userid ) /* 设置user num的id为user id*/
int     num;
char    *userid;
{
	int lockfd = ucache_lock();
    if( num > 0 && num <= MAXUSERS ) {
        if( num > uidshm->number )
            uidshm->number = num;
        strncpy( uidshm->userid[ num - 1 ], userid, IDLEN+1 );
    }
    ucache_unlock(lockfd);
}

int
searchnewuser() /* 找cache中 空闲的 user num */
{
    register int num, i;

    num = uidshm->number; /* cache 中 user 总数*/
    for(i=0; i < num; i++)
        if( uidshm->userid[i][0] == '\0' )
            return i+1 ;
    if( num < MAXUSERS )
        return( num + 1 );
    return 0 ;
}
int
searchuser(userid)
char *userid ;
{
    register int i ;

    for(i=0; i < uidshm->number; i++)
        if(!strncasecmp(userid,uidshm->userid[i],IDLEN+1))
            return i+1 ;
    return 0 ;
}

int
getuser(userid) /* 取用户信息 */
char *userid ;
{
    int uid = searchuser(userid) ;

    if(uid == 0) return 0 ;
    get_record(PASSFILE,&lookupuser,sizeof(lookupuser),uid) ;
    return uid ;
}

char *
u_namearray( buf, pnum, tag )  /* 根据tag ,生成 匹配的user id 列表 (针对所有注册用户)*/
char    buf[][ IDLEN+1 ], *tag;
int     *pnum;
{
    register struct UCACHE *reg_ushm = uidshm;
    register char       *ptr, tmp;
    register int        n, total;
    char        tagbuf[ STRLEN ];
    int         ch, num = 0;

    if( *tag == '\0' ) { /* return all user */
        *pnum = reg_ushm->number;
        return reg_ushm->userid[0];
    }
    for( n = 0; tag[n] != '\0'; n++ ) {
        tagbuf[ n ] = chartoupper( tag[n] );
    }
    tagbuf[ n ] = '\0';
    ch = tagbuf[0];
    total = reg_ushm->number; /* reg. user total num */
    for( n = 0; n < total; n++ ) {
        ptr = reg_ushm->userid[n];
        tmp = *ptr;
        if( tmp == ch || tmp == ch - 'A' + 'a' ) /* 判断第一个字符是否相同*/
            if( chkstr( tag, tagbuf, ptr ) )
                strcpy( buf[ num++ ], ptr ); /*如果匹配, add into buf */
    }
    *pnum = num;
    return buf[0];
}


