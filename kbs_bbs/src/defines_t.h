/*  这个文件是用来保存telnet部分的函数和变量声明

*/
#ifndef _DEFINES_T_H_
#define _DEFINES_T_H_

#include "select.h"

int mail_forward(struct _select_def* conf,struct fileheader* data,void* extradata);

#ifdef FLOWBANNER
char * banner_filter(char * text); //filter text, no cur-cmds allowed, text should be \0 ended
void load_site_banner(int init);
void load_board_banner(const char * board);
#endif

/*etnlegend*/
#define WAIT_RETURN do{\
    int ch = igetkey();\
    if (ch=='\r'||ch=='\n') break;\
}while(1)
            

/*screen.c*/
void do_naws(int ln, int col);
void auto_chinese();
int num_noans_chr(const char* str);
void noscroll();
void setfcolor(int i,int j);
void setbcolor(int i);
void resetcolor();
int scr_cols,scr_lns;
void clear_whole_line(int i);
void move(int y, int x);

void clrtobot();
void prints(const char *fmt, ...);
int pressreturn();
void clear();
void refresh();
void clrtoeol();
int pressanykey();
int askyn(char str[STRLEN], int defa);
void saveline(int line, int mode, char *buffer);
void redoscr();
void initscr();
void bell();
void outc(unsigned char c);
void outns(const char *str, int n);
void getyx(int *y, int *x);
void outs(register const char *str);
void printdash(char *mesg);
void scroll();
void rscroll();
void outline(char *s);

int do_gsend(char *userid[], char *title, int num);

/* newio.c */
extern bool enableESC;
int multi_getdata(int line, int col, int maxcol, char *prompt, char *buf, int len, int maxline, int clearlabel, int textmode);
void ochar(char c);
int ask(char *prompt);
void oflush();
int getdata(int line, int col, char *prompt, char *buf, int len, int echo, void *nouse, int clearlabel);
void set_alarm(int set_timeout,int set_timeoutusec, void (*timeout_func) (void *), void *data);
int igetkey();
int check_calltime();
void add_io(int fd, int timeout);
void add_flush(void (*flushfunc) ());
int igetch();
int num_in_buf();
void output(const char *s, int len);
typedef int (*select_func)(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,struct timeval *timeout);
typedef ssize_t (*read_func)(int fd, void *buf, size_t count);

/* xyz.c*/
int modify_user_mode(int mode);
/* zmodem */
int zsend_file(char *filename, char *title);

/* more.c */
int ansimore(char *filename, int promptend);
int ansimore2(char *filename, int promptend, int row, int numlines);
int ansimore_withzmodem( char *filename, int promptend,char* title);
int NNread_init();
void m_init();
int countln(char *fname);
void R_monitor(void *data);
void printacbar();
typedef void  (*generate_attach_link_t)(char* buf,int buf_len,char *ext, int len, long attachpos,void* arg);
void register_attach_link(generate_attach_link_t fn,void* arg);
int mmap_more(char *fn, int quit, char *keystr, char *title);

/* namecomplete.c */
void AddNameList(const char *name);
void CreateNameList(void);
void ApplyToNameList(int (*)(char*,void*),void*);
int namecomplete(char *prompt, char *data);
int usercomplete(char *prompt, char *data);
struct word* GetNameListHead(void);
int GetNameListCount(void);
int MaxLen(struct word*,int);
void SortNameList(int);

/* newmain_single.c */
void Net_Sleep(int times);
void showsysinfo(char * fn);
extern int talkrequest;
int count_user();
int dosearchuser(char *userid);
void showtitle(const char *title, const char *mid);
void abort_bbs(int);
void update_endline();
void docmdtitle(const char *title,const char *prompt);
int switch_code(void);

/* bbs.c*/
int read_hot_info();
int board_query();
#ifdef NEWSMTH
int set_board_rule(struct boardheader *bh, int flag);
int b_rules_edit();
#endif

int i_read_mail();
int add_attach(char* file1, char* file2, char* filename);
int zsend_attach(int ent, struct fileheader *fileinfo, char *direct);
int do_select(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg);
extern int helpmode;
int zsend_post(int ent, struct fileheader *fileinfo, char *direct);
int get_a_boardname(char *bname, char *prompt); /* 输入一个版名 */
int Read();
int ReadBoard(void);
void printutitle();             /* 屏幕显示 用户列表 title */
int check_readonly(char *checked);      /* Leeward 98.03.28 */
void record_exit_time();        /* 记录离线时间  Luzi 1998/10/23 */
int shownotepad(void);
int check_stuffmode();
int del_post(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg);
int do_reply(struct _select_def* conf,struct fileheader *fileinfo);
int do_post(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg);
void make_blist(int addfav);
int digest_post(int ent, struct fileheader *fhdr, char *direct);
int mark_post(int ent, struct fileheader *fhdr, char *direct);
int process_upload(int nUpload, int maxShow, char *ans, struct ea_attach_info* ai);
int post_article(struct _select_def* conf,char *q_file, struct fileheader *re_file);             /*用户 POST 文章 */
int do_send(char *userid, char *title,char* q_file);
int Goodbye(void);
int sequential_read2(int ent);
void do_quote(char *filepath, char quote_mode, char* q_file,char* q_user);
void setqtitle(char *stitle);
void RemoveAppendedSpace(char *ptr);
char *get_my_webdomain(int force);

/*vote.c*/
int b_notes_edit();
int b_banner_edit();
int vote_results(char* bname);
int b_closepolls();
int vote_flag(char *bname, char val, int mode);

/* mail.c */
int MailProc(void);
int m_read(void);
int chkmail();
int getmailnum(char recmaildir[STRLEN]);
int get_mailnum();
int invalidaddr(char *addr);
int doforward(char *direct, struct fileheader *fh);
int set_mailbox_prop();
int set_mailgroup_list();

/* maintain.c */
int check_systempasswd();
void stand_title(char *title);
char* gen_permstr(unsigned int,char*);
int select_user_title(const char*);
void deliverreport(char *title,char *str);
int check_proxy_IP(const char *ip,char *reason);

/* talk.c */
int friend_add();
void creat_list();
int deleteoverride(char *uident);
int num_visible_users();
int num_alcounter();
int ttt_talk(struct user_info *userinfo);
int servicepage(int line, char *mesg);
int addtooverride(char *uident);
int t_query(char* q_id);
int talkreply();
void talk_request();
int num_user_logins(char *uid);
int listfilecontent(char *fname);
int m_send(char* userid);
int m_sendnull(void);   /* m_send的一个封装*/
int t_cmpuids(int uid, struct user_info *up);
int t_override(void);
int t_pager(void);

/* delete.c */
int kick_user(int uid, char *userid, struct user_info *userinfo);
int kick_user_menu(void);
int d_user(char cid[IDLEN]);

/* read.c */
int show_author(int ent, struct fileheader *fileinfo, char *direct);
int show_authorinfo(int ent, struct fileheader *fileinfo, char *direct);
struct keeploc *getkeep(char *s, int def_topline, int def_cursline);
int sread(int passonly, int readfirst, int auser, int pnum, struct fileheader *ptitle);
void fixkeep(char *s, int first, int last);

typedef char *(*READ_FUNC) (void *, int, void *);
int i_read(int cmdmode, char *direct, void (*dotitle) (), READ_FUNC doentry, struct one_key *rcmdlist, int ssize);
void u_exit();

/* sendmsg.c */
#ifdef SMS_SUPPORT
int do_send_sms_func(char * dest, char * msgstr);
#endif
void r_msg();
extern int msg_count;
int s_msg(void);
void r_lastmsg();
int show_allmsgs(void);
int do_sendmsg(struct user_info *uentp, const char msgstr[256], int mode);

/* list.c */
void show_message(char *msg);
void setlistrange(int i);
int t_friends(void);
int fill_userlist();
int choose(int update, int defaultn, int (*title_show) (), int (*key_deal) (), int (*list_show) (), int (*read) ());

/* edit.c */
int vedit(char *filename, int saveheader,long* eff_size,long* pattachpos,int add_loginfo);
int vedit_post(char *filename, int saveheader,long* eff_size,long* pattachpos);
void bbsmain_add_loginfo(FILE *fp, struct userec *user, char *currboard, int Anony);
void keep_fail_post();

/* Announce.c */
void a_prompt(int bot, char *pmt, char *buf);
int a_chkbmfrmpath(char *path );
int a_menusearch(char *path, char *key, int level);
int a_Import(char *path, char *key, struct fileheader *fileinfo, int nomsg, char *direct, int ent);     /* Leeward 98.04.15 */
int set_import_path(char* path);

/* goodbye.c */
void showstuff(char buf[256]);
void user_display(char *filename, int number, int mode);
int countlogouts(char filename[STRLEN]);

/* userinfo.c */
void disply_userinfo(struct userec *u, int real);
int uinfo_query(struct userec *u, int real, int unum);
int modify_userinfo(int uid,int mode);  /* etnlegend, 2006.09.21, 修改用户资料接口... */

/* register.c */
void check_register_info();
void new_register();

/* help.c */
void show_help(char *fname);

/* fileshm.c */
void show_issue();
int show_statshm(char *fh, int mode);
int fill_shmfile(int mode, char *shmkey, char *fname);
void show_goodbyeshm();

/* newterm.c */
void term_init();
void do_move(int destcol, int destline, void (*outc) (char));

/* xyz.c */
int find_ip( const char *ip, int flag, char *result);
int showperminfo(struct _select_def *conf, int i);
unsigned int setperms(unsigned int pbits, unsigned int basic, char *prompt, int numbers, int (*show) (struct _select_def *, int), int (*select) (struct _select_def *));
int fhselect(struct _select_def* conf,struct fileheader *fh,long flag);

/* boards_t.c */
int show_boardinfo(const char *bname);
int query_bm_core(const char *userid,int limited);
int show_authorBM(int ent, struct fileheader *fileinfo, char *direct);
int choose_board(int newflag, const char *boardprefix,int group,int yank_flag);
/* 选择 版， readnew或readboard */

/* bm.c*/
int deny_user(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg); /* 禁止POST用户名单 维护主函数 */
int clubmember(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg);
int delete_range(struct _select_def*,struct fileheader*,void*);

/* zmodem */
int read_zsend(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg);
int bbs_zsendfile(char* filename,char* sendname);
char* bbs_zrecvfile();

/* comm_lists.c */
int exec_mbem(const char*);
int domenu(const char*);

/* super_filter.c */
int super_filter(struct _select_def* conf,struct fileheader* curfh,void* extraarg);

/* definekey.c */
int load_key(char * fn);

/* newhelp.c */
int newhelp(int mode);

/* tmpl.c */
int choose_tmpl(char *title, char *fname);

/* stuff.c */
#ifdef HAVE_IPV6_SMTH
void * ip_len2mask (int bitlen, void *vmask);
struct in6_addr * ip_mask(const struct in6_addr * addr, const struct in6_addr * mask, struct in6_addr * out);
#endif

/* bbsd_single.c */
void main_bbs(int convit, char *argv);

/* calltime.c */
time_t calc_calltime(int mode);

#endif /* _DEFINES_T_H_ */
