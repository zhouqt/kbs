#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include "bbs.h"

#undef perror
#undef printf

#ifdef SOLARIS
# include <string.h>            /* For prototype of `strlen'.  */

/* Evaluate to actual length of the `sockaddr_un' structure.  */
# define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)        \
						  + strlen ((ptr)->sun_path))
#endif

static msglist_t *msgshm;
void save_daemon_pid()
{
    FILE *fp;

    if ((fp = fopen("tmp/webmsgd.pid", "w")) == NULL) {
        perror("fopen");
        exit(-1);
    }
    fprintf(fp, "%d", getpid());
    fclose(fp);
}

void init_memory()
{
    int iscreate;

    iscreate = 0;
    msgshm = attach_shm("MSG_SHMKEY", 5200, sizeof(msglist_t) * WWW_MAX_LOGIN, &iscreate);
    if (iscreate == 0)
        bbslog("4system", "loaded an existed msgshm");
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

int init_socket()
{
    struct sockaddr_un s_un;
    int sockfd;
    char path[80];

    bzero(&s_un, sizeof(s_un));
    snprintf(path, sizeof(path), BBSHOME "/.msgd");
    unlink(path);
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(-1);
    }
    s_un.sun_family = AF_UNIX;
    strncpy(s_un.sun_path, path, sizeof(s_un.sun_path) - 1);
	s_un.sun_path[sizeof(s_un.sun_path) - 1] = '\0';
    if (bind(sockfd, (struct sockaddr *) &s_un, SUN_LEN(&s_un)) == -1) {
        perror("bind");
        exit(-1);
    }
    if (listen(sockfd, 5) == -1) {
        perror("listen");
        exit(-1);
    }

    return sockfd;
}

msglist_t *search_msglist(int utmpnum, char *userid)
{
    msglist_t *ml;
    int i;

    for (i = 0; i < WWW_MAX_LOGIN; i++) {
        ml = msgshm + i;
        if (ml->utmpnum == utmpnum && !strcmp(ml->userid, userid))
            return ml;
    }

    return NULL;
}

/*
 * 用于把消息字符串保存到用户的消息列表中
 */
int save_msg(int destutmp, char *destid, int srcutmp, char *srcid, 
			 time_t sndtime, char *msg)
{
    msglist_t *ml;

    if ((ml = search_msglist(destutmp, destid)) == NULL)
        return -1;
    if (ml->msgnum >= MSG_NUM)
        return -1;
    if (ml->msgnum == 0 && ml->current == 0) {
        strncpy(ml->msgs[0].srcid, srcid, IDLEN);
        ml->msgs[0].srcid[IDLEN] = '\0';
        ml->msgs[0].srcutmp = srcutmp;
		ml->msgs[0].sndtime = sndtime;
        strncpy(ml->msgs[0].msg, msg, MSG_LEN);
        ml->msgs[0].msg[MSG_LEN] = '\0';
        ml->msgnum++;
        ml->current++;
    } else {
        int i;

        i = (ml->msgnum + ml->current) % MSG_NUM;
        if (i == 0)
            i = MSG_NUM;
        --i;
        strncpy(ml->msgs[i].srcid, srcid, IDLEN);
        ml->msgs[i].srcid[IDLEN] = '\0';
        ml->msgs[i].srcutmp = srcutmp;
		ml->msgs[i].sndtime = sndtime;
        strncpy(ml->msgs[i].msg, msg, MSG_LEN);
        ml->msgs[i].msg[MSG_LEN] = '\0';
        ml->msgnum++;
    }

    return 0;
}

/*
 * 从用户的消息列表中取出一条消息
 */
int get_msg(int destutmp, char *destid, int *srcutmp, char *srcid, 
			time_t *sndtime, char *msg)
{
    msglist_t *ml;
    int i;

    if ((ml = search_msglist(destutmp, destid)) == NULL)
        return -1;
    if (ml->msgnum == 0)
        return -1;
    i = ml->current % MSG_NUM;
    if (i == 0)
        i = MSG_NUM;
    --i;
    *srcutmp = ml->msgs[i].srcutmp;
	*sndtime = ml->msgs[i].sndtime;
    strncpy(srcid, ml->msgs[i].srcid, IDLEN);
    srcid[IDLEN] = '\0';
    strncpy(msg, ml->msgs[i].msg, MSG_LEN);
    msg[MSG_LEN] = '\0';
    ml->msgnum--;
    ml->current++;

    return 0;
}

/*
 * 从 msglist 中分配一个空闲的 msglist entry
 */
int alloc_msglist_ent(int utmpnum, char *userid)
{
    msglist_t *ml;
    int i;

    if (search_msglist(utmpnum, userid) != NULL)
        return -1;
    for (i = 0; i < WWW_MAX_LOGIN; i++) {
        ml = msgshm + i;
        if (ml->utmpnum == 0 && ml->userid[0] == '\0') {
            ml->utmpnum = utmpnum;
            strncpy(ml->userid, userid, IDLEN);
            ml->userid[IDLEN] = '\0';
            ml->msgnum = 0;
            ml->current = 0;
            return 0;
        }
    }

    return -1;
}

/*
 * 从 msglist 中释放一个已占用的 msglist entry
 */
int free_msglist_ent(int utmpnum, char *userid)
{
    msglist_t *ml;

    if ((ml = search_msglist(utmpnum, userid)) == NULL)
        return -1;
    ml->utmpnum = 0;
    ml->userid[0] = '\0';
    return 0;
}

/*
 * 专用于 request type 为 MSGD_NEW 的情况
 * 在 msglist 中为用户分配 entry，如果允许的话。
 */
int new_user(bbsmsg_t * msgbuf)
{
    char *ptr;
    char *ptr2;
    int destutmp;
    char destid[IDLEN + 1];

    assert(msgbuf != NULL);
    if (msgbuf->type != MSGD_NEW)
        return -1;
    /* rawdata should be "NEW destid destutmp\n" */
    if ((ptr = strchr(msgbuf->rawdata, ' ')) == NULL)
        return -1;
    *ptr++ = '\0';
    if ((ptr2 = strchr(ptr, ' ')) == NULL)
        return -1;
    *ptr2++ = '\0';
    strncpy(destid, ptr, sizeof(destid) - 1);
    destid[sizeof(destid) - 1] = '\0';
    destutmp = atoi(ptr2);
    if (alloc_msglist_ent(destutmp, destid) < 0) {
        msgbuf->type = MSGD_ERR;
        snprintf(msgbuf->rawdata, sizeof(msgbuf->rawdata), "ERR Allocating user %s failed\n", destid);
        write_response(msgbuf);
        return -1;
    }
    msgbuf->type = MSGD_HLO;
    snprintf(msgbuf->rawdata, sizeof(msgbuf->rawdata), "HLO Allocating user %s succeeded\n", destid);
    write_response(msgbuf);

    return 0;
}

/*
 * 专用于 request type 为 MSGD_DEL 的情况
 * 在 msglist 中释放该用户占用的 entry，如果允许的话。
 */
int delete_user(bbsmsg_t * msgbuf)
{
    char *ptr;
    char *ptr2;
    int destutmp;
    char destid[IDLEN + 1];

    assert(msgbuf != NULL);
    if (msgbuf->type != MSGD_DEL)
        return -1;
    /* rawdata should be "DEL destid destutmp\n" */
    if ((ptr = strchr(msgbuf->rawdata, ' ')) == NULL)
        return -1;
    *ptr++ = '\0';
    if ((ptr2 = strchr(ptr, ' ')) == NULL)
        return -1;
    *ptr2++ = '\0';
    strncpy(destid, ptr, sizeof(destid) - 1);
    destid[sizeof(destid) - 1] = '\0';
    destutmp = atoi(ptr2);
    if (free_msglist_ent(destutmp, destid) < 0) {
        msgbuf->type = MSGD_ERR;
        snprintf(msgbuf->rawdata, sizeof(msgbuf->rawdata), "ERR User %s(%d) not found\n", destid, destutmp);
        write_response(msgbuf);
        return -1;
    }
    msgbuf->type = MSGD_BYE;
    snprintf(msgbuf->rawdata, sizeof(msgbuf->rawdata), "BYE Freeing user %s(%d) succeeded\n", destid, destutmp);
    write_response(msgbuf);

    return 0;
}

/*
 * 专用于 request type 为 200 的情况
 * 读入 client 发送来的 msg，并写入消息列表中，如果允许的话。
 */
/*
 * 这里有一个问题，read_msg() 如何把读出来的消息传递出来。
 * 为了结构的清晰考虑，read_msg() 里面不允许包含发送消息的代码，
 * 发送消息的任务应该交给 write_msg() 来处理。只好再定义一个
 * msgent2_t 结构用于 read_msg() 和 write_msg() 交换信息。
 */
int read_msg(bbsmsg_t * msgbuf)
{
    char *ptr;
    char *ptr2;
    int destutmp;
    char destid[IDLEN + 1];
    int srcutmp;
    char srcid[IDLEN + 1];
	time_t sndtime;

    assert(msgbuf != NULL);
    if (msgbuf->type != MSGD_SND)
        return -1;
    /* rawdata should be "SND destid destutmp srcid srcutmp sndtime\n" */
    if ((ptr = strchr(msgbuf->rawdata, ' ')) == NULL)
        return -1;
    *ptr++ = '\0';
    if ((ptr2 = strchr(ptr, ' ')) == NULL)
        return -1;
    *ptr2++ = '\0';
    strncpy(destid, ptr, sizeof(destid) - 1);
    destid[sizeof(destid) - 1] = '\0';
    destutmp = atoi(ptr2);
    if ((ptr = strchr(ptr2, ' ')) == NULL)
        return -1;
    *ptr++ = '\0';
    if ((ptr2 = strchr(ptr, ' ')) == NULL)
        return -1;
    *ptr2++ = '\0';
    strncpy(srcid, ptr, sizeof(srcid) - 1);
    srcid[sizeof(srcid) - 1] = '\0';
    srcutmp = atoi(ptr2);
    if ((ptr = strchr(ptr2, ' ')) == NULL)
        return -1;
    *ptr++ = '\0';
	sndtime = atoi(ptr);
    /* 检查 srcid 能否发消息给 destid */
    /*
     * The webmsgd program doesn't check whether destid can send message to
	 * srcid or not. The sendmsgfunc() function does the checking.
     */
    msgbuf->type = MSGD_OK;
    snprintf(msgbuf->rawdata, sizeof(msgbuf->rawdata), "OK Ready to receive %s(%d)'s message\n", destid, destutmp);
    write_response(msgbuf);

    if (read_request(msgbuf->sockfd, msgbuf) < 0)
        return -1;
    if (msgbuf->type != MSGD_MSG)
        return -1;
    /* rawdata should be "MSG msgstr\n" */
    if ((ptr = strchr(msgbuf->rawdata, ' ')) == NULL)
        return -1;
    *ptr++ = '\0';
    if ((ptr2 = strrchr(ptr, '\n')) != NULL)
        *ptr2 = '\0';
    if (save_msg(destutmp, destid, srcutmp, srcid, sndtime, ptr) < 0) {
        msgbuf->type = MSGD_ERR;
        snprintf(msgbuf->rawdata, sizeof(msgbuf->rawdata), "ERR Saving %s(%d)'s message failed\n", destid, destutmp);
        write_response(msgbuf);
        return -1;
    }
    msgbuf->type = MSGD_OK;
    snprintf(msgbuf->rawdata, sizeof(msgbuf->rawdata), "OK Saving %s(%d)'s message succeeded\n", destid, destutmp);
    write_response(msgbuf);

    return 0;
}

/* 
 * 专用于 request type 为 210 的情况
 * 另外，如果 web 端用户发送消息给 telnet 端用户的话，
 * 将绕过 webmsgd，直接调用 sendmsgfunc() 发送给 telnet 端用户。
 */
int write_msg(bbsmsg_t * msgbuf)
{
    char *ptr;
    char *ptr2;
    int destutmp;
    char destid[IDLEN + 1];
    int srcutmp;
    char srcid[IDLEN + 1];
    char buf[MSG_LEN + 1];
	time_t sndtime;

    assert(msgbuf != NULL);
    if (msgbuf->type != MSGD_RCV)
        return -1;
    /* rawdata should be "RCV destid destutmp\n" */
    if ((ptr = strchr(msgbuf->rawdata, ' ')) == NULL)
        return -1;
    *ptr++ = '\0';
    if ((ptr2 = strchr(ptr, ' ')) == NULL)
        return -1;
    *ptr2++ = '\0';
    strncpy(destid, ptr, sizeof(destid) - 1);
    destid[sizeof(destid) - 1] = '\0';
    destutmp = atoi(ptr2);
    if (get_msg(destutmp, destid, &srcutmp, srcid, &sndtime, buf) < 0) {
        msgbuf->type = MSGD_ERR;
        snprintf(msgbuf->rawdata, sizeof(msgbuf->rawdata), 
				 "ERR Geting %s(%d)'s message failed\n", destid, destutmp);
        write_response(msgbuf);
        return -1;
    }
    msgbuf->type = MSGD_FRM;
    snprintf(msgbuf->rawdata, sizeof(msgbuf->rawdata), "FRM %s %d %d\n", 
			 srcid, srcutmp, sndtime);
    write_response(msgbuf);
    if (read_request(msgbuf->sockfd, msgbuf) < 0)
        return -1;
    if (msgbuf->type != MSGD_OK)
        return -1;
    msgbuf->type = MSGD_MSG;
    snprintf(msgbuf->rawdata, sizeof(msgbuf->rawdata), "MSG %s\n", buf);
    write_response(msgbuf);

    return 0;
}

void process_request(int clientfd)
{
    bbsmsg_t msgbuf;

    if (read_request(clientfd, &msgbuf) < 0)
        return;
    switch (msgbuf.type) {
    case MSGD_NEW:
        new_user(&msgbuf);
        break;
    case MSGD_DEL:
        delete_user(&msgbuf);
        break;
    case MSGD_SND:
        read_msg(&msgbuf);
        break;
    case MSGD_RCV:
        write_msg(&msgbuf);
        break;
    default:
    }
}

int main()
{
    int sockfd;
    int connfd;
    socklen_t len;
    struct sockaddr_un cun;

    start_daemon();
    init_memory();
    sockfd = init_socket();
    /* non-fork() implementation */
    for (;;) {
        len = sizeof(cun);
        if ((connfd = accept(sockfd, (struct sockaddr *) &cun, &len)) == -1)
            continue;
        process_request(connfd);
        close(connfd);
    }

    return 0;
}
