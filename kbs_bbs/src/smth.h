#ifndef __SYSNAME_H_
#define __SYSNAME_H_

#define BBSUID 			9999
#define BBSGID 			99

#define DEFAULTBOARD    	"test"
#define MAXUSERS  		150000
#define MAXBOARD  		400
#define MAXACTIVE 		3000

#define POP3PORT		110
#define	LENGTH_SCREEN_LINE	220
#define	LENGTH_FILE_BUFFER 	160
#define	LENGTH_ACBOARD_BUFFER	150
#define	LENGTH_ACBOARD_LINE 	80

#define LIFE_DAY_USER		120
#define LIFE_DAY_LONG		666
#define LIFE_DAY_SYSOP		120
#define LIFE_DAY_NODIE		120
#define LIFE_DAY_NEW		15
#define LIFE_DAY_SUICIDE	15

#define DAY_DELETED_CLEAN	20

#define	REGISTER_WAIT_TIME	(72*60*60)
#define	REGISTER_WAIT_TIME_NAME	"72 小时"

#define NAME_BBS_ENGLISH	"smth.org"
#define	NAME_BBS_CHINESE	"水木清华"
#define NAME_BBS_NICK		"BBS 站"

#define FOOTER_MOVIE		"欢  迎  投  稿"
#define ISSUE_LOGIN		"本站使用曙光公司曙光天演服务器"
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
