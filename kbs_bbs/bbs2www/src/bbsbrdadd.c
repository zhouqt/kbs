#include "bbslib.h"

int main()
{
	FILE *fp;
	char file[200], board[200];
	int rv;
	struct boardheader x;

	init_all();
	strsncpy(board, getparm("board"), 32);
	if(!loginok)
		http_fatal("超时或未登录，请重新login");
	if (getboardnum(board, &x) == 0)
		http_fatal("此讨论区不存在");
	load_favboard(0);
	if(!has_read_perm(currentuser, x.filename))
		http_fatal("此讨论区不存在");
	rv = add_favboard(x.filename);
	switch (rv)
	{
	case -2:
		http_fatal("您预定讨论区数目已达上限，不能增加预定");
		break;
	case -1:
		http_fatal("您已经预定该讨论区或者该讨论区不存在");
		break;
	case -3:
		http_fatal("此讨论区不存在");
		break;
	default:
		;/* do nothing*/
	}
	save_favboard();
	printf("<script>top.f2.location='bbsleft'</script>\n");
	printf("预定 %s 讨论区成功<br><a href=\"javascript:history.go(-1)\">"
			"快速返回</a>", x.filename);
	http_quit();
}
