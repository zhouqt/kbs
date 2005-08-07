/*
 * $Id$
 */
#if !defined BBSLIB_H
#define BBSLIB_H

#include "bbs.h"

typedef struct user_info uinfo_t;

typedef struct boardheader bcache_t;
typedef struct friends friends_t;

extern struct user_info *u_info;

int f_append(char *file, char *buf);

struct stat *f_stat(char *file);

#define file_size(x) f_stat(x)->st_size
#define file_exist(x) (f_stat(x)->st_mtime!=0)
#define file_isdir(x) ((f_stat(x)->st_mode & S_IFDIR)!=0)
#define file_isfile(x) ((f_stat(x)->st_mode & S_IFREG)!=0)

int get_userstatusstr(char *userid, char *buf);

char *wwwCTime(time_t t);

time_t get_idle_time(struct user_info * uentp);

int strsncpy(char *s1, char *s2, int n);

char *ltrim(char *s);

char *rtrim(char *s);

#define trim(s) ltrim(rtrim(s))

int post_mail(char *userid, char *title, char *file, char *id, char *nickname, char *ip, int sig, int backup);

int del_mail(int ent, struct fileheader* fh, char* direct);

int post_article(char *board, char *title, char *file, struct userec *user, char *ip, int sig, int local_save, int anony, struct fileheader* oldx,char* attach_dir, int mailback, int is_tex);

int sig_append(FILE * fp, char *id, int sig);

int has_BM_perm(struct userec *user, char *board);

int send_msg(char *myuserid, int mypid, char *touserid, int topid, char msg[256]);

int isfriend(char *id);

int fprintf2(FILE * fp, char *s);

int get_file_ent(char *board, char *file, struct fileheader *x);

//board manage
int del_post(int ent, struct fileheader *fileinfo, char *direct, char *board);

void write_header2(FILE * fp, char *board, char *title, char *userid, char *username, int anony, int local);

int outgo_post2(struct fileheader *fh, char *board, char *userid, char *username, char *title);

int get_curr_utmpent();

int www_user_login(struct userec *user, int useridx, int kick_multi, char *fromhost, char *fullfrom, struct user_info **ppuinfo, int *putmpent);
int www_user_logoff(struct userec *user, int useridx, struct user_info *puinfo, int userinfoidx);
int www_user_init(int useridx, char *userid, int key, struct userec **x, struct user_info **y, long compat_telnet);
int www_data_init();
void www_data_detach();
int is_BM(const struct boardheader *board,const struct userec *user);
char *unix_string(char *str);
void output_ansi_html(char *buf, size_t buflen, buffered_output_t *output,char* attachlink, int is_tex, char* preview_attach_dir);
int www_generateOriginIndex(const char* board);

struct WWW_GUEST_S* www_get_guest_entry(int idx);

#define DEBUG_WAIT \
do {\
	struct stat _dw_st; \
	while (stat("/tmp/start", &_dw_st) < 0) \
		sleep(1); \
} while(0)

void output_ansi_text(char *buf, size_t buflen, 
							buffered_output_t * output, char* attachlink);
void output_ansi_javascript(char *buf, size_t buflen, 
							buffered_output_t * output, char* attachlink);

#endif                          /* BBSLIB_H */
