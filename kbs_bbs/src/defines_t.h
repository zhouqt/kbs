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

/* newio.c */
void oflush();
int getdata(int line,int col,int len,int echo,int clearlabel,int nouse,char *prompt,char *buf);

/* xyz.c*/
int modify_user_mode(int mode);

/* more.c */
int ansimore(char* filename,int promptend);

/* namecomplete.c */
void AddNameList(char* name);
void CreateNameList();

/* newmain_single.c */
int dosearchuser(char* userid);
#endif
