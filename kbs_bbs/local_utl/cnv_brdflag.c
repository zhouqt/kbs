#include "bbs.h"

#define BOARDS_FILE ".BOARDS"

typedef struct boardheader_r340 { /* This structure is used to hold data in */
    char filename[STRLEN];      /* the BOARDS files */
    char unused1[STRLEN - BM_LEN];
    char BM[BM_LEN - 1];
    char flag;
    char title[STRLEN];
    unsigned level;
    unsigned int nowid;
    unsigned int clubnum; /*如果是俱乐部，这个是俱乐部序号*/
    unsigned char unused[4];
}brdhdr_r340;

int main()
{
	struct boardheader *brd = NULL;
	struct boardheader *ptr = NULL;
	int i;
	int fd;
	struct stat fs;
	int records;

	chdir(BBSHOME);
	if ((fd = open(BOARDS_FILE, O_RDWR, 0644)) < 0)
	{
		perror("open");
		return -1;
	}
	fstat(fd, &fs);
	brd = mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if (brd == MAP_FAILED)
	{
		perror("mmap");
		return -1;
	}
	records = fs.st_size / sizeof(struct boardheader);
	for (i = 0; i < records; i++)
	{
		ptr = brd + i;
/*		ptr->flag = ptr->unused_flag;
		ptr->unused_flag = 0;*/       /*---disable by binxun */
	}
	munmap(brd, fs.st_size);

	return 0;
}

