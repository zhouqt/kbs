#include "bbslib.h"

int main()
{
	static char buf[256], buf2[256]=".";
	char toid[13];
	int topid;
	int fd;
	char fname[256];
	FILE *fp;
	off_t pos;
	char *ptr;

	init_all();
	printf("<meta http-equiv=\"pragma\" content=\"no-cache\">");
	printf("<style type=text/css>\n");
        printf("A {color: #0000FF}\n");
        printf("</style>\n");
	if(loginok==0) {
		printf("<body style='BACKGROUND-COLOR: #f8f0f0'>");
		http_quit();
	}
	sethomefile(buf, getcurruserid(), "wwwmsg");
	sethomefile(fname, getcurruserid(), ".wwwmsg.lock");
	if ((fd = open(fname, O_RDWR | O_CREAT, 0600)) < 0)
		goto failed;
	flock(fd, LOCK_EX);
	if(file_size(buf)>0)
	{
		printf("<bgsound src=\"sound/msg.wav\">\n");
		printf("<body onkeypress='checkrmsg(event.keyCode)' style='BACKGROUND-COLOR: #f0ffd0'>");
		if ((fp = fopen(buf, "r+")) == NULL)
			goto failed1;
		/* 改成读入一行, 删掉一行 */
		pos = ftell(fp);
		fgets(buf2, sizeof(buf2), fp);
		eat_file_content(fileno(fp), pos, ftell(fp)-pos);
		fclose(fp);
		printf("<table width=\"100%%\">\n");
		printf("<tr><td>");
		hprintf(buf2);
		printf("</td>\n");
		/* toid and topid need to be modified */
		ptr=strrchr(buf2,'[');
		*ptr = '\0';
		ptr = strrchr(buf2,'[');
		topid = atoi(ptr+1);
		sscanf(buf2+10, "%s", toid);
		printf("<td align=right><a target=f3 href=bbssendmsg?destid=%s&destpid=%d>[回讯息]</a> <a href=bbsgetmsg>[忽略]</a></td></tr></table>\n", toid, topid);
		http_quit();
	}
failed1:
	flock(fd, LOCK_UN);
	close(fd);

failed:
	refreshto("bbsgetmsg", 60);
}
