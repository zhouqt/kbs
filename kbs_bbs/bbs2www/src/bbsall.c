#include "bbslib.h"

int cmp_board(b1, b2)
struct boardheader *b1, *b2;
{
	return strcasecmp(b1->filename, b2->filename);
}

int main()
{
	bcache_t data[MAXBOARD], *x;
	int i, total=0;
	char *ptr;
	bcache_t *bc;

	init_all();
	bc = getbcacheaddr();
	printf("<style type=text/css>A {color: #0000f0}</style>");
	for(i=0; i<MAXBOARD; i++) {
		x=&(bc[i]);
		if(x->filename[0]<=32 || x->filename[0]>'z') continue;
		if(!has_read_perm(currentuser, x->filename)) continue;
		memcpy(&data[total], x, sizeof(struct boardheader));
		total++;
	}
	qsort(data, total, sizeof(struct boardheader), cmp_board);
	printf("<center>\n");
	printf("%s -- 全部讨论区 [讨论区数: %d]<hr color=green>", BBSNAME, total);
	printf("<table width=610>\n");
	printf("<tr><td>序号<td>讨论区名称<td>类别<td>中文描述<td>版主\n");
	for(i=0; i<total; i++) {
		printf("<tr><td>%d", i+1);
		printf("<td><a href=bbsdoc?board=%s>%s</a>", data[i].filename, data[i].filename);
		printf("<td>%6.6s", data[i].title+1);
		printf("<td><a href=bbsdoc?board=%s>%s</a>", data[i].filename, data[i].title+7);
		ptr=strtok(data[i].BM, " ,;");
		if(ptr==0) ptr="诚征版主中";
		printf("<td><a href=bbsqry?userid=%s>%s</a>", ptr, ptr);
	}
   	printf("</table><hr></center>\n");
	http_quit();
}
