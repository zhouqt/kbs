/*****
stiger:    2004.2

记录版面在线人数到日志文件，用于以后统计方便吧

crontab:  10 0 * * * /home/bbs/bin/post_brdlog

******/

#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "urlencode.c"

#ifdef NEWSMTH
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

int putout(const char *path){
	FILE *fp;
	int i;
	int totalonline = 0;
	int totalid = 0;

	if((fp=fopen(path,"w"))==NULL)
		return 0;

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

int fillbcache(void *fptr1,int idx,void* arg)
{
	char sql[256];
	FILE *fp;
    MYSQL_RES *res;
    MYSQL_ROW row;
    struct boardheader *fptr = (struct boardheader *)fptr1;
	if(fptr->filename[0]==0 || !normal_board(fptr->filename))
		return 0;

    if (fptr->flag & BOARD_GROUP)
		return 0;

	sprintf(sql, "SELECT MIN(nowid),MAX(nowid),AVG(users) FROM bonline WHERE bdate=\"%d-%d-%d\" AND bname LIKE '%s' ;", t.tm_year+1900, t.tm_mon+1, t.tm_mday, fptr->filename);
    if( mysql_real_query( &s, sql, strlen(sql) ))
        printf("%s\n",mysql_error(&s));
	else{
	    res = mysql_store_result(&s);
		if(res == NULL){
	        printf("%s\n",sql);
			return 0;
		}
	    row = mysql_fetch_row(res);

		if(row && row[0] && row[0][0]){
			x[n].yesid=atoi(row[0]);
			x[n].nowid=atoi(row[1]);
			x[n].online=atoi(row[2]);
		}else
			return 0;
		mysql_free_result(res);
	}

	strcpy(x[n].filename, fptr->filename);
	strcpy(x[n].title, fptr->title+13);

	n++;

	sprintf(sql,"%s/boards/%s.all",BONLINE_LOGDIR, fptr->filename);
	if((fp=fopen(sql,"a"))!=NULL){
		fprintf(fp,"%d-%d-%d\t%d\t%d\n", t.tm_year+1900, t.tm_mon+1, t.tm_mday, x[n-1].online, x[n-1].nowid-x[n-1].yesid);
		fclose(fp);
	}else printf("error open:%s\n", sql);
    return 0;
}

int fillboard()
{
	bzero(x, MAXBOARD * sizeof(struct _brdlog));
    return apply_record(BOARDS, fillbcache, sizeof(struct boardheader), NULL, 0,false);
}

static int get_seccode_index(char prefix)
{
    int i;

    for (i = 0; i < SECNUM; i++) {
        if (strchr(seccode[i], prefix) != NULL)
            return i;
    }
    return -1;
}

void gen_board_rank_xml()
{
    int i;
    FILE *fp;
    char xmlfile[STRLEN];
	char xml_buf[256];
	char url_buf[256];
	const struct boardheader *bp;
	int sec_id;

    snprintf(xmlfile, sizeof(xmlfile), BBSHOME "/xml/board.xml");
    if ((fp = fopen(xmlfile, "w")) == NULL)
        return;
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
    fprintf(fp, "<BoardList Desc=\"%s\">\n",encode_url(url_buf,"讨论区使用状况统计",sizeof(url_buf)));
    for (i = 0; i < n; i++) {
		bp = getbcache(x[i].filename);
		if (bp == NULL || (bp->flag & BOARD_GROUP))
			continue;
		if ((sec_id = get_seccode_index(bp->title[0])) < 0)
			continue;
        fprintf(fp, "<Board>\n");
        fprintf(fp, "<EnglishName>%s</EnglishName>\n", 
				encode_url(url_buf,encode_xml(xml_buf, x[i].filename, sizeof(xml_buf)),sizeof(url_buf)));
        fprintf(fp, "<ChineseName>%s</ChineseName>\n", 
				encode_url(url_buf,encode_xml(xml_buf, x[i].title, sizeof(xml_buf)),sizeof(url_buf)));
        fprintf(fp, "<Online>%d</Online>\n", x[i].online);
        fprintf(fp, "<Articles>%d</Articles>\n", x[i].nowid-x[i].yesid);
        fprintf(fp, "<SecId>%d</SecId>\n", sec_id);
        fprintf(fp, "</Board>\n");
    }
    fprintf(fp, "</BoardList>\n");
    fclose(fp);
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
	bzero(&x[0], sizeof(x));

    if (init_all()) {
        printf("init data fail\n");
        return -1;
    }

	mysql_init(&s);

    if (! my_connect_mysql(&s) ){
        printf("Mysql connect error: %s\n",mysql_error(&s));
        return 0;
    }

	now=time(0) - 86400 - 86400*before;
	localtime_r( &now, &t);

    fillboard();

	mysql_close(&s);

    qsort(x, n, sizeof(x[0]), online_cmp);
    /* generate boards usage result in xml format */
    gen_board_rank_xml();

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
int main(void){
	fprintf(stderr, "This program is designed for NEWSMTH only. Enable at your own risk.\n");
	return -1;
}
#endif
