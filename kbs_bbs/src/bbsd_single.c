#include "bbs.h"
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/telnet.h>
#include <sys/resource.h>
#include <varargs.h>

#ifdef LINUX
#include <arpa/inet.h>
#endif

#define	QLEN		50
#define	PID_FILE	"reclog/bbs.pid"
#define	LOG_FILE	"reclog/bbs.log"

#define MAXPORTS        4
#ifdef D_TEST
static int myports[MAXPORTS] = {49457/* , 3456, 3001, 3002, 3003 */ };
#else
static int myports[MAXPORTS] = {23,8000,8001,8002/* , 3456, 3001, 3002, 3003 */ };
#endif
static int mport;
int csock;                      /* socket for Master and Child */

int max_load = 79; /* 原值39 , modified by KCN,1999.09.07 */
/* 59->79 , modified by dong , 1999.9.7 */

/* 原值29, modified by dong, 1998.11.23 */
#undef LOAD_LIMIT	
int heavy_load;
int server_pid;

char code[3];
void
cat(filename,msg)
char *filename,*msg;
{
	/* 将msg放到以filename为名的文件中*/
    FILE*fp;

    if( (fp = fopen( filename, "a" )) != NULL ) {
        fputs( msg, fp );
        fclose( fp );
    }
}

int local_Net_Sleep(time)
time_t time;
{
	/*
	time秒内 如果 端口csock 有数据就读入然后丢弃。 
	*/
    struct timeval tv ;
    int     sr;
    fd_set fd,efd;

    tv.tv_sec = time;
    tv.tv_usec = 0;
    FD_ZERO(&fd);
    FD_ZERO(&efd);
    FD_SET(csock,&fd);
    FD_SET(csock,&efd);

    while((sr=select(csock+1,&fd,NULL,&efd,&tv))>0) {
		/* Select() returns the total number of ready        
		   descriptors in all the sets.    
		*/
        char buf[256];
        if (FD_ISSET(csock,&efd))
            break;
        if (recv(csock,buf,256,0)<=0) break;
        tv.tv_sec = time;
        tv.tv_usec = 0;
        FD_SET(csock,&fd);
        FD_SET(csock,&efd);
    };
    return 0;
}


void
local_prints(va_alist)
va_dcl
{
    va_list args;
    char buf[512], *fmt;

    va_start(args);
    fmt = va_arg(args, char *);
    vsprintf(buf, fmt, args);
    va_end(args);

#ifdef SSHBBS
    ssh_write(0,buf,strlen(buf));
#else
    write(0,buf,strlen(buf));
#endif
}

#ifdef LOAD_LIMIT
#ifdef AIX
#include <rpcsvc/rstat.h>
#endif
void
get_load( load )
/*取系统负载*/
double load[];
{
#ifdef LINUX
    FILE *fp;
    fp = fopen ("/proc/loadavg", "r");
    if (!fp) load[0] = load[1] = load[2] = 0;
    else {
        float av[3];
        fscanf (fp, "%g %g %g", av, av + 1, av + 2);
        fclose (fp);
        load[0] = av[0]; load[1] = av[1]; load[2] = av[2];
    }
#else
#ifdef BSD44
getloadavg( load, 3 );
#else
    struct statstime rs;
    rstat( "localhost", &rs );
    load[ 0 ] = rs.avenrun[ 0 ] / (double) (1 << 8);
    load[ 1 ] = rs.avenrun[ 1 ] / (double) (1 << 8);
    load[ 2 ] = rs.avenrun[ 2 ] / (double) (1 << 8);
#endif
#endif
}
#endif

int local_check_ban_IP(ip, buf)
char *ip;
char *buf;
{ /* Leeward 98.07.31
      RETURN:         
                     - 1: No any banned IP is defined now 
                       0: The checked IP is not banned
      other value over 0: The checked IP is banned, the reason is put in buf
      */
    FILE *Ban = fopen(".badIP", "r");
    char IPBan[64];
    int  IPX = - 1;
    char *ptr;

    if (!Ban)
        return IPX;
    else
        IPX ++;

    while (fgets(IPBan, 64, Ban))
    {
        if (ptr = strchr(IPBan, '\n'))
            *ptr = 0;
        if (ptr = strchr(IPBan, ' '))
        {
            *ptr ++ = 0;
            strcpy(buf, ptr);
        }
        IPX = strlen(IPBan);
        if (!strncmp(ip, IPBan, IPX))
            break;
        IPX = 0;
    }

    fclose(Ban);
    return IPX;
}

static void
telnet_init()
{
    int n, len;
    char *cmd;
    int rset;
    struct timeval to;
    char buf[64];
    char svr[] = {
        IAC, DO, TELOPT_TTYPE,
        IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE,
        IAC, WILL, TELOPT_ECHO,
        IAC, WILL, TELOPT_SGA
    };


    /* --------------------------------------------------- */
    /* init telnet protocol				 */
    /* --------------------------------------------------- */

#ifndef	LINUX
    to.tv_sec = 1;
    to.tv_usec = 1;
#endif
    cmd = svr;

    for (n = 0; n < 4; n++)
    {
        len = (n == 1 ? 6 : 3);
        send(0, cmd, len, 0);
        cmd += len;
        /* KCN !!!!!!!!!! wrong telnet
        #ifdef	LINUX
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


static void
start_daemon(inetd, port)
int inetd;
int port; /* Thor.981206: 取 0 代表 *没有参数* */
{
    int n;
    struct sockaddr_in sin;
    struct rlimit rl;
    char buf[80], data[80];
    time_t val;
    FILE* lock_pid;

    /*
     * More idiot speed-hacking --- the first time conversion makes the C
     * library open the files containing the locale definition and time zone.
     * If this hasn't happened in the parent process, it happens in the
     * children, once per connection --- and it does add up.
     */

    time(&val);
    strftime(buf, 80, "%d/%b/%Y %H:%M:%S", localtime(&val));

    /* --------------------------------------------------- */
    /* detach daemon process				 */
    /* --------------------------------------------------- */

    close(1);
    close(2);
		/*
		close file descriptor 1 and 2
		*/
    chdir(BBSHOME);	/*将当前目录转换到BBSHOME*/
    umask(07);

    if(inetd) /* Thor.981206: inetd -i */
    {
        /* Give up root privileges: no way back from here	 */
        server_pid=0;
        setgid(BBSGID);	/* setgid sets the effective group ID of the current process.*/
        setuid(BBSUID);	/* sets the effective user ID of the current process.*/
        setreuid(BBSUID,BBSUID);	/* setreuid  sets real and effective user ID's 
									of the current process. */
        setregid(BBSGID,BBSGID);	/* setregid sets real and effective group ID's 
									of the current process. */		
        mport = port;
        if (port) strcpy(code,"e");
        else strcpy(code,"d");		/*没有参数的话将code置'd'*/
        /*    sprintf(data, "%d\tinetd -i\n", getpid() );
            cat(PID_FILE, data);
        */
        return;
    }

    sprintf(buf,"bbsd start at %s",ctime(&val));
    cat(PID_FILE, buf);

    close(0);

    if (fork())
        exit(0);

    setsid();

    if (fork())
        exit(0); 

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
/*    sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);*/

    /*  if (port <= 0)  Thor.981206: port 0 代表没有参数
      {
        n = MAXPORTS - 1;
        while (n)
        {
          if (fork() == 0)
    	break;

          sleep(1);
          n--;
        }
        port = myports[n];
      }
    */

    n = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    val = 1;
    setsockopt(n, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(val));

#if 0
    setsockopt(n, SOL_SOCKET, SO_KEEPALIVE, (char *) &val, sizeof(val));

    setsockopt(n, IPPROTO_TCP, TCP_NODELAY, (char *) &val, sizeof(val));
#endif

    /* --------------------------------------------------- */
    /* Give up root privileges: no way back from here	 */
    /* --------------------------------------------------- */


    mport = port;
    if (port==6001) strcpy(code,"e");
    else strcpy(code,"d");
    sin.sin_port = htons(port);
    if ((bind(n, (struct sockaddr *)&sin, sizeof(sin)) < 0) || (listen(n, QLEN) < 0)) {
        cat(PID_FILE,strerror(errno));
        exit(1);
    }

    setgid(BBSGID);
    setuid(BBSUID);

    sprintf(buf,"reclog/bbsd.pid.%d",port);
    if ((lock_pid=fopen(buf,"w+"))==NULL) {
        cat(PID_FILE,strerror(errno));
        exit(0);
    }
    /*
      flock(lock_pid,LOCK_EX);
    */
    fprintf(lock_pid,"%d\n",getpid());
    fclose(lock_pid);
}


/* ----------------------------------------------------- */
/* reaper - clean up zombie children			 */
/* ----------------------------------------------------- */


static void
siguser1(int signo)
{
    heavy_load=1;
}
static void
siguser2(int signo)
{
    heavy_load=0;
}

static void
reaper(int signo)
{
    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}

static void
main_term(int signo)
{
    exit(0);
}

static void
main_signals()
{
    struct sigaction act;

    /* act.sa_mask = 0; */ /* Thor.981105: 标准用法 */
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    act.sa_flags = SA_RESTART;
    act.sa_handler = main_term;
    sigaction(SIGTERM, &act, NULL);

    act.sa_handler = siguser1;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_handler = siguser2;
    sigaction(SIGUSR2, &act, NULL);

    act.sa_handler=SIG_IGN;
    sigaction(SIGPIPE, &act, NULL);

    act.sa_handler = SIG_IGN;
    sigaction(SIGTTOU, &act, NULL);

    act.sa_handler = SIG_IGN;
    sigaction(SIGHUP, &act, NULL);

    /*  act.sa_handler = reaper; */
#ifdef AIX
    act.sa_handler = NULL;
    act.sa_flags = SA_RESTART|SA_NOCLDWAIT;
    sigaction(SIGCHLD, &act, NULL);
#else
    act.sa_handler = reaper;
    act.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &act, NULL);
#endif
    /* COMMAN: Ignore other signals used in bbs, to avoid group killing(pid=0)
    */

}

int
bbs_main(argv)
char* argv;
{
    int         uid;
    char        buf[256]; /* Leeward 98.07.31 */
    char bbs_prog_path[ 256 ];
    FILE        *fp;

    /*    modified by period      2000-11-13      allow localhost anyway  */
    /*    if((fp = fopen("NOLOGIN","r")) != NULL) */
#ifndef DEBUG
    if(strcmp(fromhost,"0.0.0.0") && strcmp(fromhost, "127.0.0.1") && (fp = fopen("NOLOGIN","r")) != NULL)
    {
        while(fgets(buf,256,fp) != NULL)
            local_prints(buf);
        fclose(fp);
        local_Net_Sleep(20);
        shutdown(csock,2);
        close(csock);
        exit(-1);
    }

#endif
#ifdef LOAD_LIMIT

    if ((fp = fopen("NO_LOAD_LIMIT", "r")) == NULL){  /* add by dong , 1999, 9, 10 */
        /* load control for BBS */
        {
            double      cpu_load[ 3 ];
            double         load ;

            get_load( cpu_load );
            load = cpu_load[ 0 ];
            local_prints("CPU 最近 (1,10,15) 分钟的平均负荷分别为 %.2f, %.2f, %.2f (目前上限 = %d)\r\n", cpu_load[ 0 ], cpu_load[ 1 ], cpu_load[ 2 ], max_load );
            if ( load < 0 || load > max_load ) {
                local_prints("很抱歉,目前 CPU 负荷过重, 请稍候再来\r\n");
                local_prints("\r\n因为重复连接对本站冲击太大，请您配合，不要重复多次连接");
                local_prints("\r\n请您先休息10分钟，然后再连接本站，非常感谢！\r\n");
                /*sleep( load ); */
                /*sleep(1);  modified by dong, 1999.9.10 */
                local_Net_Sleep( load );
                shutdown(csock,2);
                close(csock);

                exit( -1 );
            }
#ifdef AIX
            { /* Leeward 98.06.11 For AIX only. Please man psdanger */
                int free = psdanger(-1);
                int safe = psdanger(SIGDANGER);
                int danger = 125000;
                local_prints("RAM 当前空闲页数高出警戒阈值 %d (警戒阈值 = %d)\r\n\r\n",
                             safe, free - safe);

                if ( safe < danger/*Haohmaru.99.06.16.rem free - safe*/ ) {
                    if ((server_pid!=-1)&&(!heavy_load))
                        kill(server_pid,SIGUSR1);
                    local_prints("很抱歉,目前 RAM 被过度使用, 请稍候再来\r\n");
                    local_prints("\r\n因为重复连接对本站冲击太大，请您配合，不要重复多次连接");
                    local_prints("\r\n请您先休息10分钟，然后再连接本站，非常感谢！\r\n");

                    /*sleep( 60 ); */
                    local_Net_Sleep( 60 );
                    shutdown(csock,2);
                    close(csock);

                    exit( -1 );
                }
                if ((server_pid!=-1)&&(heavy_load))
                    kill(server_pid,SIGUSR2);
            }
#endif
        }
    }
    else
        fclose(fp);
#endif	/* LOAD_LIMIT */

#ifdef BBSRF_CHROOT
    sprintf( bbs_prog_path, "/bin/bbs", BBSHOME );
    if (chroot(BBSHOME) != 0) {
        local_prints("Cannot chroot, exit!\r\n");
        exit( -1 );
    }
#else
    sprintf( bbs_prog_path, "%s/bin/bbs", BBSHOME );
#endif

    if (local_check_ban_IP(fromhost, buf) > 0) /* Leeward 98.07.31 */
    {
        local_prints("本站目前不欢迎来自 %s 访问!\r\n原因：%s。\r\n\r\n", fromhost, buf);
        local_Net_Sleep( 60 );
        shutdown(csock,2);
        close(csock);

        exit(-1);
    }


    fromhost[16] = '\0' ;
#if 0
#ifdef D_TEST
    strcat(bbs_prog_path, "test");
    execl( bbs_prog_path,"bbstest",code, fromhost,  NULL) ; /*调用BBS*/
#else
    strcat(bbs_prog_path, "new");
    execl( bbs_prog_path,"bbsnew",code, fromhost,  NULL) ; /*调用BBS*/
#endif
#endif
    {
        struct stat st;
	struct rlimit rl;

        if (stat("core",&st)==-1) {
            rl.rlim_cur=80*1024*1024;
            rl.rlim_max=200*1024*1024;
            setrlimit(RLIMIT_CORE,&rl);
	}
    }

    main_bbs(0,argv);
    exit(-1);
    write(0,"execl failed\r\n",12);
    exit( -1 );
}
#ifndef SSHBBS
int
main(argc, argv)
int argc;
char *argv[];
{
    int csock;			/* socket for Master and Child */
    int *totaluser;
    int value;
    struct sockaddr_in sin;
    struct hostent * whee;

    /* --------------------------------------------------- */
    /* setup standalone daemon				 */
    /* --------------------------------------------------- */

    /* start_daemon(argc > 1 ? atoi(argv[1]) : 0); */
    /* Thor.981206: 取 0 代表 *没有参数* */
    /* start_daemon(argc > 1 ? atoi(argv[1]) : -1); */

    /* Thor.981207: usage,  bbsd, or bbsd 1234, or bbsd -i 1234 */
    /*  start_daemon(argc > 2, atoi(argv[argc-1]));
     KCN change it for not port parm */
    int inetd,port,listprocess;

    inetd = 0;
    if ((argc<=1) || !strcmp(argv[1],"-i") )  /*如果只有文件名或者第一个参数是“-i”*/
       inetd=1;                               /*则用inetd启动 */
    else {
       if (argc<=1) port = 23;
       else
         port = atoi(argv[1]);
       if (argc<=2) listprocess = 1;
       else
         listprocess = atoi(argv[2]);
    }
    start_daemon(inetd,port);
    main_signals();

    /* --------------------------------------------------- */
    /* attach shared memory & semaphore			 */
    /* --------------------------------------------------- */

    server_pid=getpid();
    if (!inetd) {
        for (;listprocess>0;listprocess--)
		fork();
        for (;;)
        {
/*
            value = 1;
            if (select(1, (fd_set *) & value, NULL, NULL, NULL) < 0)
                continue;
*/
            value = sizeof(sin);
            csock = accept(0, (struct sockaddr *)&sin, (socklen_t*)&value);
            if (csock < 0)
            {
/*                reaper();*/
                continue;
            }

            if (fork())
            {
                close(csock);
                continue;
            }
	    
	    /* sanshao@10.24: why next line is originally sizeof(sin) not &value */	
            getpeername(csock,(struct sockaddr*)&sin,(socklen_t*)&value);
            bbslog("0connect","connect from %s(%d) in port %d",inet_ntoa(sin.sin_addr),htons(sin.sin_port),port);
            setsid();

            close(0);

            dup2(csock, 0);
            /* COMMAN: 有人说不处理1和2号文件句柄会把stderr and stdout打进文件弄坏PASSWD之类
            想想挺有道理的说，不过为什么以前税目没有碰上过呢....
            笨蛋COMMAN:这个当然是因为stderr,stdout都被全部检查过，不会写入，唯一的可能
            发生的情况是在system调用。*/
            /* COMMAN end */
            close(csock);
            break;
        }
    }
    else {
        int sinlen = sizeof (struct sockaddr_in);
        getpeername (0,(struct sockaddr *) &sin,(void *) &sinlen);
    }

    /*
      whee = gethostbyaddr((char*)&sin.sin_addr.s_addr,sizeof(struct in_addr),AF_INET);
        if ((whee)&&(whee->h_name[0]))
          strncpy(hid, whee->h_name, 17) ;
        else
     KCN temp change it for trace IP!! don't remove. 2000.8.19*/
    {
        char*host = (char*)inet_ntoa(sin.sin_addr);
        strncpy(fromhost,host,IPLEN);
        fromhost[IPLEN]=0;
    }
    telnet_init();
    bbs_main(argv[0]);

}
#else
void ssh_exit()
{
    packet_disconnect("sshd exit");
}

int bbs_entry(void)
{
   /* 本函数供 SSH 使用 */
   int sinlen;
   struct sockaddr_in sin;
   char faint[99];
   setuid(BBSUID);
   setgid(BBSGID);
   main_signals();
   sinlen = sizeof(struct sockaddr_in);
   atexit(ssh_exit);
   getpeername (0,(struct sockaddr *) &sin,(void *) &sinlen);
   {
        char*host = (char*)inet_ntoa(sin.sin_addr);
        strncpy(fromhost,host,IPLEN);
        fromhost[IPLEN]=0;
   } 
   bbs_main(faint);
}
#endif
