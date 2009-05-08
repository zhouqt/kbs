#include "bbs.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "netinet/in.h"
#include <arpa/inet.h>
#include "pthread.h"

#define LOG_BOARD "ScoreClub"

static void log_score(char* userid, char* oid, int type, int mode, int old, int value)
{
    char fname[256], title[80], *p;
    int new = old;
    time_t current;
    FILE *fp;
    unsigned char accessed[2];
    if (mode == 0)
        new = value;
    else if (mode == 1)
        new = old + value;
    else if (mode == -1)
        new = old - value;
    current = time(0);
    p = ctime(&current);
    if (type == 2)
        sprintf(title, "scored: [%s] 接收到未知模式", userid);
    else if ((type == 3) || (type == 4))
        sprintf(title, "scored: [%s] 积分不足", userid);
    else if (type == 5)
        sprintf(title, "scored: [%s] 用户不存在", userid);
    else
        sprintf(title, "修改%s积分<%s>: [%s] %d->%d", (type == 0) ? "用户" : "管理", oid, userid, old, new);
    sprintf(fname, "tmp/scored_%ld_%s", current, oid);
    fp = fopen(fname, "w");
    if (fp) {
        if ((type == 3) || (type == 4))
            fprintf(fp, "请求扣除 %s 的%s积分 %d 分，但该用户只有 %d 分，无效请求。\n", userid, (type == 3) ? "用户" : "管理", value, old);
        else if (type < 2)
            fprintf(fp, "%-12.12s [%c] $%-12.12s [%-15.15s]  %7u -> %-7u  :%+d\n", userid, (type == 0) ? 'U' : 'M', "(scored)", &p[4], old, new, new - old);
        fprintf(fp, "\nOID = %s\n", oid);
        fclose(fp);
        accessed[0] = (type < 2) ? FILE_PERCENT : 0;
        accessed[1] = 0;
        post_file_alt(fname, NULL, title, LOG_BOARD, NULL, 0x05, accessed);
        unlink(fname);
    }
    return;
}

static int operate_score(struct userec *user, char* oid, char* opt)
{
#ifdef NEWSMTH
    int type, mode, value, score;
    char *ptr;
    if (!user)
        return 0;
    if ((opt[0] == 'u') || (opt[0] == 'U'))
        type = 0;
    else if ((opt[0] == 'm') || (opt[0] == 'M'))
        type = 1;
    else {
        log_score(user->userid, oid, 2, 0, 0, 0);
        return 1;
    }

    if (opt[1] == '+')
        mode = 1;
    else if (opt[1] == '-')
        mode = -1;
    else
        mode = 0;

    ptr = opt + 1;
    if (mode != 0)
        ptr++;
    value = atoi(ptr);
    score = (type == 0) ? user->score_user : user->score_manager;
    if (mode == 0) {
        log_score(user->userid, oid, type, mode, score, value);
        score = value;
    } else if (mode == 1) {
        log_score(user->userid, oid, type, mode, score, value);
        score += value;
    } else if (mode == -1) {
        if (score >= value) {
            log_score(user->userid, oid, type, mode, score, value);
            score -= value;
        } else {
            log_score(user->userid, oid, type + 3, mode, score, value);
            return 2;
        }
    }
    if (type == 0)
        user->score_user = score;
    else
        user->score_manager = score;
#endif /* NEWSMTH */
    return 0;
}

int do_score(int fd)
{
    char buf[256], userid[16], oid[30], opt[20];
    struct userec *u;
    int uid, ret;

    if (sock_readline(fd, buf, 255) < 4) return 0;
    if (strncmp(buf, "USR:", 4)) return 0;
    strncpy(userid, buf + 4, 15);
    userid[15]='\0';

    if (sock_readline(fd, buf, 255) < 4) return 0;
    if (strncmp(buf, "OID:", 4)) return 0;
    strncpy(oid, buf + 4, 19);
    oid[19]='\0';

    if (sock_readline(fd, buf, 255) < 4) return 0;
    if (strncmp(buf, "OPT:", 4)) return 0;
    strncpy(opt, buf + 4, 19);
    opt[19]='\0';

    while (sock_readline(fd, buf, 255) > 0);

    buf[0]='\0';
    if ((uid = getuser(userid, &u)) <= 0) {
        strcpy(buf, "ERRO: No such user\n");
        log_score(userid, oid, 5, 0, 0, 0);
    } else {
        ret = operate_score(u, oid, opt);
        if (ret == 1)
            strcpy(buf, "ERRO: Unknown operation\n");
        else if (ret == 2)
            strcpy(buf, "ERRO: Not enough score\n");
        else
            strcpy(buf, "OK: Score modified\n");
    }

    write(fd, buf, strlen(buf));
    return 0;
}

void* pt_score(void *arg)
{
    int fd = (int)(long)arg;

    pthread_detach(pthread_self());

    do_score(fd);
    close(fd);
    return NULL;
}

int main(int argc, char **argv)
{
    struct sockaddr_in sin;
    int sockfd;
    int val;
    int csock;

    setuid(BBSUID);
    setreuid(BBSUID, BBSUID);
    setgid(BBSGID);
    setregid(BBSGID, BBSGID);

    init_all();

    if (dodaemon("scored", true, true)) {
        printf("can not be daemonized, maybe another authd is already running.\n");
    }

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(1124);
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        printf("error listen to port 1124\n");
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
        pthread_create(&pt, NULL, pt_score, (void*)(long)csock);
    }

    close(sockfd);
}

