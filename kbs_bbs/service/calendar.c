#define BBSMAIN
#include "bbs.h"
#include <time.h>

char save_scr[100][240];
int save_y, save_x;
char num[11][3]={"零", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};

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
    for(i=0;i<t_lines-1;i++)
        saveline(i, 1, save_scr[i]);
    resetcolor();
    for(i=0;i<13;i++) {
        move(i, 50);
        clrtoeol();
    }
    move(0, 52);
    prints("\x1b[31;1m日  \x1b[33m一  二  三  四  五  \x1b[31m六");
    for(i=0;i<6;i++) {
        move(1+i*2, 52);
        prints("\x1b[0;36m━━━━━━━━━━━━━━");
    }
    k=0;
    for(i=1;i<=get_day(year,month);i++) {
        j=get_week(year,month,i);
        y=k*2+2;
        x=j*4+52;
        resetcolor();
        if(j==0||j==6) setfcolor(RED, 1);
        else setfcolor(YELLOW, 1);
        if(i==day) setbcolor(BLUE);
        move(y,x);
        prints("%2d", i);

        if(j==6) k++;
    }
}

int calendar_main()
{
    int i,j,ch;
    struct tm nowr;
    char buf[80];
    long eff_size;
    time_t now;
    getyx(&save_y, &save_x);
    for(i=0;i<t_lines-1;i++)
        saveline(i, 0, save_scr[i]);
    now = time(0);
    localtime_r(&now, &nowr);
    day = nowr.tm_mday;
    month = nowr.tm_mon+1;
    year = nowr.tm_year+1900;
    
    while(1){
        draw_main();
        ch = igetkey();
        if(ch==KEY_ESC) break;
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
                sprintf(buf, "home/%c/%s/%d-%02d-%02d.txt", toupper(currentuser->userid[0]), currentuser->userid, year, month, day);
                vedit(buf, 0, &eff_size, 0);
                break;
        }
    }

    for(i=0;i<t_lines-1;i++)
        saveline(i, 1, save_scr[i]);
    move(save_y, save_x);
}
