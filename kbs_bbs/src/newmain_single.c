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

int temp_numposts;              /*Haohmaru.99.4.02.ÈÃ°®¹àË®µÄÈË¿ÞÈ¥°É//grin */
int nettyNN = 0;
int count_friends, count_users; /*Add by SmallPig for count users and Friends */
char *getenv();
char *sysconf_str();
char *Ctime();
struct user_info *t_search();
void r_msg_sig(int signo);
int friend_login_wall();
int listmode;
int numofsig = 0;
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

extern char MsgDesUid[14];      /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong */
int canbemsged(uin)             /*Haohmaru.99.5.29 */
    struct user_info *uin;
{
    if (uinfo.pager & ALLMSG_PAGER)
        return true;
    if (uinfo.pager & FRIENDMSG_PAGER) {
        if (hisfriend(usernum, uin))
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
    fd_set fd, efd;
    int old;

    int csock = 0;

    tv.tv_sec = times;
    tv.tv_usec = 0;
    FD_ZERO(&fd);
    FD_ZERO(&efd);
    FD_SET(csock, &fd);
    FD_SET(csock, &efd);
    old = time(0);

    while ((sr = select(csock + 1, &fd, NULL, &efd, &tv)) > 0) {
        if (FD_ISSET(csock, &efd))
            abort_bbs(0);
        tv.tv_sec = times - (time(0) - old);
        tv.tv_usec = 0;
        FD_ZERO(&fd);
        FD_ZERO(&efd);
        FD_SET(csock, &fd);
        FD_SET(csock, &efd);
    };

}


void u_enter()
{
    memset(&uinfo, 0, sizeof(uinfo));
    uinfo.active = true;
    uinfo.pid = getpid();
/*    if( HAS_PERM(currentuser,PERM_LOGINCLOAK) && (currentuser->flags[0] & CLOAK_FLAG) && HAS_PERM(currentuser,PERM_SEECLOAK)) */

    /* Bigman 2000.8.29 ÖÇÄÒÍÅÄÜ¹»ÒþÉí */
    if ((HAS_PERM(currentuser, PERM_CHATCLOAK) || HAS_PERM(currentuser, PERM_CLOAK)) && (currentuser->flags & CLOAK_FLAG))
        uinfo.invisible = true;
    uinfo.mode = LOGIN;
    uinfo.pager = 0;
/*    uinfo.pager = !(currentuser->flags[0] & PAGER_FLAG);*/
    if (DEFINE(currentuser, DEF_FRIENDCALL)) {
        uinfo.pager |= FRIEND_PAGER;
    }
    if (currentuser->flags & PAGER_FLAG) {
        uinfo.pager |= ALL_PAGER;
        uinfo.pager |= FRIEND_PAGER;
    }
    if (DEFINE(currentuser, DEF_FRIENDMSG)) {
        uinfo.pager |= FRIENDMSG_PAGER;
    }
    if (DEFINE(currentuser, DEF_ALLMSG)) {
        uinfo.pager |= ALLMSG_PAGER;
        uinfo.pager |= FRIENDMSG_PAGER;
    }
    uinfo.uid = usernum;
    strncpy(uinfo.from, fromhost, IPLEN);
#ifdef SHOW_IDLE_TIME
    uinfo.freshtime = time(0);
#endif
    strncpy(uinfo.userid, currentuser->userid, 20);

//    strncpy(uinfo.realname, curruserdata.realname, 20);
    strncpy(uinfo.realname, currentmemo->ud.realname, 20);
    strncpy(uinfo.username, currentuser->username, 40);
    utmpent = getnewutmpent(&uinfo);
    if (utmpent == -1) {
        prints("ÈËÊýÒÑÂú,ÎÞ·¨·ÖÅäÓÃ»§ÌõÄ¿!\n");
        oflush();
        Net_Sleep(20);
        exit(-1);
    }

    getfriendstr(currentuser,get_utmpent(utmpent));
    listmode = 0;
    digestmode = false;
}

void setflags(mask, value)
    int mask, value;
{
    if (((currentuser->flags & mask) && 1) != value) {
        if (value)
            currentuser->flags |= mask;
        else
            currentuser->flags &= ~mask;
    }
}

/*---	moved to here from below	period	2000-11-19	---*/
int started = 0;
void u_exit()
{
/*---	According to ylsdd's article, deal with SUPER_CLOAK problem	---*
 *---   Added by period		2000-09-19				---*/
/* ÕâÐ©ÐÅºÅµÄ´¦ÀíÒª¹Øµô, ·ñÔòÔÚÀëÏßÊ±µÈºò»Ø³µÊ±³öÏÖÐÅºÅ»áµ¼ÖÂÖØÐ´Ãûµ¥,
 * Õâ¸öµ¼ÖÂµÄÃûµ¥»ìÂÒ±Èkick user¸ü¶à */
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
/*    if (HAS_PERM(currentuser,PERM_LOGINCLOAK)&&HAS_PERM(currentuser,PERM_SEECLOAK))*/

    /* Bigman 2000.8.29 ÖÇÄÒÍÅÄÜ¹»ÒþÉí */
    if ((HAS_PERM(currentuser, PERM_CHATCLOAK) || HAS_PERM(currentuser, PERM_CLOAK)))
        setflags(CLOAK_FLAG, uinfo.invisible);

#ifdef HAVE_BRC_CONTROL
    brc_update(currentuser->userid);
#endif

    if (utmpent > 0)
        clear_utmp(utmpent, usernum, getpid());
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

    if ((id = getuser(userid, &currentuser)) != 0)
        return usernum = id;
    return usernum = 0;
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

void abort_bbs(int signo)
{
    time_t stay;
    static bool in_abort_bbs=false;

    if(in_abort_bbs) return;
    in_abort_bbs=true;

    output("\x1b[m",3);
    oflush();
    if (uinfo.mode == POSTING || uinfo.mode == SMAIL || uinfo.mode == EDIT || uinfo.mode == EDITUFILE || uinfo.mode == EDITSFILE || uinfo.mode == EDITANN)
        keep_fail_post();
    if (started) {
        record_exit_time();
        stay = time(0) - login_start_time;
/*---	period	2000-10-20	4 debug	---*/
        newbbslog(BBSLOG_USIES, "AXXED Stay: %3ld (%s)[%d %d]", stay / 60, currentuser->username, utmpent, usernum);
        u_exit();
    }
    shutdown(0, 2);
    close(0);
    exit(0);
}

/* to be Continue to fix kick problem */
void multi_user_check()
{
    struct user_info uin;
    char buffer[40];
    int ret = 1;
    int kickmulti = -1;

    while (ret != 0) {
        ret = multilogin_user(currentuser, usernum,0);
        if (ret == 2) {
            prints("\x1b[33m±§Ç¸, Ä¿Ç°ÒÑÓÐÌ«¶à \x1b[36mguest, ÇëÉÔºòÔÙÊÔ¡£\x1b[m\n");
            pressreturn();
            oflush();
            sleep(5);
            exit(1);
        }
        if (ret == 1) {
            if (kickmulti == -1)
                getdata(0, 0, "ÄãÍ¬Ê±ÉÏÏßµÄ´°¿ÚÊý¹ý¶à£¬ÊÇ·ñÌß³ö±¾IDÆäËü´°¿Ú(Y/N)? [N]", buffer, 4, DOECHO, NULL, true);
            if (buffer[0] == 'Y' || buffer[0] == 'y' || kickmulti == 1) {
                int lres;
                int num;

                kickmulti = 1;
                if (!(num = search_ulist(&uin, cmpuids2, usernum)))
                    return;     /* user isn't logged in */
                if (uin.pid != 1) {
                    if (!uin.active || (kill(uin.pid, 0) == -1))
                        return; /* stale entry in utmp file */
        /*---	modified by period	first try SIGHUP	2000-11-08	---*/
                    lres = kill(uin.pid, SIGHUP);
                    sleep(1);
                    if (lres)
        /*---	---*/
                        kill(uin.pid, 9);
                }
                sprintf(buffer, "kicked (multi-login)");
                bbslog("user","%s",buffer);

                clear_utmp(num, usernum, uin.pid);
                continue;
            }
            oflush();
            exit(1);            /* ¶à´°¿ÚÊ±ÌßµôÒ»¸ö£¬×Ô¼ºÒ²¶ÏÏß */
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

void system_abort()
{
    if (started) {
        newbbslog(BBSLOG_USIES,"ABORT %s", currentuser->username);
        u_exit();
    }
    clear();
    prints("Ð»Ð»¹âÁÙ, ¼ÇµÃ³£À´à¸ !\n");
    oflush();
    abort_bbs(0);
    return;
}

#define MAXLIST 1000

int check_ID_lists(char * id)
{
    int i,j,k;
    FILE* fp;
    struct stat st;
    struct flock ldata;
    int fd;
    char fn[80];
    int found=0,min=0,ret=0;
    time_t now;
    struct id_struct{
        char id[IDLEN+2];
        time_t first,last;
        int t;
    } ids[MAXLIST];

    sprintf(fn, ".IDlists");
    if(stat(fn,&st)==-1) {
        memset(ids, 0, sizeof(struct id_struct)*MAXLIST);
        fd=open(fn, O_WRONLY|O_CREAT, 0600);
        write(fd, ids, sizeof(struct id_struct)*MAXLIST);
        close(fd);
    }
    now = time(0);
    if(id[0]==0) return 0;
    if(!strcmp(id, "guest")) return 0;

    fd = open(fn, O_RDWR, 0600);
    ldata.l_type = F_WRLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        bbslog("user", "%s", "reclock err");
        close(fd);
        return 0;              /* lock error*/
    }
    read(fd, ids, sizeof(struct id_struct)*MAXLIST);

    for(i=0;i<MAXLIST;i++) {
        if((double)(now-ids[i].last)>60*60) {
            ids[i].id[0]=0;
        }
        if(!strncmp(ids[i].id, id, IDLEN)){
            if((double)(now-ids[i].last)<=ID_CONNECT_CON_THRESHOLD2) {
                fp=fopen(".IDdenys", "a");
                if(fp){
                    fprintf(fp, "0 %ld %s %d\n", (unsigned int)now, id, ids[i].t);
                    fclose(fp);
                }
                if((double)(now-ids[i].last)<=5.0)
                    ret = 1;
            }
            found=1;
            ids[i].last = now;
            ids[i].t++;
            if(ret==0)
            if(ids[i].t>=10&&(ids[i].t/(double)(ids[i].last-ids[i].first)>=ID_CONNECT_CON_THRESHOLD)) {
                fp=fopen(".IDdenys", "a");
                if(fp){
                    fprintf(fp, "1 %ld %s %d\n", (unsigned int)now, id, ids[i].t);
                    fclose(fp);
                }
                if(ids[i].t/(double)(ids[i].last-ids[i].first)>=100.0/60/60)
                    ret = 1;
            }
            break;
        }
        if(ids[i].last<ids[min].last) min = i;
    }
    if(!found) {
        strcpy(ids[min].id, id);
        ids[min].first = now;
        ids[min].last = now;
        ids[min].t = 1;
    }

    lseek(fd, 0, SEEK_SET);
    write(fd, ids, sizeof(struct id_struct)*MAXLIST);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* ÍË³ö»¥³âÇøÓò*/
    close(fd);
    return ret;
}

int check_ip_acl(char * id, char * sip)
{
    char fn[160];
    int ip[4],rip[4],l,a;
    unsigned int ips, rips;
    FILE* fp;
    sprintf(fn, BBSHOME "/home/%c/%s/ipacl", toupper(id[0]), id);
    sscanf(sip, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
    ips = (ip[0]<<24)+(ip[1]<<16)+(ip[2]<<8)+ip[3];
    fp = fopen(fn, "r");
    if(fp) {
        while(!feof(fp)) {
            if(fscanf(fp, "%d.%d.%d.%d %d %d", &rip[0], &rip[1], &rip[2], &rip[3], &l, &a)<=0) break;
            rips = (rip[0]<<24)+(rip[1]<<16)+(rip[2]<<8)+rip[3];
            if(((ips>>(32-l))<<(32-l))==((rips>>(32-l))<<(32-l))||l==0) {
                fclose(fp);
                return a;
            }
        }        
        fclose(fp);
    }
    return 0;
}

void login_query()
{
    char uid[STRLEN], passbuf[40], *ptr;
    int curr_login_num, i, j;
    int curr_http_num;          /* Leeward 99.03.06 */
    int attempts;
    char fname[STRLEN], tmpstr[30], genbuf[PATHLEN];
    FILE *fn;
    char buf[256];

    curr_login_num = get_utmp_number();;
    if (curr_login_num >= MAXACTIVE) {
        ansimore("etc/loginfull", false);
        oflush();
        Net_Sleep(20);
        exit(1);
    }
    curr_http_num = 0;
/*disable by KCN     curr_http_num = num_active_http_users(); *//* Leeward 99.03.06 */

/* Ä¿Ç°Ã»ÓÐÊ¹ÓÃÕâ¸ö¶«Î÷,sigh*/
    ptr = sysconf_str("BBSNAME");
    if (ptr == NULL)
        ptr = "ÉÐÎ´ÃüÃû²âÊÔÕ¾";
/* add by KCN for input bbs */

#ifdef DOTIMEOUT
    initalarm();
#else
    signal(SIGALRM, SIG_IGN);
#endif

    ansimore("etc/issue", false);
#ifndef NINE_BUILD
    prints("\033[1m»¶Ó­¹âÁÙ ¡ô\033[31m%s[37m¡ô [36mÉÏÏßÈËÊý \033[1m%d[×î¸ß: %d](%d WWW GUEST)[m", BBS_FULL_NAME, curr_login_num + getwwwguestcount(), get_publicshm()->max_user,getwwwguestcount());
#else
    prints("\033[1m»¶Ó­¹âÁÙ ¡ô\033[31m%s\033[37m¡ô \033[36mÉÏÏßÈËÊý \033[1m%d[×î¸ß: %d]\033[m", BBS_FULL_NAME, curr_login_num + getwwwguestcount(),get_publicshm()->max_user);
#endif

#ifndef SSHBBS
    attempts = 0;
#ifdef LOGINASNEW
    prints("\n\033[1m[37mÊÔÓÃÇëÊäÈë `\033[36mguest\033[37m', ×¢²áÇëÊäÈë`\033[36mnew\033[37m',add \033[36m'.'\33[37m after your ID for BIG5[m");
#else
#ifdef NINE_BUILD
    prints("\n\033[1m[37mÇëÊäÈë´úºÅ(ÊÔÓÃÇëÊäÈë `\033[36mguest^[[37m)[m");
#else
    prints("\n\033[1m\033[37mÇëÊäÈë´úºÅ(ÊÔÓÃÇëÊäÈë `\033[36mguest\033[37m', ÔÝÍ£×¢²áÐÂÕÊºÅ,add \033[36m'.'\33[37m after your ID for BIG5\033[m");
#endif //NINE_BUILD
#endif //LOGINASNEW
    while (1) {
        if (attempts++ >= LOGINATTEMPTS) {
            ansimore("etc/goodbye", false);
            oflush();
            sleep(1);
            exit(1);
        }
/*Haohmaru.98.11.3*/
#ifdef DOTIMEOUT
        initalarm();
#else
        signal(SIGALRM, SIG_IGN);
#endif

        getdata(0, 0, "\n" LOGIN_PROMPT ": ", uid, IDLEN + 2, DOECHO, NULL, true);
        uid[IDLEN + 2] = 0;
        if (uid[strlen(uid) - 1] == '.') {
            convcode = 1;
            uid[strlen(uid) - 1] = 0;
        }
        if (strcmp(uid, "new") == 0) {
#ifdef LOGINASNEW
            if (check_ban_IP(fromhost, buf) <= 0) {
                new_register();
                sethomepath(tmpstr, currentuser->userid);
                sprintf(buf, "/bin/mv -f %s " BBSHOME "/homeback/%s", tmpstr, currentuser->userid);
                system(buf);
                setmailpath(tmpstr, currentuser->userid);       /*Haohmaru.00.04.23,ÃâµÃÄÜ¿´Ç°ÈËµÄÐÅ */
                sprintf(buf, "/bin/mv -f %s " BBSHOME "/mailback/%s", tmpstr, currentuser->userid);
                system(buf);
                /*¸øÐÂ×¢²áµÄÓÃ»§Ò»·âÐÅ added by binxun .2003-6-24*/
                #ifdef SMTH
                mail_file("deliver","etc/tonewuser",currentuser->userid,"ÖÂÐÂ×¢²áÓÃ»§µÄÐÅ",0,NULL);
                #endif
                break;
            }
            prints("±¾ÏµÍ³ÒòÎª %s µÄÔ­Òò½ûÖ¹ÄúËùÔÚÍø¶Î×¢²áÐÂÓÃ»§\n", buf);
#else
            prints("[37m±¾ÏµÍ³Ä¿Ç°ÎÞ·¨ÒÔ new ×¢²á, ÇëÓÃ guest ½øÈë.[m\n");
#endif
        } else if (*uid == '\0' || !dosearchuser(uid)) {
            prints("[32m" MSG_ERR_USERID "[m\n");
        } else
/* Add by KCN for let sysop can use extra 10 UTMP */
        if (!HAS_PERM(currentuser, PERM_ADMINMENU) && (curr_login_num >= MAXACTIVE + 10)) {
            ansimore("etc/loginfull", false);
            oflush();
            sleep(1);
            exit(1);
        } else if ( /*strcmp */ strcasecmp(uid, "guest") == 0) {
            currentuser->userlevel = PERM_DENYMAIL|PERM_DENYRELAX;
            currentuser->flags = CURSOR_FLAG | PAGER_FLAG;
            break;
        } else {
            if (!convcode)
                convcode = !(currentuser->userdefine & DEF_USEGB);      /* KCN,99.09.05 */

            if(check_ip_acl(currentuser->userid, fromhost)) {
                prints("¸Ã ID ²»»¶Ó­À´×Ô %s µÄÓÃ»§£¬byebye!", fromhost);
                oflush();
                sleep(1);
                exit(1);
            }
            getdata(0, 0, "\033[1m[37m"PASSWD_PROMPT": [m", passbuf, 39, NOECHO, NULL, true);
#ifdef NINE_BUILD
            if(!strcmp(fromhost, "10.9.0.1")||!strcmp(fromhost, "10.9.30.133")) {
		getdata(0, 0, "", buf, 20, NOECHO, NULL, true);
                if (buf[0]) strcpy(fromhost, buf);
            }
#endif

            if (!checkpasswd2(passbuf, currentuser)) {
                logattempt(currentuser->userid, fromhost);
                prints("[32mÃÜÂëÊäÈë´íÎó...[m\n");
            } else {
                if (id_invalid(uid)) {
                    prints("[31m±§Ç¸!![m\n");
                    prints("[32m±¾ÕÊºÅÊ¹ÓÃÖÐÎÄÎª´úºÅ£¬´ËÕÊºÅÒÑ¾­Ê§Ð§...[m\n");
                    prints("[32mÏë±£ÁôÈÎºÎÇ©ÃûµµÇë¸úÕ¾³¤ÁªÂç £¬Ëû(Ëý)»áÎªÄã·þÎñ¡£[m\n");
                    getdata(0, 0, "°´ [RETURN] ¼ÌÐø", genbuf, 10, NOECHO, NULL, true);
                    oflush();
                    sleep(1);
                    exit(1);
                }
                if (simplepasswd(passbuf)) {
                    prints("[33m* ÃÜÂë¹ýÓÚ¼òµ¥, ÇëÑ¡ÔñÒ»¸öÒÔÉÏµÄÌØÊâ×ÖÔª.[m\n");
                    getdata(0, 0, "°´ [RETURN] ¼ÌÐø", genbuf, 10, NOECHO, NULL, true);
                }
                /* passwd ok, covert to md5 --wwj 2001/5/7 */
#ifdef CONV_PASS
                if (currentuser->passwd[0]) {
                    bbslog("covert", "for md5passwd");
                    setpasswd(passbuf, currentuser);
                }
#endif
                break;
            }
        }
    }
#else
    getdata(0, 0, "\n°´ [RETURN] ¼ÌÐø", genbuf, 10, NOECHO, NULL, true);
#endif //SSHBBS

#ifdef CHECK_CONNECT
    if(check_ID_lists(currentuser->userid)) {
        prints("ÄãµÄÁ¬½ÓÆµÂÊ¹ý¸ß£¬byebye!");
        oflush();
        sleep(1);
        exit(1);
    }
#endif

	/* We must create home directory before initializing current userdata */
    sethomepath(genbuf, currentuser->userid);
    mkdir(genbuf, 0755);
/* init user data */
//    read_userdata(currentuser->userid, &curruserdata);

    clear();
    oflush();
    if (strcasecmp(currentuser->userid, "guest") && !HAS_PERM(currentuser, PERM_BASIC)) {
        sethomefile(genbuf, currentuser->userid, "giveup");
        fn = fopen(genbuf, "rt");
        if (fn) {
            while (!feof(fn)) {
                if (fscanf(fn, "%d %d", &i, &j) <= 0)
                    break;
                if (i == 1) {
                    fclose(fn);
                    sprintf(genbuf, "[32mÄãÒÑ¾­½äÍø£¬Àë½äÍø½áÊø»¹ÓÐ%dÌì[m\n", j - time(0) / 3600 / 24);
                    prints(genbuf);
                    oflush();
                    sleep(1);
                    exit(1);
                }
            }
            fclose(fn);
        }
        prints("[32m±¾ÕÊºÅÒÑÍ£»ú¡£ÇëÏò [36mSYSOP[32m ²éÑ¯Ô­Òò[m\n");
        oflush();
        sleep(1);
        exit(1);
    }
#ifdef DEBUG
    if (!HAS_PERM(currentuser, PERM_SYSOP)) {
        prints("±¾¶Ë¿Ú½ö¹©²âÊÔÓÃ£¬ÇëÁ¬½Ó±¾Õ¾µÄÆäËû¿ª·Å¶Ë¿Ú¡£\n");
        oflush();
        Net_Sleep(3);
        system_abort();
    }
#endif
    multi_user_check();

	if( read_user_memo( currentuser->userid, & currentmemo ) <= 0 ){
		prints("ÓÉÓÚ³ÌÐò¸üÐÂ£¬ÇëÏÈÍË³ö´ËÕÊºÅËùÓÐÁ¬½ÓÔÙÖØÐÂµÇÂ½\n");
	 	oflush();
		sleep(1);
		igetkey();
		exit(1);
	}

    alarm(0);
    signal(SIGALRM, SIG_IGN);   /*Haohmaru.98.11.12 */
    term_init();
    scrint = 1;
    sethomepath(tmpstr, currentuser->userid);
    sprintf(fname, "%s/%s.deadve", tmpstr, currentuser->userid);
    if ((fn = fopen(fname, "r")) != NULL) {
	    if(strcasecmp(currentuser->userid,"guest"))
            mail_file(currentuser->userid, fname, currentuser->userid, "²»Õý³£¶ÏÏßËù±£ÁôµÄ²¿·Ý...", BBSPOST_MOVE, NULL);
        else {
            fclose(fn);
			unlink(fname);
        }
    }
    temp_numposts = 0;          /*Haohmaru.99.4.02.ÈÃ°®¹àË®µÄÈË¿ÞÈ¥°É//grin */
}

void write_defnotepad()
{
    currentuser->notedate = time(NULL);
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
        fprintf(check, "%lu", lastnote);
        fclose(check);
        sprintf(tmp, "ÁôÑÔ°åÔÚ %s Login ¿ªÆô£¬ÄÚ¶¨¿ªÆôÊ±¼äÊ±¼äÎª %s", currentuser->userid, Ctime(lastnote));
        bbslog("user","%s",tmp);
    }
    if ((time(NULL) - lastnote) >= maxsec) {
        move(t_lines - 1, 0);
        prints("¶Ô²»Æð£¬ÏµÍ³×Ô¶¯·¢ÐÅ£¬ÇëÉÔºò.....");
        now = time(0);
        check = fopen("etc/checknotepad", "w");
        lastnote = time(NULL) - (time(NULL) % maxsec);
        fprintf(check, "%lu", lastnote);
        fclose(check);
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
                        post_file(currentuser, "", fname, bname, notetitle, 0, 1);
                        sprintf(tmp, "%s ×Ô¶¯ÕÅÌù", ntitle);
                        bbslog("user","%s",tmp);
                    }
                }
            }
            fclose(check);
        }
        sprintf(notetitle, "[%.10s] ÁôÑÔ°å¼ÇÂ¼", ctime(&now));
        if (dashf("etc/notepad")) {
            post_file(currentuser, "", "etc/notepad", "notepad", notetitle, 0, 1);
            unlink("etc/notepad");
        }
        bbslog("user","%s","×Ô¶¯·¢ÐÅÊ±¼ä¸ü¸Ä");
    }
    return;
}

void showsysinfo(char * fn)
{
    FILE* fp;
    char buf[500];
    int count=1,i,j;
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
    char ans[5], *ruser;
    unsigned unLevel = PERM_SUICIDE;

    /* ?????ºóÃæ»¹ÓÐcheck_register_info */
    newbbslog(BBSLOG_USIES,"ENTER @%s", fromhost);
    u_enter();
    sprintf(genbuf, "Enter from %-16s", fromhost);      /* Leeward: 97.12.02 */

    bbslog("user","%s",genbuf);
/*---	period	2000-10-19	4 debug	---*/
    newbbslog(BBSLOG_USIES,"ALLOC: [%d %d]", utmpent, usernum);
/*---	---*/
    started = 1;
    if (USE_NOTEPAD == 1)
        notepad_init();
    if (strcmp(currentuser->userid, "guest") != 0 && USE_NOTEPAD == 1) {
        if (DEFINE(currentuser, DEF_NOTEPAD)) {
            int noteln;

            if (lastnote > currentuser->notedate)
                currentuser->noteline = 0;
            noteln = countln("etc/notepad");
            if (lastnote > currentuser->notedate || currentuser->noteline == 0) {
                shownotepad();
                currentuser->noteline = noteln;
                write_defnotepad();
            } else if ((noteln - currentuser->noteline) > 0) {
                clear();
                ansimore2("etc/notepad", false, 0, noteln - currentuser->noteline + 1);
                prints("[31m¡Ñ©Ø¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©Ø¡Ñ[m\n");
                igetkey();
                currentuser->noteline = noteln;
                write_defnotepad();
                clear();
            }
        }
    }
    /* Leeward 98.09.24 Use SHARE MEM to diaplay statistic data below */
    if (DEFINE(currentuser, DEF_SHOWSTATISTIC)) {
        /*ansimore("0Announce/bbslists/countlogins", true); È¥µôÏÔÊ¾ÉÏÕ¾ÈËÊý */
#ifdef BLESS_BOARD
        if (dashf("etc/posts/bless"))
            ansimore("etc/posts/bless", true);
#endif
    }
    if (vote_flag(NULL, '\0', 2 /*¼ì²é¶Á¹ýÐÂµÄWelcome Ã» */ ) == 0) {
        if (dashf("Welcome")) {
            clear();
            ansimore("Welcome", true);
            vote_flag(NULL, 'R', 2 /*Ð´Èë¶Á¹ýÐÂµÄWelcome */ );
        }
    }
    clear();
    if (DEFINE(currentuser, DEF_SHOWHOT)) {     /* Leeward 98.09.24 Use SHARE MEM and disable old code
                                                   if (DEFINE(currentuser,DEF_SHOWSTATISTIC)) {
                                                   ansimore("etc/posts/day", false);
                                                   }
                                                 */
        ansimore("etc/posts/day", false);       /* Leeward: disable old code */
    }

    move(t_lines - 2 /*1 */ , 0);       /* Leeward: 98.09.24 Alter below message */
    clrtoeol();
    prints("[1;36m¡î ÕâÊÇÄúµÚ [33m%d[36m ´ÎÉÏÕ¾£¬ÉÏ´ÎÄúÊÇ´Ó [33m%s[36m Á¬Íù±¾Õ¾¡£\n", currentuser->numlogins + 1, currentuser->lasthost);
    prints("¡î ÉÏ´ÎÁ¬ÏßÊ±¼äÎª [33m%s[m ", Ctime(currentuser->lastlogin));
    igetkey();
    /* È«¹úÊ®´óÈÈÃÅ»°Ìâ added by Czz 020128 */
#ifdef CNBBS_TOPIC
    show_help("0Announce/bbslist/newsday");
#endif
    /* added end */
    if (dashf("0Announce/systeminfo")) {
        showsysinfo("0Announce/systeminfo");
        move(t_lines - 1 /*1 */ , 0);       /* Leeward: 98.09.24 Alter below message */
        clrtoeol();
        prints("[1;36m¡î °´ÈÎÒâ¼ü¼ÌÐø...[33m[m ");
        igetkey();
    }
    if (dashf("0Announce/hotinfo")) {
        ansimore("0Announce/hotinfo", false);
        move(t_lines - 1 /*1 */ , 0);       /* Leeward: 98.09.24 Alter below message */
        clrtoeol();
        prints("[1;36m¡î °´ÈÎÒâ¼ü¼ÌÐø...[33m[m ");
        igetkey();
    }
	/* Load currentuser's mailbox properties, added by flyriver, 2003.1.5 */
	uinfo.mailbox_prop = load_mailbox_prop(currentuser->userid);
    move(t_lines - 1, 0);
    sethomefile(fname, currentuser->userid, BADLOGINFILE);
    if (ansimore(fname, false) != -1) {
        getdata(t_lines - 1, 0, "ÄúÒªÉ¾³ýÒÔÉÏÃÜÂëÊäÈë´íÎóµÄ¼ÇÂ¼Âð (Y/N)? [Y] ", ans, 4, DOECHO, NULL, true);
        if (*ans != 'N' && *ans != 'n')
            unlink(fname);
    }

    strncpy(currentuser->lasthost, fromhost, IPLEN);
    currentuser->lasthost[15] = '\0';   /* dumb mistake on my part */
    currentuser->lastlogin = time(NULL);
    currentuser->numlogins++;

    /* Leeward 98.06.20 adds below 3 lines */
    if ((int) currentuser->numlogins < 1)
        currentuser->numlogins = 1;
    if ((int) currentuser->numposts < 0)
        currentuser->numposts = 0;
    if ((int) currentuser->stay < 0)
        currentuser->stay = 1;
    currentuser->userlevel &= (~unLevel);       /* »Ö¸´×ÔÉ±±êÖ¾ Luzi 98.10.10 */

    if (currentuser->firstlogin == 0) {
        currentuser->firstlogin = login_start_time - 7 * 86400;
    }
    check_register_info();
    load_mail_list(currentuser,&user_mail_list);
}

void set_numofsig()
{
    int sigln;
    char signame[STRLEN];

    sethomefile(signame, currentuser->userid, "signatures");
    sigln = countln(signame);
    numofsig = sigln / 6;
    if ((sigln % 6) != 0)
        numofsig += 1;
}

int chk_friend_book()
{
    FILE *fp;
    int idnum, n = 0;
    char buf[STRLEN], *ptr;

    move(3, 0);
    if ((fp = fopen("friendbook", "r")) == NULL)
        return n;
    prints("[1mÏµÍ³Ñ°ÈËÃû²áÁÐ±í:[m\n\n");
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
        if (idnum != usernum || idnum <= 0)
            continue;
        uin = t_search(uid, false);
        sprintf(msg, "%s ÒÑ¾­ÉÏÕ¾¡£", currentuser->userid);
        /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong */
        strcpy(MsgDesUid, uin ? uin->userid : "");
        idnum = 0;              /*Haohmaru.99.5.29.ÐÞÕýÒ»¸öbug,ÃâµÃÓÐÈËÀûÓÃÕâ¸öÀ´É§ÈÅ±ðÈË */
        if (uin != NULL && canbemsged(uin))
            idnum = do_sendmsg(uin, msg, 2);
        if (idnum)
            prints("%s ÕÒÄã£¬ÏµÍ³ÒÑ¾­¸æËßËû(Ëý)ÄãÉÏÕ¾µÄÏûÏ¢¡£\n", uid);
        else
            prints("%s ÕÒÄã£¬ÏµÍ³ÎÞ·¨ÁªÂçµ½Ëû(Ëý)£¬ÇëÄã¸úËû(Ëý)ÁªÂç¡£\n", uid);
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

void main_bbs(int convit, char *argv)
{
    extern char currdirect[255];
    char notename[STRLEN];
    int currmail;
	int summail;
	int nummail;

/* Add by KCN for avoid free_mem core dump */
    topfriend = NULL;
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
    conv_init();                /* KCN,99.09.05 */

    system_init();
    if (setjmp(byebye)) {
        system_abort();
    }
    login_query();
    user_login();
//    m_init();
    clear();
    load_key(NULL);

#ifdef HAVE_PERSONAL_DNS
  //¶¯Ì¬ÓòÃû¸üÐÂ
    if (HAS_PERM(currentuser, PERM_SYSOP)) {
        struct dns_msgbuf msg;
        int msqid;
        msqid = msgget(sysconf_eval("BBSDNS_MSG", 0x999), IPC_CREAT | 0664);
        if (msqid >= 0) {
	    struct msqid_ds buf;

            msg.mtype=1;
            strncpy(msg.userid,currentuser->userid,IDLEN);
            msg.userid[IDLEN]=0;
            //Ë®Ä¾ÊÇ¿ÉÒÔÓÃfromhostµÄ£¬²»¹ýÆäËû´ò¿ªdns·´½âµÃ¾ÍÒª¿¼ÂÇÒ»ÏÂÁË
            strncpy(msg.ip,fromhost,IPLEN);
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
    sprintf(genbuf, "sshbbsd:%s", currentuser->userid);
#else
    sprintf(genbuf, "bbsd:%s", currentuser->userid);
#endif
	set_proc_title(argv, genbuf);
#endif

#ifdef TALK_LOG
    tBBSlog_recover();             /* 2000.9.15 Bigman Ìí¼ÓÖÐ¶ÏtalkµÄ»Ö¸´ */
#endif

    setmailfile(currdirect, currentuser->userid, DOT_DIR);
    currmail = get_num_records(currdirect, sizeof(struct fileheader));
	get_mail_limit(currentuser, &summail, &nummail);
	if (currmail > nummail)
	{
		clear();
        prints("ÄãµÄÐÅ¼þ¸ß´ï %d ·â, ÇëÉ¾³ý¹ýÆÚÐÅ¼þ, Î¬³ÖÔÚ %d ·âÒÔÏÂ£¬·ñÔò½«²»ÄÜ·¢ÐÅ\n", currmail, nummail);
		pressanykey();
	}

	calc_calltime(1);
	while(calltime != 0 && calltime < time(0)){
		clear();
		move(1,0);
		prints("ÄúÓÐÒ»¸öÄÖÁåÔÚ %s",ctime(&calltime));
		move(3,0);
		prints("ÌáÊ¾ÐÅÏ¢Îª:");
		move(4,10);
		calltimememo[39]='\0';
		prints("[1;31m %s [m",calltimememo);
		prints("\n%s",ctime(&(currentuser->lastlogin)));
		move(t_lines-1,0);
		prints("                          press any key to continue...");
		refresh();
		calc_calltime(0);
		igetch();
	}

    if (HAS_PERM(currentuser, PERM_SYSOP) && dashf("new_register"))
        prints("ÓÐÐÂÊ¹ÓÃÕßÕýÔÚµÈÄúÍ¨¹ý×¢²á×ÊÁÏ¡£\n");

	chk_smsmsg(1);

    /*chk_friend_book(); */
    /* Leeward 98.12.03 */
    if (chk_friend_book()) {
        pressreturn();
    }
    clear();
    nettyNN = NNread_init();
    set_numofsig();
    if (DEFINE(currentuser, DEF_INNOTE)) {
        sethomefile(notename, currentuser->userid, "notes");
        if (dashf(notename))
            ansimore(notename, true);
    }
    b_closepolls();
    num_alcounter();
    if (count_friends > 0 && DEFINE(currentuser, DEF_LOGFRIEND))
        t_friends();
    while (1) {
        if (DEFINE(currentuser, DEF_NORMALSCR))
            domenu("TOPMENU");
        else
            domenu("TOPMENU2");
        Goodbye();
    }
}

/*Add by SmallPig*/
void update_endline()
{
    char buf[STRLEN];
    char stitle[256];
    time_t now;
    int allstay;
    int colour;

    if (DEFINE(currentuser, DEF_TITLECOLOR)) {
        colour = 4;
    } else {
        colour = currentuser->numlogins % 4 + 3;
        if (colour == 3)
            colour = 1;
    }
    if (!DEFINE(currentuser, DEF_ENDLINE)) {
        move(t_lines - 1, 0);
        clrtoeol();
        return;
    }
    now = time(0);
    allstay = (now - login_start_time) / 60;
    sprintf(buf, "[[36m%.12s[33m]", currentuser->userid);
    if (DEFINE(currentuser, DEF_NOTMSGFRIEND)) {
		if (DEFINE(currentuser,DEF_HIGHCOLOR))
        	sprintf(stitle, "[1;4%dm[33mÊ±¼ä[[36m%12.12s[33m] ºô½ÐÆ÷[ºÃÓÑ:%3s£ºÒ»°ã:%3s] Ê¹ÓÃÕß%s", colour, ctime(&now) + 4,
                (!(uinfo.pager & FRIEND_PAGER)) ? "NO " : "YES", (uinfo.pager & ALL_PAGER) ? "YES" : "NO ", buf);
		else
        	sprintf(stitle, "[4%dm[33mÊ±¼ä[[36m%12.12s[33m] ºô½ÐÆ÷[ºÃÓÑ:%3s£ºÒ»°ã:%3s] Ê¹ÓÃÕß%s", colour, ctime(&now) + 4,
                (!(uinfo.pager & FRIEND_PAGER)) ? "NO " : "YES", (uinfo.pager & ALL_PAGER) ? "YES" : "NO ", buf);
    } else {
#ifdef HAVE_FRIENDS_NUM
            num_alcounter();
            sprintf(stitle,"[1;4%dm[33mÊ±¼ä[[36m%12.12s[33m] ×ÜÈËÊý/ºÃÓÑ[%3d/%3d][%c£º%c] Ê¹ÓÃÕß%s",colour,
                    ctime(&now)+4,count_users,count_friends,(uinfo.pager&ALL_PAGER)?'Y':'N',(!(uinfo.pager&FRIEND_PAGER))?'N':'Y',buf);
#else
	if (DEFINE(currentuser,DEF_HIGHCOLOR))
        sprintf(stitle, "\x1b[1;4%dm\x1b[33mÊ±¼ä[\x1b[36m%12.12s\x1b[33m] ×ÜÈËÊý [ %3d ] [%c£º%c] Ê¹ÓÃÕß%s", colour,
                ctime(&now) + 4, get_utmp_number() + getwwwguestcount(), (uinfo.pager & ALL_PAGER) ? 'Y' : 'N', (!(uinfo.pager & FRIEND_PAGER)) ? 'N' : 'Y', buf);
	else
        sprintf(stitle, "\x1b[4%dm\x1b[33mÊ±¼ä[\x1b[36m%12.12s\x1b[33m] ×ÜÈËÊý [ %3d ] [%c£º%c] Ê¹ÓÃÕß%s", colour,
                ctime(&now) + 4, get_utmp_number() + getwwwguestcount(), (uinfo.pager & ALL_PAGER) ? 'Y' : 'N', (!(uinfo.pager & FRIEND_PAGER)) ? 'N' : 'Y', buf);
#endif
    }
    move(t_lines - 1, 0);
    prints("%s", stitle);
    clrtoeol();
    sprintf(stitle, "Í£Áô[%3d:%d]", (allstay / 60) % 1000, allstay % 60);
    move(t_lines - 1, -strlen(stitle)-1);
    prints("%s", stitle);
    resetcolor();

    /* Leeward 98.09.30 show hint for rookies */
    /* PERMs should coincide with ~bbsroot/etc/sysconf.ini: PERM_ADMENU */
    if (!DEFINE(currentuser, DEF_NORMALSCR) && MMENU == uinfo.mode && !HAS_PERM(currentuser, PERM_ACCOUNTS) 
        && !HAS_PERM(currentuser, PERM_SYSOP) && !HAS_PERM(currentuser, PERM_OBOARDS)
        && !HAS_PERM(currentuser, PERM_WELCOME) && !HAS_PERM(currentuser, PERM_ANNOUNCE)) {
        move(t_lines - 2, 0);
        clrtoeol();
        prints("[1m[32mÕâÊÇ¾«¼òÄ£Ê½Ö÷Ñ¡µ¥¡£ÒªÊ¹ÓÃÒ»°ãÄ£Ê½£¬ÇëÉè¶¨¸öÈË²ÎÊýµÚ£ÌÏîÎª£Ï£Î²¢Õý³£ÀëÕ¾ÔÙ½øÕ¾¡£[m");
    }
}


/*ReWrite by SmallPig*/
void showtitle( char *title, char*mid)
{
    char buf[STRLEN];
    char stitle[256];
    int spc1, spc2;
    int colour;
    char note[STRLEN];

    if (DEFINE(currentuser, DEF_TITLECOLOR)) {
        colour = BLUE;
    } else {
        colour = currentuser->numlogins % 4 + 3;
        if (colour == YELLOW)
            colour = RED;
    }

    if (selboard)
	    sprintf(note, "ÌÖÂÛÇø [%s]", currboard->filename);
    else {
        int bid;
        bid = getbnum(DEFAULTBOARD);
        if (bid==0) {
            bid=1; //try to get the first board
        }
        currboardent=bid;
        currboard=(struct boardheader*)getboard(bid);
        if (currboard==NULL)
            currboardent=0;
#ifdef HAVE_BRC_CONTROL
        brc_initial(currentuser->userid, DEFAULTBOARD);
#endif
        if (currboardent) {
            selboard = 1;
            sprintf(note, "ÌÖÂÛÇø [%s]", currboard->filename);
        } else
            sprintf(note, "Ä¿Ç°²¢Ã»ÓÐÉè¶¨ÌÖÂÛÇø");
    }


    spc1 = scr_cols/2 - 1 - strlen(mid) / 2;
    if ((strstr(title,"°æÖ÷")!=NULL)&&(spc1-num_noans_chr(title)<4))
	/*ÎªÁË¶à°æÖ÷ÐÞ¸Ä Bigman:2002.9.7 */
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
    setfcolor(YELLOW,DEFINE(currentuser,DEF_HIGHCOLOR));
    setbcolor(colour);
    clrtoeol();
    prints("%s", title);

    move(0, spc1);
    resetcolor();
    if(strcmp(mid, BBS_FULL_NAME)&&mid[0]!='[')
        setfcolor(CYAN, DEFINE(currentuser,DEF_HIGHCOLOR));
    else
        setfcolor(WHITE, DEFINE(currentuser,DEF_HIGHCOLOR));
    setbcolor(colour);
    if(mid[0]=='[') prints("[5m");
    prints("%s", mid);

    move(0, -strlen(note));
    resetcolor();
    setfcolor(YELLOW,DEFINE(currentuser,DEF_HIGHCOLOR));
    setbcolor(colour);
    prints("%s", note);
    resetcolor();
    move(1, 0);
}


void docmdtitle(char *title, char *prompt)
{
    char middoc[30];
    int chkmailflag = 0;
	int chksmsmsg = 0;

/*    if (getbnum(DEFAULTBOARD)) 
    {
        bp = getbcache( DEFAULTBOARD );
        memcpy( bmstr, bp->BM, BM_LEN-1 );
    }else
         strcpy(bmstr," ");
*/
    chkmailflag = chkmail();
	chksmsmsg = chk_smsmsg(0);

    if (chkmailflag == 2)       /*Haohmaru.99.4.4.¶ÔÊÕÐÅÒ²¼ÓÏÞÖÆ */
        strcpy(middoc, "[ÐÅÏä³¬ÈÝ]");
    else if (chkmailflag)
        strcpy(middoc, "[ÄúÓÐÐÅ¼þ]");
/*    else if ( vote_flag( DEFAULTBOARD, '\0' ,0) == 0&&(bp->flag&BOARD_VOTEFLAG))
        strcpy(middoc,"[ÏµÍ³Í¶Æ±ÖÐ]");*/
    else
        strcpy(middoc, BBS_FULL_NAME);

	if( chksmsmsg ){
		if( chkmailflag ){
			strcat(middoc, "[ÄúÓÐ¶ÌÐÅ]");
		}else{
			strcpy(middoc, "[ÄúÓÐ¶ÌÐÅ]");
		}
	}

    showtitle(title, middoc);
    move(1, 0);
    clrtoeol();
    prints("%s", prompt);
    clrtoeol();
}

/* 2000.9.15 Bigman »Ö¸´ÁÄÌì¼ÇÂ¼ */
#ifdef TALK_LOG

int tBBSlog_recover()
{
    char buf[256];

    sprintf(buf, "home/%c/%s/talklog", toupper(currentuser->userid[0]), currentuser->userid);

    if (strcasecmp(currentuser->userid, "guest") == 0 || !dashf(buf))
        return;

    clear();
    strcpy(genbuf, "");

    getdata(0, 0, "\033[1;32mÄúÓÐÒ»¸ö²»Õý³£¶ÏÏßËùÁôÏÂÀ´µÄÁÄÌì¼ÇÂ¼, ÄúÒª .. (M) ¼Ä»ØÐÅÏä (Q) ËãÁË£¿[Q]£º\033[m", genbuf, 4, DOECHO, NULL, true);

    if (genbuf[0] == 'M' || genbuf[0] == 'm')
        mail_file(currentuser->userid, buf, currentuser->userid, "ÁÄÌì¼ÇÂ¼", BBSPOST_MOVE, NULL);
    else
        unlink(buf);
    return;

}
#endif
