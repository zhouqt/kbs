#include "bbslib.h"

int main() {
	FILE *fp;
	int num=0, total=0, type, dt, mg=0, og=0;
	char dir[80], title[80], title2[80], title3[80], board[80], userid[80];
	bcache_t *brd;
	struct fileheader x;

	init_all();
	printf("<center>%s -- 版内文章搜索<hr color=green><br>\n", BBSNAME);
	type=atoi(getparm("type"));
	strsncpy(board, getparm("board"), 30);
	if(type==0) return show_form(board);
	strsncpy(title, getparm("title"), 60);
	strsncpy(title2, getparm("title2"), 60);
	strsncpy(title3, getparm("title3"), 60);
	strsncpy(userid, getparm("userid"), 60);
	dt=atoi(getparm("dt"));
	if(!strcasecmp(getparm("mg"), "on")) mg=1;
	if(!strcasecmp(getparm("og"), "on")) og=1;
	if(dt<0) dt=0;
	if(dt>9999) dt=9999;
	brd=getbcacheaddr(board);
	if(brd==0) http_fatal("错误的讨论区");
	strcpy(board, brd->filename);
	if(!has_read_perm(currentuser, board)) http_fatal("错误的讨论区名称");
	sprintf(dir, "boards/%s/.DIR", board);
	fp=fopen(dir, "r");
	if(fp==0) http_fatal("讨论区错误或没有目前文章");
	printf("查找讨论区'%s'内, 标题含: '%s' ", board, nohtml(title));
	if(title2[0]) printf("和 '%s' ", nohtml(title2));
	if(title3[0]) printf("不含 '%s' ", nohtml(title3));
	printf("作者为: '%s', '%d'天以内的%s文章.<br>\n", 
		userid[0] ? userid_str(userid) : "所有作者", dt, mg ? "精华" : "所有");
	printf("<table width=610>\n");
	printf("<tr><td>编号<td>标记<td>作者<td>日期<td>标题\n");
	while(1) {
		if(fread(&x, sizeof(x), 1, fp)==0) break;
		num++;
		if(title[0] && !strcasestr(x.title, title)) continue;
		if(title2[0] && !strcasestr(x.title, title2)) continue;
		if(userid[0] && strcasecmp(x.owner, userid)) continue;
		if(title3[0] && strcasestr(x.title, title3)) continue;
		if(abs(time(0)-atoi(x.filename+2))>dt*86400) continue;
		if(mg && !(x.accessed[0] & FILE_MARKED) && !(x.accessed[0] & FILE_DIGEST)) continue;
		if(og && !strncmp(x.title, "Re: ", 4)) continue;
		total++;
		printf("<tr><td>%d", num);
		printf("<td>%s", flag_str(x.accessed[0]));
		printf("<td>%s", userid_str(x.owner));
		printf("<td>%12.12s", 4+wwwCTime(atoi(x.filename+2)));
		printf("<td><a href=bbscon?board=%s&file=%s&num=%d>%40.40s </a>\n", board, x.filename, num-1, x.title);
		if(total>=999) break;
	}
	fclose(fp);
	printf("</table>\n");
	printf("<br>共找到 %d 篇文章符合条件", total);
	if(total>999) printf("(匹配结果过多, 省略第1000以后的查询结果)");
	printf("<br>\n");
	printf("[<a href=bbsdoc?board=%s>返回本讨论区</a>] [<a href='javascript:history.go(-1)'>返回上一页</a>]", board);
	http_quit();
	return 0;
}

int show_form(char *board) {
	printf("<table><form action=bbsbfind?type=1 method=post>\n");
	printf("<tr><td>版面名称: <input type=text maxlength=24 size=24 name=board value='%s'><br>\n", board);
	printf("<tr><td>标题含有: <input type=text maxlength=50 size=20 name=title> AND ");
	printf("<input type=text maxlength=50 size=20 name=title2>\n");
	printf("<tr><td>标题不含: <input type=text maxlength=50 size=20 name=title3>\n");
	printf("<tr><td>作者帐号: <input type=text maxlength=12 size=12 name=userid><br>\n");
	printf("<tr><td>时间范围: <input type=text maxlength=4  size=4  name=dt value=7> 天以内<br>\n");
	printf("<tr><td>精华文章: <input type=checkbox name=mg> ");
	printf("不含跟贴: <input type=checkbox name=og><br><br>\n");
	printf("<tr><td><input type=submit value=递交查询结果>\n");
	printf("</form></table>");
	printf("[<a href='bbsdoc?board=%s'>返回上一页</a>] [<a href=bbsfind>全站文章查询</a>]", board);
	http_quit();
}
