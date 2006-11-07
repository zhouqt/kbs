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


/* Leeward 99.03.06 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#include "bbs.h"

#define INPUT_ACTIVE 0
/*Haohmaru.98.11.3*/
#define INPUT_IDLE 1
#define WAITTIME  150

/* KCN add 1999.11.07 
#undef LOGINASNEW 
*/


extern struct screenline *big_picture;
extern struct userec *user_data;

/* extern char* pnt; */

int temp_numposts;              /*Haohmaru.99.4.02.让爱灌水的人哭去吧//grin */
int nettyNN = 0;
int count_friends, count_users; /*Add by SmallPig for count users and Friends */
char *getenv();
char *Ctime();
void r_msg_sig(int signo);
int friend_login_wall();
int listmode;
jmp_buf byebye;

int convcode = 0;               /* KCN,99.09.05 */

FILE *ufp;
int RUNSH = false;
int ERROR_READ_SYSTEM_FILE = false;
int talkrequest = false;
time_t lastnote;

struct user_info uinfo;

time_t login_start_time;
int showansi = 1;

static int i_domode = INPUT_ACTIVE;
extern time_t calltime;
extern char calltimememo[];

int switch_code(void){
    convcode = !convcode;
    redoscr();
    return convcode;
}

int canbemsged(uin)             /*Haohmaru.99.5.29 */
    struct user_info *uin;
{
    if (uinfo.pager & ALLMSG_PAGER)
        return true;
    if (uinfo.pager & FRIENDMSG_PAGER) {
        if (hisfriend(getSession()->currentuid, uin))
            return true;
    }
    return false;
}

void wait_alarm_clock(int signo)
{                               /*Haohmaru.98.11.3 */
    if (i_domode == INPUT_IDLE) {
        clear();
        exit(0);
    }
    i_domode = INPUT_IDLE;
    alarm(WAITTIME);
}

void initalarm()
{                               /*Haohmaru.98.11.3 */
    signal(SIGALRM, wait_alarm_clock);
    alarm(WAITTIME);
}

void Net_Sleep(int times)
{                               /* KCN 1999.9.15 */
    struct timeval tv;
    int sr;
    fd_set efd;
    int old;

    int csock = 0;

    tv.tv_sec = times;
    tv.tv_usec = 0;
//    FD_ZERO(&fd);
    FD_ZERO(&efd);
//    FD_SET(csock, &fd);
    FD_SET(csock, &efd);
    old = time(0);

    while ((sr = select(csock + 1, NULL, NULL, &efd, &tv)) > 0) {
        if (FD_ISSET(csock, &efd))
            abort_bbs(0);
        tv.tv_sec = times - (time(0) - old);
        tv.tv_usec = 0;
//        FD_ZERO(&fd);
        FD_ZERO(&efd);
//        FD_SET(csock, &fd);
        FD_SET(csock, &efd);
    };

}


void u_enter()
{
    memset(&uinfo, 0, sizeof(uinfo));
    uinfo.active = true;
    uinfo.pid = getpid();

    /* Bigman 2000.8.29 智囊团能够隐身 */
    if ((HAS_PERM(getCurrentUser(), PERM_CHATCLOAK) || HAS_PERM(getCurrentUser(), PERM_CLOAK)) && (getCurrentUser()->flags & CLOAK_FLAG))
        uinfo.invisible = true;
    uinfo.mode = LOGIN;
    uinfo.pager = 0;
/*    uinfo.pager = !(getCurrentUser()->flags[0] & PAGER_FLAG);*/
    if (DEFINE(getCurrentUser(), DEF_FRIENDCALL)) {
        uinfo.pager |= FRIEND_PAGER;
    }
    if (getCurrentUser()->flags & PAGER_FLAG) {
        uinfo.pager |= ALL_PAGER;
        uinfo.pager |= FRIEND_PAGER;
    }
    if (DEFINE(getCurrentUser(), DEF_FRIENDMSG)) {
        uinfo.pager |= FRIENDMSG_PAGER;
    }
    if (DEFINE(getCurrentUser(), DEF_ALLMSG)) {
        uinfo.pager |= ALLMSG_PAGER;
        uinfo.pager |= FRIENDMSG_PAGER;
    }
    uinfo.uid = getSession()->currentuid;
    strncpy(uinfo.from, getSession()->fromhost, IPLEN);
    uinfo.freshtime = time(0);
	uinfo.logintime = time(0);
    strncpy(uinfo.userid, getCurrentUser()->userid, 20);

//    strncpy(uinfo.realname, curruserdata.realname, 20);
    strncpy(uinfo.realname, getSession()->currentmemo->ud.realname, 20);
    strncpy(uinfo.username, getCurrentUser()->username, 40);
    getSession()->utmpent = getnewutmpent(&uinfo, 0);
    if (getSession()->utmpent == -1) {
        prints("人数已满,无法分配用户条目!\n");
        oflush();
        Net_Sleep(20);
        exit(-1);
    }

    getfriendstr(getCurrentUser(),get_utmpent(getSession()->utmpent), getSession());
    listmode = 0;
}

void setflags(mask, value)
    int mask, value;
{
    if (((getCurrentUser()->flags & mask) && 1) != value) {
        if (value)
            getCurrentUser()->flags |= mask;
        else
            getCurrentUser()->flags &= ~mask;
    }
}

/*---	moved to here from below	period	2000-11-19	---*/
int started = 0;
static void sync_stay(void){
    time_t now,stay;
    if(((stay=(now=time(NULL))-uinfo.logintime)<300)&&(started>1)&&(getCurrentUser()->numlogins>5))
        getCurrentUser()->numlogins--;
    else{
        if(!(now-uinfo.freshtime<IDLE_TIMEOUT))
            stay-=IDLE_TIMEOUT;
        getCurrentUser()->stay+=stay;
    }
    return;
}
void u_exit()
{
/*---	According to ylsdd's article, deal with SUPER_CLOAK problem	---*
 *---   Added by period		2000-09-19				---*/
/* 这些信号的处理要关掉, 否则在离线时等候回车时出现信号会导致重写名单,
 * 这个导致的名单混乱比kick user更多 */
    signal(SIGHUP, SIG_DFL);
    signal(SIGALRM, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
/*---	Added by period		2000-11-19	sure of this	---*/
    if (!started || !uinfo.active)
        return;
/*---		---*/
    setflags(PAGER_FLAG, (uinfo.pager & ALL_PAGER));

    /* Bigman 2000.8.29 智囊团能够隐身 */
    if ((HAS_PERM(getCurrentUser(), PERM_CHATCLOAK) || HAS_PERM(getCurrentUser(), PERM_CLOAK)))
        setflags(CLOAK_FLAG, uinfo.invisible);

#ifdef HAVE_BRC_CONTROL
    brc_update(getCurrentUser()->userid, getSession());
#endif

    /* etnlegend, 2006.04.04, 计算在线时间 */
    sync_stay();

    if (getSession()->utmpent > 0)
        clear_utmp(getSession()->utmpent, getSession()->currentuid, getpid());
}

int cmpuids(uid, up)
    char *uid;
    struct userec *up;
{
    return !strncasecmp(uid, up->userid, sizeof(up->userid));
}

int dosearchuser(userid)
    char *userid;
{
    int id;

    if ((id = getuser(userid, &getCurrentUser())) != 0)
        return getSession()->currentuid = id;
    return getSession()->currentuid = 0;
}

void talk_request(int signo)
{
    signal(SIGUSR1, talk_request);
    talkrequest = true;
    bell();
    sleep(1);
    bell();
    return;
}

extern int icurrchar, ibufsize;

void abort_bbs(int signo){
    static bool in_abort_bbs=false;
    time_t stay;
    if(in_abort_bbs)
        return;
    in_abort_bbs=true;
#ifndef SSHBBS
    output("\x1b[m",3);oflush();
#endif
    switch(uinfo.mode){
        case POSTING:
        case SMAIL:
        case EDIT:
        case EDITUFILE:
        case EDITSFILE:
        case EDITANN:
            keep_fail_post();
    }
    if(started){
        record_exit_time();
        stay=time(NULL)-uinfo.logintime;
/*---	period	2000-10-20	4 debug	---*/
        newbbslog(BBSLOG_USIES,"AXXED Stay: %3ld (%s)[%d %d]",stay/60,getCurrentUser()->username,
            getSession()->utmpent,getSession()->currentuid);
        u_exit();
    }
    shutdown(0,2);close(0);exit(0);
}

struct aol {
    int count;
    int ent[10];
	int mode[10];
    char idlemin[10][10];
    char ip[10][IPLEN+4];
    time_t login[10];
};

static int attach_online(struct user_info *uentp, int *arg, int pos){
    struct aol *a = (struct aol *)arg;
    int min;
    a->ent[a->count] = pos;
    strcpy(a->ip[a->count], uentp->from);
    a->login[a->count] = uentp->logintime;
	a->mode[a->count] = uentp->mode;
    min = (time(0) - uentp->freshtime) / 60;
    if (min) {
        snprintf(a->idlemin[a->count], 10, "[%d]", min);
    } else {
        strcpy(a->idlemin[a->count], "");
    }
    a->count++;
    if(a->count >= 10) return QUIT;
    return COUNT;
}

/* to be Continue to fix kick problem */
void multi_user_check()
{
	struct user_info *tmpinfo;
    char buffer[40];
    int ret = 1;
	int i;
	int num;
	struct aol a;

    while (ret != 0) {
        ret = multilogin_user(getCurrentUser(), getSession()->currentuid,0);
        if (ret == 3) {
            prints("\x1b[33m抱歉, 该IP有太多 \x1b[36mguest 在线, 请稍候再试。\x1b[m\n");
            pressreturn();
            oflush();
            sleep(5);
            exit(1);
        }
        if (ret == 2) {
            prints("\x1b[33m抱歉, 目前已有太多 \x1b[36mguest, 请稍候再试。\x1b[m\n");
            pressreturn();
            oflush();
            sleep(5);
            exit(1);
        }
        if (ret == 1) {
			memset(&a, 0, sizeof(a));
		    apply_utmp((APPLY_UTMP_FUNC) attach_online, 10, getCurrentUser()->userid, &a);

		    if(a.count==0) break;

			clear();
		    prints("你同时上线的窗口数过多，无法再登录。请选择希望踢除的窗口，回车断开本次连接\n");
			for(i=0;i<a.count;i++){
				move(i+2,0);
				prints("  %d: ip:%-15s 登录时间:%-24s,  %-12s %s\n",i+1, a.ip[i], a.login[i]?ctime(&a.login[i]):"未知",
                    modestring(buffer,a.mode[i], NULL, 0, NULL) , a.idlemin[i]);
			}
			buffer[0]='\0';
			move(15,0);
			getdata(0, 0, "请输入编号，回车忽略:", buffer, 3, DOECHO, NULL, true);
			num = atoi(buffer);
			if(num <1 || num >a.count){
            	oflush();
            	exit(1);
			}
            newbbslog(BBSLOG_USER,"%s","kicked (multi-login)");
			tmpinfo = get_utmpent(a.ent[num-1]);
            kick_user_utmp(getSession()->currentuid, tmpinfo, 0);
            sleep(1);
        }
    }
}

void system_init()
{
    login_start_time = time(0);
    gethostname(genbuf, 256);
#ifdef SINGLE
    if (strcmp(genbuf, SINGLE)) {
        prints("Not on a valid machine!\n");
        oflush();
        exit(-1);
    }
#endif

    signal(SIGHUP, abort_bbs);
    signal(SIGPIPE, abort_bbs);
    signal(SIGTERM, abort_bbs);
    signal(SIGQUIT, abort_bbs);
    signal(SIGINT, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGURG, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGUSR1, talk_request);
    msg_count=0;
    signal(SIGUSR2, r_msg_sig);
}

#ifdef FREE
static int strallalpha(char *uid)
{
	char *c;

	for(c=uid; *c; c++){
		if(!isalpha(*c))
			return 0;
	}

	return 1;

}
#endif

void login_query()
{
	const char *ptr;
    int curr_login_num, i;
    int curr_http_num;          /* Leeward 99.03.06 */
    char fname[STRLEN], tmpstr[30], genbuf[PATHLEN];
    FILE *fn;
#ifndef SSHBBS
    char uid[STRLEN], passbuf[40];
    int attempts;
    char buf[256];
#endif /* !defined(SSHBBS) */

    curr_login_num = get_utmp_number();;
    if (curr_login_num >= MAXACTIVE) {
        ansimore("etc/loginfull", false);
        oflush();
        Net_Sleep(20);
        exit(1);
    }
    curr_http_num = 0;
/*disable by KCN     curr_http_num = num_active_http_users(); *//* Leeward 99.03.06 */

/* 目前没有使用这个东西,sigh*/
    ptr = sysconf_str("BBSNAME");
    if (ptr == NULL)
        ptr = "尚未命名测试站";
/* add by KCN for input bbs */

    initalarm();

    ansimore("etc/issue", false);

#if 0 /* added by atppp for ZIXIA */
// ws code to reset max_user. do not enable unless you know what you are doing.
	setpublicshmreadonly(0);
	get_publicshm()->max_user = 0;
	setpublicshmreadonly(1);
#endif
	{
		int nowon,nowmax;
		nowon = curr_login_num + getwwwguestcount();
		nowmax = get_publicshm()->max_user;
    	prints("\033[1m欢迎光临 ◆\033[31m%s\033[37m◆ \033[36m上线人数 \033[1m%d[最高: %d](%d WWW GUEST)\033[m", BBS_FULL_NAME, nowon, nowon>nowmax?nowon:nowmax,getwwwguestcount());
	}
#ifndef SSHBBS
    attempts = 0;
#ifdef LOGINASNEW
    prints("\n\033[1m\033[37m试用请输入 `\033[36mguest\033[37m', 注册请输入`\033[36mnew\033[37m',add `\033[36m.\33[37m' after your ID for BIG5\033[m");
#else
    prints("\n\033[1m\033[37m请输入代号(试用请输入 `\033[36mguest\033[37m', 暂停注册新帐号,add '\033[36m.\33[37m' after your ID for BIG5\033[m");
#endif //LOGINASNEW
    while (1) {
        if (attempts++ >= 3) {
            ansimore("etc/goodbye", false);
            oflush();
            sleep(1);
            exit(1);
        }
/*Haohmaru.98.11.3*/
        initalarm();

        getdata(0, 0, "\n" LOGIN_PROMPT ": ", uid, IDLEN + 2, DOECHO, NULL, true);
        uid[IDLEN + 2] = 0;
        if (uid[strlen(uid) - 1] == '.') {
            convcode = 1;
            uid[strlen(uid) - 1] = 0;
        }
        if (strcasecmp(uid, "new") == 0) {
#ifdef LOGINASNEW
            if (check_ban_IP(getSession()->fromhost, buf) <= 0) {
                new_register();
                sethomepath(tmpstr, getCurrentUser()->userid);
                sprintf(buf, "/bin/mv -f %s " BBSHOME "/homeback/%s", tmpstr, getCurrentUser()->userid);
                system(buf);
                setmailpath(tmpstr, getCurrentUser()->userid);       /*Haohmaru.00.04.23,免得能看前人的信 */
                sprintf(buf, "/bin/mv -f %s " BBSHOME "/mailback/%s", tmpstr, getCurrentUser()->userid);
                system(buf);
                /*给新注册的用户一封信 added by binxun .2003-6-24*/
                #if defined(SMTH) || defined(ZIXIA)
                mail_file(DELIVER,"etc/tonewuser",getCurrentUser()->userid,"致新注册用户的信",0,NULL);
                #endif
                break;
            }
            prints("本系统因为 %s 的原因禁止您所在网段注册新用户\n", buf);
#else
            prints("\033[37m本系统目前无法以 new 注册, 请用 guest 进入.\033[m\n");
#endif
        } else if (*uid == '\0' || !dosearchuser(uid)
#ifdef FREE
		|| ! strallalpha(uid)
#endif
						) {
            prints("\033[32m" MSG_ERR_USERID "\033[m\n");
        } else
/* Add by KCN for let sysop can use extra 10 UTMP */
        if (!HAS_PERM(getCurrentUser(), PERM_SYSOP) && (curr_login_num >= MAXACTIVE + 10)) {
            ansimore("etc/loginfull", false);
            oflush();
            sleep(1);
            exit(1);
        } else if ( /*strcmp */ strcasecmp(uid, "guest") == 0) {
            getCurrentUser()->userlevel = PERM_DENYMAIL|PERM_DENYRELAX;
            getCurrentUser()->flags = PAGER_FLAG;
            break;
        } else {
            if (!convcode)
                convcode = !(DEFINE(getCurrentUser(), DEF_USEGB));      /* KCN,99.09.05 */

            if(check_ip_acl(getCurrentUser()->userid, getSession()->fromhost)) {
                prints("该 ID 不欢迎来自 %s 的用户，byebye!", getSession()->fromhost);
                oflush();
                sleep(1);
                exit(1);
            }
            getdata(0, 0, "\033[1m\033[37m"PASSWD_PROMPT": \033[m", passbuf, 39, NOECHO, NULL, true);

            if (!checkpasswd2(passbuf, getCurrentUser())) {
                if(passbuf[0])
                    logattempt(getCurrentUser()->userid, getSession()->fromhost, "telnet");
                prints("\033[32m密码输入错误...\033[m\n");
            } else {
                if (id_invalid(uid)) {
                    prints("\033[31m抱歉!!\033[m\n");
                    prints("\033[32m本帐号使用中文为代号，此帐号已经失效...\033[m\n");
                    prints("\033[32m想保留任何签名档请跟站长联络 ，他(她)会为你服务。\033[m\n");
                    getdata(0, 0, "按 [RETURN] 继续", genbuf, 10, NOECHO, NULL, true);
                    oflush();
                    sleep(1);
                    exit(1);
                }
                if (simplepasswd(passbuf)) {
                    prints("\033[33m* 密码过于简单, 请选择一个以上的特殊字元.\033[m\n");
                    getdata(0, 0, "按 [RETURN] 继续", genbuf, 10, NOECHO, NULL, true);
                }
                /* passwd ok, covert to md5 --wwj 2001/5/7 */
#ifdef CONV_PASS
                if (getCurrentUser()->passwd[0]) {
                    bbslog("covert", "for md5passwd");
                    setpasswd(passbuf, getCurrentUser());
                }
#endif
                break;
            }
        }
    }
#else
    prints("\n%s 欢迎您使用ssh方式访问。", getCurrentUser()->userid);
    getdata(0, 0, "按 [RETURN] 继续", genbuf, 10, NOECHO, NULL, true);
#endif //SSHBBS

#ifdef CHECK_CONNECT
    if(check_ID_lists(getCurrentUser()->userid)) {
        prints("你的连接频率过高，byebye!");
        oflush();
        sleep(1);
        exit(1);
    }
#endif

	/* We must create home directory before initializing current userdata */
    sethomepath(genbuf, getCurrentUser()->userid);
    mkdir(genbuf, 0755);
/* init user data */
//    read_userdata(getCurrentUser()->userid, &curruserdata);

    clear();
    oflush();
    if (strcasecmp(getCurrentUser()->userid, "guest") && !HAS_PERM(getCurrentUser(), PERM_BASIC)) {
        int s[GIVEUPINFO_PERM_COUNT];
        get_giveupinfo(getCurrentUser(),s);
        if(!s[0])
            prints("\033[1;33m系统错误或您已经被封禁登录权限, 请设法联系 \033[1;32mSYSOP\033[1;33m 获知原因...\033[m\n");
        else{
            i=(int)(((s[0]<0)?(-s[0]):s[0])-(time(NULL)/86400));
            if(s[0]>0)
                prints("\033[1;33m您已经处于戒网(登录)状态, 目前距离戒网结束 %d 天...\033[m\n",i);
            else
                prints("\033[1;33m你已经被封禁登录权限, 目前距离封禁结束 %d 天...\033[m\n",i);
        }
        oflush();
        sleep(1);
        exit(1);
    }
#ifdef DEBUG
    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        prints("本端口仅供测试用，请连接本站的其他开放端口。\n");
        oflush();
        Net_Sleep(3);
        abort_bbs(0);
    }
#endif
    multi_user_check();

	i = read_user_memo( getCurrentUser()->userid, & getSession()->currentmemo ) ;

	if(i==0){
		char ans[3];
		move(1,0);
		prints("由于程序更新，请先退出此帐号所有连接再重新登录\n");
		getdata(3, 0, "确定要踢除其他登录吗？(Y/n)[n]: ", ans, 2, DOECHO, NULL, true);
			if(ans[0]=='y' || ans[0]=='Y'){
				int uid;
    			struct userec *u;
				if( (uid = getuser( getCurrentUser()->userid, &u) ) != 0){
					kick_user_utmp(uid, NULL, 0);
				}
			}
	 	oflush();
		sleep(1);
		igetkey();
		exit(1);
	}else if(i<0){
		prints("初始化用户数据错误,错误号:%d\n", i);
	 	oflush();
		sleep(1);
		igetkey();
		exit(1);
	}

#ifdef BIRTHFILEPATH
	mail_birth();
#endif

    alarm(0);
    signal(SIGALRM, SIG_IGN);   /*Haohmaru.98.11.12 */
    term_init();
    scrint = 1;
    sethomepath(tmpstr, getCurrentUser()->userid);
    sprintf(fname, "%s/%s.deadve", tmpstr, getCurrentUser()->userid);
    if ((fn = fopen(fname, "r")) != NULL) {
	    if(strcasecmp(getCurrentUser()->userid,"guest"))
            mail_file(getCurrentUser()->userid, fname, getCurrentUser()->userid, "不正常断线所保留的部份...", BBSPOST_MOVE, NULL);
        else {
            fclose(fn);
			unlink(fname);
        }
    }
    temp_numposts = 0;          /*Haohmaru.99.4.02.让爱灌水的人哭去吧//grin */
}

void write_defnotepad()
{
    getCurrentUser()->notedate = time(NULL);
    return;
}

void notepad_init()
{
    FILE *check;
    char notetitle[STRLEN];
    char tmp[STRLEN * 2];
    char *fname, *bname, *ntitle;
    long int maxsec;
    time_t now;

    maxsec = 24 * 60 * 60;
    lastnote = 0;
    if ((check = fopen("etc/checknotepad", "r")) != NULL) {
        fgets(tmp, sizeof(tmp), check);
        lastnote = atol(tmp);
        fclose(check);
    } else
        lastnote = 0;
    if (lastnote == 0) {
        lastnote = time(NULL) - (time(NULL) % maxsec);
        check = fopen("etc/checknotepad", "w");
        if (check) {
            fprintf(check, "%lu", lastnote);
            fclose(check);
        }
        sprintf(tmp, "留言板在 %s Login 开启，内定开启时间时间为 %s", getCurrentUser()->userid, Ctime(lastnote));
        bbslog("user","%s",tmp);
    }
    if ((time(NULL) - lastnote) >= maxsec) {
        move(t_lines - 1, 0);
        prints("对不起，系统自动发信，请稍候.....");
        now = time(0);
        if (check) {
        check = fopen("etc/checknotepad", "w");
        lastnote = time(NULL) - (time(NULL) % maxsec);
        fprintf(check, "%lu", lastnote);
        fclose(check);
        } else lastnote=0;
        if ((check = fopen("etc/autopost", "r")) != NULL) {
            while (fgets(tmp, STRLEN, check) != NULL) {
                fname = strtok(tmp, " \n\t:@");
                bname = strtok(NULL, " \n\t:@");
                ntitle = strtok(NULL, " \n\t:@");
                if (fname == NULL || bname == NULL || ntitle == NULL)
                    continue;
                else {
                    sprintf(notetitle, "[%.10s] %s", ctime(&now), ntitle);
                    if (dashf(fname)) {
                        post_file(getCurrentUser(), "", fname, bname, notetitle, 0, 1, getSession());
                        sprintf(tmp, "%s 自动张贴", ntitle);
                        bbslog("user","%s",tmp);
                    }
                }
            }
            fclose(check);
        }
        sprintf(notetitle, "[%.10s] 留言板记录", ctime(&now));
        if (dashf("etc/notepad")) {
            post_file(getCurrentUser(), "", "etc/notepad", "notepad", notetitle, 0, 1, getSession());
            unlink("etc/notepad");
        }
        bbslog("user","%s","自动发信时间更改");
    }
    return;
}

void showsysinfo(char * fn)
{
    FILE* fp;
    char buf[500];
    int count=1,i;
    fp=fopen(fn, "r");
    if(!fp) return;
    while(!feof(fp)) {
        if(!fgets(buf, 500, fp)) break;
        if(strstr(buf, "@systeminfo@")) count++;
    }
    fclose(fp);
    i=rand()%count;
    count=0;
    clear();
    fp=fopen(fn, "r");
    while(!feof(fp)) {
        if(!fgets(buf, 500, fp)) break;
        if(strstr(buf, "@systeminfo@")) count++;
        else {
            if(count==i) prints("%s", buf);
        }
        if(count>i) break;
    }
    fclose(fp);
}

void user_login()
{
    char fname[STRLEN];
    char ans[5];
    unsigned unLevel = PERM_SUICIDE;

    /* ?????后面还有check_register_info */
    newbbslog(BBSLOG_USIES,"ENTER @%s", getSession()->fromhost);
    u_enter();
    sprintf(genbuf, "Enter from %s", getSession()->fromhost);      /* Leeward: 97.12.02 */

    bbslog("user","%s",genbuf);
/*---	period	2000-10-19	4 debug	---*/
    newbbslog(BBSLOG_USIES,"ALLOC: [%d %d]", getSession()->utmpent, getSession()->currentuid);
/*---	---*/
    started = 1;
    if (USE_NOTEPAD == 1)
        notepad_init();
    if (strcmp(getCurrentUser()->userid, "guest") != 0 && USE_NOTEPAD == 1) {
        if (DEFINE(getCurrentUser(), DEF_NOTEPAD)) {
            int noteln;

            if (lastnote > getCurrentUser()->notedate)
                getCurrentUser()->noteline = 0;
            noteln = countln("etc/notepad");
            if ((noteln > 0) && (lastnote > getCurrentUser()->notedate || getCurrentUser()->noteline == 0)) {
                shownotepad();
                getCurrentUser()->noteline = noteln;
                write_defnotepad();
            } else if ((noteln - getCurrentUser()->noteline) > 0) {
                clear();
                ansimore2("etc/notepad", false, 0, noteln - getCurrentUser()->noteline + 1);
                prints("\033[31m⊙┴———————————————————————————————————┴⊙\033[m\n");
                igetkey();
                getCurrentUser()->noteline = noteln;
                write_defnotepad();
                clear();
            }
        }
    }
    /* Leeward 98.09.24 Use SHARE MEM to diaplay statistic data below */
    if (DEFINE(getCurrentUser(), DEF_SHOWSTATISTIC)) {
        /*ansimore("0Announce/bbslists/countlogins", true); 去掉显示上站人数 */
#ifdef BLESS_BOARD
        if (dashf("etc/posts/bless"))
            ansimore("etc/posts/bless", true);
#endif
    }
    if (vote_flag(NULL, '\0', 2 /*检查读过新的Welcome 没 */ ) == 0) {
        if (dashf("Welcome")) {
            clear();
            ansimore("Welcome", true);
            vote_flag(NULL, 'R', 2 /*写入读过新的Welcome */ );
        }
    }
    clear();
    if (DEFINE(getCurrentUser(), DEF_SHOWHOT)) {     /* Leeward 98.09.24 Use SHARE MEM and disable old code
                                                   if (DEFINE(getCurrentUser(),DEF_SHOWSTATISTIC)) {
                                                   ansimore("etc/posts/day", false);
                                                   }
                                                 */
        ansimore("etc/posts/day", false);       /* Leeward: disable old code */
    }

    move(t_lines - 2 /*1 */ , 0);       /* Leeward: 98.09.24 Alter below message */
    clrtoeol();
    prints("\033[1;36m☆ 这是您第 \033[33m%d\033[36m 次上站，上次您是从 \033[33m%s\033[36m 连往本站。\n", getCurrentUser()->numlogins + 1, getCurrentUser()->lasthost);
    prints("☆ 上次连线时间为 \033[33m%s\033[m ", Ctime(getCurrentUser()->lastlogin));
    igetkey();
    /* 全国十大热门话题 added by Czz 020128 */
#ifdef CNBBS_TOPIC
    show_help("0Announce/bbslists/newsday");
#endif
    /* added end */
    if (dashf("0Announce/systeminfo")) {
        showsysinfo("0Announce/systeminfo");
        move(t_lines - 1 /*1 */ , 0);       /* Leeward: 98.09.24 Alter below message */
        clrtoeol();
        prints("\033[1;36m☆ 按任意键继续...\033[33m\033[m ");
        igetkey();
    }
    if (dashf("0Announce/hotinfo")) {
        ansimore("0Announce/hotinfo", false);
        move(t_lines - 1 /*1 */ , 0);       /* Leeward: 98.09.24 Alter below message */
        clrtoeol();
        prints("\033[1;36m☆ 按任意键继续...\033[33m\033[m ");
        igetkey();
    }
	/* Load getCurrentUser()'s mailbox properties, added by flyriver, 2003.1.5 */
	uinfo.mailbox_prop = load_mailbox_prop(getCurrentUser()->userid);
    move(t_lines - 1, 0);
    sethomefile(fname, getCurrentUser()->userid, BADLOGINFILE);
    if (ansimore(fname, false) != -1) {
        getdata(t_lines - 1, 0, "如何处理以上密码输入错误记录  (m)邮回信箱  (y)清除  (n)继续  [n]: ", ans, 4, DOECHO, NULL, true);
        ans[0] = toupper(ans[0]);
        if (ans[0] == 'M') {
            mail_file(getCurrentUser()->userid, fname, getCurrentUser()->userid, "密码输入错误记录", BBSPOST_MOVE, NULL);
        } else if (ans[0] == 'Y') {
            my_unlink(fname);
        }
    }

    strncpy(getCurrentUser()->lasthost, getSession()->fromhost, IPLEN);
    getCurrentUser()->lasthost[IPLEN-1] = '\0';   /* dumb mistake on my part */
   if(uinfo.invisible == true && HAS_PERM(getCurrentUser(), PERM_SYSOP)){
		clear();
		move(3,0);
		prints("帮助:如果输入n回车,那么这次登录不会增加登录次数,不会更改上次上站时间,隐身更彻底\n     如果直接回车,那么跟以前还是一样的\n");
        getdata(t_lines - 1, 0, "您正隐身中,这次登录需要增加登录次数吗 (Y/N)? [Y] ", ans, 4, DOECHO, NULL, true);
        if (*ans != 'N' && *ans != 'n'){
		    getCurrentUser()->lastlogin = time(NULL);
		    getCurrentUser()->numlogins++;
		}
   }else{
    getCurrentUser()->lastlogin = time(NULL);
    getCurrentUser()->numlogins++;
   }
    /* etnlegend, 2006.10.22, 我看你们还怎么减上站数! */
    started=2;

#ifndef SSHBBS
   if( (HAS_PERM(getCurrentUser(), PERM_SYSOP) || HAS_PERM(getCurrentUser(), PERM_DISS) ) && strcmp(getSession()->fromhost, "127.0.0.1")){
	   char pip[64];
	   if(check_proxy_IP(getSession()->fromhost, pip) > 0){
		   clear();
		   move(3,0);
		   prints("\033[1;31m警告:您有特殊权限,但是您现在是从穿梭地址访问本站\n     请注意密码等的安全\033[m\n");
		   while(1){
	           getdata(t_lines - 1, 0, "请输入`yes`表示您已知晓(不包括引号):", ans, 4, DOECHO, NULL, true);
			   if(!strncasecmp(ans,"yes", 3)){
				   break;
			   }
		   }
	   }else{
		   clear();
		   move(3,0);
		   prints("\033[1;31m警告:您有特殊权限,但是您现在是telnet访问本站\n     请尽量使用ssh方式,并且注意密码等的安全\033[m\n");
		   while(1){
	           getdata(t_lines - 1, 0, "请输入`y`表示您已知晓:", ans, 4, DOECHO, NULL, true);
			   if(ans[0]=='y' || ans[0]=='Y'){
				   break;
			   }
		   }
	   }
   }
#endif

    /* Leeward 98.06.20 adds below 3 lines */
    if ((int) getCurrentUser()->numlogins < 1)
        getCurrentUser()->numlogins = 1;
    if ((int) getCurrentUser()->numposts < 0)
        getCurrentUser()->numposts = 0;
    if ((int) getCurrentUser()->stay < 0)
        getCurrentUser()->stay = 1;
    getCurrentUser()->userlevel &= (~unLevel);       /* 恢复自杀标志 Luzi 98.10.10 */

    if (getCurrentUser()->firstlogin == 0) {
        getCurrentUser()->firstlogin = login_start_time - 7 * 86400;
    }
    check_register_info();
    load_mail_list(getCurrentUser(),&user_mail_list);
}

int chk_friend_book()
{
    FILE *fp;
    int idnum, n = 0;
    char buf[STRLEN], *ptr;

    move(3, 0);
    if ((fp = fopen("friendbook", "r")) == NULL)
        return n;
    prints("\033[1m系统寻人名册列表:\033[m\n\n");
    /*if((fp=fopen("friendbook","r"))==NULL)
       return n; Moved before "prints", Leeward 98.12.03 */
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        char uid[14];
        char msg[STRLEN];
        struct user_info *uin;

        ptr = strstr(buf, "@");
        if (ptr == NULL)
            continue;
        ptr++;
        strcpy(uid, ptr);
        ptr = strstr(uid, "\n");
        *ptr = '\0';
        idnum = atoi(buf);
        if (idnum != getSession()->currentuid || idnum <= 0)
            continue;
        uin = t_search(uid, false);
        sprintf(msg, "%s 已经上站。", getCurrentUser()->userid);
        /* 保存所发msg的目的uid 1998.7.5 by dong */
        strcpy(getSession()->MsgDesUid, uin ? uin->userid : "");
        idnum = 0;              /*Haohmaru.99.5.29.修正一个bug,免得有人利用这个来骚扰别人 */
        if (uin != NULL && canbemsged(uin))
            idnum = do_sendmsg(uin, msg, 2);
        if (idnum)
            prints("%s 找你，系统已经告诉他(她)你上站的消息。\n", uid);
        else
            prints("%s 找你，系统无法联络到他(她)，请你跟他(她)联络。\n", uid);
        del_from_file("friendbook", buf);
        n++;
        if (n > 15) {           /* Leeward 98.12.03 */
            pressanykey();
            move(5, 0);
            clrtobot();
        }
    }
    fclose(fp);
    return n;
}

#ifdef FB2KENDLINE
void fill_date()
{
	time_t now,next;
	struct public_data *publicshm = get_publicshm();
	char   buf[82], buf2[30], index[5], index_buf[5], *t;
	struct tm tm;
	FILE   *fp;

	now = time(0);

	if (now < publicshm->nextfreshdatetime && publicshm->date[0]!='\0')
		return;

	localtime_r(&now,&tm);
	next = now - (tm.tm_hour * 3600) - (tm.tm_min * 60) - tm.tm_sec 
		 + 86400;	/* 算出今天 0:0:00 的时间, 然後再往後加一天 */
	setpublicshmreadonly(0);
	publicshm->nextfreshdatetime = next;
	setpublicshmreadonly(1);

	fp = fopen("etc/whatdate", "r");

	if (fp == NULL)
		return;

	strftime(index_buf, 5, "%m%d", &tm);

	while (fgets(buf, 80, fp)) {
		buf[80]='\0';
        t = strchr(buf,'\n');  if(t) *t='\0';
        t = strchr(buf,'\r');  if(t) *t='\0';

		if (buf[0] == ';' || buf[0] == '#' || buf[0] == ' ' || strlen(buf)<6)
			continue;

		buf[35] = '\0';
		strncpy(index,buf,4);
		index[4] = '\0';
		strcpy(buf2,buf+5);	

		if (!strcmp(index, "0000") || !strcmp(index_buf, index) ){
			buf2[29]='\0';
			if(strlen(buf2)<29){
				int i;
				for(i=strlen(buf2);i<29;i++)
					buf2[i]=' ';
				buf2[29]='\0';
			}
			setpublicshmreadonly(0);
			strcpy(publicshm->date, buf2);
			setpublicshmreadonly(1);
		}
	}

	fclose(fp);

	return;
}

#endif

void main_bbs(int convit, char *argv)
{
    char notename[STRLEN];
    int currmail;
	int summail;
	int nummail;

/* Add by KCN for avoid free_mem core dump */
    getSession()->topfriend = NULL;
    big_picture = NULL;
    user_data = NULL;
    load_sysconf();
    resolve_ucache();
    resolve_utmp();
    resolve_boards();
    init_bbslog();
    srand(time(0) + getpid());

    /* commented by period for it changed to local variable 2000.11.12
       pnt = NULL; */

    dup2(0, 1);
#ifdef BBS_INFOD
    if (strstr(argv[0], "bbsinfo") != NULL) {
        load_sysconf();
        bbsinfod_main(argc, argv);
        exit(0);
    }
#endif
    initscr();

    convcode = convit;
    conv_init(getSession());                /* KCN,99.09.05 */

    system_init();
    if (setjmp(byebye)) {
        abort_bbs(0);
    }
    login_query();
    user_login();
//    m_init();
    clear();
    load_key(NULL);

#ifdef HAVE_PERSONAL_DNS
  //动态域名更新
    if (HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        struct dns_msgbuf msg;
        int msqid;
        msqid = msgget(sysconf_eval("BBSDNS_MSG", 0x999), IPC_CREAT | 0664);
        if (msqid >= 0) {
	    struct msqid_ds buf;

            msg.mtype=1;
            strncpy(msg.userid,getCurrentUser()->userid,IDLEN);
            msg.userid[IDLEN]=0;
            //水木是可以用fromhost的，不过其他打开dns反解得就要考虑一下了
            strncpy(msg.ip,getSession()->fromhost,IPLEN);
            msg.ip[IPLEN]=0;
            msgctl(msqid, IPC_STAT, &buf);
	    buf.msg_qbytes = (sizeof(msg)-sizeof(msg.mtype))*20;
	    msgctl(msqid, IPC_SET, &buf);

            msgsnd(msqid, &msg, sizeof(msg)-sizeof(msg.mtype), IPC_NOWAIT | MSG_NOERROR);
        }
    }
#endif
#ifndef DEBUG
#ifdef SSHBBS
    sprintf(genbuf, "sshbbsd:%s", getCurrentUser()->userid);
#else
    sprintf(genbuf, "bbsd:%s", getCurrentUser()->userid);
#endif
	set_proc_title(argv, genbuf);
#endif

#ifdef TALK_LOG
    tBBSlog_recover();             /* 2000.9.15 Bigman 添加中断talk的恢复 */
#endif

    setmailfile(genbuf, getCurrentUser()->userid, DOT_DIR);
    currmail = get_num_records(genbuf, sizeof(struct fileheader));
	get_mail_limit(getCurrentUser(), &summail, &nummail);
	if (currmail > nummail)
	{
		clear();
        prints("你的信件高达 %d 封, 请删除过期信件, 维持在 %d 封以下，否则将不能发信\n", currmail, nummail);
		pressanykey();
	}

#ifdef FB2KENDLINE
	fill_date();
#endif

	calc_calltime(1);
	while(calltime != 0 && calltime < time(0)){
		clear();
		move(1,0);
		prints("您有一个闹铃在 %s",ctime(&calltime));
		move(3,0);
		prints("提示信息为:");
		move(4,10);
		calltimememo[39]='\0';
		prints("\033[1;31m %s \033[m",calltimememo);
		prints("\n%s",ctime(&(getCurrentUser()->lastlogin)));
		move(t_lines-1,0);
		prints("                          press any key to continue...");
		refresh();
		calc_calltime(0);
		igetch();
	}

    if (HAS_PERM(getCurrentUser(), PERM_SYSOP) && dashf("new_register"))
        prints("有新使用者正在等您通过注册资料。\n");

#ifdef SMS_SUPPORT
	chk_smsmsg(1, getSession());
#endif

    /*chk_friend_book(); */
    /* Leeward 98.12.03 */
    if (!uinfo.invisible && chk_friend_book()) {
        pressreturn();
    }
    clear();
    nettyNN = NNread_init();
    if (DEFINE(getCurrentUser(), DEF_INNOTE)) {
        sethomefile(notename, getCurrentUser()->userid, "notes");
        if (dashf(notename))
            ansimore(notename, true);
    }
    b_closepolls();
    num_alcounter();
    if (count_friends > 0 && DEFINE(getCurrentUser(), DEF_LOGFRIEND))
        t_friends();
    while (1) {
        if (DEFINE(getCurrentUser(), DEF_NORMALSCR))
            domenu("TOPMENU");
        else
            domenu("TOPMENU2");
        Goodbye();
    }
}

/*Add by SmallPig*/
void update_endline()
{
#ifndef FB2KENDLINE
	char buf[STRLEN];
#endif
    char stitle[256];
    time_t now;
    int allstay;
    int colour;

    if (DEFINE(getCurrentUser(), DEF_TITLECOLOR)) {
        colour = 4;
    } else {
        colour = getCurrentUser()->numlogins % 4 + 3;
        if (colour == 3)
            colour = 1;
    }
    if (!DEFINE(getCurrentUser(), DEF_ENDLINE)) {
        move(t_lines - 1, 0);
        clrtoeol();
        return;
    }
    now = time(0);
#ifdef FLOWBANNER
	allstay = (DEFINE(getCurrentUser(), DEF_SHOWBANNER)) ? (time(0) % 3) : 0;
	if (allstay) {
		if (allstay & 1) {	//显示系统浮动信息
			struct public_data *publicshm = get_publicshm();
			if (publicshm->bannercount) 
				snprintf(stitle, 256, "\033[%s4%dm\033[33m%s", ((DEFINE(getCurrentUser(),DEF_HIGHCOLOR)) ? "1;" : ""), colour, publicshm->banners[(time(0)>>1)%publicshm->bannercount]);
			else allstay=0;
		} else {	//显示版面浮动信息
			if ((currboard)&&(currboard->bannercount))
				snprintf(stitle, 256, "\033[%s4%dm\033[33m%s", ((DEFINE(getCurrentUser(),DEF_HIGHCOLOR)) ? "1;" : ""), colour, currboard->banners[(time(0)>>1)%currboard->bannercount]);
			else allstay=0;
		}
	}
	if (!allstay) {
#endif
    allstay = (now - login_start_time) / 60;
#ifdef FB2KENDLINE
	{
		struct public_data *publicshm = get_publicshm();
		struct tm *tm;
		char mydatestring[12];
		char weeknum[7][3]={"日","一","二","三","四","五","六"};

		tm = localtime(&now);
		sprintf(mydatestring,"%02d:%02d:%02d %s", tm->tm_hour,tm->tm_min,tm->tm_sec,weeknum[tm->tm_wday]);
		num_alcounter();
		sprintf(stitle, "\033[1;44;33m[\033[36m%s\033[33m][\033[36m%11s\033[33m][\033[36m%4d\033[33m人/\033[1;36m%3d\033[33m友][\033[36m%.12s\033[33m]", 
	    publicshm->date,mydatestring,count_users,count_friends,getCurrentUser()->userid);
	}
#else
    sprintf(buf, "[\033[36m%.12s\033[33m]", getCurrentUser()->userid);
    if (DEFINE(getCurrentUser(), DEF_NOTMSGFRIEND)) {
		if (DEFINE(getCurrentUser(),DEF_HIGHCOLOR))
        	sprintf(stitle, "\033[1;4%dm\033[33m时间[\033[36m%12.12s\033[33m] 呼叫器[好友:%3s：一般:%3s] 使用者%s", colour, ctime(&now) + 4,
                (!(uinfo.pager & FRIEND_PAGER)) ? "NO " : "YES", (uinfo.pager & ALL_PAGER) ? "YES" : "NO ", buf);
		else
        	sprintf(stitle, "\033[4%dm\033[33m时间[\033[36m%12.12s\033[33m] 呼叫器[好友:%3s：一般:%3s] 使用者%s", colour, ctime(&now) + 4,
                (!(uinfo.pager & FRIEND_PAGER)) ? "NO " : "YES", (uinfo.pager & ALL_PAGER) ? "YES" : "NO ", buf);
    } else {
#ifdef HAVE_FRIENDS_NUM
            num_alcounter();
            sprintf(stitle,"\033[1;4%dm\033[33m时间[\033[36m%12.12s\033[33m] 总人数/好友[%3d/%3d][%c：%c] 使用者%s",colour,
                    ctime(&now)+4,count_users,count_friends,(uinfo.pager&ALL_PAGER)?'Y':'N',(!(uinfo.pager&FRIEND_PAGER))?'N':'Y',buf);
#else
	if (DEFINE(getCurrentUser(),DEF_HIGHCOLOR))
        sprintf(stitle, "\x1b[1;4%dm\x1b[33m时间[\x1b[36m%12.12s\x1b[33m] 总人数 [ %3d ] [%c：%c] 使用者%s", colour,
                ctime(&now) + 4, get_utmp_number() + getwwwguestcount(), (uinfo.pager & ALL_PAGER) ? 'Y' : 'N', (!(uinfo.pager & FRIEND_PAGER)) ? 'N' : 'Y', buf);
	else
        sprintf(stitle, "\x1b[4%dm\x1b[33m时间[\x1b[36m%12.12s\x1b[33m] 总人数 [ %3d ] [%c：%c] 使用者%s", colour,
                ctime(&now) + 4, get_utmp_number() + getwwwguestcount(), (uinfo.pager & ALL_PAGER) ? 'Y' : 'N', (!(uinfo.pager & FRIEND_PAGER)) ? 'N' : 'Y', buf);
#endif //HAVE_FRIENDS_NUM
    }
#endif //FB2KENDLINE
    move(t_lines - 1, 0);
    prints("%s", stitle);
    clrtoeol();
#ifdef FB2KENDLINE
    sprintf(stitle, "[\033[36m%3d\033[33m:\033[36m%d\033[33m]\033[m", (allstay / 60) % 1000, allstay % 60);
    move(t_lines - 1, -8);
#else
    sprintf(stitle, "停留[%3d:%d]", (allstay / 60) % 1000, allstay % 60);
    move(t_lines - 1, -strlen(stitle)-1);
#endif //FB2KENDLINE
    prints("%s", stitle);
    resetcolor();
#ifdef FLOWBANNER
	} else {
    move(t_lines - 1, 0);
    prints("%s", stitle);
    clrtoeol();
    resetcolor();
	}
#endif

    /* Leeward 98.09.30 show hint for rookies */
    /* PERMs should coincide with ~bbsroot/etc/sysconf.ini: PERM_ADMENU */
    if (!DEFINE(getCurrentUser(), DEF_NORMALSCR) && MMENU == uinfo.mode && !HAS_PERM(getCurrentUser(), PERM_ACCOUNTS) 
        && !HAS_PERM(getCurrentUser(), PERM_SYSOP) && !HAS_PERM(getCurrentUser(), PERM_OBOARDS)
        && !HAS_PERM(getCurrentUser(), PERM_WELCOME) && !HAS_PERM(getCurrentUser(), PERM_ANNOUNCE)) {
        move(t_lines - 2, 0);
        clrtoeol();
        prints("\033[1m\033[32m这是精简模式主选单。要使用一般模式，请设定个人参数第Ｌ项为ＯＮ并正常离站再进站。\033[m");
    }
}


/*ReWrite by SmallPig*/
void showtitle(const char *title, const char *mid){
    int spc1;
    int colour;
    char note[STRLEN];

    if (DEFINE(getCurrentUser(), DEF_TITLECOLOR)) {
        colour = BLUE;
    } else {
        colour = getCurrentUser()->numlogins % 4 + 3;
        if (colour == YELLOW)
            colour = RED;
    }

    if(currboard)
	    sprintf(note,"讨论区 [%s]",currboard->filename);
    else{
        currboardent=getbnum_safe(DEFAULTBOARD,getSession());
        currboard=(struct boardheader*)getboard(currboardent);
        if(!currboard){
            currboardent=0;
            strcpy(note,"目前尚未选定讨论区");
        }
        else{
#ifdef HAVE_BRC_CONTROL
            brc_initial(getCurrentUser()->userid,currboard->filename,getSession());
#endif /* HAVE_BRC_CONTROL */
            sprintf(note,"讨论区 [%s]",currboard->filename);
        }
    }

    spc1 = scr_cols/2 - 1 - strlen(mid) / 2;
    if ((strstr(title,"版主")!=NULL)&&(spc1-num_noans_chr(title)<4))
	/*为了多版主修改 Bigman:2002.9.7 */
    {
        strcpy(note , note + 7 );
        spc1 = scr_cols - 3 - strlen(mid) - strlen(note);
    }
/*    if (spc2 < 2)
        spc2 = 2;
    if (spc1 < 2) {
        spc2 -= 2 - spc1;
        spc1 = 2;
        if (spc2 < 2)
            spc2 = 2;
    }*/
/* Modified by Leeward 97/11/23 -- modification stops */
/* rewrite by bad */
    move(0, 0);
    resetcolor();
#ifdef FREE
    setfcolor(YELLOW,1);
#else
    setfcolor(YELLOW,DEFINE(getCurrentUser(),DEF_HIGHCOLOR));
#endif
    setbcolor(colour);
    clrtoeol();
    prints("%s", title);

    move(0, spc1);
    resetcolor();
    if(strcmp(mid, BBS_FULL_NAME)&&mid[0]!='[')
#ifdef FREE
	    setfcolor(CYAN,1);
#else
        setfcolor(CYAN, DEFINE(getCurrentUser(),DEF_HIGHCOLOR));
#endif
    else
#ifdef FREE
	    setfcolor(WHITE,1);
#else
        setfcolor(WHITE, DEFINE(getCurrentUser(),DEF_HIGHCOLOR));
#endif
    setbcolor(colour);
    if(mid[0]=='[') prints("\033[5m");
    prints("%s", mid);

    move(0, -strlen(note));
    resetcolor();
#ifdef FREE
    setfcolor(YELLOW,1);
#else
    setfcolor(YELLOW,DEFINE(getCurrentUser(),DEF_HIGHCOLOR));
#endif
    setbcolor(colour);
    prints("%s", note);
    resetcolor();
    move(1, 0);
}


void docmdtitle(const char *title,const char *prompt){
    char middoc[30];
    int chkmailflag = 0;
	int chksmsmsg = 0;

    chkmailflag = chkmail();
#ifdef SMS_SUPPORT
	chksmsmsg = chk_smsmsg(0, getSession());
#endif

    if (chkmailflag == 2)       /*Haohmaru.99.4.4.对收信也加限制 */
        strcpy(middoc, "[信箱超容]");
    else if (chkmailflag)
        strcpy(middoc, "[您有信件]");
/*    else if ( vote_flag( DEFAULTBOARD, '\0' ,0) == 0&&(bp->flag&BOARD_VOTEFLAG))
        strcpy(middoc,"[系统投票中]");*/
    else
        strcpy(middoc, BBS_FULL_NAME);

	if( chksmsmsg ){
		if( chkmailflag ){
			strcat(middoc, "[您有短信]");
		}else{
			strcpy(middoc, "[您有短信]");
		}
	}

    showtitle(title, middoc);
	if(prompt){
    move(1, 0);
    clrtoeol();
    prints("%s", prompt);
    clrtoeol();
	}
}

/* 2000.9.15 Bigman 恢复聊天记录 */
#ifdef TALK_LOG

int tBBSlog_recover()
{
    char buf[256];

    sprintf(buf, "home/%c/%s/talklog", toupper(getCurrentUser()->userid[0]), getCurrentUser()->userid);

    if (strcasecmp(getCurrentUser()->userid, "guest") == 0 || !dashf(buf))
        return;

    clear();
    strcpy(genbuf, "");

    getdata(0, 0, "\033[1;32m您有一个不正常断线所留下来的聊天记录, 您要 .. (M) 寄回信箱 (Q) 算了？[Q]：\033[m", genbuf, 4, DOECHO, NULL, true);

    if (genbuf[0] == 'M' || genbuf[0] == 'm')
        mail_file(getCurrentUser()->userid, buf, getCurrentUser()->userid, "聊天记录", BBSPOST_MOVE, NULL);
    else
        my_unlink(buf);
    return;

}
#endif
