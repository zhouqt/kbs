#ifndef _KBSBBS_ANN_H_
#define _KBSBBS_ANN_H_

/* etnlegend, 2006.10.14, 精华区底层操作修正... */

#define MAXITEMS        1024
#define PATHLEN         256
#define MTITLE_LEN      128
#define ITITLE_LEN      88

typedef struct _ann_item{
    char    title[ITITLE_LEN];
    char    fname[STRLEN];
    char   *host;
    int     port;
    long    attachpos;
}
ITEM;

typedef struct _ann_menu{
    ITEM  **p_item[MAXITEMS];       /* 可见 ITEM 指针列表 */
    ITEM   *pool[MAXITEMS];         /* 全部 ITEM 列表 */
    char    mtitle[MTITLE_LEN];     /* 当前 .Names 标题内容 */
    char   *path;                   /* 当前 .Names 文件路径 */
    void   *nowmenu;
    void   *father;
    int     num;                    /* 当前 .Names 中可显示的 ITEM 数量, 对应于 `item[]` 数组 */
    int     page;
    int     now;                    /* 当前位置 */
    int     level;
    int     total;                  /* 当前 .Names 的所有 ITEM 数量, 对应于 `pool[]` 数组 */
    time_t  modified_time;          /* 加载 .Names 时文件的时间戳 */
#ifdef ANN_COUNT
    int     count;
#endif /* ANN_COUNT */
}
MENU;

#define I_FREE(i)                                           \
    do{                                                     \
        if(i){                                              \
            free((i)->host);                                \
            free(i);                                        \
        }                                                   \
    }while(0)
#define I_ALLOC()       ((ITEM*)calloc(1,sizeof(ITEM)))

/* 取菜单(m)中序号(n)位置的项目 */
#define M_ITEM(m,n)     (*((m)->p_item[n]))

enum ANN_SORT_MODE{
    ANN_SORT_BY_FILENAME,
    ANN_SORT_BY_TITLE,
    ANN_SORT_BY_BM,
    ANN_SORT_BY_FILENAME_R,
    ANN_SORT_BY_TITLE_R,
    ANN_SORT_BY_BM_R,
    ANN_SORT_UNKNOWN
};

#endif /* _KBSBBS_ANN_H_ */

