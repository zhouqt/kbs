/*******************************************

stiger: 增加到 BONLINE_LOGDIR/year/mon/day_useronline 文件
给 gnuplot 用的
一般1小时运行10次吧，关系不是太大
然后每天快结束时会有专门的程序根据来生成dayonline.png
mkonlinepng.sh

lastcount文件可以用来snmp分析....
*******************************************/

#include <time.h>
#include <stdio.h>
#include "bbs.h"

FILE *fp;
FILE *fp_login;
FILE *fp_forcount;
int totalonline=0;
int wwwguestonline=0;
int wwwnotguestonline=0;
int telnetonline=0;
int telnetschool=0;
int wwwnotguestschool=0;
int wwwguestschool=0;

unsigned int logincount=0;
unsigned int logoutcount=0;
unsigned int wwwlogincount=0;
unsigned int wwwlogoutcount=0;
unsigned int wwwguestlogincount=0;
unsigned int wwwguestlogoutcount=0;
time_t staytime=0;
time_t wwwstaytime=0;
time_t wwwgueststaytime=0;

static int is_school(char *ip)
{
	if(! strncmp(ip, "59.66", 5) )
		return 1;
	if( !strncmp(ip, "166.111", 7))
		return 1;
	if( !strncmp(ip, "219.224", 7))
		return 1;
	if( !strncmp(ip, "211.151", 7))
		return 1;
	if( !strncmp(ip, "211.151.89.114", 14))
		return 1;
	if( !strncmp(ip, "220.194.55.56", 13))
		return 1;
	if( !strncmp(ip, "210.51.188.45", 13))
		return 1;
	if( !strncmp(ip, "202.205.10", 10))
		return 1;
	return 0;
}

int do_userlist(struct user_info *uentp, char *arg, int t)
{

	if( uentp->pid == 1){
		/* www */
		wwwnotguestonline++;
		if( is_school(uentp->from) )
			wwwnotguestschool++;
	}else{
		/* telnet */
		telnetonline ++;
		if( is_school(uentp->from) )
			telnetschool++;
	}

    return COUNT;
}

int show_wwwguest()
{
	int i;

    for (i = 0; i < MAX_WWW_GUEST; i++) {
        if (!(wwwguest_shm->use_map[i / 32] & (1 << (i % 32))) )
            continue;
	    if( is_school( inet_ntoa(wwwguest_shm->guest_entry[i].fromip)) )
			wwwguestschool++;
    }
    return 0;
}

int main(void){
	char path[256];
	struct stat st;
	time_t now;
	struct tm t;

	now = time(0);
	localtime_r( &now, &t);

    chdir(BBSHOME);
	if( stat( BONLINE_LOGDIR, &st) < 0 ){
		if(mkdir(BONLINE_LOGDIR, 0755) < 0)
			exit(0);
	}
	sprintf(path, "%s/%d", BONLINE_LOGDIR, t.tm_year+1900);
	if( stat(path, &st) < 0){
		if(mkdir(path, 0755) < 0)
			exit(0);
	}
	sprintf(path, "%s/%d/%d", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1);
	if( stat(path, &st) < 0){
		if(mkdir(path, 0755) < 0)
			exit(0);
	}

	sprintf(path,"%s/%d/%d/%d_useronline",BONLINE_LOGDIR,t.tm_year+1900,t.tm_mon+1,t.tm_mday);

	if((fp=fopen(path, "a"))==NULL){
		printf("cannot open log file\n");
		exit(0);
	}

	sprintf(path, "%s/%d/%d/%d_login", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday);

	if((fp_login=fopen(path, "a"))==NULL){
		printf("cannot open log file\n");
		exit(0);
	}

    sprintf(path, "%s/lastcount", BONLINE_LOGDIR);

    if((fp_forcount=fopen(path, "w+"))==NULL){
        printf("cannot open log file\n");
        exit(0);
    }

    resolve_utmp();
	get_publicshm();

	resolve_guest_table();

	wwwguestonline = getwwwguestcount();
	totalonline =  get_utmp_number() + wwwguestonline;

    apply_ulist_addr((APPLY_UTMP_FUNC)do_userlist, NULL);

	show_wwwguest();

	logincount = get_publicshm()->logincount;
	logoutcount = get_publicshm()->logoutcount;
	wwwlogincount = get_publicshm()->wwwlogincount;
	wwwlogoutcount = get_publicshm()->wwwlogoutcount;
	wwwguestlogincount = get_publicshm()->wwwguestlogincount;
	wwwguestlogoutcount = get_publicshm()->wwwguestlogoutcount;
	staytime = get_publicshm()->staytime;
	wwwstaytime = get_publicshm()->wwwstaytime;
	wwwgueststaytime = get_publicshm()->wwwgueststaytime;

    setpublicshmreadonly(0);
	get_publicshm()->logincount=0;
	get_publicshm()->logoutcount=0;
	get_publicshm()->wwwlogincount=0;
	get_publicshm()->wwwlogoutcount=0;
	get_publicshm()->wwwguestlogincount=0;
	get_publicshm()->wwwguestlogoutcount=0;
	get_publicshm()->staytime=0;
	get_publicshm()->wwwstaytime=0;
	get_publicshm()->wwwgueststaytime=0;
    setpublicshmreadonly(1);
	/*格式: 时间 totalonline wwwguestonline wwwnotguestonline telnetonline wwwguestschool wwwnotguestschool telnetschool */
	fprintf(fp, "%d.%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", t.tm_hour, t.tm_min/6, totalonline, wwwguestonline, wwwnotguestonline, telnetonline, wwwguestschool, wwwnotguestschool, telnetschool);
	fprintf(fp_login, "%d.%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", t.tm_hour, t.tm_min/6, logincount, logoutcount, (int)(logoutcount?(staytime/logoutcount)/10:0), wwwlogincount, wwwlogoutcount, wwwguestlogincount, wwwguestlogoutcount, (int)(wwwlogoutcount?(wwwstaytime/wwwlogoutcount)/10:0) , (int)(wwwguestlogoutcount?(wwwgueststaytime/wwwguestlogoutcount)/10:0));
	fprintf(fp_forcount, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n", totalonline, wwwguestonline, wwwnotguestonline, telnetonline, wwwguestschool, wwwnotguestschool, telnetschool);
    fclose(fp);
    fclose(fp_login);
    fclose(fp_forcount);
    return 0;
}
