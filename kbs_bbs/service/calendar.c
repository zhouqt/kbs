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
unsigned long lunarInfo[]={
0x04bd8,0x04ae0,0x0a570,0x054d5,0x0d260,0x0d950,0x16554,0x056a0,0x09ad0,0x055d2,
0x04ae0,0x0a5b6,0x0a4d0,0x0d250,0x1d255,0x0b540,0x0d6a0,0x0ada2,0x095b0,0x14977,
0x04970,0x0a4b0,0x0b4b5,0x06a50,0x06d40,0x1ab54,0x02b60,0x09570,0x052f2,0x04970,
0x06566,0x0d4a0,0x0ea50,0x06e95,0x05ad0,0x02b60,0x186e3,0x092e0,0x1c8d7,0x0c950,
0x0d4a0,0x1d8a6,0x0b550,0x056a0,0x1a5b4,0x025d0,0x092d0,0x0d2b2,0x0a950,0x0b557,
0x06ca0,0x0b550,0x15355,0x04da0,0x0a5d0,0x14573,0x052b0,0x0a9a8,0x0e950,0x06aa0,
0x0aea6,0x0ab50,0x04b60,0x0aae4,0x0a570,0x05260,0x0f263,0x0d950,0x05b57,0x056a0,
0x096d0,0x04dd5,0x04ad0,0x0a4d0,0x0d4d4,0x0d250,0x0d558,0x0b540,0x0b5a0,0x195a6,
0x095b0,0x049b0,0x0a974,0x0a4b0,0x0b27a,0x06a50,0x06d40,0x0af46,0x0ab60,0x09570,
0x04af5,0x04970,0x064b0,0x074a3,0x0ea50,0x06b58,0x055c0,0x0ab60,0x096d5,0x092e0,
0x0c960,0x0d954,0x0d4a0,0x0da50,0x07552,0x056a0,0x0abb7,0x025d0,0x092d0,0x0cab5,
0x0a950,0x0b4a0,0x0baa4,0x0ad50,0x055d9,0x04ba0,0x0a5b0,0x15176,0x052b0,0x0a930,
0x07954,0x06aa0,0x0ad50,0x05b52,0x04b60,0x0a6e6,0x0a4e0,0x0d260,0x0ea65,0x0d530,
0x05aa0,0x076a3,0x096d0,0x04bd7,0x04ad0,0x0a4d0,0x1d0b6,0x0d250,0x0d520,0x0dd45,
0x0b5a0,0x056d0,0x055b2,0x049b0,0x0a577,0x0a4b0,0x0aa50,0x1b255,0x06d20,0x0ada0,
0x14b63};
char nums[11][3]={"〇", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};
char week[7][3]={"日", "一", "二", "三", "四", "五", "六"};
char sFtv[][100] = {
"0101*元旦",
"0214 情人节",
"0307 女生节",
"0308 妇女节",
"0501 劳动节",
"0504 青年节",
"0601 儿童节",
"0701 建党节 香港回归纪念",
"0801 建军节",
"1001*国庆节",
""};

char lFtv[][100] = {
"0101*春节",
"0115 元宵节",
"0505 端午节",
"0815 中秋节",
"0909 重阳节",
"1224 小年",
"0100*除夕",
""};

char wFtv[][100] = {
"0520 母亲节 Mother's Day",
"0630 父亲节 Father's Day",
"1144 感恩节 Thanksgiving Day",
""};

int day,month,year;

#define getnum(a) (((a)[0]-'0')*10+(a)[1]-'0')

int leapMonth(int y) {
    return(lunarInfo[y-1900] & 0xf);
}

int leapDays(int y)
{
    if(leapMonth(y))  return((lunarInfo[y-1900] & 0x10000)? 30: 29);
    else return(0);
}

int lYearDays(int y) 
{
    unsigned long i, sum = 348;
    for(i=0x8000; i>0x8; i>>=1) sum += (lunarInfo[y-1900] & i)? 1: 0;
    return(sum+leapDays(y));
}

int monthDays(int y,int m) {
    return( (lunarInfo[y-1900] & (0x10000>>m))? 30: 29 );
}

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

void Lunar(int day, int * lmonth, int * lday)
{
    int i, j, leap=0, temp=0;
    int offset   = 0;
    bool isLeap;
    int lyear;

    for(j=2;j<=12;j++) offset+=get_day(1900, j);
    for(i=1901;i<year;i++) offset+=get_day2(i);
    for(j=1;j<month;j++) offset+=get_day(year, j);
    offset+=day;
    for(i=1900; i<2050 && offset>0; i++) { temp=lYearDays(i); offset-=temp; }
    if(offset<0) { offset+=temp; i--; }

    lyear = i;
    leap = leapMonth(i);
    isLeap = false;

    for(i=1; i<13 && offset>0; i++) {
        if(leap>0 && i==(leap+1) && isLeap==false)
            { --i; isLeap = true; temp = leapDays(lyear); }
        else
            { temp = monthDays(lyear, i); }
        if(isLeap==true && i==(leap+1)) isLeap = false;
        offset -= temp;
    }

    if(offset==0 && leap>0 && i==leap+1)
        if(isLeap)
            { isLeap = false; }
        else
            { isLeap = true; --i; }

    if(offset<0){ offset += temp; --i; }

    *lmonth = i;
    *lday = offset + 1;
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
    int i,j,k,x,y,lmonth,lday,i0;
    char buf[80];
    struct stat st;
    for(i=0;i<t_lines;i++)
        saveline(i, 1, save_scr[i]);
    resetcolor();
    for(i=0;i<14;i++) {
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
        Lunar(i, &lmonth, &lday);
        y=k*2+2;
        x=j*4+52;
        resetcolor();
        if(j==0||j==6) setfcolor(RED, 1);
        else setfcolor(YELLOW, 1);
        i0=0;
        while(sFtv[i0][0]) {
            if(sFtv[i0][4]=='*'&&getnum(sFtv[i0])==month&&getnum(sFtv[i0]+2)==i)
                setfcolor(RED, 1);
            i0++;
        }
        i0=0;
        while(lFtv[i0][0]) {
            if(lFtv[i0][4]=='*'&&getnum(lFtv[i0])==lmonth&&getnum(lFtv[i0]+2)==lday)
                setfcolor(RED, 1);
            i0++;
        }
        if(i==day) setbcolor(PINK);
        sprintf(buf, "home/%c/%s/%d-%02d-%02d.txt", toupper(currentuser->userid[0]), currentuser->userid, year, month, i);
        if(stat(buf, &st)!=-1) prints("\x1b[4m");
        move(y,x);
        prints("%2d", i);

        if(j==6) k++;
    }

    move(12, 56);
    resetcolor();
    Lunar(day, &lmonth, &lday);
    sprintf(buf, "农历:%d月%d日", lmonth, lday);
    move(12, 80-strlen(buf));
    resetcolor();
    prints(buf);

    k = 80;
    resetcolor();
    i0=0;
    while(sFtv[i0][0]) {
        if(getnum(sFtv[i0])==month&&getnum(sFtv[i0]+2)==i) {
            strcpy(buf, sFtv[i0]+5);
            k-=strlen(buf);
            move(13, k);
            k--;
            prints(buf);
        }
        i0++;
    }
    i0=0;
    while(lFtv[i0][0]) {
        if(getnum(lFtv[i0])==lmonth&&getnum(lFtv[i0]+2)==lday) {
            strcpy(buf, lFtv[i0]+5);
            k-=strlen(buf);
            move(13, k);
            k--;
            prints(buf);
        }
        i0++;
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

#define IV1         0x12345678
#define IV2         0xabcdef44
void tea_encipher(unsigned long* v, unsigned long* k)  
{              
    register unsigned long y=v[0],z=v[1], sum=0, delta=0x9e3779b9, n=32;
    while (n-->0) {                       
        sum += delta ;
        y += (z<<4)+k[0] ^ z+sum ^ (z>>5)+k[1];
        z += (y<<4)+k[2] ^ y+sum ^ (y>>5)+k[3];   
    } 
    v[0]=y; 
    v[1]=z; 
}

void tea_decipher(unsigned long* v,unsigned long* k)  
{
    register unsigned long n=32, sum, y=v[0], z=v[1],delta=0x9e3779b9;
    sum=delta<<5;
                       
    while (n-->0) {
        z-= (y<<4)+k[2] ^ y+sum ^ (y>>5)+k[3]; 
        y-= (z<<4)+k[0] ^ z+sum ^ (z>>5)+k[1];
        sum-=delta;  
    }
    v[0]=y; 
    v[1]=z;  
}

void encipher(char* buf,size_t len,unsigned long *k)
{
    unsigned long l[2];
    unsigned long iv[2] = {IV1,IV2};
    int i;
    if (len % 8) return;
    len = len / 8;
    for (i=0; i<len; i++) {
        memcpy(l,buf,8);
        l[0] ^= iv[0]; l[1] ^= iv[1];
        tea_encipher(l,k);
        memcpy(buf,l,8);
        iv[0] = l[0]; iv[1] = l[1];
        buf += 8;
    }
}

void decipher(char* buf,size_t len,unsigned long *k)
{
    unsigned long l[2];
    unsigned long iv[2];
    if (len % 8) return;
    buf += len;
    for (len=len/8-1; len; --len) {
        buf -= 8;
        memcpy(l,buf,8);
        tea_decipher(l,k);
        memcpy(iv,buf-8,8);
        l[0] ^= iv[0]; l[1] ^= iv[1];
        memcpy(buf,l,8);
    }
    buf -= 8;
    memcpy(l,buf,8);
    tea_decipher(l,k);
    l[0] ^= IV1; l[1] ^= IV2;
    memcpy(buf,l,8);
}

void encode_file(char * s)
{
    char buf[1024*16];
    char fn[80];
    unsigned long k[4];
    int o, i;
    FILE *fp1, *fp2;
    k[0] = sysconf_eval("CALENDAR_KEY0", 0x234251);
    k[1] = sysconf_eval("CALENDAR_KEY1", 0x1234251);
    k[2] = sysconf_eval("CALENDAR_KEY2", 0x2234251);
    k[3] = sysconf_eval("CALENDAR_KEY3", 0x3234251);
    sprintf(fn, "tmp/%s.%d.cal", currentuser->userid, rand());
    fp1 = fopen(s, "rb");
    fp2 = fopen(fn, "wb");
    while((o=fread(buf, 1, 1024*16, fp1))>0) {
        if(o%8!=0) {
            for(i=o;i<(o/8+1)*8;i++)
                buf[i]=32;
            o=(o/8+1)*8;
        }
        encipher(buf, o, k);
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

void decode_file(char * s)
{
    char buf[1024*16];
    char fn[80];
    unsigned long k[4];
    int o, i;
    FILE *fp1, *fp2;
    k[0] = sysconf_eval("CALENDAR_KEY0", 0x234251);
    k[1] = sysconf_eval("CALENDAR_KEY1", 0x1234251);
    k[2] = sysconf_eval("CALENDAR_KEY2", 0x2234251);
    k[3] = sysconf_eval("CALENDAR_KEY3", 0x3234251);
    sprintf(fn, "tmp/%s.%d.cal", currentuser->userid, rand());
    fp1 = fopen(s, "rb");
    fp2 = fopen(fn, "wb");
    while((o=fread(buf, 1, 1024*16, fp1))>0) {
        if(o%8!=0) {
            for(i=o;i<(o/8+1)*8;i++)
                buf[i]=32;
            o=(o/8+1)*8;
        }
        decipher(buf, o, k);
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
                    decode_file(buf);
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
                    decode_file(buf);
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
