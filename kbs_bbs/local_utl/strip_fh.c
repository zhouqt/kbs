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
	if (strcmp(bname, FILTER_BOARD) == 0 
#ifdef COMMEND_ARTICLE
			|| strcmp(bname, COMMEND_ARTICLE) == 0
#endif
			)
	{
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
	}
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
	fh->accessed[0] = oldfh->accessed[0];
	fh->accessed[1] = oldfh->accessed[1];
	fh->accessed[sizeof(fh->accessed) - 1] = 
								oldfh->accessed[sizeof(oldfh->accessed) - 1];
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

	if (strcmp(bh->filename, SYSMAIL_BOARD)) /* if not SYSMAIL_BOARD */
	{
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
	}

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

static void rollback_board_file(const char *bname, const char *dir)
{
	char dir_path[256];
	char old_dir_path[256];

	setbfile(dir_path, bname, dir);
	sprintf(old_dir_path, "%s.v1.2", dir_path);
	if (dashf(old_dir_path))
		rename(old_dir_path, dir_path);
}

static int rollback_board(struct boardheader * bh, void * arg)
{
	rollback_board_file(bh->filename, ".DIR");
	rollback_board_file(bh->filename, ".DIGEST");
	rollback_board_file(bh->filename, ".DELETED");
	rollback_board_file(bh->filename, ".JUNK");
	rollback_board_file(bh->filename, ".DINGDIR");

	return 0;
}

static void rollback_mail_file(const char *username, const char *dir)
{
	char dir_path[256];
	char old_dir_path[256];

	setmailfile(dir_path, username, dir);
	sprintf(old_dir_path, "%s.v1.2", dir_path);
	if (dashf(old_dir_path))
		rename(old_dir_path, dir_path);
}

static int rollback_mail(struct userec *user, char *arg)
{
	struct _mail_list ml;
	int i;
	char buf[STRLEN];

	if (user == NULL || user->userid[0] == '\0')
		return;
	bzero(&ml, sizeof(ml));
	load_mail_list(user, &ml);
	/* 系统信箱 */
	rollback_mail_file(user->userid, ".DIR");
	rollback_mail_file(user->userid, ".SENT");
	rollback_mail_file(user->userid, ".DELETED");
	/* 自定义信箱 */
	for (i = 0; i < ml.mail_list_t; i++)
	{
		sprintf(buf, ".%s", ml.mail_list[i] + 30);
		rollback_mail_file(user->userid, buf);
	}

	return 0;
}

int main(int argc, char ** argv)
{
    chdir(BBSHOME);
#undef time
    bbssettime(time(0));
    sleep(1);
	resolve_boards();
	load_ucache();
	if (argc == 2 && strcmp(argv[1], "--all") == 0)
	{
		apply_boards(strip_board, NULL);
		apply_users(strip_mail, NULL);
	}
	else if (argc == 2 && strcmp(argv[1], "--rollback") == 0)
	{
		apply_boards(rollback_board, NULL);
		apply_users(rollback_mail, NULL);
	}
	else if (argc == 3 && strcmp(argv[1], "-b") == 0)
	{
		struct boardheader *bh;
		if ((bh = getbcache(argv[2])) == NULL)
		{
			fprintf(stderr, "Board <%s> not found!\n", argv[2]);
			exit(-1);
		}
		strip_board(bh, NULL);
	}
	else if (argc == 3 && strcmp(argv[1], "-m") == 0)
	{
		struct userec *user;
		if (!getuser(argv[2], &user))
		{
			fprintf(stderr, "User <%s> not found!\n", argv[2]);
			exit(-1);
		}
		strip_mail(user, NULL);
	}
	else if (argc == 3 && strcmp(argv[1], "--rollback-board") == 0)
	{
		struct boardheader *bh;
		if ((bh = getbcache(argv[2])) == NULL)
		{
			fprintf(stderr, "Board <%s> not found!\n", argv[2]);
			exit(-1);
		}
		rollback_board(bh, NULL);
	}
	else if (argc == 3 && strcmp(argv[1], "--rollback-mail") == 0)
	{
		struct userec *user;
		if (!getuser(argv[2], &user))
		{
			fprintf(stderr, "User <%s> not found!\n", argv[2]);
			exit(-1);
		}
		rollback_mail(user, NULL);
	}
	else
	{
		fprintf(stderr, "Usage:\n", argv[0]);
		fprintf(stderr, "  %s --all                  Strip all fileheaders.\n", argv[0]);
		fprintf(stderr, "  %s --rollback             Rollback all fileheaders.\n", argv[0]);
		fprintf(stderr, "  %s -b board               Strip fileheaders of a board.\n", argv[0]);
		fprintf(stderr, "  %s -m user                Strip fileheaders of a user's mailbox.\n", argv[0]);
		fprintf(stderr, "  %s --rollback-board board Rollback fileheaders of a board.\n", argv[0]);
		fprintf(stderr, "  %s --rollback-mail user   Rollback fileheaders of a user's mailbox.\n", argv[0]);
		exit(-1);
	}

    return 0;
}
