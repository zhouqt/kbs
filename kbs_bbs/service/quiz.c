#define BBSMAIN
#include "bbs.h"
#ifdef BBS_SERVICE_QUIZ
#include <mysql.h>

MYSQL s;
MYSQL_RES *res;
MYSQL_ROW row;

int topn;
char topid[100][14];
int topscore[100][14];
int score;

void load_board()
{
    FILE* fp=fopen(".QUIZTOP", "r");
    int i;
    if(fp==NULL) {
        topn = 0;
    }
    else {
        fscanf(fp, "%d", &topn);
        for(i=0;i<topn;i++) {
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
    if(fp==NULL) {
        return;
    }
    else {
        fprintf(fp, "%d\n", topn);
        for(i=0;i<topn;i++) {
            fprintf(fp, "%s\n", topid[i]);
            fprintf(fp, "%d\n", topscore[i]);
        }
        fclose(fp);
    }
}

int check_top(int score)
{
    int i,j;
    for(i=0;i<topn;i++) {
        if(score>topscore[i]) {
            if(topn<100) topn++;
            for(j=topn;j>i;j--) {
                strcpy(topid[j], topid[j-1]);
                topscore[j] = topscore[j-1];
                strcpy(topid[i], currentuser->userid);
                topscore[i] = score;
                return 1;
            }
        }
    }
    if(topn<100) {
        strcpy(topid[topn], currentuser->userid);
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
    clear();
    load_board();

    while(1) {

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
        move(4,13);
        setfcolor(WHITE,1);
        prints("===排行榜(%d--%d)===", k*10+1, k*10+10);
        move(5,15);
        setfcolor(RED,1);
        prints("╭───────╮");
        for(i=0;i<10;i++){
            move(7+i, 15);
            setfcolor(RED,1);
            prints("│");
            if (i+k*10>=topn) {
                move(7+i, 20);
                setfcolor(GREEN,1);
                prints("--空--");
            }
            else {
                char p[100];
                move(7+i, 17);
                setfcolor(GREEN,1);
                sprintf(p, "%d %-12s %d", i+1+k*10, topid[i+k*10], topscore[i+k*10]);
                prints(users[i].userid);
            }
            move(4+i, 31);
            setfcolor(RED,1);
            prints("│");
        }
        getdata(19, 22, "开始游戏(Y-开始,N-退出)", ans, 3, 1, NULL, 1);
        if(toupper(ans[0])=='Y')
            return 1;
        if(toupper(ans[0])=='N')
            return 0;
        k++;
        if(k>=10) k=0;
    }    
}

int quiz_test()
{
    int i=0,j,k;
    char sql[100];
    score = 0;
    while(1) {
        int level, style, anscount, question[200], ans[6][200], answer[100], now[100], input[6];
        do{
            j=rand()%10000;
            sprintf(sql, "select * from quiz where id=%d", j);
            mysql_real_query(&s, sql, strlen(sql));
            res = mysql_store_result(&s);
            row = mysql_fetch_row(&res);
        }while(row==NULL);
        level = atoi(row[1]);
        style = atoi(row[2]);
        strcpy(question, row[3]);
        anscount = atoi(row[4]);
        for(j=0;j<anscount;j++) {
            strcpy(ans[j], row[5+j]);
        }
        strcpy(answer, row[11]);

        move(0,0);
        setfcolor(YELLOW, 1);
        setbcolor(BLUE);
        prints(" 开心词典测试(\x1b[31mHAPPYQUIZ\x1b[33m)        测试人:%s    目前得分:%d    题号:%d    分值:%d    类型:",
            currentuser->userid, score, i+1, level, (style==1)?"单选":"多选");
        clrtoeol();
        for(j=0;j<6;j++) now[j]='0';
        now[6]=0;
        while(1){
            resetcolor();
            move(2,0);
            prints(question);
            for(j=0;j<anscount;j++) {
                move(4+j,0);
                clrtoeol();
                prints("%c. %s  %s", 'A'+j, ans[j], (now[j]=='1'):"*":"");
            }
            getdata(anscount+6, 0, "请选择(多选题按回车结束):", input, 3, 1, NULL, 1);
            input[0] = toupper(input[0]);
            if(input[0]>='A'&&input[0]<anscount+'A') {
                if(now[input[0]-'A']=='1') now[input[0]-'A']='0';
                else now[input[0]-'A']='1';
                if(style==1) break;
            }
            if((input[0]=='\r'||input[0]=='\n')&&style==2) break;
        }
        if(strcmp(now, answer)) {
            move(anscount+8, 0);
            setfcolor(RED, 1);
            prints("你答错啦\n");
            setfcolor(BLUE, 1);
            prints("      ,\n");
            prints("     (╰╮╮\n");
            prints("∞╭⌒ヽ  ..\n");
            prints("╰(       (_Q\n");
            prints("   ∨～∨∨\n");
            break;
        }
        else {
            move(anscount+8, 0);
            setfcolor(RED, 1);
            prints("正确!");
            refresh();
            sleep(1);
            i++;
            score+=level;
        }
    }
}

int quiz_again()
{
    int rank;
    char ans[4];
    rank = check_top(score);
    move(t_lines-3, 0);
    if(rank) {
        setfcolor(RED, 1);
        prints("恭喜你，你上排行榜啦!!!");
    }
    resetcolor();
    getdata(t_lines-2, 0, "重新玩吗?", ans, 3, 1, NULL, 1);
    return toupper(ans[0])=='Y';
}

int quiz_main()
{
    int i;

    mysql_init(&s);
    if (!mysql_real_connect(&s, 
                            sysconf_str("MYSQLHOST"),
                            sysconf_str("MYSQLUSER"),
			    sysconf_str("MYSQLPASSWORD"),
			    sysconf_str("MYSQLQUIZDATABASE"),
			    sysconf_eval("MYSQLPORT",1521), sysconf_str("MYSQLSOCKET"), 0)) {
        prints("%s\n", mysql_error(&s));
        pressanykey();
        return;
    }
    if(init_quiz()) {
        do{
            quiz_test();
        }while(quiz_again());
    }

    mysql_close(&s);
}
#endif
