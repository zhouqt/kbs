/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define EXTERN

#include "bbs.h"
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/shm.h>
extern char *getenv();
static const char *invalid[] = {
   /* "unknown@",*/
    "root@",
    "gopher@",
    "bbs@",
    "guest@",
    NULL
};
static char modestr[STRLEN];
static char hh_mm_ss[8];
int my_system(const char *cmdstring)
{
    pid_t pid;
    int status;

    if (!cmdstring)
        return 1;
    switch (pid = fork()) {
    case -1:
        status = -1;
        break;
    case 0:
        {
            int fd = open("/dev/zero", O_RDWR);

            dup2(fd, 2);
            dup2(fd, 1);
            if (fd != 1 && fd != 2)
                close(fd);
            execl("/bin/sh", "sh", "-c", cmdstring, NULL);
            _exit(127);
        }
    default:
        while (waitpid(pid, &status, 0) < 0)
            if (errno != EINTR) {
                status = -1;
                break;
            }
        break;
    }
    return status;
}
char *idle_str(struct user_info *uent)
{
    time_t now, diff;
    int hh, mm;

    now = time(0);
    diff = now - uent->freshtime;
    if (diff == now)            /* @#$#!@$#@! */
        diff = 0;

#ifdef DOTIMEOUT
    /*
     * the 60 * 60 * 24 * 5 is to prevent fault /dev mount from
     * kicking out all users 
     */
    if ((diff > IDLE_TIMEOUT) && (diff < 60 * 60 * 24 * 5))
        kill(uent->pid, SIGHUP);

#endif                          /*  */
    hh = diff / 3600;
    mm = (diff / 60) % 60;
    if (hh > 0)
        snprintf(hh_mm_ss, sizeof(hh_mm_ss), "%2d:%02d", hh, mm);

    else if (mm > 0)
        sprintf(hh_mm_ss, "%d", mm);

    else
        sprintf(hh_mm_ss, "   ");
    return hh_mm_ss;
}
char *modestring(int mode, int towho, int complete, char *chatid)
{
    struct userec urec;


    /*
     * Leeward: 97.12.18: Below removing ' characters for more display width 
     */
    if (chatid) {
        if (complete)
            sprintf(modestr, "%s %s", ModeType(mode), chatid);

        else
            return ((char *) ModeType(mode));
        return (modestr);
    }
    if (mode != TALK && mode != PAGE && mode != QUERY)
        return ((char *) ModeType(mode));

    /*---	modified by period	2000-10-20	---*
        if (get_record(PASSFILE, &urec, sizeof(urec), towho) == -1)
            return (ModeType(mode));
    ---*/
    if (complete) {
        if (getuserid(urec.userid, towho) != towho)
            return (char *) ModeType(mode);
        sprintf(modestr, "%s %s", ModeType(mode), urec.userid);
    }

    else
        return ((char *) ModeType(mode));
    return (modestr);
}

int dashf(char *fname)
{
    struct stat st;

    return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}

int dashd(char *fname)
{
    struct stat st;

    return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}


/* rrr - Snagged from pbbs 1.8 */

#ifdef BBSMAIN
#ifdef CAN_EXEC
#define LOOKFIRST  (0)
#define LOOKLAST   (1)
#define QUOTEMODE  (2)
#define MAXCOMSZ (1024)
#define MAXARGS (40)
#define MAXENVS (20)
#define BINDIR "/bin/"
char *bbsenv[MAXENVS];
int numbbsenvs = 0;
#endif
#endif

/* Case Independent strncmp */
int ci_strncmp(register char *s1, register char *s2, register int n)
{
    char c1, c2;

    while (n-- > 0) {
        c1 = *s1++;
        c2 = *s2++;
        if (c1 >= 'a' && c1 <= 'z')
            c1 &= 0xdf;
        if (c2 >= 'a' && c2 <= 'z')
            c2 &= 0xdf;
        if (c1 != c2)
            return (c1 - c2);
        if (c1 == 0)
            return 0;
    }
    return 0;
}

int ci_strcmp(register char *s1, register char *s2)
{
    char c1, c2;

    while (1) {
        c1 = *s1++;
        c2 = *s2++;
        if (c1 >= 'a' && c1 <= 'z')
            c1 &= 0xdf;
        if (c2 >= 'a' && c2 <= 'z')
            c2 &= 0xdf;
        if (c1 != c2)
            return (c1 - c2);
        if (c1 == 0)
            return 0;
    }
}

#ifdef BBSMAIN
#ifdef CAN_EXEC
int bbssetenv(char *env, char *val)
{
    register int i, len;

    if (numbbsenvs == 0)
        bbsenv[0] = NULL;
    len = strlen(env);
    for (i = 0; bbsenv[i]; i++)
        if (!strncasecmp(env, bbsenv[i], len))
            break;
    if (i >= MAXENVS)
        return -1;
    if (bbsenv[i])
        free(bbsenv[i]);

    else
        bbsenv[++numbbsenvs] = NULL;
    bbsenv[i] = malloc(strlen(env) + strlen(val) + 2);
    strcpy(bbsenv[i], env);
    strcat(bbsenv[i], "=");
    strcat(bbsenv[i], val);
    return 0;
}


int do_exec(char *com, *wd)
{
    char path[MAXPATHLEN];
    char pcom[MAXCOMSZ];
    char *arglist[MAXARGS];
    char *tz;
    register int i, len;
    register int argptr;
    int status, pid, w;
    int pmode;
    void (*isig) (), (*qsig) ();

    strncpy(path, BINDIR, MAXPATHLEN);
    strncpy(pcom, com, MAXCOMSZ);
    len = Min(strlen(com) + 1, MAXCOMSZ);
    pmode = LOOKFIRST;
    for (i = 0, argptr = 0; i < len; i++) {
        if (pcom[i] == '\0')
            break;
        if (pmode == QUOTEMODE) {
            if (pcom[i] == '\001') {
                pmode = LOOKFIRST;
                pcom[i] = '\0';
                continue;
            }
            continue;
        }
        if (pcom[i] == '\001') {
            pmode = QUOTEMODE;
            arglist[argptr++] = &pcom[i + 1];
            if (argptr + 1 == MAXARGS)
                break;
            continue;
        }
        if (pmode == LOOKFIRST) {
            if (pcom[i] != ' ') {
                arglist[argptr++] = &pcom[i];
                if (argptr + 1 == MAXARGS)
                    break;
                pmode = LOOKLAST;
            } else
                continue;
        }
        if (pcom[i] == ' ') {
            pmode = LOOKFIRST;
            pcom[i] = '\0';
        }
    }
    arglist[argptr] = NULL;
    if (argptr == 0)
        return -1;
    if (*arglist[0] == '/')
        strncpy(path, arglist[0], MAXPATHLEN);

    else
        strncat(path, arglist[0], MAXPATHLEN);

#ifdef IRIX
    if ((pid = fork()) == 0) {
#else                           /*  */
    if ((pid = vfork()) == 0) {
#endif                          /*  */
        if (wd)
            if (chdir(wd)) {

                /*
                 * change by KCN 1999.09.08
                 * fprintf(stderr,"Unable to chdir to '%s'\n",wd) ;
                 */
                prints("Unable to chdir to '%s'\n", wd);
                oflush();
                exit(-1);
            }
        bbssetenv("PATH", "/bin:.");
        bbssetenv("TERM", "vt100");
        bbssetenv("USER", currentuser->userid);
        bbssetenv("USERNAME", currentuser->username);

        /*
         * added for tin's reply to 
         */
        bbssetenv("REPLYTO", currentuser->email);
        bbssetenv("FROMHOST", fromhost);

        /*
         * end of insertion 
         */
        if ((tz = getenv("TZ")) != NULL)
            bbssetenv("TZ", tz);
        if (numbbsenvs == 0)
            bbsenv[0] = NULL;
        execve(path, arglist, bbsenv);

        /*
         * change by KCN 1999.09.08
         * fprintf(stderr,"EXECV FAILED... path = '%s'\n",path) ;
         */
        prints("EXECV FAILED... path = '%s'\n", path);
        oflush();
        exit(-1);
    }
    isig = signal(SIGINT, SIG_IGN);
    qsig = signal(SIGQUIT, SIG_IGN);
    while ((w = wait(&status)) != pid && w != 1)

        /*
         * NULL STATEMENT 
         */ ;
    signal(SIGINT, isig);
    signal(SIGQUIT, qsig);
    return ((w == -1) ? w : status);
}

#endif                          /* CAN _EXEC */
#endif                          /*  */

#ifdef kill
#undef kill
#endif                          /*  */
int safe_kill(int x, int y)
{
    if (x <= 0)
        return -1;
    return kill(x, y);
}

char *Cdate(time_t clock)
{                               /* 时间 --> 英文 */

    /*
     * Leeward 2000.01.01 Adjust year display for 20** 
     */
    static char foo[24 /*22 */ ];
    struct tm *mytm = localtime(&clock);

    strftime(foo, 24 /*22 */ , "%Y-%m-%d %T %a" /*"%D %T %a" */ , mytm);
    return (foo);
}

char *Ctime(time_t clock)
{                               /* 时间 转换 成 英文 */
    char *foo;
    char *ptr = ctime(&clock);

    if ((foo = strchr(ptr, '\n')) != NULL)
        *foo = '\0';
    return (ptr);
}
int Isspace(char ch)
{
    return (ch == ' ' || ch == '\t' || ch == 10 || ch == 13);
}
char *nextword(const char **str, char *buf, int sz)
{
    while (Isspace(**str))
        (*str)++;
    while (**str && !Isspace(**str)) {
        if (sz > 0) {
            if (sz > 1)
                *buf++ = **str;
            sz--;
        }
        (*str)++;
    }
    *buf = 0;
    while (Isspace(**str))
        (*str)++;
    return buf;
}


void attach_err(int shmkey, char *name)
{
#ifdef BBSMAIN
    prints("Error! %s error! key = %x.\n", name, shmkey);
    oflush();
#else
    char buf[256];

    sprintf(buf, "Error! %s error! key = %x.", name, shmkey);
    perror(buf);
#endif
    exit(1);
}

void *attach_shm(char *shmstr, int defaultkey, int shmsize, int *iscreate)
{
    return attach_shm1(shmstr, defaultkey, shmsize, iscreate, 0, NULL);
}
void *attach_shm1(char *shmstr, int defaultkey, int shmsize, int *iscreate, int readonly, void *shmaddr)
{
    void *shmptr;
    int shmkey, shmid;

    if (shmstr)
        shmkey = sysconf_eval(shmstr, defaultkey);

    else
        shmkey = 0;
    if (shmkey < 1024)
        shmkey = defaultkey;
    shmid = shmget(shmkey, shmsize, 0);
    if (shmid < 0) {
        if (readonly) {
            attach_err(shmkey, "shmget:readonly");
            return 0;
        }
        shmid = shmget(shmkey, shmsize, IPC_CREAT | 0660);      /* modified by dong , for web application , 1998.12.1 */
        *iscreate = true;
        if (shmid < 0) {
            attach_err(shmkey, "shmget");
            exit(0);
        }
        shmptr = (void *) shmat(shmid, shmaddr, 0);
        if (shmptr == (void *) -1) {
            attach_err(shmkey, "shmat 1");
            exit(0);
        } else
            memset(shmptr, 0, shmsize);
    } else {
        *iscreate = 0;
        if (readonly)
            shmptr = (void *) shmat(shmid, shmaddr, SHM_RDONLY);

        else
            shmptr = (void *) shmat(shmid, shmaddr, 0);
        if (shmptr == (void *) -1) {
            attach_err(shmkey, "shmat 2");
            exit(0);
        }
    } return shmptr;
}

char *cexp(exp)
int exp;
{
    int expbase = 0;

    if (exp == -9999)
        return "没等级";
    if (exp <= 100 + expbase)
        return "新手上路";
    if (exp > 100 + expbase && exp <= 450 + expbase)
        return "一般站友";
    if (exp > 450 + expbase && exp <= 850 + expbase)
        return "中级站友";
    if (exp > 850 + expbase && exp <= 1500 + expbase)
        return "高级站友";
    if (exp > 1500 + expbase && exp <= 2500 + expbase)
        return "老站友";
    if (exp > 2500 + expbase && exp <= 3000 + expbase)
        return "长老级";
    if (exp > 3000 + expbase && exp <= 5000 + expbase)
        return "本站元老";
    if (exp > 5000 + expbase)
        return "开国大老";

}

char *cperf(perf)
int perf;
{

    if (perf == -9999)
        return "没等级";
    if (perf <= 5)
        return "赶快加油";
    if (perf > 5 && perf <= 12)
        return "努力中";
    if (perf > 12 && perf <= 35)
        return "还不错";
    if (perf > 35 && perf <= 50)
        return "很好";
    if (perf > 50 && perf <= 90)
        return "优等生";
    if (perf > 90 && perf <= 140)
        return "太优秀了";
    if (perf > 140 && perf <= 200)
        return "本站支柱";
    if (perf > 200)
        return "神～～";

}

int countexp(struct userec *udata)
{
    int exp;

    if (!strcmp(udata->userid, "guest"))
        return -9999;
    exp = udata->numposts + /*post_in_tin( udata->userid ) */ +udata->numlogins / 5 + (time(0) - udata->firstlogin) / 86400 + udata->stay / 3600;
    return exp > 0 ? exp : 0;
}
int countperf(struct userec *udata)
{
    int perf;
    int reg_days;

    if (!strcmp(udata->userid, "guest"))
        return -9999;
    reg_days = (time(0) - udata->firstlogin) / 86400 + 1;
    perf = ((float) (udata->numposts /*+post_in_tin( udata->userid ) */ ) / (float) udata->numlogins +
            (float) udata->numlogins / (float) reg_days) * 10;
    return perf > 0 ? perf : 0;
}

/*
 * 根据阅读模式 取某版 目录路径 
 */
char *setbdir(int digestmode, char *buf, char *boardname)
{
    char dir[STRLEN];

    switch (digestmode) {
    case DIR_MODE_DIGEST:
        strcpy(dir, DIGEST_DIR);
        break;
    case DIR_MODE_THREAD:
        strcpy(dir, THREAD_DIR);
        break;
    case DIR_MODE_MARK:
        strcpy(dir, ".MARK");
        break;
    case DIR_MODE_DELETED:
        strcpy(dir, ".DELETED");
        break;
    case DIR_MODE_JUNK:
        strcpy(dir, ".JUNK");
        break;
    case DIR_MODE_ORIGIN:
        strcpy(dir, ".ORIGIN");
        break;
    case DIR_MODE_AUTHOR:
        sprintf(dir, ".AUTHOR.%s", currentuser->userid);
        break;
    case DIR_MODE_TITLE:
        sprintf(dir, ".TITLE.%s", currentuser->userid);
        break;
	case DIR_MODE_ZHIDING:
		strcpy(dir, DING_DIR);
		break;
    case DIR_MODE_NORMAL:
    default:
        strcpy(dir, DOT_DIR);
        break;
    }
    sprintf(buf, "boards/%s/%s", boardname, dir);
    return buf;
}

char *sethomefile(char *buf, const char *userid, const char *filename)
{                               /*取某用户文件 路径 */
    if (isalpha(userid[0]))     /* 加入错误判断,提高容错性, alex 1997.1.6 */
        sprintf(buf, "home/%c/%s/%s", toupper(userid[0]), userid, filename);

    else
        sprintf(buf, "home/wrong/%s/%s", userid, filename);
    return buf;
}
char *sethomepath(char *buf, char *userid)
{                               /* 取 某用户 的home */
    if (isalpha(userid[0]))     /* 加入错误判断,提高容错性, alex 1997.1.6 */
        sprintf(buf, "home/%c/%s", toupper(userid[0]), userid);

    else
        sprintf(buf, "home/wrong/%s", userid);
    return buf;
}
char *setmailfile(char *buf, const char *userid, const char *filename)
{                               /* 取某用户mail文件 路径 */
    if (isalpha(userid[0]))     /* 加入错误判断,提高容错性, alex 1997.1.6 */
        sprintf(buf, "mail/%c/%s/%s", toupper(userid[0]), userid, filename);

    else
        sprintf(buf, "mail/wrong/%s/%s", userid, filename);
    return buf;
}
char *setmailpath(char *buf, char *userid)
{                               /* 取 某用户 的mail */
    if (isalpha(userid[0]))     /* 加入错误判断,提高容错性, alex 1997.1.6 */
        sprintf(buf, "mail/%c/%s", toupper(userid[0]), userid);

    else
        sprintf(buf, "mail/wrong/%s", userid);
    return buf;
}
char *setbpath(char *buf, char *boardname)
{                               /* 取某版 路径 */
    strcpy(buf, "boards/");
    strcat(buf, boardname);
    return buf;
}
char *setbfile(char *buf, char *boardname, char *filename)
{                               /* 取某版下文件 */
    sprintf(buf, "boards/%s/%s", boardname, filename);
    return buf;
}
void RemoveMsgCountFile(char *userID)
{
    char fname[STRLEN];

    sethomefile(fname, userID, "msgcount");
    unlink(fname);
} int id_invalid(char *userid)
{
    char *s;

    if (!isalpha(userid[0]))
        return 1;
    for (s = userid; *s != '\0'; s++) {
#ifdef NINE_BUILD
        if (*s < 1 || !isalpha(*s)) {
#else
        if (*s < 1 || !isalnum(*s)) {
#endif
            return 1;
        }
    }
    return 0;
}
int seek_in_file(char filename[STRLEN], char seekstr[STRLEN])
{
    FILE *fp;
    char buf[STRLEN];
    char *namep;

    if ((fp = fopen(filename, "r")) == NULL)
        return 0;
    while (fgets(buf, STRLEN, fp) != NULL) {
        namep = (char *) strtok(buf, ": \n\r\t");
        if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static struct public_data *publicshm;
struct public_data *get_publicshm()
{
    int iscreate;

    if (publicshm == NULL) {
        publicshm = attach_shm1(NULL, PUBLIC_SHMKEY, sizeof(*publicshm), &iscreate, 1, NULL);   /* attach public share memory readonly */
        if (iscreate) {
            bbslog("4bbstime", "time daemon not start");
            exit(1);
        }
    }
    return publicshm;
}

void detach_publicshm()
{
    shmdt(publicshm);
    publicshm = NULL;
}

int getwwwguestcount()
{
    return publicshm->www_guest_count;
}

void save_maxuser()
{
    FILE *fp;

    publicshm->max_user=get_utmp_number() + getwwwguestcount();
    publicshm->max_wwwguest= getwwwguestcount();
    if (NULL != (fp = fopen("etc/maxuser", "w"))) {
        fprintf(fp, "%d %d", publicshm->max_user,publicshm->max_wwwguest);
        fclose(fp);
    }
}

void bbssettime(time_t now)
{
    int iscreate;

    if (publicshm == NULL) {
        publicshm = (struct public_data *) attach_shm1(NULL, PUBLIC_SHMKEY, sizeof(*publicshm), &iscreate, 0, NULL);    /* attach public share memory */
        if (iscreate) {
            FILE *fp;

            if (NULL != (fp = fopen("etc/maxuser", "r"))) {
                fscanf(fp, "%d %d", &publicshm->max_user,&publicshm->max_wwwguest);
                fclose(fp);
            }
            /*
             * 初始化public共享内存区 
             */
            /*
             * 开始的sysconf.img版本号为0 
             */
            unlink("sysconf.img.0");
            publicshm->sysconfimg_version = 0;

        }
    }
    publicshm->nowtime = now;
    return;
}

int setpublicshmreadonly(int readonly)
{
    int iscreate;

    shmdt(publicshm);
    if (readonly)
        publicshm = (struct public_data *) attach_shm1(NULL, PUBLIC_SHMKEY, sizeof(*publicshm), &iscreate, 1, publicshm);

    else
        publicshm = (struct public_data *) attach_shm1(NULL, PUBLIC_SHMKEY, sizeof(*publicshm), &iscreate, 0, publicshm);
    return (publicshm == NULL);
}

time_t bbstime(time_t * t)
{
    int iscreate;

    if (publicshm == NULL) {
        publicshm = attach_shm1(NULL, PUBLIC_SHMKEY, sizeof(*publicshm), &iscreate, 1, NULL);   /* attach public share memory readonly */
        if (iscreate) {
            bbslog("4bbstime", "time daemon not start");
            exit(1);
        }
    }
    if (t)
        *t = publicshm->nowtime;
    return publicshm->nowtime;
}

int bad_user_id(char *userid)
{
    FILE *fp;
    char buf[STRLEN];
    char *ptr, ch;
    int i;

    i = 0;
    ptr = userid;
    if (!strcasecmp(userid, "deliver") || !strcasecmp(userid, "new"))
        return 1;
    while ((ch = *ptr++) != '\0') {
        i++;
        if (!isalnum(ch) && ch != '_')
            return 1;
    }
    if (i < 2)
        return 1;
    if ((fp = fopen(".badname", "r")) != NULL) {
        while (fgets(buf, STRLEN, fp) != NULL) {
            ptr = strtok(buf, " \n\t\r");
            if (ptr != NULL && *ptr != '#') {
                if (strcasecmp(ptr, userid) == 0) {
                    if (ptr[13] > 47 && ptr[13] < 58) { /*Haohmaru.99.12.24 */
                        char timebuf[50];

                        time_t t, now;

                        strncpy(timebuf, ptr + 13, 49);
                        timebuf[49] = 0;
                        ptr = timebuf;
                        while (isdigit(*ptr))
                            ptr++;
                        *ptr = 0;
                        t = atol(timebuf);
                        now = time(0);
                        if (now - t > 24 * 30 * 3600) {
                            fclose(fp);
                            return 0;
                        }
                    }
                    fclose(fp);
                    return 1;
                }
            }
            bzero(buf, STRLEN);
        }
        fclose(fp);
    }
    return 0;
}
int valid_ident(char *ident)
{
    int i;

    for (i = 0; invalid[i] != NULL; i++)
        if (strstr(ident, invalid[i]) != NULL)
            return 0;
    return 1;
}
struct _tag_t_search {
    struct user_info *result;
    int pid;
};
int _t_search(struct user_info *uentp, struct _tag_t_search *data, int pos)
{
    if (data->pid == 0) {
        data->result = uentp;
        return QUIT;
    }
    data->result = uentp;
    if (uentp->pid == data->pid)
        return QUIT;
    UNUSED_ARG(pos);
    return 0;
}
struct user_info *t_search(char *sid, int pid)
{
    struct _tag_t_search data;

    data.pid = pid;
    data.result = NULL;
    apply_utmp((APPLY_UTMP_FUNC) _t_search, 20, sid, &data);
    return data.result;
}

int getuinfopid()
{

#ifdef BBSMAIN
    return uinfo.pid;

#else                           /*  */
    return 1;

#endif                          /*  */
}
int cmpinames(const char *userid, const char *uv)
{                               /* added by Luzi 1997.11.28 */
    return !strcasecmp(userid, uv);
}
int cmpfnames(char *userid, struct friends *uv)
{
    return !strcasecmp(userid, uv->id);
}
int cmpfileinfoname(char *filename, struct fileheader *fi)
{
    return !strncmp(filename, fi->filename, FILENAME_LEN);
}
int cmpfileid(int *id, struct fileheader *fi)
{
	return (*id==fi->id);
}

int canIsend2(struct userec *user, char *userid)
{                               /* Leeward 98.04.10 */
    char buf[IDLEN + 1];
    char path[256];

    if (HAS_PERM(user, PERM_SYSOP))
        return true;
    sethomefile(path, userid, "ignores");
    if (search_record(path, buf, IDLEN + 1, (RECORD_FUNC_ARG) cmpinames, currentuser->userid))
        return false;
    /*
     * sethomefile(path, userid, "/bads");
     * if (search_record(path, buf, IDLEN + 1, (RECORD_FUNC_ARG) cmpinames, currentuser->userid))
     * return false;
     * 
     * else
     */
    return true;
}

sigjmp_buf bus_jump;
void sigbus(int signo)
{
    siglongjmp(bus_jump, 1);
};
int safe_mmapfile_handle(int fd, int openflag, int prot, int flag, void **ret_ptr, size_t * size)
{
    struct stat st;

    if (fd < 0)
        return 0;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return 0;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return 0;
    }
    if (st.st_size <= 0) {
        close(fd);
        return 0;
    }
    *ret_ptr = mmap(NULL, st.st_size, prot, flag, fd, 0);
    if (*ret_ptr == NULL)
        return 0;
    /*
     * signal(SIGSEGV,sigbus);
     */
    *size = st.st_size;
    return 1;
}
int safe_mmapfile(char *filename, int openflag, int prot, int flag, void **ret_ptr, size_t * size, int *ret_fd)
{
    int fd;
    struct stat st;

    fd = open(filename, openflag, 0600);
    if (fd < 0)
        return 0;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return 0;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return 0;
    }
    if (st.st_size <= 0) {
        close(fd);
        return 0;
    }
    *ret_ptr = mmap(NULL, st.st_size, prot, flag, fd, 0);
    if (!ret_fd)
        close(fd);

    else {
        *ret_fd = fd;
        flock(fd, LOCK_EX);
    }
    if (*ret_ptr == NULL)
        return 0;
    *size = st.st_size;
    return 1;
}
void end_mmapfile(void *ptr, int size, int fd)
{
    munmap(ptr, size);
    /*
     * signal(SIGSEGV,SIG_IGN);
     */
    if (fd != -1)
        close(fd);
}
void encodestr(register char *str)
{
    register char ch, *buf;
    int n;

    buf = str;
    while ((ch = *str++) != '\0') {
        if (*str == ch && str[1] == ch && str[2] == ch) {
            n = 4;
            str += 3;
            while (*str == ch && n < 100) {
                str++;
                n++;
            }
            *buf++ = '\01';
            *buf++ = ch;
            *buf++ = n;
        } else
            *buf++ = ch;
    }
    *buf = '\0';
}

int addtofile(filename, str)
char filename[STRLEN], str[STRLEN];


{
    FILE *fp;
    int rc;

    if ((fp = fopen(filename, "a")) == NULL)
        return -1;
    flock(fileno(fp), LOCK_EX);
    rc = fprintf(fp, "%s\n", str);
    flock(fileno(fp), LOCK_UN);
    fclose(fp);
    return (rc == EOF ? -1 : 1);
}

time_t get_exit_time(char *id, char *exittime)
{                               /* 获取离线时间，id:用户ID,
                                 * exittime:保存返回的时间，结束符为\n
                                 * 建议定义为 char exittime[40]
                                 * Luzi 1998/10/23 */
    /*
     * Leeward 98.10.26 add return value: time_t 
     */
    char path[80];

    FILE *fp;
    time_t now = 1;             /* if fopen failed return 1 -- Leeward */
    struct userec *lookupuser;

    exittime[0] = '\n';
    if ((getuser(id, &lookupuser) == 0) || (lookupuser->exittime == -1)) {
        sethomefile(path, id, "exit");
        fp = fopen(path, "rb");
        if (fp != NULL) {
            fread(&now, sizeof(time_t), 1, fp);
            fclose(fp);
            strcpy(exittime, ctime(&now));
        }
    } else {
        strcpy(exittime, ctime(&lookupuser->exittime));
        now = lookupuser->exittime;
    }
    return now;
}

int read_userdata(const char *userid, struct userdata *ud)
{
    char datafile[STRLEN];
    int fd;

    if ((userid == NULL || userid[0] == '\0') || ud == NULL)
        return -1;
    sethomefile(datafile, userid, USERDATA);
    bzero(ud, sizeof(struct userdata));
    if ((fd = open(datafile, O_RDONLY, 0644)) < 0) {
        if ((fd = open(datafile, O_WRONLY | O_CREAT, 0644)) < 0)
            return -1;
        strncpy(ud->userid, userid, sizeof(ud->userid) - 1);
        ud->userid[sizeof(ud->userid) - 1] = '\0';
        write(fd, ud, sizeof(struct userdata));
        close(fd);
        return 1;               /* created new .userdata file */
    }
    read(fd, ud, sizeof(struct userdata));
    strncpy(ud->userid, userid, sizeof(ud->userid) - 1);
    ud->userid[sizeof(ud->userid) - 1] = '\0';
    close(fd);
    return 0;                   /* success */
}

int write_userdata(const char *userid, const struct userdata *ud)
{
    char datafile[STRLEN];
    int fd;

    if ((userid == NULL || userid[0] == '\0') || ud == NULL)
        return -1;
    sethomefile(datafile, userid, USERDATA);
    if ((fd = open(datafile, O_WRONLY, 0644)) < 0)
        return -1;
    write(fd, ud, sizeof(struct userdata));
    close(fd);
    return 0;
}

void getuinfo(FILE * fn, struct userec *ptr_urec)
{
    struct userdata ud;

    read_userdata(ptr_urec->userid, &ud);
    fprintf(fn, "\n\n您的代号     : %s\n", ptr_urec->userid);
    fprintf(fn, "您的昵称     : %s\n", ptr_urec->username);
    fprintf(fn, "真实姓名     : %s\n", ud.realname);
    fprintf(fn, "居住住址     : %s\n", ud.address);
    fprintf(fn, "电子邮件信箱 : %s\n", ud.email);
    fprintf(fn, "真实 E-mail  : %s\n", ud.realemail);
    fprintf(fn, "注册日期     : %s", ctime(&ptr_urec->firstlogin));
    fprintf(fn, "最近光临日期 : %s", ctime(&ptr_urec->lastlogin));
    fprintf(fn, "最近光临机器 : %s\n", ptr_urec->lasthost);
    fprintf(fn, "上站次数     : %d 次\n", ptr_urec->numlogins);
    fprintf(fn, "文章数目     : %d (Board)\n", ptr_urec->numposts);
}

int del_from_file(char filename[STRLEN], char str[STRLEN])
{
    FILE *fp, *nfp;
    int deleted = false;
    char fnnew[256 /*STRLEN*/];
    char buf[256 /*STRLEN*/];

    if ((fp = fopen(filename, "r")) == NULL)
        return -1;
    flock(fileno(fp), LOCK_EX);
    sprintf(fnnew, "%s.%d", filename, getuid());
    if ((nfp = fopen(fnnew, "w")) == NULL) {
        flock(fileno(fp), LOCK_UN);
        fclose(fp);
        return -1;
    }
    while (fgets(buf, 256 /*STRLEN*/, fp) != NULL) {
        if (strncasecmp(buf, str, strlen(str)) == 0 && buf[strlen(str)] <= 32)
            deleted = true;

        else if (*buf > ' ')
            fputs(buf, nfp);
    }
    flock(fileno(fp), LOCK_UN);
    fclose(fp);
    fclose(nfp);
    if (!deleted)
        return -1;
    return (f_mv(fnnew, filename));
}

int simplepasswd(char *str)
{
    char ch;

    while ((ch = *str++) != '\0') {
        if (!(ch >= 'a' && ch <= 'z'))
            return 0;
    }
    return 1;
}

void logattempt(uid, frm)
char *uid, *frm;


{
    char fname[STRLEN];
    int fd, len;
    char buf[256];

    snprintf(buf, sizeof(buf), "%-12.12s  %-30s %s\n", uid, Ctime(bbstime(0)), frm);
    len = strlen(buf);
    if ((fd = open(BADLOGINFILE, O_WRONLY | O_CREAT | O_APPEND, 0644)) >= 0) {
        write(fd, buf, len);
        close(fd);
    }
    sethomefile(fname, uid, BADLOGINFILE);
    if ((fd = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0644)) >= 0) {
        write(fd, buf, len);
        close(fd);
    }
}

int check_ban_IP(char *IP, char *buf)
{                               /* Leeward 98.07.31
                                 * RETURN:
                                 * - 1: No any banned IP is defined now
                                 * 0: The checked IP is not banned
                                 * other value over 0: The checked IP is banned, the reason is put in buf
                                 */
    FILE *Ban;
    char IPBan[64];
    int IPX = -1;
    char *ptr;

    Ban = fopen(".badIP", "r");
    if (!Ban)
        return IPX;

    else
        IPX++;
    while (fgets(IPBan, 64, Ban)) {
        if ((ptr = strchr(IPBan, '\n')) != NULL)
            *ptr = 0;
        if ((ptr = strchr(IPBan, ' ')) != NULL) {
            *ptr++ = 0;
            strcpy(buf, ptr);
        }
        IPX = strlen(IPBan);
        if (*IPBan == '+')
            if (!strncmp(IP, IPBan + 1, IPX - 1))
                break;
        IPX = 0;
    }
    fclose(Ban);
    return IPX;
}

int dodaemon(char *daemonname, bool single, bool closefd)
{
    int pidfd;
    char path[MAXPATH];
    char line[20];

    if (fork())
        exit(0);
    setsid();
    if (fork())
        exit(0);
    sprintf(path, "var/%s.pid", daemonname);
    pidfd = open(path, O_RDWR | O_CREAT, 0660);
    if (write_lock(pidfd, 0, SEEK_SET, 0) < 0) {
        if (errno == EACCES || errno == EAGAIN)
            return 1;
        else
            return 2;
    }

    if (closefd) {
        int i;

        for (i = 0; i < 64; i++)
            if (i != pidfd)
                close(i);
    }
    snprintf(line, sizeof(line), "%ld\n", (long) getpid());
    ftruncate(pidfd, 0);
    write(pidfd, line, strlen(line));
    return 0;
}

int is_valid_date(int year, int month, int day)
{
    int feb;

    /*
     * 首先检查月份 
     */
    switch (month) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        /*
         * 大月 
         */
        if (day >= 1 && day <= 31)
            return 1;
        else
            return 0;
    case 4:
    case 6:
    case 9:
    case 11:
        /*
         * 小月 
         */
        if (day >= 1 && day <= 30)
            return 1;
        else
            return 0;
    case 2:
        /*
         * 是否闰年 
         */
        feb = 28 + ((year % 100 == 0) ? (year % 400 == 0) : (year % 4 == 0));
        if (day >= 1 && day <= feb)
            return 1;
        else
            return 0;
    default:
        return 0;
    }
}

/*
 * Return value:
 *      0       Success
 *     -1       so called "bad arg1"
 *     -2       so called "bad arg2"
 */
int valid_filename(char *file, int use_subdir)
{
    if (file == NULL)
        return -1;
    if (use_subdir) {
        char *ptr;

        if (strstr(file, ".."))
            return -2;
        ptr = file;
        if (isalpha(file[0]) && file[1] == '/')
            ptr = file + 2;
        if (strncmp(ptr, "M.", 2) && strncmp(ptr, "G.", 2))
            return -1;
        if (strchr(ptr, '/'))
            return -2;
    } else {
        if (strncmp(file, "M.", 2) && strncmp(file, "G.", 2))
            return -1;
        if (strstr(file, "..") || strstr(file, "/"))
            return -2;
    }
    return 0;
}

void set_proc_title(char *argv0, char *title)
{
#ifdef FREEBSD
    setproctitle("-%s", title);
#else
    strcpy(argv0, title);
#endif
}

int cmpuids2(unum, urec)
int unum;
struct user_info *urec;
{
    return (unum == urec->uid);
}

unsigned int load_mailbox_prop(char *userid)
{
    char filename[256];
    int prop = 0;
    int fd;

    sethomefile(filename, userid, ".mailbox.prop");
    if ((fd = open(filename, O_RDONLY, 0644)) > 0) {
        read(fd, &prop, sizeof(prop));
        close(fd);
    }
    return update_mailbox_prop(userid, prop);
}

unsigned int store_mailbox_prop(char *userid)
{
    char filename[256];
    int prop;
    int fd;

    prop = get_mailbox_prop(userid);
    sethomefile(filename, userid, ".mailbox.prop");
    if ((fd = open(filename, O_WRONLY | O_CREAT, 0644)) > 0) {
        write(fd, &prop, sizeof(prop));
        close(fd);
    }
    return prop;
}

unsigned int get_mailbox_prop(char *userid)
{
    struct user_info *uip;

    uip = t_search(userid, 0);
    if (uip == NULL)
        return 0;
    else
        return uip->mailbox_prop;
}

unsigned int update_mailbox_prop(char *userid, unsigned int prop)
{
    struct user_info *uip;

    uip = t_search(userid, 0);
    if (uip != NULL)
        return uip->mailbox_prop = prop;
    else
        return 0;
}

unsigned int load_mailgroup_list(const char *userid, mailgroup_list_t * mgl)
{
    char fname[STRLEN];
    int fd;

    bzero(mgl, sizeof(mailgroup_list_t));
    sethomefile(fname, userid, "mailgroup");
    if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
        read(fd, mgl, sizeof(mailgroup_list_t));
        close(fd);
    }
    return mgl->groups_num;     /* return zero for failure or no group lists */
}

int store_mailgroup_list(const char *userid, const mailgroup_list_t * mgl)
{
    char fname[STRLEN];
    int fd;

    sethomefile(fname, userid, "mailgroup");
    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
        write(fd, mgl, sizeof(mailgroup_list_t));
        close(fd);
        return 0;
    } else
        return -1;
}

/*
 * FIXME: If unpredictable system power lost often occurs, there may
 *        be a possibility that this function always return failure.
 */
static int get_mailgroup_name(const char *userid, mailgroup_list_item * item)
{
    int i;
    char groupname[sizeof(item->group_name)];
    char buf[STRLEN];
    char filename[STRLEN];
    struct stat st;
    int fd;

    sethomefile(buf, userid, "mgroups");
    if (stat(buf, &st) == -1) {
        if (mkdir(buf, 0755) == -1)
            return -1;
    } else {
        if (!(st.st_mode & S_IFDIR))
            return -1;
    }
    for (i = 0; i < MAX_MAILGROUP_NUM; i++) {
        snprintf(groupname, sizeof(groupname), "group%02d", i);
        snprintf(filename, sizeof(filename), "%s/%s", buf, groupname);
        if ((fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0600)) != -1)
            break;
    }
    if (fd == -1)
        return -1;
    close(fd);
    strcpy(item->group_name, groupname);
    return 0;
}

int add_mailgroup_item(const char *userid, mailgroup_list_t * mgl, mailgroup_list_item * item)
{
    int i;

    if (get_mailgroup_name(userid, item) < 0)
        return -1;
    for (i = 0; i < MAX_MAILGROUP_NUM; i++) {
        if (mgl->groups[i].group_name[0] == '\0') {
            memcpy(&(mgl->groups[i]), item, sizeof(mailgroup_list_item));
            mgl->groups_num++;
            return 0;
        }
    }
    return -1;
}

int add_default_mailgroup_item(const char *userid, mailgroup_list_t * mgl)
{
    mailgroup_list_item item;

    bzero(&item, sizeof(item));
    snprintf(item.group_desc, sizeof(item.group_desc), "预设群体信件组");
    return add_mailgroup_item(currentuser->userid, mgl, &item);
}

int delete_mailgroup_item(const char *userid, mailgroup_list_t * mgl, int entry)
{
    int i;
    char buf[STRLEN];
    char filename[STRLEN];

    sethomefile(buf, userid, "mgroups");
    snprintf(filename, sizeof(filename), "%s/%s", buf, mgl->groups[entry].group_name);
    for (i = entry; i < mgl->groups_num - 1; i++) {
        memcpy(&(mgl->groups[i]), &(mgl->groups[i + 1]), sizeof(mailgroup_list_item));
    }
    bzero(&(mgl->groups[i]), sizeof(mailgroup_list_item));
    mgl->groups_num--;
    unlink(filename);
    return 0;
}

int modify_mailgroup_item(const char *userid, mailgroup_list_t * mgl, int entry, mailgroup_list_item * item)
{
    memcpy(&(mgl->groups[entry]), item, sizeof(mailgroup_list_item));

    return 0;
}

int load_mailgroup(const char *userid, const char *group, mailgroup_t * mg, int num)
{
    char fname[STRLEN];
    char buf[STRLEN];
    int fd;
    int ret = 0;

    sethomefile(buf, userid, "mgroups");
    snprintf(fname, sizeof(fname), "%s/%s", buf, group);
    if ((fd = open(fname, O_RDONLY, 0600)) < 0)
        return -1;
    if (read(fd, mg, sizeof(mailgroup_t) * num) == sizeof(mailgroup_t) * num) {
        int i;
        int len;

        for (i = 0; i < num; i++) {
            len = strlen(mg[i].id);
            if (mg[i].id[len - 1] == '\n')
                mg[i].id[len - 1] = '\0';
        }
        ret = num;
    }
    close(fd);

    return ret;                 /* return zero on failure or no users,
                                 * else return the number of users had been loaded. */
}

int store_mailgroup(const char *userid, const char *group, const mailgroup_t * mg, int num)
{
    char fname[STRLEN];
    char buf[STRLEN];
    int fd;

    sethomefile(buf, userid, "mgroups");
    snprintf(fname, sizeof(fname), "%s/%s", buf, group);
    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
        write(fd, mg, sizeof(mailgroup_t) * num);
        close(fd);
        return 0;
    } else
        return -1;
}

int import_old_mailgroup(const char *userid, mailgroup_list_t * mgl)
{
    char oldgroup[STRLEN];
    char tmpgroup[STRLEN];
    char buf[STRLEN];
    int num = 0;
    int len;
    mailgroup_list_item item;
    mailgroup_t mg;
    FILE *fp;
    int fd;

    sethomefile(oldgroup, userid, "maillist");
    sprintf(buf, "tmpgroup%d", getpid());
    sethomefile(tmpgroup, userid, buf);
    if ((fp = fopen(oldgroup, "r")) == NULL)
        return -1;
    if ((fd = open(tmpgroup, O_CREAT | O_WRONLY, 0600)) < 0) {
        fclose(fp);
        return -1;
    }
    while (num < MAX_MAILGROUP_USERS && fgets(buf, sizeof(buf), fp) != NULL) {
        len = strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        strncpy(mg.id, buf, sizeof(mg.id) - 1);
        mg.id[sizeof(mg.id) - 1] = '\0';
        mg.exp[0] = '\0';
        write(fd, &mg, sizeof(mg));
        num++;
    }
    fclose(fp);
    close(fd);
    bzero(&item, sizeof(item));
    item.users_num = num;
    snprintf(item.group_desc, sizeof(item.group_desc), "老版本群体信件组");
    if (add_mailgroup_item(userid, mgl, &item) < 0) {
        unlink(tmpgroup);
        return -1;
    }
    sethomefile(buf, userid, "mgroups/");
    strcat(buf, item.group_name);
    rename(tmpgroup, buf);

    return 0;
}

int import_friends_mailgroup(const char *userid, mailgroup_list_t * mgl)
{
    char oldgroup[STRLEN];
    char tmpgroup[STRLEN];
    char buf[STRLEN];
    int num = 0;
    mailgroup_list_item item;
    mailgroup_t mg;
    struct friends fr;
    int fd2;
    int fd;

    sethomefile(oldgroup, userid, "friends");
    sprintf(buf, "tmpgroup%d", getpid());
    sethomefile(tmpgroup, userid, buf);
    if ((fd2 = open(oldgroup, O_RDONLY, 0600)) < 0)
        return -1;
    if ((fd = open(tmpgroup, O_CREAT | O_WRONLY, 0600)) < 0) {
        close(fd2);
        return -1;
    }
    while (num < MAX_MAILGROUP_USERS && read(fd2, &fr, sizeof(fr)) == sizeof(fr)) {
        strncpy(mg.id, fr.id, sizeof(mg.id) - 1);
        mg.id[sizeof(mg.id) - 1] = '\0';
        strncpy(mg.exp, fr.exp, sizeof(mg.exp) - 1);
        mg.exp[sizeof(mg.exp) - 1] = '\0';
        write(fd, &mg, sizeof(mg));
        num++;
    }
    close(fd2);
    close(fd);
    bzero(&item, sizeof(item));
    item.users_num = num;
    snprintf(item.group_desc, sizeof(item.group_desc), "好友群体信件组");
    if (add_mailgroup_item(userid, mgl, &item) < 0) {
        unlink(tmpgroup);
        return -1;
    }
    sethomefile(buf, userid, "mgroups/");
    strcat(buf, item.group_name);
    rename(tmpgroup, buf);

    return 0;
}

int add_mailgroup_user(mailgroup_list_t * mgl, int entry, mailgroup_t * users, mailgroup_t * user)
{
    int i;

    for (i = 0; i < MAX_MAILGROUP_USERS; i++) {
        if (users[i].id[0] == '\0') {
            memcpy(&users[i], user, sizeof(mailgroup_t));
            mgl->groups[entry].users_num++;
            return 0;
        }
    }
    return -1;
}

int delete_mailgroup_user(mailgroup_list_t * mgl, int entry, mailgroup_t * users, int pos)
{
    int i;

    for (i = pos; i < mgl->groups[entry].users_num - 1; i++) {
        memcpy(&users[i], &users[i + 1], sizeof(mailgroup_t));
    }
    bzero(&users[i], sizeof(mailgroup_t));
    mgl->groups[entry].users_num--;
    return 0;
}

int modify_mailgroup_user(mailgroup_t * users, int pos, mailgroup_t * user)
{
    memcpy(&users[pos], user, sizeof(mailgroup_t));

    return 0;
}

int gettmpfilename(char *retchar, char *fmt, ...){
		/* stiger : 这里没有对fname做检查 */
	char fname[STRLEN];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(fname, STRLEN-20, fmt, ap);
	va_end(ap);

	sprintf(retchar, "tmp/%d/", getpid());
    if (!dashd(retchar)) {
        mkdir(retchar, 0755);
        chmod(retchar, 0755);
	}
	strcat(retchar, currentuser->userid);
    if (!dashd(retchar)) {
        mkdir(retchar, 0755);
        chmod(retchar, 0755);
	}
	strcat(retchar,"/");
	strcat(retchar, fname);
	return 1;
}
