#include "bbs.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "netinet/in.h"
#include <arpa/inet.h>
#include "pthread.h"

int do_auth(int fd)
{
    char buf[256];
    char userid[16];
    char passwd[PASSLEN+1];
    char from[16];
    struct userec *u;
    int uid;
    int num;

    if (sock_readline(fd, buf, 255) < 4) return 0;
    if (strncmp(buf, "USR:", 4)) return 0;
    strncpy(userid, buf+4, 16);
    userid[15]='\0';

    if (sock_readline(fd, buf, 255) < 4) return 0;
    if (strncmp(buf, "PWD:", 4)) return 0;
    strncpy(passwd, buf+4, PASSLEN);
    passwd[PASSLEN]='\0';

    if (sock_readline(fd, buf, 255) < 4) return 0;
    if (strncmp(buf, "FRM:", 4)) return 0;
    strncpy(from, buf+4, 16);
    from[15]='\0';

    while (sock_readline(fd, buf, 255) > 0);

    buf[0]='\0';
    if ((uid=getuser(userid, &u)) <=0) {
        strcpy(buf, "ERR: No such user\n");
    } else if (check_ip_acl(u->userid, from)) {
        strcpy(buf, "ERR: Ip not allowed\n");
    } else if (!strcmp(userid, "guest")) {
    } else if (!checkpasswd2(passwd, u)) {
        logattempt(u->userid, from, "authd");
        strcpy(buf, "ERR: passwd error\n");
#ifndef SOLEE
    } else if (!(u->flags & ACTIVATED_FLAG)) {
        strcpy(buf, "ERR: not activated\n");
#endif
    }

    if (buf[0]) {
        write(fd, buf, strlen(buf));
        return 0;
    }

    sprintf(buf, "OK:%s\n", u->userid);
    write(fd, buf, strlen(buf));
    sprintf(buf, "UID:%d\n", uid);
    write(fd, buf, strlen(buf));
    sprintf(buf, "PRM:%s\n", XPERMSTR);
    for (num = 0; num < strlen(XPERMSTR); num++) {
        if (!(u->userlevel & (1 << num)))
            buf[4+num] = '-';
    }
    write(fd, buf, strlen(buf));
    //u->lastlogin = time(NULL);

    return 0;
}

void* pt_auth(void *arg)
{
    int fd = (int)(long)arg;

    pthread_detach(pthread_self());

    do_auth(fd);
    close(fd);
    return NULL;
}

int main(int argc, char **argv)
{
    struct sockaddr_in sin;
    int sockfd;
    int val;
    int csock;

    chdir(BBSHOME);
    resolve_ucache();
    resolve_utmp();

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(1123);
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        printf("error listen to port 1123\n");
        exit(0);
    }

    val = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(val));

    if ((bind(sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0) || (listen(sockfd, 10000) < 0)) {
        exit(1);
    }

    val = sizeof(sin);
    while (1) {
        pthread_t pt;
        csock = accept(sockfd, (struct sockaddr *) &sin, (socklen_t *) & val);
        if (csock < 0)
            continue;
        pthread_create(&pt, NULL, pt_auth, (void*)(long)csock);
    }

    close(sockfd);
}

