#include "bbslib.h"

struct deny
{
	char id[80];
	char exp[80];
	char comment[80];
	time_t free_time;
} denyuser[256];

int denynum=0;

int loaddenyuser(char *board)
{
	FILE *fp;
	char path[80], buf[256];
	char *id, *nick;

	sprintf(path, "boards/%s/deny_users", board);
	fp=fopen(path, "r");
	if(fp==0)
		return;
	while (denynum < (sizeof(denyuser)/sizeof(denyuser[0])))
	{
		if(fgets(buf, sizeof(buf), fp)==0)
			break;
		id = strchr(buf, ' ');
		if (id != NULL)
			*id = '\0';
        strcpy( denyuser[denynum].id, buf );
		strncpy(denyuser[denynum].exp, buf+13, 30);
		nick = strrchr(buf+13, '[');
		if (nick != NULL)
		{
			denyuser[denynum].free_time = atol(nick+1);
			nick--;
			if (nick - buf > 43)
			{
				*nick = '\0';
				strcpy(denyuser[denynum].comment, buf+43);
			}
		}
		denynum++;
	}
	fclose(fp);
}

int savedenyuser(char *board)
{
	FILE *fp;
	int i;
	char path[80], buf[256], *exp;

	sprintf(path, "boards/%s/deny_users", board);
	fp=fopen(path, "w");
	if(fp==0)
		return;
	for(i=0; i<denynum; i++)
	{
		int m;
		exp=denyuser[i].exp;
		if(denyuser[i].id[0]==0)
			continue;
		for(m=0; exp[m]; m++)
		{
			if(exp[m]<32 && exp[m]>0)
				exp[m]='.';
		}
		fprintf(fp, "%-12.12s %-30.30s%s\x1b[%um\n", denyuser[i].id,
			   	denyuser[i].exp, denyuser[i].comment, denyuser[i].free_time);
	}
	fclose(fp);
}

int show_form(char *board)
{
	printf("<center>%s -- 版务管理 [讨论区: %s]<hr color=\"green\">\n", BBSNAME, board);
	printf("<form action=\"bbsdenyadd\"><input type=\"hidden\" name=\"board\" value=\"%s\">",
			board);
	printf("封禁使用者<input name=\"userid\" size=\"12\"> 本版POST权 <input name=\"dt\" size=\"2\"> 天, 原因<input name=\"exp\" size=\"20\">\n");
	printf("<input type=\"submit\" value=\"确认\"></form>");
}

int inform(bcache_t *bp, char *user, char *exp, int dt)
{
	FILE *fn;
	char buf[80], title[80];
	struct userec* usr;
	struct userec saveusr;
	int my_flag;
	char *board;
	postinfo_t pi;
	time_t now;

	bzero(&pi, sizeof(pi));
	board = bp->filename;
	usr = getcurrusr();
	now = time(0);
	memcpy(&saveusr, usr, sizeof(saveusr));
	sprintf(buf, "%s 取消 %s 在 %s 的 POST 权力",
			usr->userid, user, board);
	report(buf);

	/*Haohmaru.4.1.自动发信通知并发文章于板上*/
	sprintf(buf, "etc/%s.deny", usr->userid);
	fn = fopen(buf,"w+");
	sprintf(title, "%s被取消在%s版的发文权限", user, board);

	if ((HAS_PERM(currentuser,PERM_SYSOP)||HAS_PERM(currentuser,PERM_OBOARDS)) && !chk_BM_instr(bp->BM,currentuser->userid))
	{
		my_flag = 0;
		fprintf(fn,"寄信人: SYSOP (System Operator) \n") ;
		fprintf(fn,"标  题: %s\n", title) ;
		fprintf(fn,"发信站: %s (%24.24s)\n","BBS "NAME_BBS_CHINESE"站",ctime(&now)) ;
		fprintf(fn,"来  源: "NAME_BBS_ENGLISH"\n") ;
		fprintf(fn,"\n");
		fprintf(fn,"由于您在 \x1b[4m%s\x1b[0m 版 \x1b[4m%s\x1b[0m，我很遗憾地通知您， \n",
				board, exp);
		if (dt)
			fprintf(fn,"您被暂时取消在该版的发文权力 \x1b[4m%d\x1b[0m 天，到期后请回复\n", dt);
		else
			fprintf(fn,"您被暂时取消在该版的发文权力，到期后请回复\n");
		fprintf(fn,"此信申请恢复权限。\n");
		fprintf(fn,"\n");
		fprintf(fn,"                            "NAME_BBS_CHINESE NAME_SYSOP_GROUP"值班站务：\x1b[4m%s\x1b[0m\n", usr->userid);
		fprintf(fn,"                              %s\n",ctime(&now));
		strcpy(usr->userid,"SYSOP");
		strcpy(usr->username,NAME_SYSOP);
		strcpy(usr->realname,NAME_SYSOP);
	}
	else
	{
		my_flag = 1;
		fprintf(fn,"寄信人: %s \n", usr->userid) ;
		fprintf(fn,"标  题: %s\n", title) ;
		fprintf(fn,"发信站: %s (%24.24s)\n","BBS "NAME_BBS_CHINESE"站",ctime(&now)) ;
		fprintf(fn,"来  源: %s \n", usr->lasthost) ;
		fprintf(fn,"\n");
		fprintf(fn,"由于您在 \x1b[4m%s\x1b[0m 版 \x1b[4m%s\x1b[0m，我很遗憾地通知您， \n",
				board, exp);
		if (dt)
			fprintf(fn,"您被暂时取消在该版的发文权力 \x1b[4m%d\x1b[0m 天，到期后请回复\n", dt);
		else
			fprintf(fn,"您被暂时取消在该版的发文权力，到期后请回复\n");
		fprintf(fn,"此信申请恢复权限。\n");
		fprintf(fn,"\n");
		fprintf(fn,"                              "NAME_BM":\x1b[4m%s\x1b[0m\n", usr->userid);
		fprintf(fn,"                              %s\n",ctime(&now));
	}
	fclose(fn);

	mail_file(getcurruserid(),buf, user, title,1);

	fn=fopen(buf,"w+");
	fprintf(fn,"由于 \x1b[4m%s\x1b[0m 在 \x1b[4m%s\x1b[0m 版的 \x1b[4m%s\x1b[0m 行为，\n",
			user, board, exp);
	if (dt)
		fprintf(fn,"被暂时取消在本版的发文权力 \x1b[4m%d\x1b[0m 天。\n", dt);
	else
		fprintf(fn,"您被暂时取消在该版的发文权力，到期后请回复\n");

	if (my_flag==0)
		fprintf(fn,"                            "NAME_BBS_CHINESE NAME_SYSOP_GROUP"值班站务：\x1b[4m%s\x1b[0m\n", saveusr.userid);
	else
		fprintf(fn,"                              "NAME_BM":\x1b[4m%s\x1b[0m\n", usr->userid);
	fprintf(fn,"                              %s\n",ctime(&now));
	fclose(fn);
	pi.userid = usr->userid;
	pi.username = usr->username;
	pi.title = title;
	pi.board = board;
	pi.local = 1;
	pi.anony = 0;
	pi.access = (FILE_READ << 8) | (FILE_MARKED | FILE_FORWARDED);
	post_file(buf, &pi);
	memcpy(usr, &saveusr, sizeof(saveusr));

	unlink(buf);
	printf("系统已经发信通知了%s.<br>\n", user);
}

int main()
{
	int i; 
	char exp[80], board[80], *userid;
	int dt;
	struct userec *u = NULL;
	bcache_t *bp;
	struct tm* tmtime;
	time_t undenytime;
	char buf[STRLEN];

	init_all();
   	if(!loginok)
		http_fatal("您尚未登录, 请先登录");
	strsncpy(board, getparm("board"), 30);
	strsncpy(exp, getparm("exp"), 30);
	dt=atoi(getparm("dt"));
	if(!has_read_perm(currentuser, board))
		http_fatal("错误的讨论区");
	if(!has_BM_perm(currentuser, board))
		http_fatal("你无权进行本操作");
	loaddenyuser(board);
	userid=getparm("userid");
	if(userid[0]==0)
		return show_form(board);
	if(getuser(userid, &u)==0)
		http_fatal("错误的使用者帐号");
	strcpy(userid, u->userid);
	if(dt<1 || dt>70)
		http_fatal("请输入被封天数(1-70)");
	if(exp[0]==0)
		http_fatal("请输入说明");
   	for(i=0; i<denynum; i++)
	{
		if(!strcasecmp(denyuser[i].id, userid))
			http_fatal("此用户已经被封");
	}
	if(denynum > 40)
	   	http_fatal("太多人被封了");
	strsncpy(denyuser[denynum].id, userid, 13);
	strsncpy(denyuser[denynum].exp, exp, 30);
	undenytime = time(0) + dt * 86400;
	tmtime=gmtime(&undenytime);
	sprintf(buf, "%-12.12s %2d月%2d日解", getcurruserid(),
			tmtime->tm_mon+1, tmtime->tm_mday);
	strsncpy(denyuser[denynum].comment, buf, STRLEN);
	denyuser[denynum].free_time = undenytime;
	denynum++;
	savedenyuser(board);
	printf("封禁 %s 成功<br>\n", userid);
	bp = getbcache(board);
	inform(bp, userid, exp, dt);
	printf("[<a href=\"bbsdenyall?board=%s\">返回被封帐号名单</a>]", board);
	http_quit();
}

