/*****
stiger:    2004.2

记录版面在线人数到日志文件，用于以后统计方便吧

crontab:  2 * * * * /home/bbs/bin/bonlinelog

******/


#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "config.h"

struct _brdlog
{
	char filename[STRLEN];
	char title[STRLEN];
	int yesid;
	int nowid;
	int online;
} x[MAXBOARD];

int n = 0;

int putout(char *path)
{
	FILE *fp;
	int i;

	if((fp=fopen(path,"w"))==NULL)
		return;

	fprintf(fp,"%-15.15s %-30.30s %4s %6s\n","英文版面名", "中文","平均在线","文章数");
	for(i=0;i<n;i++){
		fprintf(fp,"%-15.15s %-30.30s %4d %6d\n",x[i].filename,x[i].title,x[i].online/24,x[i].nowid-x[i].yesid);
	}
	fclose(fp);
	return 1;
}

int online_cmp(const struct _brdlog *b, const struct _brdlog *a)
{
    return (a->online - b->online);
}

int id_cmp(struct _brdlog *b, struct _brdlog *a)
{
    return ( (a->nowid - a->yesid) - (b->nowid - b->yesid) );
}


int load_data(int day, char *path)
{
	FILE *fp;
	char buf[256];
	char board[STRLEN];
	char title[STRLEN];
	int online;
	int id;
	int i;

	if((fp=fopen(path,"r"))==NULL)
		return;

	while(fgets(buf,255,fp)){
		if( sscanf(buf,"%s %s %d %d",board, title, &online, &id) != 4)
			continue;
		for(i=0;i<n;i++){
			if(!strcmp(x[i].filename, board))
				break;
		}
		if(i==n)
			continue;
		x[i].online += online;
		if(day==0)
			x[i].nowid = id;
		if(day==23)
			x[i].yesid = id;
	}
	fclose(fp);
}

int fillbcache(struct boardheader *fptr,int idx,void* arg)
{

    struct boardheader bp;
	int bnum;
    struct BoardStatus * bs;
    struct userec normaluser;

	if(fptr->filename[0]==0 || !normal_board(fptr->filename))
		return;

	strcpy(x[n].filename, fptr->filename);
	strcpy(x[n].title, fptr->title+13);
	x[n].yesid=0;
	x[n].nowid=0;
	x[n].online=0;
	n++;

    return 0;
}

int fillboard()
{
    apply_record(BOARDS, (APPLY_FUNC_ARG)fillbcache, sizeof(struct boardheader), NULL, 0,false);
}

#define BONLINE_LOGDIR "/home/bbs/bonlinelog"

main()
{
	char path[256];
	char title[256];
	struct stat st;
	time_t now;
	struct tm t;
	int i;

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

	if( stat( path, &st) < 0)
		exit(0);

    resolve_boards();
    fillboard();

	for(now=time(0),i=0;i<24;i++,now-=3600){

		localtime_r( &now, &t);
		sprintf(path, "%s/%d/%d/%d_%d.log", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour);

		load_data(i,path);
	}

    qsort(x, n, sizeof(x[0]), online_cmp);

	sprintf(path,"tmp/brdlog.%d.out",getpid());
	putout(path);

	now=time(0);
	localtime_r( &now, &t);

    sprintf(title, "%d年%2d月%2d日版面统计数据(在线排序)", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    post_file(NULL, "", path, "SysTrace", title, 0, 1);
	unlink(path);

    qsort(x, n, sizeof(x[0]), id_cmp);

	sprintf(path,"tmp/brdlog.%d.out",getpid());
	putout(path);

	now=time(0);
	localtime_r( &now, &t);

    sprintf(title, "%d年%2d月%2d日版面统计数据(文章数排序)", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    post_file(NULL, "", path, "SysTrace", title, 0, 1);
	unlink(path);

}
