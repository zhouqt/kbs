#include "bbs.h"

#undef perror
#undef printf

static msglist_t *msgshm = NULL;
static msglist_t *ml = NULL;

int main()
{
    int iscreate;
    struct user_info *ui;
    int i;

    iscreate = 0;
    msgshm = (msglist_t *) attach_shm("MSG_SHMKEY", 5200, sizeof(msglist_t) * WWW_MAX_LOGIN, &iscreate);
    if (iscreate) {
        fprintf(stderr, "WARNING: Created a new message list shared memory!\n");
        fprintf(stderr, "         Not necesary to fix message list.\n");
        fprintf(stderr, "         Exiting...");
        exit(-1);
    }
    resolve_utmp();
    for (i = 0; i < WWW_MAX_LOGIN; i++) {
        ml = msgshm + i;
        if (ml->utmpnum == 0)
            continue;
        if (ml->utmpnum < 0 || ml->utmpnum > USHM_SIZE) {
            fprintf(stdout, "faint! %d:%s:%d\n", i, ml->userid, ml->utmpnum);
            continue;
            ml->utmpnum = 0;
            ml->userid[0] = '\0';
        }
        ui = get_utmpent(ml->utmpnum);
        if (strcasecmp(ml->userid, ui->userid)) {
            fprintf(stdout, "Free %d:%s:%d\n", i, ml->userid, ml->utmpnum);
            ml->utmpnum = 0;
            ml->userid[0] = '\0';
        }
    }

    return 0;
}
