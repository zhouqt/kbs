#include "php_kbs_bbs.h"  


#ifdef HAVE_WFORUM


/* add by roy 2003.8.7 struct wwwthreadheader */
/* used for .WWWTHREAD */
struct wwwthreadheader{
	struct fileheader origin; /* groupid */
	struct fileheader lastreply;	/* id for last article */
	unsigned int articlecount; /* number of articles */
	unsigned int flags;
	unsigned int unused;   /* used for further index */
};
#define FILE_ON_TOP	0x2 /* on top mode */











/* generates .WEBTHREAD */

// balance is LH (left heavy), EH (even), RH (right heavy)
enum balance { LH, EH, RH };
typedef struct _wwwthread_treenode {
    struct wwwthreadheader content; // what's written into .WEBTHREAD finally

    /* AVL Tree supportive variables. This is used for quickly searching groupid. */
    enum balance bf;                // bf is the balance factor
    struct _wwwthread_treenode *Lchild;
    struct _wwwthread_treenode *Rchild;

    /* linked list supportive variables. This linked list is sorted by lastreply.posttime, so can be written into .WEBTHREAD linearly */
    struct _wwwthread_treenode *previous;
} wwwthread_treenode;


/* following four routines are standard AVL operation support. */
static wwwthread_treenode *AVL_RotateLeft(wwwthread_treenode * p)
{
    wwwthread_treenode *temp;
    temp = p->Rchild;
    p->Rchild = temp->Lchild;
    temp->Lchild = p;
    p = temp;
    return p;
}

static wwwthread_treenode *AVL_RotateRight(wwwthread_treenode * p)
{
    wwwthread_treenode *temp;
    temp = p->Lchild;
    p->Lchild = temp->Rchild;
    temp->Rchild = p;
    p = temp;
    return p;
}

static wwwthread_treenode *AVL_RightBalance(wwwthread_treenode * r)
{
    wwwthread_treenode *x;
    wwwthread_treenode *w;

    x = r->Rchild;
    switch (x->bf) {
    case RH:
        r->bf = EH;
        x->bf = EH;
        r = AVL_RotateLeft(r);
        break;
    case LH:
        w = x->Lchild;
        switch (w->bf) {
        case EH:
            r->bf = EH;
            x->bf = EH;
            break;
        case LH:
            r->bf = EH;
            x->bf = RH;
            break;
        case RH:
            r->bf = LH;
            x->bf = EH;
            break;
        }
        w->bf = EH;
        x = AVL_RotateRight(x);
        r->Rchild = x;
        r = AVL_RotateLeft(r);
        break;
    default:
        break;
    }
    return r;
}

static wwwthread_treenode *AVL_LeftBalance(wwwthread_treenode * r)
{
    wwwthread_treenode *x;
    wwwthread_treenode *w;

    x = r->Lchild;
    switch (x->bf) {
    case LH:
        r->bf = EH;
        x->bf = EH;
        r = AVL_RotateRight(r);
        break;
    case RH:
        w = x->Rchild;
        switch (w->bf) {
        case EH:
            r->bf = EH;
            x->bf = EH;
            break;
        case RH:
            r->bf = EH;
            x->bf = LH;
            break;
        case LH:
            r->bf = RH;
            x->bf = EH;
            break;
        }
        w->bf = EH;
        x = AVL_RotateLeft(x);
        r->Lchild = x;
        r = AVL_RotateRight(r);
        break;
    default:
        break;
    }
    return r;
}

/*
 * insert a node into the AVL Tree.
 *
 * @param proot     *proot is the place to store the current root node, can be changed if the tree is rotated after insertion
 * @param fh        *fh is the fileheader information to be stored in content field.
 * @param flags     newly created node->content.flags
 * @param previous  *previous is the place to store the previous node in the linked list. If the groupid is already in the AVL
 *                  tree, then no node is created and *previous leaves unchanged; if a node is newly created, *previous stores
 *                  the newly created node address
 *
 * @author  atppp
 */
static bool AVL_Insert(wwwthread_treenode ** proot, struct fileheader *fh, int flags, wwwthread_treenode** previous)
{
    bool tallersubtree;
    bool taller = false;
    wwwthread_treenode *root;
    int cmp;
    
    root = *proot;
    if (root == NULL) {
        root = (wwwthread_treenode *) malloc(sizeof(wwwthread_treenode));
        if (root == NULL) {
            *previous = NULL;
            return false;
        }
        root->content.lastreply = *fh;

        /*
         * here is the trick: if this post is not the original post, set origin.groupid to something else.
         * So later when written into the .WEBTHREAD file, we know whether the original post does exist or not.
         */
        if (fh->id == fh->groupid)
            root->content.origin = *fh;
        else
            root->content.origin.groupid = fh->groupid + 1;
        
        root->content.flags = flags;
        root->content.articlecount = 1;
        root->content.unused = 0;
        
        root->previous = *previous; //linked list support
        *previous = root;           //return the newly created node address in *previous, so the caller can set tail variable.

        /* AVL Tree support */
        root->Lchild = NULL;
        root->Rchild = NULL;
        root->bf = EH;
        taller = true;
    } else {
        cmp = fh->groupid - root->content.lastreply.groupid;
        if (cmp < 0) {
            tallersubtree = AVL_Insert(&(root->Lchild), fh, flags, previous);
            if (tallersubtree)
                switch (root->bf) {
                case LH:
                    root = AVL_LeftBalance(root);
                    taller = false;
                    break;
                case EH:
                    root->bf = LH;
                    taller = true;
                    break;
                case RH:
                    root->bf = EH;
                    taller = false;
                    break;
                }

            else
                taller = false;
        } else if (cmp > 0) {
            tallersubtree = AVL_Insert(&(root->Rchild), fh, flags, previous);
            if (tallersubtree)
                switch (root->bf) {
                case LH:
                    root->bf = EH;
                    taller = false;
                    break;
                case EH:
                    root->bf = RH;
                    taller = true;
                    break;
                case RH:
                    root = AVL_RightBalance(root);
                    taller = false;
                }

            else
                taller = false;
        } else {  // the groupid (node) already exists
            /* We have a bug here: sysmail board won't generate correct .WEBTHREAD file. But who cares!!! */
			if (root->content.lastreply.groupid == root->content.lastreply.id) { // this node was created by Zhiding fileheader
				root->content.lastreply = *fh;
			} else {
				root->content.articlecount++;
                if ((fh->groupid == fh->id) && (root->content.flags != FILE_ON_TOP)) { //found the original post, put into origin field
                                               /* 后面这个判断只是为了保证新旧代码产生 .WEBTHREAD 完全一致。这个细节以后还要调整 */
                    root->content.origin =*fh;
                }
            }
            taller = false;
        }
    }
    *proot = root;
    return taller;
}

static void clearWWWThreadList(wwwthread_treenode *p){
	wwwthread_treenode *q;
	while (p!=NULL) {
		q=p->previous;
		free(p);
		p=q;
	}
}

static int www_generateOriginIndex(const char* board)
/* added by roy 2003.7.17 generate .WEBTHREAD index file. Modified by atppp 2004.06.03 */
{
    struct fileheader *ptr1;
    struct flock ldata, ldata2;
    int fd, fd2, size, total, i;
    char olddirect[PATHLEN];
	char currdirect[PATHLEN];
    char *ptr;
    struct stat buf;

    /* AVL Tree support */
    wwwthread_treenode *root = NULL;

    /* linked list support */
    wwwthread_treenode *tail = NULL;
    wwwthread_treenode *temp = NULL;
    
    int bid;
    struct BoardStatus* bs;

    setbdir(DIR_MODE_NORMAL, olddirect, board);
    setbdir(DIR_MODE_WEB_THREAD, currdirect, board);
    if ((fd = open(currdirect, O_WRONLY | O_CREAT | O_TRUNC, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        return -1;      /* 创建文件发生错误*/
    }
    ldata.l_type = F_WRLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
        bbslog("user", "%s", "reclock err");
        close(fd);
        return -2;      /* lock error*/
    }
    /* 开始互斥过程*/

    if ((fd2 = open(olddirect, O_RDONLY, 0664)) == -1) {
        bbslog("user", "%s", "recopen err");
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return -3;
    }
    ldata2.l_type = F_RDLCK;
    ldata2.l_whence = 0;
    ldata2.l_len = 0;
    ldata2.l_start = 0;
    if (fcntl(fd2, F_SETLKW, &ldata2) == -1) {
        bbslog("user", "%s", "reclock err");
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        close(fd2);
        return -4;      /* lock error*/
    }
    if (fstat(fd2, &buf)==-1) {
        ldata2.l_type = F_UNLCK;
        fcntl(fd2, F_SETLKW, &ldata2);
        close(fd2);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return -5;
	}
	
    total = buf.st_size /sizeof(fileheader);

    if ((i = safe_mmapfile_handle(fd2,  PROT_READ, MAP_SHARED, &ptr, &buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata2.l_type = F_UNLCK;
        fcntl(fd2, F_SETLKW, &ldata2);
        close(fd2);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        return -5;
    }

	size=sizeof(struct wwwthreadheader);

    bid = getbid(board,NULL);
    bs = getbstatus(bid);
    for (i=bs->toptitle-1;i>=0;i--) {
        if (bs->topfh[i].groupid!=bs->topfh[i].id) continue;
        AVL_Insert(&root, &(bs->topfh[i]), FILE_ON_TOP, &temp);
        if (temp == NULL) { //malloc failure, impossible?
            clearWWWThreadList(tail);
            return -5;
        }
        tail = temp;
    }


    ptr1 = (struct fileheader *) ptr;
	for (i=total-1;i>=0;i--) {
        AVL_Insert(&root, &(ptr1[i]), 0, &temp);
        if (temp == NULL) { //malloc failure, impossible?
				clearWWWThreadList(tail);
				return -5;
        }
        tail = temp;
	}
	while (tail!=NULL) {
		temp=tail->previous;
        if (tail->content.origin.groupid != tail->content.lastreply.groupid) {
            //original post does not exist, do something
            tail->content.origin = tail->content.lastreply;
            tail->content.origin.id = tail->content.origin.groupid;
            tail->content.origin.groupid++; //indication that this thread has no original post.
        }
        write(fd,&(tail->content),size);
		free(tail);
		tail=temp;
	}
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata2.l_type = F_UNLCK;
    fcntl(fd2, F_SETLKW, &ldata2);
    close(fd2);

    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* 退出互斥区域*/
    close(fd);
    return 0;
}










static int cmp_original_date(const void *a, const void *b) {
    struct wwwthreadheader * pa;
    struct wwwthreadheader * pb;
    pa = *((struct wwwthreadheader **)a);
    pb = *((struct wwwthreadheader **)b);
    return get_posttime(&(pb->origin)) - get_posttime(&(pa->origin));
}

PHP_FUNCTION(bbs_searchtitle)
{
    char *board,*title, *title2, *title3,*author;
    int bLen,tLen,tLen2,tLen3,aLen;
    long date,mmode,attach,maxreturn; /* date < 0 search for threads whose original post time is within (-date) days. - atppp 20040727 */
    const struct boardheader *bh;
    char dirpath[STRLEN];
    int fd;
    struct stat buf;
    struct flock ldata;
    struct wwwthreadheader *ptr1=NULL;
    int threads;
    char* ptr;
    int total,i,j;
    zval * element;
    int is_bm;
    char flags[5];
    zval* columns[3];
    bool is_original_date=false;
    struct wwwthreadheader** resultList;
    char* thread_col_names[]={"origin","lastreply","articlenum"};


    if (ZEND_NUM_ARGS() != 9 || zend_parse_parameters(9 TSRMLS_CC, "sssssllll", &board, &bLen,&title,&tLen, &title2, &tLen2, &title3, &tLen3,&author, &aLen, &date,&mmode,&attach,&maxreturn) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }
    if (date < 0) {
        is_original_date = true;
        date = -date;
    } else if (date == 0) {
        date = 9999;
    }
    if (date > 9999)
        date = 9999;

    if (getbid(board, &bh) == 0)
        RETURN_LONG(-1); //"错误的讨论区";
    if (!check_read_perm(getCurrentUser(), bh))
        RETURN_LONG(-2); //您无权阅读本版;
    is_bm = is_BM(bh, getCurrentUser());
    setbdir(DIR_MODE_WEB_THREAD, dirpath, bh->filename);
    if ((fd = open(dirpath, O_RDONLY, 0)) == -1)
        RETURN_LONG(-3);   
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata)== -1) {
        close(fd);
        RETURN_LONG(-200);
    }
    if (fstat(fd, &buf) == -1) {
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        RETURN_LONG(-201);
    }

    resultList  = emalloc(maxreturn * sizeof(struct wwwthreadheader *));
    if (resultList == NULL) {   
        RETURN_LONG(-211);   
    } 

    total = buf.st_size / sizeof(struct wwwthreadheader);

    if ((i = safe_mmapfile_handle(fd, PROT_READ, MAP_SHARED, &ptr, &buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        RETURN_LONG(-4);
    }
    /*
     * fetching articles 
     */
    if (array_init(return_value) == FAILURE) {
        RETURN_LONG(-210);
    }
#ifdef HAVE_BRC_CONTROL
    brc_initial(getCurrentUser()->userid, board, getSession());
#endif
    ptr1 = (struct wwwthreadheader *) ptr;

    threads = 0;
    for (i=total-1;i>=0;i--) {
        if (title[0] && !strcasestr(ptr1[i].origin.title, title))
            continue;
        if (title2[0] && !strcasestr(ptr1[i].origin.title, title2))
            continue;
        if (author[0] && strcasecmp(ptr1[i].origin.owner, author))
            continue;
        if (title3[0] && strcasestr(ptr1[i].origin.title, title3))
            continue;
        if (abs(time(0) - get_posttime(&(ptr1[i].lastreply))) > date * 86400) {
            /* why abs? and should cache time(0) locally to speed up - atppp */
            if (ptr1[i].flags & FILE_ON_TOP) continue;
            else break; //normal article, lastreply out of range, so we can break
        }
        if (mmode && !(ptr1[i].origin.accessed[0] & FILE_MARKED) && !(ptr1[i].origin.accessed[0] & FILE_DIGEST))
            continue;
        if (attach && ptr1[i].origin.attachment==0)
            continue;

        resultList[threads] = &(ptr1[i]);
        threads++;
        if (threads>=maxreturn) 
            break;
    }

    if (is_original_date) {
        qsort(resultList, threads, sizeof(struct wwwthreadheader *), cmp_original_date);
    }

    for (i = 0; i < threads; i++) {

                MAKE_STD_ZVAL(element);
		array_init(element);
		for (j = 0; j < 3; j++) {
			MAKE_STD_ZVAL(columns[j] );
			zend_hash_update(Z_ARRVAL_P(element), thread_col_names[j], strlen(thread_col_names[j]) + 1, (void *) &columns[j] , sizeof(zval *), NULL);
		}
        make_article_flag_array(flags, &(resultList[i]->origin), getCurrentUser(), bh->filename, is_bm);
		array_init(columns[0] );
		bbs_make_article_array(columns[0], &(resultList[i]->origin), flags, sizeof(flags));

        make_article_flag_array(flags, &(resultList[i]->lastreply), getCurrentUser(), bh->filename, is_bm);
		array_init(columns[1] );
		bbs_make_article_array(columns[1], &(resultList[i]->lastreply), flags, sizeof(flags));
		ZVAL_LONG(columns[2],resultList[i]->articlecount);

		zend_hash_index_update(Z_ARRVAL_P(return_value), i + 1, (void *) &element, sizeof(zval *), NULL);

    }
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* 退出互斥区域*/
    close(fd);
    efree(resultList);
}







/**
 * 获取从start开始的num个版面主题
 * prototype:
 * array bbs_getthreads(char *board, int start, int num,int includeTop);
 *
 * @return array of loaded articles on success,
 *         FALSE on failure.
 * @author roy
 */
PHP_FUNCTION(bbs_getthreads)
{
    char *board;
    int blen;
    long start,num;
    int total;
    const struct boardheader *bp=NULL;
	char dirpath[STRLEN];
    int i,j;
    zval *element;
    int is_bm;
    char flags[5];
	int fd;
	struct stat buf;
	struct flock ldata;
	struct wwwthreadheader *ptr1=NULL;
	char* ptr;
	long includeTop;
    int ac = ZEND_NUM_ARGS();
	int begin,end;
	zval* columns[3];
	char* thread_col_names[]={"origin","lastreply","articlenum"};

    /*
     * getting arguments 
     */
    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "slll", &board, &blen, &start, &num, &includeTop) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

	if (start<0){
		RETURN_FALSE;
	}
	if (num<0){
		RETURN_FALSE;
	}
    /*
     * checking arguments 
     */
    if (getCurrentUser() == NULL) {
        RETURN_FALSE;
    }
    if ((bp = getbcache(board)) == NULL) {
        RETURN_FALSE;
    }

    is_bm = is_BM(bp, getCurrentUser());

    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }
#ifdef HAVE_BRC_CONTROL
    brc_initial(getCurrentUser()->userid, bp->filename, getSession());
#endif


    setbdir(DIR_MODE_WEB_THREAD, dirpath, bp->filename);

    if ((fd = open(dirpath, O_RDONLY, 0)) == -1) {
        RETURN_LONG(-1);   
	}
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata)==-1) {
		close(fd);
		RETURN_LONG(-200);
	}
	if (fstat(fd, &buf)==-1) {
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
		RETURN_LONG(-201);
	}
    total = buf.st_size / sizeof(struct wwwthreadheader);

    if ((i = safe_mmapfile_handle(fd, PROT_READ, MAP_SHARED, &ptr, &buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        RETURN_LONG(-2);
    }


    ptr1 = (struct wwwthreadheader *) ptr;
    /*
     * fetching articles 
     */
	 total--;
	if (!includeTop) {
		for (i=total;i>=0;i--) {
			if (!( ptr1[i].flags & FILE_ON_TOP )) 
				break;
		}
		total=i;
	} 
	begin=total-start;
	end=total-start-num+1;
	if (end<0)
		end=0;

	for (i=begin;i>=end;i--) {
		MAKE_STD_ZVAL(element);
		array_init(element);
		for (j = 0; j < 3; j++) {
			MAKE_STD_ZVAL(columns[j] );
			zend_hash_update(Z_ARRVAL_P(element), thread_col_names[j], strlen(thread_col_names[j]) + 1, (void *) &columns[j] , sizeof(zval *), NULL);
		}
        make_article_flag_array(flags, &(ptr1[i].origin), getCurrentUser(), bp->filename, is_bm);
		array_init(columns[0] );
		bbs_make_article_array(columns[0], &(ptr1[i].origin), flags, sizeof(flags));

        make_article_flag_array(flags, &(ptr1[i].lastreply), getCurrentUser(), bp->filename, is_bm);
		array_init(columns[1] );
		bbs_make_article_array(columns[1], &(ptr1[i].lastreply), flags, sizeof(flags));
		ZVAL_LONG(columns[2],ptr1[i].articlecount);

		zend_hash_index_update(Z_ARRVAL_P(return_value), begin-i, (void *) &element, sizeof(zval *), NULL);
	}
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* 退出互斥区域*/
    close(fd);
}


PHP_FUNCTION(bbs_get_today_article_num)
{
    char *board;
    int blen;
    int total;
    const struct boardheader *bp;
	char dirpath[STRLEN];
    int i;
    int ac = ZEND_NUM_ARGS();
	unsigned int articleNums;
	int fd;
	struct stat buf;
	struct flock ldata;
	struct fileheader *ptr1;
	char* ptr;
	time_t now;
	struct tm nowtm;

    /*
     * getting arguments 
     */
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &board, &blen) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    /*
     * checking arguments 
     */
    if (getCurrentUser() == NULL) {
        RETURN_LONG(-2);
    }
    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(-3);
    }
    setbdir(DIR_MODE_NORMAL, dirpath, bp->filename);

    if ((fd = open(dirpath, O_RDONLY, 0)) == -1)
        RETURN_LONG(-4);   
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata)==-1) {
		close(fd);
		RETURN_LONG(-200);
	}
	if (fstat(fd, &buf) == -1 ){
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
		RETURN_LONG(-201);
	}
    total = buf.st_size / sizeof(struct fileheader);

    if ((i = safe_mmapfile_handle(fd, PROT_READ, MAP_SHARED, &ptr, &buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        RETURN_LONG(-5);
    }
    ptr1 = (struct fileheader *) ptr;

	articleNums=0;

	now=time(NULL);
	localtime_r(&now,&nowtm);
	nowtm.tm_sec=0;
	nowtm.tm_min=0;
	nowtm.tm_hour=0;
	now=mktime(&nowtm);

	for (i=total-1;i>=0;i--) {
		if (get_posttime(ptr1+i)<now)
			break;
		articleNums++;
	}
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* 退出互斥区域*/
    close(fd);
	RETURN_LONG(articleNums);
}






/* long bbs_getthreadnum(long boardNum)
 * get number of threads
 */
PHP_FUNCTION(bbs_getthreadnum)
{
    long brdnum;
    const struct boardheader *bp = NULL;
    char dirpath[STRLEN];
    int total;
    int ac = ZEND_NUM_ARGS();
	struct stat normalStat,originStat;
	char dirpath1[STRLEN];

    /*
     * getting arguments 
     */
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l", &brdnum) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if ((bp = getboard(brdnum)) == NULL) {
        RETURN_LONG(-1);
    }
    setbdir(DIR_MODE_WEB_THREAD, dirpath, bp->filename);
	if (!stat(dirpath,&originStat))	{
		setbdir(DIR_MODE_NORMAL,dirpath1,bp->filename);
		if (!stat(dirpath1,&normalStat)){
			if (normalStat.st_mtime>originStat.st_mtime){
				www_generateOriginIndex(bp->filename);
			}
		} else {
			www_generateOriginIndex(bp->filename);
		}
	} else {
		www_generateOriginIndex(bp->filename);
	}
   total = get_num_records(dirpath, sizeof(struct wwwthreadheader));


    RETURN_LONG(total);
}




#endif //HAVE_WFORUM
