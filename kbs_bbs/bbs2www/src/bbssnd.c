/*
 * $Id$
 */
#include "bbslib.h"

int main()
{
    FILE *fp;
    char filename[80], dir[80], board[80], title[80], buf[80], buf2[80], *content,path[80];
    int r, i, sig;
	int reid;
    struct fileheader x, *oldx;
    bcache_t *brd;
    int local, anony;
    /*int filtered = 0;*/

    init_all();
    if (!loginok)
        http_fatal("匆匆过客不能发表文章，请先登录");
    strsncpy(board, getparm("board"), 18);
    strsncpy(title, getparm("title"), 50);
    /*strsncpy(oldfilename, getparm("refilename"), 80);*/
    brd = getbcache(board);
    if (brd == 0)
        http_fatal("错误的讨论区名称");
    if (brd->flag&BOARD_GROUP)
        http_fatal("错误的讨论区名称");
    strcpy(board, brd->filename);
    for (i = 0; i < strlen(title); i++) {
        if (title[i] <= 27 && title[i] >= -1)
            title[i] = ' ';
    }
    sig = atoi(getparm("signature"));
	reid = atoi(getparm("reid"));
    local = atoi(getparm("outgo")) ? 0 : 1;
    anony = atoi(getparm("anony")) ? 1 : 0;
    content = getparm("text");
    if (title[0] == 0)
        http_fatal("文章必须要有标题");
    sprintf(dir, "boards/%s/.DIR", board);
    if (true == checkreadonly(board) || !haspostperm(currentuser, board))
        http_fatal("此讨论区是唯读的, 或是您尚无权限在此发表文章.");
    if (deny_me(currentuser->userid, board) && !HAS_PERM(currentuser, PERM_SYSOP))
        http_fatal("很抱歉, 你被版务人员停止了本版的post权利.");
    if (abs(time(0) - *(int *) (u_info->from + 36)) < 6) {
        *(int *) (u_info->from + 36) = time(0);
        http_fatal("两次发文间隔过密, 请休息几秒后再试");
    }
    *(int *) (u_info->from + 36) = time(0);
	if( atoi(getparm("tmpl")) )
   		sprintf(filename, "tmp/%s.tmpl.tmp", getcurruserid());
	else{
   		sprintf(filename, "tmp/%s.%d.tmp", getcurruserid(), getpid());
    	f_append(filename, unix_string(content));
	}

    if(reid > 0){
        int pos;int fd;
        oldx = (struct fileheader*)malloc(sizeof(struct fileheader));

		setbfile(path,board,DOT_DIR);
		fd =open(path,O_RDWR);
		if(fd < 0)http_fatal("索引文件不存在");
		get_records_from_id(fd,reid,oldx,1,&pos);

		close(fd);
        if (pos < 0) {
    		free(oldx);
    		oldx = NULL;
        }
        else
        if (oldx->accessed[1] & FILE_READ)
           http_fatal("本文不能回复");
    }
    else {
        oldx = NULL;
    }
#ifdef HAVE_BRC_CONTROL
    brc_initial(currentuser->userid, board);
#endif
    if (is_outgo_board(board) && local == 0)
        local = 0;
    else
        local = 1;
    /*if (filtered == 1)
		r = post_article(FILTER_BOARD, title, filename, currentuser, fromhost, sig, local, anony, oldx);
    else*/
    if (brd->flag&BOARD_ATTACH) {
#if USE_TMPFS==1
        snprintf(buf,MAXPATH,"%s/home/%c/%s/%d/upload",TMPFSROOT,toupper(currentuser->userid[0]),
			currentuser->userid,utmpent);
#else
        snprintf(buf,MAXPATH,"%s/%s_%d",ATTACHTMPPATH,currentuser->userid,utmpent);
#endif
        r = post_article(board, title, filename, currentuser, fromhost, sig, local, anony, oldx,buf);
        f_rm(buf);
    }
    else
        r = post_article(board, title, filename, currentuser, fromhost, sig, local, anony, oldx,NULL);
    if (r < 0)
        http_fatal("内部错误，无法发文");
#ifdef HAVE_BRC_CONTROL
    brc_update(currentuser->userid);
#endif
    if(oldx)
    	free(oldx);
    unlink(filename);
    sprintf(buf, "/bbsdoc.php?board=%s", encode_url(buf2, board, sizeof(buf2)));
    if (!junkboard(board)) {
        currentuser->numposts++;
        write_posts(currentuser->userid, board, title);
    }
    redirect(buf);
    return 0;
}
