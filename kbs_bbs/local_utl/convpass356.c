/**
 * This program converts .PASSWDS file from cvs revision 3.56 to revision 3.57.
 * It also converts .userdata file.
 */
#include "bbs.h"

#define OLD_PASSWDS_FILE ".PASSWDS.OLD"
#define NEW_PASSWDS_FILE ".PASSWDS.NEW"
#undef printf
#undef perror

typedef struct userec_r356 {    /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    char flags[2];
    time_t firstlogin;
    char lasthost[16];
    unsigned int numlogins;
    unsigned int numposts;
#ifdef CONV_PASS
    char passwd[OLDPASSLEN];
	char unused_padding[2];
#endif
    char username[NAMELEN];
    unsigned int club_read_rights[MAXCLUB>>5];
    unsigned int club_write_rights[MAXCLUB>>5];
    unsigned char md5passwd[MD5PASSLEN];
    unsigned userlevel;
    time_t lastlogin;
    time_t stay;
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
	unsigned int usedspace;
}userec_r356;

typedef struct userdata_r356 {
    char userid[IDLEN + 2];
	char __reserved[2];
    /*char username[NAMELEN];*/
    char realemail[STRLEN - 16];
    char realname[NAMELEN];
    char address[STRLEN];
    char email[STRLEN];
} userdata_r356;

static void 
convert_userdata(userec_r356 *olduser)
{
	userdata_r356 oldud;
	struct userdata ud;
	char datafile[256];
	int fd;
	struct stat st;

	sethomepath(datafile, olduser->userid);
	if (stat(datafile, &st) == -1)
	{
		fprintf(stderr, "Warning: %s's home directory not found.\n",
				olduser->userid);
		fprintf(stderr, "         Creating it.\n");
		if (mkdir(datafile, 0755) == -1)
			return;
	}
	else
	{
		if (!(st.st_mode & S_IFDIR))
			return;
	}
	sethomefile(datafile, olduser->userid, USERDATA);
	if ((fd = open(datafile, O_RDWR, 0644)) < 0)
	{
		fprintf(stderr, "open %s failed.\n", datafile);
		return;
	}
	if (read(fd, &oldud, sizeof(oldud)) < 0)
		fprintf(stderr, "read %s failed.\n", datafile);
	memcpy(&ud, &oldud, sizeof(oldud));
#ifdef HAVE_BIRTHDAY
	ud.gender = olduser->gender;
	ud.birthyear = olduser->birthyear;
	ud.birthmonth = olduser->birthmonth;
	ud.birthday = olduser->birthday;
#endif
	if (write(fd, &ud, sizeof(ud)) < 0)
		fprintf(stderr, "write %s failed.\n", datafile);
	close(fd);
}

static void
convert_userec(userec_r356 *olduser, struct userec *user)
{
	memcpy(user->userid, olduser->userid, IDLEN + 2);
	memcpy(&(user->flags), olduser->flags, 1);
	user->firstlogin = olduser->firstlogin;
	memcpy(user->lasthost, olduser->lasthost, 16);
	user->numlogins = olduser->numlogins;
	user->numposts = olduser->numposts;
#ifdef CONV_PASS
	memcpy(user->passwd, olduser->passwd, OLDPASSLEN);
#endif
	memcpy(user->username, olduser->username, NAMELEN);
	memcpy(user->club_read_rights, olduser->club_read_rights,
			sizeof(user->club_read_rights));
	memcpy(user->club_write_rights, olduser->club_write_rights,
			sizeof(user->club_write_rights));
	memcpy(user->md5passwd, olduser->md5passwd, sizeof(user->md5passwd));
	user->userlevel = olduser->userlevel;
	user->lastlogin = olduser->lastlogin;
	user->stay = olduser->stay;
	user->signature = olduser->signature;
	user->userdefine[0] = olduser->userdefine;
	user->notedate = olduser->notedate;
	user->noteline = olduser->noteline;
	user->notemode = -1;
	user->exittime = olduser->exittime;
	user->usedspace = olduser->usedspace;
	convert_userdata(olduser);
}

int 
main()
{
	userec_r356 *olduser = NULL;
	userec_r356 *ptr = NULL;
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
	records = fs.st_size / sizeof(struct userec_r356);
	for (i = 0; i < records; i++)
	{
		ptr = olduser + i;
		bzero(&user, sizeof(user));
		if (ptr->userid[0] != '\0')
			convert_userec(ptr, &user);
		write(fd2, &user, sizeof(user));
	}
	close(fd2);
	munmap(olduser, fs.st_size);

	return 0;
}

