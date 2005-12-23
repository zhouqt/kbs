/*
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
*/
    
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_smth_bbs.h"  


#if PHP_MAJOR_VERSION == 5
static
     ZEND_BEGIN_ARG_INFO(one_arg_force_ref_1, 0)
             ZEND_ARG_PASS_INFO(1)
     ZEND_END_ARG_INFO();
static
     ZEND_BEGIN_ARG_INFO(two_arg_force_ref_01, 0)
             ZEND_ARG_PASS_INFO(0)
             ZEND_ARG_PASS_INFO(1)
     ZEND_END_ARG_INFO();
static
     ZEND_BEGIN_ARG_INFO(third_arg_force_ref_1111, 0)
             ZEND_ARG_PASS_INFO(1)
             ZEND_ARG_PASS_INFO(1)
             ZEND_ARG_PASS_INFO(1)
             ZEND_ARG_PASS_INFO(1)
     ZEND_END_ARG_INFO();
static
     ZEND_BEGIN_ARG_INFO(third_arg_force_ref_011, 0)
             ZEND_ARG_PASS_INFO(0)
             ZEND_ARG_PASS_INFO(1)
             ZEND_ARG_PASS_INFO(1)
     ZEND_END_ARG_INFO();
static
     ZEND_BEGIN_ARG_INFO(fourth_arg_force_ref_0001, 0)
             ZEND_ARG_PASS_INFO(0)
             ZEND_ARG_PASS_INFO(0)
             ZEND_ARG_PASS_INFO(0)
             ZEND_ARG_PASS_INFO(1)
     ZEND_END_ARG_INFO();
static
     ZEND_BEGIN_ARG_INFO(fourth_arg_force_ref_0111, 0)
             ZEND_ARG_PASS_INFO(0)
             ZEND_ARG_PASS_INFO(1)
             ZEND_ARG_PASS_INFO(1)
             ZEND_ARG_PASS_INFO(1)
     ZEND_END_ARG_INFO();
static
     ZEND_BEGIN_ARG_INFO(third_arg_force_ref_001, 0)
             ZEND_ARG_PASS_INFO(0)
             ZEND_ARG_PASS_INFO(0)
             ZEND_ARG_PASS_INFO(1)
     ZEND_END_ARG_INFO();
static
     ZEND_BEGIN_ARG_INFO(fifth_arg_force_ref_00011, 0)
             ZEND_ARG_PASS_INFO(0)
             ZEND_ARG_PASS_INFO(0)
             ZEND_ARG_PASS_INFO(0)
             ZEND_ARG_PASS_INFO(1)
             ZEND_ARG_PASS_INFO(1)
     ZEND_END_ARG_INFO();
#else //if PHP_MAJOR_VERSION == 4
static unsigned char one_arg_force_ref_1[]  = { 1, BYREF_FORCE };
static unsigned char two_arg_force_ref_01[] = { 2, BYREF_NONE, BYREF_FORCE };
static unsigned char third_arg_force_ref_1111[] = { 4, BYREF_FORCE, BYREF_FORCE, BYREF_FORCE, BYREF_FORCE };
static unsigned char third_arg_force_ref_011[] = { 3, BYREF_NONE, BYREF_FORCE, BYREF_FORCE };
static unsigned char fourth_arg_force_ref_0001[] = { 4, BYREF_NONE, BYREF_NONE, BYREF_NONE, BYREF_FORCE };
static unsigned char fourth_arg_force_ref_0111[] = { 4, BYREF_NONE, BYREF_FORCE, BYREF_FORCE, BYREF_FORCE };
static unsigned char third_arg_force_ref_001[] = { 3, BYREF_NONE, BYREF_NONE, BYREF_FORCE };
static unsigned char fifth_arg_force_ref_00011[] = { 5, BYREF_NONE, BYREF_NONE, BYREF_NONE, BYREF_FORCE , BYREF_FORCE};
#endif


#include "phpbbs.user.h"
#include "phpbbs.announce.h"
#include "phpbbs.vote.h"
#include "phpbbs.system.h"
#include "phpbbs.board.h"
#include "phpbbs.article.h"
#include "phpbbs.msg.h"
#include "phpbbs.mail.h"
#include "phpbbs.bm.h"
#include "phpbbs.reg.h"

#include "bbs.h"
#include "bbslib.h"


static char old_pwd[1024];


PHP_FUNCTION(bbs_ext_initialized);
PHP_FUNCTION(bbs_init_ext);

////////////////////////  User operation functions  ///////////////////////////
static PHP_FUNCTION(bbs_checkuserpasswd);
static PHP_FUNCTION(bbs_setuserpasswd);
static PHP_FUNCTION(bbs_getuserlevel);
static PHP_FUNCTION(bbs_getusermode);
static PHP_FUNCTION(bbs_compute_user_value);
static PHP_FUNCTION(bbs_user_level_char);
PHP_FUNCTION(bbs_user_setflag);
static PHP_FUNCTION(bbs_isonline);

//Friends
static PHP_FUNCTION(bbs_getfriends);
static PHP_FUNCTION(bbs_getonlinefriends);
static PHP_FUNCTION(bbs_countfriends);
static PHP_FUNCTION(bbs_delete_friend);
static PHP_FUNCTION(bbs_add_friend);

static PHP_FUNCTION(bbs_modify_info);
static PHP_FUNCTION(bbs_recalc_sig);
static PHP_FUNCTION(bbs_modify_nick);
static PHP_FUNCTION(bbs_findpwd_check);
static PHP_FUNCTION(bbs_update_uinfo);
static PHP_FUNCTION(bbs_setpassword);

//////////////////////// Board/Article operation functions  ////////////////////
#ifdef HAVE_WFORUM
static PHP_FUNCTION(bbs_searchtitle);
#endif
static PHP_FUNCTION(bbs_search_articles);
static PHP_FUNCTION(bbs_filteruploadfilename);
static PHP_FUNCTION(bbs_edittitle);
static PHP_FUNCTION(bbs_checkbadword);
static PHP_FUNCTION(bbs_brcaddread);
static PHP_FUNCTION(bbs_brcclear);
static PHP_FUNCTION(bbs_getarticles);
static PHP_FUNCTION(bbs_doforward);
static PHP_FUNCTION(bbs_get_records_from_id);
static PHP_FUNCTION(bbs_get_records_from_num);
static PHP_FUNCTION(bbs_get_filename_from_num);
static PHP_FUNCTION(bbs_get_threads_from_id);
static PHP_FUNCTION(bbs_get_threads_from_gid);
static PHP_FUNCTION(bbs_countarticles);
static PHP_FUNCTION(bbs_set_onboard);
static PHP_FUNCTION(bbs_docross);
static PHP_FUNCTION(bbs_docommend);
static PHP_FUNCTION(bbs_bmmanage);

static PHP_FUNCTION(bbs_delfile);
static PHP_FUNCTION(bbs_normalboard);
static PHP_FUNCTION(bbs_searchboard);
static PHP_FUNCTION(bbs_useronboard);



static PHP_FUNCTION(bbs_caneditfile);
static PHP_FUNCTION(bbs_updatearticle);
#ifdef HAVE_WFORUM
static PHP_FUNCTION(bbs_getthreadnum);
static PHP_FUNCTION(bbs_get_today_article_num);
static PHP_FUNCTION(bbs_getthreads);
#endif

////////////////////////  Mail operation functions  ///////////////////////////
static PHP_FUNCTION(bbs_postmail);



////////////////////////   WWW special functions  /////////////////////////////

#if HAVE_MYSQL_SMTH == 1
static PHP_FUNCTION(bbs_csv_to_al);
#endif

static PHP_FUNCTION(bbs2_readfile);
static PHP_FUNCTION(bbs2_readfile_text);

/*
 * define what functions can be used in the PHP embedded script
 */
static function_entry smth_bbs_functions[] = {
    PHP_BBS_USER_EXPORT_FUNCTIONS
    PHP_BBS_ANNOUNCE_EXPORT_FUNCTIONS
    PHP_BBS_VOTE_EXPORT_FUNCTIONS
    PHP_BBS_SYSTEM_EXPORT_FUNCTIONS
    PHP_BBS_BOARD_EXPORT_FUNCTIONS
    PHP_BBS_ARTICLE_EXPORT_FUNCTIONS
    PHP_BBS_MSG_EXPORT_FUNCTIONS
    PHP_BBS_MAIL_EXPORT_FUNCTIONS
    PHP_BBS_BM_EXPORT_FUNCTIONS
    PHP_BBS_REG_EXPORT_FUNCTIONS
    
    PHP_FE(bbs_ext_initialized, NULL)
    PHP_FE(bbs_init_ext, NULL)

    PHP_FE(bbs_checkuserpasswd, NULL)
    PHP_FE(bbs_setuserpasswd, NULL)
    PHP_FE(bbs_getuserlevel, NULL)
    PHP_FE(bbs_user_setflag, NULL)
#ifdef HAVE_WFORUM
    PHP_FE(bbs_searchtitle, NULL)
#endif
    PHP_FE(bbs_search_articles, NULL)
    PHP_FE(bbs_postmail, NULL)
#ifdef HAVE_WFORUM
    PHP_FE(bbs_get_today_article_num, NULL)
    PHP_FE(bbs_getthreadnum, NULL)
    PHP_FE(bbs_getthreads, NULL)
#endif
    PHP_FE(bbs_getusermode, NULL)
    PHP_FE(bbs_compute_user_value, NULL)
    PHP_FE(bbs_user_level_char, NULL)
    PHP_FE(bbs_isonline, NULL)
    PHP_FE(bbs_caneditfile,NULL)
    PHP_FE(bbs_updatearticle, NULL)
    PHP_FE(bbs_brcaddread, NULL)
    PHP_FE(bbs_brcclear, NULL)
    PHP_FE(bbs_filteruploadfilename,NULL)
    PHP_FE(bbs_edittitle, NULL)
    PHP_FE(bbs_checkbadword, NULL)
    PHP_FE(bbs_getarticles, NULL)
    PHP_FE(bbs_getfriends, NULL)
    PHP_FE(bbs_countfriends, NULL)
    PHP_FE(bbs_delete_friend, NULL)
    PHP_FE(bbs_add_friend, NULL)
    PHP_FE(bbs_doforward, NULL)
    PHP_FE(bbs_get_records_from_id, NULL)
    PHP_FE(bbs_get_records_from_num, NULL)
    PHP_FE(bbs_get_filename_from_num, NULL)
    PHP_FE(bbs_get_threads_from_id, NULL)
    PHP_FE(bbs_get_threads_from_gid, fifth_arg_force_ref_00011)
    PHP_FE(bbs_countarticles, NULL)
    PHP_FE(bbs_update_uinfo, NULL)
    PHP_FE(bbs_setpassword,NULL)
    PHP_FE(bbs_findpwd_check,NULL)
    PHP_FE(bbs_getonlinefriends,NULL)
    PHP_FE(bbs_modify_info,NULL)
    PHP_FE(bbs_recalc_sig,NULL)
    PHP_FE(bbs_modify_nick,NULL)
    PHP_FE(bbs_delfile,NULL)
    PHP_FE(bbs_normalboard,NULL)
    PHP_FE(bbs_searchboard,third_arg_force_ref_001)
    PHP_FE(bbs_useronboard,two_arg_force_ref_01)
    PHP_FE(bbs_set_onboard,NULL)
    PHP_FE(bbs_docross,NULL)
    PHP_FE(bbs_docommend,NULL)
    PHP_FE(bbs_bmmanage,NULL)

#if HAVE_MYSQL_SMTH == 1
    PHP_FE(bbs_csv_to_al, NULL)
#endif

    PHP_FE(bbs2_readfile, NULL)
    PHP_FE(bbs2_readfile_text, NULL)

    {NULL, NULL, NULL}
};

/*
 * This is the module entry structure, and some properties
 */
zend_module_entry smth_bbs_module_entry = {
    STANDARD_MODULE_HEADER,
    "smth_bbs",
    smth_bbs_functions,
    PHP_MINIT(smth_bbs),
    PHP_MSHUTDOWN(smth_bbs),
    PHP_RINIT(smth_bbs),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(smth_bbs),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(smth_bbs),
    "1.0", /* Replace with version number for your extension */
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SMTH_BBS
ZEND_GET_MODULE(smth_bbs)
#endif

/*
 * Here is the function require when the module loaded
DLEXPORT zend_module_entry *get_module()
{
    return &smth_bbs_module_entry;
};


static void setstrlen(pval * arg)
{
    arg->value.str.len = strlen(arg->value.str.val);
}
 */








/*
 * Here goes the real functions
 */

static PHP_FUNCTION(bbs_getusermode)
{
	char *userid;
	int userid_len;
	char buf[1024];

    if (zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
	
	if( userid_len > IDLEN )
		userid[IDLEN]=0;

	if( get_userstatusstr(userid, buf) == 0 )
		RETURN_LONG(0);

	RETURN_STRING(buf,1);
}

static PHP_FUNCTION(bbs_user_level_char)
{
	char *userid;
	int userid_len;
    struct userec *lookupuser;
	char permstr[USER_TITLE_LEN];

    if (zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
	
	if( userid_len > IDLEN )
		userid[IDLEN]=0;

	if( getuser(userid, &lookupuser) == 0 )
		RETURN_LONG(0);

	permstr[0]=0;
	uleveltochar(permstr, lookupuser);
	RETURN_STRING(permstr, 1);

}

static PHP_FUNCTION(bbs_compute_user_value)
{
	char *userid;
	int userid_len;
    struct userec *lookupuser;

    if (zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
	
	if( userid_len > IDLEN )
		userid[IDLEN]=0;

	if( getuser(userid, &lookupuser) == 0 )
		RETURN_LONG(0);

	RETURN_LONG( compute_user_value(lookupuser) );

}


static PHP_FUNCTION(bbs_setuserpasswd){
    char *s;
    int s_len;
    char *pw;
    int pw_len;
    int unum;
    struct userec *user;

    if (zend_parse_parameters(2 TSRMLS_CC, "ss", &s, &s_len, &pw, &pw_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    if (s_len > IDLEN)
        s[IDLEN] = 0;
    if (pw_len > PASSLEN)
        pw[PASSLEN] = 0;
	if (pw_len < 2) {
		RETURN_LONG(-1);
	}
    if ( !(unum = getuser(s, &user))) {
        RETURN_LONG(-2);
    }
	setpasswd(pw, user);
    RETURN_LONG(0);
}

static PHP_FUNCTION(bbs_checkuserpasswd){
    char *s;
    int s_len;
    char *pw;
    int pw_len;
    int unum;
    struct userec *user;

    if (zend_parse_parameters(2 TSRMLS_CC, "ss", &s, &s_len, &pw, &pw_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    if (s_len > IDLEN)
        s[IDLEN] = 0;
    if (pw_len > PASSLEN)
        pw[PASSLEN] = 0;
	if (pw_len < 2) {
		RETURN_LONG(-1);
	}
    if ( !(unum = getuser(s, &user))) {
        RETURN_LONG(-2);
    }
	   if ( !checkpasswd2(pw, user)) {
        RETURN_LONG(-3);
    }
    RETURN_LONG(0);
}

PHP_FUNCTION(bbs_user_setflag)
{
    struct userec *lookupuser;
    char *s;
    int s_len;
    long flag, set;

    if (ZEND_NUM_ARGS() != 3 || zend_parse_parameters(3 TSRMLS_CC, "sll", &s, &s_len, &flag, &set) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    if( ! getuser(s,&lookupuser) ) {
        RETURN_FALSE;
    }
    if (set) {
        lookupuser->flags |= flag;
    } else {
        lookupuser->flags &= ~flag;
    }
    RETURN_TRUE;
}


static int count_online(struct user_info *uentp, int *arg, int pos)
{
     if (uentp->invisible == 1) {
         return COUNT;
     }
     (*arg) = 1;
     UNUSED_ARG(pos);
     return QUIT;
}
/*
 * bbs_isonline(userid), return if this userid is online. If cloak, return false
 * 这个函数慎用，目前仅 wForum 用到 - atppp
 */
static PHP_FUNCTION(bbs_isonline)
{
    int can_see = 0;
    struct userec *lookupuser;
    char *s;
    int s_len;

    if (ZEND_NUM_ARGS() != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &s, &s_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    if( ! getuser(s,&lookupuser) ) {
        RETURN_FALSE;
    }
    apply_utmp((APPLY_UTMP_FUNC) count_online, 0, lookupuser->userid, &can_see);
    if (can_see) RETURN_TRUE;
    RETURN_FALSE;
}


    
static PHP_FUNCTION(bbs_getuserlevel){
    struct userec* u;
	char* user;
	int uLen;
	char title[USER_TITLE_LEN];
    if (ZEND_NUM_ARGS() != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &user, &uLen) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }
    if (getuser(user, &u)==0) {
		RETURN_LONG(-1);
	}
	uleveltochar(title,u);
	title[USER_TITLE_LEN-1]=0;
	RETURN_STRINGL(title,strlen(title),1);
}


static void make_article_flag_array(char flags[4], struct fileheader *ent, struct userec *user, char *boardname, int is_bm)
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



static PHP_FUNCTION(bbs_search_articles)
{
    char *board,*title, *title2, *title3,*author;
    int bLen,tLen,tLen2,tLen3,aLen;
    long date,mmode,origin,attach;
    boardheader_t bh;
	char dirpath[STRLEN];
	int fd;
	struct stat buf;
	struct flock ldata;
	struct fileheader *ptr1;
	char* ptr;
	int total,i;
	zval * element;
	int is_bm;
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
    struct boardheader *bp;
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
    if (getboardnum(board, &bh) == 0)
        RETURN_LONG(-1); //"错误的讨论区";
    if (!check_read_perm(getCurrentUser(), &bh))
        RETURN_LONG(-2); //您无权阅读本版;
    setbdir(DIR_MODE_NORMAL, dirpath, bh.filename);
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

    if ((i = safe_mmapfile_handle(fd, PROT_READ, MAP_SHARED, (void **) &ptr, &buf.st_size)) != 1) {
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

#ifdef HAVE_WFORUM
static int cmp_original_date(const void *a, const void *b) {
    struct wwwthreadheader * pa;
    struct wwwthreadheader * pb;
    pa = *((struct wwwthreadheader **)a);
    pb = *((struct wwwthreadheader **)b);
    return get_posttime(&(pb->origin)) - get_posttime(&(pa->origin));
}

static PHP_FUNCTION(bbs_searchtitle)
{
    char *board,*title, *title2, *title3,*author;
    int bLen,tLen,tLen2,tLen3,aLen;
    long date,mmode,attach,maxreturn; /* date < 0 search for threads whose original post time is within (-date) days. - atppp 20040727 */
    boardheader_t bh;
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
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
    struct boardheader *bp;
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
    if ((bp = getbcache(board)) == NULL) {
        RETURN_FALSE;
    }
    resultList  = emalloc(maxreturn * sizeof(struct wwwthreadheader *));
    if (resultList == NULL) {   
        RETURN_LONG(-211);   
    } 

    is_bm = is_BM(bp, getCurrentUser());
    if (getboardnum(board, &bh) == 0)
        RETURN_LONG(-1); //"错误的讨论区";
    if (!check_read_perm(getCurrentUser(), &bh))
        RETURN_LONG(-2); //您无权阅读本版;
    setbdir(DIR_MODE_WEB_THREAD, dirpath, bh.filename);
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
    total = buf.st_size / sizeof(struct wwwthreadheader);

    if ((i = safe_mmapfile_handle(fd, PROT_READ, MAP_SHARED, (void **) &ptr, &buf.st_size)) != 1) {
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
        make_article_flag_array(flags, &(resultList[i]->origin), getCurrentUser(), bp->filename, is_bm);
		array_init(columns[0] );
		bbs_make_article_array(columns[0], &(resultList[i]->origin), flags, sizeof(flags));

        make_article_flag_array(flags, &(resultList[i]->lastreply), getCurrentUser(), bp->filename, is_bm);
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
#endif

/* function bbs_caneditfile(string board, string filename);
 * 判断当前用户是否有权编辑某文件
 */
 static PHP_FUNCTION(bbs_caneditfile)
{
    char *board,*filename;
    int boardLen,filenameLen;
	char path[512];
    struct fileheader x;
    boardheader_t *brd;


    if ((ZEND_NUM_ARGS() != 2) || (zend_parse_parameters(2 TSRMLS_CC, "ss", &board, &boardLen,&filename,&filenameLen) != SUCCESS)) {
		WRONG_PARAM_COUNT;
    } 
    brd = getbcache(board);
    if (brd == NULL) {
        RETURN_LONG(-1); //讨论区名称错误
    }
	if (getCurrentUser()==NULL)
		RETURN_FALSE;
    if (!strcmp(brd->filename, "syssecurity")
        || !strcmp(brd->filename, "junk")
        || !strcmp(brd->filename, "deleted"))   /* Leeward : 98.01.22 */
         RETURN_LONG(-2);  //本版不能修改文章
    if (checkreadonly(brd->filename) == true) {
		RETURN_LONG(-3); //本版已被设置只读
    }
    if (get_file_ent(brd->filename, filename, &x) == 0) {
        RETURN_LONG(-4); //无法取得文件记录
    }
	setbfile(path, brd->filename, filename);
    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP)     /* SYSOP、当前版主、原发信人 可以编辑 */
        &&!chk_currBM(brd->BM, getCurrentUser())) {
        if (!isowner(getCurrentUser(), &x)) {
            RETURN_LONG(-5); //不能修改他人文章!
        }
    }
    /* 版主禁止POST 检查 */
    if (deny_me(getCurrentUser()->userid, brd->filename) && !HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        RETURN_LONG(-7); //您的POST权被封
    }
    RETURN_LONG(0);
}



 
/*
 * stiger: countfriends
 */
static PHP_FUNCTION(bbs_countfriends)
{
    char *userid;
    int userid_len;
    int ac = ZEND_NUM_ARGS();
	char fpath[STRLEN];
	struct stat st;

    if (ac != 1 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "s", &userid, &userid_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (userid_len > IDLEN)
        WRONG_PARAM_COUNT;

	sethomefile(fpath, userid, "friends");

    if (stat(fpath, &st) < 0)
        RETURN_FALSE;

	RETURN_LONG(st.st_size / sizeof(struct friends));
}

static int cmpfnames2(char *userid, struct friends *uv)
{
    return !strcasecmp(userid, uv->id);
}

static PHP_FUNCTION(bbs_delete_friend)
{
    char *userid;
    int userid_len;
    int ac = ZEND_NUM_ARGS();
    char buf[STRLEN];
    struct friends fh;
	int deleted;

    if (ac != 1 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "s", &userid, &userid_len) == FAILURE) {
        WRONG_PARAM_COUNT;
	}

    sethomefile(buf, getCurrentUser()->userid, "friends");

    deleted = search_record(buf, &fh, sizeof(fh), (RECORD_FUNC_ARG)cmpfnames2, userid);

    if (deleted > 0) {
        if (delete_record(buf, sizeof(fh), deleted, NULL, NULL) != -1){
			getfriendstr(getCurrentUser(),getSession()->currentuinfo,getSession());
			RETURN_LONG(0);
		} else {
			RETURN_LONG(3);
        }
    } else{
		RETURN_LONG(2);
	}
}

static PHP_FUNCTION(bbs_doforward)
{
    char *board,*filename, *tit, *target;
    int board_len,filename_len,tit_len,target_len;
    boardheader_t bh;
	char fname[STRLEN];
	long big5,noansi;
    struct boardheader *bp;
	char title[512];
	struct userec *u;
    
	if (ZEND_NUM_ARGS() != 6 || zend_parse_parameters(6 TSRMLS_CC, "ssssll", &board, &board_len,&filename, &filename_len, &tit, &tit_len, &target, &target_len, &big5, &noansi) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }

    if( target[0] == 0 )
        RETURN_LONG(-3);
    if( !strchr(target, '@') ){
        if( HAS_PERM(getCurrentUser(), PERM_DENYMAIL) )
            RETURN_LONG(-5);
        if( getuser(target,&u) == 0)
            RETURN_LONG(-6);
        big5=0;
        noansi=0;
    }

    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(-4);
    }
    if (getboardnum(board, &bh) == 0)
        RETURN_LONG(-1); //"错误的讨论区";
    if (!check_read_perm(getCurrentUser(), &bh))
        RETURN_LONG(-2); //您无权阅读本版;

    setbfile(fname, bp->filename, filename);

    if( !file_exist(fname) )
        RETURN_LONG(-7);

    snprintf(title, 511, "%.50s(转寄)", tit);

    if( !strchr(target, '@') ){
        mail_file(getCurrentUser()->userid, fname, u->userid, title,0, NULL);
		RETURN_LONG(1);
	}else{
		if( big5 == 1)
			conv_init(getSession());
		if( bbs_sendmail(fname, title, target, 0, big5, noansi, getSession()) == 0){
			RETURN_LONG(1);
		}else
			RETURN_LONG(-10);
	}
}

static PHP_FUNCTION(bbs_add_friend)
{
    char *userid;
    int userid_len;
	char *exp;
	int exp_len;
    int ac = ZEND_NUM_ARGS();
    char buf[STRLEN];
    struct friends fh;
	struct userec *lookupuser;
	int n;

    if (ac != 2 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "ss", &userid, &userid_len, &exp, &exp_len) == FAILURE) {
        WRONG_PARAM_COUNT;
	}

    if (userid_len == 0) RETURN_LONG(-4);

    memset(&fh, 0, sizeof(fh));
    sethomefile(buf, getCurrentUser()->userid, "friends");

    if ((!HAS_PERM(getCurrentUser(), PERM_SYSOP))
        && (get_num_records(buf, sizeof(struct friends)) >= MAXFRIENDS)) {
		RETURN_LONG(-1);
    }

	if(! getuser(userid,&lookupuser))
		RETURN_LONG(-4);

    n = search_record(buf, &fh, sizeof(fh), (RECORD_FUNC_ARG)cmpfnames2, lookupuser->userid);
    if (n > 0)
		RETURN_LONG(-2);

    strcpy(fh.id, lookupuser->userid);
    strncpy(fh.exp, exp, sizeof(fh.exp)-1);
    fh.exp[sizeof(fh.exp)-1] = '\0';

    n = append_record(buf, &fh, sizeof(friends_t));
    getfriendstr(getCurrentUser(),getSession()->currentuinfo,getSession());
    if (n != -1)
		RETURN_LONG(0);

	RETURN_LONG(-3);
}

/*
 * stiger: getfriends
 */
static PHP_FUNCTION(bbs_getfriends)
{
    char *userid;
    int userid_len;
	struct friends fr;
    int ac = ZEND_NUM_ARGS();
	long start;
	int fd;
	int i=0;
	char fpath[STRLEN];
    zval *element;

    if (ac != 2 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "sl", &userid, &userid_len, &start) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (userid_len > IDLEN)
        WRONG_PARAM_COUNT;

    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }

	i=0;
	sethomefile(fpath, userid, "friends");

	if( (fd=open(fpath, O_RDONLY)) < 0 )
        RETURN_FALSE;
	lseek(fd, sizeof(struct friends)*start, SEEK_CUR);
    while (read(fd, &fr, sizeof(fr)) > 0) {

        MAKE_STD_ZVAL(element);
        array_init(element);

    	add_assoc_string(element, "ID", fr.id, 1);
    	add_assoc_string(element, "EXP", fr.exp, 1);

        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);

		i++;
		if( i>=20)
			break;
    }
    close(fd);
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
static PHP_FUNCTION(bbs_getarticles)
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
    struct boardheader *bp;
    int rows;
    int i;
    zval *element;
    int is_bm;
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
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
    if (getCurrentUser() == NULL) {
        RETURN_FALSE;
    }
    if ((bp = getbcache(board)) == NULL) {
        RETURN_FALSE;
    }
    is_bm = is_BM(bp, getCurrentUser());

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
    brc_initial(getCurrentUser()->userid, bp->filename, getSession());
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
        make_article_flag_array(flags, articles + i, getCurrentUser(), bp->filename, is_bm);
        bbs_make_article_array(element, articles + i, flags, sizeof(flags));
        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
    }
    efree(articles);
}

#ifdef HAVE_WFORUM
/**
 * 获取从start开始的num个版面主题
 * prototype:
 * array bbs_getthreads(char *board, int start, int num,int includeTop);
 *
 * @return array of loaded articles on success,
 *         FALSE on failure.
 * @author roy
 */
static PHP_FUNCTION(bbs_getthreads)
{
    char *board;
    int blen;
    long start,num;
    int total;
    struct boardheader *bp=NULL;
	char dirpath[STRLEN];
    int i,j;
    zval *element;
    int is_bm;
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
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

    if ((i = safe_mmapfile_handle(fd, PROT_READ, MAP_SHARED, (void **) &ptr, &buf.st_size)) != 1) {
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

static PHP_FUNCTION(bbs_get_today_article_num){
    char *board;
    int blen;
    int total;
    struct boardheader *bp;
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

    if ((i = safe_mmapfile_handle(fd, PROT_READ, MAP_SHARED, (void **) &ptr, &buf.st_size)) != 1) {
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
#endif //HAVE_WFORUM

/**
 * Count articles in a board with specific .DIR mode.
 * prototype:
 * int bbs_countarticles(int brdnum, int mode);
 *
 * @return non-negative value on success,
 *         negative value on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_countarticles)
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

#ifdef HAVE_WFORUM

/* long bbs_getthreadnum(long boardNum)
 * get number of threads
 */
static PHP_FUNCTION(bbs_getthreadnum)
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

#endif

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
static PHP_FUNCTION(bbs_get_filename_from_num)
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
 * Get a article records from the article num.
 * prototype:
 * int bbs_get_records_from_num(string dirpath, long num, arrary articles);
 *
 * @return Record index on success,
 *       0 on failure.
 * @author stiger
 */
static PHP_FUNCTION(bbs_get_records_from_num)
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
 * Get some article records from the article id.
 * prototype:
 * int bbs_get_records_from_id(string board, long id, long mode, arrary articles);
 *
 * @return Record index on success,
 *       0 on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_get_records_from_id)
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
 * Get some thread records from an article id.
 * prototype:
 * int bbs_get_threads_from_id(long boardid, long id, long mode, long num);
 *
 * @return Records on success,
 *         FALSE on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_get_threads_from_id)
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




static PHP_FUNCTION(bbs_filteruploadfilename)
{
    char *filename;
    long flen;
    if (zend_parse_parameters(1 TSRMLS_CC, "s/", &filename, &flen) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (!flen) {
        RETURN_FALSE;
    }
    filename = filter_upload_filename(filename);
    RETURN_STRING(filename, 1);
}


/*
 * function bbs_edittitle(string boardName , int id , string newTitle , int dirMode)
 * 修改文章标题
 * @author: windinsn apr 28,2004
 * return 0 : 成功
 *        -1: 版面错误
 *        -2: 该版不能修改文章
 *        -3: 只读讨论区
 *        -4: 文件错误
 *        -5: 封禁中
 *        -6: 无权修改
 *        -7: 被过滤掉
 *	  -8: 当前模式不能编辑标题
 *        -9: 标题过长或为空
 *        -10:system error
 */

static PHP_FUNCTION(bbs_edittitle)
{
	char *board,*title;
	int  board_len,title_len;
	long  id , mode;
	char path[STRLEN];
	char dirpath[STRLEN];
	struct userec *u = NULL;
	struct fileheader f;
	struct fileheader xfh;
	struct boardheader brd;
	int bid,ent,i=0;
	int fd;
	
	int ac = ZEND_NUM_ARGS();
	if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "sls/l", &board, &board_len, &id , &title, &title_len , &mode) == FAILURE) 
		WRONG_PARAM_COUNT;
	
	if ((mode>= DIR_MODE_THREAD) && (mode<= DIR_MODE_WEB_THREAD))
        	RETURN_LONG(-8);
	if (title_len > ARTICLE_TITLE_LEN || title_len == 0)
		RETURN_LONG(-9);
	bid = getboardnum(board, &brd);
	if (bid==0) 
		RETURN_LONG(-1); //版面名称错误
	if (brd.flag&BOARD_GROUP)
	        RETURN_LONG(-1); //二级目录版
	if (!strcmp(brd.filename, "syssecurity") || !strcmp(brd.filename, "junk") || !strcmp(brd.filename, "deleted"))  
		RETURN_LONG(-2); //不允许修改文章
	if (true == checkreadonly(brd.filename))
		RETURN_LONG(-3); //只读讨论区
	if ((u = getCurrentUser())==NULL)
		RETURN_LONG(-10); //无法获得当前登录用户
	
	if (mode == DIR_MODE_DIGEST)
		setbdir(DIR_MODE_DIGEST, dirpath, brd.filename);
	else
		setbdir(DIR_MODE_NORMAL, dirpath, brd.filename);
	
	if ((fd = open(dirpath, O_RDWR, 0644)) < 0)
		RETURN_LONG(-10);
	if (!get_records_from_id(fd,id,&f,1,&ent))
	{
		close(fd);
		RETURN_LONG(-4); //无法取得文件记录
	}
	close(fd);
	if (!HAS_PERM(u,PERM_SYSOP)) //权限检查
	{
		if (!haspostperm(u, brd.filename))
	        	RETURN_LONG(-6);
	        if (deny_me(u->userid, brd.filename))
	        	RETURN_LONG(-5);
	        if (!chk_currBM(brd.BM, u))
	        {
	        	if (!isowner(u, &f))
		            RETURN_LONG(-6); //他人文章
		}
	}
    if (title_len >= ARTICLE_TITLE_LEN) {
        title[ARTICLE_TITLE_LEN - 1] = '\0';
    }
    filter_control_char(title);
	if (!strcmp(title,f.title)) //无需修改
		RETURN_LONG(0);
#ifdef FILTER
	if (check_badword_str(title, strlen(title), getSession()))
		RETURN_LONG(-7);
#endif
	setbfile(path, brd.filename, f.filename);
	if (add_edit_mark(path, 2, title, getSession()) != 1)
		RETURN_LONG(-10);
	/* update .DIR START */
	strcpy(f.title, title);
	if (mode == DIR_MODE_ZHIDING)
	{
		setbdir(DIR_MODE_ZHIDING, dirpath, brd.filename);
		ent = get_num_records(dirpath,sizeof(struct fileheader));
        	fd = open(dirpath, O_RDONLY, 0);
        }
	else
	{
		if (mode == DIR_MODE_DIGEST)
			setbdir(DIR_MODE_DIGEST, dirpath, brd.filename);
		else
			setbdir(DIR_MODE_NORMAL, dirpath, brd.filename);
		fd = open(dirpath, O_RDONLY, 0);
	}
	if (fd!=-1) 
	{
		for (i = ent; i > 0; i--)
		{
			if (0 == get_record_handle(fd, &xfh, sizeof(xfh), i)) 
			{
                		if (0 == strcmp(xfh.filename, f.filename)) 
                		{
                			ent = i;
                			break;
                		}
                	}
		}
		if (mode == DIR_MODE_ZHIDING)
		{
                	if (i!=0) 
                    		substitute_record(dirpath, &f, sizeof(f), ent);
               		board_update_toptitle(bid, true);
        	}
        	else
        	{
        		if (i!=0) 
                		substitute_record(dirpath, &f, sizeof(f), ent);
		}
	}
	close(fd);
	if (0 == i)
            RETURN_LONG(-10);
        if(mode != DIR_MODE_ORIGIN && f.id == f.groupid)
        {
		if( setboardorigin(board, -1) )
		{
			board_regenspecial(brd.filename,DIR_MODE_ORIGIN,NULL);
		}
		else
		{
			char olddirect[PATHLEN];
	    		setbdir(DIR_MODE_ORIGIN, olddirect, brd.filename);
			if ((fd = open(olddirect, O_RDWR, 0644)) >= 0)
			{
				struct fileheader tmpfh;
				if (get_records_from_id(fd, f.id, &tmpfh, 1, &ent) == 0)
				{
					close(fd);
				}
				else
				{
					close(fd);
   	                		substitute_record(olddirect, &f, sizeof(f), ent);
				}
			}
		}
	}
	setboardtitle(brd.filename, 1);	
	/* update .DIR END   */
	RETURN_LONG(0);
}

static PHP_FUNCTION(bbs_checkbadword)
{
    char *str;
    int  str_len;
    
    int ac = ZEND_NUM_ARGS();
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
#ifdef FILTER
	if (check_badword_str(str, strlen(str),getSession()))
		RETURN_TRUE;
#endif    
    RETURN_FALSE;
}


/*  function bbs_updatearticle(string boardName, string filename ,string text)  
 *  更新编辑文章
 *
 */
static PHP_FUNCTION(bbs_updatearticle)
{
	char *boardName, *filename, *content;
	int blen, flen, clen;
    FILE *fin;
    FILE *fout;
    char infile[80], outfile[80];
    char buf2[256];
    int i;
    boardheader_t *bp;
    time_t now;
    int asize;
    /*int filtered = 0;*/

	int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments 
     */
    
	if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sss/", &boardName, &blen, &filename, &flen, &content, &clen) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
   if ( (bp=getbcache(boardName))==0) {
   		RETURN_LONG(-1);
	}		

    if (clen == 0) content = "";
    else content = unix_string(content);

#ifdef FILTER
    if (check_badword_str(content, strlen(content),getSession())) {
        RETURN_LONG(-1); //修改文章失败，文章可能含有不恰当内容.
    }
#endif

    setbfile(infile, bp->filename, filename);
    sprintf(outfile, "tmp/%s.%d.editpost", getCurrentUser()->userid, getpid());
    if ((fin = fopen(infile, "r")) == NULL)
        RETURN_LONG(-10);
    if ((fout = fopen(outfile, "w")) == NULL) {
        fclose(fin);
        RETURN_LONG(-10);
    }
    for (i = 0; i < 4; i++) {
        fgets(buf2, sizeof(buf2), fin);
		if ((i==0) && (strncmp(buf2,"发信人",6)!=0)) {
			break;
		}
        fprintf(fout, "%s", buf2);
    }
    if (clen>0) fprintf(fout, "%s", content);
    now = time(0);
    fprintf(fout, "\033[36m※ 修改:·%s 於 %15.15s 修改本文·[FROM: %s]\033[m\n", getCurrentUser()->userid, ctime(&now) + 4, SHOW_USERIP(getCurrentUser(), getSession()->fromhost));
    while ((asize = -attach_fgets(buf2, sizeof(buf2), fin)) != 0) {
        if (asize <= 0) {
            if (Origin2(buf2)) {
                fprintf(fout, "%s", buf2);
            }
        } else {
            put_attach(fin, fout, asize);
        }
    }
    fclose(fin);
    fclose(fout);
    f_mv(outfile, infile);
    RETURN_LONG(0);
}



static PHP_FUNCTION(bbs_brcaddread)
{
	char *board;
	int blen;
    long fid;
	boardheader_t* bp;

    if (zend_parse_parameters(2 TSRMLS_CC, "sl", &board, &blen, &fid) != SUCCESS)
        WRONG_PARAM_COUNT;
	if ((bp=getbcache(board))==0){
		RETURN_NULL();
	}
#ifdef HAVE_BRC_CONTROL
	brc_initial(getCurrentUser()->userid, bp->filename, getSession());
	brc_add_read(fid, getSession());
	brc_update(getCurrentUser()->userid, getSession());
    /*brc_addreaddirectly(getcurrentuser()->userid, boardnum, fid);*/
#endif
    RETURN_NULL();
}

/**
 * 清除版面未读标记 
 * bbs_brcclear(string board)
 * windinsn
 * return true/false
 */
static PHP_FUNCTION(bbs_brcclear)
{
    char *board;
    int  board_len;
    struct boardheader bh;
    struct userec *u;
        
    int ac = ZEND_NUM_ARGS();
	
	if (ac != 1 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "s" , &board, &board_len) == FAILURE)
		WRONG_PARAM_COUNT;
		
    u = getCurrentUser();
    if (!u)
        RETURN_FALSE;
        
    if (getboardnum(board,&bh) == 0)
        RETURN_FALSE;
    if (!check_read_perm(u, &bh))
        RETURN_FALSE;
    if (!strcmp(u->userid,"guest"))
        RETURN_TRUE;
#ifdef HAVE_BRC_CONTROL
    brc_initial(u->userid, board, getSession());
    brc_clear(getSession());
    brc_update(u->userid, getSession());
#endif
    RETURN_TRUE;
}



/**
 * set currentuinfo for user.
 * prototype:
 * string bbs_update_uinfo(string field, value)
 * @return TRUE on success,
 *       FALSE on failure.
 * @author kcn
 */
static PHP_FUNCTION(bbs_update_uinfo)
{
    zval *value;
    char *field;
    int field_len;
    int ac = ZEND_NUM_ARGS();

    if (ac != 2 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "sz", &field, &field_len, &value) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    if (!strcmp(field, "invisible")) {
/*	    conver_to_boolean_ex(&value);
 *	    */
        getSession()->currentuinfo->invisible = Z_LVAL_P(value);
    }
    RETURN_LONG(0);
}

/**
 * set password for user.
 * prototype:
 * string bbs_setpassword(string userid, string password)
 * @if userid=="" then user=getCurrentUser()
 * @return TRUE on success,
 *       FALSE on failure.
 * @author kcn
 */
static PHP_FUNCTION(bbs_setpassword)
{
    char *userid;
    int userid_len;
    char *password;
    int password_len;
    int ac = ZEND_NUM_ARGS();
    struct userec *user;

    if (ac != 2 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "ss", &userid, &userid_len, &password, &password_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (userid != 0) {
        if (getuser(userid, &user) == 0)
            RETURN_FALSE;
    } else
        user = getCurrentUser();
    if (setpasswd(password, user) != 1)
        RETURN_FALSE;
    RETURN_TRUE;
}

static int ext_init = 0; /* Don't access this variable directly, 
						  use the following helper routines */

static int get_initialized()
{
	return ext_init;
}

static int initialize_ext()
{
    chdir(BBSHOME);
    resolve_ucache();
    resolve_utmp();
    resolve_boards();
    init_bbslog();
    www_data_init();
	ext_init = 1;

	return ext_init;
}

/**
 * return status flag of initialization of the extension.
 * prototype:
 * bool bbs_ext_initialized()
 *
 * @return TRUE if the extension had been initialized,
 *       FALSE otherwise.
 * @author flyriver
 */
PHP_FUNCTION(bbs_ext_initialized)
{
	if (!get_initialized())
        RETURN_FALSE;
    RETURN_TRUE;
}

/**
 * initialize the extension.
 * prototype:
 * bool bbs_init_ext()
 *
 * @return TRUE on success,
 *       FALSE on failure.
 * @author flyriver
 */
PHP_FUNCTION(bbs_init_ext)
{
	int ret = 0;
	if (!get_initialized())
        ret = initialize_ext();
	if (ret)
	{
    	RETURN_TRUE;
	}
	else
	{
		RETURN_FALSE;
	}
}

PHP_MINIT_FUNCTION(smth_bbs)
{
    /*
    zval *bbs_home;
    zval *bbs_full_name;

    MAKE_STD_ZVAL(bbs_home);
    ZVAL_STRING(bbs_home, BBSHOME, 1);
    MAKE_STD_ZVAL(bbs_full_name);
    ZVAL_STRING(bbs_full_name, BBS_FULL_NAME, 1);
    PHP_SET_SYMBOL(&EG(symbol_table), "BBS_HOME", bbs_home);
    PHP_SET_SYMBOL(&EG(symbol_table), "BBS_FULL_NAME", bbs_full_name);
    */
	char old_cwd[256];
	getcwd(old_cwd, sizeof(old_cwd));
	chdir(BBSHOME);
    REGISTER_STRINGL_CONSTANT("BBS_HOME",BBSHOME,strlen(BBSHOME),CONST_CS | CONST_PERSISTENT);
    REGISTER_STRINGL_CONSTANT("BBS_FULL_NAME",BBS_FULL_NAME,strlen(BBS_FULL_NAME),CONST_CS | CONST_PERSISTENT);
#ifdef SQUID_ACCL
    REGISTER_LONG_CONSTANT("SQUID_ACCL", 1, CONST_CS | CONST_PERSISTENT);
#else
    REGISTER_LONG_CONSTANT("SQUID_ACCL", 0, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef BUILD_PHP_EXTENSION
    REGISTER_LONG_CONSTANT("BUILD_PHP_EXTENSION", 1, CONST_CS | CONST_PERSISTENT);
#else
    REGISTER_LONG_CONSTANT("BUILD_PHP_EXTENSION", 0, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef HAVE_WFORUM
    REGISTER_LONG_CONSTANT("BBS_WFORUM", 1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_DEF_SOUNDMSG", DEF_SOUNDMSG, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_DEF_SHOWDETAILUSERDATA", DEF_SHOWDETAILUSERDATA, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_DEF_SHOWREALUSERDATA", DEF_SHOWREALUSERDATA, CONST_CS | CONST_PERSISTENT);
#else
    REGISTER_LONG_CONSTANT("BBS_WFORUM", 0, CONST_CS | CONST_PERSISTENT);
#endif
    REGISTER_LONG_CONSTANT("BBS_QUOTED_LINES" , QUOTED_LINES, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_QUOTE_LEV" , QUOTELEV, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_ACTIVATED_FLAG" , ACTIVATED_FLAG, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PCORP_FLAG" , PCORP_FLAG, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_LOGINOK" , PERM_LOGINOK , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_BASIC" , PERM_BASIC , CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_POSTMASK", PERM_POSTMASK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_NOZAP", PERM_NOZAP, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_BOARDS", PERM_BOARDS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_CLOAK", PERM_CLOAK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_BMAMANGER", PERM_BMAMANGER, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_SYSOP", PERM_SYSOP, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_ATTACH", BOARD_ATTACH, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_ANNONY", BOARD_ANNONY, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_JUNK", BOARD_JUNK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_OUTFLAG", BOARD_OUTFLAG, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_CLUB_READ", BOARD_CLUB_READ, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_CLUB_WRITE", BOARD_CLUB_WRITE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_CLUB_HIDE", BOARD_CLUB_HIDE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_GROUP", BOARD_GROUP, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_MAXATTACHMENTCOUNT", MAXATTACHMENTCOUNT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_MAXATTACHMENTSIZE", MAXATTACHMENTSIZE, CONST_CS | CONST_PERSISTENT);
	chdir(old_cwd);
#ifdef DEBUG
    zend_error(E_WARNING, "module init");
#endif
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(smth_bbs)
{
	if (get_initialized())
	{
		www_data_detach();
		detach_utmp();
		detach_boards();
		detach_ucache();
		detach_publicshm();
	}

#ifdef DEBUG
    zend_error(E_WARNING, "module shutdown");
#endif
    return SUCCESS;
}

PHP_RINIT_FUNCTION(smth_bbs)
{
    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    setcurrentuser(NULL, 0);
    setcurrentuinfo(NULL, -1);
#ifdef DEBUG
    zend_error(E_WARNING, "request init:%d %x", getpid(), getSession()->currentuinfo);
#endif
	getSession()->zapbuf=NULL;
	reset_output_buffer();
#ifdef SMS_SUPPORT
	getSession()->smsbuf=NULL;
	getSession()->smsresult=0;
#endif
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(smth_bbs)
{
#ifdef DEBUG
    zend_error(E_WARNING, "request shutdown");
#endif
    chdir(old_pwd);

#if defined(HAVE_BRC_CONTROL) && USE_TMPFS == 1
    //if (getCurrentUser() && getCurrentUser()->userid && (getCurrentUser()->userid[0]) )
	//free_brc_cache( getCurrentUser()->userid );
#endif
    setcurrentuser(NULL, 0);
    setcurrentuinfo(NULL, -1);
    return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION
 *  */
PHP_MINFO_FUNCTION(smth_bbs)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "smth_bbs support", "enabled");
    php_info_print_table_end();
}
/**
 * Function: post a new mail
 *  rototype:
 * int bbs_postmail(string targetid,string title,string content,long sig, long backup);
 *
 *  @return the result
 *  	0 -- success
 *		<0 error
 *  @author roy
 */
 
static PHP_FUNCTION(bbs_postmail){
	char* targetID, *title, *content;
	int  idLen, tLen,cLen;
    long backup,sig;
	int ac = ZEND_NUM_ARGS();
	char mail_title[80];
    FILE *fp;
    char fname[PATHLEN], filepath[PATHLEN], sent_filepath[PATHLEN];
    struct fileheader header;
    struct stat st;
    struct userec *touser;      /*peregrine for updating used space */

    if (ac != 5 || zend_parse_parameters(5 TSRMLS_CC, "ss/s/ll", &targetID, &idLen,&title,&tLen,&content,&cLen,&sig,&backup) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}
    
    getuser(targetID, &touser);
    if (touser == NULL) 
		RETURN_LONG(-100);//can't find user

    if (!canIsend2(getCurrentUser(), targetID)) {
        RETURN_LONG(-3);
    }

    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP) && chkusermail(touser)) {    /*Haohamru.99.4.05 */
        RETURN_LONG(-4);
    }

	strcpy(targetID, touser->userid);
    filter_control_char(title);
	if (title[0] == 0)
        strcpy(mail_title,"没主题");
	else 
		strncpy(mail_title,title,79);
	mail_title[79]=0;
    
    bzero(&header, sizeof(header));
    strcpy(header.owner, getCurrentUser()->userid);
    strncpy(header.title, mail_title, ARTICLE_TITLE_LEN - 1);
	header.title[ARTICLE_TITLE_LEN - 1] = '\0';
    setmailpath(filepath, targetID);
    if (stat(filepath, &st) == -1) {
        if (mkdir(filepath, 0755) == -1)
            RETURN_LONG(-2);
    } else {
        if (!(st.st_mode & S_IFDIR))
            RETURN_LONG(-2);
    }
    if (GET_MAILFILENAME(fname, filepath) < 0)
        RETURN_LONG(-2);
    strcpy(header.filename, fname);
    setmailfile(filepath, targetID, fname);

    fp = fopen(filepath, "w");
    if (fp == NULL)
        RETURN_LONG(-2);
    write_header(fp, getCurrentUser(), 1, NULL, mail_title, 0, 0, getSession());
    if (cLen>0) {
        f_append(fp, unix_string(content));
    }
    getCurrentUser()->signature = sig;
    if (sig < 0) {
        struct userdata ud;
        read_userdata(getCurrentUser()->userid, &ud);
        if (ud.signum > 0) {
            sig = 1 + (int) (((double)ud.signum) * rand() / (RAND_MAX + 1.0)); //(rand() % ud.signum) + 1;
        } else sig = 0;
    }
    addsignature(fp, getCurrentUser(), sig);
    fputc('\n', fp);
    fclose(fp);
    
    if (stat(filepath, &st) != -1)
        header.eff_size = st.st_size;
    setmailfile(fname, targetID, ".DIR");
    if (append_record(fname, &header, sizeof(header)) == -1)
        RETURN_LONG(-6);
    touser->usedspace += header.eff_size;
	setmailcheck(targetID);
	    
   /* 添加Log Bigman: 2003.4.7 */
    newbbslog(BBSLOG_USER, "mailed(www) %s %s", targetID, mail_title);

    if (backup) {
        strcpy(header.owner, targetID);
        setmailpath(sent_filepath, getCurrentUser()->userid);
        if (GET_MAILFILENAME(fname, sent_filepath) < 0) {
            RETURN_LONG(-7);
        }
        strcpy(header.filename, fname);
        setmailfile(sent_filepath, getCurrentUser()->userid, fname);

        f_cp(filepath, sent_filepath, 0);
        if (stat(sent_filepath, &st) != -1) {
            getCurrentUser()->usedspace += st.st_size;
            header.eff_size = st.st_size;
        } else {
            RETURN_LONG(-7);
        }
        header.accessed[0] |= FILE_READ;
        setmailfile(fname, getCurrentUser()->userid, ".SENT");
        if (append_record(fname, &header, sizeof(header)) == -1)
            RETURN_LONG(-7);
        newbbslog(BBSLOG_USER, "mailed(www) %s ", getCurrentUser()->userid);
    }
	RETURN_LONG(0);
}


static PHP_FUNCTION(bbs_modify_nick)
{
    struct userec newinfo;
    int unum;

    char* username;
    int username_len;
    int ac = ZEND_NUM_ARGS();
    long bTmp = 1;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &username, &username_len) == FAILURE)
        if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &username, &username_len, &bTmp) == FAILURE)
    {
            WRONG_PARAM_COUNT;
    }
    filter_control_char(username);
	if( username_len >= NAMELEN)
       RETURN_LONG(-1);

    if (!bTmp) {
        if( (unum = searchuser(getCurrentUser()->userid))==0)
            RETURN_LONG(-1);
        memcpy(&newinfo, getCurrentUser(), sizeof(struct userec));
        if (strcmp(newinfo.username, username)) {
            strcpy(newinfo.username, username);
        	update_user(&newinfo, unum, 1);
        }
    }

	strcpy(getSession()->currentuinfo->username, username);
    UPDATE_UTMP_STR(username, (*(getSession()->currentuinfo)));

	RETURN_LONG(0);
}

static PHP_FUNCTION(bbs_recalc_sig)
{
	struct userec newinfo;
	int unum;
	int sign;

    if( (unum = searchuser(getCurrentUser()->userid))==0)
		RETURN_LONG(-1);
	memcpy(&newinfo, getCurrentUser(), sizeof(struct userec));
    
	if( read_user_memo(getCurrentUser()->userid, &(getSession()->currentmemo)) <= 0) RETURN_LONG(-2);

    (getSession()->currentmemo)->ud.signum = calc_numofsig(getCurrentUser()->userid);
	sign = (getSession()->currentmemo)->ud.signum;

    write_userdata(getCurrentUser()->userid,&((getSession()->currentmemo)->ud) );
	end_mmapfile((getSession()->currentmemo), sizeof(struct usermemo), -1);


	if(sign>0 && newinfo.signature>0)
		RETURN_LONG(1);

	if(sign<=0 && newinfo.signature==0)
		RETURN_LONG(2);

	if(sign > 0)
    	newinfo.signature = 1;
	else
		newinfo.signature = 0;

	update_user(&newinfo, unum, 1);

	RETURN_LONG(3);

}


/* bbsinfo.php, stiger */
static PHP_FUNCTION(bbs_modify_info)
{
    char* username;
    int username_len;
    char* realname;
    int realname_len;
    char* address;
    int address_len;
    char* email;
    int email_len;

    struct userdata ud;
	struct userec newinfo;
	int unum;
    int ac = ZEND_NUM_ARGS();

    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "ssss", &username, &username_len,&realname,&realname_len,&address,&address_len,&email,&email_len) == FAILURE)
    {
            WRONG_PARAM_COUNT;
    }

    filter_control_char(username);
    filter_control_char(realname);
    filter_control_char(address);
    filter_control_char(email);

    if(username_len >= NAMELEN || realname_len >= NAMELEN || address_len >= STRLEN || email_len>= STRLEN)
       RETURN_LONG(-1);

    memset(&ud,0,sizeof(ud));
	if( read_user_memo(getCurrentUser()->userid, &(getSession()->currentmemo)) <= 0) RETURN_LONG(-2);

    if(read_userdata(getCurrentUser()->userid,&ud) < 0)RETURN_LONG(-2);

    strncpy(ud.realname, realname, NAMELEN);
    strncpy(ud.address,address,STRLEN);
    strncpy(ud.email,email,STRLEN);

	memcpy(&((getSession()->currentmemo)->ud), &ud, sizeof(ud));
	end_mmapfile((getSession()->currentmemo), sizeof(struct usermemo), -1);

    if(write_userdata(getCurrentUser()->userid,&ud) < 0)RETURN_LONG(-2);

                if( (unum = searchuser(getCurrentUser()->userid))==0)
       				RETURN_LONG(-1);
				memcpy(&newinfo, getCurrentUser(), sizeof(struct userec));
                if (strcmp(newinfo.username, username)) {

                    strcpy(newinfo.username, username);
					update_user(&newinfo, unum, 1);

					strcpy(getSession()->currentuinfo->username, username);
                    UPDATE_UTMP_STR(username, (*(getSession()->currentuinfo)));
                }

	bbslog("user","%s","change user info from www");

    RETURN_LONG(0);
}




/**
 *  Function: 根据注册姓名和email生成新的密码.如果用户名为空,则生成一个密码.
 *   string bbs_findpwd_check(string userid,string realname,string email);
 *
 *   if failed. reaturn NULL string; or return new password.
 *              by binxun
 */
static PHP_FUNCTION(bbs_findpwd_check)
{
    char*   userid,
	        *realname,
            *email;
	int     userid_len,
	        realname_len,
			email_len;
	char    pwd[30];
    struct userdata ud;
	struct userec* uc;

	int ac = ZEND_NUM_ARGS();
    chdir(BBSHOME);

    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sss", &userid,&userid_len,&realname,&realname_len,&email,&email_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	pwd[0] = 0;
    if(userid_len > IDLEN)RETURN_LONG(1);

	//生成新密码
	srand(time(NULL));
	sprintf(pwd,"%d",rand());

	if(userid_len > 0){
        if(getuser(userid,&uc) == 0)RETURN_LONG(3);
	    if(read_userdata(userid,&ud)<0)RETURN_LONG(4);

	    if(!strncmp(userid,ud.userid,IDLEN) && !strncmp(email,ud.email,STRLEN))
	    {
		    setpasswd(pwd,uc);
	    }
	    else
	        RETURN_LONG(5);
	}

    RETURN_STRING(pwd,1);
}


static int cmpuser(a, b)
    uinfo_t *a, *b;
{
    char id1[80], id2[80];

    sprintf(id1, "%d%s", !isfriend(a->userid), a->userid);
    sprintf(id2, "%d%s", !isfriend(b->userid), b->userid);
    return strcasecmp(id1, id2);
}

typedef struct _frienduserlistarg{
    int count;
    uinfo_t** user_record;
} frienduserlistarg;

static int full_utmp_friend(struct user_info *uentp, frienduserlistarg *pful)
{
    if (!uentp->active || !uentp->pid) {
        return 0;
    }
    if (!HAS_PERM(getCurrentUser(), PERM_SEECLOAK) && uentp->invisible && strcmp(uentp->userid, getCurrentUser()->userid)) {  /*Haohmaru.99.4.24.让隐身者能看见自己 */
        return 0;
    }
    if (!myfriend(uentp->uid, NULL, getSession())) {
        return 0;
    }
    if (pful->count < MAXFRIENDS) {
        pful->user_record[pful->count] = uentp;
        pful->count++;
    }
    return COUNT;
}

static int fill_friendlist(int* range, uinfo_t** user_record)
{
    int i;
    frienduserlistarg ful;
    struct user_info *u;

    ful.count = 0;
    ful.user_record = user_record;
    u = getSession()->currentuinfo;
    for (i = 0; i < u->friendsnum; i++) {
        if (u->friends_uid[i])
            apply_utmpuid((APPLY_UTMP_FUNC) full_utmp_friend, u->friends_uid[i], &ful);
    }
    *range = ful.count;
    return ful.count == 0 ? -1 : 1;
}

/**
 *  Function: 返回当前在线好友名单
 *   user_info bbs_getonlinefriends();
 *
 *  Return: user_info 结构数组 
 *  by binxun
 */
static PHP_FUNCTION(bbs_getonlinefriends)
{
    int i = 0,total = 0;
    uinfo_t* x;
    uinfo_t user[MAXFRIENDS]; //local copy from shm, for sorting, etc.
    zval* element;
    int range;
    uinfo_t *usr[MAXFRIENDS]; //指向共享内存内容
    struct userec *lookupuser;

    int ac = ZEND_NUM_ARGS();

    if (ac != 0) {
        WRONG_PARAM_COUNT;
    }

    fill_friendlist(&range, usr);
    
    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }
    //if(!usr) RETURN_LONG(0);
    
    for (i = 0; i < range; i++) {
        x = usr[i];
        if (x == NULL)continue;
        if (x->active == 0) continue;
        if (x->invisible && !HAS_PERM(getCurrentUser(), PERM_SEECLOAK)) continue;
	
        memcpy(&user[total], x , sizeof(uinfo_t));
    	total++;
        if(total >= MAXFRIENDS) break;
    }
    if(total == 0) RETURN_LONG(0);
    
    qsort(user, total, sizeof(uinfo_t), cmpuser);	
	
    for (i = 0; i < total; i++) {
        MAKE_STD_ZVAL(element);
        array_init(element);
        add_assoc_bool ( element, "invisible", user[i].invisible );
        add_assoc_bool ( element, "isfriend", isfriend(user[i].userid) );
        add_assoc_long ( element, "idle", (long)(time(0) - user[i].freshtime)/60 );
        add_assoc_string ( element, "userid", user[i].userid, 1 );       
        add_assoc_string ( element, "username", user[i].username, 1 );   
        if( getuser(user[i].userid, &lookupuser) == 0 ) lookupuser=NULL;
        add_assoc_string ( element, "userfrom", HAS_PERM(getCurrentUser(), PERM_SYSOP)?user[i].from:SHOW_USERIP(lookupuser, user[i].from), 1 );
        add_assoc_string ( element, "mode", ModeType(user[i].mode), 1 );
        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
	}
}		
/**
 * del board article
 * prototype:
 * int bbs_delfile(char* board, char* filename);
 *
 *  @return the result
 *  	0 -- success, -1 -- no perm
 *  	-2 -- wrong parameter
 *  @author binxun
 */
static PHP_FUNCTION(bbs_delfile)
{
	FILE *fp;
    boardheader_t *brd;
    struct fileheader f;
    struct userec *u = NULL;
    char dir[80], path[80];
	long result = 0;

	char* board;
	char* file;
	int board_len,file_len;
    int num = 0;

	int ac = ZEND_NUM_ARGS();

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ss", &board, &board_len,&file,&file_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	u = getCurrentUser();
	brd = getbcache(board);

	if (VALID_FILENAME(file) < 0)
        RETURN_LONG(-2);
    if (brd == 0)
        RETURN_LONG(-2);
    if (!haspostperm(u, board))
        RETURN_LONG(-2);

	setbdir(DIR_MODE_NORMAL, dir, brd->filename);
	setbfile(path, brd->filename, file);
	/*
	 * TODO: Improve the following block of codes.
	 */
    fp = fopen(dir, "r");
    if (fp == 0)
        RETURN_LONG(-2);
	while (1) {
		if (fread(&f, sizeof(struct fileheader), 1, fp) <= 0)
			break;
		if (!strcmp(f.filename, file)) {
                        if(del_post(num + 1, &f, dir, brd->filename) != 0)
				result = -1;
			else
				result = 0;
			break;
		}
		num++;
    }
    fclose(fp);

	RETURN_LONG(result);
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
static PHP_FUNCTION(bbs_normalboard)
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
static PHP_FUNCTION(bbs_searchboard)
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
static PHP_FUNCTION(bbs_useronboard)
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

    
    bid = getbnum(board);
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
                    MAKE_STD_ZVAL(element);
                    array_init(element);
                    add_assoc_string(element,"USERID","_wwwguest",1);
                    add_assoc_string(element,"HOST",inet_ntoa(wwwguest_shm->guest_entry[i].fromip),1);
                    zend_hash_index_update(Z_ARRVAL_P(users),j,(void*) &element, sizeof(zval*), NULL);
                    j ++;
                }
            }
    }
    
    RETURN_LONG(j);  
}




static PHP_FUNCTION(bbs_set_onboard)
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




/**
 * 转贴文章
 * int bbs_docross(string board,int id,string target,int out_go);
 * return  0 :seccess
 *         -1:源版面不存在
 *         -2:目标版面不存在
 *         -3:目标版面只读
 *         -4:无发文权限
 *         -5:被封禁
 *         -6:文件记录不存在
 *         -7:已经被转载过了
 *         -8:不能在板内转载
 *         -9:目标版面不支持附件
 *         -10:system error
 * @author: windinsn
 */
static PHP_FUNCTION(bbs_docross)
{
    char *board,*target;
    int  board_len,target_len;
    long  id,out_go;
    struct boardheader *src_bp;
	struct boardheader *dst_bp;
	struct fileheader f;
    int  ent;
    int  fd;
    struct userec *u = NULL;
    char path[256],ispost[10];
    
    int ac = ZEND_NUM_ARGS();
    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "slsl", &board, &board_len, &id, &target, &target_len, &out_go) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	u = getCurrentUser();
	src_bp = getbcache(board);
	if (src_bp == NULL)
	    RETURN_LONG(-1);
	strcpy(board, src_bp->filename);
	if(!check_read_perm(u, src_bp))
		RETURN_LONG(-1);
    
    dst_bp = getbcache(target);
    if (dst_bp == NULL)
        RETURN_LONG(-2);
    strcpy(target, dst_bp->filename);

#ifndef NINE_BUILD    
    if (!strcmp(board,target))
        RETURN_LONG(-8);
#endif
    
    if(!check_read_perm(u, dst_bp))
		RETURN_LONG(-2);
    if (true == checkreadonly(target))
		RETURN_LONG(-3); //只读讨论区
    if (!HAS_PERM(u,PERM_SYSOP)) { //权限检查
	    if (!haspostperm(u, target))
	        	RETURN_LONG(-4);
	    if (deny_me(u->userid, target))
	        	RETURN_LONG(-5);
	}
	
	setbdir(DIR_MODE_NORMAL, path, board);
	if ((fd = open(path, O_RDWR, 0644)) < 0)
		RETURN_LONG(-10);
    if (!get_records_from_id(fd,id,&f,1,&ent)) {
		close(fd);
		RETURN_LONG(-6); //无法取得文件记录
	}
	close(fd);
#ifndef NINE_BUILD
    if ((f.accessed[0] & FILE_FORWARDED) && !HAS_PERM(u, PERM_SYSOP)) 
        RETURN_LONG(-7);
#endif	
	
	if ((f.attachment!=0)&&!(dst_bp->flag&BOARD_ATTACH)) 
        RETURN_LONG(-9);
	
	strcpy(ispost ,((dst_bp->flag & BOARD_OUTFLAG) && out_go)?"s":"l");
	setbfile(path, board, f.filename);
	if (post_cross(u, target, board, f.title, path, 0, 0, ispost[0], 0, getSession()) == -1)
	    RETURN_LONG(-10);
    RETURN_LONG(0);
}

/**
 * int bbs_docommend(string board, int id, int confirmed);
 *
 * @param confirmed: when set false, only test if can recommend
 *
 * return 0: no error
 *       -1: 无权限
 *       -2: 源版面不存在
 *       -3: 文件记录不存在
 *       -4: 本文章已经推荐过
 *       -5: 内部版面文章
 *       -6: 被停止了推荐的权力
 *       -7: 推荐出错
 *       -10: system err
 *
 * @author atppp
 */
static PHP_FUNCTION(bbs_docommend)
{
#ifdef COMMEND_ARTICLE
    char *board;
    int  board_len;
    long  id,confirmed;
    struct userec *u;
    struct boardheader *src_bp, *commend_bp;
    struct fileheader fileinfo;
    int  ent;
    int  fd;
    char path[256];

    int ac = ZEND_NUM_ARGS();
    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sll", &board, &board_len, &id, &confirmed) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    u = getCurrentUser();

    src_bp = getbcache(board);
    if (src_bp == NULL)
        RETURN_LONG(-1);
    strcpy(board, src_bp->filename);
    if(!check_read_perm(u, src_bp))
        RETURN_LONG(-2);

    setbdir(DIR_MODE_NORMAL, path, board);
    if ((fd = open(path, O_RDWR, 0644)) < 0)
        RETURN_LONG(-10);
    if (!get_records_from_id(fd,id,&fileinfo,1,&ent)) {
        close(fd);
        RETURN_LONG(-3); //无法取得文件记录
    }
    close(fd);

    commend_bp = getbcache(COMMEND_ARTICLE);
    if (commend_bp == NULL) {
        RETURN_LONG(-7);
    }
    if (!is_BM(commend_bp, u) && !is_BM(src_bp, u)) {
        if (strcmp(u->userid, fileinfo.owner))
            RETURN_LONG(-1);
    }
    if (!HAS_PERM(getCurrentUser(), PERM_LOGINOK)) {
        RETURN_LONG(-1);
    }
    if ((fileinfo.accessed[1] & FILE_COMMEND) && !HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        RETURN_LONG(-4);
    }
    if( ! normal_board(board) ){
        RETURN_LONG(-5);
    }
    if ( deny_me(u->userid, COMMEND_ARTICLE) ) {
        RETURN_LONG(-6);
    }
    if (confirmed) {
        if (post_commend(u, board, &fileinfo ) == -1) {
            RETURN_LONG(-7);
        } else {
            struct write_dir_arg dirarg;
            struct fileheader data;
            data.accessed[1] = FILE_COMMEND;
            init_write_dir_arg(&dirarg);
            dirarg.filename = path;  
            dirarg.ent = ent;
            change_post_flag(&dirarg,DIR_MODE_NORMAL,src_bp, &fileinfo, FILE_COMMEND_FLAG, &data,false,getSession());
            free_write_dir_arg(&dirarg);
        }
    }
    RETURN_LONG(0);
#else
    RETURN_LONG(-1);
#endif
}

/**
 * int bbs_bmmanage(string board,int id,int mode,int zhiding)
 * which is used to switch article's flag
 * $mode defined in funcs.php
 * $mode = 0: do nth;
 *         1: del;
 *         2: mark;
 *         3: digest;
 *         4: noreplay;
 *         5: zhiding;
 *         6: undel		:: add by pig2532 on 2005.12.19 ::
 * return 0 : success;
 *        -1: board is NOT exist
 *        -2: do NOT have permission
 *        -3: can NOT load dir file
 *        -4: can NOT find article
 *        -9: system error
 */
static PHP_FUNCTION(bbs_bmmanage)
{
    char *board;
    int  board_len;
    long  id,mode,zhiding;
    struct boardheader* bh;
    int ret;
    char dir[STRLEN];
    int ent;
    int fd, bid;
    struct fileheader f;
    FILE *fp;
    
    /* if in DELETED mode, num is transfered instead of id at parameter "id" */
    int ac = ZEND_NUM_ARGS();
    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "slll", &board, &board_len, &id, &mode, &zhiding) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
    bid = getbnum(board);
    if (!bid) RETURN_LONG(-1);
    bh = getboard(bid);
    strcpy(board,bh->filename);
    if (!is_BM(bh, getCurrentUser()))
        RETURN_LONG(-2);
    
    if (mode == 6)  /* undel action, add by pig2532 */
    {
        int find = 0;
        setbdir(DIR_MODE_DELETED, dir, board);
        fp = fopen(dir, "r");
        if(!fp)
        {
            RETURN_LONG(-9);    /* cannot open index file */
        }
        fseek(fp, sizeof(f) * (id - 1) , SEEK_SET);   /* here variable "id" is actually num */
        if(fread(&f, sizeof(f), 1, fp) > 0)
        {
            find = 1;
        }
        fclose(fp);
        if(find == 0)
        {
            RETURN_LONG(-4);    /* article index not found, maybe SYSOP cleared them */
        }
    }
    else if (zhiding) {
        int find = 0;
        ent = 1;
        setbdir(DIR_MODE_ZHIDING, dir, board);
        fd = open(dir, O_RDWR, 0644);
        if (fd < 0) 
            RETURN_LONG(-3);
        while (1) {
    	    if (read(fd,&f, sizeof(struct fileheader)) <= 0)
    		    break;               
    	    if (f.id==id) {
    		    find=1;
    		    break;
    	    }
    	    ent++;
        }
        close(fd);
        if (!find)
            RETURN_LONG(-4);
    }
    else {
        setbdir(DIR_MODE_NORMAL, dir, board);
        fd = open(dir, O_RDWR, 0644);
        if ( fd < 0) RETURN_LONG(-3);
        if (!get_records_from_id( fd, id, &f, 1, &ent)) {
            close(fd);
            RETURN_LONG(-4);
        }
        close(fd);
    }
        
    if (mode == 6)  /* undel action, add by pig2532 */
    {
        char buf[128];
        snprintf(buf, 100, "boards/%s/.DELETED", board);
        ret = do_undel_post(board, buf, id, &f, NULL, getSession());
        if(ret == 1)
        {
            ret = 0;
        }
        else
        {
            ret = -1;
        }
    }
    else if (zhiding) {
        ret = do_del_ding(board, bid, ent, &f, getSession());
         switch(ret)
         {
         case -1:
             RETURN_LONG(-4);    /* del failed */
             break;
         case -2:
             RETURN_LONG(-9);    /* null fileheader */
             break;
         case 0:
             RETURN_LONG(0);     /* success */
             break;
         default:
             RETURN_LONG(-9);
             break;
         }
    }
    else if (mode == 1) {
        ret = del_post(ent, &f, dir, board);
    }
    else {
        struct write_dir_arg dirarg;
        struct fileheader data;
        int flag;
        data.accessed[0] = ~(f.accessed[0]);
        data.accessed[1] = ~(f.accessed[1]);
        init_write_dir_arg(&dirarg);
        if (mode == 2)
            flag = FILE_MARK_FLAG;
        else if (mode == 3)
            flag = FILE_DIGEST_FLAG;
        else if (mode == 4)
            flag = FILE_NOREPLY_FLAG;
        else if (mode == 5) {
            flag = FILE_DING_FLAG;
            data.accessed[0] = f.accessed[0]; // to reserve flags. hack! - atppp
        } else
            RETURN_LONG(-3);
        
        dirarg.filename = dir;  
        dirarg.ent = ent;
        if(change_post_flag(&dirarg,DIR_MODE_NORMAL,bh, &f, flag, &data,true,getSession())!=0)
            ret = 1;
        else
            ret = 0;
        free_write_dir_arg(&dirarg);
    }
    
    if (ret != 0)
        RETURN_LONG(-9); 

    RETURN_LONG(0);
}


#if HAVE_MYSQL_SMTH == 1
static PHP_FUNCTION(bbs_csv_to_al)
{
	int ac = ZEND_NUM_ARGS();
	char *dest;
	int dest_len;
	int ret;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &dest, &dest_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	ret = conv_csv_to_al( dest, getSession() );

	RETURN_LONG(ret);
}
#endif




/**
 * get a full threads of articles from a groupid.
 * prototype:
 * int bbs_get_threads_from_gid(int bid, int gid, int start , array &articles , int haveprev);
 *
 * @return Record index on success,
 *       0 on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_get_threads_from_gid)
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

/*
 * refer Ecma-262 
 * '\033'  -> \r (not exactly the same thing, but borrow...)
 * '\n'    -> \n
 * '\\'    -> \\
 * '\''    -> \'
 * '\"'    -> \"
 * '\0'    -> possible start of attachment
 * 0 <= char < 32 -> ignore
 * others  -> passthrough
 */

PHP_FUNCTION(bbs2_readfile)
{
    char *filename;
    int filename_len;
    char *output_buffer;
    int output_buffer_len, output_buffer_size, j;
    char c;
    char *ptr, *cur_ptr;
    long ptrlen;
    int fd;
    int in_chinese = false;
    int chunk_size = 51200;
    struct stat st;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    
    fd = open(filename, O_RDONLY);
    if (fd < 0)
        RETURN_LONG(2);
    if (fstat(fd, &st) < 0) {
        close(fd);
        RETURN_LONG(2);
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        RETURN_LONG(2);
    }
    if (st.st_size <= 0) {
        close(fd);
        RETURN_LONG(2);
    }

    ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    ptrlen = st.st_size;
    close(fd);
    if (ptr == NULL)
        RETURN_LONG(-1);

    j = ptrlen;
    if (j > chunk_size) j = chunk_size;
    output_buffer_size = 2 * j + 16;
    output_buffer = (char* )emalloc(output_buffer_size);
    output_buffer_len = 0;
    cur_ptr = ptr;
    strcpy(output_buffer + output_buffer_len, "prints('");
    output_buffer_len += 8;
    while (1) {
        for (; j >= 0 ; j--) {
            c = *cur_ptr;
            if (c == '\0') { //assume ATTACHMENT_PAD[0] is '\0'
                if (ptrlen >= ATTACHMENT_SIZE + sizeof(int) + 2) {
                    if (!memcmp(cur_ptr, ATTACHMENT_PAD, ATTACHMENT_SIZE)) {
                        ptrlen = -ptrlen;
                        break;
                    }
                }
                ptrlen--; cur_ptr++;
                continue;
            }
            if (c < 0) {
                in_chinese = !in_chinese;
                output_buffer[output_buffer_len++] = c;
            } else {
                do {
                    if (c == '\n') c = 'n';
                    else if (c == '\033') c = 'r';
                    else if (c != '\\' && c != '\'' && c != '\"') {
                        if (c >= 32) {
                            output_buffer[output_buffer_len++] = c;
                        }
                        break;
                    }
                    if (in_chinese && c == 'n') {
                        output_buffer[output_buffer_len++] = ' ';
                    }
                    output_buffer[output_buffer_len++] = '\\';
                    output_buffer[output_buffer_len++] = c;
                } while(0);
                in_chinese = false;
            }
            ptrlen--; cur_ptr++;
        }
        if (ptrlen <= 0) break;
        j = ptrlen;
        if (j > chunk_size) j = chunk_size;
        output_buffer_size += 2 * j;
        output_buffer = (char*)erealloc(output_buffer, output_buffer_size);
        if (output_buffer == NULL) RETURN_LONG(3);
    }
    if (in_chinese) {
        output_buffer[output_buffer_len++] = ' ';
    }
    strcpy(output_buffer + output_buffer_len, "');");
    output_buffer_len += 3;
    
    if (ptrlen < 0) { //attachment
        char *attachfilename, *attachptr;
        char buf[1024];
        char *startbufptr, *bufptr;
        long attach_len, attach_pos, newlen;
        int l;

        ptrlen = -ptrlen;
        strcpy(buf, "attach('");
        startbufptr = buf + strlen(buf);
        while(ptrlen > 0) {
            if (((attachfilename = checkattach(cur_ptr, ptrlen, 
                                  &attach_len, &attachptr)) == NULL)) {
                break;
            }
            attach_pos = attachfilename - ptr;
            newlen = attachptr - cur_ptr + attach_len;
            cur_ptr += newlen;
            ptrlen -= newlen;
            if (ptrlen < 0) break;
            bufptr = startbufptr;
            while(*attachfilename != '\0') {
                switch(*attachfilename) {
                    case '\'':
                    case '\"':
                    case '\\':
                        *bufptr++ = '\\'; /* TODO: boundary check */
                        /* break is missing *intentionally* */
                    default:
                        *bufptr++ = *attachfilename++;  /* TODO: boundary check */
                }
            }
            sprintf(bufptr, "', %ld, %ld);", attach_len, attach_pos);  /* TODO: boundary check */

            l = strlen(buf);
            if (output_buffer_len + l > output_buffer_size) {
                output_buffer_size = output_buffer_size + sizeof(buf) * 10;
                output_buffer = (char*)erealloc(output_buffer, output_buffer_size);
                if (output_buffer == NULL) RETURN_LONG(3);
            }
            strcpy(output_buffer + output_buffer_len, buf);
            output_buffer_len += l;
        }
    }
    munmap(ptr, st.st_size);

    RETVAL_STRINGL(output_buffer, output_buffer_len, 0);;
}

PHP_FUNCTION(bbs2_readfile_text)
{
    char *filename;
    int filename_len;
    long maxchar;
    long double_escape;
    char *output_buffer;
    int output_buffer_len, output_buffer_size, last_return = 0;
    char c;
    char *ptr, *cur_ptr;
    long ptrlen;
    int in_escape = false;
    int fd;
    char escape_seq[4][16];
    struct stat st;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll", &filename, &filename_len, &maxchar, &double_escape) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    
    fd = open(filename, O_RDONLY);
    if (fd < 0)
        RETURN_LONG(2);
    if (fstat(fd, &st) < 0) {
        close(fd);
        RETURN_LONG(2);
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        RETURN_LONG(2);
    }
    if (st.st_size <= 0) {
        close(fd);
        RETURN_LONG(2);
    }

    ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    ptrlen = st.st_size;
    close(fd);
    if (ptr == NULL)
        RETURN_LONG(-1);

    if (!maxchar) {
        maxchar = ptrlen;
    } else if (ptrlen > maxchar) {
        ptrlen = maxchar;
    }
    output_buffer_size = 2 * maxchar;
    output_buffer = (char* )emalloc(output_buffer_size);
    output_buffer_len = 0;
    cur_ptr = ptr;
    if (double_escape) {
        strcpy(escape_seq[0], "&amp;amp;");
        strcpy(escape_seq[1], "&amp;lt;");
        strcpy(escape_seq[2], "&amp;gt;");
        strcpy(escape_seq[3], "&lt;br/&gt;");
    } else {
        strcpy(escape_seq[0], "&amp;");
        strcpy(escape_seq[1], "&lt;");
        strcpy(escape_seq[2], "&gt;");
        strcpy(escape_seq[3], "<br/>");
    }
    while (ptrlen > 0) {
        c = *cur_ptr;
        if (c == '\0') { //assume ATTACHMENT_PAD[0] is '\0'
            break;
        } else if (c == '\033') {
            in_escape = true;
        } else if (!in_escape) {
            if (output_buffer_len + 16 > output_buffer_size) {
                output_buffer = (char*)erealloc(output_buffer, output_buffer_size += 128);
            }
            switch(c) {
                case '&':
                    strcpy(output_buffer + output_buffer_len, escape_seq[0]);
                    output_buffer_len += strlen(escape_seq[0]);
                    break;
                case '<':
                    strcpy(output_buffer + output_buffer_len, escape_seq[1]);
                    output_buffer_len += strlen(escape_seq[1]);
                    break;
                case '>':
                    strcpy(output_buffer + output_buffer_len, escape_seq[2]);
                    output_buffer_len += strlen(escape_seq[2]);
                    break;
                case '\n':
                    strcpy(output_buffer + output_buffer_len, escape_seq[3]);
                    output_buffer_len += strlen(escape_seq[3]);
                    last_return = output_buffer_len;
                    break;
                default:
                    if (c < 0 || c >= 32)
                        output_buffer[output_buffer_len++] = c;
                    break;
            }
        } else if (isalpha(c)) {
            in_escape = false;
        }
        ptrlen--; cur_ptr++;
    }

    munmap(ptr, st.st_size);

    RETVAL_STRINGL(output_buffer, last_return, 0);
}


