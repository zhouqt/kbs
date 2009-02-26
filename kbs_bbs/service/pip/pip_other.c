/*---------------------------------------------------------------------------*/
/* 函式特区                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;

/*名字        体力MAX法力MAX  攻击   防护     速度    财宝   特别   图档*/
const struct playrule resultmanlist[] = {
    {"茱丽叶塔", 60, 0, 20, 0, 20, 20, 20, 150, "11101", 0, 0},
    {"菲欧利娜", 60, 0, 20, 0, 30, 30, 30, 200, "01111", 0, 0},
    {"阿妮斯", 80, 0, 40, 0, 50, 35, 60, 250, "11110", 0, 0},
    {"帕多雷西亚", 85, 0, 30, 0, 80, 90, 80, 500, "10111", 0, 0},
    {"卡美拉美", 90, 0, 50, 0, 75, 70, 60, 550, "11010", 0, 0},
    {"姗娜丽娃", 90, 0, 40, 0, 10, 30, 50, 880, "10100", 0, 0}
};

/*求婚*/
int
pip_marriage_offer()
{
    time_t now;
    char buf[256];
    char ans[4];
    int money;
    int who;
    const static char *name[5][2] = { {"女商人Ａ", "商人Ａ"},
        {"女商人Ｂ", "商人Ｂ"},
        {"女商人Ｃ", "商人Ｃ"},
        {"女商人Ｄ", "商人Ｄ"},
        {"女商人Ｅ", "商人Ｅ"}
    };

    do {
        who = rand() % 5;
    } while (d.lover == (who + 3));

    money = rand() % 2000 + rand() % 3000 + 4000;
    sprintf(buf, "%s带来了金钱%d，要向你的小鸡求婚，您愿意吗？[y/N]",
            name[who][d.sex - 1], money);
#ifdef MAPLE
    getdata(b_lines - 1, 1, buf, ans, 2, 1, 0);
#else
    getdata(b_lines-1,1,buf,ans,2,DOECHO,NULL,true);
#endif    // END MAPLE
    if (ans[0] == 'y' || ans[0] == 'Y') {
        if (d.wantend != 1 && d.wantend != 4) {
            sprintf(buf,
                    "ㄚ～之前已经有婚约了，您确定要解除旧婚约，改定立婚约吗？[y/N]");
#ifdef MAPLE
            getdata(b_lines - 1, 1, buf, ans, 2, 1, 0);
#else
            getdata(b_lines-1,1,buf,ans,2,DOECHO,NULL,true);
#endif    // END MAPLE
            if (ans[0] != 'y' && ans[0] != 'Y') {
                d.social += 10;
                temppress("还是维持旧婚约好了..");
                return 0;
            }
            d.social -= rand() % 50 + 100;
        }
        d.charm -= rand() % 5 + 20;
        d.lover = who + 3;
        d.relation -= 20;
        if (d.relation < 0)
            d.relation = 0;
        if (d.wantend < 4)
            d.wantend = 2;
        else
            d.wantend = 5;
        temppress("我想对方是一个很好的伴侣..");
        now = time(0);
        sprintf(buf,
                "\033[1;37m%s %-11s的小鸡 [%s] 接受了 %s 的求婚\033[0m\n",
                Cdate(now), getCurrentUser()->userid, d.name,
                name[who][d.sex - 1]);
        pip_log_record(buf);
    } else {
        d.charm += rand() % 5 + 20;
        d.relation += 20;
        if (d.wantend == 1 || d.wantend == 4) {
            temppress("我还年轻  心情还不定...");
        } else {
            temppress("我早已有婚约了..对不起...");
        }
    }
    d.money += money;
    return 0;
}

int
pip_results_show()
{    /*收获季 */
    const static char *showname[5] = { "  ", "武斗大会", "艺术大展", "皇家舞会", "烹饪大赛" };
    char buf[256];
    int pipkey, i = 0;
    int winorlost = 0;
    int a, b[3][2], c[3];

    clear();
    move(10, 14);
    prints("\033[1;33m叮咚叮咚～ 辛苦的邮差帮我们送信来了喔...\033[0m");
    temppress("嗯  把信打开看看吧...");
    clear();
    show_resultshow_pic(0);
    sprintf(buf, "[A]%s [B]%s [C]%s [D]%s [Q]放弃:", showname[1],
            showname[2], showname[3], showname[4]);
    move(b_lines, 0);
    prints(buf);
    do {
        pipkey = igetkey();
    } while (pipkey != 'q' && pipkey != 'Q' && pipkey != 'A' && pipkey != 'a'
             && pipkey != 'B' && pipkey != 'b' && pipkey != 'C'
             && pipkey != 'c' && pipkey != 'D' && pipkey != 'd');
    a = rand() % 4 + 1;
    b[0][0] = a - 1;
    b[1][0] = a + 1;
    b[2][0] = a;
    switch (pipkey) {
        case 'A':
        case 'a':
            temppress("今年共有四人参赛～现在比赛开始");
            for (i = 0; i < 3; i++) {
                a = 0;
                b[i][1] = 0;
                sprintf(buf, "你的第%d个对手是%s", i + 1,
                        resultmanlist[b[i][0]].name);
                temppress(buf);
                a = pip_vs_man(b[i][0], resultmanlist, 2);
                if (a == 1)
                    b[i][1] = 1; /*对方输了 */
                winorlost += a;
                d.death = 0;
            }
            switch (winorlost) {
                case 3:
                    pip_results_show_ending(3, 1, b[1][0], b[0][0],
                                            b[2][0]);
                    d.hexp += rand() % 10 + 50;
                    break;
                case 2:
                    if (b[0][1] != 1) {
                        c[0] = b[0][0];
                        c[1] = b[1][0];
                        c[2] = b[2][0];
                    } else if (b[1][1] != 1) {
                        c[0] = b[1][0];
                        c[1] = b[2][0];
                        c[2] = b[0][0];
                    } else if (b[2][1] != 1) {
                        c[0] = b[2][0];
                        c[1] = b[0][0];
                        c[2] = b[1][0];
                    } else
                        break;
                    pip_results_show_ending(2, 1, c[0], c[1], c[2]);
                    d.hexp += rand() % 10 + 30;
                    break;
                case 1:
                    if (b[0][1] == 1) {
                        c[0] = b[2][0];
                        c[1] = b[1][0];
                        c[2] = b[0][0];
                    } else if (b[1][1] == 1) {
                        c[0] = b[0][0];
                        c[1] = b[2][0];
                        c[2] = b[1][0];
                    } else if (b[2][1] == 1) {
                        c[0] = b[1][0];
                        c[1] = b[0][0];
                        c[2] = b[2][0];
                    } else
                        break;
                    pip_results_show_ending(1, 1, c[0], c[1], c[2]);
                    d.hexp += rand() % 10 + 10;
                    break;
                case 0:
                    pip_results_show_ending(0, 1, b[0][0], b[1][0],
                                            b[2][0]);
                    d.hexp -= rand() % 10 + 10;
                    break;
            }
            break;
        case 'B':
        case 'b':
            temppress("今年共有四人参赛～现在比赛开始");
            show_resultshow_pic(21);
            temppress("比赛情形");
            if ((d.art * 2 + d.character) / 400 >= 5) {
                winorlost = 3;
            } else if ((d.art * 2 + d.character) / 400 >= 4) {
                winorlost = 2;
            } else if ((d.art * 2 + d.character) / 400 >= 3) {
                winorlost = 1;
            } else {
                winorlost = 0;
            }
            pip_results_show_ending(winorlost, 2, rand() % 2,
                                    rand() % 2 + 2, rand() % 2 + 4);
            d.art += rand() % 10 + 20 * winorlost;
            d.character += rand() % 10 + 20 * winorlost;
            break;
        case 'C':
        case 'c':
            temppress("今年共有四人参赛～现在比赛开始");
            if ((d.art * 2 + d.charm) / 400 >= 5) {
                winorlost = 3;
            } else if ((d.art * 2 + d.charm) / 400 >= 4) {
                winorlost = 2;
            } else if ((d.art * 2 + d.charm) / 400 >= 3) {
                winorlost = 1;
            } else {
                winorlost = 0;
            }
            d.art += rand() % 10 + 20 * winorlost;
            d.charm += rand() % 10 + 20 * winorlost;
            pip_results_show_ending(winorlost, 3, rand() % 2,
                                    rand() % 2 + 4, rand() % 2 + 2);
            break;
        case 'D':
        case 'd':
            temppress("今年共有四人参赛～现在比赛开始");
            if ((d.affect + d.cookskill * 2) / 200 >= 4) {
                winorlost = 3;
            } else if ((d.affect + d.cookskill * 2) / 200 >= 3) {
                winorlost = 2;
            } else if ((d.affect + d.cookskill * 2) / 200 >= 2) {
                winorlost = 1;
            } else {
                winorlost = 0;
            }
            d.cookskill += rand() % 10 + 20 * winorlost;
            d.family += rand() % 10 + 20 * winorlost;
            pip_results_show_ending(winorlost, 4, rand() % 2 + 2,
                                    rand() % 2, rand() % 2 + 4);
            break;
        case 'Q':
        case 'q':
            temppress("今年不参加啦.....:(");
            d.happy -= rand() % 10 + 10;
            d.satisfy -= rand() % 10 + 10;
            d.relation -= rand() % 10;
            break;
    }
    if (pipkey != 'Q' && pipkey != 'q') {
        d.tired = 0;
        d.hp = d.maxhp;
        d.happy += rand() % 20;
        d.satisfy += rand() % 20;
        d.relation += rand() % 10;
    }
    return 0;
}

int
pip_results_show_ending(winorlost, mode, a, b, c)
int winorlost, mode, a, b, c;
{
    const static char *gamename[5] = { "  ", "武斗大会", "艺术大展", "皇家舞会", "烹饪大赛" };
    int resultmoney[4] = { 0, 3000, 5000, 8000 };
    char name1[25], name2[25], name3[25], name4[25];
    char buf[256];

    if (winorlost == 3) {
        strcpy(name1, d.name);
        strcpy(name2, resultmanlist[a].name);
        strcpy(name3, resultmanlist[b].name);
        strcpy(name4, resultmanlist[c].name);
    } else if (winorlost == 2) {
        strcpy(name1, resultmanlist[a].name);
        strcpy(name2, d.name);
        strcpy(name3, resultmanlist[b].name);
        strcpy(name4, resultmanlist[c].name);
    } else if (winorlost == 1) {
        strcpy(name1, resultmanlist[a].name);
        strcpy(name2, resultmanlist[b].name);
        strcpy(name3, d.name);
        strcpy(name4, resultmanlist[c].name);
    } else {
        strcpy(name1, resultmanlist[a].name);
        strcpy(name2, resultmanlist[b].name);
        strcpy(name3, resultmanlist[c].name);
        strcpy(name4, d.name);
    }
    clear();
    move(6, 13);
    prints("\033[1;37m～～～ \033[32m本届 %s 结果揭晓 \033[37m～～～\033[0m",
           gamename[mode]);
    move(8, 15);
    prints("\033[1;41m 冠军 \033[0;1m～ \033[1;33m%-10s\033[36m  奖金 %d\033[0m", name1,
           resultmoney[3]);
    move(10, 15);
    prints("\033[1;41m 亚军 \033[0;1m～ \033[1;33m%-10s\033[36m  奖金 %d\033[0m", name2,
           resultmoney[2]);
    move(12, 15);
    prints("\033[1;41m 季军 \033[0;1m～ \033[1;33m%-10s\033[36m  奖金 %d\033[0m", name3,
           resultmoney[1]);
    move(14, 15);
    prints("\033[1;41m 最后 \033[0;1m～ \033[1;33m%-10s\033[36m \033[0m", name4);
    sprintf(buf, "今年的%s结束罗 后年再来吧..", gamename[mode]);
    d.money += resultmoney[winorlost];
    temppress(buf);
    return 0;
}
