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
void prints(...);
int pressreturn();
void clear();
void refresh();
void clrtoeol();
int pressanykey();
int askyn(char str[STRLEN],int defa);
void saveline(int line,int mode,char* buffer);
	
/* newio.c */
void oflush();
int getdata(int line, int col,char* prompt,char* buf,int  len,int  echo,int nouse,int clearlabel);

/* xyz.c*/
int modify_user_mode(int mode);

/* more.c */
int ansimore(char* filename,int promptend);

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

/* bbs.c*/
int do_reply(char* title);
int do_select(int ent,struct fileheader* fileinfo,char* direct );  /* 输入讨论区名 选择讨论区 */
int do_post();
int post_cross(char islocal,int mode);
void make_blist();
int postfile(char* filename,char* nboard,char* posttile,int mode);
int digest_post(int ent,struct fileheader *fhdr,char *direct);
int mark_post(int ent,struct fileheader *fhdr,char *direct);

/*vote.c*/
void makevdir(char* bname);
void setvfile(char* buf,char* bname,char* filename);

/* mail.c */
int chkmail();

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

/* delete.c */
int kick_user(struct user_info *userinfo);

/* read.c */
int sread(int passonly,int readfirst,int auser,int pnum,struct fileheader *ptitle);

/* sendmsg.c */
void s_msg();

/* list.c */
int t_friends();
#endif

