#include "bbslib.h"
struct deny {
	char id[80];
	char exp[80];
	int free_time;
} denyuser[256];
int denynum=0;

int loaddenyuser(char *board) {
	FILE *fp;
	char path[80], buf[256];
	sprintf(path, "boards/%s/deny_users", board);
	fp=fopen(path, "r");
	if(fp==0) return;
	while(denynum<100) {
		if(fgets(buf, 80, fp)==0) break;
		sscanf(buf, "%s %s %d", denyuser[denynum].id, denyuser[denynum].exp, &denyuser[denynum].free_time);
		denynum++;
	}
	fclose(fp);
}

int savedenyuser(char *board) {
	FILE *fp;
	int i;
	char path[80], buf[256], *exp;
	sprintf(path, "boards/%s/deny_users", board);
	fp=fopen(path, "w");
	if(fp==0) return;
	for(i=0; i<denynum; i++) {
		int m;
		exp=denyuser[i].exp;
		if(denyuser[i].id[0]==0) continue;
		for(m=0; exp[m]; m++) {
			if(exp[m]<=32 && exp[m]>0) exp[m]='.';
		}
		fprintf(fp, "%-12s %s %d\n", denyuser[i].id, denyuser[i].exp, denyuser[i].free_time);
	}
	fclose(fp);
}

int main() {
	int i; 
	char exp[80], board[80], *userid;
	int dt;
	struct userec *u = NULL;

	init_all();
   	if(!loginok) http_fatal("您尚未登录, 请先登录");
	strsncpy(board, getparm("board"), 30);
	strsncpy(exp, getparm("exp"), 30);
	dt=atoi(getparm("dt"));
	if(!has_read_perm(&currentuser, board)) http_fatal("错误的讨论区");
	if(!has_BM_perm(&currentuser, board)) http_fatal("你无权进行本操作");
	loaddenyuser(board);
	userid=getparm("userid");
	if(userid[0]==0) return show_form(board);
	if(getuser(userid, &u)==0) http_fatal("错误的使用者帐号");
	strcpy(userid, u->userid);
	if(dt<1 || dt>99) http_fatal("请输入被封天数(1-99)");
	if(exp[0]==0) http_fatal("请输入封人原因");
   	for(i=0; i<denynum; i++)
		if(!strcasecmp(denyuser[i].id, userid)) http_fatal("此用户已经被封");
	if(denynum>40) http_fatal("太多人被封了");
	strsncpy(denyuser[denynum].id, userid, 13);
	strsncpy(denyuser[denynum].exp, exp, 30);
	denyuser[denynum].free_time=time(0)+dt*86400;
	denynum++;
	savedenyuser(board);
	printf("封禁 %s 成功<br>\n", userid);
	inform(board, userid, exp, dt);
	printf("[<a href=bbsdenyall?board=%s>返回被封帐号名单</a>]", board);
	http_quit();
}

int show_form(char *board) {
	printf("<center>%s -- 版务管理 [讨论区: %s]<hr color=green>\n", BBSNAME, board);
	printf("<form action=bbsdenyadd><input type=hidden name=board value='%s'>", board);
	printf("封禁使用者<input name=userid size=12> 本版POST权 <input name=dt size=2> 天, 原因<input name=exp size=20>\n");
	printf("<input type=submit value=确认></form>");
}

int inform(char *board, char *user, char *exp, int dt) {
	FILE *fp;
	char path[80], title[80];
	sprintf(title, "[公告] 封禁%s本版POST权%d天", user, dt);
	sprintf(path, "tmp/%d.tmp", getpid());
	fp=fopen(path, "w");
	fprintf(fp, "【本公告由自动发信系统自动张贴】\n\n");
	fprintf(fp, "%s被版务人员[%s]封禁了本版POST权[%d]天.\n", user, currentuser->userid, dt);
	fprintf(fp, "原因是: %s\n", exp);
	fclose(fp);
	// temporary
	//post_article(board, title, path, "deliver", "自动发信系统", "自动发信系统", -1);
	post_mail(user, title, path, currentuser->userid, currentuser->username, fromhost, -1);
	unlink(path);
	printf("系统已经发信通知了%s.<br>\n", user);
}
