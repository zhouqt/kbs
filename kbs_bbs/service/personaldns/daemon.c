#include "bbs.h"

#ifdef HAVE_PERSONAL_DNS
extern int update_dns(const char *server, const char *zone, const char *keyname,
    const char *key, const char *host, const char *ip, int ttl); 

char update_key[100];
char update_keyname[20];
char dns_server[50];
char dns_zone[50];

int reread;
int readconfig() {
    strncpy(dns_zone,sysconf_eval("DNS_UPDATE_ZONE","me.smth.cn"),50);
    if (dns_zone[0]==0) {
    	printf("please configure dns_update_key!\n");
    	return -1;
    }

    strncpy(update_keyname,sysconf_eval("DNS_UPDATE_KEYNAME",""),20);
    if (update_keyname[0]==0) {
    	printf("please configure dns_update_key!\n");
    	return -1;
    }

    strncpy(update_key,sysconf_eval("DNS_UPDATE_KEY",""),99);
    if (update_key[0]==0) {
    	printf("please configure dns updat_key!\n");
    	return -1;
    }
    strncpy(dns_server,sysconf_eval("DNS_UPDATE_SERVER","166.111.8.237"),50);
    if (dns_server[0]==0) {
    	printf("please configure dns update server!\n");
    	return -1;
    }
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
    struct bbs_msgbuf *msg;

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
        if ((msg = rcvlog(msqid)) != NULL) {
	    update_dns(dns_server, dns_zone,
			update_keyname, update_key,
					msg->userid, msg->ip, 60); 	
        }
        if (reread) 
            if (readconfig()!=0) {
            	bbslog("3error","bbsupdated config error");
            }
    }
#else
    return 0;
#endif
}
