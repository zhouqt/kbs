/*
 * $Id$
 */
#include "bbslib.h"

int valid_fname(str)
char *str;
{
    char ch;

	if(strstr(str,"..")) return 0;

    while ((ch = *str++) != '\0') {
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || strchr("0123456789@[]-._", ch) != NULL) {
            ;
        } else {
            return 0;
        }
    }
    return 1;
}

void ann_show_item(MENU * pm, ITEM * it)
{
    char title[STRLEN];
    char *id;
    char buf[256];
    char pathbuf[256];
	char title_en[256];
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
	encode_html(title_en, title, sizeof(title_en));
	if (!file_exist(buf)) {
        printf("<td>[错误] </td><td>%s</td>", title_en);
    } else if (file_isdir(buf)) {
        snprintf(pathbuf, sizeof(pathbuf), "%s/%s", ptr == NULL ? "" : ptr, it->fname);
        printf("<td>[目录] </td><td><a href=\"bbs0an?path=%s\">%s</a></td>", 
				http_encode_string(pathbuf, sizeof(pathbuf)), 
				title_en);
    } else {
        snprintf(pathbuf, sizeof(pathbuf), "%s/%s", ptr == NULL ? "" : ptr, it->fname);
        printf("<td>[%s] </td><td><a href=\"/bbsanc.php?path=%s\">%s</a></td>", 
				it->attachpos?"附件":"文件",
				http_encode_string(pathbuf, sizeof(pathbuf)), 
				title_en);
    }
    if (id[0]) {
        printf("<td>%s</td>", userid_str(id));
    } else {
        printf("<td>&nbsp;</td>");
    }
    printf("<td>%6.6s %s</td></tr>\n", wwwCTime(file_time(buf)) + 4, wwwCTime(file_time(buf)) + 20);
}

void ann_show_toolbar(char * path, char * board)
{
    struct boardheader *bp;
    char buf[STRLEN];

		printf("<br>[<a href=\"javascript:history.go(-1)\">返回上一页</a>] ");
        if (board[0]){
            printf("[<a href=\"/bbsdoc.php?board=%s\">本讨论区</a>]\n", encode_url(buf, board, sizeof(buf)));
			if((bp=getbcache(board))!=NULL){
    			char BM[STRLEN];

			    strncpy(BM, bp->BM, sizeof(BM) - 1);
			    BM[sizeof(BM) - 1] = '\0';
    			if(chk_currBM(BM,currentuser)){
					printf("[<a href=\"/bbsmpath.php?action=add&path=%s\">加为丝路</a>]<br>",encode_url(buf,path,sizeof(buf)));
					printf("<form action=\"bbs0an\" method=\"post\">");
					printf("<input type=\"hidden\" name=\"action\" value=\"add\">");
					printf("<input type=\"hidden\" name=\"path\" value=\"%s\">",path);
					printf("目录名(只能英文和数字)<input type=\"text\" name=\"dpath\">");
					printf("标题<input type=\"text\" name=\"fname\">");
					printf("<input type=\"submit\" name=\"submit\" value=\"添加目录\">");
					printf("</form>");
				}
			}
		}
}

void ann_show_directory(char *path,char * pathbuf)
{
    MENU me;
    ITEM *its;
    char board[STRLEN];
    char buf[STRLEN];
    int i;

    if ((its = ann_alloc_items(MAXITEMS)) == NULL)
        http_fatal("分配内存失败");
    ann_set_items(&me, its, MAXITEMS);
    me.path = pathbuf;
    if (ann_load_directory(&me) == 0) {
        board[0] = '\0';
        ann_get_board(pathbuf, board, sizeof(board));
        buf[0] = '\0';
        if (board[0] != '\0')
            sprintf(buf, "%s版", board);
		else
			buf[0]='\0';
        printf("<center>\n");
        printf("%s -- %s精华区<hr class=\"default\">\n", BBSNAME, buf);
        if (me.num <= 0) {
            printf("<br>&lt;&lt; 目前没有文章 &gt;&gt;\n");
			ann_show_toolbar(path,board);
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
        printf("</table>");
		ann_show_toolbar(path,board);
        printf("</center>\n");
    }
    ann_free_items(its, MAXITEMS);
}

int ann_check_dir(char * path,char *pathbuf)
{
	int len;

    if (strstr(path, "..") || strstr(path, "SYSHome"))  /* SYSHome? */
		return 0;
    if (path[0] != '\0') {
        len = strlen(path);
        if (path[len - 1] == '/')
            path[len - 1] = '\0';
        if (path[0] == '/')
            snprintf(pathbuf, 255, "0Announce%s", path);
        else
            snprintf(pathbuf, 255, "0Announce/%s", path);
        if (ann_traverse_check(pathbuf, currentuser) < 0)
			return 0;
    } else
        strcpy(pathbuf, "0Announce");

	return 1;
}

int ann_check_bm(char *buf)
{

	char board[STRLEN];
	char *c;


	if(strncmp(buf,"0Announce/groups/",17))
		return 0;
	if((c=strchr(buf+17,'/'))==NULL) return 0;
	strncpy(board,c+1,STRLEN-1);
	board[STRLEN-1]='\0';
	if((c=strchr(board,'/'))!=NULL) *c='\0';

	if(!has_BM_perm(currentuser, board))
		return 0;

	return 1;
}

int ann_add_dir(char *path,char *pathbuf,char *fname,char *title)
{
	char fpath[PATHLEN];
	char fpath2[PATHLEN];
	char buf[STRLEN];
	MENU pm;
	FILE *fp;

	if(ann_check_bm(pathbuf) == 0)
		return -1;

	if(!valid_fname(fname))
		return -2;

	sprintf(fpath, "%s/%s", pathbuf, fname);

	if(dashf(fpath) || dashd(fpath))
		return -3;

	mkdir(fpath, 0755);
	chmod(fpath, 0755);

	bzero(&pm,sizeof(pm));
	pm.path=pathbuf;
	a_loadnames(&pm);

	sprintf(buf,"%-38.38s",title);

	a_additem(&pm,buf,fname,NULL,0,0);

	if(a_savenames(&pm))
		return -4;

	sprintf(fpath, "%s/%s/.Names", pathbuf, fname);
	if((fp=fopen(fpath,"w"))==NULL)
		return -5;

	fprintf(fp, "#\n");
	fprintf(fp, "# Title=%s\n", buf);
	fprintf(fp, "#\n");
	fclose(fp);

	return 1;

}

int main()
{
    char path[512];
	char *c;
	char action[10];
	char dpath[PATHLEN];
	char title[STRLEN];
    char pathbuf[256];
	int ret;

    init_all();
    strsncpy(path, getparm("path"), 511);

	if(! ann_check_dir(path,pathbuf))
        http_fatal("此目录不存在");

	c = getparm("action");

	if(c && *c){
		strncpy(action,c,9);
		action[9]='\0';
		if(! strcmp(action,"add") ){
			c = getparm("dpath");
			if(!c || ! *c)
				http_fatal("目录路径错误");
			strncpy(dpath,c,PATHLEN-1);
			dpath[PATHLEN-1]='\0';

			c = getparm("fname");
			if(!c || ! *c)
				http_fatal("目录题目错误");
			strncpy(title,c,STRLEN-1);
			title[STRLEN-1]='\0';

			ret = ann_add_dir(path,pathbuf,dpath,title);
			if(ret < 0){
				switch(ret){
				case -1:
					http_fatal("您没有这个权限");
					break;
				case -2:
					http_fatal("文件名不合法");
					break;
				case -3:
					http_fatal("目录已经存在");
					break;
				default:
					http_fatal("系统错误");
					break;
				}
			}
		}
	}

    ann_show_directory(path,pathbuf);
    http_quit();
}
