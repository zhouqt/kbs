/* Note the protocol field is not inside an #ifdef FILES...
   this is a waste but allows you to add/remove UL/DL support without
   rebuilding the PASSWDS file (and it's only a lil ole int anyway).
*/

struct userec {                  /* Structure used to hold information in */
        char            userid[IDLEN+2];   /* PASSFILE */
        char            fill[30];
        time_t          firstlogin;
        char            lasthost[16];
        unsigned int    numlogins;
        unsigned int    numposts;
        char            flags[2];
        char            passwd[OLDPASSLEN];
        char            username[NAMELEN];
        char            ident[NAMELEN];
        unsigned char   md5passwd[16];
        char			realemail[STRLEN-16];
        unsigned        userlevel;
        time_t          lastlogin;
        time_t          stay;
        char            realname[NAMELEN];
        char            address[STRLEN];
        char            email[STRLEN];
        int             signature;
        unsigned int    userdefine;
        time_t          notedate;
        int             noteline;
        int             notemode;
        int             unuse1;/* no use*/
        int             unuse2;/* no use*/
};

struct user_info {              /* Structure used in UTMP file */
        int     active;         /* When allocated this field is true */
        int     uid;            /* Used to find user name in passwd file */
        int     pid;            /* kill() to notify user of talk request */
        int     invisible;      /* Used by cloaking function in Xyz menu */
        int     sockactive;     /* Used to coordinate talk requests */
        int     sockaddr;       /* ... */
        int     destuid;        /* talk uses this to identify who called */
        int     mode;           /* UL/DL, Talk Mode, Chat Mode, ... */
        int     pager;          /* pager toggle, YEA, or NA */
        int     in_chat;        /* for in_chat commands   */
        char    chatid[ 16 ];   /* chat id, if in chat mode */
        char    from[ 60 ];     /* machine name the user called in from */
        time_t	freshtime;
        char    userid[ 20 ];
        char    realname[ 20 ];
        char    username[ 40 ];
};

struct friends {
        char id[13];
        char exp[15];
};

struct friends_info {
        int uid;
        char exp[15];
};


#define BM_LEN 60

struct boardheader {             /* This structure is used to hold data in */
        char filename[STRLEN];   /* the BOARDS files */
        char owner[STRLEN - BM_LEN];
        char BM[ BM_LEN - 1];
        char flag;
        char title[STRLEN ];
        unsigned level;
        unsigned char accessed[ 12 ];
};

struct fileheader {             /* This structure is used to hold data in */
        char filename[STRLEN];     /* the DIR files */
        char owner[STRLEN-4];
        long ldReadCount;     /* 阅读计数 Luzi 99/01/13 减少了owner 4个字节 */
        char title[STRLEN];
        unsigned level;
        unsigned char accessed[ 12 ];   /* struct size = 256 bytes */
} ;
typedef struct fileheader fileheader;

struct shortfile {               /* used for caching files and boards */
        char filename[STRLEN];      /* also will do for mail directories */
        char owner[STRLEN - BM_LEN];
        char BM[ BM_LEN - 1];
        char flag;
        char title[STRLEN];
        unsigned level;
        unsigned char accessed;
};

struct one_key {                  /* Used to pass commands to the readmenu */
        int key ;
        int (*fptr)() ;
} ;


#include "uhashgen.h"

#define USHM_SIZE       (MAXACTIVE + 10) /*modified by dong, 10->20, 1999.9.15*/
#define UTMP_HASHSIZE  (USHM_SIZE*4)
/* modified back by KCN,20->10, because not reboot */
struct UTMPFILE {
    struct user_info    uinfo[ USHM_SIZE ];
    int next[USHM_SIZE];
    int hashhead[UTMP_HASHSIZE+1]; /* use UCACHE_HASHSIZE/32 */
    int number;
/*
    int listhead;
    int list_prev[USHM_SIZE];  /* sorted list prev ptr
    int list_next[USHM_SIZE];  /* sorted list next ptr
*/
    time_t              uptime;
};

struct BCACHE {
    struct shortfile    bcache[ MAXBOARD ];
    int         number;
    time_t      uptime;
};


struct UCACHE {
        ucache_hashtable hashtable;
        ucache_hashtable hashusage;
	int	hashhead[UCACHE_HASHSIZE+1];
	int     next[MAXUSERS];
	time_t  uptime;
	int	number;
};

