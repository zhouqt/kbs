#include "bbs.h"

char seccode[SECNUM][5]={
	"0", "1", "3", "4", "5", "6", "7", "8", "9"
};

char secname[SECNUM][2][20]={
	"BBS 系统", "[站内]",
	"清华大学", "[本校]",
	"电脑技术", "[电脑/系统]",
	"休闲娱乐", "[休闲/音乐]",
	"文化人文", "[文化/人文]",
	"社会信息", "[社会/信息]",
	"学术科学", "[学科/语言]",
	"体育健身", "[运动/健身]",
	"知性感性", "[谈天/感性]",
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
		if (!(lvl & (PERM_CHAT)) || !(lvl & (PERM_PAGE)) || !(lvl & (PERM_POST)) ||(lvl & (PERM_DENYMAIL)) ||(lvl &(PERM_DENYPOST))) strcpy(buf,"受限");	
		else strcpy(buf,"用户");
	}
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
    case CSIE_ANNOUNCE:     return "汲取精华";
    case CSIE_TIN:          return "使用TIN";
    case CSIE_GOPHER:       return "使用Gopher";
    case MMENU:     return "主菜单";
    case ADMIN:     return "系统维护";
    case SELECT:    return "选择讨论区";
    case READBRD:   return "浏览讨论区";
    case READNEW:   return "阅读新文章";
    case  READING:  return "阅读文章";
    case  POSTING:  return "发表文章" ;
    case MAIL:      return "信件选单" ;
    case  SMAIL:    return "寄信中";
    case  RMAIL:    return "读信中";
    case TMENU:     return "谈天说地区";
    case  LUSERS:   return "看谁在线上";
    case  FRIEND:   return "找线上好友";
    case  MONITOR:  return "监看中";
    case  QUERY:    return "查询网友";
    case  TALK:     return "聊天" ;
    case  PAGE:     return "呼叫网友" ;
    case  CHAT2:    return "梦幻国度";
    case  CHAT1:    return "聊天室中";
    case  CHAT3:    return "快哉亭";
    case  CHAT4:    return "老大聊天室";
    case  IRCCHAT:  return "会谈IRC";
    case LAUSERS:   return "探视网友";
    case XMENU:     return "系统资讯";
    case  VOTING:   return "投票";
    case  BBSNET:   return "穿梭银河";
    case  EDITWELC: return "编辑 Welc";
    case EDITUFILE: return "编辑档案";
    case EDITSFILE: return "系统管理";
        /*        case  EDITSIG:  return "刻印";
                case  EDITPLAN: return "拟计画";*/
    case ZAP:       return "订阅讨论区";
    case EXCE_MJ:   return "围城争霸";
    case EXCE_BIG2: return "比大营";
    case EXCE_CHESS:return "楚河汉界";
    case NOTEPAD:   return "留言板";
    case GMENU:     return "工具箱";
    case FOURM:     return "4m Chat";
    case ULDL:      return "UL/DL" ;
    case MSG:       return "送讯息";
    case USERDEF:   return "自订参数";
    case EDIT:      return "修改文章";
    case OFFLINE:   return "自杀中..";
    case EDITANN:   return "编修精华";
    case WEBEXPLORE: return "Web浏览";
    case CCUGOPHER: return "他站精华";
    case LOOKMSGS:  return "察看讯息";
    case WFRIEND:   return "寻人名册";
    case LOCKSCREEN:return "屏幕锁定";
    default: return "去了那里!?" ;
    }
}

