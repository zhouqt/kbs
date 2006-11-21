#include "innbbsconf.h"
#include "daemon.h"
#include "bbslib.h"
#include "bbs.h"
#undef VERSION
#include "version.h"
#include "inn_funcs.h"

#ifndef MAXCLIENT
#define MAXCLIENT 10
#endif

#ifndef ChannelSize
#define ChannelSize 4096
#endif

#ifndef ReadSize
#define ReadSize 1024
#endif

#ifndef DefaultINNBBSPort
#define DefaultINNBBSPort "7777"
#endif

#ifndef HIS_MAINT
#define HIS_MAINT
#define HIS_MAINT_HOUR 5
#define HIS_MAINT_MIN  30
#endif

int Maxclient = MAXCLIENT;
ClientType *Channel = NULL;
ClientType INNBBSD_STAT;

int Max_Art_Size = MAX_ART_SIZE;

int inetdstart = 0;

int Junkhistory = 0;

char *REMOTEUSERNAME, *REMOTEHOSTNAME;

static fd_set rfd, wfd, efd, orfd;

int clearfdset(int fd){
    FD_CLR(fd, &rfd);
    return 0;
}

static void channelcreate(client)
ClientType *client;
{
    buffer_t *in, *out;

    in = &client->in;
    out = &client->out;
    if (in->data != NULL)
        free(in->data);
    in->data = (char *) mymalloc(ChannelSize);
    in->left = ChannelSize;
    in->used = 0;
    if (out->data != NULL)
        free(out->data);
    out->data = (char *) mymalloc(ChannelSize);
    out->used = 0;
    out->left = ChannelSize;
    client->ihavecount = 0;
    client->ihaveduplicate = 0;
    client->ihavefail = 0;
    client->ihavesize = 0;
    client->statcount = 0;
    client->statfail = 0;
    client->begin = time(NULL);
    client->filter_buffer = NULL;
}

void channeldestroy(ClientType *client){
    if (client->in.data != NULL) {
        free(client->in.data);
        client->in.data = NULL;
    }
    if (client->out.data != NULL) {
        free(client->out.data);
        client->out.data = NULL;
    }
    if (client->filter_buffer != NULL) {
        free(client->filter_buffer);
        client->filter_buffer = NULL;
    }
#if !defined(PowerBBS) && !defined(DBZSERVER)
    if (client->ihavecount > 0 || client->statcount > 0) {
        innbbsdlog
            ("%s@%s rec: %d dup: %d fail: %d size: %d, stat rec: %d fail: %d, time sec: %d\n",
             client->username, client->hostname, client->ihavecount, client->ihaveduplicate, client->ihavefail, client->ihavesize, client->statcount, client->statfail, time(NULL) - client->begin);
        INNBBSD_STAT.ihavecount += client->ihavecount;
        INNBBSD_STAT.ihaveduplicate += client->ihaveduplicate;
        INNBBSD_STAT.ihavefail += client->ihavefail;
        INNBBSD_STAT.ihavesize += client->ihavesize;
        INNBBSD_STAT.statcount += client->statcount;
        INNBBSD_STAT.statfail += client->statfail;
    }
#endif
}

int inndchannel(char *port, char *path)
{
    time_t tvec;
    int i;
    int bbsinnd;
    int localbbsinnd;
    struct timeval tout;
    ClientType *client = (ClientType *) mymalloc(sizeof(ClientType) * Maxclient);
    int localdaemonready = 0;

    Channel = client;

    bbsinnd = pmain(port);
    if (bbsinnd < 0) {
        perror("pmain, existing");
        docompletehalt();
        return (-1);
    }
    FD_ZERO(&rfd);
    FD_ZERO(&wfd);
    FD_ZERO(&efd);

    localbbsinnd = p_unix_main(path);
    if (localbbsinnd < 0) {
        perror("local pmain, existing");
        if (!inetdstart)
            fprintf(stderr, "if no other innbbsd running, try to remove %s\n", path);
        close(bbsinnd);
        return (-1);
    } else {
        FD_SET(localbbsinnd, &rfd);
        localdaemonready = 1;
    }

    FD_SET(bbsinnd, &rfd);
    tvec = time((time_t *) 0);
    for (i = 0; i < Maxclient; ++i) {
        client[i].fd = -1;
        client[i].access = 0;
        client[i].buffer[0] = '\0';
        client[i].mode = 0;
        client[i].in.left = 0;
        client[i].in.used = 0;
        client[i].in.data = NULL;
        client[i].out.left = 0;
        client[i].out.used = 0;
        client[i].out.data = NULL;
        client[i].midcheck = 1;
    }
    for (;;) {
        int nsel, i;

/*
          When to maintain history files.
*/
        time_t now;
        static int maint = 0;
        struct tm *local;

        if (INNBBSDshutdown()) {
            HISclose();
            innbbsdlog(" Shutdown Complete \n");
            docompletehalt();
            exit(0);
        }
        time(&now);
        local = localtime(&now);
        if (local != NULL && local->tm_hour == His_Maint_Hour && local->tm_min >= His_Maint_Min) {
            if (!maint) {
                innbbsdlog(":Maint: start (%d:%d).\n", local->tm_hour, local->tm_min);
                HISmaint();
                time(&now);
                local = localtime(&now);
                if (local != NULL)
                    innbbsdlog(":Maint: end (%d:%d).\n", local->tm_hour, local->tm_min);
                maint = 1;
            }
        } else {
            maint = 0;
        }
/*
*/
/*
	in order to maintain history, timeout every 60 seconds in case
	no connections
*/
        tout.tv_sec = 60;
        tout.tv_usec = 0;
        orfd = rfd;
        if ((nsel = select(FD_SETSIZE, &orfd, NULL, NULL, &tout)) < 0) {
            continue;
        }
        if (localdaemonready && FD_ISSET(localbbsinnd, &orfd)) {
            int ns;

            ns = tryaccept(localbbsinnd);
            if (ns < 0)
                continue;
            for (i = 0; i < Maxclient; ++i) {
                if (client[i].fd == -1)
                    break;
            }
            if (i == Maxclient) {
                static char msg[] = "502 no free descriptors\r\n";

                printf("%s", msg);
                write(ns, msg, sizeof(msg));
                close(ns);
                continue;
            }
            client[i].fd = ns;
            client[i].buffer[0] = '\0';
            client[i].mode = 0;
            client[i].midcheck = 1;
            channelcreate(&client[i]);
            FD_SET(ns, &rfd);   /* FD_SET(ns,&wfd); */
            {
                strncpy(client[i].username, "localuser", 20);
                strncpy(client[i].hostname, "localhost", 128);
                client[i].Argv.in = fdopen(ns, "r");
                client[i].Argv.out = fdopen(ns, "w");
#if !defined(PowerBBS) && !defined(DBZSERVER)
                innbbsdlog("connected from (%s@%s).\n", client[i].username, client[i].hostname);
#endif
#ifdef INNBBSDEBUG
                printf("connected from (%s@%s).\n", client[i].username, client[i].hostname);
#endif
#ifdef DBZSERVER
                fprintf(client[i].Argv.out, "200 %s InterNetNews DBZSERVER server %s (%s@%s).\r\n", MYBBSID, VERSION, client[i].username, client[i].hostname);
#else
                fprintf(client[i].Argv.out, "200 %s InterNetNews INNBBSD server %s (%s@%s).\r\n", MYBBSID, VERSION, client[i].username, client[i].hostname);
#endif
                fflush(client[i].Argv.out);
                verboselog("UNIX Connect from %s@%s\n", client[i].username, client[i].hostname);
            }
        }
        if (FD_ISSET(bbsinnd, &orfd)) {
            int ns = tryaccept(bbsinnd);
            socklen_t length;
            struct sockaddr_in there;
            char *name;
            struct hostent *hp;

            if (ns < 0)
                continue;
            for (i = 0; i < Maxclient; ++i) {
                if (client[i].fd == -1)
                    break;
            }
            if (i == Maxclient) {
                static char msg[] = "502 no free descriptors\r\n";

                printf("%s", msg);
                write(ns, msg, sizeof(msg));
                close(ns);
                continue;
            }
            client[i].fd = ns;
            client[i].buffer[0] = '\0';
            client[i].mode = 0;
            client[i].midcheck = 1;
            channelcreate(&client[i]);
            FD_SET(ns, &rfd);   /* FD_SET(ns,&wfd); */
            length = sizeof(there);
            if (getpeername(ns, (struct sockaddr *) &there, &length)
                >= 0) {

#ifdef USE_IDENT
                name = (char *) my_rfc931_name(ns, &there);
#else
		name="unknown";
#endif
                strncpy(client[i].username, name, 20);
                hp = (struct hostent *) gethostbyaddr((char *)
                                                      &there.sin_addr, sizeof(struct in_addr), there.sin_family);
                if (hp)
                    strncpy(client[i].hostname, hp->h_name, 128);
                else
                    strncpy(client[i].hostname, (char *) inet_ntoa(there.sin_addr), 128);

                client[i].Argv.in = fdopen(ns, "r");
                client[i].Argv.out = fdopen(ns, "w");
                if ((char *)
                    search_nodelist(client[i].hostname, client[i].username) == NULL) {
                    innbbsdlog(":Err: invalid connection (%s@%s).\n", client[i].username, client[i].hostname);
                    fprintf(client[i].Argv.out, "502 You are not in my access file. (%s@%s)\r\n", client[i].username, client[i].hostname);
                    fflush(client[i].Argv.out);
                    fclose(client[i].Argv.in);
                    fclose(client[i].Argv.out);
                    close(client[i].fd);
                    FD_CLR(client[i].fd, &rfd);
                    client[i].fd = -1;
                    continue;
                }
                innbbsdlog("connected from (%s@%s).\n", client[i].username, client[i].hostname);
                if (isPause()) {
                    fprintf(client[i].Argv.out, "400 Server Paused. (%s@%s)\r\n", client[i].username, client[i].hostname);
                    fflush(client[i].Argv.out);
                    fclose(client[i].Argv.in);
                    fclose(client[i].Argv.out);
                    close(client[i].fd);
                    FD_CLR(client[i].fd, &rfd);
                    client[i].fd = -1;
                    continue;
                }
#ifdef INNBBSDEBUG
                printf("connected from (%s@%s).\n", client[i].username, client[i].hostname);
#endif
#ifdef DBZSERVER
                fprintf(client[i].Argv.out, "200 %s InterNetNews DBZSERVER server %s (%s@%s).\r\n", MYBBSID, VERSION, client[i].username, client[i].hostname);
#else
                fprintf(client[i].Argv.out, "200 %s InterNetNews INNBBSD server %s (%s@%s).\r\n", MYBBSID, VERSION, client[i].username, client[i].hostname);
#endif
                fflush(client[i].Argv.out);
                verboselog("INET Connect from %s@%s\n", client[i].username, client[i].hostname);
            } else {
            }

        }
        for (i = 0; i < Maxclient; ++i) {
            int fd = client[i].fd;

            if (fd < 0) {
                continue;
            }
            if (FD_ISSET(fd, &orfd)) {
                int nr;

#ifdef DEBUG
                printf("before read i %d in.used %d in.left %d\n", i, client[i].in.used, client[i].in.left);
#endif
                nr = channelreader(client + i);
#ifdef DEBUG
                printf("after read i %d in.used %d in.left %d\n", i, client[i].in.used, client[i].in.left);
#endif
                /*
                 * int nr=read(fd,client[i].buffer,1024); 
                 */
                if (nr <= 0) {
                    FD_CLR(fd, &rfd);
                    fclose(client[i].Argv.in);
                    fclose(client[i].Argv.out);
                    close(fd);
                    client[i].fd = -1;
                    channeldestroy(client + i);
                    continue;
                }
#ifdef DEBUG
                printf("nr %d %.*s", nr, nr, client[i].buffer);
#endif
                if (client[i].access == 0) {
                    continue;
                }
            }
        }
    }
}

void commandparse(ClientType *client)
{
    char *ptr, *lastend;
    argv_t *Argv = &client->Argv;
    int (*Main) ();
    char *buffer = client->in.data;
    buffer_t *in = &client->in;
    int dataused;
    int dataleft;

#ifdef DEBUG
    printf("%s %s buffer %s", client->username, client->hostname, buffer);
#endif
    ptr = (char *) strchr(in->data + in->used, '\n');
    if (client->mode == 0) {
        if (ptr == NULL) {
            in->used += in->lastread;
            in->left -= in->lastread;
            return;
        } else {
            dataused = ptr - (in->data + in->used) + 1;
            dataleft = in->lastread - dataused;
            lastend = ptr + 1;
        }
    } else {
        if (in->used >= 5) {
            ptr = (char *) strstr(in->data + in->used - 5, "\r\n.\r\n");
        } else if (strncmp(in->data, ".\r\n", 3) == 0) {
            ptr = in->data;
        } else {
            ptr = (char *) strstr(in->data + in->used, "\r\n.\r\n");
        }
        if (ptr == NULL) {
            in->used += in->lastread;
            in->left -= in->lastread;
            return;
        } else {
            ptr[2] = '\0';
            if (strncmp(in->data, ".\r\n", 3) == 0)
                dataused = 3;
            else
                dataused = ptr - (in->data + in->used) + 5;
            dataleft = in->lastread - dataused;
            lastend = ptr + 5;
            verboselog("Get: %s@%s end of data . size %d\n", client->username, client->hostname, in->used + dataused);
            client->ihavesize += in->used + dataused;
        }
    }
    if (client->mode == 0) {
        struct Daemoncmd *dp;

        Argv->argc = 0, Argv->argv = NULL, Argv->inputline = buffer;
        if (ptr != NULL)
            *ptr = '\0';
        verboselog("Get: %s\n", Argv->inputline);
        Argv->argc = argify(in->data + in->used, &Argv->argv);
        if (ptr != NULL)
            *ptr = '\n';
        dp = (struct Daemoncmd *) searchcmd(Argv->argv[0]);
        Argv->dc = dp;
        if (Argv->dc) {
#ifdef DEBUG
            printf("enter command %s\n", Argv->argv[0]);
#endif
            if (Argv->argc < dp->argc) {
                fprintf(Argv->out, "%d Usage: %s\r\n", dp->errorcode, dp->usage);
                fflush(Argv->out);
                verboselog("Put: %d Usage: %s\n", dp->errorcode, dp->usage);
            } else if (dp->argno != 0 && Argv->argc > dp->argno) {
                fprintf(Argv->out, "%d Usage: %s\r\n", dp->errorcode, dp->usage);
                fflush(Argv->out);
                verboselog("Put: %d Usage: %s\n", dp->errorcode, dp->usage);
            } else {
                Main = Argv->dc->main;
                if (Main) {
                    fflush(stdout);
                    (*Main) (client);
                }
            }
        } else {
            fprintf(Argv->out, "500 Syntax error or bad command\r\n");
            fflush(Argv->out);
            verboselog("Put: 500 Syntax error or bad command\r\n");
        }
        deargify(&Argv->argv);
    } else {
        if (Argv->dc) {
#ifdef DEBUG
            printf("enter data mode\n");
#endif
            Main = Argv->dc->main;
            if (Main) {
                fflush(stdout);
                (*Main) (client);
            }
        }
    }
    if (client->mode == 0) {
        if (dataleft > 0) {
            strncpy(in->data, lastend, dataleft);
#ifdef INNBBSDEBUG
            printf("***** try to copy %x %x %d bytes\n", in->data, lastend, dataleft);
#endif
        } else {
            dataleft = 0;
        }
        in->left += in->used - dataleft;
        in->used = dataleft;
    }
}

int channelreader(ClientType *client)
{
    int len;
    char *ptr;
    buffer_t *in = &client->in;

    if (in->left < ReadSize + 3) {
        int need = in->used + in->left + ReadSize + 3;

        need += need / 5;
        in->data = (char *) myrealloc(in->data, need);
        in->left = need - in->used;
        verboselog("channelreader realloc %d\n", need);
    }
    len = read(client->fd, in->data + in->used, ReadSize);

    if (len <= 0)
        return len;

    in->data[len + in->used] = '\0';
    in->lastread = len;
#ifdef DEBUG
    printf("after read lastread %d\n", in->lastread);
    printf("len %d client %d\n", len, strlen(in->data + in->used));
#endif

    REMOTEHOSTNAME = client->hostname;
    REMOTEUSERNAME = client->username;
    if (client->mode == 0) {
        if ((ptr = (char *) strchr(in->data, '\n')) != NULL) {
            if (in->data[0] != '\r')
                commandparse(client);
        }
    } else {
        commandparse(client);
    }
    return len;
}

void dopipesig(int s)
{
    printf("catch sigpipe\n");
    signal(SIGPIPE, dopipesig);
}

void standaloneinit(const char *port){
    FILE *fp;
    char pid_file_name[STRLEN],buf[STRLEN],*addr;
    int ndes=getdtablesize();

    if(!inetdstart){
        if(fork())
            exit(0);
    }

    snprintf(buf,STRLEN,"%s",port);
    if((addr=strstr(buf," [ADDRESS] "))){
        addr[0]=0;
        addr+=11;
        snprintf(pid_file_name,STRLEN,BBSHOME"/var/innbbsd.%s_%s.pid",buf,addr);
    }
    else
        snprintf(pid_file_name,STRLEN,BBSHOME"/var/innbbsd.%s.pid",buf);

#ifdef DEBUG
    if(!inetdstart)
        fprintf(stderr,"PIDFILE %s ... \n",pid_file_name);
#endif

    do{
        close(ndes);
    }
    while(--ndes>2);

    if((fp=fopen(pid_file_name,"w"))){
        fprintf(fp,"%d\n",(int)getpid());
        fclose(fp);
    }
}

extern char *optarg;
extern int opterr, optind;

void innbbsusage(name)
char *name;
{
    fprintf(stderr, "Usage: %s   [options] \n", name);
    fprintf(stderr, "   -v      (verbose log)\n");
    fprintf(stderr, "   -h|?    (help)\n");
    fprintf(stderr, "   -n      (not to use in core dbz)\n");
    fprintf(stderr, "   -i      (start from inetd with wait option)\n");
    fprintf(stderr, "   -c      (maximum connections accepted, default: %d)\n", Maxclient);
    fprintf(stderr, "   -j      (keep history of junk article, default=none)\n");
    fprintf(stderr, "   -a addr (bind to the specified address, default: all)\n");
    fprintf(stderr, "   -p port (assign port number of innbbsd daemon to bind)\n");
    fprintf(stderr, "   -l path (assign socket path for innbbsd to place)\n");
}

#ifdef DEBUGNGSPLIT
main()
{
    char **ngptr;
    char buf[1024];

    gets(buf);
    ngptr = (char **) BNGsplit(buf);
    printf("line %s\n", buf);
    while (*ngptr != NULL) {
        printf("%s\n", *ngptr);
        ngptr++;
    }
}
#endif

static time_t INNBBSDstartup;

int innbbsdstartup(void){
    return INNBBSDstartup;
}

int main(argc, argv)
int argc;
char **argv;
{
    char addr[STRLEN],service[STRLEN];
    char *port, *path;
    int c, errflag = 0;
    extern int INNBBSDhalt();

#if !defined(DBZSERVER)
    initial_lang();
#endif

    chdir(BBSHOME);
	init_sessiondata(getSession());

    addr[0]=0;
    port = DefaultINNBBSPort;
    path = LOCALDAEMON;
    Junkhistory = 0;

    time(&INNBBSDstartup);
    openlog("innbbsd", LOG_PID | LOG_ODELAY, LOG_DAEMON);
    while ((c = getopt(argc, argv, "c:a:p:l:f:s:vhidn?j")) != -1)
        switch (c) {
        case 'j':
            Junkhistory = 1;
            break;
        case 'v':
            verboseon("innbbsd.log");
            break;
        case 'n':
            hisincore(0);
            break;
        case 'c':
            Maxclient = atoi(optarg);
            if (Maxclient < 0)
                Maxclient = 0;
            break;
        case 'i':{
                struct sockaddr_in there;
                socklen_t len = sizeof(there);
                int rel;

                if ((rel = getsockname(0, (struct sockaddr *) &there, &len)) < 0) {
                    fprintf(stdout, "You must run -i from inetd with inetd.conf line: \n");
                    fprintf(stdout, "service-port stream  tcp wait  bbs  /home/bbs/innbbsd innbbsd -i port\n");
                    fflush(stdout);
                    exit(5);
                }
                inetdstart = 1;
                startfrominetd(1);
            }
            break;
        case 'd':
            dbzdebug(1);
            break;
        case 's':
            Max_Art_Size = atol(optarg);
            if (Max_Art_Size < 0)
                Max_Art_Size = 0;
            break;
        case 'a':
            if(optarg[0])
                snprintf(addr,STRLEN,"%s",optarg);
            break;
        case 'p':              /* yes, assign port number here .. */
            port = optarg;
            printf("p:%s\n", optarg);
            exit(0);
            break;
        case 'l':              /* sure, assign socket path here too .. */
            path = optarg;
            printf("path:%s\n", optarg);
            break;
        case 'h':
        case '?':
        default:
            errflag++;
        }
    if (errflag > 0) {
        innbbsusage(argv[0]);
        return (1);
    }

    if(addr[0]){
        snprintf(service,STRLEN,"%s [ADDRESS] %s",port,addr);
        port=service;
    }

    if(!getuid()&&!inetdstart){
        setuid((uid_t)BBSUID);
        setgid((gid_t)BBSGID);
    }

    /*
     * edwardc.000324 防呆 code .. 防止又有人用 root 跑 innbbsd 
     */
//#ifndef _IWANT_RUN_INNBBSD_WITH_OTHER_UID_      /* who will open this? he/she is mad .. :) */
    if ((getuid() != (uid_t) BBSUID && getgid() != (gid_t) BBSGID)
        && !inetdstart) {
        fprintf(stderr, "You should run INNBBSD as your bbs account (uid: %d) or group (gid: %d).\n", (uid_t) BBSUID, (gid_t) BBSGID);
        exit(6);
    }
//#endif

    standaloneinit(port);

    initial_bbs("feed");

#ifdef DEBUG
    if (!inetdstart)
        fprintf(stderr, "Try to listen in port %s and path %s\n", port, path);
#endif

    HISmaint();
    HISsetup();
    installinnbbsd();
    sethaltfunction(INNBBSDhalt);

    signal(SIGPIPE, dopipesig);
    inndchannel(port, path);
    HISclose();
    return 0;
}

