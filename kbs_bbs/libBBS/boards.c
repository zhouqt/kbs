/*
     版的一些操作
*/

#include "bbs.h"

#define BRC_MAXNUM      50
#define BRC_ITEMSIZE    (BRC_MAXNUM * sizeof( time_t ))
#define BRC_FILESIZE BRC_ITEMSIZE*MAXBOARD

#define BRC_CACHE_NUM 60 /* 未读标记cache 20个板*/

static struct _brc_cache_entry {
	int bid;
	time_t list[ BRC_MAXNUM ];
	int changed;
} brc_cache_entry[BRC_CACHE_NUM];
static int brc_currcache;

/* .boardrc文件的结构。
    这个结构比原来要费一点硬盘空间，但读写方便了
    以后可以考虑加上zip压缩存储brc文件
*/
struct brc_struct {
	time_t list[MAXBOARD][BRC_MAXNUM];
};

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

int getfavnum()
{
	return *favbrd_list;
}

void addFavBoard(int i)
{
	int llen;
	if (getfavnum()<FAVBOARDNUM) {
	llen = ++(*favbrd_list);
	favbrd_list[llen] = i;
	};
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

void brc_update(char *userid) {
	int i;
	int fd=-1;
    	char dirfile[MAXPATH];
	
    	sethomefile( dirfile, userid, ".boardrc" );
	for (i=0;i<BRC_CACHE_NUM;i++) {
		if (brc_cache_entry[i].changed) {
			int j;
			if (fd==-1) {
				if ((fd=open(dirfile,O_RDWR))==-1) {
					bbslog("3user","can't open to readwrite",dirfile);
					return;
				};
			}
			lseek(fd,(brc_cache_entry[i].bid-1)*BRC_ITEMSIZE,SEEK_SET);
			write(fd,&brc_cache_entry[i].list,BRC_ITEMSIZE);

		}
	}
	if (fd!=-1)
		close(fd);
	return;
}


#define BRC_OLD_MAXSIZE     32768
#define BRC_OLD_MAXNUM      60
#define BRC_OLD_STRLEN      15
#define BRC_OLD_ITEMSIZE    (BRC_OLD_STRLEN + 1 + BRC_OLD_MAXNUM * sizeof( int ))

static int brc_convert_struct(char* dirfile,char* data,int size) /* 把旧的broardrc文件格式转换成新的*/
{
	struct brc_struct brc;
	char* ptr;
	int fd;
	ptr = data;
	bzero(&brc,sizeof(brc));
	while( ptr < &data[ size ] && (*ptr >= ' ' && *ptr <= 'z') ) {
	    int num;
	    char* tmp;
	    char boardname[18];
	    int bid;
	    strncpy(boardname,ptr,BRC_OLD_STRLEN);
	    boardname[BRC_OLD_STRLEN]=0;
           bid=getbnum(boardname);
           ptr+=BRC_OLD_STRLEN;
           num=(*ptr++) & 0xff;
           tmp=ptr;
    	    ptr+=sizeof(int)*num;
    	    if( num > BRC_MAXNUM ) { 
        		num = BRC_MAXNUM;
    	    }
    	    memcpy( brc.list[bid-1], tmp, num * sizeof( int ) );
	}
   	if( (fd = open( dirfile, O_WRONLY|O_CREAT )) != -1 ) {
            write( fd, &brc, sizeof(brc));
            close( fd );
    	}

	return 0;
}

static int brc_getcache(char* userid)
{
	int i,unchange=-1;
	for (i=0;i<BRC_CACHE_NUM;i++) {
		if (brc_cache_entry[i].bid==0)
			return i;
		if (brc_cache_entry[i].changed==0)
			unchange=i;
	}
	if (unchange!=-1) return unchange;
	brc_update(userid);

	return 0;
}

int brc_initial(char *userid, char *boardname ) /* 读取用户.boardrc文件，取出保存的当前版的brc_list */
{
    int entry;
    int i;
    char dirfile[MAXPATH];
    int brc_size;
    int bid=getbnum(boardname);
    int fd;
    struct boardheader const* bptr;

#ifdef BBSMAIN
    strncpy( currboard, boardname , STRLEN-BM_LEN); /*很是ft,居然在这里置currboard*/
    currboard[STRLEN-BM_LEN]=0;
#endif
    for (i=0;i<BRC_CACHE_NUM;i++) 
    	if (brc_cache_entry[i].bid==bid) {
    		brc_currcache=i;
    		return 1; /* cache 中有*/
    	}

    sethomefile( dirfile, userid, ".boardrc" );
    while (1) { /*如果是老版的.boardrc，需要重新读一遍*/
	    struct stat st;
	    if( (fd = open( dirfile, O_RDONLY )) != -1 ) {
		    fstat(fd,&st);
		    brc_size=st.st_size;
	    } else {
	            brc_size = 0;
	    }

	    if ((brc_size)&&(brc_size<BRC_FILESIZE)) { 
	    	/* 老版的boardrc,因为应该只需要转化一次，不考虑效率啦*/
	    	char brc_buffer[BRC_OLD_MAXSIZE];
		if( lseek(fd,0,SEEK_SET)  != -1 ) {
		        brc_size = read( fd, brc_buffer, sizeof( brc_buffer) );
		        close(fd);
    			 brc_size = brc_convert_struct(dirfile,brc_buffer, brc_size);
		} else {
		        brc_size = 0;
		}
	    } else break;
    }

    entry=brc_getcache(userid);
    bptr=getboard(bid);
    lseek(fd,(bid-1)*BRC_ITEMSIZE,SEEK_SET);
    read(fd,&brc_cache_entry[entry].list,BRC_ITEMSIZE);
    /*
    			 先不加入版面的创建时间的判断
    if (brc_cache_entry[entry].list[0])
    		&&(brc_cache_entry[entry].list[0]<bptr->createtime) )
    {
    		brc_cache_entry[entry].changed=1;
    		brc_cache_entry[entry].list[0]=0;
    } else*/
    {
    		brc_cache_entry[entry].changed=0;
    		brc_cache_entry[entry].bid=bid;
    }
    brc_currcache=entry;
    close(fd);
    return 1;
}


int brc_unread( int ftime) 
{
    int         n;

    for( n = 0; n < BRC_MAXNUM; n++ ) {
    	if (brc_cache_entry[brc_currcache].list[n]==0) {
		 if (n==0) return 1;
		 return 0;
        }
        if( ftime > brc_cache_entry[brc_currcache].list[n] ) {
            return 1;
        } else if( ftime == brc_cache_entry[brc_currcache].list[n] ) {
            return 0;
        }
    }
    return 0;
}

/*
int brc_has_read(char *file) {
	return !brc_unread(FILENAME2POSTTIME( file));
}
*/

void brc_add_read(char *filename) {
        int     ftime, n, i;
        ftime=atoi(&filename[2]);
        if(filename[0]!='M' && filename[0]!='G')  return;

        for (n = 0; (n < BRC_MAXNUM)&&brc_cache_entry[brc_currcache].list[n]; n++) {
                if (ftime == brc_cache_entry[brc_currcache].list[n]) {
                        return;
                } else if (ftime > brc_cache_entry[brc_currcache].list[n]) {
                        for (i =  BRC_MAXNUM - 1; i > n; i--) {
                                brc_cache_entry[brc_currcache].list[i] = brc_cache_entry[brc_currcache].list[i - 1];
                        }
                        brc_cache_entry[brc_currcache].list[n] = ftime;
			   brc_cache_entry[brc_currcache].changed = 1;
                        return;
                }
        }
        /* 这个地方加入是不对的，因为只可能有2情况，一个是根本没有unread记录，
         * 或者所有list[n]的时间之前的文章都被认为已读
         if (n!=BRC_MAXNUM) {
             brc_cache_entry[brc_currcache].list[n]=ftime;
             n++;
             if (n!=BRC_MAXNUM)
                 brc_cache_entry[brc_currcache].list[n]=0;
             brc_cache_entry[brc_currcache].changed = 1;
         }
         应该用如下
         */
        if (n==0) {
            for (n=0;n<BRC_MAXNUM-1;n++)
                brc_cache_entry[brc_currcache].list[n]=ftime;
            brc_cache_entry[brc_currcache].list[n]=0;
            brc_cache_entry[brc_currcache].changed = 1;
        }
}

int brc_clear() {
	brc_cache_entry[brc_currcache].list[0]=time(0);
	brc_cache_entry[brc_currcache].list[1]=0;
	brc_cache_entry[brc_currcache].changed=1;
}

int brc_clear_new_flag(char* filename)
{
        int     ftime, n, i;
        ftime=atoi(&filename[2]);
        if(filename[0]!='M' && filename[0]!='G')  return;
        for (n = 0; (n < BRC_MAXNUM)&&brc_cache_entry[brc_currcache].list[n]; n++) 
                if (ftime >= brc_cache_entry[brc_currcache].list[n]) 
			break;
	if (n<BRC_MAXNUM) {
		brc_cache_entry[brc_currcache].list[n] = ftime;
        	if (n+1<BRC_MAXNUM)
        		brc_cache_entry[brc_currcache].list[n+1]=0;
        	brc_cache_entry[brc_currcache].changed = 1;
	}
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


