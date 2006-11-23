/* Leeward: usage: bbspop3d [port]
   Note: 
         0. to compile, just type: make bbspop3d
         1. If no port given, I use 110 (in fact, using macro POP3PORT)
         2. If atoi(port) returns zero, I use 110  (the same as above)
         3. Using port 110 needs starting by root 
            If started by bbsroot, try port over 1023
         4. Four macros defined below: 
            BBSHOME, POP3PORT, BBSNAME and BADLOGINFILE
            You may need modify these four macros to fit your BBS settings
         5. I added codes to support three functions: 
            (1) Top // extended POP3 function
            (2) nextwordX // enable passwords including space character(s)
            (3) logattempt // bbslog password error showed when one logins BBS 
            (4) Do not erase mails marked 
         6. While making bbspop3d, you might encounter linker errors
            Try to pass the linker some libraries 
            (For example: link under SUNOS, you may need add -lnsl and -lsocket
                          link under Redhat 5.X, you may need add -lcrypt)
         7. bbspop3d.c is put in the same directory as bbs.h
         8. Client settings:
            (1) pop3 server  : Your BBS server
            (2) pop3 port    : 110(POP3PORT) or the argv[1] while starting pop3d
            (3) pop3 account : BBSID.bbs (For example: mine is "Leeward.bbs")
            (4) pop3 password: The same as the client's BBSID's password
         9. Filter the ANSI control characters(KCN)
         10. support POP3 via SSL (pop3s) (COMMAN)
         11. support attachment (KCN)
  Last modified: 2002.07
*/

#include <sys/ioctl.h>
#include "bbs.h"

#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sys/time.h>
#include <sys/stat.h>

#ifdef USE_SSL
/* COMMAN : SPOP3(pop3 through SSL) support */
#include <openssl/rsa.h>        /* SSLeay stuff */
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define CERTFILE BBSHOME "/etc/bbs.crt"
#define KEYFILE  BBSHOME "/etc/bbs.key"

SSL_CTX *ctx;
SSL *ssl;
SSL_METHOD *meth;
#endif
int use_ssl = 0;

#define BUFSIZE         1024
/*#define POP3PORT 110 remote to sysname.h*/

/* 将 MAIL_BBSDOMAIN 和 NAME_BBS_ENGLISH 分开 czz 03.03.08 */
/*
#ifdef BBSNAME
#undef BBSNAME
#endif
#define BBSNAME "@"MAIL_BBSDOMAIN
*/

struct fileheader currentmail;
struct userec alluser;

char LowUserid[20];
char LowUserPostfix[20];
char MailBoxName[20];
#define FROMTAG "From"
#define TOTAG "To"
char *ownerTag;
char *selfTag;
char genbuf[BUFSIZE];

//static void pop3_logattempt(char* uid,char* frm);       /* Leeward 98.07.25 */
#define QLEN            5

#define S_CONNECT       1
#define S_LOGIN         2


#define STRN_CPY(d,s,l) { strncpy((d),(s),(l)); (d)[(l)-1] = 0; }
#define ANY_PORT        0
//#define RFC931_PORT     113
// #define RFC931_TIMEOUT         5
/*#define POP3_TIMEOUT         60*/
/* Leeward 98.05.06 Increases TIMEOUT value for slow modem users */
#define POP3_TIMEOUT         180

int State;
int msock, sock;                /* master server socket */
static void reaper(int signo);
static char inbuf[BUFSIZE];
/*char remote_userid[STRLEN];*/
char *msg, *cmd;
int fd;
struct fileheader *fcache;
int totalnum, totalbyte, markdel, idletime;
int *postlen;

void BBSlog_usies();
void Quit();
void User();
void Pass();
void Noop();
void Stat();
void List();
void Retr();
void Rset();
void Last();
void Dele();
void Uidl();
void Top();                     /* Leeward adds, 98.01.21 */

struct commandlist {
    char *name;
    void (*fptr) ();
} cmdlists[] = {
    {
    "retr", Retr}, {
    "dele", Dele}, {
    "user", User}, {
    "pass", Pass}, {
    "stat", Stat}, {
    "list", List}, {
    "uidl", Uidl}, {
    "quit", Quit}, {
    "rset", Rset}, {
    "last", Last}, {
    "noop", Noop}, {
    "top", Top},                /* Leeward adds, 98.01.21 */
    {
NULL, NULL},};

char *crypt();

#ifdef USE_SSL
static void init_ssl(void)
{
    SSL_load_error_strings();
    SSLeay_add_ssl_algorithms();
    meth = SSLv23_server_method();
    ctx = SSL_CTX_new(meth);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    if (SSL_CTX_use_certificate_file(ctx, CERTFILE, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, KEYFILE, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the certificate public key\n");
        exit(-1);
    }

}

static void do_ssl(int fd)
{
    int err;

    ssl = SSL_new(ctx);
    if (!ssl)
        exit(-1);
    SSL_set_fd(ssl, fd);
    err = SSL_accept(ssl);
    if (err < 0)
        exit(-1);
}

#endif

static void abort_server(int signo)
{
    BBSlog_usies("ABORT SERVER");
    close(msock);
    close(sock);
    exit(1);
}

void dokill(int signo)
{
    kill(0, SIGKILL);
}

/* COMMAN : no use now 
static FILE *fsocket(domain, type, protocol)
int     domain;
int     type;
int     protocol;
{
    int     s;
    FILE   *fp;

    if ((s = socket(domain, type, protocol)) < 0) {
        return (0);
    } else {
        if ((fp = fdopen(s, "r+")) == 0) {
            close(s);
        }
        return (fp);
    }
}
*/

static int readstr(int sock, char *s, int size)
{
    int retlen = 0;
    char c;
    static char myinbuf[1024];
    static int bufsize = 0;
    static int bufptr = 0;

    do {
        /* read a char */
        if (bufptr >= bufsize) {
#ifdef USE_SSL
            if (use_ssl)
                bufsize = SSL_read(ssl, myinbuf, 1024);
            else
#endif
                bufsize = read(sock, myinbuf, 1024);
            if (bufsize <= 0) {
                *s = '\0';
                return retlen;
            }
            bufptr = 0;
        }
        retlen++;
        c = myinbuf[bufptr++];
        *s++ = c;
        if (c == '\n' || retlen == size - 1) {
            *s = '\0';
            return retlen;
        }
    } while (1);
}
static void outs(str)
    char *str;
{
    char sendbuf[BUFSIZE * 2];

    (void) bzero(sendbuf, sizeof(sendbuf));
    (void) sprintf(sendbuf, "%s\r\n", str);
#ifdef USE_SSL
    if (use_ssl)
        SSL_write(ssl, sendbuf, strlen(sendbuf));
    else
#endif
        (void) write(sock, sendbuf, strlen(sendbuf));
}

static int outc(char ch)
{
    int ret;
#ifdef USE_SSL
    if (use_ssl)
        ret=SSL_write(ssl, &ch, 1);
    else
#endif
        ret=write(sock, &ch, 1);
    return ret;
}
void outfile(filename, linenum)
    char *filename;
    int linenum;
{
    char linebuf[10240];
    char *buf, *p;              /* KCN.99.09.01 */
    char *ptr;
    off_t size;
    long left;

    if (linenum) {
        BBS_TRY {
            bool esc;
            if (safe_mmapfile(filename, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &size, NULL) == 0) {
                outs(".");
                BBS_RETURN_VOID;
            }
            esc=false;
            buf=linebuf;
            for (p=ptr,left=size;left>0&&linenum>0;p++,left--) {
                long attach_len;
                char* file,*attach;
                if (NULL !=(file = checkattach(p, left, &attach_len, &attach))) {
                    left-=(attach-p)+attach_len-1;
                    p=attach+attach_len-1;
                    uuencode(attach, attach_len, file, outc);
                    continue;
                }
                if ((*p=='\n')||(buf-linebuf>=10240-1)) {
                    linenum--;
                    *buf=0;
                    if (strcmp(linebuf, ".") == 0)
                        outs("..");
                    else
                        outs(linebuf);
                    buf=linebuf;
                    continue;
                }
                if (esc) {
                    if (*p == '\x1b') {
                        *buf = *p;
                        buf++;
                        esc = false;
                    } else if (isalpha(*p))
                        esc = false;
                }
                else {
                    if (*p=='\x1b')
                        esc=true;
                    else {
                        *buf=*p;
                        buf++;
                    }
                }
            }
        }
        BBS_CATCH {
        }
        BBS_END;
        end_mmapfile((void *) ptr, size, -1);
    }
    outs("");
    outs(".");
}


#if 0
void rfc931(rmt_sin, our_sin, dest)
    struct sockaddr_in *rmt_sin;
    struct sockaddr_in *our_sin;
    char *dest;
{
    unsigned rmt_port;
    unsigned our_port;
    struct sockaddr_in rmt_query_sin;
    struct sockaddr_in our_query_sin;
    char user[256];
    char buffer[512];
    char *cp;
    FILE *fp;
    char *result = "unknown";
    struct hostent *hp;

    /*
     * Use one unbuffered stdio stream for writing to and for reading from
     * the RFC931 etc. server. This is done because of a bug in the SunOS 
     * 4.1.x stdio library. The bug may live in other stdio implementations,
     * too. When we use a single, buffered, bidirectional stdio stream ("r+"
     * or "w+" mode) we read our own output. Such behaviour would make sense
     * with resources that support random-access operations, but not with   
     * sockets.
     */
    if ((fp = fsocket(AF_INET, SOCK_STREAM, 0)) != 0) {
        setbuf(fp, (char *) 0);

        /*
         * Set up a timer so we won't get stuck while waiting for the server.
         */

        if (setjmp(timebuf) == 0) {
            signal(SIGALRM, timeout);
            alarm(RFC931_TIMEOUT);

            /*
             * Bind the local and remote ends of the query socket to the same
             * IP addresses as the connection under investigation. We go
             * through all this trouble because the local or remote system
             * might have more than one network address. The RFC931 etc.  
             * client sends only port numbers; the server takes the IP    
             * addresses from the query socket.
             */

            our_query_sin = *our_sin;
            our_query_sin.sin_port = htons(ANY_PORT);
            rmt_query_sin = *rmt_sin;
            rmt_query_sin.sin_port = htons(RFC931_PORT);

            if (bind(fileno(fp), (struct sockaddr *) &our_query_sin, sizeof(our_query_sin)) >= 0 && connect(fileno(fp), (struct sockaddr *) &rmt_query_sin, sizeof(rmt_query_sin)) >= 0) {

                /*
                 * Send query to server. Neglect the risk that a 13-byte
                 * write would have to be fragmented by the local system and
                 * cause trouble with buggy System V stdio libraries.
                 */

                fprintf(fp, "%u,%u\r\n", ntohs(rmt_sin->sin_port), ntohs(our_sin->sin_port));
                fflush(fp);

                /*
                 * Read response from server. Use fgets()/sscanf() so we can
                 * work around System V stdio libraries that incorrectly
                 * assume EOF when a read from a socket returns less than
                 * requested.
                 */

                if (fgets(buffer, sizeof(buffer), fp) != 0
                    && ferror(fp) == 0 && feof(fp) == 0
                    && sscanf(buffer, "%u , %u : USERID :%*[^:]:%255s", &rmt_port, &our_port, user) == 3 && ntohs(rmt_sin->sin_port) == rmt_port && ntohs(our_sin->sin_port) == our_port) {

                    /*
                     * Strip trailing carriage return. It is part of the
                     * protocol, not part of the data.
                     */

                    if ((cp = strchr(user, '\r')) != NULL)
                        *cp = 0;
                    result = user;
                }
            }
            alarm(0);
        }
        fclose(fp);
    }
    STRN_CPY(dest, result, 60);

    if (strcmp(dest, "unknown") == 0)
        strcpy(dest, "");
    else
        strcat(dest, "@");

    hp = gethostbyaddr((char *) &rmt_sin->sin_addr, sizeof(struct in_addr), rmt_sin->sin_family);
    if (hp)
        strcat(dest, hp->h_name);
    else
        strcat(dest, (char *) inet_ntoa(rmt_sin->sin_addr));

}
#endif
char *nextwordlower(str)
    char **str;
{
    char *p;

    while (Isspace(**str))
        (*str)++;
    p = (*str);

    while (**str && !Isspace(**str)) {
        **str = tolower(**str);
        (*str)++;
    }

    if (**str) {
        **str = '\0';
        (*str)++;
    }
    return p;
}

char *nextword2(str)
    char **str;
{
    char *p;

    while (Isspace(**str))
        (*str)++;
    p = (*str);

    while (**str && !Isspace(**str))
        (*str)++;

    if (**str) {
        **str = '\0';
        (*str)++;
    }
    return p;
}

char *nextwordX(str)            /* Leeward: 97.12.27: enable password include space(s) */
    char **str;
{
    char *p;

    while (Isspace(**str))
        (*str)++;
    p = (*str);

    while (**str && '\t' != **str && 10 != **str && 13 != **str)
        (*str)++;

    if (**str) {
        **str = '\0';
        (*str)++;
    }
    return p;
}

void Init()
{
    State = S_CONNECT;
    LowUserid[0] = '\0';
    LowUserPostfix[0] = '\0';
    MailBoxName[0] = '\0';
    /*remote_userid[0] = 0;*/
    markdel = 0;
    idletime = 0;
}

void Login_init()
{
    int fd, i;
    struct stat st;

    totalnum = totalbyte = 0;
    sprintf(genbuf, "mail/%c/%s/%s", toupper(*LowUserid), LowUserid, MailBoxName);
    if (stat(genbuf, &st) == -1 || st.st_size == 0) {
        return;
    }
    totalnum = st.st_size / sizeof(struct fileheader);
    fcache = (struct fileheader *) malloc(st.st_size);
    postlen = (int *) malloc(sizeof(int) * totalnum);
    fd = open(genbuf, O_RDONLY);
    read(fd, fcache, st.st_size);
    close(fd);

    for (i = 0; i < totalnum; i++) {
        sprintf(genbuf, "mail/%c/%s/%s", toupper(*LowUserid), LowUserid, fcache[i].filename);
        if (stat(genbuf, &st) == -1)
            st.st_size = 0;
        postlen[i] = st.st_size + strlen(fcache[i].owner) + 10 + strlen(fcache[i].title)
            + 10 + 40;
        totalbyte += postlen[i];
        if (fcache[i].accessed[0] & FILE_MARKED)        /* Leeward 99.01.28 */
            fcache[i].accessed[1] = 'M';
        fcache[i].accessed[0] = ' ';
    }
    if (strcmp(MailBoxName, ".SENT") == 0) {
        ownerTag = TOTAG;
        selfTag = FROMTAG;
    } else {
        ownerTag = FROMTAG;
        selfTag = TOTAG;
    }    
}

void pop3_timeout(int signo)
{
    idletime++;
    if (idletime > 5) {
        BBSlog_usies("ABORT - TIMEOUT");
        close(sock);
        exit(1);
    }
    alarm(POP3_TIMEOUT);
}


int main(int argc, char **argv)
{
#ifdef HAVE_IPV6_SMTH
    struct sockaddr_in6 fsin, our;
#else
    struct sockaddr_in fsin, our;
#endif
    int on, alen, len, i, n;
    char *str;
    int childpid;
    FILE *fp;
    char addr[STRLEN],pid_file_name[PATHLEN],*p;
    int opt,pop3_port,pop3s_port,port;

    opterr=0;addr[0]=0;pop3_port=POP3PORT;pop3s_port=POP3SPORT;
    while((opt=getopt(argc,argv,"a:p:"))!=-1){
        switch(opt){
            case 'a':
                if(optarg[0])
                    snprintf(addr,STRLEN,"%s",optarg);
                break;
            case 'p':
                if(isdigit(optarg[0])){
                    pop3_port=atoi(optarg);
                    if((p=strchr(optarg,','))&&isdigit(p[1]))
                        pop3s_port=atoi(&p[1]);
                }
                break;
            default:
                fprintf(stderr,"%s\n","newpop3d [-a <addr>] [-p <pop3_port>[,<pop3s_port>]]");
                exit(1);
        }
    }

#ifndef DEBUG
    if (fork())
        exit(0);
#endif /* DEBUG */

    port=pop3_port;
    use_ssl=0;

#ifdef USE_SSL
    switch(fork()){
        case 0:
            init_ssl();
            port=pop3s_port;
            use_ssl=1;
            break;
        case -1:
            exit(1);
            break;
        default:
            break;
    }
#endif /* USE_SSL */

#ifndef CYGWIN

#ifdef NOFILE
        n=(!(NOFILE+0)?(256):(NOFILE));
#else /* ! NOFILE */
        n=256;
#endif /* NOFILE */

    do{
        close(n);
    }
    while(--n>2);

    freopen("/dev/null","r",stdin);
    freopen("/dev/null","w",stdout);
    freopen("/dev/null","w",stderr);

#ifdef TIOCNOTTY
    /* Certain platform (e.g. Interix) does not define TIOCNOTTY */
    if ((n = open("/dev/tty", O_RDWR)) > 0) {
        ioctl(n, TIOCNOTTY, 0);
        close(n);
    }
#endif
#else /* !CYGWIN */
    n = getdtablesize();
	while (n > 3)
			close(--n);
#endif

#ifdef HAVE_IPV6_SMTH
    if ((msock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        exit(1);
    }
    setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    bzero((char *) &fsin, sizeof(fsin));
    fsin.sin6_family = AF_INET6;
    if(inet_pton(AF_INET6, addr, &(fsin.sin6_addr)) <= 0)
        fsin.sin6_addr = in6addr_any;
    fsin.sin6_port = htons(port);
#else
    if ((msock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        exit(1);
    }
    setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    bzero((char *) &fsin, sizeof(fsin));
    fsin.sin_family = AF_INET;
    if(inet_pton(AF_INET, addr, &(fsin.sin_addr)) <= 0)
        fsin.sin_addr.s_addr = htonl(INADDR_ANY);
    fsin.sin_port = htons(port);
#endif

    if (bind(msock, (struct sockaddr *) &fsin, sizeof(fsin)) < 0) {
        exit(1);
    }

    signal(SIGHUP, abort_server);
    signal(SIGCHLD, reaper);
    signal(SIGINT, dokill);
    signal(SIGTERM, dokill);

    if(listen(msock, QLEN)==-1)
        exit(1);

    if(!addr[0])
        snprintf(pid_file_name,STRLEN,"var/%s.%d.pid",(!use_ssl?"newpop3d":"newpop3d_ssl"),port);
    else
        snprintf(pid_file_name,STRLEN,"var/%s.%d_%s.pid",(!use_ssl?"newpop3d":"newpop3d_ssl"),port,addr);

    if((fp=fopen(pid_file_name,"w"))){
        fprintf(fp,"%d\n",(int)getpid());
        fclose(fp);
    }

    setgid(BBSGID);
    setuid(BBSUID);
    setreuid(BBSUID, BBSUID);
    setregid(BBSGID, BBSGID);
    chdir(BBSHOME);

    resolve_ucache();
    resolve_utmp();
    while (1) {

        alen = sizeof(fsin);
        sock = accept(msock, (struct sockaddr *) &fsin, (socklen_t *) & alen);
        if (sock < 0) {
            if (errno != EINTR)
                continue;
        }
#ifndef DEBUG
        if ((childpid = fork()) < 0) {
            exit(1);
        }
#endif

        switch (childpid) {
        case 0:                /* child process */
            close(msock);

            setgid(BBSGID);
            setuid(BBSUID);
#ifdef HAVE_IPV6_SMTH
	    inet_ntop(AF_INET6, &fsin.sin6_addr, getSession()->fromhost, IPLEN);
#else
            strcpy(getSession()->fromhost, (char *) inet_ntoa(fsin.sin_addr));
#endif	
            if (check_ban_IP(getSession()->fromhost, genbuf)>0) {
                outs("-ERR your ip is baned");
                close(sock);
                exit(0);
            }
            len = sizeof our;
            getsockname(sock, (struct sockaddr *) &our, (socklen_t *) & len);

            Init();

#ifdef USE_SSL
            /* COMMAN: SSL init */
            if (use_ssl)
                do_ssl(sock);
#endif
            /*  COMMAN: do we really need ident lookup?  2002-7
               rfc931( &fsin, &our, remote_userid );
             */
            /*   COMMAN: stdio lib is of no use
               cfp = fdopen(sock, "r+");
               setbuf(cfp, (char *) 0);
             */
#ifdef USE_SSL
            sprintf(genbuf, "+OK KBS BBS POP3/POP3S server at %s starting.", NAME_BBS_ENGLISH);
#else
            sprintf(genbuf, "+OK KBS BBS POP3 server at %s starting.", NAME_BBS_ENGLISH);
#endif
            outs(genbuf);

            BBSlog_usies("CONNECT");
            alarm(0);
            signal(SIGALRM, pop3_timeout);
            alarm(POP3_TIMEOUT);

            while (readstr(sock, inbuf, sizeof(inbuf)) != 0) {

                idletime = 0;

                msg = inbuf;

                inbuf[strlen(inbuf) - 1] = '\0';
                if (inbuf[strlen(inbuf) - 1] == '\r')
                    inbuf[strlen(inbuf) - 1] = '\0';
                cmd = nextwordlower(&msg);

                if (*cmd == 0)
                    continue;

                i = 0;
                while ((str = cmdlists[i].name) != NULL) {
                    if (strcmp(cmd, str) == 0)
                        break;
                    i++;
                }

                if (str == NULL) {
                    sprintf(genbuf, "-ERR Unknown command: \"%s\".", cmd);
                    outs(genbuf);
                } else
                    (*cmdlists[i].fptr) ();

            }

            if (State == S_LOGIN) {
                free(fcache);
                free(postlen);
            }
            BBSlog_usies("ABORT");
            close(sock);
            exit(0);
            break;
        default:               /* parent process */
            close(sock);
            break;
        }
    }
}

static void reaper(int signo)
{
    int state, pid;

    signal(SIGCHLD, SIG_IGN);
    signal(SIGINT, dokill);
    signal(SIGTERM, dokill);

    while ((pid = waitpid(-1, &state, WNOHANG | WUNTRACED)) > 0);
}

void Noop()
{
    outs("+OK");
    return;
}

int get_userdata(user)
    char *user;
{
    int uid;

    if (strcmp(user, "guest") == 0) return -1;
    uid = getuser(user, &getCurrentUser());
    if (uid) {
        alluser = *getCurrentUser();
        getCurrentUser() = &alluser; /* to fix */
        return 1;
    }
    return -1;
}

void User()
{
    char *ptr;

    if (State == S_LOGIN) {
        outs("-ERR Unknown command: \"user\".");
        return;
    }

    cmd = nextword2(&msg);
    if (*cmd == 0) {
        outs("-ERR Too few arguments for the user command.");
        return;
    }
    if ((ptr = strstr(cmd, ".bbs")) != NULL || (ptr = strstr(cmd, ".BBS")) != NULL) {
        if (*(ptr + 4) != 0) {
            sprintf(genbuf, "-ERR Unknown user: \"%s\".", cmd);
            outs(genbuf);
            return;
        }
        *ptr = '\0';
    }
    /* 设置缺省邮箱 */
    strcpy(MailBoxName, ".DIR");
    if ((ptr = strchr(cmd, '.')) != NULL) {
        /* 识别邮箱 */
        if (*(ptr+1) != 0) {
           strncpy(MailBoxName, ptr, sizeof(MailBoxName));
           strncpy(LowUserPostfix, ptr+1, sizeof(LowUserPostfix));
           MailBoxName[sizeof(MailBoxName)-1] = '\0';
           LowUserPostfix[sizeof(LowUserPostfix)-1] = '\0';
        }
        *ptr = '\0';
        ptr = MailBoxName+1+strcspn(MailBoxName+1, "/.\\");
        *ptr = '\0';
        for (ptr = LowUserPostfix; *ptr != 0; ++ptr)
            *ptr = tolower(*ptr);
    }
    /*
       if (strstr(cmd, ".bbs") == NULL) {
       sprintf(genbuf, "-ERR Unknown user: \"%s\".", cmd);
       outs(genbuf);
       return;
       }

       ptr = strchr(cmd, '.');
       *ptr = '\0';
     */
    for (ptr = cmd; *ptr != 0; ++ptr)
        *ptr = tolower(*ptr);
    if (get_userdata(cmd) == 1) {
        if (check_ip_acl(getCurrentUser()->userid, getSession()->fromhost)) {
            sprintf(genbuf, "-ERR Your IP is not welcomed.");
            outs(genbuf);
        } else {
            strcpy(LowUserid, getCurrentUser()->userid);
            sprintf(genbuf, "+OK Password required for %s", cmd);
            outs(genbuf);
        }
    } else {
        sprintf(genbuf, "-ERR Unknown user: \"%s\".", cmd);
        outs(genbuf);
    }
    return;
}

void BBSlog_usies(char *buf)
{
    FILE *fp;

    if ((fp = fopen("reclog/pop3d.log", "a")) != NULL) {
        time_t now;
        struct tm *p;

        time(&now);
        p = localtime(&now);
	/* log的Y2K czz 2003.3.8 */
        fprintf(fp, "%04d/%02d/%02d %02d:%02d:%02d [%s](%s) %s\n",
                p->tm_year+1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, getCurrentUser()->userid ? getCurrentUser()->userid : "", getSession()->fromhost, buf);
        fflush(fp);
        fclose(fp);
    }
}

void Retr()
{
    int num;

    struct stat st;

    char *ptr;
	char mail_domain[STRLEN];

    if (State != S_LOGIN) {
        outs("-ERR Unknown command: \"retr\".");
        return;
    }

    cmd = nextword2(&msg);

    if (*cmd == 0) {
        outs("-ERR Too few arguments for the retr command.");
        return;
    }

    num = atoi(cmd);
    if (num <= 0 || totalnum < num) {
        sprintf(genbuf, "-ERR Message %d does not exist.", num);
        outs(genbuf);
        return;
    } else if (fcache[num - 1].accessed[0] == 'X') {
        sprintf(genbuf, "-ERR Message %d has been deleted.", num);
        outs(genbuf);
        return;
    }
    num--;
    sprintf(genbuf, "+OK %d octets", postlen[num]);
    outs(genbuf);
    sprintf(genbuf, "mail/%c/%s/%s", toupper(*LowUserid), LowUserid, fcache[num].filename);
    lstat(genbuf, &st); /* use lstat instead of stat - atppp 20041023 */
    ptr = ctime(&st.st_mtime);
    /*       Wed Jan 21 17:42:14 1998            -- ctime returns
             012345678901234567890123            -- offsets
       Date: Wed, 21 Jan 1998 17:54:33 +0800     -- RFC wants     */
    sprintf(genbuf, "Received: from insidesmtp (unknown [127.0.0.1]); %3.3s, %2.2s %3.3s %4.4s %8.8s +0800", ptr + 0, ptr + 8, ptr + 4, ptr + 20, ptr + 11);
    outs(genbuf);
    sprintf(genbuf, "Date: %3.3s, %2.2s %3.3s %4.4s %8.8s +0800", ptr + 0, ptr + 8, ptr + 4, ptr + 20, ptr + 11);
    outs(genbuf);
	snprintf(mail_domain, sizeof(mail_domain), "@%s", MAIL_BBSDOMAIN);
    if (index(fcache[num].owner, '@') == NULL) {
        if ((ptr = strchr(fcache[num].owner, ' ')) != NULL)
            *ptr = '\0';
        sprintf(genbuf,"%s: %s%s", ownerTag, fcache[num].owner, mail_domain);
    }
    else
    	sprintf(genbuf, "%s: %s", ownerTag, fcache[num].owner);
    outs(genbuf);
    sprintf(genbuf, "%s: %s%s", selfTag, getCurrentUser()->userid, mail_domain);
    outs(genbuf);
    sprintf(genbuf, "Subject: %s", fcache[num].title);
    outs(genbuf);
    outs("");
    sprintf(genbuf, "mail/%c/%s/%s", toupper(*LowUserid), LowUserid, fcache[num].filename);
    outfile(genbuf, 99999);

    setmailcheck(LowUserid);
}


void Stat()
{
    if (State != S_LOGIN) {
        outs("-ERR Unknown command: \"stat\".");
        return;
    }
    sprintf(genbuf, "+OK %d %d", totalnum, totalbyte);
    outs(genbuf);
}


void Rset()
{
    int i;

    if (State != S_LOGIN) {
        outs("-ERR Unknown command: \"rset\".");
        return;
    }

    for (i = 0; i < totalnum; i++) {
        fcache[i].accessed[0] = ' ';
    }
    markdel = 0;
    sprintf(genbuf, "+OK Maildrop has %d messages (%d octets)", totalnum, totalbyte);
    outs(genbuf);
}

void List()
{
    int i;

    if (State != S_LOGIN) {
        outs("-ERR Unknown command: \"list\".");
        return;
    }

    cmd = nextword2(&msg);

    if (*cmd == 0) {
        sprintf(genbuf, "+OK %d messages (%d octets)", totalnum, totalbyte);
        outs(genbuf);
        for (i = 0; i < totalnum; i++) {
            if (fcache[i].accessed[0] == ' ') {
                sprintf(genbuf, "%d %d", i + 1, postlen[i]);
                outs(genbuf);
            }
        }
        outs(".");
    } else {
        i = atoi(cmd);
        if (i <= 0 || totalnum < i) {
            sprintf(genbuf, "-ERR Message %d does not exist.", i);
            outs(genbuf);
            return;
        } else if (fcache[i - 1].accessed[0] == 'X') {
            sprintf(genbuf, "-ERR Message %d has been deleted.", i);
            outs(genbuf);
            return;
        }
        sprintf(genbuf, "+OK %d %d", i, postlen[i - 1]);
        outs(genbuf);
    }

}

void Top()
{                               /* Leeward adds, 98.01.21 */
    int num;
    int ln;

    struct stat st;

    char *ptr;
	char mail_domain[STRLEN];


    if (State != S_LOGIN) {
        outs("-ERR Unknown command: \"top\".");
        return;
    }

    cmd = nextword2(&msg);

    if (*cmd == 0) {
        outs("-ERR Too few arguments for the top command.");
        return;
    }

    num = atoi(cmd);
    if (num <= 0 || totalnum < num) {
        sprintf(genbuf, "-ERR Message %d does not exist.", num);
        outs(genbuf);
        return;
    } else if (fcache[num - 1].accessed[0] == 'X') {
        sprintf(genbuf, "-ERR Message %d has been deleted.", num);
        outs(genbuf);
        return;
    }

    cmd = nextword2(&msg);

    if (*cmd == 0) {
        outs("-ERR Too few arguments for the top command.");
        return;
    }

    ln = atoi(cmd);
    if (ln < 0) {
        sprintf(genbuf, "-ERR Line %d does not exist.", ln);
        outs(genbuf);
        return;
    }


    num--;
    sprintf(genbuf, "+OK %d octets", postlen[num]);
    outs(genbuf);
    sprintf(genbuf, "mail/%c/%s/%s", toupper(*LowUserid), LowUserid, fcache[num].filename);
    lstat(genbuf, &st); /* use lstat instead of stat - atppp 20041023 */
    ptr = ctime(&st.st_mtime);
    /*       Wed Jan 21 17:42:14 1998            -- ctime returns
             012345678901234567890123            -- offsets
       Date: Wed, 21 Jan 1998 17:54:33 +0800     -- RFC wants     */
    sprintf(genbuf, "Received: from insidesmtp (unknown [127.0.0.1]); %3.3s, %2.2s %3.3s %4.4s %8.8s +0800", ptr + 0, ptr + 8, ptr + 4, ptr + 20, ptr + 11);
    outs(genbuf);
    sprintf(genbuf, "Date: %3.3s, %2.2s %3.3s %4.4s %8.8s +0800", ptr + 0, ptr + 8, ptr + 4, ptr + 20, ptr + 11);
    outs(genbuf);
	snprintf(mail_domain, sizeof(mail_domain), "@%s", MAIL_BBSDOMAIN);
    if (index(fcache[num].owner, '@') == NULL) {
        if ((ptr = strchr(fcache[num].owner, ' ')) != NULL)
            *ptr = '\0';
        sprintf(genbuf,"%s: %s%s", ownerTag, fcache[num].owner, mail_domain);
    }
    else
    	sprintf(genbuf, "%s: %s", ownerTag, fcache[num].owner);
    outs(genbuf);
    sprintf(genbuf, "%s: %s%s", selfTag, getCurrentUser()->userid, mail_domain);
    outs(genbuf);
    sprintf(genbuf, "Subject: %s", fcache[num].title);
    outs(genbuf);
    outs("");
    sprintf(genbuf, "mail/%c/%s/%s", toupper(*LowUserid), LowUserid, fcache[num].filename);
    outfile(genbuf, ln);

}


void Uidl()
{
    int i;

    if (State != S_LOGIN) {
        outs("-ERR Unknown command: \"uidl\".");
        return;
    }

    cmd = nextword2(&msg);

    if (*cmd == 0) {
        outs("+OK");
        for (i = 0; i < totalnum; i++) {
            if (fcache[i].accessed[0] == ' ') {
                sprintf(genbuf, "%d %s", i + 1, fcache[i].filename);
                outs(genbuf);
            }
        }
        outs(".");
    } else {
        i = atoi(cmd);
        if (i <= 0 || totalnum < i) {
            sprintf(genbuf, "-ERR Message %d does not exist.", i);
            outs(genbuf);
            return;
        } else if (fcache[i - 1].accessed[0] == 'X') {
            sprintf(genbuf, "-ERR Message %d has been deleted.", i);
            outs(genbuf);
            return;
        }
        sprintf(genbuf, "+OK %d %s", i, fcache[i - 1].filename);
        outs(genbuf);
    }

}


void Pass()
{
    if (State == S_LOGIN) {
        outs("-ERR Unknown command: \"pass\".");
        return;
    }

    /* Leeward: 97.12.27: enable password include space(s) */
    /*cmd = nextword(&msg); */
    cmd = nextwordX(&msg);

    if (*cmd == 0) {
        outs("-ERR Too few arguments for the pass command.");
        return;
    }

    if (LowUserid[0] == '\0') {
        outs("-ERR need a USER");
        return;
    }

    if (!checkpasswd2(cmd, getCurrentUser())) {
        sprintf(genbuf, "-ERR Password supplied for \"%s\" is incorrect.", LowUserid);
        outs(genbuf);
        LowUserid[0] = '\0';
        BBSlog_usies("ERROR PASSWD");
        logattempt(getCurrentUser()->userid, getSession()->fromhost, "pop3"); /* Leeward 98.07.25 */
        return;
    }

    if (State == S_CONNECT) {
        BBSlog_usies("ENTER");
        State = S_LOGIN;
    }

    Login_init();
    sprintf(genbuf, "+OK %s has %d message(s) (%d octets).", LowUserid, totalnum, totalbyte);
    outs(genbuf);
}

void Last()
{
    if (State != S_LOGIN) {
        outs("-ERR Unknown command: \"last\".");
        return;
    }

    sprintf(genbuf, "+OK %d is the last message seen.", totalnum);
    outs(genbuf);
}

void Dele()
{
    int num;

    if (State != S_LOGIN) {
        outs("-ERR Unknown command: \"dele\".");
        return;
    }

    cmd = nextword2(&msg);

    if (*cmd == 0) {
        outs("-ERR Too few arguments for the dele command.");
        return;
    }

    num = atoi(cmd);
    if (num <= 0 || totalnum < num) {
        sprintf(genbuf, "-ERR Message %d does not exist.", num);
        outs(genbuf);
        return;
    } else if (fcache[num - 1].accessed[0] == 'X') {
        sprintf(genbuf, "-ERR Message %d has already been deleted.", num);
        outs(genbuf);
        return;
    } else if ('M' == fcache[num - 1].accessed[1]) {
        sprintf(genbuf, "-ERR Message %d has already been marked:operation cancelled.", num);
        outs(genbuf);           /* Leeward 99.01.28 */
        return;
    }
    num--;

    fcache[num].accessed[0] = 'X';
    markdel++;
    sprintf(genbuf, "+OK Message %d has been deleted.", num);
    outs(genbuf);
}

void do_delete()
{
    int i, fdr, fdw, count;
    char fpath[80], fnew[80];

    sprintf(fpath, "mail/%c/%s/%s", toupper(*LowUserid), LowUserid, MailBoxName);
    sprintf(fnew, "mail/%c/%s/%s.pop3", toupper(*LowUserid), LowUserid, MailBoxName);
    if ((fdr = open(fpath, O_RDONLY)) == -1)
        return;
    if ((fdw = open(fnew, O_RDWR | O_CREAT, 0644)) == -1)
        return;
    i = count = 0;
    while (read(fdr, &currentmail, sizeof(currentmail))) {
        if (i >= totalnum || fcache[i].accessed[0] == ' ') {
            write(fdw, &currentmail, sizeof(currentmail));
            count++;
        } else {
            sprintf(genbuf, "mail/%c/%s/%s", toupper(*LowUserid), LowUserid, currentmail.filename);
            unlink(genbuf);
        }
        i++;
    }
    close(fdr);
    close(fdw);
    unlink(fpath);
    if (count) {
        f_mv(fnew, fpath);
    } else
        unlink(fnew);
}

void Quit()
{
    if (State == S_LOGIN) {
        if (markdel) {
            do_delete();
            getuser(LowUserid, &getCurrentUser());/* 注意原来的currentuser是指向内部的临时变量*/
            get_mailusedspace(getCurrentUser(),1);
        }
        free(fcache);
        free(postlen);
    }
    BBSlog_usies("EXIT");
    sprintf(genbuf, "+OK KBS BBS POP3/POP3S server at %s signing off.", NAME_BBS_ENGLISH);
    outs(genbuf);
    close(sock);
    exit(0);
}
