#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include "bbs.h"

#ifdef SMS_SUPPORT
#undef perror
#undef printf

void * buf;
int sockfd;
int sn=0;
struct header h;
int running;

void do_exit_sig(int sig)
{
    running=0;
}

void save_daemon_pid()
{
    FILE *fp;

    if ((fp = fopen("var/bbssmsd.pid", "w")) == NULL) {
        perror("fopen");
        exit(-1);
    }
    fprintf(fp, "%d", getpid());
    fclose(fp);
}

void init_memory()
{
    void * p;
    int iscreate;

    iscreate = 0;
    p = attach_shm("SMS_SHMKEY", 8914, SMS_SHM_SIZE+sizeof(struct sms_shm_head), &iscreate);
    head = (struct sms_shm_head *) p;
    buf = p+sizeof(struct sms_shm_head);
    if (iscreate == 0)
        bbslog("4system", "loaded an existed smsshm");
    head->sem = 0;
    head->total = 0;
    head->length = 0;
}

void start_daemon()
{
    int n;

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);
/*    n = getdtablesize();
    if (fork())
        exit(0);
    if (setsid() == -1) {
        perror("setsid");
        exit(-1);
    }
    signal(SIGHUP, SIG_IGN);*/
    if (fork())
        exit(0);
    save_daemon_pid();
}

void loginas(char* user, char* pass)
{
    struct header h;
    struct LoginPacket p;
    strcpy(p.user, user);
    strcpy(p.pass, pass);
    h.Type = CMD_LOGIN;
    long2byte(sn++, h.SerialNo);
    long2byte(sizeof(p), h.BodyLength);
    long2byte(0, h.pid);
    printf("send CMD_LOGIN\n");
    write(sockfd, &h, sizeof(h));
    write(sockfd, &p, sizeof(p));
}

int sendtouser(struct GWSendSMS * h, char* buf)
{
    int uid;
    char * uident;
    struct user_info * uin;
    struct msghead hh;
    uid = byte2long(h->UserID);
    uident = getuserid2(uid);
    if(uident == NULL)
        return -1;
    uin = t_search(uident, NULL);
    if(uin == NULL)
        return -1;

    hh.frompid = -1;
    hh.topid = uin->pid;
    hh.mode = 6;
    hh.sent = 0;
    hh.time = time(0);
    strncpy(hh.id, h->SrcMobileNo, IDLEN+2);
    hh.id[IDLEN+1] = 0;
    save_msgtext(uident, &hh, buf);
    kill(uin->pid, SIGUSR2);
    return 0;
}

int requiretouser(struct RequireBindPacket * h, unsigned int sn)
{
    char uident[IDLEN+2];
    char buf[21];
    struct user_info * uin;
    struct msghead hh;
    strncpy(uident, h->cUserID, IDLEN+2);
    uident[IDLEN+1]=0;
    uin = t_search(uident, NULL);
    if(uin == NULL)
        return -1;

    hh.frompid = sn;
    hh.topid = uin->pid;
    hh.mode = 6;
    hh.sent = 0;
    hh.time = time(0);
    strncpy(hh.id, h->MobileNo, IDLEN+2);
    hh.id[IDLEN+1] = 0;
    if(h->Bind) strcpy(buf, "REQUIRE:BIND");
    else strcpy(buf, "REQUIRE:UNBIND");
    save_msgtext(uident, &hh, buf);
    kill(uin->pid, SIGUSR2);
    return 0;
}

void processremote()
{
    unsigned int pid;
    struct header reth;
    char fn[80];
    char buf[1024*10];
    FILE* fp;
    struct RequireBindPacket h1;
    struct GWSendSMS h2;
    long2byte(byte2long(h.SerialNo), reth.SerialNo);
    long2byte(0, reth.BodyLength);
    pid=byte2long(h.pid);
    sprintf(fn, "tmp/%d.res", pid);
    switch(h.Type) {
        case CMD_OK:
            printf("get CMD_OK\n");
            if(pid) {
                fp=fopen(fn, "w");
                fprintf(fp, "1\n0");
                fclose(fp);
                kill(pid, SIGUSR1);
            }
            break;
       case CMD_ERR:
	case CMD_ERR_HEAD_LENGTH:
	case CMD_ERR_DB:
	case CMD_ERR_SMS_VALIDATE_FAILED:
	case CMD_ERR_LENGTH:
	case CMD_ERR_NO_VALIDCODE:
	case CMD_ERR_NO_SUCHMOBILE:
	case CMD_ERR_REGISTERED:
	    printf("get CMD_ERR\n");
            if(pid) {
                fp=fopen(fn, "w");
                fprintf(fp, "0\n%d", h.Type);
                fclose(fp);
                kill(pid, SIGUSR1);
            }
            break;
        case CMD_REQUIRE:
	    printf("get CMD_REQUIRE\n");
            read(sockfd, &h1, sizeof(h1));
            requiretouser(&h1, byte2long(h.SerialNo));
            break;
        case CMD_GWSEND:
	    printf("get CMD_GWSEND\n");
            read(sockfd, &h2, sizeof(h2));
            read(sockfd, buf, byte2long(h2.MsgTxtLen));
            buf[byte2long(h2.MsgTxtLen)] = 0;
            if(sendtouser(&h2, buf)) {
                reth.Type = CMD_ERR;
                printf("send CMD_ERR\n");
            }
            else {
                reth.Type = CMD_OK;
                printf("send CMD_OK\n");
            }
            write(sockfd, &reth, sizeof(reth));
            break;
    }
}

void getbuf(void * h, int s)
{
    if(head->length<s) return;
    if(h)
        memcpy(h, buf, s);
    memcpy(buf, buf+s, head->length-s);
    head->length-=s;
}

void processbbs()
{
    struct header h;
    struct RegMobileNoPacket h1;
    struct CheckMobileNoPacket h2;
    struct UnRegPacket h3;
    struct BBSSendSMS h4;
    struct ReplyBindPacket h5;
    if(head->sem) return;
    if(!head->total) return;
    head->sem=1;
    while(head->total) {
        head->total--;
        getbuf(&h, sizeof(h));
        if(h.Type!=CMD_REPLY)
            long2byte(sn++, h.SerialNo);
        switch(h.Type) {
            case CMD_REG:
                printf("send CMD_REG\n");
                getbuf(&h1, sizeof(h1));
                write(sockfd, &h, sizeof(h));
                write(sockfd, &h1, sizeof(h1));
                break;
            case CMD_CHECK:
                printf("send CMD_CHECK\n");
                getbuf(&h2, sizeof(h2));
                write(sockfd, &h, sizeof(h));
                write(sockfd, &h2, sizeof(h2));
                break;
            case CMD_UNREG:
                printf("send CMD_UNREG\n");
                getbuf(&h3, sizeof(h3));
                write(sockfd, &h, sizeof(h));
                write(sockfd, &h3, sizeof(h3));
                break;
            case CMD_REPLY:
                printf("send CMD_REPLY\n");
                getbuf(&h5, sizeof(h5));
                write(sockfd, &h, sizeof(h));
                write(sockfd, &h5, sizeof(h5));
                break;
            case CMD_BBSSEND:
                printf("send CMD_BBSSEND\n");
                getbuf(&h4, sizeof(h4));
                write(sockfd, &h, sizeof(h));
                write(sockfd, &h4, sizeof(h4));
                write(sockfd, buf, byte2long(h4.MsgTxtLen));
                getbuf(NULL, byte2long(h4.MsgTxtLen));
                break;
        }
    }

    head->sem=0;
}

int main()
{
    struct sockaddr_in addr;
    fd_set readset;
    struct timeval to;
    int rc,remain=0,retr;
    struct sigaction act;

    start_daemon();
    load_sysconf();
    resolve_ucache();
    resolve_utmp();
    init_memory();
    running=0;
    errno=0;

    bzero(&act, sizeof(act));
    act.sa_handler = do_exit_sig;
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGABRT, &act, NULL);
    
    while (running) {
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))==-1) {
        printf("Unable to create socket.\n");
        shmdt(head);
        buf=NULL;
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(sysconf_str("SMS_ADDRESS"));
    addr.sin_port=htons(sysconf_eval("SMS_PORT", 4002));
    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr))<0) {
        close(sockfd);
        printf("Unable to connect.\n");
        shmdt(head);
        buf=NULL;
        return -1;
    }
    loginas(sysconf_str("SMS_USERNAME"),sysconf_str("SMS_PASSWORD"));

    while(1) {
	FD_ZERO(&readset);
        FD_SET(sockfd, &readset);
	to.tv_sec = 1;
	to.tv_usec = 0;
        if((retr=select(sockfd+1, &readset, NULL, NULL, &to))<0) break;
        if(retr) {
            if (FD_ISSET(sockfd, &readset)) {
                rc = read(sockfd, ((void*)&h)+remain, sizeof(h)-remain);
				/* add by roy 2003.7.30 */
				if (rc==0) {
					break;
				}
                if(rc<0) break;
                remain+=rc;
                if(remain==sizeof(h)) {
                    remain=0;
                    processremote();
                }
            }
        }
        processbbs();
    }
    
    close(sockfd);
        }
    shmdt(head);
    buf=NULL;
    return 0;
}
#else
main()
{
}
#endif
