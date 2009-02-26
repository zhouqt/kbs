/*---------------------------------------------------------------------------*/
/* 特殊选单:看病 减肥 战斗 拜访 朝见                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;

const struct royalset royallist[] = {
    {"T", "拜访对象", 0, 0, 0, 0, "", "" /*NULL,NULL */ },
    {"A", "星空骑兵连", 1, 10, 15, 100, "你真好，来陪我聊天..",
     "守卫星空的安全是很辛苦的.."},
    {"B", "星空００７", 1, 100, 25, 200, "真是礼貌的小鸡..我喜欢...",
     "特务就是秘密保护站长安全的人.."},
    {"C", "镇国大将军", 1, 200, 30, 250, "告诉你唷！当年那个战役很精彩喔..",
     "你真是高贵优雅的小鸡..."},
    {"D", "参谋总务长", 1, 300, 35, 300, "我帮站长管理这个国家唷..",
     "你的声音很好听耶..我很喜欢喔...:)"},
    {"E", "小天使站长", 1, 400, 35, 300, "你很有教养唷！很高兴认识你...",
     "优雅的你，请让我帮你祈福...."},
    {"F", "风筝手站长", 1, 500, 40, 350, "你好可爱喔..我喜欢你唷....",
     "对啦..以后要多多来和我玩喔..."},
    {"G", "乖小孩站长", 1, 550, 40, 350,
     "跟你讲话很快乐喔..不像站长一样无聊..",
     "来，坐我膝盖上，听我讲故事.."},
    {"H", "小米克站长", 1, 600, 50, 400, "一站之长责任重大呀..:)..",
     "谢谢你听我讲话..以后要多来喔..."},
    {"I", "星空灌水群", 2, 60, 0, 0, "不错唷..蛮机灵的喔..很可爱....",
     "来  我们一起来灌水吧...."},
    {"J", "青年帅武官", 0, 0, 0, 0,
     "你好，我是武官，刚从银河边境回来休息..",
     "希望下次还能见到你...:)"},
//NULL,         NULL,NULL,    NULL,    NULL,NULL,NULL
//{NULL,                        0,      0,      0,      0, NULL, NULL}
};

int
pip_see_doctor()
{    /*看医生 */
    char buf[256];
    long savemoney;

    savemoney = d.sick * 25;
    if (d.sick <= 0) {
        temppress("哇哩..没病来医院干嘛..被骂了..呜~~");
        d.character -= (rand() % 3 + 1);
        if (d.character < 0)
            d.character = 0;
        d.happy -= (rand() % 3 + 3);
        d.satisfy -= rand() % 3 + 2;
    } else if (d.money < savemoney) {
        sprintf(buf, "你的病要花 %ld 元喔....你不够钱啦...", savemoney);
        temppress(buf);
    } else if (d.sick > 0 && d.money >= savemoney) {
        d.tired -= rand() % 10 + 20;
        if (d.tired < 0)
            d.tired = 0;
        d.sick = 0;
        d.money = d.money - savemoney;
        move(4, 0);
        show_special_pic(1);
        temppress("药到病除..没有副作用!!");
    }
    return 0;
}

/*减肥*/
int
pip_change_weight()
{
    char genbuf[5];
    char inbuf[256];
    int weightmp;

    move(b_lines - 1, 0);
    clrtoeol();
    show_special_pic(2);
#ifdef MAPLE
    getdata(b_lines - 1, 1, "你的选择是? [Q]离开:", genbuf, 2, 1, 0);
#else
    getdata(b_lines-1,1,"你的选择是? [Q]离开:",genbuf,2,DOECHO,NULL,true);
#endif    // END MAPLE
    if (genbuf[0] == '1' || genbuf[0] == '2' || genbuf[0] == '3'
            || genbuf[0] == '4') {
        switch (genbuf[0]) {
            case '1':
                if (d.money < 80) {
                    temppress
                    ("传统增胖要80元喔....你不够钱啦...");
                } else {
#ifdef MAPLE
                    getdata(b_lines - 1, 1,
                            "需花费80元(3～5公斤)，你确定吗? [y/N]",
                            genbuf, 2, 1, 0);
#else
                    getdata(b_lines-1,1,"需花费80元(3～5公斤)，你确定吗? [y/N]",genbuf,2,DOECHO,NULL,true);
#endif    // END MAPLE
                    if (genbuf[0] == 'Y' || genbuf[0] == 'y') {
                        weightmp = 3 + rand() % 3;
                        d.weight += weightmp;
                        d.money -= 80;
                        d.maxhp -= rand() % 2;
                        d.hp -= rand() % 2 + 3;
                        show_special_pic(3);
                        sprintf(inbuf, "总共增加了%d公斤",
                                weightmp);
                        temppress(inbuf);
                    } else {
                        temppress("回心转意罗.....");
                    }
                }
                break;

            case '2':
#ifdef MAPLE
                getdata(b_lines - 1, 1,
                        "增一公斤要30元，你要增多少公斤呢? [请填数字]:",
                        genbuf, 4, 1, 0);
#else
                getdata(b_lines-1,1,"增一公斤要30元，你要增多少公斤呢? [请填数字]:",genbuf,4,DOECHO,NULL,true);
#endif    // END MAPLE
                weightmp = atoi(genbuf);
                if (weightmp <= 0) {
                    temppress("输入有误..放弃罗...");
                } else if (d.money > (weightmp * 30)) {
                    sprintf(inbuf,
                            "增加%d公斤，总共需花费了%d元，确定吗? [y/N]",
                            weightmp, weightmp * 30);
#ifdef MAPLE
                    getdata(b_lines - 1, 1, inbuf, genbuf, 2, 1, 0);
#else
                    getdata(b_lines-1,1,inbuf,genbuf,2,DOECHO,NULL,true);
#endif    // END MAPLE
                    if (genbuf[0] == 'Y' || genbuf[0] == 'y') {
                        d.money -= weightmp * 30;
                        d.weight += weightmp;
                        d.maxhp -= (rand() % 2 + 2);
                        count_tired(5, 8, "N", 100, 1);
                        d.hp -= (rand() % 2 + 3);
                        d.sick += rand() % 10 + 5;
                        show_special_pic(3);
                        sprintf(inbuf, "总共增加了%d公斤",
                                weightmp);
                        temppress(inbuf);
                    } else {
                        temppress("回心转意罗.....");
                    }
                } else {
                    temppress("你钱没那麽多啦.......");
                }
                break;

            case '3':
                if (d.money < 80) {
                    temppress
                    ("传统减肥要80元喔....你不够钱啦...");
                } else {
#ifdef MAPLE
                    getdata(b_lines - 1, 1,
                            "需花费80元(3～5公斤)，你确定吗? [y/N]",
                            genbuf, 2, 1, 0);
#else
                    getdata(b_lines-1,1,"需花费80元(3～5公斤)，你确定吗? [y/N]",genbuf,2,DOECHO,NULL,true);
#endif    // END MAPLE
                    if (genbuf[0] == 'Y' || genbuf[0] == 'y') {
                        weightmp = 3 + rand() % 3;
                        d.weight -= weightmp;
                        if (d.weight < 0)
                            d.weight = 0;
                        d.money -= 100;
                        d.maxhp += rand() % 2;
                        d.hp -= rand() % 2 + 3;
                        show_special_pic(4);
                        sprintf(inbuf, "总共减少了%d公斤",
                                weightmp);
                        temppress(inbuf);
                    } else {
                        temppress("回心转意罗.....");
                    }
                }
                break;
            case '4':
#ifdef MAPLE
                getdata(b_lines - 1, 1,
                        "减一公斤要30元，你要减多少公斤呢? [请填数字]:",
                        genbuf, 4, 1, 0);
#else
                getdata(b_lines-1,1,"减一公斤要30元，你要减多少公斤呢? [请填数字]:",genbuf,4,DOECHO,NULL,true);
#endif    // END MAPLE
                weightmp = atoi(genbuf);
                if (weightmp <= 0) {
                    temppress("输入有误..放弃罗...");
                } else if (d.weight <= weightmp) {
                    temppress("你没那麽重喔.....");
                } else if (d.money > (weightmp * 30)) {
                    sprintf(inbuf,
                            "减少%d公斤，总共需花费了%d元，确定吗? [y/N]",
                            weightmp, weightmp * 30);
#ifdef MAPLE
                    getdata(b_lines - 1, 1, inbuf, genbuf, 2, 1, 0);
#else
                    getdata(b_lines-1,1,inbuf,genbuf,2,DOECHO,NULL,true);
#endif    // END MAPLE
                    if (genbuf[0] == 'Y' || genbuf[0] == 'y') {
                        d.money -= weightmp * 30;
                        d.weight -= weightmp;
                        d.maxhp -= (rand() % 2 + 2);
                        count_tired(5, 8, "N", 100, 1);
                        d.hp -= (rand() % 2 + 3);
                        d.sick += rand() % 10 + 5;
                        show_special_pic(4);
                        sprintf(inbuf, "总共减少了%d公斤",
                                weightmp);
                        temppress(inbuf);
                    } else {
                        temppress("回心转意罗.....");
                    }
                } else {
                    temppress("你钱没那麽多啦.......");
                }
                break;
        }
    }
    return 0;
}

/*参见*/

int
pip_go_palace()
{
    pip_go_palace_screen(royallist);
    return 0;
}

int pip_go_palace_screen(const struct royalset *p)
{
    int n;
    int a;
    int b;
    int choice;
    int pipkey;
    int change;
    char buf[256];
    char inbuf1[20];
    char inbuf2[20];
    char *needmode[3] = { "      ", "礼仪表现＞", "谈吐技巧＞" };
    int save[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    d.nodone = 0;
    do {
        clear();
        show_palace_pic(0);
        move(13, 4);
        sprintf(buf,
                "\033[1;31m┌——————┤\033[37;41m 来到总司令部了  请选择你欲拜访的对象 \033[0;1;31m├——————┐\033[0m");
        prints(buf);
        move(14, 4);
        sprintf(buf,
                "\033[1;31m│                                                                  │\033[0m");
        prints(buf);

        for (n = 0; n < 5; n++) {
            a = 2 * n + 1;
            b = 2 * n + 2;
            move(15 + n, 4);
            sprintf(inbuf1, "%-10s%3d", needmode[p[a].needmode],
                    p[a].needvalue);
            if (n == 4) {
                sprintf(inbuf2, "%-10s",
                        needmode[p[b].needmode]);
            } else {
                sprintf(inbuf2, "%-10s%3d",
                        needmode[p[b].needmode],
                        p[b].needvalue);
            }
            if ((d.seeroyalJ == 1 && n == 4) || (n != 4))
                sprintf(buf,
                        "\033[1;31m│ \033[36m(\033[37m%s\033[36m) \033[33m%-10s  \033[37m%-14s     \033[36m(\033[37m%s\033[36m) \033[33m%-10s  \033[37m%-14s\033[31m│\033[0m",
                        p[a].num, p[a].name, inbuf1, p[b].num,
                        p[b].name, inbuf2);
            else
                sprintf(buf,
                        "\033[1;31m│ \033[36m(\033[37m%s\033[36m) \033[33m%-10s  \033[37m%-14s                                   \033[31m│\033[0m",
                        p[a].num, p[a].name, inbuf1);
            prints(buf);
        }
        move(20, 4);
        sprintf(buf,
                "\033[1;31m│                                                                  │\033[0m");
        prints(buf);
        move(21, 4);
        sprintf(buf,
                "\033[1;31m└—————————————————————————————————┘\033[0m");
        prints(buf);

        if (d.death == 1 || d.death == 2 || d.death == 3)
            return 0;
        /*
         * 将各人务已经给与的数值叫回来
         */
        save[1] = d.royalA; /*from守卫 */
        save[2] = d.royalB; /*from近卫 */
        save[3] = d.royalC; /*from将军 */
        save[4] = d.royalD; /*from大臣 */
        save[5] = d.royalE; /*from祭司 */
        save[6] = d.royalF; /*from宠妃 */
        save[7] = d.royalG; /*from王妃 */
        save[8] = d.royalH; /*from国王 */
        save[9] = d.royalI; /*from小丑 */
        save[10] = d.royalJ; /*from王子 */

        move(b_lines - 1, 0);
        clrtoeol();
        move(b_lines - 1, 0);
        prints("\033[1;33m [生命力] %d/%d  [疲劳度] %d \033[0m", d.hp,
               d.maxhp, d.tired);

        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        prints
        ("\033[1;37;46m  参见选单  \033[44m [字母]选择欲拜访的人物  [Q]离开星空总司令部：                    \033[0m");
        pipkey = igetkey();
        choice = pipkey - 64;
        if (choice < 1 || choice > 10)
            choice = pipkey - 96;

        if ((choice >= 1 && choice <= 10 && d.seeroyalJ == 1)
                || (choice >= 1 && choice <= 9 && d.seeroyalJ == 0)) {
            d.social += rand() % 3 + 3;
            d.hp -= rand() % 5 + 6;
            d.tired += rand() % 5 + 8;
            if (d.tired >= 100) {
                d.death = 1;
                pipdie("\033[1;31m累死了...\033[m  ", 1);
            }
            if (d.hp < 0) {
                d.death = 1;
                pipdie("\033[1;31m饿死了...\033[m  ", 1);
            }
            if (d.death == 1) {
                sprintf(buf, "掰掰了...真是悲情..");
            } else {
                if ((p[choice].needmode == 0) ||
                        (p[choice].needmode == 1
                         && d.manners >= p[choice].needvalue)
                        || (p[choice].needmode == 2
                            && d.speech >= p[choice].needvalue)) {
                    if (choice >= 1 && choice <= 9
                            && save[choice] >=
                            p[choice].maxtoman) {
                        if (rand() % 2 > 0)
                            sprintf(buf,
                                    "能和这麽伟大的你讲话真是荣幸ㄚ...");
                        else
                            sprintf(buf,
                                    "很高兴你来拜访我，但我不能给你什麽了..");
                    } else {
                        change = 0;
                        if (choice >= 1 && choice <= 8) {
                            switch (choice) {
                                case 1:
                                    change =
                                        d.
                                        character /
                                        5;
                                    break;
                                case 2:
                                    change =
                                        d.
                                        character /
                                        8;
                                    break;
                                case 3:
                                    change =
                                        d.charm / 5;
                                    break;
                                case 4:
                                    change =
                                        d.wisdom /
                                        10;
                                    break;
                                case 5:
                                    change =
                                        d.belief /
                                        10;
                                    break;
                                case 6:
                                    change =
                                        d.speech /
                                        10;
                                    break;
                                case 7:
                                    change =
                                        d.social /
                                        10;
                                    break;
                                case 8:
                                    change =
                                        d.hexp / 10;
                                    break;
                            }
                            /*
                             * 如果大於每次的增加最大量
                             */
                            if (change >
                                    p[choice].addtoman)
                                change =
                                    p[choice].
                                    addtoman;
                            /*
                             * 如果加上原先的之后大於所能给的所有值时
                             */
                            if ((change +
                                    save[choice]) >=
                                    p[choice].maxtoman)
                                change =
                                    p[choice].
                                    maxtoman -
                                    save
                                    [choice];
                            save[choice] += change;
                            d.toman += change;
                        } else if (choice == 9) {
                            save[9] = 0;
                            d.social -=
                                13 + rand() % 4;
                            d.affect +=
                                13 + rand() % 4;
                        } else if (choice == 10
                                   && d.seeroyalJ ==
                                   1) {
                            save[10] +=
                                15 + rand() % 4;
                            d.seeroyalJ = 0;
                        }
                        if (rand() % 2 > 0)
                            sprintf(buf, "%s",
                                    p[choice].
                                    words1);
                        else
                            sprintf(buf, "%s",
                                    p[choice].
                                    words2);
                    }
                } else {
                    if (rand() % 2 > 0)
                        sprintf(buf,
                                "我不和你这样的鸡谈话....");
                    else
                        sprintf(buf,
                                "你这只没教养的鸡，再去学学礼仪吧....");

                }
            }
            temppress(buf);
        }
        d.royalA = save[1];
        d.royalB = save[2];
        d.royalC = save[3];
        d.royalD = save[4];
        d.royalE = save[5];
        d.royalF = save[6];
        d.royalG = save[7];
        d.royalH = save[8];
        d.royalI = save[9];
        d.royalJ = save[10];
    } while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));

    temppress("离开星空总司令部.....");
    return 0;
}

int
pip_query()
{    /*拜访小鸡 */

#ifdef MAPLE
    userec muser;
#endif    // END MAPLE
    int id;
    char genbuf[STRLEN];
    struct userec *lookupuser;

#ifndef MAPLE
    char *msg_uid = MSG_UID;
    char *err_uid = ERR_UID;
#endif    // END MAPLE

    stand_title("拜访同伴");
    usercomplete(msg_uid, genbuf);
    if (genbuf[0]) {
        move(2, 0);
        if ((id = getuser(genbuf, &lookupuser))!=0) {
            pip_read(genbuf);
            temppress("观摩一下别人的小鸡...:p");
        } else {
            outs(err_uid);
            clrtoeol();
        }
    }
    return 0;
}

int
pip_read(genbuf)
char *genbuf;
{
    FILE *fs;
    struct chicken d1;
    char buf[200];
    const static char yo[12][5] = { "诞生", "婴儿", "幼儿", "儿童", "少年", "青年",
                                    "成年", "壮年", "更年", "老年", "古稀", "神仙"
                                  };
    int pc1, age1, age = 0, hp1=1;
#ifdef MAPLE
    sprintf(buf, "home/%s/new_chicken", genbuf);
    currutmp->destuid = genbuf;
#else
    sprintf(buf, "home/%c/%s/new_chicken", toupper(genbuf[0]), genbuf);
#endif    // END MAPLE

    if ((fs = fopen(buf, "r"))!=NULL) {
        fread(&d1, sizeof(d1), 1, fs);
        age = d1.bbtime / 1800;
        if (age == 0) /*诞生 */
            age1 = 0;
        else if (age == 1) /*婴儿 */
            age1 = 1;
        else if (age >= 2 && age <= 5) /*幼儿 */
            age1 = 2;
        else if (age >= 6 && age <= 12) /*儿童 */
            age1 = 3;
        else if (age >= 13 && age <= 15) /*少年 */
            age1 = 4;
        else if (age >= 16 && age <= 18) /*青年 */
            age1 = 5;
        else if (age >= 19 && age <= 35) /*成年 */
            age1 = 6;
        else if (age >= 36 && age <= 45) /*壮年 */
            age1 = 7;
        else if (age >= 45 && age <= 60) /*更年 */
            age1 = 8;
        else if (age >= 60 && age <= 70) /*老年 */
            age1 = 9;
        else if (age >= 70 && age <= 100) /*古稀 */
            age1 = 10;
        else if (age > 100) /*神仙 */
            age1 = 11;
        else
            age1=0;
        fclose(fs);
        move(1, 0);
        clrtobot();
#ifdef MAPLE
        prints("这是%s养的小鸡：\n", xuser.userid);
#else
        prints("这是%s养的小鸡：\n", genbuf);
#endif    // END MAPLE

        if (d1.death == 0) {
            prints
            ("\033[1;32mName：%-10s\033[m  生日：%02d年%02d月%2d日   年龄：%2d岁  状态：%s  钱钱：%d\n"
             "生命：%3d/%-3d  快乐：%-4d  满意：%-4d  气质：%-4d  智慧：%-4d  体重：%-4d\n"
             "大补丸：%-4d   食物：%-4d  零食：%-4d  疲劳：%-4d  脏脏：%-4d  病气：%-4d\n",
             d1.name, d1.year, d1.month, d1.day, age, yo[age1],
             d1.money, d1.hp, d1.maxhp, d1.happy, d1.satisfy,
             d1.character, d1.wisdom, d1.weight, d1.bighp,
             d1.food, d1.cookie, d1.tired, d1.shit, d1.sick);

            move(5, 0);
            switch (age1) {
                case 0:
                case 1:
                case 2:
                    if (d1.weight <= (60 + 10 * age - 30))
                        show_basic_pic(1);
                    else if (d1.weight > (60 + 10 * age - 30)
                             && d1.weight < (60 + 10 * age + 30))
                        show_basic_pic(2);
                    else if (d1.weight >= (60 + 10 * age + 30))
                        show_basic_pic(3);
                    break;
                case 3:
                case 4:
                    if (d1.weight <= (60 + 10 * age - 30))
                        show_basic_pic(4);
                    else if (d1.weight > (60 + 10 * age - 30)
                             && d1.weight < (60 + 10 * age + 30))
                        show_basic_pic(5);
                    else if (d1.weight >= (60 + 10 * age + 30))
                        show_basic_pic(6);
                    break;
                case 5:
                case 6:
                    if (d1.weight <= (60 + 10 * age - 30))
                        show_basic_pic(7);
                    else if (d1.weight > (60 + 10 * age - 30)
                             && d1.weight < (60 + 10 * age + 30))
                        show_basic_pic(8);
                    else if (d1.weight >= (60 + 10 * age + 30))
                        show_basic_pic(9);
                    break;
                case 7:
                case 8:
                    if (d1.weight <= (60 + 10 * age - 30))
                        show_basic_pic(10);
                    else if (d1.weight > (60 + 10 * age - 30)
                             && d1.weight < (60 + 10 * age + 30))
                        show_basic_pic(11);
                    else if (d1.weight >= (60 + 10 * age + 30))
                        show_basic_pic(12);
                    break;
                case 9:
                    show_basic_pic(13);
                    break;
                case 10:
                case 11:
                    show_basic_pic(13);
                    break;
            }
            move(18, 0);
            if (d1.shit == 0)
                prints("很干净..");
            if (d1.shit > 40 && d1.shit < 60)
                prints("臭臭的..");
            if (d1.shit >= 60 && d1.shit < 80)
                prints("好臭喔..");
            if (d1.shit >= 80 && d1.shit < 100)
                prints("\033[1;34m快臭死了..\033[m");
            if (d1.shit >= 100) {
                prints("\033[1;31m臭死了..\033[m");
                return -1;
            }

            pc1 = hp1 * 100 / d1.maxhp;
            if (pc1 == 0) {
                prints("饿死了..");
                return -1;
            }
            if (pc1 < 20)
                prints("\033[1;35m全身无力中.快饿死了.\033[m");
            if (pc1 < 40 && pc1 >= 20)
                prints("体力不太够..想吃点东西..");
            if (pc1 < 100 && pc1 >= 80)
                prints("嗯～肚子饱饱有体力..");
            if (pc1 >= 100)
                prints("\033[1;34m快撑死了..\033[m");

            pc1 = d1.tired;
            if (pc1 < 20)
                prints("精神抖抖中..");
            if (pc1 < 80 && pc1 >= 60)
                prints("\033[1;34m有点小累..\033[m");
            if (pc1 < 100 && pc1 >= 80) {
                prints("\033[1;31m好累喔，快不行了..\033[m");
            }
            if (pc1 >= 100) {
                prints("累死了...");
                return -1;
            }

            pc1 = 60 + 10 * age;
            if (d1.weight < (pc1 + 30) && d1.weight >= (pc1 + 10))
                prints("有点小胖..");
            if (d1.weight < (pc1 + 50) && d1.weight >= (pc1 + 30))
                prints("太胖了..");
            if (d1.weight > (pc1 + 50)) {
                prints("胖死了...");
                return -1;
            }

            if (d1.weight < (pc1 - 50)) {
                prints("瘦死了..");
                return -1;
            }
            if (d1.weight > (pc1 - 30) && d1.weight <= (pc1 - 10))
                prints("有点小瘦..");
            if (d1.weight > (pc1 - 50) && d1.weight <= (pc1 - 30))
                prints("太瘦了..");

            if (d1.sick < 75 && d1.sick >= 50)
                prints("\033[1;34m生病了..\033[m");
            if (d1.sick < 100 && d1.sick >= 75) {
                prints("\033[1;31m病重!!..\033[m");
            }
            if (d1.sick >= 100) {
                prints("病死了.!.");
                return -1;
            }

            pc1 = d1.happy;
            if (pc1 < 20)
                prints("\033[1;31m很不快乐..\033[m");
            if (pc1 < 40 && pc1 >= 20)
                prints("不快乐..");
            if (pc1 < 95 && pc1 >= 80)
                prints("快乐..");
            if (pc1 <= 100 && pc1 >= 95)
                prints("很快乐..");

            pc1 = d1.satisfy;
            if (pc1 < 40)
                prints("\033[31;1m不满足..\033[m");
            if (pc1 < 95 && pc1 >= 80)
                prints("满足..");
            if (pc1 <= 100 && pc1 >= 95)
                prints("很满足..");
        } else if (d1.death == 1) {
            show_die_pic(2);
            move(14, 20);
            prints("可怜的小鸡呜呼哀哉了");
        } else if (d1.death == 2) {
            show_die_pic(3);
        } else if (d1.death == 3) {
            move(5, 0);
            outs("游戏已经玩到结局罗....");
        } else {
            temppress("档案损毁了....");
        }
    } /* 有养小鸡 */
    else {
        move(1, 0);
        clrtobot();
        temppress("这一家的人没有养小鸡......");
    }
    return 0;
}
