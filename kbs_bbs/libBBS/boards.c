/*
     版的一些操作
*/

#include "bbs.h"

#define BRC_MAXNUM      50
#define BRC_ITEMSIZE    (BRC_MAXNUM * sizeof( time_t ))
#define BRC_FILESIZE BRC_ITEMSIZE*MAXBOARD

#define BRC_CACHE_NUM 60        /* 未读标记cache 20个版 */

static struct _brc_cache_entry {
    int bid;
    time_t list[BRC_MAXNUM];
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

struct favbrd_struct {
    int flag;
    char *title;
    int father;
};

static const char DirChar[] = "[DIR]";
static const char EmptyChar[] = "空";

/* added by bad 2002-08-3	FavBoardDir */
struct favbrd_struct favbrd_list[FAVBOARDNUM];
int favbrd_list_t = -1, favnow = -1;

struct newpostdata *nbrd;       /*每个版的信息 */
int *zapbuf;
int zapbuf_changed = 0;
int brdnum, yank_flag = 0;

void release_favboard()
{
    int i;

    for (i = 0; i < favbrd_list_t; i++)
        if (favbrd_list[i].flag == -1)
            free(favbrd_list[i].title);
}

void load_favboard(int dohelp)
{
    char fname[STRLEN];
    int fd, size, idx, i, j;

    sethomefile(fname, currentuser->userid, "favboard");
    favnow = -1;
    if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
        read(fd, &i, sizeof(int));
        if (i != 0x8080) {
            favbrd_list_t = i;
            for (i = 0; i < favbrd_list_t; i++) {
                read(fd, &j, sizeof(int));
                favbrd_list[i].flag = j;
                favbrd_list[i].father = -1;
            }
        } else {
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
        favbrd_list_t = 1;      /*  favorate board count    */
        favbrd_list[0].flag = 0;
        favbrd_list[0].father = -1;
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
                                                      && ((bh->level & PERM_POSTMASK)
                                                          || HAS_PERM(currentuser, bh->level)
                                                          || (bh->level & PERM_NOZAP))
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
                return;
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

void MoveFavBoard(int p, int q)
{
    struct favbrd_struct temp;
    int i;

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

int load_boards(char *boardprefix)
{
    struct boardheader *bptr;
    struct newpostdata *ptr;
    int n, k;

    if (zapbuf == NULL) {
        load_zapbuf();
    }
    brdnum = 0;
    if (yank_flag == 2) {
        for (n = 0; n < favbrd_list_t; n++)
            if (favbrd_list[n].flag == -1 && favbrd_list[n].father == favnow) {
                ptr = &nbrd[brdnum++];
                ptr->name = DirChar;
                ptr->title = favbrd_list[n].title;
                ptr->BM = DirChar;
                ptr->flag = -1;
                ptr->pos = n;
                ptr->total = 0;
                ptr->unread = 1;
                for (k = 0; k < favbrd_list_t; k++)
                    if (favbrd_list[k].father == n)
                        ptr->total++;
                ptr->zap = 0;
            } else if (favbrd_list[n].father == favnow) {
                bptr = (struct boardheader *) getboard(favbrd_list[n].flag + 1);
                if (!bptr)
                    continue;
#ifndef _DEBUG_
                if (!*bptr->filename)
                    continue;
#endif /*_DEBUG_*/
                if (!(bptr->level & PERM_POSTMASK) && !HAS_PERM(currentuser, bptr->level) && !(bptr->level & PERM_NOZAP))
                    continue;
                if (boardprefix != NULL && strchr(boardprefix, bptr->title[0]) == NULL && boardprefix[0] != '*')
                    continue;
                if (boardprefix != NULL && boardprefix[0] == '*')
                    if (!strstr(bptr->title, "●") && !strstr(bptr->title, "⊙")
                        && bptr->title[0] != '*')
                        continue;
                if (boardprefix == NULL && bptr->title[0] == '*')
                    continue;

                ptr = &nbrd[brdnum++];
                ptr->name = bptr->filename;
                ptr->title = bptr->title;
                ptr->BM = bptr->BM;
                ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
                ptr->pos = favbrd_list[n].flag;
                ptr->total = -1;
                ptr->zap = (zapbuf[favbrd_list[n].flag] == 0);
            }
    } else
        for (n = 0; n < get_boardcount(); n++) {
            bptr = (struct boardheader *) getboard(n + 1);
            if (!bptr)
                continue;
#ifndef _DEBUG_
            if (!*bptr->filename)
                continue;
#endif /*_DEBUG_*/
            if (!(bptr->level & PERM_POSTMASK) && !HAS_PERM(currentuser, bptr->level) && !(bptr->level & PERM_NOZAP)) {
                continue;
            }
            if (boardprefix != NULL && strchr(boardprefix, bptr->title[0]) == NULL && boardprefix[0] != '*')
                continue;
            if (boardprefix != NULL && boardprefix[0] == '*') {
                if (!strstr(bptr->title, "●") && !strstr(bptr->title, "⊙")
                    && bptr->title[0] != '*')
                    continue;
            }
            if (boardprefix == NULL && bptr->title[0] == '*')
                continue;
        /*---	period	2000-09-11	4 FavBoard	---*/
            if ((1 == yank_flag || (!yank_flag && (zapbuf[n] != 0 || (bptr->level & PERM_NOZAP))))) {
                ptr = &nbrd[brdnum++];
                ptr->name = bptr->filename;
                ptr->title = bptr->title;
                ptr->BM = bptr->BM;
                ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
                ptr->pos = n;
                ptr->total = -1;
                ptr->zap = (zapbuf[n] == 0);
            }
        }
    if (yank_flag == 2 && brdnum == 0) {
        ptr = &nbrd[brdnum++];
        ptr->name = DirChar;
        ptr->title = EmptyChar;
        ptr->BM = DirChar;
        ptr->flag = -1;
        ptr->pos = -1;
        ptr->total = 0;
        ptr->unread = 0;
        ptr->zap = 0;
    }
    if (brdnum == 0 && !yank_flag && boardprefix == NULL) {
        brdnum = -1;
        yank_flag = 1;
        return -1;
    }
    return 0;
}

void brc_update(char *userid)
{
    int i;
    int fd = -1;
    char dirfile[MAXPATH];

    sethomefile(dirfile, userid, ".boardrc");
    for (i = 0; i < BRC_CACHE_NUM; i++) {
        if (brc_cache_entry[i].changed) {
            if (fd == -1) {
                if ((fd = open(dirfile, O_RDWR | O_CREAT, 0600)) == -1) {
                    bbslog("3user", "can't %s open to readwrite:%s", dirfile, strerror(errno));
                    return;
                };
            }
            lseek(fd, (brc_cache_entry[i].bid - 1) * BRC_ITEMSIZE, SEEK_SET);
            write(fd, &brc_cache_entry[i].list, BRC_ITEMSIZE);

        }
    }
    if (fd != -1)
        close(fd);
    return;
}


#define BRC_OLD_MAXSIZE     32768
#define BRC_OLD_MAXNUM      60
#define BRC_OLD_STRLEN      15
#define BRC_OLD_ITEMSIZE    (BRC_OLD_STRLEN + 1 + BRC_OLD_MAXNUM * sizeof( int ))

static int brc_convert_struct(char *dirfile, char *data, int size)
{                               /* 把旧的broardrc文件格式转换成新的 */
    struct brc_struct brc;
    char *ptr;
    int fd;

    ptr = data;
    bzero(&brc, sizeof(brc));
    while (ptr < &data[size] && (*ptr >= ' ' && *ptr <= 'z')) {
        int num;
        char *tmp;
        char boardname[18];
        int bid;

        strncpy(boardname, ptr, BRC_OLD_STRLEN);
        boardname[BRC_OLD_STRLEN] = 0;
        bid = getbnum(boardname);
        if (bid <= 0 || bid >= MAXBOARD)
            break;
        ptr += BRC_OLD_STRLEN;
        num = (*ptr++) & 0xff;
        tmp = ptr;
        ptr += sizeof(int) * num;
        if (num > BRC_OLD_MAXNUM) {
            num = BRC_OLD_MAXNUM;
        }
        memcpy(brc.list[bid - 1], tmp, num * sizeof(int));
    }
    if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0600)) != -1) {
        write(fd, &brc, sizeof(brc));
        close(fd);
    } else
        unlink(dirfile);

    return 0;
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

void brc_addreaddirectly(char *userid, int bnum, int posttime)
{
    char dirfile[MAXPATH];
    int *ptr, *list;
    size_t filesize; 
    int fdr;
    int i, n;

    sethomefile(dirfile, userid, ".boardrc");
    BBS_TRY {
        if (safe_mmapfile(dirfile, O_RDWR | O_CREAT, PROT_READ | PROT_WRITE, MAP_SHARED, (void **) &ptr, &filesize, &fdr) == 0)
            BBS_RETURN_VOID;
        ftruncate(fdr, BRC_FILESIZE);
        list = ptr + BRC_ITEMSIZE * (bnum - 1);
        for (n = 0; (n < BRC_MAXNUM) && list[n]; n++) {
            if (posttime == list[n]) {
                end_mmapfile((void *) ptr, filesize, fdr);
                BBS_RETURN_VOID;
            } else if (posttime > list[n]) {
                for (i = BRC_MAXNUM - 1; i > n; i--)
                    list[i] = list[i - 1];
                list[n] = posttime;
                end_mmapfile((void *) ptr, filesize, fdr);
                BBS_RETURN_VOID;
            }
        }
        if (n == 0) {
            for (n = 0; n < BRC_MAXNUM; n++)
                list[n] = posttime;
            list[n] = 0;
        }
    }
    BBS_CATCH {
    }
    BBS_END end_mmapfile((void *) ptr, filesize, fdr);
    return;
}

int brc_initial(char *userid, char *boardname)
{                               /* 读取用户.boardrc文件，取出保存的当前版的brc_list */
    int entry;
    int i;
    char dirfile[MAXPATH];
    int brc_size;
    int bid;
    int fd;
    struct boardheader const *bptr;

    if (boardname == NULL)
        return 0;
    bid = getbnum(boardname);
#ifdef BBSMAIN
    strncpy(currboard, boardname, STRLEN - BM_LEN);     /*很是ft,居然在这里置currboard */
    currboard[STRLEN - BM_LEN] = 0;
#endif
    for (i = 0; i < BRC_CACHE_NUM; i++)
        if (brc_cache_entry[i].bid == bid) {
            brc_currcache = i;
            return 1;           /* cache 中有 */
        }

    sethomefile(dirfile, userid, ".boardrc");
    while (1) {                 /*如果是老版的.boardrc，需要重新读一遍 */
        struct stat st;

        if ((fd = open(dirfile, O_RDONLY, 0600)) != -1) {
            fstat(fd, &st);
            brc_size = st.st_size;
        } else {
            brc_size = 0;
        }

        if ((brc_size) && (brc_size < BRC_FILESIZE)) {
            /*
             * 老版的boardrc,因为应该只需要转化一次，不考虑效率啦 
             */
            char brc_buffer[BRC_OLD_MAXSIZE];

            if (lseek(fd, 0, SEEK_SET) != -1) {
                brc_size = read(fd, brc_buffer, sizeof(brc_buffer));
                close(fd);
                brc_size = brc_convert_struct(dirfile, brc_buffer, brc_size);
            } else {
                brc_size = 0;
            }
        } else
            break;
    }

    entry = brc_getcache(userid);
    bptr = getboard(bid);
    lseek(fd, (bid - 1) * BRC_ITEMSIZE, SEEK_SET);
    read(fd, &brc_cache_entry[entry].list, BRC_ITEMSIZE);
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
    close(fd);
    return 1;
}


int brc_unread(int ftime)
{
    int n;

    for (n = 0; n < BRC_MAXNUM; n++) {
        if (brc_cache_entry[brc_currcache].list[n] == 0) {
            if (n == 0)
                return 1;
            return 0;
        }
        if (ftime > brc_cache_entry[brc_currcache].list[n]) {
            return 1;
        } else if (ftime == brc_cache_entry[brc_currcache].list[n]) {
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

void brc_add_read(char *filename)
{
    int ftime, n, i;

    ftime = atoi(&filename[2]);
    if (filename[0] != 'M' && filename[0] != 'G')
        return;

    for (n = 0; (n < BRC_MAXNUM) && brc_cache_entry[brc_currcache].list[n]; n++) {
        if (ftime == brc_cache_entry[brc_currcache].list[n]) {
            return;
        } else if (ftime > brc_cache_entry[brc_currcache].list[n]) {
            for (i = BRC_MAXNUM - 1; i > n; i--) {
                brc_cache_entry[brc_currcache].list[i] = brc_cache_entry[brc_currcache].list[i - 1];
            }
            brc_cache_entry[brc_currcache].list[n] = ftime;
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
        for (n = 0; n < BRC_MAXNUM - 1; n++)
            brc_cache_entry[brc_currcache].list[n] = ftime;
        brc_cache_entry[brc_currcache].list[n] = 0;
        brc_cache_entry[brc_currcache].changed = 1;
    }
}

void brc_clear()
{
    brc_cache_entry[brc_currcache].list[0] = time(0);
    brc_cache_entry[brc_currcache].list[1] = 0;
    brc_cache_entry[brc_currcache].changed = 1;
}

void brc_clear_new_flag(char *filename)
{
    int ftime, n;

    ftime = atoi(&filename[2]);
    if (filename[0] != 'M' && filename[0] != 'G')
        return;
    for (n = 0; (n < BRC_MAXNUM) && brc_cache_entry[brc_currcache].list[n]; n++)
        if (ftime >= brc_cache_entry[brc_currcache].list[n])
            break;
    if (n < BRC_MAXNUM) {
        brc_cache_entry[brc_currcache].list[n] = ftime;
        if (n + 1 < BRC_MAXNUM)
            brc_cache_entry[brc_currcache].list[n + 1] = 0;
        brc_cache_entry[brc_currcache].changed = 1;
    }
    return;
}

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
    if (!HAS_PERM(user, PERM_POST))
        /*
         * if(!strcmp(bname, "sysop"))
         * return 1; 
 *//*
 * Leeward 98.05.21 revised by stephen 2000.10.27 
 */
        /*
         * let user denied post right post at Complain 
         */
    {
        if (!strcmp(user->userid,"guest"))
	    return 0;
        if (!strcmp(bname, "Complain"))
            return 1;           /* added by stephen 2000.10.27 */
        else if (!strcmp(bname, "sysop"))
            return 1;
        else if (!strcmp(bname, "Arbitration"))
            return 1;
        return 0;
    }                           /* stephen 2000.10.27 */
    return (HAS_PERM(user, (bcache[i - 1].level & ~PERM_NOZAP) & ~PERM_POSTMASK));
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
        mail_file(user->userid, filename, uident, buffer, 0);
    }
    post_file(user, "", filename, "undenypost", buffer, 0, 1);
    unlink(filename);
    if (notice_only)
        return 1;
    else
        return del_from_file(fn, lookupuser ? lookupuser->userid : uident);
}

int check_read_perm(struct userec *user, int num)
{
    const struct boardheader *board = getboard(num);

    if (board->level & PERM_POSTMASK || HAS_PERM(user, board->level) || (board->level & PERM_NOZAP))
        return 1;
    return 0;
}
