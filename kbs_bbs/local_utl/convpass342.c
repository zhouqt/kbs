/**
 * This program converts .PASSWDS file from cvs revision 3.41 to revision 3.42.
 */
#include "bbs.h"

#define OLD_PASSWDS_FILE ".PASSWDS.OLD"
#define NEW_PASSWDS_FILE ".PASSWDS.NEW"
#undef printf
#undef perror

typedef struct userec_r341 {    /* Structure used to hold information in */
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
    unsigned int club_read_rights[MAXCLUB>>5];
	unsigned int club_write_rights[MAXCLUB>>5];
#ifndef CONV_PASS
	char unused2[6];
#else
	char unused2[NAMELEN - (MAXCLUB>>5) * sizeof(unsigned int) * 2];
#endif
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
	unsigned int usedspace;
}userec_r341;

static void 
create_userdata(userec_r341 *olduser)
{
	struct userdata ud;
	char datafile[256];
	int fd;
	struct stat st;

	bzero(&ud, sizeof(ud));
	memcpy(ud.userid, olduser->userid, sizeof(ud.userid));
	memcpy(ud.realemail, olduser->realemail, sizeof(ud.realemail));
	memcpy(ud.realname, olduser->realname, sizeof(ud.realname));
	memcpy(ud.address, olduser->address, sizeof(ud.address));
	memcpy(ud.email, olduser->email, sizeof(ud.email));
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
	if ((fd = open(datafile, O_WRONLY | O_CREAT, 0644)) < 0)
	{
		fprintf(stderr, "open %s failed.\n", datafile);
		return;
	}
	if (write(fd, &ud, sizeof(ud)) < 0)
		fprintf(stderr, "write %s failed.\n", datafile);
	close(fd);
}

static void
convert_userec(userec_r341 *olduser, struct userec *user)
{
	memcpy(user->userid, olduser->userid, IDLEN + 2);
	memcpy(user->flags, olduser->flags, 2);
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
	user->userdefine = olduser->userdefine;
	user->notedate = olduser->notedate;
	user->noteline = olduser->noteline;
	user->notemode = -1;
	user->exittime = olduser->exittime;
#ifdef HAVE_BIRTHDAY
	user->gender = olduser->gender;
	user->birthyear = olduser->birthyear;
	user->birthmonth = olduser->birthmonth;
	user->birthday = olduser->birthday;
#endif
	user->usedspace = olduser->usedspace;
	create_userdata(olduser);
}

int 
main()
{
	userec_r341 *olduser = NULL;
	userec_r341 *ptr = NULL;
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
	records = fs.st_size / sizeof(struct userec_r341);
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

