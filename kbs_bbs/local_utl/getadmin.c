#include "bbs.h"

#undef printf
#undef perror
#undef time
#undef fprintf

int main(int argc, char **argv)
{
	struct userec *x = NULL;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s --force <username>\n", argv[0]);
		exit(-1);
	}
	if (strcmp(argv[1], "--force"))
	{
		fprintf(stderr, "Usage: %s --force <username>\n", argv[0]);
		exit(-1);
	}
	chdir(BBSHOME);
	resolve_ucache();
	resolve_utmp();

	getuser(argv[2], &x);
	if (x == NULL)
	{
		fprintf(stderr, "%s not found.\n", argv[2]);
		exit(-1);
	}
	x->userlevel |= PERM_ADMIN | PERM_SYSOP;

	return 0;
}

