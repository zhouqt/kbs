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

#include "bbs.h"

#define BADLOGINFILE    "logins.bad"
#define INPUT_ACTIVE 0
/*Haohmaru.98.11.3*/
#define INPUT_IDLE 1
#define WAITTIME  150

/* KCN add 1999.11.07 */
/*#undef LOGINASNEW */

extern struct screenline *big_picture;
extern struct userec *user_data;
/* extern char* pnt; */

int	temp_numposts;/*Haohmaru.99.4.02.»√∞Æπ‡ÀÆµƒ»Àøﬁ»•∞…//grin*/
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

/* char netty_path[ 60 ]; FAINT!!! ‘ı√¥ª·≤ª≥ˆ¥Ìƒÿ!!! Leeward: 1997.12.10 */
char netty_path[ 256 ];
char BoardName[STRLEN] ;
int utmpent = -1 ;
time_t  login_start_time;
int     showansi=1;

static int i_domode = INPUT_ACTIVE;

extern char MsgDesUid[14]; /* ±£¥ÊÀ˘∑¢msgµƒƒøµƒuid 1998.7.5 by dong */
int
canbemsged(uin)/*Haohmaru.99.5.29*/
struct user_info *uin;
{
    if (uinfo.pager&ALLMSG_PAGER) return YEA;
    if (uinfo.pager&FRIENDMSG_PAGER)
    {
        if(can_override(currentuser->userid,uin->userid))
                return YEA;
    }
    return NA;
}

void
wait_alarm_clock()/*Haohmaru.98.11.3*/
{
    if(i_domode == INPUT_IDLE) {
        clear();
        exit(0);
    }
    i_domode = INPUT_IDLE ;
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
u_enter()
{

    enter_uflags = currentuser->flags[0];
    memset( &uinfo, 0, sizeof( uinfo ) );
    uinfo.active = YEA ;
    uinfo.pid    = getpid();
/*    if( HAS_PERM(currentuser,PERM_LOGINCLOAK) && (currentuser->flags[0] & CLOAK_FLAG) && HAS_PERM(currentuser,PERM_SEECLOAK)) */

    /* Bigman 2000.8.29 ÷«ƒ“Õ≈ƒ‹πª“˛…Ì */
    if( (HAS_PERM(currentuser,PERM_CHATCLOAK) || HAS_PERM(currentuser,PERM_CLOAK)) && (currentuser->flags[0] & CLOAK_FLAG))
        uinfo.invisible = YEA;
    uinfo.mode = LOGIN ;
    uinfo.pager = 0;
/*    uinfo.pager = !(currentuser->flags[0] & PAGER_FLAG);*/
    if(DEFINE(currentuser,DEF_FRIENDCALL))
    {
        uinfo.pager|=FRIEND_PAGER;
    }
    if(currentuser->flags[0] & PAGER_FLAG)
    {
        uinfo.pager|=ALL_PAGER;
        uinfo.pager|=FRIEND_PAGER;
    }
    if(DEFINE(currentuser,DEF_FRIENDMSG))
    {
        uinfo.pager|=FRIENDMSG_PAGER;
    }
    if(DEFINE(currentuser,DEF_ALLMSG))
    {
        uinfo.pager|=ALLMSG_PAGER;
        uinfo.pager|=FRIENDMSG_PAGER;
    }
    uinfo.uid = usernum;
    strncpy( uinfo.from, fromhost, IPLEN );
#ifdef SHOW_IDLE_TIME
    uinfo.freshtime=time(0);
#endif
    iscolor=(DEFINE(currentuser,DEF_COLOR))? 1:0;
    strncpy( uinfo.userid,   currentuser->userid,   20 );
    strncpy( uinfo.realname, currentuser->realname, 20 );
    strncpy( uinfo.username, currentuser->username, 40 );
    nf=0;
    topfriend=NULL;
    getfriendstr();
    utmpent = getnewutmpent(&uinfo) ;
    if (utmpent == -1)
    {
        prints("»À ˝“—¬˙,Œﬁ∑®∑÷≈‰”√ªßÃıƒø!\n");
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
    if (((currentuser->flags[0] & mask) && 1) != value) {
        if (value) currentuser->flags[0] |= mask;
        else currentuser->flags[0] &= ~mask;
    }
}
/*---	moved to here from below	period	2000-11-19	---*/
int started = 0;
void
u_exit()
{
/*---	According to ylsdd's article, deal with SUPER_CLOAK problem	---*
 *---   Added by period		2000-09-19				---*/
/* ’‚–©–≈∫≈µƒ¥¶¿Ì“™πÿµÙ, ∑Ò‘Ú‘⁄¿Îœﬂ ±µ»∫Úªÿ≥µ ±≥ˆœ÷–≈∫≈ª·µº÷¬÷ÿ–¥√˚µ•,
 * ’‚∏ˆµº÷¬µƒ√˚µ•ªÏ¬“±»kick user∏¸∂‡ */
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
/*    if (HAS_PERM(currentuser,PERM_LOGINCLOAK)&&HAS_PERM(currentuser,PERM_SEECLOAK))*/

   /* Bigman 2000.8.29 ÷«ƒ“Õ≈ƒ‹πª“˛…Ì */
	if((HAS_PERM(currentuser,PERM_CHATCLOAK) || HAS_PERM(currentuser,PERM_CLOAK)))
        setflags(CLOAK_FLAG, uinfo.invisible);

    brc_update(currentuser->userid);

    clear_utmp(0);
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

    if( (id = getuser( userid,&currentuser )) != 0 ) 
        return usernum = id;
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
	    bbslog("1system", "AXXED Stay: %3ld (%s)[%d %d]", stay/60, currentuser->username, utmpent, usernum);
        u_exit();
    }
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
count_user()
{
    return apply_utmpuid( NULL , usernum,0);
}

/* to be Continue to fix kick problem */
void
multi_user_check()
{
    struct user_info    uin;
    int         curr_login_num;
    char        buffer[40];

    if (count_user()<1) RemoveMsgCountFile(currentuser->userid);

    if (HAS_PERM(currentuser,PERM_MULTILOG)) 
        return;  /* don't check sysops */
    curr_login_num = get_utmp_number();
    /* Leeward: 97.12.22 BMs may open 2 windows at any time */
    /* Bigman: 2000.8.17 ÷«ƒ“Õ≈ƒ‹πªø™2∏ˆ¥∞ø⁄ */
    /* stephen: 2001.10.30 ÷Ÿ≤√ø…“‘ø™¡Ω∏ˆ¥∞ø⁄ */
    if ((HAS_PERM(currentuser,PERM_BOARDS) || HAS_PERM(currentuser,PERM_CHATOP)|| HAS_PERM(currentuser,PERM_JURY) || HAS_PERM(currentuser,PERM_CHATCLOAK)) && count_user() < 2)
        return;
    /* allow multiple guest user */
    if (!strcmp("guest", currentuser->userid)) {
        if ( count_user() > MAX_GUEST_NUM ) {
            prints( "[33m±ß«∏, ƒø«∞“—”–Ã´∂‡ [36mguest, «Î…‘∫Ú‘Ÿ ‘°£[m\n");
            pressreturn();
            oflush();
            exit(1);
        }
        return;
    }
    else if ( (curr_login_num<700)&&(count_user()>=2) 
           || (curr_login_num>=700)&& (count_user()>=1) ) /*user login limit*/
    {  
        getdata(0, 0, "ƒ„Õ¨ ±…œœﬂµƒ¥∞ø⁄ ˝π˝∂‡£¨ «∑ÒÃﬂ≥ˆ±æID∆‰À¸¥∞ø⁄(Y/N)? [N]", 
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

		return ; /* ≤ªºÃ–¯ºÏ≤È£¨∑µªÿ, ≤ªÃﬂ◊‘º∫¥∞ø⁄, added by dong, 1999.1.25 */
        } 
        oflush();
        exit(1);       /* ∂‡¥∞ø⁄ ±ÃﬂµÙ“ª∏ˆ£¨◊‘º∫“≤∂œœﬂ */
    }
    if ( !search_ulist( &uin, cmpuids2, usernum) )
        return;  /* user isn't logged in */
    if (!uin.active || (kill(uin.pid,0) == -1))
        return;  /* stale entry in utmp file */

    getdata(0, 0, "ƒ˙œÎ…æ≥˝÷ÿ∏¥µƒ login ¬ (Y/N)? [N]", genbuf, 4, 
            DOECHO, NULL,YEA);

    if(genbuf[0] == 'Y' || genbuf[0] == 'y') {
       RemoveMsgCountFile(currentuser->userid);
       kill(uin.pid,9);
        sprintf(buffer, "kicked (multi-login)" );
        report(buffer);
    }
    else if ( (curr_login_num<700)&&(count_user()>=2)
              || (curr_login_num>=700)&& (count_user()>=1) )
        {
           oflush();
                exit(1);
        }          /* ‘Ÿ≈–∂œ“ª¥Œ»À ˝ Luzi 99.1.23 */


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
system_init()
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

    signal(SIGHUP,abort_bbs) ;
    signal(SIGPIPE,abort_bbs) ;
    signal(SIGTERM,abort_bbs) ;
    signal(SIGQUIT,abort_bbs) ;
    signal(SIGINT,SIG_IGN) ;
	signal(SIGALRM,SIG_IGN);
    signal(SIGURG,SIG_IGN) ;
    signal(SIGTSTP,SIG_IGN) ;
    signal(SIGTTIN,SIG_IGN) ;
    signal(SIGUSR1,talk_request) ;
    signal(SIGUSR2,r_msg) ;
}

void
system_abort()
{
    if( started ) {
        bbslog( "1ABORT", currentuser->username );
        u_exit() ;
    }
    clear();
    refresh();
    prints("–ª–ªπ‚¡Ÿ, º«µ√≥£¿¥‡∏ !\n");
    oflush();
    abort_bbs();
    return;
}

void
logattempt( uid, frm )
char *uid, *frm;
{
    char        fname[ STRLEN ];
    int         fd, len;

    sprintf( genbuf, "%-12.12s  %-30s %s\n",
                uid, Ctime( login_start_time ), frm );
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
    char        uid[STRLEN], passbuf[40], *ptr;
    int         curr_login_num;
    int 	curr_http_num; /* Leeward 99.03.06 */
    int         attempts;
    char fname[STRLEN], tmpstr[30];
    FILE *fn;   
    char buf[256];
    curr_login_num = get_utmp_number();;
    if( curr_login_num >= MAXACTIVE ) {
        ansimore( "etc/loginfull", NA );
        oflush();
        Net_Sleep( 20 );
        exit( 1 ) ;
    }
    curr_http_num = 0;
/*disable by KCN     curr_http_num = num_active_http_users(); *//* Leeward 99.03.06 */

    ptr = sysconf_str( "BBSNAME" );
    if( ptr == NULL )  ptr = "…–Œ¥√¸√˚≤‚ ‘’æ";
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

    prints( "\033[1mª∂”≠π‚¡Ÿ [31m%s[37m °Ù"ISSUE_LOGIN"°Ù [36mƒø«∞…œœﬂ»À ˝ \033[1m%d[m", BoardName, curr_login_num);
/*{
char ii[16];
sprintf(ii, "%.2f", (double)curr_login_num / (double)MAXACTIVE * 100.0);
    prints( "\033[1mª∂”≠π‚¡Ÿ [31m%s[37m °Ù±æ’æ π”√ Ôπ‚π´Àæ Ôπ‚ÃÏ—›∑˛ŒÒ∆˜°Ù [36mƒø«∞…œœﬂ»À ˝ \033[1m%s%%[m", BoardName, ii);
}*/

    if ((curr_http_num != -1) && (curr_http_num != 0)) /* dong 2000.4.18 */ 
      prints("[1m[36m+%d[m", curr_http_num); /* Leeward 99.03.06 */
#ifndef SSHBBS
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

#ifdef LOGINASNEW
        getdata( 0, 0, "\n\033[1m[37m«Î ‰»Î¥˙∫≈( ‘”√«Î ‰»Î `\033[36mguest\033[37m', ◊¢≤·«Î ‰»Î`\033[36mnew\033[37m'): [m",
#else
        getdata( 0, 0, "\n\033[1m[37m«Î ‰»Î¥˙∫≈( ‘”√«Î ‰»Î `\033[36mguest\033[37m', ‘›Õ£◊¢≤·–¬’ ∫≈): [m",
#endif
                uid, STRLEN-1, DOECHO, NULL ,YEA);
        if( strcmp( uid, "new" ) == 0 ) {
#ifdef LOGINASNEW
            if (check_ban_IP(fromhost,buf)<=0)
	    {
		new_register();
		sethomepath(tmpstr, currentuser->userid);
		sprintf( buf, "/bin/mv -f %s "BBSHOME"/homeback/%s", tmpstr,currentuser->userid);
		system( buf );
		setmailpath(tmpstr, currentuser->userid);/*Haohmaru.00.04.23,√‚µ√ƒ‹ø¥«∞»Àµƒ–≈*/
		sprintf( buf, "/bin/mv -f %s "BBSHOME"/mailback/%s", tmpstr,currentuser->userid);
		system( buf );
		break;
	    }
            prints( "±æœµÕ≥“ÚŒ™ %s µƒ‘≠“ÚΩ˚÷πƒ˙À˘‘⁄Õ¯∂Œ◊¢≤·–¬”√ªß\n",buf);
#else
            prints( "[37m±æœµÕ≥ƒø«∞Œﬁ∑®“‘ new ◊¢≤·, «Î”√ guest Ω¯»Î.[m\n" );
#endif
        } else if( *uid == '\0' || !dosearchuser( uid ) ) {
            prints( "[32m" MSG_ERR_USERID "[m\n" );
        } else 
/* Add by KCN for let sysop can use extra 10 UTMP */
        if(!HAS_PERM(currentuser,PERM_ADMINMENU)&&( curr_login_num >= MAXACTIVE+10 )) {
        	ansimore( "etc/loginfull", NA );
	        oflush();
	        sleep( 1 );
	        exit( 1 ) ;
        } else if( /*strcmp*/strcasecmp( uid, "guest" ) == 0 ) {
            currentuser->userlevel = 0;
            currentuser->flags[0] = CURSOR_FLAG | PAGER_FLAG;
            break;
        } else {
	        if (!convcode)
            	convcode=!(currentuser->userdefine&DEF_USEGB);  /* KCN,99.09.05 */
            	
            getdata( 0, 0, "\033[1m[37m«Î ‰»Î√‹¬Î: [m", passbuf, 39, NOECHO, NULL ,YEA);

	        if( !checkpasswd2(passbuf, currentuser ))
    	    {
                logattempt( currentuser->userid, fromhost );
                prints( "[32m√‹¬Î ‰»Î¥ÌŒÛ...[m\n" );
            } else {
                if( !HAS_PERM(currentuser, PERM_BASIC ) ) {
                   prints( "[32m±æ’ ∫≈“—Õ£ª˙°£«ÎœÚ [36mSYSOP[32m ≤È—Ø‘≠“Ú[m\n" );
                   oflush();
                   sleep( 1 );
                   exit( 1 );
                }
                if(id_invalid(uid))
                {
                    prints("[31m±ß«∏!![m\n");
                    prints("[32m±æ’ ∫≈ π”√÷–ŒƒŒ™¥˙∫≈£¨¥À’ ∫≈“—æ≠ ß–ß...[m\n");
                    prints("[32mœÎ±£¡Ù»Œ∫Œ«©√˚µµ«Î∏˙’æ≥§¡™¬Á £¨À˚(À˝)ª·Œ™ƒ„∑˛ŒÒ°£[m\n");
                    getdata( 0, 0, "∞¥ [RETURN] ºÃ–¯",genbuf,10,NOECHO,NULL,YEA);
                    oflush();
                    sleep( 1 );
                    exit( 1 );
                }
                if( simplepasswd( passbuf ) ) {
                    prints("[33m* √‹¬Îπ˝Ï∂ºÚµ•, «Î—°‘Ò“ª∏ˆ“‘…œµƒÃÿ ‚◊÷‘™.[m\n");
                    getdata( 0, 0, "∞¥ [RETURN] ºÃ–¯",genbuf,10,NOECHO,NULL,YEA);
                }
                /* passwd ok, covert to md5 --wwj 2001/5/7 */
                if(currentuser->passwd[0]){
                    bbslog("covert","for md5passwd");
                    setpasswd(passbuf,currentuser);
                }
                break;
            }
        }
    }
#else
   getdata( 0, 0, "\n∞¥ [RETURN] ºÃ–¯",genbuf,10,NOECHO,NULL,YEA);
#endif
#ifdef DEBUG
    if (!HAS_PERM(currentuser,PERM_SYSOP)) {
		prints("±æ∂Àø⁄Ωˆπ©≤‚ ‘”√£¨«Î¡¨Ω”±æ’æµƒ∆‰À˚ø™∑≈∂Àø⁄°£\n");
		oflush();
		Net_Sleep(3);
		system_abort();
    }
#endif
    multi_user_check();
    alarm(0);
    signal(SIGALRM, SIG_IGN);/*Haohmaru.98.11.12*/
    term_init();
    scrint = 1 ;
    sethomepath(tmpstr, currentuser->userid);
    sprintf(fname,"%s/%s.deadve", tmpstr, currentuser->userid);
    if((fn=fopen(fname,"r"))!=NULL)
    {
        mail_file(currentuser->userid,fname,currentuser->userid,"≤ª’˝≥£∂œœﬂÀ˘±£¡Ùµƒ≤ø∑›...",1);
            fclose(fn);
    }
    sethomepath( genbuf, currentuser->userid );
    mkdir( genbuf, 0755 );
    temp_numposts=0;/*Haohmaru.99.4.02.»√∞Æπ‡ÀÆµƒ»Àøﬁ»•∞…//grin*/
}

void
write_defnotepad()
{
  currentuser->notedate=time(NULL);
  return;
}

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
                sprintf(tmp,"¡Ù—‘∞Â‘⁄ %s Login ø™∆Ù£¨ƒ⁄∂®ø™∆Ù ±º‰ ±º‰Œ™ %s"
                ,currentuser->userid,Ctime(lastnote));
                report(tmp);
        }
        if((time(NULL)-lastnote)>=maxsec)
        {
                move(t_lines-1,0);
                prints("∂‘≤ª∆£¨œµÕ≥◊‘∂Ø∑¢–≈£¨«Î…‘∫Ú.....");
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
                                 sprintf(tmp,"%s ◊‘∂Ø’≈Ã˘",ntitle);
                                 report(tmp);
                              }
                           }
                        }
                        fclose(check);
                }
                sprintf(notetitle,"[%.10s] ¡Ù—‘∞Âº«¬º",ctime(&now));
                if(dashf("etc/notepad","r"))
                {
                        postfile("etc/notepad","notepad",notetitle,1);
                        unlink("etc/notepad");
                }
                report("◊‘∂Ø∑¢–≈ ±º‰∏¸∏ƒ");
        }
    return;
}


void
user_login()
{
    char        fname[ STRLEN ];
    char        ans[5], *ruser;
    unsigned unLevel=PERM_SUICIDE;

    if( strcmp( currentuser->userid, "SYSOP" ) == 0 ){
        currentuser->userlevel &= (~0);   /* SYSOP gets all permission bits */
	} /* ?????∫Û√Êªπ”–check_register_info */
 
    ruser = getenv( "REMOTEUSERNAME" );
    bbslog( "1system", "ENTER %s@%s", ruser ? ruser : "?", fromhost );
    if( ruser ) {
        sprintf( genbuf, "%s@%s", ruser, fromhost );
        if( valid_ident( genbuf ) ) {
            strncpy( currentuser->ident, genbuf, NAMELEN );
        }
        if( !valid_ident( currentuser->ident ) ) {
            currentuser->ident[0] = '\0';
        }
    }
    u_enter() ;
    sprintf(genbuf, "Enter from %-16s", fromhost); /* Leeward: 97.12.02 */

    report(genbuf) ;
/*---	period	2000-10-19	4 debug	---*/
    bbslog( "1system", "ALLOC: [%d %d]", utmpent, usernum);
/*---	---*/
    started = 1 ;
    if( USE_NOTEPAD == 1)
            notepad_init();
    if(strcmp(currentuser->userid,"guest")!=0 && USE_NOTEPAD == 1){
      if(DEFINE(currentuser,DEF_NOTEPAD))
      { 
        int noteln;
        if(lastnote>currentuser->notedate)
                currentuser->noteline=0;
        noteln=countln("etc/notepad");
        if(lastnote>currentuser->notedate||currentuser->noteline==0)
        {
                shownotepad();
                currentuser->noteline=noteln;
                write_defnotepad();
        }
        else if((noteln-currentuser->noteline)>0){
                clear();
                ansimore2("etc/notepad",NA,0,noteln-currentuser->noteline+1);
                prints("[31m°—©ÿ°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™©ÿ°—[m\n");
                igetkey();
                currentuser->noteline=noteln;
                write_defnotepad();     
                clear();
        }
      }
    }
    /* Leeward 98.09.24 Use SHARE MEM to diaplay statistic data below */
    if (DEFINE(currentuser,DEF_SHOWSTATISTIC))
    {
    if(show_statshm("0Announce/bbslists/countlogins",0))
    {
        move(0,0); clrtoeol(); /* Very strange if no these 2 commands //shake */
        refresh();
        pressanykey();
    }      
    }
    if(vote_flag(NULL,'\0',2/*ºÏ≤È∂¡π˝–¬µƒWelcome √ª*/)==0)
    {
        if(dashf( "Welcome" ))
        {
                clear();
                ansimore("Welcome",YEA);
                vote_flag(NULL,'R',2/*–¥»Î∂¡π˝–¬µƒWelcome*/);
        }
    }
    clear();
    if(DEFINE(currentuser,DEF_SHOWHOT))
    { /* Leeward 98.09.24 Use SHARE MEM and disable old code */
      if (DEFINE(currentuser,DEF_SHOWSTATISTIC)) {
        show_statshm("etc/posts/day",1);
        refresh(); 
      } else ansimore("etc/posts/day",NA); /* Leeward: disable old code */
    }
    move( t_lines - 2/*1*/, 0 ); /* Leeward: 98.09.24 Alter below message */
    clrtoeol();
    prints( "[1;36m°Ó ’‚ «ƒ˙µ⁄ [33m%d[36m ¥Œ…œ’æ£¨…œ¥Œƒ˙ «¥” [33m%s[36m ¡¨Õ˘±æ’æ°£\n", currentuser->numlogins + 1, currentuser->lasthost );
    prints( "°Ó …œ¥Œ¡¨œﬂ ±º‰Œ™ [33m%s[m ", Ctime(currentuser->lastlogin) );
    igetkey();
    /* »´π˙ Æ¥Û»»√≈ª∞Ã‚ added by Czz 020128 */
    show_help("0Announce/bbslists/newsday");
    /* added end */
    ansimore("0Announce/hotinfo",NA);
    move( t_lines - 1/*1*/, 0 ); /* Leeward: 98.09.24 Alter below message */
    clrtoeol();
    prints("[1;36m°Ó ∞¥»Œ“‚º¸ºÃ–¯...[33m[m ");
    igetkey();
    move( t_lines - 1, 0 );
    sethomefile( fname,currentuser->userid, BADLOGINFILE );
    if( ansimore( fname, NA ) != -1 ) {
        getdata( t_lines-1, 0, "ƒ˙“™…æ≥˝“‘…œ√‹¬Î ‰»Î¥ÌŒÛµƒº«¬º¬ (Y/N)? [Y] ",
        ans, 4, DOECHO, NULL ,YEA);
        if( *ans != 'N' && *ans != 'n' )
            unlink( fname );
    }

    strncpy(currentuser->lasthost, fromhost, IPLEN);
    currentuser->lasthost[15] = '\0';   /* dumb mistake on my part */
        currentuser->lastlogin = time(NULL) ;
    currentuser->numlogins++;

    /* Leeward 98.06.20 adds below 3 lines */
    if ((int)currentuser->numlogins < 1) currentuser->numlogins = 1;
    if ((int)currentuser->numposts < 0) currentuser->numposts = 0;
    if ((int)currentuser->stay < 0) currentuser->stay = 1;
    currentuser->userlevel&=(~unLevel); /* ª÷∏¥◊‘…±±Í÷æ Luzi 98.10.10 */

    if (currentuser->firstlogin == 0) {
        currentuser->firstlogin = login_start_time - 7 * 86400;
    }
    check_register_info();
}

void
set_numofsig()
{
    int sigln;
    char signame[STRLEN];

    sethomefile( signame,currentuser->userid, "signatures" );
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
        prints("[1mœµÕ≥—∞»À√˚≤·¡–±Ì:[m\n\n");
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
                sprintf(msg,"%s “—æ≠…œ’æ°£",currentuser->userid);
                /* ±£¥ÊÀ˘∑¢msgµƒƒøµƒuid 1998.7.5 by dong*/
                strcpy(MsgDesUid, uin?uin->userid:"");
                if (strcmp(uid,"Luzi")==0) {
                    if (uin!=NULL) do_sendmsg(uin,msg,2);
		}
                else {
		  idnum=0;/*Haohmaru.99.5.29.–ﬁ’˝“ª∏ˆbug,√‚µ√”–»À¿˚”√’‚∏ˆ¿¥…ß»≈±»À*/
                  if (uin!=NULL && canbemsged(uin))
			idnum=do_sendmsg(uin,msg,2);
		  if (idnum)
                    prints("%s ’“ƒ„£¨œµÕ≥“—æ≠∏ÊÀﬂÀ˚(À˝)ƒ„…œ’æµƒœ˚œ¢°£\n",uid);
                  else
                    prints("%s ’“ƒ„£¨œµÕ≥Œﬁ∑®¡™¬ÁµΩÀ˚(À˝)£¨«Îƒ„∏˙À˚(À˝)¡™¬Á°£\n",uid);
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
main_bbs(int convit,char* argv)
{
    extern char currmaildir[ STRLEN ];
    char   notename[STRLEN];
        int     currmail;

/* Add by KCN for avoid free_mem core dump */
     topfriend = NULL;
     big_picture=NULL;
     user_data = NULL;
    load_sysconf();
     resolve_ucache();
     resolve_utmp();
    resolve_boards();
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

    system_init();
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
    sprintf(argv,"bbsd:%s",currentuser->userid);
#endif

#ifdef TALK_LOG
    tlog_recover();     /* 2000.9.15 Bigman ÃÌº”÷–∂œtalkµƒª÷∏¥ */
#endif
       
    currmail=get_num_records( currmaildir, sizeof(struct fileheader) );
    if( (currmail > MAIL_LIMIT) && !HAS_PERM(currentuser,PERM_BOARDS) &&  !HAS_PERM(currentuser,PERM_SYSOP)) /* Leeward 98.05.20 */
                prints("ƒ„µƒ–≈º˛∏ﬂ¥Ô %d ∑‚, «Î…æ≥˝π˝∆⁄–≈º˛, Œ¨≥÷‘⁄ %d ∑‚“‘œ¬£¨∑Ò‘ÚΩ´≤ªƒ‹∑¢–≈\n",currmail,MAIL_LIMIT);

    if (HAS_PERM(currentuser,PERM_SYSOP)&&dashf("new_register"))
           prints("”––¬ π”√’ﬂ’˝‘⁄µ»ƒ˙Õ®π˝◊¢≤·◊ ¡œ°£\n");

    /*chk_friend_book();*/
    /* Leeward 98.12.03 */
    if (chk_friend_book()) {
	    pressreturn();
    }
    clear();
    memset(netty_path,0,sizeof(netty_path));
    nettyNN=NNread_init();
    set_numofsig();
    if(DEFINE(currentuser,DEF_INNOTE))
    {
        sethomefile(notename,currentuser->userid,"notes");
        if(dashf(notename))
                ansimore(notename,YEA);
    }
    b_closepolls();
    num_alcounter();
    if(count_friends>0&&DEFINE(currentuser,DEF_LOGFRIEND))
        t_friends();
    while( 1 ) {
        if(DEFINE(currentuser,DEF_NORMALSCR))
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
        sprintf(buf,"Ã÷¬€«¯ [%s]",currboard) ;
    else {
        brc_initial( currentuser->userid,DEFAULTBOARD );
        if (getbnum(currboard)) {
            selboard = 1 ;
            sprintf(buf,"Ã÷¬€«¯ [%s]",currboard) ;
        }
        else sprintf(buf,"ƒø«∞≤¢√ª”–…Ë∂®Ã÷¬€«¯") ;
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

    if(DEFINE(currentuser,DEF_TITLECOLOR))
    {
        colour=4;
    }else
    {
            colour=currentuser->numlogins%4+3;
            if(colour==3)
                colour=1;
    }
    if(!DEFINE(currentuser,DEF_ENDLINE))
    {
        move(t_lines-1,0);
        clrtoeol();
        return;
    }
    now=time(0);
    allstay=(now - login_start_time)/60;
    sprintf(buf,"[[36m%.12s[33m]",currentuser->userid);
    if(DEFINE(currentuser,DEF_NOTMSGFRIEND))
    {
            sprintf(stitle,"[4%dm[33m ±º‰[[36m%12.12s[33m] ∫ÙΩ–∆˜[∫√”—:%3s£∫“ª∞„:%3s]  π”√’ﬂ%-24s Õ£¡Ù[%3d:%2d][m",colour,ctime(&now)+4,
                  (!(uinfo.pager&FRIEND_PAGER))?"NO ":"YES",(uinfo.pager&ALL_PAGER)?"YES":"NO ",buf,(allstay/60)%1000,allstay%60);
    }else
    {
/*            num_alcounter();
            sprintf(stitle,"[4%dm[33m ±º‰[[36m%12.12s[33m] ◊‹»À ˝/∫√”—[%3d/%3d][%c£∫%c]  π”√’ﬂ%-24s Õ£¡Ù[%3d:%2d][m",colour,
                    ctime(&now)+4,count_users,count_friends,(uinfo.pager&ALL_PAGER)?'Y':'N',(!(uinfo.pager&FRIEND_PAGER))?'N':'Y',buf,(allstay/60)%1000,allstay%60);*/
            sprintf(stitle,"\x1b[4%dm\x1b[33m ±º‰[\x1b[36m%12.12s\x1b[33m] ◊‹»À ˝ [ %3d ] [%c£∫%c]  π”√’ﬂ%-24s Õ£¡Ù[%3d:%2d]\x1b[m",colour,
                    ctime(&now)+4,get_utmp_number(),(uinfo.pager&ALL_PAGER)?'Y':'N',(!(uinfo.pager&FRIEND_PAGER))?'N':'Y',buf,(allstay/60)%1000,allstay%60);
    }
    move(t_lines-1,0);
    clrtoeol();
    prints("%s",stitle);

    /* Leeward 98.09.30 show hint for rookies */
    /* PERMs should coincide with ~bbsroot/etc/sysconf.ini: PERM_ADMENU */
    if ( !DEFINE(currentuser,DEF_NORMALSCR)  && MMENU == uinfo.mode 
      && !HAS_PERM(currentuser,PERM_ACCOUNTS) && !HAS_PERM(currentuser,PERM_OVOTE) 
      && !HAS_PERM(currentuser,PERM_SYSOP)    && !HAS_PERM(currentuser,PERM_OBOARDS) 
      && !HAS_PERM(currentuser,PERM_WELCOME) && !HAS_PERM(currentuser,PERM_ANNOUNCE) )
    {
      move(t_lines - 2, 0);
      clrtoeol();
      prints("[1m[32m’‚ «æ´ºÚƒ£ Ω÷˜—°µ•°£“™ π”√“ª∞„ƒ£ Ω£¨«Î…Ë∂®∏ˆ»À≤Œ ˝µ⁄£ÃœÓŒ™£œ£Œ≤¢’˝≥£¿Î’æ‘ŸΩ¯’æ°£[m");
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

    if(DEFINE(currentuser,DEF_TITLECOLOR))
    {
        colour=4;
    }else
    {
            colour=currentuser->numlogins%4+3;
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
        char middoc[30];
    int chkmailflag=0;

/*    if (getbnum(DEFAULTBOARD)) 
    {
        bp = getbcache( DEFAULTBOARD );
        memcpy( bmstr, bp->BM, BM_LEN-1 );
    }else
         strcpy(bmstr," ");
*/
        chkmailflag=chkmail();

        if(chkmailflag==2)/*Haohmaru.99.4.4.∂‘ ’–≈“≤º”œﬁ÷∆*/
                strcpy(middoc,"[ƒ˙µƒ–≈œ‰≥¨π˝»›¡ø,≤ªƒ‹‘Ÿ ’–≈!]");
    else if(chkmailflag)
        strcpy(middoc,"[ƒ˙”––≈º˛]");
/*    else if ( vote_flag( DEFAULTBOARD, '\0' ,0) == 0&&(bp->flag&VOTE_FLAG))
        strcpy(middoc,"[œµÕ≥Õ∂∆±÷–]");*/
    else
        strcpy(middoc,BoardName);
        

    showtitle( title, middoc);
    move(1,0);
    clrtoeol() ;
    prints( "%s", prompt );
    clrtoeol();
}

/* 2000.9.15 Bigman ª÷∏¥¡ƒÃÏº«¬º */
#ifdef TALK_LOG

int
tlog_recover()
{
    char    buf[256];
    sprintf(buf, "home/%c/%s/talklog", toupper(currentuser->userid[0]), currentuser->userid);

    if (strcasecmp(currentuser->userid, "guest") == 0 || !dashf(buf))
        return;

    clear();
    strcpy(genbuf, "");

    getdata(0, 0, "\033[1;32mƒ˙”–“ª∏ˆ≤ª’˝≥£∂œœﬂÀ˘¡Ùœ¬¿¥µƒ¡ƒÃÏº«¬º, ƒ˙“™ .. (M) ºƒªÿ–≈œ‰ (Q) À„¡À£ø[Q]£∫\033[m", genbuf,4,DOECHO,NULL,YEA);
 
    if (genbuf[0] == 'M' || genbuf[0] == 'm')
        mail_file(currentuser->userid,buf, currentuser->userid, "¡ƒÃÏº«¬º",1);
	else
    	unlink(buf);
    return;

}
#endif
