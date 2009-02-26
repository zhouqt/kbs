
/*---------------------------------------------------------------------------*/
/*小鸡档案的读写函式            */
/*---------------------------------------------------------------------------*/
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;

/*游戏写资料入档案*/
void pip_write_file(void)
{
    FILE *ff;
    char buf[200];

#ifdef MAPLE
    sprintf(buf, "home/%s/new_chicken", getCurrentUser()->userid);
#else
    sprintf(buf, "home/%c/%s/new_chicken", toupper(getCurrentUser()->userid[0]),
            getCurrentUser()->userid);
#endif    // END MAPLE

    if ((ff = fopen(buf, "w"))!=NULL) {
        fwrite(&d, sizeof(d), 1, ff);
        fclose(ff);
    }
}

/*游戏读资料出档案*/
void pip_read_file(void)
{
    FILE *fs;
    char buf[200];

#ifdef MAPLE
    sprintf(buf, "home/%s/new_chicken", getCurrentUser()->userid);
#else
    sprintf(buf, "home/%c/%s/new_chicken", toupper(getCurrentUser()->userid[0]),
            getCurrentUser()->userid);
#endif    // END MAPLE
    if ((fs = fopen(buf, "r"))!=NULL) {
        fread(&d, sizeof(d), 1, fs);
        fclose(fs);
    }
}

/*记录到pip.log档*/
void pip_log_record(char *msg)
{
    FILE *fs;
    fs = fopen("game/pipgame/pip.log", "a+");
    if (fs == NULL) return;
    fprintf(fs, "%s", msg);
    fclose(fs);
}

/*小鸡进度储存*/
int
pip_write_backup()
{
    char *files[4] = { "没有", "进度一", "进度二", "进度叁" };
    char buf[200], buf1[200];
    char ans[3];
    int num = 0;
    int pipkey;

    show_system_pic(21);
    pip_write_file();
    do {
        move(b_lines - 2, 0);
        clrtoeol();
        move(b_lines - 1, 0);
        clrtoeol();
        move(b_lines - 1, 1);
        prints
        ("储存 [1]进度一 [2]进度二 [3]进度叁 [Q]放弃 [1/2/3/Q]：");
        pipkey = igetkey();

        if (pipkey == '1')
            num = 1;
        else if (pipkey == '2')
            num = 2;
        else if (pipkey == '3')
            num = 3;
        else
            num = 0;

    } while (pipkey != 'Q' && pipkey != 'q' && num != 1 && num != 2
             && num != 3);
    if (pipkey == 'q' || pipkey == 'Q') {
        temppress("放弃储存游戏进度");
        return 0;
    }
    move(b_lines - 2, 1);
    prints("储存档案会覆盖存储存於 [%s] 的小鸡的档案喔！请考虑清楚...",
           files[num]);
    sprintf(buf1, "确定要储存於 [%s] 档案吗？ [y/N]:", files[num]);
#ifdef MAPLE
    getdata(b_lines - 1, 1, buf1, ans, 2, 1, 0);
#else
    getdata(b_lines-1,1,buf1,ans,2,DOECHO,NULL,true);
#endif    // END MAPLE
    if (ans[0] != 'y' && ans[0] != 'Y') {
        temppress("放弃储存档案");
        return 0;
    }

    move(b_lines - 1, 0);
    clrtobot();
    sprintf(buf1, "储存 [%s] 档案完成了", files[num]);
    temppress(buf1);
#ifdef MAPLE
    sprintf(buf, "/bin/cp home/%s/new_chicken home/%s/new_chicken.bak%d",
            getCurrentUser()->userid, getCurrentUser()->userid, num);
#else
    sprintf(buf,
            "/bin/cp home/%c/%s/new_chicken home/%c/%s/new_chicken.bak%d",
            toupper(getCurrentUser()->userid[0]), getCurrentUser()->userid,
            toupper(getCurrentUser()->userid[0]), getCurrentUser()->userid, num);
#endif    // END MAPLE
    system(buf);
    return 0;
}

int
pip_read_backup()
{
    char buf[200], buf1[200], buf2[200];
    char *files[4] = { "没有", "进度一", "进度二", "进度叁" };
    char ans[3];
    int pipkey;
    int num = 0;
    int ok = 0;
    FILE *fs;

    show_system_pic(22);
    do {
        move(b_lines - 2, 0);
        clrtoeol();
        move(b_lines - 1, 0);
        clrtoeol();
        move(b_lines - 1, 1);
        prints
        ("读取 [1]进度一 [2]进度二 [3]进度叁 [Q]放弃 [1/2/3/Q]：");
        pipkey = igetkey();

        if (pipkey == '1')
            num = 1;
        else if (pipkey == '2')
            num = 2;
        else if (pipkey == '3')
            num = 3;
        else
            num = 0;

        if (num > 0) {
#ifdef MAPLE
            sprintf(buf, "home/%s/new_chicken.bak%d", getCurrentUser()->userid,
                    num);
#else
            sprintf(buf, "home/%c/%s/new_chicken.bak%d",
                    toupper(getCurrentUser()->userid[0]), getCurrentUser()->userid, num);
#endif    // END MAPLE
            if ((fs = fopen(buf, "r")) == NULL) {
                sprintf(buf, "档案 [%s] 不存在", files[num]);
                temppress(buf);
                ok = 0;
            } else {

                move(b_lines - 2, 1);
                prints
                ("读取出档案会覆盖现在正在玩的小鸡的档案喔！请考虑清楚...");
                sprintf(buf,
                        "确定要读取出 [%s] 档案吗？ [y/N]:",
                        files[num]);
#ifdef MAPLE
                getdata(b_lines - 1, 1, buf, ans, 2, 1, 0);
#else
                getdata(b_lines-1,1,buf,ans,2,DOECHO,NULL,true);
#endif    // END MAPLE
                if (ans[0] != 'y' && ans[0] != 'Y') {
                    temppress("让我再决定一下...");
                } else
                    ok = 1;
            }
        }
    } while (pipkey != 'Q' && pipkey != 'q' && ok != 1);
    if (pipkey == 'q' || pipkey == 'Q') {
        temppress("还是玩原本的游戏");
        return 0;
    }

    move(b_lines - 1, 0);
    clrtobot();
    sprintf(buf, "读取 [%s] 档案完成了", files[num]);
    temppress(buf);

#ifdef MAPLE
    sprintf(buf1, "/bin/touch home/%s/new_chicken.bak%d", getCurrentUser()->userid,
            num);
    sprintf(buf2, "/bin/cp home/%s/new_chicken.bak%d home/%s/new_chicken",
            getCurrentUser()->userid, num, getCurrentUser()->userid);
#else
    sprintf(buf1, "/bin/touch home/%c/%s/new_chicken.bak%d",
            toupper(getCurrentUser()->userid[0]), getCurrentUser()->userid, num);
    sprintf(buf2,
            "/bin/cp home/%c/%s/new_chicken.bak%d home/%c/%s/new_chicken",
            toupper(getCurrentUser()->userid[0]), getCurrentUser()->userid, num,
            toupper(getCurrentUser()->userid[0]), getCurrentUser()->userid);
#endif    // END MAPLE
    system(buf1);
    system(buf2);
    pip_read_file();
    return 0;
}

int
pip_live_again()
{
    char genbuf[80];
    time_t now;
    int tm;

    tm = (d.bbtime) / 60 / 30;

    clear();
    showtitle("小鸡复活手术中", BBS_FULL_NAME);

    now = time(0);
    sprintf(genbuf, "\033[1;33m%s %-11s的小鸡 [%s二代] 复活了！\033[m\n",
            Cdate(now), getCurrentUser()->userid, d.name);
    pip_log_record(genbuf);

    /*
     * 身体上的设定
     */
    d.death = 0;
    d.maxhp = d.maxhp * 3 / 4 + 1;
    d.hp = d.maxhp / 2 + 1;
    d.tired = 20;
    d.shit = 20;
    d.sick = 20;
    d.wrist = d.wrist * 3 / 4;
    d.weight = 45 + 10 * tm;

    /*
     * 钱减到五分之一
     */
    d.money = d.money / 5;

    /*
     * 战斗能力降一半
     */
    d.attack = d.attack * 3 / 4;
    d.resist = d.resist * 3 / 4;
    d.maxmp = d.maxmp * 3 / 4;
    d.mp = d.maxmp / 2;

    /*
     * 变的不快乐
     */
    d.happy = 0;
    d.satisfy = 0;

    /*
     * 评价减半
     */
    d.social = d.social * 3 / 4;
    d.family = d.family * 3 / 4;
    d.hexp = d.hexp * 3 / 4;
    d.mexp = d.mexp * 3 / 4;

    /*
     * 武器掉光光
     */
    d.weaponhead = 0;
    d.weaponrhand = 0;
    d.weaponlhand = 0;
    d.weaponbody = 0;
    d.weaponfoot = 0;

    /*
     * 食物剩一半
     */
    d.food = d.food / 2;
    d.medicine = d.medicine / 2;
    d.bighp = d.bighp / 2;
    d.cookie = d.cookie / 2;

    d.liveagain += 1;

    temppress("小鸡器官重建中！");
    temppress("小鸡体质恢复中！");
    temppress("小鸡能力调整中！");
    temppress("恭禧您，你的小鸡又复活罗！");
    pip_write_file();
    return 0;
}
