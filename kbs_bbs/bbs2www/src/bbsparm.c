#include "bbslib.h"

#if 0
char *user_definestr[] = {
        "活动看版",             /* DEF_ACBOARD */
        "使用彩色",             /* DEF_COLOR */
        "编辑时显示状态栏",     /* DEF_EDITMSG */
        "分类讨论区以 New 显示",/* DEF_NEWPOST */
        "选单的讯息栏",         /* DEF_ENDLINE */
        "上站时显示好友名单",   /* DEF_LOGFRIEND */
        "让好友呼叫",           /* DEF_FRIENDCALL */
        "使用自己的离站画面",   /* DEF_LOGOUT */
        "进站时显示备忘录",     /* DEF_INNOTE */
        "离站时显示备忘录",     /* DEF_OUTNOTE */
        "讯息栏模式：呼叫器/人数",/* DEF_NOTMSGFRIEND */
        "菜单模式选择：一般/精简",/* DEF_NORMALSCR */
        "阅读文章是否使用绕卷选择",/* DEF_CIRCLE */
        "阅读文章游标停於第一篇未读",/* DEF_FIRSTNEW */
        "屏幕标题色彩：一般/变换",/* DEF_TITLECOLOR */
        "接受所有人的讯息",     /* DEF_ALLMSG */
        "接受好友的讯息",       /* DEF_FRIENDMSG */
        "收到讯息发出声音",     /* DEF_SOUNDMSG */
        "离站後寄回所有讯息",   /* DEF_MAILMSG */
        "发文章时实时显示讯息",/*"所有好友上站均通知",    DEF_LOGININFORM */
        "菜单上显示帮助信息",   /* DEF_SHOWSCREEN */
        "进站时显示十大新闻",   /* DEF_SHOWHOT */
        "进站时观看留言版",     /* DEF_NOTEPAD*/
        "忽略讯息功能键: Enter/Esc", /* DEF_IGNOREMSG */
        "未使用", /* DEF_IGNOREANSIX */ /* Leeward 98.01.12 */
/*        "禁止运行ANSI扩展指令",  DEF_IGNOREANSIX */ /* Leeward 98.01.12 */
        "进站时观看上站人数统计图",  /* DEF_SHOWSTATISTIC Haohmaru 98.09.24*/
        "未读标记使用 *", /* DEF_UNREADMARK Luzi 99.01.12 */
        "使用GB码阅读", /* DEF_USEGB KCN 99.09.03 */
	"隐藏自己的IP", /* DEF_HIDEIP Haohmaru 99.12.18 */
};
#endif

int main()
{
	int i, type;
	unsigned int perm = 1;

	init_all();
	type=atoi(getparm("type"));
	printf("<center>%s -- 修改个人参数 [使用者: %s]<hr color=\"green\">\n",
			BBSNAME, currentuser->userid);
	if(!loginok)
		http_fatal("匆匆过客不能设定参数");
	if(type)
		return read_form();
	printf("<form action=\"bbsparm?type=1\" method=\"post\">\n");
	printf("<table width=\"610\">\n");
	for(i=0; i < 16; i++)
	{
		char *ptr="";

		printf("<tr>\n");
		if(currentuser->userdefine & perm)
			ptr=" checked";
		printf("<td><input type=\"checkbox\" name=\"perm%d\"%s></td><td>%s</td>\n", i, ptr, user_definestr[i]);
		ptr = "";
		if(currentuser->userdefine & (perm << 16))
			ptr=" checked";
		if (i + 16 < NUMDEFINES)
			printf("<td><input type=\"checkbox\" name=\"perm%d\"%s></td><td>%s</td>\n", i+16, ptr, user_definestr[i+16]);
		else
			printf("<td>&nbsp;</td><td>&nbsp;</td>\n");
		printf("</tr>");
		perm = perm << 1;
	}
	printf("</table>");
	printf("<input type=\"submit\" value=\"确定修改\"></form><br>以上参数大多仅在telnet方式下才有作用\n");
	http_quit();
	return 0;
}

int read_form()
{
	int i, perm=1, def=0;
	char var[100];

	for(i=0; i<NUMDEFINES; i++)
	{
		sprintf(var, "perm%d", i);
		if(strlen(getparm(var))==2)
			def+=perm;
		perm=perm*2;
	}
	currentuser->userdefine=def;
	printf("个人参数设置成功.<br><a href=\"bbsparm\">返回个人参数设置选单</a>");
}
