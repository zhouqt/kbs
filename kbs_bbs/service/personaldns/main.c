#include "bbs.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#ifdef HAVE_PERSONAL_DNS
extern int update_dns(const char *server, const char *zone, const char *keyname,
    const char *key, const char *host, const char *ip, int ttl); 

char update_key[100];
char update_keyname[20];
char dns_server[50];
char dns_zone[50];

unsigned int dns_ttl;
FILE* nsupdate_pipe=NULL;

int reread;
int getconf(char* key,char* value,int len) {
    char* data;
    data=sysconf_str(key);
    if (data==NULL) {
    	return -1;
    }
    strncpy(value,data,len-1);
    value[len-1]=0;
    return 0;
}

int readconfig() {
    if (nsupdate_pipe!=NULL) {
        fprintf(nsupdate_pipe,"quit\n");
        pclose(nsupdate_pipe); 
    }
    if (getconf("DNS_UPDATE_ZONE",dns_zone,50)!=0) {
    	bbslog("3error","please configure dns_update_key!\n");
	return -1;
    }

    if (getconf("DNS_UPDATE_KEYNAME",update_keyname,20)!=0) {
    	bbslog("3error","please configure dns_update_key!\n");
    	return -1;
    }

    if (getconf("DNS_UPDATE_KEY",update_key,99)!=0) {
    	bbslog("3error","please configure dns updat_key!\n");
    	return -1;
    }
    if (getconf("DNS_UPDATE_SERVER",dns_server,50)!=0) {
    	bbslog("3error","please configure dns update server!\n");
    	return -1;
    }
    dns_ttl=sysconf_eval("DNS_TTL", 60);

    nsupdate_pipe=popen("nsupdate 2>&1 > reclog/nsupdate.log", "w");
    if (nsupdate_pipe==NULL) {
        bbslog("3error","can't open nsupdate:%s",strerror(errno));
        return -1;
    }
    fprintf(nsupdate_pipe,"server %s\n",dns_server);
    fprintf(nsupdate_pipe,"zone %s\n",dns_zone);
    fprintf(nsupdate_pipe,"key %s %s\n",update_keyname,update_key);
    fflush(nsupdate_pipe);
    return 0;
}

static void reconfig(int signo)
{
    reread=1;
}

static void reaper()
{
    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}

#endif

int main()
{
#ifdef HAVE_PERSONAL_DNS
    int msqid, i;

    struct sigaction act;
    struct msqid_ds msqds;
    struct dns_msgbuf msg;

    umask(027);

    chdir(BBSHOME);
    reread=0; 

    sigaction(SIGHUP, &act, NULL);
    act.sa_handler = reconfig;
    sigaction(SIGPIPE, &act, NULL);
    act.sa_handler = reconfig;
#ifdef AIX
    act.sa_handler = NULL;
    act.sa_flags = SA_RESTART | SA_NOCLDWAIT;
    sigaction(SIGCHLD, &act, NULL);
#else
    act.sa_handler = reaper;
    act.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &act, NULL);
#endif

    setuid(BBSUID);
    setreuid(BBSUID, BBSUID);
    setgid(BBSGID);
    setregid(BBSGID, BBSGID);
    if (readconfig()!=0) return -1;
    dodaemon("bbsnsupdated", true, false);
    
    msqid = msgget(sysconf_eval("BBSDNS_MSG", 0x999), IPC_CREAT | 0664);
    if (msqid < 0)
        return -1;
    msgctl(msqid, IPC_STAT, &msqds);
    msqds.msg_qbytes = 50 * 1024;
    msgctl(msqid, IPC_SET, &msqds);
    while (1) {
        int retv;
        retv = msgrcv(msqid, &msg, sizeof(msg)-sizeof(msg.mtype), 0, MSG_NOERROR);
        if (retv < 0) {
            if (errno==EINTR)
                continue;
            else {
                bbslog("3error","bbsupdated(rcvlog):%s",strerror(errno));
                exit(0);
            }
        }

        fprintf(nsupdate_pipe,"update delete %s A\n",msg.userid);
        fprintf(nsupdate_pipe,"update add %s %d A %s\n",msg.userid,dns_ttl,msg.ip);
        fprintf(nsupdate_pipe,"send\n");
        fflush(nsupdate_pipe);
	bbslog("3error","update dns %s %s",msg.userid,msg.ip);

        if (reread) 
            if (readconfig()!=0) {
            	bbslog("3error","bbsupdated config error");
            }
    }
    fprintf(nsupdate_pipe,"quit\n");
    pclose(nsupdate_pipe);
#else
    return 0;
#endif
}
