#include "bbslib.h"

static int cmp_board(struct boardheader *b1, struct boardheader *b2)
{
	return strcasecmp(b1->filename, b2->filename);
}

int main()
{
	bcache_t data[MAXBOARD], *x;
	int i, total=0, sec1;
	char *cgi="bbstdoc", *ptr, *my_sec;
	bcache_t *bc;

	init_all();
	bc = getbcacheaddr();
	printf("<style type=\"text/css\">A {color: #0000f0}</style>");
	sec1=atoi(getsenv("QUERY_STRING"));
	if(sec1<0 || sec1>=SECNUM) http_fatal("错误的参数");
	if(atoi(getparm("my_def_mode"))!=0) cgi="bbsdoc";
	for(i=0; i<MAXBOARD; i++) {
		x=&(bc[i]);
		if(x->filename[0]<=32 || x->filename[0]>'z') continue;
		if(!has_read_perm(currentuser, x->filename)) continue;
		if(strchr(seccode[sec1], x->title[0]) == NULL) continue;
		memcpy(&data[total], x, sizeof(struct boardheader));
		total++;
	}
	qsort(data, total, sizeof(struct boardheader), cmp_board);
	printf("<center>\n");
	printf("%s -- 分类讨论区 [%s]<hr color=\"green\">", BBSNAME, secname[sec1]);
	printf("<table width=\"610\">\n");
	printf("<tr><td>序号</td><td>未</td><td>讨论区名称</td><td>更新时间</td><td>类别</td><td>中文描述</td><td>版主</td><td>文章数</td></tr>\n");
	for(i=0; i<total; i++) {
		char buf[100];
		sprintf(buf, "boards/%s/.DIR", data[i].filename);
		printf("<tr><td>%d</td><td>%s</td>",
			i+1, board_read(data[i].filename) ? "◇" : "◆");
		printf("<td><a href=\"%s?board=%s\">%s</a></td>", cgi, data[i].filename, data[i].filename);
		printf("<td>%12.12s</td>", 4+wwwCTime(file_time(buf)));
		printf("<td>%6.6s</td>", data[i].title+1);
		printf("<td><a href=\"%s?board=%s\">%s</a></td>", cgi, data[i].filename, data[i].title+7);
		ptr=strtok(data[i].BM, " ,;");
		if(ptr==0) ptr="诚征版主中";
		printf("<td><a href=\"bbsqry?userid=%s\">%s</a></td>", ptr, ptr);
		printf("<td>%d</td></tr>\n", filenum(data[i].filename));
	}
   	printf("</table><hr></center>\n");
	http_quit();
}

int filenum(char *board) {
	char file[256];
	sprintf(file, "boards/%s/.DIR", board);
	return file_size(file)/sizeof(struct fileheader);
}

int board_read(char *board) {
	char buf[256], path[256];
	FILE *fp;
	struct fileheader x;
	int total;
	if(!loginok) return 1;
	bzero(&x, sizeof(x));
	sprintf(buf, "boards/%s/.DIR", board);
	total=file_size(buf)/sizeof(struct fileheader);
	if(total<=0) return 1;
	fp=fopen(buf, "r+");
	fseek(fp, (total-1)*sizeof(struct fileheader), SEEK_SET);
	fread(&x, sizeof(x), 1, fp);
	fclose(fp);
	brc_initial(currentuser->userid, board);
	return brc_has_read(x.filename);
}
