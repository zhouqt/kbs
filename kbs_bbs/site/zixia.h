#ifndef __SYSNAME_H_
#define __SYSNAME_H_

#define ZIXIA  1
/*
 * 目前 ZIXIA 这个参数带入的功能变化
 * 1. 批注册单的时候显示详细信息
 * 2. 版面文章列表按 ~ 显示帖子 URL
 * 3. 防发呆字符方面的问题
 * 4. web下只要有权限进入内部讨论区就能阅读相应的精华区，即使.Names里面没列这个版面
 * 5. 进站不显示那个鬼一样的祝福榜/校内热点
 * 6. 信件列表按 s 显示每封信件的大小
 * 7. 离站选择不寄回讯息的话不要删除讯息
 * 8. web 友情链接管理
 */

#define ACBOARD_BNAME "notepad"

#define NEW_COMERS   1 /* 注册后在 newcomers 版自动发文 */
#define OWNSENDMAIL "/usr/sbin/sendmail"
#define HAPPY_BBS  0
#define HAVE_COLOR_DATE  1
#define HAVE_FRIENDS_NUM 0
#define HAVE_REVERSE_DNS 0
#define CHINESE_CHARACTER 1
#define CNBBS_TOPIC  0 /* 是否在进站过程中显示 cn.bbs.* 十大热门话题 */
#define MAIL2BOARD  0 /* 是否允许直接 mail to any board */
#define MAILOUT   0 /* 是否允许向站外主动发信 */
#define MANUAL_DENY         0   /*是否允许手动解封*/
#define BBS_SERVICE_DICT    1

#define BUILD_PHP_EXTENSION 1   /*将php lib编成php extension */

#define HAVE_WFORUM 1

#define SMTH   0  /* SMTH专有代码 */
#define FILTER   1  /* 使用文章内容过滤 */

#define MYUNLINK_BACKUPDIR "0Announce/backup"

#define ALLOW_PUBLIC_USERONBOARD 1

#undef SITE_HIGHCOLOR

#define IDLE_TIMEOUT    (60*20)

#define BBSUID    80//9999
#define BBSGID    80//99

/* for bbs2www, by flyriver, 2001.3.9 */
#define SECNUM 10
#define BBS_PAGE_SIZE 20

#define DEFAULTBOARD     "zixia.net"//test
#define FILTER_BOARD        "Filter"
#define SYSMAIL_BOARD       "sysmail"
#define MAXUSERS    150000 //150,000
#define MAXBOARD    1024//400
#define MAXCLUB                 128
#define MAXACTIVE   10000  //3000
/* remeber: if MAXACTIVE>46656 need change get_telnet_sessionid,
    make the number of session char from 3 to 4
    */
#define MAX_GUEST_NUM  1000
#define WWW_MAX_LOGIN  5000 /* 最大www用户数量 */

#define POP3PORT  3110 //110
#define POP3SPORT  995

/* ASCIIArt, by czz, 2002.7.5 */
#define LENGTH_SCREEN_LINE 256 //220
#define LENGTH_FILE_BUFFER  260 //160
#define LENGTH_ACBOARD_BUFFER 300 //150
#define LENGTH_ACBOARD_LINE  300 //80

#define LIFE_DAY_USER  365 //120
#define LIFE_DAY_LONG  666 //666
#define LIFE_DAY_SYSOP  500 //120
#define LIFE_DAY_NODIE  999 //120
#define LIFE_DAY_NEW  30 //15
#define LIFE_DAY_SUICIDE 15 //15

#define DAY_DELETED_CLEAN 97 //20
#define SEC_DELETED_OLDHOME 0  /*  3600*24*30，注册新用户如果存在这个用户的目录，保留时间*/

#define REGISTER_WAIT_TIME (1) // (72*60*60)
#define REGISTER_WAIT_TIME_NAME "1秒钟" //72 小时

#define MAIL_BBSDOMAIN      "bbs.zixia.net"
#define MAIL_MAILSERVER     "127.0.0.1:25"

#define NAME_BBS_ENGLISH "wforum.zixia.net" //smth.org
#define NAME_BBS_CHINESE "大话西游" //水木清华
#define NAME_BBS_NICK  "斧头帮" // BBS 站
#define BBS_FULL_NAME  "BBS 大话西游站"

#define FOOTER_MOVIE  "  斧  头  帮  " // 欢  迎  投  稿
#define ISSUE_LOGIN  "本站使用一台X86服务器" //本站使用曙光公司曙光天演服务器
#define ISSUE_LOGOUT  "般若波羅密！" //还是走了罗

#define NAME_USER_SHORT  "斧头帮众" //用户
#define NAME_USER_LONG  "斧头帮众" //"水木用户"
#define NAME_SYSOP  "强盗头" //"System Operator"
#define NAME_BM   "二当家的" //版主
#define NAME_POLICE  "旺财" //"警察"
#define NAME_SYSOP_GROUP "斧头帮高层" //"站务组"
#define NAME_ANONYMOUS  "Do you zixia!?" //"水母青蛙 Today!"
#define NAME_ANONYMOUS_FROM "水帘洞" //匿名天使的家
#define ANONYMOUS_DEFAULT 0

#define NAME_MATTER  "帮务" //站务
#define NAME_SYS_MANAGE  "帮内管理" //"系统维护"
#define NAME_SEND_MSG  "丢斧头" // "送讯息"
#define NAME_VIEW_MSG  "查斧头" // "送讯息"

#define CHAT_MAIN_ROOM  "zixia" //main
#define CHAT_TOPIC  "盘丝洞？明明是水帘洞嘛……" //"大家先随便聊聊吧"
#define CHAT_MSG_NOT_OP  "*** 盘丝洞不要乱闯！ ***" //"*** 您不是本聊天室的op ***"
#define CHAT_ROOM_NAME  "洞○洞"//"聊天室"
#define CHAT_SERVER  "盘丝洞" //"聊天广场"
#define CHAT_MSG_QUIT  "回到五百年前" //"切离系统"
#define CHAT_OP   "洞主" //"聊天室 op"
#define CHAT_SYSTEM  "上天" //"系统"
#define CHAT_PARTY  "帮众" // "大家"

#define DEFAULT_NICK  "旺财"
//#define LOCAL_ARTICLE_DEFAULT  0 //缺省转信

#define MSG_ERR_USERID  "嗯？这个猪头是谁？..."
#define LOGIN_PROMPT  "\033[s代号\033[4D\033[u报上名来"
#define PASSWD_PROMPT  "\033[s密码\033[4D\033[u接头暗号"

#define PERM_NEWBOARD   PERM_OBOARDS


#define USE_DEFAULT_DEFINESTR
#define USE_DEFAULT_MAILBOX_PROP_STR

/**
 * 看在线用户时的按键处理字符。
 */
#define UL_CHANGE_NICK_UPPER   'C'
#define UL_CHANGE_NICK_LOWER   'c'
#define UL_SWITCH_FRIEND_UPPER 'F'
#define UL_SWITCH_FRIEND_LOWER 'f'


#define OWN_USEREC 1
#define IPLEN 16
struct userec {                 /* Structure used to hold information in */
    char userid[IDLEN + 2];     /* PASSFILE */
    char flags; /*一些标志，戒网，版面排序之类的*/
    unsigned char title; /*用户级别*/
    time_t firstlogin;
    char lasthost[IPLEN];
    unsigned int numlogins;
    unsigned int numposts;
#ifdef CONV_PASS
    char passwd[OLDPASSLEN];
    char unused_padding[2];
#endif
    char username[NAMELEN];
    unsigned int club_read_rights[MAXCLUB>>5];
    unsigned int club_write_rights[MAXCLUB>>5];
    unsigned char md5passwd[MD5PASSLEN];
#ifndef OS_64BIT
    unsigned int userlevel;
#endif
    time_t lastlogin;
    time_t stay;
#ifdef OS_64BIT /*  align 8 bytes... */
    unsigned int userlevel;
#endif
    int signature;
    unsigned int userdefine[2];
    time_t notedate;
    int noteline;
    int unused_atppp;
    time_t exittime;
    /* 生日数据转移到 userdata 结构中 */
    unsigned int usedspace;     /* used space of user's mailbox, in bytes */
    int unused[2];
    int altar;
    int unused2[4];
};



/**
 * 文章相关部分。
 */
#define STRLEN          80
#define BM_LEN 60
#define FILENAME_LEN 20
#define OWNER_LEN 30
#define ARTICLE_TITLE_LEN 60
typedef struct fileheader {     /* This structure is used to hold data in */
    char filename[FILENAME_LEN];        /* the DIR files */
    unsigned int id, groupid, reid;
#if defined(FILTER) || defined(COMMEND_ARTICLE)
    int o_bid;
    unsigned int o_id;
    unsigned int o_groupid;
    unsigned int o_reid;
#else
    char unused1[16];
#endif
    char innflag[2];
    char owner[OWNER_LEN];
    unsigned int eff_size;
    int posttime;
    unsigned int attachment;
    char title[ARTICLE_TITLE_LEN];
    unsigned char accessed[4];
} fileheader;

typedef struct fileheader fileheader_t;

#define GET_POSTFILENAME(x,y) get_postfilename(x,y,0)
#define GET_MAILFILENAME(x,y) get_postfilename(x,y,0)
#define VALID_FILENAME(x) valid_filename(x,0)


#define HAVE_OWN_USERIP
#define SHOW_USERIP(user,x) showuserip(user,x)

//#define SMS_SUPPORT

/**
attach define
*/
#define ATTACHTMPPATH "boards/_attach"

/* zixia addon */
/*asing add*/

#define PERCENT_SIGN_SUPPORT

int NoSpaceBdT(char *title);

#define DENYPIC "0Announce/groups/AxFaction/heaven/denypic"
#define DENY_DESC_AUTOFREE      "您被放入炼丹炉火炼"
#define DENY_DESC_NOAUTOFREE   "您被压入五行山下"
#define DENY_BOARD_AUTOFREE     "被剥夺唧唧歪歪权利，放入炼丹炉火炼"
#define DENY_BOARD_NOAUTOFREE   "被压入五行山下"
#define DENY_NAME_SYSOP         "云游神仙"

int CountDenyPic(char *fn);
int GetDenyPic(FILE* denyfile,char * fn,unsigned int i,int count);

int m_altar(void);
struct userec;
int uinfo_altar(struct userec *u);
struct boardheader;
int board_change_report(char *log, struct boardheader *old, struct boardheader *new);

#endif
