#ifndef __DDD_H__
#define __DDD_H__

#ifdef DDD_ACTIVE

// 全局阅读状态
struct ddd_global_status {
    int type;  // 类型
    int sec;  // 分类讨论区号码
    int favid;  // 新分类讨论区或个人定制区位置
    int bid;  // 版面序号
    int mode;  // 阅读模式
    int pos;  // 位置
    int filter;  // 超级版面选择
    int recur;  // 递归标记
};

// ddd_global_status.type 的取值
#define GS_NONE 0 // 不在阅读状态
#define GS_ALL 1 // 所有版面列表或者分类讨论区
#define GS_NEW 2 // 新分类讨论区
#define GS_FAV 3 // 个人定制区
#define GS_GROUP 4 // 目录版面
#define GS_BOARD 5 // 版面
#define GS_MAIL 6 // 信箱

// 当前状态
#define DDD_GS_CURR (getSession()->gs_curr)
// 即将变换到的新状态
#define DDD_GS_NEW (getSession()->gs_new)


int ddd_gs_init(struct ddd_global_status* gs);
int ddd_entry();
int ddd_read_loop();
int ddd_header();
int ddd_read_all();
int ddd_read_unknown();

#endif /* DDD_ACTIVE */

#endif /* __DDD_H__ */
