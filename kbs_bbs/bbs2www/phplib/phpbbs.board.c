#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_smth_bbs.h"  

#include "bbs.h"
#include "bbslib.h"


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
    add_assoc_long(array, "TOTAL", bstatus->total);
}



#define BOARD_COLUMNS 12

char *brd_col_names[BOARD_COLUMNS] = {
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
	"CURRENTUSERS"      /* added by atppp */
};

/* added by caltary */
#define favbrd_list_t (*(getSession()->favbrd_list_count))

#if 0
static void bbs_make_board_columns(zval ** columns)
{
    int i;

    for (i = 0; i < BOARD_COLUMNS; i++) {
        MAKE_STD_ZVAL(columns[i]);
        ZVAL_STRING(columns[i], brd_col_names[i], 1);
    }
}
#endif

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
    } else {
        ZVAL_EMPTY_STRING(value);
    }
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

    if (!strcmp(getCurrentUser()->userid, "guest")) {
        ptr->unread = 1;
        return 1;
    }

    if (no_brc) return 1;

#ifdef HAVE_BRC_CONTROL
    if (!brc_initial(getCurrentUser()->userid, ptr->name, getSession())) {
        ptr->unread = 1;
    } else {
        if (brc_unread(bptr->lastpost, getSession())) {
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
    b_num = getbnum(boardname);
    if (b_num == 0)
        RETURN_LONG(0);
    bh = getboard(b_num);
    bs = getbstatus(b_num);
    if (array) {
        if (array_init(array) != SUCCESS)
            WRONG_PARAM_COUNT;
        assign_board(array, bh, bs, b_num);
    }
    RETURN_LONG(b_num);
}




/**
 * Fetch all boards which have given prefix into an array.
 * prototype:
 * array bbs_getboards(char *prefix, int group, int flag);
 *
 * prefix: 分类讨论区代号
 * group: 当获得目录版面(二级版面)内的版面时这个传入目录版面 bid，否则设置为 0
 *        prefix = '*', group = 0 的时候返回所有版面
 * flag: bit 0 (LSB): yank
 *           1      : no_brc. set to 1 when you don't need BRC info. (will speedup)
 *           2      : all_boards 只在 group = 0 的时候有效，如果设置为 1，就返回
 *                    所有版面，包括目录版面内的版面。设置成 0 的时候，目录版面
 *                    内的版面是不返回的。
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
    struct newpostdata newpost_buffer;
    struct newpostdata *ptr;
    zval **columns;
    zval *element;
    int i;
    int j;
    int ac = ZEND_NUM_ARGS();
    int brdnum, yank, no_brc, all_boards;
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

	total=get_boardcount();
    
	yank = flag & 1;
    no_brc = flag & 2;
    all_boards = (flag & 4) && (group == 0);

    if  (getSession()->zapbuf==NULL)  {
		char fname[STRLEN];
		int fd, size;

		size = total* sizeof(int);
   		getSession()->zapbuf = (int *) emalloc(size);
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
	        if (yank || getSession()->zapbuf[n] != 0 || (bptr->level & PERM_NOZAP)) {
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
		for (i=0;i<brdnum;i++) {
		  	ptr=&newpost_buffer;
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
	        for (j = 0; j < BOARD_COLUMNS; j++) {
       		    MAKE_STD_ZVAL(element);
	            bbs_make_board_zval(element, brd_col_names[j], ptr);
	            zend_hash_index_update(Z_ARRVAL_P(columns[j]), i, (void *) &element, sizeof(zval *), NULL);
	        }
		}
		efree(namelist);
	   	efree(indexlist);
    }

    efree(columns);
}

/*
  * bbs_load_favboard()
*/
PHP_FUNCTION(bbs_load_favboard)
{
        int ac = ZEND_NUM_ARGS();
        long select;
        if(ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l", &select) ==FAILURE) {
                WRONG_PARAM_COUNT;
        }
        load_favboard(0,1, getSession());
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
        RETURN_LONG(IsFavBoard(position-1, getSession())); //position是bid，但是fav数据结构里头的是-1的. - atppp
}

PHP_FUNCTION(bbs_del_favboarddir)
{
        int ac = ZEND_NUM_ARGS();
		long select;
        long position;
        if(ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ll" , &select, &position) == FAILURE){
                WRONG_PARAM_COUNT;
        }

			if(position < 0 || position>= getSession()->favbrd_list[select].bnum)
				RETURN_LONG(-1);
			if(getSession()->favbrd_list[select].bid[position]<0)
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
        i=getbnum(char_bname);
        if(i >0 && ! IsFavBoard(i - 1, getSession()))
        {
                addFavBoard(i - 1, getSession());
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
    int rows = 0;
    struct newpostdata newpost_buffer[FAVBOARDNUM];
    struct newpostdata *ptr;
    zval **columns;
    zval *element;
    int i;
    int j;
    int ac = ZEND_NUM_ARGS();
    int brdnum;
    /*
     * getting arguments 
     */
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ll", &select, &mode) == FAILURE) {
        WRONG_PARAM_COUNT;
    }


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
        load_favboard(0,2, getSession());
        if(select>=0 && select<favbrd_list_t)
            SetFav(select, getSession());
	}
	else if(mode==3){
        load_favboard(0,3, getSession());
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
        check_newpost(ptr, false);
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
