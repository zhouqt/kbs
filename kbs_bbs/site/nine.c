#include "bbs.h"

const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

const char seccode[SECNUM][5]={
	"0", "1", "3", "4", "5", "6", "7", "8"
};

const char *permstrings[] = {
        "基本权力",             /* PERM_BASIC */
        "进入聊天室",           /* PERM_CHAT */
        "呼叫他人聊天",         /* PERM_PAGE */
        "发表文章",             /* PERM_POST */
        "使用者资料正确",       /* PERM_LOGINOK */
        "计算机系本科生",       /* PERM_DCS */
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
        "男人版权限",           /* PERM_MAIL*/

};

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
    "进站时观看上站人数统计图", /* DEF_SHOWSTATISTIC Haohmaru 98.09.24 */
    "未读标记使用 *",           /* DEF_UNREADMARK Luzi 99.01.12 */
    "使用GB码阅读",             /* DEF_USEGB KCN 99.09.03 */
    "对汉字进行整字处理"  /* DEF_SPLITSCREEN 2002.9.1 */
};

const char *explain[]={
        "本站系统", 
        "同学会",
        "电脑技术",   
        "学术科学", 
        "艺术文化",  
        "人文社会",
        "体育休闲", 
        "知性感性",  
        "新闻时事",  
        NULL
};

const char *groups[]={
        "GROUP_0",
        "GROUP_1",
        "GROUP_2",
        "GROUP_3",
        "GROUP_4",
        "GROUP_5",
        "GROUP_6",
        "GROUP_7",
        "GROUP_8",
        NULL
};

const char secname[SECNUM][2][20]={
	{"本站系统", "[站内]"},
	{"同学会", "[本系]"},
	{"电脑技术", "[电脑/系统]"},
	{"学术科学", "[学科/语言]"},
	{"艺术文化", "[文化/人文]"},
	{"人文社会", "[社会/信息]"},
	{"体育休闲", "[休闲/音乐]"},
	{"知性感性", "[谈天/感性]"},
	{"新闻时事", "[新闻]"},
};

const char *mailbox_prop_str[] =
{
	"发信时保存信件到发件箱",
	"删除信件时不保存到垃圾箱",
	"版面按 'v' 时进入: 收件箱(OFF) / 信箱主界面(ON)",
};

struct _shmkey
{
	char key[20];
	int value;
};

static const struct _shmkey shmkeys[]= {
{ "BCACHE_SHMKEY",  3693 },
{ "UCACHE_SHMKEY",  3696 },
{ "UTMP_SHMKEY",    3699 },
{ "ACBOARD_SHMKEY", 9013 },
{ "ISSUE_SHMKEY",   5010 },
{ "GOODBYE_SHMKEY", 5020 },
{ "PASSWDCACHE_SHMKEY", 3697 },
{ "STAT_SHMKEY",    5100 },
{ "CONVTABLE_SHMKEY",    5101 },
{ "MSG_SHMKEY",    5200 },
{    "",   0 }
};

int get_shmkey(char *s)
{
	int n=0;
	while(shmkeys[n].key!=0)
	{
		if(!strcasecmp(shmkeys[n].key, s))
			return shmkeys[n].value;
		n++;
	}
	return 0;
}

int
uleveltochar( char *buf, struct userec *lookupuser ) /* 取用户权限中文说明 Bigman 2001.6.24*/
{
	unsigned lvl;
	char userid[IDLEN+2];
	
	lvl = lookupuser->userlevel;
	strncpy( userid, lookupuser->userid, IDLEN+2 );

    if( !(lvl &  PERM_BASIC) ) {
	strcpy( buf, "新人");
        return 0;
    }
/*    if( lvl < PERM_DEFAULT )
    {
        strcpy( buf, "- --" );
        return 1;
    }
*/

    /* Bigman: 增加中文查询显示 2000.8.10 */
    /*if( lvl & PERM_ZHANWU ) strcpy(buf,"站务");*/
    if( (lvl & PERM_ANNOUNCE) && (lvl & PERM_OBOARDS) ) strcpy(buf,"站务");
    else if  (lvl & PERM_JURY) strcpy(buf, "仲裁");/* stephen :增加中文查询"仲裁" 2001.10.31 */
    else  if( lvl & PERM_CHATCLOAK ) strcpy(buf,"元老");
    else if (lvl & PERM_CHATOP) strcpy(buf,"ChatOP");
    else if  ( lvl & PERM_BOARDS ) strcpy(buf,"版主");
    else  if( lvl & PERM_HORNOR ) strcpy(buf,"荣誉");
	/* Bigman: 修改显示 2001.6.24 */
	else if (lvl & (PERM_LOGINOK))
	{
        if (lookupuser->flags & GIVEUP_FLAG)
            strcpy(buf, "戒网");
		else if (!(lvl & (PERM_CHAT)) || !(lvl & (PERM_PAGE)) || !(lvl & (PERM_POST)) ||(lvl & (PERM_DENYMAIL))) strcpy(buf,"受限");	
		else strcpy(buf,"用户");
	}
	else if (lookupuser->flags & GIVEUP_FLAG)
        strcpy(buf, "戒网");
	else if (!(lvl & (PERM_CHAT)) && !(lvl & (PERM_PAGE))  && !(lvl & (PERM_POST))) strcpy(buf,"新人");
	else strcpy(buf,"受限");

/*    else {
        buf[0] = (lvl & (PERM_SYSOP)) ? 'C' : ' ';
        buf[1] = (lvl & (PERM_XEMPT)) ? 'L' : ' ';
        buf[2] = (lvl & (PERM_BOARDS)) ? 'B' : ' ';
        buf[3] = (lvl & (PERM_DENYPOST)) ? 'p' : ' ';
        if( lvl & PERM_ACCOUNTS ) buf[3] = 'A';
        if( lvl & PERM_SYSOP ) buf[3] = 'S'; 
        buf[4] = '\0';
    }
*/

    return 1;
}
/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* rrr - This is separated so I can suck it into the IRC source for use
   there too */

#include "modes.h"

char *
ModeType(mode)
int     mode;
{
    switch(mode) {
    case IDLE:      return "" ;
    case NEW:       return "新站友注册" ;
    case LOGIN:     return "进入本站";
    case CSIE_ANNOUNCE:     return "浏览精华区";
//    case CSIE_TIN:          return "使用TIN";
//    case CSIE_GOPHER:       return "使用Gopher";
    case MMENU:     return "主选单";
    case ADMIN:     return "管理者选单";
    case SELECT:    return "选择讨论区";
    case READBRD:   return "览遍天下";
    case READNEW:   return "览新文章";
    case  READING:  return "品味文章";
    case  POSTING:  return "\033[1;32m文豪挥笔" ;
    case MAIL:      return "处理信笺" ;
    case  SMAIL:    return "寄语信鸽";
    case  RMAIL:    return "阅览信笺";
    case TMENU:     return "聊天选单";
    case  LUSERS:   return "环顾四方";
    case  FRIEND:   return "寻找好友";
    case  MONITOR:  return "探视民情";
    case  QUERY:    return "查询网友";
    case  TALK:     return "聊天" ;
    case  PAGE:     return "呼叫" ;
//    case  CHAT2:    return "\x1b[1;33mChat2";
    case  CHAT1:    return "\x1b[1;33mChat1";
//    case  CHAT3:    return "\x1b[1;33mChat3";
//    case  CHAT4:    return "\x1b[1;33mChat4";
//    case  IRCCHAT:  return "会谈IRC";
    case LAUSERS:   return "探视网友";
    case XMENU:     return "系统资讯";
    case  VOTING:   return "投票";
//    case  BBSNET:   return "穿梭银河";
//    case  EDITWELC: return "编辑 Welc";
    case EDITUFILE: return "编辑个人档";
    case EDITSFILE: return "系统管理";
        /*        case  EDITSIG:  return "刻印";
                case  EDITPLAN: return "拟计画";*/
//    case ZAP:       return "订阅讨论区";
//    case EXCE_MJ:   return "围城争霸";
//    case EXCE_BIG2: return "比大营";
//    case EXCE_CHESS:return "楚河汉界";
    case NOTEPAD:   return "留言板";
    case GMENU:     return "工具箱";
//    case FOURM:     return "4m Chat";
//    case ULDL:      return "UL/DL" ;
    case MSG:       return "讯息中";
    case USERDEF:   return "自订参数";
    case EDIT:      return "修改文章";
    case OFFLINE:   return "自杀中..";
    case EDITANN:   return "编修精华";
//    case WEBEXPLORE: return "Web浏览";
//    case CCUGOPHER: return "他站精华";
    case LOOKMSGS:  return "察看讯息";
    case WFRIEND:   return "寻人名册";
    case LOCKSCREEN:return "屏幕锁定";
    case IMAIL:     return "寄站外信中";
    case GIVEUPNET: return "戒网中..";
    case SERVICES:    return "休闲娱乐..";
	case FRIENDTEST:  return "心有灵犀";
	case CHICKEN:	return "星空战斗鸡";
	case KILLER:        return "杀人游戏";
	case CALENDAR:  return "万年历";
	case CALENEDIT:  return "日记本";
	case DICT: return "查字典";
    case CALC: return "计算器";
    case SETACL: return "登录控制";
    case EDITOR: return "编辑器";
    default: return "去了那里!?" ;
    }
}

int multilogin_user(struct userec *user, int usernum,int mode)
{
    int logincount;
    int curr_login_num;

    logincount = apply_utmpuid(NULL, usernum, 0);

    if (logincount < 1)
        RemoveMsgCountFile(user->userid);

#ifdef FILTER
    if (!strcmp(user->userid,"menss")&&logincount<2)
        return 0;
#endif
    if (HAS_PERM(user, PERM_MULTILOG))
        return 0;               /* don't check sysops */
    curr_login_num = get_utmp_number();
    /* Leeward: 97.12.22 BMs may open 2 windows at any time */
    /* Bigman: 2000.8.17 智囊团能够开2个窗口 */
    /* stephen: 2001.10.30 仲裁可以开两个窗口 */
    if (HAS_PERM(user, PERM_SYSOP)) return 0;
    if (HAS_PERM(user, PERM_BOARDS) && strncmp(user->lasthost, "10.9.", 5)==0 && logincount==1) return 0;
    if(logincount>=1)
    return 1;
//    if ((HAS_PERM(user, PERM_BOARDS) || HAS_PERM(user, PERM_CHATOP) 
//    	|| HAS_PERM(user, PERM_JURY) || HAS_PERM(user, PERM_CHATCLOAK)
//    	|| HAS_PERM(user, PERM_BMAMANGER) )
//        && logincount < 2)
//        return 0;
    /* allow multiple guest user */
    if (!strcmp("guest", user->userid)) {
        if (logincount > MAX_GUEST_NUM) {
            return 2;
        }
        return 0;
    } else if (((curr_login_num < 700) && (logincount >= 2))
               || ((curr_login_num >= 700) && (logincount >= 1)))       /*user login limit */
        return 1;
    return 0;
}

int compute_user_value( struct userec *urec)
{
    int         value;
    
    if( ((urec->userlevel & PERM_XEMPT)||(urec->userlevel & PERM_CHATCLOAK )) && (!(urec->userlevel & PERM_SUICIDE)))
        return LIFE_DAY_NODIE;

    if( (urec->userlevel & PERM_ANNOUNCE) && (urec->userlevel & PERM_OBOARDS) )
		return LIFE_DAY_SYSOP;
	/* 站务人员生命力不变 Bigman 2001.6.23 */
	
   
    value = (time(0) - urec->lastlogin) / 60;    /* min */
    if (0 == value) value = 1; /* Leeward 98.03.30 */

    /* new user should register in 30 mins */
    if( strcmp( urec->userid, "new" ) == 0 ) {
        return (LIFE_DAY_NEW - value) / 60; /* *->/ modified by dong, 1998.12.3 */
    }

    /* 自杀功能,Luzi 1998.10.10 */
    if (urec->userlevel & PERM_SUICIDE)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value)/(60*24);
    /**********************/
    if(urec->numlogins <= 3)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value)/(60*24);
    if( !(urec->userlevel & PERM_LOGINOK) )
        return (LIFE_DAY_NEW * 24 * 60 - value)/(60*24);
    /* if (urec->userlevel & PERM_LONGID)
         return (667 * 24 * 60 - value)/(60*24); */
    return ((LIFE_DAY_USER+1) * 24 * 60 - value)/(60*24);
}


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

/**
 * 文章相关函数。
 */
time_t get_posttime(const struct fileheader *fileinfo)
{
	return atoi(fileinfo->filename + 2);
}

void set_posttime(struct fileheader *fileinfo)
{
	return;
}

void set_posttime2(struct fileheader *dest, struct fileheader *src)
{
	return;
}

/**
 * 版面相关。
 */
void build_board_structure(const char *board)
{
	return;
}

void get_mail_limit(struct userec* user,int *sumlimit,int * numlimit)
{
    *sumlimit=10000;
    *numlimit=10000;
}

/* board permissions control */
int check_read_perm(struct userec *user, const struct boardheader *board)
{
    if (board == NULL)
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

int check_see_perm(struct userec* user,const struct boardheader* board)
{
    if (board == NULL)
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

