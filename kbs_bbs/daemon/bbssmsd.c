#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include "bbs.h"

#undef perror
#undef printf

#ifndef byte
typedef unsigned char byte;
#endif

#define SMS_SHM_SIZE 1024*50

struct header{
  char Type;
  byte SerialNo[4];
  byte pid[4];
  byte BodyLength[4];   //×ÜPacket³¤¶È
};

#define CMD_LOGIN 1
#define CMD_OK 101
#define CMD_ERR 102
#define CMD_LOGOUT 2
#define CMD_REG 3
#define CMD_CHECK 4
#define CMD_UNREG 5
#define CMD_REQUIRE 6
#define CMD_REPLY 7
#define CMD_BBSSEND 8
#define CMD_GWSEND 9

#define MOBILE_NUMBER_LEN 17
#define USER_LEN 20
#define PASS_LEN 50

struct LoginPacket { //Type=1
    char user[USER_LEN];
    char pass[PASS_LEN];
};
struct RegMobileNoPacket { //Type=3
    char MobileNo[MOBILE_NUMBER_LEN];
};
struct CheckMobileNoPacket { //Type=4
    char MobileNo[MOBILE_NUMBER_LEN];
    char ValidateNo[MOBILE_NUMBER_LEN];
};
struct UnRegPacket { //Type=5
    char MobileNo[MOBILE_NUMBER_LEN];
};
struct RequireBindPacket { //Type = 6
    byte UserID[4];
    char MobileNo[MOBILE_NUMBER_LEN];
    char Bind;
};
struct ReplyBindPacket { //Type=7
    char MobileNo[MOBILE_NUMBER_LEN];
    char isSucceed;
};
struct BBSSendSMS { //Type=8
    byte UserID[4];
    char SrcMobileNo[MOBILE_NUMBER_LEN];
    char DstMobileNo[MOBILE_NUMBER_LEN];
    byte MsgTxtLen[4];
};
struct GWSendSMS { //Type=9
    byte UserID[4];
    char SrcMobileNo[MOBILE_NUMBER_LEN];
    byte MsgTxtLen[4];
};

struct sms_shm_head {
    int sem;
    int total;
    int length;
} * head;
void * buf;
int sockfd;
int sn=0;
struct header h;

inline unsigned int byte2long(byte arg[4]) {
    unsigned int tmp;
    tmp=((long)arg[0]<<24)+((long)arg[1]<<16)+((long)arg[2]<<8)+(long)arg[3];
    return tmp;
}

inline void long2byte(unsigned int num, byte* arg) {
    (arg)[0]=num>>24;
    (arg)[1]=(num<<8)>>24;
    (arg)[2]=(num<<16)>>24;
    (arg)[3]=(num<<24)>>24;
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
    p = attach_shm("SMS_SHMKEY", 8914, SMS_SHM_SIZE+sizeof(sms_shm_head), &iscreate);
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
    n = getdtablesize();
    if (fork())
        exit(0);
    if (setsid() == -1) {
        perror("setsid");
        exit(-1);
    }
    signal(SIGHUP, SIG_IGN);
    if (fork())
        exit(0);
    save_daemon_pid();
    while (n)
        close(--n);
    for (n = 1; n <= NSIG; n++)
        signal(n, SIG_IGN);
}

void loginas(char* user, char* pass)
{
    struct header h;
    struct LoginPacket p;
    strcpy(p.user, user);
    strcpy(p.pass, pass);
    h.Type = 0;
    long2byte(sn++, h.SerialNo);
    long2byte(sizeof(p), h.BodyLength);
    long2byte(0, h.pid);
    write(sockfd, &h, sizeof(h));
    write(sockfd, &p, sizeof(p));
}

int sendtouser(struct GWSendSMS * h, char* buf)
{
    return 0;
}

void processremote()
{
    unsigned int pid;
    struct header reth;
    char fn[80];
    char buf[1024*10];
    FILE* fp;
    long2byte(reth.SerialNo, byte2long(h.SerialNo));
    long2byte(reth.BodyLength, 0);
    pid=byte2long(h.pid);
    sprintf(fn, "tmp/%d.res", pid);
    switch(h.Type) {
        case CMD_OK:
            if(pid) {
                fp=fopen(fn, "w");
                fprintf(fp, "1");
                fclose(fp);
                kill(pid, SIGUSR1);
            }
            break;
        case CMD_ERR:
            if(pid) {
                fp=fopen(fn, "w");
                fprintf(fp, "0");
                fclose(fp);
                kill(pid, SIGUSR1);
            }
            break;
        case CMD_REQUIRE:
            struct RequireBindPacket h1;
            read(sockfd, &h1, sizeof(h1));
            break;
        case CMD_GWSEND:
            struct GWSendSMS h2;
            read(sockfd, &h2, sizeof(h2));
            read(sockfd, buf, byte2long(h2.MsgTxtLen));
            if(sendtouser(&h2), buf) reth.Type = CMD_ERR;
            else reth.Type = CMD_OK;
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
    if(head->sem) return;
    if(!head->total) return;
    head->sem=1;
    while(head->total) {
        head->total--;
        getbuf(&h, sizeof(h));
        long2byte(sn++, h.SerialNo);
        switch(h.Type) {
            case CMD_REG:
                struct RegMobileNoPacket h1;
                getbuf(&h1, sizeof(h1));
                write(sockfd, &h, sizeof(h));
                write(sockfd, &h1, sizeof(h1));
                break;
            case CMD_CHECK:
                struct CheckMobileNoPacket h2;
                getbuf(&h2, sizeof(h2));
                write(sockfd, &h, sizeof(h));
                write(sockfd, &h2, sizeof(h2));
                break;
            case CMD_UNREG:
                struct UnRegPacket h3;
                getbuf(&h3, sizeof(h3));
                write(sockfd, &h, sizeof(h));
                write(sockfd, &h3, sizeof(h3));
                break;
            case CMD_BBSSEND:
                struct BBSSendSMS h4;
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
    int rc,remain=0;
    start_daemon();
    init_memory();
    if(sockfd=socket(AF_INET, SOCK_STREAM, 0)==-1) {
        printf("Unable to create socket.\n");
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    addr.sin_port=htons(12345);
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0) {
        close(sockfd);
        printf("Unable to connect.\n");
        return -1;
    }
    loginas("12","abc");

    while(1) {
        rc = read(sockfd, ((void*)&h)+remain, sizeof(h)-remain);
        if(rc<0) break;
        remain+=rc;
        if(remain==sizeof(h)) {
            remain=0;
            processremote();
        }
        processbbs();
    }
    
    close(sockfd);
    return 0;
}
