#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_kbs_bbs.h"  

#include "bbs.h"
#include "bbslib.h"


void bbs_make_article_array(zval * array, struct fileheader *fh, char *flags, size_t flags_len)
{
    add_assoc_string(array, "FILENAME", fh->filename, 1);
	if (fh->o_bid > 0)
    	add_assoc_string(array, "O_BOARD", (char*)(getboard(fh->o_bid)->filename), 1); /* for compitible */
	else
    	add_assoc_string(array, "O_BOARD", "", 1); /* for compitible */
    add_assoc_long(array, "O_BID", fh->o_bid);
    add_assoc_long(array, "O_ID", fh->o_id);
    add_assoc_long(array, "ID", fh->id);
    add_assoc_long(array, "GROUPID", fh->groupid);
    add_assoc_long(array, "REID", fh->reid);
    add_assoc_long(array, "POSTTIME", get_posttime(fh));
    add_assoc_stringl(array, "INNFLAG", fh->innflag, sizeof(fh->innflag), 1);
    add_assoc_string(array, "OWNER", fh->owner, 1);
    add_assoc_string(array, "TITLE", fh->title, 1);
/*    add_assoc_long(array, "LEVEL", fh->level);*/
    add_assoc_stringl(array, "FLAGS", flags, flags_len, 1);
    add_assoc_long(array, "ATTACHPOS", fh->attachment);
    add_assoc_long(array, "EFFSIZE", fh->eff_size);
    add_assoc_long(array, "IS_TEX", fh->accessed[1] & FILE_TEX);
}


void make_article_flag_array(char flags[4], struct fileheader *ent, struct userec *user, char *boardname, int is_bm)
{
    flags[0] = get_article_flag(ent, user, boardname, is_bm, getSession());
    if (is_bm && (ent->accessed[0] & FILE_IMPORTED))
        flags[1] = 'y';
    else
        flags[1] = 'n';
    if (ent->accessed[1] & FILE_READ)
        flags[2] = 'y';
    else
        flags[2] = 'n';
    if (ent->attachment)
        flags[3] = '@';
    else
        flags[3] = ' ';
}



/**
 * Get some article records from the article id.
 * prototype:
 * int bbs_get_records_from_id(string board, long id, long mode, arrary articles);
 *
 * @return Record index on success,
 *       0 on failure.
 * @author flyriver
 */
PHP_FUNCTION(bbs_get_records_from_id)
{
	char *board;
	int blen;
	long id;
	int num;
	long mode;
	int fd;
	char dirpath[STRLEN];
#define record_cnt 3
	fileheader_t articles[record_cnt];
	struct boardheader *bp;
	int i;
	zval *element,*articlearray;
	int is_bm;
	char flags[3]; /* flags[0]: flag character
					* flags[1]: imported flag
					* flags[2]: no reply flag
					*/
    int ac = ZEND_NUM_ARGS();
    int retnum;

    if (ac != 4
        ||zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slla", &board, &blen, &id, &mode, &articlearray) == FAILURE)
    {
        WRONG_PARAM_COUNT;
    }

    /* check for parameter being passed by reference 
	if (getCurrentUser() == NULL)
	{
		RETURN_LONG(0);
	}
	*/
	if ((bp = getbcache(board)) == NULL)
	{
		RETURN_LONG(0);
	}
    is_bm = is_BM(bp, getCurrentUser());
	/*if (array_init(return_value) == FAILURE)
	{
		RETURN_LONG(0);
	}*/
    setbdir(mode, dirpath, bp->filename);
    if(mode == DIR_MODE_ZHIDING){
        struct BoardStatus* bs=getbstatus(getboardnum(bp->filename,NULL));
	num=0;
        for (i=0;i<bs->toptitle;i++) {
            if (bs->topfh[i].id==id) {
                memcpy(&articles[1],&bs->topfh[i],sizeof(struct fileheader));
                num=i+1;
                break;
            }
        }
	if(num == 0) RETURN_LONG(0);
	memset(articles,0,sizeof(struct fileheader));
	memset(articles+2,0,sizeof(struct fileheader));
    }else{

	if ((fd = open(dirpath, O_RDWR, 0644)) < 0)
	{
		RETURN_LONG(0);
	}
	if ((retnum=get_records_from_id(fd, id, articles, record_cnt, &num)) == 0)
	{
		close(fd);
		RETURN_LONG(0);
	}
	close(fd);
  }
	//MAKE_STD_ZVAL(articlearray);
	if(array_init(articlearray) != SUCCESS)
	{
                RETURN_LONG(0);
	}
	for (i = 0; i < record_cnt; i++)
	{
		MAKE_STD_ZVAL(element);
		array_init(element);
        if(articles[i].id && getCurrentUser() ){
            make_article_flag_array(flags, articles + i, getCurrentUser(), bp->filename, is_bm);
        }else{
            memset(flags, 0, sizeof(flags));
        }
		bbs_make_article_array(element, articles + i, flags, sizeof(flags));
		zend_hash_index_update(Z_ARRVAL_P(articlearray), i,
				(void*) &element, sizeof(zval*), NULL);
	}
	RETURN_LONG(num);
}




/**
 * Get a article records from the article num.
 * prototype:
 * int bbs_get_records_from_num(string dirpath, long num, arrary articles);
 *
 * @return Record index on success,
 *       0 on failure.
 * @author stiger
 */
PHP_FUNCTION(bbs_get_records_from_num)
{
	long num;
	FILE *fp;
	char *dirpath;
	int dlen;
	fileheader_t articles;
	zval *element,*articlearray;
	char flags[3]; /* flags[0]: flag character
					* flags[1]: imported flag
					* flags[2]: no reply flag
					*/
    int ac = ZEND_NUM_ARGS();
    if (ac != 3
        ||zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sla", &dirpath, &dlen, &num, &articlearray) == FAILURE)
    {
        WRONG_PARAM_COUNT;
    }

	if (getCurrentUser() == NULL)
	{
		RETURN_LONG(0);
	}

	if ((fp = fopen(dirpath, "r")) == NULL )
	{
		RETURN_LONG(0);
	}
	fseek(fp, sizeof(articles) * num, SEEK_SET);
	if( fread(&articles, sizeof(articles), 1, fp) < 1 )
	{
		fclose(fp);
		RETURN_LONG(0);
	}
	fclose(fp);

	if(array_init(articlearray) != SUCCESS)
	{
                RETURN_LONG(0);
	}

	MAKE_STD_ZVAL(element);
	array_init(element);
	flags[0]=0;
	flags[1]=0;
	flags[2]=0;
	bbs_make_article_array(element, &articles, flags, sizeof(flags));
	zend_hash_index_update(Z_ARRVAL_P(articlearray), 0,
				(void*) &element, sizeof(zval*), NULL);

	RETURN_LONG(1);
}



/**
 * Get filename from num in the DIR
 * @param sll
 * 		s: board
 * 		l: num
 * 		l: mode
 * @return error: 0
 * 		   success: s: filename
 * @author: stiger
 */
PHP_FUNCTION(bbs_get_filename_from_num)
{

	char *board;
	int blen;
	long num;
	long mode;
	struct boardheader *bp;
	char dirpath[STRLEN];
	fileheader_t fh;
	FILE *fp;

    int ac = ZEND_NUM_ARGS();
    if (ac != 3
        ||zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll", &board, &blen, &num, &mode) == FAILURE)
    {
        WRONG_PARAM_COUNT;
    }
    /* check for parameter being passed by reference */
	if (getCurrentUser() == NULL)
	{
		RETURN_LONG(0);
	}
	if ((bp = getbcache(board)) == NULL)
	{
		RETURN_LONG(0);
	}

	setbdir(mode, dirpath, bp->filename);
	if ((fp=fopen(dirpath,"r"))==NULL)
	{
		RETURN_LONG(0);
	}
	fseek(fp, sizeof(fh) * (num-1), SEEK_SET);
	if( fread(&fh, sizeof(fh), 1, fp) < 1 )
	{
		fclose(fp);
		RETURN_LONG(0);
	}
	fclose(fp);

	RETURN_STRING(fh.filename,1);
}






/**
 * Get some thread records from an article id.
 * prototype:
 * int bbs_get_threads_from_id(long boardid, long id, long mode, long num);
 *
 * @return Records on success,
 *         FALSE on failure.
 * @author flyriver
 */
PHP_FUNCTION(bbs_get_threads_from_id)
{
	long bid;
	long id;
	long num;
	long mode;
	char dirpath[STRLEN];
	fileheader_t *articles;
	const struct boardheader *bp;
	int rc;
	int i;
	zval *element;
	char flags[3] = {0x00}; /* flags[0]: flag character
							 * flags[1]: imported flag
							 * flags[2]: no reply flag
							 */
    int ac = ZEND_NUM_ARGS();
    int retnum;

    if (ac != 4
        ||zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &bid, &id, &mode, &num) == FAILURE)
    {
        WRONG_PARAM_COUNT;
    }

    /* check for parameter being passed by reference 
	*/
	if ((bp = getboard(bid)) == NULL)
	{
		RETURN_FALSE;
	}
	if (num == 0)
	{
		RETURN_FALSE;
	}
	if (num < 0)
	{
		if ((articles = emalloc((-num)*sizeof(fileheader_t))) == NULL)
		{
			RETURN_FALSE;
		}
	}
	else
	{
		if ((articles = emalloc(num*sizeof(fileheader_t))) == NULL)
		{
			RETURN_FALSE;
		}
	}
	if (array_init(return_value) == FAILURE)
	{
		retnum = 0;
		goto threads_error;
	}
	setbdir(mode, dirpath, bp->filename);
	if ((rc = get_threads_from_id(dirpath, id, articles, num)) == 0)
	{
		retnum = 0;
		goto threads_error;
	}
	for (i = 0; i < rc; i++)
	{
		MAKE_STD_ZVAL(element);
		array_init(element);
		bbs_make_article_array(element, articles + i, flags, sizeof(flags));
		zend_hash_index_update(Z_ARRVAL_P(return_value), i,
				(void*) &element, sizeof(zval*), NULL);
	}
	retnum = 1;
threads_error:
	efree(articles);
	if (retnum == 0)
		RETURN_FALSE;
}




/**
 * get a full threads of articles from a groupid.
 * prototype:
 * int bbs_get_threads_from_gid(int bid, int gid, int start , array &articles , int haveprev);
 *
 * @return Record index on success,
 *       0 on failure.
 * @author flyriver
 */
PHP_FUNCTION(bbs_get_threads_from_gid)
{
#define MAX_THREADS_NUM 512
	long bid;
	long gid;
	long start;
    zval *z_threads;
    zval *retprev;
    int i;
	const struct boardheader *bp;
	int is_bm;
	char dirpath[STRLEN];
	struct fileheader *articles;
	int retnum;
	int haveprev;
	zval *element;
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
    int ac = ZEND_NUM_ARGS();
	if( start < 0 )
		start = 0;
    

    if (ac != 5 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "lllzz", &bid , &gid, &start , &z_threads , &retprev) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    /*
     * check for parameter being passed by reference 
     */
    if (!PZVAL_IS_REF(z_threads) || !PZVAL_IS_REF(retprev)) {
        zend_error(E_WARNING, "Parameter wasn't passed by reference");
        RETURN_LONG(0);
    }
    
        if ((bp = getboard(bid)) == NULL)
	{
        RETURN_LONG(0);
	}
        is_bm = is_BM(bp, getCurrentUser());
	setbdir(DIR_MODE_NORMAL, dirpath, bp->filename);

	articles = (struct fileheader *)emalloc(MAX_THREADS_NUM * sizeof(struct fileheader));
	if (articles == NULL)
	{
        RETURN_LONG(0);
	}
	if ((retnum=get_threads_from_gid(dirpath, gid, articles, MAX_THREADS_NUM , start , &haveprev)) == 0)
	{
		efree(articles);
        RETURN_LONG(0);
	}
	
	ZVAL_LONG(retprev , haveprev);
	zval_dtor(z_threads);
	array_init(z_threads);
	for (i = 0; i < retnum; i++)
	{
		MAKE_STD_ZVAL(element);
		array_init(element);
        if(articles[i].id && getCurrentUser() ){
            make_article_flag_array(flags, articles + i, getCurrentUser(), (char*)bp->filename, is_bm);
        }else{
            memset(flags, 0, sizeof(flags));
        }
		bbs_make_article_array(element, articles + i, flags, sizeof(flags));
		zend_hash_index_update(Z_ARRVAL_P(z_threads), i,
				(void*) &element, sizeof(zval*), NULL);
	}
	efree(articles);
	RETURN_LONG(retnum);
}
