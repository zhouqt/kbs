#include "service.h"
#include "bbs.h"

int a[32][18];  //À×
int m[32][18];  //marked
int o[32][18];  //opened
int f[32][18];  //final add by stiger
int opened;//add by stiger
int totallei;//add by stiger
double stiger;//add by stiger
int steps;//add by stiger
char topID[20][20],topFROM[20][32];
char userid[20]="unknown.", fromhost[20]="unknown.";
int topT[20], gameover=0;
double topS[20];//add by stiger
int marknum=0;
int ganyingshi=0;
char recfile[20];

int winmine(void);
int win_showrec(void);
int winrefresh(void);
int winloop(void);
int windig(int,int);
int winsh(int,int);
int winhelp(void);
int dig(int,int);
int checkwin(void);
int win_checkrec(int);
int win_saverec(void);
int win_sort(void);

int winmine_main()
{
    strcpy(recfile,"mine.rec");
    strcpy(userid,getCurrentUser()->userid);
    modify_user_mode(WINMINE);
    winmine();
    return 0;
}

int winmineg_main()
{
    ganyingshi=1;
    strcpy(recfile,"mine2.rec");
    strcpy(userid,getCurrentUser()->userid);
    modify_user_mode(WINMINE);
    winmine();
    return 0;
}

int show_mines()
{
    int x, y;
    for (x=1; x<=30; x++)
        for (y=1; y<17; y++) {
            if (a[x][y]) {
                move(y-1,x*2-2);
                prints("[1;31mÀ×[m");
            }
        }
    return 0;
}

int winmine()
{
    int x,y;
    win_showrec();
    clear();
    refresh();
    while (1) {
        totallei=0;//add by stiger
        stiger=0;//add by stiger
        steps=0;//add by stiger
        opened=0;
        clear();
        for (x=0;x<=31;x++)
            for (y=0;y<=17;y++) {
                a[x][y]= 0;
                m[x][y]= 0;
                f[x][y]= 0;//add by stiger
//             o[x][y]= 0;
//
                if (x==0||x==31||y==0||y==17) o[x][y]= 1;
                else o[x][y]=0;
            }
        winrefresh();
        if (winloop()==0) break;
        pressanykey();
    }
    return 0;
}

int num_mine_beside(int x1, int y1)
{
    int dx, dy, s;
    s= 0;
    for (dx= x1-1; dx<=x1+1; dx++)
        for (dy= y1-1; dy<=y1+1; dy++)
            if (!(dx==x1&&dy==y1)&&a[dx][dy]) s++;
    return s;
}

int num_mark_beside(int x1, int y1)
{
    int dx, dy, s;
    s= 0;
    for (dx= x1-1; dx<=x1+1; dx++)
        for (dy= y1-1; dy<=y1+1; dy++)
            if (!(dx==x1&&dy==y1)&&m[dx][dy]) s++;
    return s;
}

int wininit(int x1, int y1)
{
    int n, x, y;
    int i,j;//add by stiger
    srandom(time(0));
    for (n=1; n<=99; n++) {
        do {
            x= random()%30 +1;
            y= random()%16 +1;
        } while (a[x][y]!=0||(abs(x-x1)<2&&abs(y-y1)<2));
        a[x][y]=1;
    }
    totallei=0;
    for (i=1;i<31;i++) {
        for (j=1;j<17;j++) {
            if (a[i][j]) totallei++;
        }
    }
    move(20,45);
    prints("¹²ÓÐÀ×:%d",totallei);
    refresh();
    return 0;
}

/* Ë«¼ü */
int dblclick(int x, int y)
{
    int dx, dy;
    if (x<1|| x>30|| y<1|| y>16) return 0;
    if (!o[x][y]) return 0;
    if (num_mine_beside(x, y)!=num_mark_beside(x, y)) return 0;
    for (dx=x-1;dx<=x+1;dx++)
        for (dy=y-1;dy<=y+1;dy++)
            windig(dx, dy);
    return 0;
}

/* ×ó¼ü */
int windig(int x, int y)
{
    int dx, dy;
    if (x< 1|| x> 30|| y< 1|| y> 16) return 0;
    if (o[x][y]||m[x][y]) return 0;
    o[x][y]=1;
    winsh(x, y);
    if (a[x][y]) {
        show_mines();
        gameover=1;
        return 0;
    }
    if (num_mine_beside(x, y)==0) {
        for (dx=x-1;dx<=x+1;dx++)
            for (dy=y-1;dy<=y+1;dy++)
                windig(dx, dy);
    }
    return 0;
}

/* Í¬ÉÏ, ¼Ó¿ìËÙ¶È */
int winsh0(int x, int y)
{
    int c, d;
    static char word[9][10]= {
        "¡¤", "£±", "£²", "£³", "£´", "£µ", "£¶", "£·", "£¸"
    };
    static int cc[9]= {38, 37, 32, 31, 33, 35, 36, 40, 39};
    char buf[100];
    if (!o[x][y]&&!m[x][y]) {
        prints("¡ù");
        return 0;
    }
    if (m[x][y]) {
        prints("¡ñ");
        return 0;
    }
    if (a[x][y]) {
        prints("[1;31mÀ×[m");
        return 0;
    }
    c= num_mine_beside(x, y);
    d= 1;
    if (c==0) d=0;
    sprintf(buf, "[%d;%dm%s[m", d, cc[c], word[c]);
    prints(buf);
    return 0;
}

int winsh(int x,int y)
{

//add by stiger
    if (!f[x][y] && (o[x][y] || m[x][y])) {
        f[x][y]=1;
        opened++;
        move(18,20);
        prints("ÄãÒÑ¾­´ò¿ª[1;40;33m%2d%%[m   ",opened*100/480);
    }
    move(y-1,x*2-2);
    winsh0(x,y);
    return 0;
}
/*add end*/

int winloop()
{
    int x, y, c, marked, t0, inited;
    x= 10;
    y= 8;
    inited= 0;
    marked= 0;
    t0=time(0);
    steps=0;
    while (1) {
        c= igetkey();
        //system("echo start_winloop_igetkey >> /home/bbs/stiger.test");
        steps++;
        if (c=='q' || c==Ctrl('C')) return 0;
        if ((c==KEY_UP||c=='k')&&y>1) y--;
        if ((c==KEY_DOWN||c=='j')&&y<16) y++;
        if ((c==KEY_LEFT||c=='h')&&x>1) x--;
        if ((c==KEY_RIGHT||c=='l')&&x<30) x++;
        move(20,0);
        prints("Ê±¼ä: %d ", time(0)-t0);
        move(21,0);
        prints("×ø±ê: %3d, %3d", x, y);
        move(y-1,x*2-2);
//modified by stiger,2001,6,12,¸ÄÕýmarkedÍ³¼ÆµÄ´íÎó
        if (ganyingshi) {
            marknum=o[x-1][y-1]+o[x-1][y]+o[x-1][y+1]+o[x][y-1]+o[x][y+1]+o[x+1][y-1]+o[x+1][y]+o[x+1][y+1];
            if ((m[x][y]!=1)&&(num_mine_beside(x, y)==8-marknum)) {
                if (o[x-1][y-1]!=1) { if (!m[x-1][y-1]) {m[x-1][y-1]=1;   marked++; }  winsh(x-1,y-1);}
                if (o[x-1][y]!=1) { if (!m[x-1][y]) {m[x-1][y]=1;   marked++; } winsh(x-1,y);}
                if (o[x-1][y+1]!=1) { if (!m[x-1][y+1]) {m[x-1][y+1]=1;   marked++;}  winsh(x-1,y+1);}
                if (o[x][y-1]!=1) { if (!m[x][y-1]) {m[x][y-1]=1;   marked++; } winsh(x,y-1);}
                if (o[x][y+1]!=1) { if (!m[x][y+1]) {m[x][y+1]=1;   marked++;}  winsh(x,y+1);}
                if (o[x+1][y-1]!=1) { if (!m[x+1][y-1]) {m[x+1][y-1]=1;   marked++;}  winsh(x+1,y-1);}
                if (o[x+1][y]!=1) { if (!m[x+1][y]) {m[x+1][y]=1;   marked++;}  winsh(x+1,y);}
                if (o[x+1][y+1]!=1) { if (!m[x+1][y+1]) {m[x+1][y+1]=1;   marked++;}  winsh(x+1,y+1);}
            }
            if (num_mine_beside(x, y)==num_mark_beside(x, y)) dblclick(x,y);
        }
//add by stiger,2001,6,12,¸ÄÕýmarkedÏÔÊ¾µÄÎÊÌâ
        move(20,30);
        prints("±ê¼Ç: %d ", marked);
//add end
        if (c=='H') winhelp();
        if (c=='d'|| c=='D') winrefresh();
        if (c=='f'|| c=='F') {
            if (!inited) {
                wininit(x, y);
                inited= 1;
            }
            dig(x, y);
        }
        if ((c==83|| c==115)&&!o[x][y]) {
            if (m[x][y]) {
                m[x][y]=0;
                marked--;
            } else {
                m[x][y]=1;
                marked++;
            }
            winsh(x, y);
//add by stiger,2001,6,12
            move(20,30);
            prints("±ê¼Ç: %d ", marked);
//add end
        }
        if (checkwin()==1) {
            /*add by stiger*/
            stiger=steps*1.0/(time(0)-t0);
            move(22,0);
            prints("×£ºØÄã£¡Äã³É¹¦ÁË£¡Æ½¾ùÃ¿Ãë%lf²½",stiger);
            gameover= 0;
            win_checkrec(time(0)-t0);/* added by soff ½øÐÐÅÅÐÐ¼ì²é */
            return 1;
        }
        if (gameover) {
            move(22,0);
            prints("ºÜÒÅº¶£¬ÄãÊ§°ÜÁË... ÔÙÀ´Ò»´Î°É£¡                                 ");
            gameover= 0;
            return 1;
        }
        move(y-1,x*2-2);
        refresh();
    }
    return 1;
}

int checkwin()
{
    int x,y,s;
    s=0;
    for (x=1; x<=30; x++)
        for (y=1; y<=16; y++)
            if (!o[x][y])s++;
    if (s==99) return 1;
    return 0;
}

int dig(int x, int y)
{
    if (!o[x][y])
        windig(x, y);
    else
        dblclick(x, y);
    return 0;
}

int winrefresh()
{
    int x, y;
    clear();
    move(22,0);
    prints("[1;32m¡î¼üÅÌÉ¨À×¡î[0;1m [[35m°ïÖú: H[37m] [[36mÍË³ö: Ctrl+C[37m] [[35m´ò¿ª: f[37m] [[36m±êÀ×: s][m\n");
    for (y=1; y<=16; y++) {
        move(y-1,0);
        for (x=1; x<=30; x++)
            winsh0(x, y);
    }
    refresh();
    return 0;
}

int winhelp()
{
    clear();
    prints("==»¶Ó­À´Íæ¼üÅÌÉ¨À×ÓÎÏ·== \r\n---------------------------------\r\n\r\n");
    prints("Íæ·¨ºÜ¼òµ¥£¬ºÍ[1;34mwindows[mÏÂµÄÊó±êÉ¨À×²î²»¶à.\r\n");
    prints("  '[1;32mF[m'¼üµÄ×÷ÓÃÏàµ±ÓÚÊó±êµÄ×ó¼ü¼°Ë«»÷µÄ×÷ÓÃ£¬ ³ÌÐò¸ù¾ÝÄãµã»÷µÄÎ»ÖÃ\r\n");
    prints("  ×Ô¶¯ÅÐ¶ÏÒª½øÐÐÄÄÖÖ²Ù×÷¡£\r\n");
    prints("  '[1;32mS[m'¼üÔòÏàµ±ÓÚÊó±êÓÒ¼üµÄ¹¦ÄÜ, ¿ÉÓÃÀ´±êÀ×.\r\n");
    prints("  '[1;32mH[m'¼üÓÃÀ´ÏÔÊ¾±¾°ïÖúÐÅÏ¢.\r\n");
    prints("  '[1;32mQ[m'¼üÍË³öÓÎÏ·.\r\n");
    prints("  µ±ÆÁÄ»ÂÒµôÊ±£¬¿ÉÓÃ'[1;32mD[m'¿ÉÓÃÀ´Ë¢ÐÂÆÁÄ»¡£\r\n");
    prints("µÚÒ»´Îµã»÷Ò»¶¨»á¿ªÒ»Æ¬£¬ºÜÊæ·þ°É¡£\r\n");
    prints("ÊìÁ·ºó£¬ËÙ¶È»¹ÊÇºÜ¿ìµÄ£¬¼¸ºõ¿ÉÒÔ´ïµ½Êó±êÉ¨À×µÄËÙ¶È.\r\n");
    pressanykey();
    winrefresh();
    return 0;
}

int win_loadrec()
{
    FILE *fp;
    int n;
    for (n=0; n<=19; n++) {
        strcpy(topID[n], "null.");
        topT[n]=999;
        topS[n]=0;
        strcpy(topFROM[n], "unknown.");
    }
    fp=fopen(recfile, "r");
    if (fp==NULL) {
        win_saverec();
        return 0;
    }
    for (n=0; n<=19; n++)
        fscanf(fp, "%s %d %s %lf\n", topID[n], &topT[n], topFROM[n],&topS[n]);
    fclose(fp);
    return 0;
}

int win_saverec()
{
    FILE *fp;
    int n;
    fp=fopen(recfile, "w");
    for (n=0; n<=19; n++) {
        fprintf(fp, "%s %d %s %lf\n", topID[n], topT[n], topFROM[n],topS[n]);
    }
    fclose(fp);
    return 0;
}

int win_showrec()
{
    int n;
    char buf[256];
    win_loadrec();
    clear();
    prints("[44;37m                      --==    É¨À×ÅÅÐÐ°ñ    ==--                             \r\n[m");
    prints("[41m No.          ID        TIME       Steps/sec                            [m\r\n");
    for (n=0; n<=19; n++) {
        sprintf(buf,"[1;37m%3d[32m%13s[0;37m%12d[m     %lf \r\n", n+1, topID[n], topT[n], topS[n]);
        prints(buf);
    }
    prints("[41m                                                                               [m\r\n");
    pressanykey();
    return 0;
}

int win_checkrec(int dt)
{
    char id[20];
    int n;
    win_loadrec();
    strcpy(id, userid);
    if (dt < 50) {
        clear();
        prints("Ò»ÊÇ»úÆ÷ÈË!!!\n·£ÄãË¯Ãß1·ÖÖÓ\n");
        refresh();
        sleep(60);
        pressanykey();
        return 0;
    }
    for (n=0;n<20;n++)
        if (!strcmp(topID[n], id)) {
            if (stiger > topS[n]) {
                topS[n]=stiger;
                win_saverec();
            }
            if (dt< topT[n]) {
                topT[n]= dt;
                strcpy(topFROM[n], fromhost);
                win_sort();
                win_saverec();
            }
            return 0;
        }
    if (dt<topT[19]) {
        strcpy(topID[19], id);
        topT[19]= dt;
        topS[19]= stiger;
        strcpy(topFROM[19], fromhost);
        win_sort();
        win_saverec();
        return 0;
    }
    return 0;
}

int win_sort()
{
    int n, n2, tmp;
    double tmp1;
    char tmpID[20];
    clear();
    prints("×£ºØ! ÄúË¢ÐÂÁË×Ô¼ºµÄ¼ÍÂ¼!\r\n");
    pressanykey();
    for (n=0; n<=18; n++)
        for (n2=n+1; n2<=19; n2++)
            if (topT[n]> topT[n2]) {
                tmp= topT[n];
                topT[n]= topT[n2];
                topT[n2]= tmp;
                tmp1= topS[n];
                topS[n]= topS[n2];
                topS[n2]= tmp1;
                strcpy(tmpID, topID[n]);
                strcpy(topID[n], topID[n2]);
                strcpy(topID[n2], tmpID);
                strcpy(tmpID, topFROM[n]);
                strcpy(topFROM[n], topFROM[n2]);
                strcpy(topFROM[n2], tmpID);
            }
    return 0;
}
