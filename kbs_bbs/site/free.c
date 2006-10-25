#include "bbs.h"

const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

const char seccode[SECNUM][5] = {
		"ab", "cd", "ef", "gh", "ij", "kl", "mn", "opq"
};

const char *const groups[] = {
    "GROUP_0",
    "GROUP_1",
    "GROUP_2",
    "GROUP_3",
    "GROUP_4",
    "GROUP_5",
    "GROUP_6",
    "GROUP_7",
    NULL
};
const char secname[SECNUM][2][20] = {
	{"本站系统", "[本站][系统]"},
	{"我们的家", "[班级][团体]"},
	{"人文艺术", "[人文][艺术]"},
	{"电脑技术", "[电脑][技术]"},
	{"学术科学", "[科学][学术]"},
	{"休闲娱乐", "[休闲][音乐]"},
	{"知性感性", "[闲聊][感性]"},
	{"体育新闻", "[体育][新闻][信息]"},
};


int uleveltochar(char *buf, struct userec *lookupuser)
{                               /* 取用户权限中文说明 Bigman 2001.6.24 */
    unsigned lvl;
    char userid[IDLEN + 2];

    if (lookupuser->title != 0) {
        strcpy(buf, get_user_title(lookupuser->title));
        if (buf[0] != 0)
            return 0;
    }
    lvl = lookupuser->userlevel;
    strncpy(userid, lookupuser->userid, IDLEN + 2);

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

    /*
     * Bigman: 增加中文查询显示 2000.8.10 
     */
    /*
     * if( lvl & PERM_ZHANWU ) strcpy(buf,"站务"); 
     */
    if ((lvl & PERM_ANNOUNCE) && (lvl & PERM_OBOARDS))
        strcpy(buf, "站务");
    else if (lvl & PERM_JURY)
        strcpy(buf, "仲裁");    /* stephen :增加中文查询"仲裁" 2001.10.31 */
    else if (lvl & PERM_BMAMANGER)
        strcpy(buf, "讨论区总管");
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
    /*
     * Bigman: 修改显示 2001.6.24 
     */
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
        return "穿梭银河";      //why ft?
        /*
         * return "穿梭银河";
         */
//    case EDITWELC:
//        return "编辑 Welc";
    case EDITUFILE:
        return "编辑档案";
    case EDITSFILE:
        return "系统管理";
        /*
         * case  EDITSIG:  return "刻印";
         * case  EDITPLAN: return "拟计画"; 
         */
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
    case MSGING:
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
    case FRIENDTEST:
        return "心有灵犀";
    case CHICKEN:
        return "星空战斗鸡";
    case KILLER:
        return "杀人游戏";
    case CALENDAR:
        return "万年历";
    case CALENEDIT:
        return "日记本";
    case DICT:
        return "查字典";
    case CALC:
        return "计算器";
    case SETACL:
        return "登录控制";
    case EDITOR:
        return "编辑器";
    case HELP:
        return "帮助";
    case POSTTMPL:
        return "模板发文";
    case POSTCROSS:
        return "转载文章":
    default:
        return "去了那里!?";
    }
}

struct count_arg {
    int www_count;
    int telnet_count;
};

int countuser(struct user_info *uinfo, struct count_arg *arg, int pos)
{
    if (uinfo->pid == 1)
        arg->www_count++;
    else
        arg->telnet_count++;
    return COUNT;
}

int checkguestip(struct user_info *uentp, char *arg, int count)
{
    if (!strcmp(uentp->from, arg))
        return COUNT;
    return 0;
}

int multilogin_user(struct userec *user, int usernum, int mode)
{
    int logincount;
    int curr_login_num;
    struct count_arg arg;

    bzero(&arg, sizeof(arg));
    logincount = apply_utmpuid((APPLY_UTMP_FUNC) countuser, usernum, &arg);

    if (logincount < 1)
        RemoveMsgCountFile(user->userid);

    if (HAS_PERM(user, PERM_MULTILOG))
        return 0;               /* don't check sysops */
    curr_login_num = get_utmp_number();
    /*
     * binxun 2003.5 仲裁，版主，Chatop，等都可以三登
     */
    if ((HAS_PERM(user, PERM_BOARDS) || HAS_PERM(user, PERM_CHATOP)
         || HAS_PERM(user, PERM_JURY) || HAS_PERM(user, PERM_CHATCLOAK)
         || HAS_PERM(user, PERM_BMAMANGER))
        && logincount < 3)
        return 0;

    if (!strcmp("guest", user->userid)) {
        if (logincount > MAX_GUEST_NUM)
            return 2;
#define MAX_GUEST_PER_IP 20
        if (apply_utmp((APPLY_UTMP_FUNC) checkguestip, 0, "guest", getSession()->fromhost) > MAX_GUEST_PER_IP)
            return 3;
        return 0;
    }

    /*
     * 未通过注册的用户不能双登 added by bixnun 2003.5.30 
     */
    if ((!HAS_PERM(user, PERM_LOGINOK)) && logincount > 0)
        return 1;

    if (((curr_login_num < 700) && (logincount >= 3))   /*小于700可以三登 */
        ||((curr_login_num >= 700) && (logincount >= 2) /*700人以上 */
           &&!(((arg.telnet_count == 0) && (mode == 0)) /* telnet个数为零可以再登一个telnet */
               ||(((arg.www_count == 0) && (mode == 1))))))     /*user login limit */
        return 1;
    return 0;
}

int old_compute_user_value(const struct userec *urec)
{
    int value;

    /*
     * if (urec) has CHATCLOAK permission, don't kick it 
     */
    /*
     * 元老和荣誉帐号 在不自杀的情况下， 生命力999 Bigman 2001.6.23 
     */
    /*
     * * zixia 2001-11-20 所有的生命力都使用宏替换，
     * * 在 smth.h/zixia.h 中定义 
     * * 
     */

    if (((urec->userlevel & PERM_HORNOR) || (urec->userlevel & PERM_CHATCLOAK)) && (!(urec->userlevel & PERM_SUICIDE)))
        return LIFE_DAY_NODIE;

    if (urec->userlevel & PERM_SYSOP)
        return LIFE_DAY_SYSOP;
    /*
     * 站务人员生命力不变 Bigman 2001.6.23 
     */


    value = (time(0) - urec->lastlogin) / 60;   /* min */
    if (0 == value)
        value = 1;              /* Leeward 98.03.30 */

    /*
     * 修改: 将永久帐号转为长期帐号, Bigman 2000.8.11 
     */
    if ((urec->userlevel & PERM_XEMPT) && (!(urec->userlevel & PERM_SUICIDE))) {
        if (urec->lastlogin < 988610030)
            return LIFE_DAY_LONG;       /* 如果没有登录过的 */
        else
            return (LIFE_DAY_LONG * 24 * 60 - value) / (60 * 24);
    }
    /*
     * new user should register in 30 mins 
     */
    if (strcmp(urec->userid, "new") == 0) {
        return (LIFE_DAY_NEW - value) / 60;     /* *->/ modified by dong, 1998.12.3 */
    }

    /*
     * 自杀功能,Luzi 1998.10.10 
     */
    if (urec->userlevel & PERM_SUICIDE)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value) / (60 * 24);
    /**********************/
    if (urec->numlogins <= 3)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value) / (60 * 24);
    if (!(urec->userlevel & PERM_LOGINOK))
        return (LIFE_DAY_NEW * 24 * 60 - value) / (60 * 24);
    /*
     * if (urec->userlevel & PERM_LONGID)
     * return (667 * 24 * 60 - value)/(60*24); 
     */
    return (LIFE_DAY_USER * 24 * 60 - value) / (60 * 24);
}

int compute_user_value(const struct userec *urec)
{
    int value;
    int registeryear;
    int basiclife;

    /*
     * if (urec) has CHATCLOAK permission, don't kick it 
     */
    /*
     * 元老和荣誉帐号 在不自杀的情况下， 生命力999 Bigman 2001.6.23 
     */
    /*
     * * zixia 2001-11-20 所有的生命力都使用宏替换，
     * * 在 smth.h/zixia.h 中定义 
     * * 
     */
    /*
     * 特殊处理请移动出cvs 代码 
     */

    if (urec->lastlogin < 1022036050)
        return old_compute_user_value(urec) + 15;
    /*
     * 这个是死人的id,sigh 
     */
    if ((urec->userlevel & PERM_HORNOR) && !(urec->userlevel & PERM_LOGINOK))
        return LIFE_DAY_LONG;


    if (((urec->userlevel & PERM_HORNOR) || (urec->userlevel & PERM_CHATCLOAK)) && (!(urec->userlevel & PERM_SUICIDE)))
        return LIFE_DAY_NODIE;

    if ((urec->userlevel & PERM_ANNOUNCE) && (urec->userlevel & PERM_OBOARDS))
        return LIFE_DAY_SYSOP;
    /*
     * 站务人员生命力不变 Bigman 2001.6.23 
     */


    value = (time(0) - urec->lastlogin) / 60;   /* min */
    if (0 == value)
        value = 1;              /* Leeward 98.03.30 */

    /*
     * new user should register in 30 mins 
     */
    if (strcmp(urec->userid, "new") == 0) {
        return (LIFE_DAY_NEW - value) / 60;     /* *->/ modified by dong, 1998.12.3 */
    }

    /*
     * 自杀功能,Luzi 1998.10.10 
     */
    if (urec->userlevel & PERM_SUICIDE)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value) / (60 * 24) + 15;
    /**********************/
    if (urec->numlogins <= 3)
        return (LIFE_DAY_SUICIDE * 24 * 60 - value) / (60 * 24) + 15;
    if (!(urec->userlevel & PERM_LOGINOK))
        return (LIFE_DAY_NEW * 24 * 60 - value) / (60 * 24) + 15;
    /*
     * if (urec->userlevel & PERM_LONGID)
     * return (667 * 24 * 60 - value)/(60*24); 
     */
    registeryear = (time(0) - urec->firstlogin) / 31536000;
    if (registeryear < 2)
        basiclife = LIFE_DAY_USER + 1;
    else if (registeryear >= 5)
        basiclife = LIFE_DAY_LONG + 1;
    else
        basiclife = LIFE_DAY_YEAR + 1;
    return (basiclife * 24 * 60 - value) / (60 * 24) + 15;
}

/**
 * 精华区相关函数。
 */
int ann_get_postfilename(char *filename, struct fileheader *fileinfo, MENU * pm)
{
    char fname[PATHLEN];
    char *ip;

    if (fileinfo->filename[1] == '/')
        strcpy(filename, fileinfo->filename + 2);
    else
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
    return 0;
}

/**
 * 文章相关函数。
 */
time_t get_posttime(const struct fileheader * fileinfo)
{
    if (fileinfo->filename[1] == '/')
        return fileinfo->posttime;
    else
        return atoi(fileinfo->filename + 2);
}

void set_posttime(struct fileheader *fileinfo)
{
    if (fileinfo->filename[1] == '/')
        fileinfo->posttime = atoi(fileinfo->filename + 4);
}

void set_posttime2(struct fileheader *dest, struct fileheader *src)
{
    dest->posttime = src->posttime;
}

/**
 * 版面相关。
 */
void build_board_structure(const char *board)
{
    int i;
    int len;
    char buf[STRLEN];

    len = strlen(alphabet);
    for (i = 0; i < len; i++) {
        snprintf(buf, sizeof(buf), "boards/%s/%c", board, alphabet[i]);
        mkdir(buf, 0755);
    }
    return;
}


void get_mail_limit(struct userec *user, int *sumlimit, int *numlimit)
{
    if ((!(user->userlevel & PERM_SYSOP)) && strcmp(user->userid, "Arbitrator")) {
        if (user->userlevel & PERM_JURY) {
            *sumlimit = 10000;
            *numlimit = 10000;
        } else if (user->userlevel & PERM_BMAMANGER) {
            *sumlimit = 8000;
            *numlimit = 8000;
        } else if (user->userlevel & PERM_CHATCLOAK) {
            *sumlimit = 8000;
            *numlimit = 8000;
        } else
            /*
             * if (lookupuser->userlevel & PERM_BOARDS)
             * set BM, chatop, and jury have bigger mailbox, stephen 2001.10.31 
             */
        if (user->userlevel & PERM_MANAGER) {
            *sumlimit = 4000;
            *numlimit = 4000;
        } else if (user->userlevel & PERM_LOGINOK) {
            *sumlimit = 1000;
            *numlimit = 1000;
        } else {
            *sumlimit = 15;
            *numlimit = 15;
        }
    } else {
        *sumlimit = 9999;
        *numlimit = 9999;
        return;
    }
}

/* board permissions control */
int check_read_perm(const struct userec *user, const struct boardheader *board)
{
    if (board == NULL)
        return 0;

    if (user == NULL) {
        if (board->title_level != 0)
            return 0;
    } else if (!HAS_PERM(user, PERM_OBOARDS) && board->title_level && (board->title_level != user->title))
        return 0;

    if (board->level & PERM_POSTMASK || HAS_PERM(user, board->level) || (board->level & PERM_NOZAP)) {
        if (board->flag & BOARD_CLUB_READ) {    /*俱乐部 */
            if (HAS_PERM(user, PERM_OBOARDS) && HAS_PERM(user, PERM_SYSOP))
                return 1;
            if (board->clubnum <= 0 || board->clubnum > MAXCLUB)
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

int check_see_perm(const struct userec *user, const struct boardheader *board)
{
    if (board == NULL)
        return 0;
    if (user == NULL) {
        if (board->title_level != 0)
            return 0;
    } else if (!HAS_PERM(user, PERM_OBOARDS) && board->title_level && (board->title_level != user->title))
        return 0;

    if (board->level & PERM_POSTMASK || ((user == NULL) && (board->level == 0))
        || ((user != NULL) && HAS_PERM(user, board->level))
        || (board->level & PERM_NOZAP)) {
        if (board->flag & BOARD_CLUB_HIDE) {    /*隐藏俱乐部 */
            if (user == NULL)
                return 0;
            if (HAS_PERM(user, PERM_OBOARDS))
                return 1;
            return check_read_perm(user, board);
        }
        return 1;
    }
    return 0;
}

//自动通过注册的函数  binxun
int auto_register(char *userid, char *email, int msize, session_t* session)
{
    struct userdata ud;
    struct userec *uc;
    char *item;
    char fdata[7][STRLEN];
    char genbuf[STRLEN];
    char buf[STRLEN];
    char fname[STRLEN];
    int unum;
    FILE *fout;
    int n;
    struct userec deliveruser;
    static const char * const finfo[] = { "帐号位置", "申请代号", "真实姓名", "服务单位",
        "目前住址", "连络电话", "生    日", NULL
    };
    static const char * const field[] = { "usernum", "userid", "realname", "career",
        "addr", "phone", "birth", NULL
    };

    bzero(&deliveruser, sizeof(struct userec));
    strcpy(deliveruser.userid, "deliver");
    deliveruser.userlevel = -1;
    strcpy(deliveruser.username, "自动发信系统");



    bzero(fdata, 7 * STRLEN);

    if ((unum = getuser(userid, &uc)) == 0)
        return -1;              //faild
    if (read_userdata(userid, &ud) < 0)
        return -1;

    strncpy(genbuf, email, STRLEN - 16);
    item = strtok(genbuf, "#");
    if (item)
        strncpy(ud.realname, item, NAMELEN);
    item = strtok(NULL, "#");   //学号
    item = strtok(NULL, "#");
    if (item)
        strncpy(ud.address, item, STRLEN);

    email[strlen(email) - 3] = '@';
    strncpy(ud.realemail, email, STRLEN - 16);  //email length must be less STRLEN-16


    sprintf(fdata[0], "%d", unum);
    strncpy(fdata[2], ud.realname, NAMELEN);
    strncpy(fdata[4], ud.address, STRLEN);
    strncpy(fdata[5], ud.email, STRLEN);
    strncpy(fdata[1], userid, IDLEN);

    sprintf(buf, "tmp/email/%s", userid);
    if ((fout = fopen(buf, "w")) != NULL) {
        fprintf(fout, "%s\n", email);
        fclose(fout);
    }

    if (write_userdata(userid, &ud) < 0)
        return -1;
    mail_file("deliver", "etc/s_fill", userid, "恭喜你,你已经完成注册.", 0, 0);
    //sprintf(genbuf,"deliver 让 %s 自动通过身份确认.",uinfo.userid);

    sprintf(fname, "tmp/security.%d", getpid());
    if ((fout = fopen(fname, "w")) != NULL) {
        fprintf(fout, "系统安全记录系统\n\033[32m原因：%s自动通过注册\033[m\n", userid);
        fprintf(fout, "以下是通过者个人资料");
        fprintf(fout, "\n\n您的代号     : %s\n", ud.userid);
        fprintf(fout, "您的昵称     : %s\n", uc->username);
        fprintf(fout, "真实姓名     : %s\n", ud.realname);
        fprintf(fout, "电子邮件信箱 : %s\n", ud.email);
        fprintf(fout, "真实 E-mail  : %s\n", ud.realemail);
        fprintf(fout, "服务单位     : %s\n", "");
        fprintf(fout, "目前住址     : %s\n", ud.address);
        fprintf(fout, "连络电话     : %s\n", "");
        fprintf(fout, "注册日期     : %s", ctime(&uc->firstlogin));
        fprintf(fout, "最近光临日期 : %s", ctime(&uc->lastlogin));
        fprintf(fout, "最近光临机器 : %s\n", uc->lasthost);
        fprintf(fout, "上站次数     : %d 次\n", uc->numlogins);
        fprintf(fout, "文章数目     : %d(Board)\n", uc->numposts);
        fprintf(fout, "生    日     : %s\n", "");

        fclose(fout);
        //post_file(currentuser, "", fname, "Registry", str, 0, 2);

        sprintf(genbuf, "%s 自动通过注册", ud.userid);
        post_file(&deliveruser, "", fname, "Registry", genbuf, 0, 1, getSession());
        /*
         * if (( fout = fopen(logfile,"a")) != NULL)
         * {
         * fclose(fout);
         * }
         */
    }

    sethomefile(buf, userid, "/register");
    if ((fout = fopen(buf, "w")) != NULL) {
        for (n = 0; field[n] != NULL; n++)
            fprintf(fout, "%s     : %s\n", finfo[n], fdata[n]);
        fprintf(fout, "您的昵称     : %s\n", uc->username);
        fprintf(fout, "电子邮件信箱 : %s\n", ud.email);
        fprintf(fout, "真实 E-mail  : %s\n", ud.realemail);
        fprintf(fout, "注册日期     : %s\n", ctime(&uc->firstlogin));
        fprintf(fout, "注册时的机器 : %s\n", uc->lasthost);
        fprintf(fout, "Approved: %s\n", userid);
        fclose(fout);
    }

    return 0;
}

char *showuserip(struct userec *user, char *ip)
{
    static char sip[25];
    char *c;

    if (user != NULL && (!DEFINE(user, DEF_HIDEIP)))
        return ip;
	return "*.*.*.*";
}

#ifdef SMS_SUPPORT
int smsnumber2uid(byte number[4])
{
    int uid;
    char buf[10];

    uid = byte2long(number);
    /*
     * 现在我们用的是1开头的,需要去掉开始的那个1
     */
    sprintf(buf, "%d", uid);
    if (buf[0] != '1')
        return -1;
    uid = atoi(buf + 1);
    return uid;
}

int uid2smsnumber(struct user_info *uin)
{
    char buf[10];

    sprintf(buf, "1%d", uin->uid);
    return atoi(buf);
}

int smsid2uid(char *smsid)
{
    if (smsid[0] != '1')
        return -1;
    return atoi(smsid + 1);
}

void uid2smsid(struct user_info *uin, char *smsid)
{
    sprintf(smsid, "1%d", uin->uid);
}


#endif
