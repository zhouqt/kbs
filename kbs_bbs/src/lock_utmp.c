#include "bbs.h"
char ULIST[STRLEN] ;
char genbuf[1024] ;

main()
{
    static int utmpfd;
    struct flock fl;
    fl.l_type = F_WRLCK;
    //    fl.l_whence = SEEK_SET;
    fl.l_whence = 0;
    fl.l_start = 0;
    fl.l_len = 0;

    gethostname( genbuf ,256 );
    sprintf( ULIST, "%s.%s", ULIST_BASE, genbuf );

    if( utmpfd == 0 ) {
        utmpfd = open( ULIST, O_RDWR|O_CREAT, 0600 );
        if( utmpfd < 0 )
            return -1;
    }
    flock( utmpfd, LOCK_EX );/* unlock */
    printf("lock\n");
    sleep(10);
    flock( utmpfd, LOCK_UN );/* unlock */
    printf("unlock\n");
    close(utmpfd);
}
