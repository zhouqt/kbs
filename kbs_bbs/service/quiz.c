#include "service.h"
#include "bbs.h"
#ifdef BBS_SERVICE_QUIZ
#if HAVE_MYSQL_SMTH == 1
#include <mysql.h>

SMTH_API int t_lines;
MYSQL s;
MYSQL_RES *res;
MYSQL_ROW row;

int topn;
char topid[100][14];
int topscore[100];
int score;

void load_board()
{
    FILE* fp=fopen(".QUIZTOP", "r");
    int i;
    if (fp==NULL) {
        topn = 0;
    } else {
        fscanf(fp, "%d", &topn);
        for (i=0;i<topn;i++) {
            fscanf(fp, "%s", topid[i]);
            fscanf(fp, "%d", &topscore[i]);
        }
        fclose(fp);
    }
}

void save_board()
{
    FILE* fp=fopen(".QUIZTOP", "w");
    int i;
    if (fp==NULL) {
        return;
    } else {
        fprintf(fp, "%d\n", topn);
        for (i=0;i<topn;i++) {
            fprintf(fp, "%s\n", topid[i]);
            fprintf(fp, "%d\n", topscore[i]);
        }
        fclose(fp);
    }
}

int check_top(int score)
{
    int i,j,lasttopn;
    lasttopn = topn;
    load_board();
    if (topn<lasttopn) return -1;
    for (i=0;i<topn;i++)
        if (!strcmp(topid[i],getCurrentUser()->userid)) {
            if (score<=topscore[i]) return 0;
            topn--;
            for (j=i;j<topn;j++) {
                strcpy(topid[j],topid[j+1]);
                topscore[j] = topscore[j+1];
            }
            i--;
        }
    for (i=0;i<topn;i++) {
        if (score>topscore[i]) {
            if (topn<100) topn++;
            for (j=topn-1;j>i;j--) {
                strcpy(topid[j], topid[j-1]);
                topscore[j] = topscore[j-1];
            }
            strcpy(topid[i], getCurrentUser()->userid);
            topscore[i] = score;
            return 1;
        }
    }
    if (topn<100) {
        strcpy(topid[topn], getCurrentUser()->userid);
        topscore[topn] = score;
        topn++;
        return 1;
    }
    return 0;
}

int init_quiz()
{
    int i, k=0;
    char ans[4];
    load_board();

    while (1) {
        clear();
        move(2,20);
        setfcolor(WHITE,0);
        prints("开心词典测试中心(");
        setfcolor(RED,1);
        prints("HAPPYQUIZ");
        setfcolor(WHITE,0);
        prints(")测试版0.1");
        move(3,40);
        prints("作者: ");
        setfcolor(RED,0);
        prints("bad@smth.org");
        move(5,28);
        setfcolor(WHITE,1);
        prints("===排行榜(%d--%d)===", k*10+1, k*10+10);
        move(6,22);
        setfcolor(RED,1);
        prints("╭──────────────╮");
        for (i=0;i<10;i++) {
            move(7+i, 22);
            setfcolor(RED,1);
            prints("│");
            if (i+k*10>=topn) {
                move(7+i, 34);
                setfcolor(GREEN,1);
                prints("--空--");
            } else {
                char p[100];
                move(7+i, 24);
                setfcolor(GREEN,1);
                sprintf(p, "%3d   %-12s %4d", i+1+k*10, topid[i+k*10], topscore[i+k*10]);
                prints("%s", p);
            }
            move(7+i, 52);
            setfcolor(RED,1);
            prints("│");
        }
        move(17,22);
        setfcolor(RED,1);
        prints("╰──────────────╯");

        resetcolor();
        getdata(19, 22, "开始游戏(Y-开始,N-退出)", ans, 3, 1, NULL, 1);
        if (toupper(ans[0])=='Y')
            return 1;
        if (toupper(ans[0])=='N')
            return 0;
        k++;
        if (k>=10) k=0;
    }
}

SMTH_API int kicked;
extern void ktimeout(void * data);

int quiz_test()
{
    int i=0,j,k,stat_fast=0;
    time_t t1,t2,t3;
    char sql[100];
    score = 0;
    t1 = time(0);
    while (1) {
        int level, style, anscount, order[100], order2[100], tout=0;
        char question[200], ans[6][200], answer[100], now[100], input[6];
        t2 = time(0);
        set_alarm(30, 0, ktimeout, NULL);
        do {
            j=rand()%10000+1;
            sprintf(sql, "SELECT * FROM quiz WHERE id=%d", j);
            mysql_real_query(&s, sql, strlen(sql));
            res = mysql_store_result(&s);
            row = mysql_fetch_row(res);
        } while (row==NULL);
        level = atoi(row[1]);
        style = atoi(row[2]);
        strcpy(question, row[3]);
        anscount = atoi(row[4]);
        for (j=0;j<anscount;j++) order2[j]=0;
        for (j=0;j<anscount;j++) {
            do {
                k=rand()%anscount;
            } while (order2[k]==1);
            order2[k]=1;
            order[j]=k;
        }
        for (j=0;j<anscount;j++) {
            strcpy(ans[j], row[5+order[j]]);
        }
        answer[6]=0;
        for (j=0;j<6;j++) answer[j]='0';
        for (j=0;j<anscount;j++)
            answer[j]=row[11][order[j]];
        mysql_free_result(res);
        clear();
        move(0,0);
        setfcolor(YELLOW, 1);
        setbcolor(BLUE);
        prints(" 开心词典测试(\x1b[31mHAPPYQUIZ\x1b[33m)    测试人:%s  目前得分:\x1b[31m%d\x1b[33m  题号:%d  分值:%d  类型:%s",
               getCurrentUser()->userid, score, i+1, level, (style==1)?"单选":"多选");
        clrtoeol();
        for (j=0;j<6;j++) now[j]='0';
        now[6]=0;
        while (1) {
            char bb[100];
            resetcolor();
            move(2,0);
            if (strlen(question)<70)
                prints("%s", question);
            else {
                char q2[100];
                strcpy(q2,question+70);
                question[70]=0;
                prints("%s\n%s", question, q2);
            }
            for (j=0;j<anscount;j++) {
                move(4+j,0);
                clrtoeol();
                prints("%c. %s  %s", 'A'+j, ans[j], (now[j]=='1')?"*":"");
            }
            sprintf(bb, "请选择:(A-%c,回车结束)", anscount+'A'-1);
            getdata(anscount+6, 0, bb, input, 3, 1, NULL, 1);
            if (kicked) break;
            input[0] = toupper(input[0]);
            if (input[0]>='A'&&input[0]<anscount+'A') {
                if (now[input[0]-'A']=='1') now[input[0]-'A']='0';
                else now[input[0]-'A']='1';
                if (style==1) break;
            } else
                if (input[0]==0&&style==2) break;
                else {
                    getdata(anscount+7, 0, "退出测试:(Y/N)", input, 3, 1, NULL, 1);
                    if (toupper(input[0])=='Y') {
                        set_alarm(0, 0, NULL, NULL);
                        kicked = 0;
                        return 0;
                    }
                }
        }
        if (kicked) tout=1;
        set_alarm(0, 0, NULL, NULL);
        kicked = 0;
        t3 = time(0);
        if (t3-t2<=10) stat_fast++;
        else stat_fast=0;
        if (strcmp(now, answer)||stat_fast>=10||t3-t1>=7200) {
            move(anscount+8, 0);
            setfcolor(RED, 1);
            if (tout)
                prints("你超时了\n");
            else
                prints("你答错啦\n");
            setfcolor(BLUE, 1);
            prints("      ,\n");
            prints("     (╰╮╮\n");
            prints("∞╭⌒ヽ  ..\n");
            prints("╰(       (_Q\n");
            prints("   ∨～∨∨\n");
            break;
        } else {
            move(anscount+8, 0);
            setfcolor(RED, 1);
            prints("正确!");
            refresh();
            sleep(1);
            i++;
            score+=level;
        }
    }
    set_alarm(0, 0, NULL, NULL);
    return 0;
}

int quiz_again()
{
    int rank;
    char ans[4];
    rank = check_top(score);
    move(t_lines-3, 0);
    if (rank==1) {
        setfcolor(RED, 1);
        prints("恭喜你，你上排行榜啦!!!");
        save_board();
    }
    resetcolor();
    getdata(t_lines-2, 0, "重新玩吗?", ans, 3, 1, NULL, 1);
    return toupper(ans[0])=='Y';
}

int quiz_main()
{
    mysql_init(&s);
    if (!mysql_real_connect(&s,
                            sysconf_str("MYSQLHOST"),
                            sysconf_str("MYSQLUSER"),
                            sysconf_str("MYSQLPASSWORD"),
                            sysconf_str("MYSQLQUIZDATABASE"),
                            sysconf_eval("MYSQLPORT",1521), sysconf_str("MYSQLSOCKET"), 0)) {
        prints("%s\n", mysql_error(&s));
        pressanykey();
        return 0;
    }
    if (init_quiz()) {
        do {
            quiz_test();
        } while (quiz_again());
    }

    mysql_close(&s);
    return 0;
}
#endif /* HAVE_MYSQL_SMTH == 1*/
#endif
