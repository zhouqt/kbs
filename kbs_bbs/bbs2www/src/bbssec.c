/*
 * $Id$
 */
#include "bbslib.h"

int main()
{
   	int i;

   	init_all();
	printf("<style type=\"text/css\">A {color: #0000f0}</style>");
	printf("<center>\n");
   	printf("%s -- 分类讨论区 <hr color=\"green\">\n", BBSNAME);
   	printf("<table>\n");
   	printf("<tr><td>区号</td><td>类别</td><td>描述</td></tr>\n");
   	for(i=0; i<SECNUM; i++)
	{
		printf("<tr><td>%d</td><td><a href=\"bbsboa?group=%d\">%s</a></td>",
				i, i, secname[i][0]);
		printf("<td><a href=\"bbsboa?group=%d\">%s</a></td></tr>\n",
				i, secname[i][1]);
   	}
   	printf("</table><hr>\n");
   	printf("[<a href=\"bbsall\">全部讨论区</a>]");
   	printf("[<a href=\"bbs0an\">精华公布栏</a>]");
   	printf("</center>\n");
	http_quit();
}
