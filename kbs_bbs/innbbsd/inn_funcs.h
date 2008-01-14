#ifndef INN_FUNCS_H
#define INN_FUNCS_H

#include "daemon.h"

int isfile(const char*);
int isdir(const char*);
int iszerofile(const char*);
size_t filesize(const char*);
int inetclient(const char*,const char*,const char*);
int unixclient(const char*,const char*);
void initial_lang(void);
void verboseon(const char*);
void verboselog(const char*,...);
int isverboselog(void);
void setverboseon(void);
void setverboseoff(void);
int innbbsdstartup(void);
int dbzinnbbsdstartup(void);
time_t gethisinfo(void);
void HISmaint(void);
void HISclose(void);
void HISsetup(void);
int storeDB(const char*,const char*);
int clearfdset(int);
void docompletehalt(void);
int pmain(const char*);
int p_unix_main(const char*);
int open_listen(const char*,const char*,int (*)(int));
int open_unix_listen(const char*,const char*,int (*)(int));
int dbzINNBBSDshutdown(void);
int dbzinstallinnbbsd(void);
int tryaccept(int);
int argify(const char*,char***);
int deargify(char***);
void hisincore(int);
void startfrominetd(int);
int dbzdebug(int);
int dbzisPause();
int installinnbbsd(void);
void sethaltfunction(int (*)(int));
void CloseOnExec(int fd, int flag);
int cancel_article_front(char *msgid);
int filtermatch(int result, char *target, char *pat);
int wildmat(char *text, char *p);
void str_decode(register unsigned char *dst, register unsigned char *src);
int receive_control();
int channelreader(ClientType *client);
int isPause();
int INNBBSDshutdown();

typedef struct _TIMEINFO 
{
     time_t      time;
     long        usec;
     long        tzone;
} TIMEINFO;
int date_lex();
time_t parsedate(char *p, TIMEINFO *now);

#endif /* INN_FUNCS_H */
