#include "bbslib.h"

int main() {
	FILE *fp;
	char buf[256], tmp[256], name[256], cname[256], cc[256];
	int i, r;
	init_all();
	fp=fopen("0Announce/bbslist/board2", "r");
	if(fp==0) http_fatal("error 1");
	printf("<center>%s -- 热门讨论区<hr color=green>", BBSNAME);
	printf("<table border=1>\n");
	printf("<tr><td>名次<td>版名<td>中文版名<td>人气\n");
	for(i=0; i<=15; i++) {
		if(fgets(buf, 150, fp)==0) break;
		if(i==0) continue;
		r=sscanf(buf, "%s %s %s %s %s %s", tmp, tmp, name, tmp, cname, cc);
		if(r==6) {
			printf("<tr><td>%d<td><a href=bbsdoc?board=%s>%s</a><td width=200><a href=bbsdoc?board=%s>%s</a><td>%s\n",
				i, name, name, name, cname, cc);
		}
	}
	printf("</table>\n</center>\n");
	fclose(fp);
}
