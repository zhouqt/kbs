#ifndef __SYSNAME_H_
#define __SYSNAME_H_

#define BBSUID 			9999
#define BBSGID 			99

/* for bbs2www, by flyriver, 2001.3.9 */
#define SECNUM 9

#define DEFAULTBOARD    	"test"
#define MAXUSERS  		160000
#define MAXBOARD  		400
#define MAXACTIVE 		8000
#define MAX_GUEST_NUM		800

#define POP3PORT		110
#define	LENGTH_SCREEN_LINE	220
#define	LENGTH_FILE_BUFFER 	160
#define	LENGTH_ACBOARD_BUFFER	150
#define	LENGTH_ACBOARD_LINE 	80

#define LIFE_DAY_USER		120
#define LIFE_DAY_LONG		666
#define LIFE_DAY_SYSOP		119
#define LIFE_DAY_NODIE		999
#define LIFE_DAY_NEW		15
#define LIFE_DAY_SUICIDE	15

#define DAY_DELETED_CLEAN	20
#define SEC_DELETED_OLDHOME	2592000/* 3600*24*30，注册新用户如果存在这个用户的目录，保留时间*/

#define	REGISTER_WAIT_TIME	(72*60*60)
#define	REGISTER_WAIT_TIME_NAME	"72 小时"

#define MAIL_BBSDOMAIN      "smth.org"
#define MAIL_MAILSERVER     "166.111.8.236:25"

#define NAME_BBS_ENGLISH	"smth.org"
#define	NAME_BBS_CHINESE	"水木清华"
#define NAME_BBS_NICK		"BBS 站"

#define FOOTER_MOVIE		"欢  迎  投  稿"
/*#define ISSUE_LOGIN		"本站使用曙光公司曙光天演服务器"*/
#define ISSUE_LOGIN		"曙光PC  第21届大运会指定服务器"
#define ISSUE_LOGOUT		"还是走了罗"

#define NAME_USER_SHORT		"用户"
#define NAME_USER_LONG		"水木用户"
#define NAME_SYSOP		"System Operator"
#define NAME_BM			"版主"
#define NAME_POLICE		"警察"
#define	NAME_SYSOP_GROUP	"站务组"
#define NAME_ANONYMOUS		"水母青蛙 Today!"
#define NAME_ANONYMOUS_FROM	"匿名天使的家"

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

#ifndef EXTERN
extern char *permstrings[];
extern char    *groups[];
extern char    *explain[];
#else
char *permstrings[] = {
        "基本权力",             /* PERM_BASIC */
        "进入聊天室",           /* PERM_CHAT */
        "呼叫他人聊天",         /* PERM_PAGE */
        "发表文章",             /* PERM_POST */
        "使用者资料正确",       /* PERM_LOGINOK */
        "禁止发表文章",         /* PERM_DENYPOST */
        "可隐身",               /* PERM_CLOAK */
        "可见隐身",             /* PERM_SEECLOAK */
        "长期帐号",         /* PERM_XEMPT */
        "编辑系统档案",         /* PERM_WELCOME */
        "板主",                 /* PERM_BOARDS */
        "帐号管理员",           /* PERM_ACCOUNTS */
        "水木清华智囊团",       /* PERM_CHATCLOAK */
        "投票管理员",           /* PERM_OVOTE */
        "系统维护管理员",       /* PERM_SYSOP */
        "Read/Post 限制",       /* PERM_POSTMASK */
        "精华区总管",           /* PERM_ANNOUNCE*/
        "讨论区总管",           /* PERM_OBOARDS*/
        "活动看版总管",         /* PERM_ACBOARD*/
        "不能 ZAP(讨论区专用)", /* PERM_NOZAP*/
        "聊天室OP(元老院专用)", /* PERM_CHATOP */
        "系统总管理员",         /* PERM_ADMIN */
        "荣誉帐号",           /* PERM_HONOR*/
        "特殊权限 5",           /* PERM_UNUSE?*/
        "仲裁委员",           /* PERM_JURY*/
        "特殊权限 7",           /* PERM_UNUSE?*/
        "自杀进行中",        /*PERM_SUICIDE*/
        "特殊权限 9",           /* PERM_UNUSE?*/
        "看系统讨论版",           /* PERM_UNUSE?*/
        "封禁Mail",           /* PERM_DENYMAIL*/

};

char    *explain[] = {
    "本站系统",
    "休闲娱乐",
    "电脑技术",
    "学术科学",
    "体育健身",
    "谈天说地",
    "校园信息",
    "艺术文化",
    "人文社会",
    "网络信息",
    "清华大学",
    "兄弟院校",
    "其  他",
    NULL
};

char    *groups[] = {
    "system.faq",
    "rec.faq",
    "comp.faq",
    "sci.faq",
    "sport.faq",
    "talk.faq",
    "campus.faq",
    "literal.faq",
    "soc.faq",
    "network.faq",
    "thu.faq",
    "univ.faq",
    "other.faq",
    NULL
};
#endif

#endif
