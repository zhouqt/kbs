/*
 * $Id$
 */
#include "bbslib.h"

/*int no_re=0;*/
/*	bbscon?board=xx&file=xx&start=xx 	*/

int main()
{
    FILE *fp;
    char title[256], userid[80], board[80], dir[80], file[80], *ptr;
    char brdencode[STRLEN];
    struct fileheader x;
    int i, num = 0, found = 0;

    init_all();
    strsncpy(board, getparm("board"), 32);
    strsncpy(file, getparm("file"), 32);
    printf("<center>\n");
    if (!has_read_perm(currentuser, board))
	http_fatal("错误的讨论区");
    strcpy(board, getbcache(board)->filename);
    if (loginok)
	brc_initial(currentuser->userid, board);
    printf("%s -- 主题文章阅读 [讨论区: %s]<hr class=\"class\">", BBSNAME, board);
    if (valid_filename(file) < 0)
	http_fatal("错误的参数");
    sprintf(dir, "boards/%s/.DIR", board);
    fp = fopen(dir, "r+");
    if (fp == 0)
	http_fatal("目录错误");
    while (1) {
	if (fread(&x, sizeof(x), 1, fp) <= 0)
	    break;
	num++;
	if (!strcmp(x.filename, file)) {
	    ptr = x.title;
	    if (!strncmp(ptr, "Re:", 3))
		ptr += 4;
	    strsncpy(title, ptr, 40);
	    found = 1;
	    strcpy(userid, x.owner);
	    show_file(board, &x, num - 1);
	    while (1) {
		if (fread(&x, sizeof(x), 1, fp) <= 0)
		    break;
		num++;
		if (!strncmp(x.title + 4, title, 39) && !strncmp(x.title, "Re: ", 4))
		    show_file(board, &x, num - 1);
	    }
	}
    }
    fclose(fp);
    if (found == 0)
	http_fatal("错误的文件名");
    encode_url(brdencode, board, sizeof(brdencode));
    if (!can_reply_post(board, file))
	printf("[<a href=\"bbspst?board=%s&file=%s&userid=%s&title=%s\">回文章</a>] ", brdencode, file, x.owner, http_encode_string(title, sizeof(title)));
    printf("[<a href=\"javascript:history.go(-1)\">返回上一页</a>]");
    printf("[<a href=\"bbsdoc?board=%s\">本讨论区</a>]", brdencode);
    ptr = x.title;
    if (!strncmp(ptr, "Re: ", 4))
	ptr += 4;
    printf("</center>\n");
    if (loginok)
	brc_update(currentuser->userid);
    http_quit();
}

int show_file(char *board, struct fileheader *x, int n)
{
    FILE *fp;
    char path[80], buf[512], board_url[80];

    if (loginok)
	brc_add_read(x->filename);
    sprintf(path, "boards/%s/%s", board, x->filename);
    fp = fopen(path, "r");
    if (fp == 0)
	return;
    encode_url(board_url, board, sizeof(board_url));
    printf("<table width=\"610\"><pre>\n");
    printf("[<a href=\"bbscon?board=%s&file=%s&num=%d\">本篇全文</a>] ", board_url, x->filename, n);
    printf("[<a href=\"bbspst?board=%s&file=%s&title=%s&userid=%s\">回复本文</a>] ", board_url, x->filename, encode_url(buf, x->title, sizeof(buf)), x->owner);
    printf("[本篇作者: %s]\n", userid_str(x->owner));
    /*printf("[本篇人气: %d]\n", *(int*)(x->title+73)); */
    while (1) {
	if (fgets(buf, 500, fp) == 0)
	    break;
	if (!strncmp(buf, ": ", 2))
	    continue;
	if (!strncmp(buf, "【 在 ", 4))
	    continue;
	hhprintf("%s", buf);
    }
    fclose(fp);
    printf("</pre></table><hr class=\"default\">");
}
