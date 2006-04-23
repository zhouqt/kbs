/*******************************************

stiger: 增加今日平均在线到 BBSHOME/allonline 文件
给 gnuplot 用的,用于在线统计

*******************************************/

#include <time.h>
#include <stdio.h>
#include "bbs.h"

int num=0;
int t1=0;
int t2=0;
int t3=0;
int t4=0;
int t5=0;
int t6=0;
int t7=0;
int tt1=0;
int tt2=0;
int tt3=0;
int tt4=0;
int tt5=0;
int tt6=0;
int tt7=0;

int main(int argc, char **argv){
	char path[256];
	char buf[256];
	time_t now;
	struct tm t;
	float fb;
	FILE *fp;

	now = time(0);
	if(argc > 1){
		now -= atoi(argv[1]) * 86400;
	}

	localtime_r( &now, &t);

    if (init_all()) {
        printf("init data fail\n");
        return -1;
    }

    chdir(BBSHOME);

	sprintf(path, "%s/%d/%d/%d_useronline", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday);

	{
	char path1[256];
	int logincount=0;
	int wwwlogincount=0;
	int wwwguestlogincount=0;
	int a[10];
	char title[100];
	sprintf(path1, "%s/%d/%d/%d_login", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday);
	if((fp=fopen(path1, "r"))==NULL){
		printf("cannot open login1 file\n");
		goto out1;
	}
	while(fgets(buf, 256,fp)){
		if( sscanf(buf, "%f %d %d %d %d %d %d %d %d %d", &fb, a, a+1, a+2, a+3, a+4, a+5, a+6, a+7, a+8) != 10)
			continue;
		logincount += a[0];
		wwwlogincount += a[3];
		wwwguestlogincount += a[6];
	}
	fclose(fp);

	sprintf(path1, "tmp/%d.logintmp", (int)getpid());
	if((fp=fopen(path1, "w"))==NULL){
		printf("cannot open login2 file\n");
		goto out1;
	}
	sprintf(title, "%d年%d月%d日共登录%d次", t.tm_year+1900, t.tm_mon+1, t.tm_mday, logincount+wwwlogincount+wwwguestlogincount);
	fprintf(fp, "telnet:%d\nwww:%d\nwwwguest:%d\n---------\n TOTAL:%d\n",logincount,wwwlogincount,wwwguestlogincount, logincount+wwwlogincount+wwwguestlogincount);
	fclose(fp);

    post_file(NULL, "", path1, "SysTrace", title, 0, 1, getSession());
	unlink(path1);

	}

out1:
	if((fp=fopen(path, "r"))==NULL){
		printf("cannot open log file\n");
		exit(0);
	}
	while(fgets(buf, 256, fp)){
		if( sscanf(buf, "%f %d %d %d %d %d %d %d", &fb, &t1, &t2, &t3, &t4, &t5, &t6, &t7) != 8)
			continue;
		num++;
		tt1+=t1;
		tt2+=t2;
		tt3+=t3;
		tt4+=t4;
		tt5+=t5;
		tt6+=t6;
		tt7+=t7;
	}
	fclose(fp);

	if(num==0) exit(0);

	if((fp=fopen("allonline","a"))==NULL)
		exit(0);

	fprintf(fp, "%d_%d_%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", t.tm_year+1900, t.tm_mon+1, t.tm_mday, tt1/num, tt2/num, tt3/num, tt4/num, tt5/num, tt6/num, tt7/num);
    fclose(fp);
    return 0;
}
