/*
 * $Id$
 */
#include "bbslib.h"

int main()
{
    FILE *fp;
    char buf[512], board[80], dir[80], file[80], filename[80], *ptr;
    struct fileheader x;
    int num, tmp, total;
    char board_url[80];

    init_all();
    strsncpy(board, getparm("board"), 32);
    strsncpy(file, getparm("file"), 32);
    num = atoi(getparm("num"));
    printf("<center>\n");
    if (!has_read_perm(currentuser, board))
        http_fatal("错误的讨论区");
    strcpy(board, getbcache(board)->filename);
    printf("%s -- 文章阅读 [讨论区: %s]<hr class=\"default\">", BBSNAME, board);
    if (valid_filename(file) < 0)
        http_fatal("错误的参数");
    sprintf(dir, "boards/%s/.DIGEST", board);
    total = file_size(dir) / sizeof(x);
    if (total <= 0)
        http_fatal("此讨论区不存在或者为空");
    printf("<table width=\"610\" border=\"1\">\n");
    printf("<tr><td>\n<pre>");
    sprintf(filename, "boards/%s/%s", board, file);
    fp = fopen(filename, "r");
    if (fp == 0)
        http_fatal("本文不存在或者已被删除");
    while (fgets(buf, 512, fp) != NULL) {
        hhprintf("%s", void1(buf));
    }
    fclose(fp);
    printf("</pre></td></tr>\n</table><hr class=\"default\">\n");
    printf("[<a href=\"bbssec\">分类讨论区</a>]");
    printf("[<a href=\"bbsall\">全部讨论区</a>]");
    fp = fopen(dir, "r+");
    if (fp == 0)
        http_fatal("dir error2");
    encode_url(board_url, board, sizeof(board_url));
    if (num > 0) {
        fseek(fp, sizeof(x) * (num - 1), SEEK_SET);
        fread(&x, sizeof(x), 1, fp);
        printf("[<a href=\"bbscon?board=%s&file=%s&num=%d\">上一篇</a>]", board_url, x.filename, num - 1);
    }
    printf("[<a href=\"/bbsdoc.php?board=%s\">本讨论区</a>]", board_url);
    if (num < total - 1) {
        fseek(fp, sizeof(x) * (num + 1), SEEK_SET);
        fread(&x, sizeof(x), 1, fp);
        printf("[<a href=\"bbscon?board=%s&file=%s&num=%d\">下一篇</a>]", board_url, x.filename, num + 1);
    }
    fclose(fp);
    ptr = x.title;
    if (!strncmp(ptr, "Re: ", 4))
        ptr += 4;
    printf("[<a href=\"bbstfind?board=%s&title=%s\">同主题阅读</a>]\n", board_url, encode_url(buf, void1(ptr), sizeof(buf)));
    printf("</center>\n");
    http_quit();
}
