/*
     版的一些操作
*/

#include "bbs.h"
#include <zlib.h>

#ifdef HAVE_BRC_CONTROL
#if USE_TMPFS==1
struct _brc_cache_entry* brc_cache_entry=NULL;
#else
struct _brc_cache_entry brc_cache_entry[BRC_CACHE_NUM];
#endif
static int brc_currcache=-1;
#endif

static const char NullChar[] = "";
static const char EmptyChar[] = "空";

/* added by bad 2002-08-3	FavBoardDir */
/* stiger:
   操作的favbrd_list其实是一个指针
   当自己的时候指向mybrd_list
   当讨论区目录时指向bdirshm->allbrd_list

   favbrd_list_t同理
   */
struct favbrd_struct *favbrd_list=NULL;
int *favbrd_list_count;
struct favbrd_struct mybrd_list[FAVBOARDNUM];
int mybrd_list_t = -1, favnow = 0;

#define favbrd_list_t (*favbrd_list_count)

int *zapbuf;
int zapbuf_changed = 0;

int valid_brdname(brd)
char *brd;
{
    char ch;

    ch = *brd++;
    if (!isalnum(ch) && ch != '_')
        return 0;
    while ((ch = *brd++) != '\0') {
        if (!isalnum(ch) && ch != '_' && ch != '.')
            return 0;
    }
    return 1;
}

void load_wwwboard(struct favbrd_struct *brdlist, int * brdlist_t)
{
	int fd, sign=0;

	(*brdlist_t) = 0;
	bzero(brdlist, sizeof(struct favbrd_struct)*FAVBOARDNUM);

    if ((fd = open("etc/wwwboard.dir", O_RDONLY, 0600)) != -1) {
        read(fd, &sign, sizeof(int));
		if(sign==0x8081){
            read(fd, brdlist_t, sizeof(int));
			read(fd, brdlist, sizeof(struct favbrd_struct) * (*brdlist_t) );
		}
		close(fd);
	}
	if(*brdlist_t <= 0){
        brdlist[0].bnum = 1;
        brdlist[0].bid[0] = 0;
		brdlist[0].father = -1;
		(*brdlist_t)=1;
	}	
}

void load_allboard(struct favbrd_struct *brdlist, int * brdlist_t)
{
	int fd, sign=0;

	(*brdlist_t) = 0;
	bzero(brdlist, sizeof(struct favbrd_struct)*FAVBOARDNUM);

    if ((fd = open("etc/board.dir", O_RDONLY, 0600)) != -1) {
        read(fd, &sign, sizeof(int));
		if(sign==0x8081){
            read(fd, brdlist_t, sizeof(int));
			read(fd, brdlist, sizeof(struct favbrd_struct) * (*brdlist_t) );
		}
		close(fd);
	}
	if(*brdlist_t <= 0){
        brdlist[0].bnum = 1;
        brdlist[0].bid[0] = 0;
		brdlist[0].father = -1;
		(*brdlist_t)=1;
	}	
}

void load_favboard(int dohelp,int mode)
{
    char fname[STRLEN];
    int fd, sign, i, j, k;

	if(mode==2)
		sprintf(fname,"etc/board.dir");
	else if(mode==3)
		sprintf(fname,"etc/wwwboard.dir");
	else
	    sethomefile(fname, currentuser->userid, "favboard");
    favnow = 0;
	
	if(mode==2){
		favbrd_list=bdirshm->allbrd_list;
		favbrd_list_count = &bdirshm->allbrd_list_t;
		return;
	}else if(mode==3){
		favbrd_list=bdirshm->wwwbrd_list;
		favbrd_list_count = &bdirshm->wwwbrd_list_t;
		return;
	}else{
		favbrd_list=mybrd_list;
		favbrd_list_count=&mybrd_list_t;
#ifdef BBSMAIN
		if(favbrd_list_t > 0)
			return;
#endif
	}

	bzero(favbrd_list, sizeof(struct favbrd_struct)*FAVBOARDNUM);
    if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
        read(fd, &sign, sizeof(int));
        if (sign != 0x8080 && sign!=0x8081 ) {      /* We can consider the 0x8080 magic number as a 
                                 * * version identifier of favboard file. */
            /*
             * We handle old version here. 
             */
            favbrd_list_t = 1;
            favbrd_list[0].father = -1;
            for (k=0; k<MAXBOARDPERDIR; k++) {
                if( read(fd, &j, sizeof(int)) <= 0)
					break;
				if(j<0) j=0;
                favbrd_list[0].bid[k] = j;
            }
			favbrd_list[0].bnum = k;
        } else if(sign==0x8080) {
            /*
             * We handle new version here. 
             */
			struct favbrd_struct_old{
					/*flag > 0: 版面
					  flag < 0: -flag表示这个目录实际对应的新的目录	id
					  */
				int flag;
				int father;
				char buf[256];
			} favbrd_list_tmp[FAVBOARDNUM];

            read(fd, &k, sizeof(int));
            for (i = 0; i < k; i++) {
                read(fd, &j, sizeof(int));
                favbrd_list_tmp[i].flag = j;
                if (j == -1) {
                    char len;
					int lll;
                    read(fd, &len, sizeof(char));
                    read(fd, favbrd_list_tmp[i].buf, len);
					lll = len;
					favbrd_list_tmp[i].buf[lll]='\0';
                }
                read(fd, &j, sizeof(int));
                favbrd_list_tmp[i].father = j;
            }

			favbrd_list_t=1;
			favbrd_list[0].father=-1;
			for(i=0;i<k;i++){
				if( favbrd_list_tmp[i].flag == -1 ){
					favbrd_list_tmp[i].flag = 0 - favbrd_list_t;
					strncpy(favbrd_list[favbrd_list_t].title, favbrd_list_tmp[i].buf, 80);
					favbrd_list[favbrd_list_t].title[80]=0;
					favbrd_list_t++;
				}
			}
			for(i=0;i<k;i++){
				int newfather;
				if( favbrd_list_tmp[i].father <= -1 )
					newfather=0;
				else
					newfather = 0-favbrd_list_tmp[favbrd_list_tmp[i].father].flag;

				if( favbrd_list_tmp[i].flag < 0 ){
					if(newfather >= 0 && favbrd_list[newfather].bnum < MAXBOARDPERDIR){
						favbrd_list[newfather].bid[favbrd_list[newfather].bnum]=favbrd_list_tmp[i].flag;
						favbrd_list[newfather].bnum++;
						favbrd_list[0-favbrd_list_tmp[i].flag].father = newfather;
					}
				}else{
					if(newfather >= 0 && favbrd_list[newfather].bnum < MAXBOARDPERDIR){
						favbrd_list[newfather].bid[favbrd_list[newfather].bnum]=favbrd_list_tmp[i].flag;
						favbrd_list[newfather].bnum++;
					}
				}
			}
		}else{
            read(fd, &favbrd_list_t, sizeof(int));
			read(fd, favbrd_list, sizeof(struct favbrd_struct) * favbrd_list_t);
        }
        close(fd);
    }
#ifdef BBSMAIN
    else if (dohelp) {
        int savmode;

        savmode = uinfo.mode;
        modify_user_mode(CSIE_ANNOUNCE);        /* 没合适的mode.就先用"汲取精华"吧. */
        show_help("help/favboardhelp");
        modify_user_mode(savmode);
    }
#endif
    if ((favbrd_list_t <= 0)) {
        char bn[40];
        FILE* fp=fopen("etc/initial_favboard", "r");
        if(!fp) {
            favbrd_list_t = 1;      /*  favorate board count    */
            favbrd_list[0].bnum = 1;
            favbrd_list[0].bid[0] = 0;
			favbrd_list[0].father = -1;
        } else {
            favbrd_list_t = 1;      /*  favorate board count    */
			favbrd_list[0].father = -1;
            while(!feof(fp)) {
                int k;
                if(fscanf(fp, "%s", bn)<1) break;
                k=getbnum(bn);
                if(k) {
					if(favbrd_list[0].bnum < MAXBOARDPERDIR){
						favbrd_list[0].bid[favbrd_list[0].bnum]=k-1;
						favbrd_list[0].bnum++;
					}
                }
            }
            fclose(fp);
        }
    } else if(mode!=2 && mode!=3){
        int change=0;
		struct boardheader *bh;

		for(i=0;i<favbrd_list_t;i++){
			for(j=0;j<favbrd_list[i].bnum;j++){
				fd = favbrd_list[i].bid[j];
				if (fd < 0)
					continue;
				bh = (struct boardheader *) getboard(fd + 1);
				if (fd <= get_boardcount() && (bh && bh->filename[0] && (check_see_perm(currentuser,bh)) ) )
				    continue;
				for(k=j;k<favbrd_list[i].bnum-1;k++){
					favbrd_list[i].bid[k]=favbrd_list[i].bid[k+1];
				}
				favbrd_list[i].bid[k]=0;
				favbrd_list[i].bnum--;
				j--;
				change=1;
			}
		}
		if(change)
            save_favboard(mode);
    }
}

void save_favboard(int mode)
{
    int fd, i;
    char fname[MAXPATH];

	if(mode==2)
		sprintf(fname,"etc/board.dir");
	else if(mode==3)
		sprintf(fname,"etc/wwwboard.dir");
	else
		sethomefile(fname, currentuser->userid, "favboard");

	if( (mode==2 || mode==3 ) && !HAS_PERM(currentuser,PERM_SYSOP))
		return;

    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
        i = 0x8081;
        write(fd, &i, sizeof(int));
        write(fd, &favbrd_list_t, sizeof(int));
        for (i = 0; i < favbrd_list_t; i++) {
            write(fd, &favbrd_list[i], sizeof(struct favbrd_struct));
        }
        close(fd);
    }
}

int IsFavBoard(int idx)
{
    int i;

    for (i = 0; i < favbrd_list[favnow].bnum ; i++)
        if (idx == favbrd_list[favnow].bid[i])
            return i + 1;
    return 0;
}

int ExistFavBoard(int idx)
{
    int i;

    for (i = 0; i < favbrd_list[favnow].bnum; i++)
        if (idx == favbrd_list[favnow].bid[i])
            return i + 1;
    return 0;
}

int changeFavBoardDir(int i, char *s)
{
    if (i >= favbrd_list_t)
        return -1;
    strncpy(favbrd_list[i].title, s, 80);
	favbrd_list[i].title[80]=0;
    return 0;
}

int getfavnum()
{
    return favbrd_list[favnow].bnum;
}

/* i是bid */
void addFavBoard(int i)
{
    if (favbrd_list[favnow].bnum < MAXBOARDPERDIR) {
        favbrd_list[favnow].bid[favbrd_list[favnow].bnum] = i;
		favbrd_list[favnow].bnum++;
    };
}

void addFavBoardDir(char *s)
{
    if (favbrd_list[favnow].bnum < MAXBOARDPERDIR && favbrd_list_t < FAVBOARDNUM) {
        favbrd_list[favbrd_list_t].level = 0;
        favbrd_list[favbrd_list_t].bnum = 0;
        favbrd_list[favbrd_list_t].father = favnow;
        strncpy(favbrd_list[favbrd_list_t].title, s, 80);
		favbrd_list[favbrd_list_t].title[80]=0;
		favbrd_list[favnow].bid[favbrd_list[favnow].bnum]=0-favbrd_list_t;
        favbrd_list_t++;
		favbrd_list[favnow].bnum++;
    };
}

int SetFav(int i)
{
    int j;

    j = favnow;
    favnow = i;
    return j;
}

/* i是bid */
int DelFavBoard(int i)
{
    int j,k;

	for(j=0;j<favbrd_list[favnow].bnum;j++){
		if(i==favbrd_list[favnow].bid[j]){
			for(k=j; k<favbrd_list[favnow].bnum-1; k++){
				favbrd_list[favnow].bid[k] = favbrd_list[favnow].bid[k+1];
			}
			favbrd_list[favnow].bid[k] = 0;
			favbrd_list[favnow].bnum --;
		}
	}
	return 0;
}

/* 删除 favbrd_list[father].bid[i] */
int DelFavBoardDir(int i,int fath)
{
    int j,k;
	int father=fath;
	int n;

    if (i >= favbrd_list[father].bnum)
        return favbrd_list_t;
    if (i < 0)
        return favbrd_list_t;
	//j是要删除的目录序号
	j=0-favbrd_list[father].bid[i];
    for (k = 0; k < favbrd_list[j].bnum; k++){
		//如果有子目录
        if (favbrd_list[j].bid[k] < 0) {
			//得到子目录的节点号
			n=0-favbrd_list[j].bid[k];
			//删除子目录
            DelFavBoardDir(k,j);
            if (n < j)
                j--;
			if (n < father)
				father--;
            k--;
        }
	}
	//总的目录数减一
    favbrd_list_t--;
	//移动j之后的目录
    for (k = j; k < favbrd_list_t; k++)
        favbrd_list[k] = favbrd_list[k + 1];
	bzero(&favbrd_list[k], sizeof(struct favbrd_struct));
	//如果当前的father在j之后，那么father要前移一个
	if (father >= j)
		father--;
	//吧所有的father在j之后的目录的father--
    for (k = 0; k < favbrd_list_t; k++)
        if (favbrd_list[k].father >= j)
            favbrd_list[k].father--;
	//吧所有指向j之后的bid提前一个
	for (k=0; k<favbrd_list_t;k++){
		for (n=0; n<favbrd_list[k].bnum; n++){
			if( 0-favbrd_list[k].bid[n] > j )
				favbrd_list[k].bid[n]++;
		}
	}
	//处理当前father节点的索引
	favbrd_list[father].bnum--;
	for (k=i;k<favbrd_list[father].bnum;k++)
		favbrd_list[father].bid[k] = favbrd_list[father].bid[k+1];
	favbrd_list[father].bid[k]=0;
	//处理favnow
    if (favnow >= j)
        favnow--;
    return 0;
}

/*把版面p得位置得移动到位置q, p,q start from 0 */
int MoveFavBoard(int p, int q)
{
    int k;
	int i;

	if(p<0 || q<0 || p>=favbrd_list[favnow].bnum || q>=favbrd_list[favnow].bnum)
		return -1;
	if(p == q)
		return 0;
	i=favbrd_list[favnow].bid[p];
	if( p > q ){
		for(k=p; k>q; k--)
			favbrd_list[favnow].bid[k] = favbrd_list[favnow].bid[k-1];
		favbrd_list[favnow].bid[k] = i;
	}else{
		for(k=p; k<q; k++)
			favbrd_list[favnow].bid[k] = favbrd_list[favnow].bid[k+1];
		favbrd_list[favnow].bid[k] = i;
	}
	return 0;
}

/*---   ---*/
void load_zapbuf()
{                               /* 装入zap信息 */
    char fname[STRLEN];
    int fd, size, n;

    size = MAXBOARD * sizeof(int);
    zapbuf = (int *) malloc(size);
    for (n = 0; n < MAXBOARD; n++)
        zapbuf[n] = 1;
    sethomefile(fname, currentuser->userid, ".lastread");       /*user的.lastread， zap信息 */
    if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
        size = get_boardcount() * sizeof(int);
        read(fd, zapbuf, size);
        close(fd);
    }
    zapbuf_changed = 0;
}

/*---	Modified for FavBoard functions, by period	2000-09-11 */
void save_userfile(char *fname, int numblk, char *buf)
{
    char fbuf[256];
    int fd, size;

    sethomefile(fbuf, currentuser->userid, fname);
    if ((fd = open(fbuf, O_WRONLY | O_CREAT, 0600)) != -1) {
        size = numblk * sizeof(int);
        write(fd, buf, size);
        close(fd);
    }
}

void save_zapbuf()
{
    if (zapbuf_changed != 0)
        save_userfile(".lastread", get_boardcount(), (char *) zapbuf);
}

#if 0
void save_zapbuf()
{                               /*保存Zap信息 */
    char fname[STRLEN];
    int fd, size;

    sethomefile(fname, currentuser->userid, ".lastread");
    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
        size = numboards * sizeof(int);
        write(fd, zapbuf, size);
        close(fd);
    }
}
#endif

#ifdef HAVE_BRC_CONTROL

void brc_update(const char *userid)
{
    int i;
    gzFile fd = NULL;
    char dirfile[MAXPATH];
    unsigned int data[MAXBOARD][BRC_MAXNUM];
    int count;

    if (brc_cache_entry==NULL) return;
    /*干脆不搞guest的这个算了*/
    if (!strcmp(userid,"guest")) return;
    sethomefile(dirfile, userid, BRCFILE);
    for (i = 0; i < BRC_CACHE_NUM; i++) {
        if (brc_cache_entry[i].changed) {
            break;
        }
    }
    if (i == BRC_CACHE_NUM)
        return;
    bzero(data, BRC_FILESIZE);
    if ((fd = gzopen(dirfile, "rb6")) == NULL) {
        const char *errstr;
        int gzerrno;

        errstr = gzerror(fd, &gzerrno);
        if (errno == Z_ERRNO)
            errstr = strerror(errno);
//        bbslog("3error", "can't %s open to read:%s", dirfile, errstr);
	f_rm(dirfile);
//        return;
    } else {
    count = 0;
    while (count < BRC_FILESIZE) {
        int ret;

        ret = gzread(fd, (char *) (&data) + count, BRC_FILESIZE - count);
        if (ret <= 0)
            break;
        count += ret;
    }
    gzclose(fd);
    }

    if ((fd = gzopen(dirfile, "w+b6")) == NULL) {
        const char *errstr;
        int gzerrno;

        errstr = gzerror(fd, &gzerrno);
        if (errno == Z_ERRNO)
            errstr = strerror(errno);
//        bbslog("3error", "can't %s open to write:%s", dirfile, errstr);
        f_rm(dirfile);
    }
//        return;
//            } else {

    for (i = 0; i < BRC_CACHE_NUM; i++) {
        if (brc_cache_entry[i].changed)
            memcpy(data[brc_cache_entry[i].bid - 1], &brc_cache_entry[i].list, BRC_ITEMSIZE);
    }
    count = 0;
    while (count < BRC_FILESIZE) {
        int ret;

        ret = gzwrite(fd, (char *) (&data) + count, BRC_FILESIZE - count);
        if (ret == 0)
            break;
        count += ret;
    }
    gzclose(fd);
    return;
}

static int brc_getcache(const char *userid)
{
    int i, unchange = -1;

    for (i = 0; i < BRC_CACHE_NUM; i++) {
        if (brc_cache_entry[i].bid == 0)
            return i;
        if (brc_cache_entry[i].changed == 0)
            unchange = i;
    }
    if (unchange != -1)
        return unchange;
    brc_update(userid);

    return 0;
}

void brc_addreaddirectly(char *userid, int bnum, unsigned int postid)
{
    char dirfile[MAXPATH];
    int i, n;
    int list[BRC_MAXNUM];
    gzFile fd;

    /*干脆不搞guest的这个算了*/
    if (!strcmp(userid,"guest")) return;
    sethomefile(dirfile, userid, BRCFILE);

    if ((fd = gzopen(dirfile, "w+b6")) == NULL) {
        const char *errstr;
        int gzerrno;

        errstr = gzerror(fd, &gzerrno);
        if (errno == Z_ERRNO)
            errstr = strerror(errno);
//        bbslog("3error", "can't %s open to readwrite:%s", dirfile, errstr);
        return;
    }
    gzseek(fd, BRC_ITEMSIZE * (bnum - 1), SEEK_SET);
    gzread(fd, list, BRC_MAXNUM * sizeof(int));
    for (n = 0; (n < BRC_MAXNUM) && list[n]; n++) {
        if (postid == list[n]) {
            gzclose(fd);
            return;
        } else if (postid > list[n]) {
            for (i = BRC_MAXNUM - 1; i > n; i--)
                list[i] = list[i - 1];
            list[n] = postid;
            gzseek(fd, BRC_ITEMSIZE * (bnum - 1), SEEK_SET);
            gzwrite(fd, list, BRC_MAXNUM * sizeof(int));
            gzclose(fd);
            return;
        }
    }
    if (n == 0) {
        for (n = 0; n < BRC_MAXNUM; n++)
            list[n] = postid;
        list[n] = 0;
    }
    gzseek(fd, BRC_ITEMSIZE * (bnum - 1), SEEK_SET);
    gzwrite(fd, list, BRC_MAXNUM * sizeof(int));
    gzclose(fd);
    return;
}

#if USE_TMPFS==1
void free_brc_cache(char *userid){

	if( strcmp( userid ,"guest") ){
        if (brc_cache_entry)
            munmap(brc_cache_entry,BRC_CACHE_NUM*sizeof(struct _brc_cache_entry));
    }
}
#endif

#if USE_TMPFS==1
void init_brc_cache(const char* userid,bool replace) {
    if ((brc_cache_entry==NULL)||(replace)) {
        char dirfile[MAXPATH];
        char temp[MAXPATH];
        int brcfdr;
	struct stat st;
	if (brc_cache_entry)
		munmap(brc_cache_entry,BRC_CACHE_NUM*sizeof(struct _brc_cache_entry));
        setcachehomefile(temp, userid, -1, NULL);
        mkdir(temp, 0700);
        setcachehomefile(temp, userid, -1, "entry");
	sprintf(dirfile,BBSHOME "/%s",temp);
        if(stat(dirfile, &st)<0) {
            char brc[BRC_CACHE_NUM*sizeof(struct _brc_cache_entry)];
            brcfdr = open(dirfile, O_RDWR|O_CREAT, 0600);
            memset(brc, 0, BRC_CACHE_NUM*sizeof(struct _brc_cache_entry));
            write(brcfdr, brc, BRC_CACHE_NUM*sizeof(struct _brc_cache_entry));
            close(brcfdr);
        }
        brcfdr = open(dirfile, O_RDWR, 0600);
	if (brcfdr==-1) bbslog("3error","can't open %s errno %d",dirfile,errno);
        brc_cache_entry = mmap(NULL, BRC_CACHE_NUM*sizeof(struct _brc_cache_entry), PROT_READ|PROT_WRITE, MAP_SHARED, brcfdr, 0);
	if (brc_cache_entry==MAP_FAILED) bbslog("3error","can't mmap %s errno %d",dirfile,errno);
        close(brcfdr);
    }
}
#endif
int brc_initial(const char *userid, const char *boardname)
{                               /* 读取用户.boardrc文件，取出保存的当前版的brc_list */
    int entry;
    int i;
    char dirfile[MAXPATH];
    int bid;
    gzFile brcfile;
    const struct boardheader *bptr;
    int count;

    if (boardname == NULL)
        return 0;
    bid = getbnum(boardname);
    if (bid == 0)
        return 0;
    /*干脆不搞guest的这个算了*/
    if (!strcmp(userid,"guest")) return 0;
#if USE_TMPFS==1
    init_brc_cache(userid,false);
    if (brc_cache_entry==NULL) return 0;
    if (brc_cache_entry==MAP_FAILED) return 0;
#endif

    for (i = 0; i < BRC_CACHE_NUM; i++)
        if (brc_cache_entry[i].bid == bid) {
            brc_currcache = i;
            return 1;           /* cache 中有 */
        }

    sethomefile(dirfile, userid, BRCFILE);

    if ((brcfile = gzopen(dirfile, "rb6")) == NULL)
        if ((brcfile = gzopen(dirfile, "w+b6")) == NULL)
            return 0;

    entry = brc_getcache(userid);
    bptr = getboard(bid);
    bzero(&brc_cache_entry[entry].list, BRC_ITEMSIZE);
    gzseek(brcfile, (bid - 1) * BRC_ITEMSIZE, SEEK_SET);
    count = 0;
    while (count < BRC_ITEMSIZE) {
        int ret;

        ret = gzread(brcfile, (char *) (&brc_cache_entry[entry].list) + count, BRC_ITEMSIZE);
        if (ret == 0)
            break;
        count += ret;
    }
    /*
     * 先不加入版面的创建时间的判断
     * if (brc_cache_entry[entry].list[0])
     * &&(brc_cache_entry[entry].list[0]<bptr->createtime) )
     * {
     * brc_cache_entry[entry].changed=1;
     * brc_cache_entry[entry].list[0]=0;
     * } else 
     */
    {
        brc_cache_entry[entry].changed = 0;
        brc_cache_entry[entry].bid = bid;
    }
    brc_currcache = entry;
    gzclose(brcfile);
    return 1;
}


int brc_unread(unsigned int fid)
{
    int n;

    /*干脆不搞guest的这个算了*/
    if (!strcmp(currentuser->userid,"guest")) return 1;
    for (n = 0; n < BRC_MAXNUM; n++) {
        if (brc_cache_entry[brc_currcache].list[n] == 0) {
            if (n == 0)
                return 1;
            return 0;
        }
        if (fid > brc_cache_entry[brc_currcache].list[n]) {
            return 1;
        } else if (fid == brc_cache_entry[brc_currcache].list[n]) {
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

void brc_add_read(unsigned int fid)
{
    int n, i;

    if (!currentuser) return;
    if (brc_currcache==-1) return;
    /*干脆不搞guest的这个算了*/
    if (!strcmp(currentuser->userid,"guest")) return;
    for (n = 0; (n < BRC_MAXNUM) && brc_cache_entry[brc_currcache].list[n]; n++) {
        if (fid == brc_cache_entry[brc_currcache].list[n]) {
            return;
        } else if (fid > brc_cache_entry[brc_currcache].list[n]) {
            for (i = BRC_MAXNUM - 1; i > n; i--) {
                brc_cache_entry[brc_currcache].list[i] = brc_cache_entry[brc_currcache].list[i - 1];
            }
            brc_cache_entry[brc_currcache].list[n] = fid;
            brc_cache_entry[brc_currcache].changed = 1;
            return;
        }
    }
    /*
     * 这个地方加入是不对的，因为只可能有2情况，一个是根本没有unread记录，
     * * 或者所有list[n]的时间之前的文章都被认为已读
     * if (n!=BRC_MAXNUM) {
     * brc_cache_entry[brc_currcache].list[n]=ftime;
     * n++;
     * if (n!=BRC_MAXNUM)
     * brc_cache_entry[brc_currcache].list[n]=0;
     * brc_cache_entry[brc_currcache].changed = 1;
     * }
     * 应该用如下
     */
    if (n == 0) {
        brc_cache_entry[brc_currcache].list[0] = fid;
        brc_cache_entry[brc_currcache].list[1] = 1;
        brc_cache_entry[brc_currcache].list[2] = 0;
        brc_cache_entry[brc_currcache].changed = 1;
    }
}

void brc_clear()
{
    struct boardheader const *bh;
    /*干脆不搞guest的这个算了*/
    if (!strcmp(currentuser->userid,"guest")) return;
    bh = getboard(brc_cache_entry[brc_currcache].bid);
    brc_cache_entry[brc_currcache].list[0] = bh->nowid;
    brc_cache_entry[brc_currcache].list[1] = 0;
    brc_cache_entry[brc_currcache].changed = 1;
}

void brc_clear_new_flag(unsigned int fid)
{
    int n;
    /*干脆不搞guest的这个算了*/
    if (!strcmp(currentuser->userid,"guest")) return;

    for (n = 0; (n < BRC_MAXNUM) && brc_cache_entry[brc_currcache].list[n]; n++)
        if (fid >= brc_cache_entry[brc_currcache].list[n])
            break;
    if ((n < BRC_MAXNUM) && ((brc_cache_entry[brc_currcache].list[n] != 0) || (n == 0))) {
        brc_cache_entry[brc_currcache].list[n] = fid;
        if (n + 1 < BRC_MAXNUM)
            brc_cache_entry[brc_currcache].list[n + 1] = 0;
        brc_cache_entry[brc_currcache].changed = 1;
    }
    return;
}
#endif

int poststatboard(const char *currboard)
{                               /* 判断当前版是否统计十大 */
    const struct boardheader *bh = getbcache(currboard);

    if (bh && ! (bh->flag & BOARD_POSTSTAT))
        return true;
    else
        return false;
}

int junkboard(const char *currboard)
{                               /* 判断当前版是否为 junkboards */
    struct boardheader *bh = getbcache(currboard);

    if (bh && (bh->flag & BOARD_JUNK))  /* Checking if DIR access mode is "555" */
        return true;
    else
        return false;
/*    return seek_in_file("etc/junkboards",currboard);*/
}

int checkreadonly(const char *board)
{                               /* 检查是否是只读版面 */
    struct boardheader *bh = getbcache(board);

    if (bh && (bh->flag & BOARD_READONLY))      /* Checking if DIR access mode is "555" */
        return true;
    else
        return false;
}

int anonymousboard(const char *board)
{                               /*检查版面是不是匿名版 */
    struct boardheader *bh = getbcache(board);

    if (bh && (bh->flag & BOARD_ANNONY))        /* Checking if DIR access mode is "555" */
        return true;
    else
        return false;
}

int is_outgo_board(const char *board)
{
    struct boardheader *bh = getbcache(board);

    if (bh && (bh->flag & BOARD_OUTFLAG))
        return true;
    else
        return false;
}

int is_emailpost_board(const char *board)
{
    struct boardheader *bh = getbcache(board);

    if (bh && (bh->flag & BOARD_EMAILPOST))
        return true;
    else
        return false;
}

int deny_me(const char *user,const char *board)
{                               /* 判断用户 是否被禁止在当前版发文章 */
    char buf[STRLEN];

    setbfile(buf, board, "deny_users");
    return seek_in_file(buf, user);
}



int haspostperm(const struct userec *user,const char *bname)
{                               /* 判断在 bname版 是否有post权 */
    register int i;

    /*
     * if( strcmp( bname, DEFAULTBOARD ) == 0 )  return 1; change by KCN 2000.09.01 
     */
    if ((i = getbnum(bname)) == 0)
        return 0;
    if (bcache[i-1].flag&BOARD_GROUP) //目录先不能写
        return 0;

    if (!HAS_PERM(user, PERM_POST)) {
        if(!strcasecmp(user->userid, "guest"))
            return 0;
        if (!strcmp(bname, "BBShelp"))
            return 1;
        if (!HAS_PERM(user, PERM_LOGINOK))
            return 0;
        if (!strcmp(bname, "Complain"))
            return 1;           /* added by stephen 2000.10.27 */
        else if (!strcmp(bname, "sysop"))
            return 1;
        else if (!strcmp(bname, "Arbitration"))
            return 1;
        return 0;
    }                           /* stephen 2000.10.27 */
    if (HAS_PERM(user, (bcache[i - 1].level & ~PERM_NOZAP) & ~PERM_POSTMASK)) {
        if (bcache[i - 1].flag & BOARD_CLUB_WRITE) {    /*俱乐部 */
            if (bcache[i - 1].clubnum <= 0 || bcache[i - 1].clubnum >= MAXCLUB)
                return 0;
            if (user->club_write_rights[(bcache[i - 1].clubnum - 1) >> 5] & (1 << ((bcache[i - 1].clubnum - 1) & 0x1f)))
                return 1;
            else
                return 0;
        }
        return 1;
    } else
        return 0;
    if (bcache[i-1].title_level&&(bcache[i-1].title_level!=user->title)) return 0;
}

int chk_BM_instr(const char BMstr[STRLEN - 1], const char bmname[IDLEN + 2])
{
    char *ptr;
    char BMstrbuf[STRLEN - 1];

    strcpy(BMstrbuf, BMstr);
    ptr = strtok(BMstrbuf, ",: ;|&()\0\n");
    while (1) {
        if (ptr == NULL)
            return false;
        if (!strcmp(ptr, bmname /*,strlen(currentuser->userid) */ ))
            return true;
        ptr = strtok(NULL, ",: ;|&()\0\n");
    }
}


int chk_currBM(const char BMstr[STRLEN - 1], struct userec *user)
        /*
         * 根据输入的版主名单 判断user是否有版主 权限 
         */
{
    if (HAS_PERM(currentuser, PERM_OBOARDS) || HAS_PERM(currentuser, PERM_SYSOP))
        return true;

    if (!HAS_PERM(currentuser, PERM_BOARDS))
        return false;

    return chk_BM_instr(BMstr, currentuser->userid);
}

int deldeny(struct userec *user, char *board, char *uident, int notice_only)
{                               /* 删除 禁止POST用户 */
    char fn[STRLEN];
    FILE *fn1;
    char filename[STRLEN];
    char buffer[STRLEN];
    time_t now;
    struct userec *lookupuser;

    now = time(0);
    setbfile(fn, board, "deny_users");
    /*
     * Haohmaru.4.1.自动发信通知 
     */
    sprintf(filename, "etc/%s.dny", user->userid);
    fn1 = fopen(filename, "w");
    if (HAS_PERM(user, PERM_SYSOP) || HAS_PERM(user, PERM_OBOARDS)) {
        sprintf(buffer, "[通知]");
        fprintf(fn1, "寄信人: %s \n", user->userid);
        fprintf(fn1, "标  题: %s\n", buffer);
        fprintf(fn1, "发信站: %s (%24.24s)\n", "BBS " NAME_BBS_CHINESE "站", ctime(&now));
        fprintf(fn1, "来  源: %s \n", SHOW_USERIP(NULL, fromhost));
        fprintf(fn1, "\n");
        if (!strcmp(user->userid, "deliver"))
            fprintf(fn1, "您被自动解封系统解除在 %s 版的封禁\n", board);
        else
            fprintf(fn1, "您被站务人员 %s 解除在 %s 版的封禁\n", user->userid, board);
    } else {
        sprintf(buffer, "[通知]");
        fprintf(fn1, "寄信人: %s \n", user->userid);
        fprintf(fn1, "标  题: %s\n", buffer);
        fprintf(fn1, "发信站: %s (%24.24s)\n", "BBS " NAME_BBS_CHINESE "站", ctime(&now));
        fprintf(fn1, "来  源: %s \n", SHOW_USERIP(NULL, fromhost));
        fprintf(fn1, "\n");
        fprintf(fn1, "您被 %s 版版主 %s 解除封禁\n", board, user->userid);
    }
    fclose(fn1);

    /*
     * 解封同样发文到undenypost版  Bigman:2000.6.30 
     */
    getuser(uident, &lookupuser);
    if (lookupuser == NULL)
        sprintf(buffer, "%s 解封死掉的帐号 %s 在 %s ", user->userid, uident, board);
    else {
        if (PERM_BOARDS & lookupuser->userlevel)
            sprintf(buffer, "%s 解封某版版主 %s 在 %s ", user->userid, lookupuser->userid, board);
        else
            sprintf(buffer, "%s 解封 %s 在 %s", user->userid, lookupuser->userid, board);
        mail_file(user->userid, filename, uident, buffer, 0, NULL);
    }
    post_file(user, "", filename, "undenypost", buffer, 0, 1);
    unlink(filename);
    bmlog(user->userid, board, 11, 1);
    if (notice_only)
        return 1;
    else
        return del_from_file(fn, lookupuser ? lookupuser->userid : uident);
}

int normal_board(const char *bname)
{
    register int i;
    struct boardheader bh;
    int ret=1;

    if (strcmp(bname, DEFAULTBOARD) == 0)
        return 1;
    if ((i = getboardnum(bname,&bh)) == 0)
        return 0;

    while (ret) {
#ifdef NINE_BUILD
    ret=!(bh.level&PERM_SYSOP)&&!(bh.flag&BOARD_CLUB_HIDE)&&!(bh.flag&BOARD_CLUB_READ);
#else
    ret=((bh.level == 0)||(bh.level&PERM_POSTMASK))&&!(bh.flag&BOARD_CLUB_HIDE)&&!(bh.flag&BOARD_CLUB_READ);
#endif
    if (bh.title_level) ret=0;
    if (ret&&(bh.group)) {
        memcpy(&bh,getboard(bh.group),sizeof(struct boardheader));
        continue;
    }
    break;
    }
    return ret;
}

int fav_loaddata(struct newpostdata *nbrd, int favnow,int pos,int len,bool sort,const char** input_namelist)
{
//注意，如果是目录，nbrd的flag应该为-1
    int n;
    struct boardheader *bptr=NULL;
    int brdnum;
    struct newpostdata *ptr;
    int curcount;
    int* indexlist=NULL;
    const char** namelist=NULL;

    brdnum = 0;
    curcount=0;
    if (zapbuf == NULL) {
        load_zapbuf();
    }
    if (sort) {
    	if (input_namelist==NULL)
    	    namelist=(const char**)malloc(sizeof(char**)*(pos+len-1));
    	else
    	    namelist=input_namelist;
    	indexlist=(int*)malloc(sizeof(int*)*(pos+len-1));
    }
    for (n = 0; n < favbrd_list[favnow].bnum; n++) {
        if (favbrd_list[favnow].bid[n] >=0) {
            bptr = (struct boardheader *) getboard(favbrd_list[favnow].bid[n] + 1);
            if (!bptr)
                continue;
            if (!*bptr->filename)
                continue;
            if (!check_see_perm(currentuser,bptr))
                continue;
        }else{
			if (!HAS_PERM(currentuser,favbrd_list[0-favbrd_list[favnow].bid[n]].level))
				continue;
		}
        /*肯定要计算的版面*/
        brdnum++;
        if (!sort) {
	    if (input_namelist) {
            if (favbrd_list[favnow].bid[n] < 0) 
	            input_namelist[brdnum-1]=NullChar;
			else
	            input_namelist[brdnum-1]=bptr->filename;
            }
            if (brdnum<pos||brdnum>=pos+len)
            	continue;
            if (nbrd) {
                ptr = &nbrd[brdnum-pos];
                if (favbrd_list[favnow].bid[n] < 0) {
                    ptr->name = NullChar;
                    ptr->title = favbrd_list[0-favbrd_list[favnow].bid[n]].title;
                    ptr->dir = 1;
                    ptr->BM = NullChar;
                    ptr->flag = -1;
                    ptr->tag = 0-favbrd_list[favnow].bid[n];
                    ptr->pos = n;
                    ptr->total = favbrd_list[0-favbrd_list[favnow].bid[n]].bnum;
                    ptr->unread = 1;
                    ptr->zap = 0;
                } else {
                    ptr->name = bptr->filename;
                    ptr->dir = 0;
                    ptr->title = bptr->title;
                    ptr->BM = bptr->BM;
                    ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
                    ptr->tag = n;
                    ptr->pos = favbrd_list[favnow].bid[n];
		    if (bptr->flag&BOARD_GROUP)
                    ptr->total = bptr->board_data.group_total;
		    else
                    ptr->total = -1;
                    ptr->zap = (zapbuf[favbrd_list[favnow].bid[n]] == 0);
                }
            	}
        } else {  /*如果是要排序，那么应该先排序缓存一下*/
            int i;
            const char* title;
            int j;
            if (favbrd_list[favnow].bid[n] < 0)
            	title=NullChar;
            else
            	title=bptr->filename;
            for (i=0;i<curcount;i++) {
            	if (strcasecmp(namelist[i],title)>0) break;
            }
            if ((i==curcount)&&curcount>=pos+len-1) /*已经在范围之外乐*/
            	continue;
            else
            	   for (j=(curcount>=pos+len-1)?pos+len-2:curcount;j>i;j--) {
            			namelist[j]=namelist[j-1];
            			indexlist[j]=indexlist[j-1];
             	   }
            namelist[i]=title;
            indexlist[i]=n;
            if (curcount<pos+len-1) curcount++;
        }
    }
    if (brdnum == 0) {
    	if (nbrd) {
        ptr = &nbrd[brdnum++];
        ptr->name = NullChar;
        ptr->dir = 1;
        ptr->title = EmptyChar;
        ptr->BM = NullChar;
        ptr->tag = -1;
        ptr->flag = -1;
        ptr->pos = -1;
        ptr->total = 0;
        ptr->unread = 0;
        ptr->zap = 0;
    	}
    }
    else if (sort) {
        if (nbrd) {
            for (n=pos-1;n<curcount;n++) {
    	    ptr=&nbrd[n-(pos-1)];
                if (favbrd_list[favnow].bid[indexlist[n]] >= 0) {
                    bptr = (struct boardheader *) getboard(favbrd_list[favnow].bid[indexlist[n]] + 1);
                    ptr->name = bptr->filename;
                    ptr->dir = 0;
                    ptr->title = bptr->title;
                    ptr->BM = bptr->BM;
                    ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
                    ptr->tag = indexlist[n];
                    ptr->pos = favbrd_list[favnow].bid[indexlist[n]];
		    if (bptr->flag&BOARD_GROUP)
                    ptr->total = bptr->board_data.group_total;
		    else
                    ptr->total = -1;
                    ptr->zap = (zapbuf[favbrd_list[favnow].bid[indexlist[n]]] == 0);
                } else {
                    ptr->name = NullChar;
                    ptr->title = favbrd_list[0-favbrd_list[favnow].bid[indexlist[n]]].title;
                    ptr->dir = 1;
                    ptr->BM = NullChar;
                    ptr->flag = -1;
                    ptr->tag = 0-favbrd_list[favnow].bid[indexlist[n]];
                    ptr->pos = indexlist[n];
                    ptr->total = favbrd_list[0-favbrd_list[favnow].bid[indexlist[n]]].bnum;
                    ptr->unread = 1;
                    ptr->zap = 0;
                }
            }
        }
    }
    if (sort) {
    	if (input_namelist==NULL)
    	    free(namelist);
    	free(indexlist);
    }
    return brdnum;
}

int load_boards(struct newpostdata *nbrd,char *boardprefix,int group,int pos,int len,bool sort,bool yank_flag,const char** input_namelist)
{
    int n;
    const struct boardheader *bptr;
    int brdnum;
    struct newpostdata *ptr;
    int curcount;
    const char** namelist;
    const char** titlelist;
    int* indexlist;
	time_t tnow;

	tnow = time(0);
    brdnum = 0;
    curcount=0;
    if (zapbuf == NULL) {
        load_zapbuf();
    }
    if (input_namelist==NULL)
        namelist=(const char**)malloc(sizeof(char**)*(pos+len-1));
    else
    	namelist=input_namelist;
    titlelist=(const char**)malloc(sizeof(char**)*(pos+len-1));
    indexlist=(int*)malloc(sizeof(int*)*(pos+len-1));
    for (n = 0; n < get_boardcount(); n++) {
        bptr = (struct boardheader *) getboard(n + 1);
        if (!bptr)
            continue;
        if (*bptr->filename==0)
            continue;
		if ( group == -2 ){ //新版
			if( ( tnow - bptr->createtime ) > 86400*30 || ( bptr->flag & BOARD_GROUP ) )
				continue;
		}else if ((bptr->group!=group)&&!((boardprefix==NULL)&&(group==0)))
            continue;
        if (!check_see_perm(currentuser,bptr)) {
            continue;
        }
        if ((group==0)&&(boardprefix != NULL && strchr(boardprefix, bptr->title[0]) == NULL && boardprefix[0] != '*'))
            continue;
        if (yank_flag || zapbuf[n] != 0 || (bptr->level & PERM_NOZAP)) {
            int i;
            int j;
            brdnum++;
            /*都要排序*/
            for (i=0;i<curcount;i++) {
		    int type;
		    type = 0;

		    if (!sort) {
			type = titlelist[i][0] - bptr->title[0];
                        if (type == 0)
                            type = strncasecmp(&titlelist[i][1], bptr->title + 1, 6);
                    }
                    if (type == 0)
                        type = strcasecmp(namelist[i], bptr->filename);
		    if (type>0) break;
            }
            if ((i==curcount)&&curcount>=pos+len-1) /*已经在范围之外乐*/
                continue;
            else
                for (j=(curcount>=pos+len-1)?pos+len-2:curcount;j>i;j--) {
                    namelist[j]=namelist[j-1];
                    titlelist[j]=titlelist[j-1];
                    indexlist[j]=indexlist[j-1];
                }
            namelist[i]=bptr->filename;
            titlelist[i]=bptr->title;
            indexlist[i]=n;
            if (curcount<pos+len-1) curcount++;
        }
    }
    if (nbrd) {
        for (n=pos-1;n<curcount;n++) {
            ptr=&nbrd[n-(pos-1)];
            bptr = getboard(indexlist[n]+1);
            ptr->dir = bptr->flag&BOARD_GROUP?1:0;
            ptr->name = bptr->filename;
            ptr->title = bptr->title;
            ptr->BM = bptr->BM;
            ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
            ptr->pos = indexlist[n];
            if (bptr->flag&BOARD_GROUP) {
                ptr->total = bptr->board_data.group_total;
            } else ptr->total=-1;
            ptr->zap = (zapbuf[indexlist[n]] == 0);
        }
    }
    free(titlelist);
    if (input_namelist==NULL)
        free(namelist);
    free(indexlist);
    return brdnum;
}

