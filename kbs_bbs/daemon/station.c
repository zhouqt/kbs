/* 聊天室 Server */
#include "bbs.h"
#include "chat.h"
#include <sys/ioctl.h>
#ifdef lint
#include <sys/uio.h>
#endif


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef LINUX
#include <unistd.h>
#endif

#ifdef AIX
#include <sys/select.h>
#endif

#if !RELIABLE_SELECT_FOR_WRITE
#include <fcntl.h>
#endif

#if USES_SYS_SELECT_H
#include <sys/select.h>
#endif
#define perror(x) perror(x)

#if NO_SETPGID
#define setpgid setpgrp
#endif


#define RESTRICTED(u)   (users[(u)].flags == 0) /* guest */
#define SYSOP(u)        (users[(u)].flags & PERM_SYSOP)
#define CHATOP(u)       (users[(u)].flags & PERM_CHATOP)
#define ROOMOP(u)       (users[(u)].flags & PERM_CHATROOM)

#define ROOM_LOCKED     0x1
#define ROOM_SECRET     0x2
#define ROOM_NOEMOTE    0x4
#define ROOM_SYSOP      0x8
#define LOCKED(r)       (rooms[(r)].flags & ROOM_LOCKED)
#define SECRET(r)       (rooms[(r)].flags & ROOM_SECRET)
#define NOEMOTE(r)      (rooms[(r)].flags & ROOM_NOEMOTE)
#define SYSOPED(r)      (rooms[(r)].flags & ROOM_SYSOP)


#define ROOM_ALL        (-2)
#define PERM_CHATROOM PERM_CHAT
#define CHATMAXACTIVE  200
/*---	period modify IDLEN+1 ==> IDLEN+2	2000-10-18	---*/
struct chatuser {
    int sockfd;                 /* socket to bbs server */
    int utent;                  /* utable entry for this user */
    int room;                   /* room: -1 means none, 0 means main */
    int flags;
    char userid[IDLEN + 2 /*1 */ ];     /* real userid */
    char chatid[9];             /* chat id */
    char ibuf[128];             /* buffer for sending/receiving */
    int ibufsize;               /* current size of ibuf */
    char lpIgnoreID[MAX_IGNORE][IDLEN + 2 /*1 */ ];     /* Ignored-ID list,added by Luzi 97.11 */
    char lpEmote[MAX_EMOTES][129];      /* emotes list, added by Luzi 97.12.13 */
} users[CHATMAXACTIVE];


struct chatroom {
    char name[IDLEN];           /* name of room; room 0 is "main" */
    short occupants;            /* number of users in room */
    short flags;                /* ROOM_LOCKED, ROOM_SECRET */
    char invites[CHATMAXACTIVE];        /* Keep track of invites to rooms */
    char topic[52];             /* Let the room op to define room topic */
} rooms[MAXROOM];

struct chatcmd {
    char *cmdstr;
    void (*cmdfunc) ();
    int exact;
    int bUsed;                  /* Can this act be used in main-room :added by Luzi 98.1.3 */
};


int sock = -1;                  /* the socket for listening */
int nfds;                       /* number of sockets to select on */
int num_conns;                  /* current number of connections */
fd_set allfds;                  /* fd set for selecting */
struct timeval zerotv;          /* timeval for selecting */
char chatbuf[256];              /* general purpose buffer */
#ifdef FILTER
static int filtering=1;
#endif
/* name of the main room (always exists) */

char mainroom[] = CHAT_MAIN_ROOM;
char maintopic[] = CHAT_TOPIC;

#define ENABLEMAIN 1

char *msg_not_op = CHAT_MSG_NOT_OP;
char *msg_no_such_id = "*** \033[1m%s\033[m 不在本" CHAT_ROOM_NAME "里 ***";
char *msg_not_here = "*** \033[1m%s\033[m 并没有前来" CHAT_SERVER " ***";

#ifdef FILTER
void filter_report(char* title,char *str)
{
	FILE *se;
	char fname[STRLEN];
	struct userec chatuser;

       bzero(&chatuser,sizeof(chatuser));
       strcpy(chatuser.userid,"聊天室报告");
       strcpy(chatuser.username,"聊天室密探");
       chatuser.userlevel=-1;
	sprintf(fname, "tmp/deliver.chatd-report");
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", str);
		fclose(se);
                post_file(&chatuser, "", fname, FILTER_BOARD, title, 0, 2, getSession());
		unlink(fname);
	}
}
#endif

int can_send(int myunum, int unum)
{                               /* added by Luzi 1997.11.30 */
    int i;

    for (i = 0; i < MAX_IGNORE; i++)
        if (users[unum].lpIgnoreID[i][0] != '\0')
            if (!strcasecmp(users[unum].lpIgnoreID[i], users[myunum].userid)
                && !SYSOP(myunum))
                return 0;
    return 1;
}

/* Added by ming, 96.10.12 */
/* although there is strcasecmp() and strncasecmp()
   in solaris c runtime library, it has bugs when it compare Chinese
   code like "文文" and "武武". So we use our own code.
*/

int is_valid_chatid(char *id)
{
    int i;

    if (*id == '\0')
        return 0;
    if (strlen(id) > 8)
        id[8] = 0;
    for (i = 0; i < 8 && *id; i++, id++) {
        if ( /* !isprint(*id) || */ *id == 0x1b || *id == '*' || *id == ':' || *id == '/' || *id == '%' || strstr(id, "\\n") || *id == ']')
            return 0;
    }
    if (!strncmp(id, "◎", 2))
        return 0;
    return 1;
}



char *getnextword(str)
    char **str;
{
    char *p;

    while (Isspace(**str))
        (*str)++;
    p = *str;
    while (**str && !Isspace(**str))
        (*str)++;
    if (**str) {
        **str = '\0';
        (*str)++;
    }
    return p;
}


int chatid_to_indx(chatid)
    char *chatid;
{
    register int i;

    for (i = 0; i < CHATMAXACTIVE; i++) {
        if (users[i].sockfd == -1)
            continue;
        if (!strcasecmp(chatid, users[i].chatid))
            return i;
    }
    return -1;
}

int userid_to_indx(userid)      /* add by Luzi 1997.11.18 */
    char *userid;
{
    register int i;

    for (i = 0; i < CHATMAXACTIVE; i++) {
        if (users[i].sockfd == -1)
            continue;
        if (!strcasecmp(userid, users[i].userid))
            return i;
    }
    return -1;
}

void chat_query(myunum, msg)    /* add by Luzi 1997.11.18 */
    int myunum;
    char *msg;
{
    char modestr[30];
    char *userid;
    int unum;

    userid = getnextword(&msg);
    unum = userid_to_indx(userid);
    if (unum >= 0 && users[unum].room >= 0)
        if ((rooms[users[unum].room].flags & ROOM_SECRET) == 0 || rooms[users[unum].room].invites[myunum]
            || SYSOP(myunum) || CHATOP(myunum)) {
            sprintf(modestr, "1%s", rooms[users[unum].room].name);
            send(users[myunum].sockfd, modestr, strlen(modestr), 0);
            return;
        }
    send(users[myunum].sockfd, "0", 1, 0);
}

void chat_query_ByChatid(myunum, msg)   /* add by dong , 1998.9.12 */
    int myunum;
    char *msg;
{
    char userstr[40];
    char *chatid;
    int unum;

    chatid = getnextword(&msg);
    unum = chatid_to_indx(chatid);
    if (unum >= 0 && users[unum].room >= 0) {
        sprintf(userstr, "1%s", users[unum].userid);
        send(users[myunum].sockfd, userstr, strlen(userstr), 0);
        return;
    }
    send(users[myunum].sockfd, "0", 1, 0);
}

int fuzzy_chatid_to_indx(chatid)        /* Modified by ming, 96.10.10 */
    char *chatid;               /* Fixed a bug by Leeward 99.10.08 */
{
    register int i, indx = -1;
    size_t len = strlen(chatid);

    /* search chatid first */
    for (i = 0; i < CHATMAXACTIVE; i++) {
        if (users[i].sockfd == -1)
            continue;
        if (!strncasecmp(chatid, users[i].chatid, len)) {
            if (len == strlen(users[i].chatid))
                return i;
            if (indx == -1)
                indx = i;
            else
                /*return -2; */
                indx = -2;      /* Leeward 99.10.08 */
        }
    }

    /* Alex said need not compare to userid
       if (indx != -1)
       return indx;

       for (i = 0; i < CHATMAXACTIVE; i++)
       {
       if (users[i].sockfd == -1)
       continue;
       if (!strncasecmp(chatid, users[i].userid, len))
       {
       if (len == strlen(users[i].userid))
       return i;
       if (indx == -1)
       indx = i;
       else
       return -2;
       }
       }
     */
    return indx;
}

int roomid_to_indx(roomid)      /* modify by Luzi 1998.1.16 */
    char *roomid;
{
    register int i;
    char ch = '\0';

    if (strlen(roomid) >= IDLEN) {
        ch = roomid[IDLEN - 1];
        roomid[IDLEN - 1] = '\0';
    }
    for (i = 0; i < MAXROOM; i++) {
        if (i && rooms[i].occupants == 0)
            continue;
        /*    report(roomid);
           report(rooms[i].name); */
        if (!strcasecmp(roomid, rooms[i].name))
            break;
    }
    if (ch)
        roomid[IDLEN - 1] = ch;
    if (i < MAXROOM)
        return i;
    return -1;
}


static void do_send(writefds, str)
    fd_set *writefds;
    char *str;
{
    register int i;
    int len = strlen(str);

    if (select(nfds, NULL, writefds, NULL, &zerotv) > 0) {
        for (i = 0; i < nfds; i++)
            if (FD_ISSET(i, writefds))
                send(i, str, len + 1, 0);
    }
}


void send_to_room(room, str, unum)
    int room;
    char *str;
    int unum;
{
    int i;
    fd_set writefds;

    FD_ZERO(&writefds);
#ifdef FILTER
    if (filtering) {
    	if (check_badword_str(str, strlen(str), getSession())) {
    		char title[80];
    		char content[256];
    		sprintf(title,"%s 在聊天室说坏话",users[unum].userid);
    		sprintf(content,"%s\n%s(聊天代号 %s )说:%s",Ctime(time(NULL)),
			users[unum].userid,
    			users[unum].chatid,str);
    		filter_report(title, content);
                FD_SET(users[unum].sockfd, &writefds);
                do_send(&writefds, str);
    		return;
    	}
    }
#endif
    for (i = 0; i < CHATMAXACTIVE; i++) {
        if (users[i].sockfd == -1)
            continue;
        /*    if (room == ROOM_ALL || room == users[i].room) */
        if (room == ROOM_ALL || (room == users[i].room && can_send(unum, i)))
            /* write(users[i].sockfd, str, strlen(str) + 1); */
            FD_SET(users[i].sockfd, &writefds);
    }
    do_send(&writefds, str);
}

/* add by KCN for support multi-line emote */
void send_to_room2(roomid, chatbuf, unum)
    int roomid;
    char *chatbuf;
    int unum;
{
    int i;
    char *tmpbuf, *tmpbuf2, *party2;

    i = 0;
    tmpbuf = chatbuf;
    party2 = strchr(chatbuf, ' ');
    while ((i < 6) && (tmpbuf2 = strstr(party2, "\\n"))) {
        *tmpbuf2 = 0;
        *(tmpbuf2 + 1) = ' ';
        send_to_room(roomid, tmpbuf, unum);
        *tmpbuf2 = ' ';
        *(tmpbuf2 - 1) = ' ';
        *(tmpbuf2 - 2) = ' ';
        tmpbuf = tmpbuf2 - 2;
        party2 = tmpbuf2 + 2;
        i++;
    }
    if (*party2)
        send_to_room(roomid, tmpbuf, unum);
}


void send_to_unum(unum, str)
    int unum;
    char *str;
{
    fd_set writefds;

    FD_ZERO(&writefds);
    FD_SET(users[unum].sockfd, &writefds);
    do_send(&writefds, str);
}


void send_to_unum2(unum, chatbuf)
    int unum;
    char *chatbuf;
{
    int i;
    char *tmpbuf, *tmpbuf2, *party2;

    i = 0;
    tmpbuf = chatbuf;
    party2 = strchr(chatbuf, ' ');
    while ((i < 6) && (tmpbuf2 = strstr(party2, "\\n"))) {
        *tmpbuf2 = 0;
        *(tmpbuf2 + 1) = ' ';
        send_to_unum(unum, tmpbuf);
        *tmpbuf2 = ' ';
        *(tmpbuf2 - 1) = ' ';
        *(tmpbuf2 - 2) = ' ';
        tmpbuf = tmpbuf2 - 2;
        party2 = tmpbuf2 + 2;
        i++;
    }
    if (*party2)
        send_to_unum(unum, tmpbuf);
}

void exit_room(unum, disp, msg)
    int unum;
    int disp;
    char *msg;
{
    int oldrnum = users[unum].room;

    if (oldrnum != -1) {
        if (--rooms[oldrnum].occupants) {
            switch (disp) {
            case EXIT_LOGOUT:
                sprintf(chatbuf, "*** \033[1m%s\033[m (%s) 慢慢离开了 ***", users[unum].chatid, users[unum].userid);  /* dong 1998.9.13 */
                if (msg && *msg) {
                    strcat(chatbuf, ": ");
                    strncat(chatbuf, msg, 80);
                }
                break;

            case EXIT_LOSTCONN:
                sprintf(chatbuf, "*** \033[1m%s\033[m (%s) 已" CHAT_MSG_QUIT " ***", users[unum].chatid, users[unum].userid); /* dong 1998.9.13 */
                break;

            case EXIT_KICK:
                /*sprintf(chatbuf, "*** \033[1m%s\033[m 被聊天室op踢出去了 ***", users[unum].chatid); */
                sprintf(chatbuf, "*** \033[1m%s\033[m (%s) 被" CHAT_OP "(%s)踢出去了 ***", users[unum].chatid, users[unum].userid, msg);      /* Leeward 98.03.02 *//* dong 1998.9.13 */
                break;
            }
            send_to_room(oldrnum, chatbuf, unum);
        }
    }
    users[unum].flags &= ~PERM_CHATROOM;
    users[unum].room = -1;
}


void chat_topic(unum, msg)
    int unum;
    char *msg;
{
    int rnum;

    rnum = users[unum].room;

    if (!ROOMOP(unum) && !SYSOP(unum) && !CHATOP(unum)) {
        send_to_unum(unum, msg_not_op);
        return;
    }
    if (rnum == 0 && !SYSOP(unum)) {
        send_to_unum(unum, "◎ 请勿乱改" CHAT_MAIN_ROOM CHAT_ROOM_NAME "的话题 ◎");
        return;
    }
    if (*msg == '\0') {
        send_to_unum(unum, "◎ 请指定话题 ◎");
        return;
    }

    strncpy(rooms[rnum].topic, msg, 52);
    rooms[rnum].topic[51] = '\0';
    sprintf(chatbuf, "/t%.51s", msg);
    send_to_room(rnum, chatbuf, unum);
    sprintf(chatbuf, "*** \033[1m%s\033[m 将话题改为 \033[1m%s\033[m ***", users[unum].chatid, msg);
    send_to_room(rnum, chatbuf, unum);
}

void chat_name_room(unum, msg)
    int unum;
    char *msg;
{
    int rnum;
    char *roomid = getnextword(&msg);

    rnum = users[unum].room;

    if (!ROOMOP(unum) && !SYSOP(unum) && !CHATOP(unum)) {
        send_to_unum(unum, msg_not_op);
        return;
    }
    if (*roomid == '\0') {
        send_to_unum(unum, "◎ 请指定新的" CHAT_ROOM_NAME "名称 ◎");
        return;
    }
    if (rnum == 0) {
        send_to_unum(unum, "◎ 不能更改 " CHAT_MAIN_ROOM " 聊天室的名称 ◎");
        return;
    }
    if (roomid_to_indx(roomid) >= 0) {
        send_to_unum(unum, "◎ 已经有同名的" CHAT_ROOM_NAME "了 ◎");
        return;
    }
    strncpy(rooms[rnum].name, roomid, IDLEN - 1);
    rooms[rnum].name[IDLEN - 1] = '\0';
    sprintf(chatbuf, "/z%s", rooms[rnum].name);
    send_to_room(rnum, chatbuf, unum);
    sprintf(chatbuf, "*** \033[1m%s\033[m 将" CHAT_ROOM_NAME "名称改为 \033[1m%s\033[m ***", users[unum].chatid, rooms[rnum].name);
    send_to_room(rnum, chatbuf, unum);
}

int enter_room(int unum, char *room, char *msg)
{
    int rnum;
    int op = 0;
    register int i;

    rnum = roomid_to_indx(room);
    if (rnum == -1) {
        /* new room */
        for (i = 1; i < MAXROOM; i++) {
            if (rooms[i].occupants == 0) {
                /*        report("new room"); */
                rnum = i;
                memset(rooms[rnum].invites, 0, CHATMAXACTIVE);
                strcpy(rooms[rnum].topic, maintopic);
                strncpy(rooms[rnum].name, room, IDLEN - 1);
                rooms[rnum].name[IDLEN - 1] = '\0';
                rooms[rnum].flags = 0;
                op++;
                break;
            }
        }
        if (rnum == -1) {
            send_to_unum(unum, "◎ 我们的" CHAT_ROOM_NAME "满了喔 ◎");
            return 0;
        }
    }
    if (!SYSOP(unum) && !CHATOP(unum))
        if (LOCKED(rnum) && rooms[rnum].invites[unum] == 0) {
            send_to_unum(unum, "◎ 本" CHAT_ROOM_NAME "商讨机密中，非请勿入 ◎");
            return 0;
        }
    if (!SYSOP(unum))
        if (SYSOPED(rnum) && rooms[rnum].invites[unum] == 0) {
            send_to_unum(unum, "◎ 本" CHAT_ROOM_NAME "绝密，非请勿入 ◎");
            return 0;
        }

    exit_room(unum, EXIT_LOGOUT, msg);
    users[unum].room = rnum;
    if (op)
        users[unum].flags |= PERM_CHATROOM;
    rooms[rnum].occupants++;
    rooms[rnum].invites[unum] = 0;
    sprintf(chatbuf, "*** \033[1m%s\033[m (%s) 进入 \033[1m%s\033[m " CHAT_ROOM_NAME " ***", users[unum].chatid, users[unum].userid, rooms[rnum].name);
    send_to_room(rnum, chatbuf, unum);
    sprintf(chatbuf, "/r%s", room);
    send_to_unum(unum, chatbuf);
    sprintf(chatbuf, "/t%s", rooms[rnum].topic);
    send_to_unum(unum, chatbuf);
    return 0;
}


void logout_user(unsigned int unum)
{
    int i, sockfd = users[unum].sockfd;

    close(sockfd);
    FD_CLR(sockfd, &allfds);
    memset(&users[unum], 0, sizeof(users[unum]));
    users[unum].sockfd = users[unum].utent = users[unum].room = -1;
    for (i = 0; i < MAXROOM; i++) {
        if (rooms[i].invites != NULL)
            rooms[i].invites[unum] = 0;
    }
    num_conns--;
}

int print_user_counts(unsigned int unum)
{
    int i, c, userc = 0, suserc = 0, roomc = 0;

    for (i = 0; i < MAXROOM; i++) {
        c = rooms[i].occupants;
        if (i > 0 && c > 0) {
            if (!SECRET(i) || SYSOP(unum))
                roomc++;
        }
        c = users[i].room;
        if (users[i].sockfd != -1 && c != -1) {
            if (SECRET(c) && !SYSOP(unum))
                suserc++;
            /* else modified by dong, 1998.10.6 */
            userc++;
        }
    }
    sprintf(chatbuf, "★ 欢迎光临【%s】" CHAT_SERVER "，目前已经有 %d 间" CHAT_ROOM_NAME "有客人 ★ ", NAME_BBS_CHINESE, roomc + 1);
    send_to_unum(unum, chatbuf);
    sprintf(chatbuf, "★ 本聊" CHAT_SERVER "内共有 %d 人 ", userc + 1);
    if (suserc)
        sprintf(chatbuf + strlen(chatbuf), "[其中 %d 人在秘密" CHAT_ROOM_NAME "]", suserc);
    sprintf(chatbuf + strlen(chatbuf), "★");
    send_to_unum(unum, chatbuf);
    if (!ENABLEMAIN)
        send_to_unum(unum, "\033[1m在 \033[33mmain\033[m\033[1m 大厅里禁止聊天、作小动作，可用 \033[33m/j\033[m\033[1m 指令到其它" CHAT_ROOM_NAME "进行聊天\033[m");
    send_to_unum(unum, "\033[1m输入 \033[33m/e\033[m\033[1m 离开" CHAT_SERVER "，\033[33m/w\033[m\033[1m 查看本" CHAT_ROOM_NAME "中的" NAME_USER_SHORT "\033[m");
    send_to_unum(unum, "\033[1m输入 \033[33m/r\033[m\033[1m 查看当前的" CHAT_ROOM_NAME "，\033[33m/j roomname\033[m\033[1m 进入相应" CHAT_ROOM_NAME "\033[m");
    send_to_unum(unum, "\033[1m输入 \033[33m/h\033[m\033[1m 获得帮助信息\033[m");
    return 0;
}

void get_ignore_list(unum)      /* added by Luzi 1997.11.30 */
    int unum;
{
    int fd;
    int id = 1;

    char path[60];

    sethomefile(path, users[unum].userid, "/ignores");

    /*---	Added by period	2000-10-18	just guessing problems	---*/
    for (id = 0; id < MAX_IGNORE; id++)
        users[unum].lpIgnoreID[id][0] = '\0';
    /*---		---*/

    if ((fd = open(path, O_RDONLY, 0)) == -1) {
        users[unum].lpIgnoreID[0][0] = '\0';
        return;
    }
    for (id = 0; id < MAX_IGNORE; id++)
        if (read(fd, users[unum].lpIgnoreID[id], IDLEN + 1) <= IDLEN) {
            users[unum].lpIgnoreID[id][0] = '\0';
            break;
        }
    close(fd);
    return;
}

void get_emote_list(unum)       /* added by Luzi 1998.1.25 */
    int unum;
{
    int fd;
    int id = 1;

    char path[60];

    sethomefile(path, users[unum].userid, "/emotes");

    if ((fd = open(path, O_RDONLY, 0)) == -1) {
        users[unum].lpEmote[0][0] = '\0';
        if ((fd = open(path, O_CREAT | O_WRONLY, 0664)) != -1) {
            ftruncate(fd, 128 * MAX_EMOTES);
            close(fd);
        }
        return;
    }
    for (id = 0; id < MAX_EMOTES; id++)
        if (read(fd, users[unum].lpEmote[id], 128) <= 127) {
            users[unum].lpEmote[id][0] = '\0';
            break;
        }
    close(fd);
    return;
}

void chat_ignore(unum, msg)     /* add by Luzi 1997.11.28 */
    int unum;
    char *msg;
{
    char *userid;
    short i;

    userid = getnextword(&msg);
    for (i = 0; i < MAX_IGNORE; i++)
        if (users[unum].lpIgnoreID[i][0] == '\0') {
            strcpy(users[unum].lpIgnoreID[i], userid);
            return;
        }
}

void chat_alias_add(unum, msg)  /* add by Luzi 1998.1.25 */
    int unum;
    char *msg;
{
    short i;

    for (i = 0; i < MAX_EMOTES; i++)
        if (users[unum].lpEmote[i][0] == '\0') {
            strcpy(users[unum].lpEmote[i], msg);
            return;
        }
}


void chat_listen(unum, msg)     /* add by Luzi 1997.11.28 */
    int unum;
    char *msg;
{
    char *userid;
    short i;

    userid = getnextword(&msg);
    for (i = 0; i < MAX_IGNORE; i++)
        if (users[unum].lpIgnoreID[i][0] != '\0')
            if (!strcasecmp(users[unum].lpIgnoreID[i], userid)) {
                users[unum].lpIgnoreID[i][0] = '\0';
                return;
            }
}

void chat_alias_del(unum, msg)  /* add by Luzi 1998.1.25 */
    int unum;
    char *msg;
{
    char *emoteid;
    short i, len;

    emoteid = getnextword(&msg);
    len = strlen(emoteid);
    for (i = 0; i < MAX_EMOTES; i++)
        if (users[unum].lpEmote[i][0] != '\0') {
            if (!strncasecmp(users[unum].lpEmote[i], emoteid, len))
                if (users[unum].lpEmote[i][len] == ' ') {
                    users[unum].lpEmote[i][0] = '\0';
                    return;
                }
        }
}

void call_alias(int unum, char *msg)
{                               /* added by Luzi 1998.01.25,change by KCN */
    char buf[128];
    char path[40];

    char *emoteid;
    int nIdx;


    emoteid = getnextword(&msg);

    sethomefile(path, users[unum].userid, "/emotes");
    if (!emoteid[0]) {
        int i, has = 0;

        for (i = 0; i < MAX_EMOTES; i++)
            if (users[unum].lpEmote[i][0] != '\0') {
                if (has == 0) {
                    send_to_unum(unum, "〖用户自定义emote列表〗");
                    has = 1;
                }
                send_to_unum(unum, users[unum].lpEmote[i]);
            }
        if (has == 0)
            send_to_unum(unum, "*** 还没有自定义的emote ***");
    } else {
        short len, has;
        char *arg;

        arg = getnextword(&msg);
        len = strlen(emoteid);
        has = 0;
        for (nIdx = 0; nIdx < MAX_EMOTES; nIdx++)
            if (!strncasecmp(users[unum].lpEmote[nIdx], emoteid, len))
                if (users[unum].lpEmote[nIdx][len] == ' ') {
                    has = 1;
                    break;
                }
        if (has) {
            if ((*arg) && (arg[0])) {
                send_to_unum(unum, "*** 该emote已经被定义过了 ***");
                return;
            }
            if (delete_record(path, 128, nIdx + 1, NULL, NULL) == 0) {
                send_to_unum(unum, "*** 该自定义emote已经被删除了 ***");
                users[unum].lpEmote[nIdx][0] = 0;
            } else {
                send_to_unum(unum, "*** system error ***");
            }
        } else if (!*arg)
            send_to_unum(unum, "*** 请指定emote对应的字串 ***");
        else {
            short has;

            has = 0;
            for (nIdx = 0; nIdx < MAX_EMOTES; nIdx++)
                if (users[unum].lpEmote[nIdx][0] == 0) {
                    has = 1;
                    break;
                }
            if (has) {
                sprintf(buf, "%s %s", emoteid, arg);
                if (substitute_record(path, buf, 128, nIdx + 1) == 0) {
                    send_to_unum(unum, "*** 自定义emote已经设定 ***");
                    strncpy(users[unum].lpEmote[nIdx], buf, 128);
                    users[unum].lpEmote[nIdx][128] = 0;
                } else {
                    send_to_unum(unum, "*** 系统错误 ***");
                }
            } else
                send_to_unum(unum, "*** 用户自定义emote的列表已满 ***");
        }
    }
}

int login_user(int unum, char *msg)
{
    int i, utent;
    char *utentstr;
    char *level;
    char *userid;
    char *chatid;

    utentstr = getnextword(&msg);
    level = getnextword(&msg);
    userid = getnextword(&msg);
    chatid = getnextword(&msg);

    utent = atoi(utentstr);
    for (i = 0; i < CHATMAXACTIVE; i++) {
        if (users[i].sockfd != -1 && users[i].utent == utent) {
            send_to_unum(unum, CHAT_LOGIN_BOGUS);
            return -1;
        }
    }
    if (!is_valid_chatid(chatid)) {
        send_to_unum(unum, CHAT_LOGIN_INVALID);
        return 0;
    }
    if (chatid_to_indx(chatid) != -1) {
        /* userid in use */
        send_to_unum(unum, CHAT_LOGIN_EXISTS);
        return 0;
    }

    /*  report(level); */

    users[unum].utent = utent;
    users[unum].flags = atoi(level);
    strcpy(users[unum].userid, userid);
    strncpy(users[unum].chatid, chatid, 8);
    users[unum].chatid[8] = '\0';
    send_to_unum(unum, CHAT_LOGIN_OK);
    get_emote_list(unum);       /* added by Luzi 1998.01.25 */
    get_ignore_list(unum);      /* added by Luzi 1997.11.28 */
    print_user_counts(unum);
    /*
       sprintf(chatbuf,"User entry:%d",utent);
       send_to_room(mainroom,chatbuf , unum); wrong??? KCN
       send_to_room(0,chatbuf , unum);
     */
    enter_room(unum, mainroom, (char *) NULL);
    return 0;
}


void chat_list_rooms(unum, msg)
    int unum;
    char *msg;
{
    int i, j, pause, occupants;
    char *str;

    j = 0;
    str = getnextword(&msg);
    if (*str == '\0')
        pause = 1;
    else
        pause = 0;

    if (RESTRICTED(unum)) {
        send_to_unum(unum, "◎ 抱歉！" CHAT_SYSTEM "不让你看有哪些房间有客人 ◎");
        return;
    }
    send_to_unum(unum, "\033[33m\033[44m " CHAT_ROOM_NAME "名称  │人数│话题        \033[m");
    for (i = 0; i < MAXROOM; i++) {
        occupants = rooms[i].occupants;
        if (occupants > 0) {
            if (!SYSOP(unum) && !CHATOP(unum))
                if ((rooms[i].flags & ROOM_SECRET) && (users[unum].room != i))
                    continue;
            sprintf(chatbuf, " %-12s│%4d│%s\033[m", rooms[i].name, occupants, rooms[i].topic);
            /*---	period	2000-10-24	add background color to chatroom status	---*/
            if (rooms[i].flags & ROOM_LOCKED)
                strcat(chatbuf, " [\033[30;47m锁住\033[m]");
            if (rooms[i].flags & ROOM_SECRET)
                strcat(chatbuf, " [\033[30;47m秘密\033[m]");
            if (rooms[i].flags & ROOM_NOEMOTE)
                strcat(chatbuf, " [\033[30;47m禁止动作\033[m]");
            if (rooms[i].flags & ROOM_SYSOP)
                strcat(chatbuf, " [\033[30;47m绝密\033[m]");

            send_to_unum(unum, chatbuf);
        }
    }
}


int chat_do_user_list(int unum, char *msg, int whichroom)
{
    char buf[14] = "    [公开]    ";    /*Haohmaru.99.09.18 */
    char buf1[14] = "[秘密]  [锁住]";
    char buf2[14] = "    [秘密]    ";
    char buf3[14] = "    [锁住]    ";
    int start, stop, curr = 0;
    int i, j, rnum, myroom = users[unum].room;

    j = 0;
    while (*msg && Isspace(*msg))
        msg++;
    start = atoi(msg);
    while (*msg && isdigit(*msg))
        msg++;
    while (*msg && !isdigit(*msg))
        msg++;
    stop = atoi(msg);
    /*Haohmaru.99.09.18 */
    if (1 /*users[unum].room != 0 */ ) {
        if ((rooms[whichroom].flags & ROOM_SECRET) && (rooms[whichroom].flags & ROOM_LOCKED))
            sprintf(chatbuf, "\033[33m\033[44m 聊天代号│%6s代号  │聊天室%-14s\033[m", NAME_USER_SHORT, buf1);
        else if (rooms[whichroom].flags & ROOM_SECRET)
            sprintf(chatbuf, "\033[33m\033[44m 聊天代号│%6s代号  │聊天室%-14s\033[m", NAME_USER_SHORT, buf2);
        else if (rooms[whichroom].flags & ROOM_LOCKED)
            sprintf(chatbuf, "\033[33m\033[44m 聊天代号│%6s代号  │聊天室%-14s\033[m", NAME_USER_SHORT, buf3);
        else
            sprintf(chatbuf, "\033[33m\033[44m 聊天代号│%6s代号  │聊天室%-14s\033[m", NAME_USER_SHORT, buf);
        send_to_unum(unum, chatbuf);
    } else
        send_to_unum(unum, "\033[33m\033[44m 聊天代号│" NAME_USER_SHORT "代号  │聊天室 \033[m");
    for (i = 0; i < CHATMAXACTIVE; i++) {
        rnum = users[i].room;
        if (users[i].sockfd != -1 && rnum != -1) {
            if (whichroom != -1 && whichroom != rnum)
                continue;
            if (myroom != rnum) {
                if (RESTRICTED(unum))
                    continue;
                if ((rooms[rnum].flags & ROOM_SECRET) && (rooms[rnum].invites[unum] == 0) && !SYSOP(unum) && !CHATOP(unum))
                    continue;
            }
            curr++;
            if (curr < start)
                continue;
            else if (stop && (curr > stop))
                break;
            sprintf(chatbuf, " %-8s│%-12s│%s", users[i].chatid, users[i].userid, rooms[rnum].name);
            if (ROOMOP(i))
                strcat(chatbuf, " [Op]");
            send_to_unum(unum, chatbuf);
        }
    }
    return 0;
}


void chat_list_by_room(unum, msg)
    int unum;
    char *msg;
{
    int whichroom;
    char *roomstr;

    roomstr = getnextword(&msg);
    if (*roomstr == '\0')
        whichroom = users[unum].room;
    else {
        if ((whichroom = roomid_to_indx(roomstr)) == -1) {
            sprintf(chatbuf, "◎ 没这个" CHAT_ROOM_NAME "喔 ◎");
            send_to_unum(unum, chatbuf);
            return;
        }
        if ((rooms[whichroom].flags & ROOM_SECRET) && !SYSOP(unum) && !CHATOP(unum)) {
            send_to_unum(unum, "◎ 本" CHAT_SERVER "的" CHAT_ROOM_NAME "皆公开的，没有秘密 ◎");
            return;
        }
    }
    chat_do_user_list(unum, msg, whichroom);
}


void chat_list_users(unum, msg)
    int unum;
    char *msg;
{
    chat_do_user_list(unum, msg, -1);
}

void chat_map_chatids(unum, whichroom)
    int unum;
    int whichroom;
{
    int i, c, myroom, rnum;

    myroom = users[unum].room;
    send_to_unum(unum, "\033[33m\033[44m 聊天代号 使用者代号  │ 聊天代号 使用者代号  │ 聊天代号 使用者代号 \033[m");
    for (i = 0, c = 0; i < CHATMAXACTIVE; i++) {
        rnum = users[i].room;
        if (users[i].sockfd != -1 && rnum != -1) {
            if (whichroom != -1 && whichroom != rnum)
                continue;
            if (myroom != rnum) {
                if (RESTRICTED(unum))
                    continue;
                if ((rooms[rnum].flags & ROOM_SECRET) && !SYSOP(unum) && !CHATOP(unum))
                    continue;
            }
            sprintf(chatbuf + (c * 24), " %-8s%c%-12s%s", users[i].chatid, (ROOMOP(i)) ? '*' : ' ', users[i].userid, (c < 2 ? "│" : "  "));
            if (++c == 3) {
                send_to_unum(unum, chatbuf);
                c = 0;
            }
        }
    }
    if (c > 0)
        send_to_unum(unum, chatbuf);
}


void chat_map_chatids_thisroom(unum, msg)
    int unum;
    char *msg;
{
    chat_map_chatids(unum, users[unum].room);
}


void chat_setroom(unum, msg)
    int unum;
    char *msg;
{
    char *modestr;
    int rnum = users[unum].room;
    int sign = 1;
    int flag;
    char *fstr="";

    modestr = getnextword(&msg);
    if (!ROOMOP(unum)) {
        send_to_unum(unum, msg_not_op);
        return;
    }
    if (*modestr == '+')
        modestr++;
    else if (*modestr == '-') {
        modestr++;
        sign = 0;
    }
    if (*modestr == '\0') {
        send_to_unum(unum, "◎ 请告诉" CHAT_SYSTEM "您要的" CHAT_ROOM_NAME "是: {[\033[1m+\033[m(设定)][\033[1m-\033[m(取消)]}{[\033[1ml\033[m(锁住)][\033[1ms\033[m(秘密)]}");
        return;
    }
    while (*modestr) {
        flag = 0;
        switch (*modestr) {
        case 'l':
        case 'L':
            if (!rnum && !SYSOP(unum)) {        /*added by Haohmaru,98.9.6 */
                send_to_unum(unum, "◎ " CHAT_MAIN_ROOM " " CHAT_ROOM_NAME "不能被锁住 ◎");
                break;
            }
            flag = ROOM_LOCKED;
            fstr = "锁住";
            break;
        case 's':
        case 'S':
            if (!rnum && !SYSOP(unum)) {        /*added by Haohmaru,98.9.6 */
                send_to_unum(unum, "◎ " CHAT_MAIN_ROOM CHAT_ROOM_NAME "不能被设为秘密形式 ◎");
                break;
            }
            flag = ROOM_SECRET;
            fstr = "秘密";
            break;
        case 'e':
        case 'E':
            flag = ROOM_NOEMOTE;
            fstr = "'禁止动作'";
            break;
        case 'x':
        case 'X':
            if (SYSOP(unum)) {
                flag = ROOM_SYSOP;
                fstr = "'绝密'";
            }
            break;
#ifdef FILTER
       case 'F':
       case 'f': {
        if (SYSOP(unum)) {        /*added by Haohmaru,98.9.6 */
       	char buf[80];
       	char title[80];
       	filtering=!filtering;
       	if (filtering)
                sprintf(buf, "过滤器已经打开");
       	else
                sprintf(buf, "过滤器已经关闭");
       	send_to_unum(unum,buf);
       	sprintf(title,"%s:%s",users[unum].userid, buf);
       	filter_report(title,buf);
       	return;
       	}
	}
#endif
        default:
            sprintf(chatbuf, "◎ " CHAT_SYSTEM "看不懂你的意思：[\033[1m%c\033[m] ◎", *modestr);
            send_to_unum(unum, chatbuf);
        }
        if (flag && ((rooms[rnum].flags & flag) != sign * flag)) {
            rooms[rnum].flags ^= flag;
            sprintf(chatbuf, "*** 这" CHAT_ROOM_NAME "被 \033[1m%s\033[m %s%s的形式 ***", users[unum].chatid, sign ? "设定为" : "取消", fstr);
            send_to_room(rnum, chatbuf, unum);
        }
        modestr++;
    }
}


void chat_nick(unum, msg)
    int unum;
    char *msg;
{
    char *chatid;
    int othernum;

    chatid = getnextword(&msg);
    if (!is_valid_chatid(chatid)) {
        send_to_unum(unum, "◎ 非法名字，请重新输入 ◎");
        return;
    }
    chatid[8] = '\0';
    othernum = chatid_to_indx(chatid);
    if (othernum != -1 && othernum != unum) {
        /*    send_to_unum(unum, "◎ 抱歉！有人跟你同名，所以你不能进来 ◎"); */
        send_to_unum(unum, "◎ 抱歉！有人已经使用该chatid,所以您不能再使用 ◎");
        return;
    }
    sprintf(chatbuf, "◎ \033[1m%s\033[m 将聊天代号改为 \033[1m%s\033[m ◎", users[unum].chatid, chatid);
    send_to_room(users[unum].room, chatbuf, unum);


    strcpy(users[unum].chatid, chatid);
    sprintf(chatbuf, "/n%s", users[unum].chatid);
    send_to_unum(unum, chatbuf);
}


void chat_private(int unum, char *msg)
{
    char *recipient;
    int recunum;

    recipient = getnextword(&msg);
    recunum = fuzzy_chatid_to_indx(recipient);
    if (recunum < 0) {
        /* no such user, or ambiguous */
        if (recunum == -1)
            sprintf(chatbuf, msg_no_such_id, recipient);
        else
            sprintf(chatbuf, " ◎ 对方叫什麽名字? ◎");
        send_to_unum(unum, chatbuf);
        return;
    }
    if (*msg) {
        if (can_send(unum, recunum)) {
            sprintf(chatbuf, "\033[1m*%s*\033[m ", users[unum].chatid);
            strncat(chatbuf, msg, 80);
            send_to_unum(recunum, chatbuf);
            sprintf(chatbuf, "\033[1m%s>\033[m ", users[recunum].chatid);
            strncat(chatbuf, msg, 80);
            send_to_unum(unum, chatbuf);
        }
    }
}


void put_chatid(int unum, char *str)
{
    int i;
    char *chatid = users[unum].chatid;

    memset(str, ' ', 10);
    for (i = 0; chatid[i]; i++)
        str[i] = chatid[i];
    str[i] = ':';
    str[10] = '\0';
}


int chat_allmsg(int unum, char *msg)
{
    if (*msg) {
        put_chatid(unum, chatbuf);
        strcat(chatbuf, msg);
        send_to_room(users[unum].room, chatbuf, unum);
    }
    return 0;
}


void chat_act(unum, msg)
    int unum;
    char *msg;
{
    if (*msg) {
        sprintf(chatbuf, "\033[1m%s\033[m %s", users[unum].chatid, msg);
        send_to_room2(users[unum].room, chatbuf, unum);
    }
}


void chat_join(unum, msg)
    int unum;
    char *msg;
{
    char *roomid;

    roomid = getnextword(&msg);
    if (RESTRICTED(unum)) {
        send_to_unum(unum, "◎ 你只能在这里聊天 ◎");
        return;
    }
    if (*roomid == '\0') {
        send_to_unum(unum, "◎ 请问哪个" CHAT_ROOM_NAME " ◎");
        return;
    }
    enter_room(unum, roomid, msg);
    return;
}

void chat_kick(unum, msg)
    int unum;
    char *msg;
{
    char *twit;
    int rnum = users[unum].room, rnum2;
    int recunum;

    twit = getnextword(&msg);
    if (!ROOMOP(unum) && !SYSOP(unum) && !CHATOP(unum)) {
        send_to_unum(unum, msg_not_op);
        return;
    }
    if ((recunum = chatid_to_indx(twit)) == -1) {
        sprintf(chatbuf, msg_no_such_id, twit);
        send_to_unum(unum, chatbuf);
        return;
    }
    if (rnum != users[recunum].room && !SYSOP(unum)) {
        sprintf(chatbuf, msg_not_here, users[recunum].chatid);
        send_to_unum(unum, chatbuf);
        return;
    }
    if (SYSOP(recunum) && !SYSOP(unum)) {
        send_to_unum(unum, msg_not_op);
        return;
    }

    /* Leeward 98.04.30 */
    sprintf(chatbuf, "*** 你被" CHAT_OP "(\033[1m\033[33m%s\033[m)踢出来了 ***", users[unum].userid);
    send_to_unum(recunum, chatbuf);

    rnum2 = users[recunum].room;

    /*exit_room(recunum, EXIT_KICK, (char *) NULL); */
    exit_room(recunum, EXIT_KICK, users[unum].userid);  /* Leeward 98.03.02 */

    if (rnum == 0 || rnum != rnum2)
        logout_user(recunum);
    else
        enter_room(recunum, mainroom, (char *) NULL);
}

void chat_kickid(unum, msg)     /*Haohmaru.99.6.9.根据ID踢人 */
    int unum;
    char *msg;
{
    char *twit;
    int rnum = users[unum].room, rnum2;
    int recunum;

    twit = getnextword(&msg);
    if (!ROOMOP(unum) && !SYSOP(unum) && !CHATOP(unum)) {
        send_to_unum(unum, msg_not_op);
        return;
    }
    if ((recunum = userid_to_indx(twit)) == -1) {
        sprintf(chatbuf, msg_no_such_id, twit);
        send_to_unum(unum, chatbuf);
        return;
    }
    if (rnum != users[recunum].room && !SYSOP(unum)) {
        sprintf(chatbuf, msg_not_here, users[recunum].userid);
        send_to_unum(unum, chatbuf);
        return;
    }
    if (SYSOP(recunum) && !SYSOP(unum)) {
        send_to_unum(unum, msg_not_op);
        return;
    }

    /* Leeward 98.04.30 */
    sprintf(chatbuf, "*** 你被" CHAT_OP "(\033[1m\033[33m%s\033[m)踢出来了 ***", users[unum].userid);
    send_to_unum(recunum, chatbuf);

    rnum2 = users[recunum].room;

    /*exit_room(recunum, EXIT_KICK, (char *) NULL); */
    exit_room(recunum, EXIT_KICK, users[unum].userid);  /* Leeward 98.03.02 */

    if (rnum == 0 || rnum != rnum2)
        logout_user(recunum);
    else
        enter_room(recunum, mainroom, (char *) NULL);
}

void chat_makeop(unum, msg)
    int unum;
    char *msg;
{
    char *newop = getnextword(&msg);
    int rnum = users[unum].room;
    int recunum;

    if (!ROOMOP(unum)) {
        send_to_unum(unum, msg_not_op);
        return;
    }
    if ((recunum = chatid_to_indx(newop)) == -1) {
        /* no such user */
        sprintf(chatbuf, msg_no_such_id, newop);
        send_to_unum(unum, chatbuf);
        return;
    }
    if (unum == recunum) {
        sprintf(chatbuf, "◎ 你忘了你本来就是" CHAT_OP "喔 ◎");
        send_to_unum(unum, chatbuf);
        return;
    }
    if (rnum != users[recunum].room) {
        sprintf(chatbuf, msg_not_here, users[recunum].chatid);
        send_to_unum(unum, chatbuf);
        return;
    }
    users[unum].flags &= ~PERM_CHATROOM;
    users[recunum].flags |= PERM_CHATROOM;
    sprintf(chatbuf, "*** \033[1m%s\033[m 决定让 \033[1m%s\033[m 当本" CHAT_ROOM_NAME "的" CHAT_OP " ***", users[unum].chatid, users[recunum].chatid);
    send_to_room(rnum, chatbuf, unum);
}

void chat_toggle(unum, msg)
    int unum;
    char *msg;
{
    char *togglee = getnextword(&msg);
    int rnum = users[unum].room;
    int recunum;

    if (!SYSOP(unum) && !CHATOP(unum)) {
        send_to_unum(unum, msg_not_op);
        return;
    }
    if ((recunum = chatid_to_indx(togglee)) == -1) {
        /* no such user */
        sprintf(chatbuf, msg_no_such_id, togglee);
        send_to_unum(unum, chatbuf);
        return;
    }
    if (rnum != users[recunum].room) {
        sprintf(chatbuf, msg_not_here, users[recunum].chatid);
        send_to_unum(unum, chatbuf);
        return;
    }
    if (ROOMOP(recunum)) {
        users[recunum].flags &= ~PERM_CHATROOM;
        sprintf(chatbuf, "*** \033[1m%s\033[m 取消了 \033[1m%s\033[m 的" CHAT_OP " ***", users[unum].chatid, users[recunum].chatid);
    } else {
        users[recunum].flags |= PERM_CHATROOM;
        sprintf(chatbuf, "*** \033[1m%s\033[m 设置 \033[1m%s\033[m 为本聊天室的" CHAT_OP " ***", users[unum].chatid, users[recunum].chatid);
    }
    send_to_room(rnum, chatbuf, unum);
}

void chat_invite(unum, msg)
    int unum;
    char *msg;
{
    char *invitee = getnextword(&msg);
    int rnum = users[unum].room;
    int recunum;

    if (!ROOMOP(unum)) {
        send_to_unum(unum, msg_not_op);
        return;
    }
    if ((recunum = chatid_to_indx(invitee)) == -1) {
        sprintf(chatbuf, msg_not_here, invitee);
        send_to_unum(unum, chatbuf);
        return;
    }
    if (rooms[rnum].invites[recunum] == 1) {
        sprintf(chatbuf, "*** \033[1m%s\033[m 等一下就来 ***", users[recunum].chatid);
        send_to_unum(unum, chatbuf);
        return;
    }
    rooms[rnum].invites[recunum] = 1;
    sprintf(chatbuf, "*** \033[1m%s\033[m 邀请您到 \033[1m%s\033[m " CHAT_ROOM_NAME "聊天 ***", users[unum].chatid, rooms[rnum].name);
    send_to_unum(recunum, chatbuf);
    sprintf(chatbuf, "*** \033[1m%s\033[m 等一下就来 ***", users[recunum].chatid);
    send_to_unum(unum, chatbuf);
}

/*---	Added by period		2000-09-15	---*/
/*---	NOTICE : send_to_room in SMTH codes need 3 parameters !!!	---*/
void chat_knock_room(int unum, char *msg)
{
    char *roomid;
    const char *outd = "里面";
    const char *ind = "外面";
    int rnum;

    roomid = getnextword(&msg);
    if (RESTRICTED(unum)) {
        send_to_unum(unum, "\033[1;31m◎ \033[37m你只能在这里聊天 \033[31m◎\033[m");
        return;
    }
    if (*roomid == '\0') {
        send_to_unum(unum, "\033[1;31m◎ \033[37m请问哪个" CHAT_ROOM_NAME " \033[31m◎\033[m");
        return;
    }
    rnum = roomid_to_indx(roomid);
    if (rnum == -1) {
        send_to_unum(unum, "\033[1;31m◎ \033[37m 没有这个" CHAT_ROOM_NAME " \033[31m◎\033[m");
        return;
    }
    if ((SECRET(rnum) || NOEMOTE(rnum)) && !SYSOP(unum) && !CHATOP(unum)) {
        send_to_unum(unum, "\033[1;31m◎ \033[37m 请勿打扰，谢谢合作！\033[31m◎\033[m");
        return;
    }
    sprintf(chatbuf, "\033[1;37m◎ \033[31m当当当... \033[33m%s [%s] \033[37m在%s敲门 : \033[32m%s \033[37m◎\033[m",
            users[unum].chatid, users[unum].userid, (rnum == users[unum].room) ? (outd) : (ind), (msg));
    send_to_room(rnum, chatbuf, unum);
    if (rnum != users[unum].room)
        send_to_unum(unum, chatbuf);
    return;
}

/*---	End of Addition	---*/
void chat_broadcast(unum, msg)
    int unum;
    char *msg;
{
    if (!SYSOP(unum) && !CHATOP(unum)) {
        send_to_unum(unum, "◎ 你不可以在" CHAT_SERVER "内大声喧哗 ◎");
        return;
    }
    if (*msg == '\0') {
        send_to_unum(unum, "◎ 广播内容是什麽 ◎");
        return;
    }
    sprintf(chatbuf, "◎ " CHAT_SERVER CHAT_OP " \033[1m%s\033[m 有话对大家宣布：", users[unum].chatid);
    send_to_room(ROOM_ALL, chatbuf, unum);
    sprintf(chatbuf, "【%s】", msg);
    send_to_room(ROOM_ALL, chatbuf, unum);
}


void chat_goodbye(unum, msg)
    int unum;
    char *msg;
{
    exit_room(unum, EXIT_LOGOUT, msg);
}


/* -------------------------------------------- */
/* MUD-like social commands : action             */
/* -------------------------------------------- */


struct action {
    char *verb;                 /* 动词 */
    char *part1_msg;            /* 介词 */
    char *part2_msg;            /* 动作 */
};


struct action party_data[] = {
    {"?", "很疑惑的看着", ""},
    {"admire", "对", "的景仰之情犹如滔滔江水连绵不绝"},
    {"agree", "完全同意", "的看法"},
    {"bearhug", "热情的拥抱", ""},
    {"bless", "祝福", "心想事成"},
    {"bow", "毕躬毕敬的向", "鞠躬"},
    {"bye", "看着", "的背影，凄然泪下。身后的收音机传来邓丽君的歌声:\\n\"\033[31m何日君再来.....\033[m\""},
    {"caress", "轻轻的抚摸", ""},
    {"cat", "像只小猫般地依偎在", "的怀里撒娇。"},
    {"cringe", "向", "卑躬屈膝，摇尾乞怜"},
    {"cry", "向", "嚎啕大哭"},
    {"comfort", "温言安慰", ""},
    {"clap", "向", "热烈鼓掌"},
    {"dance", "拉着", "的手翩翩起舞"},
    {"dogleg", "对", "狗腿"},
    {"drivel", "对著", "流口水"},
    {"dunno", "瞪大眼睛，天真地问：", "，你说什麽我不懂耶... :("},
    {"faint", "晕倒在", "的怀里"},
    {"fear", "对", "露出怕怕的表情"},
    {"fool", "请大家注意", "这个大白痴....\\n真是天上少有....人间仅存的活宝....\\n不看太可惜了！"},
    {"forgive", "大度的对", "说：算了，原谅你了"},
    {"giggle", "对著", "傻傻的呆笑"},
    {"grin", "对", "露出邪恶的笑容"},
    {"growl", "对", "咆哮不已"},
    {"hand", "跟", "握手"},
    {"hammer", "举起好大好大的铁锤！！哇！往",
     "头上用力一敲！\\n***************\\n*  5000000 Pt *\\n***************\\n      | |      %1★%2☆%3★%4☆%5★%6☆%0\\n      | |         好多的星星哟\\n      |_|"},
    {"heng", "看都不看", "一眼， 哼了一声，高高的把头扬起来了,不屑一顾的样子..."},
    {"hug", "轻轻地拥抱", ""},
    {"idiot", "无情地耻笑", "的痴呆。"},
    {"kick", "把", "踢的死去活来"},
    {"kiss", "轻吻", "的脸颊"},
    {"laugh", "大声嘲笑", ""},
    {"lovelook", "拉着", "的手，温柔地默默对视。目光中孕含着千种柔情，万般蜜意"},
    {"nod", "向", "点头称是"},
    {"nudge", "用手肘顶", "的肥肚子"},
    {"oh", "对", "说：“哦，酱子啊！”"},
    {"pad", "轻拍", "的肩膀"},
    {"papaya", "敲了敲", "的木瓜脑袋"},
    {"pat", "轻轻拍拍", "的头"},
    {"pinch", "用力的把", "拧的黑青"},
    {"puke", "对着", "吐啊吐啊，据说吐多几次就习惯了"},
    {"punch", "狠狠揍了", "一顿"},
    {"pure", "对", "露出纯真的笑容"},
    {"qmarry", "向", "勇敢的跪了下来:\\n\"你愿意嫁给我吗？\"\\n---真是勇气可嘉啊"},
    {"report", "偷偷地对", "说：“报告我好吗？”"},
    {"shrug", "无奈地向", "耸了耸肩膀"},
    {"sigh", "对", "叹了一口气"},
    {"slap", "啪啪的巴了", "一顿耳光"},
    {"smooch", "拥吻著", ""},
    {"snicker", "嘿嘿嘿..的对", "窃笑"},
    {"sniff", "对", "嗤之以鼻"},
    {"sorry", "痛哭流涕的请求", "原谅"},
    {"spank", "用巴掌打", "的臀部"},
    {"squeeze", "紧紧地拥抱著", ""},
    {"thank", "向", "道谢"},
    {"tickle", "咕叽!咕叽!搔", "的痒"},
    {"waiting", "深情地对", "说：每年每月的每一天，每分每秒我都在这里等着你"},
    {"wake", "努力的摇摇", "，在其耳边大叫：“快醒醒，会着凉的！”"},
    {"wave", "对著", "拼命的摇手"},
    {"welcome", "热烈欢迎", "的到来"},
    {"wink", "对", "神秘的眨眨眼睛"},
    {"xixi", "嘻嘻地对", "笑了几声"},
    {"zap", "对", "疯狂的攻击"},
    {"inn", "双眼饱含着泪水，无辜的望着", ""},
    {"mm", "色眯眯的对", "问好：“美眉好～～～～”。大色狼啊！！！"},
    {"disapp", "这下没搞头啦，为什么", "姑娘对我这个造型完全没反应？没办法！"},
    {"miss", "真诚的望着", "：我想念你我真的想念你我太--想念你了!你相不相信?"},
    {"buypig", "指着", "：“这个猪头给我切一半，谢谢！”"},
    {"rascal", "对", "大叫：“你这个臭流氓！！！！！！！！！！！！！！！！”"},
    {"qifu", "小嘴一扁，对", "哭道：“你欺负我，你欺负我！！！”"},
    {"wa", "对", "大叫一声：“哇哇哇哇哇哇酷弊了耶！！！！！！！！！！！！！！”"},
    {"feibang", "喔－－！熟归熟，", "你这样乱讲话，我一样可以告你毁谤，哈！"},
    {NULL, NULL, NULL}
};

int alias_action(unum, cmd, party)
    int unum;
    char *cmd;
    char *party;
{
    int i, len, chatlen, recunum;
    char ch, *tmpbuf, *tmpbuf2, *party2="", *party3="";

    len = strlen(cmd);
    for (i = 0; i < MAX_EMOTES; i++) {
        if (users[unum].lpEmote[i][0] == '\0')
            continue;
        if (!strncasecmp(cmd, users[unum].lpEmote[i], len)
            && users[unum].lpEmote[i][len] == ' ') {
            if (*party == '\0') {
                party = CHAT_PARTY;
                party2 = NULL;
            } else {
                party2 = party;
                party = getnextword(&party2);
                recunum = fuzzy_chatid_to_indx(party);
                if (recunum >= 0)
                    party = users[recunum].chatid;
            }
            sprintf(chatbuf, "\033[1m%s\033[m ", users[unum].chatid);
            tmpbuf = (users[unum].lpEmote[i] + len + 1);
            recunum = -1;
            while (1) {
                tmpbuf2 = strstr(tmpbuf, "%s");
                chatlen = strlen(chatbuf);
                if (tmpbuf2 == NULL) {
                    if (chatlen + strlen(tmpbuf) + 5 < 256)
                        sprintf(chatbuf + strlen(chatbuf), "%s\033[m", tmpbuf);
                    break;
                }
                if (recunum >= 0) {
                    if (party2 != NULL) {
                        party3 = getnextword(&party2);
                        recunum = fuzzy_chatid_to_indx(party3);
                        if (recunum >= 0)
                            party3 = users[recunum].chatid;
                        else if (*party3 != ' ' && *party3 != '\0')
                            recunum = 0;
                    } else
                        recunum = -1;
                }
                ch = *tmpbuf2;
                *tmpbuf2 = '\0';
                if (strlen(tmpbuf) + strlen(recunum < 0 ? party : party3) + chatlen + 9 > 255) {
                    *tmpbuf2 = ch;
                    break;
                }
                sprintf(chatbuf + strlen(chatbuf), "%s\033[1m%s\033[m", tmpbuf, recunum < 0 ? party : party3);
                *tmpbuf2 = ch;
                tmpbuf = tmpbuf2 + 2;
                recunum = 0;
            }
            /* chang to send_to_room2 call */
            send_to_room2(users[unum].room, chatbuf, unum);
            return 0;
        }
    }
    return 1;
}

int party_action(unum, cmd, party)
    int unum;
    char *cmd;
    char *party;
{
    int i;

    for (i = 0; party_data[i].verb; i++) {
        if (!strcasecmp(cmd, party_data[i].verb)) {
            if (*party == '\0') {
                party = CHAT_PARTY;
            } else {
                int recunum = fuzzy_chatid_to_indx(party);

                if (recunum < 0) {
                    /* no such user, or ambiguous */
                    if (recunum == -1)
                        sprintf(chatbuf, msg_no_such_id, party);
                    else
                        sprintf(chatbuf, "◎ 请问哪间" CHAT_ROOM_NAME " ◎");
                    send_to_unum(unum, chatbuf);
                    return 0;
                }
                party = users[recunum].chatid;
            }
            if (strcasecmp(cmd, "faint") == 0 && (strcasecmp(party, users[unum].chatid) == 0 || strcmp(party, CHAT_PARTY) == 0))
                sprintf(chatbuf, "\033[1m%s\033[m %s一声，晕倒在地...\033[m", users[unum].chatid, "咣当");
            else {
                if (strcasecmp(party, users[unum].chatid) == 0)
                    party = "自己";
                sprintf(chatbuf, "\033[1m%s\033[m %s \033[1m%s\033[m %s\033[m", users[unum].chatid, party_data[i].part1_msg, party, party_data[i].part2_msg);
            }
            send_to_room2(users[unum].room, chatbuf, unum);
            return 0;
        }
    }
    return 1;
}


/* -------------------------------------------- */
/* MUD-like social commands : speak              */
/* -------------------------------------------- */


struct action speak_data[] = {
    {"ask", "询问", NULL},
    {"chant", "歌颂", NULL},
    {"cheer", "喝采", NULL},
    {"chuckle", "轻笑", NULL},
    {"curse", "咒骂", NULL},
    {"demand", "要求", NULL},
    {"frown", "蹙眉", NULL},
    {"groan", "呻吟", NULL},
    {"grumble", "发牢骚", NULL},
    {"hum", "喃喃自语", NULL},
    {"moan", "悲叹", NULL},
    {"notice", "注意", NULL},
    {"order", "命令", NULL},
    {"ponder", "沈思", NULL},
    {"pout", "噘著嘴说", NULL},
    {"pray", "祈祷", NULL},
    {"request", "恳求", NULL},
    {"shout", "大叫", NULL},
    {"sing", "唱歌", NULL},
    {"smile", "微笑", NULL},
    {"smirk", "假笑", NULL},
    {"swear", "发誓", NULL},
    {"tease", "嘲笑", NULL},
    {"whimper", "呜咽的说", NULL},
    {"yawn", "哈欠连天", NULL},
    {"yell", "大喊", NULL},
    {NULL, NULL, NULL}
};


int speak_action(unum, cmd, msg)
    int unum;
    char *cmd;
    char *msg;
{
    int i;

    for (i = 0; speak_data[i].verb; i++) {
        if (!strcasecmp(cmd, speak_data[i].verb)) {
            sprintf(chatbuf, "\033[1m%s\033[m %s：%s\033[m", users[unum].chatid, speak_data[i].part1_msg, msg);
            send_to_room(users[unum].room, chatbuf, unum);
            return 0;
        }
    }
    return 1;
}


/* -------------------------------------------- */
/* MUD-like social commands : condition          */
/* -------------------------------------------- */


struct action condition_data[] = {
    {":D", "乐的合不拢嘴", NULL},
    {":)", "轻轻的笑了笑", NULL},
    {":P", "吐了吐舌头", NULL},
    {":(", "委屈的扁了扁嘴", NULL},
    {"applaud", "啪啪啪啪啪啪啪....", NULL},
    {"blush", "脸都红了", NULL},
    {"cough", "咳了几声", NULL},
    {"faint", "咣当一声，晕倒在地", NULL},
    {"happy", "的脸上露出了幸福的表情，并学吃饱了的猪哼哼了起来", NULL},
    {"lonely", "一个人坐在房间里，百无聊赖，希望谁来陪陪。。。。", NULL},
    {"luck", "哇！福气啦！", NULL},
    {"puke", "真恶心，我听了都想吐", NULL},
    {"shake", "摇了摇头", NULL},
    {"sleep", "Zzzzzzzzzz，真无聊，都快睡著了", NULL},
    {"so", "就酱子!!", NULL},
    {"strut", "大摇大摆地走", NULL},
    {"tongue", "吐了吐舌头", NULL},
    {"think", "歪著头想了一下", NULL},
    {"wawl", "惊天动地的哭", NULL},
    {NULL, NULL, NULL}
};


int condition_action(unum, cmd)
    int unum;
    char *cmd;
{
    int i;

    for (i = 0; condition_data[i].verb; i++) {
        if (!strcasecmp(cmd, condition_data[i].verb)) {
            sprintf(chatbuf, "\033[1m%s\033[m %s\033[m", users[unum].chatid, condition_data[i].part1_msg);
            send_to_room(users[unum].room, chatbuf, unum);
            return 1;
        }
    }
    return 0;
}


/* -------------------------------------------- */
/* MUD-like social commands : help               */
/* -------------------------------------------- */

char *dscrb[] = {
    "【 Verb + Nick：   动词 + 对方名字 】   例：//kick piggy",
    "【 Verb + Message：动词 + 要说的话 】   例：//sing 天天天蓝",
    "【 Verb：动词 】    ↑↓：旧话重提", NULL
};
struct action *verbs[] = { party_data, speak_data, condition_data, NULL };


#define SCREEN_WIDTH    80
#define MAX_VERB_LEN    10
#define VERB_NO         8

void view_action_verb(unum, verb)
    int unum;
    char *verb;
{
    int i, j=0, all, count;
    char *p="", *tmp;

    /* add by KCN for list all emote */
    count = 1;
    all = 0;
    if (verb) {
        if (!strcmp(verb, "all"))
            all = 1;
    }
    if ((!verb) || all)
        send_to_unum(unum, "/c");

    for (i = 0; dscrb[i]; i++) {
        if (!verb)
            send_to_unum(unum, dscrb[i]);
        chatbuf[0] = '\0';
        j = 0;
        while ((p = verbs[i][j++].verb) != NULL) {
            if (!verb)
                strcat(chatbuf, p);
            else
                /* add by KCN */
            if (all) {
                if (i == 0)
                    sprintf(chatbuf, "\033[1m%-8s: \033[m %s \033[1m" CHAT_PARTY "\033[m %s\033[m", verbs[i][j - 1].verb, party_data[j - 1].part1_msg, party_data[j - 1].part2_msg);
                else if (i == 1)
                    sprintf(chatbuf, "\033[1m%-8s: \033[m %s：%s\033[m", verbs[i][j - 1].verb, speak_data[j - 1].part1_msg, "我爱你来你爱我!");
                else
                    sprintf(chatbuf, "\033[1m%-8s: \033[m %s\033[m", verbs[i][j - 1].verb, condition_data[j - 1].part1_msg);

                tmp = chatbuf;
                while ((tmp = strstr(tmp, "\\n")) != NULL) {
                    count++;
                    tmp++;
                };
                count++;

                send_to_unum2(unum, chatbuf);
                continue;
            }
            if (verb)
                if (!strcmp(p, verb))
                    break;

            if ((j % VERB_NO) == 0) {
                if (!verb)
                    send_to_unum(unum, chatbuf);
                chatbuf[0] = '\0';
            } else {
                strncat(chatbuf, "        ", MAX_VERB_LEN - strlen(p));
            }
        }
        if (!verb) {
            if (j % VERB_NO)
                send_to_unum(unum, chatbuf);
        } else if ((!all) && p)
            break;
    }
    /* add by KCN */
    if (!verb)
        send_to_unum(unum, "用//help 动作,可以看到动作的说明,//help all列出所有动作示范");
    else if (!all) {
        if (!p) {
            send_to_unum(unum, "没有这个动作！");
        } else {
            sprintf(chatbuf, "动作%s的示范:", verb);
            send_to_unum(unum, chatbuf);
            j--;
            if (i == 0)
                sprintf(chatbuf, "\033[1m%s\033[m %s \033[1m" CHAT_PARTY "\033[m %s\033[m", users[unum].chatid, party_data[j].part1_msg, party_data[j].part2_msg);
            else if (i == 1)
                sprintf(chatbuf, "\033[1m%s\033[m %s：%s\033[m", users[unum].chatid, speak_data[j].part1_msg, "我爱你来你爱我!");
            else
                sprintf(chatbuf, "\033[1m%s\033[m %s\033[m", users[unum].chatid, condition_data[j].part1_msg);
            send_to_unum2(unum, chatbuf);
        }
    }
}


struct chatcmd chatcmdlist[] = {
    {"act", chat_act, 0, 0},
    {"bye", chat_goodbye, 0, 1},
    {"exit", chat_goodbye, 0, 1},
    {"flags", chat_setroom, 0, 1},
    {"invite", chat_invite, 0, 0},
    {"join", chat_join, 0, 1},
    {"kick", chat_kick, 0, 1},
    {"kid", chat_kickid, 0, 1}, /*Haohmaru.99.4.6 */
    {"msg", chat_private, 0, 1},
    {"nick", chat_nick, 0, 0},
    {"operator", chat_makeop, 0, 1},
    {"rooms", chat_list_rooms, 0, 1},
    {"whoin", chat_list_by_room, 1, 1},
    {"wall", chat_broadcast, 1, 1},
    {"rname", chat_name_room, 1, 1},    /* added by Luzi 1998.1.16 */
    {"who", chat_map_chatids_thisroom, 0, 1},
    {"list", chat_list_users, 0, 1},
    {"topic", chat_topic, 0, 1},
    {"toggle", chat_toggle, 0, 1},
    {"me", chat_act, 0, 0},
    {"q", chat_query, 1, 1},    /* exect flag from 0 to 1, modified by dong, 1998.9.12 */
    {"qc", chat_query_ByChatid, 1, 1},  /* added by dong 1998.9.12 */
    {"ignore", chat_ignore, 1, 1},      /* added by Luzi 1997.11.30 */
    {"listen", chat_listen, 1, 1},      /* added by Luzi 1997.11.30 */
    {"alias", call_alias, 1, 1},        /* added by Luzi 1998.01.25 */
    {"knock", chat_knock_room, 0, 1},   /* added by period 2000-09-15 */
    {NULL, NULL, 0, 0},
};


int command_execute(unum)
    int unum;
{
    char *msg = users[unum].ibuf;
    char *cmd;
    struct chatcmd *cmdrec;
    int match = 0;

    /* Validation routine */
    if (users[unum].room == -1) {
        /* MUST give special /! command if not in the room yet */
        if (msg[0] != '/' || msg[1] != '!')
            return -1;
        else
            return (login_user(unum, msg + 2));
    }


    /* If not a /-command, it goes to the room. */
    if (msg[0] != '/') {
        if (users[unum].room == 0 && !ENABLEMAIN)       /* added bye Luzi 1998.1.3 */
            send_to_unum(unum, "◎ " CHAT_SERVER "里禁止喧哗 ◎");
        else
            chat_allmsg(unum, msg);
        return 0;
    }

    msg++;
    cmd = getnextword(&msg);
    if (cmd[0] == '/') {

        if (!strcasecmp(cmd + 1, "help") || (cmd[1] == '\0')) {
            /* add by KCN */
            if (cmd[1] && cmd[6]) {
                /*
                   char* verb;
                   verb=getnextword(cmd+1);
                 */
                view_action_verb(unum, cmd + 6);
            } else
                view_action_verb(unum, 0);
            match = 1;
        } else if (users[unum].room == 0 && !ENABLEMAIN) {      /* added by Luzi 1998.1.3 */
            send_to_unum(unum, "◎ " CHAT_SERVER "里保持肃静 ◎");
            match = 1;
        } else if (NOEMOTE(users[unum].room)) {
            send_to_unum(unum, "◎ 本" CHAT_ROOM_NAME "禁止做小动作 ◎");
            match = 1;
        } else if (alias_action(unum, cmd + 1, msg) == 0)
            match = 1;
        else if (party_action(unum, cmd + 1, msg) == 0)
            match = 1;
        else if (speak_action(unum, cmd + 1, msg) == 0)
            match = 1;
        else
            match = condition_action(unum, cmd + 1);
    } else {
        if (!strncasecmp(cmd, "shutdown", 8)) {
            if (SYSOP(unum)) {
                match = -1;     /* SYSOP可以执行shutdown命令 */
            }
        } else {
            for (cmdrec = chatcmdlist; !match && cmdrec->cmdstr; cmdrec++) {
                if (cmdrec->exact)
                    match = !strcasecmp(cmd, cmdrec->cmdstr);
                else
                    match = !strncasecmp(cmd, cmdrec->cmdstr, strlen(cmd));
                if (match) {
                    if (ENABLEMAIN || users[unum].room || cmdrec->bUsed)        /* added by Luzi 98.1.3 */
                        if (NOEMOTE(users[unum].room) && (cmd[0] == 'a' || cmd[0] == 'A' || ((cmd[0] == 'm' || cmd[0] == 'M') && (cmd[1] == 'e' || cmd[1] == 'E'))))
                            send_to_unum(unum, "◎ 本" CHAT_ROOM_NAME "禁止作小动作 ◎");
                        else
                            cmdrec->cmdfunc(unum, msg);
                    else
                        send_to_unum(unum, "◎ " CHAT_SERVER "里禁止使用该指令 ◎");
                }
            }
        }
    }

    if (match == 0) {
        sprintf(chatbuf, "◎ " CHAT_SYSTEM "看不懂你的意思：\033[1m%s\033[m ◎", cmd);
        send_to_unum(unum, chatbuf);
    }
    memset(users[unum].ibuf, 0, sizeof users[unum].ibuf);
    if (match == -1)
        return 1;
    else
        return 0;
}


int process_chat_command(unum)
    int unum;
{
    register int i;
    int rc, ibufsize;
    short nFlag;

    if ((rc = recv(users[unum].sockfd, chatbuf, sizeof chatbuf, 0)) <= 0) {
        /* disconnected */
        exit_room(unum, EXIT_LOSTCONN, (char *) NULL);
        return -1;
    }
    ibufsize = users[unum].ibufsize;
    for (i = 0; i < rc; i++) {
        /* if newline is two characters, throw out the first */
        if (chatbuf[i] == '\r')
            continue;

        /* carriage return signals end of line */
        else if (chatbuf[i] == '\n') {
            users[unum].ibuf[ibufsize] = '\0';
            nFlag = command_execute(unum);
            if (nFlag)
                return nFlag;
            ibufsize = 0;
        }


        /* add other chars to input buffer unless size limit exceeded */
        else {
            if (ibufsize < 126)
                users[unum].ibuf[ibufsize++] = chatbuf[i];
            else {
                users[unum].ibuf[ibufsize] = '\0';
                nFlag = command_execute(unum);
                if (nFlag)
                    return nFlag;
                ibufsize = 0;
                /* Luzi增加，处理越界数据，自动换行 1999/1/8 */
            }
        }
    }
    users[unum].ibufsize = ibufsize;

    return 0;
}

int main(argc, argv)
    int argc;
    char *argv[];
{
    struct sockaddr_in sin;
    register int i;
    int sr, newsock, nFlag = 0;
    socklen_t sinsize;
    fd_set readfds;
    struct timeval *tvptr = NULL;

    chdir(BBSHOME);
#ifdef FILTER
    resolve_boards();
#endif
    /* ----------------------------- */
    /* init variable : rooms & users */
    /* ----------------------------- */
#ifdef FREEBSD
    bzero(&sin, sizeof(sin));
#endif
    strcpy(rooms[0].name, mainroom);
    /*strcpy(rooms[0].topic, maintopic); */

    /* Leeward: 98.01.04: 设定 main 的缺省标题如下所示(No chatting...);
       同时注意: 为配合 chat.c, 不可加 \033[m 在标题末尾 */
    setuid(BBSUID);
    setuid(BBSGID);

	init_sessiondata(getSession());
    if (ENABLEMAIN)
        strcpy(rooms[0].topic, "\033[1m大家先随便聊聊吧");
    else
        strcpy(rooms[0].topic, "\033[1mNo chatting in main room, type /h for help");

    for (i = 0; i < CHATMAXACTIVE; i++) {
        users[i].chatid[0] = '\0';
        users[i].sockfd = users[i].utent = -1;
    }

    /* ------------------------------ */
    /* bind chat server to port       */
    /* ------------------------------ */

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket:");
        return -1;
    }
    nFlag = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &nFlag, sizeof(nFlag));

    sin.sin_family = AF_INET;
    if (argc > 1)
        sin.sin_port = htons(CHATPORT2);
    else
        sin.sin_port = htons(CHATPORT3);

    /* change by KCN 1999.10.22
       sin.sin_addr.s_addr = INADDR_ANY;
     */
    sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        perror("bind:");
        return -1;
    }

    sinsize = sizeof(sin);
    if (getsockname(sock, (struct sockaddr *) &sin, &sinsize) == -1) {
        perror("getsockname");
        exit(1);
    }

    if (listen(sock, 5) == -1) {
        perror("listen");
        exit(1);
    }


    if (fork()) {
        return (0);
    }
    setpgid(0, 0);

    /* ------------------------------ */
    /* trap signals                   */
    /* ------------------------------ */

    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGURG, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);


    FD_ZERO(&allfds);
    FD_SET(sock, &allfds);
    nfds = sock + 1;

    while (1) {
        memcpy(&readfds, &allfds, sizeof readfds);

        if ((sr = select(nfds, &readfds, NULL, NULL, tvptr)) < 0) {
            if (errno == EINTR)
                continue;
            exit(-1);
        }

        if (sr == 0) {
            goto SHUTDOWN;      /* normal chat server shutdown */
        }

        if (tvptr)
            tvptr = NULL;

        if (FD_ISSET(sock, &readfds)) {
            sinsize = sizeof sin;
            newsock = accept(sock, (struct sockaddr *) &sin, &sinsize);
            if (newsock == -1) {
                continue;
            }
            /* Luzi 1998.3.20 判断连接是否来自本机 */
            /* disable by KCN 1999.10.22
               if (strncmp((char*)&sin.sin_addr, inbuf, h->h_length)!=0)
               {                   
               close(newsock);
               continue;
               }
             */
            for (i = 0; i < CHATMAXACTIVE; i++) {
                if (users[i].sockfd == -1) {
                    users[i].sockfd = newsock;
                    users[i].room = -1;
                    break;
                }
            }

            if (i >= CHATMAXACTIVE) {
                /* full -- no more chat users */
                close(newsock);
            } else {

#if !RELIABLE_SELECT_FOR_WRITE
                int flags = fcntl(newsock, F_GETFL, 0);

                flags |= O_NDELAY;
                fcntl(newsock, F_SETFL, flags);
#endif

                FD_SET(newsock, &allfds);
                if (newsock >= nfds)
                    nfds = newsock + 1;
                num_conns++;
            }
        }

        for (i = 0; i < CHATMAXACTIVE; i++) {
            /* we are done with newsock, so re-use the variable */
            newsock = users[i].sockfd;
            if (newsock != -1 && FD_ISSET(newsock, &readfds)) {
                nFlag = process_chat_command(i);
                if (nFlag == -1) {
                    logout_user(i);
                }
                if (nFlag == 1)
                    goto SHUTDOWN;
            }
        }
        /*    if (num_conns <= 0)
           {
           tvptr = &zerotv;
           } */
    }
    /* NOTREACHED */

  SHUTDOWN:
    close(sock);
    for (i = 0; i < CHATMAXACTIVE; i++) {
        /* we are done with newsock, so re-use the variable */
        newsock = users[i].sockfd;
        if (newsock != -1)
            shutdown(newsock, 3);
        close(newsock);
    }
    exit(0);
}
