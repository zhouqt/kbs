/*****
stiger:    2004.2

记录版面在线人数到日志文件，用于以后统计方便吧

crontab:  2 * * * * /home/bbs/bin/bonlinelog

******/


#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "config.h"


FILE *fp;

int fillbcache(struct boardheader *fptr,int idx,void* arg)
{

    struct boardheader bp;
	int bnum;
    struct BoardStatus * bs;
    struct userec normaluser;

    if (check_see_perm(NULL, fptr)==0|| strlen(fptr->filename) == 0)
        return;

    bnum = getboardnum(fptr->filename,&bp);
    bs = getbstatus(bnum);

	fprintf(fp, "%-15.15s %-40.40s %5d\n", fptr->filename, fptr->title+13, bs->currentusers);

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

	sprintf(path, "%s/%d/%d/%d_%d.log", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour);
	if((fp=fopen(path, "w"))==NULL){
		printf("cannot open log file\n");
		exit(0);
	}

    resolve_boards();
    fillboard();

    fclose(fp);
}
