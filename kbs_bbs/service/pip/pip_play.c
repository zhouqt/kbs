/*---------------------------------------------------------------------------*/
/* 玩乐选单:散步 旅游 运动 约会 猜拳                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;

int
pip_play_stroll()
{    /*散步 */
    int lucky;

    count_tired(3, 3, "Y", 100, 0);
    lucky = rand() % 7;
    if (lucky == 2) {
        d.happy += rand() % 3 + rand() % 3 + 9;
        d.satisfy += rand() % 3 + rand() % 3 + 3;
        d.shit += rand() % 3 + 3;
        d.hp -= (rand() % 3 + 5);
        move(4, 0);
        if (rand() % 2 > 0)
            show_play_pic(1);
        else
            show_play_pic(2);
        temppress("遇到朋友罗  真好.... ^_^");
    } else if (lucky == 3) {
        d.money += 100;
        d.happy += rand() % 3 + 6;
        d.satisfy += rand() % 3 + 4;
        d.shit += rand() % 3 + 3;
        d.hp -= (rand() % 3 + 4);
        move(4, 0);
        show_play_pic(3);
        temppress("捡到了100元了..耶耶耶....");
    }

    else if (lucky == 4) {
        if (rand() % 2 > 0) {
            d.happy -= (rand() % 2 + 5);
            move(4, 0);
            d.hp -= (rand() % 3 + 3);
            show_play_pic(4);
            if (d.money >= 50) {
                d.money -= 50;
                temppress("掉了50元了..呜呜呜....");
            } else {
                d.money = 0;
                d.hp -= (rand() % 3 + 3);
                temppress("钱掉光光了..呜呜呜....");
            }
            d.shit += rand() % 3 + 2;
        } else {
            d.happy += rand() % 3 + 5;
            move(4, 0);
            show_play_pic(5);
            if (d.money >= 50) {
                d.money -= 50;
                d.hp -= (rand() % 3 + 3);
                temppress("用了50元了..不可以骂我喔....");
            } else {
                d.money = 0;
                d.hp -= (rand() % 3 + 3);
                temppress("钱被我偷用光光了..:p");
            }
            d.shit += rand() % 3 + 2;
        }
    } else if (lucky == 5) {
        d.happy += rand() % 3 + 6;
        d.satisfy += rand() % 3 + 5;
        d.shit += 2;
        move(4, 0);
        if (rand() % 2 > 0)
            show_play_pic(6);
        else
            show_play_pic(7);
        temppress("好棒喔捡到玩具了说.....");
    } else if (lucky == 6) {
        d.happy -= (rand() % 3 + 10);
        d.shit += (rand() % 3 + 20);
        move(4, 0);
        show_play_pic(9);
        temppress("真是倒楣  可以去买爱国奖券");
    } else {
        d.happy += rand() % 3 + 3;
        d.satisfy += rand() % 2 + 1;
        d.shit += rand() % 3 + 2;
        d.hp -= (rand() % 3 + 2);
        move(4, 0);
        show_play_pic(8);
        temppress("没有特别的事发生啦.....");
    }
    return 0;
}

int
pip_play_sport()
{    /*运动 */
    count_tired(3, 8, "Y", 100, 1);
    d.weight -= (rand() % 3 + 2);
    d.satisfy += rand() % 2 + 3;
    if (d.satisfy > 100)
        d.satisfy = 100;
    d.shit += rand() % 5 + 10;
    d.hp -= (rand() % 2 + 8);
    d.maxhp += rand() % 2;
    d.speed += (2 + rand() % 3);
    move(4, 0);
    show_play_pic(10);
    temppress("运动好处多多啦...");
    return 0;
}

int
pip_play_date()
{    /*约会 */
    if (d.money < 150) {
        temppress("你钱不够多啦! 约会总得花点钱钱");
    } else {
        count_tired(3, 6, "Y", 100, 1);
        d.happy += rand() % 5 + 12;
        d.shit += rand() % 3 + 5;
        d.hp -= rand() % 4 + 8;
        d.satisfy += rand() % 5 + 7;
        d.character += rand() % 3 + 1;
        d.money = d.money - 150;
        move(4, 0);
        show_play_pic(11);
        temppress("约会去  呼呼");
    }
    return 0;
}

int
pip_play_outing()
{    /*郊游 */
    int lucky;
    char buf[256];

    if (d.money < 250) {
        temppress("你钱不够多啦! 旅游总得花点钱钱");
    } else {
        d.weight += rand() % 2 + 1;
        d.money -= 250;
        count_tired(10, 45, "N", 100, 0);
        d.hp -= rand() % 10 + 20;
        if (d.hp >= d.maxhp)
            d.hp = d.maxhp;
        d.happy += rand() % 10 + 12;
        d.character += rand() % 5 + 5;
        d.satisfy += rand() % 10 + 10;
        lucky = rand() % 4;
        if (lucky == 0) {
            d.maxmp += rand() % 3;
            d.art += rand() % 2;
            show_play_pic(12);
            if (rand() % 2 > 0)
                temppress
                ("心中有一股淡淡的感觉  好舒服喔....");
            else
                temppress("云水 闲情 心情好多了.....");
        } else if (lucky == 1) {
            d.art += rand() % 3;
            d.maxmp += rand() % 2;
            show_play_pic(13);
            if (rand() % 2 > 0)
                temppress
                ("有山有水有落日  形成一幅美丽的画..");
            else
                temppress("看着看着  全身疲惫都不见罗..");
        } else if (lucky == 2) {
            d.love += rand() % 3;
            show_play_pic(14);
            if (rand() % 2 > 0)
                temppress("看  太阳快没入水中罗...");
            else
                temppress("听说这是海边啦  你说呢?");
        } else if (lucky == 3) {
            d.maxhp += rand() % 3;
            show_play_pic(15);
            if (rand() % 2 > 0)
                temppress
                ("让我们疯狂在夜里的海滩吧....呼呼..");
            else
                temppress
                ("凉爽的海风迎面袭来  最喜欢这种感觉了....");
        }
        if ((rand() % 301 + rand() % 200) % 100 == 12) {
            lucky = 0;
            clear();
            sprintf(buf,
                    "\033[1;41m  星空战斗鸡 ～ %-10s                                                    \033[0m",
                    d.name);
            show_play_pic(0);
            move(17, 10);
            prints("\033[1;36m亲爱的 \033[1;33m%s ～\033[0m", d.name);
            move(18, 10);
            prints
            ("\033[1;37m看到你这样努力的培养自己的能力  让我心中十分的高兴喔..\033[m");
            move(19, 10);
            prints
            ("\033[1;36m小天使我决定给你奖赏鼓励鼓励  偷偷地帮助你一下....^_^\033[0m");
            move(20, 10);
            lucky = rand() % 7;
            if (lucky == 6) {
                prints
                ("\033[1;33m我将帮你的各项能力全部提升百分之五喔......\033[0m");
                d.maxhp = d.maxhp * 105 / 100;
                d.hp = d.maxhp;
                d.maxmp = d.maxmp * 105 / 100;
                d.mp = d.maxmp;
                d.attack = d.attack * 105 / 100;
                d.resist = d.resist * 105 / 100;
                d.speed = d.speed * 105 / 100;
                d.character = d.character * 105 / 100;
                d.love = d.love * 105 / 100;
                d.wisdom = d.wisdom * 105 / 100;
                d.art = d.art * 105 / 100;
                d.brave = d.brave * 105 / 100;
                d.homework = d.homework * 105 / 100;
            }

            else if (lucky <= 5 && lucky >= 4) {
                prints
                ("\033[1;33m我将帮你的战斗能力全部提升百分之十喔.......\033[0m");
                d.attack = d.attack * 110 / 100;
                d.resist = d.resist * 110 / 100;
                d.speed = d.speed * 110 / 100;
                d.brave = d.brave * 110 / 100;
            }

            else if (lucky <= 3 && lucky >= 2) {
                prints
                ("\033[1;33m我将帮你的魔法能力和生命力全部提升百分之十喔.......\033[0m");
                d.maxhp = d.maxhp * 110 / 100;
                d.hp = d.maxhp;
                d.maxmp = d.maxmp * 110 / 100;
                d.mp = d.maxmp;
            } else if (lucky <= 1 && lucky >= 0) {
                prints
                ("\033[1;33m我将帮你的感受能力全部提升百分之二十喔....\033[0m");
                d.character = d.character * 110 / 100;
                d.love = d.love * 110 / 100;
                d.wisdom = d.wisdom * 110 / 100;
                d.art = d.art * 110 / 100;
                d.homework = d.homework * 110 / 100;
            }

            temppress("请继续加油喔...");
        }
    }
    return 0;
}

int
pip_play_kite()
{    /*风筝 */
    count_tired(4, 4, "Y", 100, 0);
    d.weight += (rand() % 2 + 2);
    d.satisfy += rand() % 3 + 12;
    if (d.satisfy > 100)
        d.satisfy = 100;
    d.happy += rand() % 5 + 10;
    d.shit += rand() % 5 + 6;
    d.hp -= (rand() % 2 + 7);
    d.affect += rand() % 4 + 6;
    move(4, 0);
    show_play_pic(16);
    temppress("放风筝真好玩啦...");
    return 0;
}

int
pip_play_KTV()
/*KTV*/
{
    if (d.money < 250) {
        temppress("你钱不够多啦! 唱歌总得花点钱钱");
    } else {
        count_tired(10, 10, "Y", 100, 0);
        d.satisfy += rand() % 2 + 20;
        if (d.satisfy > 100)
            d.satisfy = 100;
        d.happy += rand() % 3 + 20;
        d.shit += rand() % 5 + 6;
        d.money -= 250;
        d.hp += (rand() % 2 + 6);
        d.art += rand() % 4 + 3;
        move(4, 0);
        show_play_pic(17);
        temppress("你说你  想要逃...");
    }
    return 0;
}

int
pip_play_guess()
{    /* 猜拳程式 */
    int com;
    int pipkey;
    struct tm *qtime;
    time_t now;

    time(&now);
    qtime = localtime(&now);
    d.satisfy += (rand() % 3 + 2);
    count_tired(2, 2, "Y", 100, 1);
    d.shit += rand() % 3 + 2;
    do {
        if (d.death == 1 || d.death == 2 || d.death == 3)
            return 0;
        if (pip_mainmenu(0))
            return 0;
        move(b_lines - 2, 0);
        clrtoeol();
        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        prints
        ("\033[1;44;37m  猜拳选单  \033[46m[1]我出剪刀 [2]我出石头 [3]我出布啦 [4]猜拳记录 [Q]跳出：         \033[m");
        move(b_lines - 1, 0);
        clrtoeol();
        pipkey = igetkey();
        switch (pipkey) {
#ifdef MAPLE
            case Ctrl('R'):
                if (currutmp->msgs[0].last_pid) {
                    show_last_call_in();
                    my_write(currutmp->msgs[0].last_pid,
                             "水球丢回去：");
                }
                break;
#endif    // END MAPLE
            case '4':
                situ();
                break;
        }
    } while ((pipkey != '1') && (pipkey != '2') && (pipkey != '3')
             && (pipkey != 'q') && (pipkey != 'Q'));

    com = rand() % 3;
    move(18, 0);
    clrtobot();
    switch (com) {
        case 0:
            outs("小鸡：剪刀\n");
            break;
        case 1:
            outs("小鸡：石头\n");
            break;
        case 2:
            outs("小鸡：布\n");
            break;
    }

    move(17, 0);

    switch (pipkey) {
        case '1':
            outs("你  ：剪刀\n");
            if (com == 0)
                tie();
            else if (com == 1)
                lose();
            else if (com == 2)
                win();
            break;
        case '2':
            outs("你　：石头\n");
            if (com == 0)
                win();
            else if (com == 1)
                tie();
            else if (com == 2)
                lose();
            break;
        case '3':
            outs("你　：布\n");
            if (com == 0)
                lose();
            else if (com == 1)
                win();
            else if (com == 2)
                tie();
            break;
        case 'q':
            break;
    }
    return 0;
}

int
win()
{
    d.winn++;
    d.hp -= rand() % 2 + 3;
    move(4, 0);
    show_guess_pic(2);
    move(b_lines, 0);
    temppress("小鸡输了....~>_<~");
    return 0;
}

int
tie()
{
    d.hp -= rand() % 2 + 3;
    d.happy += rand() % 3 + 5;
    move(4, 0);
    show_guess_pic(3);
    move(b_lines, 0);
    temppress("平手........-_-");
    return 0;
}

int
lose()
{
    d.losee++;
    d.happy += rand() % 3 + 5;
    d.hp -= rand() % 2 + 3;
    move(4, 0);
    show_guess_pic(1);
    move(b_lines, 0);
    temppress("小鸡赢罗....*^_^*");
    return 0;
}

int
situ()
{
    clrchyiuan(19, 21);
    move(19, 0);
    prints("你:\033[44m %d胜 %d负\033[m                     \n", d.winn, d.losee);
    move(20, 0);
    prints("鸡:\033[44m %d胜 %d负\033[m                     \n", d.losee, d.winn);

    if (d.winn >= d.losee) {
        move(b_lines, 0);
        temppress("哈..赢小鸡也没多光荣");
    } else {
        move(b_lines, 0);
        temppress("笨蛋..竟输给了鸡....ㄜ...");
    }
    return 0;
}

