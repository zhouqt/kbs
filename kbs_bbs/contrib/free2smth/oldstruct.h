/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/
/*
$Id$
*/

/* Note the protocol field is not inside an #ifdef FILES...
   this is a waste but allows you to add/remove UL/DL support without
   rebuilding the PASSWDS file (and it's only a lil ole int anyway).
*/
typedef unsigned char uschar;
struct userec {                  /* Structure used to hold information in */
        char            userid[IDLEN+2];   /* PASSFILE */
        time_t          firstlogin;
        char            lasthost[16];
        unsigned int    numlogins;
        unsigned int    numposts;
	unsigned int    nummedals; /* 奖章数 */
        unsigned int    money;   /* 存款 */
        unsigned int    bet;     /* 贷款 */
	time_t		dateforbet;
        char            flags[2];
#ifdef ENCPASSLEN
        char            passwd[ENCPASSLEN];
#else
        char            passwd[PASSLEN];
#endif
        char            username[NAMELEN];
        char            ident[NAMELEN];
        char            termtype[16];
        char            reginfo[STRLEN-16];
        unsigned int    userlevel;
        time_t          lastlogin;
        time_t          lastlogout;/* 最近离线时间 */
        time_t          stay;
        char            realname[NAMELEN];
        char            address[STRLEN];
        char            email[STRLEN-12];
        unsigned int    nummails;
        time_t          lastjustify;
        char            gender;
        unsigned char   birthyear;
        unsigned char   birthmonth;
        unsigned char   birthday;
        int             signature;
        unsigned int    userdefine;
        time_t          notedate;
        int             noteline;
		unsigned int	userdefine1;
};
 
struct user_info {               /* Structure used in UTMP file */
        int     active;          /* When allocated this field is true */
        int     uid;             /* Used to find user name in passwd file */
        int     pid;             /* kill() to notify user of talk request */
        int     invisible;       /* Used by cloaking function in Xyz menu */
        int     sockactive;      /* Used to coordinate talk requests */
        int     sockaddr;        /* ... */
        int     destuid;         /* talk uses this to identify who called */
        int     mode;            /* UL/DL, Talk Mode, Chat Mode, ... */
        int     pager;           /* pager toggle, YEA, or NA */
        int     in_chat;         /* for in_chat commands   */
        int     fnum;            /* number of friends */
        int     ext_idle;        /* has extended idle time, YEA or NA */
        char    chatid[ 10 ];    /* chat id, if in chat mode */
        char    from[ 60 ];      /* machine name the user called in from */
#ifndef BBSD
        char    tty[ 20 ];       /* tty port */
#else
	time_t	idle_time;			 /* to keep idle time */
#endif        
	int	utmpkey;
        char    userid[ 20 ];
        char    realname[ 20 ];
        char    username[NAMELEN];
	short	curboard;     //add by stiger
        unsigned short  friend[MAXFRIENDS];
        unsigned short  reject[MAXREJECTS];
};

//add by cyber & sando 2003.5.4

struct special_user {        
        char userid[20];      // 实现斑竹任期制
        char bname[20];
		time_t specialdate;
        int state;            //0~5为斑竹任期描述，6以后为其他特殊权限保留
};
//add end

struct override {
        char id[13];
        char exp[40];
};


#define BM_LEN 60

struct boardheader {             /* This structure is used to hold data in */
        char filename[STRLEN];   /* the BOARDS files */
        unsigned int nowid;
		char unused[16];
        char BM[ BM_LEN - 1];
        char flag;
        char title[STRLEN ];
        unsigned level;
        unsigned char accessed[ 12 ];
};

struct fileheader {             /* This structure is used to hold data in */
        char filename[STRLEN];     /* the DIR files */
        char owner[OWNERLEN];
		unsigned int id;
		unsigned int groupid;
		unsigned int reid;
		int attach;
		char unused[44];
        char title[STRLEN];
        unsigned level;
        unsigned char accessed[ 12 ];   /* struct size = 256 bytes */
} ;

struct shortfile {               /* used for caching files and boards */
        char filename[STRLEN];      /* also will do for mail directories */
		unsigned int nowid;
		char unused[16];
        char BM[ BM_LEN - 1];
        char flag;
        char title[STRLEN];
        unsigned level;
        unsigned char accessed;
		int inboard;      //各板在线人数, add by stiger
		int total;
		int lastpost;
};

struct one_key {                  /* Used to pass commands to the readmenu */
        int key ;
        int (*fptr)() ;
} ;


#define USHM_SIZE       (MAXACTIVE + 10)
struct UTMPFILE {
    struct user_info    uinfo[ USHM_SIZE ];
    time_t              uptime;
    unsigned short      usersum;
    int                 max_login_num;
    //long int		visit_total;
};

struct BCACHE {
    struct shortfile    bcache[ MAXBOARD ];
    int         number;
    time_t      uptime;
    time_t      pollvote;
    time_t		fresh_date;
    char		date[60];
    time_t		friendbook;		/* friendbook mtime */
};

struct UCACHE {
    char        userid[ MAXUSERS ][ IDLEN + 1 ];
    int         number;
    time_t      uptime;
	int next[MAXUSERS];
	int prev[MAXUSERS];
	int hash[26][26][256];
	struct userec passwd[MAXUSERS];
};

struct postheader
{
        char title[STRLEN];
        char ds[40];
        int reply_mode;
        char include_mode;
        int chk_anony;
        int postboard;
};


struct mail_group_index
{
	char fname[8];
	char desc[40];
	int num;
};

struct key_struct { // 自定义键 by bad
    int status[10];
    int key;
    int mapped[10];
};

struct helps{
	int modeid;
	char index[11];
	char desc[41];
	char *content;
};

struct msghead {
    int pos, len;
    char sent;
    char mode;
    char id[IDLEN + 2];
    time_t time;
    int frompid, topid;
};

#define MAXMAILLIST 20

struct mail_list {
	int num;
	char name[MAXMAILLIST][40];
	int mailnum[MAXMAILLIST];
};

#ifndef _TEMPLATE_H
#define _TEMPLATE_H

#define TEMPLATE_DIR ".templ"
#define MAX_TEMPLATE 10
#define MAX_CONTENT 20
#define TMPL_BM_FLAG 0x1
#define MAX_CONTENT_LENGTH 555
#define TMPL_NOW_VERSION 1

struct s_content {
    char text[50];
    int length;
};

struct s_template {
    char title[50];
    char title_prefix[20];
    int content_num;
    char filename[STRLEN];
    int flag;
    int version;
    char unused[16];
    char title_tmpl[STRLEN];
};

struct a_template {
    struct s_template *tmpl;
    struct s_content *cont;
};

#endif
