#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>

#include"bbs.h"

#define MAXLEN  256
#define MAXBUF  2048
#define MAXLINE  8192
#define SHORTLEN 60
#define MAXFILENUM 1024*2  /* max directories/files in a directory */
#define MAXBLOCK 1024768 /* max blob/clob block size */
#define DEFDBGFLAG 3       /* default debug flag */
#define MAXIGNORE 5       /* maximum ignored sql errcode */
#define DEFIDLEN 20
#define MAXDEPTH    15      /* maximum depth for indent */

#define DOTNAMES ".Names"
const short int gnsDirBackColor[] = { 41, 42, 43, 44, 45, 46, 47, 0 };
const short int gnsDirForeColor[] = { 37, 37, 37, 37, 37, 37, 30, 37 };

static const char *psNullStr = "";
static char psSrcDir[MAXLEN] = "";      /*      initial src dir */
static int gnDebugLevel = DEFDBGFLAG;   /*      global debug level      */
static int gnDirDepth = 0;      /*      current recursive depth of directories  */
static char psIndent[3 * MAXDEPTH + 5];  /* current indent */
static int gnIndentOutRange = 0;        /* any index in previous recursions that >= 100 */
static int gnIgnoreError = -1;  /*  0 : ignore error when executing */
static FILE *pLogFile = NULL;   /* event log file descriptor    */
static FILE *pOutFile = NULL;   /* output file descriptor       */

char *rtrim(char *buf, const char spc);
int PhaseArgs(int argc, char **argv);
int PhaseDir(char *dname, /* char * prefix, */ char *name, char *bmstr);
void PreExit(void);
int InitVar(void);

/*----------------------------------------------*
 * Main function -- program main entrance       *
 *----------------------------------------------*/
int main(int argc, char **argv)
{
    char title[MAXLEN] /*, lbuf[MAXLEN], prefix[SHORTLEN] = "" */ ;

    atexit(PreExit);
    if (InitVar() < 0)
        exit(1);
    if (0 != PhaseArgs(argc, argv)) {
        fprintf(pLogFile, "%s: Bad parameter.\n", argv[0]);
        exit(1);
    }
    if (gnDirDepth > MAXDEPTH) gnDirDepth = MAXDEPTH;
    sprintf(psIndent, "%*.*s", gnDirDepth * 3 + 2, gnDirDepth * 3 + 2, " ");
    fprintf(pOutFile, "\033[1;42;37m  %-14s%-54s%-9s\033[m\n" "-------------------------------------------------------------------------------\n", "序号", "精华区主题", "更新日期");
    if (0 != PhaseDir(psSrcDir, /* prefix, */ title, "SYSOP"))
        exit(1);
    exit(0);
}

/*----------------------------------------------*
 * Convert time_t to date string                *
 *----------------------------------------------*/
const char *CTime(time_t lt)
{
    static char ltbuf[32];

    strftime(ltbuf, 20, "%Y-%m-%d %T", localtime(&lt));
    return ltbuf;
}

/*----------------------------------------------*
 * Initialize global variables                  *
 *----------------------------------------------*/
int InitVar(void)
{
    return 0;
}

/*----------------------------------------------*
 * Function to be executed before program exit  *
 *----------------------------------------------*/
void PreExit(void)
{
    if (NULL != pLogFile && stderr != pLogFile) {
        fclose(pLogFile);
        pLogFile = NULL;
    }
    if (NULL != pOutFile && stdout != pOutFile) {
        fclose(pOutFile);
        pOutFile = NULL;
    }
}

/*-----------------------------------------------*
 * Phase directory with files and subdirectories *
 *-----------------------------------------------*/
int PhaseDir(char *dname, /* char * prefix, */ char *brdtitle, char *bmstr)
{
    int llen, lerr = 0;
    int nNumbCnt, nNameCnt, nPathCnt;
    int nNumber, lShift;
    int lntval;
    int lline;                  /*      line counter    */
    char idxfile[MAXLEN];       /*      .Name file path */
    char buf[MAXLINE];          /*      line buf        */
    char fname[MAXBUF];         /*      exact item full path with directory prefix      */
    char psName[MAXLEN];        /*      Name item buf   */
    char psPath[MAXLEN];        /*      Path item buf   */
    char psDirBM[MAXBUF] = "SYSOP";     /*      directory manager string        */
    char vcTitle[MAXBUF];       /*      directory title string  */
    char dirNameEn[MAXBUF];     /*  directory name ( with no path prefix ) */
    char psCTime[SHORTLEN];     /*  modify time string  */
    int oldIndentOutRange = gnIndentOutRange;
    /*    char psPrefix[SHORTLEN];*//*  output prefix string    */
    char *ptr, *end;
    FILE *idxfp;
    struct stat lst_p;
    time_t ltt_dir;             /*  last modification date of current directory    */

    if (NULL == dname)
        return gnIgnoreError;
    llen = strlen(dname) - 1;
    if ('/' == dname[llen])
        dname[llen] = 1;        /*      erase the trailing '/'  */

    if (0 != stat(dname, &lst_p) || !(S_ISDIR(lst_p.st_mode))) {
        if (DEFDBGFLAG <= gnDebugLevel)
            fprintf(pLogFile, "Error: there's no directory %s!\n", dname);
        return gnIgnoreError;
    }
    ltt_dir = lst_p.st_mtime;

    if (4 <= gnDebugLevel)
        fprintf(pLogFile, "%*sEntering directory %s ...\n", gnDirDepth * 2, psNullStr, dname);

    snprintf(idxfile, sizeof(idxfile), "%s/%s", dname, DOTNAMES); /*      .Names  */
    if (NULL == (idxfp = fopen(idxfile, "r+t"))) {
        if (1 <= gnDebugLevel)
            fprintf(pLogFile, "  Error opening index file %s\n", idxfile);
#ifdef _DEBUG_
        return 0;
#else
        return gnIgnoreError;
#endif
    }

    ptr = strrchr(dname, '/');
    if (NULL == ptr)
        strcpy(dirNameEn, dname);
    else
        strcpy(dirNameEn, ptr + 1);

    if (NULL != brdtitle && strlen(brdtitle) > 0)
        strcpy(vcTitle, brdtitle);
    else
        strcpy(vcTitle, "无标题");

    gnDirDepth++;

    lline = 0;                  /*      line counter    */
    nNumbCnt = nNameCnt = nPathCnt = 0;
    while (!feof(idxfp)) {
        if (NULL == fgets(buf, MAXLINE, idxfp)) {
            if (4 <= gnDebugLevel)
                fprintf(pLogFile, "%*sLeaving directory %s ...\n", gnDirDepth * 2 - 2, psNullStr, dname);
            break;
        }
        lline++;
        llen = strlen(buf);
        if (llen <= 0)
            continue;           /*      null line       */
        buf[llen - 1] = 0;      /*      erase the '\n' character        */
        rtrim(buf, ' ');        /*      erase trailing spaces   */

        if ('#' == *buf) {      /*      comment line */
            if (!strncmp(buf, "# Title=", 8) && lline < 3) {    /*      Title line(must in first 3 lines!)      */
                if (NULL != (ptr = strrchr(buf, '('))) {
                    if (!strncmp(ptr, "(BM: ", 5)) {
                        *ptr = 0;       /* form title str in buf */
                        strcpy(psDirBM, ptr + 5);
                        end = strchr(psDirBM, ')');
                        if (NULL != end)
                            *end = 0;   /* BM: ... field */
                        else
                            *psDirBM = 0;
                        end = strchr(psDirBM, ' ');     /* only one id allowed */
                        if (NULL != end)
                            *end = 0;
                    } else if (NULL != bmstr && strlen(bmstr) > 0)
                        strcpy(psDirBM, bmstr);
                }
                rtrim(buf + 8, ' ');    /* delete trailing spaces */
                llen = strlen(buf + 8);
                if (0 == llen) {
                    if (4 <= gnDebugLevel)      /* Null title line */
                        fprintf(pLogFile, "Directory has NULL title : %s\n", dname);
                } else
                    strcpy(vcTitle, buf + 8);
                /*    strftime(buf, 20, "%Y-%m-%d %T", localtime(&ltt_dir));*/
            }
            continue;
        } else if (!strncmp(buf, "Name=", 5)) {
            nNameCnt++;
            if (nNameCnt <= nPathCnt)   /* ought to be an error */
                ;
            strcpy(psName, buf + 5);
            continue;
        } else if (!strncmp(buf, "Path=~/", 7)) {
            nPathCnt++;
            if (nPathCnt <= nNumbCnt)   /* ought to be an error */
                ;
            strcpy(psPath, buf + 6);
            continue;
        } else if (!strncmp(buf, "Attach=", 7)) {
            continue;
        } else if (!strncmp(buf, "Numb=", 5)) {
            nNumbCnt++;
            nNumber = atoi(buf + 5);
            if (nNumber >= 1000)
                lShift = 4;
            else
                lShift = 3;
            if (nNameCnt != nNumbCnt || nPathCnt != nNumbCnt) { /* ought to be an error */
                *psPath = *psName = 0;
                continue;
            }
        } else {
            if (DEFDBGFLAG <= gnDebugLevel)
                fprintf(pLogFile, "Error in line %d of file %s :\n%s\n", lline, dname, buf);
            continue;
        }

        if (strstr(psName , "(BM: BMS)") || strstr(psName ,"(BM: SYSOPS)"))
            continue;
        if (strlen(psPath) <= 0)
            continue;           /* not to form loops */
        snprintf(fname, sizeof(fname), "%s%s", dname, psPath);
        if (0 != lstat(fname, &lst_p)) {
            if (DEFDBGFLAG <= gnDebugLevel)
                fprintf(pLogFile, "Error stating file %s !\n", fname);
            continue;
        }
        if (time(NULL) - lst_p.st_mtime < 3600 * 7 * 24) {
            strcpy(psCTime, CTime(lst_p.st_mtime));     /* use st_mtime, not st_ctime */
            psCTime[10] = 0;    /* only date needed */
        } else
            *psCTime = 0;

        if (gnDirDepth > MAXDEPTH || nNumber >= 100 || oldIndentOutRange)
            gnIndentOutRange = 1;
        else
            gnIndentOutRange = 0;

        if (S_ISDIR(lst_p.st_mode)) {   /*      sub-directory   */
            if (NULL != (ptr = strrchr(psName, '('))) {
                if (!strncmp(ptr, "(BM: ", 5)) {
                    *ptr = 0;   /* form title str in buf */
                    strcpy(buf, ptr + 5);
                    end = strchr(buf, ')');
                    if (NULL != end)
                        *end = 0;       /* BM: ... field */
                    else
                        *buf = 0;
                    end = strchr(buf, ' ');     /* only one id allowed */
                    if (NULL != end)
                        *end = 0;
                } else if (NULL != bmstr && strlen(bmstr) > 0)
                    strcpy(buf, bmstr);
                else
                    strcpy(buf, psDirBM);
            }
            rtrim(psName, ' '); /* delete trailing spaces */
            llen = strlen(psName);
            if (0 == llen) {
                if (4 <= gnDebugLevel)  /* Null title line */
                    fprintf(pLogFile, "Directory has NULL title : %s\n", dname);
            }
            lntval = gnDirDepth >= 15 ? 10 : 70 - gnDirDepth * 3;
            if (lntval > 39)
                lntval = 39;
            if (lntval > llen)
                lntval = llen;

            if (!gnIndentOutRange) {
                fprintf(pOutFile, "\033[33m%s%2d.\033[37m [\033[1;32m目录\033[37m]"
                        "\033[%d;%dm%.*s\033[0;31m%*.10s\033[37m\n",
                        psIndent, nNumber,
                        gnDirDepth < 7 ? gnsDirBackColor[gnDirDepth] : 0, gnDirDepth < 7 ? gnsDirForeColor[gnDirDepth] : 37, lntval, psName, 70 - gnDirDepth * 3 - lntval /*llen */ ,
                        psCTime);
            } else {
                fprintf(pOutFile, "%*.*s\033[33m%4d\033[37m. [\033[1;32m目录\033[37m]"
                        "\033[%d;%dm%.*s\033[0;31m%*.10s\033[37m\n",
                        (gnDirDepth - 1) * 3, (gnDirDepth - 1) * 3, " ", nNumber,
                        gnDirDepth < 7 ? gnsDirBackColor[gnDirDepth] : 0, gnDirDepth < 7 ? gnsDirForeColor[gnDirDepth] : 37, lntval, psName, 70 - gnDirDepth * 3 - lntval /*llen */ ,
                        psCTime);
            }

            if (!gnIndentOutRange)
                sprintf(&psIndent[gnDirDepth * 3 - 1], "%2d.", nNumber);
            if (PhaseDir(fname, /*psPrefix, */ psName, psDirBM) < 0) {
                lerr = -1;
                break;
            }
            if (!gnIndentOutRange)
                psIndent[gnDirDepth * 3 - 1] = '\0';
        } else if (S_ISREG(lst_p.st_mode)) {    /*      regular file    */
            lntval = gnDirDepth >= 15 ? 10 : 70 - gnDirDepth * 3;
            if (lntval > 39)
                lntval = 39;
            /*   if(lntval > llen) lntval = llen;*/
            if (!gnIndentOutRange) {
                fprintf(pOutFile, "\033[33m%s\033[37m%2d. [\033[1;36m文件\033[0;37m]"
                        "%-*.*s\033[31m%*.10s\033[37m\n", psIndent, nNumber, lntval, lntval, psName, 70 - gnDirDepth * 3 - lntval, psCTime);
            } else {
                fprintf(pOutFile, "%*.*s%4d. [\033[1;36m文件\033[0;37m]"
                        "%-*.*s\033[31m%*.10s\033[37m\n", (gnDirDepth - 1) * 3, (gnDirDepth - 1) * 3, " ", nNumber, lntval, lntval, psName, 70 - gnDirDepth * 3 - lntval, psCTime);
            }
        } else {                /*      unknown item type       */
            if (DEFDBGFLAG <= gnDebugLevel)
                fprintf(pLogFile, "Error: Unknown item encountered at line %d of %s\n", lline, idxfile);
            continue;
        }
    }                           /*      while(!feof(idxfp))     */
    fclose(idxfp);
    gnIndentOutRange = oldIndentOutRange;
    gnDirDepth--;
    return lerr;
}

/*----------------------------------------------*
 * Phase command line parameters                *
 *----------------------------------------------*/
int PhaseArgs(int argc, char **argv)
{
    int idx = 0, len;

    pLogFile = stderr;
    pOutFile = stdout;
    if (1 == argc)
        return -1;
    while (++idx < argc) {
        switch (*(argv[idx])) {
            case '-':              /*      parameters      */
                if ('d' == argv[idx][1]) {  /* Debug Level */
                    idx++;
                    if (argc <= idx)
                        return -1;
                    if (DEFDBGFLAG != gnDebugLevel)
                        fprintf(pLogFile, "Option -v encountered, -d skipped...\n");
                    else if (1 != sscanf(argv[idx], "%d", &gnDebugLevel))
                        return -1;
                } else if ('i' == argv[idx][1]) {   /* assume initial depth */
                    idx++;
                    if (argc <= idx)
                        return -1;
                    if (1 != sscanf(argv[idx], "%d", &gnDirDepth))
                        return -1;
                } else if ('o' == argv[idx][1]) {   /* output file */
                    idx++;
                    if (argc <= idx)
                        return -1;
                    if (stdout == pOutFile)
                        if (NULL == (pOutFile = fopen(argv[idx], "a+"))) {
                            pOutFile = stdout;
                            fprintf(pLogFile, "Error opening output file %s.\n", argv[idx]);
                            return -1;
                        }
                } else if ('l' == argv[idx][1]) {   /* log infor into file */
                    idx++;
                    if (argc <= idx)
                        return -1;
                    if (stderr == pLogFile)
                        if (NULL == (pLogFile = fopen(argv[idx], "a+"))) {
                            pLogFile = stderr;
                            fprintf(pLogFile, "Error opening log file %s.\n", argv[idx]);
                            return -1;
                        }
                } else if ('v' == argv[idx][1]) {   /* verbose mode */
                    if (DEFDBGFLAG != gnDebugLevel) {
                        fprintf(pLogFile, "Option -v encountered, -d skipped...\n");
                        gnDebugLevel = DEFDBGFLAG + 1;
                    } else
                        gnDebugLevel++;
                    if ('v' == argv[idx][2])
                        gnDebugLevel++;
                } else if ('n' == argv[idx][1]) {   /* ignore error */
                    gnIgnoreError = 0;
                } else {            /* unknown command parameter */
                    return -1;
                }
                break;
            default:               /*      root dir of source directory tree       */
                if (strlen(psSrcDir) > 0)
                    return -1;
                else if (strlen(argv[idx]) >= MAXLEN - 1) {
                    fprintf(pLogFile, "Argument too long.");
                    return -1;
                }
                strcpy(psSrcDir, argv[idx]);
                len = strlen(psSrcDir) - 1;
                if ('/' == psSrcDir[len])
                    psSrcDir[len] = 0;
                break;
        }
    }
    if (strlen(psSrcDir) <= 0)
        return -1;
    return 0;
}

/*----------------------------------------------*
 * Erase given character at end of string       *
 *----------------------------------------------*/
char *rtrim(char *buf, const char spc)
{
    char *ptr;
    int nlen;

    if (NULL == buf)
        return NULL;
    nlen = strlen(buf) - 1;
    ptr = buf + nlen;
    for (; ptr >= buf; ptr--) {
        if (spc == *ptr)
            *ptr = 0;
        else
            break;
    }
    return buf;
}
