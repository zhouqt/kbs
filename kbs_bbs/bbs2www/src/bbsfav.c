/*
 * $Id$
 */
#include "bbslib.h"

struct favbrd_struct {
    int flag;
    char *title;
    int father;
};
extern struct favbrd_struct favbrd_list[FAVBOARDNUM];
extern int favbrd_list_t;
extern int favnow;

/*static int cmp_board(struct boardheader *b1, struct boardheader *b2)
{
	return strcasecmp(b1->filename, b2->filename);
}*/

/*
 * from boards_t.c
*/
int cmpboard(brd, tmp)          /*排序用 */
    struct newpostdata *brd, *tmp;
{
    register int type = 0;

    if (!(currentuser->flags & BRDSORT_FLAG)) {
        type = brd->title[0] - tmp->title[0];
        if (type == 0)
            type = strncasecmp(brd->title + 1, tmp->title + 1, 6);

    }
    if (type == 0)
        type = strcasecmp(brd->name, tmp->name);
    return type;
}

static int check_newpost(struct newpostdata *ptr)
{
#ifdef HAVE_BRC_CONTROL
    struct BoardStatus *bptr;

    ptr->total = ptr->unread = 0;

    bptr = getbstatus(ptr->pos+1);
    if (bptr == NULL)
        return 0;
    ptr->total = bptr->total;

    if (!brc_initial(currentuser->userid, ptr->name)) {
        ptr->unread = 1;
    } else {
        if (brc_unread(bptr->lastpost)) {
            ptr->unread = 1;
        }
    }
    return 1;
#else
    ptr->unread = 1;
    return 1;
#endif
}

/*
 * Return value:
 *     board number loaded for success
 *     -1                  for error
*/
int brd_show_boards(int sec, int yank)
{
    char buf[256];
    struct newpostdata newpost_buffer[MAXBOARD];
    struct newpostdata *ptr;
    char *ptr2;
    int i;
    int brdnum;
    int yank_flag;

    yank_flag = yank;
    if (!strcmp(currentuser->userid, "guest"))
        yank_flag = 1;
    brdnum = 0;
    if ((brdnum=fav_loaddata(newpost_buffer,sec,1,FAVBOARDNUM,1,NULL)) == -1)
        return -1;
/*    qsort(nbrd, brdnum, sizeof(nbrd[0]), (int (*)(const void *, const void *)) cmpboard);
    */
    printf("<style type=\"text/css\">A {color: #0000f0}</style>");
    printf("<center>\n");
    printf("%s -- 个人定制区 [%s]", BBSNAME, (sec == -1) ? "根目录" : favbrd_list[sec].title);
/*	if (yank_flag == 0)
	{
		printf("[<a href=\"bbsboa?group=%d&yank=1\">本区所有看版</a>]",
			   sec, yank_flag);
	}
	else
		printf("[<a href=\"bbsboa?group=%d\">本区已订阅看版</a>]", sec);*/
    printf("[<a href=\"/bbssec.php\">分类讨论区</a>]\n");
    printf("<hr color=\"green\">\n");
    printf("<table width=\"610\">\n");
    printf("<tr><td>序号</td><td>未</td><td>讨论区名称</td><td>类别</td><td>中文描述</td><td>版主</td><td>文章数</td></tr>\n");
    if (sec != -1)
        printf("<tr><td></td><td><img src=\"/images/folder.gif\"></td><td>"
               "<a target=\"f3\" href=\"bbsfav?select=%d\">返回上一级</a></td>" "<td></td><td></td><td></td><td></td></tr>\n", favbrd_list[sec].father);
    for (i = 0; i < brdnum; i++) {
        ptr = &newpost_buffer[i];
        if (ptr->flag == -1) {
            int j;

            j = ptr->tag;
            if (j != -1) {
                printf("<tr><td>%d</td><td><img src=\"/images/folder.gif\"></td><td>"
                       " <a target=\"f3\" href=\"bbsfav?select=%d\">%s</a></td>"
                       "<td></td><td></td><td></td><td></td>" "<td><a target=\"f3\" href=\"bbsfav?select=%d&delete=%d\">删除</a></td></tr>\n", i + 1, j, ptr->title, sec, j);
            }
        } else {
            check_newpost(ptr);
            printf("<tr><td>%d</td><td>%s</td>", i + 1, ptr->unread ? "◆" : "◇");
            encode_url(buf, ptr->name, sizeof(buf));
            printf("<td>%c<a href=\"/bbsdoc.php?board=%s\">%s</a></td>", ptr->zap ? '*' : ' ', buf, ptr->name);
            printf("<td>%6.6s</td>", ptr->title + 1);
            printf("<td><a href=\"/bbsdoc.php?board=%s\">%s</a></td>", buf, ptr->title + 7);
            strncpy(buf, ptr->BM, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            if (buf[0] <= ' ') {
                printf("<td>诚征版主中</td>");
            } else {
                if ((ptr2 = strchr(buf, ' ')) != NULL)
                    *ptr2 = '\0';
                if (!isalpha(buf[0]))
                    printf("<td>%s</td>", buf);
                else
                    printf("<td><a href=\"bbsqry?userid=%s\">%s</a></td>", buf, buf);
            }
            printf("<td>%d</td>", ptr->total);
            printf("<td><a target=\"f3\" href=\"bbsfav?select=%d&delete=%d\">删除</a></td></tr>", sec, IsFavBoard(ptr->pos) - 1);
            printf("</tr>\n");
        }
    }
    printf("</table>");
    printf("<form action=bbsfav>增添目录<input name=dname size=24 maxlength=20 type=text value=''> " "<input type=submit value=确定><input type=hidden name=select value=%d></form>", sec);
    printf("<form action=bbsfav>增添版面<input name=bname size=24 maxlength=20 type=text value=''> " "<input type=submit value=确定><input type=hidden name=select value=%d></form>", sec);
    printf("<hr></center>\n");
    return brdnum;
}

int main()
{
    int sec;
    char *s;

    init_all();
    sec = atoi(getparm("select"));
    load_favboard(0);
    if (sec < -1 || sec >= favbrd_list_t)
        http_fatal("错误的参数");
    SetFav(sec);
    if (strcmp((s = getparm("delete")), "")) {
        DelFavBoard(atoi(s));
        save_favboard();
    }
    if (strcmp((s = getparm("dname")), "")) {
        if (strlen(s) <= 20) {
            addFavBoardDir(0, s);
            save_favboard();
        }
    }
    if (strcmp((s = getparm("bname")), "")) {
        int i;

        if (s != NULL && *s) {
            i = getbnum(s);
            if (i > 0 && !IsFavBoard(i - 1)) {
                addFavBoard(i - 1);
                save_favboard();
            }
        }
    }
    /*
       if (atoi(getparm("my_def_mode"))==0)
       cgi="bbstdoc";
     */
    brd_show_boards(sec, 2);
    release_favboard();
    http_quit();
}
