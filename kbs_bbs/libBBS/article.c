#include "bbs.h"

void cancelpost(char    *board,char *userid,struct fileheader *fh,int     owned,int     autoappend);
int get_postfilename(char* filename,char* direct)
{
    static const char post_sufix[]="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int fp;
    time_t now;
    int i;
    char fname[255];
    int pid=getpid();
    /* ×Ô¶¯Éú³É POST ÎÄ¼þÃû */
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

int cmpname(fhdr,name)  /* Haohmaru.99.3.30.±È½Ï Ä³ÎÄ¼þÃûÊÇ·ñºÍ µ±Ç°ÎÄ¼þ ÏàÍ¬ */
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
        fail = delete_record(direct,sizeof(struct fileheader),ent,cmpname,fileinfo->filename);
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
            fprintf( fn, "\n\n\t\t±¾ÎÄÕÂÒÑ±» %s É¾³ý.\n",
                     currentuser->userid );
            fclose( fn );
        }
*/
        if ((YEA != digestmode) /* ²»¿ÉÒÔÓÃ ¡°NA ==¡± ÅÐ¶Ï£ºdigestmode ÈýÖµ */
                &&!((fileinfo->accessed[0]&FILE_MARKED)
                    &&(fileinfo->accessed[1]& FILE_READ)
                    &&(fileinfo->accessed[0]& FILE_FORWARDED)))
        { /* Leeward 98.06.17 ÔÚÎÄÕªÇøÉ¾ÎÄ²»¼õÎÄÕÂÊýÄ¿ */
            if (owned)
            {
                if ((int)user->numposts > 0 && !junkboard(board))
                {
                    user->numposts--;/*×Ô¼ºÉ¾³ýµÄÎÄÕÂ£¬¼õÉÙpostÊý*/
                }
            } else if ( !strstr(usrid,".")&&BMDEL_DECREASE&&decpost/*°æÖ÷É¾³ý,¼õÉÙPOSTÊý*/){
                struct userec* lookupuser;
                int id = getuser(usrid,&lookupuser);
                if(id && (int)lookupuser->numposts > 0 && !junkboard(board) && strcmp(board, "sysmail") ) /* SYSOP MAIL°æÉ¾ÎÄ²»¼õÎÄÕÂ Bigman: 2000.8.12*/
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


void add_loginfo(char* filepath,struct userec* user,char* currboard,int Anony)    /* POST ×îºóÒ»ÐÐ Ìí¼Ó */
{
    FILE *fp;
    int color,noidboard;
    char fname[STRLEN];

    noidboard=(seek_in_file("etc/anonymous",currboard)&&Anony); /* etc/anonymousÎÄ¼þÖÐ ÊÇÄäÃû°æ°æÃû */
    color=(user->numlogins%7)+31; /* ÑÕÉ«Ëæ»ú±ä»¯ */
    sethomefile( fname, user->userid,"signatures" );
    fp=fopen(filepath,"a");
    if (!dashf(fname)||          /* ÅÐ¶ÏÊÇ·ñÒÑ¾­ ´æÔÚ Ç©Ãûµµ */
            user->signature==0||noidboard)
    {       fputs("\n--\n", fp);
    }else{ /*Bigman 2000.8.10ÐÞ¸Ä,¼õÉÙ´úÂë */
        fprintf(fp,"\n");}
    /* ÓÉBigmanÔö¼Ó:2000.8.10 Announce°æÄäÃû·¢ÎÄÎÊÌâ */
    if (!strcmp(currboard,"Announce"))
        fprintf(fp, "[m[%2dm¡ù À´Ô´:¡¤%s %s¡¤[FROM: %s][m\n"
                ,color,BBS_FULL_NAME,email_domain(),
                NAME_BBS_CHINESE" BBSÕ¾");
    else
        fprintf(fp, "\n[m[%2dm¡ù À´Ô´:¡¤%s %s¡¤[FROM: %s][m\n"
                ,color,BBS_FULL_NAME,email_domain(),(noidboard)?NAME_ANONYMOUS_FROM:fromhost);

    fclose(fp);
    return;
}

void addsignature(FILE *fp,int blank,struct userec* user)
{
    FILE *sigfile;
    int  i,valid_ln=0;
    char tmpsig[MAXSIGLINES][256];
    char inbuf[256];
    char fname[STRLEN];
    char tmp[STRLEN];

    sethomefile( fname, user->userid,"signatures" );
    if ((sigfile = fopen(fname, "r"))== NULL)
    {return;}
    if ( blank ) fputs("\n", fp);
    fputs("--\n", fp);
    for (i=1; i<=(user->signature-1)*MAXSIGLINES&user->signature!=1; i++)
    {
        if (!fgets(inbuf, sizeof(inbuf), sigfile)){
            fclose(sigfile);
            return;}
    }
    for (i=1; i<=MAXSIGLINES; i++) {
        if (fgets(inbuf, sizeof(inbuf), sigfile))
        {
            if(inbuf[0]!='\n')
                valid_ln=i;
            strcpy(tmpsig[i-1],inbuf);
        }
        else break;
    }
    fclose(sigfile);
    for(i=1;i<=valid_ln;i++)
        fputs(tmpsig[i-1], fp);
    /*fclose(sigfile); Leeward 98.03.29: Extra fclose is a BUG! */
}

int write_posts(char *id, char *board, char *title)
{
    char *ptr;
    time_t now;
    struct posttop postlog, pl;

    if(junkboard(board)||normal_board(board)!=1)
        return ;
    now = time(0) ;
    strcpy(postlog.author, id);
    strcpy(postlog.board, board);
    ptr = title;
    if (!strncmp(ptr, "Re: ", 4))
        ptr += 4;
    strncpy(postlog.title, ptr, 65);
    postlog.date = now;
    postlog.number = 1;

    { /* added by Leeward 98.04.25 
    TODO: Õâ¸öµØ·½ÓÐµã²»Í×,Ã¿´Î·¢ÎÄÒª±éÀúÒ»´Î,±£´æµ½.XpostÖÐ,
    ÓÃÀ´Íê³ÉÊ®´ó·¢ÎÄÍ³¼ÆÕë¶ÔID¶ø²»ÊÇÎÄÕÂ.²»ºÃ
    	KCN*/
        char buf[STRLEN];
        int  log = 1;
        FILE *fp = fopen(".Xpost", "r");

        if (fp)
        {
            while (!feof(fp))
            {
                fread(&pl, sizeof(pl), 1, fp);
                if (feof(fp)) break;

                if (!strcmp(pl.title, postlog.title)
                        && !strcmp(pl.author, postlog.author)
                        && !strcmp(pl.board, postlog.board))
                {
                    log = 0;
                    break;
                }
            }
            fclose(fp);
        }

        if (log)
        {
            append_record(".Xpost", &postlog, sizeof(postlog));
            append_record(".post", &postlog, sizeof(postlog));
        }
    }

    append_record(".post.X", &postlog, sizeof(postlog));
}

void write_header(FILE *fp,struct userec* user,int in_mail,char* board,char* title,int Anony,int mode)
{
    int  noname;
    char uid[20] ;
    char uname[40] ;
    time_t now;

    now = time(0) ;
    strncpy(uid,user->userid,20) ;
    uid[19] = '\0' ;
    if (in_mail)
#if defined(MAIL_REALNAMES)
    	strncpy(uname,user->realname,NAMELEN) ;
#else
		strncpy(uname,user->username,NAMELEN) ;
#endif
    else
#if defined(POSTS_REALNAMES)
        strncpy(uname,user->realname,NAMELEN) ;
#else
        strncpy(uname,user->username,NAMELEN) ;
#endif
    /* uid[39] = '\0' ; SO FUNNY:-) ¶¨ÒåµÄ 20 ÕâÀïÈ´ÓÃ 39 !
                        Leeward: 1997.12.11 */
    uname[39] = 0; /* ÆäÊµÊÇÐ´´í±äÁ¿ÃûÁË! ºÙºÙ */
    title[STRLEN-10] = '\0' ;
    noname=seek_in_file("etc/anonymous",board);
    if(in_mail)
        fprintf(fp,"¼ÄÐÅÈË: %s (%s)\n",uid,uname) ;
    else
    {
        if(mode==0&&!(noname&&Anony))
        {
            write_posts(user->userid,board,title);
        }

        if (!strcmp(board,"Announce"))
            /* added By Bigman */
            fprintf(fp,"·¢ÐÅÈË: %s (%s), ÐÅÇø: %s       \n","SYSOP",
                    NAME_SYSOP,board) ;
        else
            fprintf(fp,"·¢ÐÅÈË: %s (%s), ÐÅÇø: %s       \n",(noname&&Anony)?board:uid,
                    (noname&&Anony)?NAME_ANONYMOUS:uname,board) ;
    } 

    fprintf(fp,"±ê  Ìâ: %s\n",title) ;
    fprintf(fp,"·¢ÐÅÕ¾: %s (%24.24s)\n",BBS_FULL_NAME,ctime(&now)) ;
    if(in_mail)
        fprintf(fp,"À´  Ô´: %s \n",fromhost) ;
    fprintf(fp,"\n");

}

void getcross(char* filepath,char* quote_file,struct userec* user,int in_mail,char* board,char* title,int Anony,int mode)      /* °Ñquote_file¸´ÖÆµ½filepath (×ªÌù»ò×Ô¶¯·¢ÐÅ)*/
{
    FILE        *inf, *of;
    char        buf[256];
    char        owner[256];
    int                     count;
    time_t      now;

    now=time(0);
    inf=fopen(quote_file,"r");
    of = fopen( filepath, "w" );
    if(inf==NULL || of ==NULL)
    {
        /*---	---*/
        if(NULL != inf) fclose(inf);
        if(NULL != of ) fclose(of) ;
        /*---	---*/

        report("Cross Post error");
        return ;
    }
    if(mode==0/*×ªÌù*/)
    {
        int normal_file;         
        int header_count;        
        normal_file=1;           

        write_header(of,user,in_mail,board,title,Anony,1/*²»Ð´Èë .posts*/);
        if(fgets( buf, 256, inf ) != NULL)
            { for(count=8;buf[count]!=' ';count++)
                owner[count-8]=buf[count];}
        owner[count-8]='\0';
        if(in_mail==YEA)
            fprintf( of, "[1;37m¡¾ ÒÔÏÂÎÄ×Ö×ªÔØ×Ô [32m%s [37mµÄÐÅÏä ¡¿\n",user->userid);
        else
            fprintf( of, "¡¾ ÒÔÏÂÎÄ×Ö×ªÔØ×Ô %s ÌÖÂÛÇø ¡¿\n",board);
        if (id_invalid(owner)) normal_file=0;
       if (normal_file) {                                                   
         for (header_count=0;header_count<3;header_count++) {               
           if ( fgets( buf, 256, inf ) == NULL) break;/*Clear Post header*/ 
          }                                                        
          if ((header_count!=2)||(buf[0]!='\n')) normal_file=0;    
        }                                                          
        if (normal_file)                                           
            fprintf( of, "¡¾ Ô­ÎÄÓÉ %s Ëù·¢±í ¡¿\n",owner);        
        else                                                       
            fseek(inf,0,SEEK_SET);                                 

    }else if(mode==1/*×Ô¶¯·¢ÐÅ*/)
    {
        fprintf( of,"·¢ÐÅÈË: deliver (×Ô¶¯·¢ÐÅÏµÍ³), ÐÅÇø: %s\n",board);
        fprintf( of,"±ê  Ìâ: %s\n",title);
        fprintf( of,"·¢ÐÅÕ¾: %s×Ô¶¯·¢ÐÅÏµÍ³ (%24.24s)\n\n",BBS_FULL_NAME,ctime(&now));
        fprintf( of,"¡¾´ËÆªÎÄÕÂÊÇÓÉ×Ô¶¯·¢ÐÅÏµÍ³ËùÕÅÌù¡¿\n\n");
    }else if(mode==2)
    {
        write_header(of,user,in_mail,board,title,Anony,0/*Ð´Èë .posts*/);
    }
    while( fgets( buf, 256, inf ) != NULL)
    {
        if((strstr(buf,"¡¾ ÒÔÏÂÎÄ×Ö×ªÔØ×Ô ")&&strstr(buf,"ÌÖÂÛÇø ¡¿"))||(strstr(buf,"¡¾ Ô­ÎÄÓÉ")&&strstr(buf,"Ëù·¢±í ¡¿")))
            continue; /* ±ÜÃâÒýÓÃÖØ¸´ */
        else
            fprintf( of, "%s", buf );
    }
    fclose( inf );
    fclose( of);
    *quote_file = '\0';
}


