#ifndef __READ_H__
#define __READ_H__

#define DONOTHING       0       /* Read menu command return states */
#define FULLUPDATE      1       /* Entire screen was destroyed in this oper */
#define PARTUPDATE      2       /* Only the top three lines were destroyed */
#define DOQUIT          3       /* Exit read menu was executed */
#define NEWDIRECT       4       /* Directory has changed, re-read files */
#define READ_NEXT       5       /* Direct read next file */
#define READ_PREV       6       /* Direct read prev file */
#define GOTO_NEXT       7       /* Move cursor to next */
#define DIRCHANGED      8       /* Index file was changed */
#define NEWSCREEN	9	/* split the screen */
#define CHANGEMODE  10  /* 换版面了或者是换模式了*/
#define SELCHANGE   11 /*选择变了,对应SHOW_SELCHANGE*/ 

/*
  阅读的键定义回调函数，参数依次为
  struct _select_def* conf 列表数据结构
  void* data 当前选择的数据
  void* extraarg 额外传递的参数
  */
typedef int (*READ_KEY_FUNC)(struct _select_def*,void*,void*);

/*
  显示每一项的回调函数
  参数依次为:
  char* outputbuf,输出的缓冲区
  int ent 位置
  void* data 数据
  void* readdata 用于显示相关数据的额外参数，比如同主题的主题文章。
  struct _select_def* conf 列表数据结构
  返回:
  显示的字符串。应该是一个指向outputbuf的指针
  */
typedef char *(*READ_ENT_FUNC) (char *, int, void *,void*,struct _select_def*);
int fileheader_thread_read(struct _select_def* conf, struct fileheader* fh,int ent, void* extraarg);
int find_nextnew(struct _select_def* conf,int begin);

struct key_command {                /* Used to pass commands to the readmenu */
    int key;
    READ_KEY_FUNC fptr;
    void* arg;
};

enum {
    READ_NORMAL,
    READ_THREAD,
    READ_NEW,
    READ_AUTHOR
};

struct read_arg {
    int returnvalue; /* 用于设置i_read的返回值*/
    /* save argument */
    enum BBS_DIR_MODE mode;
    enum BBS_DIR_MODE newmode; /*当返回NEWDIRECT的时候，设置这个*/
    char* direct;
    char* dingdirect;
    void (*dotitle) ();
    READ_ENT_FUNC doentry;
    struct key_command *rcmdlist;
    int ssize;
    int oldpos; /*在同主题阅读的时候，保存原始位置*/
    time_t lastupdatetime; /*上次更新时间*/

    struct write_dir_arg* writearg;

    int bid;
    struct boardheader* board;
    struct BoardStatus* boardstatus;

    /*用于确定当前的阅读模式，如果是
    READ_NORMAL  正常的顺序阅读
    READ_THREAD  正在主题阅读
    */
    int readmode; 

    char* data; //readed data
    int fd; //filehandle,open always

    void* readdata;

    int filecount; //the item count of file
};


/* 获得当前的pos所属的文件名,主要是为了区分置顶和普通的.DIR*/
char* read_getcurrdirect(struct _select_def* conf);

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
  @param applycurrent 是否对当前位置的fileheader应用func
  @param down 查找主题方向，如果是1,向下查找，否则向上
  @param arg 传递给func的参数
  @return 应用的主题个数
*/

int apply_thread(struct _select_def* conf, struct fileheader* fh,APPLY_THREAD_FUNC func,bool applycurrent, bool down,void* arg);

/*@param buf 显示的字符串
   @param num 序号
   @param data 数据
   @param readdata 上次阅读的数据,用于判断同主题之类
   @param conf _select_conf结构指针
*/

int new_i_read(enum BBS_DIR_MODE cmdmode, char *direct, void (*dotitle) (struct _select_def*), READ_ENT_FUNC doentry, struct key_command *rcmdlist, int ssize);

/* some function for fileheader */
int auth_search(struct _select_def* conf, struct fileheader *fileinfo,void* extraarg);
int title_search(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int thread_search(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int post_search(struct _select_def* conf, struct fileheader* fh, void* extraarg);

#define SR_FIRSTNEW     0
#define SR_FIRST            1
#define SR_LAST             2
#define SR_NEXT             3
#define SR_PREV             4
#define SR_READ            5    /*同主题阅读*/
#define SR_READX           6   /*同主题阅读，保存原始位置*/


#define SR_FIRSTNEWDOWNSEARCH 100

int thread_read(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int author_read(struct _select_def* conf, struct fileheader* fh, void* extraarg);

int read_sendmsgtoauthor(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int read_showauthor(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int read_authorinfo(struct _select_def *conf,struct fileheader *fh,void *arg);
int read_cross(struct _select_def* conf, struct fileheader* fh, void* extraarg);
int read_zsend(struct _select_def* conf, struct fileheader* fh, void* extraarg);


int read_addauthorfriend(struct _select_def* conf, struct fileheader* fh, void* extraarg);

int read_splitscreen(struct _select_def* conf, struct fileheader* fh, void* extraarg);

/*
    设置当前阅读的fileheader用于同主题判断
*/
void setreadpost(struct _select_def* conf,struct fileheader* fh);

int read_showauthorBM(struct _select_def* conf, struct fileheader* fh, void* extraarg);

int getPos(int mode,char* direct,struct boardheader* bh);
void savePos(int mode,char* direct,int pos,struct boardheader* bh);

/*调用无参数的函数*/
int read_callfunc0(struct _select_def* conf, void* data, void* extraarg);
/*判断两个标题是否是同主题*/
bool isThreadTitle(char* a,char* b);
#ifdef PERSONAL_CORP
int read_importpc(struct _select_def* conf, struct fileheader* fh, void* extraarg);
#endif

int prepare_write_dir(struct write_dir_arg *filearg, struct fileheader *fileinfo, int mode);

#endif  //__READ_H__
