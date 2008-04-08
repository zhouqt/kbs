#include "stdio.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "netinet/in.h"
#include <arpa/inet.h>
#include "unistd.h"
#include "stdlib.h"
#include "string.h"

#include "libauth.h"

static int sock_readline(int socket, char *buf, unsigned int size){
    unsigned int n=0;
    while(1){
        if(read(socket, buf+n, 1) <= 0){
            return -1;
        }
        n++;
        if(n >= size) { buf[n]='\0'; return n; }
        if(buf[n-1]=='\r'){ n--; continue; }
        if(buf[n-1]=='\n'){ buf[n-1]='\0'; return n-1; }
    }   
}       

int bbs_auth(struct bbsauth *a, const char *userid, const char *passwd, char *fromip){
        int ret;
		int uid=0;
        char buf[256];
        struct sockaddr_in sin;
		int sockfd;

        sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        if(sockfd < 0)
            return -2;

        sin.sin_family = AF_INET;
        sin.sin_port = htons(1123);
        inet_aton("127.0.0.1", &sin.sin_addr);

        if(connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
                close(sockfd);
                return -3;
        }

        sprintf(buf, "USR:%s\n", userid);
        if(write(sockfd, buf, strlen(buf))<0) return -3;
        sprintf(buf, "PWD:%s\n", passwd);
        if(write(sockfd, buf, strlen(buf))<0) return -3;
        sprintf(buf, "FRM:%s\n", fromip);
        if(write(sockfd, buf, strlen(buf))<0) return -3;
        write(sockfd, "\n", 1);

        ret = sock_readline(sockfd, buf, 255);
        if(ret < 0){
            close(sockfd);
            return -4;
        }
        if(strncmp(buf, "OK:", 3) || strcasecmp(userid, buf+3) ){
			close(sockfd);
			return -1;
		}

		strncpy(a->userid, buf+3, 16);
		a->userid[14]='\0';

                while(1){
                        int i;
                        ret = sock_readline(sockfd, buf, 255);
                        if(ret < 4) break;
						if(!strncmp(buf, "UID:", 4)){
							uid = atoi(buf+4);
						}
						if(!strncmp(buf, "PRM:", 4)){
							strncpy(a->perm, buf+4, 32);
							a->perm[32]='\0';
						}
                }
        close(sockfd);
        return uid;
}
