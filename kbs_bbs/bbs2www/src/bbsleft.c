#define MY_CSS "/bbsold.css"
#include "bbslib.h"

int main() {
	char buf[1024], *ptr;
	int i;

  	init_all();
	printf("<style type=\"text/css\">A {color: #000080} BODY {BACKGROUND-COLOR: #b0e0b0; FONT-SIZE: 14px;}</style><br>\n"
	"<script>"
"	function closebut(x, y) {"
"		if(document.img0) document.img0.src='/folder.gif';"
"		if(document.img1) document.img1.src='/folder.gif';"
"		if(document.img2) document.img2.src='/folder.gif';"
"		if(document.img3) document.img3.src='/folder.gif';"
"		if(document.img4) document.img4.src='/folder.gif';"
"		if(document.img5) document.img5.src='/folder.gif';"
"		if(document.all.div0) document.all.div0.style.display='none';"
"		if(document.all.div1) document.all.div1.style.display='none';"
"		if(document.all.div2) document.all.div2.style.display='none';"
"		if(document.all.div3) document.all.div3.style.display='none';"
"		if(document.all.div4) document.all.div4.style.display='none';"
"		if(document.all.div5) document.all.div5.style.display='none';"
"		x.style.display='block';"
"		y.src='/folder2.gif';"
"	}"
"	function t(x, y) {"
"		if(x.style.display!='none') {"
"			x.style.display='none';"
"			y.src='/folder.gif';"
"		}"
"		else"
"			closebut(x, y);"
"	}"
"	function openchat() {"
"		url='bbschat';"
"		chatWidth=screen.availWidth;"
"		chatHeight=screen.availHeight;"
"		winPara='toolbar=no,location=no,status=no,menubar=no,scrollbars=auto,resizable=yes,left=0,top=0,'+'width='+(chatWidth-10)+',height='+(chatHeight-27);"
"		window.open(url,'_blank',winPara);"
"	}"
"	</script>");
	printf("<nobr>\n");
	if(!loginok) {
                printf("<center>"
"               	<form action=\"bbslogin\" method=\"post\" target=\"_top\"><br>"
"		bbs用户登录<br>"
"               	帐号 <input style=\"height:20px;BACKGROUND-COLOR:e0f0e0\" type=\"text\" name=\"id\" maxlength=\"12\" size=\"8\"><br>"
"               	密码 <input style=\"height:20px;BACKGROUND-COLOR:e0f0e0\" type=\"password\" name=\"pw\" maxlength=\"39\" size=\"8\"><br>"
"               	<input style=\"width:72px; height:22px; BACKGROUND-COLOR:b0e0b0\" type=\"submit\" value=\"登录进站\">"
"		</center>");
	}
	else
	{
        char buf[256]="未注册用户";
		printf("用户: <a href=\"bbsqry?userid=%s\" target=\"f3\">%s</a><br>", 
				getcurruserid(), getcurruserid());
		uleveltochar(buf, getcurrusr());
        printf("身份: %s<hr style=\"color:2020f0; height:1px\" width=\"84px\" align=\"left\">", buf);
        printf("<a href=\"bbslogout\" target=\"_top\">注销本次登录</a><br>\n");
	}
  	printf("<hr style=\"color:2020f0; height=1px\" width=\"84px\" align=\"left\"><br><img src=\"/link0.gif\"><a href=\"bbsall\" target=\"f3\">讨论区首页</a><br>\n");
        printf("<img src=\"/link0.gif\"><a target=\"f3\" href=\"bbs0an\">精华公布栏</a><br>\n");
   	printf("<img src=\"/link0.gif\"><a target=\"f3\" href=\"bbstop10\">今日十大</a><br>\n");
	printf("<img src=\"/link0.gif\"><a target=\"f3\" href=\"bbstopb10\">热门讨论区</a><br>\n");
	if(loginok)
	{
		int i, mybrdnum=0;
		const struct boardheader  *bptr;
		printf("<img src=\"/folder.gif\" name=\"img0\"><a href=\"javascript: t(document.all.div0, document.img0)\">个人定制区</a><br>\n");
		printf("<div id=\"div0\" style=\"display:none\">\n");
		load_favboard(0);
		mybrdnum = get_favboard_count();
  		for(i=0; i<mybrdnum; i++)
		{
			bptr = getboard(get_favboard(i+1)+1);
			if (bptr == NULL)
				continue;
     		printf("<img src=\"/link.gif\">"
					"<a target=\"f3\" href=\"bbsdoc?board=%s\"> %s</a><br>\n",
					bptr->filename, bptr->filename);
		}
		printf("<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsmybrd\">"
				" 预定管理</a><br>\n");
		printf("</div>\n");
	}
  	printf(" <img src=\"/folder.gif\" name=\"img1\"><a href=\"javascript: t(document.all.div1, document.img1)\">分类讨论区</a><br>"
"		<div id=\"div1\" style=\"display:none\">\n");
	for (i = 0; i < SECNUM; i++)
		printf("<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsboa?%d\"> %s</a><br>",
				i, secname[i][0]);
	printf("</div><img src=\"/folder.gif\" name=\"img2\"><a href=\"javascript: t(document.all.div2, document.img2)\">谈天说地区</a><br>\n");
	printf("<div id=\"div2\" style=\"display:none\">\n");
  	if(loginok) {
		printf("<img src=\"/link.gif\"><a href=\"bbsfriend\" target=\"f3\"> 在线好友</a><br>\n");
	}
  	printf("<img src=\"/link.gif\"><a href=\"bbsusr\" target=\"f3\"> 环顾四方</a><br>\n");
  	printf("<img src=\"/link.gif\"><a href=\"bbsqry\" target=\"f3\"> 查询网友</a><br>\n");
        if(loginok&&currentuser->userlevel & PERM_PAGE) {
                printf("<img src=\"/link.gif\"><a href=\"bbssendmsg\" target=\"f3\"> 发送讯息</a><br>\n");
                printf("<img src=\"/link.gif\"><a href=\"bbsmsg\" target=\"f3\"> 查看所有讯息</a><br>\n");
	}
	printf("</div>\n");
	ptr="";
	if(loginok&&currentuser->userlevel & PERM_CLOAK)
		ptr="<img src=\"/link.gif\"><a target=\"f3\" onclick=\"return confirm('确实切换隐身状态吗?')\" href=\"bbscloak\"> 切换隐身</a><br>\n";
	if(loginok)
                printf("<img src=\"/folder.gif\" name=\"img3\"><a href=\"javascript: t(document.all.div3, document.img3)\">个人工具箱</a><br>"
"			<div id=\"div3\" style=\"display:none\">"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsinfo\"> 个人资料</a><br>"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsplan\"> 改说明档<a><br>"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbssig\"> 改签名档<a><br>"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbspwd\"> 修改密码</a><br>"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsparm\"> 修改个人参数</a><br>"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsmywww\"> WWW个人定制</a><br>"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsnick\"> 临时改昵称</a><br>"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsstat\"> 排名统计</a><br>"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsfall\"> 设定好友</a><br>"
"			%s</div>"
"			<img src=\"/folder.gif\" name=\"img5\"><a href=\"javascript: t(document.all.div5, document.img5)\">处理信件区</a><br>"
"			<div id=\"div5\" style=\"display:none\">"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsnewmail\"> 阅览新邮件</a><br>"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsmail\"> 所有邮件</a><br>"
"			<img src=\"/link.gif\"><a target=\"f3\" href=\"bbspstmail\"> 发送邮件</a><br>"
"			</div>", ptr);
	printf("<img src=\"/folder.gif\" name=\"img4\">");
	printf("<a href=\"javascript: t(document.all.div4, document.img4)\">特别服务区</a><br>\n");
	printf("<div id=\"div4\" style=\"display:none\">\n");
	printf("<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsalluser\">所有使用者</a><br>\n");
	printf("<img src=\"/link.gif\"><a target=\"f3\" href=\"bbsadl\">下载精华区</a><br>\n");
	printf("</div>\n");
  	printf("<img src=\"/link0.gif\"><a href=\"bbsfind\" target=\"f3\">文章查询</a><br>\n");
	printf("<img src=\"/link0.gif\"><a href=\"bbssel\" target=\"f3\">查找讨论区</a><br>\n");
	printf("<img src=\"/telnet.gif\"><a href=\"telnet:%s\">Telnet登录</a><br>\n", NAME_BBS_ENGLISH);
	if(!loginok) 
		printf("<img src=\"/link0.gif\"><a href=\"javascript:void open('bbsreg', '', 'width=620,height=550')\">新用户注册</a><br>\n");
   	if(loginok) {
		if(HAS_PERM(currentuser,PERM_LOGINOK) && !HAS_PERM(currentuser,PERM_POST))
			printf("<script>alert('您被封禁了全站发表文章的权限, 请参看Announce版公告, 期满后在sysop版申请解封. 如有异议, 可在appeal版提出申诉.')</script>\n");
		if(count_new_mails()>0) 
			printf("<script>alert('您有新信件!')</script>\n");
	}
	if(loginok && !(currentuser->userlevel & PERM_LOGINOK) && !has_fill_form()) 
		printf("<a target=\"f3\" href=\"bbsform\">填写注册单</a><br>\n");
	if(loginok) printf("<br><a href=\"javascript:openchat()\">[聊天广场<font color=\"red\">测试中</font>]</a>");
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
