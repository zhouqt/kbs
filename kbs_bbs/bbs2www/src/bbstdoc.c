/*
 * $Id$
 */
#include "bbslib.h"
char *stat1();

int main()
{
    FILE *fp;
    char board[80], dir[80], *ptr;
    char brdencode[256], buf[256];
    bcache_t *x1;
    struct fileheader *data;
    int i, start, total2 = 0, total, sum = 0;

    init_all();
    strsncpy(board, getparm("board"), 32);
    x1 = getbcache(board);
    if (x1 == 0)
        http_fatal("错误的讨论区");
    strcpy(board, x1->filename);
    if (!check_read_perm(currentuser, x1))
        http_fatal("错误的讨论区");
    sprintf(dir, "boards/%s/.DIR", board);
    fp = fopen(dir, "r");
    if (fp == 0)
        http_fatal("错误的讨论区目录");
    total = file_size(dir) / sizeof(struct fileheader);
    data = calloc(sizeof(struct fileheader), total);
    if (data == 0)
        http_fatal("内存溢出");
    total = fread(data, sizeof(struct fileheader), total, fp);
    fclose(fp);
    for (i = 0; i < total; i++)
        if (strncmp(data[i].title, "Re:", 3))
            total2++;
    start = atoi(getparm("start"));
    if (strlen(getparm("start")) == 0 || start > total2 - 19)
        start = total2 - 19;
    if (start < 0)
        start = 0;
    encode_url(brdencode, board, sizeof(brdencode));
    printf("<nobr><center>\n");
    printf("%s -- 主题阅读: [讨论区: %s] 版主[%s] 文章%d, 主题%d个<hr color=\"green\">\n", BBSNAME, board, userid_str(x1->BM), total, total2);
    if (total <= 0)
        http_fatal("本讨论区目前没有文章");
    printf("<table width=\"610\">\n");
    printf("<tr><td>序号</td><td>状态</td><td>作者</td><td>日期</td><td>标题</td><td>回帖</td></tr>\n");
    for (i = 0; i < total; i++) {
        if (!strncmp(data[i].title, "Re:", 3))
            continue;
        sum++;
        if (sum - 1 < start)
            continue;
        if (sum - 1 > start + 19)
            break;
        printf("<tr><td>%d</td><td>%s</td><td>%s</td>", sum + 1, flag_str(data[i].accessed[0]), userid_str(data[i].owner));
        printf("<td>%6.6s</td>", wwwCTime(atoi(data[i].filename + 2)) + 4);
        printf("<td><a href=\"bbstcon?board=%s&file=%s\">○ %38.38s </a></td><td>%s</td></tr>", brdencode, data[i].filename, nohtml(data[i].title), stat1(data, i, total));
    }
    printf("</table><hr>\n");
    if (start > 0)
        printf("<a href=\"bbstdoc?board=%s&start=%d\">上一页</a> ", brdencode, start - 19);
    if (start < total2 - 19)
        printf("<a href=\"bbstdoc?board=%s&start=%d\">下一页</a> ", brdencode, start + 19);
    printf("<a href=\"/bbsnot.php?board=%s\">进版画面</a> ", brdencode);
    printf("<a href=\"/bbsdoc.php?board=%s\">一般模式</a> ", brdencode);
    printf("<a href=\"/bbsgdoc.php?board=%s\">文摘区</a> ", brdencode);
    ann_get_path(board, buf, sizeof(buf));
    printf("<a href=\"bbs0an?path=%s\">精华区</a> ", http_encode_string(buf, sizeof(buf)));
    /*printf("<a href=/an/%s.tgz>下载精华区</a> ", board); */
    printf("<a href=\"bbspst.html?board=%s&sig=%d&attach=%d\">发表文章</a> <br>\n", brdencode,currentuser->signature,x1->flag&BOARD_ATTACH);
    free(data);
    printf("<form action=\"bbstdoc?board=%s\" method=\"post\">\n", brdencode);
    printf("<input type=\"submit\" value=\"跳转到\"> 第 <input type=\"text\" name=\"start\" size=\"4\"> 篇");
    printf("</form>\n");
    http_quit();
}

char *stat1(struct fileheader *data, int from, int total)
{
    static char buf[256];
    char *ptr = data[from].title;
    int i, re = 0 /*, click=*(int*)(data[from].title+73) */ ;

    for (i = from; i < total; i++) {
        if (!strncmp(ptr, data[i].title + 4, 40)) {
            re++;
            /*click+=*(int*)(data[i].title+73); */
        }
    }
    sprintf(buf, "<font color=\"%s\">%d</font>", re > 9 ? "red" : "black", re);
    return buf;
}
