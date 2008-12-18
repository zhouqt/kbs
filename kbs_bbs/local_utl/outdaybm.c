#include <stdio.h>
#include <string.h>
#include <time.h>
#if !defined __LINUX__ && !defined FREEBSD
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#include "bbs.h"
//#include "shm_rel.c"

#define MAXCHECK MAXBOARD*3

typedef int cmpfunc(const void *, const void *);

typedef struct {
    char uname[IDLEN + 2];
    char board[STRLEN];
    time_t inday;
} BMInfo;

static BMInfo *pBMInfo;
static int nBMCount=0;

int cmpBMLogin(BMInfo * fst, BMInfo * snd)
{
    return (fst->inday - snd->inday);
}

int isNormalBoard(const struct boardheader *brd)
{
    if (brd->level & PERM_NOZAP)
        return 1;
    else if (brd->level & PERM_LOGINOK)
        return 1;
    return (brd->level == 0);
}

void checkBMs(void)
{
    char lbuf[256], tbuf[80], mdate[80];

    char *fmt = " \033[%2d;%2dm%-14.14s%-32.32s%-24.24s%8.8s\033[m";
    char *timefmt = "%Y年%m月%d日 %T";
    int i, dftime;
    int fgc = 37, bgc = 44;
    time_t now;
    BMInfo *pBM;

    now = time(NULL);           /*  current time stamp  */
    sprintf(lbuf, fmt, fgc, bgc, "用户代号", "任职版面", "上任时间", "距今天数");
    fprintf(stdout, "\033[m\n%s\n", lbuf);
    for (i = 0; i < nBMCount; i++) {
        if (i % 2) {
            fgc = 37;
            bgc = 44;
        } else {
            fgc = 31;
            bgc = 46;
        }

        pBM = &(pBMInfo[i]);
        dftime = (now - pBM->inday) / 86400;
        strftime(mdate, 30, timefmt, localtime(&(pBM->inday)));
        sprintf(tbuf, "%d", dftime);
        sprintf(lbuf, fmt, fgc, bgc, pBM->uname, pBM->board, mdate, tbuf);
        fprintf(stdout, "%s\n", lbuf);
    }
    strftime(mdate, 30, timefmt, localtime(&now));
    fprintf(stdout, "\n    \n    统计时间∶%s\n", mdate);
}

int main(int argc, char **argv)
{
    BMInfo bmusr[MAXCHECK];
    char buf[256];
    FILE *fp;
    char uname[IDLEN+2];
    char board[STRLEN];
    time_t inday;
    char renming[STRLEN];
    const struct boardheader *bp;

    pBMInfo = bmusr;

    chdir(BBSHOME);
    resolve_boards();

    if ((fp=fopen("etc/bmat","r"))==NULL) {
        fprintf(stdout,"cannot read bmat file\n");
        exit(0);
    }

    while (fgets(buf, 256,fp)) {
        if (sscanf(buf,"%s %s %lu %s",uname, board, &inday,renming) != 4)
            continue;
        if (!strcasecmp(uname, "SYSOP") || !isascii(*uname))
            continue;
        bp = getbcache(board);
        if (bp==NULL)
            continue;
        if (!isNormalBoard(bp))
            continue;
        if (nBMCount >= MAXCHECK)
            continue;
        strcpy(pBMInfo[nBMCount].uname,uname);
        strcpy(pBMInfo[nBMCount].board, board);
        pBMInfo[nBMCount].inday = inday;
        nBMCount++;
    }
    fclose(fp);

    qsort(pBMInfo, nBMCount, sizeof(BMInfo), (cmpfunc *) cmpBMLogin);
    checkBMs();

    pBMInfo = NULL;

    return 0;
}
