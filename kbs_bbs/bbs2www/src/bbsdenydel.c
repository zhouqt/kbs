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

int main()
{
	int i; 
	char board[80], *userid;

	init_all();
   	if(!loginok)
	   	http_fatal("您尚未登录, 请先登录");
	strsncpy(board, getparm("board"), 30);
	if(!has_read_perm(currentuser, board))
	   	http_fatal("错误的讨论区");
	if(!has_BM_perm(currentuser, board))
	   	http_fatal("你无权进行本操作");
	loaddenyuser(board);
	userid = getparm("userid");
   	for(i=0; i<denynum; i++)
   	{
		if(!strcasecmp(denyuser[i].id, userid))
	   	{
			denyuser[i].id[0]=0;
			savedenyuser(board);
			printf("已经给 %s 解封. <br>\n", userid);
			inform(board, userid);
			printf("[<a href=\"bbsdenyall?board=%s\">返回被封名单</a>]", board);
			http_quit();
		}
	}
	http_fatal("这个用户不在被封名单中");
	http_quit();
	return 0;
}

int inform(char *board, char *user)
{
    FILE* fn1;
    char filename[STRLEN];
    char buffer[STRLEN];
    time_t now;
    struct userec* lookupuser;
	struct userec* usr;
	postinfo_t pi;

    now = time(0);
	usr = getcurrusr();
	bzero(&pi, sizeof(pi));
    /*Haohmaru.4.1.自动发信通知*/
    sprintf(filename,"etc/%s.dny", usr->userid);
    fn1 = fopen(filename,"w");
	sprintf(buffer,"[通知]");
	fprintf(fn1,"寄信人: %s \n",usr->userid) ;
	fprintf(fn1,"标  题: %s\n",buffer) ;
	fprintf(fn1,"发信站: %s (%24.24s)\n",
			"BBS "NAME_BBS_CHINESE"站",ctime(&now)) ;
	fprintf(fn1,"来  源: %s \n",usr->lasthost) ;
	fprintf(fn1,"\n");
    if (HAS_PERM(currentuser,PERM_SYSOP) || HAS_PERM(currentuser,PERM_OBOARDS))
        fprintf(fn1,"您被站务人员 %s 解除在 %s 板的封禁\n",usr->userid,board);
    else
        fprintf(fn1,"您被 %s 板板主 %s 解除封禁\n",board,usr->userid);
    fclose(fn1);
    mail_file(getcurruserid(),filename, user, buffer,0);

    /*解封同样发文到undenypost版  Bigman:2000.6.30*/ 
    getuser(user, &lookupuser); 
    if (PERM_BOARDS & lookupuser->userlevel)
    	sprintf(buffer,"%s 解封某板板主 %s 在 %s ", usr->userid, user, board); 
    else
        sprintf(buffer,"%s 解封 %s 在 %s", usr->userid, user, board);
	pi.userid = usr->userid;
	pi.username = usr->username;
	pi.title = buffer;
	pi.board = "undenypost";
	pi.local = 1;
	pi.anony = 0;
	pi.access = 0;
	post_file(filename, &pi);
    unlink(filename);

	printf("系统已经发信通知了%s.<br>\n", user);
}

