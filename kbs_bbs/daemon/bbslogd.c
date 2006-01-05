#include "bbs.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
struct bbs_msgbuf *rcvlog(int msqid)
{
    static char buf[1024];
    struct bbs_msgbuf *msgp = (struct bbs_msgbuf *) buf;
    int retv;

    retv = msgrcv(msqid, msgp, sizeof(buf) - sizeof(msgp->mtype) - 2, 0, MSG_NOERROR);
    if (retv < 0) {
	if (errno==EINTR)
            return NULL;
	else {
	    bbslog("3error","bbslogd(rcvlog):%s",strerror(errno));
	    exit(0);
	}
    }
    retv-=((char*)msgp->mtext-(char*)&msgp->msgtime);
#ifdef NEWPOSTLOG
	if (msgp->mtype == BBSLOG_POST){
		if(retv <= sizeof(struct _new_postlog)){
			return NULL;
		}
		return msgp;
	}
#endif
#ifdef NEWBMLOG
	if (msgp->mtype == BBSLOG_BM){
		if(retv <= sizeof(struct _new_bmlog)){
			return NULL;
		}
		return msgp;
	}
#endif
    while (retv > 0 && msgp->mtext[retv - 1] == 0)
        retv--;
    if (retv==0) return NULL;
    if (msgp->mtext[retv - 1] != '\n') {
        msgp->mtext[retv] = '\n';
        retv++;
    }
    msgp->mtext[retv]=0;
    return msgp;
}

struct taglogconfig {
    char *filename;
    int bufsize;                /* 缓存大小，如果是 0，不缓存 */

    /*
     * 运行时参数 
     */
    int bufptr;                 /* 使用缓存位置 */
    char *buf;                  /* 缓存 */
    int fd;                     /* 文件句柄 */
};
static struct taglogconfig logconfig[] = {
    {"usies", 100 * 1024, 0, NULL, -1},
    {"user.log", 100 * 1024, 0, NULL, 0},
    {"boardusage.log", 100 * 1024, 0, NULL, 0},
    {"sms.log", 10 * 1024, 0, NULL, 0},
    {"debug.log", 10 * 1024, 0, NULL, 0}
};

#if defined(NEWPOSTLOG) || defined(NEWBMLOG)
static MYSQL s;
static int postlog_start=0;
static time_t mysqlclosetime=0;
static int mysql_fail=0;

static void opennewpostlog()
{
	mysql_init (&s);

	if (! my_connect_mysql(&s) ){
		bbslog("3system","mysql connect error:%s",mysql_error(&s));
		return;
	}
	postlog_start = 1;
	mysqlclosetime=0;
	return;
}

static void closenewpostlog()
{
	bbslog("3system","mysql log closed");
	mysql_close(&s);
	postlog_start=0;
	mysqlclosetime = time(0);
}
#endif

static void openbbslog(int first)
{
    int i;
    for (i = 0; i < sizeof(logconfig) / sizeof(struct taglogconfig); i++) {
		if (!first && !strcmp(logconfig[i].filename,"boardusage.log") && logconfig[i].fd )
			continue;
        if (logconfig[i].filename) {
            logconfig[i].fd = open(logconfig[i].filename, O_WRONLY);
            if (logconfig[i].fd < 0)
                logconfig[i].fd = creat(logconfig[i].filename, 0644);
            if (logconfig[i].fd < 0)
                bbslog("3error","can't open log file:%s.%s",logconfig[i].filename,strerror(errno));
        }
        if (logconfig[i].buf==NULL)
            logconfig[i].buf = malloc(logconfig[i].bufsize);
    }

#if defined(NEWPOSTLOG) || defined(NEWBMLOG)
	if(first || !postlog_start)
		opennewpostlog();
#endif

}
static void writelog(struct bbs_msgbuf *msg)
{
    char header[256];
    struct tm *n;
    struct taglogconfig *pconf;
    char ch;

#if defined(NEWPOSTLOG) || defined(NEWBMLOG)
	if(!postlog_start && mysqlclosetime && time(0)-mysqlclosetime>600)
		opennewpostlog();
#endif

#ifdef NEWBMLOG
	if (msg->mtype == BBSLOG_BM){
		char sqlbuf[512];
		struct _new_bmlog * ppl = (struct _new_bmlog *)( &msg->mtext[1]) ;
		int affect;

		if(!postlog_start)
			return;

		if(ppl->value == 0)
			return;

		msg->mtext[0]=0;

		sprintf(sqlbuf, "UPDATE bmlog SET `log%d`=`log%d`+%d WHERE userid='%s' AND bname='%s' AND month=MONTH(CURDATE()) AND year=YEAR(CURDATE()) ;", ppl->type, ppl->type, ppl->value, msg->userid, ppl->boardname );

		if( mysql_real_query(&s,sqlbuf,strlen(sqlbuf)) || (affect=(int)mysql_affected_rows(&s))<0 ){
			mysql_fail ++;
			bbslog("3system","mysql bmlog error:%s",mysql_error(&s));
			if(mysql_fail > 10)
				closenewpostlog();
			return;
		}

		if(affect <= 0){
			sprintf(sqlbuf, "INSERT INTO bmlog (`id`, `userid`, `bname`, `month`, `year`, `log%d` ) VALUES (NULL, '%s', '%s', MONTH(CURDATE()), YEAR(CURDATE()), '%d' );", ppl->type, msg->userid, ppl->boardname, ppl->value);

			if( mysql_real_query( &s, sqlbuf, strlen(sqlbuf) )){
				mysql_fail ++;
				bbslog("3system","mysql bmlog error:%s",mysql_error(&s));
				if(mysql_fail > 10)
					closenewpostlog();
			}else
				mysql_fail = 0;
		}else{
			mysql_fail = 0;
		}

		return;
	}
#endif

#ifdef NEWPOSTLOG
	if (msg->mtype == BBSLOG_POST && postlog_start){

		char newtitle[161];
		char sqlbuf[512];
		struct _new_postlog * ppl = (struct _new_postlog *) ( &msg->mtext[1]) ;
		char newts[20];

		msg->mtext[0]=0;

		mysql_escape_string(newtitle, ppl->title, strlen(ppl->title));

		sprintf(sqlbuf, "INSERT INTO postlog VALUES (NULL, '%s', '%s', '%s', '%s', '%d');", msg->userid, ppl->boardname, newtitle, tt2timestamp(msg->msgtime, newts), ppl->threadid );

		if( mysql_real_query( &s, sqlbuf, strlen(sqlbuf) )){
			mysql_fail ++;
			bbslog("3system","mysql postlog error:%s",mysql_error(&s));
			if(mysql_fail > 10)
				closenewpostlog();
		}else{
			mysql_fail = 0;

			return;
		}
	}

	if (msg->mtype == BBSLOG_POST){
		struct _new_postlog * ppl = (struct _new_postlog *) ( &msg->mtext[1]) ;

		msg->mtype = BBSLOG_USER;

    	if ((msg->mtype < 0) || (msg->mtype > sizeof(logconfig) / sizeof(struct taglogconfig)))
        	return;
    	pconf = &logconfig[msg->mtype-1];

    	if (pconf->fd<0) return;
    	n = localtime(&msg->msgtime);

    	snprintf(header, 256, "[%02u/%02u %02u:%02u:%02u %5lu %lu] %s post '%s' on '%s'\n", n->tm_mon + 1, n->tm_mday, n->tm_hour, n->tm_min, n->tm_sec, (long int) msg->pid, msg->mtype, msg->userid, ppl->title, ppl->boardname);
    	if (pconf->buf) {
        	if ((int) (pconf->bufptr + strlen(header)) <= pconf->bufsize) {
            	strcpy(&pconf->buf[pconf->bufptr], header);
            	pconf->bufptr += strlen(header);
            	return;
        	}
    	}

/*目前log还是分散的，就先lock,seek吧*/
    	flock(pconf->fd, LOCK_SH);
    	lseek(pconf->fd, 0, SEEK_END);

    	if (pconf->buf && pconf->bufptr) {
        	write(pconf->fd, pconf->buf, pconf->bufptr);
        	pconf->bufptr = 0;
    	}
    	flock(pconf->fd, LOCK_UN);

		return;
	}

#endif

    if ((msg->mtype < 0) || (msg->mtype > sizeof(logconfig) / sizeof(struct taglogconfig)))
        return;
    pconf = &logconfig[msg->mtype-1];

    if (pconf->fd<0) return;
    n = localtime(&msg->msgtime);

    ch=msg->mtext[0];
    msg->mtext[0]=0;
    snprintf(header, 256, "[%02u/%02u %02u:%02u:%02u %5lu %lu] %s %c%s", n->tm_mon + 1, n->tm_mday, n->tm_hour, n->tm_min, n->tm_sec, (long int) msg->pid, msg->mtype, msg->userid,ch,&msg->mtext[1]);
    if (pconf->buf) {
        if ((int) (pconf->bufptr + strlen(header)) <= pconf->bufsize) {
            strcpy(&pconf->buf[pconf->bufptr], header);
            pconf->bufptr += strlen(header);
            return;
        }
    }

/*目前log还是分散的，就先lock,seek吧*/
    flock(pconf->fd, LOCK_SH);
    lseek(pconf->fd, 0, SEEK_END);

    if (pconf->buf && pconf->bufptr) {
        write(pconf->fd, pconf->buf, pconf->bufptr);
        pconf->bufptr = 0;
    }
    flock(pconf->fd, LOCK_UN);
}

static void flushlog(int signo)
{
    int i;
    for (i = 0; i < sizeof(logconfig) / sizeof(struct taglogconfig); i++) {
        struct taglogconfig *pconf;

        pconf = &logconfig[i];
        if (pconf->fd>=0 && pconf->buf && pconf->bufptr) {
            flock(pconf->fd, LOCK_SH);
            lseek(pconf->fd, 0, SEEK_END);
            write(pconf->fd, pconf->buf, pconf->bufptr);
            pconf->bufptr = 0;
            flock(pconf->fd, LOCK_UN);
        }
        if (signo!=-1)
            close(pconf->fd);
    }
    if (signo==-1) return;
#if defined(NEWPOSTLOG) || defined(NEWBMLOG)
	closenewpostlog();
#endif
    exit(0);
}

static void flushBBSlog_exit()
{
    flushlog(-1);
}
bool gb_trunclog;
bool truncboard;
static void trunclog(int signo)
{
    int i;
    flushlog(-1);
    
    for (i = 0; i < sizeof(logconfig) / sizeof(struct taglogconfig); i++) {
        struct taglogconfig *pconf;

		if (! strcmp(logconfig[i].filename,"boardusage.log"))
			continue;
        pconf = &logconfig[i];
        if (pconf->fd>=0) {
        	char buf[MAXPATH];
        	int j;
        	close(pconf->fd);
		j=0;
        	while (1) {
        	    sprintf(buf,"%s.%d",pconf->filename,j);
        	    if (!dashf(buf))
        	    	break;
		    j++;
        	}
        	f_mv(pconf->filename,buf);
        }
    }
    openbbslog(0);
    gb_trunclog=true;
}

static void truncboardlog(int signo)
{
    int i;
    flushlog(-1);
    
    for (i = 0; i < sizeof(logconfig) / sizeof(struct taglogconfig); i++) {
		if (strcmp(logconfig[i].filename,"boardusage.log"))
			continue;
        if (logconfig[i].fd>=0) {
        	close(logconfig[i].fd);
        	f_mv(logconfig[i].filename,"boardusage.log.0");
        }
        if (logconfig[i].filename) {
            logconfig[i].fd = open(logconfig[i].filename, O_WRONLY);
            if (logconfig[i].fd < 0)
                logconfig[i].fd = creat(logconfig[i].filename, 0644);
            if (logconfig[i].fd < 0)
                bbslog("3error","can't open log file:%s.%s",logconfig[i].filename,strerror(errno));
        }
        if (logconfig[i].buf==NULL)
            logconfig[i].buf = malloc(logconfig[i].bufsize);
    }
    truncboard=true;
}

static void flushBBSlog_time(int signo)
{
    flushlog(-1);
    alarm(60*10); /*十分钟flush一次*/
}

static void do_truncboardlog()
{
    truncboard=false;
}

static void do_trunclog()
{
    gb_trunclog=false;
}

int main()
{
    int msqid, i;
    struct bbs_msgbuf *msg;

    struct sigaction act;

    umask(027);

    chdir(BBSHOME);
    setuid(BBSUID);
    setreuid(BBSUID, BBSUID);
    setgid(BBSGID);
    setregid(BBSGID, BBSGID);
    dodaemon("bbslogd", true, true);

    atexit(flushBBSlog_exit);
    bzero(&act, sizeof(act));
    act.sa_handler = flushlog;
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGABRT, &act, NULL);
    sigaction(SIGHUP, &act, NULL);
    act.sa_handler = flushBBSlog_time;
    sigaction(SIGALRM, &act, NULL);
    act.sa_handler = trunclog;
    sigaction(SIGUSR1, &act, NULL);
    act.sa_handler = truncboardlog;
    sigaction(SIGUSR2, &act, NULL);
    alarm(60*10); /*十分钟flush一次*/

    msqid = init_bbslog();
    if (msqid < 0)
        return -1;

    gb_trunclog=false;
    truncboard=false;
    openbbslog(1);
    while (1) {
        if ((msg = rcvlog(msqid)) != NULL)
            writelog(msg);
        if (gb_trunclog)
        	do_trunclog();
		else if(truncboard)
			do_truncboardlog();
    }
    flushlog(-1);
    for (i = 0; i < sizeof(logconfig) / sizeof(struct taglogconfig); i++) {
        free(logconfig[i].buf);
    }
}
