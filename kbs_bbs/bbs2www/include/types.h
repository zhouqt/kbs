/*
 * $Id$
 */
#ifndef FB_TYPES_H
#define FB_TYPES_H

typedef struct user_info uinfo_t;

#ifdef SMTH
typedef struct boardheader bcache_t;
typedef struct friends friends_t;
#else
typedef struct shortfile bcache_t;
typedef struct override friends_t;
#endif

#include <time.h>


#endif                          /* FB_TYPES_H */
