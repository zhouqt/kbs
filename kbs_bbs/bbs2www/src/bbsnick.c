#include "bbslib.h"

int main() {
	int i;
	unsigned char nick[80];
	init_all();
	if(!loginok) http_fatal("匆匆过客无法改变昵称");
	strsncpy(nick, getparm("nick"), 30);
	if(nick[0]==0) {
		printf("%s -- 临时改变昵称(环顾四方有效) [使用者: %s]<hr color=green>\n", BBSNAME, currentuser->userid);
		printf("<form action=bbsnick>新昵称<input name=nick size=24 maxlength=24 type=text value='%s'> \n", 
			u_info->username);
		printf("<input type=submit value=确定>");
		printf("</form>");
		http_quit();
	}
	for(i=0; nick[i]; i++)
		if(nick[i]<32 || nick[i]==255) nick[i]=' ';
	strsncpy(u_info->username, nick, 32);
	printf("临时变更昵称成功");
}
