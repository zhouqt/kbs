#include "bbslib.h"

int main() {
	FILE *fp;
	char buf[512], dir[80], file[80], path[80], *ptr, *id;
	struct fileheader x;
	int num, tmp, total;
	init_all();
	strsncpy(file, getparm("file"), 32);
	num=atoi(getparm("num"));
	printf("<center>\n");
	id=currentuser->userid;
	printf("%s -- 阅读信件 [使用者: %s]<hr>\n", BBSNAME, id);
	if(strncmp(file, "M.", 2)) http_fatal("错误的参数1");
	if(strstr(file, "..") || strstr(file, "/")) http_fatal("错误的参数2");
	sprintf(dir, "mail/%c/%s/.DIR", toupper(id[0]), id);
	total=file_size(dir)/sizeof(x);
	if(total<=0) http_fatal("错误的参数3");
	printf("<table width=610 border=1>\n");
	printf("<tr><td>\n<pre>");
	sprintf(path, "mail/%c/%s/%s", toupper(id[0]), id, file);
	fp=fopen(path, "r");
	if(fp==0) http_fatal("本文不存在或者已被删除");
	while(1) {
		if(fgets(buf, 512, fp)==0) break;
		hhprintf("%s", void1(buf));
	}
	fclose(fp);
	printf("</pre>\n</table><hr>\n");
	printf("[<a onclick='return confirm(\"你真的要删除这封信吗?\")' href=bbsdelmail?file=%s> 删除</a>]", file);
	fp=fopen(dir, "r+");
	if(fp==0) http_fatal("dir error2");
	if(num>0) {
		fseek(fp, sizeof(x)*(num-1), SEEK_SET);
		fread(&x, sizeof(x), 1, fp);
		printf("[<a href=bbsmailcon?file=%s&num=%d>上一篇</a>]", x.filename, num-1);
	}
	printf("[<a href=bbsmail>返回信件列表</a>]");
	if(num<total-1) {
		fseek(fp, sizeof(x)*(num+1), SEEK_SET);
      		fread(&x, sizeof(x), 1, fp);
      		printf("[<a href=bbsmailcon?file=%s&num=%d>下一篇</a>]", x.filename, num+1);
	}
	if(num>=0 && num<total) {
		char title2[80];
		fseek(fp, sizeof(x)*num, SEEK_SET);
		if(fread(&x, sizeof(x), 1, fp)>0) {
			x.accessed[0]|=FILE_READ;
			fseek(fp, sizeof(x)*num, SEEK_SET);
			fwrite(&x, sizeof(x), 1, fp);
			ptr=strtok(x.owner, " (");
			if(ptr==0) ptr=" ";
			strcpy(title2, x.title);
			if(strncmp(x.title, "Re:",3)) sprintf(title2, "Re: %s", x.title);
			title2[60]=0;
			printf("[<a href='bbspstmail?userid=%s&file=%s&title=%s'>回信</a>]",
				ptr, x.filename, title2);
		}
	}
	fclose(fp);
   	printf("</center>\n"); 
	http_quit();
}
