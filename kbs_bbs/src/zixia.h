#ifndef __SYSNAME_H_
#define __SYSNAME_H_

#define ZIXIA

#define BBSUID 			80 //9999
#define BBSGID 			80 //99

/* for bbs2www, by flyriver, 2001.3.9 */
#define SECNUM 10

#define DEFAULTBOARD    	"zixia.net"//test
#define MAXUSERS  		40000 //150,000
#define MAXBOARD  		256 //400
#define MAXACTIVE 		999  //3000
#define MAX_GUEST_NUM		99

#define POP3PORT		3110	//110
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

#define MSG_ERR_USERID		"嗯？这个猪头是谁？..."



#ifndef EXTERN
extern char *permstrings[];
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
        NAME_BBS_CHINESE "智囊团",       /* PERM_CHATCLOAK */
        "投票管理员",           /* PERM_OVOTE */
        "系统维护管理员",       /* PERM_SYSOP */
        "Read/Post 限制",       /* PERM_POSTMASK */
        "精华区总管",           /* PERM_ANNOUNCE*/
        "讨论区总管",           /* PERM_OBOARDS*/
        "活动看版总管",         /* PERM_ACBOARD*/
        "不能 ZAP(讨论区专用)", /* PERM_NOZAP*/
        "聊天室OP(元老院专用)", /* PERM_CHATOP */
        "系统总管理员",         /* PERM_ADMIN */
        "荣誉帐号",          	/* PERM_HORNOR*/
        "看秘密精华区",         /* PERM_SECANC*/
        "看AKA版",           /* PERM_GIRL*/
        "看Sexy版",           /* PERM_SEXY*/
        "自杀进行中",           /* PERM_SUICIDE?*/
        "观音姐姐",           /* PERM_MM*/
        "看系统讨论版",           /* PERM_DISS*/
        "封禁Mail",           /* PERM_DENYMAIL*/

};
#endif

static char    *explain[] = {
	"斧 头 帮",
	"大话西游",
	"大小分舵",
        "吃喝玩乐",
	"唧唧歪歪",
	"吟风弄月",
        "西游之路",
        "豆腐西施",
	"月光宝盒",
        "大圣取经",
         NULL
};
 
static char    *groups[] = {
	"AxFaction",
	"zixia",
	"Factions",
	"Entertain",
	"Watering",
	"Poem",
	"GoWest",
	"DouFuGirl",
  	"PandoraBox",
  	"Reserve",
         NULL
};
#endif
