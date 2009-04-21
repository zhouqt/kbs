
#include "bbs.h"

static int check_newpost(struct newpostdata *ptr);

// 检查版面是否有新帖子（这个函数是从boards_t.c里直接拿过来的）
static int check_newpost(struct newpostdata *ptr)
{
    struct BoardStatus *bptr;

    if (ptr->dir)
        return 0;

    ptr->total = ptr->unread = 0;

    bptr = getbstatus(ptr->pos+1);
    if (bptr == NULL)
        return 0;
    ptr->total = bptr->total;
    ptr->currentusers = bptr->currentusers;

#ifdef HAVE_BRC_CONTROL
    if (!brc_initial(getCurrentUser()->userid, ptr->name, getSession())) {
        ptr->unread = 1;
    } else {
        if (brc_board_unread(ptr->pos+1, getSession())) {
            ptr->unread = 1;
        }
    }
#endif
    ptr->lastpost = bptr->lastpost;
    return 1;
}


// 版面列表状态共用函数

// select的参数
struct ddd_read_list_arg {
    struct newpostdata *boardlist;
};

// select回调函数 读取版面列表
static int ddd_read_list_getdata(struct _select_def* conf, int pos, int len)
{
    struct ddd_read_list_arg *arg;
    char *prefix, buf[STRLEN];
    int sort;

    arg = (struct ddd_read_list_arg *)(conf->arg);
    sort = (getCurrentUser()->flags & BRDSORT_FLAG) ? ((getCurrentUser()->flags & BRDSORT1_FLAG) + 1) : 0;
    // 读取全部或分区版面列表
    if (DDD_GS_CURR.type == GS_ALL) {
        // 全部版面
        if (DDD_GS_CURR.sec == 0)
            prefix = NULL;
        // 分类讨论区的某一区
        else {
            sprintf(buf, "EGROUP%c", (char)(DDD_GS_CURR.sec));
            prefix = (char *)sysconf_str(buf);
        }
        conf->item_count = load_boards(arg->boardlist, prefix, 0, pos, len, sort, 0, NULL, getSession());
    }
    // 读取新分类讨论区或个人定制区
    else if ((DDD_GS_CURR.type == GS_NEW) || (DDD_GS_CURR.type == GS_FAV)) {
        conf->item_count = fav_loaddata(arg->boardlist, DDD_GS_CURR.favid, pos, len, sort, NULL, getSession());
    }
    return SHOW_CONTINUE;
}

// select回调函数 选择了某一个版面
static int ddd_read_list_onselect(struct _select_def* conf)
{
    struct ddd_read_list_arg *arg;
    struct newpostdata *ptr;

    arg = (struct ddd_read_list_arg *)(conf->arg);
    ptr = &(arg->boardlist[conf->pos - conf->page_pos]);
    // 如果是新分类讨论区或个人收藏夹的目录
    if(ptr->dir >= 1) {
        DDD_GS_NEW.type = DDD_GS_CURR.type;
        DDD_GS_NEW.favid = ptr->tag;
        DDD_GS_NEW.pos = 1;
        DDD_GS_NEW.recur = 1;
    }
    // 如果是目录版面
    else if (ptr->flag & BOARD_GROUP) {
        DDD_GS_NEW.type = GS_GROUP;
        DDD_GS_NEW.bid = getboardnum(ptr->name, NULL);
        DDD_GS_NEW.recur = 1;
    }
    // 如果是普通版面
    else {
        DDD_GS_NEW.type = GS_BOARD;
        DDD_GS_NEW.bid = getboardnum(ptr->name, NULL);
        DDD_GS_NEW.mode = DIR_MODE_NORMAL;
        DDD_GS_NEW.pos = 1;
        DDD_GS_NEW.recur = 1;
    }

    return SHOW_SELECT;
}

// select回调函数 显示版面信息
static int ddd_read_list_showdata(struct _select_def* conf, int pos)
{
    struct ddd_read_list_arg *arg;
    struct newpostdata *ptr;
    char flag[20], f, onlines[20], tmpBM[BM_LEN + 1];

    arg = (struct ddd_read_list_arg *)(conf->arg);
    ptr = &(arg->boardlist[pos - conf->page_pos]);

    // 目录版面包含的版面数
    if (ptr->flag & BOARD_GROUP) {
        prints(" %4d  ＋ ", ptr->total);
        strcpy(onlines, "    ");
    }
    // 普通版面的文章数、未读标记和在线人数
    else {
        check_newpost(ptr);
        prints(" %4d%s%s ", ptr->total, (ptr->total > 99999) ? "" : ((ptr->total > 9999) ? " " : "  "), ptr->unread ? "◆" : "◇");
        sprintf(onlines, "%4d", (ptr->currentusers > 9999) ? 9999 : ptr->currentusers);
    }
    // 是新分类讨论区或个人定制区的目录
    if(ptr->dir >= 1) {
        prints(" %s", ptr->title);
    // 是一般的版面或目录版面
    } else {
        // 俱乐部标记
        if ((ptr->flag & BOARD_CLUB_READ) && (ptr->flag & BOARD_CLUB_WRITE))
            f = 'A';
        else if (ptr->flag & BOARD_CLUB_READ)
            f = 'c';
        else if (ptr->flag & BOARD_CLUB_WRITE)
            f = 'p';
        else
            f = ' ';
        sprintf(flag, "\033[1;3%cm%c", (ptr->flag & BOARD_CLUB_HIDE) ? '1' : '3', f);
        // 版面英文名和投票标记
        prints(" %-16s%s%s", ptr->name, (ptr->flag & BOARD_VOTEFLAG) ? "\033[1;31mV" : " ", flag);
        // 只读标记和中文叙述
        if (checkreadonly(ptr->name))
            prints("\033[1;32m[只读]\033[m%-32s", ptr->title + 7);
        else
            prints("\033[m%-32s", ptr->title + 1);
        // 在线人数和版大
        strncpy(tmpBM, ptr->BM, BM_LEN);
        tmpBM[BM_LEN] = 0;
        prints(" %s %-12s", onlines, (ptr->BM[0] <= ' ') ? "诚征版主中" : strtok(tmpBM, " "));
    }
    return SHOW_CONTINUE;
}

// select回调函数 预处理按键
static int ddd_read_list_prekeycommand(struct _select_def* conf, int* command)
{
    return SHOW_CONTINUE;
}

// select回调函数 处理按键
static int ddd_read_list_keycommand(struct _select_def* conf, int command)
{
    return SHOW_CONTINUE;
}

// select回调函数 显示标题
static int ddd_read_list_showtitle(struct _select_def* conf)
{
    int sort;
    sort = (getCurrentUser()->flags & BRDSORT_FLAG) ? ((getCurrentUser()->flags & BRDSORT1_FLAG) + 1) : 0;
    clear();
    ddd_header();
    move(2, 0);
    prints("\033[1;37;44m  全部 未读 %s讨论区名称\033[1;37;44m       V 类别 转信  中  文  叙  述       %s在线\033[1;37;44m 版  主", (sort == 1) ? "\033[1;36;44m" : "", (sort & BRDSORT1_FLAG) ? "\033[1;36;44m" : "");
    clrtoeol();
    prints("\033[m");
    update_endline();
    return SHOW_CONTINUE;
}

// 版面列表状态的入口
int ddd_read_list()
{
    struct _select_def conf;
    struct ddd_read_list_arg arg;
    POINT *pts;
    int i, ret;

    // 计算列表中项目的显示位置
    pts = (POINT *)malloc(BBS_PAGESIZE * sizeof(POINT));
    for (i=0; i<BBS_PAGESIZE; i++) {
        pts[i].x = 1;
        pts[i].y = i + 3;
    }

    bzero((char *)&conf, sizeof(struct _select_def));
    bzero((char *)&arg, sizeof(struct ddd_read_list_arg));
    arg.boardlist = (struct newpostdata *)malloc(BBS_PAGESIZE * sizeof(struct newpostdata));

    conf.item_per_page = BBS_PAGESIZE;
    conf.flag = LF_NUMSEL | LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    conf.prompt = ">";
    conf.item_pos = pts;
    conf.arg = &arg;
    conf.pos = DDD_GS_CURR.pos;
    conf.page_pos = ((conf.pos - 1) / BBS_PAGESIZE) * BBS_PAGESIZE + 1;
    conf.get_data = ddd_read_list_getdata;
    conf.on_select = ddd_read_list_onselect;
    conf.show_data = ddd_read_list_showdata;
    conf.pre_key_command = ddd_read_list_prekeycommand;
    conf.key_command = ddd_read_list_keycommand;
    conf.show_title = ddd_read_list_showtitle;

    ret = list_select_loop(&conf);

    switch (ret) {
        case SHOW_QUIT:
            DDD_GS_NEW.type = GS_NONE;
            break;
    }

    free(arg.boardlist);
    free(pts);
    return 0;
}


// GS_ALL的入口 所有版面列表或者分区版面列表
int ddd_read_all() {
    return ddd_read_list();
}


// GS_NEW的入口 新分类讨论区
int ddd_read_new() {
    load_favboard(2, getSession());
    return ddd_read_list();
}


// GS_FAV的入口 个人定制区
int ddd_read_fav() {
    load_favboard(1, getSession());
    return ddd_read_list();
}

