#include "bbs.h"

char *curuserid;

int query_BM(struct userec *user, char *arg)
{
    curuserid = user->userid;
    user->usedspace = 0xffff;
    return 1;
}

static void 
usage()
{
	fprintf(stderr, "Usage: resetMailTotal [userid]\n\n");
	fprintf(stderr, "    If userid is missing, this program will reset all userids' mail space,\n");
	fprintf(stderr, "    else only reset the specified userid's mail space.\n");
}

int 
main(int argc, char ** argv)
{
	struct userec *user = NULL;

    chdir(BBSHOME);
    resolve_ucache();
    resolve_boards();
	if (argc == 1)
    	apply_users(query_BM, NULL);
	else if (argc == 2)
	{
		getuser(argv[1], &user);
		if (user == NULL)
		{
			fprintf(stderr, "User %s not found.\n", argv[1]);
			return -1;
		}
		user->usedspace = 0xFFFF;
	}
	else
		usage();

	return 0;
}
