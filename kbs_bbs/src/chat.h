/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* chat.h - port numbers for the chat rooms -- changing them is wise. */
#define CHATPORT  4231
#define CHATPORT1 1386
#define CHATPORT2 2486
#define CHATPORT3 3586
#define CHATPORT4 4686

#define DEFAULT_CHATROOM        "Le_Jardin_Secret"      /* Default and Couldn't be changed */
#define MAX_CHATROOM    10      /* MAX 10 Chat-Rooms */
#define MAX_PORTS       25      /* MAX users in one Chat-Room */
#define IDLEN           12      /* ID Length (must match in BBS.H) */
#define CHAT_IDLEN      8       /* Chat ID Length in Chat-Room */
#define CHAT_CMDLEN     8       /* MAX 8 characters of CHAT-ROOM commands */
#define CHAT_NAMELEN    20      /* MAX 20 characters of CHAT-ROOM NAME */
#define CHAT_TITLELEN   40      /* MAX 40 characters of CHAT-ROOM TITLE */
#define CHATLINE_MAX    255
#define TITLE_ROW       0       /* Where to display the title (48,0) */
#define TITLE_COL       48      /* 80 - strlen("Title : ") - CHAT_TITLELEN */
#define DELTA_TIME      300     /* 5 mins * 60 = 300 */
#define MAX_EMOTES      36      /* emotes user defined */

enum ERROR {
    ERROR_COMMAND, ERROR_HELP_TOPIC, ERROR_ACTION,

    ERROR_NO_USER, ERROR_NO_ROOM,

    ERROR_NEED_NICK, ERROR_NEED_ID, ERROR_NEED_ROOM,
    ERROR_NEED_TITLE
};

enum SORRY {
    SORRY_EXIST_ROOM, SORRY_EXIST_NICK,

    SORRY_RESERVE_ROOM, SORRY_CROWDED_ROOM,
    SORRY_NOFREE_ROOM, SORRY_INTIME_TITLE,
    SORRY_NOT_LEADER
};


#define CLOAK_BIT       0x0001
#define INCHAT_BIT      0x0002
#define LEADER_BIT      0x0004
struct person_info {
    int portfd;
    int uid;                    /* Used to find person in PASSWD file */
    unsigned userlevel;         /* Level of this user */
    unsigned flag;
    char bbsid[IDLEN + 1];
    char chatid[CHAT_IDLEN + 1];
};

struct chat_info {
    char name[CHAT_NAMELEN + 1];        /* Chat-room Name */
    char title[CHAT_TITLELEN + 1];      /* Chat-room Title */
    int time;                   /* Set time of Chat-room Title */
    unsigned access_level;      /* Who can access this room */
    int numports;
    struct person_info person[MAX_PORTS];
};


/* 2001/5/6 added by wwj */
#define MAXMESSAGE 5
#define MAXLASTCMD 32

typedef struct {
    int cfd;                    /* 聊天室的socket */

    char buf[512];
    int bufptr;

    char chatid[10];
    char chatroom[IDLEN];       /* Chat-Room Name */
    char topic[STRLEN];

    int chatline;               /* Where to display message now */
    int outputcount;

    int cmdpos;
    char lastcmd[MAXLASTCMD][128];

    int apply_count;
    char *apply_buf;

    FILE *rec;
} chatcontext;


struct chat_command {
    char *cmdname;              /* Char-room command length */
    void (*cmdfunc) (chatcontext * pthis, const char *arg);     /* Pointer to function */
    short nCmdLenth;            /*命令字串判别需要匹配的字符数 */
};


#define MAXROOM         32


#define EXIT_LOGOUT     0
#define EXIT_LOSTCONN   -1
#define EXIT_CLIERROR   -2
#define EXIT_TIMEDOUT   -3
#define EXIT_KICK       -4


#define CHAT_LOGIN_OK       "OK"
#define CHAT_LOGIN_EXISTS   "EX"
#define CHAT_LOGIN_INVALID  "IN"
#define CHAT_LOGIN_BOGUS    "BG"

/*
 * Chat Room 中禁用於 nick 的字元
 */
#define BADCIDCHARS " *"
