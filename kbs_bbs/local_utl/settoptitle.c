#include "bbs.h"

main()
{
    int n;
    struct boardheader * bptr;
    chdir(BBSHOME);
    resolve_boards();
    for (n = 0; n < get_boardcount(); n++) {
        bptr = (struct boardheader *) getboard(n + 1);
        if (!bptr)
            continue;
        if (*bptr->filename==0)
            continue;
        board_update_toptitle(n + 1, 1);
    }
}

