/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* ¾«»ªÇø Ïà¹Ø º¯Êı */

#include "bbs.h"

#define MAXITEMS        1024
#define PATHLEN         256
#define A_PAGESIZE      (t_lines - 5)

#define ADDITEM         0
#define ADDGROUP        1
#define ADDMAIL         2
#define ADDGOPHER       3

int bmonly=0;
char    *email_domain();
char    Importname[STRLEN];
void    a_menu();
void	a_report();/*Haohmaru.99.12.06.°åÖ÷¾«»ªÇø²Ù×÷¼ÇÂ¼£¬×÷Îª¿¼²é¹¤×÷µÄÒÀ¾İ*/

extern char     BoardName[];
extern void     a_prompt();  /* added by netty */
char		r_genbuf[256];

typedef struct { /* changed by period from 72 to 84 2000-10-17 (%38s + "(BM:   )" + 12*3) */
    char        title[ 84/*72*/ ];
    char        fname[ 80 ];
    char        *host;
    int         port;
} ITEM;

int     a_fmode=1;
int     t_search_down();
int     t_search_up();

typedef struct {
    ITEM        *item[ MAXITEMS ];
    char        mtitle[ STRLEN];
    char        *path;
    int         num, page, now;
    int         level;
} MENU;

void
a_report(s)/* Haohmaru.99.12.06 */
char *s ;
{
    int fd ;

    if((fd = open("a_trace",O_WRONLY|O_CREAT,0644)) != -1 ) {
        char buf[512] ;
        /*char timestr[10], *thetime;*/ /* Leeward 98.04.27 */
        char timestr[24], *thetime;
        time_t dtime;
        time(&dtime);
        thetime = ctime(&dtime);
        /*strncpy(timestr, &(thetime[11]), 8);*/
        strncpy(timestr, thetime, 20);
        /*timestr[8] = '\0';*/
        timestr[20] = '\0';
        flock(fd,LOCK_EX) ;
        lseek(fd,0,SEEK_END) ;
        sprintf(buf,"%s %s %s\n",currentuser->userid, timestr, s) ;
        write(fd,buf,strlen(buf)) ;
        flock(fd,LOCK_UN) ;
        close(fd) ;
        return ;
    }
}

int
valid_fname( str )
char    *str;
{
    char        ch;

    while( (ch = *str++) != '\0' ) {
        if( (ch >= 'A' && ch <= 'Z' ) || (ch >= 'a' && ch <= 'z' ) ||
                strchr( "0123456789@[]-._", ch ) != NULL ) {
            ;
        } else {
            return 0;
        }
    }
    return 1;
}

void
a_showmenu( pm )   /* ¾«»ªÇø ²Ëµ¥ ×´Ì¬ */
MENU    *pm;
{
    struct stat st;
    struct tm   *pt;
    char        title[ STRLEN*2 ],kind[20];
    char        fname[ STRLEN ];
    char        ch;
    char        buf[STRLEN],genbuf[STRLEN*2];
    time_t      mtime;
    int         n;
    int chkmailflag=0;

    clear();
    chkmailflag=chkmail();

    if(chkmailflag==2)/*Haohmaru.99.4.4.¶ÔÊÕĞÅÒ²¼ÓÏŞÖÆ*/
    {
        prints("[5m");
        sprintf(genbuf,"[ÄúµÄĞÅÏä³¬¹ıÈİÁ¿,²»ÄÜÔÙÊÕĞÅ!]");
    }
    else if(chkmailflag)
    {
        prints("[5m");
        sprintf(genbuf,"[ÄúÓĞĞÅ¼ş]");
    }
    else
        strncpy(genbuf,pm->mtitle,STRLEN*2-1);
    sprintf( buf, "%*s", (80-strlen(genbuf))/2, "" );
    prints( "[44m%s%s%s[m\n",buf, genbuf,buf );
    prints("            F ¼Ä»Ø×Ô¼ºµÄĞÅÏä©§¡ü¡ı ÒÆ¶¯©§¡ú <Enter> ¶ÁÈ¡©§¡û,q Àë¿ª[m\n");
    prints( "[44m[37m ±àºÅ  %-45s Õû  Àí           %8s [m","[Àà±ğ] ±ê    Ìâ"
            ,a_fmode == 2 ? "µµ°¸Ãû³Æ" : "±à¼­ÈÕÆÚ");
    prints( "\n" );
    if( pm->num == 0 )
        prints( "      << Ä¿Ç°Ã»ÓĞÎÄÕÂ >>\n" );
    for( n = pm->page; n < pm->page + 19 && n < pm->num; n++ ) {
        strncpy( title, pm->item[n]->title ,STRLEN*2-1);
        if( a_fmode ) {
            sprintf( fname,"%s", pm->item[n]->fname );
            sprintf( genbuf, "%s/%s", pm->path, fname );
            if( a_fmode == 2 ) {
                ch = (dashf( genbuf ) ? ' ' : (dashd( genbuf ) ? '/' : ' '));
                fname[10]='\0';
            } else {
                if( dashf( genbuf )||dashd( genbuf ) ) {
                    stat( genbuf, &st );
                    mtime = st.st_mtime;
                }else
                    mtime=time(0);

                pt = localtime( &mtime );
                sprintf( fname, "[1m%04d[m.[1m%02d[m.[1m%02d[m", pt->tm_year+1900,
                         pt->tm_mon+1, pt->tm_mday );
                ch=' ';
            }
            if( pm->item[n]->host != NULL) {
                strcpy( kind, "[[33mÁ¬Ïß[m]" );
            }else if(dashf(genbuf)){
                strcpy( kind, "[[36mÎÄ¼ş[m]" );
            }else if( dashd( genbuf ) ) {
                strcpy( kind, "[Ä¿Â¼]" );
            } else {
                strcpy( kind, "[[32m´íÎó[m]" );
            }
            if(!strncmp(title,"[Ä¿Â¼] ",7)||!strncmp(title,"[ÎÄ¼ş] ",7)
                    ||!strncmp(title,"[Á¬Ïß] ",7))
                sprintf( genbuf, "%-s %-55.55s%-s%c",kind, title+7, fname, ch );
            else
                sprintf( genbuf, "%-s %-55.55s%-s%c",kind, title, fname, ch );
            strncpy( title, genbuf ,STRLEN*2-1);
        }
        prints( "  %3d  %s\n", n+1, title);
    }
    clrtobot();
    move( t_lines-1, 0 );
    prints( "%s", (pm->level & PERM_BOARDS) ?
            "[31m[44m[°å  Ö÷]  [33mËµÃ÷ h ©¦ Àë¿ª q,¡û ©¦ ĞÂÔöÎÄÕÂ a ©¦ ĞÂÔöÄ¿Â¼ g ©¦ ±à¼­µµ°¸ e        [m" :
            "[31m[44m[¹¦ÄÜ¼ü] [33m ËµÃ÷ h ©¦ Àë¿ª q,¡û ©¦ ÒÆ¶¯ÓÎ±ê k,¡ü,j,¡ı ©¦ ¶ÁÈ¡×ÊÁÏ Rtn,¡ú         [m" );
}

void
a_additem( pm, title, fname ,host ,port) /* ²úÉúITEM object,²¢³õÊ¼»¯*/
MENU    *pm;
char    *title, *fname, *host;
int     port;
{
    ITEM        *newitem;

    if( pm->num < MAXITEMS ) {
        newitem = (ITEM *) malloc( sizeof(ITEM) );
        strncpy( newitem->title, title,sizeof(newitem->title)-1 );
        if(host!=NULL)
        {
            newitem->host=(char *)malloc(sizeof(char)*(strlen(host)+1));
            strcpy( newitem->host, host); 
        }else
            newitem->host=host;
        newitem->port=port;
        strncpy( newitem->fname, fname,sizeof(newitem->fname)-1 );
        pm->item[ (pm->num)++ ] = newitem;
    }
}

int
a_loadnames( pm )  /* ×°Èë .Names */
MENU    *pm;
{
    FILE        *fn;
    ITEM        litem;
    char        buf[ PATHLEN ], *ptr;
    char        hostname[STRLEN];

    pm->num = 0;
    sprintf( buf, "%s/.Names", pm->path ); /*.Names¼ÇÂ¼²Ëµ¥ĞÅÏ¢*/
    if( (fn = fopen( buf, "r" )) == NULL )
        return 0;
    hostname[0]='\0';
    while( fgets( buf, sizeof(buf), fn ) != NULL ) {
        if( (ptr = strchr( buf, '\n' )) != NULL )
            *ptr = '\0';
        if( strncmp( buf, "Name=", 5 ) == 0 ) {
            strncpy( litem.title, buf + 5, sizeof(litem.title) );
        } else if( strncmp( buf, "Path=", 5 ) == 0 ) {
            if( strncmp( buf, "Path=~/", 7 ) == 0 )
                strncpy( litem.fname, buf + 7, sizeof(litem.fname) );
            else
                strncpy( litem.fname, buf + 5, sizeof(litem.fname) );
            if((!strstr(litem.title,"(BM: BMS)")||HAS_PERM(currentuser,PERM_BOARDS))&&
                 (!strstr(litem.title,"(BM: SYSOPS)")||HAS_PERM(currentuser,PERM_SYSOP))&&
 	        (!strstr(litem.title,"(BM: ZIXIAs)")||HAS_PERM(currentuser,PERM_SECANC)))
            {
                if(strstr(litem.fname,"!@#$%")) /*È¡ host & port */
                {
                    char *ptr1,*ptr2,gtmp[STRLEN];
                    strncpy(gtmp,litem.fname,STRLEN-1);
                    ptr1=strtok(gtmp,"!#$%@");
                    strcpy(hostname,ptr1);
                    ptr2=strtok(NULL,"@");
                    strncpy(litem.fname,ptr2,sizeof(litem.fname)-1);
                    litem.port=atoi(strtok(NULL,"@"));
                }
                a_additem( pm, litem.title, litem.fname ,(strlen(hostname)==0)? /*²úÉúITEM*/
                           NULL:hostname,litem.port);
            }
            hostname[0]='\0';
        } else if( strncmp( buf, "# Title=", 8 ) == 0 ) {
            if( pm->mtitle[0] == '\0' )
                strcpy( pm->mtitle, buf + 8 );
        }else if( strncmp( buf, "Host=", 5 ) == 0 ) {
            strcpy( hostname, buf + 5 );
        }else if( strncmp( buf, "Port=", 5 ) == 0 ) {
            litem.port=atoi(buf + 5);
        }
    }
    fclose( fn );
    return 1;
}

void
a_savenames( pm ) /*±£´æµ±Ç°MENUµ½ .Names */
MENU    *pm;
{
    FILE        *fn;
    ITEM        *item;
    char        fpath[ PATHLEN ];
    int         n;

    sprintf( fpath, "%s/.Names", pm->path );
    if( (fn = fopen( fpath, "w" )) == NULL )
        return;
    fprintf( fn, "#\n" );
    if(!strncmp(pm->mtitle,"[Ä¿Â¼] ",7)||!strncmp(pm->mtitle,"[ÎÄ¼ş] ",7)
            ||!strncmp(pm->mtitle,"[Á¬Ïß] ",7))
    {
        fprintf( fn, "# Title=%s\n", pm->mtitle+7 );
    }
    else
    {
        fprintf( fn, "# Title=%s\n", pm->mtitle );
    }
    fprintf( fn, "#\n" );
    for( n = 0; n < pm->num; n++ ) {
        item = pm->item[n];
        if(!strncmp(item->title,"[Ä¿Â¼] ",7)||!strncmp(item->title,"[ÎÄ¼ş] ",7)
                ||!strncmp(item->title,"[Á¬Ïß] ",7))
        {
            fprintf( fn, "Name=%s\n",       item->title+7 );
        }
        else
            fprintf( fn, "Name=%s\n",       item->title );
        if(item->host!=NULL)
        {
            fprintf( fn, "Host=%s\n",     item->host );
            fprintf( fn, "Port=%d\n",     item->port );
            fprintf( fn, "Type=1\n");
            fprintf( fn, "Path=%s\n",     item->fname );
        }else
            fprintf( fn, "Path=~/%s\n",     item->fname );
        fprintf( fn, "Numb=%d\n",       n+1 );
        fprintf( fn, "#\n" );
    }
    fclose( fn );
    chmod( fpath, 0644 );
}

void
a_prompt( bot, pmt, buf ) /* ¾«»ªÇø×´Ì¬ÏÂ ÊäÈë*/
int     bot;
char    *pmt, *buf;
{
    move(t_lines+bot, 0);
    clrtoeol();
    getdata( t_lines+bot, 0, pmt, buf, 39, DOECHO, NULL ,YEA);
}

/* a_SeSave ÓÃÀ´É¾³ı´æµ½Ôİ´æµµÊ±µÄÎÄ¼şÍ·ºÍÎ² Life 1997.4.6 */
int a_SeSave( path, key, fileinfo ,nomsg)
char    *path, *key;
struct fileheader *fileinfo;
int    nomsg;
{

    char        board[ STRLEN ];
    char        ans[ STRLEN ];
    FILE        *inf, *outf;
    char        qfile[ STRLEN ], filepath[ STRLEN ];
    char        buf[256], *ptr;

    sprintf( qfile, "boards/%s/%s", key, fileinfo->filename);
    sprintf( filepath, "tmp/se.%s", currentuser->userid );
    outf = fopen( filepath, "w" );
    if( *qfile != '\0' && (inf = fopen( qfile, "r" )) != NULL ) {
        fgets( buf, 256, inf );
        fprintf( outf, "%s", buf );
        fprintf( outf, "\n" );
        while (fgets( buf, 256, inf ) != NULL)
            if( buf[0] == '\n' )  break;


        while( fgets( buf, 256, inf ) != NULL )
        {
            if( strcmp( buf, "--\n" ) == 0 )
                break;
            if( buf[ 250 ] != '\0' )
                strcpy( buf+250, "\n" );
            fprintf( outf, "%s", buf );
        }
        fprintf( outf, "\n\n" );
        fclose( inf );
    }
    fclose( outf );

    sprintf( board, "tmp/bm.%s", currentuser->userid );
    if( dashf( board ) ) {
        sprintf ( genbuf, "Òª¸½¼ÓÔÚ¾ÉÔİ´æµµÖ®ááÂğ?(Y/N/C) [Y]: " );
        if(!nomsg)
            a_prompt( -1, genbuf, ans );
        /*if( ans[0] == 'N' || ans[0] == 'n' ||nomsg) {*/
        /* Leeward 98.04.16: fix bugs */
        if( (ans[0] == 'N' || ans[0] == 'n') && (!nomsg) ) {
		/*
            sprintf( genbuf, "/bin/cp -r %s  tmp/bm.%s", filepath, currentuser->userid );
	    */
            sprintf( genbuf, "tmp/bm.%s", filepath, currentuser->userid );
	    f_cp(filepath,genbuf,0);
        }
        else if(ans[0] == 'C' || ans[0] == 'c')
            return 1;
        else
        {
            sprintf( genbuf, "/bin/cat %s >> tmp/bm.%s", filepath, currentuser->userid );
            system( genbuf );
        }
    }
    else {
	    /*
        sprintf( genbuf, "/bin/cp -r %s  tmp/bm.%s", filepath , currentuser->userid );
	*/
	    f_cp(filepath,genbuf,0);
    }
    sprintf( genbuf, " ÒÑ½«¸ÃÎÄÕÂ´æÈëÔİ´æµµ, Çë°´ÈÎºÎ¼üÒÔ¼ÌĞø << " );
    unlink( filepath );
    sprintf(r_genbuf,"½« %s ´æÈëÔİ´æµµ",filepath);
    a_report(r_genbuf);
    if(!nomsg)
        a_prompt( -1, genbuf, ans );
    return 1;
}


/* added by netty to handle post saving into (0)Announce */
int
a_Save(char    *path,char *key,struct fileheader *fileinfo,int nomsg,
	char *direct,int ent)

{

    char        board[ STRLEN ];
    char        ans[ STRLEN ];

    sprintf( board, "tmp/bm.%s", currentuser->userid );
    if( dashf( board ) ) {
        sprintf ( genbuf, "Òª¸½¼ÓÔÚ¾ÉÔİ´æµµÖ®ºóÂğ?(Y/N/C) [Y]: " );
        if(!nomsg)
            a_prompt( -1, genbuf, ans );
        /*if( ans[0] == 'N' || ans[0] == 'n' ||nomsg) {*/
        /* Leeward 97.11.18: fix bugs */
        if( (ans[0] == 'N' || ans[0] == 'n') && (!nomsg) ) {
		/*
            sprintf( genbuf, "/bin/cp -r boards/%s/%s  tmp/bm.%s", key , fileinfo->filename , currentuser->userid );
	    */
            sprintf( genbuf, "boards/%s/%s", key, fileinfo->filename);
            sprintf( board, "tmp/bm.%s", currentuser->userid );
	    f_cp(genbuf,board,0);
        }
        else if(ans[0] == 'C' || ans[0] == 'c')
            return 1;
        else
        {
            sprintf( genbuf, "/bin/cat boards/%s/%s >> tmp/bm.%s", key , fileinfo->filename , currentuser->userid );
    	system( genbuf );
            /*                   sprintf( genbuf, "/bin/cp -r boards/%s/%s  tmp/bm.%s", key , fileinfo->filename , currentuser->userid );
            */
        }
    }
    else {
	    /*
        sprintf( genbuf, "/bin/cp -r boards/%s/%s  tmp/bm.%s", key , fileinfo->filename , currentuser->userid );
	*/
        sprintf( genbuf, "boards/%s/%s", key, fileinfo->filename);
        sprintf( board, "tmp/bm.%s", currentuser->userid );
	f_cp(genbuf,board,0);
    }
    sprintf( genbuf, " ÒÑ½«¸ÃÎÄÕÂ´æÈëÔİ´æµµ, Çë°´ÈÎºÎ¼üÒÔ¼ÌĞø << " );
    sprintf(r_genbuf,"½« boards/%s/%s ´æÈëÔİ´æµµ",key,fileinfo->filename);
    fileinfo->accessed[0] |= FILE_IMPORTED;
    substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
    a_report(r_genbuf);
    if(!nomsg)
        a_prompt( -1, genbuf, ans );
    return 1;
}

/* added by netty to handle post saving into (0)Announce */
int
a_Import( path, key, fileinfo ,nomsg, direct, ent)
char    *path, *key;
struct fileheader *fileinfo;
int nomsg;
char *direct;  /* Leeward 98.04.15 */
int ent;
{

    FILE        *fn;
    char        fname[ STRLEN ],*ip,bname[PATHLEN];
    char        buf[ PATHLEN ], *ptr;
    int         len, ch;
    MENU        pm;
    char        ans[ STRLEN ];

    modify_user_mode( CSIE_ANNOUNCE );
    len = strlen( key );
    sprintf( buf, "%s/.Search", path );
    if((fn = fopen( buf, "r" )) != NULL ) {
        while( fgets( buf, sizeof( buf ), fn ) != NULL ) {
            if( strncmp( buf, key, len ) == 0 && buf[len] == ':' &&
                    (ptr = strtok( &buf[ len+1 ], " \t\n" )) != NULL ) {
                sprintf( Importname, "%s/%s", path, ptr );
                fclose( fn );

                /* Leeward: 97.12.17: ¶Ô°æÖ÷µÄ¶à¸ö´°¿ÚÍ¬²½Ë¿Â· */

                /* by zixia: Ïà¶ÔÂ·¾­ sprintf(genbuf, "%s/%s", BBSHOME, Importname); */
		sprintf(genbuf, "%s", Importname);
                ptr = strstr(genbuf, ".faq/");
                if (ptr)
                {
                    if (ptr = strchr(ptr + 5, '/'))
                        strcpy(ptr + 1, ".BMpath");
                    else
                        strcat(genbuf, "/.BMpath");
                    fn = fopen(genbuf, "rt");
                    if (fn)
                    {
                        fgets(netty_path, 256, fn);
                        fclose(fn);
                    }
                }

                if ( netty_path[0]!='\0') {
                    /* Ö±½Ó¼ÓÈëµ½¾«»ªÇøÄÚ£¬²»ÓÃÈ·ÈÏ Life */
                    pm.path=netty_path;
                }
                /* µ±È»¾Í×¢µôÁËÀ²¡«¡«¡«¡«·Ï»° ^_&
                                     sprintf ( genbuf, "½«¸ÃÎÄÕÂ·Å½ø %s,È·¶¨Âğ?(Y/N) [N]: " , netty_path );
                                     if(!nomsg)
                                         a_prompt( -1, genbuf, ans );
                                     if( ans[0] == 'Y' || ans[0] == 'y' || nomsg) {
                                        pm.path=netty_path;
                                     }
                                     else {
                                        sprintf( genbuf, "Äã¿ÉÒÔµ½¾«»ªÇøÉè¶¨±ğµÄµ±Ç°Â·¾¶ ,Çë°´ÈÎºÎ¼üÒÔ½áÊø ..");
                                        a_prompt( -1, genbuf, ans );
                                        return 1;
                                     }
                                   }  
                */
                else {
                    sprintf ( genbuf, "½«¸ÃÎÄÕÂ·Å½ø %s,È·¶¨Âğ?(Y/N) [N]: " , Importname );
                    if(!nomsg)
                        a_prompt( -1, genbuf, ans );
                    if( ans[0] == 'Y' || ans[0] == 'y' ||nomsg) {
                        pm.path =  Importname;
                    }
                    else {
                        sprintf( genbuf, "Äã¸Ä±äĞÄÒâÁË?? ,Çë°´ÈÎºÎ¼üÒÔ½áÊø << " , fileinfo->title );
                        a_prompt( -1, genbuf, ans );
                        return 1;
                    }
                }
                strcpy( pm.mtitle, "" );
                a_loadnames ( &pm );
                strcpy(fname,fileinfo->filename);
                sprintf(bname,"%s/%s",pm.path , fname);
                ip=&fname[strlen(fname)-1];
                while(dashf(bname))
                {
                    if(*ip == 'Z')
                        ip++,*ip = 'A', *(ip + 1) = '\0' ;
                    else
                        (*ip)++ ;
                    sprintf(bname,"%s/%s",pm.path , fname);
                }
                sprintf( genbuf, "%-38.38s %s ", fileinfo->title , currentuser->userid );
                a_additem( &pm, genbuf , fname ,NULL,0);
                a_savenames( &pm );
		/*
                sprintf( genbuf, "/bin/cp -r boards/%s/%s %s", key , fileinfo->filename , bname );
		*/
                sprintf( genbuf, "boards/%s/%s", key , fileinfo->filename );
		f_cp(genbuf,bname,0);

                /* Leeward 98.04.15 */
                sprintf(genbuf, " ÊÕÈë¾«»ªÇøÄ¿Â¼ %s, Çë°´ Enter ¼ÌĞø << " , /*fileinfo->title,*/ pm.path);
                sprintf(r_genbuf,"½« boards/%s/%s ÊÕÈëÄ¿Â¼ %s",key,fileinfo->filename,pm.path+17);
                a_report(r_genbuf);
                if(!nomsg)
                    a_prompt( -1, genbuf, ans );

                /* Leeward 98.04.15 add below FILE_IMPORTED */
                fileinfo->accessed[0] |= FILE_IMPORTED;
                substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);

                for( ch = 0; ch < pm.num; ch++ )
                    free( pm.item[ ch ] );
                return 1;
            }
        }
        /*---	added by period	2000-09-21	---*/
        fclose( fn );
        /*---	---*/
    }
    return 0;
}

int
a_menusearch( path, key, level )
char    *path, *key;
int     level;
{
    FILE        *fn;
    char        bname[ STRLEN ],bpath[ STRLEN ];
    char        buf[ PATHLEN ], *ptr;
    int         len;
    struct  stat st;
    struct boardheader fhdr;

    if( key == NULL ) {
        key = bname;
        a_prompt( -1, "ÊäÈëÓûËÑÑ°Ö®ÌÖÂÛÇøÃû³Æ: ", key );
    }

    setbpath( bpath, key);
    if((*key== '\0') || (stat(bpath,&st) == -1)) /* ÅĞ¶ÏboardÊÇ·ñ´æÔÚ */
        return 0;
    if(!(st.st_mode & S_IFDIR)) 
        return 0;
    if (getboardnum(key,&fhdr)==0) return 0;


    if (!((fhdr.level & PERM_POSTMASK) || HAS_PERM(currentuser,fhdr.level)
            ||(fhdr.level & PERM_NOZAP))) {
	return 0;
    }


    len = strlen( key );
    sprintf( buf, "%s/.Search", path );
    if( len > 0 && (fn = fopen( buf, "r" )) != NULL ) {
        while( fgets( buf, sizeof( buf ), fn ) != NULL ) {
            if( strncmp( buf, key, len ) == 0 && buf[len] == ':' &&
                    (ptr = strtok( &buf[ len+1 ], " \t\n" )) != NULL ) {
                sprintf( bname, "%s/%s", path, ptr );
                fclose( fn );
                a_menu( "", bname, level ,0);
                return 1;
            }
        }
        /*---	added by period	2000-09-21	---*/
        fclose( fn );
        /*---	---*/
    }
    return 0;
}

void
a_forward( path, pitem, mode )
char    *path;
ITEM    *pitem;
int     mode;
{
    struct boardheader fhdr;
    char        fname[ PATHLEN ], *mesg;

    sprintf( fname, "%s/%s", path, pitem->fname );
    if( dashf( fname ) ) {
        strncpy( fhdr.title,    pitem->title, STRLEN );
        strncpy( fhdr.filename, pitem->fname, STRLEN );
        switch( doforward( path, &fhdr, mode ) ) {
        case  0: mesg = "ÎÄÕÂ×ª¼ÄÍê³É!\n";          break;
        case -1: mesg = "system error!!.\n";        break;
        case -2: mesg = "invalid address.\n";       break;
        case -552: prints("\n[1m[33mĞÅ¼ş³¬³¤£¨±¾Õ¾ÏŞ¶¨ĞÅ¼ş³¤¶ÈÉÏÏŞÎª %d ×Ö½Ú£©£¬È¡Ïû×ª¼Ä²Ù×÷[0m[0m\n\nÇë¸æÖªÊÕĞÅÈË£¨Ò²Ğí¾ÍÊÇÄú×Ô¼º°É:PP£©£º\n\n*1* Ê¹ÓÃ [1m[33mWWW[0m[0m ·½Ê½·ÃÎÊ±¾Õ¾£¬ËæÊ±¿ÉÒÔ±£´æÈÎÒâ³¤¶ÈµÄÎÄÕÂµ½×Ô¼ºµÄ¼ÆËã»ú£»\n*2* Ê¹ÓÃ [1m[33mpop3[0m[0m ·½Ê½´Ó±¾Õ¾ÓÃ»§µÄĞÅÏäÈ¡ĞÅ£¬Ã»ÓĞÈÎºÎ³¤¶ÈÏŞÖÆ¡£\n*3* Èç¹û²»ÊìÏ¤±¾Õ¾µÄ WWW »ò pop3 ·şÎñ£¬ÇëÔÄ¶Á [1m[33mAnnounce[0m[0m °æÓĞ¹Ø¹«¸æ¡£\n", MAXMAILSIZE); break;
        default: mesg = "È¡Ïû×ª¼Ä¶¯×÷.\n";
        }
        prints( mesg );
    } else {
        move( t_lines-1, 0 );
        prints( "ÎŞ·¨×ª¼Ä´ËÏîÄ¿.\n" );
    }
    pressanykey();
}

/*
void
a_download( fname )
char    *fname;
{
    char        ans[ STRLEN ], *ptr;

    move( t_lines-1, 0 );
    if( dashf( fname ) ) {
        ptr = fname;
        if( (ptr = strrchr( fname, '/' )) != NULL )
            ptr++;
        sprintf( genbuf, "Ê¹ÓÃ Z-Modem ´«ËÍ %s µµ°¸Âğ? (Y/N) [N]: ", ptr );
        a_prompt( -2, genbuf, ans );
        if( *ans == 'y' || *ans == 'Y' ) {
            sprintf( genbuf, "bin/sz -ve %s", fname );
            system( genbuf );
        }
    } else {
        prints( "ÎŞ·¨´«ËÍ´ËÏîÄ¿.\n" );
        egetch();
    }
}
*/

void
a_newitem( pm, mode )  /* ÓÃ»§´´½¨ĞÂµÄ ITEM */
MENU    *pm;
int     mode;
{
    char uident[STRLEN] ;
    char        board[ STRLEN ], title[ STRLEN ];
    char        fname[ STRLEN ], fpath[ PATHLEN ],fpath2[ PATHLEN ];
    char        *mesg, *domain;
    FILE        *pn;
    char        ans[ STRLEN ];

    pm->page = 9999;
    switch( mode ) {
    case ADDITEM:
        mesg = "ÇëÊäÈëĞÂÎÄ¼şÖ®Ó¢ÃûÃû³Æ(¿Éº¬Êı×Ö)£º";     break;
    case ADDGROUP:
        mesg = "ÇëÊäÈëĞÂÄ¿Â¼Ö®Ó¢ÃûÃû³Æ(¿Éº¬Êı×Ö)£º";     break;
    case ADDMAIL:
        sprintf( board, "tmp/bm.%s", currentuser->userid );
        if( !dashf( board ) ) {
            sprintf( genbuf, "°¥Ñ½!! ÇëÏÈÖÁ¸Ã°å(ÌÖÂÛÇø)½«ÎÄÕÂ´æÈëÔİ´æµµ! << "  );
            a_prompt( -1, genbuf, ans );
            return;
        }
        mesg = "ÇëÊäÈëÎÄ¼şÖ®Ó¢ÎÄÃû³Æ(¿Éº¬Êı×Ö)£º";       break;
    }
    a_prompt( -2, mesg, fname );
    if( *fname == '\0' )  return;
    sprintf( fpath, "%s/%s", pm->path, fname );
    if( !valid_fname( fname ) ) {
        sprintf( genbuf, "°¥Ñ½!! Ãû³ÆÖ»ÄÜ°üº¬Ó¢ÎÄ¼°Êı×Ö! << "  );
        a_prompt( -1, genbuf, ans );
    } else if( dashf( fpath ) || dashd( fpath ) ) {
        sprintf( genbuf, "°¥Ñ½!! ÏµÍ³ÄÚÒÑ¾­ÓĞ %s Õâ¸öÎÄ¼ş´æÔÚÁË! << " , fname );
        a_prompt( -1, genbuf, ans );
    } else {
        mesg = "ÇëÊäÈëÎÄ¼ş»òÄ¿Â¼Ö®ÖĞÎÄÃû³Æ <<  ";
        a_prompt( -1, mesg, title );
        if( *title == '\0' )  return;
        sprintf(r_genbuf,"´´½¨ĞÂÎÄ¼ş»òÄ¿Â¼ %s (±êÌâ: %s)",fpath+17,title);
        a_report(r_genbuf);
        switch( mode ) {
        case ADDITEM:
            /*vedit( fpath, 0 );*/
            if (-1 == vedit(fpath, 0))
                return; /* Leeward 98.06.12 fixes bug */
            chmod( fpath, 0644 );
            break;
        case ADDGROUP:
            mkdir( fpath, 0755 );
            chmod( fpath, 0755 );
            break;
        case ADDMAIL:
	    /*
            sprintf( genbuf, "mv -f %s %s",board, fpath );
	    */
	    f_mv(board,fpath);
            break;
        }
        if( mode != ADDGROUP )
            sprintf( genbuf, "%-38.38s %s ", title, currentuser->userid );
        else
        {
            /*Add by SmallPig*/
            if( HAS_PERM(currentuser, PERM_SYSOP ||HAS_PERM(currentuser, PERM_ANNOUNCE )) ){
                move(1,0) ;
                clrtoeol() ;
                /*$$$$$$$$ Multi-BM Input, Modified By Excellent $$$$$$$*/
                getdata(1,0,"°åÖ÷: ",uident,STRLEN-1,DOECHO,NULL,YEA) ;
                if(uident[0]!='\0')
                    sprintf( genbuf, "%-38.38s(BM: %s)", title,uident);
                else
                    sprintf( genbuf, "%-38.38s", title);
            }
            else
                sprintf( genbuf, "%-38.38s", title);
        }
        a_additem( pm, genbuf, fname  ,NULL,0);
        a_savenames( pm );
        if( mode == ADDGROUP )
        {
            sprintf( fpath2, "%s/%s/.Names", pm->path,fname );
            if( (pn = fopen( fpath2, "w" )) != NULL )
            {
                fprintf( pn, "#\n" );
                fprintf( pn, "# Title=%s\n", genbuf );
                fprintf( pn, "#\n" );
                fclose(pn);
            }
        }
    }
}

void
a_moveitem( pm )  /*¸Ä±ä ITEM ´ÎĞò*/
MENU    *pm;
{
    ITEM        *tmp;
    char        newnum[ STRLEN ];
    int         num, n, temp;

    sprintf( genbuf, "ÇëÊäÈëµÚ %d ÏîµÄĞÂ´ÎĞò: ", pm->now+1 );
    temp = pm->now+1;
    a_prompt( -2, genbuf, newnum );
    num = (newnum[0] == '$') ? 9999 : atoi( newnum ) - 1;
    if( num >= pm->num )  num = pm->num-1;
    else if( num < 0 )  return;
    tmp = pm->item[ pm->now ];
    if( num > pm->now ) {
        for( n = pm->now; n < num; n++ )
            pm->item[ n ] = pm->item[ n+1 ];
    } else {
        for( n = pm->now; n > num; n-- )
            pm->item[ n ] = pm->item[ n-1 ];
    }
    pm->item[ num ] = tmp;
    pm->now = num;
    a_savenames( pm );
    sprintf(r_genbuf,"¸Ä±ä %s ÏÂµÚ %d ÏîµÄ´ÎĞòµ½µÚ %d Ïî",pm->path+17,temp ,pm->now+1);
    a_report(r_genbuf);
}

void
a_copypaste( pm, paste )
MENU    *pm;
int     paste;
{
	/* KCN 2002.03.22,ÏÂÃæ±äÁ¿µÄstaticÓ¦¸Ã¿ÉÒÔÈ¥µô*/
    char title[ STRLEN ], filename[ STRLEN ], fpath[ PATHLEN ];
    ITEM        *item;
    char        ans[ STRLEN ], newpath[ PATHLEN ];
    FILE        *fn; /* Leeward 98.02.19 */

    move( t_lines-1, 0 );
    if( !paste ) {
        item = pm->item[ pm->now ];
        strncpy( title, item->title, STRLEN );
        strncpy( filename, item->fname, STRLEN );
        sprintf( genbuf, "%s/%s", pm->path, filename );
        strncpy( fpath, genbuf, PATHLEN );
        prints( "¿½±´±êÊ¶Íê³É¡£×¢Òâ£ºÕ³ÌùÎÄÕÂáá²ÅÄÜÓÃ d ÃüÁî½«ÎÄÕÂÉ¾³ı! -- Çë°´ÈÎÒâ¼ü¼ÌĞø << " );
        egetch();

        /* Leeward: 98.02.19: ¶Ô°æÖ÷µÄ¶à¸ö´°¿ÚÍ¬²½ C/P ²Ù×÷ */
        sprintf(genbuf, "home/%c/%s/.CP", toupper(currentuser->userid[0]), currentuser->userid);
        fn = fopen(genbuf, "wt");
        if (fn)
        {
            fputs(title, fn);
            fputs("\n", fn);
            fputs(filename, fn);
            fputs("\n", fn);
            fputs(fpath, fn);
            fputs("\n", fn);
            fclose(fn);
        }
        else
        {
            prints( "File open ERROR -- please report Leeward " );
            egetch();
        }
    } else {
        /* Leeward: 98.02.19: ¶Ô°æÖ÷µÄ¶à¸ö´°¿ÚÍ¬²½ C/P ²Ù×÷ */
        sprintf(genbuf, "home/%c/%s/.CP", toupper(currentuser->userid[0]), currentuser->userid);
        fn = fopen(genbuf, "rt");
        if (fn)
        {
            fgets(title, STRLEN, fn);
            if ('\n' == title[strlen(title) - 1])
                title[strlen(title) - 1] = 0;
            fgets(filename, STRLEN, fn);
            if ('\n' == filename[strlen(filename) - 1])
                filename[strlen(filename) - 1] = 0;
            fgets(fpath, /*STRLEN*/ PATHLEN, fn); /* Leeward 98.04.15 */
            if ('\n' == fpath[strlen(fpath) - 1])
                fpath[strlen(fpath) - 1] = 0;
            fclose(fn);
        }

        sprintf( newpath, "%s/%s", pm->path, filename );
        if( *title == '\0' || *filename == '\0' ) {
            prints( "ÇëÏÈÊ¹ÓÃ copy ÃüÁîÔÙÊ¹ÓÃ paste ÃüÁî. " );
            egetch();
        } else if( dashf( newpath ) || dashd( newpath ) ) {
            prints( "%s %s ÒÑ¾­´æÔÚ. ", (dashd(newpath) ? "Ä¿Â¼" : "ÎÄ¼ş"), filename );
            egetch();
        } else if( strstr( newpath, fpath ) != NULL ) {
            prints( "ÎŞ·¨½«Ò»¸öÄ¿Â¼°á½ø×Ô¼ºµÄ×ÓÄ¿Â¼ÖĞ, »áÔì³ÉËÀÑ­»·. " );
            egetch();
        } else {
            /* modified by cityhunter to simplify annouce c/p */
            sprintf( genbuf, "ÄúÈ·¶¨ÒªÕ³Ìù%s %s Âğ? (C/L/N)CÎª¸´ÖÆ·½Ê½ LÎªÁ´½Ó·½Ê½ [N]: ", (dashd(fpath) ? "Ä¿Â¼" : "ÎÄ¼ş"), filename);
            a_prompt( -2, genbuf, ans );
            if( ans[0] == 'C' || ans[0] == 'c' ) {
                if (dashd(fpath))
                { /* ÊÇÄ¿Â¼ */
                    sprintf( genbuf, "/bin/cp -rp %s %s", fpath, newpath );
                    system( genbuf );
                }
                else
                { /* ÊÇÎÄ¼ş 
                    sprintf( genbuf, "/bin/cp -p %s %s", fpath, newpath );*/
		    f_cp(fpath,newpath,0);
                }
                a_additem( pm, title, filename  ,NULL,0);
                a_savenames( pm );
                sprintf(r_genbuf,"¸´ÖÆ¾«»ªÇøÎÄ¼ş»òÄ¿Â¼: %s",genbuf);
                a_report(r_genbuf);
            }
            else if( ans[0] == 'L' || ans[0] == 'l' ) {
                if (dashd(fpath))
                { /* ÊÇÄ¿Â¼ */
                    sprintf( genbuf, "/bin/cp -rp %s %s", fpath, newpath );
                    system( genbuf );
                }
                else
                { /* ÊÇÎÄ¼ş 
                    sprintf( genbuf, "/bin/ln %s %s", fpath, newpath );*/
		    f_ln(fpath,newpath);
                }
                a_additem( pm, title, filename  ,NULL,0);
                a_savenames( pm );
                sprintf(r_genbuf,"¸´ÖÆ¾«»ªÇøÎÄ¼ş»òÄ¿Â¼: %s",genbuf);
                a_report(r_genbuf);
            }
        }
        /*            sprintf( genbuf, "ÄúÈ·¶¨ÒªÕ³Ìù%s %s Âğ? (Y/N) [N]: ", (dashd(fpath) ? "Ä¿Â¼" : "ÎÄ¼ş"), filename);
                    a_prompt( -2, genbuf, ans );
                    if( ans[0] == 'Y' || ans[0] == 'y' ) {
                        if (dashd(fpath))
                        { 
                            sprintf( genbuf, "/bin/cp -rp %s %s", fpath, newpath );
                        }
                        else
                        { 
                          sprintf( genbuf, "Ê¹ÓÃÁ´½Ó·½Ê½(L)»¹ÊÇ¸´ÖÆ·½Ê½(C)£¿Ç°ÕßÄÜ´ó´ó½ÚÊ¡´ÅÅÌ¿Õ¼ä (L/C) [L]: ", filename ); 
                          a_prompt( -2, genbuf, ans );
                          if( ans[0] == 'C' || ans[0] == 'c' ) 
                            sprintf( genbuf, "/bin/cp -p %s %s", fpath, newpath );
                          else
                            sprintf( genbuf, "/bin/ln %s %s", fpath, newpath );
                        }
                        system( genbuf );
                        a_additem( pm, title, filename  ,NULL,0);
                        a_savenames( pm );
        		sprintf(r_genbuf,"¸´ÖÆ¾«»ªÇøÎÄ¼ş»òÄ¿Â¼: %s",genbuf);
         	        a_report(r_genbuf);
                    }
                }
        */
    }
    pm->page = 9999;
}

void
a_delete( pm )
MENU    *pm;
{
    ITEM        *item;
    char        fpath[ PATHLEN ];
    char        ans[ STRLEN ];
    int         n;

    item = pm->item[ pm->now ];
    move( t_lines-2, 0 );
    prints( "%5d  %-50s\n", pm->now+1, item->title );
    sprintf( fpath, "%s/%s", pm->path, item->fname );
    if( dashf( fpath ) ) {
        a_prompt( -1, "É¾³ı´ËÎÄ¼ş, È·¶¨Âğ?(Y/N) [N]£º", ans );
        if( ans[0] != 'Y' && ans[0] != 'y' )
            return;
        unlink( fpath );
    } else if( dashd( fpath ) ) {
        a_prompt( -1, "É¾³ıÕû¸ö×ÓÄ¿Â¼, ±ğ¿ªÍæĞ¦Å¶, È·¶¨Âğ?(Y/N) [N]: ", ans );
        if( ans[0] != 'Y' && ans[0] != 'y' )
            return;
	/*
        sprintf( genbuf, "/bin/rm -rf %s", fpath );*/
	f_rm(fpath);
    }
    free( item );
    (pm->num)--;
    for( n = pm->now; n < pm->num; n++ )
        pm->item[n] = pm->item[n+1];
    a_savenames( pm );
    sprintf(r_genbuf,"É¾³ıÎÄ¼ş»òÄ¿Â¼: %s",fpath+17);
    a_report(r_genbuf);
}

void
a_newname( pm )
MENU    *pm;
{
    ITEM        *item;
    char        fname[ STRLEN ];
    char        fpath[ PATHLEN ];
    char        *mesg;

    item = pm->item[ pm->now ];
    a_prompt( -2, "ĞÂÎÄ¼şÃû: ", fname );
    if( *fname == '\0' )  return;
    sprintf( fpath, "%s/%s", pm->path, fname );
    if( !valid_fname( fname ) ) {
        mesg = "²»ºÏ·¨ÎÄ¼şÃû³Æ.";
    } else if( dashf( fpath ) || dashd( fpath ) ) {
        mesg = "ÏµÍ³ÖĞÒÑÓĞ´ËÎÄ¼ş´æÔÚÁË.";
    } else {
        sprintf( genbuf, "%s/%s", pm->path, item->fname );
        if( Rename( genbuf, fpath ) == 0 ) {
            strcpy( item->fname, fname );
            sprintf(r_genbuf,"¸ü¸ÄÎÄ¼şÃû: %s -> %s",genbuf+17,fpath+17);
            a_report(r_genbuf);
            a_savenames( pm );
            return;
        }
        mesg = "ÎÄ¼şÃû¸ü¸ÄÊ§°Ü !!";
    }
    prints( mesg );
    egetch();
}

void
a_manager( pm, ch )
MENU    *pm;
int     ch;
{
    char uident[STRLEN] ;
    ITEM        *item;
    char        fpath[ PATHLEN ], changed_T[ STRLEN ], ans[ STRLEN ];

    if( pm->num > 0 ) {
        item = pm->item[ pm->now ];
        sprintf( fpath, "%s/%s", pm->path, item->fname );
    }
    switch( ch ) {
    case 'a':  a_newitem( pm, ADDITEM );    break;
    case 'g':  a_newitem( pm, ADDGROUP );   break;
    case 'i':  a_newitem( pm, ADDMAIL );    break;
        /*case 'G':  a_newitem( pm, ADDGOPHER );    break;*/
    case 'p':  a_copypaste( pm, 1 );        break;
    case 'f':  pm->page = 9999;
        sprintf ( genbuf, "Â·¾¶Îª %s, ÒªÉèÎªµ±Ç°Â·¾¶Âğ?(Y/N) [Y]: " , pm->path );
        a_prompt( -1, genbuf, ans );
        /*if( ans[0] == 'Y' || ans[0] == 'y' ) {*/
        if( ans[0] != 'N' && ans[0] != 'n' ) {
            strcpy( netty_path,  pm->path );
            /*sprintf( genbuf, "ÒÑ½«¸ÃÂ·¾¶ÉèÎªµ±Ç°Â·¾¶, Çë°´ÈÎºÎ¼üÒÔ¼ÌĞø <<" );
            a_prompt( -1, genbuf, ans );Leeward 98.04.15 */  
            { /* Leeward: 97.12.17: ¶Ô°æÖ÷µÄ¶à¸ö´°¿ÚÍ¬²½Ë¿Â· */
                FILE *sl;
                char *ptr;

                /* by zixia: ÓÃÏà¶ÔÂ·¾­ sprintf(genbuf, "%s/%s", BBSHOME,netty_path);*/
		sprintf(genbuf, "%s", netty_path);
                ptr = strstr(genbuf, ".faq/");
                if (ptr)
                {
                    if (ptr = strchr(ptr + 5, '/'))
                        strcpy(ptr + 1, ".BMpath");
                    else
                        strcat(genbuf, "/.BMpath");
                    sl = fopen(genbuf, "wt");
                    if (sl)
                    {
                        fputs(netty_path, sl);
                        fclose(sl);
                    }
                }
            } /* End if  # Leeward */
        }
        break;
    }
    if( pm->num > 0 )  switch( ch ) {
        case 's':  if( ++a_fmode >= 3 ) a_fmode = 1;
            pm->page = 9999;     break;
        case 'm':  a_moveitem( pm );
            pm->page = 9999;     break;
        case 'd':  a_delete( pm );
            pm->page = 9999;     break;
    case 'V':  case 'v':
            if( HAS_PERM(currentuser, PERM_SYSOP ) ) {
                if (ch == 'v')
                    sprintf(fpath, "%s/.Names", pm->path);
                else
                    sprintf(fpath, "0Announce/.Search"); /*.Search¿ØÖÆ¸÷°æ¶ÔÓ¦µÄ¾«»ªÇø*/

                if (dashf(fpath)) {
                    modify_user_mode( EDITANN );
                    vedit( fpath, 0);
                    modify_user_mode( CSIE_ANNOUNCE );
                }
                pm->page = 9999;
            }
            break;
        case 't':  a_prompt( -2, "ĞÂ±êÌâ: ", changed_T );
            /* modified by netty to properly handle title change,add bm by SmallPig */
            if( *changed_T )  {
                if( dashf( fpath ) ) {
                    sprintf( genbuf, "%-38.38s %s ", changed_T , currentuser->userid );
                    strcpy( item->title, genbuf );
                    sprintf(r_genbuf,"¸Ä±äÎÄ¼ş %s µÄ±êÌâ",fpath+17);
                    a_report(r_genbuf);
                } else if( dashd( fpath ) ) {
                    if( HAS_PERM(currentuser, PERM_SYSOP ||HAS_PERM(currentuser, PERM_ANNOUNCE )) ){
                        move(1,0) ;
                        clrtoeol() ;
                        /*usercomplete("°åÖ÷: ",uident) ;*/
                        /*$$$$$$$$ Multi-BM Input, Modified By Excellent $$$$$$$*/
                        getdata(1,0,"°åÖ÷: ",uident,STRLEN-1,DOECHO,NULL,YEA) ;
                        if(uident[0]!='\0')
                            sprintf( genbuf, "%-38.38s(BM: %s)", changed_T,uident);
                        else
                            sprintf( genbuf, "%-38.38s", changed_T);
                    }
                    else
                        sprintf( genbuf, "%-38.38s",changed_T );

                    strcpy( item->title, genbuf );
                    sprintf(r_genbuf,"¸Ä±äÄ¿Â¼ %s µÄ±êÌâ",fpath+17);
                    a_report(r_genbuf);
                }
                a_savenames( pm );
            }
            pm->page = 9999;     break;
        case 'e':  if (dashf(fpath)){
                modify_user_mode( EDITANN );
                vedit( fpath, 0 );
                modify_user_mode( CSIE_ANNOUNCE );
                sprintf(r_genbuf,"ĞŞ¸ÄÎÄÕÂ %s µÄÄÚÈİ",pm->path+17);
                a_report(r_genbuf);
            }
            pm->page = 9999;
            break;
        case 'n':  a_newname( pm );
            pm->page = 9999;     break;
        case 'c':  a_copypaste( pm,0);  break;
        case '=':  t_search_down();     break;
        case '+':  t_search_up();       break;
        }
}

void
a_menu( maintitle, path, lastlevel,lastbmonly )
char    *maintitle, *path;
int     lastlevel,lastbmonly;
{
    MENU        me;
    char        fname[ PATHLEN ],tmp[STRLEN];
    int         ch;
    char        *bmstr;
    char        buf[STRLEN];
    char        *ptr1,*ptr2;
    int         port;
    int         bmonly;
    int         number=0;

    modify_user_mode( CSIE_ANNOUNCE );
    me.path = path;
    strcpy( me.mtitle, maintitle );
    me.level = lastlevel;
    bmonly=lastbmonly;
    a_loadnames( &me ); /* Load .Names */

    strcpy(buf,me.mtitle);
    bmstr=strstr(buf,"(BM:");
    if(bmstr!=NULL)
    {
        if(chk_currBM(bmstr+4,currentuser)||HAS_PERM(currentuser,PERM_SYSOP))
            me.level |= PERM_BOARDS;
        else if(bmonly==1&&!(me.level & PERM_BOARDS))
            return;
    }

    if(strstr( me.mtitle, "(BM: BMS)") ||strstr( me.mtitle, "(BM: SECRET)")||
            strstr( me.mtitle, "(BM: SYSOPS)"))
        bmonly=1;

    strcpy(buf,me.mtitle);
    bmstr=strstr(buf,"(BM:");

    me.page = 9999;
    me.now = 0;
    while( 1 ) {
        if( me.now >= me.num && me.num > 0 ) {
            me.now = me.num - 1;
        } else if( me.now < 0 ) {
            me.now = 0;
        }
        if( me.now < me.page || me.now >= me.page + A_PAGESIZE ) {
            me.page = me.now - (me.now % A_PAGESIZE);
            a_showmenu( &me );
        }
        move( 3 + me.now - me.page, 0 );        prints( "->" );
        ch = egetch();
        move( 3 + me.now - me.page, 0 );        prints( "  " );
        if( ch == 'Q' || ch == 'q' || ch == KEY_LEFT || ch == EOF )
            break;
EXPRESS: /* Leeward 98.09.13 */
        switch( ch ) {
        case Ctrl('Z'): r_lastmsg(); /* Leeward 98.07.30 support msgX */
            break;
case KEY_UP: case 'K': case 'k':
            if( --me.now < 0 )  me.now = me.num-1;
            break;
case KEY_DOWN: case 'J': case 'j':
            if( ++me.now >= me.num )  me.now = 0;
            break;
    case KEY_PGUP: case Ctrl( 'B' ):
                        if( me.now >= A_PAGESIZE )      me.now -= A_PAGESIZE;
                else if( me.now > 0 )           me.now = 0;
                else                            me.now = me.num - 1;
            break;
case KEY_PGDN: case Ctrl( 'F' ): case ' ':
            if( me.now < me.num-A_PAGESIZE) me.now += A_PAGESIZE;
            else if( me.now < me.num - 1 )  me.now = me.num - 1;
            else                            me.now = 0;
            break;
        case Ctrl('P'):
                if(!HAS_PERM(currentuser, PERM_POST ))
                        break;
 		if( !me.item[ me.now ] )
 			break;
            sprintf( fname, "%s/%s", path, me.item[ me.now ]->fname );
            if(!dashf(fname))
                break;
    if( me.now < me.num ) {
                char bname[30];
                clear();
                if(get_a_boardname(bname,"ÇëÊäÈëÒª×ªÌùµÄÌÖÂÛÇøÃû³Æ: "))
                {
                    move(1,0);
                    clrtoeol();
                    strcpy(tmp,currboard);
                    strcpy(currboard,bname);
                    if(deny_me(currentuser->userid,currboard))
                    {
                        prints("¶Ô²»Æğ£¬ÄãÔÚ %s °å±»Í£Ö¹·¢±íÎÄÕÂµÄÈ¨Á¦",bname);
                        pressreturn();
                        strcpy(currboard,tmp);
                        me.page = 9999;
                        break;
                    }
                    if (!haspostperm(currentuser,currboard))
                    {
                        move( 1, 0 );
                        prints("ÄúÉĞÎŞÈ¨ÏŞÔÚ %s ·¢±íÎÄÕÂ.\n",currboard);
                        prints("Èç¹ûÄúÉĞÎ´×¢²á£¬ÇëÔÚ¸öÈË¹¤¾ßÏäÄÚÏêÏ¸×¢²áÉí·İ\n");
                        prints("Î´Í¨¹ıÉí·İ×¢²áÈÏÖ¤µÄÓÃ»§£¬Ã»ÓĞ·¢±íÎÄÕÂµÄÈ¨ÏŞ¡£\n");
                        prints("Ğ»Ğ»ºÏ×÷£¡ :-) \n");
                        pressreturn();
                        strcpy(currboard,tmp);
                        me.page = 9999;
                        break;
                    }
                    if (check_readonly(currboard))
                    {
                        strcpy(currboard,tmp);
                        me.page = 9999;
                        break;
                    }
                    strcpy(currboard,tmp);
                    sprintf(tmp,"ÄãÈ·¶¨Òª×ªÌùµ½ %s °åÂğ",bname);
                    if(askyn(tmp,0)==1)
                    {
                        postfile( fname,bname,me.item[ me.now ]->title ,2);
                        move(2,0);
                        sprintf(tmp,"[1mÒÑ¾­°ïÄã×ªÌùÖÁ %s °åÁË[m",bname);
                        prints(tmp);
                        refresh();
                        sleep(1);
                    }
                }
                me.page = 9999;
            }
            show_message(NULL);
            break;
        case 'h':
            show_help("help/announcereadhelp");
            me.page = 9999;
            break;
    case '\n': case '\r':
            if( number > 0 ) {
                me.now = number - 1;
                number=0;
                continue;
            }
    case 'R': case 'r':
        case KEY_RIGHT:
            if( me.now < me.num ) {
                if(me.item[ me.now ]->host!=NULL)
                {
                    /* gopher(me.item[ me.now ]->host,me.item[ me.now ]->fname,
                             me.item[ me.now ]->port,me.item[ me.now ]->title);*/
                    me.page = 9999;
                    break;
                }else
                    sprintf( fname, "%s/%s", path, me.item[ me.now ]->fname );
                if( dashf( fname ) ) {
                    /*ansimore( fname, YEA );*/
                    /* Leeward 98.09.13 ĞÂÌí¹¦ÄÜ¡Ã
                    £¬ÓÃÉÏ£¯ÏÂ¼ıÍ·Ö±½ÓÌø×ªµ½Ç°£¯ºóÒ»Ïî */
                    ansimore( fname, NA );
                    prints("[1m[44m[31m[ÔÄ¶Á¾«»ªÇø×ÊÁÏ]  [33m½áÊø Q,¡û ©¦ ÉÏÒ»Ïî×ÊÁÏ U,¡ü©¦ ÏÂÒ»Ïî×ÊÁÏ <Enter>,<Space>,¡ı [m");
                    switch( ch = egetch() )
                    {
            case KEY_DOWN: case ' ': case '\n':
                        if ( ++ me.now >= me.num )  me.now = 0;
                        ch = KEY_RIGHT; goto EXPRESS;
                    case KEY_UP:
                        if (-- me.now < 0 )  me.now = me.num - 1;
                        ch = KEY_RIGHT; goto EXPRESS;
                case Ctrl('Z'): case 'h':
                        goto EXPRESS;
                    default: break;
                    }
                } else if( dashd( fname ) ) {
                    a_menu( me.item[ me.now ]->title, fname, me.level ,bmonly);
                }
                me.page = 9999;
            }
            break;
        case '/':
            if( a_menusearch( path, NULL, me.level ) )
                me.page = 9999;
            break;
        case 'F':
        case 'U':
            if( me.now < me.num && HAS_PERM(currentuser, PERM_BASIC ) ) {
                a_forward( path, me.item[ me.now ], ch == 'U' );
                me.page = 9999;
            }
            break;
        case 'o': t_friends(); me.page=9999; break;/*Haohmaru 98.09.22*/
        case 'u': clear(); modify_user_mode(QUERY); t_query(); modify_user_mode( CSIE_ANNOUNCE ); me.page=9999; break;/*Haohmaru.99.11.29*/
        case '!': Goodbye(); me.page=9999;break;/*Haohmaru 98.09.24*/
            /*
                        case 'Z':
                            if( me.now < me.num && HAS_PERM(currentuser, PERM_BASIC ) ) {
                                sprintf( fname, "%s/%s", path, me.item[ me.now ]->fname );
                                a_download( fname );
                                me.page = 9999;
                            }
                            break;
            */
        }
        if( ch >= '0' && ch <= '9' ) {
            number = number * 10 + (ch - '0');
            ch = '\0';
        } else {
            number = 0;
        }
        if( me.level & PERM_BOARDS )
            a_manager( &me, ch );
    }
    for( ch = 0; ch < me.num; ch++ )
        free( me.item[ ch ] );
}

int
linkto(path,fname,title)
char *path,*title,*fname;
{
    MENU    pm;
    char    fpath[STRLEN];


    pm.path=path;

    strcpy(pm.mtitle,title);
    a_loadnames(&pm);
    a_additem(&pm,title,fname ,NULL,0);
    a_savenames(&pm);
}

int
add_grp(group,gname,bname,title)  /* ¾«»ªÇø ¼Ó Ä¿Â¼ */
char group[STRLEN],bname[STRLEN],title[STRLEN],gname[STRLEN];
{
    FILE        *fn;
    char        buf[ PATHLEN ], *ptr;
    char        searchname[STRLEN];
    char        gpath[STRLEN*2];
    char        bpath[STRLEN*2];
    char        old_bpath[STRLEN*2];
    int         len, ch;
    MENU        pm;
    char        ans[ STRLEN ];

    sprintf( buf, "0Announce/.Search");
    sprintf(searchname,"%s: groups/%s/%s",bname,group,bname);
    sprintf(gpath,"0Announce/groups/%s",group);
    sprintf(bpath,"%s/%s",gpath,bname);
    if(!dashd("0Announce"))
    {
        mkdir( "0Announce", 0755 );
        chmod( "0Announce", 0755 );
        if( (fn = fopen( "0Announce/.Names", "w" )) == NULL )
            return -1; 
        fprintf( fn, "#\n" );
        fprintf( fn, "# Title=%s ¾«»ªÇø¹«²¼À¸\n", BoardName );
        fprintf( fn, "#\n" );
        fclose(fn);
    }
    if(!seek_in_file(buf,bname))
        addtofile(buf,searchname);
    if(!dashd("0Announce/groups"))
    {
        mkdir( "0Announce/groups", 0755 );
        chmod( "0Announce/groups", 0755 );

        linkto("0Announce","groups","ÌÖÂÛÇø¾«»ª");
    }
    if(!dashd(gpath))
    {
        mkdir( gpath, 0755 );
        chmod( gpath, 0755 );
        linkto("0Announce/groups",group,gname);
    }
    if(!dashd(bpath))
    {
        mkdir( bpath, 0755 );
        chmod( bpath, 0755 );
        linkto(gpath,bname,title);
        sprintf( buf, "%s/.Names", bpath );
        if( (fn = fopen( buf, "w" )) == NULL )
        {
            return -1;
        }
        fprintf( fn, "#\n" );
        fprintf( fn, "# Title=%s\n", title );
        fprintf( fn, "#\n" );
        fclose(fn);
    }
    return 0;
}

int
del_grp(grp,bname,title)
char grp[STRLEN],bname[STRLEN],title[STRLEN];
{
    char        buf[ STRLEN ], *ptr,buf2[STRLEN],buf3[30];
    char        searchname[STRLEN];
    char        gpath[STRLEN*2];
    char        bpath[STRLEN*2];
    char        check[30];
    int         len, i,n;
    MENU        pm;
    char        ans[ STRLEN ];

    strcpy(buf3,grp);
    sprintf( buf, "0Announce/.Search");
    sprintf(gpath,"0Announce/groups/%s",buf3);
    sprintf(bpath,"%s/%s",gpath,bname);
    /*
    sprintf(genbuf,"/bin/rm -fr %s",bpath) ;
    */

    f_rm(bpath);

    pm.path=gpath;
    a_loadnames(&pm);
    for(i=0;i<pm.num;i++)
    {
        strcpy(buf2,pm.item[i]->title);
        strcpy(check,strtok(pm.item[i]->fname,"/~\n\b"));
        if(strstr(buf2,title)&&!strcmp(check,bname))
        {
            free(pm.item[i]);
            (pm.num)--;
            for( n = i; n < pm.num; n++ )
                pm.item[n] = pm.item[n+1];
            a_savenames(&pm);
            break;
        }
    }
    return 0;
}

int
edit_grp(bname,grp,title,newtitle)
char bname[STRLEN],grp[STRLEN],title[STRLEN],newtitle[100];
{
    char        buf[ STRLEN ], *ptr,buf2[STRLEN],buf3[30];
    char        searchname[STRLEN];
    char        gpath[STRLEN*2];
    char        bpath[STRLEN*2];
    int         len, i;
    MENU        pm;
    char        ans[ STRLEN ];

    strcpy(buf3,grp);
    sprintf( buf, "0Announce/.Search");
    sprintf(gpath,"0Announce/groups/%s",buf3);
    sprintf(bpath,"%s/%s",gpath,bname);
    if(!seek_in_file(buf,bname))
        return 0;

    pm.path=gpath;
    a_loadnames(&pm);
    for(i=0;i<pm.num;i++)
    {
        strcpy(buf2,pm.item[i]->title);
        if(strstr(buf2,title)&&strstr(pm.item[i]->fname,bname))
        {
            strcpy(pm.item[i]->title,newtitle);
            break;
        }
    }
    a_savenames(&pm);
    pm.path=bpath;
    a_loadnames(&pm);
    strcpy(pm.mtitle,newtitle);
    a_savenames(&pm);

    return 0;
}

void
Announce()
{
    sprintf( genbuf, "%s ¾«»ªÇø¹«²¼À¸", BoardName );
    a_menu( genbuf, "0Announce", HAS_PERM(currentuser,PERM_ANNOUNCE) ? PERM_BOARDS : 0 ,0);
    clear();
}


