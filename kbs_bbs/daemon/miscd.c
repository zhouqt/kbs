#include <sys/types.h>
#include <sys/wait.h>
#include "sys/socket.h"
#include "netinet/in.h"
#include <signal.h>


#include "bbs.h"

void do_exit()
{
    flush_ucache();
    bbslog("4miscdaemon", "flush passwd file");
}

void do_exit_sig(int sig)
{
    do_exit();
    exit(0);
}

static char genbuf1[255];
int killdir(char *basedir, char *filename)
{
    int fd;
    struct stat st;
    struct fileheader *files, *afile;
    int i;
    int now = (time(NULL) / 86400) % 100;
    int count = 0;
    int deleted = 0;

    strcpy(genbuf1, basedir);
    strcat(genbuf1, "/");
    strcat(genbuf1, filename);
    fd = open(genbuf1, O_RDWR);
    if (fd < 0)
        return 0;
    if (flock(fd, LOCK_EX) < 0) {
        close(fd);
        return 0;
    }
    if (fstat(fd, &st) < 0) {
        flock(fd, LOCK_UN);
        close(fd);
        return 0;
    }
    if ((files = (struct fileheader *) malloc(st.st_size)) == NULL) {
        flock(fd, LOCK_UN);
        close(fd);
        return 0;
    }
    if (read(fd, files, st.st_size) < 0) {
        free(files);
        flock(fd, LOCK_UN);
        close(fd);
        return 0;
    }
    lseek(fd, 0, 0);
    for (i = 0, afile = files; i < st.st_size / sizeof(struct fileheader); i++, afile++) {
	int delta;
	delta=now-afile->accessed[11];
	if (delta<0) delta+=100;
        if (delta > DAY_DELETED_CLEAN) {
            strcpy(genbuf1, basedir);
            strcat(genbuf1, "/");
            strcat(genbuf1, afile->filename);
            unlink(genbuf1);
            deleted++;
        } else {
            write(fd, afile, sizeof(struct fileheader));
            count += sizeof(struct fileheader);
        }
    }
    ftruncate(fd, count);
    flock(fd, LOCK_EX);
    free(files);
    close(fd);
    return deleted;
}

int dokilldir(char *board)
{
    char hehe[255];
    int killed;

    if (!board[0])
        return 0;
    strcpy(hehe, "boards");
    strcat(hehe, "/");
    strcat(hehe, board);
    killed = killdir(hehe, ".DELETED") + killdir(hehe, ".JUNK");
    newbbslog(BBSLOG_USIES, "deleted %d files in %s board", killed, board);
    return killed;
}

int doaboard(struct boardheader *brd,void* arg)
{
    if (!brd)
        return 0;
    return dokilldir(brd->filename);
}

int dokillalldir()
{
    resolve_boards();
    apply_boards(doaboard,NULL);
    return 0;
}
static char tmpbuf[255];
static char genbuf1[255];

#if 0
char *setmailpath(buf, userid)  /* 取 某用户 的mail */
    char *buf, *userid;
{
    if (isalpha(userid[0]))     /* 加入错误判断,提高容错性, alex 1997.1.6 */
        sprintf(buf, "mail/%c/%s", toupper(userid[0]), userid);
    else
        sprintf(buf, "mail/wrong/%s", userid);
    return buf;
}

char *sethomepath(buf, userid)  /* 取 某用户 的home */
    char *buf, *userid;
{
    if (isalpha(userid[0]))     /* 加入错误判断,提高容错性, alex 1997.1.6 */
        sprintf(buf, "home/%c/%s", toupper(userid[0]), userid);
    else
        sprintf(buf, "home/wrong/%s", userid);
    return buf;
}
#endif
int killauser(struct userec *theuser, char *data)
{
    int a;
    struct userec *ft;

    if (!theuser || theuser->userid[0] == 0)
        return 0;
    a = compute_user_value(theuser);
    if ((a <= 0)&&strcmp(theuser->userid,"guest")) {
        newbbslog(BBSLOG_USIES, "kill user %s", theuser->userid);
        a = getuser(theuser->userid, &ft);
        setmailpath(tmpbuf, theuser->userid);
        sprintf(genbuf1, "/bin/rm -rf %s", tmpbuf);
        system(genbuf1);
        sethomepath(tmpbuf, theuser->userid);
        sprintf(genbuf1, "/bin/rm -rf %s", tmpbuf);
        system(genbuf1);
        sprintf(genbuf1, "/bin/rm -fr tmp/email/%s", theuser->userid);
        system(genbuf1);
        setuserid2(a, "");
        theuser->userlevel = 0;
        /*strcpy(theuser->address, "");*/
        strcpy(theuser->username, "");
        /*strcpy(theuser->realname, "");*/
    }

    return 0;
}

int dokilluser()
{
    newbbslog(BBSLOG_USIES, "Started kill users\n");
    apply_users(killauser, NULL);
    newbbslog(BBSLOG_USIES, "kill users done\n");
    return 0;
}
int updateauser(struct userec *theuser, char *data)
{
    FILE *fn;
    char genbuf[255];
    int lcount = 0, tcount, s[10][2], i, j;

    if ((theuser->userlevel & PERM_BASIC) && (theuser->userlevel & PERM_POST)
        && (theuser->userlevel & PERM_CHAT) && (theuser->userlevel & PERM_PAGE)
        && !(theuser->userlevel & PERM_DENYMAIL)
        && !(theuser->userlevel & PERM_DENYRELAX))
        return 0;
    if (!(theuser->userlevel & PERM_LOGINOK))
        return 0;
    sethomefile(genbuf, theuser->userid, "giveup");
    fn = fopen(genbuf, "rt");
    if (fn) {
        while (!feof(fn)) {
            if (fscanf(fn, "%d %d", &i, &j) <= 0)
                break;
            s[lcount][0] = i;
            s[lcount][1] = j;
            lcount++;
        }
        fclose(fn);
        tcount = lcount;
        for (i = 0; i < lcount; i++) {
            if (s[i][1] <= time(0) / 3600 / 24) {
                tcount--;
                switch (s[i][0]) {
                case 1:
                    theuser->userlevel |= PERM_BASIC;
                    break;
                case 2:
                    theuser->userlevel |= PERM_POST;
                    break;
                case 3:
                    theuser->userlevel |= PERM_CHAT;
                    break;
                case 4:
                    theuser->userlevel |= PERM_PAGE;
                    break;
                case 5:
                    theuser->userlevel &= ~PERM_DENYMAIL;
                    break;
                case 6:
                    theuser->userlevel &= ~PERM_DENYRELAX;
                    break;
                }
                s[i][1] = 0;
            }
        }
        if (theuser->flags & GIVEUP_FLAG && tcount == 0)
            theuser->flags &= ~GIVEUP_FLAG;
        if (tcount == 0)
            unlink(genbuf);
        else {
            fn = fopen(genbuf, "wt");
            for (i = 0; i < lcount; i++)
                if (s[i][1] > 0)
                    fprintf(fn, "%d %d\n", s[i][0], s[i][1]);
            fclose(fn);
        }
    }
    return 0;
}

int doupdategiveupuser()
{
    newbbslog(BBSLOG_USIES, "Started update giveup users\n");
    apply_users(updateauser, NULL);
    newbbslog(BBSLOG_USIES, "update giveup users done\n");
    return 0;
}

int getnextday4am()
{
    time_t now = time(0);
    struct tm *tm = localtime(&now);

    if (tm->tm_hour >= 4) {
        now += 86400;
        tm = localtime(&now);
    }
    tm->tm_hour = 4;
    tm->tm_sec = 0;
    tm->tm_min = 4;
    return mktime(tm);
}

int ismonday()
{
    time_t now = time(0);
    struct tm *tm = localtime(&now);

    return tm->tm_wday == 1;
}

static char *username;
static char *cmd;
static int num;
struct requesthdr {
    int command;
    union {
        struct user_info utmp;
        int uent;
    } u_info;
} utmpreq;
int getutmprequest(int m_socket)
{
    int len;
    struct sockaddr_in sin;
    int s;
    char *pnum;
    char *phdr = (char *) &utmpreq;
    int totalread;

    len = sizeof(sin);
    for (s = accept(m_socket, &sin, &len);; s = accept(m_socket, &sin, &len)) {
        if ((s <= 0) && errno != EINTR) {
            bbslog("3system", "utmpd:accept %s", strerror(errno));
            exit(-1);
        }
        if (s <= 0)
            continue;
        memset(&utmpreq, 0, sizeof(utmpreq));
        len = 1;

        while (totalread < sizeof(utmpreq) && len > 0) {
            len = read(s, phdr, sizeof(utmpreq) - totalread);
            if (len > 0) {
                totalread += len;
                phdr += len;
            }
        }
        if (len <= 0) {
            close(s);
            continue;
        }
        close(s);
    }
    return s;
}

int getrequest(int m_socket)
{
    int len;
    struct sockaddr_in sin;
    int s;
    char *pnum;

    len = sizeof(sin);

    for (s = accept(m_socket, &sin, &len);; s = accept(m_socket, &sin, &len)) {
        if ((s <= 0) && errno != EINTR) {
            bbslog("3system", "userd:accept %s", strerror(errno));
            exit(-1);
        }
        if (s <= 0)
            continue;
        memset(tmpbuf, 0, 255);
        len = read(s, tmpbuf, 255);
        if (len <= 0) {
            close(s);
            continue;
        }
        strtok(tmpbuf, " ");
        cmd = tmpbuf;
        username = strtok(NULL, " ");
        pnum = strtok(NULL, " ");
        num = pnum ? atoi(pnum) : 0;
        if (strcmp(tmpbuf, "QUIT") == 0)
            exit(0);
        if (strcmp(tmpbuf, "NEW") == 0)
            break;
        if (strcmp(tmpbuf, "SET") == 0)
            break;
        if (strcmp(tmpbuf, "DEL") == 0) {
            pnum = username;
            num = atoi(pnum);
            break;
        }
        close(s);
    }
    return s;
}

void putrequest(int sock, int id)
{
    write(sock, &id, sizeof(id));
    close(sock);
}

void userd()
{
    int m_socket;

    struct sockaddr_in sin;
    int sinlen = sizeof(sin);
    int opt = 1;

    bzero(&sin, sizeof(sin));
    if ((m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        bbslog("3system", "userd:socket %s", strerror(errno));
        exit(-1);
    }
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, 4);
    memset(&sin, 0, sinlen);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(60001);
#ifdef HAVE_INET_ATON
    inet_aton("127.0.0.1", &sin.sin_addr);
#elif defined HAVE_INET_PTON
	inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
#else
	/* Is it OK? */
    my_inet_aton("127.0.0.1", &sin.sin_addr);
#endif
    if (0 != bind(m_socket, (struct sockaddr *) &sin, sizeof(sin))) {
        bbslog("3system", "userd:bind %s", strerror(errno));
        exit(-1);
    }
    if (0 != listen(m_socket, 5)) {
        bbslog("3system", "userd:listen %s", strerror(errno));
        exit(-1);
    }
    while (1) {
        int sock, id;

        sock = getrequest(m_socket);
        if (!strcmp(cmd, "NEW"))
            id = getnewuserid(username);
        if (!strcmp(cmd, "SET")) {
            setuserid2(num, username);
            id = 0;
        }
        if (!strcmp(cmd, "DEL")) {
            setuserid2(num, "");
            id = 0;
        }
        putrequest(sock, id);
    }
    return;
}

void utmpd()
{
    int m_socket;

    struct sockaddr_in sin;
    int sinlen = sizeof(sin);
    int opt = 1;

    bzero(&sin, sizeof(sin));
    if ((m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        bbslog("3system", "utmpd:socket %s", strerror(errno));
        exit(-1);
    }
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, 4);
    memset(&sin, 0, sinlen);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(60002);
#ifdef HAVE_INET_ATON
    inet_aton("127.0.0.1", &sin.sin_addr);
#elif defined HAVE_INET_PTON
	inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
#else
	/* Is it OK? */
    my_inet_aton("127.0.0.1", &sin.sin_addr);
#endif
    if (0 != bind(m_socket, (struct sockaddr *) &sin, sizeof(sin))) {
        bbslog("3system", "utmpd:bind %s", strerror(errno));
        exit(-1);
    }
    if (0 != listen(m_socket, 5)) {
        bbslog("3system", "utmpd:listen %s", strerror(errno));
        exit(-1);
    }
    while (1) {
        int sock, id;

        sock = getutmprequest(m_socket);
#if 0
        {                       /*kill user */
            time_t now;
            struct user_info *uentp;

            now = time(NULL);
            if ((now > utmpshm->uptime + 120) || (now < utmpshm->uptime - 120)) {
                int n;

                utmpshm->uptime = now;
                bbslog("1system", "UTMP:Clean user utmp cache");
                for (n = 0; n < USHM_SIZE; n++) {
                    utmpshm->uptime = now;
                    uentp = &(utmpshm->uinfo[n]);
                    if (uentp->active && uentp->pid && kill(uentp->pid, 0) == -1) {     /*uentp检查 */
                        char buf[STRLEN];

                        strncpy(buf, uentp->userid, IDLEN + 2);
                        clear_utmp(n + 1);
                        RemoveMsgCountFile(buf);
                    }
                }
            }
        }
#endif
        /* utmp */
        switch (utmpreq.command) {
        case 1:                // getnewutmp
            id = getnewutmpent2(&utmpreq.u_info.utmp);
            break;
        case 2:
            id = -1;
            break;              // clear, by uentp
        case 3:                // clear, by id
            /*这个代码有错误的，因为pid不能不判断 */
            clear_utmp(utmpreq.u_info.uent, 0, 0);
            id = 0;
            break;
        default:
            id = -1;
            break;
        }
        putrequest(sock, id);
    }
    return;
}


void flushd()
{
    struct sigaction act;

    atexit(do_exit);
    bzero(&act, sizeof(act));
    act.sa_handler = do_exit_sig;
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGABRT, &act, NULL);

    while (1) {
        sleep(2 * 60 * 60);
        flush_ucache();
        bbslog("4miscdaemon", "flush passwd file");
    };
}

static void reaper()
{
    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}

void timed()
{
    setpublicshmreadonly(0);
    while (1) {
#undef time
        bbssettime(time(0));
        sleep(1);
#define time(x) bbstime(x)
    }
}

static int miscd_dodaemon(char *argv1, char *daemon)
{
    struct sigaction act;
    char *commandline;
    char commbuf[10];
    char ch;

    if (load_ucache() != 0) {
        printf("ft,load ucache error!");
        exit(-1);
    }

#if USE_TMPFS==1
    /* init tmpfs */
    sprintf(genbuf1,"%s/home",TMPFSROOT);
    mkdir(genbuf1,0700);
    sprintf(genbuf1,"%s/boards",TMPFSROOT);
    mkdir(genbuf1,0700);
    for (ch='A';ch<='Z';ch++) {
    sprintf(genbuf1,"%s/home/%c",TMPFSROOT,ch);
    mkdir(genbuf1,0700);
    }
#endif
    resolve_boards();
#if HAVE_WWW==1
    resolve_guest_table();
#endif

    if (argv1 != NULL) {
        switch (fork()) {
        case -1:
            printf("faint, i can't fork.\n");
            exit(0);
            break;
        case 0:
            break;
        default:
            exit(0);
            break;
        }
        commandline = argv1;
    } else {
        commandline = commbuf;
    }
    setsid();
#ifdef AIX
    setpgrp();
#elif defined FREEBSD
    setpgid(0, 0);
#else
    // by zixia setpgrp(0, 0);
    setpgrp();
#endif
#ifdef AIX
    act.sa_handler = NULL;
    act.sa_flags = SA_RESTART | SA_NOCLDWAIT;
    sigaction(SIGCHLD, &act, NULL);
#else
    act.sa_handler = reaper;
    act.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &act, NULL);
#endif
    if (((daemon == NULL) || (!strcmp(daemon, "timed"))) && ((argv1 == NULL) || fork())) {
        strcpy(commandline, "timed");
        timed();
    }

    if (((daemon == NULL) || (!strcmp(daemon, "killd"))) && ((argv1 == NULL) || fork())) {
        strcpy(commandline, "killd");
        while (1) {
            int ft;

            if (argv1 == NULL) {
                dokilluser();
                doupdategiveupuser();
            } else {
                switch (fork()) {
                case -1:
                    bbslog("3error", "fork failed\n");
                    break;
                case 0:
                    dokilluser();
                    doupdategiveupuser();
                    exit(0);
                    break;
                default:
                    break;
                }
            }
            switch (fork()) {
                case -1:
                    bbslog("3error", "fork failed\n");
                    break;
                case 0:
                    dokillalldir();
                    exit(0);
                    break;
                default:
                    break;
            }
            ft = time(0);
            do {
                sleep(86400 - (time(0) - ft));  /* 1 day */
            } while (ft + 86400 > time(0));
        };
    }
    if (((daemon == NULL) || (!strcmp(daemon, "userd"))) && ((argv1 == NULL) || fork())) {
        strcpy(commandline, "userd");
        userd();
    }
    if ((daemon == NULL) || (!strcmp(daemon, "flushd"))) {
        strcpy(commandline, "flushd");
        flushd();
    }
    return 0;
}
int main(int argc, char *argv[])
{
    chdir(BBSHOME);
    setuid(BBSUID);
    setgid(BBSGID);
    setreuid(BBSUID, BBSUID);
    setregid(BBSGID, BBSGID);

#undef time
    bbssettime(time(0));
    sleep(1);
#define time(x) bbstime(x)
    setpublicshmreadonly(0);
    setpublicshmreadonly(1);
    init_bbslog();
    if (argc > 1) {
        if (strcasecmp(argv[1], "killuser") == 0) {
            if (resolve_ucache() != 0)
                return -1;
            return dokilluser();
        }
        if (strcasecmp(argv[1], "giveup") == 0) {
            if (resolve_ucache() != 0)
                return -1;
            return doupdategiveupuser();
        }
        if (strcasecmp(argv[1], "allboards") == 0)
            return dokillalldir();
        if (strcasecmp(argv[1], "daemon") == 0)
            return miscd_dodaemon(argv[1], argv[2]);
        if (strcasecmp(argv[1], "killdir") == 0)
            return dokilldir(argv[2]);
        return miscd_dodaemon(NULL, argv[1]);
    }
    printf("Usage : %s killuser to kill old users\n", argv[0]);
    printf("        %s allboards to delete all old files\n", argv[0]);
    printf("        %s daemon to run as a daemon\n", argv[0]);
    printf("        %s BOARDNAME to delete old file in BOARDNAME\n", argv[0]);
    printf("That's all, folks\n");

    return 0;
}
