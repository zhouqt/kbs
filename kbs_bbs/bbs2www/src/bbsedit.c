#include "bbslib.h"

int main() {
   	FILE *fp;
   	int type=0, i, num;
	char userid[80], buf[512], path[512], file[512], board[512], title[80]="";
   	bcache_t *brd;
   	struct fileheader  *x;

   	init_all();
	if(!loginok) http_fatal("匆匆过客不能修改文章，请先登录");
	strsncpy(board, getparm("board"), 20);
	type=atoi(getparm("type"));
	brd=getbcache(board);
	if(brd==0) http_fatal("错误的讨论区");
	strcpy(board, brd->filename);
	strsncpy(file, getparm("file"), 20);
	if(!haspostperm(currentuser, board))
		http_fatal("错误的讨论区或者您无权在此讨论区发表文章");
	if (valid_filename(file) < 0)
		http_fatal("错误的参数");
   	x=get_file_ent(board, file);
	/*if(strstr(file, "..") || strstr(file, "/")) http_fatal("错误的参数");*/
	if(x==0) http_fatal("错误的参数");
	/* 此处尚未检查站务和斑竹 */
	if(strcmp(x->owner, currentuser->userid)) http_fatal("你无权修改此文章");
	/* added by flyriver, 2001.12.10 */
	/* 同名ID不能修改老ID的文章 */
#ifdef HAPPY_BBS
	if (x->posttime < currentuser->firstlogin)
		http_fatal("你无权修改此文章");
#else
	if (file_time(file) < currentuser->firstlogin)
		http_fatal("你无权修改此文章");
#endif /* HAPPY_BBS */
	printf("<center>%s -- 修改文章 [使用者: %s]<hr color=green>\n", BBSNAME, currentuser->userid);
	if(type!=0) return update_form(board, file);
   	printf("<table border=1>\n");
	printf("<tr><td>");
	printf("<tr><td><form method=post action=bbsedit>\n");
   	printf("使用标题: %s 讨论区: %s<br>\n", nohtml(x->title), board);
   	printf("本文作者：%s<br>\n", currentuser->userid);
   	printf("<textarea name=text rows=20 cols=80 wrap=physicle>");
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(path, "r");
	if(fp==0) http_fatal("文件丢失");
	/* 这里还需要添加代码，以防止用户修改文件头和来源 */
	while(1) {
		if(fgets(buf, 500, fp)==0) break;
		if(!strcasestr(buf, "</textarea>")) printf("%s", buf);
	}
	fclose(fp);
   	printf("</textarea>\n");
   	printf("<tr><td class=post align=center>\n");
	printf("<input type=hidden name=type value=1>\n");
	printf("<input type=hidden name=board value=%s>\n", board);
	printf("<input type=hidden name=file value=%s>\n", file);
	printf("<input type=submit value=存盘> \n");
   	printf("<input type=reset value=重置></form>\n");
	printf("</table>");
	http_quit();
}

int update_form(char *board, char *file) {
	FILE *fp;
	char *buf=getparm("text"), path[80];
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(path, "w");
	if(fp==0) http_fatal("无法存盘");
	fprintf(fp, "%s", buf);
	fprintf(fp, "\n※ 修改:．%s 於 %s 修改本文．[FROM: %s] ", currentuser->userid, wwwCTime(time(0))+4, fromhost);
	fclose(fp);
	printf("修改文章成功.<br><a href=bbsdoc?board=%s>返回本讨论区</a>", board);
}
