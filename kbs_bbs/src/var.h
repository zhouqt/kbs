#ifndef GLOBAL_VAR_H
#define GLOBAL_VAR_H

struct UTMPHEAD {
    int next[USHM_SIZE];
    int hashhead[UTMP_HASHSIZE + 1];    /* use UCACHE_HASHSIZE/32 */
    int number;
    int listhead;
    int list_prev[USHM_SIZE];   /* sorted list prev ptr */
    int list_next[USHM_SIZE];   /* sorted list next ptr */
    time_t uptime;
};

#ifndef SMTH_API
#define SMTH_API extern
#endif

struct newpostdata {            
    char dir;    /* added by bad  0-board 1-board directory 2-mail 3-function */
    const char *name, *title, *BM;
    unsigned int flag;
    int pos; /*如果是版面，这个是版面的bcache位置,如果是收藏夹，是收藏家的flag*/
    int total, tag;
    int currentusers;
    char unread, zap;
    int lastpost;
    int (*fptr) ();
};

/* global unique variable */
SMTH_API struct boardheader *bcache;
SMTH_API struct BCACHE *brdshm;
extern struct BDIRCACHE	*bdirshm;
SMTH_API struct UTMPFILE *utmpshm;
extern struct UTMPHEAD *utmphead;
extern int WORDBOUND, WHOLELINE, NOUPPER, INVERSE, FILENAMEONLY, SILENT, FNAME;
extern int ONLYCOUNT;
extern struct UCACHE *uidshm;

extern struct WWW_GUEST_TABLE *wwwguest_shm;


/* in log.c */
extern int bdoatexit;
extern int disablelog;
extern int logmsqid;

/* in sysconf.c */
extern struct smenuitem *menuitem;
extern int sysconf_menu;

extern struct public_data *publicshm;

/* some other global
00000000 l     O .bss   00000004 sysconf_ptr
0000000c l     O .bss   00000004 sysconf_len
00000004 l     O .bss   00000004 sysconf_buf
00000008 l     O .bss   00000004 sysconf_key
00000010 l     O .bss   00000004 sysconf_diff
00000014 l     O .bss   00000004 sysconf_size
00000000 l     O .bss   00000004 badword_img
00000004 l     O .bss   00000004 badimg_time
00000008 l     O .bss   00000004 badword_img_size
*/
struct _sigjmp_stack {
    sigjmp_buf bus_jump;
    struct _sigjmp_stack* next;
};

typedef struct {
    struct userec *currentuser;
    int currentuid;
    struct usermemo *currentmemo;
    struct friends_info* topfriend;
    
    char fromhost[IPLEN + 1];

    struct favbrd_struct *favbrd_list;
    int *favbrd_list_count;
    struct favbrd_struct mybrd_list[FAVBOARDNUM];
    int mybrd_list_t;
    int favnow;
	int nowfavmode; //used by web
    
    int *zapbuf;
    int zapbuf_changed;

    char MsgDesUid[20];
    char msgerr[255];
    
    int  num_of_matched;
    int total_line;
    char *CurrentFileName;
    
     struct _sigjmp_stack *sigjmp_stack;
#ifdef SMS_SUPPORT
    int lastsmsstatus;
    struct sms_shm_head* head;
    void * smsbuf;
    int smsresult;
    struct user_info * smsuin;
#endif

#ifdef HAVE_BRC_CONTROL
    struct _brc_cache_entry* brc_cache_entry;
    int brc_currcache;
#endif

    char gb2big_savec[2];
    char big2gb_savec[2];

	char strbuf[STRLEN];

	int utmpent;
    struct user_info *currentuinfo;
} session_t;

#ifndef THREADSAFE
#define getCurrentUser() (getSession()->currentuser)
#define setCurrentUser(x) (getSession()->currentuser=x)
#endif
#endif
