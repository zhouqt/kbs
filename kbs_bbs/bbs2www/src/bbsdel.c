#include "bbslib.h"

char genbuf[ 1024 ];
char currfile[STRLEN] ;

/* ent 是 1-based 的*/
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

int main()
{
	FILE *fp;
	bcache_t *brd;
	struct fileheader f;
	struct userec *u = NULL;
	char buf[80], dir[80], path[80], board[80], file[80], *id;
	int num=0;

	init_all();
	if(!loginok) http_fatal("请先登录");
	id=currentuser->userid;
	strsncpy(board, getparm("board"), 60);
	strsncpy(file, getparm("file"), 20);
	brd=getbcache(board);
	if(strncmp(file, "M.", 2) && strncmp(file, "G.", 2))
		http_fatal("错误的参数");
	if(strstr(file, "..")) http_fatal("错误的参数");
	if(brd==0) http_fatal("版面错误");
	if(!haspostperm(currentuser, board)) http_fatal("错误的讨论区");
	sprintf(dir, "boards/%s/.DIR", board);
	sprintf(path, "boards/%s/%s", board, file);
	fp=fopen(dir, "r");
	if(fp==0)
		http_fatal("错误的参数");
	while(1)
	{
		if(fread(&f, sizeof(struct fileheader), 1, fp)<=0)
			break;
		if(!strcmp(f.filename, file))
		{
			switch(del_post(num+1, &f, dir, board))
			{
			case DONOTHING:
				http_fatal("你无权删除该文");
				break;
			default:
				printf("删除成功.<br><a href=\"bbsdoc?board=%s\">返回本讨论区</a>", board);
			}
			http_quit();
		}
		num++;
	}
	fclose(fp);
	printf("文件不存在, 删除失败.<br>\n");
	printf("<a href=\"bbsdoc?board=%s\">返回本讨论区</a>", board);
	http_quit();
}
