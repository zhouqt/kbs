#ifndef FB_TYPES_H
#define FB_TYPES_H

typedef struct user_info   uinfo_t;
#ifdef SMTH
typedef struct boardheader bcache_t;
typedef struct friends     friends_t;
#else
typedef struct shortfile bcache_t;
typedef struct override    friends_t;
#endif

#include <time.h>

struct postinfo
{
	char *userid;
	char *username;
	char *title;
	time_t posttime;
	char *board;
	int local;
	int anony;
	int access;
	int sig;
	char *from;
};

typedef struct postinfo postinfo_t;

#endif /* FB_TYPES_H */
