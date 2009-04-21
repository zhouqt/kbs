
#include "bbs.h"

// 初始化状态结构体
int ddd_gs_init(struct ddd_global_status* gs)
{
    gs->type = GS_NONE;
    gs->sec = 0;
    gs->favid = 0;
    gs->bid = 0;
    gs->mode = DIR_MODE_NORMAL;
    gs->pos = 0;
    gs->filter = 0;
    gs->recur = 0;
    return 0;
}

// 测试入口
int ddd_entry()
{
    char ans[2];

    ddd_gs_init(&DDD_GS_CURR);
    ddd_gs_init(&DDD_GS_NEW);

    while (true) {
        clear();
        ddd_header();
        move(3, 3);
        prints("大打倒测试主菜单");
        move(5, 3);
        prints("(B) 所有版面列表");
        move(6, 3);
        prints("(0-9) 分类讨论区");
        move(7, 3);
        prints("(X) 新分类讨论区");
        move(8, 3);
        prints("(F) 个人定制区");
        move(9, 3);
        prints("(S) 选择版面");
        move(10, 3);
        prints("(M) 信箱");
        update_endline();
        getdata(12, 3, "请选择: ", ans, 2, DOECHO, NULL, true);
        if ((ans[0] == 'b') || (ans[0] == 'B')) {
            DDD_GS_CURR.type = GS_ALL;
            DDD_GS_CURR.sec = 0;
            DDD_GS_CURR.pos = 1;
        } else if ((ans[0] >= '0') && (ans[0] <= '9')) {
            DDD_GS_CURR.type = GS_ALL;
            DDD_GS_CURR.sec = (int)ans[0];
            DDD_GS_CURR.pos = 1;
        } else if ((ans[0] == 'x') || (ans[0] == 'X')) {
            DDD_GS_CURR.type = GS_NEW;
            DDD_GS_CURR.favid = 0;
            DDD_GS_CURR.pos = 1;
        } else if ((ans[0] == 'f') || (ans[0] == 'F')) {
            DDD_GS_CURR.type = GS_FAV;
            DDD_GS_CURR.favid = 0;
            DDD_GS_CURR.pos = 1;
        } else if ((ans[0] == 's') || (ans[0] == 'S')) {
            int bid, type;
            if(ddd_choose_board(&bid, &type)) {
                DDD_GS_CURR.type = type;
                DDD_GS_CURR.bid = bid;
                DDD_GS_CURR.pos = 1;
            }
            else
                DDD_GS_CURR.type = GS_NONE;
        } else if (ans[0] == 0)
            break;
        ddd_read_loop();
    }

    return 0;
}

// 主循环
int ddd_read_loop()
{
    struct ddd_global_status gs_this_level;
    while (true) {
        memcpy(&DDD_GS_NEW, &DDD_GS_CURR, sizeof(struct ddd_global_status));
        // 根据当前状态的类型进入不同的阅读函数
        switch (DDD_GS_CURR.type) {

            case GS_NONE:
                break;

            case GS_ALL:
                ddd_read_all();
                break;

            case GS_NEW:
                ddd_read_new();
                break;

            case GS_FAV:
                ddd_read_fav();
                break;

            case GS_GROUP:
                ddd_read_group();
                break;

            default:
                ddd_read_unknown();
                break;
        }
        // 如果新状态需要递归
        if (DDD_GS_NEW.recur) {
            // 记录此层状态
            DDD_GS_NEW.recur = 0;
            memcpy(&gs_this_level, &DDD_GS_CURR, sizeof(struct ddd_global_status));
            memcpy(&DDD_GS_CURR, &DDD_GS_NEW, sizeof(struct ddd_global_status));
            // 递归进去
            ddd_read_loop();
            // 恢复此层状态
            memcpy(&DDD_GS_CURR, &gs_this_level, sizeof(struct ddd_global_status));
        }
        // 如果新状态不需要递归
        else
            memcpy(&DDD_GS_CURR, &DDD_GS_NEW, sizeof(struct ddd_global_status));
        if (DDD_GS_CURR.type == GS_NONE)
            break;
    }
    return 0;
}

// 显示站点标题
int ddd_header()
{
    int colour, centerpos, rightpos, l1, l2;
    char lefttxt[STRLEN], righttxt[STRLEN];

    // 自动颜色变换
    if (DEFINE(getCurrentUser(), DEF_TITLECOLOR)) {
        colour = 4;
    } else {
        colour = getCurrentUser()->numlogins % 4 + 3;
        if (colour == 3)
            colour = 1;
    }

    move(0, 0);
    setbcolor(colour);
    switch (DDD_GS_CURR.type) {
        case GS_ALL:
            strcpy(lefttxt, "[讨论区列表]");
            if (DDD_GS_CURR.sec == 0)
                strcpy(righttxt, "所有版面");
            else
                sprintf(righttxt, "第%c区", (char)(DDD_GS_CURR.sec));
            break;
        case GS_NEW:
            strcpy(lefttxt, "[讨论区列表]");
            strcpy(righttxt, "新分类讨论区");
            break;
        case GS_FAV:
            strcpy(lefttxt, "[个人定制区]");
            strcpy(righttxt, "");
            break;
        case GS_GROUP:
            strcpy(lefttxt, "[讨论区列表]");
            sprintf(righttxt, "目录[%d]", DDD_GS_CURR.bid);
            break;
        case GS_BOARD:
            strcpy(lefttxt, "版主:");
            sprintf(righttxt, "版面[%d]", DDD_GS_CURR.bid);
            break;
        case GS_MAIL:
            strcpy(lefttxt, "[信箱]");
            strcpy(righttxt, "");
            break;
        default:
            strcpy(lefttxt, "我在哪里？");
            strcpy(righttxt, "我在哪里？");
            break;
    }
    // 计算中间的位置
    l1 = strlen(lefttxt);
    l2 = strlen(righttxt);
    centerpos = l1 + (scr_cols - l1 - l2 - strlen(BBS_FULL_NAME)) / 2;
    rightpos = scr_cols - l2;
    prints("\033[1;33m%s", lefttxt);
    clrtoeol();
    move(0, centerpos);
    prints("\033[1;37m%s", BBS_FULL_NAME);
    move(0, rightpos);
    prints("\033[1;33m%s", righttxt);
    prints("\033[m\n");
    prints(" \033[1;32m大打倒全局状态\033[m  type=%d  sec=%d  favid=%d  bid=%d  mode=%d  filter=%d", DDD_GS_CURR.type, DDD_GS_CURR.sec, DDD_GS_CURR.favid, DDD_GS_CURR.bid, DDD_GS_CURR.mode, DDD_GS_CURR.filter);
    return 0;
}

// 搞不清楚的时候就显示这个
int ddd_read_unknown()
{
    clear();
    ddd_header();
    move(3, 3);
    prints("未知阅读状态。");
    update_endline();
    WAIT_RETURN;
    DDD_GS_NEW.type = GS_NONE;
    DDD_GS_NEW.recur = 0;
    return 0;
}

// 让用户选择版面
int ddd_choose_board(int* bid, int* type) {
    char bname[STRLEN];
    int ret;
    struct boardheader *bh;
    // 提示选择讨论区
    move(1, 0);
    clrtoeol();
    prints("选择讨论区: ");
    make_blist(0, 1);
    // 运行自动补齐
    ret = namecomplete(NULL, bname);
    // 如果没选择
    if(bname[0] == 0)
        return 0;
    *bid = getbnum_safe(bname, getSession(), 1);
    if(*bid == 0) {
        move(2, 0);
        prints("错误的讨论区。");
        return 0;
    }
    bh = getboard(*bid);
    if(bh->flag & BOARD_GROUP)
        *type = GS_GROUP;
    else
        *type = GS_BOARD;
    return 1;
}

