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
    if (getconf("DNS_UPDATE_ZONE",dns_zone,50)!=0) {
    	printf("please configure dns_update_key!\n");
	return -1;
    }

    if (getconf("DNS_UPDATE_KEYNAME",update_keyname,20)!=0) {
    	printf("please configure dns_update_key!\n");
    	return -1;
    }

    if (getconf("DNS_UPDATE_KEY",update_key,99)!=0) {
    	printf("please configure dns updat_key!\n");
    	return -1;
    }
    if (getconf("DNS_UPDATE_SERVER",dns_server,50)!=0) {
    	printf("please configure dns update server!\n");
    	return -1;
    }
    dns_ttl=sysconf_eval("DNS_TTL", 60);
    return 0;
}

static void reconfig(int signo)
{
    reread=1;
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
    if (readconfig()!=0) return -1;
    sigaction(SIGHUP, &act, NULL);
    act.sa_handler = reconfig;
    setuid(BBSUID);
    setreuid(BBSUID, BBSUID);
    setgid(BBSGID);
    setregid(BBSGID, BBSGID);
    dodaemon("bbsnsupdated", true, true);
    
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

	update_dns(dns_server, dns_zone,
		update_keyname, update_key,
		msg.userid, msg.ip, dns_ttl); 	
	bbslog("3error","update dns %s %s",msg.userid,msg.ip);
        if (reread) 
            if (readconfig()!=0) {
            	bbslog("3error","bbsupdated config error");
            }
    }
#else
    return 0;
#endif
}
