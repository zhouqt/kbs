/*
 * $Id$
 */
#include "bbslib.h"

int main()
{
    FILE *fp;
    char filename[80], dir[80], board[80], title[80], buf[80], oldfilename[80], *content;
    int r, i, sig;
    struct fileheader x, *oldx;
    bcache_t *brd;
    int local, anony;

    init_all();
    if (!loginok)
        http_fatal("匆匆过客不能发表文章，请先登录");
    strsncpy(board, getparm("board"), 18);
    strsncpy(title, getparm("title"), 50);
    strsncpy(oldfilename, getparm("refilename"), 80);
    brd = getbcache(board);
    if (brd == 0)
        http_fatal("错误的讨论区名称");
    strcpy(board, brd->filename);
    for (i = 0; i < strlen(title); i++) {
        if (title[i] <= 27 && title[i] >= -1)
            title[i] = ' ';
    }
    sig = atoi(getparm("signature"));
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
    sprintf(filename, "tmp/%s.%d.tmp", getcurruserid(), getpid());
    f_append(filename, unix_string(content));
    oldx = (struct fileheader*)malloc(sizeof(struct fileheader));
    if(oldfilename[0]){
    	int pos = search_record(dir, oldx, sizeof(fileheader), (RECORD_FUNC_ARG) cmpname, oldfilename);
    	if (pos <= 0) oldx = NULL;
    }
    brc_initial(currentuser->userid, board);
    r = post_article(board, title, filename, currentuser, fromhost, sig, local, anony, oldx);
    if (r <= 0)
        http_fatal("内部错误，无法发文");
    brc_update(currentuser->userid);
    free(oldx);
    unlink(filename);
    sprintf(buf, "bbsdoc?board=%s", board);
    if (!junkboard(board)) {
        currentuser->numposts++;
        write_posts(currentuser->userid, board, title);
    }
    redirect(buf);
    return 0;
}
