
/*
   These are the 16 basic permission bits. 
   All but the last one are used in comm_lists.c and help.c to control user 
   access to priviliged functions. The symbolic names are given to roughly
   correspond to their actual usage; feel free to use them for different
   purposes though. The PERM_SPECIAL1 and PERM_SPECIAL2 are not used by 
   default and you can use them to set up restricted boards or chat rooms.
*/

#define PERM_BASIC      000001
#define PERM_CHAT       000002
#define PERM_PAGE       000004
#define PERM_POST       000010
#define PERM_LOGINOK    000020
#define PERM_DENYPOST   000040
#define PERM_CLOAK      000100
#define PERM_SEECLOAK   000200
#define PERM_XEMPT      000400
#define PERM_WELCOME    001000
#define PERM_BOARDS     002000
#define PERM_ACCOUNTS   004000
#define PERM_CHATCLOAK  010000
#define PERM_OVOTE      020000
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
#define PERM_SEXY      0200000000
#define PERM_SUICIDE   0400000000
#define PERM_MM	 	01000000000
#define PERM_DISS        02000000000
#define PERM_DENYMAIL   	04000000000


/* means the rest is a post mask */

/* This is the default permission granted to all new accounts. */
#define PERM_DEFAULT    (PERM_BASIC | PERM_CHAT | PERM_PAGE | PERM_POST | PERM_LOGINOK)

/* These permissions are bitwise ORs of the basic bits. They work that way
   too. For example, anyone with PERM_SYSOP or PERM_BOARDS or both has
   PERM_SEEBLEVELS. */

#define PERM_ADMINMENU  (PERM_ACCOUNTS | PERM_OVOTE | PERM_SYSOP|PERM_OBOARDS|PERM_WELCOME)
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

#define PERM_SETADDR    PERM_BASIC     /* to set address for forwarding */
#define PERM_FORWARD    PERM_BASIC     /* to do the forwarding */

/* Don't mess with this. */
#define HAS_PERM(user,x) ((x)?((user)->userlevel)&(x):1)
#define DEFINE(user,x)     ((x)?((user)->userdefine)&(x):1)


/*#define NUMPERMS (30)*//*Define in bbs.h*/

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
#define DEF_IGNOREMSG    040000000   /* Added by Marco */
/*#define DEF_IGNOREANSIX  0100000000   Leeward 98.01.12 */
#define DEF_SHOWSTATISTIC 0200000000 /* Haohmaru */
#define DEF_UNREADMARK 0400000000 /* Luzi 99.01.12 */
#define DEF_USEGB     01000000000 /* KCN,99.09.05 */
/*#define DEF_HIDEIP    02000000000  Haohmaru,99.12.18*/

/*#define PERM_POSTMASK  0100000  */   /* means the rest is a post mask */


/* #define NUMDEFINES 23 */
/* #define NUMDEFINES 24 */
#define NUMDEFINES 28 /* Leeward 98.01.12,Haohmaru,98.09.24 KCN,99.09.03,要用DEF_HIDEIP时请改为29*/

#ifndef EXTERN
extern char *user_definestr[];
#else
/* You might want to put more descriptive strings for SPECIAL1 and SPECIAL2
   depending on how/if you use them. */
char *user_definestr[] = {
        "活动看版",             /* DEF_ACBOARD */
        "使用彩色",             /* DEF_COLOR */
        "编辑时显示状态栏",     /* DEF_EDITMSG */
        "分类讨论区以 New 显示",/* DEF_NEWPOST */
        "选单的讯息栏",         /* DEF_ENDLINE */
        "上站时显示好友名单",   /* DEF_LOGFRIEND */
        "让好友呼叫",           /* DEF_FRIENDCALL */
        "使用自己的离站画面",   /* DEF_LOGOUT */
        "进站时显示备忘录",     /* DEF_INNOTE */
        "离站时显示备忘录",     /* DEF_OUTNOTE */
        "讯息栏模式：呼叫器/人数",/* DEF_NOTMSGFRIEND */
        "菜单模式选择：一般/精简",/* DEF_NORMALSCR */
        "阅读文章是否使用绕卷选择",/* DEF_CIRCLE */
        "阅读文章游标停於第一篇未读",/* DEF_FIRSTNEW */
        "屏幕标题色彩：一般/变换",/* DEF_TITLECOLOR */
        "接受所有人的讯息",     /* DEF_ALLMSG */
        "接受好友的讯息",       /* DEF_FRIENDMSG */
        "收到讯息发出声音",     /* DEF_SOUNDMSG */
        "离站後寄回所有讯息",   /* DEF_MAILMSG */
        "发文章时实时显示讯息",/*"所有好友上站均通知",    DEF_LOGININFORM */
        "菜单上显示帮助信息",   /* DEF_SHOWSCREEN */
        "进站时显示十大新闻",   /* DEF_SHOWHOT */
        "进站时观看留言版",     /* DEF_NOTEPAD*/
        "忽略讯息功能键: Enter/Esc", /* DEF_IGNOREMSG */
        "未使用", /* DEF_IGNOREANSIX */ /* Leeward 98.01.12 */
/*        "禁止运行ANSI扩展指令",  DEF_IGNOREANSIX */ /* Leeward 98.01.12 */
        "进站时观看上站人数统计图",  /* DEF_SHOWSTATISTIC Haohmaru 98.09.24*/
        "未读标记使用 *", /* DEF_UNREADMARK Luzi 99.01.12 */
        "使用GB码阅读", /* DEF_USEGB KCN 99.09.03 */
	"隐藏自己的IP", /* DEF_HIDEIP Haohmaru 99.12.18 */
};
#endif

