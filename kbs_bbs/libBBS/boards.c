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

#include "bbs.h"

#define BBS_PAGESIZE    (t_lines - 4)

#define BRC_MAXSIZE     32768
#define BRC_MAXNUM      60
#define BRC_STRLEN      15
#define BRC_ITEMSIZE    (BRC_STRLEN + 1 + BRC_MAXNUM * sizeof( int ))
#define UNREAD_TIME     (login_start_time - 30 * 86400)
/* added period 2000-09-11	4 FavBoard */
#define FAVBOARDNUM     20

char    brc_buf[ BRC_MAXSIZE ];
int     brc_size, brc_changed = 0;
char    brc_name[ BRC_STRLEN ];
int     brc_list[ BRC_MAXNUM ], brc_num;
/* added period 2000-09-11	4 FavBoard */
int     favbrd_list[FAVBOARDNUM+1];

void load_favboard(int dohelp);
void save_favboard();
void save_userfile(char * fname, int blknum, char * buf);
int  IsFavBoard(int idx);
int  DelFavBoard(int i);

char    *sysconf_str();
extern time_t   login_start_time;
extern int      numboards;
extern struct shortfile *bcache;

struct newpostdata {
    char        *name, *title, *BM;
    char        flag;
    int         pos, total;
    char        unread, zap;
} *nbrd; /*Ã¿¸ö°æµÄÐÅÏ¢*/

int     *zapbuf;
int     brdnum, yank_flag = 0;
char    *boardprefix;

void
EGroup( cmd )
char *cmd;
{
    char        buf[ STRLEN ];

    sprintf( buf, "EGROUP%c", *cmd );
    boardprefix = sysconf_str( buf );
    choose_board( DEFINE(DEF_NEWPOST)?1:0 );
}

void
Boards()
{
    boardprefix = NULL;
    choose_board( 0 );
}

void
New()
{
    boardprefix = NULL;
    choose_board( 1 );
}
/*---	Added by period	2000-09-11	Favorate Board List	---*
 *---	use yank_flag=2 to reflect status 			---*
 *---	corresponding code added: comm_lists.c			---*
 *---		add entry in array sysconf_cmdlist[]		---*/
void FavBoard()
{
    int ifnew = 1, yanksav;
    /*    if(heavyload()) ifnew = 0; */	/* no heavyload() in FB2.6x */
    yanksav = yank_flag;
    yank_flag = 2;
    boardprefix = NULL;
    if(!*favbrd_list) load_favboard(1);
    choose_board(ifnew);
    yank_flag = yanksav;
}

void load_favboard(int dohelp)
{
    char fname[STRLEN];
    int  fd, size, idx;
    setuserfile(fname, "favboard");
    if( (fd = open( fname, O_RDONLY, 0600 )) != -1 ) {
        size = (FAVBOARDNUM+1) * sizeof( int );
        read( fd, favbrd_list, size );
        close( fd );
    } else if(dohelp) {
        int savmode;
        savmode = uinfo.mode;
        modify_user_mode(CSIE_ANNOUNCE);	/* Ã»ºÏÊÊµÄmode.¾ÍÏÈÓÃ"¼³È¡¾«»ª"°É. */
        show_help("help/favboardhelp");
        modify_user_mode(savmode);
    }
    if(*favbrd_list<= 0) {
        *favbrd_list = 1;       /*  favorate board count    */
        *(favbrd_list+1) = 0;   /*  default sysop board     */
    }
    else {
        int num = *favbrd_list;
        if(*favbrd_list > FAVBOARDNUM)	/*	maybe file corrupted	*/
            *favbrd_list = FAVBOARDNUM;
        idx = 0;
        while(++idx <= *favbrd_list) {
            fd = favbrd_list[idx];
            if(fd >= 0 && fd <= numboards && (
                        bcache[fd].filename[0]
                        && ( (bcache[fd].level & PERM_POSTMASK)
                             || HAS_PERM(bcache[fd].level)
                             || (bcache[fd].level&PERM_NOZAP) )
                    )
              )
                continue;
            DelFavBoard(idx);   /*  error correction    */
        }
        if(num != *favbrd_list) save_favboard();
    }
}

void save_favboard()
{
    save_userfile("favboard", (FAVBOARDNUM+1), (char *)favbrd_list);
}

int IsFavBoard(int idx)
{
    int i;
    for(i=1;i<=*favbrd_list;i++) if(idx == favbrd_list[i]) return i;
    return 0;
}

int DelFavBoard(int i)
{
    int lnum;
    if(i > *favbrd_list) return *favbrd_list;
    lnum = --(*favbrd_list);
    for(;i<=lnum;i++) favbrd_list[i] = favbrd_list[i+1];
    if(!lnum) {
        *favbrd_list = 1;       /*  favorate board count    */
        *(favbrd_list+1) = 0;   /*  default sysop board     */
    }
}
/*---   ---*/
void
load_zapbuf()  /* ×°ÈëzapÐÅÏ¢*/
{
    char        fname[ STRLEN ];
    int         fd, size, n;

    size = MAXBOARD * sizeof( int );
    zapbuf = (int *) malloc( size );
    for( n = 0; n < MAXBOARD; n++ )
        zapbuf[n] = 1;
    setuserfile( fname, ".lastread" ); /*userµÄ.lastread£¬ zapÐÅÏ¢*/
    if( (fd = open( fname, O_RDONLY, 0600 )) != -1 ) {
        size = numboards * sizeof( int );
        read( fd, zapbuf, size );
        close( fd );
    }
}
/*---	Modified for FavBoard functions, by period	2000-09-11 */
void save_userfile(char * fname, int numblk, char * buf)
{
    char        fbuf[ 256 ];
    int         fd, size;

    setuserfile( fbuf, fname );
    if( (fd = open( fbuf, O_WRONLY | O_CREAT, 0600 )) != -1 ) {
        size = numblk * sizeof( int );
        write( fd, buf, size );
        close( fd );
    }
}

void save_zapbuf()
{
    save_userfile(".lastread", numboards, (char *)zapbuf);
}

#if 0
void
save_zapbuf() /*±£´æZapÐÅÏ¢*/
{
    char        fname[ STRLEN ];
    int         fd, size;

    setuserfile( fname, ".lastread" );
    if( (fd = open( fname, O_WRONLY | O_CREAT, 0600 )) != -1 ) {
        size = numboards * sizeof( int );
        write( fd, zapbuf, size );
        close( fd );
    }
}
#endif

int
load_boards()
{
    struct shortfile    *bptr;
    struct newpostdata  *ptr;
    int         n;

    resolve_boards();
    if( zapbuf == NULL ) {
        load_zapbuf();
    }
    brdnum = 0;
    for( n = 0; n < numboards; n++ ) {
        bptr = &bcache[ n ];
#ifndef _DEBUG_
        if(!*bptr->filename) continue;
#endif /*_DEBUG_*/
        if( !(bptr->level & PERM_POSTMASK) && !HAS_PERM(bptr->level) && !(bptr->level&PERM_NOZAP))
        {
            continue;
        }
        if( boardprefix != NULL &&
                strchr( boardprefix, bptr->title[0]) == NULL&&boardprefix[0]!='*')
            continue;
        if(boardprefix != NULL&&boardprefix[0]=='*')
        {
            if(!strstr(bptr->title,"¡ñ")&&!strstr(bptr->title,"¡Ñ")
                    && bptr->title[0]!='*')
                continue;
        }
        if(boardprefix == NULL && bptr->title[0]=='*')
            continue;
        /*---	period	2000-09-11	4 FavBoard	---*/
        /*        if( yank_flag || zapbuf[ n ] != 0||(bptr->level&PERM_NOZAP) ) {*/ /*³õÊ¼»¯°æÐÅÏ¢*/
        if( ( 1 == yank_flag || (!yank_flag && (zapbuf[ n ] != 0||(bptr->level&PERM_NOZAP) )) )
                || (2 == yank_flag && IsFavBoard(n)) ) {
            ptr = &nbrd[ brdnum++ ];
            ptr->name  = bptr->filename;
            ptr->title = bptr->title;
            ptr->BM    = bptr->BM;
            ptr->flag  = bptr->flag|((bptr->level&PERM_NOZAP)?NOZAP_FLAG:0);
            ptr->pos = n;
            ptr->total = -1;
            ptr->zap = (zapbuf[ n ] == 0);
        }
    }
    if(brdnum==0&&!yank_flag&&boardprefix == NULL)
    {
        brdnum=-1;
        yank_flag=1;
        return -1;
    }
    return 0;
}

int
search_board( num )
int     *num;
{
    static  int     i = 0,find = YEA;
    static  char    bname[STRLEN];
    int         n,ch,tmpn=NA;

    if (find == YEA)
    {
        bzero(bname, sizeof(bname));
        find = NA;
        i = 0;
    }
    while (1)
    {
        move(t_lines-1, 0);
        clrtoeol();
        prints("ÇëÊäÈëÒªÕÒÑ°µÄ board Ãû³Æ£º%s",bname);
        ch = egetch();

        if (isprint2(ch))
        {
            bname[i++] = ch;
            for (n = 0; n < brdnum; n++)
            {
                if (!ci_strncmp(nbrd[n].name, bname, i))
                {
                    tmpn=YEA;
                    *num = n;
                    if(!strcmp(nbrd[n].name, bname))
                        return 1/*ÕÒµ½ÀàËÆµÄ°å£¬»­ÃæÖØ»­*/;
                }
            }
            if(tmpn)
                return 1;
            if (find == NA)
            {
                bname[--i] = '\0';
            }
            continue;
        }
        else if (ch == Ctrl('H') || ch == KEY_LEFT || ch == KEY_DEL ||
                 ch == '\177')
        {
            i--;
            if (i < 0)
            {
                find = YEA;
                break;
            }
            else
            {
                bname[i] = '\0';
                continue;
            }
        }
        else if (ch == '\t')
        {
            find = YEA;
            break;
        }
        else if (Ctrl('Z') == ch)
        {
            r_lastmsg(); /* Leeward 98.07.30 support msgX */
            break;
        }
        else if (ch == '\n' || ch == '\r' || ch == KEY_RIGHT)
        {
            find = YEA;
            break;
        }
        bell(1);
    }
    if (find)
    {
        move(t_lines-1, 0);
        clrtoeol();
        return 2/*½áÊøÁË*/;
    }
    return 1;
}

int
check_newpost( ptr )
struct newpostdata *ptr;
{
    struct fileheader fh ;
    struct stat st;
    char        filename[ STRLEN ];
    int         fd, offset, total;

    ptr->total = ptr->unread = 0;
    setbdir( genbuf, ptr->name );
    if( (fd = open( genbuf, O_RDWR )) < 0 ) /* ¶ÁboardÄ¿Â¼Ê±¼äÎÄ¼þ*/
        return 0;
    fstat( fd, &st );
    total = st.st_size / sizeof( fh );
    if( total <= 0 ) {
        close( fd );
        return 0;
    }
    ptr->total = total;
    if( !brc_initial( ptr->name ) ) {/*×°Èë±¾°æbrc list */
        ptr->unread = 1;
    } else {
        offset = (int)((char *)&(fh.filename[0]) - (char *)&(fh));/*fhÎÞÓÃ£¬½ö½öÊÇ»ñÈ¡¸÷ÖÖsizeºÍoffsetÓÃ*/
        lseek( fd, offset + (total-1) * sizeof(fh), SEEK_SET );/*×îºóÒ»¸öÎÄ¼þÃû*/
        if( read( fd, filename, STRLEN ) > 0 && brc_unread( filename ) ) {
            ptr->unread = 1;
        }
    }
    close( fd );
    return 1;
}

int
unread_position( dirfile, ptr )
char    *dirfile;
struct newpostdata *ptr;
{
    struct fileheader fh ;
    char        filename[ STRLEN ];
    int         fd, offset, step, num;

    num = ptr->total + 1;
    if( ptr->unread && (fd = open( dirfile, O_RDWR )) > 0 ) {
        if( !brc_initial( ptr->name ) ) {
            num = 1;
        } else {
            offset = (int)((char *)&(fh.filename[0]) - (char *)&(fh));
            num = ptr->total - 1;
            step = 4;
            while( num > 0 ) {
                lseek( fd, offset + num * sizeof(fh), SEEK_SET );
                if( read( fd, filename, STRLEN ) <= 0 ||
                        !brc_unread( filename ) )  break;
                num -= step;
                if( step < 32 )  step += step / 2;
            }
            if( num < 0 )  num = 0;
            while( num < ptr->total ) {
                lseek( fd, offset + num * sizeof(fh), SEEK_SET );
                if( read( fd, filename, STRLEN ) <= 0 ||
                        brc_unread( filename ) )  break;
                num ++;
            }
        }
        close( fd );
    }
    if( num < 0 )  num = 0;
    return num;
}
int
show_authorBM(ent,fileinfo,direct)
int ent ;
struct fileheader *fileinfo ;
char *direct ;
{
    struct shortfile    *bptr;
    int         tuid=0;
    int         n;

    if(!HAS_PERM( PERM_ACCOUNTS )||!strcmp(fileinfo->owner,"Anonymous")||!strcmp(fileinfo->owner,"deliver"))
        return DONOTHING;
    else
    {
        if( !( tuid=getuser(fileinfo->owner) ) ) {
            clrtobot();
            prints("²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ\n") ;
            pressanykey() ;
            move(2,0) ;
            clrtobot() ;
            return FULLUPDATE ;
        }

        move( 3, 0 );
        if( !(lookupuser.userlevel & PERM_BOARDS)){
            clrtobot();
            prints("ÓÃ»§%s²»ÊÇ°æÖ÷!\n",lookupuser.userid);
            pressanykey() ;
            move(2,0) ;
            clrtobot() ;
            return FULLUPDATE ;
        }
        clrtobot();
        prints("ÓÃ»§%sÎªÒÔÏÂ°æµÄ°æÖ÷\n\n",lookupuser.userid);

        prints("©³©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©×©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©·\n");
        prints("©§            °æÓ¢ÎÄÃû            ©§            °æÖÐÎÄÃû            ©§\n");

        for( n = 0; n < numboards; n++ ) {
            bptr = &bcache[ n ];
            if( chk_BM_instr(bptr->BM,lookupuser.userid) == YEA){
                prints("©Ç©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©ï©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©Ï\n");
                prints("©§%-32s©§%-32s©§\n",bptr->filename,bptr->title+12);
            }
        }
        prints("©»©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©ß©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¿\n");
        pressanykey() ;
        move(2,0) ;
        clrtobot() ;
        return FULLUPDATE;
    }
}

/* inserted by cityhunter to query BM */
int
query_bm( )
{
    struct shortfile    *bptr;
    int         n;
    char        tmpBM[BM_LEN-1];
    char        uident[STRLEN];
    int         tuid=0;

    modify_user_mode(QUERY);
    move(2,0);
    clrtobot();
    prints("<ÊäÈëÊ¹ÓÃÕß´úºÅ, °´¿Õ°×¼ü¿ÉÁÐ³ö·ûºÏ×Ö´®>\n");
    move(1,0);
    clrtoeol();
    prints("²éÑ¯Ë­: ");
    usercomplete(NULL,uident);
    if(uident[0] == '\0') {
        clear() ;
        return FULLUPDATE ;
    }
    if(!(tuid = getuser(uident))) {
        move(2,0) ;
        clrtoeol();
        prints("[1m²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ[m\n") ;
        pressanykey() ;
        move(2,0) ;
        clrtoeol() ;
        return FULLUPDATE ;
    }

    move( 3, 0 );
    if( !(lookupuser.userlevel & PERM_BOARDS))
    {
        prints("ÓÃ»§%s²»ÊÇ°æÖ÷!\n",lookupuser.userid);
        pressanykey() ;
        move(2,0) ;
        clrtoeol() ;
        return FULLUPDATE ;
    }
    prints("ÓÃ»§%sÎªÒÔÏÂ°æµÄ°æÖ÷\n\n",lookupuser.userid);

    prints("©³©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©×©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©·\n");
    prints("©§            °æÓ¢ÎÄÃû            ©§            °æÖÐÎÄÃû            ©§\n");

    for( n = 0; n < numboards; n++ ) {
        bptr = &bcache[ n ];
        if( chk_BM_instr(bptr->BM,lookupuser.userid) == YEA)
        {
            prints("©Ç©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©ï©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©Ï\n");
            prints("©§%-32s©§%-32s©§\n",bptr->filename,bptr->title+12);
        }
    }
    prints("©»©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©ß©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¿\n");
    pressanykey() ;
    move(2,0) ;
    clrtoeol() ;
    return FULLUPDATE;
}
/* end of insertion */

void
show_brdlist( page, clsflag, newflag )  /* show board list */
int     page, clsflag, newflag;
{
    struct newpostdata  *ptr;
    int         n;
    char        tmpBM[BM_LEN-1];
    char        buf[STRLEN]; /* Leeward 98.03.28 */

    if( clsflag )
    {
        clear();
        docmdtitle( "[ÌÖÂÛÇøÁÐ±í]", "  [mÖ÷Ñ¡µ¥[¡û,e] ÔÄ¶Á[¡ú,r] Ñ¡Ôñ[¡ü,¡ý] ÁÐ³ö[y] ÅÅÐò[s] ËÑÑ°[/] ÇÐ»»[c] ÇóÖú[h]\n" );
        prints( "[44m[37m %s ÌÖÂÛÇøÃû³Æ       V  Àà±ð ×ªÐÅ  %-24s °å  Ö÷   %s   [m\n",
                newflag ? "È«²¿ Î´¶Á" : "±àºÅ  ", "ÖÐ  ÎÄ  Ðð  Êö" ,newflag ? "":"   ");
    }

    move( 3, 0 );
    for( n = page; n < page + BBS_PAGESIZE; n++ ) {
        if( n >= brdnum ) {
            prints( "\n" );
            continue;
        }
        ptr = &nbrd[n];
        if( !newflag )
            prints( " %4d %c", n+1, ptr->zap&&!(ptr->flag&NOZAP_FLAG) ? '-' : ' ' );/*zap±êÖ¾*/
        else if( ptr->zap&&!(ptr->flag&NOZAP_FLAG) ) {
            /*ptr->total = ptr->unread = 0;
            prints( "    -    -" );*/
            /* Leeward: 97.12.15: extended display */
            check_newpost( ptr );
            prints( " %4d%s%s ", ptr->total, ptr->total > 9999 ? " " : "  ", ptr->unread ? "¡ô" : "¡ó" );/*ÊÇ·ñÎ´¶Á*/
        } else {
            if( ptr->total == -1 ) {
                refresh();
                check_newpost( ptr );
            }
            prints( " %4d%s%s ", ptr->total, ptr->total > 9999 ? " " : "  ", ptr->unread ? "¡ô" : "¡ó" );/*ÊÇ·ñÎ´¶Á*/
        }
        strcpy(tmpBM,ptr->BM);

        /* Leeward 98.03.28 Displaying whether a board is READONLY or not */
        if (YEA == check_readonly(ptr->name))
            sprintf(buf, "¡ôÖ»¶Á¡ô%s", ptr->title + 8);
        else
            sprintf(buf, " %s", ptr->title + 1);
        prints("%c%-16s %s%-36s %-12s\n", ((newflag && ptr->zap && !(ptr->flag&NOZAP_FLAG)) ? '*' : ' '), ptr->name,(ptr->flag&VOTE_FLAG) ? "[31mV[m" : " ", buf, ptr->BM[0] <= ' ' ? "³ÏÕ÷°åÖ÷ÖÐ" : strtok(tmpBM," ")); /*µÚÒ»¸ö°æÖ÷*/
    }
    refresh();
}

int
cmpboard( brd, tmp ) /*ÅÅÐòÓÃ*/
struct newpostdata      *brd, *tmp;
{
    register int        type = 0;

    if( !(currentuser.flags[0] & BRDSORT_FLAG) )
    {
        type = brd->title[0] - tmp->title[0];
        if(type==0)
            type=ci_strncmp( brd->title+1, tmp->title+1,6);

    }
    if( type == 0 )
        type = ci_strcmp( brd->name, tmp->name );
    return type;
}

int
choose_board( newflag ) /* Ñ¡Ôñ °æ£¬ readnew»òreadboard */
int     newflag;
{
    static int  num;
    struct newpostdata newpost_buffer[ MAXBOARD ];
    struct newpostdata *ptr;
    int         page, ch, tmp, number,tmpnum;
    int         loop_mode=0;

    if( !strcmp( currentuser.userid, "guest" ) )
        yank_flag = 1;
    nbrd = newpost_buffer;
    modify_user_mode( newflag ? READNEW : READBRD );
    brdnum = number = 0;
    /* show_brdlist( 0, 1, newflag );*/ /*board listÏÔÊ¾ µÄ 2´ÎÏÔÊ¾ÎÊÌâ½â¾ö! 96.9.5 alex*/
    while( 1 ) {
        if( brdnum <= 0 ) { /*³õÊ¼»¯*/
            if(load_boards()==-1)
                continue;
            qsort( nbrd, brdnum, sizeof( nbrd[0] ), cmpboard );
            page = -1;
            if( brdnum <= 0 )  break;
        }
        if( num < 0 )  num = 0;
        if( num >= brdnum )  num = brdnum - 1;
        if( page < 0 ) {
            if( newflag ) {/* Èç¹ûÊÇreadnewµÄ»°£¬ÔòÌøµ½ÏÂÒ»¸öÎ´¶Á°æ*/
                tmp = num;
                while( num < brdnum ) {
                    ptr = &nbrd[ num ];
                    if( ptr->total == -1 ) check_newpost( ptr );
                    if( ptr->unread )  break;
                    num++;
                }
                if( num >= brdnum )  num = tmp;
            }
            page = (num / BBS_PAGESIZE) * BBS_PAGESIZE; /*page¼ÆËã*/
            show_brdlist( page, 1, newflag );
            update_endline();
        }
        if( num < page || num >= page + BBS_PAGESIZE ) {
            page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
            show_brdlist( page, 0, newflag );
            update_endline();
        }
        move( 3+num-page,0 ); prints( ">", number ); /*ÏÔÊ¾µ±Ç°board±êÖ¾*/
        if(loop_mode==0)
        {
            ch = egetch();
        }
        move( 3+num-page,0 ); prints( " " );
        if( ch == 'q' || ch == 'e' || ch == KEY_LEFT || ch == EOF )
            break;
        switch( ch ) {
        case Ctrl('Z'): r_lastmsg(); /* Leeward 98.07.30 support msgX */
            break;
        case 'R':  /* Leeward 98.04.24 */
            {
                char fname[STRLEN], restore[256];

                if(!strcmp(currentuser.userid,"guest")) /* guest ²»±Ø */
                    break;

                saveline(t_lines-2, 0, NULL);
                move(t_lines-2, 0);
                clrtoeol();
                getdata(t_lines-2, 0,"[1m[5m[31mÁ¢¼´¶ÏÏß[m¡Ã[1m[33mÒÔ±ã»Ö¸´ÉÏ´ÎÕý³£Àë¿ª±¾Õ¾Ê±µÄÎ´¶Á±ê¼Ç (Y/N)£¿ [N][m: ", restore,4,DOECHO,NULL,YEA);
                if ('y' == restore[0] || 'Y' == restore[0])
                {
                    setuserfile(fname, ".boardrc" );
                    sprintf(restore, "/bin/cp %s.bak %s", fname, fname);
                    system(restore);

                    move(t_lines-2, 0);
                    clrtoeol();
                    prints("[1m[33mÒÑ»Ö¸´ÉÏ´ÎÕý³£Àë¿ª±¾Õ¾Ê±µÄÎ´¶Á±ê¼Ç[m");
                    move(t_lines-1, 0);
                    clrtoeol();
                    getdata(t_lines-1, 0,"[1m[32mÇë°´ Enter ¶ÏÏß£¬È»ºóÖØÐÂµÇÂ¼ 8-) [m", restore,1,DOECHO,NULL,YEA);
                    abort_bbs();
                }
                saveline(t_lines-2, 1, NULL);
                break;
            }
        case 'X': /* Leeward 98.03.28 Set a board READONLY */
            {
                char buf[STRLEN];

                if (!HAS_PERM(PERM_SYSOP) && !HAS_PERM(PERM_OBOARDS)) break;
                if (!strcmp(nbrd[num].name, "syssecurity")
                        ||!strcmp(nbrd[num].name, "Filter")
                        ||!strcmp(nbrd[num].name, "junk")
                        ||!strcmp(nbrd[num].name, "deleted"))
                    break; /* Leeward 98.04.01 */

                sprintf(buf, "chmod 555 boards/%s", nbrd[num].name);
                system(buf);

                /* Bigman 2000.12.11:ÏµÍ³¼ÇÂ¼ */
                sprintf(genbuf,"Ö»¶ÁÌÖÂÛÇø %s ",nbrd[num].name);
                securityreport(genbuf);
                sprintf(genbuf, " readonly board %s",nbrd[num].name);
                report(genbuf);

                show_brdlist(page, 0, newflag);
                break;
            }
        case 'Y': /* Leeward 98.03.28 Set a board READABLE */
            {
                char buf[STRLEN];

                if (!HAS_PERM(PERM_SYSOP) && !HAS_PERM(PERM_OBOARDS)) break;

                sprintf(buf, "chmod 755 boards/%s", nbrd[num].name);
                system(buf);

                /* Bigman 2000.12.11:ÏµÍ³¼ÇÂ¼ */
                sprintf(genbuf,"½â¿ªÖ»¶ÁÌÖÂÛÇø %s ",nbrd[num].name);
                securityreport(genbuf);
                sprintf(genbuf, " readable board %s",nbrd[num].name);
                report(genbuf);

                show_brdlist(page, 0, newflag);
                break;
            }
case 'L':case 'l':  /* Luzi 1997.10.31 */
            if(uinfo.mode!=LOOKMSGS)
            {
                show_allmsgs();
                page = -1;
                break;
            }
            else
                return DONOTHING;
        case 'H':          /* Luzi 1997.10.31 */
            r_lastmsg();
            break;
case 'W':case 'w':       /* Luzi 1997.10.31 */
            if (!HAS_PERM(PERM_PAGE)) break;
            s_msg();
            page=-1;
            break;
        case 'u':		/*Haohmaru.99.11.29*/
            {	int oldmode = uinfo.mode;
                clear();
                modify_user_mode(QUERY);
                t_query();
                page=-1;
                modify_user_mode(oldmode);
                break;
            }
        case '!':
            Goodbye();
            page=-1;
            break;
case 'O':case 'o':       /* Luzi 1997.10.31 */
            { /* Leeward 98.10.26 fix a bug by saving old mode */
                int  savemode = uinfo.mode;
                if (!HAS_PERM(PERM_BASIC)) break;
                t_friends();
                page=-1;
                modify_user_mode(savemode);
                /* return FULLUPDATE;*/
                break;
            }
case 'P': case 'b': case Ctrl('B'): case KEY_PGUP:
            if( num == 0 )  num = brdnum - 1;
            else  num -= BBS_PAGESIZE;
            break;
case 'C':case 'c': /*ÔÄ¶ÁÄ£Ê½*/
            if(newflag==1)
                newflag=0;
            else
                newflag=1;
            show_brdlist( page, 1, newflag );
            break;
case 'N': case ' ': case Ctrl('F'): case KEY_PGDN:
            if( num == brdnum - 1 )  num = 0;
            else  num += BBS_PAGESIZE;
            break;
case 'p': case 'k': case KEY_UP:
            if( num-- <= 0 )  num = brdnum - 1;
            break;
case 'n': case 'j': case KEY_DOWN:
            if( ++num >= brdnum )  num = 0;
            break;
        case '$':
            num = brdnum - 1;       break;
        case 'h':
            show_help("help/boardreadhelp");
            page = -1;
            break;
        case '/': /*ËÑË÷board */
            move( 3+num-page,0 ); prints( ">", number );
            tmpnum=num;
            tmp = search_board( &num );
            move( 3+tmpnum-page,0 ); prints( " ", number );
            if(tmp==1)
                loop_mode=1;
            else
            {
                loop_mode=0;
                update_endline();
            }
            break;
        case 's':   /* sort/unsort -mfchen */
            currentuser.flags[0] ^= BRDSORT_FLAG; /*ÅÅÐò·½Ê½*/
            qsort( nbrd, brdnum, sizeof( nbrd[0] ), cmpboard );/*ÅÅÐò*/
            page = 999;
            break;
            /*---	added period 2000-09-11	4 FavBoard	---*/
        case 'a':
            if(2 == yank_flag) {
                char bname[STRLEN];
                int i = 0;
                if(*favbrd_list >= FAVBOARDNUM) {
                    move(2, 0);
                    clrtoeol();
                    prints("¸öÈËÈÈÃÅ°æÊýÒÑ¾­´ïÉÏÏÞ(%d)£¡", FAVBOARDNUM);
                    pressreturn();
                    show_brdlist( page, 1, newflag );  /*  refresh screen */
                    break;
                }
                move(0,0) ;
                clrtoeol();
                prints("ÊäÈëÌÖÂÛÇøÓ¢ÎÄÃû (´óÐ¡Ð´½Ô¿É£¬°´¿Õ°×¼ü×Ô¶¯ËÑÑ°): ") ;
                clrtoeol() ;

                make_blist() ;
                namecomplete((char *)NULL,bname) ;
                CreateNameList() ;             /*  free list memory. */
                if(*bname) i = getbnum(bname);
                if( i > 0 && !IsFavBoard(i-1) ) {
                    int llen;
                    llen = ++(*favbrd_list);
                    favbrd_list[llen] = i-1;
                    save_favboard();
                    brdnum = -1;    /*  force refresh board list */
                } else {
                    move(2,0);
                    prints("²»ÕýÈ·µÄÌÖÂÛÇø.\n");
                    pressreturn();
                    show_brdlist( page, 1, newflag );  /*  refresh screen */
                }
            }
            break;
        case 'd':
            if(2 == yank_flag) {
                DelFavBoard( IsFavBoard(nbrd[num].pos) );
                save_favboard();
                brdnum = -1;    /*  force refresh board list. */
            }
            break;
            /*---	End of Addition	---*/
        case 'y':
            if(yank_flag < 2) { /*--- Modified 4 FavBoard 2000-09-11	---*/
                yank_flag = !yank_flag;
                brdnum = -1;
            }
            break;
        case 'z': /* Zap*/
            if(yank_flag < 2) { /*--- Modified 4 FavBoard 2000-09-11	---*/
                if( HAS_PERM( PERM_BASIC )&&!(nbrd[num].flag&NOZAP_FLAG)) {
                    ptr = &nbrd[num];
                    ptr->zap = !ptr->zap;
                    ptr->total = -1;
                    zapbuf[ ptr->pos ] = (ptr->zap ? 0 : login_start_time);
                    page = 999;
                }
            }
            break;
        case KEY_HOME:
            num=0;
            break;
        case KEY_END:
            num = brdnum - 1;
            break;
    case '\n': case '\r': /*Ö±½ÓÊäÈëÊý×Ö Ìø×ª*/
            if( number > 0 ) {
                num = number - 1;
                break;
            }
            /* fall through */
    case 'r': case KEY_RIGHT: /*½øÈë board*/
            {
                char    buf[ STRLEN ];

                ptr = &nbrd[num];
                brc_initial( ptr->name );
                memcpy( currBM, ptr->BM, BM_LEN -1);
                if( DEFINE(DEF_FIRSTNEW) ) {
                    setbdir( buf, currboard );
                    tmp = unread_position( buf, ptr );
                    page = tmp - t_lines / 2;
                    getkeep( buf, page > 1 ? page : 1, tmp + 1 );
                }
                Read();

                /* Leeward 98.05.17: updating unread flag on exiting Read() */
                /* if (-1 != load_boards())
                   qsort( nbrd, brdnum, sizeof( nbrd[0] ), cmpboard ); */

                if( zapbuf[ ptr->pos ] > 0 && brc_num > 0 ) {
                    zapbuf[ ptr->pos ] = brc_list[0];
                }
                ptr->total = page = -1;
                modify_user_mode( newflag ? READNEW : READBRD );
                break;
            }
        case 'v':		/*Haohmaru.2000.4.26*/
            clear();
            m_read();
            show_brdlist( page, 1, newflag );
            break;
        default:
            ;
        }
        if( ch >= '0' && ch <= '9' ) {
            number = number * 10 + (ch - '0');
            ch = '\0';
        } else {
            number = 0;
        }
    }
    clear();
    save_zapbuf();
    return -1 ;
}

char *
brc_getrecord( ptr, name, pnum, list ) /*È¡³öÒ»¸ö°æµÄbrclist*/
char    *ptr, *name;
int     *pnum, *list;
{
    int         num;
    char        *tmp;

    strncpy( name, ptr, BRC_STRLEN );
    ptr += BRC_STRLEN;
    num = (*ptr++) & 0xff;
    tmp = ptr + num * sizeof( int );
    if( num > BRC_MAXNUM ) {
        num = BRC_MAXNUM;
    }
    *pnum = num;
    memcpy( list, ptr, num * sizeof( int ) );
    return tmp;
}

char *
brc_putrecord( ptr, name, num, list ) /* ±£´æÒ»¸ö°æµÄbrclist*/
char    *ptr, *name;
int     num, *list;
{
    if( num > 0 /*&& list[0] > UNREAD_TIME */) {
        if( num > BRC_MAXNUM ) {
            num = BRC_MAXNUM;
        }
        /*        while( num > 1 && list[num-1] < UNREAD_TIME ) {
                    num--;
                }*/
        strncpy( ptr, name, BRC_STRLEN );
        ptr += BRC_STRLEN;
        *ptr++ = num;
        memcpy( ptr, list, num * sizeof( int ) );
        ptr += num * sizeof( int );
    }
    return ptr;
}

void
brc_update() /* ±£´æµ±Ç°µÄbrclistµ½ÓÃ»§µÄ.boardrc*/
{
    char        dirfile[ STRLEN ], *ptr;
    char        tmp_buf[ BRC_MAXSIZE - BRC_ITEMSIZE ], *tmp;
    char        tmp_name[ BRC_STRLEN ];
    int         tmp_list[ BRC_MAXNUM ], tmp_num;
    int         fd, tmp_size;

    if( brc_changed == 0 ) {
        return;
    }
    ptr = brc_buf;
    if( brc_num > 0 ) {
        ptr = brc_putrecord( ptr, brc_name, brc_num, brc_list );
    }
    if( 1 ) {
        setuserfile( dirfile, ".boardrc" );
        if( (fd = open( dirfile, O_RDONLY )) != -1 ) {
            tmp_size = read( fd, tmp_buf, sizeof( tmp_buf ) );
            close( fd );
        } else {
            tmp_size = 0;
        }
    }
    tmp = tmp_buf;
    while( tmp < &tmp_buf[ tmp_size ] && (*tmp >= ' ' && *tmp <= 'z') ) {
        tmp = brc_getrecord( tmp, tmp_name, &tmp_num, tmp_list );
        if( strncmp( tmp_name, currboard, BRC_STRLEN ) != 0 ) {
            /*---	correct pointer over-boundary problem	period	2000-10-09	---*/
            /*---	problem of using 'f' in unreaded board cause bbs process crash	---*/
            if(ptr + BRC_STRLEN+1 + tmp_num * sizeof(int) < brc_buf + BRC_MAXSIZE)
                /*---		---*/
                ptr = brc_putrecord( ptr, tmp_name, tmp_num, tmp_list );
        }
    }
    brc_size = (int)(ptr - brc_buf);

    if( (fd = open( dirfile, O_WRONLY|O_CREAT, 0644)) != -1 ) {
        ftruncate( fd, 0 );
        write( fd, brc_buf, brc_size );
        close( fd );
    }
    brc_changed = 0;
}

int
brc_initial( boardname ) /* ¶ÁÈ¡ÓÃ»§.boardrcÎÄ¼þ£¬È¡³ö±£´æµÄµ±Ç°°æµÄbrc_list */
char    *boardname;
{
    char        dirfile[ STRLEN ], *ptr;
    int         fd;

    if( strcmp( currboard, boardname ) == 0 ) {
        return brc_num;
    }
    brc_update(); /*ÏÈ±£´æµ±Ç°µÄbrc_list*/
    strcpy( currboard, boardname );
    brc_changed = 0;
    if( brc_buf[0] == '\0' ) {
        setuserfile( dirfile, ".boardrc" );
        if( (fd = open( dirfile, O_RDONLY )) != -1 ) {
            brc_size = read( fd, brc_buf, sizeof( brc_buf ) );
            close( fd );
        } else {
            brc_size = 0;
        }
    }
    ptr = brc_buf;
    while( ptr < &brc_buf[ brc_size ] && (*ptr >= ' ' && *ptr <= 'z') ) {
        ptr = brc_getrecord( ptr, brc_name, &brc_num, brc_list );
        if( strncmp( brc_name, currboard, BRC_STRLEN ) == 0 ) {
            return brc_num;
        }
    }
    strncpy( brc_name, boardname, BRC_STRLEN );
    brc_list[0] = 1;
    brc_num = 1;
    return 0;
}

void
brc_addlist( filename )  /*BRClist °´Ë³Ðò²åÈë filetime (filetime=filename) */
char    *filename;
{
    int         ftime, n, i;

    if(!strcmp(currentuser.userid,"guest"))
        return;
    ftime = atoi( &filename[2] );
    if( (filename[0] != 'M'&&filename[0] != 'G') || filename[1] != '.' /*|| ftime <= UNREAD_TIME*/ ) {
        return;
    }
    if( brc_num <= 0 ) {
        brc_list[ brc_num++ ] = ftime;
        brc_changed = 1;
        return;
    }
    for( n = 0; n < brc_num; n++ ) {
        if( ftime == brc_list[n] ) {
            return;
        } else if( ftime > brc_list[n] ) {
            if( brc_num < BRC_MAXNUM )  brc_num++;
            for( i = brc_num - 1; i > n; i-- ) {
                brc_list[ i ] = brc_list[ i - 1 ];
            }
            brc_list[ n ] = ftime;
            brc_changed = 1;
            return;
        }
    }
    if( brc_num < BRC_MAXNUM ) {
        brc_list[ brc_num++ ] = ftime;
        brc_changed = 1;
    }
}

int
brc_unread( filename ) /*Èç¹ûfile±Èbrc listÖÐµÄ¶¼ÐÂ£¬Ôò Î´¶Á */
char    *filename;
{
    int         ftime, n;

    ftime = atoi( &filename[2] );
    if( (filename[0] != 'M'&&filename[0] != 'G') || filename[1] != '.' /*|| ftime <= UNREAD_TIME*/ ) {
        return 0;
    }
    if( brc_num <= 0 )
        return 1;
    for( n = 0; n < brc_num; n++ ) {
        if( ftime > brc_list[n] ) {
            return 1;
        } else if( ftime == brc_list[n] ) {
            return 0;
        }
    }
    return 0;
}

