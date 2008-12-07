#ifndef __DDD_H__
#define __DDD_H__

#ifdef DDD_ACTIVE

struct ddd_global_status {
    int type;
    int sec;
    int favid;
    int bid;
    int mode;
    int pos;
    int filter;
    int recur;
};


#define GS_NONE 0 // 不在阅读状态
#define GS_ALL 1 // 所有版面列表或者分类讨论区
#define GS_NEW 2 // 新分类讨论区
#define GS_FAV 3 // 个人定制区
#define GS_GROUP 4 // 目录版面
#define GS_BOARD 5 // 版面
#define GS_MAIL 6 // 信箱

#define DDD_GS_CURR (getSession()->gs_curr)
#define DDD_GS_NEW (getSession()->gs_new)


int ddd_gs_init(struct ddd_global_status* gs);
int ddd_entry();
int ddd_read_loop();
int ddd_header();
int ddd_read_all();
int ddd_read_unknown();

#endif /* DDD_ACTIVE */

#endif /* __DDD_H__ */
