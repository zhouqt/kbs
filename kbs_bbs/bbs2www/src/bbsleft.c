/*
 * $Id$
 */
#define MY_CSS "/bbsold.css"
#include "bbslib.h"

void printdiv(int n, char *str, char *imgurl)
{
	printf("<div id=\"div%da\" class=\"r\"><img id=\"img%d\" src=\"%s\">",
		   n, n, imgurl);
	printf("<A href=\"javascript:changemn('%d');\">%s</A></div>\n", n, str);
	printf("<div id=\"div%d\" class=\"s\">\n", n);
}

int main()
{
	char buf[1024], *ptr;
	int i;

  	init_all();
	printf("<style type=\"text/css\">A {color: #000080}</style><br>\n");
	printf("<script src=\"/func.js\"></script>"
	"<body class=\"dark\" leftmargin=\"5\" topmargin=\"1\" MARGINHEIGHT=\"1\" MARGINWIDTH=\"1\">");
	printf("<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n");
	printf("<tr><td>");
	/*if(!loginok)
	{
		printf( "<form action=\"/bbslogin.jsp\" method=\"post\" target=\"_top\"><br>\n"
NAME_BBS_CHINESE "用户登录<br>\n"
"帐号 <input class=\"default\" type=\"text\" name=\"id\" maxlength=\"12\" size=\"8\"><br>\n"
"密码 <input class=\"default\" type=\"password\" name=\"passwd\" maxlength=\"39\" size=\"8\"><br>\n"
"<input class=\"button\" type=\"submit\" value=\"登录进站\">\n"
"</form>\n");
	}
	else*/
	{
        char buf[256]="未注册用户";
		if (currentuser==NULL)
			http_fatal("hehe:%d",loginok);
		printf("用户: <a href=\"bbsqry?userid=%s\" target=\"f3\">%s</a><br>", 
				getcurruserid(), getcurruserid());
		uleveltochar(buf, getcurrusr());
        printf("身份: %s<br>\n", buf);
        printf("<a href=\"/bbslogout.jsp\" target=\"_top\">注销本次登录</a><br>\n");
	}
	printf("</td></tr><tr><td>\n");
	printf("<hr style=\"color:2020f0; height=1px\" width=\"84px\" align=\"center\">\n");
	printf("</td></tr></table>");
  	printf("<img src=\"/images/home.gif\"><a href=\"bbssec\" target=\"f3\">讨论区首页</a><br>\n");
        printf("<img src=\"/images/link0.gif\"><a target=\"f3\" href=\"bbs0an\">精华公布栏</a><br>\n");
   	printf("<img src=\"/images/link0.gif\"><a target=\"f3\" href=\"bbstop10\">今日十大</a><br>\n");
	/*printf("<img src=\"/images/link0.gif\"><a target=\"f3\" href=\"bbstopb10\">热门讨论区</a><br>\n");*/
	if(loginok)
	{
		int i, mybrdnum=0;
		const struct boardheader  *bptr;
		printdiv(1,"个人定制区","/images/folder.gif");
		load_favboard(0);
		mybrdnum = get_favboard_count();
 		for(i=0; i<mybrdnum; i++)
		{
			bptr = getboard(get_favboard(i+1)+1);
			if (bptr == NULL)
				continue;
     		printf("<img src=\"/images/link.gif\">"
					" <a target=\"f3\" href=\"bbsdoc?board=%s\">%s</a><br>\n",
					bptr->filename, bptr->filename);
		}
		printf("<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsmybrd\">"
				"预定管理</a><br>\n");
		printf("</div>\n");
	}
	printdiv(2,"分类讨论区","/images/folder.gif");
	for (i = 0; i < SECNUM; i++)
		printf("<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsboa?group=%d\">%s</a><br>\n",
				i, secname[i][0]);
	printf("</div>\n");

	printdiv(3,"谈天说地区","/images/folder.gif");
  	if(loginok) {
		printf("<img src=\"/images/link.gif\"> <a href=\"bbsfriend\" target=\"f3\">在线好友</a><br>\n");
	}
  	printf("<img src=\"/images/link.gif\"> <a href=\"bbsusr\" target=\"f3\">环顾四方</a><br>\n");
  	printf("<img src=\"/images/link.gif\"> <a href=\"bbsqry\" target=\"f3\">查询网友</a><br>\n");
        if(loginok&&currentuser->userlevel & PERM_PAGE) {
                printf("<img src=\"/images/link.gif\"> <a href=\"bbssendmsg\" target=\"f3\">发送讯息</a><br>\n");
                printf("<img src=\"/images/link.gif\"> <a href=\"bbsmsg\" target=\"f3\">查看所有讯息</a><br>\n");
	}
	printf("</div>\n");
	ptr="";
	if(loginok&&currentuser->userlevel & PERM_CLOAK)
		ptr="<img src=\"/images/link.gif\"> <a target=\"f3\" onclick=\"return confirm('确实切换隐身状态吗?')\" href=\"bbscloak\">切换隐身</a><br>\n";
	if(loginok)
	{
		printdiv(4,"个人工具箱","/images/folder.gif");
			printf(
"			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsinfo\">个人资料</a><br>\n"
"			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsplan\">改说明档<a><br>\n"
"			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbssig\">改签名档<a><br>\n"
"			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbspwd\">修改密码</a><br>\n"
"			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsparm\">修改个人参数</a><br>\n"
/*
"			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsmywww\">WWW个人定制</a><br>\n"
*/
"			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsnick\">临时改昵称</a><br>\n"
/*"		<img src=\"/images/link.gif\"><a target=\"f3\" href=\"bbsstat\"> 排名统计</a><br>"*/
"			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsfall\">设定好友</a><br>\n"
"			%s</div>",ptr);
		printdiv(5,"处理信件区","/images/folder.gif");
		printf("			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsnewmail\">阅览新邮件</a><br>\n"
"			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsmail\">所有邮件</a><br>\n");
		if (can_send_mail())
			printf("			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbspstmail\">发送邮件</a><br>\n");
		printf("			</div>\n");
	}

	/*printdiv(6,"特别服务区","/images/folder.gif");
	printf("<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsalluser\">所有使用者</a><br>\n");*/
	/*printf("<img src=\"/images/link.gif\"><a target=\"f3\" href=\"bbsadl\">下载精华区</a><br>\n");*/
	/*printf("</div>\n");*/
	printf("<div class=\"r\">");
  	/*printf("<img src=\"/images/link0.gif\"><a href=\"bbsfind\" target=\"f3\">文章查询</a><br>\n");*/
	printf("<img src=\"/images/find.gif\"><a href=\"bbssel\" target=\"f3\">查找讨论区</a><br>\n");
	printf("<img src=\"/images/telnet.gif\"><a href=\"telnet:%s\">Telnet登录</a><br>\n", NAME_BBS_ENGLISH);
	/*if(!loginok) 
		printf("<img src=\"/images/link0.gif\"><a href=\"javascript:void open('bbsreg', '', 'width=620,height=550')\">新用户注册</a><br>\n");*/
   	if(loginok) {
		if(HAS_PERM(currentuser,PERM_LOGINOK) && !HAS_PERM(currentuser,PERM_POST))
			printf("<script>alert('您被封禁了全站发表文章的权限, 请参看Announce版公告, 期满后在sysop版申请解封. 如有异议, 可在Complain版提出申诉.')</script>\n");
		if(count_new_mails()>0) 
			printf("<script>alert('您有新信件!')</script>\n");
	}
	/*if(loginok && !(currentuser->userlevel & PERM_LOGINOK) && !has_fill_form()) 
		printf("<a target=\"f3\" href=\"bbsform\">填写注册单</a><br>\n");*/
	if(loginok && can_enter_chatroom())
		printf("<br><a href=\"javascript:openchat()\">["CHAT_SERVER"<font color=\"red\">测试中</font>]</a>");
	printf("</div><script>if(isNS4) arrange();if(isOP)alarrangeO();</script>");
  	printf("</body>");
}

int count_new_mails() {
        struct fileheader x1;
        int n, unread=0;
	char buf[1024];
        FILE *fp;
        if(!loginok&&currentuser->userid[0]==0) return 0;
        sprintf(buf, "%s/mail/%c/%s/.DIR", BBSHOME, toupper(currentuser->userid[0]), currentuser->userid);
        fp=fopen(buf, "r");
        if(fp==0)
			return unread;
        while(fread(&x1, sizeof(x1), 1, fp)>0)
                if(!(x1.accessed[0] & FILE_READ)) unread++;
        fclose(fp);
	return unread;
}
