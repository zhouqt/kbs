#include "bbs.h"

#define OLD_PASSWDS_FILE ".PASSWDS.OLD"
#define NEW_PASSWDS_FILE ".PASSWDS.NEW"
#undef printf
#undef perror

struct olduserec {                 /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    /*char fill[30];*/
    time_t firstlogin;
    char lasthost[16];
    unsigned int numlogins;
    unsigned int numposts;
    char flags[2];
#ifdef CONV_PASS
    char passwd[OLDPASSLEN];
#endif
    char username[NAMELEN];
    char ident[NAMELEN];
    unsigned char md5passwd[MD5PASSLEN];
    char realemail[STRLEN - 16];
    unsigned userlevel;
    time_t lastlogin;
    time_t stay;
    char realname[NAMELEN];
    char address[STRLEN];
    char email[STRLEN];
    int signature;
    unsigned int userdefine;
    time_t notedate;
    int noteline;
    int notemode;
    time_t exittime;
#ifdef HAVE_BIRTHDAY
	char            gender;
	unsigned char   birthyear;
	unsigned char   birthmonth;
	unsigned char   birthday;
#endif
};

static void
convert_userec(struct olduserec *olduser, struct userec *user)
{
	memcpy(user->userid, olduser->userid, IDLEN + 2);
	user->firstlogin = olduser->firstlogin;
	memcpy(user->lasthost, olduser->lasthost, 16);
	user->numlogins = olduser->numlogins;
	user->numposts = olduser->numposts;
	memcpy(user->flags, olduser->flags, 2);
#ifdef CONV_PASS
	memcpy(user->passwd, olduser->passwd, PASSLEN);
#endif
	memcpy(user->username, olduser->username, NAMELEN);
	/*memcpy(user->ident, olduser->ident, NAMELEN);*/
	memcpy(user->md5passwd, olduser->md5passwd, sizeof(user->md5passwd));
	memcpy(user->realemail, olduser->realemail, sizeof(user->realemail));
	user->userlevel = olduser->userlevel;
	user->lastlogin = olduser->lastlogin;
	user->stay = olduser->stay;
	memcpy(user->realname, olduser->realname, NAMELEN);
	memcpy(user->address, olduser->address, STRLEN);
	memcpy(user->email, olduser->email, STRLEN - 12);
	user->signature = olduser->signature;
	user->userdefine = olduser->userdefine;
	user->notedate = olduser->notedate;
	user->noteline = olduser->noteline;
	user->notemode = -1;
	user->exittime = 0;
#ifdef HAVE_BIRTHDAY
	user->gender = olduser->gender;
	user->birthyear = olduser->birthyear;
	user->birthmonth = olduser->birthmonth;
	user->birthday = olduser->birthday;
#endif
}

int 
main()
{
	struct olduserec *olduser = NULL;
	struct olduserec *ptr = NULL;
	struct userec user;
	int fd, fd2;
	struct stat fs;
	int records;
	int i;
	int allocid;

	chdir(BBSHOME);

	if ((fd = open(OLD_PASSWDS_FILE, O_RDONLY, 0644)) < 0)
	{
		perror("open");
		return -1;
	}
	if ((fd2 = open(NEW_PASSWDS_FILE, O_WRONLY | O_CREAT, 0644)) < 0)
	{
		perror("open");
		return -1;
	}
	fstat(fd, &fs);
	olduser = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (olduser == MAP_FAILED)
	{
		perror("mmap");
		return -1;
	}
	records = fs.st_size / sizeof(struct olduserec);
	for (i = 0; i < records; i++)
	{
		ptr = olduser + i;
		if (ptr->userid[0] == '\0')
			continue;
		bzero(&user, sizeof(user));
		convert_userec(ptr, &user);
		write(fd2, &user, sizeof(user));
	}
	close(fd2);
	munmap(olduser, fs.st_size);

	return 0;
}

