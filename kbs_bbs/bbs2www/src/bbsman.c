/*
 * $Id$
 */
#include "bbslib.h"

char genbuf[1024];
char currfile[STRLEN];

int main()
{
    int i, total = 0, mode;
    char board[80], *ptr;
    char buf[STRLEN];
    bcache_t *brd;

    init_all();
    if (!loginok)
        http_fatal("请先登录");
    strsncpy(board, getparm("board"), 60);
    mode = atoi(getparm("mode"));
    brd = getbcache(board);
    if (brd == 0)
        http_fatal("错误的讨论区");
    strcpy(board, brd->filename);
    if (!has_BM_perm(currentuser, board))
        http_fatal("你无权访问本页");
    if (mode <= 0 || mode > 5)
        http_fatal("错误的参数");
    printf("<table>");
    for (i = 0; i < parm_num && i < 40; i++) {
        if (!strncmp(parm_name[i], "box", 3)) {
            total++;
            if (mode == 1)
                do_del(board, atoi(parm_name[i] + 3));
            if (mode == 2)
                do_set(board, atoi(parm_name[i] + 3), FILE_MARK_FLAG);
            if (mode == 3)
                do_set(board, atoi(parm_name[i] + 3), FILE_DIGEST_FLAG);
            if (mode==4)
                do_set(board, atoi(parm_name[i] + 3), FILE_NOREPLY_FLAG);
            if (mode == 5)
                do_set(board, atoi(parm_name[i] + 3), FILE_DING_FLAG);
        }
       	else if (!strncmp(parm_name[i], "boz", 3)) {
            total++;
            if (mode == 1)
                do_del_zd(board, atoi(parm_name[i] + 3));
            if (mode == 2)
                do_set_zd(board, atoi(parm_name[i] + 3), FILE_MARK_FLAG);
            if (mode == 3)
                do_set_zd(board, atoi(parm_name[i] + 3), FILE_DIGEST_FLAG);
            if (mode==4)
                do_set_zd(board, atoi(parm_name[i] + 3), FILE_NOREPLY_FLAG);
            if (mode == 5)
                do_del_zd(board, atoi(parm_name[i] + 3));
        }
    }
    printf("</table>");
    if (total <= 0)
        printf("请先选定文章<br>\n");
    printf("<br><a href=\"/bbsmdoc.php?board=%s\">返回管理模式</a>", encode_url(buf, board, sizeof(buf)));
    http_quit();
}

/* modified by stiger,20030414 */
int do_del(char *board, int id)
{
    int fd;
    int ent;
    char dir[256];
    struct fileheader f;

    sprintf(dir, "boards/%s/.DIR", board);
    fd = open(dir, O_RDWR, 0644);
    if (fd < 0)
        http_fatal("错误的参数");
    if( get_records_from_id( fd, id, &f, 1, &ent) ){
	close(fd);
        switch (del_post(ent, &f, dir, board)) {
        case DONOTHING:
            http_fatal("你无权删除该文");
            break;
        default:
            printf("<tr><td>%s  </td><td>标题:%s </td><td>删除成功.</td></tr>\n", f.owner, nohtml(f.title));
	}
	return;
    }

    printf("<tr><td></td><td></td><td>文件不存在.</td></tr>\n");
}

int do_del_zd(char *board, int id)
{
    FILE *fp;
	int ffind=0;
    int ent=1;
    char dir[256];
    struct fileheader f;

    sprintf(dir, "boards/%s/.DINGDIR", board);

	fp = fopen(dir, "r+");
    if (fp == 0) 
        http_fatal("错误的参数"); 
    while (1) {
	    if (fread(&f, sizeof(struct fileheader), 1, fp) <= 0)
		    break;               
	    if (f.id==id) {
		    ffind=1;
		    break;
	    }
	    ent++;
    }
    fclose(fp);

    if( ffind ){
        switch (del_post(ent, &f, dir, board)) {
        case DONOTHING:
            http_fatal("你无权删除该文");
            break;
        default:
            printf("<tr><td>%s  </td><td>标题:%s </td><td>删除成功.</td></tr>\n", f.owner, nohtml(f.title));
	}
	return;
    }

    printf("<tr><td></td><td></td><td>文件不存在.</td></tr>\n");
}

/* 加 G 时并没有 post 到文摘区 */
/* modified by stiger,20030414 */
int do_set(char *board, int id, int flag)
{
    int fd;
    char dir[256];
    struct fileheader f;
    int ent;

	setbdir(DIR_MODE_NORMAL, dir, board);
    fd = open(dir, O_RDWR, 0644);
    if( fd >= 0 && get_records_from_id( fd, id, &f, 1, &ent) )
	{
        close(fd);
		if(change_post_flag(NULL, currentuser, 0, board, ent, &f, dir, flag, 0)!=DONOTHING)
			printf("<tr><td>%s</td><td>标题:%s</td><td>标记成功.</td></tr>\n", f.owner, nohtml(f.title));
		else
			printf("<tr><td>%s</td><td>标题:%s</td><td>标记不成功.</td></tr>\n", f.owner, nohtml(f.title));
    }else{
        close(fd);
        printf("<tr><td></td><td></td><td></td><td>文件不存在.</td></tr>\n");
    }
    
}


int do_set_zd(char *board, int id, int flag)
{
    FILE *fp;
    char dir[256];
    struct fileheader f;
    int ent=1;
    int ffind=0;

    sprintf(dir,"boards/%s/.DINGDIR",board);
    
	fp = fopen(dir, "r+");
    if (fp == 0) 
        http_fatal("错误的参数"); 
    while (1) {
	    if (fread(&f, sizeof(struct fileheader), 1, fp) <= 0)
		    break;               
	    if (f.id==id) {
		    ffind=1;
		    break;
	    }
	    ent++;
    }
    fclose(fp);

	if(ffind){
		if(change_post_flag(NULL, currentuser, 0, board, ent, &f, dir, flag, 0)!=DONOTHING)
			printf("<tr><td>%s</td><td>标题:%s</td><td>标记成功.</td></tr>\n", f.owner, nohtml(f.title));
		else
			printf("<tr><td>%s</td><td>标题:%s</td><td>标记不成功.</td></tr>\n", f.owner, nohtml(f.title));
    }else{
        printf("<tr><td></td><td></td><td></td><td>文件不存在.</td></tr>\n");
    }
}
