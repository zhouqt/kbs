#include "bbslib.h"

int main() {
   	int i;

   	init_all();
	printf("<style type=text/css>A {color: #0000f0}</style>");
	printf("<center>\n");
   	printf("%s -- 分类讨论区 <hr color=green>\n", BBSNAME);
   	printf("<table>\n");
   	printf("<tr><td>区号<td>类别<td>描述\n");
   	for(i=0; i<SECNUM; i++) {
      		printf("<tr><td>%d<td><a href=bbsboa?%d>%s</a>", i, i, secname[i][0]);
      		printf("<td><a href=bbsboa?%d>%s</a>\n", i, secname[i][1]);
   	}
   	printf("</table><hr>\n");
   	printf("[<a href=bbsall>全部讨论区</a>]");
   	printf("[<a href=bbs0an>精华公布栏</a>]");
   	printf("</center>\n");
	http_quit();
}
