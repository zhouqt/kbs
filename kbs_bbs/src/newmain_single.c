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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "bbs.h"

#define BADLOGINFILE    "logins.bad"
#define INPUT_ACTIVE 0/*Haohmaru.98.11.3*/
#define INPUT_IDLE 1
#define WAITTIME  150

/* KCN add 1999.11.07 
#undef LOGINASNEW */

extern struct screenline *big_picture;
extern struct userec *user_data;
/* extern char* pnt; */
extern struct friend *topfriend;

int	temp_numposts;/*Haohmaru.99.4.02.ÈÃ°®¹àË®µÄÈË¿ÞÈ¥°É//grin*/
int     nettyNN=0;
int     count_friends,count_users;/*Add by SmallPig for count users and Friends*/
int     iscolor=1;
int     nf;
char    *getenv();
char    *sysconf_str();
char    *Ctime();
struct user_info *t_search();
void    r_msg();
int             friend_login_wall();
int     listmode;
int     numofsig=0;
jmp_buf byebye ;

int convcode=0; /* KCN,99.09.05 */
extern conv_init(); /* KCN,99.09.05 */        

FILE *ufp ;
int     RUNSH=NA;
int     ERROR_READ_SYSTEM_FILE=NA;
int talkrequest = NA ;
int ntalkrequest = NA ;
int enter_uflags;
time_t lastnote;

struct user_info uinfo ;

/* char netty_path[ 60 ]; FAINT!!! ÔõÃ´»á²»³ö´íÄØ!!! Leeward: 1997.12.10 */
char netty_path[ 256 ];
char fromhost[ 60 ] ;
char tty_name[ 20 ] ;

char BoardName[STRLEN] ;
char ULIST[STRLEN] ;
int utmpent = -1 ;
time_t  login_start_time;
int     showansi=1;

extern char MsgDesUid[14]; /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong */
 
static int i_mode = INPUT_ACTIVE;/*Haohmaru.98.11.3*/

int
canbemsged(uin)/*Haohmaru.99.5.29*/
struct user_info *uin;
{
    if (uinfo.pager&ALLMSG_PAGER) return YEA;
    if (uinfo.pager&FRIENDMSG_PAGER)
    {
        if(can_override(currentuser.userid,uin->userid))
                return YEA;
    }
    return NA;
}

void
wait_alarm_clock()/*Haohmaru.98.11.3*/
{
    if(i_mode == INPUT_IDLE) {
        clear();
        kill(getpid(),SIGHUP) ;
    }
    i_mode = INPUT_IDLE ;
    alarm(WAITTIME) ;
}

void
initalarm()/*Haohmaru.98.11.3*/
{
        signal(SIGALRM,wait_alarm_clock) ;
        alarm(WAITTIME) ;
}

void abort_bbs();

int Net_Sleep(times) /* KCN 1999.9.15 */
int times;
{
        struct timeval tv ;
        int     sr;
        fd_set fd,efd;
	int old;

        int csock = 0;
        tv.tv_sec = times;
        tv.tv_usec = 0;
        FD_ZERO(&fd);
        FD_ZERO(&efd);
        FD_SET(csock,&fd);
        FD_SET(csock,&efd);
	old=time(0);

        while((sr=select(csock+1,&fd,NULL,&efd,&tv))>0) {
                if (FD_ISSET(csock,&efd))
                        abort_bbs();
                tv.tv_sec = times-(time(0)-old);
	        tv.tv_usec = 0;
	        FD_ZERO(&fd);
	        FD_ZERO(&efd);
	        FD_SET(csock,&fd);
	        FD_SET(csock,&efd);
        };

}

void
log_usies( mode, mesg )
char *mode, *mesg;
{
    time_t      now;
    FILE        *fp;
    char        buf[ 256 ], *fmt;

    now = time(0);
    fmt = currentuser.userid[0] ? "%s %s %-12s %s\n" : "%s %s %s%s\n";
    sprintf( buf, fmt, Ctime( &now )+4, mode, currentuser.userid, mesg );
    if( (fp = fopen( "usies", "a" )) != NULL ) {
        fputs( buf, fp );
        fclose( fp );
    }
}

void
u_enter()
{

    enter_uflags = currentuser.flags[0];
    memset( &uinfo, 0, sizeof( uinfo ) );
    uinfo.active = YEA ;
    uinfo.pid    = getpid();
/*    if( HAS_PERM(PERM_LOGINCLOAK) && (currentuser.flags[0] & CLOAK_FLAG) && HAS_PERM(PERM_SEECLOAK)) */

    /* Bigman 2000.8.29 ÖÇÄÒÍÅÄÜ¹»ÒþÉí */
    if( (HAS_PERM(PERM_CHATCLOAK) || HAS_PERM(PERM_CLOAK)) && (currentuser.flags[0] & CLOAK_FLAG))
        uinfo.invisible = YEA;
    uinfo.mode = LOGIN ;
    uinfo.pager = 0;
/*    uinfo.pager = !(currentuser.flags[0] & PAGER_FLAG);*/
    if(DEFINE(DEF_FRIENDCALL))
    {
        uinfo.pager|=FRIEND_PAGER;
    }
    if(currentuser.flags[0] & PAGER_FLAG)
    {
        uinfo.pager|=ALL_PAGER;
        uinfo.pager|=FRIEND_PAGER;
    }
    if(DEFINE(DEF_FRIENDMSG))
    {
        uinfo.pager|=FRIENDMSG_PAGER;
    }
    if(DEFINE(DEF_ALLMSG))
    {
        uinfo.pager|=ALLMSG_PAGER;
        uinfo.pager|=FRIENDMSG_PAGER;
    }
    uinfo.uid = usernum;
    strncpy( uinfo.from, fromhost, 60 );
#ifdef SHOW_IDLE_TIME
/* KCN modified for my error  1999.9.1*/
    if (tty_name[0])
    strncpy( uinfo.tty, tty_name, 20 );
    else {
      time_t now;
      now=time(0);
      memcpy(uinfo.tty+1,&now,sizeof(time_t));
    }
#endif
    iscolor=(DEFINE(DEF_COLOR))? 1:0;
    strncpy( uinfo.userid,   currentuser.userid,   20 );
    strncpy( uinfo.realname, currentuser.realname, 20 );
    strncpy( uinfo.username, currentuser.username, 40 );
    memset( uinfo.stuff, '-', sizeof( uinfo.stuff ) );
    nf=0;
    topfriend=NULL;
    getfriendstr();
    utmpent = getnewutmpent(&uinfo) ;
    if (utmpent == -1)
    {
        prints("ÈËÊýÒÑÂú,ÎÞ·¨·ÖÅäÓÃ»§ÌõÄ¿!\n");
        oflush();
	Net_Sleep(20);
        exit(-1);
    }

    listmode=0;
    digestmode=NA;
}

void
setflags(mask, value)
int mask, value;
{
    if (((currentuser.flags[0] & mask) && 1) != value) {
        if (value) currentuser.flags[0] |= mask;
        else currentuser.flags[0] &= ~mask;
    }
}
/*---	moved to here from below	period	2000-11-19	---*/
int started = 0;
void
u_exit()
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
    if(!started || !uinfo.active) return;
/*---		---*/
    setflags(PAGER_FLAG, (uinfo.pager&ALL_PAGER));
/*    if (HAS_PERM(PERM_LOGINCLOAK)&&HAS_PERM(PERM_SEECLOAK))*/

   /* Bigman 2000.8.29 ÖÇÄÒÍÅÄÜ¹»ÒþÉí */
	if((HAS_PERM(PERM_CHATCLOAK) || HAS_PERM(PERM_CLOAK)))
        setflags(CLOAK_FLAG, uinfo.invisible);

    if( currentuser.flags[0] != enter_uflags ) {
        set_safe_record();
        substitute_record(PASSFILE,&currentuser,sizeof(currentuser),usernum);
    }

    uinfo.active = NA ;
    uinfo.pid = 0 ;
    uinfo.invisible = YEA ;
    uinfo.sockactive = NA ;
    uinfo.sockaddr = 0 ;
    uinfo.destuid = 0 ;
#ifdef SHOW_IDLE_TIME
    strcpy(uinfo.tty, "NoTTY");
#endif
    update_utmp();
}

int
cmpuids(uid,up)
char *uid ;
struct userec *up ;
{
    return !strncasecmp(uid,up->userid,sizeof(up->userid)) ;
}

int
dosearchuser(userid)
char *userid ;
{
    int         id;

    if( (id = getuser( userid )) != 0 ) {
        if( cmpuids( userid, &lookupuser ) ) {
            memcpy( &currentuser, &lookupuser, sizeof(currentuser) );
            return usernum = id;
        }
    }
    memset( &currentuser, 0, sizeof( currentuser ) );
    return usernum = 0;
}

/*int started = 0;*/
/*
void
ntalk_request()
{
    signal(SIGUSR2,ntalk_request) ;
    ntalkrequest = YEA ;
    bell(); bell(); bell();
    sleep( 1 );
    bell(); bell(); bell(); bell(); bell();
    return ;
}
*/

void
talk_request()
{
    signal(SIGUSR1,talk_request) ;
    talkrequest = YEA ;
    bell(); bell(); bell();
    sleep( 1 );
    bell(); bell(); bell(); bell(); bell();
    return ;
}


void
abort_bbs()
{
    time_t      stay;

    if(uinfo.mode==POSTING||uinfo.mode==SMAIL||uinfo.mode==EDIT
        ||uinfo.mode==EDITUFILE||uinfo.mode==EDITSFILE||uinfo.mode==EDITANN)
                keep_fail_post();
    if( started ) {
        record_exit_time();
        stay = time( 0 ) - login_start_time;
/*---	period	2000-10-20	4 debug	---*/
	sprintf( genbuf, "Stay: %3ld (%s)[%d %d]", stay/60, currentuser.username, utmpent, usernum);
/*	sprintf( genbuf, "Stay: %3ld (%s)", stay / 60, currentuser.username );*/
        log_usies( "AXXED", genbuf );
        u_exit();
    }
    free_mem(); /* free the memory , by dong 1998.8.29 */
    shutdown(0,2);
    close(0);
    exit( 0 );

}

int
cmpuids2(unum, urec)
int unum;
struct user_info *urec;
{
    return (unum == urec->uid);
}

int
count_multi( uentp )
struct user_info *uentp;
{
    static int  count;

    if( uentp == NULL ) {
        int     num = count;
        count = 0;
        return num;
    }
    if( !uentp->active || !uentp->pid )
        return 0;
    if( uentp->uid == usernum )
        count++;
    return 1;
}

int
count_user()
{
    count_multi( NULL );
    apply_ulist( count_multi );
    return count_multi( NULL );
}

void RemoveMsgCountFile()
{
  char fname[STRLEN];
  setuserfile(fname,"msgcount");
  unlink(fname);
 }

void RemoveMsgCountFile2(userID)
char *userID;
{
  char fname[STRLEN];
  sethomefile(fname,userID,"msgcount");
  unlink(fname);
 }

void
multi_user_check()
{
    struct user_info    uin;
    int         curr_login_num;
    char        buffer[40];

    if (count_user()<1) RemoveMsgCountFile();

    if (HAS_PERM(PERM_MULTILOG)) 
        return;  /* don't check sysops */
    curr_login_num = num_active_users(); /* add by alex , 4/15/97 */
    /* Leeward: 97.12.22 BMs may open 2 windows at any time */
    /* Bigman: 2000.8.17 ÖÇÄÒÍÅÄÜ¹»¿ª2¸ö´°¿Ú */
    if ((HAS_PERM(PERM_BOARDS) || HAS_PERM(PERM_CHATOP) || HAS_PERM(PERM_CHATCLOAK)) && count_user() < 2)
        return;
    /* allow multiple guest user */
    if (!strcmp("guest", currentuser.userid)) {
        if ( count_user() > 24 ) {
            prints( "[33m±§Ç¸, Ä¿Ç°ÒÑÓÐÌ«¶à [36mguest, ÇëÉÔºòÔÙÊÔ¡£[m\n");
            pressreturn();
            oflush();
            exit(1);
        }
        return;
    }
    else if ( (curr_login_num<700)&&(count_user()>=2) 
           || (curr_login_num>=700)&& (count_user()>=1) ) /*user login limit*/
    {  
        getdata(0, 0, "ÄãÍ¬Ê±ÉÏÏßµÄ´°¿ÚÊý¹ý¶à£¬ÊÇ·ñÌß³ö±¾IDÆäËü´°¿Ú(Y/N)? [N]", 
                genbuf, 4, DOECHO, NULL, YEA);
        if(genbuf[0] == 'Y' || genbuf[0] == 'y') 
        {
		int lres;
                if ( !search_ulist( &uin, cmpuids2, usernum) )
                        return;  /* user isn't logged in */
                if (!uin.active || (kill(uin.pid,0) == -1))
                        return;  /* stale entry in utmp file */
/*---	modified by period	first try SIGHUP	2000-11-08	---*/
		lres = kill(uin.pid, SIGHUP);
		sleep(1);
		if(lres)
/*---	---*/
                kill(uin.pid,9);
                sprintf(buffer, "kicked (multi-login)" );
                report(buffer);
                log_usies( "KICK ", currentuser.username );

		return ; /* ²»¼ÌÐø¼ì²é£¬·µ»Ø, ²»Ìß×Ô¼º´°¿Ú, added by dong, 1999.1.25 */
        } 
        oflush();
        exit(1);       /* ¶à´°¿ÚÊ±ÌßµôÒ»¸ö£¬×Ô¼ºÒ²¶ÏÏß */
    }
    if ( !search_ulist( &uin, cmpuids2, usernum) )
        return;  /* user isn't logged in */
    if (!uin.active || (kill(uin.pid,0) == -1))
        return;  /* stale entry in utmp file */

    getdata(0, 0, "ÄúÏëÉ¾³ýÖØ¸´µÄ login Âð (Y/N)? [N]", genbuf, 4, 
            DOECHO, NULL,YEA);

    if(genbuf[0] == 'Y' || genbuf[0] == 'y') {
       RemoveMsgCountFile();
       kill(uin.pid,9);
        sprintf(buffer, "kicked (multi-login)" );
        report(buffer);
        log_usies( "KICK ", currentuser.username );
    }
    else if ( (curr_login_num<700)&&(count_user()>=2)
              || (curr_login_num>=700)&& (count_user()>=1) )
        {
           oflush();
                exit(1);
        }          /* ÔÙÅÐ¶ÏÒ»´ÎÈËÊý Luzi 99.1.23 */


}

int
simplepasswd( str )
char *str;
{
    char        ch;

    while( (ch = *str++) != '\0' ) {
        if( ! (ch >= 'a' && ch <= 'z') )
            return 0;
    }
    return 1;
}

void
system_init(char *sourceip)
{
    char        *rhost;

    login_start_time = time( 0 );
    gethostname( genbuf ,256 );
#ifdef SINGLE
    if( strcmp( genbuf, SINGLE ) ) {
        prints("Not on a valid machine!\n") ;
	oflush();
        exit(-1) ;
    }
#endif
    sprintf( ULIST, "%s.%s", ULIST_BASE, genbuf );

/*    if( argc >= 3 ) {
*/        strncpy( fromhost, sourceip, 60 );
/*    } else {
        fromhost[0] = '\0';
    }
*/    if( (rhost = getenv( "REMOTEHOSTNAME" )) != NULL )
        strncpy( fromhost, rhost, 60 );
#ifdef SHOW_IDLE_TIME
/*    if(argc >= 4) { 
        strncpy( tty_name, argv[3], 20 ) ;
    } else {
*/        tty_name[0] = '\0' ;
/*    }
*/
#endif

    signal(SIGHUP,abort_bbs) ;
    signal(SIGPIPE,abort_bbs) ;
    signal(SIGTERM,abort_bbs) ;
    signal(SIGQUIT,abort_bbs) ;
#ifndef lint
    signal(SIGINT,SIG_IGN) ;
#ifdef DOTIMEOUT
    init_alarm();
#else
    signal(eIGALRM,SIG_SIG) ;
#endif
    signal(SIGURG,SIG_IGN) ;
    signal(SIGTSTP,SIG_IGN) ;
    signal(SIGTTIN,SIG_IGN) ;
    signal(SIGUSR1,talk_request) ;
    signal(SIGUSR2,r_msg) ;
/*    signal(SIGUSR2,ntalk_request) ;
    signal(SIGTTOU,r_msg) ;*/
#endif
}

void
system_abort()
{
    if( started ) {
        log_usies( "ABORT", currentuser.username );
        u_exit() ;
    }
    clear();
    refresh();
    prints("Ð»Ð»¹âÁÙ, ¼ÇµÃ³£À´à¸ !\n");
    oflush();
    abort_bbs();
    return;
    free_mem(); /* Leeward 98.10.13 */
    exit(0) ;
}

void
logattempt( uid, frm )
char *uid, *frm;
{
    char        fname[ STRLEN ];
    int         fd, len;

    sprintf( genbuf, "%-12.12s  %-30s %s\n",
                uid, Ctime( &login_start_time ), frm );
    len = strlen( genbuf );
    if( (fd = open( BADLOGINFILE, O_WRONLY|O_CREAT|O_APPEND, 0644 )) > 0 ) {
        write( fd, genbuf, len );
        close( fd );
    }
    sethomefile( fname, uid, BADLOGINFILE );
    if( (fd = open( fname, O_WRONLY|O_CREAT|O_APPEND, 0644 )) > 0 ) {
        write( fd, genbuf, len );
        close( fd );
    }
}
int
check_ban_IP(char *IP, char *buf)
{ /* Leeward 98.07.31
  RETURN:
 - 1: No any banned IP is defined now
 0: The checked IP is not banned
  other value over 0: The checked IP is banned, the reason is put in buf
  */
  FILE *Ban = fopen(".badIP", "r");
  char IPBan[64];
  int  IPX = - 1;
  char *ptr;

  if (!Ban)
    return IPX;
  else
    IPX ++;

  while (fgets(IPBan, 64, Ban))
  {
    if (ptr = strchr(IPBan, '\n'))
      *ptr = 0;
    if (ptr = strchr(IPBan, ' '))
    {
      *ptr ++ = 0;
      strcpy(buf, ptr);
    }
    IPX = strlen(IPBan);
    if (*IPBan=='+')
        if (!strncmp(IP, IPBan+1, IPX-1))
          break;
    IPX = 0;
  }

  fclose(Ban);
  return IPX;
}

#ifdef AIX /* Leeward 99.03.06 */
int num_active_http_users()
{
    int a, b ;
    struct hostent *h ;
    char buf[2048] ;
    char hostname[STRLEN] ;
    struct sockaddr_in sin ;
    char *ptr;
    
    gethostname(hostname,STRLEN) ;
    if(!(h = gethostbyname(hostname))) /*perror("gethostbyname") ;*/
        return -1 ;
    memset(&sin, 0, sizeof sin) ;
    sin.sin_family = h->h_addrtype ;
    memcpy( &sin.sin_addr, h->h_addr, h->h_length) ;
    sin.sin_port = 80 ;
    a = socket(sin.sin_family,SOCK_STREAM,0) ;
    if((connect(a, (struct sockaddr *)&sin, sizeof sin))) /*perror("connect err") ;*/
        return -1 ;
    write(a,"GET /server-status\r\n", 20) ;
    read(a,buf,2048) ;
    close(a) ;
    ptr = strstr(buf, " requests currently being processed,");
    if (NULL == ptr)
        return -1;
    for (; '\n' != *ptr; ptr --) ;
    sscanf(ptr + 1, "%d requests currently being processed, %d idle servers", &a, &b);
    return (a+b);
}
#endif

void
login_query()
{
    char        uid[STRLEN], passbuf[PASSLEN], *ptr;
    int         curr_login_num;
    int 	curr_http_num; /* Leeward 99.03.06 */
    int         attempts;
    char fname[STRLEN], tmpstr[30];
    FILE *fn;   
    char buf[256];
    curr_login_num = num_active_users();
    if( curr_login_num >= MAXACTIVE ) {
        ansimore( "etc/loginfull", NA );
        oflush();
        Net_Sleep( 20 );
        exit( 1 ) ;
    }
    curr_http_num = 0;
/*disable by KCN     curr_http_num = num_active_http_users(); *//* Leeward 99.03.06 */

    ptr = sysconf_str( "BBSNAME" );
    if( ptr == NULL )  ptr = "ÉÐÎ´ÃüÃû²âÊÔÕ¾";
    strcpy( BoardName, ptr );
/* add by KCN for input bbs */
#ifdef DOTIMEOUT
        initalarm();
#else
        signal(SIGALRM, SIG_IGN);
#endif
    output("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",22);
    attempts=5;
    while(attempts) {
    getdata(0, 0, "Please input(login as) bbs or bb5 here ==>",passbuf,6, DOECHO, NULL,YEA);
    if (!strcmp(passbuf,"bbs")) break;
    if (!strcmp(passbuf,"bbsbm")) break;
    if (!strcmp(passbuf,"bbsop")) break;
    if (!strcmp(passbuf,"bb5")) {convcode=1;break;}
    prints("You entered an invalid login name or password.\n");
    attempts--;
    if (attempts==0)  {
	prints("login invalid.\n");
	oflush();
	exit(-1);
    }
    }

    /* Leeward 98.09.24 Use SHARE MEM and disable the old code */
    if(fill_shmfile(1,"etc/issue","ISSUE_SHMKEY"))
    {
        show_issue();
    }                
    /*strcpy(fname,"etc/issue"); Leeward: disable the old code */
/*    if(dashf(fname,"r")) This block is disabled for long, not for SHARE MEM
    {
        issues=countlogouts(fname);
        if(issues>=1)
        {
                user_display(fname,(issues==1)?1:
                                   ((time(0)/24*60*60)%(issues))+1,NA);
        }
    }*/
    /*ansimore(fname,NA); Leeward: disable the old code */

    prints( "\033[1m»¶Ó­¹âÁÙ [31m%s[37m ¡ô±¾Õ¾Ê¹ÓÃÊï¹â¹«Ë¾Êï¹âÌìÑÝ·þÎñÆ÷¡ô [36mÄ¿Ç°ÉÏÏßÈËÊý \033[1m%d[m", BoardName, curr_login_num);
/*{
char ii[16];
sprintf(ii, "%.2f", (double)curr_login_num / (double)MAXACTIVE * 100.0);
    prints( "\033[1m»¶Ó­¹âÁÙ [31m%s[37m ¡ô±¾Õ¾Ê¹ÓÃÊï¹â¹«Ë¾Êï¹âÌìÑÝ·þÎñÆ÷¡ô [36mÄ¿Ç°ÉÏÏßÈËÊý \033[1m%s%%[m", BoardName, ii);
}*/

    if ((curr_http_num != -1) && (curr_http_num != 0)) /* dong 2000.4.18 */ 
      prints("[1m[36m+%d[m", curr_http_num); /* Leeward 99.03.06 */

    attempts = 0;
    while( 1 ) {
        if( attempts++ >= LOGINATTEMPTS ) {
            ansimore( "etc/goodbye", NA );
            oflush();
            sleep( 1 );
            exit( 1 );
        }
/*Haohmaru.98.11.3*/
#ifdef DOTIMEOUT
        initalarm();
#else
        signal(SIGALRM, SIG_IGN);
#endif

        getdata( 0, 0, "\n\033[1m[37mÇëÊäÈë´úºÅ(ÊÔÓÃÇëÊäÈë `\033[36mguest\033[37m', ×¢²áÇëÊäÈë`\033[36mnew\033[37m'): [m",/*
        getdata( 0, 0, "\n\033[1m[37mÇëÊäÈë´úºÅ(ÊÔÓÃÇëÊäÈë `\033[36mguest\033[37m'; ÒòÏµÍ³¹ÊÕÏ£¬ÔÝÍ£×¢²áÐÂÕÊºÅ¹¦ÄÜÒ»¶ÎÊ±¼ä£¬¼ûÁÂ): [m",*/
                uid, STRLEN-1, DOECHO, NULL ,YEA);
        if( strcmp( uid, "new" ) == 0 ) {
#ifdef LOGINASNEW
            /*prints( "\033[32m´íÎóµÄÊ¹ÓÃÕß´úºÅ...\033[m\n" );*/
            if (check_ban_IP(fromhost,buf)<=0)
	    {
	        memset( &currentuser, 0, sizeof( currentuser ) );
		new_register();
		sethomepath(tmpstr, currentuser.userid);
		sprintf( buf, "/bin/mv -f %s /home0/bbs/homeback/%s", tmpstr,currentuser.userid);
		system( buf );
		setmailpath(tmpstr, currentuser.userid);/*Haohmaru.00.04.23,ÃâµÃÄÜ¿´Ç°ÈËµÄÐÅ*/
		sprintf( buf, "/bin/mv -f %s /home0/bbs/mailback/%s", tmpstr,currentuser.userid);
		system( buf );
		break;
	    }
            prints( "±¾ÏµÍ³ÒòÎª %s µÄÔ­Òò½ûÖ¹ÄúËùÔÚÍø¶Î×¢²áÐÂÓÃ»§\n",buf);
#else
            prints( "[37m±¾ÏµÍ³Ä¿Ç°ÎÞ·¨ÒÔ new ×¢²á, ÇëÓÃ guest ½øÈë.[m\n" );
#endif
        } else if( *uid == '\0' || !dosearchuser( uid ) ) {
            prints( "[32m´íÎóµÄÊ¹ÓÃÕß´úºÅ...[m\n" );
        } else 
/* Add by KCN for let sysop can use extra 10 UTMP */
        if(!HAS_PERM(PERM_ADMINMENU)&&( curr_login_num >= MAXACTIVE+10 )) {
        	ansimore( "etc/loginfull", NA );
	        oflush();
	        sleep( 1 );
	        exit( 1 ) ;
        } else if( /*strcmp*/strcasecmp( uid, "guest" ) == 0 ) {
            currentuser.userlevel = 0;
            currentuser.flags[0] = CURSOR_FLAG | PAGER_FLAG;
            break;
        } else {
	    if (!convcode)
            	convcode=!(currentuser.userdefine&DEF_USEGB);  /* KCN,99.09.05 */
            getdata( 0, 0, "\033[1m[37mÇëÊäÈëÃÜÂë: [m", passbuf, PASSLEN, NOECHO, NULL ,YEA);
            passbuf[8] = '\0';
/*  COMMAN : Ê¹ÓÃÀÏµÄ checkpassword ÒÔ½ÚÊ¡ CPU Load
            if( !checkpasswd(currentuser.passwd, passbuf ))
*/
	    if( !checkpasswd2(currentuser.passwd, passbuf ))
	    {
                logattempt( currentuser.userid, fromhost );
                prints( "[32mÃÜÂëÊäÈë´íÎó...[m\n" );
            } else {
                if( !HAS_PERM( PERM_BASIC ) ) {
                    prints( "[32m±¾ÕÊºÅÒÑÍ£»ú¡£ÇëÏò [36mSYSOP[32m ²éÑ¯Ô­Òò[m\n" );
                    oflush();
                    sleep( 1 );
                    exit( 1 );
                }
                if(id_invalid(uid))
                {
                  prints("[31m±§Ç¸!![m\n");
                  prints("[32m±¾ÕÊºÅÊ¹ÓÃÖÐÎÄÎª´úºÅ£¬´ËÕÊºÅÒÑ¾­Ê§Ð§...[m\n");
                  prints("[32mÏë±£ÁôÈÎºÎÇ©ÃûµµÇë¸úÕ¾³¤ÁªÂç £¬Ëû(Ëý)»áÎªÄã·þÎñ¡£[m\n");
                  getdata( 0, 0, "°´ [RETURN] ¼ÌÐø",genbuf,10,NOECHO,NULL,YEA);
                    oflush();
                    sleep( 1 );
                    exit( 1 );
                }
                if( simplepasswd( passbuf ) ) {
                    prints("[33m* ÃÜÂë¹ýì¶¼òµ¥, ÇëÑ¡ÔñÒ»¸öÒÔÉÏµÄÌØÊâ×ÖÔª.[m\n");
                    getdata( 0, 0, "°´ [RETURN] ¼ÌÐø",genbuf,10,NOECHO,NULL,YEA);
                }
                break;
            }
        }
    }
#ifdef DEBUG
    if (!HAS_PERM(PERM_SYSOP)) {
	prints("±¾¶Ë¿Ú½ö¹©²âÊÔÓÃ£¬ÇëÁ¬½Ó±¾Õ¾µÄÆäËû¿ª·Å¶Ë¿Ú¡£\n");
	oflush();
	Net_Sleep(3);
	system_abort();
    }
#endif
    multi_user_check();
    signal(SIGALRM, SIG_IGN);/*Haohmaru.98.11.12*/
    alarm(IDLE_TIMEOUT);
    term_init("vt100");
    scrint = 1 ;
    sethomepath(tmpstr, currentuser.userid);
    sprintf(fname,"%s/%s.deadve", tmpstr, currentuser.userid);
    if((fn=fopen(fname,"r"))!=NULL)
    {
        mail_file(fname,currentuser.userid,"²»Õý³£¶ÏÏßËù±£ÁôµÄ²¿·Ý...");
        unlink(fname);
            fclose(fn);
    }
    sethomepath( genbuf, currentuser.userid );
    mkdir( genbuf, 0755 );
    temp_numposts=0;/*Haohmaru.99.4.02.ÈÃ°®¹àË®µÄÈË¿ÞÈ¥°É//grin*/
}

int
valid_ident( ident )
char *ident;
{
    static char *invalid[] = { "unknown@", "root@", "gopher@", "bbs@",
        "guest@", NULL };
    int         i;

    for( i = 0; invalid[i] != NULL; i++ )
        if( strstr( ident, invalid[i] ) != NULL )
            return 0;
    return 1;
}

void
write_defnotepad()
{
  currentuser.notedate=time(NULL);
  set_safe_record();
  substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
  return;
}

/*
void
defnotepad()
{
  FILE          *def2;
  char          fname[STRLEN],spbuf[STRLEN];
  
 
  setuserfile(fname,"defnotepad");
  if((def2=fopen(fname,"r"))!=NULL)
  {
    fgets(spbuf,sizeof(spbuf),def2);
    currentuser.notemode=spbuf[0]-'0';
    if(currentuser.notemode==2)
    {
        fgets(spbuf,sizeof(spbuf),def2);
        currentuser.notedate=atol(spbuf);
        fgets(spbuf,sizeof(spbuf),def2);
        currentuser.noteline=atol(spbuf);
    }
    fclose(def2);
    unlink(fname);
  }

  if(currentuser.notemode==-1)
  {
     fclose(def2);
     while(1){
        getdata( 0, 0, "ÇëÊäÈëÄãÒªµÄÁôÑÔ°åÄ£Ê½ (1) È«¿´ (2) Ö»¿´Ã»¿´¹ýµÄ (3) ¶¼²»¿´: ", spbuf, 2, DOECHO, NULL ,YEA);
        if(spbuf[0]=='1' || spbuf[0]=='2' || spbuf[0]=='3')
                break;
        else
                continue;
     }
     currentuser.notemode=spbuf[0]-'0';
     if(currentuser.notemode==2)
     {
        currentuser.notedate=time(NULL);
        currentuser.noteline=0;     
     }
     set_safe_record();
     substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
     return;
  }
}  
*/

void
notepad_init()
{
    FILE *check;
    char notetitle[STRLEN];
    char tmp[STRLEN*2];
    char *fname,*bname,*ntitle;
    long int  maxsec;
    time_t now;
    
    maxsec=24*60*60;
    lastnote=0;
    if( (check = fopen( "etc/checknotepad", "r" )) != NULL )
    {
        fgets(tmp,sizeof(tmp),check);
        lastnote=atol(tmp);
        fclose(check);
    }
    else
        lastnote=0;
        if(lastnote==0)
        {
                lastnote=time(NULL)-(time(NULL)%maxsec);
                check=fopen( "etc/checknotepad", "w" );
                fprintf(check,"%d",lastnote);
                fclose(check);
                sprintf(tmp,"ÁôÑÔ°åÔÚ %s Login ¿ªÆô£¬ÄÚ¶¨¿ªÆôÊ±¼äÊ±¼äÎª %s"
                ,currentuser.userid,Ctime(&lastnote));
                report(tmp);
        }
        if((time(NULL)-lastnote)>=maxsec)
        {
                move(t_lines-1,0);
                prints("¶Ô²»Æð£¬ÏµÍ³×Ô¶¯·¢ÐÅ£¬ÇëÉÔºò.....");
                refresh();
                now=time(0);
                check=fopen( "etc/checknotepad", "w" );
                lastnote=time(NULL)-(time(NULL)%maxsec);
                fprintf(check,"%d",lastnote);
                fclose(check);
                if((check=fopen("etc/autopost","r"))!=NULL)
                {
                        while(fgets(tmp,STRLEN,check)!=NULL)
                        {
                           fname=strtok(tmp," \n\t:@");
                           bname=strtok(NULL," \n\t:@");
                           ntitle=strtok(NULL," \n\t:@");
                           if(fname==NULL||bname==NULL||ntitle==NULL)
                              continue;
                           else
                           {
                              sprintf(notetitle,"[%.10s] %s",ctime(&now),ntitle);
                              if(dashf(fname))
                              {
                                 postfile(fname,bname,notetitle,1);
                                 sprintf(tmp,"%s ×Ô¶¯ÕÅÌù",ntitle);
                                 report(tmp);
                              }
                           }
                        }
                        fclose(check);
                }
                sprintf(notetitle,"[%.10s] ÁôÑÔ°å¼ÇÂ¼",ctime(&now));
                if(dashf("etc/notepad","r"))
                {
                        postfile("etc/notepad","notepad",notetitle,1);
                        unlink("etc/notepad");
                }
                report("×Ô¶¯·¢ÐÅÊ±¼ä¸ü¸Ä");
        }
    return;
}


void
user_login()
{
    char        fname[ STRLEN ];
    char        ans[5], *ruser;
    unsigned unLevel=PERM_SUICIDE;

    if( strcmp( currentuser.userid, "SYSOP" ) == 0 ){
        currentuser.userlevel &= (~0);   /* SYSOP gets all permission bits */
	} /* ?????ºóÃæ»¹ÓÐcheck_register_info */
 
    ruser = getenv( "REMOTEUSERNAME" );
    sprintf( genbuf, "%s@%s", ruser ? ruser : "?", fromhost );
    log_usies( "ENTER", genbuf );
    if( ruser ) {
        sprintf( genbuf, "%s@%s", ruser, fromhost );
        if( valid_ident( genbuf ) ) {
            strncpy( currentuser.ident, genbuf, NAMELEN );
        }
        if( !valid_ident( currentuser.ident ) ) {
            currentuser.ident[0] = '\0';
        }
    }
    u_enter() ;
    sprintf(genbuf, "Enter from %-16s", fromhost); /* Leeward: 97.12.02 */
    if (!strcmp(fromhost,"127.0.0.1")) {
        system("netstat -na|grep 127.0.0.1>/home0/bbs/KCN/see-it!");
    }

    report(genbuf) ;
/*---	period	2000-10-19	4 debug	---*/
    sprintf( genbuf, "[%d %d]", utmpent, usernum);
    log_usies( "ALLOC", genbuf );
/*---	---*/
    started = 1 ;
    if( USE_NOTEPAD == 1)
            notepad_init();
    if(strcmp(currentuser.userid,"guest")!=0 && USE_NOTEPAD == 1){
      if(DEFINE(DEF_NOTEPAD))
      { 
        int noteln;
        if(lastnote>currentuser.notedate)
                currentuser.noteline=0;
        noteln=countln("etc/notepad");
        if(lastnote>currentuser.notedate||currentuser.noteline==0)
        {
                shownotepad();
                currentuser.noteline=noteln;
                write_defnotepad();
        }
        else if((noteln-currentuser.noteline)>0){
                clear();
                ansimore2("etc/notepad",NA,0,noteln-currentuser.noteline+1);
                prints("[31m¡Ñ©Ø¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª©Ø¡Ñ[m\n");
                igetkey();
                currentuser.noteline=noteln;
                write_defnotepad();     
                clear();
        }
      }
    }
    /* Leeward 98.09.24 Use SHARE MEM to diaplay statistic data below */
    if (DEFINE(DEF_SHOWSTATISTIC))
    {
    if(show_statshm("0Announce/bbslists/countlogins",0))
    {
        move(0,0); clrtoeol(); /* Very strange if no these 2 commands //shake */
        refresh();
        pressanykey();
    }      
    }
    if(vote_flag(NULL,'\0',2/*¼ì²é¶Á¹ýÐÂµÄWelcome Ã»*/)==0)
    {
        if(dashf( "Welcome" ))
        {
                clear();
                ansimore("Welcome",YEA);
                vote_flag(NULL,'R',2/*Ð´Èë¶Á¹ýÐÂµÄWelcome*/);
        }
    }
    clear();
    if(DEFINE(DEF_SHOWHOT))
    { /* Leeward 98.09.24 Use SHARE MEM and disable old code */
      if (DEFINE(DEF_SHOWSTATISTIC)) {
        show_statshm("etc/posts/day",1);
        refresh(); 
      } else ansimore("etc/posts/day",NA); /* Leeward: disable old code */
    }
    move( t_lines - 2/*1*/, 0 ); /* Leeward: 98.09.24 Alter below message */
    clrtoeol();
    prints( "[1;36m¡î ÕâÊÇÄúµÚ [33m%d[36m ´ÎÉÏÕ¾£¬ÉÏ´ÎÄúÊÇ´Ó [33m%s[36m Á¬Íù±¾Õ¾¡£\n", currentuser.numlogins + 1, currentuser.lasthost );
    prints( "¡î ÉÏ´ÎÁ¬ÏßÊ±¼äÎª [33m%s[m ", Ctime(&currentuser.lastlogin) );
    igetkey();
    ansimore("0Announce/hotinfo",NA);
    move( t_lines - 1/*1*/, 0 ); /* Leeward: 98.09.24 Alter below message */
    clrtoeol();
    prints("[1;36m¡î °´ÈÎÒâ¼ü¼ÌÐø...[33m[m ");
    igetkey();
    move( t_lines - 1, 0 );
    setuserfile( fname, BADLOGINFILE );
    if( ansimore( fname, NA ) != -1 ) {
        getdata( t_lines-1, 0, "ÄúÒªÉ¾³ýÒÔÉÏÃÜÂëÊäÈë´íÎóµÄ¼ÇÂ¼Âð (Y/N)? [Y] ",
        ans, 4, DOECHO, NULL ,YEA);
        if( *ans != 'N' && *ans != 'n' )
            unlink( fname );
    }

    strncpy(currentuser.lasthost, fromhost, 16);
    currentuser.lasthost[15] = '\0';   /* dumb mistake on my part */
        currentuser.lastlogin = time(NULL) ;
    set_safe_record();
    currentuser.numlogins++;

    /* Leeward 98.06.20 adds below 3 lines */
    if ((int)currentuser.numlogins < 1) currentuser.numlogins = 1;
    if ((int)currentuser.numposts < 0) currentuser.numposts = 0;
    if ((int)currentuser.stay < 0) currentuser.stay = 1;
    currentuser.userlevel&=(~unLevel); /* »Ö¸´×ÔÉ±±êÖ¾ Luzi 98.10.10 */

    substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
    if (currentuser.firstlogin == 0) {
        currentuser.firstlogin = login_start_time - 7 * 86400;
    }
    check_register_info();
}

void
set_numofsig()
{
    int sigln;
    char signame[STRLEN];

    setuserfile( signame, "signatures" );
    sigln=countln(signame);
    numofsig=sigln/6;
    if((sigln%6)!=0)
         numofsig+=1;
}
int
chk_friend_book()
{
        FILE *fp;
        int idnum,n=0;
        char buf[STRLEN],*ptr;

        move(3,0);
        if((fp=fopen("friendbook","r"))==NULL)
                return n;
        prints("[1mÏµÍ³Ñ°ÈËÃû²áÁÐ±í:[m\n\n");
        /*if((fp=fopen("friendbook","r"))==NULL)
                return n; Moved before "prints", Leeward 98.12.03 */
        while(fgets(buf,sizeof(buf),fp)!=NULL)
        {
                char uid[14];
                char msg[STRLEN];
                struct user_info *uin;

                ptr=strstr(buf,"@");
                if(ptr==NULL)
                   continue;
                ptr++;
                strcpy(uid,ptr);
                ptr=strstr(uid,"\n");
                *ptr='\0';
                idnum=atoi(buf);
                if(idnum!=usernum||idnum<=0)
                   continue;    
                uin=t_search(uid,NA);
                sprintf(msg,"%s ÒÑ¾­ÉÏÕ¾¡£",currentuser.userid);
                /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong*/
                strcpy(MsgDesUid, uin?uin->userid:"");
                if (strcmp(uid,"Luzi")==0) {
                    if (uin!=NULL) do_sendmsg(uin,msg,2);
		}
                else {
		  idnum=0;/*Haohmaru.99.5.29.ÐÞÕýÒ»¸öbug,ÃâµÃÓÐÈËÀûÓÃÕâ¸öÀ´É§ÈÅ±ðÈË*/
                  if (uin!=NULL && canbemsged(uin))
			idnum=do_sendmsg(uin,msg,2);
		  if (idnum)
                    prints("%s ÕÒÄã£¬ÏµÍ³ÒÑ¾­¸æËßËû(Ëý)ÄãÉÏÕ¾µÄÏûÏ¢¡£\n",uid);
                  else
                    prints("%s ÕÒÄã£¬ÏµÍ³ÎÞ·¨ÁªÂçµ½Ëû(Ëý)£¬ÇëÄã¸úËû(Ëý)ÁªÂç¡£\n",uid);
                  del_from_file("friendbook",buf);
                }
                n++;
                if (n>15) { /* Leeward 98.12.03 */
                   pressanykey();
                   move (5,0);
                   clrtobot();
                }
        }
        fclose(fp);
        return n;
}

void
main_bbs(char *originhost, int convit,char* argv)
{
    extern char currmaildir[ STRLEN ];
    char   notename[STRLEN];
        int     currmail;

/* Add by KCN for avoid free_mem core dump */
     topfriend = NULL;
     big_picture=NULL;
     user_data = NULL;
     resolve_ucache();
     /* commented by period for it changed to local variable 2000.11.12
     pnt = NULL; */
    
    dup2(0,1);
#ifdef BBS_INFOD
    if (strstr( argv[ 0 ], "bbsinfo") != NULL) {
        load_sysconf();
        bbsinfod_main(argc, argv);
        exit( 0 );
    }
#endif
    initscr() ;

    load_sysconf();
#if 0
    if( argc < 2 || ((*argv[1] != 'h' )&& ( *argv[1] != 'd') && ( *argv[1] != 'e') )) {
	/* KCN add 'd' mode for bbsd 1999.9.1 */
        prints( "You cannot execute this program directly.\n" );
	oflush();
        exit( -1 );
    }
    if (*argv[1] == 'e') convcode=1;
#endif
	convcode = convit;
    conv_init();  /* KCN,99.09.05 */

    system_init( originhost );
    if( setjmp(byebye) ) {
        system_abort();
    }
    get_tty();
    init_tty();
    login_query();
    user_login();
    m_init();
    clear(); 

#ifndef DEBUG
    sprintf(argv,"bbsd:%s",currentuser.userid);
#endif

#ifdef TALK_LOG
    tlog_recover();     /* 2000.9.15 Bigman Ìí¼ÓÖÐ¶ÏtalkµÄ»Ö¸´ */
#endif
       
    currmail=get_num_records( currmaildir, sizeof(struct fileheader) );
    if( (currmail > MAIL_LIMIT) && !HAS_PERM(PERM_BOARDS) &&  !HAS_PERM(PERM_SYSOP)) /* Leeward 98.05.20 */
                prints("ÄãµÄÐÅ¼þ¸ß´ï %d ·â, ÇëÉ¾³ý¹ýÆÚÐÅ¼þ, Î¬³ÖÔÚ %d ·âÒÔÏÂ£¬·ñÔò½«²»ÄÜ·¢ÐÅ\n",currmail,MAIL_LIMIT);

    if (HAS_PERM(PERM_SYSOP)&&dashf("new_register"))
           prints("ÓÐÐÂÊ¹ÓÃÕßÕýÔÚµÈÄúÍ¨¹ý×¢²á×ÊÁÏ¡£\n");

    /*chk_friend_book();*/
    /* Leeward 98.12.03 */
    if (chk_friend_book()) {
/*    if(!uinfo.invisible)
            apply_ulist(friend_login_wall);
*/    pressreturn();
    }
    clear();
    memset(netty_path,0,sizeof(netty_path));
    nettyNN=NNread_init();
    set_numofsig();
    if(DEFINE(DEF_INNOTE))
    {
        setuserfile(notename,"notes");
        if(dashf(notename))
                ansimore(notename,YEA);
    }
    b_closepolls();
    num_alcounter();
    if(count_friends>0&&DEFINE(DEF_LOGFRIEND))
        t_friends();
    while( 1 ) {
        if(DEFINE(DEF_NORMALSCR))
                domenu( "TOPMENU" );
        else
                domenu( "TOPMENU2" );
        Goodbye();
    }
}

int refscreen = NA ;

int
egetch()
{
    int rval ;

    check_calltime();
    if (talkrequest) {
        talkreply() ;
        refscreen = YEA ;
        return -1 ;
    }
/*    if (ntalkrequest) {
        ntalkreply() ;
        refscreen = YEA ;
        return -1 ;
    }*/
    while( 1 ) {
        rval = igetkey();
        if(talkrequest) {
            talkreply() ;
            refscreen = YEA ;
            return -1 ;
        }/*
        if(ntalkrequest) {
            ntalkreply() ;
            refscreen = YEA ;
            return -1 ;
        }*/
        if( rval != Ctrl('L') )
            break;
        redoscr();
    }
    refscreen = NA ;
    return rval ;
}

char *
boardmargin()
{
    static char buf[STRLEN] ;
    if(selboard)
        sprintf(buf,"ÌÖÂÛÇø [%s]",currboard) ;
    else {
        brc_initial( DEFAULTBOARD );
        if (getbnum(currboard)) {
            selboard = 1 ;
            sprintf(buf,"ÌÖÂÛÇø [%s]",currboard) ;
        }
        else sprintf(buf,"Ä¿Ç°²¢Ã»ÓÐÉè¶¨ÌÖÂÛÇø") ;
    }
    return buf ;
}
/*Add by SmallPig*/
void
update_endline()
{   
    char        buf[ STRLEN ];
    char        stitle[256];
    time_t      now;
    int         allstay;
    int         colour;

    if(DEFINE(DEF_TITLECOLOR))
    {
        colour=4;
    }else
    {
            colour=currentuser.numlogins%4+3;
            if(colour==3)
                colour=1;
    }
    if(!DEFINE(DEF_ENDLINE))
    {
        move(t_lines-1,0);
        clrtoeol();
        return;
    }
    now=time(0);
    allstay=(now - login_start_time)/60;
    sprintf(buf,"[[36m%.12s[33m]",currentuser.userid);
    if(DEFINE(DEF_NOTMSGFRIEND))
    {
            sprintf(stitle,"[4%dm[33mÊ±¼ä[[36m%12.12s[33m] ºô½ÐÆ÷[ºÃÓÑ:%3s£ºÒ»°ã:%3s] Ê¹ÓÃÕß%-24s Í£Áô[%3d:%2d][m",colour,ctime(&now)+4,
                  (!(uinfo.pager&FRIEND_PAGER))?"NO ":"YES",(uinfo.pager&ALL_PAGER)?"YES":"NO ",buf,(allstay/60)%1000,allstay%60);
    }else
    {
            num_alcounter();
            sprintf(stitle,"[4%dm[33mÊ±¼ä[[36m%12.12s[33m] ×ÜÈËÊý/ºÃÓÑ[%3d/%3d][%c£º%c] Ê¹ÓÃÕß%-24s Í£Áô[%3d:%2d][m",colour,
                    ctime(&now)+4,count_users,count_friends,(uinfo.pager&ALL_PAGER)?'Y':'N',(!(uinfo.pager&FRIEND_PAGER))?'N':'Y',buf,(allstay/60)%1000,allstay%60);
    }
    move(t_lines-1,0);
    clrtoeol();
    prints("%s",stitle);

    /* Leeward 98.09.30 show hint for rookies */
    /* PERMs should coincide with ~bbsroot/etc/sysconf.ini: PERM_ADMENU */
    if ( !DEFINE(DEF_NORMALSCR)  && MMENU == uinfo.mode 
      && !HAS_PERM(PERM_ACCOUNTS) && !HAS_PERM(PERM_OVOTE) 
      && !HAS_PERM(PERM_SYSOP)    && !HAS_PERM(PERM_OBOARDS) 
      && !HAS_PERM(PERM_WELCOME) && !HAS_PERM(PERM_ANNOUNCE) )
    {
      move(t_lines - 2, 0);
      clrtoeol();
      prints("[1m[32mÕâÊÇ¾«¼òÄ£Ê½Ö÷Ñ¡µ¥¡£ÒªÊ¹ÓÃÒ»°ãÄ£Ê½£¬ÇëÉè¶¨¸öÈË²ÎÊýµÚ£ÌÏîÎª£Ï£Î²¢Õý³£ÀëÕ¾ÔÙ½øÕ¾¡£[m");
    } 
}


/*ReWrite by SmallPig*/
void
showtitle( title, mid )
char    *title, *mid;
{
    char        buf[ STRLEN ], *note;
    char        stitle[256];
    int         spc1,spc2;
    int         colour;

    if(DEFINE(DEF_TITLECOLOR))
    {
        colour=4;
    }else
    {
            colour=currentuser.numlogins%4+3;
            if(colour==3)
                colour=1;
    }
    note = boardmargin();
    spc1 = 39-strlen(title)-strlen(mid)/2 ;
    spc2 = 40-strlen(note)-strlen(mid)/2 ;
/* Modified by Leeward 97/11/23 -- modification starts */
/* If title is too long (BM names too long), spc1 < 2 (even < 0)
   Then we should decrease spc2 to avoid the line length exceed default(80)

   Sulution: "spc2 -= 2 - spc1"
   Attention: "spc1 = 2" should AFTER "spc2 -= 2 - spc1" !!!
*/
    /*if( spc1 < 2 )  spc1 = 2;
    if( spc2 < 2 )  spc2 = 2;*/
    if( spc2 < 2 )  spc2 = 2;
    if( spc1 < 2 )
    {
      spc2 -= 2 - spc1;
      spc1 = 2;
      if( spc2 < 2 )  spc2 = 2;
    }
/* Modified by Leeward 97/11/23 -- modification stops */          
    move(0,0);
    clrtoeol();
    sprintf( buf, "%*s", spc1, "" );
    if(!strcmp(mid,BoardName))
    {
        sprintf(stitle, "[4%dm[33m%s%s[37m%s[4%dm",colour, title, buf,mid,colour);
        prints("%s",stitle);
    }
    else if(mid[0]=='[')
    {
        sprintf( stitle,"[4%dm[33m%s%s[37m[5m%s[m[4%dm",colour, title, buf,mid,colour);
        prints("%s",stitle);
    }
    else 
    {
        sprintf( stitle,"[4%dm[33m%s%s[36m%s",colour, title, buf,mid);
        prints("%s",stitle);
    }
    sprintf( buf, "%*s", spc2, "" );
    prints( "%s[33m%s[m\n", buf, note );
    update_endline();
    move(1,0);
}


void
docmdtitle( title, prompt )
char    *title, *prompt;
{
/*    char   middoc[30],bmstr[BM_LEN -1 ];
    struct shortfile    *bp; */
        char middoc[30];
    struct shortfile    *getbcache();
    int chkmailflag=0;

/*    if (getbnum(DEFAULTBOARD)) 
    {
        bp = getbcache( DEFAULTBOARD );
        memcpy( bmstr, bp->BM, BM_LEN-1 );
    }else
         strcpy(bmstr," ");
*/
        chkmailflag=chkmail();

        if(chkmailflag==2)/*Haohmaru.99.4.4.¶ÔÊÕÐÅÒ²¼ÓÏÞÖÆ*/
                strcpy(middoc,"[ÄúµÄÐÅÏä³¬¹ýÈÝÁ¿,²»ÄÜÔÙÊÕÐÅ!]");
    else if(chkmailflag)
        strcpy(middoc,"[ÄúÓÐÐÅ¼þ]");
/*    else if ( vote_flag( DEFAULTBOARD, '\0' ,0) == 0&&(bp->flag&VOTE_FLAG))
        strcpy(middoc,"[ÏµÍ³Í¶Æ±ÖÐ]");*/
    else
        strcpy(middoc,BoardName);
        

    showtitle( title, middoc);
    move(1,0);
    clrtoeol() ;
    prints( "%s", prompt );
    clrtoeol();
}

/* 2000.9.15 Bigman »Ö¸´ÁÄÌì¼ÇÂ¼ */
#ifdef TALK_LOG

int
tlog_recover()
{
    char    buf[256];
    sprintf(buf, "home/%c/%s/talklog", toupper(currentuser.userid[0]), currentuser.userid);

    if (strcasecmp(currentuser.userid, "guest") == 0 || !dashf(buf))
        return;

    clear();
    strcpy(genbuf, "");

    getdata(0, 0, "\033[1;32mÄúÓÐÒ»¸ö²»Õý³£¶ÏÏßËùÁôÏÂÀ´µÄÁÄÌì¼ÇÂ¼, ÄúÒª .. (M) ¼Ä»ØÐÅÏä (Q) ËãÁË£¿[Q]£º\033[m", genbuf,4,DOECHO,NULL,YEA);
 
    if (genbuf[0] == 'M' || genbuf[0] == 'm')
        mail_file(buf, currentuser.userid, "ÁÄÌì¼ÇÂ¼");

    unlink(buf);
    return;

}
#endif
