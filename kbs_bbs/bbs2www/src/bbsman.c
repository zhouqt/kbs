#include "bbslib.h"

char genbuf[ 1024 ];
char currfile[STRLEN] ;

int del_post(int ent, struct fileheader *fileinfo, char *direct, char *board)
{
    FILE        *fn;
    char        buf[512];
    char        usrid[STRLEN];
    char        *t ;
    int         owned, fail;
	struct userec *user;
	char bm_str[BM_LEN-1];
	struct boardheader    *bp;

	user = getcurrusr();
	bp = getbcache( board );
	memcpy( bm_str, bp->BM, BM_LEN -1);
    if (!strcmp(board, "syssecurity")
            ||!strcmp(board, "junk")
            ||!strcmp(board, "deleted"))    /* Leeward : 98.01.22 */
        return DONOTHING ;

    if( fileinfo->owner[0] == '-')
	{
        return FULLUPDATE;
    }
    owned = isowner(user,fileinfo);
    /* change by KCN  ! strcmp( fileinfo->owner, currentuser->userid ); */
    strcpy(usrid,fileinfo->owner);
    if( !(owned) && !HAS_PERM(currentuser,PERM_SYSOP) )
        if( !chk_currBM(bm_str,currentuser))
        {
            return DONOTHING ;
        }
    if (do_del_post(currentuser,ent,fileinfo,direct,board,0,1)!=0)
    	    return FULLUPDATE ;
    return DIRCHANGED;
}

int main() {
	int i, total=0, mode;
	char board[80], *ptr;
	bcache_t *brd;

	init_all();
	if(!loginok)
		http_fatal("请先登录");
	strsncpy(board, getparm("board"), 60);
	mode=atoi(getparm("mode"));
	brd=getbcache(board);
	if(brd==0)
		http_fatal("错误的讨论区");
	strcpy(board, brd->filename);
	if(!has_BM_perm(currentuser, board))
		http_fatal("你无权访问本页");
	if(mode<=0 || mode>5)
		http_fatal("错误的参数");
	printf("<table>");
	for(i=0; i<parm_num && i<40; i++)
	{
		if(!strncmp(parm_name[i], "box", 3))
		{
			total++;
			if(mode==1)
				do_del(board, parm_name[i]+3);
			if(mode==2)
				do_set(board, parm_name[i]+3, FILE_MARKED);
			if(mode==3)
				do_set(board, parm_name[i]+3, FILE_DIGEST);
			if(mode==5)
				do_set(board, parm_name[i]+3, 0);
		}
	}
	printf("</table>");
	if(total<=0) printf("请先选定文章<br>\n");
	printf("<br><a href=\"bbsmdoc?board=%s\">返回管理模式</a>", board);
	http_quit();
}

int do_del(char *board, char *file) {
	FILE *fp;
	int num=0;
	char path[256], buf[256], dir[256], *id=currentuser->userid;
	struct fileheader f;
	struct userec *u = NULL;
	bcache_t *brd=getbcache(board);

	sprintf(dir, "boards/%s/.DIR", board);
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(dir, "r");
	if(fp==0)
		http_fatal("错误的参数");
	while(1)
	{
		if(fread(&f, sizeof(struct fileheader), 1, fp)<=0) break;
		if(!strcmp(f.filename, file))
		{
			switch(del_post(num, &f, dir, board))
			{
			case DONOTHING:
				http_fatal("你无权删除该文");
				break;
			default:
				printf("<tr><td>%s  </td><td>标题:%s </td><td>删除成功.</td></tr>\n", f.owner, nohtml(f.title));
			}
			return;
		}
		num++;
	}
	fclose(fp);
	printf("<tr><td></td><td>%s</td><td>文件不存在.</td></tr>\n", file);
}

/* 加 G 时并没有 post 到文摘区 */
int do_set(char *board, char *file, int flag)
{
	FILE *fp;
	char path[256], dir[256];
	struct fileheader f;

	sprintf(dir, "boards/%s/.DIR", board);
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(dir, "r+");
	if(fp==0)
		http_fatal("错误的参数");
	while(1)
	{
		if(fread(&f, sizeof(struct fileheader), 1, fp)<=0)
			break;
		if(!strcmp(f.filename, file))
		{
			f.accessed[0]|=flag;
			if(flag==0)
				f.accessed[0]=0;
			fseek(fp, -1*sizeof(struct fileheader), SEEK_CUR);
			fwrite(&f, sizeof(struct fileheader), 1, fp);
			fclose(fp);
			printf("<tr><td>%s</td><td>标题:%s</td><td>标记成功.</td></tr>\n",
					f.owner, nohtml(f.title));
			return;
		}
	}
	fclose(fp);
	printf("<tr><td></td><td></td><td>%s</td><td>文件不存在.</td></tr>\n",
			file);
}
