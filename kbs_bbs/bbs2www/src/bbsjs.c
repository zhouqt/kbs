#include "bbslib.h"

int main()
{
  	FILE *fp;
    	static char s[300];
	init_all();
	printf("<font style=\"font-size:12px\">\n");
  	printf("<center>欢迎访问[%s], 目前在线人数(www/all) [<font color=\"green\">%d/%d</font>]", 
    		count_www(), count_online());
	printf("</font>");
}

