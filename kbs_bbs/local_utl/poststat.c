/*-------------------------------------------------------*/
/* util/poststat.c      ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : Õ≥º∆ΩÒ»’°¢÷‹°¢‘¬°¢ƒÍ»»√≈ª∞Ã‚                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------
#define DEBUG*/

#include <stdio.h>
#include <time.h>
#include "bbs.h"
#define	DELETE

char *myfile[] = { "day", "week", "month", "year", "bless" };
int mycount[4] = { 7, 4, 12 };
int mytop[] = { 10, 50, 100, 100, 10 };
char *mytitle[] = { "»’ Æ¥Û»»√≈ª∞Ã‚",
    "÷‹ŒÂ Æ¥Û»»√≈ª∞Ã‚",
    "‘¬∞Ÿ¥Û»»√≈ª∞Ã‚",
    "ƒÍ∂»∞Ÿ¥Û»»√≈ª∞Ã‚",
    "»’ Æ¥Û÷‘–ƒ◊£∏£"
};


#define HASHSIZE 1024
#define TOPCOUNT 200
#ifdef BLESS_BOARD
const char *surfix_bless[21] = {
    "  \x1b[1;34m°Ù  ",
    "\x1b[1;32m©≥\x1b[0;32m©ÿ\x1b[1m©∑",
    "\x1b[32m©»\x1b[1m…Ó\x1b[0;32m©¿",
    "\x1b[1;32m©ª\x1b[0;32m©–\x1b[1m©ø",
    "  \x1b[32m©¶  ",
    "\x1b[1;32m©≥\x1b[0;32m©ÿ\x1b[1m©∑",
    "\x1b[32m©»\x1b[1m«È\x1b[0;32m©¿",
    "\x1b[1;32m©ª\x1b[0;32m©–\x1b[1m©ø",
    "  \x1b[32m©¶  ",
    "\x1b[1;32m©≥\x1b[0;32m©ÿ\x1b[1m©∑",
    "\x1b[32m©»\x1b[1mµƒ\x1b[0;32m©¿",
    "\x1b[1;32m©ª\x1b[0;32m©–\x1b[1m©ø",
    "  \x1b[32m©¶  ",
    "\x1b[1;32m©≥\x1b[0;32m©ÿ\x1b[1m©∑",
    "\x1b[32m©»\x1b[1m◊£\x1b[0;32m©¿",
    "\x1b[1;32m©ª\x1b[0;32m©–\x1b[1m©ø",
    "  \x1b[32m©¶  ",
    "\x1b[1;32m©≥\x1b[0;32m©ÿ\x1b[1m©∑",
    "\x1b[32m©»\x1b[1m∏£\x1b[0;32m©¿",
    "\x1b[1;32m©ª\x1b[0;32m©–\x1b[1m©ø",
    "  \x1b[1;34m°Ù  "
};
#endif

struct fileheader               /* This structure is used to hold data in */
 fh[1];

struct postrec {
//    char author[13];            /* author name */
    char board[IDLEN + 6];      /* board name */
//    char title[66];             /* title name */
	unsigned int groupid;
    time_t date;                /* last post's date */
    int number;                 /* post number */
    struct postrec *next;       /* next rec */
} *bucket[HASHSIZE], *blessbucket[HASHSIZE];


/* 100 bytes */
struct posttop top[TOPCOUNT], postlog;


int hash(key)
unsigned int key;
{
    return key % HASHSIZE;
}


/* ---------------------------------- */
/* hash structure : array + link list */
/* ---------------------------------- */


void search(struct posttop *t)
{
    struct postrec *p, *q, *s;
    int i, found = 0;

    i = hash(t->groupid);
    q = NULL;
#ifdef BLESS_BOARD
    if (!strcasecmp(t->board, BLESS_BOARD))
        p = blessbucket[i];
    else
#endif
        p = bucket[i];
    while (p && (!found)) {
        if (p->groupid == t->groupid && !strcmp(p->board, t->board))
            found = 1;
        else {
            q = p;
            p = p->next;
        }
    }

    if (found) {
        p->number += t->number;
        if (p->date < t->date)  /* »°ΩœΩ¸»’∆⁄ */
            p->date = t->date;
    } else {
        s = (struct postrec *) malloc(sizeof(struct postrec));
        memcpy(s, t, sizeof(struct posttop));
        s->next = NULL;
        if (q == NULL)
#ifdef BLESS_BOARD
            if (!strcasecmp(t->board, BLESS_BOARD))
                blessbucket[i] = s;
            else
#endif
                bucket[i] = s;
        else
            q->next = s;
    }
}


int sort(pp, count)
struct postrec *pp;
{
    int i, j;

    for (i = 0; i <= count; i++) {
        if (pp->number > top[i].number) {
            if (count < TOPCOUNT - 1)
                count++;
            for (j = count - 1; j >= i; j--)
                memcpy(&top[j + 1], &top[j], sizeof(struct posttop));

            memcpy(&top[i], pp, sizeof(struct posttop));
            break;
        }
    }
    return count;
}


void load_stat(fname)
char *fname;
{
    FILE *fp;

    if ((fp = fopen(fname, "r")) != NULL) {
        int count = fread(top, sizeof(struct posttop), TOPCOUNT, fp);

        fclose(fp);
        while (count)
            search(&top[--count]);
    }
}

void writestat(int mytype, struct postrec *dobucket[HASHSIZE])
{
    struct postrec *pp;
    FILE *fp;
    int i, j;
    char *p, curfile[40];

    /*
     * Haohmaru.99.11.20.ºÏ≤È «∑Ò“—±ª…æ 
     */
	int fd;
    char dirfile[80];
    int exist, real;
	fileheader_t fh;

    /*
     * Bigman.2000.8.28: –ﬁ∏ƒÕ≥º∆∑Ω Ω 
     */
    int m, n;
    char BoardName[10][13];
    char buf[40];

/* ---------------------------------------------- */
    /*
     * sort top 100 issue and save results            
     */
    /*
     * ---------------------------------------------- 
     */

    memset(top, 0, sizeof(top));
    for (i = j = 0; i < HASHSIZE; i++) {
        for (pp = dobucket[i]; pp; pp = pp->next)
            j = sort(pp, j);
    }

    p = myfile[mytype];
    sprintf(curfile, "etc/posts/%s.0", p);
    if ((fp = fopen(curfile, "w")) != NULL) {
        fwrite(top, sizeof(struct posttop), j, fp);
        fclose(fp);
    }

    sprintf(curfile, "etc/posts/%s", p);
    if ((fp = fopen(curfile, "w")) != NULL) {
#ifdef BLESS_BOARD
        if (mytype == 4)
            fprintf(fp, "              \x1b[1;33m©§©§ \x1b[31m°Ó\x1b[33m°Ó\x1b[32m°Ó \x1b[41;32m  \x1b[33m±æ»’ Æ¥Û÷‘–ƒ◊£∏£  \x1b[m\x1b[1;32m °Ó\x1b[31m°Ó\x1b[33m°Ó ©§©§\x1b[m\n\n");
        else
#endif
            fprintf(fp, "                [34m-----[37m=====[41m ±æ%s [m=====[34m-----[m\n\n", mytitle[mytype]);

        i = mytop[mytype];
//        if (j > i)
//            j = i;
#ifdef  DEBUG
        printf("i : %d, j: %d \n", i, j);
#endif
        real = 0;
        for (i = 0; i < j && real < mytop[mytype]; i++) {
            strcpy(buf, ctime(&top[i].date));
            buf[20] = NULL;
            p = buf + 4;

            sprintf(dirfile, "boards/%s/.DIR", top[i].board);
			if ((fd = open(dirfile, O_RDWR, 0644)) < 0)
				continue;

    		if( get_records_from_id(fd, top[i].groupid, &fh, 1, NULL) == 0 ){
				close(fd);
				continue;
			}
			close(fd);

#ifndef NINE_BUILD
                m = 0;
                for (n = 0; n < real; n++) {
                    if (!strcmp(top[i].board, BoardName[n]))
                        m++;
                }
#ifdef BLESS_BOARD
				if(mytype != 4){
#endif

                if (m >= 3)
                    continue;
#ifdef BLESS_BOARD
				}
#endif

#else
                if (!strcmp(top[i].board, "test"))
                    continue;
#endif

                strcpy(BoardName[real], top[i].board);

            real++;


#ifdef BLESS_BOARD
            if (mytype == 4)
                fprintf(fp,
                        "                                            %s \x1b[1;31m%4d\x1b[0;37m»À      %s\x1b[m\n"
                        "\x1b[1mµ⁄\x1b[31m%2d \x1b[37m√˚ \x1b[4%dm %-51.51s\x1b[m \x1b[1;33m%-12s%s\x1b[m\n",
                        p, top[i].number, surfix_bless[(real - 1) * 2], real, (real - 1) / 2 + 1, fh.title, fh.owner, surfix_bless[(real - 1) * 2 + 1]);
            else
#endif
                fprintf(fp,
                        "[37mµ⁄[31m%3d[37m √˚ [37m–≈«¯ : [33m%-16s[37m°æ[32m%s[37m°ø[36m%4d [37m»À[35m%16s\n"
                        "     [37m±ÍÃ‚ : [44m[37m%-60.60s[m\n", !mytype ? real : (i + 1), top[i].board, p, top[i].number, fh.owner, fh.title);
        }


#ifdef BLESS_BOARD
        if (mytype == 4)
            fprintf(fp, "                                                                         %s\x1b[m", surfix_bless[20]);
#endif
        fclose(fp);
    }
}

void poststat(int mytype, time_t now, struct tm *ptime)
{
    static char *logfile = ".newpost";
    static char *oldfile = ".newpost.old";

    char buf[40], curfile[40] = "etc/posts/day.0";
    struct postrec *pp;
    FILE *fp;
    int i;

    if (mytype < 0) {
        /*
         * --------------------------------------- 
         */
        /*
         * load .post and statictic processing     
         */
        /*
         * --------------------------------------- 
         */

        remove(oldfile);
        rename(logfile, oldfile);
        if ((fp = fopen(oldfile, "r")) == NULL)
            return;
        mytype = 0;
        load_stat(curfile);
#ifdef BLESS_BOARD
        load_stat("etc/posts/bless.0");
#endif

        while (fread(top, sizeof(struct posttop), 1, fp))
            search(top);
        fclose(fp);
    } else {
        /*
         * ---------------------------------------------- 
         */
        /*
         * load previous results and statictic processing 
         */
        /*
         * ---------------------------------------------- 
         */

        char *p;

#ifdef BLESS_BOARD
        if (mytype == 0) {
            unlink("etc/posts/bless.0");
            post_file(NULL, "", "etc/posts/bless", BLESS_BOARD, " Æ¥Û◊£∏£", 0, 1);
        }
#endif
        i = mycount[mytype];
        p = myfile[mytype];
        while (i) {
            sprintf(buf, "etc/posts/%s.%d", p, i);
            sprintf(curfile, "etc/posts/%s.%d", p, --i);
            load_stat(curfile);
            rename(curfile, buf);
        }
        mytype++;
    }


    /*
     * free statistics 
     */

    writestat(mytype, bucket);
#ifdef BLESS_BOARD
    if (mytype == 0)
        writestat(4, blessbucket);
#endif
    for (i = 0; i < HASHSIZE; i++) {
        for (pp = bucket[i]; pp; pp = pp->next)
            free(pp);
        bucket[i] = NULL;
    }
#ifdef BLESS_BOARD
    for (i = 0; i < HASHSIZE; i++) {
        for (pp = blessbucket[i]; pp; pp = pp->next)
            free(pp);
        blessbucket[i] = NULL;
    }
#endif
}


int main(int argc, char **argv)
{
    time_t now;
    struct tm ptime;
    int i;

    chdir(BBSHOME);

    time(&now);
    ptime = *localtime(&now);
    if (argc == 2) {
        i = atoi(argv[1]);
        if (i != 0) {
            poststat(i, now, &ptime);
            return 0;
        }
    }

    resolve_boards();
    if (ptime.tm_hour == 0) {
        if (ptime.tm_mday == 1)
            poststat(2, now, &ptime);
        if (ptime.tm_wday == 0)
            poststat(1, now, &ptime);
        poststat(0, now, &ptime);
    }
    poststat(-1, now, &ptime);
    if (ptime.tm_hour == 23) {
        char fname[STRLEN];

        sprintf(fname, "%dƒÍ%2d‘¬%2d»’ Æ¥Û»»√≈ª∞Ã‚", ptime.tm_year + 1900, ptime.tm_mon + 1, ptime.tm_mday);
        post_file(NULL, "", "etc/posts/day", "BBSLists", fname, 0, 1);
        if (ptime.tm_wday == 6) {
            sprintf(fname, "%dƒÍ%2d‘¬%2d»’±æ÷‹ŒÂ Æ¥Û»»√≈ª∞Ã‚", ptime.tm_year + 1900, ptime.tm_mon + 1, ptime.tm_mday);
            post_file(NULL, "", "etc/posts/week", "BBSLists", fname, 0, 1);
        }
    }
    return 0;
}
