#include "bbs.h"

int get_postfilename(char* filename,char* direct)
{
    static const char post_sufix[]="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int fp;
    time_t now;
    int i;
    char fname[255];
    int pid=getpid();
    /* 自动生成 POST 文件名 */
    now = time(NULL);
    for (i=0;i<10;i++) {
        sprintf(filename,"M.%d.%c%c",now,post_sufix[(pid+i)%62],post_sufix[(pid*i)%62]);
	sprintf(fname,"%s/%s",direct,filename);
    	if ((fp = open(fname,O_CREAT|O_EXCL|O_WRONLY,0644)) != -1) {
		break;
	};
    }
    if (fp==-1) return -1;
    close(fp) ;
    return 0;
}

int isowner(struct userec* user,struct fileheader* fileinfo)
{
    char buf[25];
    time_t posttime;
    if (strcmp(fileinfo->owner,user->userid))
        return 0;
    posttime = atoi(fileinfo->filename+2);
    if (posttime<user->firstlogin)
        return 0;
    return 1;
}

int cmpname(fhdr,name)  /* Haohmaru.99.3.30.比较 某文件名是否和 当前文件 相同 */
struct fileheader *fhdr ;
char  name[STRLEN];
{
    if(!strncmp(fhdr->filename,name,STRLEN))
        return 1 ;
    return 0 ;
}

int do_del_post(struct userec* user,int ent ,struct fileheader *fileinfo ,char *direct ,char* board,int digestmode,int decpost)
{
    FILE        *fn;
    char        buf[512];
    char        usrid[STRLEN];
    char        *t ;
    int         owned, fail;

    strcpy(buf,direct) ;
    if( (t = strrchr(buf,'/')) != NULL )
        *t = '\0' ;
/* .post.X not use???! KCN
postreport(fileinfo->title, -1, currboard); added by alex, 96.9.12 */
/*    if( keep <= 0 ) {*/
        fail = delete_file(direct,sizeof(struct fileheader),ent,cmpname,fileinfo->filename);
/*
    } else {
        fail = update_file(direct,sizeof(struct fileheader),ent,cmpfilename,
                           cpyfilename);
    }
    */
    owned = isowner(user,fileinfo);
    if( !fail ) {
        cancelpost( board, user->userid, fileinfo, owned ,1);
	 updatelastpost(board);
/*
        sprintf(buf,"%s/%s",buf,fileinfo->filename) ;
        if(keep >0)  if ( (fn = fopen( buf, "w" )) != NULL ) {
            fprintf( fn, "\n\n\t\t本文章已被 %s 删除.\n",
                     currentuser->userid );
            fclose( fn );
        }
*/
        if ((YEA != digestmode) /* 不可以用 “NA ==” 判断：digestmode 三值 */
                &&!((fileinfo->accessed[0]&FILE_MARKED)
                    &&(fileinfo->accessed[1]& FILE_READ)
                    &&(fileinfo->accessed[0]& FILE_FORWARDED)))
        { /* Leeward 98.06.17 在文摘区删文不减文章数目 */
            if (owned)
            {
                if ((int)user->numposts > 0 && !junkboard(board))
                {
                    user->numposts--;/*自己删除的文章，减少post数*/
                }
            } else if ( !strstr(usrid,".")&&BMDEL_DECREASE&&decpost/*版主删除,减少POST数*/){
                struct userec* lookupuser;
                int id = getuser(usrid,&lookupuser);
                if(id && (int)lookupuser->numposts > 0 && !junkboard(board) && strcmp(board, "sysmail") ) /* SYSOP MAIL版删文不减文章 Bigman: 2000.8.12*/
                { /* Leeward 98.06.21 adds above later 2 conditions */
                    lookupuser->numposts--;
                }
            }
        }
    	 bbslog("1bbs","Del '%s' on '%s'",fileinfo->title,board) ; /* bbslog*/
        return 0;
    }
    return -1;
}

/* by ylsdd 
   unlink action is taked within cancelpost if in mail mode,
   otherwise this item is added to the file '.DELETED' under
   the board's directory, the filename is not changed. 
   Unlike the fb code which moves the file to the deleted
   board.
*/
void cancelpost( board, userid, fh, owned ,autoappend)
char    *board, *userid;
struct fileheader *fh;
int     owned;
int     autoappend;
{
    struct fileheader   postfile;
    char oldpath[sizeof(genbuf)];
    struct fileheader* ph;
    time_t now;
#ifdef BBSMAIN
    if(uinfo.mode==RMAIL)  {
       sprintf(oldpath,"mail/%c/%s/%s",toupper(currentuser->userid[0]),
                currentuser->userid,fh->filename);
       unlink(oldpath);
       return;
    }
#endif
    if (autoappend) ph=&postfile;
    else ph=fh;

    sprintf(oldpath, "/board/%s/%s.html", board, fh->filename);
    ca_expire_file(oldpath);

    if (autoappend) {
      bzero(&postfile,sizeof(postfile));
      strcpy( postfile.filename, fh->filename );
      strncpy( postfile.owner, fh->owner, IDLEN+2 );
      postfile.owner[IDLEN+1]=0;
    };
    now=time(NULL);
    sprintf( oldpath, "%-32.32s - %s", fh->title, userid );
    strncpy( ph->title,oldpath, STRLEN );
    ph->title[STRLEN-1]=0;
    ph->accessed[11]=now/(3600*24)%100; /*localtime(&now)->tm_mday;*/
    if (autoappend) {
        setbdir( (owned)?5:4,oldpath, board );
        append_record( oldpath, &postfile, sizeof(postfile) );
    }
}



