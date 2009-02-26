#include <sys/times.h>
#include "service.h"
#include "bbs.h"

#define RECORDFILE "etc/tt.dat"
char *userid,*from;
char tt_id[20][20], tt_ip[20][30];
int tt_scr[20];

#ifndef CLK_TCK
#define CLK_TCK sysconf(_SC_CLK_TCK)
#endif

int tt_load_record()
{
    int n;
    FILE *fp=fopen(RECORDFILE, "r");
    if (fp==0) {
        fp=fopen(RECORDFILE, "w");
        for (n=0; n<20; n++)
            fprintf(fp, "%s %s %d\n", "none", "0.0.0.0", 0);
        fclose(fp);
        fp=fopen(RECORDFILE, "r");
    }
    for (n=0; n<20; n++)
        fscanf(fp, "%s %s %d", tt_id[n], tt_ip[n], &tt_scr[n]);
    fclose(fp);
    return 0;
}

int tt_save_record()
{
    int n, m1, m2;
    char id[20], ip[30];
    int scr;
    FILE *fp=fopen(RECORDFILE, "w");
    for (m1=0; m1<20; m1++)
        for (m2=m1+1; m2<20; m2++)
            if (tt_scr[m1]<tt_scr[m2]) {
                strcpy(id, tt_id[m1]);
                strcpy(ip, tt_ip[m1]);
                scr=tt_scr[m1];
                strcpy(tt_id[m1], tt_id[m2]);
                strcpy(tt_ip[m1], tt_ip[m2]);
                tt_scr[m1]=tt_scr[m2];
                strcpy(tt_id[m2], id);
                strcpy(tt_ip[m2], ip);
                tt_scr[m2]=scr;
            }
    for (n=0; n<20; n++)
        fprintf(fp, "%s %s %d\n", tt_id[n], tt_ip[n], tt_scr[n]);
    fclose(fp);
    return 0;
}

int tt_check_record(int score)
{
    int n;
    tt_load_record();
    for (n=0; n<20; n++)
        if (!strcasecmp(tt_id[n], userid)) {
            if (tt_scr[n]>score) return 0;
            tt_scr[n]=score;
            strncpy(tt_ip[n], from, 16);
            tt_ip[n][16]=0;
            tt_save_record();
            return 1;
        }
    if (tt_scr[19]<score) {
        tt_scr[19]=score;
        strcpy(tt_id[19], userid);
        strncpy(tt_ip[19], from, 16);
        tt_ip[19][16]=0;
        tt_save_record();
        return 1;
    }
    return 0;
}

int tt_game()
{
    char c[30], fbuf[10];
    char chars[]="ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ";
    int m, n, score, round;
    int t;
    int ch;
    int wrong=0;

    srand(getpid() + time(NULL));
    tt_load_record();
    clear();
    move(1, 0);
    prints("               本站打字高手排行榜\n%4s %-12s %-24s %5s(WPMs)\n", "名次", "帐号", "来源", "速度");
    for (n=0; n<20; n++) {
        snprintf(fbuf, 10, "%5.1f", tt_scr[n]/10.);
        prints("%4d %-12s %-24s %-5s\n", n+1, tt_id[n], "隐藏" /* tt_ip[n] */, fbuf);
    }

    round = 0;
start:
    round++;
    if (round % 3 == 1) {
        pressanykey();
        clear();
        move(2, 0);
        prints("\033[1;32mＢＢＳ\033[m打字练习程序. (大小写均可, 输入第一字符前的等待不计时. \033[1;32mESC\033[m or \033[1;32m^D\033[m 退出.)\n\n");
    }
    for (n=0; n<30; n++) {
        c[n] = rand() % 26;
        prints("%c%c", chars[c[n]*2], chars[c[n]*2+1]);
    }
    prints("\n");
    m = 0;
    t = times(NULL);
    while (m<30) {
        ch = igetkey();
        if (ch == KEY_ESC || ch == Ctrl('D') || ch == Ctrl('C')) return 0;

        if (m==0 && abs(times(NULL)-t)>300) {
            prints("\033\n超时! 你必须在\033[1;32m3\033[m秒钟以内开始!\n");
            goto start;
        }

        if (ch == c[m]+65 || ch == c[m]+97) {
            prints("%c%c", chars[c[m]*2], chars[c[m]*2+1]);
            if (m == 0) {
                t = times(NULL);
            }
            m++;
            usleep(60000);
        } else
            wrong++;
    }
    score = 18000 * CLK_TCK / (int)(times(NULL) - t);
    snprintf(fbuf, 10, "%5.2f", score/10.0);
    prints("\n\nSpeed=%-5s WPMs\n\n", fbuf);
    if (wrong > 30) prints("\033[1;32m你错的太多了！成绩无效\n\n");
    else if (tt_check_record(score)) prints("\033[1;33m祝贺！您刷新了自己的纪录！\033[m\n\n");
    goto start;
}

int tt_main()
{
    chdir(BBSHOME);
    userid = getCurrentUser()->userid;
    from = getSession()->fromhost;
    modify_user_mode(TETRIS);

    tt_game();
    return 0;
}

