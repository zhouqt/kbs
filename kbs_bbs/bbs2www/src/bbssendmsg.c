#include "bbslib.h"

extern char msgerr[255];
int main()
{
	int i;
	int mode, destutmp=0;
	char destid[20], msg[256];

	init_all();
	if(!loginok)
		http_fatal("匆匆过客不能发讯息, 请先登录！");
	strsncpy(destid, getparm("destid"), 13);
	strsncpy(msg, getparm("msg"), 51);
	destutmp=atoi(getparm("destutmp"));
	if(destid[0]==0 || msg[0]==0)
	{
		char buf3[256];
		strcpy(buf3, "<body onload=\"document.form0.msg.focus()\">");
		if(destid[0]==0) strcpy(buf3, "<body onload=\"document.form0.destid.focus()\">");
		printf("%s\n", buf3);
		printf("	<form name=\"form0\" action=\"bbssendmsg\" method=\"post\">"
		"		  <input type=\"hidden\" name=\"destutmp\" value=\"%d\">"
		"	送讯息给: <input name=\"destid\" maxlength=\"12\" value=\"%s\" size=\"12\"><br>"
		"	讯息内容: <input name=\"msg\" maxlength=\"50\" size=\"50\" value=\"%s\"><br>"
		"		  <input type=\"submit\" value=\"确认\" width=\"6\">"
		"	</form> ", destutmp, destid, msg);
		http_quit();
	}
	if(getusernum(destid)<0)
		http_fatal("查无此人");
	/*if (destpid > 100)
		destpid -= 100;*/
	printf("<body onload=\"document.form1.b1.focus()\">\n");
	if(!strcasecmp(destid, currentuser->userid))
		printf("你不能给自己发讯息！");
	else 
	{
		int result;
		if((result = send_msg(getcurruserid(), get_utmpent_num(u_info), destid, destutmp, msg))==1) 
			printf("已经帮你送出消息");
		else if (result == -1 )
			printf ("发送消息失败，%s",msgerr);
		else
			printf("发送消息失败, 此人目前不在线或者无法接受消息");
	}
	printf("<script>top.fmsg.location=\"bbsgetmsg\"</script>\n");
	printf("<br><form name=\"form1\"><input name=\"b1\" type=\"button\" onclick=\"history.go(-2)\" value=\"[返回]\">");
	printf("</form>");
	http_quit();

	return 0;
}
