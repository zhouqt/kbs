/*
 * $Id$
 */
#include "bbslib.h"

/*int no_re=0;*/

int show_file(char *board, int bid, struct boardheader* bh, struct fileheader *x, char* brdencode);

#define MAX_THREADS_NUM 512

int main()
{
    char board[80], dir[STRLEN];
    char brdencode[STRLEN];
    struct fileheader *fh;
    int i, num;
	int start,shownum;
	int haveprev=0, havenext=0;
    struct boardheader bh;
	int gid; /* group id */
	int bid;

    initwww_all();
    strsncpy(board, getparm("board"), 32);
	gid = atoi(getparm("gid"));
	if( gid < 0 ) gid=0;
	start = atoi(getparm("start"));
	if( start <= 0 ) start = gid;
	shownum = atoi(getparm("num"));
	if( shownum <= 0 ) shownum = 20;
	/*
    strsncpy(file, getparm("file"), 32);
	*/
    if ((bid = getboardnum(board,&bh))==0||!check_read_perm(getCurrentUser(), &bh))
        http_fatal("错误的讨论区");
    strcpy(board, getbcache(board)->filename);
    encode_url(brdencode, board, sizeof(brdencode));

	fh = (struct fileheader *)malloc(shownum * sizeof(struct fileheader));
	if (fh == NULL)
        http_fatal("错误的参数");
	setbdir(DIR_MODE_NORMAL, dir, board);
	if ((num = get_threads_from_gid(dir, gid, fh, shownum, start, &haveprev)) == 0)
	{
		free(fh);
        http_fatal("错误的参数");
		/* the process already terminated */
	}

#ifdef HAVE_BRC_CONTROL
    if ((loginok)&&strcmp(getCurrentUser()->userid,"guest"))
        brc_initial(getCurrentUser()->userid, board, getSession());
#endif
    printf("%s -- 主题文章阅读 [讨论区: %s]<hr class=\"default\" />", BBSNAME, board);

	if( num == shownum )
		havenext = fh[shownum-1].id;
	if( haveprev || havenext )
		printf("<center>\n");
	if( haveprev ){
    	printf("[<a href=\"/cgi-bin/bbs/bbstcon?board=%s&gid=%d&start=%d&num=%d\">上一页</a>]", brdencode, gid, haveprev, shownum);
	}
	if( havenext ){
    	printf("[<a href=\"/cgi-bin/bbs/bbstcon?board=%s&gid=%d&start=%d&num=%d\">下一页</a>]", brdencode, gid, havenext, shownum);
	}
	if( haveprev || havenext )
		printf("</center>\n");

	for (i = 0; i < num ; i++){
            show_file(board, bid, &bh, fh + i, brdencode);
	}

	free(fh);
    printf("<hr class=\"default\" /><center>");
    printf("[<a href=\"javascript:history.go(-1)\">后退</a>]");
	if( haveprev ){
    	printf("[<a href=\"/cgi-bin/bbs/bbstcon?board=%s&gid=%d&start=%d&num=%d\">上一页</a>]", brdencode, gid, haveprev, shownum);
	}
	if( havenext ){
    	printf("[<a href=\"/cgi-bin/bbs/bbstcon?board=%s&gid=%d&start=%d&num=%d\">下一页</a>]", brdencode, gid, havenext, shownum);
	}
    printf("[<a href=\"/bbsdoc.php?board=%s\">本讨论区</a>]</center>", brdencode);
#ifdef HAVE_BRC_CONTROL
    if ((loginok)&&strcmp(getCurrentUser()->userid,"guest"))
        brc_update(getCurrentUser()->userid, getSession());
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
			if (safe_mmapfile_handle(fd,  PROT_READ, MAP_SHARED,
						(void **)&ptr, (off_t *)&filesize) == 0)
			{
				flock(fd, LOCK_UN);
				free_output(out);
				close(fd);
				BBS_RETURN(0);
			}
			output_ansi_javascript(ptr, filesize, out, www_url);
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

int show_file(char *board, int bid, struct boardheader* bh, 
		struct fileheader *x, char* brdencode)
{
    char path[80], buf[512], board_url[80];
	char www_url[200];
    char* title;

#ifdef HAVE_BRC_CONTROL
    if ((loginok)&&strcmp(getCurrentUser()->userid,"guest"))
        brc_add_read(x->id, getSession());
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
	printf("<script language=\"Javascript\" src=\"/jscon.php?bid=%d&id=%d\"></script>\n",
			bid, x->id);
	//show_article(path,www_url);
    /*printf("[本篇人气: %d]\n", *(int*)(x->title+73)); */
	printf("</td></tr></table>\n");
}
