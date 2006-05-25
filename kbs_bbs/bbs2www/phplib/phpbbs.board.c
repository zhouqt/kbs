#include "php_kbs_bbs.h"  

static void assign_board(zval * array, const struct boardheader *board, const struct BoardStatus* bstatus, int num)
{
    add_assoc_long(array, "NUM", num); // kept for back compatible
    add_assoc_long(array, "BID", num);
    add_assoc_string(array, "NAME", (char*)board->filename, 1);
    /*
     * add_assoc_string(array, "OWNER", board->owner, 1);
     */
    add_assoc_string(array, "BM", (char*)board->BM, 1);
    add_assoc_long(array, "FLAG", board->flag);
    add_assoc_string(array, "DESC", (char*)board->title + 13, 1);
    add_assoc_stringl(array, "CLASS", (char*)board->title + 1, 6, 1);
    add_assoc_stringl(array, "SECNUM", (char*)board->title, 1, 1);
    add_assoc_long(array, "LEVEL", board->level);
    add_assoc_long(array, "CURRENTUSERS", bstatus->currentusers);
    add_assoc_long(array, "LASTPOST", bstatus->lastpost);
    add_assoc_long(array, "TOTAL", bstatus->total);
}



char *brd_col_names[] = {
    "NAME",
    "DESC",
    "CLASS",
    "BM",
    "ARTCNT",                   /* article count */
    "UNREAD",
    "ZAPPED",
    "BID",
    "POSITION",                  /* added by caltary */
    "FLAG" ,          /* is group ?*/
    "NPOS" ,
    "CURRENTUSERS",      /* added by atppp */
    "LASTPOST"
};
#define BOARD_COLUMNS sizeof(brd_col_names)/sizeof(char *)

/* added by caltary */
#define favbrd_list_t (*(getSession()->favbrd_list_count))


/* TODO: this is very stupid... need to speed it up */
static void bbs_make_board_zval(zval * value, char *col_name, struct newpostdata *brd)
{
    int len = strlen(col_name);

    if (strncmp(col_name, "ARTCNT", len) == 0) {
        ZVAL_LONG(value, brd->total);
    } else if (strncmp(col_name, "UNREAD", len) == 0) {
        ZVAL_LONG(value, brd->unread);
    } else if (strncmp(col_name, "ZAPPED", len) == 0) {
        ZVAL_LONG(value, brd->zap);
    } else if (strncmp(col_name, "CLASS", len) == 0) {
        ZVAL_STRINGL(value, (char *)brd->title + 1, 6, 1);
    } else if (strncmp(col_name, "DESC", len) == 0) {
        ZVAL_STRING(value, (char *)brd->title + 13, 1);
    } else if (strncmp(col_name, "NAME", len) == 0) {
        ZVAL_STRING(value, (char *)brd->name, 1);
    } else if (strncmp(col_name, "BM", len) == 0) {
        ZVAL_STRING(value, (char *)brd->BM, 1);
    /* added by caltary */
    } else if (strncmp(col_name, "POSITION", len) == 0){
        ZVAL_LONG(value, brd->pos);/*added end */
    } else if (strncmp(col_name, "FLAG", len) == 0){
        ZVAL_LONG(value, brd->flag);/*added end */
    } else if (strncmp(col_name, "BID", len) == 0){
        ZVAL_LONG(value, brd->pos+1);/*added end */
    } else if (strncmp(col_name, "NPOS", len) == 0){
        ZVAL_LONG(value, brd->pos);/*added end */
    } else if (strncmp(col_name, "CURRENTUSERS", len) == 0){
        ZVAL_LONG(value, brd->currentusers);
    } else if (strncmp(col_name, "LASTPOST", len) == 0){
        ZVAL_LONG(value, brd->lastpost);
    } else {
        ZVAL_EMPTY_STRING(value);
    }
}

static void assign_board_zval(zval * array, struct newpostdata *brd)
{
    add_assoc_string(array, "NAME", (char *)brd->name, 1);
    add_assoc_string(array, "DESC", (char *)brd->title + 13, 1);
    add_assoc_stringl(array, "CLASS", (char *)brd->title + 1, 6, 1);
    add_assoc_string(array, "BM", (char *)brd->BM, 1);
    add_assoc_long(array, "ARTCNT", brd->total);
    add_assoc_long(array, "UNREAD", brd->unread);
    add_assoc_long(array, "ZAPPED", brd->zap);
    add_assoc_long(array, "BID", brd->pos+1);
    add_assoc_long(array, "POSITION", brd->pos);
    add_assoc_long(array, "FLAG", brd->flag);
    add_assoc_long(array, "NPOS", brd->pos);
    add_assoc_long(array, "CURRENTUSERS", brd->currentusers);
    add_assoc_long(array, "LASTPOST", brd->lastpost);
}

static void bbs_make_favdir_zval(zval * value, char *col_name, struct newpostdata *brd)
{
    int len = strlen(col_name);

    if (strncmp(col_name, "DESC", len) == 0) {
        ZVAL_STRING(value, (char *)brd->title, 1);
    } else if (strncmp(col_name, "NAME", len) == 0) {
        ZVAL_STRING(value, (char *)brd->name, 1);
    } else if (strncmp(col_name, "POSITION", len) == 0){
		/* 保存目录的上一级的索引值 */
        ZVAL_LONG(value, getSession()->favbrd_list[brd->tag].father);
    } else if (strncmp(col_name, "FLAG", len) == 0){
        ZVAL_LONG(value, (brd->flag == 0xffffffff) ? -1L : brd->flag);/*added end */
    } else if (strncmp(col_name, "BID", len) == 0){
		/* 保存目录的索引值 */
        ZVAL_LONG(value, brd->tag);/*added end */
    } else if (strncmp(col_name, "NPOS", len) == 0){
		/* 保存目录的索引值 */
        ZVAL_LONG(value, brd->pos);/*added end */
    } else {
        ZVAL_EMPTY_STRING(value);
    }
}

static void assign_favdir_zval(zval * array, struct newpostdata *brd)
{
    add_assoc_string(array, "NAME", (char *)brd->name, 1);
    add_assoc_string(array, "DESC", (char *)brd->title, 1);
    add_assoc_string(array, "CLASS", "", 1);
    add_assoc_string(array, "BM", (char *)brd->BM, 1);
    add_assoc_long(array, "ARTCNT", 0);
    add_assoc_long(array, "UNREAD", 0);
    add_assoc_long(array, "ZAPPED", 0);
    add_assoc_long(array, "BID", brd->tag);
    add_assoc_long(array, "POSITION", getSession()->favbrd_list[brd->tag].father);
    add_assoc_long(array, "FLAG", (brd->flag == 0xffffffff) ? -1L : brd->flag);
    add_assoc_long(array, "NPOS", brd->pos);
    add_assoc_long(array, "CURRENTUSERS", 0);
    add_assoc_long(array, "LASTPOST", 0);
}



/* TODO: move this function into bbslib. */
/* no_brc added by atppp 20040706 */
static int check_newpost(struct newpostdata *ptr, bool no_brc)
{
    struct BoardStatus *bptr;

    ptr->total = ptr->unread = 0;

    bptr = getbstatus(ptr->pos+1);
    if (bptr == NULL)
        return 0;
    ptr->total = bptr->total;
    ptr->currentusers = bptr->currentusers;
    ptr->lastpost = bptr->lastpost;

    if (!strcmp(getCurrentUser()->userid, "guest")) {
        ptr->unread = 1;
        return 1;
    }

    if (no_brc) return 1;

#ifdef HAVE_BRC_CONTROL
    if (!brc_initial(getCurrentUser()->userid, ptr->name, getSession())) {
        ptr->unread = 1;
    } else {
        if (brc_board_unread(ptr->pos+1, getSession())) {
            ptr->unread = 1;
        }
    }
#endif
    return 1;
}




PHP_FUNCTION(bbs_getboard)
{
    zval *array;
    char *boardname;
    int boardname_len;
    const struct boardheader *bh;
    const struct BoardStatus *bs;
    int b_num;

    if (ZEND_NUM_ARGS() == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "s", &boardname, &boardname_len) != SUCCESS)
            WRONG_PARAM_COUNT;
        array = NULL;
    } else {
        if (ZEND_NUM_ARGS() == 2) {
            if (zend_parse_parameters(2 TSRMLS_CC, "sa", &boardname, &boardname_len, &array) != SUCCESS)
                WRONG_PARAM_COUNT;
        } else
            WRONG_PARAM_COUNT;
    }
    if (boardname_len > BOARDNAMELEN)
        boardname[BOARDNAMELEN] = 0;
    b_num = getbid(boardname, &bh);
    if (b_num == 0)
        RETURN_LONG(0);
    bs = getbstatus(b_num);
    if (array) {
        if (array_init(array) != SUCCESS)
            WRONG_PARAM_COUNT;
        assign_board(array, bh, bs, b_num);
    }
    RETURN_LONG(b_num);
}

/*
 * return: null: 版面不存在或者当前用户没有阅读权限
 *        false/true: 是不是 normalboard
 */
PHP_FUNCTION(bbs_safe_getboard)
{
    zval *array;
    char *boardname;
    int boardname_len;
    long bid;
    int nb;
    const struct boardheader *bh;
    const struct BoardStatus *bs;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lsa", &bid, &boardname, &boardname_len, &array) != SUCCESS)
        WRONG_PARAM_COUNT;

    if (bid) {
        bh = getboard(bid);
        if (bh == NULL) {
            RETURN_NULL();
        }
    } else {
        if (boardname_len == 0) {
            RETURN_NULL();
        }
        if (boardname_len > BOARDNAMELEN) {
            boardname[BOARDNAMELEN] = 0;
        }
        bid = getbid(boardname, &bh);
        if (bid == 0) {
            RETURN_NULL();
        }
    }
    nb = public_board(bh);
    if (!nb) {
        if (getCurrentUser() == NULL) {
            RETURN_NULL();
        }
        if (!check_read_perm(getCurrentUser(), bh)) {
            RETURN_NULL();
        }
    }
    bs = getbstatus(bid);
    if (array_init(array) != SUCCESS) {
        RETURN_NULL();
    }
    assign_board(array, bh, bs, bid);
    if (nb) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}



/**
 * Fetch all boards which have given prefix into an array.
 * prototype:
 * array bbs_getboards(char *prefix, int group, int flag);
 *
 * prefix: 分类讨论区代号
 * group: 当获得目录版面(二级版面)内的版面时这个传入目录版面 bid，否则设置为 0
 *        prefix = '*', group = 0 的时候返回所有版面
 * flag: bit 0 (LSB): yank (no use now)
 *           1      : no_brc. set to 1 when you don't need BRC info. (will speedup)
 *           2      : all_boards 只在 group = 0 的时候有效，如果设置为 1，就返回
 *                    所有版面，包括目录版面内的版面。设置成 0 的时候，目录版面
 *                    内的版面是不返回的。
 *           3      : 新的返回值模式
 *
 * @return array of loaded boards on success,
 *         FALSE on failure.
 * @author roy 
 *
 * original version by flyriver - removed by atppp
 */
PHP_FUNCTION(bbs_getboards)
{
    /*
     * TODO: The name of "yank" must be changed, this name is totally
     * shit, but I don't know which name is better this time.
     */
    char *prefix;
    int plen;
    long flag, group;
    struct newpostdata *newpost_buffer;
    struct newpostdata *ptr;
    zval **columns;
    zval *element;
    int i;
    int j;
    int ac = ZEND_NUM_ARGS();
    int brdnum, yank, no_brc, all_boards, new_return_mode;
    int total;   

    /*
     * getting arguments 
     */
    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sll", &prefix, &plen, &group,&flag) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

	if (plen == 0) {
		RETURN_FALSE;
	}
    if (getCurrentUser() == NULL) {
        RETURN_FALSE;
    }

	total=get_boardcount();
    
	yank = flag & 1;
    no_brc = flag & 2;
    all_boards = (flag & 4) && (group == 0);
    new_return_mode = (flag & 8);

#if 0
    if  (getSession()->zapbuf==NULL)  {
		char fname[STRLEN];
		int fd, size;

		size = total* sizeof(int);
   		getSession()->zapbuf = (int *) malloc(size);
		if (getSession()->zapbuf==NULL) {
			RETURN_FALSE;
		}
    	for (i = 0; i < total; i++)
        	getSession()->zapbuf[i] = 1;
	   	sethomefile(fname, getCurrentUser()->userid, ".lastread");       /*user的.lastread， zap信息 */
        if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
	        size = total * sizeof(int);
	        read(fd, getSession()->zapbuf, size);
	   	    close(fd);
	    } 
    }
#endif

    brdnum = 0;
    {
	    int n;
	    struct boardheader const *bptr;
	    const char** namelist;
        int* indexlist;
		time_t tnow;

		tnow = time(0);
        namelist=(const char**)emalloc(sizeof(char**)*(total));
		if (namelist==NULL) {
			RETURN_FALSE;
		}
	    indexlist=(int*)emalloc(sizeof(int*)*(total));
		if (indexlist==NULL) {
			RETURN_FALSE;
		}
	    for (n = 0; n < total; n++) {
	        bptr = getboard(n + 1);
	        if (!bptr)
	            continue;
	        if (*(bptr->filename)==0)
	            continue;
			if ( group == -2 ){
				if( ( tnow - bptr->createtime ) > 86400*30 || ( bptr->flag & BOARD_GROUP ) )
					continue;
			}else if (!all_boards && (bptr->group!=group))
	            continue;
	        if (!check_see_perm(getCurrentUser(),bptr)) {
	            continue;
	        }
	        if ((group==0)&&( strchr(prefix, bptr->title[0]) == NULL && prefix[0] != '*'))
	            continue;
	        /* if (yank || getSession()->zapbuf[n] != 0 || (bptr->level & PERM_NOZAP)) */ {
	            /*都要排序*/
	            for (i=0;i<brdnum;i++) {
				    if ( strcasecmp(namelist[i], bptr->filename)>0) 
						break;
				}
				for (j=brdnum;j>i;j--) {
						namelist[j]=namelist[j-1];
					   	indexlist[j]=indexlist[j-1];
				}
			   	namelist[i]=bptr->filename;
			   	indexlist[i]=n;
			   	brdnum++;
		   	}
	   	}
        newpost_buffer = (struct newpostdata*)emalloc(sizeof(struct newpostdata) * brdnum);
		for (i=0;i<brdnum;i++) {
		  	ptr=&(newpost_buffer[i]);
		   	bptr = getboard(indexlist[i]+1);
		   	ptr->dir = bptr->flag&BOARD_GROUP?1:0;
		   	ptr->name = (char*)bptr->filename;
		   	ptr->title = (char*)bptr->title;
		   	ptr->BM = (char*)bptr->BM;
		   	ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
		   	ptr->pos = indexlist[i];
		   	if (bptr->flag&BOARD_GROUP) {
			   	ptr->total = bptr->board_data.group_total;
		   	} else ptr->total=-1;
		   	ptr->zap = (getSession()->zapbuf[indexlist[i]] == 0);
   			check_newpost(ptr, no_brc);
		}

        if (new_return_mode) {
            if (array_init(return_value) == FAILURE) {
                RETURN_FALSE;
            }
            for(i=0;i<brdnum;i++) {
                MAKE_STD_ZVAL(element);
                array_init(element);
                assign_board_zval(element, &(newpost_buffer[i]));
                zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *)&element, sizeof(zval*), NULL);
            }
        } else {
            /*
             * setup column names 
             */
            if (array_init(return_value) == FAILURE) {
                RETURN_FALSE;
            }
            columns = emalloc(BOARD_COLUMNS * sizeof(zval *));
        	if (columns==NULL) {
        		RETURN_FALSE;
        	}
            for (i = 0; i < BOARD_COLUMNS; i++) {
                MAKE_STD_ZVAL(element);
                array_init(element);
                columns[i] = element;
                zend_hash_update(Z_ARRVAL_P(return_value), brd_col_names[i], strlen(brd_col_names[i]) + 1, (void *) &element, sizeof(zval *), NULL);
            }

    		for (i=0;i<brdnum;i++) {
                ptr=&(newpost_buffer[i]);
    	        for (j = 0; j < BOARD_COLUMNS; j++) {
           		    MAKE_STD_ZVAL(element);
    	            bbs_make_board_zval(element, brd_col_names[j], ptr);
    	            zend_hash_index_update(Z_ARRVAL_P(columns[j]), i, (void *) &element, sizeof(zval *), NULL);
    	        }
    		}
            efree(columns);
        }
        efree(newpost_buffer);
		efree(namelist);
	   	efree(indexlist);
    }

}






PHP_FUNCTION(bbs_checkorigin)
{
	char *board;
    int board_len;
    int ac = ZEND_NUM_ARGS();
	int total;

    /*
     * getting arguments 
     */
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &board, &board_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    if (!setboardorigin(board, -1)) {
    	RETURN_LONG(0);
    }
	total = board_regenspecial(board,DIR_MODE_ORIGIN,NULL);

   	RETURN_LONG(total);
}

PHP_FUNCTION(bbs_checkmark)
{
	char *board;
    int board_len;
    int ac = ZEND_NUM_ARGS();
	int total;

    /*
     * getting arguments 
     */
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &board, &board_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    if (!setboardmark(board, -1)) {
    	RETURN_LONG(0);
    }
	total = board_regenspecial(board,DIR_MODE_MARK,NULL);

   	RETURN_LONG(total);
}

PHP_FUNCTION(bbs_getbdes)
{
	char *board;
	int board_len;
	const struct boardheader *bp=NULL;
    int ac = ZEND_NUM_ARGS();

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &board, &board_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(0);
    }
	RETURN_STRING((char *)(bp->des),1);
}

PHP_FUNCTION(bbs_getbname)
{
	long brdnum;
	const struct boardheader *bp=NULL;
    int ac = ZEND_NUM_ARGS();

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l", &brdnum) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if ((bp = getboard(brdnum)) == NULL) {
        RETURN_LONG(0);
    }
	RETURN_STRING((char *)(bp->filename),1);
}

PHP_FUNCTION(bbs_checkreadperm)
{
    long user_num, boardnum;
    struct userec *user;

    if (zend_parse_parameters(2 TSRMLS_CC, "ll", &user_num, &boardnum) != SUCCESS)
        WRONG_PARAM_COUNT;
    user = getuserbynum(user_num);
    if (user == NULL)
        RETURN_LONG(0);
    RETURN_LONG(check_read_perm(user, getboard(boardnum)));
}

PHP_FUNCTION(bbs_checkpostperm)
{
    long user_num, boardnum;
    struct userec *user;
    const struct boardheader *bh;

    if (zend_parse_parameters(2 TSRMLS_CC, "ll", &user_num, &boardnum) != SUCCESS)
        WRONG_PARAM_COUNT;
    user = getuserbynum(user_num);
    if (user == NULL)
        RETURN_LONG(0);
	bh=getboard(boardnum);
	if (bh==0) {
		RETURN_LONG(0);
	}
    RETURN_LONG(haspostperm(user, bh->filename));
}





/**
 * check a board is normal board
 * prototype:
 * int bbs_normal(char* boardname);
 *
 *  @return the result
 *  	1 -- normal board
 *  	0 -- no
 *  @author kcn
 */
PHP_FUNCTION(bbs_normalboard)
{
    int ac = ZEND_NUM_ARGS();
    char* boardname;
    int name_len;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &boardname, &name_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	RETURN_LONG(normal_board(boardname));
}

/**
 * search board by keyword
 * function bbs_searchboard(string keyword,int exact,array boards)
 * @author: windinsn May 17,2004
 * return true/false
 */
PHP_FUNCTION(bbs_searchboard)
{
    char *keyword;
    int keyword_len;
    long exact;
    zval *element,*boards;
    boardheader_t *bc;
    int i;
    char *board1,*title;

    int ac = ZEND_NUM_ARGS();
    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sla", &keyword, &keyword_len, &exact ,&boards) == FAILURE)
		WRONG_PARAM_COUNT;
	
	if (!*keyword)
        RETURN_FALSE;
    
    if (array_init(boards) != SUCCESS)
        RETURN_FALSE;
    
    bc = bcache;
    if (exact) { //精确查找
        for (i = 0; i < MAXBOARD; i++) {
            board1 = bc[i].filename;
            title = bc[i].title + 13;
            if (!check_read_perm(getCurrentUser(), &bc[i]))
                 continue;
            if (!strcasecmp(keyword, board1)) {
                MAKE_STD_ZVAL(element);
                array_init(element);
                add_assoc_string(element,"NAME",board1,1);
                add_assoc_string(element,"DESC",bc[i].des,1);
                add_assoc_string(element,"TITLE",title,1);
                zend_hash_index_update(Z_ARRVAL_P(boards),0,(void*) &element, sizeof(zval*), NULL);
                RETURN_TRUE;
            }
        }
        RETURN_FALSE;
    }
    else { //模糊查找
        int total = 0;
        for (i = 0; i < MAXBOARD; i++) {
            board1 = bc[i].filename;
            title = bc[i].title + 13;
            if (!check_read_perm(getCurrentUser(), &bc[i]))
                continue;
            if (strcasestr(board1,keyword) || strcasestr(title,keyword) || strcasestr(bc[i].des,keyword)) {
                MAKE_STD_ZVAL(element);
                array_init(element);
                add_assoc_string(element,"NAME",board1,1);
                add_assoc_string(element,"DESC",bc[i].des,1);
                add_assoc_string(element,"TITLE",title,1);
                zend_hash_index_update(Z_ARRVAL_P(boards),total,(void*) &element, sizeof(zval*), NULL);
                total ++;
            }
        }
        
        RETURN_LONG(total);
   }
}

/**
 * int bbs_useronboard(string baord,array users)
 * show users on board
 * $users = array(
 *              string 'USERID'  
 *              string 'HOST'
 *              );
 * return user numbers , less than 0 when failed
 * @author: windinsn
 *
 */
PHP_FUNCTION(bbs_useronboard)
{
    char *board;
    int   board_len;
    zval *element,*users;
    int bid,i,j;
    long seecloak=0;
    
    int ac = ZEND_NUM_ARGS();
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sz", &board, &board_len, &users) == FAILURE) {
        if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "szl", &board, &board_len, &users, &seecloak) == FAILURE) {
            WRONG_PARAM_COUNT;
        }
	}

    
    bid = getbid(board, NULL);
    if (bid == 0)
        RETURN_LONG(-1);
#ifndef ALLOW_PUBLIC_USERONBOARD
    if(! HAS_PERM(getCurrentUser(), PERM_SYSOP))
		RETURN_LONG(-1);
    seecloak = 1;
#endif
    if (array_init(users) != SUCCESS)
        RETURN_LONG(-1);
    
    j = 0;  
	for (i=0;i<USHM_SIZE;i++) {
        struct user_info* ui;
        ui=get_utmpent(i+1);
        if (ui->active&&ui->currentboard) {
            if (!seecloak && ui->invisible==1) continue;
            if (ui->currentboard == bid) {
                MAKE_STD_ZVAL(element);
                array_init(element);
                add_assoc_string(element,"USERID",ui->userid,1);
                add_assoc_string(element,"HOST",ui->from,1);
                zend_hash_index_update(Z_ARRVAL_P(users),j,(void*) &element, sizeof(zval*), NULL);
                j ++;
            }
        }
    }
    
    resolve_guest_table();
    for (i=0;i<MAX_WWW_GUEST;i++) {
        if (wwwguest_shm->use_map[i / 32] & (1 << (i % 32)))
            if (wwwguest_shm->guest_entry[i].currentboard) {
                if (wwwguest_shm->guest_entry[i].currentboard == bid) {
                    char buf[IPLEN+4];
                    MAKE_STD_ZVAL(element);
                    array_init(element);
                    add_assoc_string(element,"USERID","_wwwguest",1);
                    inet_ntop(AF_INET, &wwwguest_shm->guest_entry[i].fromip, buf, IPLEN);
                    add_assoc_string(element,"HOST",buf,1);
                    zend_hash_index_update(Z_ARRVAL_P(users),j,(void*) &element, sizeof(zval*), NULL);
                    j ++;
                }
            }
    }
    
    RETURN_LONG(j);  
}




PHP_FUNCTION(bbs_set_onboard)
{
	int ac = ZEND_NUM_ARGS();
	long boardnum,count;
	int oldboard;
    struct WWW_GUEST_S *guestinfo = NULL;

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ll", &boardnum, &count) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
    if (getCurrentUser()==NULL) RETURN_FALSE;
    if (getSession()->currentuinfo==NULL) RETURN_FALSE;
    if (!strcmp(getCurrentUser()->userid,"guest")) {
        guestinfo=www_get_guest_entry(getSession()->utmpent);
        oldboard=guestinfo->currentboard;
    } else
        oldboard=getSession()->currentuinfo->currentboard;
    if (oldboard)
        board_setcurrentuser(oldboard, -1);
    
    board_setcurrentuser(boardnum, count);
    if (!strcmp(getCurrentUser()->userid,"guest")) {
        if (count>0)
            guestinfo->currentboard = boardnum;
        else
            guestinfo->currentboard = 0;
    }
    else {
        if (count>0)
            getSession()->currentuinfo->currentboard = boardnum;
        else
            getSession()->currentuinfo->currentboard = 0;
    }
    RETURN_TRUE;
}





/*
  * bbs_load_favboard()
*/
PHP_FUNCTION(bbs_load_favboard)
{
        long select, mode=1;
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &select, &mode) ==FAILURE) {
                WRONG_PARAM_COUNT;
        }
		getSession()->mybrd_list_t = 0;
        load_favboard(mode, getSession());
        if(select>=0 && select<favbrd_list_t)
        {
                SetFav(select, getSession());
                RETURN_LONG(0);
        }
        else 
                RETURN_LONG(-1);
}

PHP_FUNCTION(bbs_is_favboard)
{
        int ac = ZEND_NUM_ARGS();
        long position;
        if(ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l" ,&position) == FAILURE){
                WRONG_PARAM_COUNT;
        }
        RETURN_LONG(IsFavBoard(position-1, getSession(), 1, getSession()->favnow)); //position是bid，但是fav数据结构里头的是-1的. - atppp
}

PHP_FUNCTION(bbs_del_favboarddir)
{
        int ac = ZEND_NUM_ARGS();
		long select;
        long position;
        if(ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ll" , &select, &position) == FAILURE){
                WRONG_PARAM_COUNT;
        }
		if(select < 0 || select >= FAVBOARDNUM)
				RETURN_LONG(-1);
		if(getSession()->nowfavmode != 1)
				RETURN_LONG(-1);

			if(position < 0 || position>= getSession()->mybrd_list[select].bnum)
				RETURN_LONG(-1);

			if(getSession()->mybrd_list[select].bid[position]<0)
				DelFavBoardDir(position,select, getSession());
			else
				RETURN_LONG(-1);
        	save_favboard(1, getSession());
			RETURN_LONG(0);

}

PHP_FUNCTION(bbs_get_dirname)
{
        int ac = ZEND_NUM_ARGS();
		long select;
		char title[256];

        if(ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l" , &select) == FAILURE){
                WRONG_PARAM_COUNT;
        }
	if(select < 0 || select >= favbrd_list_t )
		RETURN_LONG(0);

	FavGetTitle(select,title, getSession());

    RETURN_STRING( title, 1);

}

PHP_FUNCTION(bbs_get_father)
{
        int ac = ZEND_NUM_ARGS();
		long select;
        if(ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l" , &select) == FAILURE){
                WRONG_PARAM_COUNT;
        }

		RETURN_LONG( FavGetFather(select, getSession()) );
}

PHP_FUNCTION(bbs_del_favboard)
{
        int ac = ZEND_NUM_ARGS();
		long select;
        long position;
        if(ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ll" , &select, &position) == FAILURE){
                WRONG_PARAM_COUNT;
        }
		if(getSession()->nowfavmode != 1)
				RETURN_LONG(-1);
        	DelFavBoard(position, getSession());
        	save_favboard(1, getSession());
			RETURN_LONG(0);
}
//add fav dir
PHP_FUNCTION(bbs_add_favboarddir)
{
        int ac = ZEND_NUM_ARGS();
        int char_len;   
        char *char_dname;
        if(ac != 1 || zend_parse_parameters(1 TSRMLS_CC,"s",&char_dname,&char_len) ==FAILURE){
                WRONG_PARAM_COUNT;
        }
        if(char_len <= 20)
        {
				if(getSession()->nowfavmode != 1)
					RETURN_LONG(-1);
                addFavBoardDir(char_dname, getSession());
                save_favboard(1, getSession());
        }
        RETURN_LONG(char_len);
}

PHP_FUNCTION(bbs_add_favboard)
{
        int ac = ZEND_NUM_ARGS();
        int char_len;
        char *char_bname;
        int i;
        if(ac !=1 || zend_parse_parameters(1 TSRMLS_CC,"s",&char_bname,&char_len) ==FAILURE){
                WRONG_PARAM_COUNT;
        }
				if(getSession()->nowfavmode != 1)
					RETURN_LONG(-1);
        i=getbid(char_bname,NULL);
        if(i >0 && ! IsFavBoard(i - 1, getSession(), -1, -1))
        {
                addFavBoard(i - 1, getSession(), -1, -1);
                save_favboard(1, getSession());
        }
}

/**
 * Fetch all fav boards which have given prefix into an array.
 * prototype:
 * array bbs_fav_boards(char *prefix, int yank);
 *
 * @return array of loaded fav boards on success,
 *         FALSE on failure.
 * @
 */


PHP_FUNCTION(bbs_fav_boards)
{
    long select;
    long mode;
    long flag = 0;
    int rows = 0;
    struct newpostdata newpost_buffer[FAVBOARDNUM];
    struct newpostdata *ptr;
    zval **columns;
    zval *element;
    int i;
    int j;
    int brdnum;
    int new_return_mode;
    /*
     * getting arguments 
     */
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|l", &select, &mode, &flag) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    new_return_mode = (flag & 1);


    /*
     * loading boards 
     */
    /*
     * handle some global variables: getCurrentUser(), yank, brdnum, 
     * * nbrd.
     */
    /*
     * NOTE: getCurrentUser() SHOULD had been set in funcs.php, 
     * * but we still check it. 
     */

	if (mode==2){
        load_favboard(2, getSession());
        if(select>=0 && select<favbrd_list_t)
            SetFav(select, getSession());
	}
	else if(mode==3){
        load_favboard(3, getSession());
        if(select>=0 && select<favbrd_list_t)
            SetFav(select, getSession());
	}

	if (getCurrentUser() == NULL) {
        RETURN_FALSE;
    }
    brdnum = 0;
    
    if ((brdnum = fav_loaddata(newpost_buffer, select, 1, FAVBOARDNUM, 1, NULL, getSession())) <= -1) {
        RETURN_FALSE;
    }
    /*
     * fill data in output array. 
     */
    /*
     * setup column names 
     */
    rows=brdnum;
    for (i = 0; i < rows; i++) {
        ptr = &newpost_buffer[i];
        check_newpost(ptr, false);
    }


    if (new_return_mode) {
        if (array_init(return_value) == FAILURE) {
            RETURN_FALSE;
        }
        for(i=0;i<brdnum;i++) {
            MAKE_STD_ZVAL(element);
            array_init(element);
            ptr = &newpost_buffer[i];
			if (ptr->flag == 0xffffffff) /* the item is a directory */
            	assign_favdir_zval(element, &(newpost_buffer[i]));
			else
            	assign_board_zval(element, &(newpost_buffer[i]));
            zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *)&element, sizeof(zval*), NULL);
        }
    } else {
        if (array_init(return_value) == FAILURE) {
            RETURN_FALSE;
        }
        columns = emalloc(BOARD_COLUMNS * sizeof(zval *));

    	if (columns==NULL){
    		RETURN_FALSE;
    	}
        for (i = 0; i < BOARD_COLUMNS; i++) {
            MAKE_STD_ZVAL(element);
            array_init(element);
            columns[i] = element;
            zend_hash_update(Z_ARRVAL_P(return_value), brd_col_names[i], strlen(brd_col_names[i]) + 1, (void *) &element, sizeof(zval *), NULL);
        }
       /*
         * fill data for each column 
         */
       for (i = 0; i < rows; i++) {
            ptr = &newpost_buffer[i];
            for (j = 0; j < BOARD_COLUMNS; j++) {
                MAKE_STD_ZVAL(element);
    			if (ptr->flag == 0xffffffff) /* the item is a directory */
                	bbs_make_favdir_zval(element, brd_col_names[j], ptr);
    			else
                	bbs_make_board_zval(element, brd_col_names[j], ptr);
                zend_hash_index_update(Z_ARRVAL_P(columns[j]), i, (void *) &element, sizeof(zval *), NULL);
            }       
        }
            
        efree(columns);
    }
    
}
