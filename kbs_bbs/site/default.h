/**
  缺省的配置文件
*/

#ifndef __SYS_DEFAULT_H_
#define __SYS_DEFAULT_H_

#ifndef TMPFSROOT
#define TMPFSROOT "cache"
#endif

#ifndef SMTH /*水木清华专有代码,缺省使能*/
#define SMTH 1
#else
#if SMTH == 0
#undef SMTH
#endif
#endif

#ifdef HAPPY_BBS
#if HAPPY_BBS == 0 /* HAPPY BBS专有代码 */
#undef HAPPY_BBS
#endif
#endif

#ifdef FREE
#if FREE == 0 /* FREE BBS专有代码 */
#undef FREE
#endif
#endif

#ifdef FLOWBANNER
#ifndef MAXBANNER
#define MAXBANNER 8
#endif
#ifndef BANNERSIZE
#define BANNERSIZE 120
#endif
#endif

#if NEW_COMERS == 0
#undef NEW_COMERS		/* 注册后在 newcomers 版自动发文 */
#endif

#if HAVE_COLOR_DATE == 0
#undef HAVE_COLOR_DATE
#endif

#if HAVE_FRIENDS_NUM == 0
#undef HAVE_FRIENDS_NUM
#endif

#if HAVE_REVERSE_DNS == 0 /*反向dns解析 */
#undef HAVE_REVERSE_DNS
#endif

#if HAVE_WFORUM == 0
#undef HAVE_WFORUM
#endif


/* 这个是由于历史原因添加在这里的，主要是因为一些转换程序仍旧需要这个定义 - atppp 20051004 */
#ifdef HAVE_WFORUM
#define HAVE_USERMONEY 1
#endif



/* 默认情况下加入 HAVE_BIRTHDAY 支持 - atppp 20040819 */
#ifdef HAVE_BIRTHDAY
#if HAVE_BIRTHDAY == 0
#undef HAVE_BIRTHDAY
#endif
#else
#define HAVE_BIRTHDAY 1
#endif

#ifndef CHINESE_CHARACTER /*汉字整字处理,缺省使能*/
#define CHINESE_CHARACTER
#else
#if CHINESE_CHARACTER == 0
#undef CHINESE_CHARACTER
#endif
#endif

#if CNBBS_TOPIC	== 0		/* 是否在进站过程中显示 cn.bbs.* 十大热门话题 */
#undef CNBBS_TOPIC		/* 是否在进站过程中显示 cn.bbs.* 十大热门话题 */
#endif

#if MAIL2BOARD == 0		/* 是否允许直接 mail to any board */
#undef MAIL2BOARD		/* 是否允许直接 mail to any board */
#endif

#if MAILOUT == 0		/* 是否允许向站外主动发信 */
#undef MAILOUT			/* 是否允许向站外主动发信 */
#endif

#ifndef MANUAL_DENY		/* 手动解封，缺省使能*/
#define MANUAL_DENY
#else
#if MANUAL_DENY == 0
#undef MANUAL_DENY
#endif
#endif

#ifndef BBS_SERVICE_DICT	/* 词典功能 */
#define BBS_SERVICE_DICT
#else
#if BBS_SERVICE_DICT == 0
#undef BBS_SERVICE_DICT
#endif
#endif

#ifndef BBS_SERVICE_QUIZ	/* 词典功能 */
#define BBS_SERVICE_QUIZ
#else
#if BBS_SERVICE_QUIZ == 0
#undef BBS_SERVICE_QUIZ
#endif
#endif

#if HAVE_PERSONAL_DNS == 0	/*个人域名服务，缺省不使能*/
#undef HAVE_PERSONAL_DNS
#endif

#ifndef HAVE_BRC_CONTROL
#define HAVE_BRC_CONTROL
#else
#if HAVE_BRC_CONTROL == 0
#undef HAVE_BRC_CONTROL
#endif
#endif

#ifndef FILTER /*过滤器*/
#define FILTER
#else
#if FILTER == 0
#undef FILTER
#endif
#endif



#ifndef IDLE_TIMEOUT
#define IDLE_TIMEOUT    (60*20) 
#endif

#ifndef MAX_WWW_GUEST_IDLE_TIME
#define MAX_WWW_GUEST_IDLE_TIME (3600)
#endif

#ifndef BBSUID
#error You must define BBSUID in site.h for your site!
#endif

#ifndef BBSGID
#error You must define BBSGID in site.h for your site!
#endif


#ifndef SECNUM /* 分区个数*/
#error must define SECNUM in your site.h
#endif

#ifndef DEFAULTBOARD
#define DEFAULTBOARD    	"test"
#endif

#ifndef FILTER_BOARD
#define FILTER_BOARD        "Filter"
#endif

#ifndef SYSMAIL_BOARD
#define SYSMAIL_BOARD       "sysmail"
#endif

#ifndef BLESS_BOARD
#define BLESS_BOARD "Blessing"
#endif

#ifndef MAXUSERS
#define MAXUSERS  		20000
#endif

#ifndef MAXCLUB
#define MAXCLUB			128
#endif

#ifndef MAXBOARD
#define MAXBOARD  		400
#endif

#ifndef MAXACTIVE
#define MAXACTIVE 		8000
#endif
/* remeber: if MAXACTIVE>46656 need change get_telnet_sessionid,
    make the number of session char from 3 to 4
    */

#ifndef MAX_GUEST_NUM
#define MAX_GUEST_NUM		800
#endif

#ifndef WWW_MAX_LOGIN
#define WWW_MAX_LOGIN 1000
#endif

#ifndef MAX_WWW_GUEST
#define MAX_WWW_GUEST 30016
#endif

#ifndef POP3PORT
#define POP3PORT		110
#endif

#ifndef POP3SPORT
#define POP3SPORT		995
#endif

#ifndef DELIVER
#define DELIVER "deliver"
#endif

/* ASCIIArt, by czz, 2002.7.5 */
#ifndef       LENGTH_SCREEN_LINE
#define       LENGTH_SCREEN_LINE      255
#endif

#ifndef       LENGTH_FILE_BUFFER
#define       LENGTH_FILE_BUFFER      255
#endif

#ifndef       LENGTH_ACBOARD_BUFFER
#define       LENGTH_ACBOARD_BUFFER   200
#endif

#ifndef       LENGTH_ACBOARD_LINE
#define       LENGTH_ACBOARD_LINE     300
#endif

#ifndef DAY_DELETED_CLEAN /*版面垃圾箱和回收站的清除天数*/
#define DAY_DELETED_CLEAN	97
#endif

#ifndef LEN_FRIEND_EXP		/*如果从FB2k升级到smth,这里请设置为40 */
#define LEN_FRIEND_EXP		15
#endif

#ifndef SEC_DELETED_OLDHOME
#define SEC_DELETED_OLDHOME	2592000 /* 3600*24*30，注册新用户如果存在这个用户的目录，保留时间 */
#endif

#ifndef	REGISTER_WAIT_TIME /*填注册单需要等待的时间*/
#define	REGISTER_WAIT_TIME	0
#endif

#ifndef REGISTER_WAIT_TIME_NAME	
#define	REGISTER_WAIT_TIME_NAME	"马上"
#endif

#ifndef MAIL_BBSDOMAIN
#define MAIL_BBSDOMAIN      "smth.org"
#endif

#ifndef NAME_BBS_ENGLISH
#define NAME_BBS_ENGLISH	"smth.org"
#endif

#ifndef	NAME_BBS_CHINESE
#define	NAME_BBS_CHINESE	"水木社区"
#endif

#ifndef BBS_FULL_NAME
#define BBS_FULL_NAME "BBS 水木社区站"
#endif

#ifndef FOOTER_MOVIE
#define FOOTER_MOVIE		"欢  迎  投  稿"
#endif

/*#define ISSUE_LOGIN		"本站使用曙光公司曙光天演服务器"*/
#ifndef ISSUE_LOGIN
#define ISSUE_LOGIN		"曙光PC  第21届大运会指定服务器"
#endif

#ifndef ISSUE_LOGOUT
#define ISSUE_LOGOUT		"还是走了罗"
#endif

#ifndef NAME_USER_SHORT
#define NAME_USER_SHORT		"用户"
#endif

#ifndef NAME_SYSOP
#define NAME_SYSOP		"System Operator"
#endif

#ifndef NAME_BM
#define NAME_BM			"版主"
#endif

#ifndef NAME_POLICE
#define NAME_POLICE		"警察"
#endif

#ifndef	NAME_SYSOP_GROUP
#define	NAME_SYSOP_GROUP	"站务组"
#endif

#ifndef NAME_ANONYMOUS /*匿名发文的昵称*/
#define NAME_ANONYMOUS		"水母青蛙 Today!"
#endif

#ifndef NAME_ANONYMOUS_FROM /*缺省匿名的来源*/
#define NAME_ANONYMOUS_FROM	"匿名天使的家"
#endif

#ifndef ANONYMOUS_DEFAULT /*缺省不匿名*/
#define ANONYMOUS_DEFAULT 0
#endif

#ifndef NAME_MATTER
#define NAME_MATTER		"站务"
#endif

#ifndef NAME_SYS_MANAGE
#define NAME_SYS_MANAGE		"系统维护"
#endif

#ifndef NAME_SEND_MSG
#define NAME_SEND_MSG		"送讯息"
#endif

#ifndef NAME_VIEW_MSG
#define NAME_VIEW_MSG		"送讯息"
#endif

#ifndef CHAT_MAIN_ROOM
#define CHAT_MAIN_ROOM		"main"
#endif

#ifndef CHAT_TOPIC
#define	CHAT_TOPIC		"大家先随便聊聊吧"
#endif

#ifndef CHAT_MSG_NOT_OP
#define CHAT_MSG_NOT_OP		"*** 您不是本聊天室的op ***"
#endif

#ifndef CHAT_ROOM_NAME
#define	CHAT_ROOM_NAME		"聊天室"
#endif

#ifndef CHAT_SERVER
#define	CHAT_SERVER		"聊天广场"
#endif

#ifndef CHAT_MSG_QUIT
#define CHAT_MSG_QUIT		"切离系统"
#endif

#ifndef CHAT_OP
#define CHAT_OP			"聊天室 op"
#endif

#ifndef CHAT_SYSTEM
#define CHAT_SYSTEM		"系统"
#endif

#ifndef CHAT_PARTY
#define	CHAT_PARTY		"大家"
#endif

#ifndef DEFAULT_NICK
#define DEFAULT_NICK		"每天爱你多一些"
#endif

#ifndef MSG_ERR_USERID
#define MSG_ERR_USERID		"错误的使用者代号..."
#endif

#ifndef LOGIN_PROMPT
#define LOGIN_PROMPT		"请输入代号"
#endif

#ifndef PASSWD_PROMPT
#define PASSWD_PROMPT		"请输入密码"
#endif

#ifndef DENY_NAME_SYSOP
#define DENY_DESC_AUTOFREE	"您被暂时取消在该版的发文权力"
#define DENY_DESC_NOAUTOFREE	DENY_DESC_AUTOFREE
#define DENY_BOARD_AUTOFREE	"被暂时取消在本版的发文权力"
#define DENY_BOARD_NOAUTOFREE	"被暂时取消在该版的发文权力，到期后请回复"
#define DENY_NAME_SYSOP		"值班站务"
#endif

#ifndef NUMPERMS
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
#define PERM_CHECKCD      0200000000
#define PERM_SUICIDE   0400000000
#define PERM_COLLECTIVE 	01000000000
#define PERM_DISS        02000000000
#define PERM_DENYMAIL   	04000000000


#define XPERMSTR "bTCPRp#@XWBA$VS!DEM1234567890%"
/* means the rest is a post mask */

/* This is the default permission granted to all new accounts. */
#define PERM_DEFAULT    (PERM_BASIC | PERM_CHAT | PERM_PAGE | PERM_POST | PERM_LOGINOK)
#define PERM_MULTILOG   (PERM_SYSOP | PERM_ANNOUNCE | PERM_OBOARDS)
#define PERM_MANAGER    (PERM_CHATOP | PERM_JURY | PERM_BOARDS)

#endif //permission define NUMPERMS





#ifndef PERM_AUTOSET
#define PERM_AUTOSET	PERM_BASIC
#endif


#ifndef NUMDEFINES

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
#define DEF_CHCHAR				02000000000
#define DEF_SHOWDETAILUSERDATA	04000000000
#define DEF_SHOWREALUSERDATA	010000000000
#define DEF_HIDEIP			040000000001LL
#define DEF_SHOWSIZE            040000000002LL

#define NUMDEFINES 34
#endif

/* Don't mess with this. */
#define HAS_PERM(user,x) ((x)?((user)->userlevel)&(x):1)
#define DEFINE(user,x)     ((x)?((user)->userdefine[def_list(x)])&(x):1)
#define TDEFINE(x) ((x)?(tmpuser)&(x):1)
#define SET_DEFINE(user,x) ((user)->userdefine[def_list(x)] |= x)
#define SET_UNDEFINE(user,x) ((user)->userdefine[def_list(x)] &= ~x)
#define SET_CHANGEDEFINE(user,x) ((user)->userdefine[def_list(x)] ^= x)


#define TDEF_SPLITSCREEN 000001



extern const char * const permstrings[];
extern const char    * const groups[];
extern const char * const user_definestr[];
extern const char * const user_definestr1[];
#ifdef NEW_HELP
extern const char * const helpmodestr[];
#endif

#ifdef SMS_SUPPORT
#define NUMSMSDEF 1
#define SMSDEF_AUTOTOMOBILE      000001
extern const char * const user_smsdefstr[];
#endif
extern const char * const mailbox_prop_str[];

extern const char seccode[SECNUM][5];
extern const char secname[SECNUM][2][20];

/**
 * 看在线用户时的按键处理字符。
 */
#ifndef UL_CHANGE_NICK_UPPER
#define UL_CHANGE_NICK_UPPER   'C'
#endif

#ifndef UL_CHANGE_NICK_LOWER
#define UL_CHANGE_NICK_LOWER   'c'
#endif

#ifndef UNREAD_SIGN
#define UNREAD_SIGN '*'
#endif

#ifndef UL_SWITCH_FRIEND_UPPER
#define UL_SWITCH_FRIEND_UPPER 'F'
#endif

#ifndef UL_SWITCH_FRIEND_LOWER
#define UL_SWITCH_FRIEND_LOWER 'f'
#endif


/**
 * 文章相关部分。
 */
#ifndef STRLEN
#define STRLEN          80
#endif

#ifndef BM_LEN
#define BM_LEN 60
#endif

#ifndef FILENAME_LEN
#define FILENAME_LEN 20
#endif

#ifndef OWNER_LEN
#define OWNER_LEN 14
#endif

#ifndef ARTICLE_TITLE_LEN
#define ARTICLE_TITLE_LEN 60
#endif

#ifdef HAVE_FILEHEADER_DEFINE

/**
 * 文章相关部分。
 */
typedef struct fileheader {     /* This structure is used to hold data in */
    char filename[FILENAME_LEN];        /* the DIR files */
    unsigned int id, groupid, reid;

    /* please kill these four fuckers */
    int o_bid;
    unsigned int o_id;
    unsigned int o_groupid;
    unsigned int o_reid;

    char innflag[2];
    char owner[OWNER_LEN];
    unsigned int eff_size; /* 版面文章: 表示有效字节数，信件: 表示文件大小 - atppp */
    int posttime;
    unsigned int attachment;
    char title[ARTICLE_TITLE_LEN];
    unsigned char accessed[4];
} fileheader;

typedef struct fileheader fileheader_t;
#endif

#ifndef GET_POSTFILENAME
#define GET_POSTFILENAME(x,y) get_postfilename(x,y,0)
#endif

#ifndef GET_MAILFILENAME
#define GET_MAILFILENAME(x,y) get_postfilename(x,y,0)
#endif

#ifndef VALID_FILENAME
#define VALID_FILENAME(x) valid_filename(x,0)
#endif

#ifndef POSTFILE_BASENAME
#define POSTFILE_BASENAME(x) (x)
#endif

#ifndef MAILFILE_BASENAME
#define MAILFILE_BASENAME(x) (x)
#endif

#ifndef SHOW_USERIP
#define SHOW_USERIP(y,x) x
#endif



/**
 * Mailbox properties.
 * 
 * @author flyriver
 */
#define MBP_SAVESENTMAIL      0x00000001
#define MBP_FORCEDELETEMAIL   0x00000002
#define MBP_MAILBOXSHORTCUT   0x00000004

#define MBP_NUMS 3

#ifndef MBP_DEFAULT
#define MBP_DEFAULT           0x00000001  //默认（新注册用户）的邮箱属性
#endif

#define HAS_MAILBOX_PROP(u, x) ((u)->mailbox_prop & x)
/**
 * Mailgroup macros.
 *
 * @author flyriver
 */
#define MAX_MAILGROUP_NUM 30
#define MAX_MAILGROUP_USERS 300


/***
  new help
*/
#ifdef NEW_HELP

#ifndef NUMHELPMODE

#define NUMHELPMODE 14
#define HELP_BOARD 1
#define HELP_GOODBOARD 2
#define HELP_ARTICLE 3
#define HELP_ANNOUNCE 4
#define HELP_EDIT 5
#define HELP_CHAT 6
#define HELP_VOTE 7
#define HELP_FRIEND 8
#define HELP_TIME 9
#define HELP_TMPL 10
#define HELP_FRIENDTEST 11
#define HELP_CHICKEN 12
#define HELP_KILLER 13
#define HELP_MAIL 14

#endif //NUMHELPMODE
#endif //NEW_HELP

/**
attach define
*/
#ifndef ATTACHTMPPATH
#define ATTACHTMPPATH "boards/_attach"
#endif

/*
 * max number of attachments in one article
 */
#ifndef MAXATTACHMENTCOUNT
#define MAXATTACHMENTCOUNT	20
#endif

#ifndef MAXATTACHMENTSIZE
#define MAXATTACHMENTSIZE 5*1024*1024                     //文章附件文件长度上限
#endif


#if CHECK_IP_LINK==0
#undef CHECK_IP_LINK
#endif

#ifndef ID_CONNECT_CON_THRESHOLD //防止上站机
#define ID_CONNECT_CON_THRESHOLD 20.0/60/60
#define ID_CONNECT_CON_THRESHOLD2 30.0
#endif

#ifndef CHECK_CONNECT /*是否检查连接频率并ban*/
#define CHECK_CONNECT
#else
#if CHECK_CONNECT == 0
#undef CHECK_CONNECT
#endif
#endif

#ifndef ZMODEM_RATE
#define ZMODEM_RATE 5000
#endif
#if ZMODEM_RATE == 0
#undef ZMODEM_RATE
#endif

#ifndef QUOTED_LINES
#define QUOTED_LINES 10
#endif

#ifndef MAXnettyLN
#define MAXnettyLN      5       /* lines of  activity board  */
#endif

#ifndef QUOTELEV
#define QUOTELEV 0
#endif

#ifndef FIRSTARTICLE_SIGN
#define FIRSTARTICLE_SIGN "●"
#endif

#ifndef BONLINE_LOGDIR /* 各类 log 在 BBSHOME 下的存放目录 */
#define BONLINE_LOGDIR "bonlinelog"
#endif


#ifndef USERD_PORT
#define USERD_PORT  60001
#endif

#define ISV4ADDR(addr) ((addr.s6_addr[0]==0)&&(addr.s6_addr[1]==0)&& \
	(addr.s6_addr[2]==0)&&(addr.s6_addr[3]==0)&&(addr.s6_addr[4]==0)&& \
	(addr.s6_addr[5]==0)&&(addr.s6_addr[6]==0)&&(addr.s6_addr[7]==0)&& \
	(addr.s6_addr[8]==0)&&(addr.s6_addr[9]==0)&& \
	(addr.s6_addr[10]==0xff)&&(addr.s6_addr[11]==0xff))



/* old bbsconfig.h */

/* 
   Turn this on to allow users to create their own accounts by typing 'new'
   at the "Enter userid:" prompt. Comment out to restrict access to accounts
   created by the Sysop (see important note in README.install). 
*/
#define LOGINASNEW 1 

#define MAXSIGLINES    6 /* max. # of lines appended for post signature */

/*Define this for Use Notepad.*/
#define USE_NOTEPAD 1

/* end old bbsconfig.h */

#if !defined(DELETE_RANGE_RESERVE_DIGEST) && defined(FREE)
#define DELETE_RANGE_RESERVE_DIGEST 1
#endif /* !DELETE_RANGE_RESERVE_DIGEST && FREE */

#if defined(DELETE_RANGE_RESERVE_DIGEST) && (DELETE_RANGE_RESERVE_DIGEST == 0)
#undef DELETE_RANGE_RESERVE_DIGEST
#endif /* DELETE_RANGE_RESERVE_DIGEST && DELETE_RANGE_RESERVE_DIGEST == 0 */

#endif /* __SYS_DEFAULT_H_ */

