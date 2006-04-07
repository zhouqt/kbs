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
	localtime_r( &now, &t);

    chdir(BBSHOME);

	sprintf(path, "%s/%d/%d/%d_useronline", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday);

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
