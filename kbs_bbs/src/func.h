/* define all function that need to by used in whole project */
#ifndef __FUNC_H__
#define __FUNC_H__
#if HAVE_MYSQL_SMTH == 1
#include <mysql.h>
void mysql_report_error(MYSQL *s);
#endif

#ifdef __cplusplus
extern "C" {
#endif

	char *showuserip(struct userec *user, char *ip);
    char *c_exp(int exp);
    char *c_perf(int perf);
	int def_list(long long XX);

int query_super_filter_mmap(struct fileheader *allfh, int start, int total, int down, struct super_filter_query_arg *q_arg);
int query_super_filter(int fd, struct super_filter_query_arg *q_arg);

char *gb2big(char *s, int *plen, int inst, session_t* session);
char *big2gb( char *s, int *plen, int inst, session_t* session);

/* defined in pass.c */
    int checkpasswd(const char *passwd, const char *test);
    int checkpasswd2(const char *passwd, const struct userec *user);
    int setpasswd(const char *passwd, struct userec *user);


    char *Cdate(time_t clock);  /* 时间 --> 英文 */
    char *Ctime(time_t clock);  /* 时间 转换 成 英文 */
    char *nextword(const char **str, char *buf, int sz);


/* defined in ucache.c */
    int lock_user(char* userid);
    void unlock_user(int fd);

    void load_user_title(ARG_VOID);
    struct userec *getuserbynum(int num);
    int getnewuserid2(char *userid);
    unsigned int ucache_hash(const char *userid);       /* hash function export for utmp usage */

    int resolve_ucache(ARG_VOID);
    void detach_ucache(ARG_VOID);

    int getuserid(char *userid, int uid);
    void setuserid(int num, const char *userid);        /* 设置user num的id为user id */
    int setuserid2(int num, const char *userid);        /* 设置user num的id为user id,userd使用 */
    int searchuser(const char *userid);
    int getuser(const char *userid,struct userec **lookupuser);        /* 取用户信息 ，返回值在lookupuser里面 */
    char *u_namearray(char buf[][IDLEN + 1], int *pnum, char *tag);
    char *getuserid2(int uid);
    int update_user(struct userec *user, int num, int all);
    int apply_users(int(*func)(struct userec*,void*),void *arg);
    int apply_uids(int(*func)(struct userec*,int,void*),void *arg);
    int getnewuserid(char *userid);

    int flush_ucache(ARG_VOID);
    int load_ucache(ARG_VOID);
    int get_giveupinfo(struct userec *user,int s[GIVEUPINFO_PERM_COUNT]);
    int save_giveupinfo(struct userec *user,int s[GIVEUPINFO_PERM_COUNT]);
#ifdef DENYANONY
	int giveup_addpost(char *userid);
#endif
    int do_after_login(struct userec* user,int unum,int mode);
    int do_after_logout(struct userec* user,struct user_info* uinfo,int unum, int mode,int locked);

void setcachehomefile(char* path,const char* user,int unum, char* file);
void init_cachedata(const char* userid,int unum);
void flush_cachedata(const char* userid);
int clean_cachedata(const char* userid,int unum);

char* get_user_title(unsigned char titleidx);
void set_user_title(unsigned char titleidx,char* newtitle);

/* 根据tag ,生成 匹配的user id 列表 (针对所有注册用户)*/


/* 
   in bbslog.c 
   
   如果from[0]是一个数字，那么表明这个log的优先级。缺省优先级是 0

*/
    int bbslog(const char *from, const char *fmt, ...);
#ifdef NEWPOSTLOG
	void newpostlog(const char *userid, const char *boardname, const char *title, int groupid);
#endif
#ifdef NEWBMLOG
int newbmlog(const char *userid, const char *boardname, int type, int value);
#endif
    int bmlog(const char *id, const char *boardname, int type, int value);
    int init_bbslog(ARG_VOID);
    void newbbslog(int type, const char *fmt, ...);

/* defined in utmp.c */
    int getnewutmpent2(struct user_info *up, int is_www);
    typedef int (*APPLY_UTMP_FUNC) (struct user_info *, void *, int pos);
    void resolve_utmp(ARG_VOID);
    void detach_utmp(ARG_VOID);
    int getnewutmpent(struct user_info *up, int is_www);
#define CHECK_UENT(uident) ((getSession()->utmpent > 0 && getSession()->utmpent <= USHM_SIZE ) && \
			(utmpshm->uinfo[ getSession()->utmpent - 1 ].uid==(uident)))
#define UPDATE_UTMP(field,entp) { if (CHECK_UENT((entp).uid)) \
			utmpshm->uinfo[ getSession()->utmpent- 1 ].field=(entp).field; }
#define UPDATE_UTMP_STR(field,entp) { if (CHECK_UENT((entp).uid)) \
			strcpy(utmpshm->uinfo[ getSession()->utmpent- 1 ].field,(entp).field); }
    int search_ulist(struct user_info *uentp, int (*fptr) (int, struct user_info *), int farg); /* ulist 中 search 符合fptr函数的 user */
    void clear_utmp2(int uent);
    void clear_utmp(int uentp, int useridx, int pid);   /*先锁住utmp调用clear_utmp2,并校验useridx */
    int kick_user_utmp(int uid, struct user_info *uentp, int signal);
    int apply_ulist(APPLY_UTMP_FUNC fptr, void *arg);
    int apply_utmpuid(APPLY_UTMP_FUNC fptr, int uid, void *arg);
    int apply_ulist_addr(APPLY_UTMP_FUNC fptr, void *arg);      /* apply func on user list */
    int get_utmp_number(ARG_VOID);      /* return active user */
    struct user_info *get_utmpent(int utmpnum);
    int get_utmpent_num(struct user_info *uent);        /* return utmp number */

    int apply_utmp(APPLY_UTMP_FUNC fptr, int maxcount,const char *userid, void *arg);
    int getfriendstr(struct userec *user, struct user_info *puinfo,session_t* session);
    int myfriend(int uid, char *fexp,session_t* session);
    bool hisfriend(int uid, struct user_info *him);
    int update_username(const char *userid,const char *o_name,const char *n_name);

/* defined in stuff.c */
	int calc_numofsig();
    int add_mailgroup_user(mailgroup_list_t * mgl, int entry, mailgroup_t * users, mailgroup_t * user);
    int delete_mailgroup_user(mailgroup_list_t * mgl, int entry, mailgroup_t * users, int pos);
    int modify_mailgroup_user(mailgroup_t * users, int pos, mailgroup_t * user);
    unsigned int load_mailgroup_list(const char *userid, mailgroup_list_t * mgl);
    int load_mailgroup(const char *userid, const char *group, mailgroup_t * mg, int num);
    int store_mailgroup_list(const char *userid, const mailgroup_list_t * mgl);
    int store_mailgroup(const char *userid, const char *group, const mailgroup_t * mg, int num);
    int import_friends_mailgroup(const char *userid, mailgroup_list_t * mgl);
    int add_mailgroup_item(const char *userid, mailgroup_list_t * mgl, mailgroup_list_item * item);
    int delete_mailgroup_item(const char *userid, mailgroup_list_t * mgl, int entry);
    int add_default_mailgroup_item(const char *userid, mailgroup_list_t * mgl,session_t* session);
    int modify_mailgroup_item(const char *userid, mailgroup_list_t * mgl, int entry, mailgroup_list_item * item);
    int import_old_mailgroup(const char *userid, mailgroup_list_t * mgl);

    int my_unlink(char *fname);/*封装unlink,用于定义备份的文件*/
    int my_f_rm(char *fname);/*封装f_rm,用于定义备份的文件*/
    char* gettmpfilename(char *name,const char *format,...);    /* 在 tmp 目录下散列出临时文件名, 出错时返回 NULL ... */
    int setmailcheck(char *userid); /*设置mail检查标志*/
    void save_maxuser(ARG_VOID);
    void detach_publicshm(ARG_VOID);
    int multilogin_user(struct userec *user, int usernum, int mode);
    time_t get_exit_time(const struct userec *lookupuser, char *exittime);
    int dashf(const char *fname);
    int dashd(const char *fname);
    int seek_in_file(const char* filename, const char* seekstr);
	char *setbdir(enum BBS_DIR_MODE digestmode, char *buf,const  char *boardname);
	void makevdir(const char *bname);
	void setvfile(char *buf, const char *bname, const char *filename);
    int my_system(const char *cmdstring);
    char *modestring(char* buf,int mode, int towho, int complete, char *chatid);
    int countexp(struct userec *udata);
    int countperf(struct userec *udata);
    int compute_user_value(const struct userec *urec);
    void *attach_shm(char *shmstr, int defaultkey, int shmsize, int *iscreate);
    void *attach_shm1(char *shmstr, int defaultkey, int shmsize, int *iscreate, int readonly, void *shmaddr);
    void    remove_shm(char *shmstr, int defaultkey, int shmsize);
    void attach_err(int shmkey, char *name);
    int id_invalid(const char *userid);
    int addtofile(char filename[STRLEN], char str[STRLEN]);
    void bbssettime(time_t t);
    time_t bbstime(time_t * t);
    int getwwwguestcount(ARG_VOID);
    int del_from_file(char filename[STRLEN], char str[STRLEN]);
    int setpublicshmreadonly(int readonly);
    struct public_data *get_publicshm(ARG_VOID);        /*获得public shm指针 */
    char *sethomefile(char *buf, const char *userid, const char *filename);     /*取某用户文件 路径 */
    char *sethomepath(char *buf, const char *userid); /* 取 某用户 的home */

    char *setmailfile(char *buf, const char *userid, const char *filename);     /* 取某用户mail文件 路径 */
    char *setmailpath(char *buf, const char *userid); /* 取 某用户 的mail */
    char *setbpath(char *buf, const char *boardname); /* 取某版 路径 */
    char *setbfile(char *buf,const char *boardname,const char *filename); /* 取某版下文件 */
    void RemoveMsgCountFile(const char *userID);
    int bad_user_id(const char *userid);      /* 检查.badname是否允许注册的 */
    int valid_ident(const char *ident);       /* 检查合法的ident */
    struct user_info *t_search(const char *sid, int pid);
    int cmpinames(const char *userid, const char *uv);
    int cmpfnames(const char *userid, const struct friends *uv);
    int cmpfileinfoname(const char *filename, const struct fileheader *fi);
	int cmpfileid(const int *id, const struct fileheader *fi);
    int dodaemon(char *daemonname, bool single, bool closefd);

    int canIsend2(struct userec *src, const char *userid);
    void sigbus(int signo);
    void encodestr(register char *str);
    int Isspace(char ch);
    char *idle_str(char* buf,struct user_info *uent);
    int read_userdata(const char *userid, struct userdata *ud);
    int write_userdata(const char *userid, struct userdata *ud);
    void getuinfo(FILE * fn, struct userec *ptr_urec);
    int simplepasswd(char *str);
    void logattempt(char *uid, char *frm, char *action);
    int check_ban_IP(char *IP, char *buf);
    int is_valid_date(int year, int month, int day);
    int valid_filename(char *file, int use_subdir);
    int cmpuids2(int unum, struct user_info *urec);
    unsigned int load_mailbox_prop(char *userid);
    unsigned int store_mailbox_prop(char *userid);
    unsigned int get_mailbox_prop(char *userid);
    unsigned int update_mailbox_prop(char *userid, unsigned int prop);
    int gen_title(const char *boardname );
    int read_user_memo( char *userid, struct usermemo ** ppum );
#if HAVE_MYSQL_SMTH == 1
#ifdef BMSLOG
	int bms_add(char *userid, char *boardname, time_t in, int out, char *memo );
	int bms_update(char *userid, char *boardname, time_t in, int out, char *memo );
	int bms_del(char *userid, char *boardname);
#endif
#endif
#ifdef BIRTHFILEPATH
	int mail_birth();
#endif
    int check_ID_lists(char * id);
    int check_ip_acl(char * id, char * sip);
    char* filter_upload_filename(char *s);
    char* process_control_chars(char *s,const char *reserved_chars);
    char* strnzhcpy(char *dest, const char *src, size_t n);
    void trimstr(char*);
    void securityreport(char *str, struct userec *lookupuser, char fdata[7][STRLEN], session_t *session);

#ifndef CYGWIN
#define time(x) bbstime(x)
#endif

    sigjmp_buf* push_sigbus(ARG_VOID);
    void popup_sigbus(ARG_VOID);

#define BBS_TRY                             \
    if(!sigsetjmp(*push_sigbus(),1)){       \
        signal(SIGBUS,sigbus);

#define BBS_CATCH                           \
    }                                       \
    else{

#define BBS_END                             \
    }                                       \
    do{                                     \
        popup_sigbus();                     \
    }                                       \
    while(0)

#define BBS_RETURN(x)                       \
    do{                                     \
        popup_sigbus();                     \
        return (x);                         \
    }                                       \
    while(0)

#define BBS_RETURN_VOID                     \
    do{                                     \
        popup_sigbus();                     \
        return;                             \
    }                                       \
    while(0)

    int safe_mmapfile(char *filename, int openflag, int prot, int flag, char **ret_ptr, off_t * size, int *ret_fd);
    int safe_mmapfile_handle(int fd, int prot, int flag, char **ret_ptr, off_t * size);
    void end_mmapfile(void *ptr, off_t size, int fd);
    void set_proc_title(char *argv0, char *title);

/* 0-1 locks using semaphore, flock and fcntl lock replacement */
/* @author kxn */

void lock_sem(int lockid);
void unlock_sem(int lockid);
void unlock_sem_check(int lockid);

    /* etnlegend, 2006.11.17, 获取动态链接库中的函数指针... */
    void* dl_function(const char *s_library,const char *s_function,void **p_handle);

/* define in bcache.c */
	void flush_bcache(); /* 同步bcache*/
    void board_setcurrentuser(int idx,int num); /*设置在线用户计数*/
    int getbnum_safe(const char *bname, session_t *session);
    void resolve_boards(ARG_VOID);
    int get_boardcount(ARG_VOID);
    const struct boardheader *getbcache(const char *bname);
    int getbid(const char *bname, const struct boardheader **pbh);
    int public_board(const struct boardheader *bh);
    int normal_board(const char *bname);
    int getboardnum(const char *bname, struct boardheader *bh);       /* board name --> board No. & not check level */

    int get_manageable_bids(struct userec *user, char *buf, int buflen);
	int fill_super_board(struct userec* user,const char *searchname, int result[], int max);
    int add_board(struct boardheader *newboard);
    void build_board_structure(const char *board);
    int apply_boards(int (*func) (struct boardheader*,void*),void* arg);   /* 对所有版 应用 func函数 */
    int apply_bids(int (*func) (struct boardheader* bh,int bid, void* arg),void* arg);
    int delete_board(int bid, session_t* session);     /* delete board entry */
    struct boardheader const *getboard(int num);
    int set_board(int bid, struct boardheader *board, struct boardheader *oldbh);
    struct BoardStatus *getbstatus(int index);  /* 获得版面的发文状态结构 */
    int updatelastpost(const char *board);
    int setboardmark(const char *board, int i);
    int setboardorigin(const char *board, int i);
    int setboardtitle(const char *board, int i);
    int board_setreadonly(const char *board, int readonly);   /* 设置版面只读属性 */
    int get_nextid(const char *boardname);    /*生成文章索引号并自动加一 */
	int get_nextid_bid(int bid);
    void board_update_toptitle(int bid,bool needlock); /*改变置顶个数*/

    int add_bm(struct userec *user,const struct boardheader *bh,int pos,int bms_log);
    int del_bm(struct userec *user,const struct boardheader *bh,int pos,int concurrent);
    void process_no_bm(const struct boardheader *bh);

    int resolve_guest_table(ARG_VOID); /* www guest shm */
    int www_guest_lock(ARG_VOID);
    void www_guest_unlock(int fd);

/* define in boards.c */
	int valid_brdname(char *brd);
    void detach_boards(ARG_VOID);
    int anonymousboard(const char *board);
    int load_boards(struct newpostdata *nbrd, const char *boardprefix, int group, int pos, int len, int sort, int yank_flag, const char **input_namelist,session_t* session);


    int getfavnum(session_t* session);
    void save_zapbuf(session_t* session);
	void addFavBoard(int i,session_t* session, int favmode, int favnow);
    void addFavBoardDir(char *,session_t* session);
    int changeFavBoardDir(int i, char *s,session_t* session);
    int changeFavBoardDirEname(int i, char *s,session_t* session);
	int EnameInFav(char *ename,session_t* session);
//    int ExistFavBoard(int idx,session_t* session);
	void load_favboard(int mode,session_t* session);
	void load_myboard(session_t *session, int force);
    void save_favboard(int mode,session_t* session);
    int FavGetFather(int select,session_t* session);
    void save_userfile(char *fname, int blknum, char *buf,session_t* session);
	int IsFavBoard(int idx,session_t* session, int favmode, int favnow);
    int MoveFavBoard(int p, int q,session_t* session);
    int DelFavBoard(int i,session_t* session);
	int DelFavBoardDir(int i,int fath,session_t* session);
    int SetFav(int i,session_t* session);
	char * FavGetTitle(int select,char *title,session_t* session);
	void load_allboard(struct favbrd_struct *brdlist, int * brdlist_t);
	void load_wwwboard(struct favbrd_struct *brdlist, int * brdlist_t);

    int brc_initial(const char *userid, const char *boardname,session_t* session);
    int fav_loaddata(struct newpostdata *nbrd, int favnow, int pos, int len, int sort,const char **input_namelist,session_t* session);

    void init_brc_cache(const char* userid,bool replace, session_t* session);
    void free_brc_cache(char* userid, session_t* session);
    void brc_clear_new_flag(unsigned fid, int bid, session_t* session);      /* 清除版面的到这篇文章未读标记 */
    void brc_update(const char *userid,session_t* session);      /* 保存当前的brclist到用户的.boardrc */
    void brc_add_read(unsigned int fid, int bid, session_t* session);
    void brc_clear(int bid, session_t* session);
    int brc_unread(unsigned int fid, session_t* session);
    int brc_board_unread(int bid, session_t* session);

    int junkboard(const char *currboard);     /* 判断是否为 junkboards */
    int checkreadonly(const char *board);     /* 判断是不是只读版面 */
    int deny_me(const char *user,const char *board);       /* 判断用户 是否被禁止在当前版发文章 */
    int haspostperm(const struct userec *user,const char *bname);  /* 判断在 bname版 是否有post权 */
#ifdef NEWSMTH
    int check_score_level(const struct userec *user,const struct boardheader *bh);
#endif /* NEWSMTH */
    int chk_BM_instr(const char BMstr[STRLEN - 1], const char bmname[IDLEN + 2]);       /*查询字符串中是否包含 bmname */
    int chk_currBM(const char BMstr[STRLEN - 1], const struct userec *user);  /* 根据输入的版主名单 判断user是否有版主 权限 */
    int check_board_delete_read_perm(const struct userec *user,const struct boardheader *board, int jury);
    int deldeny(struct userec *user, char *board, char *uident, int notice_only, session_t* session);       /* 删除 禁止POST用户 */
    int check_read_perm(const struct userec *user, const struct boardheader *board);
    int check_see_perm(const struct userec *user, const struct boardheader *board);
    int is_outgo_board(const char *board);
    int poststatboard(const char *board);
    int is_emailpost_board(const char *board);
	int board_regenspecial(const char *board, int mode, char *index);

    int set_user_club_perm(struct userec *user,const struct boardheader *board,int write_perm);
    int del_user_club_perm(struct userec *user,const struct boardheader *board,int write_perm);
    int get_user_club_perm(const struct userec *user,const struct boardheader *board,int write_perm);
    int club_maintain_send_mail(const char *userid,const char *comment,int type,int write_perm,const struct boardheader *bh,session_t *session);

/* define in article.c */

    /* etnlegend, 2006.04.20, 区段删除核心 */
    int delete_range_base(const char *videntity,const char *vdir_src,const char *vdir_dst,int vid_from,int vid_to,int vmode,
        int (*func)(const char*,struct fileheader*),const struct stat *vst_src);
    int cancel_inn(const char *board,const struct fileheader *file);

    struct write_dir_arg {
      char* filename; /*.dir的文件名*/
      int fd;                 //文件句柄
      struct fileheader* fileptr;   //文件mmap指针
      int ent;               //当前位置
      off_t size;           //文件大小
      bool needclosefd; //释放结构的时候是否需要关闭文件，内部使用
      bool needlock; //是否需要自己lock文件
    };
    void init_write_dir_arg(struct write_dir_arg*filearg);
    int malloc_write_dir_arg(struct write_dir_arg*filearg);
    void free_write_dir_arg(struct write_dir_arg*filearg);

    
    /*加入置顶*/
    int add_top(struct fileheader* fileinfo,const char* boardname,int flag);
    /*推荐文章*/
    int post_commend(struct userec *user, const char *fromboard, struct fileheader *fileinfo);

	/* Search_Bin 
	 * 功能：依据key, 对ptr传入的.DIR索引进行二分查找
	 */
	int Search_Bin(struct fileheader*ptr, int key, int start, int end);

    /*
     * mmap_search_dir_apply
     * 功能:mmap struct fileheader结构的文件，找到需要的记录,
     * 找到后调用func,传入start整个文件mmap的开始，ent为记录号，基 1 计数，
     * 根据文章ID做二分查找
     * 调用的时候
     * match=true,完全匹配
     * match=false,找到该插入的前一条记录。
     * 同时利用index 返回记录的编号。
     */
    typedef int (*DIR_APPLY_FUNC) (int fd, struct fileheader * start, int ent, int total, struct fileheader * data, bool match);
    typedef int (*search_handler_t) (int fd, fileheader_t * base, int ent, int total, bool match, void *arg);
    int mmap_dir_search(int fd, const fileheader_t * key, search_handler_t func, void *arg);
    int mmap_search_apply(int fd, struct fileheader *buf, DIR_APPLY_FUNC func);
    int get_effsize(char * ffn);
    int get_effsize_attach(char *ffn, unsigned int *att);
	

    int ann_article_import(const char *bname, const char *title, const char *fname, const char *userid);
    

/**
 * Get some records from article id. If this function is successfully
 * returned, the article record with the id is put at the center of 
 * buf logically. For example, if the user provided an buffer of three
 * records, then the article record with the id is copied to buf[1],
 * its previous record will be copied to buf[0] (but if the previous
 * record does not exist, buf[0] will be filled with zero), and its
 * next record will be copied to buf[2] (if the next record does not
 * exist, buf[2] will be filled will zero).
 *
 * @param fd The file descriptor of the .DIR file
 * @param id The article id to be searched in the .DIR file
 * @param buf The user provided buffer to hold the matched records
 * @param num The number of records that the buffer can hold
 * @param index The record number of the record corresponding to id
 * @return >0 The number of matched records
 *          0 No record matched or execution failed
 * @author flyriver
 */
    int get_records_from_id(int fd, int id, fileheader_t * buf, int num, int *index);
    int get_ent_from_id(int mode, int id, const char *bname);
    int get_ent_from_id_ext(int mode, int id, const char *bname, fileheader_t *fh);

	int get_threads_from_id(const char *filename, int id, fileheader_t *buf, int num);
	int get_threads_from_gid(const char *filename, int gid, fileheader_t *buf, int num, int start, int * haveprev, int operate, struct userec *user);

    int ann_article_import(const char *bname, const char *title, const char *fname, const char *userid);
    
    int Origin2(char text[256]);
/*加入edit mark*/
    int add_edit_mark(char *fname, int mode, char *title, session_t* session);
    int get_postfilename(char *filename, char *direct, int use_subdir);
    int mail_file(char *fromid, char *tmpfile, char *userid, char *title, int unlink, struct fileheader *fh);
    int mail_file_sent(char *fromid, char *tmpfile, char *userid, char *title, int unlink, session_t* session);     /*peregrine */
    int update_user_usedspace(int delta, struct userec *user);
    int getmailnum(char *recmaildir);
    int isowner(const struct userec *user, const struct fileheader *fileinfo);
    int deny_modify_article(const struct boardheader *bh, const struct fileheader *fileinfo, int mode, session_t* session);
    int deny_del_article(const struct boardheader *bh, const struct fileheader *fileinfo, session_t* session);
    int do_del_post(struct userec *user,struct write_dir_arg *dirarg,struct fileheader *fileinfo, const char *board,int currmode,int flag,session_t* session);
	int do_undel_post(char* boardname, char *dirfname, int num, struct fileheader *fileinfo, char *title, session_t* session);
    int do_del_ding(char *boardname, int bid, int ent, struct fileheader *fh, session_t* session);

    int cmpname(struct fileheader *fhdr, char name[STRLEN]);
/* Haohmaru.99.3.30.比较 某文件名是否和 当前文件 相同 */

    void addsignature(FILE * fp, struct userec *user, int sig); /*增加User的签名挡 */

/*添加  POST 最后一行 的来源*/
    void add_loginfo(char *filepath, struct userec *user, char *currboard, int Anony, session_t* session);

    void write_header(FILE * fp, struct userec *user, int in_mail, const char *board, const char *title, int Anony, int mode, session_t* session);

/*写入.post文件供分析*/
    int write_posts(char *id, const char *board, unsigned int groupid);
    void cancelpost(const char *board, const char *userid, struct fileheader *fh, int owned, int autoappend, session_t* session);
    int outgo_post(struct fileheader *fh, const char *board, const char *title, session_t* session);
    int after_post(struct userec *user, struct fileheader *fh, const char *boardname, struct fileheader *re, int poststat, session_t* session);
    int post_file(struct userec *user, const char *fromboard, const char *filename, const char *nboard, const char *posttitle, int Anony, int mode, session_t* session);
    int post_cross(struct userec *user, const struct boardheader *toboard, const char *fromboard, const char *title, const char *filename, int Anony, int in_mail, char islocal, int mode, session_t* session);   /* (自动生成文件名) 转贴或自动发信 */

    int change_post_flag(struct write_dir_arg* dirarg,int currmode, const struct boardheader*board,
        struct fileheader *fileinfo, int flag,struct fileheader * data,bool dobmlog, session_t* session);


/**
 * A function return flag character of an article.
 * 
 * @param ent pointer to fileheader structure of the article
 * @param user pointer to userec structure of the user
 * @param is_bm nonzero for board manager, zero for others
 * @return flag character of the article
 * @author flyriver
 */
    char get_article_flag(struct fileheader *ent, struct userec *user, const char *boardname, int is_bm, char *common_flag,  session_t* session);
    time_t get_posttime(const struct fileheader *fileinfo);
    void set_posttime(struct fileheader *fileinfo);
    char* checkattach(char *buf, long size,long *len,char** attachptr);

/**
 * 一个能检测attach的fgets
 * 发现attach返回1
 * 文件尾返回-1
 */
    int attach_fgets(char* s,int size,FILE* stream);
    int skip_attach_fgets(char* s,int size,FILE* stream);
    int put_attach(FILE* in, FILE* out, int size);

    const char *get_mime_type_from_ext(const char *ext);
    const char *get_mime_type(const char *filename);
    int get_attachment_type(const char *filename);
    int get_attachment_type_from_ext(const char *ext);

    /* etnlegend - 附件修改核心 */
    long ea_dump(int fd_src,int fd_dst,long offset);
    long ea_locate(int fd,struct ea_attach_info *ai);
    long ea_append(int fd,struct ea_attach_info *ai,const char *fn, const char *original_filename);
    long ea_delete(int fd,struct ea_attach_info *ai,int pos);

    int getattachtmppath(char *buf, size_t buf_len, session_t *session);
    int upload_post_append(FILE *fp, struct fileheader *post_file, session_t *session);
    int upload_read_fileinfo(struct ea_attach_info *ai, session_t *session);
    int upload_del_file(const char *original_file, session_t *session);
    int upload_add_file(const char *filename, char *original_filename, session_t *session);

    
/* define in record.c */
    int safewrite(int fd, void *buf, int size);
    typedef int (*RECORD_FUNC_ARG) (void *, void *);
    typedef int (*APPLY_FUNC_ARG) (void *, int, void *);
    int delete_record(char *dirname, int size, int ent, RECORD_FUNC_ARG filecheck, void *arg);
    int apply_record(char *filename, APPLY_FUNC_ARG fptr, int size, void *arg, int applycopy, bool reverse);
    int append_record(char *filename, void *record, int size);
    int substitute_record(char *filename, void *rptr, int size, int id);
	int move_record(char *filename, int size, int id, int toid, RECORD_FUNC_ARG filecheck, void *arg);
    int search_record_back_lite(int fd, int size, int start, int num, RECORD_FUNC_ARG fptr, void *farg, void *rptr, int sorted);
    int search_record(char *filename, void *rptr, int size, RECORD_FUNC_ARG fptr, void *farg);
    long get_num_records(char *filename, int size);
    long get_sum_records(char *fpath, int size);        /*获得.DIR里面文件大小总数 */
    long get_mailusedspace(struct userec *user, int force);     /*peregrine */
    int get_record_handle(int fd, void *rptr, int size, int id);
    int get_record(char *filename, void *rptr, int size, int id);
    int get_records(char *filename, void *rptr, int size, int id, int number);
    int read_get_records(char *filename, char *filename1, char *rptr, int size, int id, int number);
    int search_record_back(int fd,      /* idx file handle */
                           int size,    /* record size */
                           int start,   /* where to start reverse search */
                           RECORD_FUNC_ARG fptr,        /* compare function */
                           void *farg,  /* additional param to call fptr() / original record */
                           void *rptr,  /* record data buffer to be used for reading idx file */
                           int sorted); /* if records in file are sorted */
    void load_mail_list(struct userec *user, struct _mail_list *mail_list);
    void save_mail_list(struct _mail_list *mail_list,session_t* session);


/* define in sysconf.c */
    const char *sysconf_str(const char *key);
    const char *sysconf_str_default(const char *key, const char *default_value);
    int sysconf_eval(const char *key, int defaultval);
    struct smenuitem *sysconf_getmenu(const char *menu_name);
    void build_sysconf(const char *configfile,const  char *imgfile);
    void load_sysconf(ARG_VOID);
    char* sysconf_relocate(const char *data); /*重新定位menuitem里面的字符串 */
    int check_sysconf(ARG_VOID);

/* libmsg.c */
	void msgmail(char *did, const char *buf) ;
    int get_unreadcount(char *uident);
    int get_unreadmsg(char *uident);
    int load_msghead(int id, char *uident, int index, struct msghead *head);
    int load_msgtext(char *uident, struct msghead *head, char *msgbuf);
    int translate_msg(char* src, struct msghead *head, char* dest,session_t* session);
    int get_msgcount(int id, char *uident);
    void mail_msg(struct userec* user,session_t* session);
    int clear_msg(char *uident);
    int sendmsgfunc(struct user_info *uentp, const char *msgstr, int mode, int srcpid, session_t* session);
    int canmsg(struct userec *fromuser, struct user_info *uin);
    int can_override(char *userid, char *whoasks);
    int msg_can_sendmsg(struct userec* user,char *userid, int utmpnum);
    int receive_webmsg(int destutmp, char *destid, int *srcpid, char *srcid, time_t *sndtime, char *msg);
    int conv_csv_to_al(char *fname,session_t * session);
#ifdef SMS_SUPPORT
#if HAVE_MYSQL_SMTH == 1
int get_sql_smsmsg( struct smsmsg * smdata, char *userid, char *dest, time_t start_time, time_t end_time, int type, 					int level, int start, int num, char *msgtxt, int desc, session_t* session);
int sign_smsmsg_read(int id , session_t* session);
#endif
int DoReplyCheck(char * n, unsigned int sn, char isSucceed,session_t * session);
int sms_init_memory(session_t* session);
int DoUnReg(char * n, session_t* session);
int DoSendSMS(char * n, char * d, char * c, session_t* session);
int save_msgtext(char *uident, struct msghead * head,const char *msgbuf, session_t* session);
int save_smsmsg(char *uident, struct msghead *head, char *msgbuf, int readed, session_t* session);
int count_sql_smsmsg( char *userid, char *dest, time_t start_time, time_t end_time, int type, int level, char *msgtxt , session_t* session);
int chk_smsmsg(int force , session_t* session);
#endif

#if HAVE_MYSQL_SMTH == 1
char * get_al_mobile( char *userid, char *mobile,session_t* session);
int get_sql_al( struct addresslist * smdata, char *userid, char *dest, char *group,int start, int num, int order, char *msgtxt);
int add_sql_al(char *userid, struct addresslist *al, char *msgbuf);
int count_sql_al( char *userid, char *dest, char *group, char *msgtxt);
MYSQL * my_connect_mysql(MYSQL *s);
MYSQL * my_connect_mysql_blog(MYSQL *s);
MYSQL * my_connect_mysql_dict(MYSQL *s);
#endif

char * tt2timestamp( time_t tt, char *c);

#ifdef PERSONAL_CORP
int pc_conv_file_to_body( char **body, char *fname);
int pc_load_usr( struct _pc_selusr **ps, char prefix);
int get_pc_users( struct pc_users * pu, char * userid );
int get_pc_a_node( struct pc_nodes * pn, unsigned long nid);
int count_pc_nodes( int uid, unsigned long pid, int type, int access );
int get_pc_nodes( struct pc_nodes * pn, int uid, unsigned long pid, int type, int access, int start, int num, int withbody);
int get_pc_a_com( struct pc_comments * pn, unsigned long cid );
int count_pc_comments( unsigned long nid);
int get_pc_comments( struct pc_comments * pn, unsigned long nid, int start, int num, int withbody);
int add_pc_users(struct pc_users *pn);
int add_pc_nodes(struct pc_nodes *pn);
int pc_in_blacklist(char * userid , unsigned long pcuid);
int add_pc_comments(struct pc_comments *pn);
int del_pc_users(struct pc_users *pn);
int del_pc_nodes( unsigned long nid , int access , int uid );
int pc_del_junk(int uid);
int del_pc_node_junk(unsigned int nid , int access , int uid );
int del_pc_comments( unsigned long nid, unsigned long cid );
int pc_paste_node(unsigned long nid, int newuid, int newaccess, unsigned long newpid ) ;
int pc_add_visitcount(unsigned long nid);
int pc_add_user(void);
int import_to_pc(int ent, struct fileheader *fileinfo, char *direct);

int pc_read(char *userid);
int pc_logs(struct pc_logs *pn);

#endif
/* site.c */
    void set_posttime2(struct fileheader *dest, struct fileheader *src);
    char *ModeType(int mode);
    const char *email_domain(ARG_VOID);
    int uleveltochar(char *buf, struct userec *lookupuser);

    int safe_kill(int x, int y);

    void get_mail_limit(struct userec *user, int *sumlimit, int *numlimit);

/* bbs_sendmail.c */
    int mail_buf(struct userec*fromuser, char *mail_buf, char *userid, char *title, session_t* session);
    int chkusermail(struct userec *user);
    int check_mail_perm(struct userec *fromuser, struct userec *touser);
    int bbs_sendmail(char *fname, char *title, char *receiver, int isbig5, int noansi,session_t* session);
    int check_query_mail(char *qry_mail_dir, int *total_mail);
/* convcode.c */
    void conv_init(session_t* session);
    char *gb2big(char *s, int *plen, int inst, session_t* session);
    char *big2gb( char *s, int *plen, int inst, session_t* session);

/* libann.c */
#ifdef FB2KPC
	int fb2kpc_is_owner(char *path);
#endif
    unsigned int check_ann(const struct boardheader* bh);
    int ann_get_board(char *path, char *board, size_t len);
    int ann_get_path(char *board, char *path, size_t len);
	int valid_fname(char *str);
    int a_additem(MENU *pm,const char *title,const char *fname,char *host,int port,long attachpos);    /* 产生ITEM object,并初始化 */
    int a_delitem(MENU *pm,int index);
    int a_loadnames(MENU *pm, session_t *session);             /* 装入 .Names */
    int a_savenames(MENU *pm);             /*保存当前MENU到 .Names */
    void a_freenames(MENU *pm);
    int a_sort_items(MENU *pm,enum ANN_SORT_MODE mode,session_t *session);
    int save_import_path(char ** i_path,char ** i_title,time_t* i_path_time, session_t* session );
    void load_import_path(char ** i_path,char ** i_title, time_t* i_path_time,int * i_path_select, session_t* session);
    void free_import_path(char ** i_path,char ** i_title,time_t* i_path_time);
    int edit_group(const struct boardheader* oldbh, const struct boardheader* newbh);

    int a_SeSave(char *path, const char *key, struct fileheader *fileinfo, bool append, char *direct, int ent,int mode, char *userid);
    int a_Save(char *path, const char *key, struct fileheader *fileinfo, bool append, char *direct, int ent, char *userid);

    long a_append_attachment(char *fpath, char *attachpath);

/* check the user's access for the path
  return < 0 deny access
  return ==0 has access and it can be access by any body
  return >0 need some extra permission to access it
*/
    int ann_traverse_check(char *path, struct userec *user);
    char *ann_numtopath(char *path, char *numpath, struct userec *user);
    /*
     * in site.c 
     */
    int ann_get_postfilename(char *filename, struct fileheader *fileinfo, MENU * pm);

	/* in libtmpl.c */
int orig_tmpl_init(char * board, int mode, struct a_template ** ptemp);
int orig_tmpl_free(struct a_template ** pptemp, int temp_num);
int orig_tmpl_save(struct a_template * ptemp, int temp_num, char *board);

#define FILENAME2POSTTIME(x) (atoi(((char*)x)+2))

/* filter */
    int check_badword(char *checkfile, int len, session_t* session);
    int check_filter(char *patternfile, char *checkfile, int defaultval, session_t* session);
    int check_badword_str(char *string, int str_len, session_t* session);
    
/* libc */
#ifndef HAVE_MEMMEM
void *memmem(register const void *s, size_t slen, register const void *p, 
		size_t plen);
#endif /* ! HAVE_MEMMEM */

/* xml.c */
char *encode_xml(char *buf, const char *str, size_t buflen);
char *string_copy(char *buf, const char *str, size_t * buflen);


#ifdef SMS_SUPPORT
int smsid2uid(char* smsid);
void uid2smsid(struct user_info* uin,char* smsid);
int uid2smsnumber(struct user_info* uin);
int smsnumber2uid(byte number[4]);
unsigned int byte2long(byte arg[4]);
void long2byte(unsigned int num, byte* arg);
#endif

/* lvar.c*/
#ifndef THREADSAFE

#ifdef CYGWIN
session_t* getSession();
#else
extern session_t g_session;
#define getSession() (&g_session)
#endif

#else
session_t * getSession();
#endif
void init_sessiondata(session_t* session);
int init_all(ARG_VOID);
#ifdef __cplusplus
}
#endif
#endif
