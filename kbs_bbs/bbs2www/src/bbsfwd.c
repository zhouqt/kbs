#include "bbslib.h"

int main() {
	struct fileheader *x;
	char board[80], file[80], target[80];
	struct userec *u = NULL;

	init_all();
	strsncpy(board, getparm("board"), 30);
	strsncpy(file, getparm("file"), 30);
	strsncpy(target, getparm("target"), 30);
	if(!loginok) http_fatal("匆匆过客不能进行本项操作");
	if(!has_read_perm(currentuser, board)) http_fatal("错误的讨论区");
	x=get_file_ent(board, file);
	if(x==0) http_fatal("错误的文件名");
	printf("<center>%s -- 转寄/推荐给好友 [使用者: %s]<hr color=green>\n", BBSNAME, currentuser->userid);
	if(target[0]) {
		if(!strstr(target, "@")) {
			if(getuser(target, &u) == 0)
				http_fatal("错误的使用者帐号");
			strcpy(target, u->userid);
		}
		return do_fwd(x, board, target);
	}
	printf("<table><tr><td>\n");
	printf("文章标题: %s<br>\n", nohtml(x->title));
	printf("文章作者: %s<br>\n", x->owner);
	printf("原讨论区: %s<br>\n", board);
	printf("<form action=bbsfwd method=post>\n");
	printf("<input type=hidden name=board value=%s>", board);
	printf("<input type=hidden name=file value=%s>", file);
	printf("把文章转寄给 <input name=target size=30 maxlength=30 value=%s> (请输入对方的id或email地址). <br>\n",
		currentuser->email);
	printf("<input type=submit value=确定转寄></form>");
	return 0;
}

int do_fwd(struct fileheader *x, char *board, char *target) {
	FILE *fp, *fp2;
	char title[512], buf[512], path[200], i;
	sprintf(path, "boards/%s/%s", board, x->filename);
	if(!file_exist(path)) http_fatal("文件内容已丢失, 无法转寄");
	sprintf(title, "[转寄] %s", x->title);
	title[60]=0;
	post_mail(target, title, path, currentuser->userid, currentuser->username, fromhost, -1);
	printf("文章已转寄给'%s'<br>\n", nohtml(target));
	printf("[<a href='javascript:history.go(-2)'>返回</a>]");
}
