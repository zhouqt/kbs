/*
 * autoclear.c - a tool to drop out-of-date and cancelled articles to
 *               dustbin according to the parameters and the settings
 *               located in BBSHOME/etc/autoclear
 * Usage:
 * 	autoclear [day] [max] [min]
 * 
 * porting from Maple 2.36 by Nighthawk.bbs@www.cn.nctu.edu.tw
 * changed for smth_bbs
 * $Id$
 */

#include <stdlib.h>
#include <strings.h>
#include "bbs.h"
//#include "site.h"
//#include "record.c"

#define DEF_DAYS        365*10
#define DEF_MAXP        999999
#define DEF_MINP        9999

#define EXPIRE_CONF     BBSHOME"/etc/autoclear"

char *bpath = BBSHOME "/boards";

//report()
//{
//}

struct life {
    char bname[16];             /* board ID */
    int days;                   /* expired days */
    int maxp;                   /* max post */
    int minp;                   /* min post */
};
typedef struct life life;

void expire(brd)
life *brd;
{
    struct fileheader head;
    struct stat state;
    char lockfile[128], tmpfile[128], bakfile[128];
    char fpath[128], index[128], *fname;
    int total;
    int fd, fdr, fdw, done, keep;
    int duetime, ftime;

    printf("%s\n", brd->bname);

#ifdef  VERBOSE
    if (brd->days < 1) {
        printf(":Err: expire time must more than 1 day.\n");
        return;
    } else if (brd->maxp < 100) {
        printf(":Err: maxmum posts number must more than 100.\n");
        return;
    }
#endif

    sprintf(index, "%s/%s/.DIR", bpath, brd->bname);
    sprintf(lockfile, "%s.lock", index);
    if ((fd = open(lockfile, O_RDWR | O_CREAT | O_APPEND, 0644)) == -1)
        return;
    flock(fd, LOCK_EX);

    strcpy(fpath, index);
    fname = (char *) strrchr(fpath, '.');

    duetime = time(NULL) - brd->days * 24 * 60 * 60;
    done = 0;
    if ((fdr = open(index, O_RDONLY, 0)) > 0) {
        fstat(fdr, &state);
        total = state.st_size / sizeof(head);
        sprintf(tmpfile, "%s.new", index);
        unlink(tmpfile);
        if ((fdw = open(tmpfile, O_WRONLY | O_CREAT | O_EXCL, 0644)) > 0) {
            while (read(fdr, &head, sizeof head) == sizeof head) {
                done = 1;
                ftime = atoi(head.filename + 2);
                if (head.owner[0] == '-')
                    keep = 0;
                else if (head.accessed[0] & FILE_MARKED || total <= brd->minp)
                    keep = 1;
                else if (ftime < duetime || total > brd->maxp)
                    keep = 0;
                else
                    keep = 1;

                if (keep) {
                    if (safewrite(fdw, &head, sizeof head) == -1) {
                        done = 0;
                        break;
                    }
                } else {
                    strcpy(fname, head.filename);
                    unlink(fpath);
                    printf("\t%s\n", fname);
                    total--;
                }
            }
            close(fdw);
        }
        close(fdr);
    }
    if (done) {
        sprintf(bakfile, "%s.old", index);
        if (rename(index, bakfile) != -1)
            rename(tmpfile, index);
    }
    flock(fd, LOCK_UN);
    close(fd);
}

main(argc, argv)
char *argv[];
{
    FILE *fin;
    int number;
    size_t count;
    life db, table[MAXBOARD], *key;
    struct dirent *de;
    DIR *dirp;
    char *ptr, *bname, buf[256];

    db.days = ((argc > 1)
               && (number = atoi(argv[1])) > 0) ? number : DEF_DAYS;
    db.maxp = ((argc > 2)
               && (number = atoi(argv[2])) > 0) ? number : DEF_MAXP;
    db.minp = ((argc > 3)
               && (number = atoi(argv[3])) > 0) ? number : DEF_MINP;

    /* -------------- */
    /* load autoclear */
    /* -------------- */

    count = 0;
    if (fin = fopen(EXPIRE_CONF, "r")) {
        while (fgets(buf, 256, fin)) {
            if (buf[0] == '#')
                continue;

            bname = (char *) strtok(buf, " \t\r\n");
            if (bname && *bname) {
                ptr = (char *) strtok(NULL, " \t\r\n");
                if (ptr && (number = atoi(ptr)) > 0) {
                    key = &(table[count++]);
                    strcpy(key->bname, bname);
                    key->days = number;
                    key->maxp = db.maxp;
                    key->minp = db.minp;

                    ptr = (char *) strtok(NULL, " \t\r\n");
                    if (ptr && (number = atoi(ptr)) > 0) {
                        key->maxp = number;

                        ptr = (char *) strtok(NULL, " \t\r\n");
                        if (ptr && (number = atoi(ptr)) > 0) {
                            key->minp = number;
                        }
                    }
                }
            }
        }
        fclose(fin);
    }
    if (count > 1) {
        qsort(table, count, sizeof(life), (const void *) strcasecmp);
    }
    /* ---------------- */
    /* visit all boards */
    /* ---------------- */

    if (!(dirp = opendir(bpath))) {
        printf(":Err: unable to open %s\n", bpath);
        return -1;
    }
    while (de = readdir(dirp)) {
        ptr = de->d_name;
        if (ptr[0] > ' ' && ptr[0] != '.') {
            if (count)
                key = (life *) bsearch(ptr, table, count, sizeof(life), (const void *) strcasecmp);
            else
                key = NULL;
            if (!key)
//                key = &db;
		continue;
            strcpy(key->bname, ptr);
            expire(key);
        }
    }
    closedir(dirp);
}
