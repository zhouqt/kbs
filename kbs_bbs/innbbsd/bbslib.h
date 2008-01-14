#ifndef BBSLIB_H
#define BBSLIB_H

#include "bbs.h"
#include <netinet/in.h>

typedef struct nodelist_t {
    char *node;
    char *exclusion;
    char *host;
    char *protocol;
    char *comments;
    int feedtype;
    FILE *feedfp;
} nodelist_t;

#ifdef FILTER
typedef struct filter_t {
    char *group;
    char *rcmdfilter, *scmdfilter;
} filter_t;

typedef char *(*FuncPtr) ();

#endif

typedef struct newsfeeds_t {
    char *newsgroups;
    char *board;
    char *path;
#ifdef FILTER
    FuncPtr rfilter, sfilter;
    char *rcmdfilter, *scmdfilter;
#endif
} newsfeeds_t;

typedef struct overview_t {
    char *board, *filename, *group;
    time_t mtime;
    char *from, *subject;
} overview_t;

extern char MYBBSID[];
extern char ECHOMAIL[];
extern char BBSFEEDS[];
extern char LOCALDAEMON[];
extern char INNDHOME[];
extern char HISTORY[];
extern char LOGFILE[];
extern char INNBBSCONF[];
extern char FILTERCTL[];
extern nodelist_t *NODELIST;
extern nodelist_t **NODELIST_BYNODE;
extern newsfeeds_t *NEWSFEEDS, **NEWSFEEDS_BYBOARD;
extern int NFCOUNT, NLCOUNT;
extern int Expiredays, His_Maint_Min, His_Maint_Hour;
extern int LOCALNODELIST, NONENEWSFEEDS;
extern int Maxclient;

# ifndef ARG
#  ifdef __STDC__
#   define ARG(x) x
#  else
#   define ARG(x) ()
#  endif
# endif

int initial_bbs ARG((char *));
char *restrdup ARG((char *, char *));
nodelist_t *search_nodelist ARG((char *, char *));
newsfeeds_t *search_group ARG((char *));
void *mymalloc ARG((int));
void *myrealloc ARG((void *, int));

char **BNGsplit(char *line);

char *fileglue(char* fmt,...);
int isdirv(char *fmt,...);
unsigned long mtimev(char *fmt,...);

char *DBfetch(char *key);

nodelist_t *search_nodelist_bynode(char *node);
FuncPtr search_filtercmd(char *cmd);
newsfeeds_t *search_board(char *board);
filter_t *search_filter(char *group);
char *filterdata(newsfeeds_t *arp, int direction, char *indata, char **filterdata);
char *parselocaltime(char *date_buf);

char *my_rfc931_name(int herefd, struct sockaddr_in *there);
void testandmkdir(const char *dir);
int readfilterfile(char *inndhome);
int readnffile(const char *inndhome);
int readnlfile(char *inndhome, char *outgoing);

void innbbsdlog(char* fmt,...);

#endif
