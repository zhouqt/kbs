#include "bbslib.h"

int main()
{
	int i; 
	int fnum;
	char filename[STRLEN];
	friends_t *frnds;

	init_all();
   	if(!loginok)
	   	http_fatal("您尚未登录, 请先登录");
	sethomefile(filename,currentuser->userid, "friends");
	fnum = get_num_records(filename, sizeof(friends_t));
	if (fnum <= 0)
		http_fatal("您尚未设定好友名单");
	if ((!HAS_PERM(currentuser,PERM_ACCOUNTS) && !HAS_PERM(currentuser,PERM_SYSOP)))
		fnum = (fnum >= MAXFRIENDS) ? MAXFRIENDS : fnum;
	frnds = (friends_t *)calloc(sizeof(friends_t), fnum);
	get_records(filename, frnds, sizeof(friends_t), 1, fnum);
   	printf("<center>\n");
   	printf("%s -- 好友名单 [使用者: %s]<hr color=\"green\"><br>\n",
		   	BBSNAME, currentuser->userid);
   	printf("您共设定了 %d 位好友<br>", fnum);
   	printf("<table border=\"1\"><tr><td>序号</td><td>好友代号</td><td>好友说明</td><td>删除好友</td></tr>");
   	for(i = 0; i < fnum; i++)
   	{
		printf("<tr><td>%d</td>", i+1);
		printf("<td><a href=\"bbsqry?userid=%s\">%s</a></td>",
			   	frnds[i].id, frnds[i].id);
		printf("<td>%s</td>\n", nohtml(frnds[i].exp));
		printf("<td>[<a onclick=\"return confirm('确实删除吗?')\" href=\"bbsfdel?userid=%s\">删除</a>]</td></tr>", frnds[i].id);
	}
   	printf("</table><hr color=\"green\">\n");
	printf("[<a href=\"bbsfadd\">添加新的好友</a>]</center>\n");
	http_quit();
}
