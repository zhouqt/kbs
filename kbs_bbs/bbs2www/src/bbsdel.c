#include "bbslib.h"

char genbuf[ 1024 ];
char currfile[STRLEN] ;

int
ca_expire_file(const char *URL)
{
    int  r;
    char dir[MAX_CA_PATH_LEN + 64];

/* KCN,speed up expire 
    if (r = ca_badpath(URL))
        return r;
    else
    {
*/
        strcpy(dir, CACHE_ROOT);
        strncat(dir, URL, MAX_CA_PATH_LEN);
        dir[4 + MAX_CA_PATH_LEN] = 0;
	return unlink(dir);
/*
        return ca_rmdir(dir);
    }
*/
}

void
cancelpost( board, userid, fh, owned ,autoappend)
char    *board, *userid;
struct fileheader *fh;
int     owned;
int     autoappend;
{
    struct fileheader   postfile;
    char oldpath[sizeof(genbuf)];
    int         tmpdigestmode;
    struct fileheader* ph;
    time_t now;

    if (autoappend)
		ph=&postfile;
    else
		ph=fh;

    sprintf(genbuf, "/board/%s/%s.html", board, fh->filename);
    ca_expire_file(genbuf);

    if (autoappend)
	{
      bzero(&postfile,sizeof(postfile));
      strcpy( postfile.filename, fh->filename );
      strncpy( postfile.owner, fh->owner, IDLEN+2 );
      postfile.owner[IDLEN+1]=0;
    };
    now=time(NULL);
    sprintf( genbuf, "%-32.32s - %s", fh->title, userid );
    strncpy( ph->title, genbuf, STRLEN );
    ph->title[STRLEN-1] = 0;
    ph->accessed[11] = now/(3600*24)%100; /*localtime(&now)->tm_mday;*/
    if (autoappend)
	{
		if (owned)
        	setbfile( genbuf, board, ".JUNK");
		else
        	setbfile( genbuf, board, ".DELETED");
        append_record( genbuf, &postfile, sizeof(postfile) );
    }
}

int cmpfilename(fhdr)  /* 比较 某文件名是否和 当前文件 相同 */
struct fileheader *fhdr ;
{
    if(!strncmp(fhdr->filename,currfile,STRLEN))
        return 1 ;
    return 0 ;
}

// ent 是 1-based 的
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
    strcpy(buf,direct) ;
    if( (t = strrchr(buf,'/')) != NULL )
        *t = '\0' ;
    sprintf(genbuf,"Del '%s' on '%s'",fileinfo->title,board) ;
    report(genbuf) ;
    postreport(fileinfo->title, -1/* del 1 post*/, board);
    strncpy(currfile,fileinfo->filename,STRLEN) ;
	fail = delete_file(direct,sizeof(struct fileheader),ent,cmpfilename);
    if( !fail )
	{
        cancelpost( board, user->userid, fileinfo, owned ,1);
        sprintf(genbuf,"%s/%s",buf,fileinfo->filename) ;
        if ((fileinfo->filename[0] != 'G')
                &&!((fileinfo->accessed[0]&FILE_MARKED)
                    &&(fileinfo->accessed[1]& FILE_READ)
                    &&(fileinfo->accessed[0]& FILE_FORWARDED)))
        { /* Leeward 98.06.17 在文摘区删文不减文章数目 */
            if (owned)
            {
                if ((int)user->numposts > 0 && !junkboard(board))
                {
                    user->numposts--;/*自己删除的文章，减少post数*/
					save_user_data(user);
                }
            }
			/*版主删除,减少POST数*/
			else if ( !strstr(usrid,".") && BMDEL_DECREASE)
			{
				struct userec* lookupuser;
                int id = getuser(usrid, &lookupuser);

                /* SYSOP MAIL版删文不减文章 Bigman: 2000.8.12*/
				if(id && (int)lookupuser->numposts > 0 && !junkboard(board)
						&& strcmp(board, "sysmail") )
                { /* Leeward 98.06.21 adds above later 2 conditions */
                    lookupuser->numposts--;
                }
            }
        }
        return DIRCHANGED;
    }
    return FULLUPDATE ;
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
