/*
 * $Id$
 */
#include "bbslib.h"

int main()
{
	int style;
	char buf[STRLEN];

	init_all();
	style = atoi(getparm("STYLE"));
	sprintf(buf, "%d", style);
	setcookie("STYLE", buf);
	printf("<script language=\"Javascript\">\n");
	printf("top.f2.navigate('/cgi-bin/bbs/bbsleft');\n");
	printf("top.f3.navigate(top.f3.location);\n");
	printf("top.f4.navigate(top.f4.location);\n");
	printf("</script>\n");
	http_quit();
}
