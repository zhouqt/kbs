#include "bbslib.h"

int main()
{
	char filename[80];

	init_all();
	if(!loginok) http_fatal("匆匆过客不能处理讯息，请先登录");
	/*sprintf(filename, "home/%c/%s/msgfile", toupper(currentuser->userid[0]), currentuser->userid);*/
	setmsgfile(filename, currentuser->userid);
	post_mail(currentuser->userid, "所有讯息备份", filename, currentuser->userid, currentuser->username, fromhost, -1);
	unlink(filename);
	sethomefile(filename, currentuser->userid,"msgcount");
	unlink(filename);
	printf("讯息备份已经寄回您的信箱");
	printf("<a href='javascript:history.go(-2)'>返回</a>");
	http_quit();
}
