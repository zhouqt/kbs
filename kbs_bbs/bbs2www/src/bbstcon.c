/*
 * $Id$
 */
#include "bbslib.h"

/*int no_re=0;*/
/*	bbscon?board=xx&file=xx&start=xx 	*/

int show_file(char *board,struct boardheader* bh,struct fileheader *x, int n, char* brdencode);
int main()
{
    FILE *fp;
    char title[256], userid[80], board[80], dir[80], file[80], *ptr;
    char brdencode[STRLEN];
    struct fileheader x,oldx;
    int i, num = 0, found = 0;
    struct boardheader bh;

    init_all();
    strsncpy(board, getparm("board"), 32);
    strsncpy(file, getparm("file"), 32);
    encode_url(brdencode, board, sizeof(brdencode));
    if (getboardnum(board,&bh)==0||!check_read_perm(currentuser, &bh))
        http_fatal("错误的讨论区");
    strcpy(board, getbcache(board)->filename);
#ifdef HAVE_BRC_CONTROL
    if ((loginok)&&strcmp(currentuser->userid,"guest"))
        brc_initial(currentuser->userid, board);
#endif
    printf("%s -- 主题文章阅读 [讨论区: %s]<hr class=\"default\" />", BBSNAME, board);
    if (VALID_FILENAME(file) < 0)
        http_fatal("错误的参数");
    sprintf(dir, "boards/%s/.DIR", board);
    fp = fopen(dir, "r+");
    if (fp == 0)
        http_fatal("目录错误");
    while (1) {
        if (fread(&oldx, sizeof(x), 1, fp) <= 0)
            break;
        num++;
        if (!strcmp(oldx.filename, file)) {
            ptr = oldx.title;
            if (!strncmp(ptr, "Re:", 3))
                ptr += 4;
            strsncpy(title, ptr, 40);
            found = 1;
            strcpy(userid, oldx.owner);
            show_file(board, &bh, &oldx, num - 1,brdencode);
            while (1) {
                if (fread(&x, sizeof(x), 1, fp) <= 0)
                    break;
                num++;
                if (!strncmp(x.title + 4, title, 39) && !strncmp(x.title, "Re: ", 4))
                    show_file(board, &bh, &x, num - 1,brdencode);
            }
        }
    }
    fclose(fp);
    printf("<hr class=\"default\" />");
    if (found == 0)
        http_fatal("错误的文件名");
//    if (!can_reply_post(board, file))
//        printf("[<a href=\"bbspst?board=%s&file=%s&userid=%s&title=Re: %s&refilename=%s\">回文章</a>]", brdencode, file, oldx.owner, encode_url(title, void1(ptr), sizeof(title)), oldx.filename);
//        printf("[<a href=\"bbspst?board=%s&file=%s&userid=%s&title=%s\">回文章</a>] ", brdencode, file, x.owner, http_encode_string(title, sizeof(title)));
    printf("[<a href=\"javascript:history.go(-1)\">返回上一页</a>]");
    printf("[<a href=\"/bbsdoc.php?board=%s\">本讨论区</a>]", brdencode);
    ptr = x.title;
    if (!strncmp(ptr, "Re: ", 4))
        ptr += 4;
#ifdef HAVE_BRC_CONTROL
    if ((loginok)&&strcmp(currentuser->userid,"guest"))
        brc_update(currentuser->userid);
#endif
    http_quit();
}

int show_article(char *filename,char *www_url)
{
	int fd;

    if ((fd = open(filename, O_RDONLY, 0644)) < 0) {
        printf("打开本文失败!<br />\n");
        return 0;
    } else {
		size_t filesize;
		char *ptr;
		const int outbuf_len = 4096;
		buffered_output_t *out;

		if (flock(fd, LOCK_EX) == -1){
			close(fd);
			return 0;
		}
		if ((out = alloc_output(outbuf_len)) == NULL)
		{
			flock(fd, LOCK_UN);
			close(fd);
			return 0;
		}
		BBS_TRY
		{
			if (safe_mmapfile_handle(fd, O_RDONLY, PROT_READ, MAP_SHARED,
						(void **)&ptr, &filesize) == 0)
			{
				flock(fd, LOCK_UN);
				free_output(out);
				close(fd);
				BBS_RETURN(0);
			}
			output_ansi_html(ptr, filesize, out, www_url);
		}
		BBS_CATCH
		{
		}
		BBS_END end_mmapfile((void *)ptr, filesize, -1);
		flock(fd, LOCK_UN);
		free_output(out);
		close(fd);
        return 1;
    }
}

int show_file(char *board,struct boardheader* bh,struct fileheader *x, int n, char* brdencode)
{
    char path[80], buf[512], board_url[80];
	char www_url[200];
    char* title;

#ifdef HAVE_BRC_CONTROL
    if ((loginok)&&strcmp(currentuser->userid,"guest"))
        brc_add_read(x->id);
#endif
    sprintf(path, "boards/%s/%s", board, x->filename);
    encode_url(board_url, board, sizeof(board_url));
    printf("<table width=\"90%%\" class=\"BODY\">\n");
	printf("<tr><td class=\"default\">\n");
	sprintf(www_url,"/bbscon.php?board=%s&id=%d",board_url,x->id);
    printf("[<a href=\"%s\">本篇全文</a>]", www_url);
    if (strncmp(x->title,"Re:",3))
	    title=x->title;
    else
	    title=x->title+4;
    if ((x->accessed[1] & FILE_READ) == 0)
        printf("[<a href=\"/bbspst.php?board=%s&reid=%d\">回文章</a>]", brdencode, x->id);
	printf("[<a href=\"/bbspstmail.php?board=%s&file=%s&userid=%s&title=Re: %s\">回信给作者</a>]",
		   brdencode, x->filename, x->owner, encode_url(buf, title, sizeof(buf)));
    printf("[本篇作者: %s]<br />\n", userid_str(x->owner));
	show_article(path,www_url);
    /*printf("[本篇人气: %d]\n", *(int*)(x->title+73)); */
	printf("</td></tr></table>\n");
}
