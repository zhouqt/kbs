#include "bbs.h"

#undef printf
#undef perror
#undef time
#undef fprintf

int main(int argc, char **argv)
{
	struct userec *x = NULL;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <username>\n", argv[0]);
		exit(-1);
	}
	chdir(BBSHOME);
	resolve_ucache();
	resolve_utmp();

	getuser(argv[1], &x);
	if (x == NULL)
	{
		fprintf(stderr, "%s not found.\n", argv[1]);
		exit(-1);
	}
	x->userlevel |= PERM_ADMIN | PERM_SYSOP;

	return 0;
}

