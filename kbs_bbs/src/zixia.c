#include "bbs.h"

int
uleveltochar( char* buf, struct userec *lookupuser )
{
	unsigned lvl;
	char userid[IDLEN+2];
	
	lvl = lookupuser->userlevel;
	strncpy( userid, lookupuser->userid, IDLEN+2 );

	// buf[10], buf 最多 4 个汉字 + 1 byte （\0结尾）
	//根据 level
	//
	if( ( lvl & PERM_DENYPOST ) || !( lvl | PERM_POST ) ) strcpy( buf, "哑巴" ); 
      	else if ( ! (lvl & PERM_BASIC) ) strcpy( buf, "瞎子" ); 
	else if( lvl < PERM_DEFAULT ) strcpy( buf, "书生" );
    	else if( lvl & PERM_SYSOP ) strcpy(buf,"帮主");
      	else if( lvl & PERM_MM && lvl & PERM_CHATCLOAK ) strcpy(buf,"晶晶姑娘");
      	else if( lvl & PERM_MM ) strcpy(buf,"观音姐姐");
      	else  if( lvl & PERM_CHATCLOAK ) strcpy(buf,"无敌牛虱");
      	else if  ( lvl & PERM_BOARDS ) strcpy(buf,"二当家的");
      	else strcpy(buf,"斧头帮众");

	//中文说明，根据 level
    	if( !strcmp(lookupuser->userid,"SYSOP")
		    || !strcmp(lookupuser->userid,"Rama") )
	    strcpy( buf, "强盗头" );
    	else if( !strcmp(lookupuser->userid,"zixia") )
	    strcpy( buf, "旺财" );
    	else if( !strcmp(lookupuser->userid,"halen") )
	    strcpy( buf, "小皮卡秋" );
    	else if( !strcmp(lookupuser->userid,"cityhunter") 
		    || !strcmp(lookupuser->userid,"soso")
		    || !strcmp(lookupuser->userid,"Czz")
	  	    || !strcmp(lookupuser->userid,"KCN") )
	    strcpy( buf, "牛魔王" );
    	else if( !strcmp(lookupuser->userid,"guest") )
	    strcpy( buf, "葡萄" );

    	return 1;
}

#include "modes.h"

char *
ModeType(mode)
int     mode;
{
    switch(mode) {
    case IDLE:      return "^@^zz..ZZ" ;
    case NEW:       return "新" NAME_USER_SHORT "注册" ;
    case LOGIN:     return "进入" NAME_BBS_NICK;
    case CSIE_ANNOUNCE:     return "汲取精华";
    case CSIE_TIN:          return "使用TIN";
    case CSIE_GOPHER:       return "使用Gopher";
    case MMENU:     return "主菜单";
    case ADMIN:     return NAME_SYS_MANAGE;
    case SELECT:    return "选择讨论区";
    case READBRD:   return "浏览讨论区";
    case READNEW:   return "察看新文章";
    case  READING:  return "审查文章";
    case  POSTING:  return "批阅文章" ;
    case MAIL:      return "信件选单" ;
    case  SMAIL:    return "寄信中";
    case  RMAIL:    return "读信中";
    case TMENU:     return "谈天说地区";
    case  LUSERS:   return "看谁在线上";
    case  FRIEND:   return "找线上好友";
    case  MONITOR:  return "监看中";
    case  QUERY:    return "查询" NAME_USER_SHORT;
    case  TALK:     return "聊天" ;
    case  PAGE:     return "呼叫" NAME_USER_SHORT;
    case  CHAT2:    return "梦幻国度";
    case  CHAT1:    return CHAT_SERVER "中";
    case  CHAT3:    return "快哉亭";
    case  CHAT4:    return "老大聊天室";
    case  IRCCHAT:  return "会谈IRC";
    case LAUSERS:   return "探视" NAME_USER_SHORT;
    case XMENU:     return "系统资讯";
    case  VOTING:   return "投票";
    case  BBSNET:   return "波若波罗蜜";
    case  EDITWELC: return "编辑 Welc";
    case EDITUFILE: return "编辑档案";
    case EDITSFILE: return NAME_SYS_MANAGE;
    case  EDITSIG:  return "刻印";
    case  EDITPLAN: return "拟计画";
    case ZAP:       return "订阅讨论区";
    case EXCE_MJ:   return "围城争霸";
    case EXCE_BIG2: return "比大营";
    case EXCE_CHESS:return "楚河汉界";
    case NOTEPAD:   return "留言板";
    case GMENU:     return "工具箱";
    case FOURM:     return "4m Chat";
    case ULDL:      return "UL/DL" ;
    case MSG:       return NAME_SEND_MSG;
    case USERDEF:   return "自订参数";
    case EDIT:      return "修改文章";
    case OFFLINE:   return "自杀中..";
    case EDITANN:   return "编修精华";
    case WWW:       return "悠游 WWW";
    case CCUGOPHER: return "他站精华";
    case LOOKMSGS:  return NAME_VIEW_MSG;
    case WFRIEND:   return "寻人名册";
    case LOCKSCREEN:return "屏幕锁定";
    default: return "去了那里!?" ;
    }
}

