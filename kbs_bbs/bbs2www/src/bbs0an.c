/*
 * $Id$
 */
#include "bbslib.h"

void ann_show_item(MENU * pm, ITEM * it)
{
    char title[STRLEN];
    char *id;
    char buf[256];
    char pathbuf[256];
    char *ptr;

    strncpy(title, it->title, sizeof(title) - 1);
    title[sizeof(title) - 1] = '\0';
    if (strlen(title) <= 39) {
        id = "";
    } else {
        if ((ptr = strchr(title + 38, '(')) != NULL) {
            *ptr = '\0';
            id = ptr + 1;
            if (strncmp(id, "BM: ", 4) == 0)
                id += 4;
            if ((ptr = strchr(id, ')')) != NULL)
                *ptr = '\0';
        } else if ((ptr = strchr(title + 38, ' ')) != NULL) {
            *ptr = '\0';
            id = ptr + 1;
            trim(id);
        } else
            id = "";
        rtrim(title);
    }
    printf("<tr><td>%d</td>", pm->now + 1);
    sprintf(buf, "%s/%s", pm->path, it->fname);
    ptr = strchr(pm->path, '/');
    if (!file_exist(buf)) {
        printf("<td>[错误] </td><td>%s</td>", nohtml(title));
    } else if (file_isdir(buf)) {
        snprintf(pathbuf, sizeof(pathbuf), "%s/%s", ptr == NULL ? "" : ptr, it->fname);
        printf("<td>[目录] </td><td><a href=\"bbs0an?path=%s\">%s</a></td>", http_encode_string(pathbuf, sizeof(pathbuf)), nohtml(title));
    } else {
        snprintf(pathbuf, sizeof(pathbuf), "%s/%s", ptr == NULL ? "" : ptr, it->fname);
        printf("<td>[文件] </td><td><a href=\"bbsanc?path=%s\">%s</a></td>", http_encode_string(pathbuf, sizeof(pathbuf)), nohtml(title));
    }
    if (id[0]) {
        printf("<td>%s</td>", userid_str(id));
    } else {
        printf("<td>&nbsp;</td>");
    }
    printf("<td>%6.6s %s</td></tr>\n", wwwCTime(file_time(buf)) + 4, wwwCTime(file_time(buf)) + 20);
}

void ann_show_directory(char *path)
{
    MENU me;
    ITEM *its;
    int len;
    char board[STRLEN];
    char pathbuf[256];
    char buf[STRLEN];
    int i;

    if (strstr(path, "..") || strstr(path, "SYSHome"))  /* SYSHome? */
        http_fatal("此目录不存在");
    if (path[0] != '\0') {
        len = strlen(path);
        if (path[len - 1] == '/')
            path[len - 1] = '\0';
        if (path[0] == '/')
            snprintf(pathbuf, sizeof(pathbuf), "0Announce%s", path);
        else
            snprintf(pathbuf, sizeof(pathbuf), "0Announce/%s", path);
        if (ann_traverse_check(pathbuf, currentuser) < 0)
            http_fatal("此目录不存在");
    } else
        strcpy(pathbuf, "0Announce");
    if ((its = ann_alloc_items(MAXITEMS)) == NULL)
        http_fatal("分配内存失败");
    ann_set_items(&me, its, MAXITEMS);
    me.path = pathbuf;
    if (ann_load_directory(&me) == 0) {
        board[0] = '\0';
        ann_get_board(path, board, sizeof(board));
        buf[0] = '\0';
        if (board[0] != '\0')
            sprintf(buf, "%s版", board);
        printf("<center>\n");
        printf("%s -- %s精华区<hr class=\"default\">\n", BBSNAME, buf);
        if (me.num <= 0) {
            printf("<br>&lt;&lt; 目前没有文章 &gt;&gt;\n");
            http_quit();
        }
        printf("<table class=\"default\" border=\"1\" width=\"610\">\n");
        printf("<tr><td>编号</td><td>类别</td><td>标题</td><td>整理</td><td>日期</td></tr>\n");
        me.now = 0;
        for (i = 0; i < me.num; i++) {
            trim(me.item[i]->title);
            ann_show_item(&me, me.item[i]);
            me.now++;
        }
        printf("</table><br>[<a href=\"javascript:history.go(-1)\">返回上一页</a>] ");
        if (board[0])
            printf("[<a href=\"/bbsdoc.php?board=%s\">本讨论区</a>]\n", encode_url(buf, board, sizeof(buf)));
        printf("</center>\n");
    }
    ann_free_items(its, MAXITEMS);
}

int main()
{
    char path[512];

    init_all();
    strsncpy(path, getparm("path"), 511);
    ann_show_directory(path);
    http_quit();
}
