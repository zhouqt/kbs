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

#include "bbsconfig.h"          /* User-configurable stuff */
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
#ifndef	DEBUG_MEMORY
#ifndef FREEBSD
#include <malloc.h>
#endif
#else
#include "mpatrol.h"
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdarg.h>

#ifndef _cplusplus

#ifndef HAVE_BOOL
typedef enum { false = 0, true = 1 } bool;
#else

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#endif
#endif

#include "site.h"
#include "default.h"

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
extern int errno;
#endif

#define DOECHO (1)              /* Flags to getdata input function */
#define NOECHO (0)

#ifndef strdup
char *strdup();                 /* External function declarations */
#endif
char *bfile();

extern FILE *ufp;               /* External variable declarations */
extern long ti;

#endif                          /* BBSIRC */

#ifdef FRIEND_MULTI_GROUP
#define MAXFRIENDS (500)
#define MAXFRIENDSGROUP 20
#else
#define MAXFRIENDS (400)
#endif

/*#ifdef SMS_SUPPORT*/
#define MOBILE_NUMBER_LEN 17
/*#endif*/
/*#define NUMPERMS (31)*/

#define MAX_SIGNATURES 20       /* 最大签名档个数 by flyriver, 2002.11.10 */

#define MAX_MSG_SIZE 1024       /* 最大消息长度 by bad, 2003-2-14 */

#define ANNPATH_NUM	40

/*#define FILE_BUFSIZE      160    max. length of a file in SHM*/
#define FILE_BUFSIZE      LENGTH_FILE_BUFFER    /* max. length of a file in SHM */
#define FILE_MAXLINE      24    /* max. line of a file in SHM */
#define MAX_GOODBYE       5     /* 离站画面数 */
#define MAX_ISSUE         5     /* 最大进站数 */
#define MAX_EXP           5     /* 离站画面数 */
#define MAX_DIGEST        1500  /* 最大文摘数 */
#define MAX_DING          5	/* 最大置顶数 */

/* added by bad 2002-08-2	FavBoardDir */
#define FAVBOARDNUM     100     /*收藏夹最大条目*/
#define MAILBOARDNUM	20      /*自定义邮箱最大数目*/


#define MORE_BUFSIZE    4096
#define MAXnettyLN      5       /* lines of  activity board  */
/*#define ACBOARD_BUFSIZE      300    max. length of each line for activity board  */
/*#define ACBOARD_MAXLINE      80     max. lines of  activity board  */
#define ACBOARD_BUFSIZE      LENGTH_ACBOARD_BUFFER      /* max. length of each line for activity board  */
#define ACBOARD_MAXLINE      LENGTH_ACBOARD_LINE        /* max. lines of  activity board  */
#define STRLEN   80             /* Length of most string data */
#define IPLEN   16              /* Length of most string data */
#define NAMELEN  40             /* Length of username/realname */
#define IDLEN    12             /* Length of userids */
#define BOARDNAMELEN 30         /* max length of board */
#define PASSLEN 39
#define MD5PASSLEN 16
#define OLDPASSLEN  14          /* Length of encrypted passwd field */
#define MAXGOPHERITEMS 9999     /*max of gopher items */
#define PASSFILE   ".PASSWDS"   /* Name of file User records stored in */
#define ULIST "UTMP"            /* Names of users currently on line */
#define POSTLOGFILE ".post.X"   /* Name of file which log posting report */
#ifndef BBSIRC

#define FLUSH    ".PASSFLUSH"   /* Stores date for user cache flushing */
#define WWWFLUSH ".WWWPASSFLUSH"        /* user cache flushing for www regist */
#define BOARDS   ".BOARDS"      /* File containing list of boards */
#define DOT_DIR  ".DIR"         /* Name of Directory file info */
#define THREAD_DIR ".THREAD"
#define DIGEST_DIR  ".DIGEST"   /* Name of Directory file info */
#define DING_DIR ".DINGDIR"	/* 置顶文章.DIR , stiger */
#define NAMEFILE "BoardName"    /* File containing site name of bbs */
#define BADLOGINFILE    "logins.bad"
#define USERDATA ".userdata"

#define QUIT 0x666              /* Return value to abort recursive functions */
#define COUNT 0x2               /* count record */

#define FILE_SIGN 0x1           /* In article mode, Sign , Bigman 2000.8.12 ,in accessed[0] */
#define FILE_OWND  0x2          /* accessed array */
#define FILE_VISIT 0x4
#define FILE_MARKED 0x8
#define FILE_DIGEST 0x10      /* Digest Mode*/  /*For SmallPig Digest Mode */
#define FILE_REPLIED 0x20       /* in mail ,added by alex, 96.9.7 */
#define FILE_FORWARDED 0x40     /* in mail ,added by alex, 96.9.7 */
#define FILE_IMPORTED 0x80      /* Leeward 98.04.15 */

/* roy 2003.07.21 */
#define FILE_WWW_POST	0x1 /* post by www */
#define FILE_ON_TOP	0x2 /* on top mode */
#define FILE_VOTE	0x4 /* article with votes */

#ifdef FILTER
#define FILE_CENSOR 0x20        /* for accessed[1], flyriver, 2002.9.29 */
#define BADWORD_IMG_FILE "etc/badwordv2.img"
#endif
#define FILE_READ  0x1          /* Ownership flags used in fileheader structure in accessed[1] */
#define FILE_DEL  0x2           /* In article mode, Sign , Bigman 2000.8.12 ,in accessed[1] */
#define FILE_MAILBACK	0x4		/* reply articles mail to owner's mailbox, accessed[1] */


/* Leeward 98.05.11:
In /etc/sendmail.cf:
# maximum message size
# Changed by Netwolf(32K)
O MaxMessageSize=32000
*/
#define MAXMAILSIZE (32000L - 2000)
/* 2000 bytes for sendmail header */

#define BOARD_VOTEFLAG 0x1
#define BOARD_NOZAPFLAG 0x2
#define BOARD_READONLY 0x4
#define BOARD_JUNK	0x8
#define BOARD_ANNONY 0x10
#define BOARD_OUTFLAG 0x20      /* for outgo boards */
#define BOARD_CLUB_READ  0x40  /*限制读的俱乐部*/
#define BOARD_CLUB_WRITE  0x80  /*限制写的俱乐部*/
#define BOARD_CLUB_HIDE  0x100  /*隐藏俱乐部*/
#define BOARD_ATTACH        0x200 /*可以使用附件的版面*/
#define BOARD_GROUP 0x400   /*目录*/
#define BOARD_EMAILPOST 0x800 /* Email 发文 */
/* boardheader.flag 的最高八位留给用户自定义用途: 0xXX000000 */

#define ZAPPED  0x1             /* For boards...tells if board is Zapped */

/* these are flags in userec.flags[0] */
#define PAGER_FLAG   0x1        /* true if pager was OFF last session */
#define CLOAK_FLAG   0x2        /* true if cloak was ON last session */
#define BRDSORT_FLAG 0x20       /* true if the boards sorted alphabetical */
#define CURSOR_FLAG  0x80       /* true if the cursor mode open */
#define GIVEUP_FLAG  0x4        /* true if the user is giving up  by bad 2002.7.6 */
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

#endif                          /* BBSIRC */

#include "struct.h"
#include "screen.h"
#include "webmsg.h"
#include "ann.h"
#include "ansi.h"
#include "output.h"
#include "calltime.h"
#include "tmpl.h"

/* add by period , header for function prototypes */
#include "prototype.h"

/*#ifndef BBSIRC*/
#include "modes.h"              /* The list of valid user modes */

#define DONOTHING       0       /* Read menu command return states */
#define FULLUPDATE      1       /* Entire screen was destroyed in this oper */
#define PARTUPDATE      2       /* Only the top three lines were destroyed */
#define DOQUIT          3       /* Exit read menu was executed */
#define NEWDIRECT       4       /* Directory has changed, re-read files */
#define READ_NEXT       5       /* Direct read next file */
#define READ_PREV       6       /* Direct read prev file */
#define GOTO_NEXT       7       /* Move cursor to next */
#define DIRCHANGED      8       /* Index file was changed */
#define NEWSCREEN	9	/* split the screen */
#define CHANGEMODE  10  /* 换版面了或者是换模式了*/

#define I_TIMEOUT   (-2)        /* Used for the getchar routine select call */
#define I_OTHERDATA (-333)      /* interface, (-3) will conflict with chinese */

#define SCREEN_SIZE (23)        /* Used by read menu  */

#define Min(a,b) ((a<b)?a:b)
#define Max(a,b) ((a>b)?a:b)

/*#endif*//* !BBSIRC */

/*SREAD Define*/
#define SR_BMBASE       (10)
#define SR_BMDEL        (11)
#define SR_BMMARK       (12)
#define SR_BMDIGEST     (13)
#define SR_BMIMPORT     (14)
#define SR_BMTMP        (15)
#define SR_BMMARKDEL   (16)
#define SR_BMNOREPLY    (17)
#define SR_BMTOTAL      (18)
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
#define KEY_F1            0x0207
#define KEY_F2            0x0208
#define KEY_F3            0x0209
#define KEY_F4            0x020a
#define KEY_F5            0x020b
#define KEY_F6            0x020c
#define KEY_F7            0x020d
#define KEY_F8            0x020e
#define KEY_F9            0x020f
#define KEY_F10            0x0210
#endif

#define Ctrl(c)         ( c & 037 )
#define isprint2(c)     ( ((c) & 0xe0) && ((c)!=127) )

#ifdef  SYSV
#define bzero(tgt, len)         memset( tgt, 0, len )
#define bcopy(src, tgt, len)    memcpy( tgt, src, len)

#define usleep(usec)            { struct timeval t;t.tv_sec = usec / 1000000; t.tv_usec = usec % 1000000;select( 0, NULL, NULL, NULL, &t);}

#endif                          /* SYSV */

/* =============== ANSI EDIT ================== */
#define   ANSI_RESET    "\033[37;40m\033[m"
#define   ANSI_REVERSE  "\033[7m\033[4m"
/*extern int editansi;*/
extern int KEY_ESC_arg;

/* ============================================ */

#ifdef BBSMAIN
#define Sleep Net_Sleep
extern void Net_Sleep(int);
#endif

#define MAX_IGNORE      30      /* ignore-users' count */
#define NUMBUFFER 80            /* number of records to preload. ylsdd */

#ifdef SAFE_KILL
#define kill(x,y) safe_kill(x,y)
#endif

#ifdef PROFILE
#undef memcpy
#define memcpy(x,y,z) pr_memcpy(x,y,z)
#endif

#define chartoupper(c)  ((c >= 'a' && c <= 'z') ? c+'A'-'a' : c)

/*
#define strncasecmp(x,y,n) ci_strncmp(x,y,n)
#define strcasecmp(x,y) ci_strcmp(x,y)
*/

#ifdef NULL
#undef NULL
#endif

#define NULL 0
#define UNUSED_ARG(a) {(a)=(a); /* null */ }

#ifdef OS_LACK_SOCKLEN
typedef size_t socklen_t;
#endif

#define BBS_PAGESIZE    (t_lines - 4)

/* added by bad 2002.8.1 */
#define FILE_MARK_FLAG 0
#define FILE_NOREPLY_FLAG 1
#define FILE_SIGN_FLAG 2
#define FILE_DELETE_FLAG 3
#define FILE_DIGEST_FLAG 4
#define FILE_TITLE_FLAG 5
#define FILE_IMPORT_FLAG 6
#ifdef FILTER
#define FILE_CENSOR_FLAG 7
#endif
#define FILE_ATTACHPOS_FLAG 8
#define FILE_DING_FLAG 9	/* stiger,置顶 */

/**
 * Enumeration values for the so-called board .DIR file.
 *
 * @author flyriver
 */
enum BBS_DIR_MODE
{
    DIR_MODE_NORMAL  = 0, /** .DIR */
    DIR_MODE_DIGEST  = 1, /** .DIGEST */
    DIR_MODE_THREAD  = 2, /** .THREAD */
    DIR_MODE_MARK    = 3, /** .MARK */
    DIR_MODE_DELETED = 4, /** .DELETED */
    DIR_MODE_JUNK    = 5, /** .JUNK */
    DIR_MODE_ORIGIN  = 6, /** .ORIGIN */
    DIR_MODE_AUTHOR  = 7, /** .AUTHOR.userid */
    DIR_MODE_TITLE   = 8, /** .TITLE.userid */
	DIR_MODE_ZHIDING = 9  /** .DINGDIR */
};

enum BBSLOG_TYPE
{
    BBSLOG_USIES =1,
    BBSLOG_USER  =2,
    BBSLOG_BOARDUSAGE =3
};

enum BBSPOST_MODE
{
    BBSPOST_COPY=0,
    BBSPOST_MOVE=1,
    BBSPOST_LINK=2
};

#define ATTACHMENT_PAD "\0\0\0\0\0\0\0\0"
#define ATTACHMENT_SIZE 8
#include "system.h"
#include "vars.h"
#include "func.h"
#ifdef BBSMAIN
#include "defines_t.h"
#endif

#define system my_system
#ifndef MAXPATH
#define MAXPATH 255
#endif

#ifdef NJU_WWWBBS
#define MAXREJECTS (32)
#define BBSNAME NAME_BBS_CHINESE
#define BBSHOST NAME_BBS_ENGLISH
#define WWW_LOG             "bbslog/www.log"    /* "www.bbslog" */
#define WWW_BADLOGIN        "bbslog/badlogin.www"       /* "badlogin.www" */
#define SYS_MSGFILE         "msgfile"
#define SYS_MSGFILELOG      "msgfile.log"       /* "msgfile.me" */
#endif

#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define PINK 5
#define CYAN 6
#define WHITE 7

#define BRC_MAXNUM      50
#define BRC_ITEMSIZE    (BRC_MAXNUM * sizeof( unsigned int ))
#define BRC_FILESIZE BRC_ITEMSIZE*MAXBOARD

#if USE_TMPFS==0
#define BRC_CACHE_NUM 20        /* 未读标记cache 20个版 */
#else
#define BRC_CACHE_NUM 50        /* 未读标记被cache在tmpfs中了 */
#endif

#define BRCFILE ".boardrc.gz"
struct _brc_cache_entry {
    int bid;
    unsigned int list[BRC_MAXNUM];
    int changed;
};

#define FRIENDSGROUP_LIST 01
#define FRIENDSGROUP_MSG 02
#define FRIENDSGROUP_PAGER 04
#define FRIENDSGROUP_MAIL 08
#define FRIENDSGROUP_SEEME 010
#define FRIENDSGROUP_UNUSE 020

#ifdef SMS_SUPPORT

#ifndef byte
typedef unsigned char byte;
#endif

#define SMS_SHM_SIZE 1024*50

struct header{
  char Type;
  byte SerialNo[4];
  byte pid[4];
  byte BodyLength[4];   //总Packet长度
};

#define CMD_LOGIN 1
#define CMD_OK 101
#define CMD_ERR 102
#define CMD_ERR_HEAD_LENGTH 103
#define CMD_ERR_DB 104
#define CMD_ERR_SMS_VALIDATE_FAILED 105
#define CMD_ERR_LENGTH 106
#define CMD_ERR_NO_VALIDCODE 107
#define CMD_ERR_NO_SUCHMOBILE 108
#define CMD_ERR_REGISTERED 109
#define CMD_LOGOUT 2
#define CMD_REG 3
#define CMD_CHECK 4
#define CMD_UNREG 5
#define CMD_REQUIRE 6
#define CMD_REPLY 7
#define CMD_BBSSEND 8
#define CMD_GWSEND 9

#define USER_LEN 20
#define PASS_LEN 50

struct LoginPacket { //Type=1
    char user[USER_LEN];
    char pass[PASS_LEN];
};
struct RegMobileNoPacket { //Type=3
    char MobileNo[MOBILE_NUMBER_LEN];
    char cUserID[IDLEN+2];
};
struct CheckMobileNoPacket { //Type=4
    char MobileNo[MOBILE_NUMBER_LEN];
    char cUserID[IDLEN+2];
    char ValidateNo[MOBILE_NUMBER_LEN];
};
struct UnRegPacket { //Type=5
    char MobileNo[MOBILE_NUMBER_LEN];
    char cUserID[IDLEN+2];
};
struct RequireBindPacket { //Type = 6
    char cUserID[IDLEN+2];
    char MobileNo[MOBILE_NUMBER_LEN];
    char Bind;
};
struct ReplyBindPacket { //Type=7
    char MobileNo[MOBILE_NUMBER_LEN];
    char isSucceed;
};
struct BBSSendSMS { //Type=8
    byte UserID[4];
    char SrcMobileNo[MOBILE_NUMBER_LEN];
    char SrccUserID[IDLEN+2];
    char DstMobileNo[MOBILE_NUMBER_LEN];
    byte MsgTxtLen[4];
};
struct GWSendSMS { //Type=9
    byte UserID[4];
    char SrcMobileNo[MOBILE_NUMBER_LEN];
    byte MsgTxtLen[4];
};

struct sms_shm_head {
    int sem;
    int total;
    int length;
} * head;

#endif


#endif                          /* of _BBS_H_ */
