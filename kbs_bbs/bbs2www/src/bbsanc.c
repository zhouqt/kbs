#include "bbslib.h"

int main() {
	FILE *fp;
	char *board, path[512], buf[512], ch, tmp[80], *ptr; 
	init_all();
	strsncpy(path, getparm("path"), 511);
	board=getbfroma(path);
	buf[0]=0;
	if(board[0]) sprintf(buf, "%s版", board);
	printf("<center>%s -- %s精华区文章阅读<hr>\n", BBSNAME, board);
	if(strstr(path, ".Search") || strstr(path, ".Names")|| strstr(path, "..")|| strstr(path, "SYSHome"))
		http_fatal("错误的文件名");
	sprintf(buf, "0Announce%s", path);
	printf("<table border=1 width=610>");
	printf("<tr><td><pre>");
	fp=fopen(buf, "r");
	if(fp==0) http_fatal("错误的文件名");
	while(1) {
		if(fgets(buf, 256, fp)==0) break;
		hhprintf("%s", void1(buf));
	}
	fclose(fp);
   	printf("</pre></table><hr>\n");
	printf("[<a href='javascript:history.go(-1)'>返回上一页</a>] ");
   	if(board[0]) printf("[<a href=bbsdoc?board=%s>本讨论区</a>]", board);
   	printf("</center>\n"); 
	http_quit();
}
