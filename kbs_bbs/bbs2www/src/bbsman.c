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
                do_set(board, atoi(parm_name[i] + 3), 0);
        }
    }
    printf("</table>");
    if (total <= 0)
        printf("请先选定文章<br>\n");
    printf("<br><a href=\"/bbsmdoc.php?board=%s\">返回管理模式</a>", encode_url(buf, board, sizeof(buf)));
    http_quit();
}

/* modified by stiger,use id */
int do_del(char *board, int id)
{
    FILE *fp;
    int num = 0;
    //char path[256], buf[256], dir[256] ;
    char dir[256];
    struct fileheader f;
    //struct userec *u = NULL;
    bcache_t *brd = getbcache(board);

    sprintf(dir, "boards/%s/.DIR", board);
    //sprintf(path, "boards/%s/%s", board, file);
    fp = fopen(dir, "r");
    if (fp == 0)
        http_fatal("错误的参数");
    while (1) {
        if (fread(&f, sizeof(struct fileheader), 1, fp) <= 0)
            break;
        if (f.id==id) {
            switch (del_post(num, &f, dir, board)) {
            case DONOTHING:
                http_fatal("你无权删除该文");
                break;
            default:
                printf("<tr><td>%s  </td><td>标题:%s </td><td>删除成功.</td></tr>\n", f.owner, nohtml(f.title));
            }
            return;
        }
        num++;
    }
    fclose(fp);
    printf("<tr><td></td><td></td><td>文件不存在.</td></tr>\n");
}

/* 加 G 时并没有 post 到文摘区 */
/* modified by stiger,use id */
int do_set(char *board, int id, int flag)
{
    FILE *fp;
    char path[256], dir[256];
    struct fileheader f;
    int ent=1;
    int ffind=0;

    sprintf(dir, "boards/%s/.DIR", board);
    //sprintf(path, "boards/%s/%s", board, file);
    fp = fopen(dir, "r+");
    if (fp == 0)
        http_fatal("错误的参数");
    while (1) {
        if (fread(&f, sizeof(struct fileheader), 1, fp) <= 0)
            break;
        if (f.id==id) {
/*
            if (flag==FILE_READ)
                f.accessed[1] |= flag;
            else
                f.accessed[0] |= flag;
            if (flag == 0) {
                f.accessed[0] &= ~(FILE_MARKED|FILE_DIGEST|FILE_SIGN);
                f.accessed[1] &= ~(FILE_READ);
            }
            fseek(fp, -1 * sizeof(struct fileheader), SEEK_CUR);
            fwrite(&f, sizeof(struct fileheader), 1, fp);
            fclose(fp);
            printf("<tr><td>%s</td><td>标题:%s</td><td>标记成功.</td></tr>\n", f.owner, nohtml(f.title));
            return;
*/
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
    }else
        printf("<tr><td></td><td></td><td></td><td>文件不存在.</td></tr>\n");
}

