#include "bbs.h"

extern char BoardName[];
typedef struct
{
    char    *match;
    char    *replace;
}
logout;

int
countlogouts(filename)
char filename[STRLEN];
{
    FILE    *fp;
    char    buf[256];
    int count=0;

    if((fp = fopen(filename, "r")) == NULL)
        return 0;

    while(fgets(buf, 255, fp) != NULL)
    {
        if(strstr(buf,"@logout@")||strstr(buf,"@issue@"))
            count++;
    }
    return count+1;
}


user_display(filename,number,mode)
char    *filename;
int number,mode;
{
    FILE    *fp;
    char    buf[256],buf2[STRLEN];
    int     count=1;

    clear();
    move(1,0);
    if((fp = fopen(filename, "r")) == NULL)
        return;

    while(fgets(buf, 255, fp) != NULL)
    {
        if(strstr(buf,"@logout@")||strstr(buf,"@issue@"))
        {
            count++;
            continue;
        }
        if(count==number)
        {
            if(mode==YEA)
                showstuff(buf);
            else
            {
                prints("%s",buf);
            }
        }
        else if(count>number)
            break;
        else
            continue;
    }
    refresh();
    fclose(fp);
    return;
}

/*
char *
cexp(exp)
int exp;
{
        int expbase=0;

        if(exp==-9999)
                return "没等级";
        if(exp<=100+expbase)
                return "新手上路";
        if(exp>100+expbase&&exp<=450+expbase)
                return "一般站友";
        if(exp>450+expbase&&exp<=850+expbase)
                return "中级站友";
        if(exp>850+expbase&&exp<=1500+expbase)
                return "高级站友";
        if(exp>1500+expbase&&exp<=2500+expbase)
                return "老站友";
        if(exp>2500+expbase&&exp<=3000+expbase)
                return "长老级";
        if(exp>3000+expbase&&exp<=5000+expbase)
                return "本站元老";
        if(exp>5000+expbase)
                return "开国大老";
        
}

char *
cperf(perf)
int perf;
{        
        
        if(perf==-9999)
                return "没等级";
        if(perf<=5)
                return "赶快加油";
        if(perf>5&&perf<=12)
                return "努力中";
        if(perf>12&&perf<=35)
                return "还不错";
        if(perf>35&&perf<=50)
                return "很好";
        if(perf>50&&perf<=90)
                return "优等生";
        if(perf>90&&perf<=140)
                return "太优秀了";
        if(perf>140&&perf<=200)
                return "本站支柱";
        if(perf>200)
                return "神～～";

}
*/
int
countexp(udata)
struct userec *udata;
{
    int exp;

    if(!strcmp(udata->userid,"guest"))
        return -9999;
    exp=udata->numposts+post_in_tin( udata->userid )+udata->numlogins/5+(time(0)-udata->firstlogin)/86400+udata->stay/3600;
    return exp>0?exp:0;
}

int
countperf(udata)
struct userec *udata;
{
    int perf;
    int reg_days;

    if(!strcmp(udata->userid,"guest"))
        return -9999;
    reg_days=(time(0)-udata->firstlogin)/86400+1;
    perf=((float)(udata->numposts+post_in_tin( udata->userid ))/(float)udata->numlogins+
          (float)udata->numlogins/(float)reg_days)*10;
    return perf>0?perf:0;
}

showstuff(buf)
char    buf[256];
{
    extern time_t   login_start_time;
    int     frg,
    i,
    matchfrg,
    strlength,
    cnt,
    tmpnum,
    dble,
    count;
    static  char
    numlogins[10],
    numposts[10],
    rgtday[35],
    lasttime[35],
    thistime[35],
    stay[10],
    alltime[20]
#ifdef _DETAIL_UEXP_
    ,
    tin[10],
    exper[10],
    ccperf[20],
    perf[10],
    exp[10],
    ccexp[20]
#endif
    ;

    char    buf2[STRLEN],
    *ptr,
    *ptr2;
    time_t  now;

    static logout loglst[] =
        {
            "userid",       currentuser->userid,
            "username",     currentuser->username,
            "realname",     currentuser->realname,
            "address",      currentuser->address,
            "email",        currentuser->email,
            "realemail",    currentuser->realemail,
            "ident",        currentuser->ident,
            "rgtday",       rgtday,
            "log",          numlogins,
            "pst",          numposts,
            "lastlogin",    lasttime,
            "lasthost",     currentuser->lasthost,
            "now",          thistime,
            "bbsname",      BoardName,
            "stay",         stay,
            "alltime",      alltime,
#ifdef _DETAIL_UEXP_
            "tin",          tin,
            "exp",          exp,
            "cexp",         ccexp,
            "perf",         perf,
            "cperf",        ccperf,
#endif
            NULL,           NULL,
        };
    now=time(0);
    /*---	modified by period	hide posts/logins	2000-11-02	---*/
#ifdef _DETAIL_UEXP_
    tmpnum=countexp(&currentuser);
    sprintf(exp,"%d",tmpnum);
    strcpy(ccexp,cexp(tmpnum));
    tmpnum=countperf(&currentuser);
    sprintf(perf,"%d",tmpnum);
    strcpy(ccperf,cperf(tmpnum));
    sprintf(tin, "%d", post_in_tin(currentuser->userid));
#endif
    sprintf(alltime,"%d小时%d分钟",currentuser->stay/3600,(currentuser->stay/60)%60);
    sprintf(rgtday, "%24.24s",ctime(&currentuser->firstlogin));
    sprintf(lasttime, "%24.24s",ctime(&currentuser->lastlogin));
    sprintf(thistime,"%24.24s",ctime(&now));
    sprintf(stay,"%d",(time(0) - login_start_time) / 60);
    /*---	modified by period	hide posts/logins	2000-11-02	---*/
#ifndef _DETAIL_UINFO_
    if(uinfo.mode == RMAIL && (!HAS_PERM(PERM_ADMINMENU)) ) {
        strcpy(numlogins, "$log");
        strcpy(numposts,  "$pst");
    } else
#endif
    {
        sprintf(numlogins, "%d", currentuser->numlogins);
        sprintf(numposts, "%d", currentuser->numposts);
    }


    frg = 1;
    ptr2 = buf;
    do
    {
        if(ptr = strchr(ptr2, '$'))
        {
            matchfrg = 0;
            *ptr = '\0';
            prints("%s", ptr2);
            ptr += 1;
            for (i = 0; loglst[i].match != NULL; i++)
            {
                if(strstr(ptr, loglst[i].match) == ptr)
                {
                    strlength=strlen(loglst[i].match);
                    ptr2 = ptr+strlength;
                    for(cnt=0; *(ptr2+cnt) == ' '; cnt++);
                    sprintf(buf2,"%-*.*s", cnt?strlength+cnt:strlength+1, strlength+cnt,loglst[i].replace);
                    prints("%s",buf2);
                    ptr2 += (cnt?(cnt-1):cnt);
                    matchfrg=1;
                    break;
                }
            }
            if(!matchfrg)
            {
                prints("$");
                ptr2 = ptr;
            }
        }
        else
        {
            prints("%s", ptr2);
            frg = 0;
        }
    }
    while(frg);
    return;
}
