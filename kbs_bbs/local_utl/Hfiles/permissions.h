
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
#define UNUSE2         04000000
#define UNUSE3         010000000
#define UNUSE4         020000000
#define UNUSE5         040000000
#define UNUSE6         0100000000
#define UNUSE7         0200000000
#define UNUSE8         0400000000
#define UNUSE9         01000000000
#define UNUSE10        02000000000
#define UNUSE11        04000000000


/* means the rest is a post mask */

/* This is the default permission granted to all new accounts. */
#define PERM_DEFAULT    (PERM_BASIC | PERM_CHAT | PERM_PAGE | PERM_POST | PERM_LOGINOK)

/* These permissions are bitwise ORs of the basic bits. They work that way
   too. For example, anyone with PERM_SYSOP or PERM_BOARDS or both has
   PERM_SEEBLEVELS. */

#define PERM_ADMINMENU  (PERM_ACCOUNTS | PERM_OVOTE | PERM_SYSOP|PERM_OBOARDS|PERM_WELCOME)
#define PERM_MULTILOG   PERM_SYSOP
#define PERM_LOGINCLOAK (PERM_SYSOP | PERM_ACCOUNTS | PERM_BOARDS | PERM_WELCOME)
#define PERM_SEEULEVELS (PERM_SYSOP | PERM_BOARDS)
#define PERM_SEEBLEVELS (PERM_SYSOP | PERM_BOARDS)
#define PERM_MARKPOST   (PERM_SYSOP | PERM_BOARDS)
#define PERM_UCLEAN     (PERM_SYSOP | PERM_ACCOUNTS)
#define PERM_NOTIMEOUT  PERM_SYSOP

#define PERM_SENDMAIL   0
#define PERM_READMAIL   PERM_BASIC
#define PERM_VOTE       PERM_BASIC

/* These are used only in Internet Mail Forwarding */
/* You may want to be more restrictive than the default, especially for an
   open access BBS. */

#define PERM_SETADDR    PERM_BASIC     /* to set address for forwarding */
#define PERM_FORWARD    PERM_BASIC     /* to do the forwarding */

/* Don't mess with this. */
#define HAS_PERM(x)     ((x)?currentuser.userlevel&(x):1)
#define DEFINE(x)     ((x)?currentuser.userdefine&(x):1)


#ifndef EXTERN
extern char *permstrings[];
#else

/*#define NUMPERMS (30)*//*Define in bbs.h*/

/* You might want to put more descriptive strings for SPECIAL1 and SPECIAL2
   depending on how/if you use them. */
char *permstrings[] = {
        "基本权力",             /* PERM_BASIC */
        "进入聊天室",           /* PERM_CHAT */
        "呼叫他人聊天",         /* PERM_PAGE */
        "发表文章",             /* PERM_POST */
        "使用者资料正确",       /* PERM_LOGINOK */
        "禁止发表文章",         /* PERM_DENYPOST */
        "网路游戏(QKMJ)",       /* PERM_CLOAK */
        "人工智慧群",           /* PERM_SEECLOAK */
        "帐号永久保留",         /* PERM_XEMPT */
        "编辑系统档案",         /* PERM_WELCOME */
        "板主",                 /* PERM_BOARDS */
        "帐号管理员",           /* PERM_ACCOUNTS */
        "四百年来第一站智囊团", /* PERM_CHATCLOAK */
        "投票管理员",           /* PERM_OVOTE */
        "系统维护管理员",       /* PERM_SYSOP */
        "Read/Post 限制",       /* PERM_POSTMASK */
        "精华区总管",           /* PERM_ANNOUNCE*/
        "讨论区总管",           /* PERM_OBOARDS*/
        "活动看版总管",         /* PERM_ACBOARD*/
        "不能 ZAP(讨论区专用)", /* PERM_NOZAP*/
        "特殊权限 2",           /* PERM_UNUSE?*/
        "特殊权限 3",           /* PERM_UNUSE?*/
        "特殊权限 4",           /* PERM_UNUSE?*/
        "特殊权限 5",           /* PERM_UNUSE?*/
        "特殊权限 6",           /* PERM_UNUSE?*/
        "特殊权限 7",           /* PERM_UNUSE?*/
        "特殊权限 8",           /* PERM_UNUSE?*/
        "特殊权限 9",           /* PERM_UNUSE?*/
        "特殊权限10 ",           /* PERM_UNUSE?*/
        "特殊权限11",           /* PERM_UNUSE?*/

};
#endif

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
/*#define PERM_POSTMASK  0100000  */   /* means the rest is a post mask */


#define NUMDEFINES 23
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
        "萤幕标题色彩：一般/变换",/* DEF_TITLECOLOR */
        "接受所有人的讯息",     /* DEF_ALLMSG */
        "接受好友的讯息",       /* DEF_FRIENDMSG */
        "收到讯息发出声音",     /* DEF_SOUNDMSG */
        "离站後寄回所有讯息",   /* DEF_MAILMSG */
        "所有好友上站均通知",         /* DEF_LOGININFORM */
        "菜单上显示帮助信息",         /* DEF_SHOWSCREEN */
        "进站时显示十大新闻",         /* DEF_SHOWHOT */
        "进站时观看留言版",         /* DEF_NOTEPAD*/
};
#endif

