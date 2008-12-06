#include "bbs.h"

int ddd_gs_init(struct ddd_global_status* gs) {
    gs->type = GS_NONE;
    gs->sec = 0;
    gs->favid = 0;
    gs->bid = 0;
    gs->mode = DIR_MODE_NORMAL;
    gs->filter = 0;
    return 0;
}

int ddd_entry() {
    char ans[2];

    ddd_gs_init(&DDD_GS_CURR); 
    ddd_gs_init(&DDD_GS_NEW);

    while(true) {
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
        if(ans[0] == 0)
            break;
        ddd_read_loop();
    }

    return 0;
}

int ddd_read_loop() {
    struct ddd_global_status gs_this_level;
    while(true) {
        switch(DDD_GS_CURR.type) {
            
        default:
            ddd_unknown();
            break;
        }
        if(DDD_GS_CURR.type == GS_NONE)
            break;
    }
    return 0;
}

int ddd_header() {
    move(0, 0);
    prints("\033[1;36;44m(ddd) \033[37m%s  \033[33m", BBS_FULL_NAME);
    switch(DDD_GS_CURR.type) {
    case GS_ALL:
        if(DDD_GS_CURR.sec == -1)
            prints("所有版面");
        else
            prints("分类讨论区第%d区", DDD_GS_CURR.sec);
        break;
    case GS_NEW:
        prints("新分类讨论区");
        break;
    case GS_FAV:
        prints("个人定制区");
        break;
    case GS_GROUP:
        prints("目录版面[%d]", DDD_GS_CURR.bid);
        break;
    case GS_BOARD:
        prints("版面[%d]", DDD_GS_CURR.bid);
        break;
    case GS_MAIL:
        prints("信箱");
        break;
    default:
        prints("没在大打倒主循环");
        break;
    }
    clrtoeol();
    prints("\033[m\n");
    prints(" type=%d  sec=%d  favid=%d  bid=%d  mode=%d  filter=%d", DDD_GS_CURR.type, DDD_GS_CURR.sec, DDD_GS_CURR.favid, DDD_GS_CURR.bid, DDD_GS_CURR.mode, DDD_GS_CURR.filter);
    return 0;
}

int ddd_unknown() {
    clear();
    move(3, 3);
    prints("未知阅读状态。");
    WAIT_RETURN;
    return 0;
}

