#include "bbslib.h"

int main()
{
   	FILE *fp;
	char userid[80], filename[80], dir[80], title[80], title2[80], buf[80], *content;
	int t, i, sig, backup;
	struct fileheader x;
   	struct userec *u = NULL;

	init_all();
	if(!loginok)
		http_fatal("匆匆过客不能写信，请先登录");
   	strsncpy(userid, getparm("userid"), 40);
   	strsncpy(title, getparm("title"), 50);
	backup=strlen(getparm("backup"));
	if(strchr(userid, '@') || strchr(userid,'|')
		|| strchr(userid,'&') || strchr(userid,';'))
	{
		http_fatal("错误的收信人帐号");
	}
	getuser(userid,&u);
	if(u==0)
		http_fatal("错误的收信人帐号");
	strcpy(userid, u->userid);
  	for(i=0; i<strlen(title); i++)
		if(title[i]<27 && title[i]>=-1) title[i]=' ';
   	sig=atoi(getparm("signature"));
   	content=getparm("text");
   	if(title[0]==0)
      		strcpy(title, "没主题");
	sprintf(filename, "tmp/%s.%d.tmp", userid, getpid());
	if (f_append(filename, content) < 0)
		http_fatal("发信失败");
	sprintf(title2, "{%s} %s", userid, title);
	title2[70]=0;
	post_mail(userid, title, filename, currentuser->userid, currentuser->username, fromhost, sig-1);
	if(backup)
		post_mail(currentuser->userid, title2, filename, currentuser->userid, currentuser->username, fromhost, sig-1);
	unlink(filename);
	printf("信件已寄给%s.<br>\n", userid);
	if(backup)
		printf("信件已经备份.<br>\n");
	printf("<a href=\"javascript:history.go(-2)\">返回</a>");
	http_quit();
}
