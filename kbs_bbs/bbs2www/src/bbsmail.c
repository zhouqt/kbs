#include "bbslib.h"

int main() {
	FILE *fp;
	int filetime, i, start, total, type;
	char *ptr, buf[512], path[80], dir[80];
	struct fileheader *data;
   	init_all();
	if(!loginok) http_fatal("您尚未登录, 请先登录");
	strsncpy(buf, getparm("start"), 10);
	start=atoi(buf);
	if(buf[0]==0) start=999999;
   	printf("<center>\n");
   	printf("%s -- 信件列表 [使用者: %s]<hr color=green>\n", BBSNAME, currentuser->userid);
   	sprintf(dir, "mail/%c/%s/.DIR", toupper(currentuser->userid[0]), currentuser->userid);
   	total=file_size(dir)/sizeof(struct fileheader);
	if(total<0 || total>30000) http_fatal("too many mails");
   	data=(struct fileheader *)calloc(total, sizeof(struct fileheader));
   	if(data==0) http_fatal("memory overflow");
	fp=fopen(dir, "r");
	if(fp==0) http_fatal("dir error");
	total=fread(data, sizeof(struct fileheader), total, fp);
	fclose(fp);
	if(start>total-19) start=total-19;
	if(start<0) start=0;
      	printf("<table width=610>\n");
      	printf("<tr><td>序号<td>状态<td>发信人<td>日期<td>信件标题\n");
      	for(i=start; i<start+19 && i<total; i++) {
	 	int type='N';
	 	printf("<tr><td>%d", i+1);
         	if(data[i].accessed[0] & FILE_READ) type=' ';
         	if(data[i].accessed[0] & FILE_MARKED) type= (type=='N') ? 'M' : 'm';
		printf("<td>%c", type);
 		ptr=strtok(data[i].owner, " (");
		if(ptr==0) ptr=" ";
		ptr=nohtml(ptr);
		printf("<td><a href=bbsqry?userid=%s>%13.13s</a>", ptr, ptr);
	 	filetime=atoi(data[i].filename+2);
         	printf("<td>%12.12s", wwwCTime(filetime)+4);
         	printf("<td><a href=bbsmailcon?file=%s&num=%d>", data[i].filename, i);
	 	if(strncmp("Re: ", data[i].title, 4)) printf("★ ");
         	hprintf("%42.42s", void1(data[i].title));
	 	printf("</a>\n");
      	}
      	free(data);
      	printf("</table><hr>\n");
	printf("[信件总数: %d]", total);
	printf("[<a href=bbspstmail>发送信件</a>]");
	if(start>0) {
		i=start-19;
		if(i<0) i=0;
		printf("[<a href=bbsmail?start=0>第一页</a>] ");
		printf("[<a href=bbsmail?start=%d>上一页</a>] ", i);
	}
	if(start<total-19) {
		i=start+19;
		if(i>total-1) i=total-1;
		printf("[<a href=bbsmail?start=%d>下一页</a>] ", i);
		printf("[<a href=bbsmail>最后一页</a>]");
	}
        printf("<form><input type=submit value=跳转到> 第 <input style='height:20px' type=text name=start size=3> 封</form>");
	http_quit();
}
