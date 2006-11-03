#include "php_kbs_bbs.h"  

void bbs_make_article_array(zval * array, struct fileheader *fh, char *flags, size_t flags_len)
{
    const struct boardheader *bh;
    char title[ARTICLE_TITLE_LEN];
    add_assoc_string(array, "FILENAME", fh->filename, 1);
	if (fh->o_bid > 0 && ((bh = getboard(fh->o_bid)) != NULL))
    	add_assoc_string(array, "O_BOARD", (char*)(bh->filename), 1); /* for compitible */
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
    strcpy(title, fh->title);
    //process_control_chars(title,NULL);
    add_assoc_string(array, "TITLE", title, 1);
/*    add_assoc_long(array, "LEVEL", fh->level);*/
    add_assoc_stringl(array, "FLAGS", flags, flags_len, 1);
    add_assoc_long(array, "ATTACHPOS", fh->attachment);
    add_assoc_long(array, "EFFSIZE", fh->eff_size);
    add_assoc_long(array, "IS_TEX", fh->accessed[1] & FILE_TEX);
}


void make_article_flag_array(char flags[5], struct fileheader *ent, struct userec *user, const char *boardname, int is_bm)
{
    flags[0] = get_article_flag(ent, user, boardname, is_bm, &(flags[4]), getSession());
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
	const struct boardheader *bp;
	int i;
	zval *element,*articlearray;
	int is_bm;
	char flags[5];
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
    is_bm = getCurrentUser() ? is_BM(bp, getCurrentUser()) : 0;
	/*if (array_init(return_value) == FAILURE)
	{
		RETURN_LONG(0);
	}*/
    setbdir(mode, dirpath, bp->filename);
    if(mode == DIR_MODE_ZHIDING){
        struct BoardStatus* bs=getbstatus(getbid(bp->filename,NULL));
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
	const struct boardheader *bp;
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
    char flags[5];
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
	if ((retnum=get_threads_from_gid(dirpath, gid, articles, MAX_THREADS_NUM , start , &haveprev, 0, getCurrentUser())) == 0)
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















/**
 * Count articles in a board with specific .DIR mode.
 * prototype:
 * int bbs_countarticles(int brdnum, int mode);
 *
 * @return non-negative value on success,
 *         negative value on failure.
 * @author flyriver
 */
PHP_FUNCTION(bbs_countarticles)
{
    long brdnum;
    long mode;
    const struct boardheader *bp = NULL;
    char dirpath[STRLEN];
    int total;
    int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments 
     */
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ll", &brdnum, &mode) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if ((bp = getboard(brdnum)) == NULL) {
        RETURN_LONG(-1);
    }
    setbdir(mode, dirpath, bp->filename);

	if(mode == DIR_MODE_THREAD){
//		if(setboardtitle(board, -1)){
			gen_title(bp->filename);
//		}
	}

    total = get_num_records(dirpath, sizeof(struct fileheader));
    /* add by stiger */
	if( mode == DIR_MODE_NORMAL ){
    	sprintf(dirpath,"boards/%s/%s",bp->filename, DING_DIR);
    	total += get_num_records(dirpath, sizeof(struct fileheader));
	}
    /* add end */
    RETURN_LONG(total);
}


/**
 * Fetch a list of articles in a board into an array.
 * prototype:
 * array bbs_getarticles(char *board, int start, int num, int mode);
 *
 * @return array of loaded articles on success,
 *         FALSE on failure.
 * @author flyriver
 */
PHP_FUNCTION(bbs_getarticles)
{
    char *board;
    int blen;
    long start;
    long num;
    long mode;
    char dirpath[STRLEN];
    char dirpath1[STRLEN];	/* add by stiger */
    int total;
    struct fileheader *articles;
    const struct boardheader *bp;
    int rows;
    int i;
    zval *element;
    int is_bm;
    char flags[5];
    int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments 
     */
    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "slll", &board, &blen, &start, &num, &mode) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    /*
     * checking arguments 
     */
    if ((bp = getbcache(board)) == NULL) {
        RETURN_FALSE;
    }
    is_bm = getCurrentUser() ? is_BM(bp, getCurrentUser()) : 0;

    setbdir(mode, dirpath, bp->filename);
    total = get_num_records(dirpath, sizeof(struct fileheader));
    /* add by stiger */
	if(mode == DIR_MODE_NORMAL){
    	sprintf(dirpath1,"boards/%s/" DING_DIR,bp->filename);
    	total += get_num_records(dirpath1, sizeof(struct fileheader));
	}
    /* add end */
    if (start > (total - num + 1))
        start = (total - num + 1);
    if (start <= 0)
        start = 1;

    /*
     * fetching articles 
     */
    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }
#ifdef HAVE_BRC_CONTROL
    if (getCurrentUser()) brc_initial(getCurrentUser()->userid, bp->filename, getSession());
#endif
    articles = emalloc(num * sizeof(struct fileheader));
	if (articles==NULL) {
		RETURN_FALSE;
	}
    /* modified by stiger */
	if(mode == DIR_MODE_NORMAL)
    	rows = read_get_records(dirpath, dirpath1, (char *)articles, sizeof(struct fileheader), start, num);
	else
    	rows = get_records(dirpath, articles, sizeof(struct fileheader), start, num);
    for (i = 0; i < rows; i++) {
        MAKE_STD_ZVAL(element);
        array_init(element);
        if ( getCurrentUser() ){
            make_article_flag_array(flags, articles + i, getCurrentUser(), bp->filename, is_bm);
        }else{
            memset(flags, 0, sizeof(flags));
        }
        bbs_make_article_array(element, articles + i, flags, sizeof(flags));
        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
    }
    efree(articles);
}





PHP_FUNCTION(bbs_search_articles)
{
    char *board,*title, *title2, *title3,*author;
    int bLen,tLen,tLen2,tLen3,aLen;
    long date,mmode,origin,attach;
    const struct boardheader *bh;
	char dirpath[STRLEN];
	int fd;
	struct stat buf;
	struct flock ldata;
	struct fileheader *ptr1;
	char* ptr;
	int total,i;
	zval * element;
	int is_bm;
    char flags[5];
    const struct boardheader *bp;
	int found;
	int i1,i2;
	time_t timeLimit;


    if (ZEND_NUM_ARGS() != 9 || zend_parse_parameters(9 TSRMLS_CC, "sssssllll", &board, &bLen,&title,&tLen, &title2, &tLen2, &title3, &tLen3,&author, &aLen, &date,&mmode,&attach,&origin) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }
    if (date <= 0)
        date = 9999;
    if (date > 9999)
        date = 9999;
    if ((bp = getbcache(board)) == NULL) {
        RETURN_FALSE;
    }
    is_bm = is_BM(bp, getCurrentUser());
    if (getbid(board, &bh) == 0)
        RETURN_LONG(-1); //"错误的讨论区";
    if (!check_read_perm(getCurrentUser(), bh))
        RETURN_LONG(-2); //您无权阅读本版;
    setbdir(DIR_MODE_NORMAL, dirpath, bh->filename);
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
    total = buf.st_size / sizeof(struct fileheader);

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
    ptr1 = (struct fileheader *) ptr;

	i1=0;
	i2=total-1;
	timeLimit = time(0) - date*86400;
	if (total > 1) {
		while( i1 < i2 ){
			i=(i1+i2)/2;
			if( timeLimit > get_posttime(ptr1+i) ){
				i1=i+1;
			}else if(timeLimit < get_posttime(ptr1+i) ){
				i2=i-1;
			}else
				break;
		}
		while( i>=0 && timeLimit <= get_posttime(ptr1+i) )
			i--;

		i++;
	} else {
		i = 0;
	}

	for (found=0;i<total;i++) {
		if (title[0] && !strcasestr(ptr1[i].title, title))
	        continue;
	    if (title2[0] && !strcasestr(ptr1[i].title, title2))
	        continue;
	    if (author[0] && strcasecmp(ptr1[i].owner, author))
	        continue;
		if (title3[0] && strcasestr(ptr1[i].title, title3))
			continue;
		if (timeLimit > get_posttime(ptr1+i))
			continue;
		if (mmode && !(ptr1[i].accessed[0] & FILE_MARKED) && !(ptr1[i].accessed[0] & FILE_DIGEST))
			continue;
		if (origin && (ptr1[i].groupid!=ptr1[i].id) )
			continue;
		if (attach && ptr1[i].attachment==0)
			continue;

			MAKE_STD_ZVAL(element);
			array_init(element);
            make_article_flag_array(flags, ptr1+i , getCurrentUser(), board, is_bm);
			bbs_make_article_array(element, ptr1+i, flags, sizeof(flags));
			add_assoc_long(element, "NUM",i);
			zend_hash_index_update(Z_ARRVAL_P(return_value),found, (void *) &element, sizeof(zval *), NULL);
			found++;
			if (found>=999){
				break;
			}

	}
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* 退出互斥区域*/
    close(fd);
}

