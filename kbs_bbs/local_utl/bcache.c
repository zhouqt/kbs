/* share memory 处理 
                            bcache:版 , ucache:所有注册用户, utmp cache:在线user 
*/


#include "../bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#define chartoupper(c)  ((c >= 'a' && c <= 'z') ? c+'A'-'a' : c)

struct BCACHE   *brdshm;
struct UCACHE   *uidshm;
struct UTMPFILE *utmpshm;
struct userec lookupuser;
struct shortfile *bcache;
int     usernumber;
int     numboards = -1 ;

int
sysconf_eval( key )
char    *key;
{
    return( strtol( key, NULL, 0 ) );
}

void *
attach_shm( shmstr, defaultkey, shmsize )  /* attach share memory */
char    *shmstr;
int     defaultkey, shmsize;
{
    void        *shmptr;
    int         shmkey, shmid;

    shmkey = sysconf_eval( shmstr );
    if( shmkey < 1024 )
        shmkey = defaultkey;
    shmid = shmget( shmkey, shmsize, 0 );
    if( shmid < 0 ) {
        shmid = shmget( shmkey, shmsize, IPC_CREAT | 0660 ); 
        shmptr = (void *) shmat( shmid, NULL, 0 );
        memset( shmptr, 0, shmsize );
    } else {
        shmptr = (void *) shmat( shmid, NULL, 0 );
    }
    return shmptr;
}


void
resolve_utmp()
{
    if( utmpshm == NULL ) {
        utmpshm = attach_shm( "UTMP_SHMKEY", 3699, sizeof( *utmpshm ) );/*attach user tmp cache */
    }
}

void show_online_user()
{
    int i;
    if( uidshm == NULL ) {
       uidshm = attach_shm( "UTMP_SHMKEY", 3696, sizeof( *uidshm ) ); 
       /* uidshm = attach_shm( "UCACHE_SHMKEY", 3696, sizeof( *uidshm ) );*/ 
/*attach to user shm */
    }
    printf("%d",uidshm->number);

    for(i=0; i < uidshm->number; i++)
        printf("%s\n",uidshm->userid[i]);

}

