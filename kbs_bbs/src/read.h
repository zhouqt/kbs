#ifndef __READ_H__
#define __READ_H__

typedef int (*READ_KEY_FUNC)(struct _select_def*,void*,void*);

struct key_command {                /* Used to pass commands to the readmenu */
    int key;
    READ_KEY_FUNC fptr;
    void* arg;
};

enum {
    READ_NORMAL,
    READ_THREAD
};

struct read_arg {
    /* save argument */
    enum BBS_DIR_MODE mode;
    char* direct;
    void (*dotitle) ();
    READ_FUNC doentry;
    struct key_command *rcmdlist;
    int ssize;

    /*用于确定当前的阅读模式，如果是
    READ_NORMAL  正常的顺序阅读
    READ_THREAD  正在主题阅读
    */
    int readmode; 

    char* data; //readed data
    int fd; //filehandle,open always

    int filecount; //the item count of file
};

enum {
        APPLY_CONTINUE,
        APPLY_QUIT,
        APPLY_REAPPLY
};

/* 应用于apply_thread的函数
  conf是select结构
  fh是当前的fileheader
  ent是序号
  arg是传给apply_thread的额外参数
*/
typedef int (*APPLY_THREAD_FUNC)(struct _select_def* conf,struct fileheader* fh,int ent,void* arg);

/*对同主题应用同一个func函数
  @param conf 当前的select结构
  @param fh 当前的fileheader指针
  @param func 应用于主题结构的函数。
        返回APPLY_CONTINUE继续应用下一个主题结构
        返回APPLY_QUIT则停止执行。
        返回APPLY_REAPPLY则重复应用这一个位置的fileheader
  @param down 查找主题方向，如果是1,向下查找，否则向上
  @param arg 传递给func的参数
  @return 应用的主题个数
*/

int apply_thread(struct _select_def* conf, struct fileheader* fh,APPLY_THREAD_FUNC func, bool down,void* arg);

int new_i_read(enum BBS_DIR_MODE cmdmode, char *direct, void (*dotitle) (), READ_FUNC doentry, struct key_command *rcmdlist, int ssize);

/* some function for fileheader */
int auth_search(struct _select_def* conf, struct fileheader *fileinfo,void* extraarg);
int title_search(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int thread_search(struct _select_def* conf, struct fileheader* fh, void* extraarg);

#define SR_FIRSTNEW     0
#define SR_FIRST            1
#define SR_LAST             2
#define SR_NEXT             3
#define SR_PREV             4
#define SR_FIRSTNEWDOWNSEARCH 100

int thread_read(struct _select_def* conf, struct fileheader* fh, void* extraarg);

int read_sendmsgtoauthor(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int read_showauthor(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int read_showauthorinfo(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int read_cross(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int read_zsend(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int read_addauthorfriend(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int read_showauthorBM(struct _select_def* conf, struct fileheader* fh, void* extraarg);

#ifdef PERSONAL_CORP
int read_importpc(struct _select_def* conf, struct fileheader* fh, void* extraarg);
#endif
#endif  //__READ_H__
