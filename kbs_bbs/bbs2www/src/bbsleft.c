/*
 * $Id$
 */
#define MY_CSS "/bbsold.css"
#include "bbslib.h"

void printmenubegin()
{
    printf("<tr><td nowrap>\n");
}

void printdiv(int n, char *str, char *imgurl)
{
    printmenubegin();
    printf("<img id=\"img%d\" src=\"%s\">", n, imgurl);
    printf("<A href=\"javascript:changemn('%d');\">%s</A></div>\n", n, str);
    printf("<div id=\"div%d\" style=\"display: none\">\n", n);
}

void printmenuend()
{
    printf("</td><td>&nbsp;</td></tr>\n");
}

void printmenu(char* s)
{
    printmenubegin();
    printf("%s",s);
    printmenuend();
}

int main()
{
    char buf[256], *ptr;
    int i;

    init_all();
    printf("<style type=\"text/css\">A {color: #000080}</style><br>\n");
    printf("<script src=\"/func.js\"></script>" "<body class=\"dark\" leftmargin=\"5\" topmargin=\"1\" MARGINHEIGHT=\"1\" MARGINWIDTH=\"1\">");
    printf("<table  cellSpacing=0 cellPadding=0 width=\"100%\" border=0><tr><td>");
    printf("<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n");
    printf("<tr><td>");
    if (currentuser == NULL)
        http_fatal("hehe:%d", loginok);
    printf("用户: <a href=\"bbsqry?userid=%s\" target=\"f3\">%s</a><br>", getcurruserid(), getcurruserid());
    uleveltochar(buf, getcurrusr());
    printf("身份: %s<br>\n", buf);
    printf("<a href=\"/bbslogout.php\" target=\"_top\">注销本次登录</a><br>\n");
    printf("</td></tr><tr><td>\n");
    printf("<hr style=\"color:#2020f0; height:1px\" width=\"84px\" align=\"center\">\n");
    printf("</td></tr></table>");
    printf("</td></tr><tr><td>");
    printf("<table border=0 cellspacing=0 cellpadding=0 align=right width=95%>");
    printf("<img src=\"/images/home.gif\"><a href=\"/bbssec.php\" target=\"f3\">讨论区首页</a><br>\n");
    printf("<img src=\"/images/link0.gif\"><a target=\"f3\" href=\"bbs0an\">精华公布栏</a><br>\n");
    printf("<img src=\"/images/link0.gif\"><a target=\"f3\" href=\"/bbstop10.php\">今日十大</a><br>\n");
    /*printf("<img src=\"/images/link0.gif\"><a target=\"f3\" href=\"bbstopb10\">热门讨论区</a><br>\n"); */
    if (loginok) {
        int i, mybrdnum = 0;
        const struct boardheader *bptr;
        char *name;

        printf("<img src=\"/images/folder.gif\">" "<a target=\"f3\" href=\"bbsfav?select=-1\">个人定制区</a><br>\n");
/*		printdiv(1,"个人定制区","/images/folder.gif");
		load_favboard(0);
		mybrdnum = getfavnum();
 		for(i=0; i<mybrdnum; i++)
		{
			name = get_favboard(i);
			if (name == NULL)
				continue;
			if (get_favboard_type(i))
     		printf("<img src=\"/images/link.gif\">"
				" <a target=\"f3\" href=\"/bbsdoc.php?board=%s\">%s</a><br>\n",
				encode_url(buf, name, sizeof(buf)), name);
			else
     		printf("<img src=\"/images/folder.gif\">"
				" <a target=\"f3\" href=\"bbsfav?select=%d\">%s</a><br>\n",
				get_favboard_id(i), name);
		}
		printf("<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsmybrd\">"
				"预定管理</a><br>\n");
		printf("</div>\n");
		release_favboard();*/
    }
    printdiv(2, "分类讨论区", "/images/folder.gif");
    for (i = 0; i < SECNUM; i++)
        printf("<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbsboa.php?group=%d\">%s</a><br>\n", i, secname[i][0]);
    printf("</div>\n");
    printmenuend();

    printdiv(3, "谈天说地区", "/images/folder.gif");
    if (loginok) {
        printf("<img src=\"/images/link.gif\"> <a href=\"bbsfriend\" target=\"f3\">在线好友</a><br>\n");
    }
    printf("<img src=\"/images/link.gif\"> <a href=\"bbsusr\" target=\"f3\">环顾四方</a><br>\n");
    printf("<img src=\"/images/link.gif\"> <a href=\"bbsqry\" target=\"f3\">查询网友</a><br>\n");
    if (loginok && currentuser->userlevel & PERM_PAGE) {
        printf("<img src=\"/images/link.gif\"> <a href=\"/bbssendmsg.php\" target=\"f3\">发送讯息</a><br>\n");
        printf("<img src=\"/images/link.gif\"> <a href=\"/bbsmsg.php\" target=\"f3\">查看所有讯息</a><br>\n");
    }
    printf("</div>\n");
    printmenuend();
    ptr = "";
    if (loginok && currentuser->userlevel & PERM_CLOAK)
        ptr = "<img src=\"/images/link.gif\"> <a target=\"f3\" onclick=\"return confirm('确实切换隐身状态吗?')\" href=\"/bbscloak.php\">切换隐身</a><br>\n";
    if (loginok) {
        printdiv(4, "个人工具箱", "/images/folder.gif");
        printf("			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsinfo\">个人资料</a><br>\n"
               "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbsplan.php\">改说明档<a><br>\n"
               "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbssig.php\">改签名档<a><br>\n"
               "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbspwd\">修改密码</a><br>\n"
               "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsparm\">修改个人参数</a><br>\n"
/*
"			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsmywww\">WWW个人定制</a><br>\n"
*/
               "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsnick\">临时改昵称</a><br>\n"
/*"		<img src=\"/images/link.gif\"><a target=\"f3\" href=\"bbsstat\"> 排名统计</a><br>"*/
               "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsfall\">设定好友</a><br>\n" "			%s</div>", ptr);
        printmenuend();
        printdiv(5, "处理信件区", "/images/folder.gif");
        printf("			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsnewmail\">阅览新邮件</a><br>\n"
               "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsmail\">所有邮件</a><br>\n"
         	   "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbsmailbox_system.php\">系统预定义邮箱</a><br>\n"
		   "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbsmailbox_custom.php\">自定义邮箱</a><br>\n");
        if (can_send_mail())
            printf("			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbspstmail\">发送邮件</a><br>\n");
        printf("			</div>\n");
        printmenuend();
    }

    printdiv(6, "界面风格", "/images/folder.gif");
    printf("<img src=\"/images/link.gif\"> <a href=\"javascript:change_style(0)\">小字体</a><br>\n");
    printf("<img src=\"/images/link.gif\"> <a href=\"javascript:change_style(1)\">大字体</a><br>\n");
    /*printf("<img src=\"/images/link.gif\"><a target=\"f3\" href=\"bbsadl\">下载精华区</a><br>\n"); */
    printf("</div>\n");
    printmenuend();
    printf("<div class=\"r\">");
    /*printf("<img src=\"/images/link0.gif\"><a href=\"bbsfind\" target=\"f3\">文章查询</a><br>\n"); */
    printmenu("<img src=\"/images/find.gif\"><a href=\"bbssel\" target=\"f3\">查找讨论区</a><br>\n");
    printmenu("<img src=\"/images/find.gif\"><a href=\"/games/index.html\" target=\"f3\">休闲娱乐区</a><br>\n");
    printmenubegin();
	printf("<table><tr><form action=/bbsdoc.php target=\"f3\"><td>&nbsp;&nbsp;"
			"<input type=text name=board maxlength=20 "
			"size=9 value=选择讨论区 onclick=\"this.select()\"></td></form></tr></table>\n");
    printf("<img src=\"/images/telnet.gif\"><a href=\"telnet:%s\">Telnet登录</a><br>\n", NAME_BBS_ENGLISH);
    printmenuend();
#ifdef SMTH
    printmenu("<img src=\"/images/telnet.gif\"><a href=\"/data/fterm-smth.zip\">fterm下载</a><br>\n");
#endif
    /*if(!loginok) 
       printf("<img src=\"/images/link0.gif\"><a href=\"javascript:void open('bbsreg', '', 'width=620,height=550')\">新用户注册</a><br>\n"); */
    if (loginok) {
        if (HAS_PERM(currentuser, PERM_LOGINOK) && !HAS_PERM(currentuser, PERM_POST))
            printmenu("<script>alert('您被封禁了全站发表文章的权限, 请参看Announce版公告, 期满后在sysop版申请解封. 如有异议, 可在Complain版提出申诉.')</script>\n");
        if (count_new_mails() > 0)
            printf("<script>alert('您有新信件!')</script>\n");
    }
    /*if(loginok && !(currentuser->userlevel & PERM_LOGINOK) && !has_fill_form()) 
       printf("<a target=\"f3\" href=\"bbsform\">填写注册单</a><br>\n"); */
    if (loginok && can_enter_chatroom())
        printmenu("<br><a href=\"javascript:openchat()\">[" CHAT_SERVER "]</a>");
    printf("<script>if(isNS4) arrange();if(isOP)alarrangeO();</script></p>");
    printf("</table></td></tr>");
#ifdef SMTH
    printf("<tr><td>");
    printf("<TABLE cellSpacing=0 cellPadding=0 width=\"90%\" border=0>");
    printf("<tr><td>\n");
    printf("<p>全国网络计算大赛正在举行，大赛奖项高达20万元，其中一等奖奖金高达10万元。详情请见</p>");
    printf("</tr></td>\n");
    printf("<tr><td>\n");
    printf("<a href=\"http://www.hpc-contest.edu.cn\"  target=\"_blank\">http://www.hpc-contest.edu.cn</a><br></p>");
    printf("</tr></td></table>\n");
    printf("</td></tr>");
#endif
    printf("</table>");
    printf("</body>");
}

int count_new_mails()
{
    struct fileheader x1;
    int n, unread = 0;
    char buf[1024];
    FILE *fp;

    if (!loginok && currentuser->userid[0] == 0)
        return 0;
    sprintf(buf, "%s/mail/%c/%s/.DIR", BBSHOME, toupper(currentuser->userid[0]), currentuser->userid);
    fp = fopen(buf, "r");
    if (fp == 0)
        return unread;
    while (fread(&x1, sizeof(x1), 1, fp) > 0)
        if (!(x1.accessed[0] & FILE_READ))
            unread++;
    fclose(fp);
    return unread;
}
