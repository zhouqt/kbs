#include "bbslib.h"

int main() {
	FILE *fp;
	char buf[512], board[80], dir[80], file[80], filename[80], *ptr;
	struct fileheader x;
	int num, tmp, total;
	struct userec *user = NULL;

	init_all();
	strsncpy(board, getparm("board"), 32);
	strsncpy(file, getparm("file"), 32);
	num=atoi(getparm("num"));
	printf("<center>\n");
	if(!has_read_perm(currentuser, board)) http_fatal("错误的讨论区");
	strcpy(board, getbcache(board)->filename);
	printf("%s -- 文章阅读 [讨论区: %s]<hr color=\"green\">", BBSNAME, board);
	/*if(strncmp(file, "M.", 2) && strncmp(file, "G.", 2)) http_fatal("错误的参数1");
	if(strstr(file, "..") || strstr(file, "/")) http_fatal("错误的参数2");*/
	if (valid_filename(file) < 0)
		http_fatal("错误的参数");
	sprintf(dir, "boards/%s/.DIR", board);
	total=file_size(dir)/sizeof(x);
	if(total<=0) http_fatal("此讨论区不存在或者为空");
	printf("<table width=\"610\" border=\"1\">\n");
	printf("<tr><td>\n<pre>");
	sprintf(filename, "boards/%s/%s", board, file);
	fp=fopen(filename, "r");
	if(fp==0) http_fatal("本文不存在或者已被删除");
	while(1) {
		char *id, *s;
		if(fgets(buf, 512, fp)==0) break;
		if(!strncmp(buf, "发信人: ", 8)) {
			ptr=strdup(buf);
			id=strtok(ptr+8, " ");
			s=strtok(0, "");
			if(id==0) id=" ";
			if(s==0) s="\n";
			if(strlen(id)<13 && getuser(id, &user)) {
				printf("发信人: %s%s", userid_str(id), s);
				free(ptr);
				continue;
			}
			free(ptr);
		}
		if(!strncmp(buf, ": ", 2)) printf("<font color=\"808080\">");
		hhprintf("%s", void1(buf));
		if(!strncmp(buf, ": ", 2)) printf("</font>");
	}
	fclose(fp);
	printf("</pre></td></tr>\n</table><hr>\n");
	printf("[<a href=\"bbsfwd?board=%s&file=%s\">转寄/推荐</a>]", board, file);
	printf("[<a href=\"bbsccc?board=%s&file=%s\">转贴</a>]", board, file);
	printf("[<a onclick=\"return confirm('你真的要删除本文吗?')\" href=\"bbsdel?board=%s&file=%s\">删除文章</a>]", board, file);
	printf("[<a href=\"bbsedit?board=%s&file=%s\">修改文章</a>]", board, file);
	fp=fopen(dir, "r+");
	if(fp==0) http_fatal("dir error2");
	if(num>0) {
		fseek(fp, sizeof(x)*(num-1), SEEK_SET);
		fread(&x, sizeof(x), 1, fp);
		printf("[<a href=\"bbscon?board=%s&file=%s&num=%d\">上一篇</a>]", board, x.filename, num-1);
	}
	printf("[<a href=\"bbsdoc?board=%s\">本讨论区</a>]", board);
	if(num<total-1) {
		fseek(fp, sizeof(x)*(num+1), SEEK_SET);
      		fread(&x, sizeof(x), 1, fp);
      		printf("[<a href=\"bbscon?board=%s&file=%s&num=%d\">下一篇</a>]", board, x.filename, num+1);
	}
	if(num>0 && num<=total) {
		fseek(fp, sizeof(x)*num, SEEK_SET);
		fread(&x, sizeof(x), 1, fp);
		/* 去掉人气值功能 */
		brc_initial(currentuser->userid, board);
		brc_add_read(x.filename);
		brc_update(currentuser->userid);
	}
	fclose(fp);
     	ptr=x.title;
     	if(!strncmp(ptr, "Re: ", 4)) ptr+=4;
	ptr[60]=0;
        printf("[<a href=\"bbspst?board=%s&file=%s&userid=%s&title=Re: %s \">回文章</a>]",
               	board, file, x.owner, void1(ptr));
     	printf("[<a href=\"bbstfind?board=%s&title=%s \">同主题阅读</a>]\n", board, void1(ptr));
   	printf("</center>\n"); 
	http_quit();
}
