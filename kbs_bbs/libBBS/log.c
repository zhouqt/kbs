/*
    bbslog.c
    Copyright (C) 2001, wwj@j32.org
                  2002, kcn@j32.org
*/


#include "bbs.h"
#include <stdarg.h>
#include <sys/ipc.h>
#include <sys/msg.h>


/*

  优先级
  
  0   message
  1   notice
  2   warning
  3   error
  4   cirtical error
*/

typedef struct _logconfig {
    int compare;                /* -1 小于等于, 0 相等,  1 大于等于 */
    int prio;                   /* 优先级 */
    char *name;                 /* 确定的来源，NULL是全匹配 */
    char *file;                 /* 文件名，相对于 BBSHOME/reclog ，如果是NULL，丢弃 */
    int bufsize;                /* 缓存大小，如果是 0，不缓存 */
    int searchnext;             /* 如果这个匹配了，还查找后面的config吗？ */

    /*
     * 运行时参数 
     */
    int bufptr;                 /* 使用缓存位置 */
    char *buf;                  /* 缓存 */
    int fd;                     /* 文件句柄 */
} logconfig;

static logconfig logconf[] = {
    {1, 3, NULL, "error.log", 0, 1, 0, NULL, 0},        /* 错误记录 */
    {1, 0, "connect", "connect.log", 0, 0, 0, NULL, 0}, /* 连接记录 */
    {1, 0, "user", "user.log", 0, 0, 0, NULL, 0},       /*所有用户的使用记录，使用缓冲 */
    {1, 0, "usermsg", "msg.log", 0, 0, 0, NULL, 0},     /* 所有用户的message记录 */
    {1, 0, "boardusage", "boardusage.log", 0, 0, 0, NULL, 0},   /* 版面使用记录 */
    {1, 0, "chatd", "trace.chatd", 1024, 0, 0, NULL, 0},        /* 聊天室使用记录，使用缓冲 */
    {1, 0, NULL, "trace", 0, 0, 0, NULL, 0}     /* 最后所有的记录都在这里 */
};

int bdoatexit = 0;


static void getheader(char *header, const char *from, int prio,session_t* session)
{
    struct tm *pt;
    time_t tt;

    time(&tt);
    pt = localtime(&tt);

    sprintf(header, "[%02u/%02u %02u:%02u:%02u %5d %d.%s] %s ", pt->tm_mon + 1, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec, (int)getpid(), prio, from, (session==NULL||session->currentuser == NULL) ? "(unknown user)" : session->currentuser->userid);
}

/* 写入log, 如果buf==NULL那么flush。否则根据大小决定是否缓存 */
static void writelog(logconfig * pconf, const char *from, int prio, const char *buf, session_t* session)
{
    char header[64];

    if (!from)
        from = pconf->name;
    if (!from)
        from = "unknown";

    getheader(header, from, prio, session);

    if (buf && pconf->buf) {
        if ((int) (pconf->bufptr + strlen(header) + strlen(buf) + 2) <= pconf->bufsize) {
            strcpy(&pconf->buf[pconf->bufptr], header);
            pconf->bufptr += strlen(header);
            strcpy(&pconf->buf[pconf->bufptr], buf);
            pconf->bufptr += strlen(buf);
            strcpy(&pconf->buf[pconf->bufptr], "\r\n");
            pconf->bufptr += 2;
            return;
        }
    }

    flock(pconf->fd, LOCK_SH);
    lseek(pconf->fd, 0, SEEK_END);

    if (pconf->buf && pconf->bufptr) {
        write(pconf->fd, pconf->buf, pconf->bufptr);
        pconf->bufptr = 0;
    }
    if (buf) {
        write(pconf->fd, header, strlen(header));
        write(pconf->fd, buf, strlen(buf));
        write(pconf->fd, "\r\n", 2);
    }
    flock(pconf->fd, LOCK_UN);
}

static void logatexit()
{
    logconfig *pconf;

    pconf = logconf;

    while (pconf - logconf < (int) (sizeof(logconf) / sizeof(logconfig))) {
        if (pconf->buf && pconf->bufptr)
            writelog(pconf, NULL, 0, NULL,getSession());
        if (pconf->buf)
            free(pconf->buf);
        if (pconf->fd > 0)
            close(pconf->fd);
        pconf++;
    }
}

int bbslog(const char *from, const char *fmt, ...)
{
    static int in_bbslog = 0;
    logconfig *pconf;
    int prio;
    char buf[512];
    va_list v;

    prio = 0;

    if (from[0] >= '0' && from[0] <= '9') {
        prio = from[0] - '0';
        from++;
    }

    /* disable unimportant log. atppp 20060426 
     * 以前是 BBSMAIN 下不记录 prio == 0 的 log
     * 现在是所有情况都不记录了，反正也没什么有用的东东
     */
    if (prio == 0)
        return 0;

    if (strlen(from) > 16)
        return -3;
    if (!fmt || !*fmt)
        return 0;

    va_start(v, fmt);
    vsprintf(buf, fmt, v);
    if (in_bbslog) {
        fprintf(stderr, "%s", buf);
        exit(1);
    }
    in_bbslog = 1;


    pconf = logconf;

    while (pconf - logconf < (int) (sizeof(logconf) / sizeof(logconfig))) {
        if ((pconf->compare == 1 && prio >= pconf->prio) || (pconf->compare == 0 && prio == pconf->prio) || (pconf->compare == -1 && prio <= pconf->prio)) {
            if (!pconf->name || !strcasecmp(pconf->name, from)) {
                if (pconf->fd <= 0) {   /* init it! */
                    if (!pconf->file)
                        return 0;       /* discard it */
                    pconf->fd = open(pconf->file, O_WRONLY);
                    if (pconf->fd < 0)
                        pconf->fd = creat(pconf->file, 0644);
                    if (pconf->fd < 0)
                        return -1;

                    if (pconf->bufsize) {
                        if (!bdoatexit) {
                            atexit(logatexit);
                            bdoatexit = 1;
                        }
                        pconf->buf = (char *) malloc(pconf->bufsize);
                        pconf->bufptr = 0;
                    }
                }
                writelog(pconf, from, prio, buf, getSession());
                if (!pconf->searchnext)
                    break;
            }
        }
        pconf++;
    }
    if (pconf->fd >= 0 && !pconf->bufsize) {
        close(pconf->fd);
        pconf->fd = -1;
    }
    
    in_bbslog = 0;
    return 0;
}

#define DATALEN 100

int bmlog(const char *id, const char *boardname, int type, int value)
{
/*
type - meaning
  0          停留时间
  1          进版
  2          版内发文
  3          收入文摘
  4          去掉文摘
  5          区段
  6          标记文章
  7          去掉标记
  8          删除文章
  9          恢复删除
  10        封禁
  11        解封
  12        收入精华
  13        整理精华
  14        相同主题
*/
#ifndef NEWBMLOG
    int fd, data[DATALEN];
    struct flock ldata;
    struct stat buf;
    char direct[PATHLEN];
#endif /* NEWBMLOG */
    const struct boardheader *btemp;
    char BM[PATHLEN];

    btemp = getbcache(boardname);
    if (btemp == NULL)
        return 0;
    strncpy(BM, btemp->BM, sizeof(BM) - 1);
    BM[sizeof(BM) - 1] = '\0';
    if (!chk_BM_instr(BM, id))
        return 0;

#ifdef NEWBMLOG
	return newbmlog(id, boardname, type, value);
#else
    sprintf(direct, "boards/%s/.bm.%s", boardname, id);
    if ((fd = open(direct, O_RDWR | O_CREAT, 0644)) == -1)
        return 0;
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        close(fd);
        return 0;
    }
    fstat(fd, &buf);
    if (buf.st_size < DATALEN * sizeof(int)) {
        memset(data, 0, sizeof(int) * DATALEN);
    } else
        read(fd, data, sizeof(int) * DATALEN);
    if (type >= 0 && type < DATALEN)
        data[type] += value;
    lseek(fd, 0, SEEK_SET);
    write(fd, data, sizeof(int) * DATALEN);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);
    close(fd);
    return 0;
#endif
}

int init_bbslog()
{
    int msqid;
    struct msqid_ds buf;

    msqid = msgget(sysconf_eval("BBSBBSLOG_MSG", 0x888), IPC_CREAT | 0664);
    if (msqid < 0)
        return -1;
    msgctl(msqid, IPC_STAT, &buf);
    buf.msg_qbytes = 50 * 1024;
    msgctl(msqid, IPC_SET, &buf);
    return msqid;
}


void newbbslog(int type, const char *fmt, ...)
{
    char buf[512];
    struct bbs_msgbuf *msg = (struct bbs_msgbuf *) buf;

    va_list v;

    if (!fmt || !*fmt)
        return;
    if (disablelog)
        return;
    if (logmsqid == -1 ) {
        logmsqid = init_bbslog();
        if (logmsqid ==-1 ) {
            disablelog = 1;
            return;
        }
    }
    va_start(v, fmt);


    msg->mtype = type;
    msg->pid = getpid();
    msg->msgtime = time(0);
    if (getSession()&&getSession()->currentuser)
        strncpy(msg->userid, getSession()->currentuser->userid, IDLEN);
    else
        strncpy(msg->userid, "[null]", IDLEN);

    vsnprintf(msg->mtext, sizeof(buf) - ((char *) msg->mtext - (char *) msg), fmt, v);
    msgsnd(logmsqid, msg, strlen(msg->mtext) + ((char *) msg->mtext - (char *) msg) - sizeof(msg->mtype) + 1, IPC_NOWAIT | MSG_NOERROR);
}

#ifdef NEWPOSTLOG
void newpostlog(const char *userid, const char *boardname, const char *title, int groupid)
{
    char buf[512];
    struct bbs_msgbuf *msg = (struct bbs_msgbuf *) buf;
	struct _new_postlog *ppostlog = (struct _new_postlog *) (buf + ((char *)msg->mtext - (char *)msg) + 1);

	if (userid[0]=='\0' || boardname=='\0')
		return;
    if (disablelog)
        return;
    if (logmsqid == -1 ) {
        logmsqid = init_bbslog();
        if (logmsqid ==-1 ) {
            disablelog = 1;
            return;
        }
    }

    msg->mtype = BBSLOG_POST;
    msg->pid = getpid();
    msg->msgtime = time(0);
    strncpy(msg->userid, userid, IDLEN);

	strncpy(ppostlog->boardname, boardname, BOARDNAMELEN);
	ppostlog->boardname[BOARDNAMELEN-1]='\0';
	ppostlog->threadid = groupid;
	strncpy(ppostlog->title, title, 80);
	ppostlog->title[80]='\0';

    msgsnd(logmsqid, msg, sizeof(struct _new_postlog) + ((char *) msg->mtext - (char *) msg) - sizeof(msg->mtype) + 1, IPC_NOWAIT | MSG_NOERROR);
}
#endif

#ifdef NEWBMLOG
int newbmlog(const char *userid, const char *boardname, int type, int value)
{
/*
type - meaning
  0          停留时间
  1          进版
  2          版内发文
  3          收入文摘
  4          去掉文摘
  5          区段
  6          标记文章
  7          去掉标记
  8          删除文章
  9          恢复删除
  10        封禁
  11        解封
  12        收入精华
  13        整理精华
  14        相同主题
*/
    char buf[512];
    struct bbs_msgbuf *msg = (struct bbs_msgbuf *) buf;
	struct _new_bmlog *ppostlog = (struct _new_bmlog *) (buf + ((char *)msg->mtext - (char *)msg) + 1);

	if (userid[0]=='\0' || boardname=='\0')
		return 0;
    if (disablelog)
        return 0;
    if (logmsqid == -1 ) {
        logmsqid = init_bbslog();
        if (logmsqid ==-1 ) {
            disablelog = 1;
            return 0;
        }
    }

    msg->mtype = BBSLOG_BM;
    msg->pid = getpid();
    msg->msgtime = time(0);
    strncpy(msg->userid, userid, IDLEN);

	strncpy(ppostlog->boardname, boardname, BOARDNAMELEN);
	ppostlog->boardname[BOARDNAMELEN-1]='\0';
	ppostlog->type = type;
	ppostlog->value = value;

    msgsnd(logmsqid, msg, sizeof(struct _new_bmlog) + ((char *) msg->mtext - (char *) msg) - sizeof(msg->mtype) + 1, IPC_NOWAIT | MSG_NOERROR);

	return 0;
}
#endif
