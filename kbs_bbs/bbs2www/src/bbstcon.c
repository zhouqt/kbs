#include "bbslib.h"

int no_re=0;
/*	bbscon?board=xx&file=xx&start=xx 	*/

int main() {
	FILE *fp;
	char title[256], userid[80], buf[512], board[80], dir[80], file[80], filename[80], *ptr;
	struct fileheader x;
	int i, num=0, found=0;
	init_all();
	strsncpy(board, getparm("board"), 32);
	strsncpy(file, getparm("file"), 32);
	printf("<center>\n");
	if(!has_read_perm(currentuser, board)) http_fatal("错误的讨论区");
	strcpy(board, getbcache(board)->filename);
	if(loginok) brc_initial(currentuser->userid, board);
	printf("%s -- 主题文章阅读 [讨论区: %s]<hr color=green>", BBSNAME, board);
	if(strncmp(file, "M.", 2) && strncmp(file, "G.", 2)) http_fatal("错误的参数1");
	if(strstr(file, "..") || strstr(file, "/")) http_fatal("错误的参数2");
	sprintf(dir, "boards/%s/.DIR", board);
	if(!strcmp(board, "noticeboard")) no_re=1;
	fp=fopen(dir, "r+");
	if(fp==0) http_fatal("目录错误");
	while(1) {
		if(fread(&x, sizeof(x), 1, fp)<=0) break;
		num++;
		if(!strcmp(x.filename, file)) {
			ptr=x.title;
			if(!strncmp(ptr, "Re:", 3)) ptr+=4;
			strsncpy(title, ptr, 40);
			(*(int*)(x.title+73))++;
			fseek(fp, -1*sizeof(x), SEEK_CUR);
			fwrite(&x, sizeof(x), 1, fp);
			found=1;
			strcpy(userid, x.owner);
			show_file(board, &x, num-1);
			while(1) {
				if(fread(&x, sizeof(x), 1, fp)<=0) break;
				num++;
				if(!strncmp(x.title+4, title, 39) && !strncmp(x.title, "Re: ", 4))
					show_file(board, &x, num-1);
			}
		}
	}
	fclose(fp);
	if(found==0) http_fatal("错误的文件名");
   	if(!no_re) printf("[<a href='bbspst?board=%s&file=%s&userid=%s&title=%s'>回文章</a>] ",
		board, file, x.owner, title);
	printf("[<a href='javascript:history.go(-1)'>返回上一页</a>]");
	printf("[<a href=bbsdoc?board=%s>本讨论区</a>]", board);
     	ptr=x.title;
     	if(!strncmp(ptr, "Re: ", 4)) ptr+=4;
   	printf("</center>\n"); 
	if(loginok) brc_update(currentuser->userid);
	http_quit();
}

int show_file(char *board, struct fileheader *x, int n) {
	FILE *fp;
	char path[80], buf[512];
	if(loginok) brc_add_read(x->filename);
	sprintf(path, "boards/%s/%s", board, x->filename);
	fp=fopen(path, "r");
	if(fp==0) return;
	printf("<table width=610><pre>\n");
	printf("[<a href=bbscon?board=%s&file=%s&num=%d>本篇全文</a>] ", board, x->filename, n);
	printf("[<a href='bbspst?board=%s&file=%s&title=%s&userid=%s'>回复本文</a>] ", 
		board, x->filename, x->title, x->owner);
	printf("[本篇作者: %s] ", userid_str(x->owner));
	printf("[本篇人气: %d]\n", *(int*)(x->title+73));
	while(1) {
		if(fgets(buf, 500, fp)==0) break;
		if(!strncmp(buf, ": ", 2)) continue;
		if(!strncmp(buf, "【 在 ", 4)) continue;
		hhprintf("%s", buf);
	}
	fclose(fp);
	printf("</pre></table><hr color=green>");
}
