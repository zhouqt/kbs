/*****
stiger:    2004.2

记录版面在线人数到日志文件，用于以后统计方便吧

crontab:  10 0 * * * /home/bbs/bin/post_brdlog

******/

#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "config.h"

#ifdef HAVE_MYSQL_SMTH
struct _brdlog
{
	char filename[STRLEN];
	char title[STRLEN];
	int yesid;
	int nowid;
	int online;
} x[MAXBOARD];

int n = 0;
MYSQL s;
struct tm t;

int putout(char *path)
{
	FILE *fp;
	int i;
	int totalonline = 0;
	int totalid = 0;

	if((fp=fopen(path,"w"))==NULL)
		return;

	fprintf(fp,"    %-15.15s %-30.30s %4s %6s\n","英文版面名", "中文","平均在线","文章数");
	for(i=0;i<n;i++){
		fprintf(fp,"%3d %-15.15s %-30.30s %4d %6d\n",i+1,x[i].filename,x[i].title,x[i].online,x[i].nowid-x[i].yesid);
		totalonline += x[i].online;
		totalid+=x[i].nowid-x[i].yesid;
	}
	fprintf(fp,"    %-15.15s %-30.30s %4d %6d\n","总计","",totalonline,totalid);
	fclose(fp);
	return 1;
}

int online_cmp(const void *b, const void *a)
{
    return (((struct _brdlog *)a)->online - ((struct _brdlog *)b)->online);
}

int id_cmp(const void *b, const void *a)
{
    return ( (((struct _brdlog *)a)->nowid - ((struct _brdlog *)a)->yesid)
           - (((struct _brdlog *)b)->nowid - ((struct _brdlog *)b)->yesid) );
}

int fillbcache(struct boardheader *fptr,int idx,void* arg)
{

    struct boardheader bp;
	int bnum;
    struct BoardStatus * bs;
    struct userec normaluser;
	char sql[256];
	FILE *fp;
    MYSQL_RES *res;
    MYSQL_ROW row;

	if(fptr->filename[0]==0 || !normal_board(fptr->filename))
		return;

    if (fptr->flag & BOARD_GROUP)
		return;

	strcpy(x[n].filename, fptr->filename);
	strcpy(x[n].title, fptr->title+13);

	sprintf(sql, "SELECT MIN(nowid),MAX(nowid),AVG(users) FROM bonline WHERE bdate=\"%d-%d-%d\" AND bname LIKE '%s' ;", t.tm_year+1900, t.tm_mon+1, t.tm_mday, fptr->filename);
    if( mysql_real_query( &s, sql, strlen(sql) ))
        printf("%s\n",mysql_error(&s));
	else{
	    res = mysql_store_result(&s);
	    row = mysql_fetch_row(res);

		if(row && row[0] && row[0][0]){
			x[n].yesid=atoi(row[0]);
			x[n].nowid=atoi(row[1]);
			x[n].online=atoi(row[2]);
		}
		mysql_free_result(res);
	}
	n++;

	sprintf(sql,"%s/boards/%s.all",BONLINE_LOGDIR, fptr->filename);
	if((fp=fopen(sql,"a"))!=NULL){
		fprintf(fp,"%d-%d-%d\t%d\n", t.tm_year+1900, t.tm_mon+1, t.tm_mday, x[n-1].online);
		fclose(fp);
	}
    return 0;
}

int fillboard()
{
	bzero(x, MAXBOARD * sizeof(struct _brdlog));
    apply_record(BOARDS, (APPLY_FUNC_ARG)fillbcache, sizeof(struct boardheader), NULL, 0,false);
}

int main(int argc,char **argv)
{
	char path[256];
	char title[256];
	time_t now;
	int before;

	if(argc > 1)
		before = atoi(argv[1]);
	else
		before = 0;

	chdir(BBSHOME);

    if (init_all()) {
        printf("init data fail\n");
        return -1;
    }

	mysql_init(&s);

    if (! my_connect_mysql(&s) ){
        printf("Mysql connect error: %s\n",mysql_error(&s));
        return 0;
    }

	now=time(0) - 3600 - 86400*before;
	localtime_r( &now, &t);

    fillboard();

	mysql_close(&s);

    qsort(x, n, sizeof(x[0]), online_cmp);

	sprintf(path,"tmp/brdlog.%d.out",getpid());
	putout(path);

    sprintf(title, "%d年%2d月%2d日版面统计数据(在线排序)", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    post_file(NULL, "", path, "SysTrace", title, 0, 1, getSession());
	unlink(path);

    qsort(x, n, sizeof(x[0]), id_cmp);

	sprintf(path,"tmp/brdlog.%d.out",getpid());
	putout(path);

    sprintf(title, "%d年%2d月%2d日版面统计数据(文章数排序)", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    post_file(NULL, "", path, "SysTrace", title, 0, 1, getSession());
	unlink(path);

	return 0;
}
#else
int main()
{
	fprintf(stderr, "MySQL support had been disabled.\n");
	return -1;
}
#endif
