/*
 * This program converts home and mail data from Firebird 3.0K beta6 to smth_bbs
 */
#include "bbs.h"
#include "oldstruct30k.h"

#define MY_BBSHOME "/bbs"
#define PASSWDS_FILE ".PASSWDS"
#undef printf
#undef perror

static void 
convert_friend_list(struct userec *user)
{
	char dir[256];
	char dir_bak[256];
	int rfd;
	int wfd;
	struct stat fs;
	struct override oldfr;
	struct friends fr;
	int i;

	sethomefile(dir, user->userid, "friends");
	sethomefile(dir_bak, user->userid, "friends.OLD");
	if (stat(dir, &fs) < 0)
		return;
	rename(dir, dir_bak);
	rfd = open(dir_bak, O_RDONLY, 0644);
	wfd = open(dir, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	i = 0;
	while (i < MAXFRIENDS && read(rfd, &oldfr, sizeof(oldfr)) == sizeof(oldfr))
	{
		strcpy(fr.id, oldfr.id);
		strncpy(fr.exp, oldfr.exp, sizeof(fr.exp)-1);
		fr.exp[sizeof(fr.exp)-1] = '\0';
		write(wfd, &fr, sizeof(fr));
		i++;
	}
	close(rfd);
	close(wfd);
	unlink(dir_bak);
	printf("Converting user %s's friend list successfully\n", user->userid);
}

static void
convert_mail_flag(struct oldfileheader *oldfh, struct fileheader *fh)
{
	char flag;
	char oldflag;

	flag = 0;
	oldflag = oldfh->accessed[0];
	if (oldflag & MY_FILE_READ)
		flag |= FILE_READ;
	if (oldflag & MY_FILE_OWND)
		flag |= FILE_OWND;
	if (oldflag & MY_FILE_VISIT)
		flag |= FILE_VISIT;
	if (oldflag & MY_FILE_MARKED)
		flag |= FILE_MARKED;
	if (oldflag & MY_FILE_DIGEST)
		flag |= FILE_DIGEST;
	fh->accessed[0] = flag;
}

static void 
convert_mail(struct userec *user)
{
	char dir[256];
	struct stat fs;
	int records;
	int i;
	int fd;
	struct oldfileheader *oldfh;
	struct oldfileheader *ptr;
	struct fileheader fh;

	setmailfile(dir, user->userid, ".DIR");
	if ((fd = open(dir, O_RDWR, 0644)) < 0)
	{
		fprintf(stderr, "Converting %s's mailbox failed.\n", user->userid);
		return;
	}
	fstat(fd, &fs);
	records = fs.st_size / sizeof(struct oldfileheader);
	oldfh = mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (oldfh == MAP_FAILED)
	{
		perror("mmap");
		close(fd);
		return;
	}
	for (i = 0; i < records; i++)
	{
		ptr = oldfh + i;
		memcpy(&fh, ptr, sizeof(fh));
		convert_mail_flag(ptr, &fh);
		memcpy(ptr, &fh, sizeof(fh));
	}
	munmap(oldfh, fs.st_size);
	close(fd);
	printf("Converting user %s's mailbox successfully\n", user->userid);
}

static void
convert_user(struct userec *user)
{
	convert_friend_list(user);
	convert_mail(user);
}

int 
main()
{
	struct userec *ptr = NULL;
	struct userec *user;
	int fd;
	struct stat fs;
	int records;
	int i;

	chdir(BBSHOME);
	resolve_boards();
	resolve_ucache();
	resolve_utmp();

	if ((fd = open(PASSWDS_FILE, O_RDONLY, 0644)) < 0)
	{
		perror("open");
		return -1;
	}
	fstat(fd, &fs);
	user = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (user == MAP_FAILED)
	{
		perror("mmap");
		return -1;
	}
	records = fs.st_size / sizeof(struct userec);
	for (i = 0; i < records; i++)
	{
		ptr = user + i;
		if (ptr->userid[0] == '\0')
			continue;
		convert_user(ptr);
		printf("Converting user %s's data successfully\n\n", ptr->userid);
	}
	munmap(user, fs.st_size);

	return 0;
}
