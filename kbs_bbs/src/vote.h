#ifndef _VOTE_H
#define _VOTE_H

#define VOTE_YN         (1)
#define VOTE_SINGLE     (2)
#define VOTE_MULTI      (3)
#define VOTE_VALUE      (4)
#define VOTE_ASKING     (5)

struct ballot {
    char uid[IDLEN+2];            /* 投票人       */
    unsigned int voted;         /* 投票的内容   */
    char msg[3][STRLEN];        /* 建议事项     */
};

struct votebal {
    char userid[IDLEN + 1];
    char title[STRLEN];
    char type;
    char items[32][38];
    int maxdays;
    int maxtkt;
    int totalitems;
    time_t opendate;
};

/*
struct votebal
{
        char            userid[IDLEN+1];
        char            title[STRLEN];
        char            type;
        char            items[32][38];
        int             maxdays;
        int             maxtkt;
        int             totalitems;
        time_t          opendate;
        int             stay;
        int             day;
        unsigned int    numlogins;
        unsigned int    numposts;
}
;
*/
struct votelimit {              /*Haohmaru.99.11.17.根据版主设的限制条件判断是否让该使用者投票 */
    int stay;
    int day;
    unsigned int numlogins;
    unsigned int numposts;
};

char *vote_type[] = { "是非", "单选", "复选", "数字", "问答" };

#endif
