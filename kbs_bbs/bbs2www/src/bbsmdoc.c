#include "bbslib.h"

int main()
{
	FILE *fp;
	char board[80], dir[80], *ptr;
	bcache_t *x1;
	struct fileheader x;
	int i, start, total;

 	init_all();
	strsncpy(board, getparm("board"), 32);
	x1=getbcache(board);
	if(x1==0)
		http_fatal("错误的讨论区");
	strcpy(board, x1->filename);
	if(!has_read_perm(currentuser, board))
		http_fatal("错误的讨论区");
	if(!has_BM_perm(currentuser, board))
		http_fatal("您没有权限访问本页");
	sprintf(dir, "boards/%s/.DIR", board);
	fp=fopen(dir, "r");
	if(fp==0)
		http_fatal("错误的讨论区目录");
	total=file_size(dir)/sizeof(struct fileheader);
	start=atoi(getparm("start"));
	if(strlen(getparm("start"))==0 || start>total-20)
		start=total-20;
  	if(start<0)
		start=0;
	printf("<nobr><center>\n");
	printf("%s -- [讨论区: %s] 版主[%s] 文章数[%d]<hr color=\"green\">\n", 
		BBSNAME, board, userid_str(x1->BM), total);
	if(total<=0)
		http_fatal("本讨论区目前没有文章");
	printf("<form name=\"form1\" method=\"post\" action=\"bbsman\">\n");
	printf("<table width=\"613\">\n");
	printf("<tr><td>序号</td><td>管理</td><td>状态</td><td>作者</td><td>日期</td><td>标题</td></tr>\n");
	fseek(fp, start*sizeof(struct fileheader), SEEK_SET);
	for(i=0; i<20; i++)
	{
		char filename[80];
		if(fread(&x, sizeof(x), 1, fp)<=0)
			break;
		sprintf(filename, "boards/%s/%s", board, x.filename);
		printf("<tr><td>%d</td>", start+i+1);
		printf("<td><input style=\"height:18px\" name=\"box%s\" type=\"checkbox\"></td>",
			x.filename);
		printf("<td>%s</td><td>%s</td>",
				flag_str(x.accessed[0]), userid_str(x.owner));
		printf("<td>%12.12s</td>", wwwCTime(atoi(x.filename+2))+4);
		printf("<td><a href=\"bbscon?board=%s&file=%s&num=%d\">%s%36.36s </a></td></tr>",
		board, x.filename, start+i,
		strncmp(x.title, "Re: ", 4) ? "○ " : "",
		void1(nohtml(x.title)));
	}
	printf("</table>\n");
	printf("<input type=\"hidden\" name=\"mode\" value=\"\">\n");
	printf("<input type=\"hidden\" name=\"board\" value=\"%s\">\n", board);
	printf("<input type=\"button\" value=\"删除\" onclick=\"document.form1.mode.value=1; document.form1.submit();\">\n");
	printf("<input type=\"button\" value=\"加M\" onclick=\"document.form1.mode.value=2; document.form1.submit();\">\n");
	printf("<input type=\"button\" value=\"加G\" onclick=\"document.form1.mode.value=3; document.form1.submit();\">\n");
	printf("<input type=\"button\" value=\"不可Re\" onclick=\"document.form1.mode.value=4; document.form1.submit();\">\n");
	printf("<input type=\"button\" value=\"清除MG\" onclick=\"document.form1.mode.value=5; document.form1.submit();\">\n");
	printf("</form>\n");
	if(start>0) {
		printf("<a href=bbsmdoc?board=%s&start=%d>上一页</a> ",
			board, start<20 ? 0 : start-20);
	}
	if(start<total-20) {
		printf("<a href=bbsmdoc?board=%s&start=%d>下一页</a> ",
			board, start+20);
	}
	printf("<a href=bbsdoc?board=%s>一般模式</a> ", board);
	printf("<a href=bbsdenyall?board=%s>封人名单</a> ", board);
	printf("<a href=bbsmnote?board=%s>编辑进版画面</a> ", board);
	fclose(fp);
	printf("<form action=bbsmdoc?board=%s method=post>\n", board);
	printf("<input type=submit value=跳转到> 第 <input type=text name=start size=4> 篇");
	printf("</form>\n");
	http_quit();
}
