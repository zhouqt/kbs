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

struct favbrd_struct {
    int flag;
    char *title;
    int father;
};

static const char NullChar[] = "";
static const char EmptyChar[] = "空";

/* added by bad 2002-08-3	FavBoardDir */
struct favbrd_struct favbrd_list[FAVBOARDNUM];
int favbrd_list_t = -1, favnow = -1;

int *zapbuf;
int zapbuf_changed = 0;

void release_favboard()
{
    int i;

    for (i = 0; i < favbrd_list_t; i++)
        if (favbrd_list[i].flag == -1)
            free(favbrd_list[i].title);
}

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

void load_favboard(int dohelp)
{
    char fname[STRLEN];
    int fd, size, idx, i, j;

    sethomefile(fname, currentuser->userid, "favboard");
    favnow = -1;
    if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
        read(fd, &i, sizeof(int));
        if (i != 0x8080) {      /* We can consider the 0x8080 magic number as a 
                                 * * version identifier of favboard file. */
            /*
             * We handle old version here. 
             */
            favbrd_list_t = i;
            for (i = 0; i < favbrd_list_t; i++) {
                read(fd, &j, sizeof(int));
                favbrd_list[i].flag = j;
                favbrd_list[i].father = -1;
            }
        } else {
            /*
             * We handle new version here. 
             */
            read(fd, &favbrd_list_t, sizeof(int));
            for (i = 0; i < favbrd_list_t; i++) {
                read(fd, &j, sizeof(int));
                favbrd_list[i].flag = j;
                if (j == -1) {
                    char len;
                    read(fd, &len, sizeof(char));
                    favbrd_list[i].title = (char *) malloc(len);
                    read(fd, favbrd_list[i].title, len);
                }
                read(fd, &j, sizeof(int));
                favbrd_list[i].father = j;
            }
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
            favbrd_list[0].flag = 0;
            favbrd_list[0].father = -1;
        } else {
            favbrd_list_t = 1;      /*  favorate board count    */
            favbrd_list[0].flag = 0;
            favbrd_list[0].father = -1;
            while(!feof(fp)) {
                int k;
                if(fscanf(fp, "%s", bn)<1) break;
                k=getbnum(bn);
                if(k) {
                    favbrd_list[favbrd_list_t].flag = k-1;
                    favbrd_list[favbrd_list_t].father = -1;
                    favbrd_list_t++;
                }
            }
            fclose(fp);
        }
    } else {
        int num = favbrd_list_t;

        idx = -1;
        while (++idx < favbrd_list_t) {
            struct boardheader *bh;

            if (favbrd_list[idx].flag == -1)
                continue;
            fd = favbrd_list[idx].flag;
            bh = (struct boardheader *) getboard(fd + 1);
            if (fd >= 0 && fd <= get_boardcount() && (bh && bh->filename[0]
                                                      && (check_see_perm(currentuser,bh))
                )
                )
                continue;
            DelFavBoard(idx);   /*  error correction    */
        }
        if (num != favbrd_list_t)
            save_favboard();
    }
}

void save_favboard()
{
    int fd, i, j;
    char fname[MAXPATH];

    sethomefile(fname, currentuser->userid, "favboard");
    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
        i = 0x8080;
        write(fd, &i, sizeof(int));
        write(fd, &favbrd_list_t, sizeof(int));
        for (i = 0; i < favbrd_list_t; i++) {
            j = favbrd_list[i].flag;
            write(fd, &j, sizeof(int));
            if (j == -1) {
                char len = strlen(favbrd_list[i].title) + 1;
                write(fd, &len, sizeof(char));
                write(fd, favbrd_list[i].title, len);
            }
            j = favbrd_list[i].father;
            write(fd, &j, sizeof(int));
        }
        close(fd);
    }
}

int IsFavBoard(int idx)
{
    int i;

    for (i = 0; i < favbrd_list_t; i++)
        if (idx == favbrd_list[i].flag && favnow == favbrd_list[i].father)
            return i + 1;
    return 0;
}

int ExistFavBoard(int idx)
{
    int i;

    for (i = 0; i < favbrd_list_t; i++)
        if (idx == favbrd_list[i].flag)
            return i + 1;
    return 0;
}

int changeFavBoardDir(int i, char *s)
{
    if (i >= favbrd_list_t)
        return -1;
    if (favbrd_list[i].flag != -1)
        return -1;
    free(favbrd_list[i].title);
    favbrd_list[i].title = (char *) malloc(strlen(s) + 1);
    strcpy(favbrd_list[i].title, s);
}

int getfavnum()
{
    int i, count = 0;

    for (i = 0; i < favbrd_list_t; i++)
        if (favnow == favbrd_list[i].father)
            count++;
    return count;
}

void addFavBoard(int i)
{
    int llen;

    if (favbrd_list_t < FAVBOARDNUM) {
        favbrd_list[favbrd_list_t].flag = i;
        favbrd_list[favbrd_list_t].father = favnow;
        favbrd_list_t++;
    };
}

void addFavBoardDir(int i, char *s)
{
    int llen, j;

    if (favbrd_list_t < FAVBOARDNUM && strlen(s) <= 20) {
        for (j = 0; j < favbrd_list_t; j++)
            if ((favbrd_list[j].father == favnow) && (favbrd_list[j].flag == -1) && !strcmp(favbrd_list[j].title, s))
                return ;
        favbrd_list[favbrd_list_t].flag = -1;
        favbrd_list[favbrd_list_t].father = favnow;
        favbrd_list[favbrd_list_t].title = (char *) malloc(strlen(s) + 1);
        strcpy(favbrd_list[favbrd_list_t].title, s);
        favbrd_list_t++;
    };
}

int SetFav(int i)
{
    int j;

    j = favnow;
    favnow = i;
    return j;
}

int DelFavBoard(int i)
{
    int j;

    if (i >= favbrd_list_t)
        return favbrd_list_t;
    if (i < 0)
        return favbrd_list_t;
    if (favbrd_list[i].flag == -1)
        for (j = 0; j < favbrd_list_t; j++)
            if (favbrd_list[j].father == i) {
                DelFavBoard(j);
                if (j < i)
                    i--;
                j--;
            }
    favbrd_list_t--;
    if (favbrd_list[i].flag == -1)
        free(favbrd_list[i].title);
    for (j = i; j < favbrd_list_t; j++)
        favbrd_list[j] = favbrd_list[j + 1];
    for (j = 0; j < favbrd_list_t; j++)
        if (favbrd_list[j].father >= i)
            favbrd_list[j].father--;
    if (favnow >= i)
        favnow--;
    if (!favbrd_list_t) {
        favbrd_list_t = 1;      /*  favorite board count    */
        favbrd_list[0].flag = 0;        /*  default sysop board     */
    }
    return 0;
}

/* 注意,q现在是当前移动的位置
  返回favnow
*/
int MoveFavBoard(int p, int q, int fav_father)
{
    struct favbrd_struct temp;
    int i;

    int count;
    count=0;
    if (q!=0) {
        for (i=0;i<favbrd_list_t;i++) {
            if (favbrd_list[i].father==fav_father) {
            count++;
            if (count==q+1)
                break;
            }
        }
	if (i==favbrd_list_t)
	    return;
        q = i;
    }
    if (p == q)
        return;
    if (p < 0 || p >= favbrd_list_t)
        return;
    if (q < 0 || q >= favbrd_list_t)
        return;
    for (i = 0; i < favbrd_list_t; i++)
        if (favbrd_list[i].father == p)
            favbrd_list[i].father = q;
        else if (p > q && favbrd_list[i].father >= q && favbrd_list[i].father < p)
            favbrd_list[i].father++;
        else if (p < q && favbrd_list[i].father <= q && favbrd_list[i].father > p)
            favbrd_list[i].father--;
    if (p > q && favnow >= q && favnow < p)
        favnow++;
    if (p < q && favnow <= q && favnow > p)
        favnow--;
    temp = favbrd_list[p];
    if (p > q)
        for (i = p; i > q; i--)
            favbrd_list[i] = favbrd_list[i - 1];
    else
        for (i = p; i < q; i++)
            favbrd_list[i] = favbrd_list[i + 1];
    favbrd_list[q] = temp;
    return favnow;
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

void brc_update(char *userid)
{
    int i;
    gzFile fd = NULL;
    char dirfile[MAXPATH];
    unsigned int data[MAXBOARD][BRC_MAXNUM];
    size_t count;

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

static int brc_getcache(char *userid)
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
    size_t filesize;
    int fdr;
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
void init_brc_cache(char* userid,bool replace) {
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
	if (brc_cache_entry==-1) bbslog("3error","can't mmap %s errno %d",dirfile,errno);
        close(brcfdr);
    }
}
#endif
int brc_initial(char *userid, char *boardname)
{                               /* 读取用户.boardrc文件，取出保存的当前版的brc_list */
    int entry;
    int i;
    char dirfile[MAXPATH];
    int brc_size;
    int bid;
    gzFile brcfile;
    struct boardheader const *bptr;
    size_t count;

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
    if (brc_cache_entry==-1) return 0;
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
    struct boardheader *bh;
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

int junkboard(char *currboard)
{                               /* 判断当前版是否为 junkboards */
    struct boardheader *bh = getbcache(currboard);

    if (bh && (bh->flag & BOARD_JUNK))  /* Checking if DIR access mode is "555" */
        return true;
    else
        return false;
/*    return seek_in_file("etc/junkboards",currboard);*/
}

int checkreadonly(char *board)
{                               /* 检查是否是只读版面 */
    struct boardheader *bh = getbcache(board);

    if (bh && (bh->flag & BOARD_READONLY))      /* Checking if DIR access mode is "555" */
        return true;
    else
        return false;
}

int anonymousboard(char *board)
{                               /*检查版面是不是匿名版 */
    struct boardheader *bh = getbcache(board);

    if (bh && (bh->flag & BOARD_ANNONY))        /* Checking if DIR access mode is "555" */
        return true;
    else
        return false;
}

int is_outgo_board(char *board)
{
    struct boardheader *bh = getbcache(board);

    if (bh && (bh->flag & BOARD_OUTFLAG))
        return true;
    else
        return false;
}

int is_emailpost_board(char *board)
{
    struct boardheader *bh = getbcache(board);

    if (bh && (bh->flag & BOARD_EMAILPOST))
        return true;
    else
        return false;
}

int deny_me(char *user, char *board)
{                               /* 判断用户 是否被禁止在当前版发文章 */
    char buf[STRLEN];

    setbfile(buf, board, "deny_users");
    return seek_in_file(buf, user);
}



int haspostperm(struct userec *user, char *bname)
{                               /* 判断在 bname版 是否有post权 */
    register int i;

#ifdef BBSMAIN
    if (digestmode)
        return 0;
#endif
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
        fprintf(fn1, "来  源: %s \n", fromhost);
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
        fprintf(fn1, "来  源: %s \n", fromhost);
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

int normal_board(char *bname)
{
    register int i;
    struct boardheader bh;

    if (strcmp(bname, DEFAULTBOARD) == 0)
        return 1;
    if ((i = getboardnum(bname,&bh)) == 0)
        return 0;
#ifdef NINE_BUILD
    return !(bh.level&PERM_SYSOP)&&!(bh.flag&BOARD_CLUB_HIDE)&&!(bh.flag&BOARD_CLUB_READ);
#else
    return (bh.level == 0)&&!(bh.flag&BOARD_CLUB_HIDE)&&!(bh.flag&BOARD_CLUB_READ);
#endif
}

int fav_loaddata(struct newpostdata *nbrd, int favnow,int pos,int len,bool sort,char** input_namelist)
{
//注意，如果是目录，nbrd的flag应该为-1
    int n, k;
    struct boardheader *bptr;
    int brdnum;
    struct newpostdata *ptr;
    int curcount;
    int* indexlist;
    char** namelist;

    brdnum = 0;
    curcount=0;
    if (zapbuf == NULL) {
        load_zapbuf();
    }
    if (sort) {
    	if (input_namelist==NULL)
    	    namelist=(char**)malloc(sizeof(char**)*(pos+len-1));
    	else
    	    namelist=input_namelist;
    	indexlist=(int*)malloc(sizeof(int*)*(pos+len-1));
    }
    for (n = 0; n < favbrd_list_t; n++) {
        if (favbrd_list[n].father != favnow)
        	continue;
        if (favbrd_list[n].flag != -1) {
            bptr = (struct boardheader *) getboard(favbrd_list[n].flag + 1);
            if (!bptr)
                continue;
            if (!*bptr->filename)
                continue;
            if (!check_see_perm(currentuser,bptr))
                continue;
        }
        /*肯定要计算的版面*/
        brdnum++;
        if (!sort) {
	    if (input_namelist) {
                if (favbrd_list[n].flag == -1) 
	            input_namelist[brdnum-1]=NullChar;
		else
	            input_namelist[brdnum-1]=bptr->filename;
            }
            if (brdnum<pos||brdnum>=pos+len)
            	continue;
            if (nbrd) {
                ptr = &nbrd[brdnum-pos];
                if (favbrd_list[n].flag == -1) {
                    ptr->name = NullChar;
                    ptr->title = favbrd_list[n].title;
                    ptr->dir = 1;
                    ptr->BM = NullChar;
                    ptr->flag = -1;
                    ptr->tag = n;
                    ptr->pos = 0;
                    ptr->total = 0;
                    ptr->unread = 1;
                    for (k = 0; k < favbrd_list_t; k++)
                        if (favbrd_list[k].father == n)
                            ptr->total++;
                    ptr->zap = 0;
                } else {
                    ptr->name = bptr->filename;
                    ptr->dir = 0;
                    ptr->title = bptr->title;
                    ptr->BM = bptr->BM;
                    ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
                    ptr->tag = n;
                    ptr->pos = favbrd_list[n].flag;
		    if (bptr->flag&BOARD_GROUP)
                    ptr->total = bptr->board_data.group_total;
		    else
                    ptr->total = -1;
                    ptr->zap = (zapbuf[favbrd_list[n].flag] == 0);
                }
            	}
        } else {  /*如果是要排序，那么应该先排序缓存一下*/
            int i;
            char* title;
            int j;
            if (favbrd_list[n].flag == -1)
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
                if (favbrd_list[indexlist[n]].flag != -1) {
                    bptr = (struct boardheader *) getboard(favbrd_list[indexlist[n]].flag + 1);
                    ptr->name = bptr->filename;
                    ptr->dir = 0;
                    ptr->title = bptr->title;
                    ptr->BM = bptr->BM;
                    ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
                    ptr->tag = indexlist[n];
                    ptr->pos = favbrd_list[indexlist[n]].flag;
		    if (bptr->flag&BOARD_GROUP)
                    ptr->total = bptr->board_data.group_total;
		    else
                    ptr->total = -1;
                    ptr->zap = (zapbuf[favbrd_list[indexlist[n]].flag] == 0);
                } else {
                    ptr->name = NullChar;
                    ptr->title = favbrd_list[indexlist[n]].title;
                    ptr->dir = 1;
                    ptr->BM = NullChar;
                    ptr->flag = -1;
                    ptr->tag = indexlist[n];
                    ptr->pos = 0;
                    ptr->total = 0;
                    ptr->unread = 1;
                    for (k = 0; k < favbrd_list_t; k++)
                        if (favbrd_list[k].father == indexlist[n])
                            ptr->total++;
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

int load_boards(struct newpostdata *nbrd,char *boardprefix,int group,int pos,int len,bool sort,bool yank_flag,char** input_namelist)
{
    int n, k;
    struct boardheader *bptr;
    int brdnum;
    struct newpostdata *ptr;
    int curcount;
    char** namelist;
    char** titlelist;
    int* indexlist;
	time_t tnow;

	tnow = time(0);
    brdnum = 0;
    curcount=0;
    if (zapbuf == NULL) {
        load_zapbuf();
    }
    if (input_namelist==NULL)
        namelist=(char**)malloc(sizeof(char**)*(pos+len-1));
    else
    	namelist=input_namelist;
    titlelist=(char**)malloc(sizeof(char**)*(pos+len-1));
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
		}else if ((bptr->group!=group)&&(boardprefix!=NULL)&&(group!=0))
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
                    int ret;
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

