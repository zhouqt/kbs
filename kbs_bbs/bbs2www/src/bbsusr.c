#include "bbslib.h"

int my_t_lines;

int get_online_users()
{
	int i2 = 0;

	apply_ulist_addr((APPLY_UTMP_FUNC)full_utmp,(char*)&i2);
	return i2;
}

void display_online_users(int start, int total, int my_t_lines)
{
	int i;
	uinfo_t **user;

	user = get_ulist_addr();
	if(my_t_lines<10 || my_t_lines>40) my_t_lines=20;
	if(start>total-5) start=total-5;
	if(start<0) start=0;
	printf("<table border=\"1\" width=\"610\">\n");
	printf("<tr><td>序号</td><td>友</td><td>使用者代号</td><td>使用者昵称</td><td>来自</td><td>动态</td><td>发呆</td></tr>\n");
	for(i=start; i<start+my_t_lines && i<total; i++)
	{
		int dt=(time(0)-get_idle_time(user[i]))/60;
		printf("<tr><td>%d</td>", i+1);
		printf("<td>%s", isfriend(user[i]->userid) ? "√" : "  ");
		printf("%s</td>", 
				user[i]->invisible ? "<font color=\"green\">C</font>" : " ");
		printf("<td><a href=\"bbsqry?userid=%s\">%s</a></td>", 
				user[i]->userid, user[i]->userid);
		printf("<td><a href=\"bbsqry?userid=%s\">%24.24s </a></td>", 
				user[i]->userid, nohtml(user[i]->username));
		printf("<td>%20.20s </td>", user[i]->from);
		printf("<td>%s</td>", 
				user[i]->invisible ? "隐身中..." : ModeType(user[i]->mode));
		if(dt==0) {
			printf("<td> \n");
		} else {
			printf("<td>%d\n", dt);
		}
		printf("</td></tr>\n");
	}
	printf("</table>\n");
}

int main()
{
	int i, start, total, fh, shmkey, shmid; 
	char search[80];

	init_all();
	if (loginok)
		getfriendstr();
	printf("<center>\n");
	printf("%s -- 在线用户列表 [目前在线: %d人]<hr>\n",
			BBSNAME, count_online());
	total = get_online_users();
	start=atoi(getparm("start"));
	my_t_lines=atoi(getparm("my_t_lines"));
	if(my_t_lines<10 || my_t_lines>40)
		my_t_lines = 20;
	display_online_users(start, total, my_t_lines);
	printf("<hr>");
	printf("[<a href=\"bbsufind?search=*\">全部</a>] ");
	for(i='A'; i<='Z'; i++)
		printf("[<a href=\"bbsufind?search=%c\">%c</a>]", i, i);
	printf("<br>\n");
	printf("[<a href=\"bbsfriend\">在线好友</a>] ");
	if(start>0)
		printf("[<a href=\"bbsusr?start=%d\">上一页</a>]", start-20);
	if(start<total-my_t_lines)
		printf("[<a href=\"bbsusr?start=%d\">下一页</a>]", start+my_t_lines);
	printf("<br><form action=\"bbsusr\">\n");
	printf("<input type=\"submit\" value=\"跳转到第\"> ");
	printf("<input type=\"input\" size=\"4\" name=\"start\"> 个使用者</form>");
	printf("</center>\n");
	http_quit();
}
