#include "bbs.h"

#ifdef USE_DEFAULT_ALPHABET
const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
#endif

#ifdef USE_DEFAULT_SECODE
const char seccode[SECNUM][5] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C",
};
const char    *groups[] = {
    "system.faq",
    "thu.faq",
    "univ.faq",
    "sci.faq",
    "rec.faq",
    "literal.faq",
    "social.faq",
    "game.faq",
    "sport.faq",
    "talk.faq",
    "info.faq",
    "develop.faq",
    "os.faq",
    "comp.faq",
    NULL
};
const char secname[SECNUM][2][20] = {
    {"BBS 系统", "[站内]"},
    {"清华大学", "[本校]"},
    {"学术科学", "[学科/语言]"},
    {"休闲娱乐", "[休闲/音乐]"},
    {"文化人文", "[文化/人文]"},
    {"社会信息", "[社会/信息]"},
    {"游戏天地", "[游戏/对战]"},
    {"体育健身", "[运动/健身]"},
    {"知性感性", "[谈天/感性]"},
    {"电脑信息", "[电脑/信息]"},
    {"软件开发", "[语言/工具]"},
    {"操作系统", "[系统/内核]"},
    {"电脑技术", "[专项技术]"},
};

const char    *explain[] = {
    "本站系统",
    "清华大学",
    "校园信息",
    "学术科学",
    "休闲娱乐",
    "文化人文",
    "社会信息",
    "游戏天地",
    "体育健身",
    "知性感性",
    "电脑信息",
    "软件开发",
    "操作系统",
    "电脑技术",
    NULL
};

#endif

#ifdef USE_DEFAULT_PERMSTRINGS
const char *permstrings[] = {
        "基本权力",             /* PERM_BASIC */
        "进入聊天室",           /* PERM_CHAT */
        "呼叫他人聊天",         /* PERM_PAGE */
        "发表文章",             /* PERM_POST */
        "使用者资料正确",       /* PERM_LOGINOK */
        "实习站务",         /* PERM_BMMANAGER */
        "可隐身",               /* PERM_CLOAK */
        "可见隐身",             /* PERM_SEECLOAK */
        "长期帐号",         /* PERM_XEMPT */
        "编辑系统档案",         /* PERM_WELCOME */
        "板主",                 /* PERM_BOARDS */
        "帐号管理员",           /* PERM_ACCOUNTS */
        "水木清华智囊团",       /* PERM_CHATCLOAK */
        "封禁娱乐权限",           /* PERM_DENYRELAX */
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
        "集体专用帐号",           /* PERM_COLLECTIVE*/
        "看系统讨论版",           /* PERM_UNUSE?*/
        "封禁Mail",           /* PERM_DENYMAIL*/

};
#endif

#ifdef USE_DEFAULT_DEFINESTR
/* You might want to put more descriptive strings for SPECIAL1 and SPECIAL2
   depending on how/if you use them. */
const char *user_definestr[] = {
    "活动看版",                 /* DEF_ACBOARD */
    "使用彩色",                 /* DEF_COLOR */
    "编辑时显示状态栏",         /* DEF_EDITMSG */
    "分类讨论区以 New 显示",    /* DEF_NEWPOST */
    "选单的讯息栏",             /* DEF_ENDLINE */
    "上站时显示好友名单",       /* DEF_LOGFRIEND */
    "让好友呼叫",               /* DEF_FRIENDCALL */
    "使用自己的离站画面",       /* DEF_LOGOUT */
    "进站时显示备忘录",         /* DEF_INNOTE */
    "离站时显示备忘录",         /* DEF_OUTNOTE */
    "讯息栏模式：呼叫器/人数",  /* DEF_NOTMSGFRIEND */
    "菜单模式选择：一般/精简",  /* DEF_NORMALSCR */
    "阅读文章是否使用绕卷选择", /* DEF_CIRCLE */
    "阅读文章游标停於第一篇未读",       /* DEF_FIRSTNEW */
    "屏幕标题色彩：一般/变换",  /* DEF_TITLECOLOR */
    "接受所有人的讯息",         /* DEF_ALLMSG */
    "接受好友的讯息",           /* DEF_FRIENDMSG */
    "收到讯息发出声音",         /* DEF_SOUNDMSG */
    "离站后寄回所有讯息",       /* DEF_MAILMSG */
    "发文章时实时显示讯息",     /*"所有好友上站均通知",    DEF_LOGININFORM */
    "菜单上显示帮助信息",       /* DEF_SHOWSCREEN */
    "进站时显示十大新闻",       /* DEF_SHOWHOT */
    "进站时观看留言版",         /* DEF_NOTEPAD */
    "忽略讯息功能键: Enter/Esc",        /* DEF_IGNOREMSG */
    "使用高亮界面",                   /* DEF_HIGHCOLOR */
    "观看人数统计和祝福榜", /* DEF_SHOWSTATISTIC Haohmaru 98.09.24 */
    "未读标记使用 *",           /* DEF_UNREADMARK Luzi 99.01.12 */
    "使用GB码阅读",             /* DEF_USEGB KCN 99.09.03 */
    "对汉字进行整字处理",  /* DEF_SPLITSCREEN 2002.9.1 */
    "显示详细用户数据",  /*DEF_SHOWDETAILUSERDATA 2003.7.31 */
    "显示真实用户数据" /*DEF_REALDETAILUSERDATA 2003.7.31 */
};
#endif

#ifdef USE_DEFAULT_MAILBOX_PROP_STR
const char *mailbox_prop_str[] =
{
	"发信时保存信件到发件箱",
	"删除信件时不保存到垃圾箱",
	"版面按 'v' 时进入: 收件箱(OFF) / 信箱主界面(ON)",
};
#endif

#ifdef USE_DEFAULT_LEVELCHAR
int uleveltochar(char *buf, struct userec *lookupuser)
{                               /* 取用户权限中文说明 Bigman 2001.6.24 */
    unsigned lvl;
    char userid[IDLEN + 2];

    lvl = lookupuser->userlevel;
    strncpy(userid, lookupuser->userid, IDLEN + 2);

#ifdef HAVE_CUSTOM_USER_TITLE
    if (lookupuser->title!=0) {
        strcpy(buf,get_user_title(lookupuser->title));
        if (buf[0]!=0) return 0;
    }
#endif
    if (!(lvl & PERM_BASIC) && !(lookupuser->flags & GIVEUP_FLAG)) {
        strcpy(buf, "新人");
        return 0;
    }
/*    if( lvl < PERM_DEFAULT )
    {
        strcpy( buf, "- --" );
        return 1;
    }
*/

    /* Bigman: 增加中文查询显示 2000.8.10 */
    /*if( lvl & PERM_ZHANWU ) strcpy(buf,"站务"); */
    if ((lvl & PERM_ANNOUNCE) && (lvl & PERM_OBOARDS))
        strcpy(buf, "站务");
    else if (lvl & PERM_JURY)
        strcpy(buf, "仲裁");    /* stephen :增加中文查询"仲裁" 2001.10.31 */
    else if (lvl & PERM_BMAMANGER)
        strcpy(buf, "实习站务");
    else if (lvl & PERM_COLLECTIVE)
        strcpy(buf, "专用");
    else if (lvl & PERM_CHATCLOAK)
        strcpy(buf, "元老");
    else if (lvl & PERM_CHATOP)
        strcpy(buf, "ChatOP");
    else if (lvl & PERM_BOARDS)
        strcpy(buf, "版主");
    else if (lvl & PERM_HORNOR)
        strcpy(buf, "荣誉");
    /* Bigman: 修改显示 2001.6.24 */
    else if (lvl & (PERM_LOGINOK)) {
        if (lookupuser->flags & GIVEUP_FLAG)
            strcpy(buf, "戒网");
        else if (!(lvl & (PERM_CHAT)) || !(lvl & (PERM_PAGE)) || !(lvl & (PERM_POST)) 
        		|| (lvl & (PERM_DENYMAIL)) || (lvl & (PERM_DENYRELAX)))
            strcpy(buf, "受限");
        else
            strcpy(buf, "用户");
    } else if (lookupuser->flags & GIVEUP_FLAG)
        strcpy(buf, "戒网");
    else if (!(lvl & (PERM_CHAT)) && !(lvl & (PERM_PAGE)) && !(lvl & (PERM_POST)))
        strcpy(buf, "新人");
    else
        strcpy(buf, "受限");

/*    else {
        buf[0] = (lvl & (PERM_SYSOP)) ? 'C' : ' ';
        buf[1] = (lvl & (PERM_XEMPT)) ? 'L' : ' ';
        buf[2] = (lvl & (PERM_BOARDS)) ? 'B' : ' ';
        buf[3] = !(lvl & (PERM_POST)) ? 'p' : ' ';
        if( lvl & PERM_ACCOUNTS ) buf[3] = 'A';
        if( lvl & PERM_SYSOP ) buf[3] = 'S'; 
        buf[4] = '\0';
    }
*/

    return 1;
}
#endif

#ifdef USE_DEFAULT_MODE

#include "modes.h"

char *ModeType(mode)
    int mode;
{
    switch (mode) {
    case IDLE:
        return "";
    case NEW:
        return "新站友注册";
    case LOGIN:
        return "进入本站";
    case CSIE_ANNOUNCE:
        return "汲取精华";
//    case CSIE_TIN:
//        return "使用TIN";
//    case CSIE_GOPHER:
//        return "使用Gopher";
    case MMENU:
        return "主菜单";
    case ADMIN:
        return "系统维护";
    case SELECT:
        return "选择讨论区";
    case READBRD:
        return "浏览讨论区";
    case READNEW:
        return "阅读新文章";
    case READING:
        return "阅读文章";
    case POSTING:
        return "发表文章";
    case MAIL:
        return "信件选单";
    case SMAIL:
        return "寄信中";
    case RMAIL:
        return "读信中";
    case TMENU:
        return "谈天说地区";
    case LUSERS:
        return "看谁在线上";
    case FRIEND:
        return "找线上好友";
//    case MONITOR:
//        return "监看中";
    case QUERY:
        return "查询网友";
    case TALK:
        return "聊天";
    case PAGE:
        return "呼叫网友";
//    case CHAT2:
//        return "梦幻国度";
    case CHAT1:
        return "聊天室中";
//    case CHAT3:
//        return "快哉亭";
//    case CHAT4:
//        return "老大聊天室";
//    case IRCCHAT:
//        return "会谈IRC";
    case LAUSERS:
        return "探视网友";
    case XMENU:
        return "系统资讯";
    case VOTING:
        return "投票";
    case BBSNET:
        return "系统维护"; //ft
	/*
        return "穿梭银河";
	*/
//    case EDITWELC:
//        return "编辑 Welc";
    case EDITUFILE:
        return "编辑档案";
    case EDITSFILE:
        return "系统管理";
        /*        case  EDITSIG:  return "刻印";
           case  EDITPLAN: return "拟计画"; */
//    case ZAP:
//        return "订阅讨论区";
//    case EXCE_MJ:
//        return "围城争霸";
//    case EXCE_BIG2:
//        return "比大营";
//    case EXCE_CHESS:
//        return "楚河汉界";
    case NOTEPAD:
        return "留言板";
    case GMENU:
        return "工具箱";
//    case FOURM:
//        return "4m Chat";
//    case ULDL:
//        return "UL/DL";
    case MSG:
        return "讯息中";
    case USERDEF:
        return "自订参数";
    case EDIT:
        return "修改文章";
    case OFFLINE:
        return "自杀中..";
    case EDITANN:
        return "编修精华";
    case WEBEXPLORE:
        return "Web浏览";
//    case CCUGOPHER:
//        return "他站精华";
    case LOOKMSGS:
        return "察看讯息";
    case WFRIEND:
        return "寻人名册";
    case LOCKSCREEN:
        return "屏幕锁定";
    case IMAIL:
	return "寄站外信中";
    case GIVEUPNET:
        return "戒网中..";
    case SERVICES:
        return "休闲娱乐..";
	case FRIENDTEST:  return "心有灵犀";
    case CHICKEN:
	return "星空战斗鸡";
    default:
        return "去了那里!?";
    }
}

#endif

#ifdef USE_DEFAULT_MULTILOGIN_CHECK

int multilogin_user(struct userec *user, int usernum,int mode)
{
    return 0;
}

#endif

#ifdef USE_DEFAULT_USER_LIFE
#define LIFE_DAY_USER		120
#define LIFE_DAY_YEAR          365
#define LIFE_DAY_LONG		666
#define LIFE_DAY_SYSOP		120
#define LIFE_DAY_NODIE		999
#define LIFE_DAY_NEW		15
#define LIFE_DAY_SUICIDE	15
int compute_user_value(struct userec *urec)
{
    int value;
    int registeryear;
    int basiclife;

    /* if (urec) has CHATCLOAK permission, don't kick it */
    /* 元老和荣誉帐号 在不自杀的情况下， 生命力999 Bigman 2001.6.23 */
    /* 这个是死人的id,sigh */
    if ((urec->userlevel & PERM_HORNOR) && !(urec->userlevel & PERM_LOGINOK))
        return LIFE_DAY_LONG;


    if (((urec->userlevel & PERM_HORNOR) || (urec->userlevel & PERM_CHATCLOAK)) && (!(urec->userlevel & PERM_SUICIDE)))
        return LIFE_DAY_NODIE;

    if ((urec->userlevel & PERM_ANNOUNCE) && (urec->userlevel & PERM_OBOARDS))
        return LIFE_DAY_SYSOP;
    /* 站务人员生命力不变 Bigman 2001.6.23 */


    value = (time(0) - urec->lastlogin) / 60;   /* min */
    if (0 == value)
        value = 1;              /* Leeward 98.03.30 */

    /* new user should register in 30 mins */
    if (strcmp(urec->userid, "new") == 0) {
        return (LIFE_DAY_NEW - value) / 60;     /* *->/ modified by dong, 1998.12.3 */
    }

    /* 自杀功能,Luzi 1998.10.10 */
    if (urec->userlevel & PERM_SUICIDE)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value) / (60 * 24);
    /**********************/
    if (urec->numlogins <= 3)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value) / (60 * 24);
    if (!(urec->userlevel & PERM_LOGINOK))
        return (LIFE_DAY_NEW * 24 * 60 - value) / (60 * 24);
    /* if (urec->userlevel & PERM_LONGID)
       return (667 * 24 * 60 - value)/(60*24); */
    registeryear = (time(0) - urec->firstlogin) / 31536000;
    if (registeryear < 2)
        basiclife = LIFE_DAY_USER + 1;
    else if (registeryear >= 5)
        basiclife = LIFE_DAY_LONG + 1;
    else
        basiclife = LIFE_DAY_YEAR + 1;
    return (basiclife * 24 * 60 - value) / (60 * 24);
}
#endif

#ifdef USE_DEFAULT_ANNOUNCE_FILENAME
/**
 * 精华区相关函数。
 */
int ann_get_postfilename(char *filename, struct fileheader *fileinfo,
						MENU *pm)
{
	char fname[PATHLEN];
	char *ip;

	strcpy(filename, fileinfo->filename);
	sprintf(fname, "%s/%s", pm->path, filename);
	ip = &filename[strlen(filename) - 1];
	while (dashf(fname)) {
		if (*ip == 'Z')
			ip++, *ip = 'A', *(ip + 1) = '\0';
		else
			(*ip)++;
		sprintf(fname, "%s/%s", pm->path, filename);
	}
}
#endif

#ifdef USE_DEFAULT_GET_POSTTIME
/**
 * 文章相关函数。
 */
time_t get_posttime(const struct fileheader *fileinfo)
{
	return atoi(fileinfo->filename + 2);
}
#endif

#ifdef USE_DEFAULT_SET_POSTTIME
void set_posttime(struct fileheader *fileinfo)
{
	return;
}
#endif

#ifdef USE_DEFAULT_SET_POSTTIME2
void set_posttime2(struct fileheader *dest, struct fileheader *src)
{
	return;
}
#endif

#ifdef USE_DEFAULT_BUILD_BOARD
/**
 * 版面相关。
 */
void build_board_structure(const char *board)
{
	return;
}
#endif


#ifdef USE_DEFAULT_MAIL_LIMIT /*邮件容量限制*/
void get_mail_limit(struct userec* user,int *sumlimit,int * numlimit)
{
	/*unlimit*/
	*sumlimit = 9999;
	*numlimit = 9999;
	return;
}
#endif

/* board permissions control */
#ifdef USE_DEFAULT_READ_PERM
int check_read_perm(struct userec *user, const struct boardheader *board)
{
    if (board == NULL)
        return 0;
    if (!HAS_PERM(user, PERM_OBOARDS)&&board->title_level
        &&(board->title_level!=user->title))
        return 0;


    if (board->level & PERM_POSTMASK || HAS_PERM(user, board->level) || (board->level & PERM_NOZAP)) {
        if (board->flag & BOARD_CLUB_READ) {    /*俱乐部*/
            if (HAS_PERM(user,PERM_OBOARDS)&&HAS_PERM(user, PERM_SYSOP))
                return 1;
            if (board->clubnum <= 0 || board->clubnum >= MAXCLUB)
                return 0;
            if (user->club_read_rights[(board->clubnum - 1) >> 5] & (1 << ((board->clubnum - 1) & 0x1f)))
                return 1;
            else
                return 0;
        }
        return 1;
    }
    return 0;
}
#endif /* USE_DEFAULT_READ_PERM */

#ifdef USE_DEFAULT_SEE_PERM
int check_see_perm(struct userec* user,const struct boardheader* board)
{
    if (board == NULL)
        return 0;
    if (!HAS_PERM(user, PERM_OBOARDS)&&board->title_level
        &&(board->title_level!=user->title))
        return 0;
    if (board->level & PERM_POSTMASK
    	|| ((user==NULL)&&(board->level==0))
    	|| ((user!=NULL)&& HAS_PERM(user, board->level) )
    	|| (board->level & PERM_NOZAP))
	{
        if (board->flag & BOARD_CLUB_HIDE)     /*隐藏俱乐部*/
		{
			if (user==NULL) return 0;
			   if (HAS_PERM(user, PERM_OBOARDS))
					return 1;
			   return check_read_perm(user,board);
		}
        return 1;
    }
    return 0;
}

#endif /* USE_DEFAULT_SEE_PERM */

#ifdef SMS_SUPPORT
#ifndef NOT_USE_DEFAULT_SMS_FUNCTIONS
int smsid2uid(char* smsid) {
	return getuser(smsid,NULL);
}

void uid2smsid(struct user_info* uin,char* smsid)
{
	sprintf(smsid,"%s",uin->userid);
}

int uid2smsnumber(struct user_info* uin){
	return uin->uid;
}
int smsnumber2uid(byte number[4]){
	return byte2long(number);
}
#endif
	
#endif
