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

#ifndef  _BBS_H_
#define _BBS_H_

#ifndef BBSIRC


/* Global includes, needed in most every source file... */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#ifndef FREEBSD
#include <malloc.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mman.h>

/* System names.. by zixia.*/
#include BBSSITE_H

#include "config.h"             /* User-configurable stuff */
#include "permissions.h"

#define PUBLIC_SHMKEY	3700 
/*这个是唯一一个定义死的SHMKEY,因为sysconf_eval需要
public shm,而attach shm又需要sysconf_eval,ft*/
/* add by KCN ,disable perror */
#ifdef BBSMAIN
#define perror(x) prints(x)
#endif
 
#define VERSION_ID "Firebird BBS 2.5GB"
#define ADD_EDITMARK 1

#ifndef LOCK_EX
#define LOCK_EX         2       /* exclusive lock */
#define LOCK_UN         8       /* unlock */
#endif

#ifdef XINU
extern int errno ;
#endif

#define YEA (1)        /* Booleans  (Yep, for true and false) */
#define NA  (0) 

#define DOECHO (1)     /* Flags to getdata input function */
#define NOECHO (0)

#ifndef strdup
char *strdup() ;       /* External function declarations */
#endif
char *bfile() ;

extern FILE *ufp ;     /* External variable declarations */
extern long ti ;

#endif /* BBSIRC */

#define MAXFRIENDS (400)
#define NUMPERMS (30)

/*#define FILE_BUFSIZE      160    max. length of a file in SHM*/
#define FILE_BUFSIZE      LENGTH_FILE_BUFFER   /* max. length of a file in SHM*/
#define FILE_MAXLINE      24    /* max. line of a file in SHM */
#define MAX_GOODBYE       5     /* 离站画面数 */
#define MAX_ISSUE         5     /* 最大进站数 */
#define MAX_EXP           5     /* 离站画面数 */
#define MAX_DIGEST        1500  /* 最大文摘数 */

#define MORE_BUFSIZE    4096
#define MAXnettyLN      5     /* lines of  activity board  */        
/*#define ACBOARD_BUFSIZE      150    max. length of each line for activity board  */
/*#define ACBOARD_MAXLINE      80     max. lines of  activity board  */
#define ACBOARD_BUFSIZE      LENGTH_ACBOARD_BUFFER   /* max. length of each line for activity board  */
#define ACBOARD_MAXLINE      LENGTH_ACBOARD_LINE    /* max. lines of  activity board  */
#define STRLEN   80    /* Length of most string data */
#define IPLEN   16    /* Length of most string data */
#define NAMELEN  40    /* Length of username/realname */
#define IDLEN    12    /* Length of userids */
#define OLDPASSLEN  14    /* Length of encrypted passwd field */
#define MAXGOPHERITEMS 9999 /*max of gopher items*/
#define PASSFILE   ".PASSWDS"      /* Name of file User records stored in */
#define ULIST "UTMP"         /* Names of users currently on line */
#define POSTLOGFILE ".post.X" /* Name of file which log posting report */
#ifndef BBSIRC 

#define FLUSH    ".PASSFLUSH"    /* Stores date for user cache flushing */
#define WWWFLUSH ".WWWPASSFLUSH" /* user cache flushing for www regist */
#define BOARDS   ".BOARDS"       /* File containing list of boards */
#define DOT_DIR  ".DIR"          /* Name of Directory file info */
#define THREAD_DIR ".THREAD"
#define DIGEST_DIR  ".DIGEST"          /* Name of Directory file info */
#define NAMEFILE "BoardName"     /* File containing site name of bbs */

#define QUIT 0x666               /* Return value to abort recursive functions */
#define COUNT 0x2				/* count record */

#define FILE_READ  0x1        /* Ownership flags used in fileheader structure in accessed[1]*/
#define FILE_OWND  0x2        /* accessed array */
#define FILE_VISIT 0x4
#define FILE_MARKED 0x8
#define FILE_DIGEST 0x10      /* Digest Mode*//*For SmallPig Digest Mode*/
#define FILE_REPLIED 0x20      /* in mail ,added by alex, 96.9.7*/
#define FILE_FORWARDED 0x40      /* in mail ,added by alex, 96.9.7*/
#define FILE_IMPORTED 0x80      /* Leeward 98.04.15 */
#define FILE_SIGN 0x1		/* In article mode, Sign , Bigman 2000.8.12 ,in accessed[0]*/ 
#define FILE_DEL  0x2		/* In article mode, Sign , Bigman 2000.8.12 ,in accessed[1]*/ 

/* Leeward 98.05.11:
In /etc/sendmail.cf:
# maximum message size
# Changed by Netwolf(32K)
O MaxMessageSize=32000
*/
#define MAXMAILSIZE (32000L - 2000)
/* 2000 bytes for sendmail header */

#define VOTE_FLAG 0x1
#define NOZAP_FLAG 0x2
#define BOARD_READONLY 0x4

#define ZAPPED  0x1           /* For boards...tells if board is Zapped */

/* these are flags in userec.flags[0] */
#define PAGER_FLAG   0x1   /* true if pager was OFF last session */
#define CLOAK_FLAG   0x2   /* true if cloak was ON last session */
#define SIG_FLAG     0x8   /* true if sig was turned OFF last session */
#define BRDSORT_FLAG 0x20  /* true if the boards sorted alphabetical */
#define CURSOR_FLAG  0x80  /* true if the cursor mode open */
#define ACTIVE_BOARD 0x200
/* For All Kinds of Pagers */
#define ALL_PAGER       0x1
#define FRIEND_PAGER    0x2
#define ALLMSG_PAGER    0x4
#define FRIENDMSG_PAGER 0x8

#define SHIFTMODE(usernum,mode) ((usernum<MAXUSERS)?mode:mode<<4)

#define SETFILEMODE(array,usernum,mode) (array[usernum%MAXUSERS] |= ((usernum<MAXUSERS)?mode:mode<<4))

#define CLRFILEMODE(array,usernum,mode) (array[usernum%MAXUSERS] &= ((usernum<MAXUSERS)?~mode:~(mode<<4)))

#define CHECKFILEMODE(array,usernum,mode) (array[usernum%MAXUSERS] & ((usernum<MAXUSERS)?mode:mode<<4))
#define USERIDSIZE (16)
#define USERNAMESZ (24)
#define TERMTYPESZ (10)
/* END */

#endif /* BBSIRC */

#include "struct.h"
#include "webmsg.h"

/* add by period , header for function prototypes */
#include "prototype.h"

/*#ifndef BBSIRC*/
#include "modes.h"                /* The list of valid user modes */

#define DONOTHING       0       /* Read menu command return states */
#define FULLUPDATE      1       /* Entire screen was destroyed in this oper*/
#define PARTUPDATE      2       /* Only the top three lines were destroyed */
#define DOQUIT          3       /* Exit read menu was executed */
#define NEWDIRECT       4       /* Directory has changed, re-read files */
#define READ_NEXT       5       /* Direct read next file */
#define READ_PREV       6       /* Direct read prev file */
#define GOTO_NEXT       7       /* Move cursor to next */
#define DIRCHANGED      8       /* Index file was changed */

#define I_TIMEOUT   (-2)         /* Used for the getchar routine select call */
#define I_OTHERDATA (-333)       /* interface, (-3) will conflict with chinese */

#define SCREEN_SIZE (23)         /* Used by read menu  */

#define Min(a,b) ((a<b)?a:b)
#define Max(a,b) ((a>b)?a:b)

/*#endif*/ /* !BBSIRC */

/*SREAD Define*/
#define SR_BMBASE       (10)
#define SR_BMDEL        (11)
#define SR_BMMARK       (12)
#define SR_BMDIGEST     (13)
#define SR_BMIMPORT     (14)
#define SR_BMTMP        (15)
#define SR_BMMARKDEL   (16)
#define SR_BMNOREPLY    (17)
/*SREAD Define*/

#ifndef EXTEND_KEY
#define EXTEND_KEY
#define KEY_TAB         9
#define KEY_ESC         27
#define KEY_UP          0x0101
#define KEY_DOWN        0x0102
#define KEY_RIGHT       0x0103
#define KEY_LEFT        0x0104
#define KEY_HOME        0x0201
#define KEY_INS         0x0202
#define KEY_DEL         0x0203
#define KEY_END         0x0204
#define KEY_PGUP        0x0205
#define KEY_PGDN        0x0206
#endif

#define Ctrl(c)         ( c & 037 )
#define isprint2(c)     ( ((c) & 0xe0) && ((c)!=127) )

#ifdef  SYSV
#define bzero(tgt, len)         memset( tgt, 0, len )
#define bcopy(src, tgt, len)    memcpy( tgt, src, len)

#define usleep(usec)            { struct timeval t;t.tv_sec = usec / 1000000; t.tv_usec = usec % 1000000;select( 0, NULL, NULL, NULL, &t);}

#endif  /* SYSV */

/* =============== ANSI EDIT ================== */
#define   ANSI_RESET    "\033[37;40m\033[0m"
#define   ANSI_REVERSE  "\033[7m\033[4m"
extern    int  editansi;
extern    int  KEY_ESC_arg;
/* ============================================ */

extern int Net_Sleep(int);

#define MAX_IGNORE      30      /* ignore-users' count */
#define NUMBUFFER 80  /* number of records to preload. ylsdd*/

#ifdef SAFE_KILL
#define kill(x,y) safe_kill(x,y)
#endif

#ifdef PROFILE
#undef memcpy
#define memcpy(x,y,z) pr_memcpy(x,y,z)
#endif

#define chartoupper(c)  ((c >= 'a' && c <= 'z') ? c+'A'-'a' : c)

#define strncasecmp(x,y,n) ci_strncmp(x,y,n)
#define strcasecmp(x,y) ci_strcmp(x,y)

#ifdef NULL
#undef NULL
#endif

#define NULL 0
#define UNUSED_ARG(a) {(a)=(a); /* null */ }

#ifdef OS_LACK_SOCKLEN
typedef size_t socklen_t;
#endif

#ifndef BBS_PAGESIZE
#define BBS_PAGESIZE    (t_lines - 4)
#endif

#define BRC_MAXSIZE     32768
#define BRC_MAXNUM      60
#define BRC_STRLEN      15
#define BRC_ITEMSIZE    (BRC_STRLEN + 1 + BRC_MAXNUM * sizeof( int ))
#define UNREAD_TIME     (login_start_time - 30 * 86400)
/* added period 2000-09-11	4 FavBoard */
#define FAVBOARDNUM     20


#include "system.h"
#include "vars.h"
#include "func.h"

#define system my_system

#ifdef NJU_WWWBBS
#define MAXREJECTS (32)
#define BBSNAME NAME_BBS_CHINESE
#define BBSHOST NAME_BBS_ENGLISH
#define WWW_LOG             "bbslog/www.log"   /* "www.bbslog" */
#define WWW_BADLOGIN        "bbslog/badlogin.www"  /* "badlogin.www" */
#define SYS_MSGFILE         "msgfile"
#define SYS_MSGFILELOG      "msgfile.log"   /* "msgfile.me" */
#endif
#endif /* of _BBS_H_ */

