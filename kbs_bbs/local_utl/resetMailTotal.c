#include "bbs.h"

char *curuserid;

int query_BM(struct userec *user, char *arg)
{
    curuserid = user->userid;
    user->usedspace = 0xffff;
    return 1;
}

main(int argc, char ** argv)
{
    chdir(BBSHOME);
    resolve_ucache();
    resolve_boards();
    apply_users(query_BM, NULL);
}
