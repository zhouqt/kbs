#include "bbs.h"

struct UCACHE   *uidshm; /*= NULL*/

void show_online_user()
{
    if( uidshm == NULL ) {
        uidshm = attach_shm( "UCACHE_SHMKEY", 7912, sizeof( *uidshm ) );
        if( uidshm !=NULL) {
  		printf("%d",uidshm->number); 
	}
    }
}

void main(argc,argv)
int argc;
char * argv[];
{
    show_online_user();
}
