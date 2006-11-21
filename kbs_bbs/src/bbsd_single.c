/* etnlegend, 2006.04.25, clean up bbsd_single ... */

#include "bbs.h"
#include <arpa/telnet.h>
#include <sys/resource.h>

#ifdef HAVE_REVERSE_DNS
#include <netdb.h>
#endif /* HAVE_REVERSE_DNS */

#undef LOAD_LIMIT /* Temporarily Disable All Load Limit Detection */

#if defined(LOAD_LIMIT) && defined(AIX)
#include <rpcsvc/rstat.h>
#endif /* LOAD_LIMIT && AIX */

#define SOCKFD                          3           /* listen sock file descriptor, should be set to 3 for consistent ! */
#define MAX_PENDING_CONNECTIONS         50
#define MAXLIST                         1000
#define CON_THRESHOLD                   (5.0/18)    /* (1000.0/3600) */
#define CON_THRESHOLD2                  1.0

#ifdef HAVE_IPV6_SMTH
#define KBS_SIN_MEMBER(_sin,_member)    _sin.sin6_##_member
#define KBS_SIN_FAMILY                  AF_INET6
#define KBS_SIN_ADDR_DEFAULT            in6addr_any
struct ip_struct{           /* size on 32-bit / 64-bit machine */
    struct in6_addr ip;     /*           16       16           */
    time_t first;           /*            4        8           */
    time_t last;            /*            4        8           */
    int t;                  /*            4        4           */
};                          /*           28       36   bytes   */
typedef struct sockaddr_in6             KBS_SOCKADDR_IN;
typedef struct in6_addr                 KBS_IN_ADDR;
#ifdef LEGACY_IPV4_DISPLAY
#define KBS_SET_FROMHOST(_sin,_from)                                            \
    (                                                                           \
        !ISV4ADDR(KBS_SIN_MEMBER(_sin,addr))?                                   \
        inet_ntop(AF_INET6,&KBS_SIN_MEMBER(_sin,addr),_from,IPLEN):             \
        inet_ntop(AF_INET,&KBS_SIN_MEMBER(_sin,addr).s6_addr[12],_from,IPLEN)   \
    )
#endif /* LEGACY_IPV4_DISPLAY */
#else /* ! HAVE_IPV6_SMTH */
#define KBS_SIN_MEMBER(_sin,_member)    _sin.sin_##_member
#define KBS_SIN_FAMILY                  AF_INET
#define KBS_SIN_ADDR_DEFAULT            inaddr_any
struct ip_struct{           /* size on 32-bit / 64-bit machine */
    unsigned char ip[4];    /*            4        4           */
    int t;                  /*            4        4           */
    time_t first;           /*            4        8           */
    time_t last;            /*            4        8           */
};                          /*           16       24   bytes   */
typedef struct sockaddr_in              KBS_SOCKADDR_IN;
typedef struct in_addr                  KBS_IN_ADDR;
#endif /* HAVE_IPV6_SMTH */

#define KBS_SET_SIN_FAMILY(_sin)        do{KBS_SIN_MEMBER(_sin,family)=KBS_SIN_FAMILY;}while(0)
#define KBS_SET_SIN_PORT(_sin,_port)    do{KBS_SIN_MEMBER(_sin,port)=htons(_port);}while(0)
#define KBS_SET_SIN_ADDR(_sin,_addr)    do{KBS_SIN_MEMBER(_sin,addr)=_addr;}while(0)
#ifndef KBS_SET_FROMHOST
#define KBS_SET_FROMHOST(_sin,_from)    inet_ntop(KBS_SIN_FAMILY,&KBS_SIN_MEMBER(_sin,addr),_from,IPLEN)
#endif /* KBS_SET_FROMHOST */

#ifndef SSHBBS
#define KBS_WRITE(_fd,_ptr,_len)        write(_fd,_ptr,_len)
static const unsigned char cmd[]={
    IAC,DO,TELOPT_TTYPE,                        /* cmd 0 size = 3  */
    IAC,SB,TELOPT_TTYPE,TELQUAL_SEND,IAC,SE,    /* cmd 1 size = 6  */
    IAC,WILL,TELOPT_ECHO,                       /* cmd 2 size = 3  */
    IAC,WILL,TELOPT_SGA,                        /* cmd 3 size = 3  */
    IAC,WILL,TELOPT_BINARY,                     /* cmd 4 size = 3  */
    IAC,DO,TELOPT_NAWS,                         /* cmd 5 size = 3  */
    IAC,DO,TELOPT_BINARY                        /* cmd 6 size = 3  */
};                                              /* total size = 24 */
#ifndef HAVE_IPV6_SMTH                                                                                                                     
static struct in_addr inaddr_any;
#endif /* HAVE_IPV6_SMTH */
static int mport;
static int no_fork;
static int server_pid;
const select_func x_select=select;
const read_func x_read=read;
#else /* SSHBBS */
#include "ssh_funcs.h"
#define KBS_WRITE(_fd,_ptr,_len)        ssh_write(_fd,_ptr,_len)
extern char **saved_argv;
static int ssh_exiting;
const select_func x_select=ssh_select;
const read_func x_read=ssh_read;
#endif /* ! SSHBBS */

static const int max_load = 79;
static int heavy_load;
static int initIP;
static struct ip_struct *ips;
static struct ip_struct *bads;
static struct ip_struct *proxies;

static inline int proxy_getpeername(int sockfd,struct sockaddr *addr,socklen_t *len){
    return getpeername(sockfd,addr,len);
}
static inline int local_Net_Sleep(time_t time){
    fd_set fds,efds;
    struct timeval tv;
    char buf[256];
    FD_ZERO(&fds);FD_ZERO(&efds);
    FD_SET(0,&fds);FD_SET(0,&efds);
    for(tv.tv_sec=time,tv.tv_usec=0;select(1,&fds,NULL,&efds,&tv)>0;tv.tv_sec=time,tv.tv_usec=0){
        if(FD_ISSET(0,&efds)||!(recv(0,buf,256,0)>0))
            break;
        FD_SET(0,&fds);FD_SET(0,&efds);
    }
    return 0;
}
static inline int local_prints(const char *fmt,...){
    va_list ap;
    char buf[1024];
    va_start(ap,fmt);
    vsprintf(buf,fmt,ap);
    va_end(ap);
    return KBS_WRITE(0,buf,strlen(buf));
}
#ifdef LOAD_LIMIT
static inline int get_load(double *load){
#if defined(HAVE_GETLOADAVG)
    return getloadavg(load,3);
#elif defined(LINUX) /* ! HAVE_GETLOADAVG && LINUX */
    FILE *fp;
    int ret;
    double avg[3];
    load[0]=0;
    load[1]=0;
    load[2]=0;
    if(!(fp=fopen("/proc/loadavg","r")))
        return 0;
    ret=fscanf(fp,"%g %g %g",&avg[0],&avg[1],&avg[2]);
    fclose(fp);
    switch(ret){
        case 3:
            load[2]=avg[2];
        case 2:
            load[1]=avg[1];
        case 1:
            load[0]=avg[0];
            break;
        default:
            return -1;
    }
    return ret;
#else /* ! HAVE_GETLOADAVG && ! LINUX */
    struct statstime rsts;
    rstat("localhost",&rsts);
    load[0]=(rs.avenrun[0]*0.00390625); /* div by 256 */
    load[1]=(rs.avenrun[1]*0.00390625);
    load[2]=(rs.avenrun[2]*0.00390625);
#endif /* HAVE_GETLOADAVG */
}
#endif /* LOAD_LIMIT */
static void sig_user1(int sig){
    heavy_load=1;
    return;
}
static void sig_user2(int sig){
    heavy_load=0;
    return;
}
static void sig_reaper(int sig){
    while(waitpid(-1,NULL,(WNOHANG|WUNTRACED))>0)
        continue;
    return;
}
static void sig_mainterm(int sig){
    exit(0);
}
static int main_signals(void){
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    act.sa_flags=SA_RESTART;
    act.sa_handler=sig_mainterm;
    sigaction(SIGTERM,&act,NULL);
    act.sa_handler=sig_user1;
    sigaction(SIGUSR1,&act,NULL);
    act.sa_handler=sig_user2;
    sigaction(SIGUSR2,&act,NULL);
    act.sa_handler=SIG_IGN;
    sigaction(SIGPIPE,&act,NULL);
    act.sa_handler=SIG_IGN;
    sigaction(SIGTTOU,&act,NULL);
    act.sa_handler=SIG_IGN;
    sigaction(SIGHUP,&act,NULL);
#ifndef AIX
    act.sa_handler=sig_reaper;
    act.sa_flags=SA_RESTART;
#else /* AIX */
    act.sa_handler=NULL;
    act.sa_flags=(SA_RESTART|SA_NOCLDWAIT);
#endif /* ! AIX */
    sigaction(SIGCHLD,&act,NULL);
    return 0;
}
#ifdef HAVE_REVERSE_DNS
static void dns_query_timeout(int sig){
    longjmp(byebye,sig);
}
static void getremotehost(char *host,size_t len){
    KBS_SOCKADDR_IN sin;
    socklen_t sinlen;
    struct hostent *hp;
    char buf[128],*p;
    sinlen=sizeof(KBS_SOCKADDR_IN);
    proxy_getpeername(0,(struct sockaddr*)&sin,&sinlen);
    if(!setjmp(byebye)){
        signal(SIGALRM,dns_query_timeout);
        alarm(5);
        hp=gethostbyaddr(&(KBS_SIN_MEMBER(sin,addr)),sizeof(KBS_SIN_MEMBER(sin,addr)),KBS_SIN_MEMBER(sin,family));
        alarm(0);
    }
    if(hp
#ifdef HAVE_IPV6_SMTH
        &&!strchr(hp->h_name,':')
#endif /* HAVE_IPV6_SMTH */
        )
        snprintf(buf,128,"%s",hp->h_name);
    else
        KBS_SET_FROMHOST(sin,buf);
    if((p=strstr(buf,"."NAME_BBS_ENGLISH)))
        *p=0;
    snprintf(host,len,"%s",buf);
    return;
}
#endif /* HAVE_REVERSE_DNS */
int check_IP_lists(
#ifndef HAVE_IPV6_SMTH
    unsigned int IP2
#else /* HAVE_IPV6_SMTH */
    struct in6_addr sip
#endif /* ! HAVE_IPV6_SMTH */
){
    FILE *fp;
    char buf[1024];
    int i,found,min,ret;
    time_t now;
#ifndef HAVE_IPV6_SMTH
    unsigned int ip[4];
#else /* HAVE_IPV6_SMTH */
    struct in6_addr rip;
#endif /* ! HAVE_IPV6_SMTH */
    found=0;min=0;ret=0;
    if(!initIP){
        ips=(struct ip_struct*)malloc(MAXLIST*sizeof(struct ip_struct));
        bads=(struct ip_struct*)malloc(MAXLIST*sizeof(struct ip_struct));
        proxies=(struct ip_struct*)malloc(MAXLIST*sizeof(struct ip_struct));
        memset(ips,0,MAXLIST*sizeof(struct ip_struct));
        memset(bads,0,MAXLIST*sizeof(struct ip_struct));
        memset(proxies,0,MAXLIST*sizeof(struct ip_struct));
        if(!ips||!bads||!proxies)
            return -1;
        if((fp=fopen(".denyIP","r"))){
            for(i=0;fgets(buf,1024,fp);i++){
#ifndef HAVE_IPV6_SMTH
                if(!(sscanf(buf,"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3])>0))
                    break;
                bads[i].ip[0]=ip[0];
                bads[i].ip[1]=ip[1];
                bads[i].ip[2]=ip[2];
                bads[i].ip[3]=ip[3];
#else /* HAVE_IPV6_SMTH */
                if(!(sscanf(buf,"%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:"
                    "%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX",&rip.s6_addr[0],
                    &rip.s6_addr[1],&rip.s6_addr[2],&rip.s6_addr[3],&rip.s6_addr[4],&rip.s6_addr[5],
                    &rip.s6_addr[6],&rip.s6_addr[7],&rip.s6_addr[8],&rip.s6_addr[9],&rip.s6_addr[10],
                    &rip.s6_addr[11],&rip.s6_addr[12],&rip.s6_addr[13],&rip.s6_addr[14],&rip.s6_addr[15])>0))
                    break;
                ip_cpy(bads[i].ip,rip);
#endif /* ! HAVE_IPV6_SMTH */
            }
            fclose(fp);
        }
        if((fp=fopen("etc/proxyIP","r"))){
            for(i=0;fgets(buf,1024,fp);i++){
#ifndef HAVE_IPV6_SMTH
                if(!(sscanf(buf,"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3])>0))
                    break;
                proxies[i].ip[0]=ip[0];
                proxies[i].ip[1]=ip[1];
                proxies[i].ip[2]=ip[2];
                proxies[i].ip[3]=ip[3];
#else /* HAVE_IPV6_SMTH */
                if(!(sscanf(buf,"%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:"
                    "%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX",&rip.s6_addr[0],
                    &rip.s6_addr[1],&rip.s6_addr[2],&rip.s6_addr[3],&rip.s6_addr[4],&rip.s6_addr[5],
                    &rip.s6_addr[6],&rip.s6_addr[7],&rip.s6_addr[8],&rip.s6_addr[9],&rip.s6_addr[10],
                    &rip.s6_addr[11],&rip.s6_addr[12],&rip.s6_addr[13],&rip.s6_addr[14],&rip.s6_addr[15])>0))
                    break;
                ip_cpy(proxies[i].ip,rip);
#endif /* ! HAVE_IPV6_SMTH */
            }
            fclose(fp);
        }
        initIP=1;
    }
    now=time(NULL);
#ifndef HAVE_IPV6_SMTH
    if(!(ip[0]=(IP2&0xFF)))
        return 0;
    ip[1]=((IP2>>8)&0xFF);
    ip[2]=((IP2>>16)&0xFF);
    ip[3]=((IP2>>24)&0xFF);
    for(i=0;i<MAXLIST;i++){
        if(!(bads[i].ip[0]))
            break;
        if((ip[0]==bads[i].ip[0])&&(ip[1]==bads[i].ip[1])
            &&(ip[2]==bads[i].ip[2])&&(ip[3]==bads[i].ip[3]))
            return 1;
    }
    for(i=0;i<MAXLIST;i++){
        if(!(proxies[i].ip[0]))
            break;
        if((ip[0]==proxies[i].ip[0])&&(ip[1]==proxies[i].ip[1])
            &&(ip[2]==proxies[i].ip[2])&&(ip[3]==proxies[i].ip[3]))
            return 0;
    }
    for(i=0;i<MAXLIST;i++){
        if((double)(now-ips[i].last)>3600)
            ips[i].ip[0]=0;
        if((ip[0]==ips[i].ip[0])&&(ip[1]==ips[i].ip[1])
            &&(ip[2]==ips[i].ip[2])&&(ip[3]==ips[i].ip[3])){
            if(!((double)(now-ips[i].last)>CON_THRESHOLD2)){
                if((fp=fopen(".IPdenys","a"))){
                    fprintf(fp,"0 %ld %d.%d.%d.%d %d\n",now,ip[0],ip[1],ip[2],ip[3],ips[i].t);
                    fclose(fp);
                }
                ret=1;
            }
            found=1;
            ips[i].last=now;
            ips[i].t++;
            if(!(ips[i].t<10)&&!((ips[i].t/(double)(ips[i].last-ips[i].first))<CON_THRESHOLD)){
                ips[i].t=100000;
                if((fp=fopen(".IPdenys","a"))){
                    fprintf(fp,"1 %ld %d.%d.%d.%d %d\n",now,ip[0],ip[1],ip[2],ip[3],ips[i].t);
                    fclose(fp);
                }
                ret=1;
            }
            break;
        }
        if(ips[i].last<ips[min].last)
            min=i;
    }
    if(!found){
        ips[min].ip[0]=ip[0];
        ips[min].ip[1]=ip[1];
        ips[min].ip[2]=ip[2];
        ips[min].ip[3]=ip[3];
        ips[min].first=now;
        ips[min].last=now;
        ips[min].t=1;
    }
#else /* HAVE_IPV6_SMTH */
    memset(&rip,0,sizeof(struct in6_addr));
    for(i=0;i<MAXLIST;i++){
        if(!ip_cmp(rip,bads[i].ip))
            break;
        if(!ip_cmp(sip,bads[i].ip))
            return 1;
    }
    for(i=0;i<MAXLIST;i++){
        if(!ip_cmp(rip,proxies[i].ip))
            break;
        if(!ip_cmp(sip,proxies[i].ip))
            return 0;
    }
    for(i=0;i<MAXLIST;i++){
        if((double)(now-ips[i].last)>3600)
            memset(&ips[i].ip,0,sizeof(struct in6_addr));
        if(!ip_cmp(ips[i].ip,sip)){
            if(!((double)(now-ips[i].last)>CON_THRESHOLD2)){
                if((fp=fopen(".IPdenys","a"))){
                    fprintf(fp,"0 %ld %02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:"
                        "%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX %d\n",now,rip.s6_addr[0],
                        rip.s6_addr[1],rip.s6_addr[2],rip.s6_addr[3],rip.s6_addr[4],rip.s6_addr[5],
                        rip.s6_addr[6],rip.s6_addr[7],rip.s6_addr[8],rip.s6_addr[9],rip.s6_addr[10],
                        rip.s6_addr[11],rip.s6_addr[12],rip.s6_addr[13],rip.s6_addr[14],rip.s6_addr[15],ips[i].t);
                    fclose(fp);
                }
                ret=1;
            }
            found=1;
            ips[i].last=now;
            ips[i].t++;
            if(!(ips[i].t<10)&&!((ips[i].t/(double)(ips[i].last-ips[i].first))<CON_THRESHOLD)){
                ips[i].t=100000;
                if((fp=fopen(".IPdenys","a"))){
                    fprintf(fp,"1 %ld %02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:"
                        "%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX:%02hhX%02hhX %d\n",now,rip.s6_addr[0],
                        rip.s6_addr[1],rip.s6_addr[2],rip.s6_addr[3],rip.s6_addr[4],rip.s6_addr[5],
                        rip.s6_addr[6],rip.s6_addr[7],rip.s6_addr[8],rip.s6_addr[9],rip.s6_addr[10],
                        rip.s6_addr[11],rip.s6_addr[12],rip.s6_addr[13],rip.s6_addr[14],rip.s6_addr[15],ips[i].t);
                    fclose(fp);
                }
                ret=1;
            }
            break;
        }
        if(ips[i].last<ips[min].last)
            min=i;
    }
    if(!found){
        ip_cpy(ips[min].ip,rip);
        ips[min].first=now;
        ips[min].last=now;
        ips[min].t=1;
    }
#endif /* ! HAVE_IPV6_SMTH */
    return ret;
}
static int bbs_main(char *argv){
#define BBS_MAIN_EXIT(time)     do{local_Net_Sleep(time);shutdown(0,2);close(0);return -1;}while(0)
    FILE *fp;
    struct stat st;
    struct rlimit rl;
    char buf[256];
#ifndef DEBUG
    if(strcmp(getSession()->fromhost,"0.0.0.0")&&strcmp(getSession()->fromhost,"127.0.0.1")
        &&((fp=fopen("NOLOGIN","r")))){
        while(fgets(buf,256,fp))
            local_prints("%s",buf);
        fclose(fp);
        BBS_MAIN_EXIT(20);
    }
#endif /* ! DEBUG */
#ifdef LOAD_LIMIT
    if(!stat("NO_LOAD_LIMIT",&st)&&S_ISREG(st.st_mode)){
        double load,cpu_load[3];
        get_load(cpu_load);
        load=cpu_load[0];
        local_prints("CPU 最近 (1,10,15) 分钟的平均负荷分别为 %.2f, %.2f, %.2f (目前上限 = %d)\r\n",
            cpu_load[0],cpu_load[1],cpu_load[2],max_load);
        if((load<0)||(load>max_load)){
            local_prints("%s\r\n\r\n%s\r\n%s\r\n",
                "很抱歉, 目前 CPU 负荷过重, 请稍候再来",
                "因为重复连接对本站冲击太大, 请您配合, 不要重复多次连接",
                "请您先休息 10 分钟, 然后再连接本站, 非常感谢!");
            BBS_MAIN_EXIT(((time_t)load));
        }
#ifdef AIX
        {
            int free=psdanger(-1);
            int safe=psdanger(SIGDANGER);
            int danger=125000;
            local_prints("RAM 当前空闲页数高出警戒阈值 %d (警戒阈值 = %d)\r\n\r\n",safe,(free-safe));
            if(safe<danger){
                if((server_pid!=-1)&&(!heavy_load))
                    kill(server_pid,SIGUSR1);
                local_prints("%s\r\n\r\n%s\r\n%s\r\n",
                    "很抱歉, 目前 RAM 被过度使用, 请稍候再来",
                    "因为重复连接对本站冲击太大, 请您配合, 不要重复多次连接",
                    "请您先休息 10 分钟, 然后再连接本站, 非常感谢!");
                BBS_MAIN_EXIT(60);
            }
            if((server_pid!=-1)&&heavy_load)
                kill(server_pid,SIGUSR2);
        }
#endif /* AIX */
    }
#endif /* LOAD_LIMIT */
#ifdef BBSRF_CHROOT
    if(chroot(BBSHOME)==-1){
        local_prints("Error while chroot to %s, exiting ...\r\n",BBSHOME);
        return -1;
    }
#endif /* BBSRF_CHROOT */
    getSession()->fromhost[IPLEN-1]=0;
    if(check_ban_IP(getSession()->fromhost,buf)>0){
        local_prints("本站目前不欢迎来自 %s 访问!\r\n原因: %s\r\n\r\n",getSession()->fromhost,buf);
        BBS_MAIN_EXIT(60);
    }
#ifdef HAVE_REVERSE_DNS
    getremotehost(getSession()->fromhost,IPLEN);
#endif /* HAVE_REVERSE_DNS */

    if(stat("core",&st)==-1){
        rl.rlim_cur=125829120;  /* 120M */
        rl.rlim_max=209715200;  /* 200M */
        setrlimit(RLIMIT_CORE,&rl);
    }
    main_bbs(0,argv);
    return -1;

#undef BBS_MAIN_EXIT
}
#ifndef SSHBBS
static int telnet_init(void){
    send(0,&cmd[0],3*sizeof(unsigned char),0);
    send(0,&cmd[3],6*sizeof(unsigned char),0);
    send(0,&cmd[9],3*sizeof(unsigned char),0);
    send(0,&cmd[12],3*sizeof(unsigned char),0);
    send(0,&cmd[15],3*sizeof(unsigned char),0);
    send(0,&cmd[18],3*sizeof(unsigned char),0);
    send(0,&cmd[21],3*sizeof(unsigned char),0);
    return 0;
}
static int start_daemon(int inetd,int port,const char *addr){
    static const int optval=1;
    KBS_SOCKADDR_IN sin;
    KBS_IN_ADDR inaddr;
    char pid_file_name[PATHLEN],pid_string[16];
    int sockfd,fd,maxfd;
    if(chdir(BBSHOME)==-1)
        exit(3);
    umask(0007);
    mport=port;
    if(inetd){
        if((fd=open("/dev/null",O_RDWR,0660))==-1||dup2(fd,1)==-1||dup2(fd,2)==-1)
            exit(2);
        if(fd>2)
            close(fd);
    }
    else{
        if(!no_fork){
            switch(fork()){
                case -1:
                    exit(2);
                case 0:
                    break;
                default:
                    exit(0);
            }
            setsid();
            switch(fork()){
                case -1:
                    exit(2);
                case 0:
                    break;
                default:
                    exit(0);
            }
        }
        KBS_SET_SIN_FAMILY(sin);
        if(!addr||!(inet_pton(KBS_SIN_FAMILY,addr,&inaddr)>0)){
            KBS_SET_SIN_ADDR(sin,KBS_SIN_ADDR_DEFAULT);
            snprintf(pid_file_name,PATHLEN,"var/bbsd.%d.pid",port);
        }
        else{
            KBS_SET_SIN_ADDR(sin,inaddr);
            snprintf(pid_file_name,PATHLEN,"var/bbsd.%d_%s.pid",port,addr);
        }
        KBS_SET_SIN_PORT(sin,port);
        if((fd=open(pid_file_name,O_RDWR|O_CREAT|O_TRUNC,0660))==-1)
            exit(2);
        if(fd!=4){
            if(dup2(fd,4)==-1)  /* file descriptor 4 statically means the opened pid file with exclusive lock ! */
                exit(2);
            close(fd);
        }
        if(write_lock(4,0,SEEK_SET,0)==-1){
            switch(errno){
                case EACCES:
                case EAGAIN:
                    fprintf(stderr,"BBS daemon on port <%d> had already been started!\n",port);
                    break;
                default:
                    fprintf(stderr,"Could not get exclusive lock on pid file: %s/%s\n",BBSHOME,pid_file_name);
                    break;
            }
            exit(0);
        }
#ifdef NOFILE
        maxfd=(!(NOFILE+0)?(256):(NOFILE));
#else /* ! NOFILE */
        maxfd=256;
#endif /* NOFILE */
        close(0);close(1);close(2);close(3);
        for(fd=5;fd<maxfd;fd++)
            close(fd);
#define SD_EXIT(_status) do{unlink(pid_file_name);exit(_status);}while(0)
        if((fd=open("/dev/null",O_RDWR,0660))==-1||dup2(fd,0)==-1||dup2(fd,1)==-1||dup2(fd,2)==-1)
            SD_EXIT(2);    /* file descriptor 0 and 1 and 2 statically means the opened character file /dev/null ! */
        if(fd>4)
            close(fd);
        snprintf(pid_string,sizeof(pid_string),"%d\n",(server_pid=getpid()));
        write(4,pid_string,strlen(pid_string));
        if((sockfd=socket(KBS_SIN_FAMILY,SOCK_STREAM,IPPROTO_TCP))==-1)
            SD_EXIT(1);
        if(sockfd!=SOCKFD){
            if(dup2(sockfd,SOCKFD)==-1)
                SD_EXIT(2);
            close(sockfd);
        }
        if(setsockopt(SOCKFD,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(const int))==-1)
            SD_EXIT(1);
        if(bind(SOCKFD,(struct sockaddr*)&sin,sizeof(KBS_SOCKADDR_IN))==-1)
            SD_EXIT(1);
        if(listen(SOCKFD,MAX_PENDING_CONNECTIONS)==-1)
            SD_EXIT(1);
#undef SD_EXIT
    }
    setuid(BBSUID);
    setgid(BBSGID);
    setreuid(BBSUID,BBSUID);
    setregid(BBSGID,BBSGID);
    return 0;
}
static int bbs_inet_main(char* argv){
    KBS_SOCKADDR_IN sin;
    socklen_t sinlen;
    sinlen=sizeof(KBS_SOCKADDR_IN);
    getpeername(0,(struct sockaddr*)&sin,&sinlen);
    KBS_SET_FROMHOST(sin,getSession()->fromhost);
    telnet_init();
    return bbs_main(argv);
}
static int bbs_standalone_main(char* argv){
    KBS_SOCKADDR_IN sin;
    socklen_t sinlen;
    char addr_buf[IPLEN];
    int sockfd,count;
    time_t lasttime,now;
    lasttime=time(NULL);
    count=0;
    while(1){
        sinlen=sizeof(KBS_SOCKADDR_IN);
#ifdef SMTH
        if((now=time(NULL))!=lasttime)
            count=0;
        else{
            if(count>5)
                sleep(1);
        }
#endif /* SMTH */
        sockfd=accept(SOCKFD,(struct sockaddr*)&sin,&sinlen);
        count++;
        if(sockfd==-1)
            continue;
        proxy_getpeername(sockfd,(struct sockaddr*)&sin,&sinlen);
#ifdef CHECK_IP_LINK
#ifdef HAVE_IPV6_SMTH
        if(check_IP_lists(sin.sin6_addr)==1){
#else /* ! HAVE_IPV6_SMTH */
        if(check_IP_lists(sin.sin_addr.s_addr)==1){
#endif /* HAVE_IPV6_SMTH */
            close(sockfd);
            continue;
        }
#endif /* CHECK_IP_LINK */
        if(!no_fork){
            switch(fork()){
                case -1:
                    exit(3);
                case 0:
                    break;
                default:
                    close(sockfd);
                    continue;
            }
        }
        KBS_SET_FROMHOST(sin,addr_buf);
        bbslog("0Connect","connect from %d (%d) in port %d",addr_buf,htons(KBS_SIN_MEMBER(sin,port)),mport);
        setsid();
        if(dup2(sockfd,0)==-1)      /* dup tcp link to fd 0 and then in the main_bbs func also to fd 1 */
            exit(2);
        close(3);                   /* close listen sock fd in child session */
        close(4);                   /* close pid file fd in child session */
        close(sockfd);              /* close accept peer fd in child session */
        break;                      /* leave fd 2 still open holding /dev/null */
    }
    KBS_SET_FROMHOST(sin,getSession()->fromhost);
    telnet_init();
    return bbs_main(argv);
}
int main(int argc,char **argv){
    char addr[STRLEN];
    int ret,inetd,port;
    addr[0]=0;inetd=0;port=23;
    while((ret=getopt(argc,argv,"idha:p:"))!=-1){
        switch(ret){
            case 'i':
                inetd=1;
                break;
            case 'd':
                no_fork=1;
                break;
            case 'h':
                puts("usage: bbsd [-i] [-d] [-h] [-a <addr>] [-p <port>]");
                return 0;
            case 'a':
                if(optarg[0])
                    snprintf(addr,STRLEN,"%s",optarg);
                break;
            case 'p':
                if(!isdigit(optarg[0]))
                    return -1;
                port=atoi(optarg);
                break;
            case '?':
                return -1;
        }
    }
#ifndef HAVE_IPV6_SMTH
    inaddr_any.s_addr=htonl(INADDR_ANY);
#endif /* HAVE_IPV6_SMTH */
    start_daemon(inetd,port,(!addr[0]?NULL:addr));
    main_signals();
    return (!inetd?bbs_standalone_main(argv[0]):bbs_inet_main(argv[0]));
}
#else /* SSHBBS */
void ssh_exit(void){
    if(ssh_exiting)
        return;
    ssh_exiting=1;
    abort_bbs(0);
    packet_disconnect("sshbbsd exit");
    return;
}
int bbs_entry(void){
    KBS_SOCKADDR_IN sin;
    socklen_t sinlen;
    setuid(BBSUID);
    setgid(BBSGID);
    main_signals();
    sinlen=sizeof(KBS_SOCKADDR_IN);
    atexit(ssh_exit);
    proxy_getpeername(0,(struct sockaddr*)&sin,&sinlen);
    KBS_SET_FROMHOST(sin,getSession()->fromhost);
    return bbs_main(saved_argv[0]);
}
#endif /* ! SSHBBS */

