/*******************************************

stiger: 增加到 BONLINE_LOGDIR/year/mon/day_useronline 文件
给 gnuplot 用的
一般1小时运行10次吧，关系不是太大
每天第一次运行时会把昨天的 day_useronline 文件拷贝到
BBSHOME/dayonline
然后每天0:20会有专门的程序根据BBSHOME/dayonline生成dayonline.png

如果当天的BONLINE_LOGDIR/year/mon/day_useronlie文件不存在
则表示是今天第一次运行.

*******************************************/

#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "config.h"

FILE *fp;
int totalonline=0;
int wwwguestonline=0;
int wwwnotguestonline=0;
int telnetonline=0;
int telnetschool=0;
int wwwnotguestschool=0;
int wwwguestschool=0;

#define BONLINE_LOGDIR "/home/bbs/bonlinelog"

static int is_school(char *ip)
{
	if(! strncmp(ip, "219.224", 7) )
		return 1;
	if( !strncmp(ip, "166.111", 7))
		return 1;
	return 0;
}

int do_userlist(struct user_info *uentp, char *arg, int t)
{

	if( uentp->mode == WWW ){
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

}

main()
{
	char path[256];
	char path1[256];
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

	sprintf(path, "%s/%d/%d/%d_useronline", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour);

	if( stat(path, &st) < 0 ){
		/*表示今天第一次运行，需要把昨天的数据拷贝出去 */
		time_t yesterd = now - 86400;
		struct tm ty;

		localtime_r( &yesterd, &ty);
		sprintf(path1, "%s/%d/%d/%d_useronline", BONLINE_LOGDIR, ty.tm_year+1900, ty.tm_mon+1, ty.tm_mday, ty.tm_hour);
		f_cp( path1, BBSHOME"/dayonline", 0644);
	}

	if((fp=fopen(path, "a"))==NULL){
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

	/*格式: 时间 totalonline wwwguestonline wwwnotguestonline telnetonline wwwguestschool wwwnotguestschool telnetschool */
	fprintf(fp, "%d.%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", t.tm_hour, t.tm_min/6, totalonline, wwwguestonline, wwwnotguestonline, telnetonline, wwwguestschool, wwwnotguestschool, telnetschool);
    fclose(fp);
}
