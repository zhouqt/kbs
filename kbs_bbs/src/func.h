/* define all function that need to by used in whole project */
#ifndef __FUNC_H__
#define __FUNC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* defined in pass.c */
    int checkpasswd(const char *passwd, const char *test);
    int checkpasswd2(const char *passwd, const struct userec *user);
    int setpasswd(const char *passwd, struct userec *user);


    char *Cdate(time_t clock);  /* 时间 --> 英文 */
    char *Ctime(time_t clock);  /* 时间 转换 成 英文 */
    char *nextword(const char **str, char *buf, int sz);


/* defined in ucache.c */
    struct userec *getuserbynum(int num);
    int getnewuserid2(char *userid);
    unsigned int ucache_hash(const char *userid);       /* hash function export for utmp usage */

    void resolve_ucache();
    void detach_ucache();

    int getuserid(char *userid, int uid);
    void setuserid(int num, const char *userid);        /* 设置user num的id为user id */
    int setuserid2(int num, const char *userid);        /* 设置user num的id为user id,userd使用 */
    int searchuser(const char *userid);
    int getuser(const char *userid, struct userec **lookupuser);        /* 取用户信息 ，返回值在lookupuser里面 */
    char *u_namearray(char buf[][IDLEN + 1], int *pnum, char *tag);
    char *getuserid2(int uid);
    int update_user(struct userec *user, int num, int all);
    int apply_users(int (*fptr) (struct userec *, char *), char *arg);
    int getnewuserid(char *userid);

    int flush_ucache();
    int load_ucache();
    int get_giveupinfo(char *userid, int *basicperm, int s[10][2]);
    void save_giveupinfo(struct userec *lookupuser, int lcount, int s[10][2]);
    int do_after_login(struct userec* user,int unum,int mode);
    int do_after_logout(struct userec* user,struct user_info* uinfo,int unum, int mode);

#if USE_TMPFS==1
void setcachehomefile(char* path,char* user,int unum, char* file);
void init_cachedata(char* userid,int unum);
void flush_cachedata(char* userid);
int clean_cachedata(char* userid,int unum);
#endif

/* 根据tag ,生成 匹配的user id 列表 (针对所有注册用户)*/


/* 
   in bbslog.c 
   
   如果from[0]是一个数字，那么表明这个log的优先级。缺省优先级是 0

*/
    int bbslog(const char *from, const char *fmt, ...);
    int bmlog(char *id, char *boardname, int type, int value);
    int init_bbslog();
    void newbbslog(int type, const char *fmt, ...);

/* defined in utmp.c */
    typedef int (*APPLY_UTMP_FUNC) (struct user_info *, void *, int pos);
    void resolve_utmp();
    void detach_utmp();
    int getnewutmpent(struct user_info *up);
    int real_getnewutmpent(struct user_info *up);
#define CHECK_UENT(uident) ((utmpent > 0 && utmpent <= USHM_SIZE ) && \
			(utmpshm->uinfo[ utmpent - 1 ].uid==(uident)))
#define UPDATE_UTMP(field,entp) { if (CHECK_UENT((entp).uid)) \
			utmpshm->uinfo[ utmpent- 1 ].field=(entp).field; }
#define UPDATE_UTMP_STR(field,entp) { if (CHECK_UENT((entp).uid)) \
			strcpy(utmpshm->uinfo[ utmpent- 1 ].field,(entp).field); }
    int search_ulist(struct user_info *uentp, int (*fptr) (int, struct user_info *), int farg); /* ulist 中 search 符合fptr函数的 user */
#if 0
    void clear_utmp2(struct user_info *uentp);
#endif
    void clear_utmp2(int uentp);
    void clear_utmp(int uentp, int useridx, int pid);   /*先锁住utmp调用clear_utmp2,并校验useridx */
    int apply_ulist(APPLY_UTMP_FUNC fptr, void *arg);
    int apply_utmpuid(APPLY_UTMP_FUNC fptr, int uid, void *arg);
    int apply_ulist_addr(APPLY_UTMP_FUNC fptr, void *arg);      /* apply func on user list */
    int get_utmp_number();      /* return active user */
    struct user_info *get_utmpent(int utmpnum);
    int get_utmpent_num(struct user_info *uent);        /* return utmp number */

    int t_cmpuids(int uid, struct user_info *up);
    int apply_utmp(APPLY_UTMP_FUNC fptr, int maxcount, char *userid, void *arg);
    int getfriendstr(struct userec *user, struct user_info *puinfo);
    int myfriend(int uid, char *fexp);
    bool hisfriend(int uid, struct user_info *him);

/* defined in newio.c */
    void output(char *s, int len);

/* defined in stuff.c */
    void save_maxuser();
    void detach_publicshm();
    int multilogin_user(struct userec *user, int usernum, int mode);
    time_t get_exit_time(char *id, char *exittime);
    int dashf(char *fname);
    int dashd(char *fname);
    int seek_in_file(char filename[STRLEN], char seekstr[STRLEN]);
    char *setbdir(int digestmode, char *buf, char *boardname);
    int my_system(const char *cmdstring);
    char *modestring(int mode, int towho, int complete, char *chatid);
    int countexp(struct userec *udata);
    int countperf(struct userec *udata);
    int compute_user_value(struct userec *urec);
    void *attach_shm(char *shmstr, int defaultkey, int shmsize, int *iscreate);
    void *attach_shm1(char *shmstr, int defaultkey, int shmsize, int *iscreate, int readonly, void *shmaddr);
    void attach_err(int shmkey, char *name);
    int id_invalid(char *userid);
    int addtofile(char filename[STRLEN], char str[STRLEN]);
    void bbssettime(time_t t);
    time_t bbstime(time_t * t);
    int getwwwguestcount();
    int del_from_file(char filename[STRLEN], char str[STRLEN]);
    int setpublicshmreadonly(int readonly);
    struct public_data *get_publicshm();        /*获得public shm指针 */
    char *sethomefile(char *buf, const char *userid, const char *filename);     /*取某用户文件 路径 */
    char *sethomepath(char *buf, char *userid); /* 取 某用户 的home */

    char *setmailfile(char *buf, const char *userid, const char *filename);     /* 取某用户mail文件 路径 */
    char *setmailpath(char *buf, char *userid); /* 取 某用户 的mail */
    char *setbpath(char *buf, char *boardname); /* 取某版 路径 */
    char *setbfile(char *buf, char *boardname, char *filename); /* 取某版下文件 */
    void RemoveMsgCountFile(char *userID);
    int bad_user_id(char *userid);      /* 检查.badname是否允许注册的 */
    int valid_ident(char *ident);       /* 检查合法的ident */
    struct user_info *t_search();
    int getunifopid();
    int cmpinames(const char *userid, const char *uv);
    int cmpfnames(char *userid, struct friends *uv);
    int cmpfileinfoname(char *filename, struct fileheader *fi);
	int cmpfileid(int *id, struct fileheader *fi);
    int dodaemon(char *daemonname, bool single, bool closefd);

    int canIsend2(struct userec *user, char *userid);
    void sigbus(int signo);
    void encodestr(register char *str);
    int Isspace(char ch);
    char *idle_str(struct user_info *uent);
    int read_userdata(const char *userid, struct userdata *ud);
    int write_userdata(const char *userid, struct userdata *ud);
    void getuinfo(FILE * fn, struct userec *ptr_urec);
    int simplepasswd(char *str);
    void logattempt(char *uid, char *frm);
    int check_ban_IP(char *IP, char *buf);
    int is_valid_date(int year, int month, int day);
    int valid_filename(char *file, int use_subdir);
    int cmpuids2(int unum, struct user_info *urec);
    unsigned int load_mailbox_prop(char *userid);
    unsigned int store_mailbox_prop(char *userid);
    unsigned int get_mailbox_prop(char *userid);
    unsigned int update_mailbox_prop(char *userid, unsigned int prop);
	int gen_title(char *boardname );
	size_t read_user_memo( char *userid, struct usermemo ** ppum );

#define time(x) bbstime(x)

#define BBS_TRY \
    if (!sigsetjmp(bus_jump, 1)) { \
        signal(SIGBUS, sigbus);

#define BBS_CATCH \
    } \
    else { \

#define BBS_END } \
    signal(SIGBUS, SIG_IGN);

#define BBS_RETURN(x) {signal(SIGBUS, SIG_IGN);return (x);}
#define BBS_RETURN_VOID {signal(SIGBUS, SIG_IGN);return;}

    int safe_mmapfile(char *filename, int openflag, int prot, int flag, void **ret_ptr, size_t * size, int *ret_fd);
    int safe_mmapfile_handle(int fd, int openflag, int prot, int flag, void **ret_ptr, size_t * size);
    void end_mmapfile(void *ptr, int size, int fd);
    void set_proc_title(char *argv0, char *title);

/* define in bcache.c */
    int getbnum(char *bname);
    void resolve_boards();
    int get_boardcount();
    struct boardheader *getbcache(char *bname);
    int normal_board(char *bname);
    int getboardnum(char *bname, struct boardheader *bh);       /* board name --> board No. & not check level */

    int add_board(struct boardheader *newboard);
    void build_board_structure(const char *board);
    int apply_boards(int (*func) (struct boardheader*,void*),void* arg);   /* 对所有版 应用 func函数 */
    int delete_board(char *boardname, char *title);     /* delete board entry */
    struct boardheader const *getboard(int num);
    int set_board(int bid, struct boardheader *board, struct boardheader *oldbh);
    struct BoardStatus *getbstatus(int index);  /* 获得版面的发文状态结构 */
    int updatelastpost(char *board);
    int setboardmark(char *board, int i);
    int setboardorigin(char *board, int i);
    int setboardtitle(char *board, int i);
    int board_setreadonly(char *board, int readonly);   /* 设置版面只读属性 */
    int get_nextid(char *boardname);    /*生成文章索引号并自动加一 */
#if HAVE_WWW==1
    int resolve_guest_table(); /* www guest shm */
    int www_guest_lock();
    void www_guest_unlock(int fd);
#endif

/* define in boards.c */
	int valid_brdname(char *brd);
    void detach_boards();
    int anonymousboard(char *board);
    int load_boards(struct newpostdata *nbrd, char *boardprefix, int group, int pos, int len, bool sort, bool yank_flag, char **input_namelist);
#if USE_TMPFS==1
    void init_brc_cache(char* userid,bool replace);
#endif

    void brc_clear_new_flag(unsigned fid);      /* 清除版面的到这篇文章未读标记 */

    int getfavnum();
    void save_zapbuf();
    void addFavBoard(int);
    void addFavBoardDir(int, char *);
    void release_favboard();
    int changeFavBoardDir(int i, char *s);
    int ExistFavBoard(int idx);
    void load_favboard(int dohelp);
    void save_favboard();
    void save_userfile(char *fname, int blknum, char *buf);
    int IsFavBoard(int idx);
    int MoveFavBoard(int p, int q, int fav_father);
    int DelFavBoard(int i);
    int SetFav(int i);

    int brc_initial(char *userid, char *boardname);
    char *brc_putrecord(char *ptr, char *name, int num, int *list);
    int fav_loaddata(struct newpostdata *nbrd, int favnow, int pos, int len, bool sort, char **input_namelist);
    /*
     * 保存一个版的brclist 
     */
    void brc_update(char *userid);      /* 保存当前的brclist到用户的.boardrc */
    void brc_add_read(unsigned int fid);
    void brc_addreaddirectly(char *userid, int bnum, unsigned int fid);
    void brc_clear();
    int brc_unread(unsigned int fid);
    int junkboard(char *currboard);     /* 判断是否为 junkboards */
    int checkreadonly(char *board);     /* 判断是不是只读版面 */
    int deny_me(char *user, char *board);       /* 判断用户 是否被禁止在当前版发文章 */
    int haspostperm(struct userec *user, char *bname);  /* 判断在 bname版 是否有post权 */
    int chk_BM_instr(const char BMstr[STRLEN - 1], const char bmname[IDLEN + 2]);       /*查询字符串中是否包含 bmname */
    int chk_currBM(const char BMstr[STRLEN - 1], struct userec *user);  /* 根据输入的版主名单 判断user是否有版主 权限 */
    int deldeny(struct userec *user, char *board, char *uident, int notice_only);       /* 删除 禁止POST用户 */
    int check_read_perm(struct userec *user, const struct boardheader *board);
    int check_see_perm(struct userec *user, const struct boardheader *board);
    int is_outgo_board(char *board);
    int is_emailpost_board(char *board);

/* define in article.c */
	/* Search_Bin 
	 * 功能：依据key, 对ptr传入的.DIR索引进行二分查找
	 */
	int Search_Bin(char *ptr, int key, int start, int end);
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
    int get_effsize(char * ffn);
	

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

	int get_threads_from_id(const char *filename, int id, fileheader_t *buf, int num);

    int Origin2(char text[256]);
/*加入edit mark*/
    int add_edit_mark(char *fname, int mode, char *title);
    int get_postfilename(char *filename, char *direct, int use_subdir);
    int mail_file(char *fromid, char *tmpfile, char *userid, char *title, int unlink, struct fileheader *fh);
    int mail_file_sent(char *fromid, char *tmpfile, char *userid, char *title, int unlink);     /*peregrine */
    int update_user_usedspace(int delta, struct userec *user);
    int getmailnum(char *recmaildir);
    int isowner(struct userec *user, struct fileheader *fileinfo);
    int do_del_post(struct userec *user, int ent, struct fileheader *fileinfo, char *direct, char *board, int digestmode, int decpost);
    /*
     * 删除文章，digestmode定义阅读模式，decpost表示斑竹删除是否减文章数 
     */

    int cmpname(struct fileheader *fhdr, char name[STRLEN]);
/* Haohmaru.99.3.30.比较 某文件名是否和 当前文件 相同 */

    void addsignature(FILE * fp, struct userec *user, int sig); /*增加User的签名挡 */

/*添加  POST 最后一行 的来源*/
    void add_loginfo(char *filepath, struct userec *user, char *currboard, int Anony);

/* 把quote_file复制到filepath (转贴或自动发信)*/
    void getcross(char *filepath, char *quote_file, struct userec *user, int in_mail, char *board, char *title, int Anony, int mode, char *sourceboard);

    void write_header(FILE * fp, struct userec *user, int in_mail, char *board, char *title, int Anony, int mode);

/*写入.post文件供分析*/
    int write_posts(char *id, char *board, char *title);
    void cancelpost(char *board, char *userid, struct fileheader *fh, int owned, int autoappend);
    int outgo_post(struct fileheader *fh, char *board, char *title);
    int after_post(struct userec *user, struct fileheader *fh, char *boardname, struct fileheader *re);
    int post_file(struct userec *user, char *fromboard, char *filename, char *nboard, char *posttitle, int Anony, int mode);
    int post_cross(struct userec *user, char *toboard, char *fromboard, char *title, char *filename, int Anony, int in_mail, char islocal, int mode);   /* (自动生成文件名) 转贴或自动发信 */

/* bad 2002.8.16 */
    int dele_digest(char *dname, char *direc);
    int change_post_flag(char *currBM, struct userec *currentuser, int digestmode, char *currboard, int ent, struct fileheader *fileinfo, char *direct, int flag, int prompt);

/**
 * A function return flag character of an article.
 * 
 * @param ent pointer to fileheader structure of the article
 * @param user pointer to userec structure of the user
 * @param is_bm nonzero for board manager, zero for others
 * @return flag character of the article
 * @author flyriver
 */
    char get_article_flag(struct fileheader *ent, struct userec *user, char *boardname, int is_bm);
    time_t get_posttime(const struct fileheader *fileinfo);
    void set_posttime(struct fileheader *fileinfo);
    char* checkattach(char *buf, long size,long *len,char** attachptr);
/* roy 2003.7.23 */
#ifdef HAVE_USERMONEY
    int get_score(struct userec *user);
    int set_score(struct userec *user, int score);
    int add_score(struct userec *user, int score_addition);
    int get_money(struct userec *user);
    int set_money(struct userec *user, int money);
    int add_money(struct userec *user, int money_addition);
#endif

/**
 * 一个能检测attach的fgets
 * 发现attach返回1
 * 文件尾返回-1
 */
    int attach_fgets(char* s,int size,FILE* stream);
    int skip_attach_fgets(char* s,int size,FILE* stream);
    int put_attach(FILE* in, FILE* out, int size);
/* define in record.c */
    int safewrite(int fd, void *buf, int size);
    typedef int (*RECORD_FUNC_ARG) (void *, void *);
    typedef int (*APPLY_FUNC_ARG) (void *, int, void *);
    int delete_record(char *dirname, int size, int ent, RECORD_FUNC_ARG filecheck, void *arg);
    int apply_record(char *filename, APPLY_FUNC_ARG fptr, int size, void *arg, int applycopy, bool reverse);
    int append_record(char *filename, void *record, int size);
    int substitute_record(char *filename, void *rptr, int size, int id);
    int search_record(char *filename, void *rptr, int size, RECORD_FUNC_ARG fptr, void *farg);
    long get_num_records(char *filename, int size);
    long get_sum_records(char *fpath, int size);        /*获得.DIR里面文件大小总数 */
    long get_mailusedspace(struct userec *user, int force);     /*peregrine */
    int get_record_handle(int fd, void *rptr, int size, int id);
    int get_record(char *filename, void *rptr, int size, int id);
    int delete_range(char *filename, int id1, int id2, int del_mode);
    int get_records(char *filename, void *rptr, int size, int id, int number);
    int read_get_records(char *filename, char *filename1, void *rptr, int size, int id, int number);
    int search_record_back(int fd,      /* idx file handle */
                           int size,    /* record size */
                           int start,   /* where to start reverse search */
                           RECORD_FUNC_ARG fptr,        /* compare function */
                           void *farg,  /* additional param to call fptr() / original record */
                           void *rptr,  /* record data buffer to be used for reading idx file */
                           int sorted); /* if records in file are sorted */
    int del_range(int ent, struct fileheader *fileinfo, char *direct, int mailmode);
    void load_mail_list(struct userec *user, struct _mail_list *mail_list);
    void save_mail_list(struct _mail_list *mail_list);


/* define in sysconf.c */
    char *sysconf_str(char *key);
    int sysconf_eval(char *key, int defaultval);
    struct smenuitem *sysconf_getmenu(char *menu_name);
    void build_sysconf(char *configfile, char *imgfile);
    void load_sysconf();
    char *sysconf_relocate(char *data); /*重新定位menuitem里面的字符串 */
    int check_sysconf();

/* libmsg.c */
    int addto_msglist(int utmpnum, char *userid);
    int sendmsgfunc(struct user_info *uentp, const char *msgstr, int mode);
    int canmsg(struct userec *fromuser, struct user_info *uin);
    int can_override(char *userid, char *whoasks);
    int delfrom_msglist(int utmpnum, char *userid);
    int msg_can_sendmsg(char *userid, int utmpnum);
#ifdef SMS_SUPPORT
#if HAVE_MYSQL == 1
int get_sql_smsmsg( struct smsmsg * smdata, char *userid, char *dest, time_t start_time, time_t end_time, int type, 					int level, int start, int num, char *msgtxt, int desc);
int sign_smsmsg_read(int id );
#endif
#endif
int chk_smsmsg(int force );

#if HAVE_MYSQL == 1
int get_sql_al( struct addresslist * smdata, char *userid, char *dest, char *group,int start, int num, int order, char *msgtxt);
int add_sql_al(char *userid, struct addresslist *al, char *msgbuf);
#endif
/* site.c */
    char *ModeType(int mode);
    char *email_domain();
    int get_shmkey(char *s);
    int uleveltochar(char *buf, struct userec *lookupuser);

    int safe_kill(int x, int y);

    void main_bbs(int convit, char *argv);
    void get_mail_limit(struct userec *user, int *sumlimit, int *numlimit);

/* bbs_sendmail.c */
    int chkusermail(struct userec *user);
    int chkreceiver(struct userec *fromuser, struct userec *touser);
    int bbs_sendmail(char *fname, char *title, char *receiver, int isuu, int isbig5, int noansi);
    int check_query_mail(char qry_mail_dir[STRLEN]);
/* convcode.c */
    void conv_init();

/* libann.c */
    void ann_add_item(MENU * pm, ITEM * it);
    int ann_load_directory(MENU * pm);
    ITEM *ann_alloc_items(size_t num);
    void ann_free_items(ITEM * it, size_t num);
    void ann_set_items(MENU * pm, ITEM * it, size_t num);
    int ann_get_board(char *path, char *board, size_t len);
    int ann_get_path(char *board, char *path, size_t len);
	int valid_fname(char *str);
    void a_additem(MENU* pm,char* title,char* fname,char* host,int port,long attachpos);    /* 产生ITEM object,并初始化 */
    int a_loadnames(MENU* pm);             /* 装入 .Names */
    int a_savenames(MENU* pm);             /*保存当前MENU到 .Names */
    void a_freenames(MENU * pm);
    int save_import_path(char ** i_path,char ** i_title,int * i_path_time );
	void load_import_path(char ** i_path,char ** i_title, int * i_path_time,int * i_path_select);
	void free_import_path(char ** i_path,char ** i_title,int * i_path_time);
	int linkto(char *path, char *fname, char *title);
	int add_grp(char group[STRLEN], char bname[STRLEN], char title[STRLEN], char gname[STRLEN]);

/* check the user's access for the path
  return < 0 deny access
  return ==0 has access and it can be access by any body
  return >0 need some extra permission to access it
*/
    int ann_traverse_check(char *path, struct userec *user);
    /*
     * in site.c 
     */
    int ann_get_postfilename(char *filename, struct fileheader *fileinfo, MENU * pm);

	/* in libtmpl.c */
int orig_tmpl_init(char * board, int mode, struct a_template ** ptemp);
int orig_tmpl_free(struct a_template ** pptemp, int temp_num);
int orig_tmpl_save(struct a_template * ptemp, int temp_num, char *board);

/* zmodem */
    int zsend_file(char *filename, char *title);
#define FILENAME2POSTTIME(x) (atoi(((char*)x)+2))

/* filter */
    int check_badword(char *checkfile);
    int check_filter(char *patternfile, char *checkfile, int defaultval);
    int check_badword_str(char *string, int str_len);
    
/* register */
#ifdef HAVE_TSINGHUA_INFO_REGISTER
int auto_register(char* userid,char* email,int msize);
#endif

/* libc */
#ifndef HAVE_MEMMEM
void *memmem(register const void *s, size_t slen, register const void *p, 
		size_t plen);
#endif /* ! HAVE_MEMMEM */

/* calltime.c */
time_t calc_calltime(int mode);

#ifdef SMS_SUPPORT
void uid2smsnumber(struct user_info* uin,char* number);
int smsnumber2uid(byte number[4]);
unsigned int byte2long(byte arg[4]);
void long2byte(unsigned int num, byte* arg);
#endif

#ifdef __cplusplus
}
#endif
#endif
