#include "bbs.h"

#undef perror
#undef printf

int main()
{
	int iscreate;
	msglist_t *msgshm;
	msglist_t *ml;
	struct user_info *ui;
	int i;

	iscreate = 0;
	msgshm = attach_shm("MSG_SHMKEY", 5200, sizeof(msglist_t)*WWW_MAX_LOGIN,
			&iscreate);
	if (iscreate)
	{
		fprintf(stderr, "WARNING: Created a new message list shared memory!\n");
		fprintf(stderr, "         Not necesary to fix message list.\n");
		fprintf(stderr, "         Exiting...");
		exit(-1);
	}
	resolve_utmp();
	for (i = 0; i < WWW_MAX_LOGIN; i++)
	{
		ml = msgshm + i;
		ui = get_utmpent(ml->utmpnum);
		if (ui == NULL || strcasecmp(ml->userid, ui->userid))
		{
			ml->utmpnum = 0;
			ml->userid[0] = '\0';
			fprintf(stdout, "Free %d:%s:%d\n",
					i, ml->userid, ml->utmpnum);
		}
	}

	return 0;
}
