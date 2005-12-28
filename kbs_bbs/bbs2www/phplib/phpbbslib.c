/*
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
*/
    
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_kbs_bbs.h"  


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

#include "bbs.h"
#include "bbslib.h"

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
#include "phpbbs.file.h"
#include "phpbbs.post.h"
#include "phpbbs.friend.h"

#ifdef HAVE_WFORUM
#include "phpbbs.wforum.h"
#endif


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


static PHP_FUNCTION(bbs_modify_info);
static PHP_FUNCTION(bbs_recalc_sig);
static PHP_FUNCTION(bbs_modify_nick);
static PHP_FUNCTION(bbs_findpwd_check);
static PHP_FUNCTION(bbs_update_uinfo);
static PHP_FUNCTION(bbs_setpassword);

//////////////////////// Board/Article operation functions  ////////////////////
static PHP_FUNCTION(bbs_bmmanage);
static PHP_FUNCTION(bbs_delfile);



////////////////////////  Mail operation functions  ///////////////////////////
static PHP_FUNCTION(bbs_postmail);



////////////////////////   WWW special functions  /////////////////////////////

#if HAVE_MYSQL_SMTH == 1
static PHP_FUNCTION(bbs_csv_to_al);
#endif



/*
 * define what functions can be used in the PHP embedded script
 */
static function_entry kbs_bbs_functions[] = {
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
    PHP_BBS_FILE_EXPORT_FUNCTIONS
    PHP_BBS_POST_EXPORT_FUNCTIONS
    PHP_BBS_FRIEND_EXPORT_FUNCTIONS
#ifdef HAVE_WFORUM
    PHP_BBS_WFORUM_EXPORT_FUNCTIONS
#endif

    PHP_FE(bbs_ext_initialized, NULL)
    PHP_FE(bbs_init_ext, NULL)

    PHP_FE(bbs_checkuserpasswd, NULL)
    PHP_FE(bbs_setuserpasswd, NULL)
    PHP_FE(bbs_getuserlevel, NULL)
    PHP_FE(bbs_user_setflag, NULL)
    PHP_FE(bbs_postmail, NULL)
    PHP_FE(bbs_getusermode, NULL)
    PHP_FE(bbs_compute_user_value, NULL)
    PHP_FE(bbs_user_level_char, NULL)
    PHP_FE(bbs_isonline, NULL)
    PHP_FE(bbs_update_uinfo, NULL)
    PHP_FE(bbs_setpassword,NULL)
    PHP_FE(bbs_findpwd_check,NULL)
    PHP_FE(bbs_modify_info,NULL)
    PHP_FE(bbs_recalc_sig,NULL)
    PHP_FE(bbs_modify_nick,NULL)
    PHP_FE(bbs_delfile,NULL)
    PHP_FE(bbs_bmmanage,NULL)

#if HAVE_MYSQL_SMTH == 1
    PHP_FE(bbs_csv_to_al, NULL)
#endif

    {NULL, NULL, NULL}
};

/*
 * This is the module entry structure, and some properties
 */
zend_module_entry kbs_bbs_module_entry = {
    STANDARD_MODULE_HEADER,
    "kbs_bbs",
    kbs_bbs_functions,
    PHP_MINIT(kbs_bbs),
    PHP_MSHUTDOWN(kbs_bbs),
    PHP_RINIT(kbs_bbs),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(kbs_bbs),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(kbs_bbs),
    "1.0", /* Replace with version number for your extension */
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_KBS_BBS
ZEND_GET_MODULE(kbs_bbs)
#endif

/*
 * Here is the function require when the module loaded
DLEXPORT zend_module_entry *get_module()
{
    return &kbs_bbs_module_entry;
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
    get_publicshm();
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
	if (!get_initialized()) {
        RETURN_FALSE;
	} else {
        RETURN_TRUE;
	}
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

PHP_MINIT_FUNCTION(kbs_bbs)
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

PHP_MSHUTDOWN_FUNCTION(kbs_bbs)
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

PHP_RINIT_FUNCTION(kbs_bbs)
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

PHP_RSHUTDOWN_FUNCTION(kbs_bbs)
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
PHP_MINFO_FUNCTION(kbs_bbs)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "kbs_bbs support", "enabled");
    php_info_print_table_end();
}

/**
 * Function: post a new mail
 *  rototype:
 * int bbs_postmail(string targetid,string title,string content,long sig, long backup);
 *
 *  @return the result
 *  	0 -- success
 *		-1   index file failed to open
 *      -2   file/dir creation failed
 *      -3   receiver refuses
 *      -4   receiver reaches mail limit
 *      -5   send too frequently
 *      -6   receiver index append failed
 *      -7   sender index append failed
 *      -8   invalid renum
 *      -100 invalid user
 *  @author roy
 */
static PHP_FUNCTION(bbs_postmail){
	char *recvID, *title, *content;
	char targetID[IDLEN+1];
	int  idLen, tLen,cLen;
    long backup,sig,renum;
	int ac = ZEND_NUM_ARGS();
	char mail_title[80];
    FILE *fp;
    char fname[PATHLEN], filepath[PATHLEN], sent_filepath[PATHLEN];
    struct fileheader header;
    struct stat st;
    struct userec *touser;      /*peregrine for updating used space */
	char *refname,*dirfname;
	int find=-1,fhcount=0,refname_len,dirfname_len;

    if(ac == 5)		/* use this to send a new mail */
	{
		if(zend_parse_parameters(5 TSRMLS_CC, "ss/s/ll", &recvID, &idLen,&title,&tLen,&content,&cLen,&sig,&backup) == FAILURE)
		{
			WRONG_PARAM_COUNT;
		}
		strncpy(targetID, recvID, sizeof(targetID));
		targetID[sizeof(targetID)-1] = '\0';
	}
    else if(ac == 7)		/* use this to reply a mail */
	{
		if(zend_parse_parameters(7 TSRMLS_CC, "ssls/s/ll", &dirfname, &dirfname_len, &refname, &refname_len, &renum, &title, &tLen, &content, &cLen, &sig, &backup) == FAILURE)
		{
			WRONG_PARAM_COUNT;
		}
	}
	else
	{
		WRONG_PARAM_COUNT;
	}

    if (abs(time(0) - getSession()->currentuinfo->lastpost) < 6) {
        getSession()->currentuinfo->lastpost = time(0);
        RETURN_LONG(-5); // 两次发文间隔过密, 请休息几秒后再试
    }
    getSession()->currentuinfo->lastpost = time(0);

	/* read receiver's id from mail when replying, by pig2532 */
	if(ac == 7)
	{
		if(stat(dirfname, &st)==-1)
        {
            RETURN_LONG(-1);    /* error reading stat */
        }
        if((renum<0)||(renum>=(st.st_size/sizeof(fileheader))))
        {
            RETURN_LONG(-8);    /* no such mail to reply */
        }
		if((fp = fopen(dirfname, "r+")) == NULL)
		{
			RETURN_LONG(-1);		/* error openning .DIR */
		}
		fseek(fp, sizeof(header) * renum, SEEK_SET);
		if(fread(&header, sizeof(header), 1, fp) > 0 )	/* read fileheader by renum */
		{
			if(strcmp(header.filename, refname) == 0)
			{
				find = renum;
			}
		}
		if(find == -1)
		{
			rewind(fp);
			while(true)		/* find the fileheader */
			{
				if(fread(&header, sizeof(header), 1, fp) <= 0)
				{
					break;
				}
				if(strcmp(header.filename, refname) == 0)
				{
					find = fhcount;
					break;
				}
				fhcount++;
			}
		}
		if(find == -1)
		{	/* file not found */
			fclose(fp);
			RETURN_LONG(-8);
		}
		else
		{
			strncpy(targetID, header.owner, sizeof(targetID));
			targetID[sizeof(targetID)-1] = '\0';
			if(!(header.accessed[0] & FILE_REPLIED))
			{	/* set the replied flag */
				header.accessed[0] |= FILE_REPLIED;
				fseek(fp, sizeof(header) * find, SEEK_SET);
				fwrite(&header, sizeof(header), 1, fp);
			}
			fclose(fp);
		}
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

	strncpy(targetID, touser->userid, sizeof(targetID));
	targetID[sizeof(targetID)-1] = '\0';
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








