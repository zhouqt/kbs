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
    printf("<center>\n");
    if (getboardnum(board,&bh)==0||!check_read_perm(currentuser, &bh))
        http_fatal("错误的讨论区");
    strcpy(board, getbcache(board)->filename);
    if ((loginok)&&strcmp(currentuser->userid,"guest"))
        brc_initial(currentuser->userid, board);
    printf("%s -- 主题文章阅读 [讨论区: %s]<hr class=\"class\">", BBSNAME, board);
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
    printf("</center>\n");
    if ((loginok)&&strcmp(currentuser->userid,"guest"))
        brc_update(currentuser->userid);
    http_quit();
}

int show_file(char *board,struct boardheader* bh,struct fileheader *x, int n, char* brdencode)
{
    FILE *fp;
    char path[80], buf[512], board_url[80];
    char* title;

    if ((loginok)&&strcmp(currentuser->userid,"guest"))
        brc_add_read(x->id);
    sprintf(path, "boards/%s/%s", board, x->filename);
    fp = fopen(path, "r");
    if (fp == 0)
        return;
    encode_url(board_url, board, sizeof(board_url));
    printf("<table width=\"610\"><pre>\n");
    printf("[<a href=\"bbscon?board=%s&id=%d&num=%d\">本篇全文</a>] ", board_url, x->id, n);
    if (strncmp(x->title,"Re:",3))
	    title=x->title;
    else
	    title=x->title+4;
    if ((x->accessed[1] & FILE_READ) == 0)
        printf("[<a href=\"bbspst?board=%s&file=%s&userid=%s&title=Re: %s&refilename=%s&attach=%d\">回文章</a>]", 
            brdencode, x->filename, x->owner, encode_url(buf, title, sizeof(buf)), x->filename,bh->flag&BOARD_ATTACH?1:0);
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
