#if !defined BBSLIB_H
#define BBSLIB_H

#include "bbs.h"
#include "chat.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdarg.h>
/*#include "crypt.h"*/
#include "types.h"
/*#include "crypt.h"*/

#ifdef SMTH
#define CACHE_ROOT "/backup/www/htdocs"
#define MAX_CA_PATH_LEN 1024
#define report(x)        bbslog("user",x)
#endif

#define FIRST_PAGE	"/index.html"
#define CSS_FILE 	"/bbs.css"
#define CHARSET		"gb2312"
#define UCACHE_SHMKEY	get_shmkey("UCACHE_SHMKEY")
#define UTMP_SHMKEY	get_shmkey("UTMP_SHMKEY")
#define BCACHE_SHMKEY	get_shmkey("BCACHE_SHMKEY")

#ifndef MAXREJECTS
#define MAXREJECTS	(0xff)
#endif

#ifndef BBSHOST
#define BBSHOST NAME_BBS_ENGLISH
#endif

#ifndef BBSNAME
#define BBSNAME NAME_BBS_CHINESE
#endif

#define WWW_LOG             "bbslog/www.log"   /* "www.bbslog" */
#define WWW_BADLOGIN        "bbslog/badlogin.www"  /* "badlogin.www" */
#define SYS_MSGFILE         "msgfile"
#define SYS_MSGFILELOG      "msgfile.log"   /* "msgfile.me" */

//typedef int (*APPLY_UTMP_FUNC)(struct user_info*,char*,int pos);


extern char seccode[SECNUM][5];
extern char secname[SECNUM][2][20];
extern int loginok;
extern struct userec* currentuser;
extern struct user_info *u_info;
extern struct UTMPFILE *shm_utmp;
extern struct UCACHE *shm_ucache;
extern char fromhost[IPLEN+1];
extern friends_t fff[200];
extern int friendnum;
extern char parm_name[256][80], *parm_val[256];
extern int parm_num;
extern friends_t bbb[MAXREJECTS];
extern int badnum;

extern struct user_info *user_record[USHM_SIZE];
extern struct userec *user_data;
extern int friendmode, range;

int junkboard(char *board);

struct post_log {
	char	author[IDLEN+1];
	char	board[18];
	char	title[66];
	time_t	date;
	int	number;
};
/*struct _shmkey {
	char key[20];
	int value;
};*/

char *strcasestr();
char *ModeType();
char *anno_path_of();

int file_has_word(char *file, char *word);

int f_append(char *file, char *buf);

struct stat *f_stat(char *file);

#define file_size(x) f_stat(x)->st_size
#define file_time(x) f_stat(x)->st_mtime
#define file_rtime(x) f_stat(x)->st_atime
#define file_exist(x) (file_time(x)!=0)
#define file_isdir(x) ((f_stat(x)->st_mode & S_IFDIR)!=0)
#define file_isfile(x) ((f_stat(x)->st_mode & S_IFREG)!=0)

#define PERM_BLEVELS    (PERM_SYSOP | PERM_OBOARDS)
/* PERM_DENYMAIL in SMTH */
#define PERM_SPECIAL8  04000000000

#ifndef SMTH
int get_record(void *buf, int size, int num, char *file);
#else
int get_record(char *filename, char *rptr, int size, int id);
#endif

int put_record(void *buf, int size, int num, char *file);

int del_record(char *file, int size, int num);

char *wwwCTime(time_t t);

char *noansi(char *s);

char *nohtml(char *s);

char *strright(char *s, int len);

char *strcasestr(char *s1, char *s2);

int strsncpy(char *s1, char *s2, int n);

char *ltrim(char *s);

char *rtrim(char *s);

#define trim(s) ltrim(rtrim(s))

#define setcookie(a, b)	printf("<script>document.cookie='%s=%s'</script>\n", a, b)
#define redirect(x)	printf("<meta http-equiv='Refresh' content='0; url=%s'>\n", x)
#define refreshto(x, t)	printf("<meta http-equiv='Refresh' content='%d; url=%s'>\n", t, x)
#define cgi_head()	printf("Content-type: text/html; charset=%s\n\n", CHARSET)

char *getsenv(char *s);

int http_quit();

int http_fatal(char *fmt, ...);

int strnncpy(char *s, int *l, char *s2);

int hsprintf(char *s, char *fmt, ...);

int hprintf(char *fmt, ...);

int hhprintf(char *fmt, ...);

int parm_add(char *name, char *val);

int http_init();

int __to16(char c);

int __unhcode(char *s);

char *getparm(char *var);

int get_shmkey(char *s);

int shm_init();

int user_init(struct userec **x, struct user_info **y);

int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig);

int post_imail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig);

#ifndef SMTH
int post_article(char *board, char *title, char *file, char *id, char *nickname, char *ip, int sig);
#else
int post_article(char *board, char *title, char *file, struct userec *user,
		        char *ip, int sig, int local_save, int anony);
#endif

int sig_append(FILE *fp, char *id, int sig);

char* anno_path_of(char *board);

int has_BM_perm(struct userec *user, char *board);

int has_read_perm(struct userec *user, char *board);

bcache_t *getbcache(char *board);

int count_mails(char *id, int *total, int *unread);

int findnextutmp(char *id, int from);
#ifndef SMTH
/*int sethomefile(char *buf, char *id, char *file); define in func.h*/
#else /* SMTH */
char *sethomefile(char *buf, char *userid, char *filename);
#endif /* SMTH */

int send_msg(char *myuserid, int mypid, char *touserid, int topid, char msg[256]);

char *horoscope(int month, int day);

char *ModeType(int mode);

char   *cexp(int exp);

char   *cperf(int perf);

int count_life_value(struct userec *urec);

int countexp(struct userec *x);

int countperf(struct userec *x);

int modify_mode(struct user_info *x, int newmode);

int save_user_data(struct userec *x);

int is_bansite(char *ip);

int user_perm(struct userec *x, int level);

int getusernum(char *id);

#ifndef SMTH
struct userec *getuser(char *id);
#else /* SMTH */
int getuser(const char *userid,struct userec** user);
#endif /* SMTH */

int checkpasswd(const char *pw_crypted, const char *pw_try);

int checkuser(char *id, char *pw);

int count_id_num(char *id);

int count_online();

int count_online2();

int loadfriend(char *id);

int isfriend(char *id);

int loadbad(char *id);

int isbad(char *id);

int init_all();

int init_no_http();

char *void1(unsigned char *s);

char *sec(char c);

char *flag_str(int access);

char *flag_str2(int access, int has_read);

char *userid_str(char *s);

int fprintf2(FILE *fp, char *s);

struct fileheader *get_file_ent(char *board, char *file);

char *getbfroma(char *path);

int set_my_cookie();

int has_fill_form();

bcache_t *getbcacheaddr();

char *crypt1(char *buf, char *salt);

struct dir {
        char board[20];
        char userid[14];
        char showname[40];
        char exp[80];
        char type[30];
        int filename;
        int date;
        int level;
        int size;
        int live;
        int click;
        int active;
	int accessed;
};

#ifdef SMTH
int full_utmp(struct user_info* uentp,int* count);
struct user_info **get_ulist_addr();
uinfo_t *get_user_info(int utmpnum);
#endif /* SMTH */

char *getcurruserid();

struct userec *getcurrusr();

struct userec *setcurrusr(struct userec *user);

uinfo_t *getcurruinfo();

#define FAVBOARDNUM     20

void save_favboard();

int DelFavBoard(int i);

void load_favboard(int dohelp);

int IsFavBoard(int idx);

int get_favboard(int num);

int get_favboard_count();

int add_favboard(char *brdname);

struct boardheader const* getboard(int num);

int isowner(struct userec *user, struct fileheader *fileinfo);

void write_header2(FILE *fp, char *board, char *title, 
					char *userid, char *username, int anony);

int outgo_post2(struct fileheader *fh, char *board, 
				char *userid, char *username, char *title);

int post_file(char *filename, postinfo_t *pi);

char * setmailpath( char *buf, char *userid );

int get_unifile(char *filename, char *key, int mode);

#endif /* BBSLIB_H */
