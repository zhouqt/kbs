#include "bbslib.h"

struct boardheader data[MAXBOARD];
int mybrdnum=0;

int cmp_board(b1, b2)
struct boardheader *b1, *b2;
{
        return strcasecmp(b1->filename, b2->filename);
}

int main()
{
	int total=0, i, i1, i2, type;
	char path[200];
	char *brd;
   	FILE *fp;
	bcache_t *bc;

	init_all();
	if(!loginok)
		http_fatal("尚未登录或者超时");
	type=atoi(getparm("type"));
	if(type!=0)
	{
		read_submit();
		http_quit();
	}
   	/*setuserfile(path, currentuser->userid,"favboard");
	fp=fopen(path, "r");
   	if(fp) {
		mybrdnum=fread(mybrd, sizeof(mybrd[0]), 30, fp);
   		fclose(fp);
	}*/
	load_favboard(0);
   	printf("<center>\n");
	printf("<style type=\"text/css\">A {color: 000080} </style>\n");
	printf("个人预定讨论区管理(您目前预定了%d个讨论区，最多可预定%d个)<hr>\n",
			get_favboard_count(),FAVBOARDNUM);
	printf("<form action=\"bbsmybrd?type=1&confirm1=1\" method=\"post\">\n");
	printf("<input type=\"hidden\" name=\"confirm1\" value=\"1\">\n");
	printf("<table>\n");
	bc = getbcacheaddr();
	for(i=0; i<MAXBOARD; i++)
	{
		if(has_read_perm(currentuser, bc[i].filename))
		{
			memcpy(&data[total], &(bc[i]), sizeof(struct boardheader));
			total++;
		}
	}
	qsort(data, total, sizeof(struct boardheader), cmp_board);
	for(i=0; i<total; i++)
	{
		char *buf3="";
		if(IsFavBoard(getbnum(data[i].filename)-1))
			buf3=" checked";
		if(i%3==0)
			printf("\n<tr>");
		printf("<td><input type=\"checkbox\" name=\"%s\" %s><a href=\"bbsdoc?board=%s\">%s(%s)</a>", 
			data[i].filename, buf3,data[i].filename,
			data[i].filename, data[i].title+11);
	}
	printf("</table><hr>\n");
	printf("<input type=\"submit\" value=\"确认预定\"> <input type=\"reset\" value=\"复原\">\n");
	printf("</form>\n");
	http_quit();
}

/*int ismyboard(char *board) {
	int i;
	for(i=0; i<mybrdnum; i++)
		if(!strcasecmp(board, mybrd[i])) return 1;
	return 0;
}*/

int read_submit()
{
	int i, i1, i2, n;
	char buf1[200];
	char *brd;
	FILE *fp;
	int mybrdnum=0;

	if(!strcmp(getparm("confirm1"), ""))
		http_fatal("参数错误");
	for(i=0; i<parm_num; i++)
	{
		if(!strcasecmp(parm_val[i], "on"))
		{
			if(mybrdnum >= FAVBOARDNUM)
				http_fatal("您试图预定超过%d个讨论区", FAVBOARDNUM);
			if(!has_read_perm(currentuser, parm_name[i]))
			{
				printf("警告: 无法预定'%s'讨论区<br>\n", nohtml(parm_name[i]));
				continue;
			}
			add_favboard(parm_name[i]);
			mybrdnum++;
		}
	}
	save_favboard();
	printf("<script>top.f2.location='bbsleft'</script>修改预定讨论区成功，您现在一共预定了%d个讨论区:<hr>\n", mybrdnum);
	printf("[<a href=\"javascript:history.go(-2)\">返回</a>]");
}
