#include "../bbs.h"

struct UCACHE   *uidshm; /*= NULL*/

void main(argc,argv)
int argc;
char * argv[];
{
    show_online_user();
}
