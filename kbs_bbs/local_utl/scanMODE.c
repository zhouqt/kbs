#include "bbs.h"

int mode=0,count=0;

int check(struct user_info *uentp, char *u)
{
    if (uentp->mode==mode) count++;
    return 0;
}

int main(int argc, char **argv)
{
    if (argc<=1) return 0;
    mode = atoi(argv[1]);
    chdir(BBSHOME);
    resolve_boards();
    resolve_ucache();
    resolve_utmp();
    apply_ulist_addr((APPLY_UTMP_FUNC)check,NULL);
    printf("%s: %d\n", ModeType(mode), count);
    return 0;
}

