#include <stdio.h>
#include <string.h>
#include <time.h>
#if !defined __LINUX__ && !defined FREEBSD
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#include "bbs.h"
//#include "shm_rel.c"

#define MAXUSRBM 16
#define MAXCHECK MAXBOARD*3

typedef int cmpfunc(const void *, const void *);

typedef struct {
    char uname[IDLEN + 2];
    int brd[MAXUSRBM];
    int uid;
    time_t lastlogin;
} BMInfo;

static BMInfo *pBMInfo;
static int nBMCount;
static int nWarningTime = 7;    /* seconds of 7 days    */
int cmpBMInfo(BMInfo * fst, BMInfo * snd)
{
    return (fst->uid - snd->uid);
}

int cmpBMLogin(BMInfo * fst, BMInfo * snd)
{
    return (fst->lastlogin - snd->lastlogin);
}

int isNormalBoard(const struct boardheader *brd)
{
    if (brd->level & PERM_NOZAP)
        return 1;
    else if (brd->level & PERM_LOGINOK)
        return 1;
    return (brd->level == 0);
}

int BM2User(int nbrd, char *uname)
{
    int i, j, uid;

    if (!strcasecmp(uname, "SYSOP") || id_invalid(uname))
        return 0;
    if (!isNormalBoard(&(bcache[nbrd])))
        return 0;
    /*  Anonymous board */
/*    if(!strcmp(uname, bcache[nbrd].filename) && bcache[nbrd].flag & ANONY_FLAG)*/
/*        return 0;*/
    for (i = 0; i < nBMCount; i++)
        if (!strcasecmp(pBMInfo[i].uname, uname))
            break;
    if (nBMCount <= i) {
        if (nBMCount >= MAXCHECK)
            return -1;
        nBMCount++;
        for (j = 0; j < MAXUSRBM; j++)
            pBMInfo[i].brd[j] = 0;
        uid = pBMInfo[i].uid = searchuser(uname);
        strcpy(pBMInfo[i].uname, uid ? getuserid2(uid) : uname);
        pBMInfo[i].lastlogin = uid ? 0 : -1;
    }
    for (j = 0; j < MAXUSRBM; j++)
        if (!(pBMInfo[i].brd[j])) {
            pBMInfo[i].brd[j] = nbrd + 1;
            break;
        }
    if (j >= MAXUSRBM)
        fprintf(stderr, "%s too many BMs!\n", uname);
    return i;
}

int isUserOnline(int uid)
{
    int i;

    for (i = 0; i < USHM_SIZE; i++) {
        if (!(utmpshm->uinfo[i].active))
            continue;
        if (uid == utmpshm->uinfo[i].uid)
            return (i + 1);
    }
    return 0;
}

int checkRecord(char *passfile)
{
    int fd;
    int i, lsize, uid, lerr = 0;
    struct userec lookupuser;

    lsize = sizeof(lookupuser);
    if (-1 == (fd = open(passfile, O_RDONLY, 0))) {
        fprintf(stderr, "Error opening %s\n", passfile);
        return (-1);
    }
    for (i = 0; i < nBMCount; i++) {
        if (!(uid = pBMInfo[i].uid))
            continue;
        lseek(fd, lsize * (uid - 1), SEEK_SET);
        if (read(fd, (char *) &lookupuser, lsize) != lsize) {
            fprintf(stderr, "Error reading user record for %s\n", pBMInfo[i].uname);
            lerr = -1;
            break;
        }
        pBMInfo[i].lastlogin = lookupuser.lastlogin;
    }
    close(fd);
    return lerr;
}

void checkBMs(void)
{
    char lbuf[256], tbuf[80], mdate[80];

/*    char * fmt = " %14.14s%36.36s%24.24s%8.8s";*/
    char *fmt = " \033[%2d;%2dm%-14.14s%-32.32s%-24.24s%8.8s\033[m";
    char *timefmt = "%Y年%m月%d日 %T";
    char *nulstr = "";
    int i, j, uid, brd, dftime, warningBM = 0;
    int fgc = 37, bgc = 44;
    time_t now;
    BMInfo *pBM;

    now = time(NULL);           /*  current time stamp  */
    sprintf(lbuf, fmt, fgc, bgc, "用户代号", "任职版面", "最近上站时间", "距今天数");
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
        if (!(uid = pBM->uid))  /* continue */
            ;
        if (!(brd = pBM->brd[0]))
            continue;           /*  maybe error???  */

        if (isUserOnline(uid))
            pBM->lastlogin = now;
        dftime = (now - pBM->lastlogin) / 86400;
        if (dftime >= nWarningTime && uid)
            warningBM++;                                   /*---*/
        if (uid)
            strftime(mdate, 30, timefmt, localtime(&(pBM->lastlogin)));
/*        else if(!(pBM->brd[1]) && bcache[brd-1].flag & ANONY_FLAG)*/
/*            strcpy(mdate, "[匿名版所属帐号]");*/
        else
            strcpy(mdate, "[该帐号已注销]");
        sprintf(tbuf, "%d", dftime);
        sprintf(lbuf, fmt, fgc, bgc, pBM->uname, bcache[brd - 1].filename, mdate, uid ? tbuf : nulstr);
        fprintf(stdout, "%s\n", lbuf);
        for (j = 1; j < MAXUSRBM; j++) {
            if (!(brd = pBM->brd[j]))
                break;
            sprintf(lbuf, fmt, fgc, bgc, nulstr, bcache[brd - 1].filename, nulstr, nulstr);
            fprintf(stdout, "%s\n", lbuf);
        }
    }
    strftime(mdate, 30, timefmt, localtime(&now));
    fprintf(stdout, "\n    超过 %d 天未上站的版主共 %d 位\n" "\n    统计时间∶%s\n", nWarningTime, warningBM, mdate);
}

int main(int argc, char **argv)
{
    BMInfo bmusr[MAXCHECK];
    int i;
    char tbuf[256];
    char passfile[256], bhome[256];
    char *ptr;

    chdir(BBSHOME);

    pBMInfo = bmusr;
    resolve_utmp();
    resolve_boards();
    resolve_ucache();
    if (2 <= argc) {
        if (strlen(argv[1]) >= 200) {
            fprintf(stderr, "BBSHOME too long...\n");
            exit(-1);
        }
        sprintf(bhome, "%s/", argv[1]);
    } else
        *bhome = 0;
    sprintf(passfile, "%s%s", bhome, PASSFILE);

    for (i = 0; i < get_boardcount(); i++) {
        if (!(bcache[i].filename[0]))
            continue;           /* invalid board    */
        if (bcache[i].BM[0] <= ' ')
            continue;           /* no board manager */
        strcpy(tbuf, bcache[i].BM);
        ptr = strtok(tbuf, " ");
        while (ptr) {
            BM2User(i, ptr);
            ptr = strtok(NULL, " ");
        }
    }

    qsort(pBMInfo, nBMCount, sizeof(BMInfo), (cmpfunc *) cmpBMInfo);
    if (checkRecord(passfile) < 0)
        exit(-1);
    qsort(pBMInfo, nBMCount, sizeof(BMInfo), (cmpfunc *) cmpBMLogin);
    checkBMs();

    pBMInfo = NULL;

    return 0;
}
