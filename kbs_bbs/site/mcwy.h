#ifndef __SYSNAME_H_
#define __SYSNAME_H_

#define BBSUID 			9999
#define BBSGID 			9999

#define USE_TMPFS 1
#define TMPFSROOT "cache"
#define SMTH    0 /*作为SMTH的测试站嘛*/
#define HAVE_TSINGHUA_INFO_REGISTER 0
int auto_register(char *userid, char *email, int msize);

/** 使用缺省的FILEHeader结构*/
//#define HAVE_FILEHEADER_DEFINE

/** 定义使用缺省的函数 */
//#define USE_DEFAULT_SECODE
#define USE_DEFAULT_ALPHABET
//#define USE_DEFAULT_PERMSTRINGS
#define USE_DEFAULT_DEFINESTR
#define USE_DEFAULT_MAILBOX_PROP_STR
#define MBP_DEFAULT           0x00000001
#define USE_DEFAULT_LEVELCHAR
#define USE_DEFAULT_MODE
//#define USE_DEFAULT_MULTILOGIN_CHECK
#define USE_DEFAULT_USER_LIFE
#define USE_DEFAULT_ANNOUNCE_FILENAME
#define USE_DEFAULT_GET_POSTTIME
#define USE_DEFAULT_SET_POSTTIME
#define USE_DEFAULT_SET_POSTTIME2
#define USE_DEFAULT_BUILD_BOARD
#define USE_DEFAULT_MAIL_LIMIT /*邮件容量限制*/
#define USE_DEFAULT_READ_PERM
#define USE_DEFAULT_SEE_PERM
#define REGISTER_TSINGHUA_WAIT_TIME 0
#define WWW_MAX_LOGIN 128

#define TELNET_WORD_WRAP 1 //Telnet下长行自动词间换行

#define HAVE_WFORUM 1
#define QUOTELEV 1
#define QUOTED_LINES 10

#define PERSONAL_CORP 1

#define HAVE_USERMONEY 1

#define CHECK_CONNECT 0

#define COMMEND_ARTICLE "Recommend"

#define BUILD_PHP_EXTENSION

#define USE_SEM_LOCK 1

#define HAVE_OWN_USERIP
#define SHOW_USERIP(user,x) showuserip(user,x)

//==atppp added==
#define SECNUM 7
extern const char secname[SECNUM][2][20];

#define DEFAULTBOARD        "SYSOP"
#define MAXACTIVE           512
#define MAX_GUEST_NUM       32
#define MAX_WWW_GUEST       256
#define MAIL_BBSDOMAIN      "bbs.stanford.edu"
#define NAME_BBS_ENGLISH    "bbs.stanford.edu"
#define NAME_BBS_CHINESE    "牧场物语"
#define BBS_FULL_NAME       "牧场物语"
#define NAME_ANONYMOUS      "匿名天使"

#define NEW_COMERS 1
#define HAVE_COLOR_DATE 1
#define CONV_PASS 1
#define HAVE_FRIENDS_NUM 1
#define MAIL2BOARD 1
#ifndef FILTER
#define FILTER 0
#endif
#define BOARD_SHOW_ONLINE 1
#define CNBBS_TOPIC 1


#define STRLEN          80
#define BM_LEN 60
#define FILENAME_LEN 20
#define OWNER_LEN 14
#define ARTICLE_TITLE_LEN 60
typedef struct fileheader {     /* This structure is used to hold data in */
    char filename[FILENAME_LEN];      /* the DIR files */
    unsigned int id, groupid, reid;
	int o_bid;
    unsigned int o_id;
    unsigned int o_groupid;
    unsigned int o_reid;
    char innflag[2];
    char owner[OWNER_LEN];
    unsigned int eff_size;
    time_t posttime;
    long attachment;
    char title[ARTICLE_TITLE_LEN];
    unsigned char accessed[4];
} fileheader;

typedef struct fileheader fileheader_t;

#define DAY_DELETED_CLEAN 7
//#define FLOWBANNER 1
#define COLOR_ONLINE 1

#define NUMPERMS (30)

#define PERM_BASIC      000001
#define PERM_CHAT       000002
#define PERM_PAGE       000004
#define PERM_POST       000010
#define PERM_LOGINOK    000020
#define PERM_BMAMANGER	000040
#define PERM_CLOAK      000100
#define PERM_SEECLOAK   000200
#define PERM_XEMPT      000400
#define PERM_WELCOME    001000
#define PERM_BOARDS     002000
#define PERM_ACCOUNTS   004000
#define PERM_CHATCLOAK  010000
#define PERM_DENYRELAX      020000
#define PERM_SYSOP      040000
#define PERM_POSTMASK  0100000
#define PERM_ANNOUNCE  0200000
#define PERM_OBOARDS   0400000
#define PERM_ACBOARD   01000000
#define PERM_NOZAP     02000000
#define PERM_CHATOP    04000000
#define PERM_ADMIN     010000000
#define PERM_HORNOR   	020000000
#define PERM_6SEC    040000000
#define PERM_JURY      0100000000
#define PERM_CHECKCD      0200000000
#define PERM_SUICIDE   0400000000
#define PERM_COLLECTIVE 	01000000000
#define PERM_DISS        02000000000
#define PERM_DENYMAIL   	04000000000


#define XPERMSTR "bTCPRp#@XWBA$VS!DEM1234567890%"
/* means the rest is a post mask */

/* This is the default permission granted to all new accounts. */
#define PERM_DEFAULT    (PERM_BASIC | PERM_CHAT | PERM_PAGE | PERM_POST | PERM_LOGINOK)

/* These permissions are bitwise ORs of the basic bits. They work that way
   too. For example, anyone with PERM_SYSOP or PERM_BOARDS or both has
   PERM_SEEBLEVELS. */

#define PERM_ADMINMENU  (PERM_ACCOUNTS | PERM_SYSOP|PERM_OBOARDS|PERM_WELCOME)
#define PERM_MULTILOG   (PERM_SYSOP | PERM_ANNOUNCE | PERM_OBOARDS)
#define PERM_LOGINCLOAK (PERM_SYSOP | PERM_ACCOUNTS | PERM_BOARDS | PERM_OBOARDS | PERM_WELCOME)
#define PERM_SEEULEVELS (PERM_SYSOP | PERM_BOARDS)
#define PERM_SEEBLEVELS (PERM_SYSOP | PERM_BOARDS)
#define PERM_MARKPOST   (PERM_SYSOP | PERM_BOARDS)
#define PERM_UCLEAN     (PERM_SYSOP | PERM_ACCOUNTS)
#define PERM_NOTIMEOUT  PERM_SYSOP
/* PERM_MANAGER will be used to allow 2 windows and 
bigger mailbox. --stephen 2001.10.31*/
#define PERM_MANAGER    (PERM_CHATOP | PERM_JURY | PERM_BOARDS)
#define PERM_SENDMAIL   0
#define PERM_READMAIL   PERM_BASIC
#define PERM_VOTE       PERM_BASIC

/* These are used only in Internet Mail Forwarding */
/* You may want to be more restrictive than the default, especially for an
   open access BBS. */

#define PERM_SETADDR    PERM_BASIC      /* to set address for forwarding */
#define PERM_FORWARD    PERM_BASIC      /* to do the forwarding */

/* Don't mess with this. */
#define HAS_PERM(user,x) ((x)?((user)->userlevel)&(x):1)
#define DEFINE(user,x)     ((x)?((user)->userdefine[def_list(x)])&(x):1)

#endif //permission define NUMPERMS

#ifndef PERM_AUTOSET
#define PERM_AUTOSET	PERM_BASIC


#endif
