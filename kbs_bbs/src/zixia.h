#ifndef __SYSNAME_H_
#define __SYSNAME_H_

#define BBSUID 			80 //9999
#define BBSGID 			80 //99

#define DEFAULTBOARD    	"zixia.net"//test
#define MAXUSERS  		20000 //150,000
#define MAXBOARD  		256 //400
#define MAXACTIVE 		666  //3000

#define POP3PORT		3110	//110
#define	LENGTH_SCREEN_LINE	256	//220
#define	LENGTH_FILE_BUFFER 	260	//160
#define	LENGTH_ACBOARD_BUFFER	200	//150
#define	LENGTH_ACBOARD_LINE 	300	//80

#define LIFE_DAY_USER		366	//120
#define LIFE_DAY_LONG		666	//666
#define LIFE_DAY_SYSOP		500	//120
#define LIFE_DAY_NODIE		999	//120
#define LIFE_DAY_NEW		30	//15
#define LIFE_DAY_SUICIDE	15	//15

#define DAY_DELETED_CLEAN	7	//20

#define	REGISTER_WAIT_TIME	(30*60) // (72*60*60)
#define	REGISTER_WAIT_TIME_NAME	"30分钟" //72 小时

#define NAME_BBS_ENGLISH	"bbs.zixia.net" //smth.org
#define	NAME_BBS_CHINESE	"大话西游" //水木清华
#define NAME_BBS_NICK		"斧头帮" // BBS 站

#define FOOTER_MOVIE		"  斧  头  帮  " // 欢  迎  投  稿
#define ISSUE_LOGIN		"本站使用一台X86服务器" //本站使用曙光公司曙光天演服务器
#define ISSUE_LOGOUT		"般若波羅密！" //还是走了罗

#define NAME_USER_SHORT		"帮众" //用户
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
#define NAME_VIEW_MSG		"丢斧头" // "送讯息"

#define CHAT_MAIN_ROOM		"zixia" //main
#define	CHAT_TOPIC		"盘丝洞？明明是水帘洞嘛……" //"大家先随便聊聊吧"
#define CHAT_MSG_NOT_OP		"*** 盘丝洞不要乱闯！ ***" //"*** 您不是本聊天室的op ***"
#define	CHAT_ROOM_NAME		"洞○洞"//"聊天室"
#define	CHAT_SERVER		"盘丝洞" //"聊天广场"
#define CHAT_MSG_QUIT		"回到五百年前" //"切离系统"
#define CHAT_OP			"洞主" //"聊天室 op"
#define CHAT_SYSTEM		"上天" //"系统"
#define	CHAT_PARTY		"帮众" // "大家"



static char    *explain[] = {
	"斧 头 帮",
	"大话西游",
	"大小分舵",
        "吃喝玩乐",
	"唧唧歪歪",
	"吟风弄月",
        "西游取经",
        "豆腐西施",
	"月光宝盒",
        "保 留 地",
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
#if 0
    static char    *explain[] = {
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

    static char    *groups[] = {
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
