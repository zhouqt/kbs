/*
 * $Id$
 */
#include "bbslib.h"

int main()
{
    FILE *fp;
    char buf[512], board[80], dir[80], file[80], filename[80], *ptr;
    char buf2[512];
    struct fileheader x;
    struct fileheader oldx;
    int num, tmp, total;
    struct userec *user = NULL;
    struct boardheader *bp;

    init_all();
    strsncpy(board, getparm("board"), 32);
    strsncpy(file, getparm("file"), 32);
    num = atoi(getparm("num"));
    printf("<center>\n");
    if (!has_read_perm(currentuser, board))
        http_fatal("错误的讨论区");
    bp = getbcache(board);
    strcpy(board, bp->filename);
    if (valid_filename(file) < 0)
        http_fatal("错误的参数");
    if (get_file_ent(board, file, &x) <= 0)
        http_fatal("错误的文件名");
    memcpy(&oldx, &x, sizeof(x));
    printf("%s -- 文章阅读 [讨论区: %s]<hr color=\"green\">", BBSNAME, board);
    sprintf(dir, "boards/%s/.DIR", board);
    total = file_size(dir) / sizeof(x);
    if (total <= 0)
        http_fatal("此讨论区不存在或者为空");
    printf("<table width=\"610\" border=\"1\">\n");
    printf("<tr><td>\n<pre>");
    sprintf(filename, "boards/%s/%s", board, file);
    fp = fopen(filename, "r");
    if (fp == 0)
        http_fatal("本文不存在或者已被删除");
    while (1) {
        char *id, *s;

        if (fgets(buf, 512, fp) == 0)
            break;
        if (!strncmp(buf, "发信人: ", 8)) {
            ptr = strdup(buf);
            id = strtok(ptr + 8, " ");
            s = strtok(0, "");
            if (id == 0)
                id = " ";
            if (s == 0)
                s = "\n";
            if (strlen(id) < 13 && getuser(id, &user)) {
                printf("发信人: %s%s", userid_str(id), s);
                free(ptr);
                continue;
            }
            free(ptr);
        }
        if (!strncmp(buf, ": ", 2))
            printf("<font color=\"#008080\"><i>");
        hhprintf("%s", buf);
        if (!strncmp(buf, ": ", 2))
            printf("</i></font>");
    }
    fclose(fp);

    brc_initial(currentuser->userid, board);
    brc_add_read(x.filename);
    brc_update(currentuser->userid);
    encode_url(buf2, board, sizeof(buf2));
    printf("</pre></td></tr>\n</table><hr>\n");
    printf("[<a href=\"bbsfwd?board=%s&file=%s\">转寄/推荐</a>]", buf2, file);
    printf("[<a href=\"bbsccc?board=%s&file=%s\">转贴</a>]", buf2, file);
    if (can_delete_post(bp, &x, getcurrusr()))
        printf("[<a onclick=\"return confirm('你真的要删除本文吗?')\" href=\"bbsdel?board=%s&file=%s\">删除文章</a>]", buf2, file);
    if (can_edit_post(bp, &x, getcurrusr())) {
        printf("[<a href=\"bbsedit?board=%s&file=%s\">修改文章</a>]", buf2, file);
    }
    /*
     * TODO: 下面需要两次读操作，应该减为一个。
     */
    fp = fopen(dir, "r+");
    if (fp == 0)
        http_fatal("dir error2");
    if (num > 0) {
        fseek(fp, sizeof(x) * (num - 1), SEEK_SET);
        fread(&x, sizeof(x), 1, fp);
        printf("[<a href=\"bbscon?board=%s&file=%s&num=%d\">上一篇</a>]", buf2, x.filename, num - 1);
    }
    printf("[<a href=\"bbsdoc?board=%s\">本讨论区</a>]", buf2);
    if (num < total - 1) {
        fseek(fp, sizeof(x) * (num + 1), SEEK_SET);
        fread(&x, sizeof(x), 1, fp);
        printf("[<a href=\"bbscon?board=%s&file=%s&num=%d\">下一篇</a>]", buf2, x.filename, num + 1);
    }
    fclose(fp);
    ptr = oldx.title;
    if (!strncmp(ptr, "Re: ", 4))
        ptr += 4;
    ptr[60] = 0;
    if ((oldx.accessed[1] & FILE_READ) == 0)
        printf("[<a href=\"bbspst?board=%s&file=%s&userid=%s&title=Re: %s&refilename=%s\">回文章</a>]", buf2, file, oldx.owner, encode_url(buf, void1(ptr), sizeof(buf)), encode_url(buf2, void1(oldx.filename), sizeof(buf2)));
    printf("[<a href=\"bbstfind?board=%s&title=%s\">同主题阅读</a>]", buf2, encode_url(buf, void1(ptr), sizeof(buf)));
    printf("[<a href=\"javascript:history.go(-1)\">快速返回</a>]\n");
    printf("</center>\n");
    http_quit();
}
