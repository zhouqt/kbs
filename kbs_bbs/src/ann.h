#ifndef _ANN_H
#define _ANN_H

#define MAXITEMS        1024
#define PATHLEN         256

typedef struct {
    /* changed by period from 72 to 84 2000-10-17 (%38s + "(BM:   )" + 12*3) */
    char title[84 /*72 */ ];
    char fname[80];
    char *host;
    int port;
    long    attachpos;
} ITEM;

typedef struct {
    ITEM *item[MAXITEMS];
    char mtitle[STRLEN];
    char *path;
    int num, page, now;
    int level;
    time_t modified_time;
} MENU;

#endif
