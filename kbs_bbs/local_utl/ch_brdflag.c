#include "bbs.h"

#undef printf
#undef perror
#undef time
#undef fprintf

int main(int argc, char **argv)
{
	struct boardheader fh, newfh;
	int pos;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s --force <board>\n", argv[0]);
		exit(-1);
	}
	if (strcmp(argv[1], "--force"))
	{
		fprintf(stderr, "Usage: %s --force <board>\n", argv[0]);
		exit(-1);
	}
	chdir(BBSHOME);
	resolve_boards();

    pos = getboardnum(argv[2], &fh);
	if (pos == 0)
	{
		fprintf(stderr, "%s not found.\n", argv[2]);
		exit(-1);
	}
	memcpy(&newfh, &fh, sizeof(newfh));
	/*newfh.flag |= BOARD_SUPER_CLUB;*/
	set_board(pos, &newfh, &fh);

	return 0;
}

