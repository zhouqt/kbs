/******************************************************
日记日历2003, 作者: bad@smth.org  Qian Wenjie
在水木清华bbs系统上运行

本游戏是自由软件，请随意复制移植
请在修改后的文件头部保留版权信息
******************************************************/
#define BBSMAIN
#include "bbs.h"
#include <time.h>

char save_scr[LINEHEIGHT][LINELEN*3];
int save_y, save_x;
unsigned long lunarInfo[]={
0x4bd8,0x4ae0,0xa570,0x54d5,0xd260,0xd950,0x5554,0x56af,0x9ad0,0x55d2,
0x4ae0,0xa5b6,0xa4d0,0xd250,0xd295,0xb54f,0xd6a0,0xada2,0x95b0,0x4977,
0x497f,0xa4b0,0xb4b5,0x6a50,0x6d40,0xab54,0x2b6f,0x9570,0x52f2,0x4970,
0x6566,0xd4a0,0xea50,0x6a95,0x5adf,0x2b60,0x86e3,0x92ef,0xc8d7,0xc95f,
0xd4a0,0xd8a6,0xb55f,0x56a0,0xa5b4,0x25df,0x92d0,0xd2b2,0xa950,0xb557,
0x6ca0,0xb550,0x5355,0x4daf,0xa5b0,0x4573,0x52bf,0xa9a8,0xe950,0x6aa0,
0xaea6,0xab50,0x4b60,0xaae4,0xa570,0x5260,0xf263,0xd950,0x5b57,0x56a0,
0x96d0,0x4dd5,0x4ad0,0xa4d0,0xd4d4,0xd250,0xd558,0xb540,0xb6a0,0x95a6,
0x95bf,0x49b0,0xa974,0xa4b0,0xb27a,0x6a50,0x6d40,0xaf46,0xab60,0x9570,
0x4af5,0x4970,0x64b0,0x74a3,0xea50,0x6b58,0x5ac0,0xab60,0x96d5,0x92e0,
0xc960,0xd954,0xd4a0,0xda50,0x7552,0x56a0,0xabb7,0x25d0,0x92d0,0xcab5,
0xa950,0xb4a0,0xbaa4,0xad50,0x55d9,0x4ba0,0xa5b0,0x5176,0x52bf,0xa930,
0x7954,0x6aa0,0xad50,0x5b52,0x4b60,0xa6e6,0xa4e0,0xd260,0xea65,0xd530,
0x5aa0,0x76a3,0x96d0,0x4afb,0x4ad0,0xa4d0,0xd0b6,0xd25f,0xd520,0xdd45,
0xb5a0,0x56d0,0x55b2,0x49b0,0xa577,0xa4b0,0xaa50,0xb255,0x6d2f,0xada0,
0x4b63,0x937f,0x49f8,0x4970,0x64b0,0x68a6,0xea5f,0x6b20,0xa6c4,0xaaef,
0x92e0,0xd2e3,0xc960,0xd557,0xd4a0,0xda50,0x5d55,0x56a0,0xa6d0,0x55d4,
0x52d0,0xa9b8,0xa950,0xb4a0,0xb6a6,0xad50,0x55a0,0xaba4,0xa5b0,0x52b0,
0xb273,0x6930,0x7337,0x6aa0,0xad50,0x4b55,0x4b6f,0xa570,0x54e4,0xd260,
0xe968,0xd520,0xdaa0,0x6aa6,0x56df,0x4ae0,0xa9d4,0xa4d0,0xd150,0xf252,
0xd520};
char nums[11][3]={"〇", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};
char week[7][3]={"日", "一", "二", "三", "四", "五", "六"};
char sFtv[][100] = {
"0101*新年元旦",
"0210 国际气象节",
"0214 情人节",
"0307 清华女生节",
"0308 国际妇女节",
"0312 植树节",
"0314 国际警察日",
"0315 国际消费者权益日",
"0317 中国国医节",
"0322 世界水日",
"0323 世界气象日",
"0401 愚人节",
"0407 世界卫生日",
"0422 世界地球日",
"0501 国际劳动节",
"0504 中国五四青年节",
"0508 世界红十字日",
"0512 国际护士节",
"0515 国际家庭日",
"0517 世界电信日",
"0531 世界无烟日", 
"0601 国际儿童节",
"0605 世界环境日",
"0623 国际奥林匹克日",
"0701 中国共产党建党日",
"0711 世界人口日",
"0801 中国建军节",
"0908 国际扫盲日",
"0910 教师节",
"0927 世界旅游日",
"1001*国庆节",
"1031 世界勤俭日",
"1110 世界青年节",
"1117 国际大学生节 世界学生节",
"1201 世界艾滋病日",
"1203 世界残疾人日",
"1224 平安夜",
"1225 圣诞节",
""};

char lFtv[][100] = {
"0101*春节",
"0115 元宵节",
"0202 龙抬头节",
"0505 端午节",
"0707 七七中国情人节",
"0815 中秋节",
"0909 重阳节",
"1208 腊八节",
"1223 灶君(祭灶)节",
"0100*除夕",
""};

char wFtv[][100] = {
"0520 国际母亲节",
"0630 父亲节",
"0911 劳动节",
"1144 感恩节",
"0480 清华校庆日",
""};

char lmonths[14][20] = {
"","正","二","三","四","五","六","七","八","九","十","十一","腊",""};

int sTermInfo[] = {0,21208,42467,63836,85337,107014,128867,150921,173149,195551,218072,240693,263343,285989,308563,331033,353350,375494,397447,419210,440795,462224,483532,504758};
int day,month,year;
int lyear;

#define getnum(a) (((a)[0]-'0')*10+(a)[1]-'0')

int leapMonth(int y) {
    int lm = lunarInfo[y-1900] & 0xf;
    return(lm==0xf?0:lm);
}

int leapDays(int y)
{
     if(leapMonth(y)) return( (lunarInfo[y-1899]&0xf)==0xf? 30: 29);
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

    for(j=2;j<=12;j++) offset+=get_day(1900, j);
    for(i=1901;i<year;i++) offset+=get_day2(i);
    for(j=1;j<month;j++) offset+=get_day(year, j);
    offset+=day;
    for(i=1900; i<2100 && offset>0; i++) { temp=lYearDays(i); offset-=temp; }
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

char solarTerm[24][8] = 
{"小寒","大寒","立春","雨水","惊蛰","春分",
 "清明","谷雨","立夏","小满","芒种","夏至",
 "小暑","大暑","立秋","处暑","白露","秋分",
 "寒露","霜降","立冬","小雪","大雪","冬至"};

int sTerm(int y, int n, int day)
{
    double k=
       (31556925974.7*(y-1900) + sTermInfo[n]*60000 + 1.0/24*2 + 1.0/24/60*5)/86400000-6;
    int pass, i, j;
    pass = (int) k;
    for(i=1900;i<year;i++) pass-=get_day2(i);
    for(i=1;i<month;i++) pass-=get_day(year, i);
    pass-=day;
    return (pass==0);
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
    char buf[80],buf2[80];
    struct stat st;
    for(i=0;i<t_lines;i++)
        saveline(i, 1, save_scr[i]);
    resetcolor();
    for(i=0;i<14;i++) {
        move(i, scr_cols-32);
        clrtoeol();
    }
    move(0, scr_cols-28);
    prints("\x1b[31;1m日  \x1b[33m一  二  三  四  五  \x1b[31m六");
    for(i=0;i<6;i++) {
        move(1+i*2, scr_cols-28);
        prints("\x1b[0;36m━━━━━━━━━━━━━━");
    }
    prints("\x1b[32;1m");
    move(1, scr_cols-31);
    prints("%s", nums[year/1000]);
    move(2, scr_cols-31);
    prints("%s", nums[year/100%10]);
    move(3, scr_cols-31);
    prints("%s", nums[year/10%10]);
    move(4, scr_cols-31);
    prints("%s", nums[year%10]);
    move(5, scr_cols-31);
    prints("年");
    if(month>10) {
        move(7, scr_cols-31);
        prints("%s", nums[10]);
        move(8, scr_cols-31);
        prints("%s", nums[month%10]);
        move(9, scr_cols-31);
        prints("月");
    }
    else {
        move(7, scr_cols-31);
        prints("%s", nums[month]);
        move(8, scr_cols-31);
        prints("月");
    }
    k=0;
    for(i=1;i<=get_day(year,month);i++) {
        j=get_week(year,month,i);
        Lunar(i, &lmonth, &lday);
        y=k*2+2;
        x=j*4+scr_cols-28;
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

    resetcolor();
    Lunar(day, &lmonth, &lday);
    if(lday<=10) sprintf(buf2, "初%s", nums[lday]);
    else if(lday<=19) sprintf(buf2, "十%s", nums[lday-10]);
    else if(lday==20) sprintf(buf2, "二十");
    else if(lday<=29) sprintf(buf2, "廿%s", nums[lday-20]);
    else if(lday==30) sprintf(buf2, "三十");
    else if(lday<=39) sprintf(buf2, "卅%s", nums[lday-30]);
    else sprintf(buf2, "□");
    sprintf(buf, "农历 %s月%s", lmonths[lmonth], buf2);
    move(12, scr_cols-strlen(buf));
    resetcolor();
    prints(buf);

    k = scr_cols;
    resetcolor();
    i0=0;
    while(sFtv[i0][0]) {
        if(getnum(sFtv[i0])==month&&getnum(sFtv[i0]+2)==day) {
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
    i0=0;
    while(wFtv[i0][0]) {
        if(getnum(wFtv[i0])==month&&(wFtv[i0][3]-'0'==get_week(year,month,day))&&
            ((wFtv[i0][2]-'1'==day/7)||('8'-wFtv[i0][2]==(get_day(year,month)+1-day)/7))) {
            strcpy(buf, wFtv[i0]+5);
            k-=strlen(buf);
            move(13, k);
            k--;
            prints(buf);
        }
        i0++;
    }
    for(i=0;i<24;i++)
        if(sTerm(lyear, i, day)) {
            strcpy(buf, solarTerm[i]);
            k-=strlen(buf);
            move(13, k);
            k--;
            prints(buf);
        }
    move(t_lines-1, scr_cols);
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
                    getdata(13, 48, "  确认删除该日日记[y/N]", title, 3, 1, 0, 1);
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
