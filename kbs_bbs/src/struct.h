#ifndef __STRUCT_H__
#define __STRUCT_H__
/* Note the protocol field is not inside an #ifdef FILES...
   this is a waste but allows you to add/remove UL/DL support without
   rebuilding the PASSWDS file (and it's only a lil ole int anyway).
*/
struct userec {                 /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    char flags; /*一些标志，戒网，版面排序之类的*/
    unsigned char title; /*用户级别*/
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
	/* 生日数据转移到 userdata 结构中 */
    unsigned int usedspace;     /* used space of user's mailbox, in bytes */
#ifdef HAVE_USERMONEY
    int money;
    int score;
    char unused[20];
#endif
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
#ifdef HAVE_BIRTHDAY
	char            gender;
	unsigned char   birthyear;
	unsigned char   birthmonth;
	unsigned char   birthday;
#endif
    char reg_email[STRLEN]; /* registry email . added by binxun . 2003.6.6 */
/*#ifdef SMS_SUPPORT*/
    bool mobileregistered;
    char mobilenumber[MOBILE_NUMBER_LEN];
/*#endif*/
/* add by roy 2003.07.23 for wbbs*/
    char OICQ[STRLEN];
    char ICQ[STRLEN];
    char MSN[STRLEN];
    char homepage[STRLEN];
    int userface_img;
    char userface_url[STRLEN];
    unsigned char userface_width;
    unsigned char userface_height;
    unsigned int group;
    char country[STRLEN];
    char province[STRLEN];
    char city[STRLEN];
    unsigned char shengxiao;
    unsigned char bloodtype;
    unsigned char religion;
    unsigned char profession;
    unsigned char married;
    unsigned char education;
    char graduateschool[STRLEN];
    unsigned char character;
    char photo_url[STRLEN];
    char telephone[STRLEN];
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
#ifdef HAVE_WFORUM
	char fill[35];
	unsigned char yank;
#else 
    char fill[36];
#endif
    time_t freshtime;
    int utmpkey;
    unsigned int mailbox_prop;  /* properties of currentuser's mailbox */
    char userid[20];
    char realname[20];
    char username[40];
    int friendsnum;
    int friends_uid[MAXFRIENDS];
#ifdef FRIEND_MULTI_GROUP
    unsigned int friends_p[MAXFRIENDS];
#endif
    int currentboard;
	int mailcheck;				/* if have new mail, stiger */
								/* 0: need recheck
								 * 1: not check  */
};

struct usermemo {
	struct userdata ud;
};

struct friends {
    char id[13];
    char exp[15];
#ifdef FRIEND_MULTI_GROUP
    int groupid;
#endif
};
struct friends_info {
    char exp[15];
};
struct friends_group {
    char name[15];
    unsigned int p;
    char f;
    char b;
};

struct boardheader {            /* This structure is used to hold data in */
    char filename[STRLEN];      /* the BOARDS files */
    char BM[BM_LEN];
    char title[STRLEN];
    unsigned level;
    unsigned int nowid;
    unsigned int clubnum; /*如果是俱乐部，这个是俱乐部序号*/
    unsigned int flag;
    union {
    unsigned int adv_club; /* 用于指定club对应关系,0表示没有,1表示序号*/
    unsigned int group_total; /*如果是二级目录，应该是目录的版面总数*/
    } board_data;
    time_t createtime;
    int toptitle;
    char ann_path[128];
    int group; /*所属目录*/
    char des[196]; /*版面描述,用于www的版面说明和search */
};

/* add by roy 2003.8.7 struct wwwthreadheader */
/* used for .WWWTHREAD */
struct wwwthreadheader{
	struct fileheader origin; /* groupid */
	struct fileheader lastreply;	/* id for last article */
	unsigned int articlecount; /* number of articles */
	unsigned int flags;
	unsigned int unused;   /* used for further index */
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
    int currentusers;
#ifdef HAVE_WFORUM
	int todaynum;
#endif
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
    unsigned int max_user;
    unsigned int max_wwwguest;
    char unused[1004];
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

struct smsmsg {
	int id;
	char userid[13];
	char dest[13];
	char time[15];
	int type;
	int level;
	char *context;
	int readed;
};

struct addresslist{
	int id;
	char userid[13];
	char name[15];
	char bbsid[15];
	char school[100];
	char zipcode[7];
	char homeaddr[100];
	char companyaddr[100];
	char tel_o[20];
	char tel_h[20];
	char mobile[15];
	char email[30];
	char qq[10];
	int birth_year;
	int birth_month;
	int birth_day;
	char *memo;
	char group[10];
};

#define AL_ORDER_NAME 1
#define AL_ORDER_BBSID 2
#define AL_ORDER_GROUPNAME 3
#define AL_ORDER_COUNT 3

struct key_struct { // 自定义键 by bad
    int status[10];
    int key;
    int mapped[10];
};

//个人文集 by stiger
#ifdef PERSONAL_CORP

struct _pc_selusr
{
	char userid[IDLEN+2];
	char corpusname[41];
	time_t createtime;
};

struct pc_users {
	int uid;	//not uid in PASSWD, 是个人文集的uid
	char username[IDLEN+2];
	char corpusname[41];
	char description[201];
	char theme[11];
	int nodelimit;
	int dirlimit;
	time_t createtime;
};

struct pc_nodes {
	unsigned long nid;		//node id, auto increment

	//pid不为零:
	//   type: 0: 收藏夹文章, pid表示收藏夹目录nodes nid
	//   type: 1: 收藏夹目录, pid表示父收藏夹目录nodes nid
	//pid == 0:
	//   type: 0: 普通文章
	//   type: 1: 收藏夹跟目录
	unsigned long pid;

	//type: 0: 文章
	//      1: 收藏夹目录
	int type;

	char source[11];
	char hostname[21];
	time_t created;
	time_t changed;
	int uid;

	//是否允许评论
	// 0: 不允许
	// 1: 允许登陆用户
	// 2: 允许guest
	int comment;
	long commentcount;
	char subject[201];
	char *body;

	//权限
	// 0: 公开
	// 1: 好友
	// 2: 私人
	// 3: 收藏
	// 4: 垃圾
	int access;
	int visitcount;
};

struct pc_comments {
	unsigned long cid;
	unsigned long nid;
	int uid;
	char hostname[21];
	char username[21];
	char subject[201];
	time_t created;
	time_t changed;
	char *body;
};

#define PC_DEFAULT_NODELIMIT 300
#define PC_DEFAULT_DIRLIMIT 300

#endif

#if HAVE_WWW==1

#define MAX_WWW_MAP_ITEM (MAX_WWW_GUEST/32)

struct WWW_GUEST_S {
    int key;
    time_t freshtime;
    time_t logintime;
    int currentboard;
};

struct WWW_GUEST_TABLE {
    int use_map[MAX_WWW_MAP_ITEM + 1];
    time_t uptime;
    struct WWW_GUEST_S guest_entry[MAX_WWW_GUEST];
};

#endif

#ifdef HAVE_PERSONAL_DNS
struct dns_msgbuf {
	long int mtype;
	char userid[IDLEN+1];
	char ip[IPLEN+1];
};
#endif
#endif

