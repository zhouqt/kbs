#include "innbbsconf.h"
#include "daemon.h"

extern int errno;
static void reapchild(s)
int s;
{
    int state;

    while (waitpid(-1, &state, WNOHANG | WUNTRACED) > 0) {
        /*
         * printf("reaping child\n"); 
         */
    }
}

void dokill(s)
int s;
{
    kill(0, SIGKILL);
}

static int INETDstart = 0;
void startfrominetd(int flag)
{
    INETDstart = flag;
}

void standalonesetup(int fd)
{
    int on = 1;
    struct linger foobar;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on))
        < 0)
        syslog(LOG_ERR, "setsockopt (SO_REUSEADDR): %m");
    foobar.l_onoff = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &foobar, sizeof(foobar)) < 0)
        syslog(LOG_ERR, "setsockopt (SO_LINGER): %m");
}

static const char *UNIX_SERVER_PATH;
static int (*halt) (int);

void sethaltfunction(int (*haltfunc) (int))
{
    halt = haltfunc;
}

void docompletehalt(void){
    exit(0);
}

void doremove(int s)
{
    if (halt != NULL)
        (*halt) (s);
    else
        docompletehalt();
}

int initunixserver(const char *path,const char *protocol)
{
    struct sockaddr_un s_un;

    /*
     * unix endpoint address 
     */
    struct protoent *pe;        /* protocol information entry */
    int s;

    bzero((char *) &s_un, sizeof(s_un));
    s_un.sun_family = AF_UNIX;
    strcpy(s_un.sun_path, path);
    if (protocol == NULL)
        protocol = "tcp";
    /*
     * map protocol name to protocol number 
     */
    pe = getprotobyname(protocol);
    if (pe == NULL) {
        fprintf(stderr, "%s: Unknown protocol.\n", protocol);
        return (-1);
    }
    /*
     * Allocate a socket 
     */
    s = socket(PF_UNIX, strcmp(protocol, "tcp") ? SOCK_DGRAM : SOCK_STREAM, 0);
    if (s < 0) {
        printf("protocol %d\n", pe->p_proto);
        perror("socket");
        return -1;
    }
    /*
     * standalonesetup(s); 
     */
    signal(SIGHUP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGCHLD, reapchild);
    UNIX_SERVER_PATH = path;
    signal(SIGINT, doremove);
    signal(SIGTERM, doremove);

    chdir("/");
    if (bind(s, (struct sockaddr *) &s_un, sizeof(struct sockaddr_un)) < 0) {
        perror("bind");
        perror(path);
        return -1;
    }
    listen(s, 10);
    return s;
}

int initinetserver(const char *service,const char *protocol){
    /* service should be "<service name or port> [[ADDRESS] <addr>]" */
    struct servent *se;
    struct protoent *pe;
    struct sockaddr_in sin;
    char buf[128],*addr;
    int sock,use_random_port,optval;
    memset(&sin,0,sizeof(struct sockaddr_in));
    sin.sin_family=AF_INET;
    if(!service)
        service=DEFAULTPORT;
    if(!protocol)
        protocol="tcp";
    snprintf(buf,128,"%s",service);
    if((addr=strstr(buf," [ADDRESS] "))){
        addr[0]=0;
        addr+=11;
    }
    if(buf[0]=='0'&&!buf[1])
        use_random_port=1;
    else
        use_random_port=0;
    if(!(pe=getprotobyname(protocol))){
        fprintf(stderr,"%s: unknown protocol ... \n",protocol);
        return -1;
    }
    if(!addr||inet_pton(AF_INET,addr,&(sin.sin_addr))<=0)
        sin.sin_addr.s_addr=htonl(INADDR_ANY);
    if(!(se=getservbyname(buf,protocol))){
        if(!(sin.sin_port=htons((u_short)atoi(buf)))&&!use_random_port){
            fprintf(stderr,"%s/%s: unknown service ... \n",buf,protocol);
            return -1;
        }
    }
    else
        sin.sin_port=se->s_port;
    if((sock=socket(PF_INET,(!strcmp(protocol,"tcp")?SOCK_STREAM:SOCK_DGRAM),pe->p_proto))<0){
        perror("socket");
        return -1;
    }
    optval=1;
    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int))<0){
        perror("setsockopt");
        return -1;
    }
    standalonesetup(sock);
    signal(SIGHUP,SIG_IGN);
    signal(SIGUSR1,SIG_IGN);
    signal(SIGCHLD,reapchild);
    signal(SIGINT,dokill);
    signal(SIGTERM,dokill);
    chdir("/");
    if(bind(sock,(struct sockaddr*)&sin,sizeof(struct sockaddr_in))<0){
        perror("bind");
        return -1;
    }
    if(listen(sock,10)<0){
        perror("listen");
        return -1;
    }
#ifdef DEBUG
    do{
        socklen_t len=sizeof(struct sockaddr_in);
        getsockname(sock,&sin,&len);
        printf("PORT: ALLOC %d\n",sin.sin_port);
    }
#endif
    return sock;
}

int open_unix_listen(const char *path,const char *protocol,int (*initfunc)(int))
{
    int s;

    s = initunixserver(path, protocol);
    if (s < 0) {
        return -1;
    }
    if (initfunc != NULL) {
        printf("in inetsingleserver before initfunc s %d\n", s);
        if ((*initfunc) (s) < 0) {
            perror("initfunc error");
            return -1;
        }
        printf("end inetsingleserver before initfunc \n");
    }
    return s;
}

int open_listen(const char *service,const char *protocol,int (*initfunc)(int))
{
    int s;

    if (!INETDstart)
        s = initinetserver(service, protocol);
    else
        s = 0;
    if (s < 0) {
        return -1;
    }
    if (initfunc != NULL) {
        printf("in inetsingleserver before initfunc s %d\n", s);
        if ((*initfunc) (s) < 0) {
            perror("initfunc error");
            return -1;
        }
        printf("end inetsingleserver before initfunc \n");
    }
    return s;
}

int tryaccept(int s)
{
    int ns;
    socklen_t fromlen;
    struct sockaddr sockaddr;   /* Internet endpoint address */
    fromlen = sizeof(struct sockaddr_in);

#ifdef DEBUGSERVER
    fputs("Listening again\n", stdout);
#endif
    do {
        ns = accept(s, &sockaddr, &fromlen);
        errno = 0;
    } while (ns < 0 && errno == EINTR);
    return ns;
}

#if 0 //etnlegend, 不用的先注释掉...

int inetsingleserver(const char *service,const char *protocol,int (*serverfunc)(int),int (*initfunc)(int))
{
    int s;

    if (!INETDstart)
        s = initinetserver(service, protocol);
    else
        s = 0;
    if (s < 0) {
        return -1;
    }
    if (initfunc != NULL) {
        printf("in inetsingleserver before initfunc s %d\n", s);
        if ((*initfunc) (s) < 0) {
            perror("initfunc error");
            return -1;
        }
        printf("end inetsingleserver before initfunc \n");
    }
    {
        int ns = tryaccept(s);
        int result = 0;

        if (ns < 0 && errno != EINTR) {
#ifdef DEBUGSERVER
            perror("accept");
#endif
        }
        close(s);
        if (serverfunc != NULL)
            result = (*serverfunc) (ns);
        close(ns);
        return (result);
    }
}

int inetserver(const char *service,const char *protocol,int (*serverfunc)(int))
{
    int s;

    if (!INETDstart)
        s = initinetserver(service, protocol);
    else
        s = 0;
    if (s < 0) {
        return -1;
    }
    for (;;) {
        int ns = tryaccept(s);
        int result = 0;
        int pid;

        if (ns < 0 && errno != EINTR) {
#ifdef DEBUGSERVER
            perror("accept");
#endif
            continue;
        }
#ifdef DEBUGSERVER
        fputs("Accept OK\n", stdout);
#endif
        pid = fork();
        if (pid == 0) {
            close(s);
            if (serverfunc != NULL)
                result = (*serverfunc) (ns);
            close(ns);
            exit(result);
        } else if (pid < 0) {
            perror("fork");
            return -1;
        }
        close(ns);
    }
    return 0;
}

#endif /* 0 */

int inetclient(const char *server,const char *service,const char *protocol){
    struct servent *se;         /* service information entry */
    struct hostent *he;         /* host information entry */
    struct protoent *pe;        /* protocol information entry */
    struct sockaddr_in sin;     /* Internet endpoint address */
    int port, s;

    bzero((char *) &sin, sizeof(sin));
    sin.sin_family = AF_INET;

    if (service == NULL)
        service = DEFAULTPORT;
    if (protocol == NULL)
        protocol = "tcp";
    if (server == NULL)
        server = DEFAULTSERVER;
    /*
     * map service name to port number 
     */
    /*
     * service ---> port 
     */
    se = getservbyname(service, protocol);
    if (se == NULL) {
        port = htons((u_short) atoi(service));
        if (port == 0) {
            fprintf(stderr, "%s/%s: Unknown service.\n", service, protocol);
            return (-1);
        }
    } else
        port = se->s_port;
    sin.sin_port = port;

    /*
     * map server hostname to IP address, allowing for dotted decimal 
     */
    he = gethostbyname(server);
    if (he == NULL) {
        sin.sin_addr.s_addr = inet_addr(server);
        if (sin.sin_addr.s_addr == INADDR_NONE) {
            fprintf(stderr, "%s: Unknown host.\n", server);
            return (-1);
        }
    } else
        bcopy(he->h_addr, (char *) &sin.sin_addr, he->h_length);

    /*
     * map protocol name to protocol number 
     */
    pe = getprotobyname(protocol);
    if (pe == NULL) {
        fprintf(stderr, "%s: Unknown protocol.\n", protocol);
        return (-1);
    }
    /*
     * Allocate a socket 
     */
    s = socket(PF_INET, strcmp(protocol, "tcp") ? SOCK_DGRAM : SOCK_STREAM, pe->p_proto);
    if (s < 0) {
        perror("socket");
        return -1;
    }
    /*
     * Connect the socket to the server 
     */
    if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        /*
         * perror("connect"); 
         */
        return -1;
    }
    return s;
}

int unixclient(const char *path,const char *protocol){
    struct protoent *pe;        /* protocol information entry */
    struct sockaddr_un s_un;    /* unix endpoint address */
    int s;

    bzero((char *) &s_un, sizeof(s_un));
    s_un.sun_family = AF_UNIX;

    if (path == NULL)
        path = DEFAULTPATH;
    if (protocol == NULL)
        protocol = "tcp";
    strcpy(s_un.sun_path, path);

    /*
     * map protocol name to protocol number 
     */
    pe = getprotobyname(protocol);
    if (pe == NULL) {
        fprintf(stderr, "%s: Unknown protocol.\n", protocol);
        return (-1);
    }
    /*
     * Allocate a socket 
     */
    s = socket(PF_UNIX, strcmp(protocol, "tcp") ? SOCK_DGRAM : SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return -1;
    }
    /*
     * Connect the socket to the server 
     */
    if (connect(s, (struct sockaddr *) &s_un, sizeof(s_un)) < 0) {
        /*
         * perror("connect"); 
         */
        return -1;
    }
    return s;
}
