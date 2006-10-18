#ifndef __SYSNAME_H_
#define __SYSNAME_H_

#define NEW_COMERS 		0       /* 注册后在 newcomers 版自动发文 */
#define HAVE_USERMONEY 1
#define CONV_PASS		1
#define HAPPY_BBS		0
#define FREE 1
#define HAVE_COLOR_DATE		1
#define HAVE_FRIENDS_NUM	1
#define HAVE_REVERSE_DNS	0
#define CHINESE_CHARACTER	1
#define CNBBS_TOPIC		0       /* 是否在进站过程中显示 cn.bbs.* 十大热门话题 */
#define MAIL2BOARD		0       /* 是否允许直接 mail to any board */
#define MAILOUT			0       /* 是否允许向站外主动发信 */
#define MANUAL_DENY		0       /*是否允许手动解封 */
#define BBS_SERVICE_DICT	0
#define HAVE_PERSONAL_DNS	0       /*个人域名服务 */

#define BUILD_PHP_EXTENSION 1   /*将php lib编成php extension */

#define HAVE_WFORUM		0
#undef RAW_ARTICLE
#define SMTH			0
#ifndef FILTER
#define FILTER			0
#endif
#define IDLE_TIMEOUT    (60*20)

#define BBSUID 			510
#define BBSGID 			510

/* for bbs2www, by flyriver, 2001.3.9 */
#define SECNUM 8
#define BBS_PAGE_SIZE 20

#define SQUID_ACCL

#define DEFAULTBOARD    	"test"
#define FILTER_BOARD        "Filter"
#define SYSMAIL_BOARD       "sysmail"
#define BLESS_BOARD "Blessing"

#define MAXUSERS  		20000
#define MAXCLUB			128
#define MAXBOARD  		1024
#define MAXACTIVE 		2000
/* remeber: if MAXACTIVE>46656 need change get_telnet_sessionid,
    make the number of session char from 3 to 4
    */
#define MAX_GUEST_NUM		1000
#define WWW_MAX_LOGIN 5000

#define POP3PORT		110
#define POP3SPORT		995
/* ASCIIArt, by czz, 2002.7.5 */
#define       LENGTH_SCREEN_LINE      255
#define       LENGTH_FILE_BUFFER      255
#define       LENGTH_ACBOARD_BUFFER   300
#define       LENGTH_ACBOARD_LINE     300

#define LIFE_DAY_USER		120
#define LIFE_DAY_YEAR          365
#define LIFE_DAY_LONG		666
#define LIFE_DAY_SYSOP		120
#define LIFE_DAY_NODIE		999
#define LIFE_DAY_NEW		15
#define LIFE_DAY_SUICIDE	15

#define DAY_DELETED_CLEAN	97
#define SEC_DELETED_OLDHOME	2592000 /* 3600*24*30，注册新用户如果存在这个用户的目录，保留时间 */

#define	REGISTER_WAIT_TIME	(48*60*60)
#define	REGISTER_WAIT_TIME_NAME	"48 小时"

#define MAIL_BBSDOMAIN      "ppjj.org"
#define MAIL_MAILSERVER     "127.0.0.1:25"

#define NAME_BBS_ENGLISH	"bbs.ee.tsinghua.edu.cn"
#define	NAME_BBS_CHINESE	"自由空间"
#define NAME_BBS_NICK		"Free"

#define BBS_FULL_NAME "自由空间"

#define FOOTER_MOVIE		"欢  迎  投  稿"
/*#define ISSUE_LOGIN		"本站使用曙光公司曙光天演服务器"*/
#define ISSUE_LOGIN		"welcome"
#define ISSUE_LOGOUT		"还是走了罗"

#define NAME_USER_SHORT		"用户"
#define NAME_USER_LONG		"用户"
#define NAME_SYSOP		"System Operator"
#define NAME_BM			"版主"
#define NAME_POLICE		"警察"
#define	NAME_SYSOP_GROUP	"站务组"
#define NAME_ANONYMOUS		"水母青蛙 Today!"
#define NAME_ANONYMOUS_FROM	"匿名天使的家"
#define ANONYMOUS_DEFAULT 0

#define NAME_MATTER		"站务"
#define NAME_SYS_MANAGE		"系统维护"
#define NAME_SEND_MSG		"送讯息"
#define NAME_VIEW_MSG		"送讯息"

#define CHAT_MAIN_ROOM		"main"
#define	CHAT_TOPIC		"大家先随便聊聊吧"
#define CHAT_MSG_NOT_OP		"*** 您不是本聊天室的op ***"
#define	CHAT_ROOM_NAME		"聊天室"
#define	CHAT_SERVER		"聊天广场"
#define CHAT_MSG_QUIT		"切离系统"
#define CHAT_OP			"聊天室 op"
#define CHAT_SYSTEM		"系统"
#define	CHAT_PARTY		"大家"

#define DEFAULT_NICK		"每天爱你多一些"

#define MSG_ERR_USERID		"错误的使用者代号..."
#define LOGIN_PROMPT		"请输入代号"
#define PASSWD_PROMPT		"请输入密码"

#define USE_DEFAULT_PERMSTRINGS
#define USE_DEFAULT_DEFINESTR
#define USE_DEFAULT_MAILBOX_PROP_STR

/**
 * 看在线用户时的按键处理字符。
 */
#define UL_CHANGE_NICK_UPPER   'C'
#define UL_CHANGE_NICK_LOWER   'c'
#define UL_SWITCH_FRIEND_UPPER 'F'
#define UL_SWITCH_FRIEND_LOWER 'f'

#define OWN_USEREC 1
#define IPLEN 16
struct userec {                 /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    char flags; /*一些标志，戒网，版面排序之类的*/
    unsigned char title; /*用户级别*/
    time_t firstlogin;
    char lasthost[IPLEN];
    unsigned int numlogins;
    unsigned int numposts;
#ifdef CONV_PASS
    char passwd[35];
	char unused_padding[2];
#endif
    char username[NAMELEN];
    unsigned int club_read_rights[MAXCLUB>>5];
    unsigned int club_write_rights[MAXCLUB>>5];
    unsigned char md5passwd[MD5PASSLEN];
#ifndef OS_64BIT
    unsigned int userlevel;
#endif
    time_t lastlogin;
    time_t stay;
#ifdef OS_64BIT /*  align 8 bytes... */
    unsigned int userlevel;
#endif
    int signature;
    unsigned int userdefine[2];
    time_t notedate;
    int noteline;
    int notemode;
    time_t exittime;
	/* 生日数据转移到 userdata 结构中 */
    unsigned int usedspace;     /* used space of user's mailbox, in bytes */
    int money;
    int score;
    int unused[5];
};

/** 使用缺省的FILEHeader结构*/
#define HAVE_FILEHEADER_DEFINE

#define GET_POSTFILENAME(x,y) get_postfilename(x,y,0)
#define GET_MAILFILENAME(x,y) get_postfilename(x,y,0)
#define VALID_FILENAME(x) valid_filename(x,1)
#define POSTFILE_BASENAME(x) (((char *)(x))+2)
#define MAILFILE_BASENAME(x) (x)

/**
attach define
*/
#define ATTACHTMPPATH "boards/_attach"

//#define CHECK_IP_LINK 1

//#define SMS_SUPPORT

//#define PERSONAL_CORP

#define NEW_HELP
#define HAVE_DEFAULT_HELPMODE

#define COMMEND_ARTICLE "Recommend"

//#define NOT_USE_DEFAULT_SMS_FUNCTIONS

#define HAVE_OWN_USERIP
#define SHOW_USERIP(user,x) showuserip(user,x)

//#define AUTO_CHECK_REGISTER_FORM        //自动通过注册单

#define QUOTED_LINES 10
#define QUOTELEV 0

//#define BIRTHFILENUM 12
//#define BIRTHFILEPATH "0Announce/groups/system.faq/SYSOP/birthfile"

#define MYUNLINK_BACKUPDIR "backup"

#define BMSLOG
#define NEWPOSTLOG
#define NEWBMLOG

#define NEWPOSTSTAT

#define FB2KENDLINE
#define LOWCOLOR_ONLINE
#define OPEN_NOREPLY

#define FIRSTARTICLE_SIGN "◆"

#define POST_QUIT

#define ANN_COUNT

#define ANN_SHOW_WELCOME

#define ANN_CTRLK
#define ANN_GUESTBOOK
#define ANN_AUTONAME
#define FB2KPC "pc"

#define ACBOARD_BNAME "notepad"

#define MAXnettyLN      7       /* lines of  activity board  */
#define DENYANONY

#define AUTOREMAIL
#define OWNSENDMAIL "/usr/lib/sendmail"

#define UNREAD_SIGN '+'
#define OPEN_BMONLINE
//#define USE_SEM_LOCK
#endif
