#include "bbs.h"

extern char BoardName[];
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

	enum {
		ST_USERID,
		ST_USERNAME,
		ST_REALNAME,
		ST_ADDRESS,
		ST_EMAIL,
		ST_REALEMAIL,
		ST_IDENT,
		ST_RGTDAY,
		ST_NUMLOGINS,
		ST_NUMPOSTS,
		ST_LASTTIME,
		ST_LASTHOST,
		ST_THISTIME,
		ST_BOARDNAME,
		ST_STAY,
		ST_ALLTIME,
#ifdef _DETAIL_UEXP_
		ST_TIN,
		ST_EXP,
		ST_CEXP,
		ST_PERF,
		ST_CCPERF,
#endif		
		ST_END
	};
    char numlogins[10],numposts[10],rgtday[35];
	char lasttime[35],thistime[35],stay[10],
    char alltime[20];
    
#ifdef _DETAIL_UEXP_
    char tin[10],exper[10],ccperf[20],perf[10],exp[10],ccexp[20];
#endif
	char* stuffstr[ST_END];

    char    buf2[STRLEN],*ptr,*ptr2;
    time_t  now;

    static char* loglst[] =
        {
            "userid",
            "username",
            "realname",
            "address", 
            "email",   
            "realemail",
            "ident",
            "rgtday",
            "log",   
            "pst",   
            "lastlogin",
            "lasthost",
            "now",   
            "bbsname",
            "stay",    
            "alltime",  
#ifdef _DETAIL_UEXP_
            "tin",     
            "exp",    
            "cexp",   
            "perf",    
            "cperf",   
#endif
            NULL,           NULL,
        };

    stuffstr[ST_USERID]=currentuser->userid;
	stuffstr[ST_USERNAME]=currentuser->username;
	stuffstr[ST_REALNAME]=currentuser->realname;
	stuffstr[ST_ADDRESS]=currentuser->address;
	stuffstr[ST_EMAIL]=currentuser->email;
	stuffstr[ST_REALEMAIL]=currentuser->realemail;
	stuffstr[ST_IDENT]=currentuser->ident;
	stuffstr[ST_RGTDAY]=rgtday;
	stuffstr[ST_NUMLOGINS]=numlogins;
	stuffstr[ST_NUMPOSTS]=numposts;
	stuffstr[ST_LASTTIME]=lasttime;
	stuffstr[ST_LASTHOST]=currentuser->lasthost;
	stuffstr[ST_THISTIME]=thistime;
	stuffstr[ST_BOARDNAME]=BoardName;
	stuffstr[ST_STAY]=stay;
	stuffstr[ST_ALLTIME]=alltime;
#ifdef _DETAIL_UEXP_
	stuffstr[ST_TIN]=tin;
	stuffstr[ST_EXP]=exp;
	stuffstr[ST_CEXP]=ccexp;
	stuffstr[ST_PERF]=perf;
	stuffstr[ST_CCPERF]=ccperf;
#endif		

    now=time(0);
    /*---	modified by period	hide posts/logins	2000-11-02	---*/
#ifdef _DETAIL_UEXP_
    tmpnum=countexp(currentuser);
    sprintf(exp,"%d",tmpnum);
    strcpy(ccexp,cexp(tmpnum));
    tmpnum=countperf(currentuser);
    sprintf(perf,"%d",tmpnum);
    strcpy(ccperf,cperf(tmpnum));
    sprintf(tin, "%d", post_in_tin(currentuser->userid));
#endif
    sprintf(alltime,"%d小时%d分钟",currentuser->stay/3600,(currentuser->stay/60)%60);
    sprintf(rgtday, "%24.24s",ctime(currentuser->firstlogin));
    sprintf(lasttime, "%24.24s",ctime(currentuser->lastlogin));
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
            outs(ptr2);
            ptr ++;
            for (i = 0; i<ST_END; i++)
            {
            	strlength=strlen(loglst[i]);
                if(!strncmp(ptr, loglst[i],strlength)
                {
                	/*
                    ptr2 = ptr+strlength;
                    for(cnt=0; *(ptr2+cnt) == ' '; cnt++);
                    sprintf(buf2,"%-*.*s", cnt?strlength+cnt:strlength+1, strlength+cnt,stuffstr[i]);
                    outs(buf2);
                    ptr2 += (cnt?(cnt-1):cnt);
                    */
                    outs(stuffstr[i]);
                	ptr2 = ptr+strlength;
                    matchfrg=1;
                    break;
                }
            }
            if(!matchfrg)
            {
                outc('$');
                ptr2 = ptr;
            }
        }
        else
        {
            outs(ptr2);
            frg = 0;
        }
    }
    while(frg);
    return;
}
