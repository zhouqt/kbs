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

char    brc_buf[ BRC_MAXSIZE ];
int     brc_size, brc_changed = 0;
char    brc_name[ BRC_STRLEN ];
int     brc_list[ BRC_MAXNUM ], brc_num;
/* added period 2000-09-11	4 FavBoard */
int     favbrd_list[FAVBOARDNUM+1];

struct newpostdata *nbrd; /*每个版的信息*/
int     *zapbuf;
int	zapbuf_changed=0;
int     brdnum, yank_flag = 0;
char    *boardprefix;

void load_favboard(int dohelp)
{
    char fname[STRLEN];
    int  fd, size, idx;
    sethomefile(fname,currentuser->userid, "favboard");
    if( (fd = open( fname, O_RDONLY, 0600 )) != -1 ) {
        size = (FAVBOARDNUM+1) * sizeof( int );
        read( fd, favbrd_list, size );
        close( fd );
    }
#ifdef BBSMAIN
    else if(dohelp) {
        int savmode;
        savmode = uinfo.mode;
        modify_user_mode(CSIE_ANNOUNCE);	/* 没合适的mode.就先用"汲取精华"吧. */
        show_help("help/favboardhelp");
        modify_user_mode(savmode);
    }
#endif
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
        	struct boardheader* bh;
            fd = favbrd_list[idx];
            bh = (struct boardheader*) getboard(fd+1);
            if(fd >= 0 && fd <= get_boardcount() && (
            			bh &&
                        bh->filename[0]
                        && ( (bh->level & PERM_POSTMASK)
                             || HAS_PERM(currentuser,bh->level)
                             || (bh->level&PERM_NOZAP) )
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
    return 0;
}
/*---   ---*/
void
load_zapbuf()  /* 装入zap信息*/
{
    char        fname[ STRLEN ];
    int         fd, size, n;

    size = MAXBOARD * sizeof( int );
    zapbuf = (int *) malloc( size );
    for( n = 0; n < MAXBOARD; n++ )
        zapbuf[n] = 1;
    sethomefile( fname,currentuser->userid, ".lastread" ); /*user的.lastread， zap信息*/
    if( (fd = open( fname, O_RDONLY, 0600 )) != -1 ) {
        size = get_boardcount() * sizeof( int );
        read( fd, zapbuf, size );
        close( fd );
    }
    zapbuf_changed=0;
}
/*---	Modified for FavBoard functions, by period	2000-09-11 */
void save_userfile(char * fname, int numblk, char * buf)
{
    char        fbuf[ 256 ];
    int         fd, size;

    sethomefile( fbuf, currentuser->userid,fname );
    if( (fd = open( fbuf, O_WRONLY | O_CREAT, 0600 )) != -1 ) {
        size = numblk * sizeof( int );
        write( fd, buf, size );
        close( fd );
    }
}

void save_zapbuf()
{
    if (zapbuf_changed!=0)
    	save_userfile(".lastread", get_boardcount(), (char *)zapbuf);
}

#if 0
void
save_zapbuf() /*保存Zap信息*/
{
    char        fname[ STRLEN ];
    int         fd, size;

    sethomefile( fname, currentuser->userid,".lastread" );
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
    struct boardheader  *bptr;
    struct newpostdata  *ptr;
    int         n;

    if( zapbuf == NULL ) {
        load_zapbuf();
    }
    brdnum = 0;
    for( n = 0; n < get_boardcount(); n++ ) {
    	bptr = (struct boardheader*)getboard(n+1);
    	if (!bptr) continue;
#ifndef _DEBUG_
        if(!*bptr->filename) continue;
#endif /*_DEBUG_*/
        if( !(bptr->level & PERM_POSTMASK) && !HAS_PERM(currentuser,bptr->level) && !(bptr->level&PERM_NOZAP))
        {
            continue;
        }
        if( boardprefix != NULL &&
                strchr( boardprefix, bptr->title[0]) == NULL&&boardprefix[0]!='*')
            continue;
        if(boardprefix != NULL&&boardprefix[0]=='*')
        {
            if(!strstr(bptr->title,"●")&&!strstr(bptr->title,"⊙")
                    && bptr->title[0]!='*')
                continue;
        }
        if(boardprefix == NULL && bptr->title[0]=='*')
            continue;
        /*---	period	2000-09-11	4 FavBoard	---*/
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


char *
brc_getrecord( ptr, name, pnum, list ) /*取出一个版的brclist*/
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
brc_putrecord( ptr, name, num, list ) /* 保存一个版的brclist*/
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

void brc_update(char *userid, char *board) {
        char    dirfile[STRLEN], *ptr;
        char    tmp_buf[BRC_MAXSIZE - BRC_ITEMSIZE], *tmp;
        char    tmp_name[BRC_STRLEN];
        int     tmp_list[BRC_MAXNUM], tmp_num;
        int     fd, tmp_size;
	if (brc_changed == 0) {
		return;
	}

        ptr = brc_buf;
        if (brc_num > 0) {
                ptr = brc_putrecord(ptr, brc_name, brc_num, brc_list);
        }
        if (1) {
                sethomefile(dirfile, userid, ".boardrc");
                if ((fd = open(dirfile, O_RDONLY)) != -1) {
                        tmp_size = read(fd, tmp_buf, sizeof(tmp_buf));
                        close(fd);
                } else {
                        tmp_size = 0;
                }
        }
        tmp = tmp_buf;
        while (tmp < &tmp_buf[tmp_size] && (*tmp >= ' ' && *tmp <= 'z')) {
                tmp = brc_getrecord(tmp, tmp_name, &tmp_num, tmp_list);
                if (strncmp(tmp_name, board, BRC_STRLEN) != 0) {
                        ptr = brc_putrecord(ptr, tmp_name, tmp_num, tmp_list);
                }
        }
        brc_size = (int) (ptr - brc_buf);
        if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1) {
                ftruncate(fd, 0);
                write(fd, brc_buf, brc_size);
                close(fd);
        }
	brc_changed = 0;
}

int
brc_initial(char *userid, char *boardname ) /* 读取用户.boardrc文件，取出保存的当前版的brc_list */
{
    char        dirfile[ STRLEN ], *ptr;
    int         fd;

#ifdef BBSMAIN
    if( strcmp( currboard, boardname ) == 0 ) {
        return brc_num;
    }
    brc_update(currentuser->userid,currboard); /*先保存当前的brc_list*/
    strcpy( currboard, boardname );
    if( brc_buf[0] == '\0' ) {
#endif
        sethomefile( dirfile, userid, ".boardrc" );
        if( (fd = open( dirfile, O_RDONLY )) != -1 ) {
            brc_size = read( fd, brc_buf, sizeof( brc_buf ) );
            close( fd );
        } else {
            brc_size = 0;
        }
#ifdef BBSMAIN
    }
#endif
    ptr = brc_buf;
    while( ptr < &brc_buf[ brc_size ] && (*ptr >= ' ' && *ptr <= 'z') ) {
        ptr = brc_getrecord( ptr, brc_name, &brc_num, brc_list );
        if( strncmp( brc_name, boardname , BRC_STRLEN ) == 0 ) {
            return brc_num;
        }
    }
    strncpy( brc_name, boardname, BRC_STRLEN );
    brc_list[0] = 1;
    brc_num = 1;
    return 0;
}


void
brc_addlist( filename )/*  BRClist 按顺序插入 filetime (filetime=filename) */
char    *filename;
{
    int         ftime, n, i;

    if(!strcmp(currentuser->userid,"guest"))
        return;
    ftime = atoi( &filename[2] );
    if( (filename[0] != 'M'&&filename[0] != 'G') || filename[1] != '.' ) {
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

int brc_unread_t( int ftime) 
{
    int         n;

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

int
brc_unread( filename ) /*如果file比brc list中的都新，则 未读 */
char    *filename;
{
    int         ftime, n;

    ftime = atoi( &filename[2] );
    if( (filename[0] != 'M'&&filename[0] != 'G') || filename[1] != '.' /*|| ftime <= UNREAD_TIME*/ ) {
        return 0;
    }
    return brc_unread_t(ftime);
}

int brc_has_read(char *file) {
	return !brc_unread(file);
}

int brc_add_read(char *filename) {
        int     ftime, n, i;
        ftime=atoi(&filename[2]);
        if(filename[0]!='M' && filename[0]!='G')  return;
        if(brc_num<=0) {
                brc_list[brc_num++] = ftime;
		brc_changed = 1;
                return;
        }
        for (n = 0; n < brc_num; n++) {
                if (ftime == brc_list[n]) {
                        return;
                } else if (ftime > brc_list[n]) {
                        if (brc_num < BRC_MAXNUM)
                                brc_num++;
                        for (i = brc_num - 1; i > n; i--) {
                                brc_list[i] = brc_list[i - 1];
                        }
                        brc_list[n] = ftime;
			brc_changed = 1;
                        return;
                }
        }
        if(brc_num<BRC_MAXNUM) brc_list[brc_num++] = ftime;
	brc_changed = 1;
}

int brc_clear() {
	int i;
	char filename[20];
	for(i=0; i<BRC_MAXNUM; i++) {
		sprintf(filename, "M.%d.A", time(0)-i);
		brc_add_read(filename);
	}
}

int brc_clear_new_flag(char* filename)
{
        int     ftime, n, i;
        ftime=atoi(&filename[2]);
        if(filename[0]!='M' && filename[0]!='G')  return;
        for (n = 0; n < brc_num; n++) 
                if (ftime >= brc_list[n]) 
			break;
	if (n<BRC_MAXNUM) {
		brc_list[n] = ftime;
		brc_num=n+1;
        	brc_changed = 1;
		for(i=n+1; i<BRC_MAXNUM; i++) 
			brc_list[i]=ftime-(i-n);
	}
	brc_num=BRC_MAXNUM;
	brc_changed = 1;
	return;
}

int junkboard(char* currboard)  /* 判断当前版是否为 junkboards */
{
    return seek_in_file("etc/junkboards",currboard);
}

int
checkreadonly( char *board) /* 检查是否是只读版面 */
{
    struct boardheader* bh=getbcache(board);
    if (bh&&(bh->flag & BOARD_READONLY)) /* Checking if DIR access mode is "555" */
        return YEA;
    else
        return NA;
}

int
deny_me(char* user,char* board)   /* 判断用户 是否被禁止在当前版发文章 */
{
    char buf[STRLEN];

    setbfile(buf,board,"deny_users");
    return seek_in_file(buf,user);
}



int haspostperm(struct userec* user,char *bname) /* 判断在 bname版 是否有post权 */
{
    register int i;

#ifdef BBSMAIN
    if(digestmode)
        return 0;
#endif
    /*    if( strcmp( bname, DEFAULTBOARD ) == 0 )  return 1; change by KCN 2000.09.01 */
    if ((i = getbnum(bname)) == 0) return 0;
    if (HAS_PERM(user,PERM_DENYPOST))
        /*if(!strcmp(bname, "sysop"))
               return 1;*/ /* Leeward 98.05.21 revised by stephen 2000.10.27*/ 
        /* let user denied post right post at Complain*/
    {if (!strcmp(bname, "Complain")) return 1;/* added by stephen 2000.10.27*/
        else if(!strcmp(bname, "sysop"))
            return 1;} /* stephen 2000.10.27 */
    if (!HAS_PERM(user,PERM_POST)) return 0;
    return (HAS_PERM(user,(bcache[i-1].level&~PERM_NOZAP) & ~PERM_POSTMASK));
}


int chk_BM_instr(char BMstr[STRLEN-1],char    bmname[IDLEN+2])
{
    char *ptr;
    char BMstrbuf[STRLEN-1];

    strcpy(BMstrbuf,BMstr);
    ptr=strtok(BMstrbuf,",: ;|&()\0\n");
    while(1)
    {
        if(ptr==NULL)
            return NA;
        if(!strcmp(ptr,bmname/*,strlen(currentuser->userid)*/))
            return YEA;
        ptr=strtok(NULL,",: ;|&()\0\n");
    }
}


int chk_currBM(char BMstr[STRLEN-1],struct userec* user)   
	/* 根据输入的版主名单 判断user是否有版主 权限*/
{
    char *ptr;
    char BMstrbuf[STRLEN-1];

    if(HAS_PERM(currentuser,PERM_OBOARDS)||HAS_PERM(currentuser,PERM_SYSOP))
        return YEA;

    if(!HAS_PERM(currentuser,PERM_BOARDS))
        return NA;

    return chk_BM_instr(BMstr, currentuser->userid);
}


