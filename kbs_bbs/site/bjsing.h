#ifndef __SYSNAME_H_
#define __SYSNAME_H_

#define BBSUID 			9999
#define BBSGID 			999

#define USE_TMPFS 1
#define TMPFSROOT "cache"

#define DQPI    1               /*作为DQPI的测试站嘛 */
#define SMTH	1
#define HAVE_TSINGHUA_INFO_REGISTER 0

#ifndef REGISTER_TSINGHUA_WAIT_TIME 
#define REGISTER_TSINGHUA_WAIT_TIME (240*60)
#endif

#define HAVE_COLOR_DATE		1
#define HAVE_TEMPORARY_NICK	1
#define HAVE_REVERSE_DNS	0
#define CHINESE_CHARACTER	1
#define	ANTISPAM_MAILADDR	0
#define CNBBS_TOPIC		0
#define MAIL2BOARD		0
#define MAILOUT			0
#define MANUAL_DENY		1
#define BBS_SERVICE_DICT	1
#define BBS_SERVICE_QUIZ	1
#define HAVE_PERSONAL_DNS	0
#define BOARD_SHOW_ONLINE	1
#define HAVE_BRC_CONTROL	1
#define FILTER			0   /*过滤器 */
#define HAVE_WFORUM		1
#define HAVE_TSINGHUA_INFO_REGISTER	0
#define	IDLE_TIMEOUT		(60*20)  /*发呆时间*/
#define	SECNUM			10	 /* 分区个数 */
#define CHAT_MSG_NOT_OP		"*** 您不是聊天室老大 ***"
#define CHAT_ROOM_NAME		"快意聊天室"
#define CHAT_SERVER		"北极星空间站"
#define HAVE_CUSTOM_USER_TITLE	1


#define MSG_ERR_USERID		"这里没有这个人，请输入 \'new\' 注册一个吧."
#define LOGIN_PROMPT		"请输入代号"
#define PASSWD_PROMPT		"请输入密码"


#define WWW_MAX_LOGIN 128
#define CONV_PASS 1
#define HAVE_BIRTHDAY 1
#define NEW_COMERS 0
#define FIVEGAME 1
#define	TALK_LOG 1

#define BLESS_BOARD "Party"
#define DEFAULTBOARD            "Test"
#define SYSMAIL_BOARD       "junk"
#define MAXUSERS                20000
#define MAXCLUB                 128
#define MAXBOARD                256
#define MAXACTIVE               800
#define MAX_GUEST_NUM           80
#define WWW_MAX_LOGIN 100
#define MAX_WWW_GUEST 30
#define DAY_DELETED_CLEAN       97  		/*版面垃圾箱和回收站的清除天数 */
#define REGISTER_WAIT_TIME      0		 /*填注册单需要等待的时间 */
#define MAIL_BBSDOMAIN      "bjsing.net"
#define NAME_BBS_CHINESE        "北极星"
#define NAME_BBS_ENGLISH	"bjsing.net"
#define NAME_BBS_NICK           "BBS 站"
#define BBS_FULL_NAME "BBS 北极星站"
#define ISSUE_LOGIN             "热烈庆祝北极星建站五周年"
#define NAME_ANONYMOUS          "北极星过客!"
#define BUILD_PHP_EXTENSION 1 /*将php lib编成php extension*/






/**
 *  * 文章相关部分。
 *   */
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
	char unused2[42];
	unsigned int eff_size;
	long attachment;
	char title[STRLEN];
	unsigned level;
	unsigned char accessed[12]; /* struct size = 256 bytes */
} fileheader;

typedef struct fileheader fileheader_t;


#define DEF_SHOWDETAILUSERDATA 04000000000
#define DEF_SHOWREALUSERDATA   010000000000
#define PERSONAL_CORP


#endif
