/*
 * This program converts .PASSWD from Firebird 2000 to smth_bbs
 */

#include "bbs.h"

#define OLD_PASSWDS_FILE ".PASSWDS.fb2k"
#define NEW_PASSWDS_FILE ".PASSWDS.NEW"
#undef printf
#undef perror

#define HAVE_BIRTHDAY 1
#define CONV_PASS     1
#define HAVE_USERMONEY 1

typedef struct olduserec {      /* Structure used to hold information in */
    char userid[14];     /* PASSFILE */
    time_t firstlogin;
    char lasthost[16];
    unsigned int numlogins;
    unsigned int numposts;
    unsigned int nummedals;        /* 奖章数 */
    unsigned int money;         /* 金钱 */
    unsigned int bet;        /* 存款 */
    time_t dateforbet;            /* 存入时间 */
    char flags[2];
    char passwd[35];
    char username[40];
    char ident[40];
    char termtype[16];
    char reginfo[STRLEN - 16];
    unsigned userlevel;
    time_t lastlogin;
    time_t lastlogout;          /* 最近离线时间 */
    time_t stay;
    char realname[40];
    char address[STRLEN];
    char email[STRLEN - 12];
    unsigned int nummails;
    time_t lastjustify;
    char gender;
    unsigned char birthyear;
    unsigned char birthmonth;
    unsigned char birthday;
    int signature;
    unsigned int userdefine;
    time_t notedate;
    int noteline;
    unsigned int userdefine1;
} olduserec;

struct override_old{
		char id[13];
		char exp[40];
};

static void conv_friend(char *userid)
{
	FILE *fp,*fp1;
	char buf[256],buf1[256],buf2[256];
	struct override_old oldf;
	struct friends newf;

	sethomefile(buf,userid,"friends");
	sethomefile(buf1,userid,"friends.new");
	sethomefile(buf2,userid,"friends.old");

	if((fp=fopen(buf,"r"))==NULL)
		return;

	if((fp1=fopen(buf1,"w"))==NULL){
		fclose(fp1);
		return;
	}

	while(fread(&oldf, sizeof(oldf),1, fp)){
		bzero(&newf, sizeof(newf));
		strcpy(newf.id, oldf.id);
		newf.id[12]=0;
		fwrite(&newf, sizeof(newf),1,fp1);
	}
	fclose(fp);
	fclose(fp1);

	rename(buf,buf2);
	rename(buf1,buf);

}

static void create_userdata(olduserec * olduser)
{
    struct userdata ud;
    char datafile[256];
    int fd;
    struct stat st;

    bzero(&ud, sizeof(ud));
    memcpy(ud.userid, olduser->userid, sizeof(ud.userid));
    memcpy(ud.realemail, olduser->reginfo, sizeof(ud.realemail));
    memcpy(ud.realname, olduser->realname, sizeof(ud.realname));
    memcpy(ud.address, olduser->address, sizeof(ud.address));
    memcpy(ud.email, olduser->email, sizeof(ud.email));
#ifdef HAVE_BIRTHDAY
    ud.gender = olduser->gender;
    ud.birthyear = olduser->birthyear;
    ud.birthmonth = olduser->birthmonth;
    ud.birthday = olduser->birthday;
#endif

    sethomepath(datafile, olduser->userid);
    if (stat(datafile, &st) == -1) {
        fprintf(stderr, "Warning: %s's home directory not found.\n", olduser->userid);
        fprintf(stderr, "         Creating it.\n");
        if (mkdir(datafile, 0755) == -1)
            return;
    } else {
        if (!(st.st_mode & S_IFDIR))
            return;
    }
    sethomefile(datafile, olduser->userid, USERDATA);
    if ((fd = open(datafile, O_WRONLY | O_CREAT, 0644)) < 0) {
        fprintf(stderr, "open %s failed.\n", datafile);
        return;
    }
    if (write(fd, &ud, sizeof(ud)) < 0)
        fprintf(stderr, "write %s failed.\n", datafile);
    close(fd);
}

static void convert_userec(struct olduserec *olduser, struct userec *user)
{
    memcpy(user->userid, olduser->userid, IDLEN + 2);
    printf("Tranfering %s ....\n", user->userid);

    user->flags = olduser->flags[0];
    user->flags |= CURSOR_FLAG;
/*
    if (user->userlevel & 0x0400000000)
	user->flags |= PCORP_FLAG;
    user->flags &= ~CLOAK_FLAG;
    user->flags &= ~GIVEUP_FLAG;
*/
    
    user->title = 0;
    user->firstlogin = olduser->firstlogin;
    memcpy(user->lasthost, olduser->lasthost, 16);
    user->numlogins = olduser->numlogins;
    user->numposts = olduser->numposts;
#ifdef CONV_PASS
    memcpy(user->passwd, olduser->passwd, 35);
    bzero(user->unused_padding, 2);
#endif
    memcpy(user->username, olduser->username, NAMELEN);
    bzero(user->club_read_rights, sizeof(user->club_read_rights));
    bzero(user->club_write_rights, sizeof(user->club_write_rights));
    bzero(user->md5passwd, sizeof(user->md5passwd));
    user->userlevel = olduser->userlevel;

    user->userlevel &= ~PERM_BMAMANGER;
    user->userlevel &= ~PERM_DENYRELAX;
    user->userlevel &= ~PERM_NOZAP;
    user->userlevel &= ~PERM_CHATOP;
    user->userlevel &= ~PERM_ADMIN;
    user->userlevel &= ~PERM_HORNOR;
    user->userlevel &= ~PERM_JURY;
    user->userlevel &= ~PERM_CHECKCD;
    user->userlevel &= ~PERM_SUICIDE;
    user->userlevel &= ~PERM_COLLECTIVE;
    user->userlevel &= ~PERM_DISS;
	if(olduser->userlevel & 040000)
    	user->userlevel |= PERM_ADMIN;
	if(!strcmp(user->userid,"stiger")){
    	user->userlevel |= PERM_SYSOP;
    	user->userlevel |= PERM_ADMIN;
	}
	if(olduser->userlevel & 02000000000)
    	user->userlevel &= ~PERM_DENYMAIL;
	else
    	user->userlevel |= PERM_DENYMAIL;

	if(olduser->userlevel & 0400000000)
		user->flags |= PCORP_FLAG;
	else
		user->flags &= ~PCORP_FLAG;

    user->lastlogin = olduser->lastlogin;
    user->stay = olduser->stay;
    user->signature = olduser->signature;
    user->userdefine[0] = olduser->userdefine;
 
	if(olduser->userdefine & 0x00000004)
		user->userdefine[0] |= DEF_FRIENDMSG;
	else
		user->userdefine[0] &= ~DEF_FRIENDMSG;
	if(olduser->userdefine & 0x00000002)
		user->userdefine[0] |= DEF_ALLMSG;
	else
		user->userdefine[0] &= ~DEF_ALLMSG;
	if(olduser->userdefine & 0x00000008)
		user->userdefine[0] |= DEF_SOUNDMSG;
	else
		user->userdefine[0] &= ~DEF_SOUNDMSG;
	if(olduser->userdefine & 0x00000010)
		user->userdefine[0] |= DEF_COLOR;
	else
		user->userdefine[0] &= ~DEF_COLOR;
	if(olduser->userdefine & 0x00000020)
		user->userdefine[0] |= DEF_ACBOARD;
	else
		user->userdefine[0] &= ~DEF_ACBOARD;
	if(olduser->userdefine & 0x00000040)
		user->userdefine[0] |= DEF_ENDLINE;
	else
		user->userdefine[0] &= ~DEF_ENDLINE;
	if(olduser->userdefine & 0x00000080)
		user->userdefine[0] |= DEF_EDITMSG;
	else
		user->userdefine[0] &= ~DEF_EDITMSG;
	if(olduser->userdefine & 0x00000200)
		user->userdefine[0] |= DEF_NORMALSCR;
	else
		user->userdefine[0] &= ~DEF_NORMALSCR;
	if(olduser->userdefine & 0x00000400)
		user->userdefine[0] |= DEF_NEWPOST;
	else
		user->userdefine[0] &= ~DEF_NEWPOST;
	if(olduser->userdefine & 0x00000001)
		user->userdefine[0] |= DEF_FRIENDCALL;
	else
		user->userdefine[0] &= ~DEF_FRIENDCALL;
	if(olduser->userdefine1 & 0x00000004)
		user->userdefine[0] |= DEF_IGNOREMSG;
	else
		user->userdefine[0] &= ~DEF_IGNOREMSG;
	if(olduser->userdefine1 & 0x00000001)
		user->userdefine[0] |= DEF_MAILMSG;
	else
		user->userdefine[0] &= ~DEF_MAILMSG;
	if(olduser->userdefine & 0x00200000)
		user->signature = -1;
	if(olduser->userdefine & 0x02000000)
		user->userdefine[0] |= DEF_CHCHAR;
	else
		user->userdefine[0] &= ~DEF_CHCHAR;
	if(olduser->userdefine & 0x10000000)
		user->userdefine[1] &= ~DEF_HIDEIP;
	else
		user->userdefine[1] |= DEF_HIDEIP;
	if(olduser->userdefine & 0x01000000)
		user->userdefine[0] |= DEF_UNREADMARK;
	else
		user->userdefine[0] &= ~DEF_UNREADMARK;
	if(olduser->userdefine & 0x00400000)
    	user->userdefine[0] |= DEF_SHOWDETAILUSERDATA;
	else
    	user->userdefine[0] &= ~DEF_SHOWDETAILUSERDATA;
	if(olduser->userdefine & 0x40000000)
		user->userdefine[1] &= ~DEF_AUTOREMAIL;
	else
		user->userdefine[1] |= DEF_AUTOREMAIL;

    user->userdefine[0] |= DEF_LOGININFORM;
    user->userdefine[0] |= DEF_CIRCLE;
    user->userdefine[0] |= DEF_SHOWSCREEN;
    user->userdefine[0] |= DEF_TITLECOLOR;
	user->userdefine[0] &= ~DEF_LOGOUT;
	user->userdefine[0] &= ~DEF_INNOTE;
	user->userdefine[0] &= ~DEF_OUTNOTE;
	user->userdefine[0] &= ~DEF_HIGHCOLOR;
	user->userdefine[0] &= ~DEF_NOTMSGFRIEND;
    user->userdefine[0] &= ~DEF_SHOWREALUSERDATA;
    
    user->notedate = olduser->notedate;
    user->noteline = olduser->noteline;
    user->notemode = -1;
    user->exittime = olduser->lastlogout;
    user->usedspace = 0;
#ifdef HAVE_USERMONEY
    user->money = olduser->money;
    user->score = olduser->nummedals;
#endif
    if (strcasecmp(user->userid, "new") == 0)
        return;
    create_userdata(olduser);
	conv_friend(user->userid);
	{
		char dkey[256];
    	sethomefile(dkey, user->userid, "definekey");
//		f_cp("service/definekey.free", dkey, 0);
	}
}

int main()
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

    if ((fd = open(OLD_PASSWDS_FILE, O_RDONLY, 0644)) < 0) {
        perror("open");
        return -1;
    }
    if ((fd2 = open(NEW_PASSWDS_FILE, O_WRONLY | O_CREAT, 0644)) < 0) {
        perror("open");
        return -1;
    }
    fstat(fd, &fs);
    olduser = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (olduser == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    records = fs.st_size / sizeof(struct olduserec);
    for (i = 0; i < records; i++) {
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
