/* ÁÄÌìÊÒ Server */
#undef BBSMAIN
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
char *msg_no_such_id = "*** [1m%s[m ²»ÔÚ±¾" CHAT_ROOM_NAME "Àï ***";
char *msg_not_here = "*** [1m%s[m ²¢Ã»ÓĞÇ°À´" CHAT_SERVER " ***";

#ifdef FILTER
void filter_report(char* title,char *str)
{
	FILE *se;
	char fname[STRLEN];
	struct userec chatuser;

       bzero(&chatuser,sizeof(chatuser));
       strcpy(chatuser.userid,"ÁÄÌìÊÒ±¨¸æ");
       strcpy(chatuser.username,"ÁÄÌìÊÒÃÜÌ½");
       chatuser.userlevel=-1;
	sprintf(fname, "tmp/deliver.chatd-report");
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", str);
		fclose(se);
                post_file(&chatuser, "", fname, FILTER_BOARD, title, 0, 2);
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
   code like "ÎÄÎÄ" and "ÎäÎä". So we use our own code.
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
    if (!strncmp(id, "¡ò", 2))
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
    	if (check_badword_str(str, strlen(str))) {
    		char title[80];
    		char content[256];
    		sprintf(title,"%s ÔÚÁÄÌìÊÒËµ»µ»°",users[unum].userid);
    		sprintf(content,"%s\n%s(ÁÄÌì´úºÅ %s )Ëµ:%s",Ctime(time(NULL)),
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
                sprintf(chatbuf, "*** [1m%s[m (%s) ÂıÂıÀë¿ªÁË ***", users[unum].chatid, users[unum].userid);  /* dong 1998.9.13 */
                if (msg && *msg) {
                    strcat(chatbuf, ": ");
                    strncat(chatbuf, msg, 80);
                }
                break;

            case EXIT_LOSTCONN:
                sprintf(chatbuf, "*** [1m%s[m (%s) ÒÑ" CHAT_MSG_QUIT " ***", users[unum].chatid, users[unum].userid); /* dong 1998.9.13 */
                break;

            case EXIT_KICK:
                /*sprintf(chatbuf, "*** [1m%s[m ±»ÁÄÌìÊÒopÌß³öÈ¥ÁË ***", users[unum].chatid); */
                sprintf(chatbuf, "*** [1m%s[m (%s) ±»" CHAT_OP "(%s)Ìß³öÈ¥ÁË ***", users[unum].chatid, users[unum].userid, msg);      /* Leeward 98.03.02 *//* dong 1998.9.13 */
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
        send_to_unum(unum, "¡ò ÇëÎğÂÒ¸Ä" CHAT_MAIN_ROOM CHAT_ROOM_NAME "µÄ»°Ìâ ¡ò");
        return;
    }
    if (*msg == '\0') {
        send_to_unum(unum, "¡ò ÇëÖ¸¶¨»°Ìâ ¡ò");
        return;
    }

    strncpy(rooms[rnum].topic, msg, 52);
    rooms[rnum].topic[51] = '\0';
    sprintf(chatbuf, "/t%.51s", msg);
    send_to_room(rnum, chatbuf, unum);
    sprintf(chatbuf, "*** [1m%s[m ½«»°Ìâ¸ÄÎª [1m%s[m ***", users[unum].chatid, msg);
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
        send_to_unum(unum, "¡ò ÇëÖ¸¶¨ĞÂµÄ" CHAT_ROOM_NAME "Ãû³Æ ¡ò");
        return;
    }
    if (rnum == 0) {
        send_to_unum(unum, "¡ò ²»ÄÜ¸ü¸Ä " CHAT_MAIN_ROOM " ÁÄÌìÊÒµÄÃû³Æ ¡ò");
        return;
    }
    if (roomid_to_indx(roomid) >= 0) {
        send_to_unum(unum, "¡ò ÒÑ¾­ÓĞÍ¬ÃûµÄ" CHAT_ROOM_NAME "ÁË ¡ò");
        return;
    }
    strncpy(rooms[rnum].name, roomid, IDLEN - 1);
    rooms[rnum].name[IDLEN - 1] = '\0';
    sprintf(chatbuf, "/z%s", rooms[rnum].name);
    send_to_room(rnum, chatbuf, unum);
    sprintf(chatbuf, "*** \033[1m%s\033[m ½«" CHAT_ROOM_NAME "Ãû³Æ¸ÄÎª \033[1m%s\033[m ***", users[unum].chatid, rooms[rnum].name);
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
            send_to_unum(unum, "¡ò ÎÒÃÇµÄ" CHAT_ROOM_NAME "ÂúÁËà¸ ¡ò");
            return 0;
        }
    }
    if (!SYSOP(unum) && !CHATOP(unum))
        if (LOCKED(rnum) && rooms[rnum].invites[unum] == 0) {
            send_to_unum(unum, "¡ò ±¾" CHAT_ROOM_NAME "ÉÌÌÖ»úÃÜÖĞ£¬·ÇÇëÎğÈë ¡ò");
            return 0;
        }
    if (!SYSOP(unum))
        if (SYSOPED(rnum) && rooms[rnum].invites[unum] == 0) {
            send_to_unum(unum, "¡ò ±¾" CHAT_ROOM_NAME "¾øÃÜ£¬·ÇÇëÎğÈë ¡ò");
            return 0;
        }

    exit_room(unum, EXIT_LOGOUT, msg);
    users[unum].room = rnum;
    if (op)
        users[unum].flags |= PERM_CHATROOM;
    rooms[rnum].occupants++;
    rooms[rnum].invites[unum] = 0;
    sprintf(chatbuf, "*** [1m%s[m (%s) ½øÈë [1m%s[m " CHAT_ROOM_NAME " ***", users[unum].chatid, users[unum].userid, rooms[rnum].name);
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
    sprintf(chatbuf, "¡ï »¶Ó­¹âÁÙ¡¾" NAME_BBS_CHINESE "¡¿" CHAT_SERVER "£¬Ä¿Ç°ÒÑ¾­ÓĞ %d ¼ä" CHAT_ROOM_NAME "ÓĞ¿ÍÈË ¡ï ", roomc + 1);
    send_to_unum(unum, chatbuf);
    sprintf(chatbuf, "¡ï ±¾ÁÄ" CHAT_SERVER "ÄÚ¹²ÓĞ %d ÈË ", userc + 1);
    if (suserc)
        sprintf(chatbuf + strlen(chatbuf), "[ÆäÖĞ %d ÈËÔÚÃØÃÜ" CHAT_ROOM_NAME "]", suserc);
    sprintf(chatbuf + strlen(chatbuf), "¡ï");
    send_to_unum(unum, chatbuf);
    if (!ENABLEMAIN)
        send_to_unum(unum, "[1mÔÚ [33mmain[m[1m ´óÌüÀï½ûÖ¹ÁÄÌì¡¢×÷Ğ¡¶¯×÷£¬¿ÉÓÃ [33m/j[m[1m Ö¸Áîµ½ÆäËü" CHAT_ROOM_NAME "½øĞĞÁÄÌì[m");
    send_to_unum(unum, "[1mÊäÈë [33m/e[m[1m Àë¿ª" CHAT_SERVER "£¬[33m/w[m[1m ²é¿´±¾" CHAT_ROOM_NAME "ÖĞµÄ" NAME_USER_SHORT "[m");
    send_to_unum(unum, "[1mÊäÈë [33m/r[m[1m ²é¿´µ±Ç°µÄ" CHAT_ROOM_NAME "£¬[33m/j roomname[m[1m ½øÈëÏàÓ¦" CHAT_ROOM_NAME "[m");
    send_to_unum(unum, "[1mÊäÈë [33m/h[m[1m »ñµÃ°ïÖúĞÅÏ¢[m");
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
                    send_to_unum(unum, "¡¼ÓÃ»§×Ô¶¨ÒåemoteÁĞ±í¡½");
                    has = 1;
                }
                send_to_unum(unum, users[unum].lpEmote[i]);
            }
        if (has == 0)
            send_to_unum(unum, "*** »¹Ã»ÓĞ×Ô¶¨ÒåµÄemote ***");
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
                send_to_unum(unum, "*** ¸ÃemoteÒÑ¾­±»¶¨Òå¹ıÁË ***");
                return;
            }
            if (delete_record(path, 128, nIdx + 1, NULL, NULL) == 0) {
                send_to_unum(unum, "*** ¸Ã×Ô¶¨ÒåemoteÒÑ¾­±»É¾³ıÁË ***");
                users[unum].lpEmote[nIdx][0] = 0;
            } else {
                send_to_unum(unum, "*** system error ***");
            }
        } else if (!*arg)
            send_to_unum(unum, "*** ÇëÖ¸¶¨emote¶ÔÓ¦µÄ×Ö´® ***");
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
                    send_to_unum(unum, "*** ×Ô¶¨ÒåemoteÒÑ¾­Éè¶¨ ***");
                    strncpy(users[unum].lpEmote[nIdx], buf, 128);
                    users[unum].lpEmote[nIdx][128] = 0;
                } else {
                    send_to_unum(unum, "*** ÏµÍ³´íÎó ***");
                }
            } else
                send_to_unum(unum, "*** ÓÃ»§×Ô¶¨ÒåemoteµÄÁĞ±íÒÑÂú ***");
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
        send_to_unum(unum, "¡ò ±§Ç¸£¡" CHAT_SYSTEM "²»ÈÃÄã¿´ÓĞÄÄĞ©·¿¼äÓĞ¿ÍÈË ¡ò");
        return;
    }
    send_to_unum(unum, "[33m[44m " CHAT_ROOM_NAME "Ãû³Æ  ©¦ÈËÊı©¦»°Ìâ        [m");
    for (i = 0; i < MAXROOM; i++) {
        occupants = rooms[i].occupants;
        if (occupants > 0) {
            if (!SYSOP(unum) && !CHATOP(unum))
                if ((rooms[i].flags & ROOM_SECRET) && (users[unum].room != i))
                    continue;
            sprintf(chatbuf, " %-12s©¦%4d©¦%s\033[m", rooms[i].name, occupants, rooms[i].topic);
            /*---	period	2000-10-24	add background color to chatroom status	---*/
            if (rooms[i].flags & ROOM_LOCKED)
                strcat(chatbuf, " [\033[30;47mËø×¡\033[m]");
            if (rooms[i].flags & ROOM_SECRET)
                strcat(chatbuf, " [\033[30;47mÃØÃÜ\033[m]");
            if (rooms[i].flags & ROOM_NOEMOTE)
                strcat(chatbuf, " [\033[30;47m½ûÖ¹¶¯×÷\033[m]");
            if (rooms[i].flags & ROOM_SYSOP)
                strcat(chatbuf, " [\033[30;47m¾øÃÜ\033[m]");

            send_to_unum(unum, chatbuf);
        }
    }
}


int chat_do_user_list(int unum, char *msg, int whichroom)
{
    char buf[14] = "    [¹«¿ª]    ";    /*Haohmaru.99.09.18 */
    char buf1[14] = "[ÃØÃÜ]  [Ëø×¡]";
    char buf2[14] = "    [ÃØÃÜ]    ";
    char buf3[14] = "    [Ëø×¡]    ";
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
            sprintf(chatbuf, "[33m[44m ÁÄÌì´úºÅ©¦%6s´úºÅ  ©¦ÁÄÌìÊÒ%-14s[m", NAME_USER_SHORT, buf1);
        else if (rooms[whichroom].flags & ROOM_SECRET)
            sprintf(chatbuf, "[33m[44m ÁÄÌì´úºÅ©¦%6s´úºÅ  ©¦ÁÄÌìÊÒ%-14s[m", NAME_USER_SHORT, buf2);
        else if (rooms[whichroom].flags & ROOM_LOCKED)
            sprintf(chatbuf, "[33m[44m ÁÄÌì´úºÅ©¦%6s´úºÅ  ©¦ÁÄÌìÊÒ%-14s[m", NAME_USER_SHORT, buf3);
        else
            sprintf(chatbuf, "[33m[44m ÁÄÌì´úºÅ©¦%6s´úºÅ  ©¦ÁÄÌìÊÒ%-14s[m", NAME_USER_SHORT, buf);
        send_to_unum(unum, chatbuf);
    } else
        send_to_unum(unum, "[33m[44m ÁÄÌì´úºÅ©¦" NAME_USER_SHORT "´úºÅ  ©¦ÁÄÌìÊÒ [m");
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
            sprintf(chatbuf, " %-8s©¦%-12s©¦%s", users[i].chatid, users[i].userid, rooms[rnum].name);
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
            sprintf(chatbuf, "¡ò Ã»Õâ¸ö" CHAT_ROOM_NAME "à¸ ¡ò");
            send_to_unum(unum, chatbuf);
            return;
        }
        if ((rooms[whichroom].flags & ROOM_SECRET) && !SYSOP(unum) && !CHATOP(unum)) {
            send_to_unum(unum, "¡ò ±¾" CHAT_SERVER "µÄ" CHAT_ROOM_NAME "½Ô¹«¿ªµÄ£¬Ã»ÓĞÃØÃÜ ¡ò");
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
    send_to_unum(unum, "[33m[44m ÁÄÌì´úºÅ Ê¹ÓÃÕß´úºÅ  ©¦ ÁÄÌì´úºÅ Ê¹ÓÃÕß´úºÅ  ©¦ ÁÄÌì´úºÅ Ê¹ÓÃÕß´úºÅ [m");
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
            sprintf(chatbuf + (c * 24), " %-8s%c%-12s%s", users[i].chatid, (ROOMOP(i)) ? '*' : ' ', users[i].userid, (c < 2 ? "©¦" : "  "));
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
    char *fstr;

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
        send_to_unum(unum, "¡ò Çë¸æËß" CHAT_SYSTEM "ÄúÒªµÄ" CHAT_ROOM_NAME "ÊÇ: {[[1m+[m(Éè¶¨)][[1m-[m(È¡Ïû)]}{[[1ml[m(Ëø×¡)][[1ms[m(ÃØÃÜ)]}");
        return;
    }
    while (*modestr) {
        flag = 0;
        switch (*modestr) {
        case 'l':
        case 'L':
            if (!rnum && !SYSOP(unum)) {        /*added by Haohmaru,98.9.6 */
                send_to_unum(unum, "¡ò " CHAT_MAIN_ROOM " " CHAT_ROOM_NAME "²»ÄÜ±»Ëø×¡ ¡ò");
                break;
            }
            flag = ROOM_LOCKED;
            fstr = "Ëø×¡";
            break;
        case 's':
        case 'S':
            if (!rnum && !SYSOP(unum)) {        /*added by Haohmaru,98.9.6 */
                send_to_unum(unum, "¡ò " CHAT_MAIN_ROOM CHAT_ROOM_NAME "²»ÄÜ±»ÉèÎªÃØÃÜĞÎÊ½ ¡ò");
                break;
            }
            flag = ROOM_SECRET;
            fstr = "ÃØÃÜ";
            break;
        case 'e':
        case 'E':
            flag = ROOM_NOEMOTE;
            fstr = "'½ûÖ¹¶¯×÷'";
            break;
        case 'x':
        case 'X':
            if (SYSOP(unum)) {
                flag = ROOM_SYSOP;
                fstr = "'¾øÃÜ'";
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
                sprintf(buf, "¹ıÂËÆ÷ÒÑ¾­´ò¿ª");
       	else
                sprintf(buf, "¹ıÂËÆ÷ÒÑ¾­¹Ø±Õ");
       	send_to_unum(unum,buf);
       	sprintf(title,"%s:%s",users[unum].userid, buf);
       	filter_report(title,buf);
       	return;
       	}
	}
#endif
        default:
            sprintf(chatbuf, "¡ò " CHAT_SYSTEM "¿´²»¶®ÄãµÄÒâË¼£º[[1m%c[m] ¡ò", *modestr);
            send_to_unum(unum, chatbuf);
        }
        if (flag && ((rooms[rnum].flags & flag) != sign * flag)) {
            rooms[rnum].flags ^= flag;
            sprintf(chatbuf, "*** Õâ" CHAT_ROOM_NAME "±» [1m%s[m %s%sµÄĞÎÊ½ ***", users[unum].chatid, sign ? "Éè¶¨Îª" : "È¡Ïû", fstr);
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
        send_to_unum(unum, "¡ò ·Ç·¨Ãû×Ö£¬ÇëÖØĞÂÊäÈë ¡ò");
        return;
    }
    chatid[8] = '\0';
    othernum = chatid_to_indx(chatid);
    if (othernum != -1 && othernum != unum) {
        /*    send_to_unum(unum, "¡ò ±§Ç¸£¡ÓĞÈË¸úÄãÍ¬Ãû£¬ËùÒÔÄã²»ÄÜ½øÀ´ ¡ò"); */
        send_to_unum(unum, "¡ò ±§Ç¸£¡ÓĞÈËÒÑ¾­Ê¹ÓÃ¸Ãchatid,ËùÒÔÄú²»ÄÜÔÙÊ¹ÓÃ ¡ò");
        return;
    }
    sprintf(chatbuf, "¡ò [1m%s[m ½«ÁÄÌì´úºÅ¸ÄÎª [1m%s[m ¡ò", users[unum].chatid, chatid);
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
            sprintf(chatbuf, " ¡ò ¶Ô·½½ĞÊ²÷áÃû×Ö? ¡ò");
        send_to_unum(unum, chatbuf);
        return;
    }
    if (*msg) {
        if (can_send(unum, recunum)) {
            sprintf(chatbuf, "[1m*%s*[m ", users[unum].chatid);
            strncat(chatbuf, msg, 80);
            send_to_unum(recunum, chatbuf);
            sprintf(chatbuf, "[1m%s>[m ", users[recunum].chatid);
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
        sprintf(chatbuf, "[1m%s[m %s", users[unum].chatid, msg);
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
        send_to_unum(unum, "¡ò ÄãÖ»ÄÜÔÚÕâÀïÁÄÌì ¡ò");
        return;
    }
    if (*roomid == '\0') {
        send_to_unum(unum, "¡ò ÇëÎÊÄÄ¸ö" CHAT_ROOM_NAME " ¡ò");
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
    sprintf(chatbuf, "*** Äã±»" CHAT_OP "([1m[33m%s[m)Ìß³öÀ´ÁË ***", users[unum].userid);
    send_to_unum(recunum, chatbuf);

    rnum2 = users[recunum].room;

    /*exit_room(recunum, EXIT_KICK, (char *) NULL); */
    exit_room(recunum, EXIT_KICK, users[unum].userid);  /* Leeward 98.03.02 */

    if (rnum == 0 || rnum != rnum2)
        logout_user(recunum);
    else
        enter_room(recunum, mainroom, (char *) NULL);
}

void chat_kickid(unum, msg)     /*Haohmaru.99.6.9.¸ù¾İIDÌßÈË */
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
    sprintf(chatbuf, "*** Äã±»" CHAT_OP "([1m[33m%s[m)Ìß³öÀ´ÁË ***", users[unum].userid);
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
        sprintf(chatbuf, "¡ò ÄãÍüÁËÄã±¾À´¾ÍÊÇ" CHAT_OP "à¸ ¡ò");
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
    sprintf(chatbuf, "*** [1m%s[m ¾ö¶¨ÈÃ [1m%s[m µ±±¾" CHAT_ROOM_NAME "µÄ" CHAT_OP " ***", users[unum].chatid, users[recunum].chatid);
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
        sprintf(chatbuf, "*** [1m%s[m È¡ÏûÁË [1m%s[m µÄ" CHAT_OP " ***", users[unum].chatid, users[recunum].chatid);
    } else {
        users[recunum].flags |= PERM_CHATROOM;
        sprintf(chatbuf, "*** [1m%s[m ÉèÖÃ [1m%s[m Îª±¾ÁÄÌìÊÒµÄ" CHAT_OP " ***", users[unum].chatid, users[recunum].chatid);
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
        sprintf(chatbuf, "*** [1m%s[m µÈÒ»ÏÂ¾ÍÀ´ ***", users[recunum].chatid);
        send_to_unum(unum, chatbuf);
        return;
    }
    rooms[rnum].invites[recunum] = 1;
    sprintf(chatbuf, "*** [1m%s[m ÑûÇëÄúµ½ [1m%s[m " CHAT_ROOM_NAME "ÁÄÌì ***", users[unum].chatid, rooms[rnum].name);
    send_to_unum(recunum, chatbuf);
    sprintf(chatbuf, "*** [1m%s[m µÈÒ»ÏÂ¾ÍÀ´ ***", users[recunum].chatid);
    send_to_unum(unum, chatbuf);
}

/*---	Added by period		2000-09-15	---*/
/*---	NOTICE : send_to_room in SMTH codes need 3 parameters !!!	---*/
void chat_knock_room(int unum, char *msg)
{
    char *roomid;
    const char *outd = "ÀïÃæ";
    const char *ind = "ÍâÃæ";
    int rnum;

    roomid = getnextword(&msg);
    if (RESTRICTED(unum)) {
        send_to_unum(unum, "\033[1;31m¡ò \033[37mÄãÖ»ÄÜÔÚÕâÀïÁÄÌì \033[31m¡ò\033[m");
        return;
    }
    if (*roomid == '\0') {
        send_to_unum(unum, "\033[1;31m¡ò \033[37mÇëÎÊÄÄ¸ö" CHAT_ROOM_NAME " \033[31m¡ò\033[m");
        return;
    }
    rnum = roomid_to_indx(roomid);
    if (rnum == -1) {
        send_to_unum(unum, "\033[1;31m¡ò \033[37m Ã»ÓĞÕâ¸ö" CHAT_ROOM_NAME " \033[31m¡ò\033[m");
        return;
    }
    if ((SECRET(rnum) || NOEMOTE(rnum)) && !SYSOP(unum) && !CHATOP(unum)) {
        send_to_unum(unum, "\033[1;31m¡ò \033[37m ÇëÎğ´òÈÅ£¬Ğ»Ğ»ºÏ×÷£¡\033[31m¡ò\033[m");
        return;
    }
    sprintf(chatbuf, "\033[1;37m¡ò \033[31mµ±µ±µ±... \033[33m%s [%s] \033[37mÔÚ%sÇÃÃÅ : \033[32m%s \033[37m¡ò\033[m",
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
        send_to_unum(unum, "¡ò Äã²»¿ÉÒÔÔÚ" CHAT_SERVER "ÄÚ´óÉùĞú»© ¡ò");
        return;
    }
    if (*msg == '\0') {
        send_to_unum(unum, "¡ò ¹ã²¥ÄÚÈİÊÇÊ²÷á ¡ò");
        return;
    }
    sprintf(chatbuf, "¡ò " CHAT_SERVER CHAT_OP " [1m%s[m ÓĞ»°¶Ô´ó¼ÒĞû²¼£º", users[unum].chatid);
    send_to_room(ROOM_ALL, chatbuf, unum);
    sprintf(chatbuf, "¡¾%s¡¿", msg);
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
    char *verb;                 /* ¶¯´Ê */
    char *part1_msg;            /* ½é´Ê */
    char *part2_msg;            /* ¶¯×÷ */
};


struct action party_data[] = {
    {"?", "ºÜÒÉ»óµÄ¿´×Å", ""},
    {"admire", "¶Ô", "µÄ¾°ÑöÖ®ÇéÓÌÈçÌÏÌÏ½­Ë®Á¬Ãà²»¾ø"},
    {"agree", "ÍêÈ«Í¬Òâ", "µÄ¿´·¨"},
    {"bearhug", "ÈÈÇéµÄÓµ±§", ""},
    {"bless", "×£¸£", "ĞÄÏëÊÂ³É"},
    {"bow", "±Ï¹ª±Ï¾´µÄÏò", "¾Ï¹ª"},
    {"bye", "¿´×Å", "µÄ±³Ó°£¬ÆàÈ»ÀáÏÂ¡£ÉíºóµÄÊÕÒô»ú´«À´µËÀö¾ıµÄ¸èÉù:\\n\"[31mºÎÈÕ¾ıÔÙÀ´.....[m\""},
    {"caress", "ÇáÇáµÄ¸§Ãş", ""},
    {"cat", "ÏñÖ»Ğ¡Ã¨°ãµØÒÀÙËÔÚ", "µÄ»³ÀïÈö½¿¡£"},
    {"cringe", "Ïò", "±°¹ªÇüÏ¥£¬Ò¡Î²ÆòÁ¯"},
    {"cry", "Ïò", "º¿ßû´ó¿Ş"},
    {"comfort", "ÎÂÑÔ°²Î¿", ""},
    {"clap", "Ïò", "ÈÈÁÒ¹ÄÕÆ"},
    {"dance", "À­×Å", "µÄÊÖôæôæÆğÎè"},
    {"dogleg", "¶Ô", "¹·ÍÈ"},
    {"drivel", "¶ÔÖø", "Á÷¿ÚË®"},
    {"dunno", "µÉ´óÑÛ¾¦£¬ÌìÕæµØÎÊ£º", "£¬ÄãËµÊ²÷áÎÒ²»¶®Ò®... :("},
    {"faint", "ÔÎµ¹ÔÚ", "µÄ»³Àï"},
    {"fear", "¶Ô", "Â¶³öÅÂÅÂµÄ±íÇé"},
    {"fool", "Çë´ó¼Ò×¢Òâ", "Õâ¸ö´ó°×³Õ....\\nÕæÊÇÌìÉÏÉÙÓĞ....ÈË¼ä½ö´æµÄ»î±¦....\\n²»¿´Ì«¿ÉÏ§ÁË£¡"},
    {"forgive", "´ó¶ÈµÄ¶Ô", "Ëµ£ºËãÁË£¬Ô­ÁÂÄãÁË"},
    {"giggle", "¶ÔÖø", "ÉµÉµµÄ´ôĞ¦"},
    {"grin", "¶Ô", "Â¶³öĞ°¶ñµÄĞ¦Èİ"},
    {"growl", "¶Ô", "ÅØÏø²»ÒÑ"},
    {"hand", "¸ú", "ÎÕÊÖ"},
    {"hammer", "¾ÙÆğºÃ´óºÃ´óµÄÌú´¸£¡£¡ÍÛ£¡Íù",
     "Í·ÉÏÓÃÁ¦Ò»ÇÃ£¡\\n***************\\n*  5000000 Pt *\\n***************\\n      | |      %1¡ï%2¡î%3¡ï%4¡î%5¡ï%6¡î%0\\n      | |         ºÃ¶àµÄĞÇĞÇÓ´\\n      |_|"},
    {"heng", "¿´¶¼²»¿´", "Ò»ÑÛ£¬ ºßÁËÒ»Éù£¬¸ß¸ßµÄ°ÑÍ·ÑïÆğÀ´ÁË,²»Ğ¼Ò»¹ËµÄÑù×Ó..."},
    {"hug", "ÇáÇáµØÓµ±§", ""},
    {"idiot", "ÎŞÇéµØ³ÜĞ¦", "µÄ³Õ´ô¡£"},
    {"kick", "°Ñ", "ÌßµÄËÀÈ¥»îÀ´"},
    {"kiss", "ÇáÎÇ", "µÄÁ³¼Õ"},
    {"laugh", "´óÉù³°Ğ¦", ""},
    {"lovelook", "À­×Å", "µÄÊÖ£¬ÎÂÈáµØÄ¬Ä¬¶ÔÊÓ¡£Ä¿¹âÖĞÔĞº¬×ÅÇ§ÖÖÈáÇé£¬Íò°ãÃÛÒâ"},
    {"nod", "Ïò", "µãÍ·³ÆÊÇ"},
    {"nudge", "ÓÃÊÖÖâ¶¥", "µÄ·Ê¶Ç×Ó"},
    {"oh", "¶Ô", "Ëµ£º¡°Å¶£¬½´×Ó°¡£¡¡±"},
    {"pad", "ÇáÅÄ", "µÄ¼ç°ò"},
    {"papaya", "ÇÃÁËÇÃ", "µÄÄ¾¹ÏÄÔ´ü"},
    {"pat", "ÇáÇáÅÄÅÄ", "µÄÍ·"},
    {"pinch", "ÓÃÁ¦µÄ°Ñ", "Å¡µÄºÚÇà"},
    {"puke", "¶Ô×Å", "ÍÂ°¡ÍÂ°¡£¬¾İËµÍÂ¶à¼¸´Î¾ÍÏ°¹ßÁË"},
    {"punch", "ºİºİ×áÁË", "Ò»¶Ù"},
    {"pure", "¶Ô", "Â¶³ö´¿ÕæµÄĞ¦Èİ"},
    {"qmarry", "Ïò", "ÓÂ¸ÒµÄ¹òÁËÏÂÀ´:\\n\"ÄãÔ¸Òâ¼Ş¸øÎÒÂğ£¿\"\\n---ÕæÊÇÓÂÆø¿É¼Î°¡"},
    {"report", "ÍµÍµµØ¶Ô", "Ëµ£º¡°±¨¸æÎÒºÃÂğ£¿¡±"},
    {"shrug", "ÎŞÄÎµØÏò", "ËÊÁËËÊ¼ç°ò"},
    {"sigh", "¶Ô", "Ì¾ÁËÒ»¿ÚÆø"},
    {"slap", "Å¾Å¾µÄ°ÍÁË", "Ò»¶Ù¶ú¹â"},
    {"smooch", "ÓµÎÇÖø", ""},
    {"snicker", "ºÙºÙºÙ..µÄ¶Ô", "ÇÔĞ¦"},
    {"sniff", "¶Ô", "àÍÖ®ÒÔ±Ç"},
    {"sorry", "Í´¿ŞÁ÷ÌéµÄÇëÇó", "Ô­ÁÂ"},
    {"spank", "ÓÃ°ÍÕÆ´ò", "µÄÍÎ²¿"},
    {"squeeze", "½ô½ôµØÓµ±§Öø", ""},
    {"thank", "Ïò", "µÀĞ»"},
    {"tickle", "¹¾ß´!¹¾ß´!É¦", "µÄÑ÷"},
    {"waiting", "ÉîÇéµØ¶Ô", "Ëµ£ºÃ¿ÄêÃ¿ÔÂµÄÃ¿Ò»Ìì£¬Ã¿·ÖÃ¿ÃëÎÒ¶¼ÔÚÕâÀïµÈ×ÅÄã"},
    {"wake", "Å¬Á¦µÄÒ¡Ò¡", "£¬ÔÚÆä¶ú±ß´ó½Ğ£º¡°¿ìĞÑĞÑ£¬»á×ÅÁ¹µÄ£¡¡±"},
    {"wave", "¶ÔÖø", "Æ´ÃüµÄÒ¡ÊÖ"},
    {"welcome", "ÈÈÁÒ»¶Ó­", "µÄµ½À´"},
    {"wink", "¶Ô", "ÉñÃØµÄÕ£Õ£ÑÛ¾¦"},
    {"xixi", "ÎûÎûµØ¶Ô", "Ğ¦ÁË¼¸Éù"},
    {"zap", "¶Ô", "·è¿ñµÄ¹¥»÷"},
    {"inn", "Ë«ÑÛ±¥º¬×ÅÀáË®£¬ÎŞ¹¼µÄÍû×Å", ""},
    {"mm", "É«ÃĞÃĞµÄ¶Ô", "ÎÊºÃ£º¡°ÃÀÃ¼ºÃ¡«¡«¡«¡«¡±¡£´óÉ«ÀÇ°¡£¡£¡£¡"},
    {"disapp", "ÕâÏÂÃ»¸ãÍ·À²£¬ÎªÊ²Ã´", "¹ÃÄï¶ÔÎÒÕâ¸öÔìĞÍÍêÈ«Ã»·´Ó¦£¿Ã»°ì·¨£¡"},
    {"miss", "Õæ³ÏµÄÍû×Å", "£ºÎÒÏëÄîÄãÎÒÕæµÄÏëÄîÄãÎÒÌ«--ÏëÄîÄãÁË!ÄãÏà²»ÏàĞÅ?"},
    {"buypig", "Ö¸×Å", "£º¡°Õâ¸öÖíÍ·¸øÎÒÇĞÒ»°ë£¬Ğ»Ğ»£¡¡±"},
    {"rascal", "¶Ô", "´ó½Ğ£º¡°ÄãÕâ¸ö³ôÁ÷Ã¥£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡¡±"},
    {"qifu", "Ğ¡×ìÒ»±â£¬¶Ô", "¿ŞµÀ£º¡°ÄãÆÛ¸ºÎÒ£¬ÄãÆÛ¸ºÎÒ£¡£¡£¡¡±"},
    {"wa", "¶Ô", "´ó½ĞÒ»Éù£º¡°ÍÛÍÛÍÛÍÛÍÛÍÛ¿á±×ÁËÒ®£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡¡±"},
    {"feibang", "à¸£­£­£¡Êì¹éÊì£¬", "ÄãÕâÑùÂÒ½²»°£¬ÎÒÒ»Ñù¿ÉÒÔ¸æÄã»Ù°ù£¬¹ş£¡"},
    {NULL, NULL, NULL}
};

int alias_action(unum, cmd, party)
    int unum;
    char *cmd;
    char *party;
{
    int i, len, chatlen, recunum;
    char ch, *tmpbuf, *tmpbuf2, *party2, *party3;

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
                        sprintf(chatbuf, "¡ò ÇëÎÊÄÄ¼ä" CHAT_ROOM_NAME " ¡ò");
                    send_to_unum(unum, chatbuf);
                    return 0;
                }
                party = users[recunum].chatid;
            }
            if (strcasecmp(cmd, "faint") == 0 && (strcasecmp(party, users[unum].chatid) == 0 || strcmp(party, CHAT_PARTY) == 0))
                sprintf(chatbuf, "\033[1m%s\033[m %sÒ»Éù£¬ÔÎµ¹ÔÚµØ...\033[m", users[unum].chatid, "ßÛµ±");
            else {
                if (strcasecmp(party, users[unum].chatid) == 0)
                    party = "×Ô¼º";
                sprintf(chatbuf, "[1m%s[m %s [1m%s[m %s[m", users[unum].chatid, party_data[i].part1_msg, party, party_data[i].part2_msg);
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
    {"ask", "Ñ¯ÎÊ", NULL},
    {"chant", "¸èËÌ", NULL},
    {"cheer", "ºÈ²É", NULL},
    {"chuckle", "ÇáĞ¦", NULL},
    {"curse", "ÖäÂî", NULL},
    {"demand", "ÒªÇó", NULL},
    {"frown", "õ¾Ã¼", NULL},
    {"groan", "ÉëÒ÷", NULL},
    {"grumble", "·¢ÀÎÉ§", NULL},
    {"hum", "à«à«×ÔÓï", NULL},
    {"moan", "±¯Ì¾", NULL},
    {"notice", "×¢Òâ", NULL},
    {"order", "ÃüÁî", NULL},
    {"ponder", "ÉòË¼", NULL},
    {"pout", "àÙÖø×ìËµ", NULL},
    {"pray", "Æíµ»", NULL},
    {"request", "¿ÒÇó", NULL},
    {"shout", "´ó½Ğ", NULL},
    {"sing", "³ª¸è", NULL},
    {"smile", "Î¢Ğ¦", NULL},
    {"smirk", "¼ÙĞ¦", NULL},
    {"swear", "·¢ÊÄ", NULL},
    {"tease", "³°Ğ¦", NULL},
    {"whimper", "ÎØÑÊµÄËµ", NULL},
    {"yawn", "¹şÇ·Á¬Ìì", NULL},
    {"yell", "´óº°", NULL},
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
            sprintf(chatbuf, "[1m%s[m %s£º%s[m", users[unum].chatid, speak_data[i].part1_msg, msg);
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
    {":D", "ÀÖµÄºÏ²»Â£×ì", NULL},
    {":)", "ÇáÇáµÄĞ¦ÁËĞ¦", NULL},
    {":P", "ÍÂÁËÍÂÉàÍ·", NULL},
    {":(", "Î¯ÇüµÄ±âÁË±â×ì", NULL},
    {"applaud", "Å¾Å¾Å¾Å¾Å¾Å¾Å¾....", NULL},
    {"blush", "Á³¶¼ºìÁË", NULL},
    {"cough", "¿ÈÁË¼¸Éù", NULL},
    {"faint", "ßÛµ±Ò»Éù£¬ÔÎµ¹ÔÚµØ", NULL},
    {"happy", "µÄÁ³ÉÏÂ¶³öÁËĞÒ¸£µÄ±íÇé£¬²¢Ñ§³Ô±¥ÁËµÄÖíºßºßÁËÆğÀ´", NULL},
    {"lonely", "Ò»¸öÈË×øÔÚ·¿¼äÀï£¬°ÙÎŞÁÄÀµ£¬Ï£ÍûË­À´ÅãÅã¡£¡£¡£¡£", NULL},
    {"luck", "ÍÛ£¡¸£ÆøÀ²£¡", NULL},
    {"puke", "Õæ¶ñĞÄ£¬ÎÒÌıÁË¶¼ÏëÍÂ", NULL},
    {"shake", "Ò¡ÁËÒ¡Í·", NULL},
    {"sleep", "Zzzzzzzzzz£¬ÕæÎŞÁÄ£¬¶¼¿ìË¯ÖøÁË", NULL},
    {"so", "¾Í½´×Ó!!", NULL},
    {"strut", "´óÒ¡´ó°ÚµØ×ß", NULL},
    {"tongue", "ÍÂÁËÍÂÉàÍ·", NULL},
    {"think", "ÍáÖøÍ·ÏëÁËÒ»ÏÂ", NULL},
    {"wawl", "¾ªÌì¶¯µØµÄ¿Ş", NULL},
    {NULL, NULL, NULL}
};


int condition_action(unum, cmd)
    int unum;
    char *cmd;
{
    int i;

    for (i = 0; condition_data[i].verb; i++) {
        if (!strcasecmp(cmd, condition_data[i].verb)) {
            sprintf(chatbuf, "[1m%s[m %s[m", users[unum].chatid, condition_data[i].part1_msg);
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
    "¡¾ Verb + Nick£º   ¶¯´Ê + ¶Ô·½Ãû×Ö ¡¿   Àı£º//kick piggy",
    "¡¾ Verb + Message£º¶¯´Ê + ÒªËµµÄ»° ¡¿   Àı£º//sing ÌìÌìÌìÀ¶",
    "¡¾ Verb£º¶¯´Ê ¡¿    ¡ü¡ı£º¾É»°ÖØÌá", NULL
};
struct action *verbs[] = { party_data, speak_data, condition_data, NULL };


#define SCREEN_WIDTH    80
#define MAX_VERB_LEN    10
#define VERB_NO         8

void view_action_verb(unum, verb)
    int unum;
    char *verb;
{
    int i, j, all, count;
    char *p, *tmp;

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
                    sprintf(chatbuf, "[1m%-8s: [m %s [1m" CHAT_PARTY "[m %s[m", verbs[i][j - 1].verb, party_data[j - 1].part1_msg, party_data[j - 1].part2_msg);
                else if (i == 1)
                    sprintf(chatbuf, "[1m%-8s: [m %s£º%s[m", verbs[i][j - 1].verb, speak_data[j - 1].part1_msg, "ÎÒ°®ÄãÀ´Äã°®ÎÒ!");
                else
                    sprintf(chatbuf, "[1m%-8s: [m %s[m", verbs[i][j - 1].verb, condition_data[j - 1].part1_msg);

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
        send_to_unum(unum, "ÓÃ//help ¶¯×÷,¿ÉÒÔ¿´µ½¶¯×÷µÄËµÃ÷,//help allÁĞ³öËùÓĞ¶¯×÷Ê¾·¶");
    else if (!all) {
        if (!p) {
            send_to_unum(unum, "Ã»ÓĞÕâ¸ö¶¯×÷£¡");
        } else {
            sprintf(chatbuf, "¶¯×÷%sµÄÊ¾·¶:", verb);
            send_to_unum(unum, chatbuf);
            j--;
            if (i == 0)
                sprintf(chatbuf, "[1m%s[m %s [1m" CHAT_PARTY "[m %s[m", users[unum].chatid, party_data[j].part1_msg, party_data[j].part2_msg);
            else if (i == 1)
                sprintf(chatbuf, "[1m%s[m %s£º%s[m", users[unum].chatid, speak_data[j].part1_msg, "ÎÒ°®ÄãÀ´Äã°®ÎÒ!");
            else
                sprintf(chatbuf, "[1m%s[m %s[m", users[unum].chatid, condition_data[j].part1_msg);
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
            send_to_unum(unum, "¡ò " CHAT_SERVER "Àï½ûÖ¹Ğú»© ¡ò");
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
            send_to_unum(unum, "¡ò " CHAT_SERVER "Àï±£³ÖËà¾² ¡ò");
            match = 1;
        } else if (NOEMOTE(users[unum].room)) {
            send_to_unum(unum, "¡ò ±¾" CHAT_ROOM_NAME "½ûÖ¹×öĞ¡¶¯×÷ ¡ò");
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
                match = -1;     /* SYSOP¿ÉÒÔÖ´ĞĞshutdownÃüÁî */
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
                            send_to_unum(unum, "¡ò ±¾" CHAT_ROOM_NAME "½ûÖ¹×÷Ğ¡¶¯×÷ ¡ò");
                        else
                            cmdrec->cmdfunc(unum, msg);
                    else
                        send_to_unum(unum, "¡ò " CHAT_SERVER "Àï½ûÖ¹Ê¹ÓÃ¸ÃÖ¸Áî ¡ò");
                }
            }
        }
    }

    if (match == 0) {
        sprintf(chatbuf, "¡ò " CHAT_SYSTEM "¿´²»¶®ÄãµÄÒâË¼£º[1m%s[m ¡ò", cmd);
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
                /* LuziÔö¼Ó£¬´¦ÀíÔ½½çÊı¾İ£¬×Ô¶¯»»ĞĞ 1999/1/8 */
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
    long sinsize;
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

    /* Leeward: 98.01.04: Éè¶¨ main µÄÈ±Ê¡±êÌâÈçÏÂËùÊ¾(No chatting...);
       Í¬Ê±×¢Òâ: ÎªÅäºÏ chat.c, ²»¿É¼Ó [m ÔÚ±êÌâÄ©Î² */
    setuid(BBSUID);
    setuid(BBSGID);
    if (ENABLEMAIN)
        strcpy(rooms[0].topic, "\033[1m´ó¼ÒÏÈËæ±ãÁÄÁÄ°É");
    else
        strcpy(rooms[0].topic, "[1mNo chatting in main room, type /h for help");

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
    if (getsockname(sock, (struct sockaddr *) &sin, (socklen_t *) & sinsize) == -1) {
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
            newsock = accept(sock, (struct sockaddr *) &sin, (socklen_t *) & sinsize);
            if (newsock == -1) {
                continue;
            }
            /* Luzi 1998.3.20 ÅĞ¶ÏÁ¬½ÓÊÇ·ñÀ´×Ô±¾»ú */
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
