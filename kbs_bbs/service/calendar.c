/******************************************************
日记日历2003, 作者: bad@smth.org  Qian Wenjie
在水木清华bbs系统上运行

本游戏是自由软件，请随意复制移植
请在修改后的文件头部保留版权信息
******************************************************/
#define BBSMAIN
#include "bbs.h"
#include <time.h>

char save_scr[100][240];
int save_y, save_x;
char nums[11][3]={"〇", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};
char week[7][3]={"日", "一", "二", "三", "四", "五", "六"};
int holiday_m[]={1, 2, 3, 3, 4, 5, 5, 6, 7, 8, 10, -1};
int holiday_d[]={1, 14, 8, 9, 1, 1, 4, 1, 1, 1, 1};
char holiday_s[][30]={"新年元旦", "情人节", "国际妇女节", "清华女生节", "愚人节", "国际劳动节", 
    "中国五四青年节", "国际儿童节", "中国共产党建党日", "中国建军节", "国庆节"};

int day,month,year;

int get_day(int year, int month)
{
    int day[12]={31,28,31,30,31,30,31,31,30,31,30,31};
    if(month==2&&(year%4==0&&(year%100!=0||year%400==0))) return 29;
    else return day[month-1];
}

int get_day2(int year)
{
    if(year%4==0&&(year%100!=0||year%400==0)) return 366;
    else return 365;
}

int get_week(int year, int month, int day)
{
    int total=0,i;
    for(i=1900;i<year;i++) total+=get_day2(i)%7;
    for(i=1;i<month;i++) total+=get_day(year, i)%7;
    total+=day;
    return total%7;
}

void draw_main()
{
    int i,j,k,x,y;
    char buf[80];
    struct stat st;
    for(i=0;i<t_lines;i++)
        saveline(i, 1, save_scr[i]);
    resetcolor();
    for(i=0;i<13;i++) {
        move(i, 48);
        clrtoeol();
    }
    move(0, 52);
    prints("\x1b[31;1m日  \x1b[33m一  二  三  四  五  \x1b[31m六");
    for(i=0;i<6;i++) {
        move(1+i*2, 52);
        prints("\x1b[0;36m━━━━━━━━━━━━━━");
    }
    prints("\x1b[32;1m");
    move(1, 49);
    prints("%s", nums[year/1000]);
    move(2, 49);
    prints("%s", nums[year/100%10]);
    move(3, 49);
    prints("%s", nums[year/10%10]);
    move(4, 49);
    prints("%s", nums[year%10]);
    move(5, 49);
    prints("年");
    if(month>10) {
        move(7, 49);
        prints("%s", nums[10]);
        move(8, 49);
        prints("%s", nums[month%10]);
        move(9, 49);
        prints("月");
    }
    else {
        move(7, 49);
        prints("%s", nums[month]);
        move(8, 49);
        prints("月");
    }
    k=0;
    for(i=1;i<=get_day(year,month);i++) {
        j=get_week(year,month,i);
        y=k*2+2;
        x=j*4+52;
        resetcolor();
        if(j==0||j==6||month==1&&i==1) setfcolor(RED, 1);
        else setfcolor(YELLOW, 1);
        if(i==day) setbcolor(PINK);
        sprintf(buf, "home/%c/%s/%d-%02d-%02d.txt", toupper(currentuser->userid[0]), currentuser->userid, year, month, i);
        if(stat(buf, &st)!=-1) prints("\x1b[4m");
        move(y,x);
        prints("%2d", i);

        if(j==6) k++;
    }
    i=0;
    while(holiday_m[i]!=-1) {
        if(month==holiday_m[i]&&day==holiday_d[i]) {
            strcpy(buf, holiday_s[i]);
            move(12, 80-strlen(buf));
            resetcolor();
            prints("%s", buf);
        }
        i++;
    }
    move(t_lines-1, 80);
}

int newfile(char * s)
{
    struct stat st;
    FILE* fp;
    if(stat(s, &st)==-1) {
        fp=fopen(s, "w");
        fprintf(fp, "标题: \n");
        fprintf(fp, "%d/%02d/%02d                星期%s                天气:\n\n", year, month, day, week[get_week(year,month,day)]);
        fclose(fp);
        return 1;
    }
    return 0;
}

void encode_file(char * s)
{
    char buf[1024*16];
    char fn;
    int o, i;
    FILE *fp1, *fp2;
    sprintf(fn, "tmp/%d.cal", rand());
    fp1 = fopen(s, "rb");
    fp2 = fopen(fn, "wb");
    while((o=fread(buf, 1, 1024*16, fp1))>0) {
        for(i=0;i<o;i++)
            buf[i]=buf[i]^0x1f;
        fwrite(buf, 1, o, fp2);
    }
    fclose(fp1);
    fclose(fp2);
    fp1 = fopen(s, "wb");
    fp2 = fopen(fn, "rb");
    while((o=fread(buf, 1, 1024*16, fp2))>0) {
        fwrite(buf, 1, o, fp1);
    }
    fclose(fp1);
    fclose(fp2);
    unlink(fn);
}

extern int incalendar;

int calendar_main()
{
    int i,j,ch,oldmode,cc;
    struct tm nowr;
    struct stat st;
    char buf[80], title[80];
    long eff_size;
    time_t now;
    incalendar = 1;
    oldmode = uinfo.mode;
    modify_user_mode(CALENDAR);
    getyx(&save_y, &save_x);
    for(i=0;i<t_lines;i++)
        saveline(i, 0, save_scr[i]);
    now = time(0);
    localtime_r(&now, &nowr);
    day = nowr.tm_mday;
    month = nowr.tm_mon+1;
    year = nowr.tm_year+1900;
    
    while(1){
        draw_main();
        ch = igetkey();
        if(ch==KEY_ESC||toupper(ch)=='Q'||ch==KEY_F10) break;
        switch(ch) {
            case KEY_UP:
                if(day>7) day-=7;
                else day=1;
                break;
            case KEY_DOWN:
                if(day+7<=get_day(year,month)) day+=7;
                else day=get_day(year,month);
                break;
            case KEY_LEFT:
                day--;
                if(day<=0) day=1;
                break;
            case KEY_RIGHT:
                day++;
                if(day>get_day(year,month)) day=get_day(year,month);
                break;
            case KEY_PGUP:
                month--;
                if(month<=0) {
                    year--;
                    month=12;
                }
                break;
            case KEY_PGDN:
                month++;
                if(month>12) {
                    year++;
                    month=1;
                }
                break;
            case 13:
            case 10:
                cc = 0;
                modify_user_mode(CALENEDIT);
                sprintf(buf, "home/%c/%s/%d-%02d-%02d.txt", toupper(currentuser->userid[0]), currentuser->userid, year, month, day);
                if(stat(buf, &st)!=-1)
                    encode_file(buf);
                else
                    cc = newfile(buf);
                if(vedit(buf, 0, &eff_size, 0)&&cc) unlink(buf);
                else encode_file(buf);
                modify_user_mode(CALENDAR);
                break;
            case 32:
                sprintf(buf, "home/%c/%s/%d-%02d-%02d.txt", toupper(currentuser->userid[0]), currentuser->userid, year, month, day);
                sprintf(title, "%d/%02d/%02d", year, month, day);
                if(stat(buf, &st)!=-1) {
                    encode_file(buf);
                    ansimore_withzmodem(buf, true, title);
                    encode_file(buf);
                }
                break;
            case KEY_HOME:
                day = nowr.tm_mday;
                month = nowr.tm_mon+1;
                year = nowr.tm_year+1900;
                break;
            case KEY_DEL:
                sprintf(buf, "home/%c/%s/%d-%02d-%02d.txt", toupper(currentuser->userid[0]), currentuser->userid, year, month, day);
                if(stat(buf, &st)!=-1) {
                    getdata(13, 48, "确认删除该日日记[y/N]", title, 3, 1, 0, 1);
                    if(toupper(title[0])=='Y')
                        unlink(buf);
                }
                break;
        }
    }

    for(i=0;i<t_lines;i++)
        saveline(i, 1, save_scr[i]);
    move(save_y, save_x);
    modify_user_mode(oldmode);
    incalendar = 0;
    resetcolor();
}
