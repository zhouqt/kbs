/*---------------------------------------------------------------------------*/
/* 基本选单:饮食 清洁 亲亲 休息                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;

int
pip_basic_takeshower()
{    /*洗澡 */
    int lucky;

    d.shit -= 20;
    if (d.shit < 0)
        d.shit = 0;
    d.hp -= rand() % 2 + 3;
    move(4, 0);
    lucky = rand() % 3;
    if (lucky == 0) {
        show_usual_pic(1);
        temppress("我是干净的小鸡  cccc....");
    } else if (lucky == 1) {
        show_usual_pic(7);
        temppress("马桶 嗯～～");
    } else {
        show_usual_pic(2);
        temppress("我爱洗澡 lalala....");
    }
    return 0;
}

int
pip_basic_takerest()
{    /*休息 */
//     count_tired(5,20,"Y",100,0);
    d.tired -= rand() % 15;
    d.tired = d.tired > 0 ? d.tired : 0;

    if (d.hp > d.maxhp)
        d.hp = d.maxhp;
    d.shit += 1;
    move(4, 0);
    show_usual_pic(5);
    temppress("再按一下我就起床罗....");
    show_usual_pic(6);
    temppress("喂喂喂..该起床罗......");
    return 0;
}

int
pip_basic_kiss()
{    /*亲亲 */
    if (rand() % 2 > 0) {
        d.happy += rand() % 3 + 4;
        d.satisfy += rand() % 2 + 1;
    } else {
        d.happy += rand() % 2 + 1;
        d.satisfy += rand() % 3 + 4;
    }
    count_tired(1, 2, "N", 100, 1);
    d.shit += rand() % 5 + 4;
    d.relation += rand() % 2;
    move(4, 0);
    show_usual_pic(3);
    if (d.shit < 60) {
        temppress("来嘛! 啵一个.....");
    } else {
        temppress("亲太多也是会脏死的喔....");
    }
    return 0;
}

int
pip_basic_feed()
{    /* 饮食 */
    time_t now;
    char buf[256];
    int pipkey;

    d.nodone = 1;

    do {
        if (d.death == 1 || d.death == 2 || d.death == 3)
            return 0;
        if (pip_mainmenu(1))
            return 0;
        move(b_lines - 2, 0);
        clrtoeol();
        move(b_lines - 2, 1);
        sprintf(buf, "%s该做什麽事呢?", d.name);
        prints(buf);
        now = time(0);
        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        prints
        ("\033[1;44;37m  饮食选单  \033[46m[1]吃饭 [2]零食 [3]补丸 [4]灵芝 [5]人参 [6]雪莲 [Q]跳出：         \033[m");
        pip_time_change(now);
        pipkey = igetkey();
        pip_time_change(now);

        switch (pipkey) {
            case '1':
                if (d.food <= 0) {
                    move(b_lines, 0);
                    temppress("没有食物罗..快去买吧！");
                    break;
                }
                move(4, 0);
                if ((d.bbtime / 60 / 30) < 3)
                    show_feed_pic(0);
                else
                    show_feed_pic(1);
                d.food--;
                d.hp += 50;
                if (d.hp >= d.maxhp) {
                    d.hp = d.maxhp;
                    d.weight += rand() % 2;
                }
                d.nodone = 0;
                temppress("每吃一次食物会恢复体力50喔!");
                break;

            case '2':
                if (d.cookie <= 0) {
                    move(b_lines, 0);
                    temppress("零食吃光罗..快去买吧！");
                    break;
                }
                move(4, 0);
                d.cookie--;
                d.hp += 100;
                if (d.hp >= d.maxhp) {
                    d.hp = d.maxhp;
                    d.weight += (rand() % 2 + 2);
                } else {
                    d.weight += (rand() % 2 + 1);
                }
                if (rand() % 2 > 0)
                    show_feed_pic(2);
                else
                    show_feed_pic(3);
                d.happy += (rand() % 3 + 4);
                d.satisfy += rand() % 3 + 2;
                d.nodone = 0;
                temppress("吃零食容易胖喔...");
                break;

            case '3':
                if (d.bighp <= 0) {
                    move(b_lines, 0);
                    temppress("没有大补丸了耶! 快买吧..");
                    break;
                }
                d.bighp--;
                d.hp += 600;
                d.tired -= 20;
                d.weight += rand() % 2;
                move(4, 0);
                show_feed_pic(4);
                d.nodone = 0;
                temppress("补丸..超极棒的唷...");
                break;

            case '4':
                if (d.medicine <= 0) {
                    move(b_lines, 0);
                    temppress("没有灵芝罗..快去买吧！");
                    break;
                }
                move(4, 0);
                show_feed_pic(1);
                d.medicine--;
                d.mp += 50;
                if (d.mp >= d.maxmp) {
                    d.mp = d.maxmp;
                }
                d.nodone = 0;
                temppress("每吃一次灵芝会恢复法力50喔!");
                break;

            case '5':
                if (d.ginseng <= 0) {
                    move(b_lines, 0);
                    temppress("没有千年人参耶! 快买吧..");
                    break;
                }
                d.ginseng--;
                d.mp += 500;
                d.tired -= 20;
                move(4, 0);
                show_feed_pic(1);
                d.nodone = 0;
                temppress("千年人参..超极棒的唷...");
                break;

            case '6':
                if (d.snowgrass <= 0) {
                    move(b_lines, 0);
                    temppress("没有天山雪莲耶! 快买吧..");
                    break;
                }
                d.snowgrass--;
                d.mp = d.maxmp;
                d.hp = d.maxhp;
                d.tired -= 0;
                d.sick = 0;
                move(4, 0);
                show_feed_pic(1);
                d.nodone = 0;
                temppress("天山雪莲..超极棒的唷...");
                break;

#ifdef MAPLE
            case Ctrl('R'):
                if (currutmp->msgs[0].last_pid) {
                    show_last_call_in();
                    my_write(currutmp->msgs[0].last_pid,
                             "水球丢回去：");
                }
                d.nodone = 0;
                break;
#endif    // END MAPLE
        }
    } while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));

    return 0;
}
