/* share memory 处理
                            bcache:版 , ucache:所有注册用户, utmp cache:在线user 
*/

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

#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>

struct BCACHE   *brdshm;
struct shortfile *bcache;
int     numboards = -1 ;

int
fillbcache(struct boardheader *fptr,char* arg)/*fill board cache */
{
    struct shortfile *bptr;

    if( numboards >= MAXBOARD )
        return 0;
    bptr = &bcache[ numboards++ ];
    memcpy( bptr, fptr, sizeof( struct shortfile ) );
    return 0 ;
}

/* 为了在人数多的时候减少flock, 重读BOARDS, 将原来的resolve_boards改为reload_boards,
   并重写了resolve_boards. ylsdd 2001.4.26 */
void
reload_boards()
{
    struct stat st;
    time_t      now;
    int lockfd;
    int iscreate;

    if( brdshm == NULL ) {
        brdshm = attach_shm( "BCACHE_SHMKEY", 3693, sizeof( *brdshm ) ,&iscreate); /* attach board share memory*/
    }
    numboards = brdshm->number;
    bcache = brdshm->bcache;
    now = time( NULL );

    lockfd = open( "bcache.lock", O_RDWR|O_CREAT, 0600 );

    if( lockfd < 0 ) {
        log( "3CACHE", "reload bcache lock error!!!!" );
        return;
    }

    flock(lockfd,LOCK_EX);

    if( stat( BOARDS, &st ) < 0 ) {
        st.st_mtime = now - 3600;
    }
    if( brdshm->uptime < st.st_mtime || brdshm->uptime < now - 3600 ) { /* 定期update board share memory*/
        log( "1CACHE", "reload bcache" );
        /*brdshm->uptime = now;*/
        numboards = 0;
        apply_record( BOARDS, fillbcache, sizeof(struct boardheader),0 );
        brdshm->number = numboards;
        brdshm->uptime = now;
    }
    flock(lockfd,LOCK_UN);
    close(lockfd);
}

void
resolve_boards()
{
    struct stat st;
    time_t      now;
    int lockfd;
    static int n=0;
    int iscreate;

    if( brdshm == NULL ) {
        brdshm = attach_shm( "BCACHE_SHMKEY", 3693, sizeof( *brdshm ) ,&iscreate); /* attach board share memory*/
    }
    numboards = brdshm->number;
    bcache = brdshm->bcache;
    
    if( brdshm->uptime < 999999 ) {
        reload_boards();
        return;
    }

    n++;
    if(n%50) return;
    
    now = time( NULL );
    if( stat( BOARDS, &st ) < 0 ) {
        st.st_mtime = now - 3600;
    }
    if( brdshm->uptime < st.st_mtime || brdshm->uptime < now - 3600 )
        reload_boards();
}

int
apply_boards(func) /* 对所有版 应用 func函数*/
int (*func)() ;
{
    register int i ;

    resolve_boards();
    for(i=0;i<numboards;i++)
        if( bcache[i].level & PERM_POSTMASK || HAS_PERM( bcache[i].level ) || (bcache[i].level&PERM_NOZAP))
            if((*func)(&bcache[i]) == QUIT)
                return QUIT;
    return 0;
}

struct shortfile *
            getbcache( bname ) /* get board cache, 通过board name */
            char *bname ;
{
    register int i ;

    resolve_boards();
    for(i=0;i<numboards;i++)
        if( bcache[i].level & PERM_POSTMASK || HAS_PERM( bcache[i].level ) || (bcache[i].level&PERM_NOZAP))
            if( !strncasecmp( bname, bcache[i].filename, STRLEN ) )
                return &bcache[i];
    return NULL;
}

int
getbnum( bname ) /* board name --> board No. */
char    *bname;
{
    register int i;

    resolve_boards();/* attach shm*/
    for(i=0;i<numboards;i++)
        if( bcache[i].level & PERM_POSTMASK || HAS_PERM( bcache[i].level )|| (bcache[i].level&PERM_NOZAP))
            if(!strncasecmp( bname, bcache[i].filename, STRLEN ) )
                return i+1 ;
    return 0 ;
}
/*---	added by period		2000-11-07	to be used in postfile	---*/
int
getboardnum( bname ) /* board name --> board No. & not check level */
char    *bname;
{
    register int i;

    resolve_boards();/* attach shm*/
    for(i=0;i<numboards;i++)
        if(!strncasecmp( bname, bcache[i].filename, STRLEN ) )
            return i+1 ;
    return 0 ;
} /*---	---*/

int
haspostperm(bname) /* 判断在 bname版 是否有post权 */
char *bname;
{
    register int i;

#ifdef BBSMAIN
    if(digestmode)
        return 0;
#endif
    /*    if( strcmp( bname, DEFAULTBOARD ) == 0 )  return 1; change by KCN 2000.09.01 */
    if ((i = getbnum(bname)) == 0) return 0;
    if (HAS_PERM(PERM_DENYPOST))
        /*if(!strcmp(bname, "sysop"))
               return 1;*/ /* Leeward 98.05.21 revised by stephen 2000.10.27*/ 
        /* let user denied post right post at Complain*/
    {if (!strcmp(bname, "Complain")) return 1;/* added by stephen 2000.10.27*/
        else if(!strcmp(bname, "sysop"))
            return 1;} /* stephen 2000.10.27 */
    if (!HAS_PERM(PERM_POST)) return 0;
    return (HAS_PERM((bcache[i-1].level&~PERM_NOZAP) & ~PERM_POSTMASK));
}

int
normal_board(bname) /* bname版 是否是 normal level */
char *bname;
{
    register int i;

    if( strcmp( bname, DEFAULTBOARD ) == 0 )  return 1;
    if ((i = getbnum(bname)) == 0) return 0;
    return (bcache[i-1].level==0);
}

