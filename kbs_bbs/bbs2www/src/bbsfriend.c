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
	int i, total=0, fh, shmkey, shmid; 
	uinfo_t *x;
	uinfo_t **usr;

	init_all();
	if (!loginok)
		http_fatal("匆匆过客没有好友列表.");
	getfriendstr();
	set_friendmode(1);
	fill_userlist();
	usr = get_ulist_addr();
	printf("<center>\n");
	printf("%s -- 在线好友列表 [使用者: %s]<hr>\n", BBSNAME,
			currentuser->userid);
	for(i=0; i < USHM_SIZE; i++)
	{
		x=usr[i];
		if(x == NULL || x->active==0)
			continue;
		if(x->active==0) continue;
		if(x->invisible && !HAS_PERM(currentuser,PERM_SEECLOAK)) continue;
		memcpy(&user[total], x, sizeof(uinfo_t));
		total++;
	}
	printf("<table border=\"1\" width=\"610\">\n");
	printf("<tr><td>序号<td>友<td>使用者代号<td>使用者昵称<td>来自<td>动态<td>发呆\n");
	qsort(user, total, sizeof(uinfo_t), cmpuser);
	for(i=0; i<total; i++)
	{
		int dt=(time(0)-get_idle_time(&user[i]))/60;
		printf("<tr><td>%d", i+1);
		printf("<td>%s", "√");
		printf("%s", user[i].invisible ? "<font color=\"green\">C</font>" : " ");
		printf("<td><a href=\"bbsqry?userid=%s\">%s</a>", user[i].userid, user[i].userid);
		printf("<td><a href=\"bbsqry?userid=%s\">%24.24s </a>", user[i].userid, nohtml(user[i].username));
		printf("<td>%20.20s ", user[i].from);
		printf("<td>%s", user[i].invisible ? "隐身中..." : ModeType(user[i].mode));
		if(dt==0) {
			printf("<td> \n");
		} else {
			printf("<td>%d\n", dt);
		}
	}
	printf("</table>\n");
	if(total==0) printf("目前没有好友在线");
	printf("<hr>");
	printf("[<a href=\"bbsfall\">全部好友名册</a>]");
	printf("</center>\n");
	http_quit();
}
