/*
 * $Id$
 */
#include "bbslib.h"

extern int     brdnum;
extern int yank_flag;

/*static int cmp_board(struct boardheader *b1, struct boardheader *b2)
{
	return strcasecmp(b1->filename, b2->filename);
}*/

/*
 * from boards_t.c
*/
int
cmpboard( brd, tmp ) /*排序用*/
struct newpostdata      *brd, *tmp;
{
	register int        type = 0;

	if ( !(currentuser->flags[0] & BRDSORT_FLAG) )
	{
		type = brd->title[0] - tmp->title[0];
		if (type==0)
			type=strncasecmp( brd->title+1, tmp->title+1,6);

	}
	if ( type == 0 )
		type = strcasecmp( brd->name, tmp->name );
	return type;
}

static int check_newpost( struct newpostdata *ptr)
{
	struct BoardStatus* bptr;
	ptr->total = ptr->unread = 0;

	bptr = getbstatus(ptr->pos);
	if (bptr == NULL)
		return 0;
	ptr->total = bptr->total;

	if (!brc_initial(currentuser->userid,ptr->name))
	{
		ptr->unread = 1;
	}
	else
	{
		if (brc_unread(bptr->lastpost))
		{
			ptr->unread = 1;
		}
	}
	return 1;
}

/*
 * Return value:
 *     board number loaded for success
 *     -1                  for error
*/
int brd_show_boards(int sec, char *cgi, int yank)
{
	char        buf[ 256 ];
	struct newpostdata newpost_buffer[ MAXBOARD ];
	struct newpostdata  *ptr;
	char *ptr2;
	int i;
	char* boardprefix;

	sprintf( buf, "EGROUP%d", sec);
	boardprefix = sysconf_str( buf );
	yank_flag = yank;
	if ( !strcmp( currentuser->userid, "guest" ) )
		yank_flag = 1;
	if(yank != 0)
		yank_flag = 1;
	nbrd = newpost_buffer;
	brdnum = 0;
	if (load_boards() == -1)
		return -1;
	qsort( nbrd, brdnum, sizeof( nbrd[0] ), 
		   (int (*)(const void *, const void *))cmpboard );
	printf("<style type=\"text/css\">A {color: #0000f0}</style>");
	printf("<center>\n");
	printf("%s -- 分类讨论区 [%s]", BBSNAME, secname[sec]);
	if (yank_flag == 0)
	{
		printf("[<a href=\"bbsboa?group=%d&yank=1\">本区所有看版</a>]",
			   sec, yank_flag);
	}
	else
		printf("[<a href=\"bbsboa?group=%d\">本区已订阅看版</a>]", sec);
	printf("[<a href=\"bbssec\">分类讨论区</a>]\n");
	printf("<hr color=\"green\">\n");
	printf("<table width=\"610\">\n");
	printf("<tr><td>序号</td><td>未</td><td>讨论区名称</td><td>类别</td><td>中文描述</td><td>版主</td><td>文章数</td></tr>\n");
	for (i=0; i<brdnum; i++)
	{
		ptr = &nbrd[i];
		check_newpost( ptr );
		printf("<tr><td>%d</td><td>%s</td>", i+1, ptr->unread ? "◆" : "◇");
		encode_url(buf, ptr->name, sizeof(buf));
		printf("<td>%c<a href=\"%s?board=%s\">%s</a></td>",
			   ptr->zap ? '*' : ' ', cgi, buf, ptr->name);
		printf("<td>%6.6s</td>", ptr->title+1);
		printf("<td><a href=\"%s?board=%s\">%s</a></td>",
				cgi, buf, ptr->title+7);
		strncpy(buf, ptr->BM, sizeof(buf)-1);
		buf[sizeof(buf)-1] = '\0';
		if (buf[0] <= ' ')
		{
			printf("<td>诚征板主中</td>");
		}
		else
		{
			if ((ptr2 = strchr(buf, ' ')) != NULL)
				*ptr2 = '\0';
			if (!isalpha(buf[0]))
				printf("<td>%s</td>", buf);
			else
				printf("<td><a href=\"bbsqry?userid=%s\">%s</a></td>",
					buf, buf);
		}
		printf("<td>%d</td></tr>\n", ptr->total);
	}
	printf("</table><hr></center>\n");
	return brdnum;
}

int main()
{
	int sec1;
	int yank;
	char *cgi="bbsdoc";

	init_all();
	sec1 = atoi(getparm("group"));
	yank = atoi(getparm("yank"));
	if (sec1<0 || sec1>=SECNUM)
		http_fatal("错误的参数");
	/*
	if (atoi(getparm("my_def_mode"))==0)
		cgi="bbstdoc";
		*/
	brd_show_boards(sec1, cgi, yank);
	http_quit();
}

