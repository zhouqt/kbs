/*  这个文件是用来保存telnet部分的函数和变量声明

*/
#ifdef BBSMAIN

#ifdef INTERNET_EMAIL
int mail_uforward(int ent,struct fileheader *fileinfo,char *direct );
int mail_forward(int ent,struct fileheader *fileinfo ,char *direct );
#endif

/*screen.c*/
void move(int y,int x); 
void clrtobot();
void prints(char * fmt, ...);
int pressreturn();
void clear();
void refresh();
void clrtoeol();
int pressanykey();
int askyn(char str[STRLEN],int defa);
void saveline(int line,int mode,char* buffer);
void standend();
void standout();
void redoscr();
void initscr() ;
void bell();
void outc(register const unsigned char c);
void outns(const unsigned char *str, int n);
void getyx(int *y,int *x);
void outs(register const char *str);

>>>>>>> 3.13


/* newio.c */
void oflush();
int getdata(int line, int col,char* prompt,char* buf,int  len,int  echo,int nouse,int clearlabel);
void set_alarm(int set_timeout,void (*timeout_func)(void*),void* data);
int igetkey();
void check_calltime();

/* xyz.c*/
int modify_user_mode(int mode);

/* more.c */
int ansimore(char* filename,int promptend);
int ansimore2(char* filename,int promptend,int row,int numlines);
int NNread_init();
void m_init();
int countln(char* fname);

/* namecomplete.c */
void AddNameList(char* name);
void CreateNameList();
int namecomplete(char* prompt,char* data);
int usercomplete(char* prompt,char* data);

/* newmain_single.c */
int dosearchuser(char* userid);
void showtitle(char* title,char* mid);
void abort_bbs();
int egetch();
void update_endline();

/* bbs.c*/
void record_exit_time();   /* 记录离线时间  Luzi 1998/10/23 */
void shownotepad();
int check_stuffmode();
int do_reply(char* title);
int do_select(int ent,struct fileheader* fileinfo,char* direct );  /* 输入讨论区名 选择讨论区 */
int do_post();
int post_cross(char islocal,int mode);
void make_blist();
int postfile(char* filename,char* nboard,char* posttile,int mode);
int digest_post(int ent,struct fileheader *fhdr,char *direct);
int mark_post(int ent,struct fileheader *fhdr,char *direct);
int post_article();                         /*用户 POST 文章 */
int do_send(char* userid,char* title);
int post_cross(char islocal,int mode);
int digest_mode();
int thread_mode();
int deleted_mode();
int junk_mode();
int Goodbye();
int post_reply(int ent,struct fileheader *fileinfo,char *direct );
int sequential_read2(int ent);
void setquotefile(char filepath[]);
void do_quote(char *filepath,char quote_mode);

/*vote.c*/
void makevdir(char* bname);
void setvfile(char* buf,char* bname,char* filename);
int  b_closepolls();
int vote_flag(char* bname,char val,int mode);

/* mail.c */
int chkmail();
int getmailnum(char recmaildir[STRLEN]);
int get_mailnum();
int invalidaddr(char*addr);

/* maintain.c */
int check_systempasswd();
void stand_title(char* title);
int get_grp(char seekstr[STRLEN]);
int del_grp(char grp[STRLEN],char bname[STRLEN],char title[STRLEN]);
int securityreport(char* str,struct userec* lookupuser,char fdata[7][STRLEN]);

/* talk.c */
int del_from_file(char filename[STRLEN],char str[STRLEN]);
int addtooverride(char* uident);
int t_query(char q_id[IDLEN]);
int talkreply();
int friend_add(int ent,struct friends * fh,char* direct);
void talk_request();
int getfriendstr();
int num_user_logins(char* uid);

/* delete.c */
int kick_user(struct user_info *userinfo);
int d_user(char cid[IDLEN]);
void getuinfo(FILE *fn,struct userec *ptr_urec);

/* read.c */
int sread(int passonly,int readfirst,int auser,int pnum,struct fileheader *ptitle);
void fixkeep(char* s,int first,int last);
void i_read( int cmdmode,char *direct ,int (*dotitle)() ,char *(*doentry)(char*,int,char*),struct one_key *rcmdlist,int ssize);
void u_exit();

/* sendmsg.c */
void s_msg();
void r_lastmsg();
int show_allmsgs();
int do_sendmsg(struct user_info *uentp,const char msgstr[256],int mode);

/* list.c */
int t_friends();

/* edit.c */
int Origin2(char text[256]);
int vedit(char *filename,int saveheader);
int vedit_post(char *filename,int saveheader );

/* Announce.c */
void a_prompt(int bot,char* pmt,char* buf);
int a_SeSave(char    *path,char*key,struct fileheader *fileinfo,int    nomsg);
int a_menusearch(char* path,char* key,int level);
int a_Save(char    *path,char *key,struct fileheader *fileinfo,int nomsg,
	char *direct,int ent);
int a_Import(char *path,char *key,struct fileheader *fileinfo,int nomsg,char *direct,int ent);  /* Leeward 98.04.15 */

/* goodbye.c */
void showstuff(char buf[256]);
int user_display(char* filename,int number,int mode);
int countlogouts(char filename[STRLEN]);

/* userinfo.c */
void disply_userinfo(struct userec *u ,int real);

/* commlist.c */
int domenu(char* menu_name);

/* register.c */
void check_register_info();
void new_register();

/* help.c */
void show_help(char * fname);

/* fileshm.c */
int show_statshm(char *fh,int mode);
int fill_shmfile(int mode,char* shmkey,char* fname);
void show_goodbyeshm();

/* newterm.c */
int term_init();

void  record_exit_time();

#endif

