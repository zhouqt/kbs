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
int ansimore2(char* filename,int promptend,int row,int numlines);

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
int post_article();                         /*用户 POST 文章 */
int do_send(char* userid,char* title);
int post_cross(char islocal,int mode);
int a_Save(char    *path,char *key,struct fileheader *fileinfo,int nomsg,
	char *direct,int ent);
int a_Import(char *path,char *key,struct fileheader *fileinfo,int nomsg,char *direct,int ent);  /* Leeward 98.04.15 */

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
int talkreply();

/* delete.c */
int kick_user(struct user_info *userinfo);
int d_user(char cid[IDLEN]);

/* read.c */
int sread(int passonly,int readfirst,int auser,int pnum,struct fileheader *ptitle);
void fixkeep(char* s,int first,int last);

/* sendmsg.c */
void s_msg();
void r_lastmsg();

/* list.c */
int t_friends();

/* edit.c */
int Origin2(char text[256]);
int vedit(char *filename,int saveheader);
int vedit_post(char *filename,int saveheader );

/* Announce.c */
void a_prompt(int bot,char* pmt,char* buf);

#endif

