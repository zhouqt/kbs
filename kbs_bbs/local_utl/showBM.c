#include "bbs.h"

char *curuserid;

int check_BM(struct boardheader *bptr,void* arg)
{
    if ((bptr->level != 0) && !(bptr->level & PERM_POSTMASK))
        return 0;
    if (chk_BM_instr(bptr->BM, curuserid) == true)
        printf("%s(%s) ", bptr->filename, bptr->title + 12);
    return 0;
}

int query_BM(struct userec *user, void *arg)
{
    if (!(user->userlevel & PERM_BOARDS))
        return 0;
    printf("%s: ", user->userid);
    curuserid = user->userid;
    apply_boards(check_BM,NULL);
    printf("\n");
    return 0;
}

int main(void)
{
    chdir(BBSHOME);
    resolve_ucache();
    resolve_boards();
    return apply_users(query_BM, NULL);
}
