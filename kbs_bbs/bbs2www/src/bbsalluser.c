#include "bbslib.h"

#define BBS_PAGESIZE    (20)

struct userec *get_passwd_addr()
{
	int unum;

	unum = searchuser(currentuser->userid);
	if (unum == 0)
		return NULL;
	else
		return currentuser - unum + 1;
}

int main()
{
	char buf[32];
	struct userec *x;
	int total=0;
	int p, i, start;

	init_all();
	total = allusers();
	x = get_passwd_addr();
	getfriendstr();
	printf("<center>%s -- 所有使用者列表 [用户总数: %d]<hr color=\"green\">\n", BBSNAME, total);
	start=atoi(getparm("start"));
	if(start>total || start<0)
		start=0;
	printf("<table width=\"610\" border=\"1\">\n");
	printf("<tr><td>编号<td>好友<td>使用者代号<td>昵称<td>上站次数<td>文章数<td>最近光临时间\n");
	for(i=0; i<BBS_PAGESIZE; i++)
	{
		if (start+i > total-1)
			break;
		if (x[start+i].userid[0] == '\0')
			continue;
		printf("<tr><td>%d<td>%s<td><a href=\"bbsqry?userid=%s\">%s</a><td>%s<td>%d<td>%d<td>%s\n",
			start+i+1, isfriend(x[start+i].userid) ? "<font color=\"green\">√</font>" : " ",
			x[start+i].userid, x[start+i].userid,
			nohtml(x[start+i].username), x[start+i].numlogins,
			x[start+i].numposts, Ctime(x[start+i].lastlogin)+4);
	}
	printf("</table><hr color=\"green\">\n");
	if(start>0)
		printf("[<a href=\"bbsalluser?start=0\">第一页</a>] ");
	if(start>0)
		printf("[<a href=\"bbsalluser?start=%d\">上一页</a>] ",
				start-BBS_PAGESIZE<0 ? 0 : start-BBS_PAGESIZE);
	if(start<total-(BBS_PAGESIZE-1))
		printf("[<a href=\"bbsalluser?start=%d\">下一页</a>]",
				start+BBS_PAGESIZE);
	if(start<total-(BBS_PAGESIZE-1))
		printf("[<a href=\"bbsalluser?start=%d\">最后一页</a>]\n",
				total-(BBS_PAGESIZE-1));
	printf("<form action=\"bbsalluser\">\n");
	printf("<input type=\"submit\" value=\"跳转到\">");
	printf("第<input name=\"start\" type=\"text\" size=\"5\">个使用者\n");
	printf("</form>\n");
	http_quit();
}
