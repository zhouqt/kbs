#include "bbslib.h"

int main() {
	char path[80];

	init_all();
	if(!loginok) http_fatal("匆匆过客不能处理讯息, 请先登录");
	/*sethomefile(path, currentuser->userid, "msgfile.me");*/
	setmsgfile(path, currentuser->userid);
	unlink(path);
	sethomefile(path, currentuser->userid,"msgcount");
	unlink(path);
	printf("已删除所有讯息备份");
}
