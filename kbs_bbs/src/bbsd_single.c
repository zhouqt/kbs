#include "bbs.h"
#include <arpa/telnet.h>
#include <sys/resource.h>
#ifdef HAVE_REVERSE_DNS
#include <netdb.h>
#endif

#define	QLEN		50
#define	PID_FILE	"reclog/bbs.pid"
#define	BBSLOG_FILE	"reclog/bbs.log"

static int mport;
int csock;                      /* socket for Master and Child */

int max_load = 79;              /* 原值39 , modified by KCN,1999.09.07 */

int proxy_getpeername(int csock,struct sockaddr* psaddr,int* plen)
{
    int ret=getpeername(csock,psaddr,plen);
#ifdef SMTH
    struct sockaddr_in* psin=(struct sockaddr_in*)psaddr;
    if ((psin->sin_addr.s_addr==0xea086fa6)||
        (psin->sin_addr.s_addr==0xeb086fa6)) { //166.111.8.234
        int buf;
        read(csock,&buf,4);
        if (buf==0x330123) {
            read(csock,&psin->sin_addr.s_addr,4);
        }
    }
#endif
    return ret;
}

/* 59->79 , modified by dong , 1999.9.7 */

/* 原值29, modified by dong, 1998.11.23 */
#undef LOAD_LIMIT
int heavy_load;
int server_pid;

/* COMMAN: for debugging use */
static int no_fork = 0;

char code[3];
void cat(filename, msg)
    char *filename, *msg;
{
    /* 将msg放到以filename为名的文件中 */
    FILE *fp;

    if ((fp = fopen(filename, "a")) != NULL) {
        fputs(msg, fp);
        fclose(fp);
    }
}

int local_Net_Sleep(time)
    time_t time;
{
    /*
       time秒内 如果 端口csock 有数据就读入然后丢弃。 
     */
    struct timeval tv;
    int sr;
    fd_set fd, efd;

    tv.tv_sec = time;
    tv.tv_usec = 0;
    FD_ZERO(&fd);
    FD_ZERO(&efd);
    FD_SET(csock, &fd);
    FD_SET(csock, &efd);

    while ((sr = select(csock + 1, &fd, NULL, &efd, &tv)) > 0) {
        /* Select() returns the total number of ready        
           descriptors in all the sets.    
         */
        char buf[256];

        if (FD_ISSET(csock, &efd))
            break;
        if (recv(csock, buf, 256, 0) <= 0)
            break;
        tv.tv_sec = time;
        tv.tv_usec = 0;
        FD_SET(csock, &fd);
        FD_SET(csock, &efd);
    };
    return 0;
}

void local_prints(char *fmt, ...)
{
    va_list args;
    char buf[512];

    va_start(args, fmt);
    /*fmt = va_arg(args, char *);*/

    vsprintf(buf, fmt, args);
    va_end(args);

#ifdef SSHBBS
    ssh_write(0, buf, strlen(buf));
#else
    write(0, buf, strlen(buf));
#endif
}

#ifdef LOAD_LIMIT
#ifdef AIX
#include <rpcsvc/rstat.h>
#endif
void get_load(load)
/*取系统负载*/
    double load[];
{
#ifdef LINUX
    FILE *fp;

    fp = fopen("/proc/loadavg", "r");
    if (!fp)
        load[0] = load[1] = load[2] = 0;
    else {
        float av[3];

        fscanf(fp, "%g %g %g", av, av + 1, av + 2);
        fclose(fp);
        load[0] = av[0];
        load[1] = av[1];
        load[2] = av[2];
    }
#else
#ifdef BSD44
    getloadavg(load, 3);
#else
    struct statstime rs;

    rstat("localhost", &rs);
    load[0] = rs.avenrun[0] / (double) (1 << 8);
    load[1] = rs.avenrun[1] / (double) (1 << 8);
    load[2] = rs.avenrun[2] / (double) (1 << 8);
#endif
#endif
}
#endif

static void telnet_init()
{
    int n, len;
    char *cmd;
    char svr[] = {
        IAC, DO, TELOPT_TTYPE,   
        IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE, 
        IAC, WILL, TELOPT_ECHO,
        IAC, WILL, TELOPT_SGA,
        IAC, WILL, TELOPT_BINARY,
        IAC, DO, TELOPT_NAWS,
        IAC, DO,   TELOPT_BINARY
    };


    /* --------------------------------------------------- */
    /* init telnet protocol                              */
    /* --------------------------------------------------- */

    cmd = svr;

    for (n = 0; n < 7; n++) {
        len = (n == 1 ? 6 : 3);
        send(0, cmd, len, 0);
        cmd += len;
        /* KCN !!!!!!!!!! wrong telnet
           #ifdef       LINUX
           to.tv_sec = 1;
           to.tv_usec = 1;
           #endif

           rset = 1;
           if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
           recv(0, buf, sizeof(buf), 0);
         */
    }
}

/* ----------------------------------------------------- */
/* stand-alone daemon					 */
/* ----------------------------------------------------- */


static void start_daemon(inetd, port)
    int inetd;
    int port;                   /* Thor.981206: 取 0 代表 *没有参数* */
{
    int n;
    struct sockaddr_in sin;
    char buf[80];
    time_t val;
    FILE *lock_pid;

    /*
     * More idiot speed-hacking --- the first time conversion makes the C
     * library open the files containing the locale definition and time zone.
     * If this hasn't happened in the parent process, it happens in the
     * children, once per connection --- and it does add up.
     */

    time(&val);
    strftime(buf, 80, "%d/%b/%Y %H:%M:%S", localtime(&val));

    /* --------------------------------------------------- */
    /* detach daemon process                             */
    /* --------------------------------------------------- */

    /*
       close file descriptor 1 and 2
     */
    chdir(BBSHOME);             /*将当前目录转换到BBSHOME */
    umask(07);

    if (inetd) {                /* Thor.981206: inetd -i */
        /* Give up root privileges: no way back from here        */
        close(1);
        close(2);
        server_pid = 0;
        setgid(BBSGID);         /* setgid sets the effective group ID of the current process. */
        setuid(BBSUID);         /* sets the effective user ID of the current process. */
        setreuid(BBSUID, BBSUID);       /* setreuid  sets real and effective user ID's 
                                           of the current process. */
        setregid(BBSGID, BBSGID);       /* setregid sets real and effective group ID's 
                                           of the current process. */
        mport = port;
        if (port)
            strcpy(code, "e");
        else
            strcpy(code, "d");  /*没有参数的话将code置'd' */
        /*    sprintf(data, "%d\tinetd -i\n", getpid() );
           cat(PID_FILE, data);
         */
        return;
    }
    else {
      sprintf(buf, "bbsd.%d", port);
      switch (dodaemon(buf,true,true)) {
      	case 0:
      		break;
      	case 1:
      		fprintf(stderr,"A bbsd on %d already running!\n",port);
      		exit(0);
      	case 2:
      		fprintf(stderr,"can't lock pid file:var/%s.pid\n",buf);
      		exit(0);
      }
      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = htonl(INADDR_ANY);
      /*    sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);*/
      n = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

      val = 1;
      setsockopt(n, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(val));

#if 0
      setsockopt(n, SOL_SOCKET, SO_KEEPALIVE, (char *) &val, sizeof(val));

      setsockopt(n, IPPROTO_TCP, TCP_NODELAY, (char *) &val, sizeof(val));
#endif

      mport = port;
      if (port == 6001)
        strcpy(code, "e");
      else
        strcpy(code, "d");
      sin.sin_port = htons(port);
      if ((bind(n, (struct sockaddr *) &sin, sizeof(sin)) < 0) || (listen(n, QLEN) < 0)) {
        exit(1);
      }

      setgid(BBSGID);
      setuid(BBSUID);
    }
}


/* ----------------------------------------------------- */
/* reaper - clean up zombie children			 */
/* ----------------------------------------------------- */


static void siguser1(int signo)
{
    heavy_load = 1;
}
static void siguser2(int signo)
{
    heavy_load = 0;
}

static void reaper(int signo)
{
    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}

static void main_term(int signo)
{
    exit(0);
}

static void main_signals()
{
    struct sigaction act;

/* act.sa_mask = 0; *//* Thor.981105: 标准用法 */
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    act.sa_flags = SA_RESTART;
    act.sa_handler = main_term;
    sigaction(SIGTERM, &act, NULL);

    act.sa_handler = siguser1;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_handler = siguser2;
    sigaction(SIGUSR2, &act, NULL);

    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, NULL);

    act.sa_handler = SIG_IGN;
    sigaction(SIGTTOU, &act, NULL);

    act.sa_handler = SIG_IGN;
    sigaction(SIGHUP, &act, NULL);

    /*  act.sa_handler = reaper; */
#ifdef AIX
    act.sa_handler = NULL;
    act.sa_flags = SA_RESTART | SA_NOCLDWAIT;
    sigaction(SIGCHLD, &act, NULL);
#else
    act.sa_handler = reaper;
    act.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &act, NULL);
#endif
    /* COMMAN: Ignore other signals used in bbs, to avoid group killing(pid=0)
     */

}

#ifdef HAVE_REVERSE_DNS
static void 
dns_query_timeout(int sig)
{
	longjmp(byebye, sig);
}

static void getremotehost(int sockfd, char *rhost, int buf_len)
{
	struct hostent *hp;
	char *ptr;
	struct sockaddr_in sin;
	int value;
	char buf[STRLEN];
	
	value = sizeof(sin);
	proxy_getpeername(csock, (struct sockaddr *) &sin, (socklen_t *) & value);
	if (setjmp(byebye) == 0)
	{
		signal(SIGALRM, dns_query_timeout);
		alarm(5);
		hp = gethostbyaddr((char *) &(sin.sin_addr), sizeof(struct in_addr),
		                   sin.sin_family);
		alarm(0);
	}
	strncpy(buf, hp ? hp->h_name : (char *) inet_ntoa(sin.sin_addr), 
		sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	if ((ptr = strstr(buf, "." NAME_BBS_ENGLISH)) != NULL)
		*ptr = '\0';
	strncpy(rhost, buf, buf_len - 1);
	rhost[buf_len - 1] = '\0';
}
#endif /* HAVE_REVERSE_DNS */

#define MAXLIST 1000
#define CON_THRESHOLD 1000.0/60/60
#define CON_THRESHOLD2 1.0

struct ip_struct{
    unsigned char ip[4];
    time_t first,last;
    int t;
} * ips, * bads, * proxys;
bool initIP=false;


int check_IP_lists(unsigned int IP2)
{
    int i,j,k;
    FILE* fp;
    int ip[4];
    int found=0,min=0,ret=0;
    time_t now;
    if(!initIP) {
        ips = (struct ip_struct *) malloc(sizeof(struct ip_struct)*MAXLIST);
        bads = (struct ip_struct *) malloc(sizeof(struct ip_struct)*MAXLIST);
        proxys = (struct ip_struct *) malloc(sizeof(struct ip_struct)*MAXLIST);
        memset(ips, 0, sizeof(struct ip_struct)*MAXLIST);
        memset(bads, 0, sizeof(struct ip_struct)*MAXLIST);
        memset(proxys, 0, sizeof(struct ip_struct)*MAXLIST);
        fp=fopen(".denyIP", "r");
        if(fp) {
            i=0;
            while(!feof(fp)) {
                if(fscanf(fp, "%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3])<=0) break;
                bads[i].ip[0]=ip[0];
                bads[i].ip[1]=ip[1];
                bads[i].ip[2]=ip[2];
                bads[i].ip[3]=ip[3];
                i++;
            }
            fclose(fp);
        }
        fp=fopen(".proxyIP", "r");
        if(fp) {
            i=0;
            while(!feof(fp)) {
                if(fscanf(fp, "%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3])<=0) break;
                proxys[i].ip[0]=ip[0];
                proxys[i].ip[1]=ip[1];
                proxys[i].ip[2]=ip[2];
                proxys[i].ip[3]=ip[3];
                i++;
            }
            fclose(fp);
        }
        initIP=true;
    }
    now = time(0);
    ip[0]=IP2&0xff;
    ip[1]=(IP2&0xff00)>>8;
    ip[2]=(IP2&0xff0000)>>16;
    ip[3]=(IP2&0xff000000)>>24;
    if(ip[0]==0) return 0;
    for(i=0;i<MAXLIST;i++) {
        if(bads[i].ip[0]==0) break;
        if(ip[0]==bads[i].ip[0]&&ip[1]==bads[i].ip[1]&&ip[2]==bads[i].ip[2]&&ip[3]==bads[i].ip[3])
            return 1;
    }
    for(i=0;i<MAXLIST;i++) {
        if(proxys[i].ip[0]==0) break;
        if(ip[0]==proxys[i].ip[0]&&ip[1]==proxys[i].ip[1]&&ip[2]==proxys[i].ip[2]&&ip[3]==proxys[i].ip[3])
            return 0;
    }
    for(i=0;i<MAXLIST;i++) {
        if((double)(now-ips[i].last)>60*60) {
            ips[i].ip[0]=0;
        }
        if(ip[0]==ips[i].ip[0]&&ip[1]==ips[i].ip[1]&&ip[2]==ips[i].ip[2]&&ip[3]==ips[i].ip[3]){
            if((double)(now-ips[i].last)<=CON_THRESHOLD2) {
                fp=fopen(".IPdenys", "a");
                if(fp){
                    fprintf(fp, "0 %ld %d.%d.%d.%d %d\n", (unsigned int)now, ip[0],ip[1],ip[2],ip[3], ips[i].t);
                    fclose(fp);
                }
                ret = 1;
            }
            found=1;
            ips[i].last = now;
            ips[i].t++;
            if(ips[i].t>=10&&(ips[i].t/(double)(ips[i].last-ips[i].first)>=CON_THRESHOLD)) {
                ips[i].t=100000;
                fp=fopen(".IPdenys", "a");
                if(fp){
                    fprintf(fp, "1 %ld %d.%d.%d.%d %d\n", (unsigned int)now, ip[0],ip[1],ip[2],ip[3], ips[i].t);
                    fclose(fp);
                }
                ret = 1;
            }
            break;
        }
        if(ips[i].last<ips[min].last) min = i;
    }
    if(!found) {
        ips[min].ip[0]=ip[0];
        ips[min].ip[1]=ip[1];
        ips[min].ip[2]=ip[2];
        ips[min].ip[3]=ip[3];
        ips[min].first = now;
        ips[min].last = now;
        ips[min].t = 1;
    }

    return ret;
}

int bbs_main(argv)
    char *argv;
{
    char buf[256];              /* Leeward 98.07.31 */
    char bbs_prog_path[256];
    FILE *fp;

    /*    modified by period      2000-11-13      allow localhost anyway  */
    /*    if((fp = fopen("NOLOGIN","r")) != NULL) */
#ifndef DEBUG
    if (strcmp(fromhost, "0.0.0.0") && strcmp(fromhost, "127.0.0.1") && (fp = fopen("NOLOGIN", "r")) != NULL) {
        while (fgets(buf, 256, fp) != NULL)
            local_prints("%s", buf);
        fclose(fp);
        local_Net_Sleep(20);
        shutdown(csock, 2);
        close(csock);
        return -1;
    }
#endif
#ifdef LOAD_LIMIT

    if ((fp = fopen("NO_LOAD_LIMIT", "r")) == NULL) {   /* add by dong , 1999, 9, 10 */
        /* load control for BBS */
        {
            double cpu_load[3];
            double load;

            get_load(cpu_load);
            load = cpu_load[0];
            local_prints("CPU 最近 (1,10,15) 分钟的平均负荷分别为 %.2f, %.2f, %.2f (目前上限 = %d)\r\n", cpu_load[0], cpu_load[1], cpu_load[2], max_load);
            if (load < 0 || load > max_load) {
                local_prints("很抱歉,目前 CPU 负荷过重, 请稍候再来\r\n");
                local_prints("\r\n因为重复连接对本站冲击太大，请您配合，不要重复多次连接");
                local_prints("\r\n请您先休息10分钟，然后再连接本站，非常感谢！\r\n");
                /*sleep( load ); */
                /*sleep(1);  modified by dong, 1999.9.10 */
                local_Net_Sleep(load);
                shutdown(csock, 2);
                close(csock);

                return -1;
            }
#ifdef AIX
            {                   /* Leeward 98.06.11 For AIX only. Please man psdanger */
                int free = psdanger(-1);
                int safe = psdanger(SIGDANGER);
                int danger = 125000;

                local_prints("RAM 当前空闲页数高出警戒阈值 %d (警戒阈值 = %d)\r\n\r\n", safe, free - safe);

                if (safe < danger /*Haohmaru.99.06.16.rem free - safe */ ) {
                    if ((server_pid != -1) && (!heavy_load))
                        kill(server_pid, SIGUSR1);
                    local_prints("很抱歉,目前 RAM 被过度使用, 请稍候再来\r\n");
                    local_prints("\r\n因为重复连接对本站冲击太大，请您配合，不要重复多次连接");
                    local_prints("\r\n请您先休息10分钟，然后再连接本站，非常感谢！\r\n");

                    /*sleep( 60 ); */
                    local_Net_Sleep(60);
                    shutdown(csock, 2);
                    close(csock);

                    return -1;
                }
                if ((server_pid != -1) && (heavy_load))
                    kill(server_pid, SIGUSR2);
            }
#endif
        }
    } else
        fclose(fp);
#endif                          /* LOAD_LIMIT */

#ifdef BBSRF_CHROOT
    sprintf(bbs_prog_path, "/bin/bbs", BBSHOME);
    if (chroot(BBSHOME) != 0) {
        local_prints("Cannot chroot, exit!\r\n");
        return -1;
    }
#else
    sprintf(bbs_prog_path, "%s/bin/bbs", BBSHOME);
#endif

    fromhost[16] = '\0';
    if (check_ban_IP(fromhost, buf) > 0) {      /* Leeward 98.07.31 */
        local_prints("本站目前不欢迎来自 %s 访问!\r\n原因：%s。\r\n\r\n", fromhost, buf);
        local_Net_Sleep(60);
        shutdown(csock, 2);
        close(csock);

        return -1;
    }

#ifdef HAVE_REVERSE_DNS
	getremotehost(csock, fromhost, sizeof(fromhost));
#endif

#if 0
#ifdef D_TEST
    strcat(bbs_prog_path, "test");
    execl(bbs_prog_path, "bbstest", code, fromhost, NULL);      /*调用BBS */
#else
    strcat(bbs_prog_path, "new");
    execl(bbs_prog_path, "bbsnew", code, fromhost, NULL);       /*调用BBS */
#endif
#endif
    {
        struct stat st;
        struct rlimit rl;

        if (stat("core", &st) == -1) {
            rl.rlim_cur = 120 * 1024 * 1024;
            rl.rlim_max = 200 * 1024 * 1024;
            setrlimit(RLIMIT_CORE, &rl);
        }
    }

    main_bbs(0, argv);
    return -1;
    write(0, "execl failed\r\n", 12);
    return -1;
}

#ifndef SSHBBS

typedef int (*bbs_handler)(char*);

enum bbs_handlers{
  BBS_STANDALONE,
  BBS_INETD,
  BBS_PREFORK,
  BBS_HANDLERS
};

#define FROMHOST(sin) \
  {\
    char *host = (char *) inet_ntoa(sin.sin_addr);\
    strncpy(fromhost, host, IPLEN);\
    fromhost[IPLEN] = 0;\
  }

static int bbs_standalone_main(char* argv)
{
  int csock;                  /* socket for Master and Child */
  int value;
  struct sockaddr_in sin;

#if 0
  for (; listprocess > 0; listprocess--)
    fork();
#endif
  for (;;) {
    value = sizeof(sin);
    csock = accept(0, (struct sockaddr *) &sin, (socklen_t *) & value);
    if (csock < 0) {
      /*                reaper();*/
      continue;
    }
    /* COMMAN :do not fork in debugging mode */
    proxy_getpeername(csock, (struct sockaddr *) &sin, (socklen_t *) & value);
#ifdef SMTH
    if (sin.sin_addr.s_addr==0xea086fa6) { //166.111.8.234
        int buf;
        read(csock,&buf,4);
        if (buf==0x330123) {
            read(csock,&sin.sin_addr.s_addr,4);
        }
    }
#endif

#ifdef CHECK_IP_LINK
    if (check_IP_lists(sin.sin_addr.s_addr)) {
        close(csock);
        continue;
    }
#endif

    if (fork()) {
      close(csock);
      continue;
    }
    /* sanshao@10.24: why next line is originally sizeof(sin) not &value */

    bbslog("0connect", "connect from %s(%d) in port %d", inet_ntoa(sin.sin_addr), htons(sin.sin_port), mport);
    setsid();

    if (csock!=0) {
        close(0);
        dup2(csock, 0);
        close(csock);
    };
    for (csock=1;csock<4;csock++)
    	close(csock);
    break;
  }

  FROMHOST(sin);

  telnet_init();
  return bbs_main(argv);

}

static int bbs_inet_main(char* argv)
{
  struct sockaddr_in sin;
  int sinlen = sizeof(sin);

  getpeername(0, (struct sockaddr *) &sin, (void *) &sinlen);

  FROMHOST(sin);

  telnet_init();
  return bbs_main(argv);
}

extern int bbs_prefork_main(char*);

#undef FROMHOST

bbs_handler handlers[]={
  bbs_standalone_main,
  bbs_inet_main,
#ifdef USING_PREFORK
  bbs_prefork_main,
#endif
};

int main(argc, argv)
    int argc;
    char *argv[];
{

    /* --------------------------------------------------- */
    /* setup standalone daemon                           */
    /* --------------------------------------------------- */

    /* sanshao@2002.8
     * use getopt instead
     * bbsd [-i] [-d] [-p port]
     */
    int mode, port;
    int c;

    mode = BBS_STANDALONE;

    while ((c = getopt(argc, argv, "ifdp:")) != -1){
      switch (c){
      case 'i':
	mode = BBS_INETD;
	break;
      case 'f':
	mode = BBS_PREFORK;
	break;
      case 'd':
	no_fork = 1;
	break;
      case 'p':
	port=atoi(optarg);
	break;
      case '?':
	return -1;
      }
    }

#if 0
    if ((argc <= 1) || !strcmp(argv[1], "-i"))  /*如果只有文件名或者第一个参数是“-i” */
        inetd = 1;              /*则用inetd启动 */
    else if (!strcmp(argv[1], "-d"))
        no_fork = 1;
    /* COMMAN: 如果有 -d 参数则不 fork */
    else {
        if (argc <= 1)
            port = 23;
        else
            port = atoi(argv[1]);
        if (argc <= 2)
            listprocess = 1;
        else
            listprocess = atoi(argv[2]);
    }
#endif
    start_daemon((mode == BBS_INETD), port);
    main_signals();

    /* --------------------------------------------------- */
    /* attach shared memory & semaphore                  */
    /* --------------------------------------------------- */

    server_pid = getpid();
    return (*handlers[mode])(argv[0]);


    /*
       whee = gethostbyaddr((char*)&sin.sin_addr.s_addr,sizeof(struct in_addr),AF_INET);
       if ((whee)&&(whee->h_name[0]))
       strncpy(hid, whee->h_name, 17) ;
       else
       KCN temp change it for trace IP!! don't remove. 2000.8.19 */
}
#else
void ssh_exit()
{
    system_abort();
    packet_disconnect("sshbbsd exit");
}
extern char **saved_argv;
int bbs_entry(void)
{
    /* 本函数供 SSH 使用 */
    int sinlen;
    struct sockaddr_in sin;

    setuid(BBSUID);
    setgid(BBSGID);
    main_signals();
    sinlen = sizeof(struct sockaddr_in);
    atexit(ssh_exit);
    proxy_getpeername(0, (struct sockaddr *) &sin, (void *) &sinlen);
    {
        char *host = (char *) inet_ntoa(sin.sin_addr);

        strncpy(fromhost, host, IPLEN);
        fromhost[IPLEN] = 0;
    }
    return bbs_main(saved_argv[0]);
}
#endif
