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

#include "kbs_config.h"

/* Global includes, needed in most every source file... */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <stdarg.h>

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif /* HAVE_STDDEF_H */

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif /* HAVE_STDINT_H */

#include <setjmp.h>
#include <errno.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/shm.h>
#include <sys/socket.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif /* HAVE_NETINET_IN_H */

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif /* HAVE_ARPA_INET_H */

#ifdef HAVE_TERMIO_H
#include <termio.h>
#endif /* HAVE_TERMIO_H */

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif /* HAVE_TERMIOS_H */

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif /* HAVE_DLFCN_H */

#ifndef DEBUG_MEMORY
#ifndef FREEBSD
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif /* HAVE_MALLOC_H */
#endif /*FREEBSD*/
#else /* DEBUG_MEMOTY */
#include "mpatrol.h"
#endif /* DEBUG_MEMORY */

#ifndef HAVE_BOOL
typedef enum { false = 0, true = 1 } bool;
#else /* HAVE_BOOL */

#ifndef true
#define true 1
#endif /* true */

#ifndef false
#define false 0
#endif /* false */

#endif /* HAVE_BOOL */

#ifndef TRUE
#define TRUE true
#endif /* TRUE */

#ifndef FALSE
#define FALSE false
#endif /* FALSE */

#ifndef _cplusplus
#ifndef HAVE_BYTE
typedef unsigned char byte;
#endif /* HAVE_BYTE */
#endif /* _cplusplus */

#ifndef HAVE_ISBLANK
#define isblank(c) ((c) == (int)' ' || (c) == (int)'\t' || (c) == (int)'\v')
#endif /* HAVE_ISBLANK */

#ifndef HAVE_SIG_T
typedef void (*sig_t)(int);
#endif /* HAVE_SIG_T */

#ifndef HAVE_SOCKLEN_T
typedef size_t socklen_t;
#endif /* HAVE_SOCKLEN_T */

#if SIZEOF_INT_P == SIZEOF_INT
typedef int POINTDIFF;
#elif SIZEOF_INT_P == SIZEOF_LONG_INT
typedef long int POINTDIFF;
#elif SIZEOF_INT_P == SIZEOF_LONG_LONG_INT
typedef long long int POINTDIFF;
#elif SIZEOF_INT_P == SIZEOF_INT64
typedef int64 POINTDIFF;
#elif SIZEOF_INT_P == SIZEOF_INT32
typedef int32 POINTDIFF;
#endif

#ifdef SOLARIS
typedef u_longlong_t u_int64_t;
#endif /* SOLARIS */

#define IDLEN           12  /* Length of userids */
#define OLDPASSLEN      14  /* Length of encrypted passwd field */
#define STRLEN          80  /* Length of most string data */
#define NAMELEN         40  /* Length of username/realname */
#define BOARDNAMELEN    30  /* max length of board */
#define PASSLEN         39
#define MD5PASSLEN      16

#include "site.h"
#include "default.h"

#ifndef IPLEN
#ifdef HAVE_IPV6_SMTH
#define IPLEN   46
#else /* HAVE_IPV6_SMTH */
#define IPLEN   16
#endif /* HAVE_IPV6_SMTH */
#endif /* IPLEN */

#ifdef HAVE_IPV6_SMTH
#define IPBITS   128
#define ip_cmp(x, y) memcmp(&x, &y, sizeof(struct in6_addr))
#define ip_cpy(x, y) memcpy(&x, &y, sizeof(struct in6_addr))
#else /* HAVE_IPV6_SMTH */
#define IPBITS   32
#define ip_cmp(x, y) ((uint32_t)x!=(uint32_t)y)
#define ip_cpy(x, y) ((uint32_t)x=(uint32_t)y)
#endif /* HAVE_IPV6_SMTH */

#define PUBLIC_SHMKEY	3700
/*这个是唯一一个定义死的SHMKEY,因为sysconf_eval需要
public shm,而attach shm又需要sysconf_eval,ft*/
/* add by KCN ,disable perror */

#ifdef BBSMAIN
#define perror(x) prints(x)
#endif /* BBSMAIN */

#define ARG_VOID void

#define VERSION_ID "Firebird BBS 2.5GB"
#define ADD_EDITMARK 1

#ifdef XINU
extern int errno;
#endif /* XINU */

#define DOECHO (1)              /* Flags to getdata input function */
#define NOECHO (0)

#ifndef HAVE_STRDUP
char *strdup(const char*);      /* External function declarations */
#endif /* HAVE_STRDUP */

char *bfile(void);

extern FILE *ufp;               /* External variable declarations */
extern long ti;

#ifdef FRIEND_MULTI_GROUP
#define MAXFRIENDS (500)
#define MAXFRIENDSGROUP 20
#else /* FRIEND_MULTI_GROUP */
#define MAXFRIENDS (400)
#endif /* FRIEND_MULTI_GROUP */

#define MOBILE_NUMBER_LEN   17
#define MAX_SIGNATURES      20      /* 最大签名档个数 by flyriver, 2002.11.10 */
#define MAX_MSG_SIZE        1024    /* 最大消息长度 by bad, 2003-2-14 */
#define ANNPATH_NUM	        40

#define FILE_BUFSIZE    LENGTH_FILE_BUFFER  /* max. length of a file in SHM */
#define FILE_MAXLINE    24                  /* max. line of a file in SHM */
#define MAX_GOODBYE     5                   /* 离站画面数 */
#define MAX_ISSUE       5                   /* 最大进站数 */
#define MAX_EXP         5                   /* 离站画面数 */
#define MAX_DIGEST      3000                /* 最大文摘数 */
#define MAX_DING        10                  /* 最大置顶数 */

/* added by bad 2002-08-2	FavBoardDir */
#define FAVBOARDNUM     100                 /*收藏夹最大条目*/
#define MAXBOARDPERDIR	100		            /*每个收藏夹目录最大版面数目*/
#define MAILBOARDNUM	20                  /*自定义邮箱最大数目*/

#define MORE_BUFSIZE    4096
#define ACBOARD_BUFSIZE LENGTH_ACBOARD_BUFFER   /* max. length of each line for activity board  */
#define ACBOARD_MAXLINE LENGTH_ACBOARD_LINE     /* max. lines of  activity board  */

#define MAXGOPHERITEMS  9999                /* max of gopher items */
#define PASSFILE        ".PASSWDS"          /* Name of file User records stored in */
#define ULIST           "UTMP"              /* Names of users currently on line */
#define POSTLOGFILE     ".post.X"           /* Name of file which log posting report */

#define USER_TITLE_LEN	18
#define USER_TITLE_FILE "etc/title"
#define FLUSH           ".PASSFLUSH"        /* Stores date for user cache flushing */
#define WWWFLUSH        ".WWWPASSFLUSH"     /* user cache flushing for www regist */
#define BOARDS          ".BOARDS"           /* File containing list of boards */
#define DOT_DIR         ".DIR"              /* Name of Directory file info */
#define THREAD_DIR      ".THREAD"
#define DIGEST_DIR      ".DIGEST"           /* Name of Directory file info */
#define DING_DIR        ".DINGDIR"	        /* 置顶文章.DIR , stiger */
#define NAMEFILE        "BoardName"         /* File containing site name of bbs */
#define BADLOGINFILE    "logins.bad"
#define USERDATA        ".userdata"

#define QUIT            0x666       /* Return value to abort recursive functions */
#define COUNT           0x2         /* count record */

#define FILE_SIGN       0x1         /* In article mode, Sign , Bigman 2000.8.12 ,in accessed[0] */
#define FILE_OWND       0x2         /* accessed array */
#define FILE_PERCENT    0x4         /* %标记 */
#define FILE_MARKED     0x8
#define FILE_DIGEST     0x10        /* Digest Mode*/  /*For SmallPig Digest Mode */
#define FILE_REPLIED    0x20        /* in mail ,added by alex, 96.9.7 */
#define FILE_FORWARDED  0x40        /* in mail ,added by alex, 96.9.7 */
#define FILE_IMPORTED   0x80        /* Leeward 98.04.15 */

#define FILE_CENSOR     0x20        /* for accessed[1], flyriver, 2002.9.29 */
#define FILE_READ       0x1         /* Ownership flags used in fileheader structure in accessed[1] */
#define FILE_DEL        0x2         /* In article mode, Sign , Bigman 2000.8.12 ,in accessed[1] */
#define FILE_MAILBACK	0x4		    /* reply articles mail to owner's mailbox, accessed[1] */
#define FILE_COMMEND    0x8		    /* 推荐文章,stiger , in accessed[1], */
#define FILE_TEX        0x80        /* tex article, in accessed[1], added by atppp 20040729 */

#define BADWORD_IMG_FILE    "etc/badwordv3.img"
#define MAXMAILSIZE         (30000)

#define CHECK_MAIL          0x1	    /* 0 需要检查信笺  1 不需要重新检查 */
#define CHECK_MSG           0x2	    /* 0 没有 msg 1 有 msg */

#define BOARD_VOTEFLAG      0x1
#define BOARD_NOZAPFLAG     0x2
#define BOARD_READONLY      0x4
#define BOARD_JUNK	        0x8
#define BOARD_ANNONY        0x10
#define BOARD_OUTFLAG       0x20    /* for outgo boards */
#define BOARD_CLUB_READ     0x40    /* 限制读的俱乐部 */
#define BOARD_CLUB_WRITE    0x80    /* 限制写的俱乐部 */
#define BOARD_CLUB_HIDE     0x100   /* 隐藏俱乐部 */
#define BOARD_ATTACH        0x200   /* 可以使用附件的版面 */
#define BOARD_GROUP         0x400   /* 目录 */
#define BOARD_EMAILPOST     0x800   /* Email 发文 */
#define BOARD_POSTSTAT      0x1000  /* 不统计十大 */
#define BOARD_NOREPLY       0x2000  /* 不可 re 文 */
#define BOARD_RULES         0x4000  /* 治版方针 ok */
/* boardheader.flag 的最高八位留给用户自定义用途: 0xXX000000 */

#define ZAPPED              0x1     /* For boards...tells if board is Zapped */

/* these are flags in userec.flags[0] */
#define PAGER_FLAG          0x1     /* true if pager was OFF last session */
#define CLOAK_FLAG          0x2     /* true if cloak was ON last session */
#define BRDSORT_FLAG        0x20    /* true if the boards sorted alphabetical */
#define GIVEUP_FLAG         0x4     /* true if the user is giving up  by bad 2002.7.6 */
#define PCORP_FLAG	        0x40    /* true if have personalcorp */
#define ACTIVATED_FLAG      0x8     /* true if email activated - atppp 20050405 */
#define BRDSORT1_FLAG       0x10    /* true if sort by online */
#define USEREC_FLAGS_UNUSED_FLAG  0x80  /* ask atppp if you want to use this bit - 20051215 */

/* For All Kinds of Pagers */
#define ALL_PAGER       0x1
#define FRIEND_PAGER    0x2
#define ALLMSG_PAGER    0x4
#define FRIENDMSG_PAGER 0x8
#define USERIDSIZE      16
#define USERNAMESZ      24
#define TERMTYPESZ      10
/* END */

#include "struct.h"
#include "screen.h"
#include "webmsg.h"
#include "ann.h"
#include "ansi.h"
#include "output.h"
#include "calltime.h"
#include "tmpl.h"
#include "modes.h"              /* The list of valid user modes */

#define I_TIMEOUT   (-2)    /* Used for the getchar routine select call */
#define I_OTHERDATA (-333)  /* interface, (-3) will conflict with chinese */
#define SCREEN_SIZE (23)    /* Used by read menu  */

#define Min(a,b)    (((a) < (b))? (a) : (b))
#define Max(a,b)    (((a) > (b))? (a) : (b))

#ifndef EXTEND_KEY
#define EXTEND_KEY
#define KEY_TAB     0x0009
#define KEY_ESC     0x001b
#define KEY_UP      0x0101
#define KEY_DOWN    0x0102
#define KEY_RIGHT   0x0103
#define KEY_LEFT    0x0104
#define KEY_HOME    0x0201
#define KEY_INS     0x0202
#define KEY_DEL     0x0203
#define KEY_END     0x0204
#define KEY_PGUP    0x0205
#define KEY_PGDN    0x0206
#define KEY_F1      0x0207
#define KEY_F2      0x0208
#define KEY_F3      0x0209
#define KEY_F4      0x020a
#define KEY_F5      0x020b
#define KEY_F6      0x020c
#define KEY_F7      0x020d
#define KEY_F8      0x020e
#define KEY_F9      0x020f
#define KEY_F10     0x0210
#endif /* EXTEND_KEY */

#define Ctrl(c)     ((c) & 0x1f)
#define isprint2(c) (((c) & 0xe0) && ((c)!=0x7f))

#ifndef HAVE_BZERO
#define bzero(ptr,size)     memset((ptr),0,(size))
#endif /* HAVE_BZERO */

#ifndef HAVE_BCOPY
#define bcopy(src,dst,size) memcpy((dst),(src),(size))
#endif /* HAVE_BCOPY */

#ifndef HAVE_USLEEP
#define usleep(usec)        do{struct timeval tv;tv.tv_sec=usec/1000000;tv.tv_usec=usec%1000000;select(0,NULL,NULL,NULL,&tv);}while(0)
#endif /* HAVE_USLEEP */

/* =============== ANSI EDIT ================== */
#define   ANSI_RESET    "\033[37;40m\033[m"
#define   ANSI_REVERSE  "\033[7m\033[4m"
extern int KEY_ESC_arg;
/* ============================================ */

#define MAX_IGNORE      300     /* ignore-users' count */
#define NUMBUFFER       80      /* number of records to preload. ylsdd */

#ifdef kill
#undef kill
#endif /* kill */
#define kill(x,y)       safe_kill(x,y)

#ifdef PROFILE
#ifdef memcpy
#undef memcpy
#endif /* memcpy */
#define memcpy(x,y,z)   pr_memcpy(x,y,z)
#endif /* PROFILE */

#define chartoupper(c)  (((c) >= 'a' && (c) <= 'z')? ((c) - 32) : (c))

#ifdef NULL
#undef NULL
#endif /* NULL */
#define NULL 0

#define UNUSED_ARG(a) do{(a) = (a);}while(0)
#define BBS_PAGESIZE  (t_lines - 4)

/* added by bad 2002.8.1 */
#define FILE_MARK_FLAG      0x1
#define FILE_NOREPLY_FLAG   0x2
#define FILE_SIGN_FLAG      0x4
#define FILE_DELETE_FLAG    0x8
#define FILE_DIGEST_FLAG    0x10
#define FILE_TITLE_FLAG     0x20
#define FILE_IMPORT_FLAG    0x40

#ifdef FILTER
#define FILE_CENSOR_FLAG    0x80
#endif /* FILTER */

#define FILE_ATTACHPOS_FLAG 0x100
#define FILE_DING_FLAG      0x200	/* stiger,置顶 */
#define FILE_EFFSIZE_FLAG   0x400
#define FILE_COMMEND_FLAG   0x800
#define FILE_PERCENT_FLAG   0x1000

/* 0-1 locks used sem count, @author kxn */
#define SEMLOCK_COUNT       20
#define UCACHE_SEMLOCK      0
#define UTMP_SEMLOCK        1
#define BCACHE_SEMLOCK      2
#define BSTATUS_SEMLOCK     3

/* Enumeration values for the so-called board .DIR file, @author flyriver */
enum BBS_DIR_MODE {
    DIR_MODE_NORMAL     = 0,        /* .DIR */
    DIR_MODE_DIGEST     = 1,        /* .DIGEST */
    DIR_MODE_THREAD     = 2,        /* .THREAD */
    DIR_MODE_MARK       = 3,        /* .MARK */
    DIR_MODE_DELETED    = 4,        /* .DELETED */
    DIR_MODE_JUNK       = 5,        /* .JUNK */
    DIR_MODE_ORIGIN     = 6,        /* .ORIGIN */
    DIR_MODE_AUTHOR     = 7,        /* .AUTHOR.userid */
    DIR_MODE_TITLE      = 8,        /* .TITLE.userid */
    DIR_MODE_SUPERFITER = 9,        /* .Search.userid */
    DIR_MODE_WEB_THREAD = 10,       /* .WEBTHREAD */
    DIR_MODE_ZHIDING    = 11,       /* .DINGDIR */
    DIR_MODE_MAIL ,                 /* 邮件模式 */
    DIR_MODE_FRIEND,                /* 好友名单 */
    DIR_MODE_TOP10,                 /* 十大模式 */
    DIR_MODE_SELF,                  /* 自删模式, .SELF.userid */
    DIR_MODE_UNKNOWN                /* 未知模式 */
};

enum BBSLOG_TYPE {
    BBSLOG_USIES        = 1,
    BBSLOG_USER         = 2,
    BBSLOG_BOARDUSAGE   = 3,
    BBSLOG_SMS          = 4,
    BBSLOG_DEBUG        = 5,
	BBSLOG_POST         = 6,
	BBSLOG_BM           = 7
};

enum BBSPOST_MODE {
    BBSPOST_COPY        = 0,
    BBSPOST_MOVE        = 1,
    BBSPOST_LINK        = 2
};

#define ATTACHMENT_PAD      "\0\0\0\0\0\0\0\0"
#define ATTACHMENT_SIZE     8

#include "system.h"
#define system              my_system

#ifndef MAXPATH
#define MAXPATH 255
#endif /* MAXPATH */

#define BLACK           0
#define RED             1
#define GREEN           2
#define YELLOW          3
#define BLUE            4
#define PINK            5
#define CYAN            6
#define WHITE           7

#define BRC_MAXNUM      50
#define BRC_ITEMSIZE    (BRC_MAXNUM * sizeof( unsigned int ))
#define BRC_FILESIZE    BRC_ITEMSIZE*MAXBOARD

#define BRC_CACHE_NUM   50      /* 未读标记被 cache 在 tmpfs 中了 */

#define BRCFILE         ".boardrc.gz"

struct _brc_cache_entry {
    int bid;
    unsigned int list[BRC_MAXNUM];
    int changed;
};

#define FRIENDSGROUP_LIST   0x1
#define FRIENDSGROUP_MSG    0x2
#define FRIENDSGROUP_PAGER  0x4
#define FRIENDSGROUP_MAIL   0x8
#define FRIENDSGROUP_SEEME  0x10
#define FRIENDSGROUP_UNUSE  0x20

#ifdef SMS_SUPPORT
#define SMS_SHM_SIZE        1024*50

struct header{
  char Type;
  byte SerialNo[4];
  byte pid[4];
  byte BodyLength[4];               //总Packet长度
};

#define CMD_LOGIN                   1
#define CMD_OK                      101
#define CMD_ERR                     102
#define CMD_ERR_HEAD_LENGTH         103
#define CMD_ERR_DB                  104
#define CMD_ERR_SMS_VALIDATE_FAILED 105
#define CMD_ERR_LENGTH              106
#define CMD_ERR_NO_VALIDCODE        107
#define CMD_ERR_NO_SUCHMOBILE       108
#define CMD_ERR_REGISTERED          109
#define CMD_EXCEEDMONEY_LIMIT       110
#define CMD_LOGOUT                  2
#define CMD_REG                     3
#define CMD_CHECK                   4
#define CMD_UNREG                   5
#define CMD_REQUIRE                 6
#define CMD_REPLY                   7
#define CMD_BBSSEND                 8
#define CMD_GWSEND                  9
#define CMD_LINK                    20

#define USER_LEN    20
#define PASS_LEN    50

struct LoginPacket {                /* Type = 1 */
    char user[USER_LEN];
    char pass[PASS_LEN];
};
struct RegMobileNoPacket {          /* Type = 3 */
    char MobileNo[MOBILE_NUMBER_LEN];
    char cUserID[IDLEN+2];
};
struct CheckMobileNoPacket {        /* Type = 4 */
    char MobileNo[MOBILE_NUMBER_LEN];
    char cUserID[IDLEN+2];
    char ValidateNo[MOBILE_NUMBER_LEN];
};
struct UnRegPacket {                /* Type = 5 */
    char MobileNo[MOBILE_NUMBER_LEN];
    char cUserID[IDLEN+2];
};
struct RequireBindPacket {          /* Type = 6 */
    char cUserID[IDLEN+2];
    char MobileNo[MOBILE_NUMBER_LEN];
    byte Bind;
};
struct ReplyBindPacket {            /* Type = 7 */
    char MobileNo[MOBILE_NUMBER_LEN];
    byte isSucceed;
};
struct BBSSendSMS {                 /* Type = 8 */
    byte UserID[4];
    char SrcMobileNo[MOBILE_NUMBER_LEN];
    char SrccUserID[IDLEN+2];
    char DstMobileNo[MOBILE_NUMBER_LEN];
    byte MsgTxtLen[4];
};
struct GWSendSMS {                  /* Type = 9 */
    byte UserID[4];
    char SrcMobileNo[MOBILE_NUMBER_LEN];
    byte MsgTxtLen[4];
};
struct sms_shm_head {
    int sem;
    int total;
    int length;
};
extern struct sms_shm_head *head;

#endif /* SMS_SUPPORT */

#define GIVEUPINFO_PERM_COUNT   6       /* etnlegend, 2005.11.26, 增加有时限封禁支持并修正戒网的一些问题 */

/* etnlegend, 2006.04.20, 区段删除核心 */
#define DELETE_RANGE_BASE_MODE_TOKEN    0x01        /* 文章列表, 删除拟删文章 */
#define DELETE_RANGE_BASE_MODE_RANGE    0x02        /* 文章列表和信件列表, 普通区段删除 */
#define DELETE_RANGE_BASE_MODE_FORCE    0x04        /* 文章列表和信件列表, 强制区段删除 */
#define DELETE_RANGE_BASE_MODE_MPDEL    0x08        /* 文章列表, 设置拟删标记 */
#define DELETE_RANGE_BASE_MODE_CLEAR    0x10        /* 文章列表, 清除拟删标记 */
#define DELETE_RANGE_BASE_MODE_MAIL     0x20        /* 信件列表模式 */
#define DELETE_RANGE_BASE_MODE_CHECK    0x40        /* 校验源 DIR 文件修改 */
#define DELETE_RANGE_BASE_MODE_OVERM    0x80        /* 拟删标记优先权高于保留标记 */
#define DELETE_RANGE_BASE_MODE_OPMASK   (0\
    |DELETE_RANGE_BASE_MODE_TOKEN\
    |DELETE_RANGE_BASE_MODE_RANGE\
    |DELETE_RANGE_BASE_MODE_FORCE\
    |DELETE_RANGE_BASE_MODE_MPDEL\
    |DELETE_RANGE_BASE_MODE_CLEAR\
)                                                   /* 区段删除模式掩码 */
/* --END--, etnlegend, 2006.04.20, 区段删除核心 */

#define ARG_NOPROMPT_FLAG       1       /* 操作不提示 */
#define ARG_DELDECPOST_FLAG     2       /* 删除操作要减文章数 */
#define ARG_BMFUNC_FLAG         4       /* 版主操作标志 */

#define BLK_SIZ                 10240

#include "var.h"

#ifdef BBSMAIN
#include "vars.h"
#include "defines_t.h"
#endif /* BBSMAIN */

#include "func.h"

#endif /* _BBS_H_ */

