#ifndef __WEBMSG_H__
#define __WEBMSG_H__

#include "bbs.h"

#define MSG_LEN MAX_MSG_SIZE
#define MSG_NUM 5

typedef struct _msgent_t {
    int srcutmp;                /* 消息发送者的 utmp 号 */
    time_t sndtime;             /* 消息发送的时间 */
    char srcid[IDLEN + 1];      /* 消息发送者的 ID */
    char msg[MSG_LEN + 1];      /* 消息的内容 */
} msgent_t;

#if 0
typedef struct _msgent2_t {
    int destutmp;               /* 消息接收者的 utmp 号 */
    char destid[IDLEN + 1];     /* 消息接收者的 ID */
    int srcutmp;                /* 消息发送者的 utmp 号 */
    char srcid[IDLEN + 1];      /* 消息发送者的 ID */
    char msg[MSG_LEN + 1];      /* 消息的内容 */
} msgent2_t;
#endif

typedef struct _msglist_t {
    int msgnum;                 /* 用户未读消息的条数 */
    int current;                /* 用户当前未读消息的序号 */
    int utmpnum;                /* 消息接收者的 utmp 号 */
    char userid[IDLEN + 1];     /* 消息接收者的 ID */
    msgent_t msgs[MSG_NUM];     /* 消息列表 */
} msglist_t;

typedef struct _msg_t {
    int type;                   /* request or response type */
    int sockfd;                 /* */
    char rawdata[STRLEN + MSG_LEN];   /* */
} bbsmsg_t;

/*static msg_t     msgbuf;*/

enum {
    MSGD_NEW = 100,
    MSGD_DEL = 110,
    MSGD_SND = 200,
    MSGD_RCV = 210,
    MSGD_OK = 250,
    MSGD_FRM = 260,
    MSGD_HLO = 270,
    MSGD_BYE = 280,
    MSGD_MSG = 300,
    MSGD_ERR = 400
};

#define read_request(x,y) read_peer(x,y)
#define read_response(x,y) read_peer(x,y)
#define write_request(x) write_peer(x)
#define write_response(x) write_peer(x)

#endif                          /* __WEBMSG_H__  */
