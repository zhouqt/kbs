#ifndef __STRUCT_H__
#define __STRUCT_H__
/* Note the protocol field is not inside an #ifdef FILES...
   this is a waste but allows you to add/remove UL/DL support without
   rebuilding the PASSWDS file (and it's only a lil ole int anyway).
*/
struct userec {                 /* Structure used to hold information in */
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
    unsigned int usedspace;     /* used space of user's mailbox, in bytes */
};

struct userdata
{
    char userid[IDLEN + 2];
	char __reserved[2];
    /*char username[NAMELEN];*/
    char realemail[STRLEN - 16];
    char realname[NAMELEN];
    char address[STRLEN];
    char email[STRLEN];
};

struct user_info {              /* Structure used in UTMP file */
    int active;                 /* When allocated this field is true */
    int uid;                    /* Used to find user name in passwd file */
    int pid;                    /* kill() to notify user of talk request */
    int invisible;              /* Used by cloaking function in Xyz menu */
    int sockactive;             /* Used to coordinate talk requests */
    int sockaddr;               /* ... */
    int destuid;                /* talk uses this to identify who called */
    int mode;                   /* UL/DL, Talk Mode, Chat Mode, ... */
    int pager;                  /* pager toggle, true, or false */
    int in_chat;                /* for in_chat commands   */
    char chatid[16];            /* chat id, if in chat mode */
    char from[IPLEN + 4];       /* machine name the user called in from */
    time_t logintime;
    char fill[36];
    time_t freshtime;
    int utmpkey;
	unsigned int mailbox_prop;  /* properties of currentuser's mailbox */
    char userid[20];
    char realname[20];
    char username[40];
    int friendsnum;
    int friends_uid[MAXFRIENDS];
};
struct friends {
    char id[13];
    char exp[15];
};
struct friends_info {
    char exp[15];
};

struct boardheader {            /* This structure is used to hold data in */
    char filename[STRLEN];      /* the BOARDS files */
    char BM[BM_LEN];
    char title[STRLEN];
    unsigned level;
    unsigned int nowid;
    unsigned int clubnum; /*如果是俱乐部，这个是俱乐部序号*/
    unsigned int flag;
    unsigned int adv_club; /* 用于指定club对应关系,0表示没有,1表示序号*/
    char des[200]; /*版面描述,用于www的版面说明和search */
};

/* XXX: struct fileheader moved to site.h */

struct one_key {                /* Used to pass commands to the readmenu */
    int key;
    int (*fptr) ();
};

#define USHM_SIZE       (MAXACTIVE + 10)        /*modified by dong, 10->20, 1999.9.15 */
/* modified back by KCN,20->10, because not reboot */
#define UTMP_HASHSIZE  (USHM_SIZE*4)
struct UTMPFILE {
    struct user_info uinfo[USHM_SIZE];
};
struct BoardStatus {            /* use this to speed up board list */
    int total;
    int lastpost;
    bool updatemark;
    bool updatetitle;
    bool updateorigin;
};
struct BCACHE {
    int numboards;
    struct BoardStatus bstatus[MAXBOARD];
};
struct posttop {
    char author[IDLEN + 1];     /* author name */
    char board[IDLEN + 6];      /* board name */
    char title[66];             /* title name */
    time_t date;                /* last post's date */
    int number;                 /* post number */
};
struct public_data {
    time_t nowtime;
    int sysconfimg_version;
    int www_guest_count;
    char unused[1012];
};
struct smenuitem {
    int line, col, level;
    char *name, *desc, *arg;
    char *func_name;
    /*
     * 内存中应该是func名字,因为还要考虑到修改func列表
     * 导致执行的程序不一样的情况 
     */
};

struct bbs_msgbuf {
	long int mtype;
	time_t msgtime;
	pid_t pid;
	char userid[IDLEN];
	char mtext[1];
};

struct _mail_list{
    char mail_list[MAILBOARDNUM][40];
    int mail_list_t;
} ;

typedef struct
{
	char group_name[8]; /** "groupXX\0", XX stands for 00 to 99 */
	unsigned int users_num;
	char group_desc[40];
	char __reserved[12];
}mailgroup_list_item;

typedef struct
{
	unsigned int groups_num;
	mailgroup_list_item groups[MAX_MAILGROUP_NUM];
}mailgroup_list_t;

typedef struct friends mailgroup_t;

struct msghead {
    int pos, len;
    char sent;
    char mode;
    char id[IDLEN+2];
    time_t time;
    int frompid, topid;
};

#endif

