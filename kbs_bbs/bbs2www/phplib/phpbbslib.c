#include "php_kbs_bbs.h"
#include "ext/standard/info.h"

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

#include "phpbbs.admin.h"
#include "phpbbs.announce.h"
#include "phpbbs.article.h"
#include "phpbbs.bm.h"
#include "phpbbs.board.h"
#include "phpbbs.file.h"
#include "phpbbs.friend.h"
#include "phpbbs.mail.h"
#include "phpbbs.msg.h"
#include "phpbbs.post.h"
#include "phpbbs.reg.h"
#include "phpbbs.session.h"
#include "phpbbs.system.h"
#include "phpbbs.user.h"
#include "phpbbs.vote.h"
#ifdef HAVE_WFORUM
#include "phpbbs.wforum.h"
#endif


static char old_pwd[1024];


PHP_FUNCTION(bbs_ext_initialized);
PHP_FUNCTION(bbs_init_ext);

#if HAVE_MYSQL_SMTH == 1
PHP_FUNCTION(bbs_csv_to_al);
#endif



/*
 * define what functions can be used in the PHP embedded script
 */
static function_entry kbs_bbs_functions[] = {
    PHP_BBS_ADMIN_EXPORT_FUNCTIONS
    PHP_BBS_ANNOUNCE_EXPORT_FUNCTIONS
    PHP_BBS_ARTICLE_EXPORT_FUNCTIONS
    PHP_BBS_BM_EXPORT_FUNCTIONS
    PHP_BBS_BOARD_EXPORT_FUNCTIONS
    PHP_BBS_FRIEND_EXPORT_FUNCTIONS
    PHP_BBS_FILE_EXPORT_FUNCTIONS
    PHP_BBS_MAIL_EXPORT_FUNCTIONS
    PHP_BBS_MSG_EXPORT_FUNCTIONS
    PHP_BBS_POST_EXPORT_FUNCTIONS
    PHP_BBS_REG_EXPORT_FUNCTIONS
    PHP_BBS_SESSION_EXPORT_FUNCTIONS
    PHP_BBS_SYSTEM_EXPORT_FUNCTIONS
    PHP_BBS_USER_EXPORT_FUNCTIONS
    PHP_BBS_VOTE_EXPORT_FUNCTIONS
#ifdef HAVE_WFORUM
    PHP_BBS_WFORUM_EXPORT_FUNCTIONS
#endif

    PHP_FE(bbs_ext_initialized, NULL)
    PHP_FE(bbs_init_ext, NULL)

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

static int phpbbs_zap_buf[MAXBOARD];
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
// #define REGISTER_STRING_CONSTANT(name, str, flags)  zend_register_string_constant((name), sizeof(name), (str), (flags), module_number TSRMLS_CC)
#define MY_REGISTER_STRING_CONSTANT(name, str, flags)  zend_register_string_constant((name), strlen(name)+1, (char*)(str), (flags), module_number TSRMLS_CC)

    int i;
	char old_cwd[256], buf[256];
    const char *c;
	getcwd(old_cwd, sizeof(old_cwd));
	chdir(BBSHOME);
    MY_REGISTER_STRING_CONSTANT("BBS_HOME",BBSHOME,CONST_CS | CONST_PERSISTENT);
    MY_REGISTER_STRING_CONSTANT("BBS_FULL_NAME",BBS_FULL_NAME,CONST_CS | CONST_PERSISTENT);

    c=sysconf_str("BBS_WEBDOMAIN");
    if(c==NULL){
        c=sysconf_str("BBSDOMAIN");
        if (c==NULL) c = ""; //ft, should I say TODO here?
	}
    MY_REGISTER_STRING_CONSTANT("BBS_WEBDOMAIN",c,CONST_CS | CONST_PERSISTENT);
    MY_REGISTER_STRING_CONSTANT("BBS_XPERMSTR", XPERMSTR, CONST_CS | CONST_PERSISTENT);

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
#ifdef ANN_AUTONAME
    REGISTER_LONG_CONSTANT("BBS_ANN_AUTONAME", 1, CONST_CS | CONST_PERSISTENT);
#else
    REGISTER_LONG_CONSTANT("BBS_ANN_AUTONAME", 0, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef PERSONAL_CORP
    REGISTER_LONG_CONSTANT("BBS_HAVE_BLOG", 1, CONST_CS | CONST_PERSISTENT);
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
    REGISTER_LONG_CONSTANT("BBS_PERM_DISS", PERM_DISS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_ACCOUNTS", PERM_ACCOUNTS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_ADMIN", PERM_ADMIN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_WELCOME", PERM_WELCOME, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_ANNOUNCE", PERM_ANNOUNCE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_ATTACH", BOARD_ATTACH, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_ANNONY", BOARD_ANNONY, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_JUNK", BOARD_JUNK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_OUTFLAG", BOARD_OUTFLAG, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_CLUB_READ", BOARD_CLUB_READ, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_CLUB_WRITE", BOARD_CLUB_WRITE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_CLUB_HIDE", BOARD_CLUB_HIDE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_GROUP", BOARD_GROUP, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_POSTSTAT", BOARD_POSTSTAT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_EMAILPOST", BOARD_EMAILPOST, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_NOREPLY", BOARD_NOREPLY, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_MAXATTACHMENTCOUNT", MAXATTACHMENTCOUNT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_MAXATTACHMENTSIZE", MAXATTACHMENTSIZE, CONST_CS | CONST_PERSISTENT);

    REGISTER_LONG_CONSTANT("BBS_SECNUM", SECNUM, CONST_CS | CONST_PERSISTENT);
    for(i=0;i<SECNUM;i++) {
        sprintf(buf, "BBS_SECCODE%d", i);
        MY_REGISTER_STRING_CONSTANT(buf, seccode[i], CONST_CS | CONST_PERSISTENT);
        sprintf(buf, "BBS_GROUP%d", i);
        MY_REGISTER_STRING_CONSTANT(buf, groups[i], CONST_CS | CONST_PERSISTENT);
        sprintf(buf, "BBS_SECNAME%d_0", i);
        MY_REGISTER_STRING_CONSTANT(buf, secname[i][0], CONST_CS | CONST_PERSISTENT);
        sprintf(buf, "BBS_SECNAME%d_1", i);
        MY_REGISTER_STRING_CONSTANT(buf, secname[i][1], CONST_CS | CONST_PERSISTENT);
    }

    REGISTER_LONG_CONSTANT("BBS_NUMPERMS", NUMPERMS, CONST_CS | CONST_PERSISTENT);
    for(i=0; i<NUMPERMS; i++) {
        sprintf(buf, "BBS_PERMSTRING%d", i);
        MY_REGISTER_STRING_CONSTANT(buf, permstrings[i], CONST_CS | CONST_PERSISTENT);
    }

#define DEFINE_MODE_CONSTANT(str) REGISTER_LONG_CONSTANT("BBS_MODE_"#str, str, CONST_CS | CONST_PERSISTENT)

    DEFINE_MODE_CONSTANT(CSIE_ANNOUNCE);
    DEFINE_MODE_CONSTANT(MMENU);
    DEFINE_MODE_CONSTANT(GMENU);
    DEFINE_MODE_CONSTANT(SELECT);
    DEFINE_MODE_CONSTANT(READING);
    DEFINE_MODE_CONSTANT(POSTING);
    DEFINE_MODE_CONSTANT(POSTTMPL);
    DEFINE_MODE_CONSTANT(MAIL);
    DEFINE_MODE_CONSTANT(SMAIL);
    DEFINE_MODE_CONSTANT(RMAIL);
    DEFINE_MODE_CONSTANT(MSGING);
    DEFINE_MODE_CONSTANT(LUSERS);
    DEFINE_MODE_CONSTANT(FRIEND);
    DEFINE_MODE_CONSTANT(QUERY);
    DEFINE_MODE_CONSTANT(VOTING);
    DEFINE_MODE_CONSTANT(EDITUFILE);
    DEFINE_MODE_CONSTANT(USERDEF);
    DEFINE_MODE_CONSTANT(EDIT);
    DEFINE_MODE_CONSTANT(LOOKMSGS);
    DEFINE_MODE_CONSTANT(WEBEXPLORE);
    DEFINE_MODE_CONSTANT(PC);
    DEFINE_MODE_CONSTANT(BLOG);

    for (i=0; i<MAXBOARD; i++) phpbbs_zap_buf[i] = 1;


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
	getSession()->zapbuf = phpbbs_zap_buf;
    getSession()->fromhost[0] = '\0';


#ifdef HAVE_BRC_CONTROL
    getSession()->brc_cache_entry=NULL;
    getSession()->brc_currcache=-1;
#endif


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

#ifdef HAVE_BRC_CONTROL
    if (getCurrentUser() && getCurrentUser()->userid && (getCurrentUser()->userid[0]) ) {
        free_brc_cache( getCurrentUser()->userid, getSession() );
    }
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











#if HAVE_MYSQL_SMTH == 1
PHP_FUNCTION(bbs_csv_to_al)
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








