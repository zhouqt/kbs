/* define variable used by whole project */

extern struct UTMPFILE *utmpshm;
extern struct UCACHE *uidshm;
extern int utmpent;
extern struct userec* currentuser;

extern struct friends_info *topfriend;

extern int     nf;

extern int scrint ;               /* Set when screen has been initialized */
                                  /* Used by namecomplete *only* */

extern int digestmode;            /*To control Digestmode*/
extern struct user_info uinfo ;   /* Ditto above...utmp entry is stored here
                                     and written back to the utmp file when
                                     necessary (pretty darn often). */ 
extern int usernum ;      /* Index into passwds file user record */
extern int utmpent ;      /* Index into this users utmp file entry */
extern int count_friends,count_users; /*Add by SmallPig for count users and friends*/

extern int t_lines, t_columns;    /* Screen size / width */
extern struct userec lookupuser ; /* Used when searching for other user info */


extern int         nettyNN;
extern char netty_path[] ; /* 纪念本站创始人之一  netty */
extern char netty_board[] ; /* 纪念本站创始人之一  netty */
extern char currboard[] ; /* name of currently selected board */
extern char currBM[] ;    /* BM of currently selected board */

extern int selboard ;           /* THis flag is true if above is active */

extern char genbuf[1024] ;      /* generally used global buffer */

extern struct commands cmdlist[] ; /* main menu command list */

extern jmp_buf byebye ;        /* Used for exception condition like I/O error*/

extern struct commands xyzlist[] ;   /* These are command lists for all the */
extern struct commands talklist[] ;  /* sub-menus */
extern struct commands maillist[] ;
extern struct commands dellist[] ;
extern struct commands maintlist[] ;

extern char save_title[] ;    /* These are used by the editor when inserting */
extern char save_filename[] ; /* header information */
extern int in_mail ;
extern int dumb_term ;
extern int showansi;

extern char fromhost[IPLEN+1];
extern time_t   login_start_time;

extern struct boardheader *bcache;
extern struct BCACHE *brdshm;
#ifdef BBSMAIN
extern int idle_count;
#endif

struct newpostdata {
    char        *name, *title, *BM;
    char        flag;
    int         pos, total;
    char        unread, zap;
};

extern struct newpostdata *nbrd; /*每个版的信息*/

