#include "bbs.h"

#undef printf
#undef perror

typedef struct fileheader_v1_2 {
    char filename[20];      /* the DIR files */
    unsigned int id, groupid, reid;
#if defined(FILTER) || defined(COMMEND_ARTICLE)
    char o_board[20];
    unsigned int o_id;
    unsigned int o_groupid;
    unsigned int o_reid;
    char unused1[14];
#else
    char unused1[46];
#endif
    char innflag[2];
    char owner[30];
    char unused2[42];
    unsigned int eff_size;

    long attachment;
    char title[80];
    unsigned level;
    unsigned char accessed[12]; /* struct size = 256 bytes */
} fileheader_v1_2;

static void strip_fileheader(const fileheader_v1_2 *oldfh, fileheader *fh,
							const char *bname, const char *dir)
{
	strncpy(fh->filename, oldfh->filename, FILENAME_LEN - 1);
	fh->filename[FILENAME_LEN - 1] = '\0';
	fh->id = oldfh->id;
	fh->groupid = oldfh->groupid;
	fh->reid = oldfh->reid;
#if defined(FILTER) || defined(COMMEND_ARTICLE)
	if (oldfh->o_board[0] != '\0'
		&& (fh->o_bid = getboardnum(oldfh->o_board, NULL)) == 0)
	{
		fprintf(stderr, 
				"Warning: Cannot find original board <%s>: %s/%s/id<%d>.\n",
				oldfh->o_board, bname, dir, oldfh->id);
	}
	fh->o_id = oldfh->o_id;
	fh->o_groupid = oldfh->o_groupid;
	fh->o_reid = oldfh->o_reid;
#endif
	fh->innflag[0] = oldfh->innflag[0];
	fh->innflag[1] = oldfh->innflag[1];
	strncpy(fh->owner, oldfh->owner, OWNER_LEN - 1);
	fh->owner[OWNER_LEN - 1] = '\0';
	fh->eff_size = oldfh->eff_size;
	set_posttime(fh);
	fh->attachment = oldfh->attachment;
	strncpy(fh->title, oldfh->title, ARTICLE_TITLE_LEN - 1);
	fh->title[ARTICLE_TITLE_LEN - 1] = '\0';
	memcpy(fh->accessed, oldfh->accessed, sizeof(fh->accessed));
}

static void strip_mail_fileheader(const fileheader_v1_2 *oldfh, fileheader *fh,
							const char *username, const char *dir)
{
	strncpy(fh->filename, oldfh->filename, FILENAME_LEN - 1);
	fh->filename[FILENAME_LEN - 1] = '\0';
	strncpy(fh->owner, oldfh->owner, OWNER_LEN - 1);
	fh->owner[OWNER_LEN - 1] = '\0';
	fh->attachment = oldfh->attachment;
	strncpy(fh->title, oldfh->title, ARTICLE_TITLE_LEN - 1);
	fh->title[ARTICLE_TITLE_LEN - 1] = '\0';
	memcpy(fh->accessed, oldfh->accessed, sizeof(fh->accessed));
}

static void strip_index_file(const char *bname, const char *dir)
{
	char old_dir_path[256];
	char dir_path[256];
	char buf[256];
	int old_fd;
	int fd;
	fileheader_v1_2 *ptr;
	size_t fsize;

	setbfile(old_dir_path, bname, dir);
	if ((old_fd = open(old_dir_path, O_RDONLY, 0644)) < 0)
	{
		fprintf(stderr, "Warning: %s/%s not found.\n", bname, dir);
		return;
	}
	sprintf(buf, "%s.NEW", dir);
	setbfile(dir_path, bname, buf);
	if ((fd = open(dir_path, O_RDWR | O_CREAT, 0644)) < 0)
	{
		close(old_fd);
		return;
	}
	if (safe_mmapfile_handle(old_fd, PROT_READ, MAP_SHARED, 
				(void **)&ptr, (off_t *)&fsize))
	{
		int i;
		int rec_count;
		fileheader fh;

		rec_count = fsize / sizeof(fileheader_v1_2);
		for (i = 0; i < rec_count; i++)
		{
			bzero(&fh, sizeof(fh));
			strip_fileheader(ptr + i, &fh, bname, dir);
			write(fd, &fh, sizeof(fh));
		}
		end_mmapfile((void*)ptr, fsize, -1);
		close(fd);
		close(old_fd);
		sprintf(buf, "%s.v1.2", old_dir_path);
		rename(old_dir_path, buf);
		rename(dir_path, old_dir_path);
	}
	else
	{
		close(fd);
		close(old_fd);
	}
}

static int strip_board(struct boardheader * bh, void * arg)
{
	char dir_path[256];
	strip_index_file(bh->filename, ".DIR");
	strip_index_file(bh->filename, ".DIGEST");
	strip_index_file(bh->filename, ".DELETED");
	strip_index_file(bh->filename, ".JUNK");
	strip_index_file(bh->filename, ".DINGDIR");
	setbfile(dir_path, bh->filename, ".ORIGIN");
	unlink(dir_path);
	setbfile(dir_path, bh->filename, ".MARK");
	unlink(dir_path);
	setbfile(dir_path, bh->filename, ".THREAD");
	unlink(dir_path);

	return 0;
}

static void strip_mail_index_file(const char *username, const char *dir)
{
	char old_dir_path[256];
	char dir_path[256];
	char buf[256];
	int old_fd;
	int fd;
	fileheader_v1_2 *ptr;
	size_t fsize;

	setmailfile(old_dir_path, username, dir);
	if ((old_fd = open(old_dir_path, O_RDONLY, 0644)) < 0)
	{
		fprintf(stderr, "Warning: maildir %s/%s not found.\n", username, dir);
		return;
	}
	sprintf(buf, "%s.NEW", dir);
	setmailfile(dir_path, username, buf);
	if ((fd = open(dir_path, O_RDWR | O_CREAT, 0644)) < 0)
	{
		close(old_fd);
		return;
	}
	if (safe_mmapfile_handle(old_fd, PROT_READ, MAP_SHARED, 
				(void **)&ptr, (off_t *)&fsize))
	{
		int i;
		int rec_count;
		fileheader fh;

		rec_count = fsize / sizeof(fileheader_v1_2);
		for (i = 0; i < rec_count; i++)
		{
			bzero(&fh, sizeof(fh));
			strip_mail_fileheader(ptr + i, &fh, username, dir);
			write(fd, &fh, sizeof(fh));
		}
		end_mmapfile((void*)ptr, fsize, -1);
		close(fd);
		close(old_fd);
		sprintf(buf, "%s.v1.2", old_dir_path);
		rename(old_dir_path, buf);
		rename(dir_path, old_dir_path);
	}
	else
	{
		close(fd);
		close(old_fd);
	}
}

static int strip_mail(struct userec *user, char *arg)
{
	struct _mail_list ml;
	int i;
	char buf[STRLEN];

	if (user == NULL || user->userid[0] == '\0')
		return;
	bzero(&ml, sizeof(ml));
	load_mail_list(user, &ml);
	/* 系统信箱 */
	strip_mail_index_file(user->userid, ".DIR");
	strip_mail_index_file(user->userid, ".SENT");
	strip_mail_index_file(user->userid, ".DELETED");
	/* 自定义信箱 */
	for (i = 0; i < ml.mail_list_t; i++)
	{
		sprintf(buf, ".%s", ml.mail_list[i] + 30);
		strip_mail_index_file(user->userid, buf);
	}

	return 0;
}

int main(int argc, char ** argv)
{
	/*
	int fd;
	struct stat fs;
	struct boardheader *brd, *ptr;
	int i;
	int records;
	struct userec *usr, *uptr;

    chdir(BBSHOME);
	if ((fd = open(BOARDS, O_RDONLY, 0644)) < 0) {
		perror("open");
		return -1;
	}
	fstat(fd, &fs);
	brd = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (brd == MAP_FAILED) {
		perror("mmap");
		return -1;
	}
	records = fs.st_size / sizeof (struct boardheader);
	for (i = 0; i < records; i++) {
		ptr = brd + i;
		if (ptr->filename[0] != '\0')
			strip_board(ptr, NULL);
	}
	munmap(brd, fs.st_size);

	//apply_users(strip_mail, NULL);
	if ((fd = open(PASSFILE, O_RDONLY, 0644)) < 0) {
		perror("open");
		return -1;
	}
	fstat(fd, &fs);
	usr = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (usr == MAP_FAILED) {
		perror("mmap");
		return -1;
	}
	records = fs.st_size / sizeof (struct userec);
	for (i = 0; i < records; i++) {
		uptr = usr + i;
		if (uptr->userid[0] != '\0')
			strip_mail(uptr, NULL);
	}
	munmap(usr, fs.st_size);
	*/
    chdir(BBSHOME);
	resolve_boards();
	resolve_ucache();
	apply_boards(strip_board, NULL);
	apply_users(strip_mail, NULL);

    return 0;
}
