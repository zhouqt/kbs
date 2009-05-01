
#include "bbs.h"

// 初始化状态结构体
int grl_gs_init(struct grl_global_status* gs)
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
int grl_entry()
{
    char ans[2];

    grl_gs_init(&GRL_GS_CURR);
    grl_gs_init(&GRL_GS_NEW);

    while (true) {
        clear();
        grl_header();
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
            GRL_GS_CURR.type = GS_ALL;
            GRL_GS_CURR.sec = 0;
            GRL_GS_CURR.pos = 1;
        } else if ((ans[0] >= '0') && (ans[0] <= '9')) {
            GRL_GS_CURR.type = GS_ALL;
            GRL_GS_CURR.sec = (int)ans[0];
            GRL_GS_CURR.pos = 1;
        } else if ((ans[0] == 'x') || (ans[0] == 'X')) {
            GRL_GS_CURR.type = GS_NEW;
            GRL_GS_CURR.favid = 0;
            GRL_GS_CURR.pos = 1;
        } else if ((ans[0] == 'f') || (ans[0] == 'F')) {
            GRL_GS_CURR.type = GS_FAV;
            GRL_GS_CURR.favid = 0;
            GRL_GS_CURR.pos = 1;
        } else if ((ans[0] == 's') || (ans[0] == 'S')) {
            int bid, type;
            if(grl_choose_board(&bid, &type)) {
                GRL_GS_CURR.type = type;
                GRL_GS_CURR.bid = bid;
                GRL_GS_CURR.mode = DIR_MODE_NORMAL;
                GRL_GS_CURR.pos = 1;
            }
            else
                GRL_GS_CURR.type = GS_NONE;
        } else if (ans[0] == 0)
            break;
        grl_read_loop();
    }

    return 0;
}

// 主循环
int grl_read_loop()
{
    struct grl_global_status gs_this_level;
    while (true) {
        memcpy(&GRL_GS_NEW, &GRL_GS_CURR, sizeof(struct grl_global_status));
        // 根据当前状态的类型进入不同的阅读函数
        switch (GRL_GS_CURR.type) {

            case GS_NONE:
                break;

            case GS_ALL:
                grl_read_all();
                break;

            case GS_NEW:
                grl_read_new();
                break;

            case GS_FAV:
                grl_read_fav();
                break;

            case GS_GROUP:
                grl_read_group();
                break;

            default:
                grl_read_unknown();
                break;
        }
        // 如果新状态需要递归
        if (GRL_GS_NEW.recur) {
            // 记录此层状态
            GRL_GS_NEW.recur = 0;
            memcpy(&gs_this_level, &GRL_GS_CURR, sizeof(struct grl_global_status));
            memcpy(&GRL_GS_CURR, &GRL_GS_NEW, sizeof(struct grl_global_status));
            // 递归进去
            grl_read_loop();
            // 恢复此层状态
            memcpy(&GRL_GS_CURR, &gs_this_level, sizeof(struct grl_global_status));
        }
        // 如果新状态不需要递归
        else
            memcpy(&GRL_GS_CURR, &GRL_GS_NEW, sizeof(struct grl_global_status));
        if (GRL_GS_CURR.type == GS_NONE)
            break;
    }
    return 0;
}

// 显示站点标题
int grl_header()
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
    switch (GRL_GS_CURR.type) {
        case GS_ALL:
            strcpy(lefttxt, "[讨论区列表]");
            if (GRL_GS_CURR.sec == 0)
                strcpy(righttxt, "所有版面");
            else
                sprintf(righttxt, "第%c区", (char)(GRL_GS_CURR.sec));
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
            sprintf(righttxt, "目录[%d]", GRL_GS_CURR.bid);
            break;
        case GS_BOARD:
            strcpy(lefttxt, "版主:");
            sprintf(righttxt, "版面[%d]", GRL_GS_CURR.bid);
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
    prints(" \033[1;32m大打倒全局状态\033[m  type=%d  sec=%d  favid=%d  bid=%d  mode=%d  filter=%d", GRL_GS_CURR.type, GRL_GS_CURR.sec, GRL_GS_CURR.favid, GRL_GS_CURR.bid, GRL_GS_CURR.mode, GRL_GS_CURR.filter);
    return 0;
}

// 搞不清楚的时候就显示这个
int grl_read_unknown()
{
    clear();
    grl_header();
    move(3, 3);
    prints("未知阅读状态。");
    update_endline();
    WAIT_RETURN;
    GRL_GS_NEW.type = GS_NONE;
    GRL_GS_NEW.recur = 0;
    return 0;
}

// 让用户选择版面
int grl_choose_board(int* bid, int* type) {
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

