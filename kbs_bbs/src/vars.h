#ifndef KBS_VARS_H_INCLUDED
#define KBS_VARS_H_INCLUDED

/* define variable used by whole project */

#ifndef SMTH_API
#define SMTH_API extern
#endif
SMTH_API struct UTMPFILE *utmpshm;
/*extern struct UCACHE *uidshm;*/
SMTH_API struct userec *currentuser;
//extern struct userdata curruserdata;
SMTH_API struct friends_info *topfriend;

SMTH_API int scrint;              /* Set when screen has been initialized */

                                  /* Used by namecomplete *only* */

extern struct user_info uinfo;  /* Ditto above...utmp entry is stored here
                                   and written back to the utmp file when
                                   necessary (pretty darn often). */
extern int count_friends, count_users;  /*Add by SmallPig for count users and friends */

extern int t_lines, t_columns;  /* Screen size / width */
extern struct userec lookupuser;        /* Used when searching for other user info */

extern struct _mail_list user_mail_list;
extern int nettyNN;
extern char netty_board[];      /* 纪念本站创始人之一  netty */
extern struct boardheader* currboard;        /* name of currently selected board */
extern int currboardent;
extern char currBM[];           /* BM of currently selected board */

extern char genbuf[1024];       /* generally used global buffer */

extern jmp_buf byebye;          /* Used for exception condition like I/O error */

extern char save_title[];       /* These are used by the editor when inserting */
extern int in_mail;
extern int dumb_term;
extern int showansi;

extern time_t login_start_time;

SMTH_API struct boardheader *bcache;
SMTH_API struct BCACHE *brdshm;
SMTH_API struct BDIRCACHE *bdirshm;

extern int idle_count;



extern struct WWW_GUEST_TABLE *wwwguest_shm;


#ifdef SMS_SUPPORT
extern int smsresult;
extern void* smsbuf;
#endif
extern const char secname[SECNUM][2][20];

SMTH_API int msg_count;

#endif /* ! KBS_VARS_H_INCLUDED */

