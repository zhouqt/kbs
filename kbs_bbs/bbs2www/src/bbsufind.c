#include "bbslib.h"

uinfo_t user[USHM_SIZE];

int cmpuser(a, b)
uinfo_t *a, *b;
{
	char id1[80], id2[80];
	sprintf(id1, "%d%s", !isfriend(a->userid), a->userid);
	sprintf(id2, "%d%s", !isfriend(b->userid), b->userid);
	return strcasecmp(id1, id2);
}

int main()
{
	int i, start, total=0, total2=0, fh, shmkey, shmid; 
	uinfo_t *x;
	char search;
	uinfo_t **usr;

	init_all();
	if (loginok)
		getfriendstr();
	fill_userlist();
	usr = get_ulist_addr();
	printf("<center>\n");
	printf("%s -- 在线用户查询 [在线总人数: %d人]<hr>\n", BBSNAME, count_online());
	for(i=0; i < USHM_SIZE; i++)
	{
		x=usr[i];
		if(x == NULL || x->active==0)
			continue;
		if(x->invisible && !HAS_PERM(currentuser,PERM_SEECLOAK))
			continue;
		memcpy(&user[total], x, sizeof(uinfo_t));
		total++;
	}
	search=toupper(getparm("search")[0]);
	if(search!='*' && (search<'A' || search>'Z'))
		http_fatal("错误的参数");
	if(search=='*') {
		printf("所有在线使用者<br>\n");
	} else {
		printf("字母'%c'开头的在线使用者.<br>\n", search);
	}

	printf("<table border=\"1\" width=\"610\">\n");
	printf("<tr><td>序号<td>友<td>使用者代号<td>使用者昵称<td>来自<td>动态<td>发呆\n");
	qsort(user, total, sizeof(uinfo_t), cmpuser);
	for(i=0; i<total; i++)
	{
		int dt=(time(0)-get_idle_time(&user[i]))/60;

		if(toupper(user[i].userid[0])!=search && search!='*')
			continue;
		printf("<tr><td>%d</td>", i+1);
		printf("<td>%s", isfriend(user[i].userid) ? "√" : "  ");
		printf("%s</td>",
				user[i].invisible ? "<font color=\"green\">C</font>" : " ");
		printf("<td><a href=\"bbsqry?userid=%s\">%s</a></td>", 
				user[i].userid, user[i].userid);
		printf("<td><a href=\"bbsqry?userid=%s\">%24.24s </a></td>",
				user[i].userid, nohtml(user[i].username));
		printf("<td>%20.20s </td>", user[i].from);
		printf("<td>%s</td>",
				user[i].invisible ? "隐身中..." : ModeType(user[i].mode));
		if(dt==0)
			printf("<td> </td></tr>\n");
		else
			printf("<td>%d</td></tr>\n", dt);
		total2++;
	}
	printf("</table>\n");
	printf("本项在线: %d人", total2);
	printf("<hr>");
	if(search!='*')
		printf("[<a href=\"bbsufind?search=*\">全部</a>] ");
	for(i='A'; i<='Z'; i++)
	{
		if(i==search)
			printf("[%c]", i);
		else
			printf("[<a href=\"bbsufind?search=%c\">%c</a>]", i, i);
	}
    printf("<br>\n");
	printf("[<a href=\"javascript:history.go(-1)\">返回</a>] [<a href=\"bbsusr\">一般模式</a>] ");
	printf("</center>\n");
	http_quit();
}
