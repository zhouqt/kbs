/*
 * $Id$
 */
#include "bbslib.h"

char *eff_size();
int my_t_lines;

int main()
{
    FILE *fp;
    char board[80], dir[80], *ptr;
    char brdencode[256], buf[256];
    bcache_t *x1;
    struct fileheader x;
    int i, start, total;

    init_all();
    strsncpy(board, getparm("board"), 32);
    x1 = getbcache(board);
    if (x1 == 0)
        http_fatal("错误的讨论区");
    strcpy(board, x1->filename);
    if (!has_read_perm(currentuser, board))
        http_fatal("错误的讨论区");
    /*getcwd(dir, sizeof(dir)-1); */
    sprintf(dir, "boards/%s/.DIR", board);
    if ((fp = fopen(dir, "r")) == NULL)
        http_fatal("Open .DIR failed.");
    total = file_size(dir) / sizeof(struct fileheader);
    start = atoi(getparm("start"));
    my_t_lines = atoi(getparm("my_t_lines"));
    if (my_t_lines < 10 || my_t_lines > 40)
        my_t_lines = 20;
    if (strlen(getparm("start")) == 0 || start > total - my_t_lines)
        start = total - my_t_lines;
    if (start < 0)
        start = 0;
    brc_initial(currentuser->userid, board);
    encode_url(brdencode, board, sizeof(brdencode));
    printf("<nobr><center>\n");
    printf("%s -- [讨论区: %s] 版主[%s] 文章数[%d] <a href=\"bbsbrdadd?board=%s\">预定本版</a><hr color=\"green\">\n", BBSNAME, board, userid_str(x1->BM), total, brdencode);
    if (total <= 0)
        http_fatal("本讨论区目前没有文章");
    printf("<table width=\"613\">\n");
    printf("<tr><td>序号</td><td>状态</td><td>作者</td><td>日期</td><td>标题</td></tr>\n");
    if (fp) {
        fseek(fp, start * sizeof(struct fileheader), SEEK_SET);
        for (i = 0; i < my_t_lines; i++) {
            char filename[80];
            char *ptr, *font1 = "", *font2 = "";

            if (fread(&x, sizeof(x), 1, fp) <= 0)
                break;
            ptr = flag_str2(x.accessed[0], !brc_unread(FILENAME2POSTTIME(x.filename)));
            if (ptr[0] == 'N') {
                font1 = "<font color=\"#909090\">";
                font2 = "</font>";
            }
            sprintf(filename, "boards/%s/%s", board, x.filename);
            printf("<tr><td>%d</td><td>%s%s%s</td><td>%s</td>", start + i + 1, font1, ptr, font2, userid_str(x.owner));
            /* 只显示日期 */
            printf("<td>%6.6s</td>", wwwCTime(atoi(x.filename + 2)) + 4);
            /* 去掉统计字节数的功能, 浪费系统资源 */
            printf("<td><a href=\"bbscon?board=%s&file=%s&num=%d\">%s%s </a></td>", brdencode, x.filename, start + i, strncmp(x.title, "Re: ", 4) ? "○ " : "", void1(nohtml(x.title)));
            /* 去掉人气值功能 */
            printf("</tr>\n");
        }
        printf("</table><hr>\n");
    }
    /*printf("<a href=bbsfdoc?board=%s>文件上载</a> ", board); */
    printf("<a href=\"bbspst?board=%s\">发表文章</a> \n", brdencode);
    printf("<a href=\"javascript:location=location\">刷新</a> ");
    if (start > 0)
        printf("<a href=\"bbsdoc?board=%s&start=%d\">上一页</a> ", brdencode, start - my_t_lines);
    if (start < total - 20)
        printf("<a href=\"bbsdoc?board=%s&start=%d\">下一页</a> ", brdencode, start + my_t_lines);
    /*
       printf("<a href=\"bbstdoc?board=%s\">主题模式</a> ", board);
     */
    if (has_BM_perm(currentuser, board))
        printf("<a href=\"bbsmdoc?board=%s\">管理模式</a> ", brdencode);
    printf("<a href=\"bbsnot?board=%s\">进版画面</a> ", brdencode);
    printf("<a href=\"bbsgdoc?board=%s\">文摘区</a> ", brdencode);
    ann_get_path(board, buf, sizeof(buf));
    printf("<a href=\"bbs0an?path=%s\">精华区</a> ", http_encode_string(buf, sizeof(buf)));
    /*printf("<a href=\"/an/%s.tgz\">下载精华区</a> ", board); */
    printf("<a href=\"bbsbfind?board=%s\">版内查询</a> \n", brdencode);
    if (loginok)
        printf("<a href=\"bbsclear?board=%s&start=%d\">清除未读</a> \n", brdencode, start);
    fclose(fp);
    i = get_seccode_index(x1->title[0]);
    printf("<a href=\"bbsboa?group=%d\">返回[%s]</a> \n", i, secname[i][0]);
    printf("<form name=\"form1\" action=\"bbsdoc?board=%s\" method=\"post\" onsubmit='this.start.value-=1'>\n", brdencode);
    printf("<input type=\"submit\" value=\"跳转到\"> 第 <input type=\"text\" name=\"start\" size=\"4\"> 篇");
    printf("</form>\n");
    http_quit();
}
