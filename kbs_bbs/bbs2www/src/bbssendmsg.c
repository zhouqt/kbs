#include "bbslib.h"

int main()
{
	int i;
	int mode, destpid=0;
	char destid[20], msg[256];

	init_all();
	if(!loginok) http_fatal("匆匆过客不能发讯息, 请先登录！");
	strsncpy(destid, getparm("destid"), 13);
	strsncpy(msg, getparm("msg"), 51);
	destpid=atoi(getparm("destpid"));
	if(destid[0]==0 || msg[0]==0) {
		char buf3[256];
		strcpy(buf3, "<body onload=\"document.form0.msg.focus()\">");
		if(destid[0]==0) strcpy(buf3, "<body onload=\"document.form0.destid.focus()\">");
		printf("%s\n", buf3);
		printf("
			<form name=\"form0\" action=\"bbssendmsg\" method=\"post\">
				  <input type=\"hidden\" name=\"destpid\" value=\"%d\">
			送讯息给: <input name=\"destid\" maxlength=\"12\" value=\"%s\" size=\"12\"><br>
			讯息内容: <input name=\"msg\" maxlength=\"50\" size=\"50\" value=\"%s\"><br>
				  <input type=\"submit\" value=\"确认\" width=\"6\">
			</form>
		", destpid, destid, msg);
		http_quit();
	}
	if(getusernum(destid)<0) http_fatal("查无此人");
	if (destpid > 100)
		destpid -= 100;
	printf("<body onload=\"document.form1.b1.focus()\">\n");
	/*for(i=0; i<MAXACTIVE; i++)
		if(shm_utmp->uinfo[i].active)
		if(!strcasecmp(shm_utmp->uinfo[i].userid, destid)) {
			if(destpid!=0 && shm_utmp->uinfo[i].pid!=destpid) continue;
			destpid=shm_utmp->uinfo[i].pid;
			if(!(shm_utmp->uinfo[i].pager & ALLMSG_PAGER)) continue;
			if(shm_utmp->uinfo[i].invisible && !(currentuser->userlevel & PERM_SEECLOAK)) continue;
			mode=shm_utmp->uinfo[i].mode;
			if(mode==BBSNET || mode==PAGE) continue;*/
			if(!strcasecmp(destid, currentuser->userid))
				printf("你不能给自己发讯息！");
			else {
				if(send_msg(currentuser->userid, u_info->pid, destid, destpid, msg)==0) 
					printf("已经帮你送出消息");
				else
					printf("发送消息失败, 此人目前不在线或者无法接受消息");
			}
			printf("<script>top.fmsg.location=\"bbsgetmsg\"</script>\n");
			printf("<br><form name=\"form1\"><input name=\"b1\" type=\"button\" onclick=\"history.go(-2)\" value=\"[返回]\">");
			printf("</form>");
			http_quit();
	/*	}*/
	/*printf("此人目前不在线或者无法接受消息");
        printf("<script>top.fmsg.location='bbsgetmsg'</script>\n");
        printf("<br><form name=form1><input name=b1 type=button onclick='history.go(-2)' value='[返回]'>");
        printf("</form>");
	http_quit();*/

	return 0;
}
