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

int show_fav_all(int f, const char *desc, int d)
{
    char * s;
    int i=0,j,k,l;
    if(d==1) {
		printdiv(d, desc, "/images/folder.gif");
		printf("<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsfav?select=-1\">定制</a><br>\n");
    }
        //printf("<div id=div%d style=\"display: none\">\n", f+100);
    else
	{
		printf("<img src=\"/images/folder2.gif\"><a target=\"f3\" href=\"bbsfav?select=%d\">%s</a><br>\n", f, desc);
        printf("<div>");
	}
    do{
        SetFav(f);
        s = get_favboard(i);
        j = get_favboard_type(i);
        k = get_favboard_id(i);
        if(s!=NULL) {
            for(l=0;l<((j!=1)?d:d-1);l++) printf("　");
            if(j==1)
                printf("<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbsdoc.php?board=%s\">%s</a><br>\n", s, s);
            else {
                show_fav_all(k, s, d+1);
            }
        }
        i++;
        
    } while(s!=NULL);
    printf("</div>\n");
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
#ifdef ZIXIA
	if (!strcmp("guest",getcurruserid())){
		printf("      <form target=\"_top\" action=\"/bbslogin.php\" method=\"post\" name=\"form1\"> \n");
		printf("      <table CELLPADDING=2 CELLSPACING=2><thead> \n");
		printf("      <th colspan=2>快速登陆</th> \n");
		printf("      <thead><tbody> \n");
		printf("      <tr><TD ALIGN=\"RIGHT\" nowrap>账号:</TD><TD ><INPUT TYPE=text size=\"10\" name=\"id\"></TD></tr> \n");
		printf("      <tr><TD ALIGN=\"RIGHT\" nowrap>密码:</TD><TD ><INPUT TYPE=password size=\"10\" name=\"passwd\" maxlength=\"39\"></TD></tr> \n");
		printf("     <tr><TD align=center colspan=2><input type=submit value=\"登 录\" name=submit1></TD></tr> \n ");
		printf("      </tbody></table> </form> \n");
	} else {
		printf("<a href=\"/bbslogout.php\" target=\"_top\">注销本次登录</a><br>\n");
	}
#else
    printf("<a href=\"/bbslogout.php\" target=\"_top\">注销本次登录</a><br>\n");
#endif
    printf("</td></tr><tr><td>\n");
    printf("<hr style=\"color:#2020f0; height:1px\" width=\"84px\" align=\"center\">\n");
    printf("</td></tr></table>");
    printf("</td></tr><tr><td>");
    printf("<table border=0 cellspacing=0 cellpadding=0 align=right width=95%>");
    printf("<img src=\"/images/home.gif\"><a href=\"/bbssec.php\" target=\"f3\">讨论区首页</a><br>\n");
    printf("<img src=\"/images/new.gif\"><a target=\"f3\" href=\"/bbsboa.php?group=0&group2=-2\">新开讨论区</a><br>\n");
    printf("<img src=\"/images/link0.gif\"><a target=\"f3\" href=\"bbs0an\">精华公布栏</a><br>\n");
    printf("<img src=\"/images/link0.gif\"><a target=\"f3\" href=\"/bbstop10.php\">今日十大</a><br>\n");
    /*printf("<img src=\"/images/link0.gif\"><a target=\"f3\" href=\"bbstopb10\">热门讨论区</a><br>\n"); */
    if (loginok) {
        int i, mybrdnum = 0;
        const struct boardheader *bptr;
        char *name;

        load_favboard(0);
        show_fav_all(-1, "个人定制区", 1);
        release_favboard();
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
        printf("			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbsfillform.html\">填写注册单</a><br>\n"
               "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"bbsinfo\">个人资料</a><br>\n"
               "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbsplan.php\">改说明档</a><br>\n"
               "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbssig.php\">改签名档</a><br>\n"
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
        printf("			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbsnewmail.php\">阅览新邮件</a><br>\n"
               "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbsreadmail.php\">所有邮件</a><br>\n"
         	   "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbsmailbox_system.php\">系统预定义邮箱</a><br>\n"
		   "			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbsmailbox_custom.php\">自定义邮箱</a><br>\n");
        if (can_send_mail())
            printf("			<img src=\"/images/link.gif\"> <a target=\"f3\" href=\"/bbspstmail.php\">发送邮件</a><br>\n");
        printf("			</div>\n");
        printmenuend();
    }

    printdiv(6, "界面风格", "/images/folder.gif");
    printf("<img src=\"/images/link.gif\"> <a href=\"/bbsstyle.php?s=0\">小字体</a><br>\n");
    printf("<img src=\"/images/link.gif\"> <a href=\"/bbsstyle.php?s=1\">大字体</a><br>\n");
    /*printf("<img src=\"/images/link.gif\"><a target=\"f3\" href=\"bbsadl\">下载精华区</a><br>\n"); */
    printf("</div>\n");
    printmenuend();
#ifdef SMTH
	printf("<tr><td nowrap>");
	printf("<img id=\"img7\" src=\"/images/folder.gif\"><A href=\"javascript:changemn('7');\">水木特刊web版</A></div>");
	printf("<div id=\"div7\" style=\"display: none\">");
    printf("<img src=\"/images/link.gif\"> <a href=\"/express/0603/smth_express.htm\" target=\"_blank\">2003年6月号</a><br>\n");
    printf("<img src=\"/images/link.gif\"> <a href=\"/express/0703/smth_express.htm\" target=\"_blank\">2003年7月号</a><br>\n");
    printf("</div>\n");
    printmenuend();
#endif
    printf("<div class=\"r\">");
    printmenu("<img src=\"/images/find.gif\"><a href=\"/games/index.html\" target=\"f3\">休闲娱乐区</a><br>\n");
    printmenubegin();
	printf("<table><tr><form action=\"bbssel\" target=\"f3\"><td>&nbsp;&nbsp;"
			"<input type=\"text\" name=\"board\" maxlength=\"20\" "
			"size=\"9\" value=\"选择讨论区\" onmouseover=\"this.select()\"></td></form></tr></table>\n");
    printf("<img src=\"/images/telnet.gif\"><a href=\"telnet:%s\">Telnet登录</a><br>\n", NAME_BBS_ENGLISH);
    printmenuend();
#ifdef SMTH
    printmenu("<img src=\"/images/telnet.gif\"><a href=\"/data/fterm-smth.zip\">fterm下载</a><br>\n");
#endif
    /*if(!loginok) 
       printf("<img src=\"/images/link0.gif\"><a href=\"javascript:void open('bbsreg', '', 'width=620,height=550')\">新用户注册</a><br>\n"); */
    if (loginok) {
        if (HAS_PERM(currentuser, PERM_LOGINOK) && !HAS_PERM(currentuser, PERM_POST))
            printmenu("<script>alert('您处于戒网中, 或者您被封禁了全站发表文章的权限, 请参看Announce版公告, 期满后在sysop版申请解封. 如有异议, 可在Complain版提出申诉.')</script>\n");
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
    if (currentuser->userlevel&PERM_SYSOP)
        printf("<a href=\"/bbsmboard.php\" target=\"f3\">开设版面</a><br></p>");
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
