#ifndef _SELECT_H
#define _SELECT_H
#define SHOW_QUIT 0             /*函数退出 */
#define SHOW_DIRCHANGE 1        /*需要重新获取数据刷新列表 */
#define SHOW_REFRESH 2          /* 只重画列表 */
#define SHOW_REFRESHSELECT 3    /* 只重画选择的那一项 */
#define SHOW_SELCHANGE 4        /* 只改变了选择 */
#define SHOW_SELECT 5           /* 选择了列表之后返回 */
#define SHOW_CONTINUE 6         /*继续循环 */

#define	LF_MULTIPAGE 	0x1     /*多页 */
#define	LF_HILIGHTSEL	0x2     /*选择的行变色 */
#define	LF_VSCROLL	0x4     /*每个item占一行 */
#define	LF_NEEDFORCECLEAN 0x8
#define LF_FORCEREFRESHSEL 0x10
    /*
     * 如果每个item不等长，需要强制用空格清除 
     */
#define	LF_BELL			0x10    /*错误的时候响铃 */
#define	LF_LOOP		0x20    /*循环的，就是最后一个按后到最前一个，最前一个按前到最后一个 */
#define	LF_NUMSEL		0x40    /*用数字选择*/

#define  LF_ACTIVE	0x10000 /*列表被激活标志 */
#define  LF_INITED	0x20000 /*列表已经初始化完毕 */

/*扩展的功能键定义*/
#define KEY_REFRESH		0x1000
#define KEY_ACTIVE 		0x1001
#define KEY_DEACTIVE 	0x1002
#define KEY_SELECT		0x1003
#define KEY_INIT		0x1004
#define KEY_TALK		0x1005
#define KEY_TIMEOUT		0x1006

typedef struct tagPOINT {

    int x, y;
} POINT;


struct key_translate {

    int ch;
    int command;
};
struct _select_def {
    int flag;                   /*风格 */
    int item_count;             /*总item个数 */
    int item_per_page;          /*一页总共有几个item，一般应该等于list_linenum */
    POINT *item_pos;            /*一页里面每个item的位置，如果为空，则按一行一个排列 */
    POINT title_pos;            /*标题位置 */
    POINT endline_pos;          /*页末位置 */
    void *arg;                  /*其他传递的参数 */
/*	int page_num;  当前页号 */
    int pos;                    /* 当前选择的item位置 */
    int page_pos;               /*当前页的第一个item编号 */
    char *prompt;               /*选择的item前面的提示字符 */
    POINT cursor_pos;           /*光标位置 */
    int new_pos;                /*当返回SHOW_SELECTCHANGE 的时候，把新的位置放在这里 */
    struct key_translate *key_table;    /*键定义表 */
    /*
     * 内部使用的变量 
     */
    int tmpnum; /*用于定义了LF_NUMSEL的数字保存*/
    
    int (*init) (struct _select_def * conf);    /*初始化 */
    int (*page_init) (struct _select_def * conf);       /*翻页初始化，此时pos位置已经被改变了 */
    int (*get_data) (struct _select_def * conf, int pos, int len);      /*获得pos位置,长度为len的数据 */
    int (*show_data) (struct _select_def * conf, int pos);      /*显示pos位置的数据。 */
    int (*show_title) (struct _select_def * conf);      /*显示标题。 */
    int (*show_endline) (struct _select_def * conf);    /*显示行末。 */
    int (*pre_key_command) (struct _select_def * conf, int* command);        /*在上下左右被处理前处理键盘输入 */
    int (*key_command) (struct _select_def * conf, int command);        /*处理键盘输入 */
    void (*quit) (struct _select_def * conf);    /*结束 */
    int (*on_selchange) (struct _select_def * conf, int new_pos);       /*改变选择的时候的回调函数 */

    /*选择了某一个*/
    int (*on_select) (struct _select_def * conf);       
    
    int (*active) (struct _select_def * conf);  /*激活列表 */
    int (*deactive) (struct _select_def * conf);        /*列表失去焦点 */
};
int list_select(struct _select_def *conf, int key);
int list_select_loop(struct _select_def *conf);

/* 简单的选择框*/
#define SIF_SINGLE 0x1
#define SIF_NUMBERKEY	0x100  /* 数字选择0-9*/
#define SIF_ALPHAKEY	0x200  /*字母选择a-z*/
#define SIF_ESCQUIT	0x400  /* ESC退出 */
#define SIF_RIGHTSEL 0x800 /* 右键选择*/


#define SIT_SELECT	0x1
#define SIT_EDIT	0x2
#define SIT_CHECK	0x3

struct _select_item {
	int x,y;
	int hotkey;
	int type;
	void* data;
};

union _select_return_value {
	bool selected;
	char* returnstr;
};

int simple_select_loop(const struct _select_item* item_conf,int flag,int titlex,int titley,union _select_return_value* ret);
#endif

