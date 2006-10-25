/*
     版的一些操作
*/

#include "bbs.h"
#include <zlib.h>

static const char NullChar[] = "";
static const char EmptyChar[] = "空";

/* added by bad 2002-08-3	FavBoardDir */
/* stiger:
   操作的session->favbrd_list其实是一个指针
   当自己的时候指向session->mybrd_list
   当讨论区目录时指向bdirshm->allbrd_list

   favbrd_list_t同理
   */
#define favbrd_list_t (*session->favbrd_list_count)

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

/***20060110, stiger, 代码整理*********/
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

void save_favboard1(char *name,struct favbrd_struct *brdlist, int * brdlist_t);

void load_myboard1(struct userec *user, struct favbrd_struct *brdlist, int * brdlist_t, int force){
    char fname[STRLEN];
    int fd, sign, i, j, k;

	if(!force){
		if(*brdlist_t > 0) return;
	}

    sethomefile(fname, user->userid, "favboard");
	bzero(brdlist, sizeof(struct favbrd_struct)*FAVBOARDNUM);
    if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
        read(fd, &sign, sizeof(int));
        if (sign != 0x8080 && sign!=0x8081 ) {      /* We can consider the 0x8080 magic number as a 
                                 * * version identifier of favboard file. */
            /*
             * We handle old version here. 
             */
            *brdlist_t = 1;
            brdlist[0].father = -1;
            for (k=0; k<MAXBOARDPERDIR; k++) {
                if( read(fd, &j, sizeof(int)) <= 0)
					break;
				if(j<0) j=0;
                brdlist[0].bid[k] = j;
            }
			brdlist[0].bnum = k;
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

			*brdlist_t=1;
			brdlist[0].father=-1;
			for(i=0;i<k;i++){
				if( favbrd_list_tmp[i].flag == -1 ){
					favbrd_list_tmp[i].flag = 0 - *brdlist_t;
					strncpy(brdlist[*brdlist_t].title, favbrd_list_tmp[i].buf, 60);
					brdlist[*brdlist_t].title[60]=0;
					(*brdlist_t)++;
				}
			}
			for(i=0;i<k;i++){
				int newfather;
				if( favbrd_list_tmp[i].father <= -1 || favbrd_list_tmp[i].father >= k)
					newfather = 0;
				else
					newfather = 0-favbrd_list_tmp[favbrd_list_tmp[i].father].flag;
				if (newfather >= *brdlist_t) continue;	

				if( favbrd_list_tmp[i].flag < 0 ){
					if(newfather >= 0 && brdlist[newfather].bnum < MAXBOARDPERDIR
					   && ( -favbrd_list_tmp[i].flag < *brdlist_t )){
						brdlist[newfather].bid[brdlist[newfather].bnum]=favbrd_list_tmp[i].flag;
						brdlist[newfather].bnum++;
						brdlist[0-favbrd_list_tmp[i].flag].father = newfather;
					}
				}else{
					if(newfather >= 0 && brdlist[newfather].bnum < MAXBOARDPERDIR){
						brdlist[newfather].bid[brdlist[newfather].bnum]=favbrd_list_tmp[i].flag;
						brdlist[newfather].bnum++;
					}
				}
			}
		}else{
            read(fd, brdlist_t, sizeof(int));
			read(fd, brdlist, sizeof(struct favbrd_struct) * (*brdlist_t));
        }
        close(fd);
    }

    if ((*brdlist_t <= 0)) {
        char bn[40];
        FILE* fp=fopen("etc/initial_favboard", "r");
        if(!fp) {
            *brdlist_t = 1;      /*  favorate board count    */
            brdlist[0].bnum = 1;
            brdlist[0].bid[0] = 0;
			brdlist[0].father = -1;
        } else {
            *brdlist_t = 1;      /*  favorate board count    */
			brdlist[0].father = -1;
            while(!feof(fp)) {
                int k;
                const struct boardheader *bh;
                if(fscanf(fp, "%s", bn)<1) break;
                k=getbid(bn,&bh);
                if(k) {
                    if (!check_see_perm(user,bh)) continue;

					if(brdlist[0].bnum < MAXBOARDPERDIR){
						brdlist[0].bid[brdlist[0].bnum]=k-1;
						brdlist[0].bnum++;
					}
                }
            }
            fclose(fp);
        }
    //} else if(mode!=2 && mode!=3){
    } else {
        int change=0;
		struct boardheader *bh;

		for(i=0;i<*brdlist_t;i++){
			for(j=0;j<brdlist[i].bnum;j++){
				fd = brdlist[i].bid[j];
				if (fd < 0)
					continue;
				bh = (struct boardheader *) getboard(fd + 1);
				if (fd <= get_boardcount() && (bh && bh->filename[0] && (check_see_perm(user,bh)) ) )
				    continue;
				for(k=j;k<brdlist[i].bnum-1;k++){
					brdlist[i].bid[k]=brdlist[i].bid[k+1];
				}
				brdlist[i].bid[k]=0;
				brdlist[i].bnum--;
				j--;
				change=1;
			}
		}
		if(change)
            save_favboard1(fname, brdlist, brdlist_t);
    }
}

void load_myboard(session_t *session, int force){
	load_myboard1(session->currentuser, session->mybrd_list, &session->mybrd_list_t, force);
}

void load_favboard(int mode,session_t* session)
{
    session->favnow = 0;
	session->nowfavmode = mode;
	
	if(mode==2){
		session->favbrd_list=bdirshm->allbrd_list;
		session->favbrd_list_count = &bdirshm->allbrd_list_t;
		return;
	}else if(mode==3){
		session->favbrd_list=bdirshm->wwwbrd_list;
		session->favbrd_list_count = &bdirshm->wwwbrd_list_t;
		return;
	}else{
		session->favbrd_list=session->mybrd_list;
		session->favbrd_list_count=&session->mybrd_list_t;

		load_myboard(session, 0);
	}


	return ;
}

void save_favboard1(char *fname,struct favbrd_struct *brdlist, int * brdlist_t)
{
    int fd, i;

    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
        i = 0x8081;
        write(fd, &i, sizeof(int));
        write(fd, brdlist_t, sizeof(int));
        for (i = 0; i < *brdlist_t; i++) {
            write(fd, &(brdlist[i]), sizeof(struct favbrd_struct));
        }
        close(fd);
    }
}

void save_favboard(int mode,session_t* session)
{
    char fname[MAXPATH];

	if( (mode==2 || mode==3 ) && !HAS_PERM(session->currentuser,PERM_SYSOP))
		return;

	if(mode==2){
		sprintf(fname,"etc/board.dir");
		save_favboard1(fname, bdirshm->allbrd_list, &bdirshm->allbrd_list_t);
	} else if(mode==3) {
		sprintf(fname,"etc/wwwboard.dir");
		save_favboard1(fname, bdirshm->wwwbrd_list, &bdirshm->wwwbrd_list_t);
	} else {
		sethomefile(fname, session->currentuser->userid, "favboard");
		save_favboard1(fname, session->mybrd_list, &session->mybrd_list_t);
	}

}

int EnameInFav(char *ename,session_t* session)
{
    int i;

    for (i = 0; i < bdirshm->allbrd_list_t ; i++){
        if ( ! strcasecmp( bdirshm->allbrd_list[i].ename , ename) ){
			if( bdirshm->allbrd_list[i].level==0 || HAS_PERM(session->currentuser, bdirshm->allbrd_list[i].level) )
            	return i + 1;
			else
				return 0;
		}
	}
    return 0;
}

int IsFavBoard1(int idx,struct favbrd_struct *brdlist, int favnow)
{
    int i;

    for (i = 0; i < brdlist[favnow].bnum ; i++)
        if (idx == brdlist[favnow].bid[i])
            return i + 1;
    return 0;
}

int IsFavBoard(int idx,session_t* session, int favmode, int favnow)
{
    int fn;
	struct favbrd_struct *brdlist;

	if(favnow < 0 || favmode <= 0){
		fn = session->favnow;
		brdlist = session->favbrd_list;
	} else {
		fn = favnow;
		if(favmode == 2)
			brdlist = bdirshm->allbrd_list;
		else if(favmode == 3)
			brdlist = bdirshm->wwwbrd_list;
		else
			brdlist = session->mybrd_list;
	}

	return IsFavBoard1(idx, brdlist, fn);
}

/*

int IsMyFavBoard(int idx,session_t* session, int favnow)
{
	return IsFavBoard1(idx, session->mybrd_list, favnow);
}

int ExistFavBoard(int idx,session_t* session)
{
    int i;

    for (i = 0; i < session->favbrd_list[session->favnow].bnum; i++)
        if (idx == session->favbrd_list[session->favnow].bid[i])
            return i + 1;
    return 0;
}
*/

int changeFavBoardDirEname(int i, char *s,session_t* session)
{
    if (i >= favbrd_list_t)
        return -1;
    strncpy(session->favbrd_list[i].ename, s, 20);
	session->favbrd_list[i].ename[19]=0;
    return 0;
}

int changeFavBoardDir(int i, char *s,session_t* session)
{
    if (i >= favbrd_list_t)
        return -1;
    strncpy(session->favbrd_list[i].title, s, 60);
	session->favbrd_list[i].title[60]=0;
    return 0;
}

int getfavnum(session_t* session)
{
    return session->favbrd_list[session->favnow].bnum;
}

/* i是bid */
static void addFavBoard1(int i,struct favbrd_struct *brdlist, int favnow)
{
    if (brdlist[favnow].bnum < MAXBOARDPERDIR) {
        brdlist[favnow].bid[brdlist[favnow].bnum] = i;
		brdlist[favnow].bnum++;
    };
}

void addFavBoard(int i,session_t* session, int favmode, int favnow)
{
    int fn;
	struct favbrd_struct *brdlist;

	if(favnow < 0 || favmode <= 0){
		fn = session->favnow;
		brdlist = session->favbrd_list;
	} else {
		fn = favnow;
		if(favmode == 2)
			brdlist = bdirshm->allbrd_list;
		else if(favmode == 3)
			brdlist = bdirshm->wwwbrd_list;
		else
			brdlist = session->mybrd_list;
	}

	addFavBoard1(i, brdlist, fn);

	return;
}

/*
void addMyFavBoard(int idx,session_t* session, int favnow)
{
	return addFavBoard1(idx, session->mybrd_list, favnow);
}
*/

void addFavBoardDir(char *s,session_t* session)
{
    if (session->favbrd_list[session->favnow].bnum < MAXBOARDPERDIR && favbrd_list_t < FAVBOARDNUM) {
        session->favbrd_list[favbrd_list_t].level = 0;
        session->favbrd_list[favbrd_list_t].bnum = 0;
        session->favbrd_list[favbrd_list_t].father = session->favnow;
        strncpy(session->favbrd_list[favbrd_list_t].title, s, 80);
		session->favbrd_list[favbrd_list_t].title[80]=0;
		session->favbrd_list[session->favnow].bid[session->favbrd_list[session->favnow].bnum]=0-favbrd_list_t;
        favbrd_list_t++;
		session->favbrd_list[session->favnow].bnum++;
    };
}

int SetFav(int i,session_t* session)
{
    int j;

    j = session->favnow;
    session->favnow = i;
    return j;
}

/* i是bid */
int DelFavBoard(int i,session_t* session)
{
    int j,k;

	for(j=0;j<session->favbrd_list[session->favnow].bnum;j++){
		if(i==session->favbrd_list[session->favnow].bid[j]){
			for(k=j; k<session->favbrd_list[session->favnow].bnum-1; k++){
				session->favbrd_list[session->favnow].bid[k] = session->favbrd_list[session->favnow].bid[k+1];
			}
			session->favbrd_list[session->favnow].bid[k] = 0;
			session->favbrd_list[session->favnow].bnum --;
		}
	}
	return 0;
}

char * FavGetTitle(int select,char *title,session_t* session)
{
	title[0]=0;
	if(select < 0 || select >= favbrd_list_t)
		return NULL;
	strcpy(title,session->favbrd_list[select].title);
		return title;
}

int FavGetFather(int select,session_t* session)
{
	if(select < 0 || select >= favbrd_list_t)
		return 0;
	return session->favbrd_list[select].father;
}

/* 删除 session->favbrd_list[father].bid[i] */
int DelFavBoardDir(int i,int fath,session_t* session)
{
    int j,k;
	int father=fath;
	int n;

    if (i >= session->favbrd_list[father].bnum)
        return favbrd_list_t;
    if (i < 0)
        return favbrd_list_t;
	//j是要删除的目录序号
	j=0-session->favbrd_list[father].bid[i];
    for (k = 0; k < session->favbrd_list[j].bnum; k++){
		//如果有子目录
        if (session->favbrd_list[j].bid[k] < 0) {
			//得到子目录的节点号
			n=0-session->favbrd_list[j].bid[k];
			//删除子目录
            DelFavBoardDir(k,j,session);
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
        session->favbrd_list[k] = session->favbrd_list[k + 1];
	bzero(&session->favbrd_list[k], sizeof(struct favbrd_struct));
	//如果当前的father在j之后，那么father要前移一个
	if (father >= j)
		father--;
	//吧所有的father在j之后的目录的father--
    for (k = 0; k < favbrd_list_t; k++)
        if (session->favbrd_list[k].father >= j)
            session->favbrd_list[k].father--;
	//吧所有指向j之后的bid提前一个
	for (k=0; k<favbrd_list_t;k++){
		for (n=0; n<session->favbrd_list[k].bnum; n++){
			if( 0-session->favbrd_list[k].bid[n] > j )
				session->favbrd_list[k].bid[n]++;
		}
	}
	//处理当前father节点的索引
	session->favbrd_list[father].bnum--;
	for (k=i;k<session->favbrd_list[father].bnum;k++)
		session->favbrd_list[father].bid[k] = session->favbrd_list[father].bid[k+1];
	session->favbrd_list[father].bid[k]=0;
	//处理session->favnow
    if (session->favnow >= j)
        session->favnow--;
    return 0;
}

/*把版面p得位置得移动到位置q, p,q start from 0 */
int MoveFavBoard(int p, int q,session_t* session)
{
    int k;
	int i;

	if(p<0 || q<0 || p>=session->favbrd_list[session->favnow].bnum || q>=session->favbrd_list[session->favnow].bnum)
		return -1;
	if(p == q)
		return 0;
	i=session->favbrd_list[session->favnow].bid[p];
	if( p > q ){
		for(k=p; k>q; k--)
			session->favbrd_list[session->favnow].bid[k] = session->favbrd_list[session->favnow].bid[k-1];
		session->favbrd_list[session->favnow].bid[k] = i;
	}else{
		for(k=p; k<q; k++)
			session->favbrd_list[session->favnow].bid[k] = session->favbrd_list[session->favnow].bid[k+1];
		session->favbrd_list[session->favnow].bid[k] = i;
	}
	return session->favnow;
}

/*---   ---*/
void load_zapbuf(session_t* session)
{                               /* 装入zap信息 */
    char fname[STRLEN];
    int fd, size, n;

    size = MAXBOARD * sizeof(int);
    session->zapbuf = (int *) malloc(size);
    for (n = 0; n < MAXBOARD; n++)
        session->zapbuf[n] = 1;
    sethomefile(fname, session->currentuser->userid, ".lastread");       /*user的.lastread， zap信息 */
    if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
        size = get_boardcount() * sizeof(int);
        read(fd, session->zapbuf, size);
        close(fd);
    }
    session->zapbuf_changed = 0;
}

/*---	Modified for FavBoard functions, by period	2000-09-11 */
void save_userfile(char *fname, int numblk, char *buf,session_t* session)
{
    char fbuf[256];
    int fd, size;

    sethomefile(fbuf, session->currentuser->userid, fname);
    if ((fd = open(fbuf, O_WRONLY | O_CREAT, 0600)) != -1) {
        size = numblk * sizeof(int);
        write(fd, buf, size);
        close(fd);
    }
}

void save_zapbuf(session_t* session)
{
    if (session->zapbuf_changed != 0)
        save_userfile(".lastread", get_boardcount(), (char *) session->zapbuf,session);
}

#if 0
void save_session->zapbuf()
{                               /*保存Zap信息 */
    char fname[STRLEN];
    int fd, size;

    sethomefile(fname, session->currentuser->userid, ".lastread");
    if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1) {
        size = numboards * sizeof(int);
        write(fd, session->zapbuf, size);
        close(fd);
    }
}
#endif

#ifdef HAVE_BRC_CONTROL

void brc_update(const char *userid,session_t* session)
{
    int i;
    gzFile fd = NULL;
    char dirfile[MAXPATH], dirfile_tmp[MAXPATH];
    unsigned int data[MAXBOARD][BRC_MAXNUM];
    int count;

    if (session->brc_cache_entry==NULL) return;
    /*干脆不搞guest的这个算了*/
    if (!strcmp(userid,"guest")) return;
    sethomefile(dirfile, userid, BRCFILE);
    sethomefile(dirfile_tmp, userid, BRCFILE ".tmp");
    for (i = 0; i < BRC_CACHE_NUM; i++) {
        if (session->brc_cache_entry[i].changed) {
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
        bbslog("3error", "can't %s open to read:%s", dirfile, errstr);
//      f_rm(dirfile);
        return;
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

    if ((fd = gzopen(dirfile_tmp, "w+b6")) == NULL) {
        const char *errstr;
        int gzerrno;

        errstr = gzerror(fd, &gzerrno);
        if (errno == Z_ERRNO)
            errstr = strerror(errno);
        bbslog("3error", "can't %s open to write:%s", dirfile_tmp, errstr);
        f_rm(dirfile_tmp);
        return;
    }

    for (i = 0; i < BRC_CACHE_NUM; i++) {
        if (session->brc_cache_entry[i].changed) {
            memcpy(data[session->brc_cache_entry[i].bid - 1], &session->brc_cache_entry[i].list, BRC_ITEMSIZE);
            session->brc_cache_entry[i].changed = 0;
        }
    }
    count = 0;
    while (count < BRC_FILESIZE) {
        int ret;

        ret = gzwrite(fd, (char *) (&data) + count, BRC_FILESIZE - count);
        if (ret <= 0)
            break;
        count += ret;
    }
    gzclose(fd);
    if (count == BRC_FILESIZE) {
        f_mv(dirfile_tmp, dirfile);
    }
    return;
}

static int brc_getcache(const char *userid,session_t* session)
{
    int i, unchange = -1;

    for (i = 0; i < BRC_CACHE_NUM; i++) {
        if (session->brc_cache_entry[i].bid == 0)
            return i;
        if (session->brc_cache_entry[i].changed == 0)
            unchange = i;
    }
    if (unchange != -1)
        return unchange;
    brc_update(userid,session);

    return 0;
}

#if 0 //atppp 20060106
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
#endif

void free_brc_cache(char *userid,session_t* session){

	if( strcmp( userid ,"guest") ){
        if (session->brc_cache_entry)
            munmap((void *)session->brc_cache_entry,BRC_CACHE_NUM*sizeof(struct _brc_cache_entry));
    }
}

void init_brc_cache(const char* userid,bool replace,session_t* session) {
    if ((session->brc_cache_entry==NULL)||(replace)) {
        char dirfile[MAXPATH];
        int brcfdr;
        struct stat st;
        if (session->brc_cache_entry)
            munmap((void *)session->brc_cache_entry,BRC_CACHE_NUM*sizeof(struct _brc_cache_entry));
        setcachehomefile(dirfile, userid, -1, NULL);
        mkdir(dirfile, 0700);
        setcachehomefile(dirfile, userid, -1, "entry");
        if(stat(dirfile, &st)<0) {
            char brc[BRC_CACHE_NUM*sizeof(struct _brc_cache_entry)];
            brcfdr = open(dirfile, O_RDWR|O_CREAT, 0600);
            memset(brc, 0, BRC_CACHE_NUM*sizeof(struct _brc_cache_entry));
            write(brcfdr, brc, BRC_CACHE_NUM*sizeof(struct _brc_cache_entry));
            close(brcfdr);
        }
        brcfdr = open(dirfile, O_RDWR, 0600);
        if (brcfdr==-1) bbslog("3error","can't open %s errno %d",dirfile,errno);
        session->brc_cache_entry = (struct _brc_cache_entry *) mmap(NULL, BRC_CACHE_NUM*sizeof(struct _brc_cache_entry), PROT_READ|PROT_WRITE, MAP_SHARED, brcfdr, 0);
        if (session->brc_cache_entry==MAP_FAILED) {
            bbslog("3error","can't mmap %s errno %d",dirfile,errno);
            session->brc_cache_entry = NULL; //added by atppp 20040724
        }
        session->brc_currcache = -1; //added by atppp 20040719
        close(brcfdr);
    }
}
int brc_initial(const char *userid, const char *boardname,session_t* session)
{                               /* 读取用户.boardrc文件，取出保存的当前版的brc_list */
    int entry;
    int i;
    char dirfile[MAXPATH];
    int bid;
    gzFile brcfile;
    const struct boardheader *bptr;
    int count;

    /*干脆不搞guest的这个算了*/
    if (!strcmp(userid,"guest")) return 0;

    if (boardname == NULL)
        return 0;
    bid = getbid(boardname,NULL);
    if (bid == 0)
        return 0;

    init_brc_cache(userid,false,session);
    if (session->brc_cache_entry==NULL) return 0;
    // if (session->brc_cache_entry==MAP_FAILED) return 0; //removed by atppp 20040724

    for (i = 0; i < BRC_CACHE_NUM; i++)
        if (session->brc_cache_entry[i].bid == bid) {
            session->brc_currcache = i;
            return 1;           /* cache 中有 */
        }

    sethomefile(dirfile, userid, BRCFILE);

    if ((brcfile = gzopen(dirfile, "rb6")) == NULL)
        if ((brcfile = gzopen(dirfile, "w+b6")) == NULL)
            return 0;

    entry = brc_getcache(userid,session);
    bptr = getboard(bid);
    bzero(&session->brc_cache_entry[entry].list, BRC_ITEMSIZE);
    gzseek(brcfile, (bid - 1) * BRC_ITEMSIZE, SEEK_SET);
    count = 0;
    while (count < BRC_ITEMSIZE) {
        int ret;

        ret = gzread(brcfile, (char *) (&session->brc_cache_entry[entry].list) + count, BRC_ITEMSIZE);
        if (ret == 0)
            break;
        count += ret;
    }
    /*
     * 先不加入版面的创建时间的判断
     * if (session->brc_cache_entry[entry].list[0])
     * &&(session->brc_cache_entry[entry].list[0]<bptr->createtime) )
     * {
     * session->brc_cache_entry[entry].changed=1;
     * session->brc_cache_entry[entry].list[0]=0;
     * } else 
     */
    {
        session->brc_cache_entry[entry].changed = 0;
        session->brc_cache_entry[entry].bid = bid;
    }
    session->brc_currcache = entry;
    gzclose(brcfile);
    return 1;
}

inline static int valid_brc(int bid, session_t* session) {
    if (!session->currentuser) return 0;
    if (session->brc_currcache==-1) return 0;
    if (session->brc_cache_entry==NULL) return 0;
    /*干脆不搞guest的这个算了*/
    if (!strcmp(session->currentuser->userid,"guest")) return 0;
    if (bid && (session->brc_cache_entry[session->brc_currcache].bid != bid)) return 0;
    return 1;
}


/* 这个函数没有写入操作，所以不传入 bid 的话，即使出错也不会有危害 */
int brc_unread(unsigned int fid,session_t* session)
{
    int n;
    unsigned int thisid;

    if (!valid_brc(0, session)) return 1;
    for (n = 0; n < BRC_MAXNUM; n++) {
        thisid = session->brc_cache_entry[session->brc_currcache].list[n];
        if (thisid == 0) {
            if (n == 0)
                return 1;
            return 0;
        }
        if (fid > thisid) {
            return 1;
        } else if (fid == thisid) {
            return 0;
        }
    }
    return 0;
}

int brc_board_unread(int bid, session_t* session)
{
    int n;
    unsigned int thisid, fid, nowid;
    struct BoardStatus const *bs;

    if (!valid_brc(bid, session)) return 1;

    bs = getbstatus(bid);
    fid = bs->lastpost;
    nowid = bs->nowid;
    for (n = 0; n < BRC_MAXNUM; n++) {
        thisid = session->brc_cache_entry[session->brc_currcache].list[n];
        if (thisid == 0) {
            if (n == 0)
                return 1;
            return 0;
        }
        if (thisid > nowid) {
            brc_clear(bid, session);
            return 0;
        }
        if (fid > thisid) {
            return 1;
        } else if (fid == thisid) {
            return 0;
        }
    }
    return 0;
}


void brc_add_read(unsigned int fid, int bid, session_t* session)
{
    int n, i;

    if (!valid_brc(bid, session)) return;
    for (n = 0; (n < BRC_MAXNUM) && session->brc_cache_entry[session->brc_currcache].list[n]; n++) {
        if (fid == session->brc_cache_entry[session->brc_currcache].list[n]) {
            return;
        } else if (fid > session->brc_cache_entry[session->brc_currcache].list[n]) {
            for (i = BRC_MAXNUM - 1; i > n; i--) {
                session->brc_cache_entry[session->brc_currcache].list[i] = session->brc_cache_entry[session->brc_currcache].list[i - 1];
            }
            session->brc_cache_entry[session->brc_currcache].list[n] = fid;
            session->brc_cache_entry[session->brc_currcache].changed = 1;
            return;
        }
    }
    /*
     * 这个地方加入是不对的，因为只可能有2情况，一个是根本没有unread记录，
     * * 或者所有list[n]的时间之前的文章都被认为已读
     * if (n!=BRC_MAXNUM) {
     * session->brc_cache_entry[session->brc_currcache].list[n]=ftime;
     * n++;
     * if (n!=BRC_MAXNUM)
     * session->brc_cache_entry[session->brc_currcache].list[n]=0;
     * session->brc_cache_entry[session->brc_currcache].changed = 1;
     * }
     * 应该用如下
     */
    if (n == 0) {
        session->brc_cache_entry[session->brc_currcache].list[0] = fid;
        session->brc_cache_entry[session->brc_currcache].list[1] = 1;
        session->brc_cache_entry[session->brc_currcache].list[2] = 0;
        session->brc_cache_entry[session->brc_currcache].changed = 1;
    }
}

void brc_clear(int bid, session_t* session)
{
    struct BoardStatus const *bs;

    if (!valid_brc(bid, session)) return;

    bs=getbstatus(session->brc_cache_entry[session->brc_currcache].bid);
    session->brc_cache_entry[session->brc_currcache].list[0] = bs->nowid;
    session->brc_cache_entry[session->brc_currcache].list[1] = 0;
    session->brc_cache_entry[session->brc_currcache].changed = 1;
}

void brc_clear_new_flag(unsigned int fid, int bid, session_t* session)
{
    int n;

    if (!valid_brc(bid, session)) return;

    for (n = 0; (n < BRC_MAXNUM) && session->brc_cache_entry[session->brc_currcache].list[n]; n++)
        if (fid >= session->brc_cache_entry[session->brc_currcache].list[n])
            break;
    if ((n < BRC_MAXNUM) && ((session->brc_cache_entry[session->brc_currcache].list[n] != 0) || (n == 0))) {
        session->brc_cache_entry[session->brc_currcache].list[n] = fid;
        if (n + 1 < BRC_MAXNUM)
            session->brc_cache_entry[session->brc_currcache].list[n + 1] = 0;
        session->brc_cache_entry[session->brc_currcache].changed = 1;
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
    const struct boardheader *bh = getbcache(currboard);

    if (bh && (bh->flag & BOARD_JUNK))  /* Checking if DIR access mode is "555" */
        return true;
    else
        return false;
/*    return seek_in_file("etc/junkboards",currboard);*/
}

int checkreadonly(const char *board)
{                               /* 检查是否是只读版面 */
    const struct boardheader *bh = getbcache(board);

    if (bh && (bh->flag & BOARD_READONLY))      /* Checking if DIR access mode is "555" */
        return true;
    else
        return false;
}

int anonymousboard(const char *board)
{                               /*检查版面是不是匿名版 */
    const struct boardheader *bh = getbcache(board);

    if (bh && (bh->flag & BOARD_ANNONY))        /* Checking if DIR access mode is "555" */
        return true;
    else
        return false;
}

int is_outgo_board(const char *board)
{
    const struct boardheader *bh = getbcache(board);

    if (bh && (bh->flag & BOARD_OUTFLAG))
        return true;
    else
        return false;
}

int is_emailpost_board(const char *board)
{
    const struct boardheader *bh = getbcache(board);

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
    const struct boardheader *bh;

    /*
     * if( strcmp( bname, DEFAULTBOARD ) == 0 )  return 1; change by KCN 2000.09.01 
     */
    if ((i = getbid(bname, &bh)) == 0)
        return 0;
    if (!check_read_perm(user, bh))
        return 0;
    if (bh->flag&BOARD_GROUP) //目录先不能写
        return 0;

    if (!HAS_PERM(user, PERM_POST)) {
        if(!strcasecmp(user->userid, "guest"))
            return 0;
#ifndef FREE
        if (!strcmp(bname, "BBShelp"))
#else
        if (!strcmp(bname, "sysop"))
#endif
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
    if(HAS_PERM(user,(bh->level&~(PERM_NOZAP|PERM_POSTMASK)))){
        if(bh->flag&BOARD_CLUB_WRITE){
            if(!HAS_PERM(user,PERM_SYSOP)){
                if(!(bh->clubnum>0)||bh->clubnum>MAXCLUB)
                    return 0;
                if(!(user->club_write_rights[(bh->clubnum-1)>>5]&(1<<((bh->clubnum-1)&0x1f))))
                    return 0;
            }
        }
        return 1;
    }
    return 0;
    //if (bcache[i-1].title_level&&(bcache[i-1].title_level!=user->title)) return 0;
}

#ifdef NEWSMTH
int check_score_level(const struct userec *user,const struct boardheader *bh){
    return (chk_currBM(bh->BM,user)||!(user->score_user<bh->score_level));
}
#endif /* NEWSMTH */

int chk_BM_instr(const char BMstr[STRLEN - 1], const char bmname[IDLEN + 2])
{
    char *ptr;
    char BMstrbuf[STRLEN - 1];

    strcpy(BMstrbuf, BMstr);
    ptr = strtok(BMstrbuf, ",: ;|&()\0\n");
    while (1) {
        if (ptr == NULL)
            return false;
        if (!strcmp(ptr, bmname /*,strlen(session->getCurrentUser()->userid) */ ))
            return true;
        ptr = strtok(NULL, ",: ;|&()\0\n");
    }
}


int chk_currBM(const char BMstr[STRLEN - 1], const struct userec *user)
        /*
         * 根据输入的版主名单 判断user是否有版主 权限 
         */
{
    if (HAS_PERM(user, PERM_OBOARDS) || HAS_PERM(user, PERM_SYSOP))
        return true;

    if (!HAS_PERM(user, PERM_BOARDS))
        return false;

    return chk_BM_instr(BMstr, user->userid);
}


/*stephen : check whether current useris in the list of "jury" 2001.11.1*/
static int isJury(const struct userec *user, const struct boardheader *board)
{
    char buf[STRLEN];

    if (!HAS_PERM(user, PERM_JURY))
        return 0;
    makevdir(board->filename);
    setvfile(buf, board->filename, "jury");
    return seek_in_file(buf, user->userid);
}


/* etnlegend, 2005.11.27, 判断某一特定用户是否可以阅读某一特定版面的回收站 */
static int check_board_delete_read_perm_core(const struct userec *user,const struct boardheader *board){
    struct stat st;
    struct flock lc;
    char buf[256];
    int bid,fd,ret;
    void *p;

    if(!user||!board)
        return 0;
    if(!HAS_PERM(user,PERM_BOARDS)||!user->title)
        return 0;
    sethomefile(buf,user->userid,"board_delete_read");
    if(stat(buf,&st)||!S_ISREG(st.st_mode))
        return 0;
    if(st.st_mtime<board->createtime)
        return 0;
    if(!(bid=getbid(board->filename,NULL)))
        return 0;
    bid--;
    if(!((bid>>3)<st.st_size))
        return 0;
    if((fd=open(buf,O_RDONLY,0644))==-1)
        return 0;
    lc.l_type=F_RDLCK;
    lc.l_whence=SEEK_SET;
    lc.l_start=0;
    lc.l_len=0;
    lc.l_pid=0;
    if(fcntl(fd,F_SETLK,&lc)!=-1){
        if((p=mmap(NULL,st.st_size,PROT_READ,MAP_SHARED,fd,0))==MAP_FAILED)
            ret=0;
        else{
            ret=(((unsigned char*)p)[bid>>3]&(1<<(bid&0x07)));
            munmap(p,st.st_size);
        }
        lc.l_type=F_UNLCK;
        lc.l_whence=SEEK_SET;
        lc.l_start=0;
        lc.l_len=0;
        lc.l_pid=0;
        fcntl(fd,F_SETLKW,&lc);
    }
    else
        ret=0;
    close(fd);
    return ret;
}

int check_board_delete_read_perm(const struct userec *user,const struct boardheader *board, int jury){
    return (chk_currBM(board->BM, user) || check_board_delete_read_perm_core(user,board)
         || (jury && isJury(user, board)));
}


int deldeny(struct userec *user, char *board, char *uident, int notice_only,session_t* session)
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
    gettmpfilename(filename, "deny");
    fn1 = fopen(filename, "w");
    if (HAS_PERM(user, PERM_SYSOP) || HAS_PERM(user, PERM_OBOARDS)) {
        sprintf(buffer, "[通知]");
        fprintf(fn1, "寄信人: %s \n", user->userid);
        fprintf(fn1, "标  题: %s\n", buffer);
        fprintf(fn1, "发信站: %s (%24.24s)\n", BBS_FULL_NAME, ctime(&now));
        fprintf(fn1, "来  源: %s \n", SHOW_USERIP(NULL, session->fromhost));
        fprintf(fn1, "\n");
        if (!strcmp(user->userid, DELIVER))
            fprintf(fn1, "您被自动解封系统解除在 %s 版的封禁\n", board);
        else
            fprintf(fn1, "您被站务人员 %s 解除在 %s 版的封禁\n", user->userid, board);
    } else {
        sprintf(buffer, "[通知]");
        fprintf(fn1, "寄信人: %s \n", user->userid);
        fprintf(fn1, "标  题: %s\n", buffer);
        fprintf(fn1, "发信站: %s (%24.24s)\n", BBS_FULL_NAME, ctime(&now));
        fprintf(fn1, "来  源: %s \n", SHOW_USERIP(NULL, session->fromhost));
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
    post_file(user, "", filename, "undenypost", buffer, 0, 1, session);
    unlink(filename);
    bmlog(user->userid, board, 11, 1);
    if (notice_only)
        return 1;
    else
        return del_from_file(fn, lookupuser ? lookupuser->userid : uident);
}

/* etnlegend, 2005.10.03 */
/*

    发信人: flyriver (江~~忙碌生活), 信区: ****
    标  题: Re: 小bug吧 (转载)
    发信站: 水木社区 (Mon Oct  3 11:10:18 2005), 站内


    我认为改动 normal_board() 这类的函数要慎重，
    如果测试不够充分就容易造成信息泄漏。

    normal_board() 最初好像是为了避免内部版面也过滤关键字编写的，
    现在的用途已经大大扩展，所以还是得小心。

*/
int public_board(const struct boardheader *bh){
    //有身份限制或者有俱乐部读限制
    if(bh->title_level||bh->flag&BOARD_CLUB_READ)
        return 0;
    //无权限限制或者限制发表权限或者限制读取权限但 PERM_DEFAULT 可以访问
    if(!(bh->level)||bh->level&(PERM_POSTMASK|PERM_DEFAULT))
        return 1;
    //其余情况
    return 0;
}
int normal_board(const char *bname){
    const struct boardheader *bh;
    if(!(bh=getbcache(bname)))
        return 0;
    return public_board(bh);
}


int fav_loaddata(struct newpostdata *nbrd, int favnow,int pos,int len,int sort,const char** input_namelist,session_t* session)
{
//注意，如果是目录，nbrd的flag应该为0xffffffff
    int n;
    struct boardheader *bptr=NULL;
    int brdnum;
    struct newpostdata *ptr;
    int curcount;
    int* indexlist=NULL;
	int *sort1list=NULL;
	int thisonline = 0;
    const char** namelist=NULL;

    brdnum = 0;
    curcount=0;
    if (session->zapbuf == NULL) {
        load_zapbuf(session);
    }
    if (sort) {
		if(sort&BRDSORT1_FLAG)
			sort1list=(int*)malloc(sizeof(int*)*(pos+len-1));
		else if (input_namelist==NULL)
    	    namelist=(const char**)malloc(sizeof(char**)*(pos+len-1));
    	else
    	    namelist=input_namelist;
    	indexlist=(int*)malloc(sizeof(int*)*(pos+len-1));
    }
    for (n = 0; n < session->favbrd_list[favnow].bnum; n++) {
        if (session->favbrd_list[favnow].bid[n] >=0) {
            bptr = (struct boardheader *) getboard(session->favbrd_list[favnow].bid[n] + 1);
            if (!bptr)
                continue;
            if (!*bptr->filename)
                continue;
            if (!check_see_perm(session->currentuser,bptr))
                continue;
        }else{
			if (favnow<0) continue;
			if (!HAS_PERM(session->currentuser,session->favbrd_list[0-session->favbrd_list[favnow].bid[n]].level))
				continue;
		}
        /*肯定要计算的版面*/
        brdnum++;
        if (!sort) {
	    if (input_namelist) {
            if (session->favbrd_list[favnow].bid[n] < 0) 
	            input_namelist[brdnum-1]=NullChar;
			else
	            input_namelist[brdnum-1]=bptr->filename;
            }
            if (brdnum<pos||brdnum>=pos+len)
            	continue;
            if (nbrd) {
                ptr = &nbrd[brdnum-pos];
                if (session->favbrd_list[favnow].bid[n] < 0) {
                    ptr->name = NullChar;
                    ptr->title = session->favbrd_list[0-session->favbrd_list[favnow].bid[n]].title;
                    ptr->dir = 1;
                    //ptr->BM = NullChar;
                    ptr->BM = session->favbrd_list[0-session->favbrd_list[favnow].bid[n]].ename;
                    ptr->flag = 0xffffffff;
                    ptr->tag = 0-session->favbrd_list[favnow].bid[n];
                    ptr->pos = n;
                    ptr->total = session->favbrd_list[0-session->favbrd_list[favnow].bid[n]].bnum;
                    ptr->unread = 1;
                    ptr->zap = 0;
                } else {
                    ptr->name = bptr->filename;
                    ptr->dir = 0;
                    ptr->title = bptr->title;
                    ptr->BM = bptr->BM;
                    ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
                    ptr->tag = n;
                    ptr->pos = session->favbrd_list[favnow].bid[n];
		    if (bptr->flag&BOARD_GROUP)
                    ptr->total = bptr->board_data.group_total;
		    else
                    ptr->total = -1;
                    ptr->zap = (session->zapbuf[session->favbrd_list[favnow].bid[n]] == 0);
                }
            	}
        } else {  /*如果是要排序，那么应该先排序缓存一下*/
            int i;
            const char* title = NULL;
            int j;
			if(sort&BRDSORT1_FLAG){
				struct BoardStatus *tpr;
            	if (session->favbrd_list[favnow].bid[n] < 0)
					thisonline=9999;
    	        else{
    				tpr = getbstatus(session->favbrd_list[favnow].bid[n]+1);
					if(tpr) thisonline = tpr->currentusers;
					else thisonline=0;
				}
			}else{
            if (session->favbrd_list[favnow].bid[n] < 0)
            	title=NullChar;
            else
            	title=bptr->filename;
			}
            for (i=0;i<curcount;i++) {
				if(sort&BRDSORT1_FLAG){
					if(sort1list[i] < thisonline) break;
				}
				else{
					if (strcasecmp(namelist[i],title)>0) break;
				}
            }
            if ((i==curcount)&&curcount>=pos+len-1) /*已经在范围之外乐*/
            	continue;
            else
            	   for (j=(curcount>=pos+len-1)?pos+len-2:curcount;j>i;j--) {
						if(sort&BRDSORT1_FLAG) sort1list[j]=sort1list[j-1];
						else namelist[j]=namelist[j-1];
            			indexlist[j]=indexlist[j-1];
             	   }
			if(sort&BRDSORT1_FLAG) sort1list[i]=thisonline;
			else namelist[i]=title;
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
        ptr->flag = 0xffffffff;
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
                if (session->favbrd_list[favnow].bid[indexlist[n]] >= 0) {
                    bptr = (struct boardheader *) getboard(session->favbrd_list[favnow].bid[indexlist[n]] + 1);
                    ptr->name = bptr->filename;
                    ptr->dir = 0;
                    ptr->title = bptr->title;
                    ptr->BM = bptr->BM;
                    ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
                    ptr->tag = indexlist[n];
                    ptr->pos = session->favbrd_list[favnow].bid[indexlist[n]];
		    if (bptr->flag&BOARD_GROUP)
                    ptr->total = bptr->board_data.group_total;
		    else
                    ptr->total = -1;
                    ptr->zap = (session->zapbuf[session->favbrd_list[favnow].bid[indexlist[n]]] == 0);
                } else {
                    ptr->name = NullChar;
                    ptr->title = session->favbrd_list[0-session->favbrd_list[favnow].bid[indexlist[n]]].title;
                    ptr->dir = 1;
                    //ptr->BM = NullChar;
                    ptr->BM = session->favbrd_list[0-session->favbrd_list[favnow].bid[indexlist[n]]].ename;
                    ptr->flag = 0xffffffff;
                    ptr->tag = 0-session->favbrd_list[favnow].bid[indexlist[n]];
                    ptr->pos = indexlist[n];
                    ptr->total = session->favbrd_list[0-session->favbrd_list[favnow].bid[indexlist[n]]].bnum;
                    ptr->unread = 1;
                    ptr->zap = 0;
                }
            }
        }
    }
    if (sort) {
		if(sort&BRDSORT1_FLAG)
			free(sort1list);
		else if (input_namelist==NULL)
    	    free(namelist);
    	free(indexlist);
    }
    return brdnum;
}

int load_boards(struct newpostdata *nbrd,const char *boardprefix,int group,int pos,int len,int sort,int yank_flag,const char** input_namelist,session_t* session)
{
    int n;
    const struct boardheader *bptr;
    int brdnum;
    struct newpostdata *ptr;
    int curcount;
    const char** namelist;
    const char** titlelist;
    int* indexlist;
    int* sortlist;
    int thisonline = 0;
	struct BoardStatus *tpr;
	time_t tnow;

	tnow = time(0);
    brdnum = 0;
    curcount=0;
    if (session->zapbuf == NULL) {
        load_zapbuf(session);
    }
    if (input_namelist==NULL)
        namelist=(const char**)malloc(sizeof(char**)*(pos+len-1));
    else
    	namelist=input_namelist;
    titlelist=(const char**)malloc(sizeof(char**)*(pos+len-1));
    indexlist=(int*)malloc(sizeof(int*)*(pos+len-1));
    sortlist=(int*)malloc(sizeof(int*)*(pos+len-1));
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
        if (!check_see_perm(session->currentuser,bptr)) {
            continue;
        }
        if ((group==0)&&(boardprefix != NULL && strchr(boardprefix, bptr->title[0]) == NULL && boardprefix[0] != '*'))
            continue;
        if (yank_flag || session->zapbuf[n] != 0 || (bptr->level & PERM_NOZAP)) {
            int i;
            int j;
            brdnum++;
		    if (sort & BRDSORT1_FLAG){
    				tpr = getbstatus(n+1);
					if(tpr) thisonline = tpr->currentusers;
					else thisonline=0;
		    }
            /*都要排序*/
            for (i=0;i<curcount;i++) {
		    int type;
		    type = 0;

		    if (sort & BRDSORT1_FLAG){
			type = thisonline - sortlist[i];
		    }else if (!sort) {
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
                    sortlist[j]=sortlist[j-1];
                    indexlist[j]=indexlist[j-1];
                }
            namelist[i]=bptr->filename;
            titlelist[i]=bptr->title;
            sortlist[i]=thisonline;
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
            ptr->zap = (session->zapbuf[indexlist[n]] == 0);
        }
    }
    free(titlelist);
    free(sortlist);
    if (input_namelist==NULL)
        free(namelist);
    free(indexlist);
    return brdnum;
}

/* club functions */
int set_user_club_perm(struct userec *user,const struct boardheader *board,int write_perm){
    if(!user||!board||!(board->flag&(BOARD_CLUB_READ|BOARD_CLUB_WRITE))
        ||!(board->clubnum>0)||(board->clubnum>MAXCLUB))
        return -1;
    if(!write_perm)
        user->club_read_rights[(board->clubnum-1)>>5]|=(1<<((board->clubnum-1)&0x1F));
    else
        user->club_write_rights[(board->clubnum-1)>>5]|=(1<<((board->clubnum-1)&0x1F));
    return 0;
}
int del_user_club_perm(struct userec *user,const struct boardheader *board,int write_perm){
    if(!user||!board||!(board->flag&(BOARD_CLUB_READ|BOARD_CLUB_WRITE))
        ||!(board->clubnum>0)||(board->clubnum>MAXCLUB))
        return -1;
    if(!write_perm)
        user->club_read_rights[(board->clubnum-1)>>5]&=~(1<<((board->clubnum-1)&0x1F));
    else
        user->club_write_rights[(board->clubnum-1)>>5]&=~(1<<((board->clubnum-1)&0x1F));
    return 0;
}
int get_user_club_perm(const struct userec *user,const struct boardheader *board,int write_perm){
    if(!user||!board||!(board->flag&(BOARD_CLUB_READ|BOARD_CLUB_WRITE))
        ||!(board->clubnum>0)||(board->clubnum>MAXCLUB))
        return 0;
    if(!write_perm)
        return (user->club_read_rights[(board->clubnum-1)>>5]&(1<<((board->clubnum-1)&0x1F)));
    else
        return (user->club_write_rights[(board->clubnum-1)>>5]&(1<<((board->clubnum-1)&0x1F)));
}
int club_maintain_send_mail(const char *userid,const char *comment,int type,int write_perm,const struct boardheader *bh,session_t *session){
    FILE *fp;
    char fn[256],title[256];
    sprintf(fn,"tmp/club_notify_%ld_%d",time(NULL),(int)getpid());
    if(!(fp=fopen(fn,"w")))
        return -1;
    if(!type)
        sprintf(title,"%s 由 %s 授予 %s 俱乐部%s权限",userid,session->currentuser->userid,
            bh->filename,(!write_perm?"读取":"发表"));
    else
        sprintf(title,"%s 被 %s 取消 %s 俱乐部%s权限",userid,session->currentuser->userid,
            bh->filename,(!write_perm?"读取":"发表"));
    write_header(fp,session->currentuser,0,bh->filename,title,0,0,session);
    fprintf(fp,"附加说明: %s\n",comment);
    fclose(fp);
    post_file(session->currentuser,"",fn,bh->filename,title,0,-1,session);
    mail_file(session->currentuser->userid,fn,(char*)userid,title,BBSPOST_MOVE,NULL);
    unlink(fn);
    return 0;
}

