/**
 * This program converts .PASSWDS file from Firebird 3.0K beta6 to SMTH_BBS.
*/

#include "bbs.h"
#include "oldstruct30k.h"

#define MY_BBSHOME "/home/smth"
#define PASSWDS_FILE ".PASSWDS.OLD"
#undef printf
#undef perror

static void
convert_permission(struct olduserec *olduser, struct userec *user)
{
	unsigned int oldperm;
	unsigned int perm;

	oldperm = olduser->userlevel;
	perm = 0;
	if (oldperm & MY_PERM_DENYPOST)
		perm &= ~PERM_UNUSE;
	if (oldperm & MY_PERM_CHATCLOAK)
		perm &= ~PERM_CHATCLOAK;	/* drop it */
	if (oldperm & MY_PERM_FORCEPAGE)
		perm &= ~MY_PERM_FORCEPAGE;	/* drop it */
	if (oldperm & MY_PERM_EXT_IDLE)
		perm &= ~MY_PERM_EXT_IDLE;	/* drop it */
	if (oldperm & MY_PERM_SPECIAL1)
		perm &= ~MY_PERM_SPECIAL1;	/* drop it */
	if (oldperm & MY_PERM_BASIC)
		perm |= PERM_BASIC;
	if (oldperm & MY_PERM_CHAT)
		perm |= PERM_CHAT;
	if (oldperm & MY_PERM_PAGE)
		perm |= PERM_PAGE;
	if (oldperm & MY_PERM_POST)
		perm |= PERM_POST;
	if (oldperm & MY_PERM_LOGINOK)
		perm |= PERM_LOGINOK;
	if (oldperm & MY_PERM_CLOAK)
		perm |= PERM_CLOAK;
	if (oldperm & MY_PERM_SEECLOAK)
		perm |= PERM_SEECLOAK;
	if (oldperm & MY_PERM_XEMPT)
		perm |= PERM_XEMPT;
	if (oldperm & MY_PERM_WELCOME)
		perm |= PERM_WELCOME;
	if (oldperm & MY_PERM_BOARDS)
		perm |= PERM_BOARDS;
	if (oldperm & MY_PERM_ACCOUNTS)
		perm |= PERM_ACCOUNTS;
	if (oldperm & MY_PERM_OVOTE)
		perm |= PERM_OVOTE;
	if (oldperm & MY_PERM_SYSOP)
		perm |= PERM_ADMIN;			/* convert it */
	if (oldperm & MY_PERM_POSTMASK)
		perm |= PERM_POSTMASK;
	if (oldperm & MY_PERM_ANNOUNCE)
		perm |= PERM_ANNOUNCE;
	if (oldperm & MY_PERM_OBOARDS)
		perm |= PERM_OBOARDS;
	if (oldperm & MY_PERM_ACBOARD)
		perm |= PERM_ACBOARD;
	if (oldperm & MY_PERM_NOZAP)
		perm |= PERM_NOZAP;
	if (oldperm & MY_PERM_SPECIAL2)
		perm |= PERM_CHATOP;	/* convert it */
	if (oldperm & MY_PERM_SPECIAL4)
		perm |= PERM_SYSOP;		/* convert it */
	user->userlevel = perm;
}

static void
convert_userdefine(struct olduserec *olduser, struct userec *user)
{
	unsigned int oldudef;
	unsigned int udef;

	oldudef = olduser->userdefine;
	udef ^= udef;
	udef = ~udef;
	if (!(oldudef & MY_DEF_FRIENDCALL))
		udef &= ~DEF_FRIENDCALL;
	if (!(oldudef & MY_DEF_ALLMSG))
		udef &= ~DEF_ALLMSG;
	if (!(oldudef & MY_DEF_FRIENDMSG))
		udef &= ~DEF_FRIENDMSG;
	if (!(oldudef & MY_DEF_SOUNDMSG))
		udef &= ~DEF_SOUNDMSG;
	if (!(oldudef & MY_DEF_COLOR))
		udef &= ~DEF_COLOR;
	if (!(oldudef & MY_DEF_ENDLINE))
		udef &= ~DEF_ENDLINE;
	if (!(oldudef & MY_DEF_EDITMSG))
		udef &= ~DEF_EDITMSG;
	if (!(oldudef & MY_DEF_NOTMSGFRIEND))
		udef &= ~DEF_NOTMSGFRIEND;
	if (!(oldudef & MY_DEF_NORMALSCR))
		udef &= ~DEF_NORMALSCR;
	if (!(oldudef & MY_DEF_NEWPOST))
		udef &= ~DEF_NEWPOST;
	if (!(oldudef & MY_DEF_CIRCLE))
		udef &= ~DEF_CIRCLE;
	if (!(oldudef & MY_DEF_FIRSTNEW))
		udef &= ~DEF_FIRSTNEW;
	if (!(oldudef & MY_DEF_LOGFRIEND))
		udef &= ~DEF_LOGFRIEND;
	if (!(oldudef & MY_DEF_INNOTE))
		udef &= ~DEF_INNOTE;
	if (!(oldudef & MY_DEF_OUTNOTE))
		udef &= ~DEF_OUTNOTE;
	if (!(oldudef & MY_DEF_MAILMSG))
		udef &= ~DEF_MAILMSG;
	if (!(oldudef & MY_DEF_LOGOUT))
		udef &= ~DEF_LOGOUT;
	if (!(oldudef & MY_DEF_NOTEPAD))
		udef &= ~DEF_NOTEPAD;
	if (!(oldudef & MY_DEF_DELDBLCHAR))
		udef &= ~DEF_CHCHAR;
	if (!(oldudef & MY_DEF_USEGB))
		udef &= ~DEF_USEGB;
	user->userdefine = udef;
}

static void
convert_userec(struct olduserec *olduser, struct userec *user)
{
	memcpy(user->userid, olduser->userid, IDLEN + 2);
	user->firstlogin = olduser->firstlogin;
	memcpy(user->lasthost, olduser->lasthost, 16);
	user->numlogins = olduser->numlogins;
	user->numposts = olduser->numposts;
	memcpy(user->flags, olduser->flags, 2);
	memcpy(user->passwd, olduser->passwd, MYPASSLEN);
	memcpy(user->username, olduser->username, NAMELEN);
	memcpy(user->ident, olduser->ident, NAMELEN);
	bzero(user->md5passwd, MD5PASSLEN);
//	memcpy(user->realemail, olduser->reginfo, STRLEN - 16);
	convert_permission(olduser, user);
	user->lastlogin = olduser->lastlogin;
	user->stay = olduser->stay;
	memcpy(user->realname, olduser->realname, NAMELEN);
	memcpy(user->address, olduser->address, STRLEN);
	memcpy(user->email, olduser->email, STRLEN);
	user->signature = olduser->signature;
	convert_userdefine(olduser, user);
	user->notedate = olduser->notedate;
	user->noteline = olduser->noteline;
	user->notemode = olduser->notemode;
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
	int fd;
	struct stat fs;
	int records;
	int i;
	int allocid;

	chdir(BBSHOME);
	resolve_ucache();
	resolve_utmp();

	chdir(MY_BBSHOME);
	if ((fd = open(PASSWDS_FILE, O_RDONLY, 0644)) < 0)
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
		allocid = getnewuserid2(user.userid);
		if (allocid > MAXUSERS || allocid <= 0)
		{
			fprintf(stderr, "New user %s failed\n", user.userid);
			continue;
		}
		bbslog("1system", "APPLY: uid %d for %s", allocid, user.userid);
	    update_user(&user, allocid, 1);
		if (!searchuser(user.userid))
		{
			fprintf(stderr, "User failed to create %d-%s\n",
					allocid, user.userid);
		}
		else
		{
			printf("Creating user %d-%s successfully\n",
					allocid, user.userid);
		}
	}
	munmap(olduser, fs.st_size);

	return 0;
}

