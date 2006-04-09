#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "urlencode.c"

struct binfo {
    char boardname[20];
    char expname[50];
    int times;
    int sum;
} st[MAXBOARD];

int numboards = 0;

int brd_cmp(b, a)
    struct binfo *a, *b;
{
    if (a->times != b->times)
        return (a->times - b->times);
    return a->sum - b->sum;
}

int total_cmp(b, a)
    struct binfo *a, *b;
{
    if (a->sum != b->sum)
        return (a->sum - b->sum);
    return a->times - b->times;
}

int average_cmp(b, a)
    struct binfo *a, *b;
{
    int a_ave, b_ave;

    if (a->times)
        a_ave = a->sum / a->times;
    else
        a_ave = 0;
    if (b->times)
        b_ave = b->sum / b->times;
    else
        b_ave = 0;

    if (a_ave != b_ave)
        return (a_ave - b_ave);
    return a->sum - b->sum;
}

int record_data(const char *board,int sec){
    int i;

    for (i = 0; i < numboards; i++) {
        if (!strcmp(st[i].boardname, board)) {
            st[i].times++;
            st[i].sum += sec;
            return 1;
        }
    }
    return 0;
}

int add_data(const struct binfo *btmp){
    int i;

    for (i = 0; i < numboards; i++) {
        if (!strcmp(st[i].boardname, btmp->boardname)) {
            st[i].times += btmp->times;
            st[i].sum += btmp->sum;
            return 1;
        }
    }
    return 0;
}

int fillbcache(const struct boardheader *fptr,int idx,void* arg)
{
    if (numboards >= MAXBOARD)
        return 0;
    if(!check_see_perm(NULL,fptr)||!*(fptr->filename))
        return 0;
    if (fptr->flag & BOARD_GROUP)
        return 0;
    strcpy(st[numboards].boardname, fptr->filename);
    strcpy(st[numboards].expname, fptr->title + 13);
    st[numboards].times = 0;
    st[numboards].sum = 0;
    numboards++;
    return 0;
}

int fillboard(void){
    return apply_record(BOARDS, (APPLY_FUNC_ARG)fillbcache, sizeof(struct boardheader), NULL, 0,false);
}

char *timetostr(i)
    int i;
{
    static char str[30];
    int minute, sec, hour;

    minute = (i / 60);
    hour = minute / 60;
    minute = minute % 60;
    sec = i & 60;
    sprintf(str, "%2d:%2d:%2d", hour, minute, sec);
    return str;
}

int gen_usage(char *buf, char *buf1, char *buf2, char *buf3)
{
    FILE *op, *op1, *op2, *op3;
    int c[3];
    int max[3];
    unsigned int ave[3];
    int i, j, k;
    char *blk[10] = {
        /* 方框太难看了 modified by Czz */
//      "  ","  ", "  ", "  ", "  ",
        "  ", "▏", "▎", "▍", "▌",
//      "□","□", "□", "□", "□",
        "▋", "▊", "▉", "█", "█",
        /* modified end */
    };

	/*注:等待改*/
    if ((op = fopen(buf, "w")) == NULL || (op1 = fopen(buf1, "w")) == NULL || (op2 = fopen(buf2, "w")) == NULL || (op3=fopen(buf3, "w")) == NULL ) {
            printf("Can't Write file\n");
            return 1;
    }

    qsort(st, numboards, sizeof(st[0]), brd_cmp);

    printf("%d", numboards);
    ave[0] = 0;
    ave[1] = 0;
    ave[2] = 0;
    max[1] = 0;
    max[0] = 0;
    max[2] = 0;
    for (i = 0; i < numboards; i++) {
        ave[0] += st[i].times;
        ave[1] += st[i].sum;
        ave[2] += st[i].times == 0 ? 0 : st[i].sum / st[i].times;
        if (max[0] < st[i].times) {
            max[0] = st[i].times;
        }
        if (max[1] < st[i].sum) {
            max[1] = st[i].sum;
        }
        if (max[2] < (st[i].times == 0 ? 0 : st[i].sum / st[i].times)) {
            max[2] = (st[i].times == 0 ? 0 : st[i].sum / st[i].times);
        }
    }
    c[0] = max[0] / 30 + 1;
    c[1] = max[1] / 30 + 1;
    c[2] = max[2] / 30 + 1;
    numboards++;
    st[numboards - 1].times = ave[0] / numboards;
    st[numboards - 1].sum = ave[1] / numboards;
    strcpy(st[numboards - 1].boardname, "Average");
    strcpy(st[numboards - 1].expname, "总平均");

    fprintf(op, "名次 %-15.15s%-25.25s %5s %8s %10s\n", "讨论区名称", "中文叙述", "人次", "累积时间", "平均时间");
    fprintf(op3, "      \033[37m1 \033[m\033[34m%2s\033[37m= %d (总人次) \033[37m1 \033[m\033[32m%2s\033[37m= %s (累积总时数) \033[37m1 \033[m\033[31m%2s\033[37m= %d 秒(平均时数)\n\n",
                blk[9], c[0], blk[9], timetostr(c[1]), blk[9], c[2]);

    for (i = 0; i < numboards; i++) {

            /* generate 0Announce/bbslists/board2 file */
            fprintf(op, "%4d\033[m %-15.15s%-25.25s %5d %-.8s %10d\n", i + 1, st[i].boardname, st[i].expname, st[i].times, timetostr(st[i].sum), st[i].times == 0 ? 0 : st[i].sum / st[i].times);

			/* 声成 board1, 图表 */
            fprintf(op3, "      \033[37m第\033[31m%3d \033[37m名 讨论区名称：\033[31m%s \033[35m%s\033[m\n", i + 1, st[i].boardname, st[i].expname);
            fprintf(op3, "\033[37m    ┌————————————————————————————————————\n");
            fprintf(op3, "\033[37m人次│\033[m\033[34m");
            for (j = 0; j < st[i].times / c[0]; j++) {
                fprintf(op3, "%2s", blk[9]);
            }
            fprintf(op3, "%2s \033[37m%d\033[m\n", blk[(st[i].times % c[0]) * 10 / c[0]], st[i].times);
            fprintf(op3, "\033[1;37m时间│\033[m\033[32m");
            for (j = 0; j < st[i].sum / c[1]; j++) {
                fprintf(op3, "%2s", blk[9]);
            }
            fprintf(op3, "%2s \033[37m%s\033[m\n", blk[(st[i].sum % c[1]) * 10 / c[1]], timetostr(st[i].sum));
            j = st[i].times == 0 ? 0 : st[i].sum / st[i].times;
            fprintf(op3, "\033[37m平均│\033[m\033[31m");
            for (k = 0; k < j / c[2]; k++) {
                fprintf(op3, "%2s", blk[9]);
            }
            fprintf(op3, "%2s \033[37m%s\033[m\n", blk[(j % c[2]) * 10 / c[2]], timetostr(j));
            fprintf(op3, "\033[37m    └————————————————————————————————————\033[m\n\n");
    }
    fclose(op);
    fclose(op3);

	/*生成 总时间排序的 */
        qsort(st, numboards - 1, sizeof(st[0]), total_cmp);
        fprintf(op1, "名次 %-15.15s%-25.25s %8s %5s %10s\n", "讨论区名称", "中文叙述", "累积时间", "人次", "平均时间");
        for (i = 0; i < numboards; i++)
            fprintf(op1, "%4d %-15.15s%-25.25s %-.8s %5d %10d\n", i + 1, st[i].boardname, st[i].expname, timetostr(st[i].sum), st[i].times, st[i].times == 0 ? 0 : st[i].sum / st[i].times);
        fclose(op1);

	/* 生成 平均时间排序的 */
        qsort(st, numboards - 1, sizeof(st[0]), average_cmp);
        fprintf(op2, "名次 %-15.15s%-25.25s %10s %5s %8s\n", "讨论区名称", "中文叙述", "平均时间", "累积时间", "人次");
        for (i = 0; i < numboards; i++)
            fprintf(op2, "%4d %-15.15s%-25.25s %10d %-.8s %5d\n", i + 1, st[i].boardname, st[i].expname, st[i].times == 0 ? 0 : st[i].sum / st[i].times, timetostr(st[i].sum), st[i].times);
        fclose(op2);

		numboards --;
    return 0;
}

int main(void){
	char path[256];
	FILE *fp;
	char buf[256], buf1[256],buf2[256], buf3[256], buf4[256];
	struct stat stt;
	time_t now;
	struct tm t;
	int sec;
	int fd;
	int i;
    char *p, bname[20], *q;
	char weeklogfile[256];

	now = time(0);
	localtime_r( &now, &t);

    chdir(BBSHOME);

	if( stat( BONLINE_LOGDIR, &stt) < 0 ){
		if(mkdir(BONLINE_LOGDIR, 0755) < 0)
			exit(0);
	}
	sprintf(path, "%s/%d", BONLINE_LOGDIR, t.tm_year+1900);
	if( stat(path, &stt) < 0){
		if(mkdir(path, 0755) < 0)
			exit(0);
	}
	sprintf(path, "%s/%d/%d", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1);
	if( stat(path, &stt) < 0){
		if(mkdir(path, 0755) < 0)
			exit(0);
	}

	sprintf(buf4, "%s/%d/%d/%d_boarduse.visit", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday);
	sprintf(buf1, "%s/%d/%d/%d_boarduse.total", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday);
	sprintf(buf2, "%s/%d/%d/%d_boarduse.average", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday);
	sprintf(buf3, "%s/%d/%d/%d_boarduse.visittable", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday);

    strcpy(weeklogfile, BBSHOME "/boardusage.week");

	/*生成今日数据*/
	system("killall -USR2 bbslogd");
	/* bbslogd完成比较慢，休息一会再去处理 */
	sleep(100);
    if ((fp = fopen(BBSHOME "/boardusage.log.0", "r")) == NULL) {
        printf("cann't open boardusage.log.0\n");
        return 1;
    }

    resolve_boards();
    fillboard();

	/*加上今日数据*/
    while (fgets(buf, 256, fp)) {
        if (strlen(buf) < 57)
            continue;
        if((p=strstr(buf,"Stay: "))!=NULL){
            q = p - 21;
            q = strtok(q, " ");
            strcpy(bname, q);
            sec = atoi(p + 6);
            record_data(bname, sec);
        }
    }
    fclose(fp);

	/*统计今日数据*/
	gen_usage(buf4,buf1,buf2,buf3);

    strcpy(buf4, BBSHOME "/0Announce/bbslists/board2");
    strcpy(buf1, BBSHOME "/0Announce/bbslists/totaltime");
    strcpy(buf2, BBSHOME "/0Announce/bbslists/averagetime");
    strcpy(buf3, BBSHOME "/0Announce/bbslists/board1");
	
	/*加上这周数据*/
	if((fd=open(weeklogfile,O_RDONLY)) >=0 ){
		struct binfo stmp;
		while( read(fd, &stmp,sizeof(stmp)) >= sizeof(stmp) ){
					add_data( &stmp );
				}
				close(fd);
			}

			/*统计这周数据*/
			gen_usage(buf4,buf1,buf2,buf3);

			/*写入这周数据*/
			if((fd=open(weeklogfile, O_WRONLY | O_CREAT ,0644 )) >= 0){
				for(i=0; i<numboards; i++)
					write(fd, &(st[i]), sizeof(struct binfo));
		close(fd);
	}

	numboards++;

	/*每周四计算完后，清楚这周记录，并备份boardusage.week */
	sprintf(buf, "%s/%d/%d/%d_%d.boardusage.week.bak", BONLINE_LOGDIR, t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour);
	if( t.tm_wday == 3 )
		f_mv(weeklogfile, buf);
    return 0;
}
