#ifndef __SYSNAME_H_
#define __SYSNAME_H_

#define ZIXIA		1

#define USE_TMPFS           1   /*使用内存文件系统加速*/
#define TMPFSROOT   "cache" /*tmpfs的根在bbshome/cache */
#define NINE_BUILD 		0
#define NEW_COMERS 		1	/* 注册后在 newcomers 版自动发文 */
#define HAVE_BIRTHDAY 		1
#define HAPPY_BBS		0
#define HAVE_COLOR_DATE		0
#define HAVE_TEMPORARY_NICK	0
#define HAVE_FRIENDS_NUM	0
#define HAVE_REVERSE_DNS	0
#define CHINESE_CHARACTER	1
#define ANTISPAM_MAILADDR	0	/* 转信后的文章隐藏真实 MAIL */
#if ANTISPAM_MAILADDR == 1
#define	ANTISPAM_MAILDN		"zixia.net"
#endif
#define CNBBS_TOPIC		0	/* 是否在进站过程中显示 cn.bbs.* 十大热门话题 */
#define MAIL2BOARD		0	/* 是否允许直接 mail to any board */
#define MAILOUT			0	/* 是否允许向站外主动发信 */
#define MANUAL_DENY         0   /*是否允许手动解封*/
#define BBS_SERVICE_DICT    1
#define HAVE_TSINGHUA_INFO_REGISTER 0 /* 允许从清华信息系统注册 */

#define HAVE_WFORUM 1

#define SMTH			1		/* SMTH专有代码 */
#define FILTER			1		/* 使用文章内容过滤 */

#undef SITE_HIGHCOLOR
/* 
 *    Define DOTIMEOUT to set a timer to bbslog out users who sit idle on the system.
 *       Then decide how long to let them stay: MONITOR_TIMEOUT is the time in
 *          seconds a user can sit idle in Monitor mode; IDLE_TIMEOUT applies to all
 *             other modes. 
 */
#define DOTIMEOUT 1

/* 
 *    These are moot if DOTIMEOUT is commented; leave them defined anyway. 
 */
#define IDLE_TIMEOUT    (60*20) 
#define MONITOR_TIMEOUT (60*20) 

#define BBSUID 			80//9999
#define BBSGID 			80//99

/* for bbs2www, by flyriver, 2001.3.9 */
#define SECNUM 10
#define BBS_PAGE_SIZE 20

#define DEFAULTBOARD    	"zixia.net"//test
#define FILTER_BOARD        "Filter"
#define SYSMAIL_BOARD       "sysmail"
#define MAXUSERS  		40000 //150,000
#define MAXBOARD  		400//400
#define MAXCLUB                 128
#define MAXACTIVE 		999  //3000
/* remeber: if MAXACTIVE>46656 need change get_telnet_sessionid,
    make the number of session char from 3 to 4
    */
#define MAX_GUEST_NUM		99
#define WWW_MAX_LOGIN		300 /* 最大www用户数量 */

#define POP3PORT		3110	//110
#define POP3SPORT		995

/* ASCIIArt, by czz, 2002.7.5 */
#define	LENGTH_SCREEN_LINE	256	//220
#define	LENGTH_FILE_BUFFER 	260	//160
#define	LENGTH_ACBOARD_BUFFER	200	//150
#define	LENGTH_ACBOARD_LINE 	300	//80

#define LIFE_DAY_USER		365	//120
#define LIFE_DAY_LONG		666	//666
#define LIFE_DAY_SYSOP		500	//120
#define LIFE_DAY_NODIE		999	//120
#define LIFE_DAY_NEW		30	//15
#define LIFE_DAY_SUICIDE	15	//15

#define DAY_DELETED_CLEAN	20	//20
#define SEC_DELETED_OLDHOME	0 	/*  3600*24*30，注册新用户如果存在这个用户的目录，保留时间*/

#define	REGISTER_WAIT_TIME	(30*60) // (72*60*60)
#define	REGISTER_WAIT_TIME_NAME	"30分钟" //72 小时

#define MAIL_BBSDOMAIN      "bbs.zixia.net"
#define MAIL_MAILSERVER     "127.0.0.1:25"

#define NAME_BBS_ENGLISH	"bbs.zixia.net" //smth.org
#define	NAME_BBS_CHINESE	"大话西游" //水木清华
#define NAME_BBS_NICK		"斧头帮" // BBS 站
#define BBS_FULL_NAME		"BBS 大话西游站"

#define FOOTER_MOVIE		"  斧  头  帮  " // 欢  迎  投  稿
#define ISSUE_LOGIN		"本站使用一台X86服务器" //本站使用曙光公司曙光天演服务器
#define ISSUE_LOGOUT		"般若波羅密！" //还是走了罗

#define NAME_USER_SHORT		"斧头帮众" //用户
#define NAME_USER_LONG		"斧头帮众" //"水木用户"
#define NAME_SYSOP		"强盗头" //"System Operator"
#define NAME_BM			"二当家的" //版主
#define NAME_POLICE		"旺财" //"警察"
#define	NAME_SYSOP_GROUP	"斧头帮高层" //"站务组"
#define NAME_ANONYMOUS		"Do you zixia!?" //"水母青蛙 Today!"
#define NAME_ANONYMOUS_FROM	"水帘洞" //匿名天使的家
#define ANONYMOUS_DEFAULT 0

#define NAME_MATTER		"帮务" //站务
#define NAME_SYS_MANAGE		"帮内管理" //"系统维护"
#define NAME_SEND_MSG		"丢斧头" // "送讯息"
#define NAME_VIEW_MSG		"查斧头" // "送讯息"

#define CHAT_MAIN_ROOM		"zixia" //main
#define	CHAT_TOPIC		"盘丝洞？明明是水帘洞嘛……" //"大家先随便聊聊吧"
#define CHAT_MSG_NOT_OP		"*** 盘丝洞不要乱闯！ ***" //"*** 您不是本聊天室的op ***"
#define	CHAT_ROOM_NAME		"洞○洞"//"聊天室"
#define	CHAT_SERVER		"盘丝洞" //"聊天广场"
#define CHAT_MSG_QUIT		"回到五百年前" //"切离系统"
#define CHAT_OP			"洞主" //"聊天室 op"
#define CHAT_SYSTEM		"上天" //"系统"
#define	CHAT_PARTY		"帮众" // "大家"

#define DEFAULT_NICK		"旺财"
//#define LOCAL_ARTICLE_DEFAULT 	0	//缺省转信

#define MSG_ERR_USERID		"嗯？这个猪头是谁？..."
#define LOGIN_PROMPT		"报上名来"
#define PASSWD_PROMPT		"接头暗号"

/* 权限位定义 */
/*
   These are the 16 basic permission bits. 
   All but the last one are used in comm_lists.c and help.c to control user 
   access to priviliged functions. The symbolic names are given to roughly
   correspond to their actual usage; feel free to use them for different
   purposes though. The PERM_SPECIAL1 and PERM_SPECIAL2 are not used by 
   default and you can use them to set up restricted boards or chat rooms.
*/

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
#define PERM_SECANC    040000000
#define PERM_JURY      0100000000
#define PERM_SEXY      0200000000/*#define PERM_CHECKCD      0200000000*/
#define PERM_SUICIDE   0400000000
#define PERM_MM         01000000000/*#define PERM_COLLECTIVE 	01000000000*/
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
#define DEFINE(user,x)     ((x)?((user)->userdefine)&(x):1)

#define TDEFINE(x) ((x)?(tmpuser)&(x):1)

/* 用户自订参数定义 */
#define DEF_ACBOARD      000001
#define DEF_COLOR        000002
#define DEF_EDITMSG      000004
#define DEF_NEWPOST      000010
#define DEF_ENDLINE      000020
#define DEF_LOGFRIEND    000040
#define DEF_FRIENDCALL   000100
#define DEF_LOGOUT       000200
#define DEF_INNOTE       000400
#define DEF_OUTNOTE      001000
#define DEF_NOTMSGFRIEND 002000
#define DEF_NORMALSCR    004000
#define DEF_CIRCLE       010000
#define DEF_FIRSTNEW     020000
#define DEF_TITLECOLOR   040000
#define DEF_ALLMSG       0100000
#define DEF_FRIENDMSG    0200000
#define DEF_SOUNDMSG     0400000
#define DEF_MAILMSG      01000000
#define DEF_LOGININFORM  02000000
#define DEF_SHOWSCREEN   04000000
#define DEF_SHOWHOT      010000000
#define DEF_NOTEPAD      020000000
#define DEF_IGNOREMSG    040000000      /* Added by Marco */
#define DEF_HIGHCOLOR	0100000000   /*Leeward 98.01.12 */
#define DEF_SHOWSTATISTIC 0200000000    /* Haohmaru */
#define DEF_UNREADMARK 0400000000       /* Luzi 99.01.12 */
#define DEF_USEGB     01000000000       /* KCN,99.09.05 */
#define DEF_CHCHAR    02000000000		
#define DEF_SHOWDETAILUSERDATA 04000000000
#define DEF_SHOWREALUSERDATA   010000000000
//#define DEF_SPLITSCREEN 02000000000 /* bad 2002.9.1 */
/*#define DEF_HIDEIP    02000000000  Haohmaru,99.12.18*/

/*#define PERM_POSTMASK  0100000  *//* means the rest is a post mask */

#define NUMDEFINES 31//29


#define TDEF_SPLITSCREEN 000001


extern const char *permstrings[];
extern const char    *groups[];
extern const char    *explain[];
extern const char *user_definestr[];
extern const char *mailbox_prop_str[];

/**
 * 看在线用户时的按键处理字符。
 */
#define UL_CHANGE_NICK_UPPER   'C'
#define UL_CHANGE_NICK_LOWER   'c'
#define UL_SWITCH_FRIEND_UPPER 'F'
#define UL_SWITCH_FRIEND_LOWER 'f'

/**
 * 文章相关部分。
 */
#define STRLEN          80
#define BM_LEN 60
#define FILENAME_LEN 20
#define OWNER_LEN 30
typedef struct fileheader {     /* This structure is used to hold data in */
    char filename[FILENAME_LEN];      /* the DIR files */
    unsigned int id, groupid, reid;
#ifdef FILTER
    char o_board[STRLEN - BM_LEN];
    unsigned int o_id;
    unsigned int o_groupid;
    unsigned int o_reid;
    char unused1[14];
#else
    char unused1[46];
#endif
    char innflag[2];
    char owner[OWNER_LEN];
    char unused2[38];
    unsigned int eff_size;
    time_t posttime;
    long attachment;
    char title[STRLEN];
    unsigned level;
    unsigned char accessed[12]; /* struct size = 256 bytes */
} fileheader;

typedef struct fileheader fileheader_t;

#define GET_POSTFILENAME(x,y) get_postfilename(x,y,0)
#define GET_MAILFILENAME(x,y) get_postfilename(x,y,0)
#define VALID_FILENAME(x) valid_filename(x,0)

// WWW部分

//修改密码之类的页面使用https
#define SECURE_HTTPS

/**
 * Mailbox properties.
 * 
 * @author flyriver
 */

#define MBP_SAVESENTMAIL      0x00000001
#define MBP_FORCEDELETEMAIL   0x00000002
#define MBP_MAILBOXSHORTCUT   0x00000004

#define MBP_NUMS 3

#define HAS_MAILBOX_PROP(u, x) ((u)->mailbox_prop & x)

/**
 * Mailgroup macros.
 *
 * @author flyriver
 */
#define MAX_MAILGROUP_NUM 30
#define MAX_MAILGROUP_USERS 300

#define SMS_SUPPORT

/**
attach define
*/
#define ATTACHTMPPATH "boards/_attach"
#endif
