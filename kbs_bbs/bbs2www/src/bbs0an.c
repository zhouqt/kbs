#include "bbslib.h"

int main() {
	FILE *fp;
	int count, i, index=0, total=0;
	char *ptr, path[512], names[512], name[1024][80], file[1024][80], buf[512], title[256]=" ";
	char *board;
	init_all();
	printf("<center>\n");
	strsncpy(path, getparm("path"), 511);
	if(strstr(path, "..") || strstr(path, "SYSHome")) /* SYSHome?*/
		http_fatal("此目录不存在");
	sprintf(names, "0Announce%s/.Names", path);
	fp=fopen(names, "r");
	board=getbfroma(path);
	if(fp==0)
		http_fatal("目录不存在");
	while(1)
	{
		if(fgets(buf, 511, fp)==0) break;
		if(!strncmp(buf, "# Title=", 8)) strcpy(title, buf+8);
		if(!strncmp(buf, "Name=", 5) && total<1023) {
			strcpy(name[total], trim(buf+5));
			strcpy(file[total], "");
			total++;
		}
		if(!strncmp(buf, "Path=~", 6) && total>0) {
			sprintf(file[total-1], "%s", trim(buf+6));
		}
	}
	if(strstr(title, "SYSOPS") && !(currentuser->userlevel & PERM_SYSOP))
		http_fatal("错误的目录");
	buf[0]=0;
	if(board[0])
		sprintf(buf, "%s版", board);
	printf("%s --%s精华区<hr color=\"green\">\n", BBSNAME, buf);
	if(total<=0) {
		printf("<br><< 目前没有文章 >>\n");
		http_quit();
	}
       	printf("<table border=\"1\" width=\"610\">\n");
       	printf("<tr><td>编号</td><td>类别</td><td>标题</td><td>作者</td><td>日期</td></tr>");
	for(i=0; i<total; i++) {
		char *id;
		if(strstr(name[i], "SYSOPS")) continue;
		index++;
		if(strlen(name[i])<=39) {
			id="";
		} else {
			name[i][38]=0;
			id=name[i]+39;
			if(!strncmp(id, "BM: ", 4)) id+=4;
			ptr=strchr(id, ')');
			if(ptr) ptr[0]=0;
		}
		printf("<tr><td>%d</td>", index+1);
		sprintf(buf, "0Announce%s%s", path, file[i]);
		if(!file_exist(buf)) {
			printf("<td>[错误] </td><td>%s</td>", nohtml(name[i]));
		} else if(file_isdir(buf)) {
			printf("<td>[目录] </td><td><a href=\"bbs0an?path=%s%s\">%s</a></td>", path, file[i], nohtml(name[i]));
		} else {
			printf("<td>[文件] </td><td><a href=\"bbsanc?path=%s%s\">%s</a></td>", path, file[i], nohtml(name[i]));
		}
		if(id[0]) {
			printf("<td>%s</td>", userid_str(id));
		} else {
			printf("<td>&nbsp;</td>");
		}
		printf("<td>%6.6s %s</td></tr>", wwwCTime(file_time(buf))+4, wwwCTime(file_time(buf))+20);
	}
	printf("</table><br>[<a href=\"javascript:history.go(-1)\">返回上一页</a>] ");
	if(board[0]) printf("[<a href=\"bbsdoc?board=%s\">本讨论区</a>]", board);
	http_quit();
}

int get_count(char *path)
{
	FILE *fp;
	char buf[256];
	int counts=0;

	sprintf(buf, "0Announce%s/.counts", path);
	if(!file_exist(buf)) {
		fp=fopen(buf, "w+");
	} else {
		fp=fopen(buf, "r+");
	}
	if (fp == NULL)
		return 0;
	flock(fileno(fp), LOCK_EX);
	fscanf(fp, "%d", &counts);
	counts++;
	fseek(fp, 0, SEEK_SET);
	fprintf(fp, "%d\n", counts);
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return counts;
}
