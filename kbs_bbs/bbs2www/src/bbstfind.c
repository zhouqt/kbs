#include "bbslib.h"

int main() {
	FILE *fp;
	char buf[1024], title[80], board[80], dir[80], first_file[80];
	bcache_t *x1;
	struct fileheader x, x0;
	int sum=0, total=0;

 	init_all();
	strsncpy(board, getparm("board"), 32);
	strsncpy(title, getparm("title"), 42);
	x1=getbcache(board);
	if(x1==0) http_fatal("错误的讨论区");
	strcpy(board, x1->filename);
	if(!has_read_perm(currentuser, board)) http_fatal("错误的讨论区");
	sprintf(buf, "bbsman?board=%s&mode=1", board);
	sprintf(dir, "boards/%s/.DIR", board);
        fp=fopen(dir, "r");
        if(fp==0) http_fatal("错误的讨论区目录");
	printf("<center>%s -- 同主题查找 [讨论区: %s] [主题 '%s']<hr color=green>\n", 
		BBSNAME, board, nohtml(title));
	printf("<table border=1><tr><td>编号<td>作者<td>日期<td>标题\n");
	while(1) {
		if(fread(&x, sizeof(x), 1, fp)==0) break;
		sum++;
		if(!strncmp(title, x.title, 40) || 
		  (!strncmp(title, x.title+4, 40) && !strncmp(x.title, "Re: ", 4))) {
			if(total==0) strcpy(first_file, x.filename);
			printf("<tr><td>%d", sum);
			printf("<td>%s", userid_str(x.owner));
			if(!(x.accessed[0]&(FILE_MARKED|FILE_DIGEST))) {
				char buf2[20];
				sprintf(buf2, "&box%s=on", x.filename);
				if(strlen(buf)<500) strcat(buf, buf2);
			}
			printf("<td>%6.6s", wwwCTime(atoi(x.filename+2))+4);
			printf("<td><a href=bbscon?board=%s&file=%s&num=%d>%s</a>\n", 
				board, x.filename, sum-1, void1(x.title));
			total++;
		}
	}
	fclose(fp);
	printf("</table><hr>\n共找到 %d 篇 \n", total);
	printf("<a href=bbsdoc?board=%s>本讨论区</a> ", board);
	if(total>0) {
		printf("<a href=bbstcon?board=%s&file=%s>本主题全部展开</a> ", board, first_file);
		if(has_BM_perm(currentuser, board)) 
			printf("<a onclick='return confirm(\"确定同主题全部删除?\")' href=%s>同主题删除</a>", buf);
	}
	http_quit();
}
