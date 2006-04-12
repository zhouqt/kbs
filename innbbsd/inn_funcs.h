#ifndef INN_FUNCS_H
#define INN_FUNCS_H

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
int tryaccept(int);
int argify(const char*,char***);
int deargify(char***);
void hisincore(int);
void startfrominetd(int);
int dbzdebug(int);
int installinnbbsd(void);
void sethaltfunction(int (*)(int));

#endif /* INN_FUNCS_H */
