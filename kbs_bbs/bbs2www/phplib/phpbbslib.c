#include <php.h>
#include "bbs.h"

static ZEND_FUNCTION(bbs_getuser);

static ZEND_MINIT_FUNCTION(bbs_module_init);
static ZEND_MSHUTDOWN_FUNCTION(bbs_module_shutdown);
static ZEND_RINIT_FUNCTION(bbs_request_init);
static ZEND_RSHUTDOWN_FUNCTION(bbs_request_shutdown); 

/*
 * define what functions can be used in the PHP embedded script
 */
static function_entry bbs_php_functions[] = {
        ZEND_FE(bbs_getuser, NULL)
        {NULL,NULL,NULL}
};

/*
 * This is the module entry structure, and some properties
 */

static zend_module_entry bbs_php_module_entry = {
	STANDARD_MODULE_HEADER,
        "bbs module",				/* extension name */
        bbs_php_functions,				/* extension function list */
        ZEND_MINIT(bbs_module_init),						/* extension-wide startup function */
	ZEND_MSHUTDOWN(bbs_module_shutdown),						/* extension-wide shutdown function */
	ZEND_RINIT(bbs_request_init),						/* per-request startup function */
	ZEND_RSHUTDOWN(bbs_request_shutdown),						/* per-request shutdown function */
	NULL,						/* information function */
	"1.0",
        STANDARD_MODULE_PROPERTIES
};

/*
 * Here is the function require when the module loaded
 */
DLEXPORT zend_module_entry *get_module() {
        return &bbs_php_module_entry;
};


static void setstrlen(pval * arg)
{
	arg->value.str.len=strlen(arg->value.str.val);
}

static void assign_user(zval* array,struct userec* user)
{
	add_assoc_string(array,"userid",user->userid,1);
	add_assoc_long(array,"firstlogin",user->firstlogin);
	add_assoc_stringl(array,"lasthost",user->lasthost,IPLEN,1);
	add_assoc_long(array,"numlogins",user->numlogins);
	add_assoc_long(array,"numposts",user->numposts);
	add_assoc_long(array,"flag1",user->flags[0]);
	add_assoc_long(array,"flag2",user->flags[1]);
	add_assoc_string(array,"username",user->username,1);
	add_assoc_string(array,"ident",user->ident,1);
	add_assoc_stringl(array,"md5passwd",(char*)user->md5passwd,16,1);
	add_assoc_string(array,"realemail",user->realemail,1);
	add_assoc_long(array,"userlevel",user->userlevel);
	add_assoc_long(array,"lastlogin",user->lastlogin);
	add_assoc_long(array,"stay",user->stay);
	add_assoc_string(array,"realname",user->realname,1);
	add_assoc_string(array,"address",user->address,1);
	add_assoc_string(array,"email",user->email,1);
	add_assoc_long(array,"signature",user->signature);
	add_assoc_long(array,"userdefine",user->userdefine);
	add_assoc_long(array,"notedate",user->notedate);
	add_assoc_long(array,"noteline",user->noteline);
	add_assoc_long(array,"notemode",user->notemode);
}
/*
 * Here goes the real functions
 */

/* arguments: userid, username, ipaddr, operation */
static ZEND_FUNCTION(bbs_getuser)
{
        long                    v1;
	struct	userec *lookupuser;
	char* s;
	int s_len;
	zval* user_array;

        if (zend_parse_parameters(2 TSRMLS_CC, "sa" ,&s,&s_len, &user_array) != SUCCESS) {
                WRONG_PARAM_COUNT;
        }

	s[IDLEN]=0;
        v1=getuser(s, &lookupuser);

	if (v1==0)
		RETURN_LONG(0);

	if(array_init(user_array) != SUCCESS)
		RETURN_LONG(0);
	assign_user(user_array,lookupuser);
/*        RETURN_STRING(retbuf, 1);
 *        */
        RETURN_LONG(v1);
}

static char old_pwd[1024];
static ZEND_MINIT_FUNCTION(bbs_module_init)
{
	getcwd(old_pwd,1023);
	old_pwd[1023]=0;
	chdir(BBSHOME);
	resolve_ucache();
	resolve_utmp();
	resolve_boards();
#ifdef SQUID_ACCL
	REGISTER_MAIN_LONG_CONSTANT("SETTING_SQUID_ACCL", 1, CONST_CS | CONST_PERSISTENT);
#else
	REGISTER_MAIN_LONG_CONSTANT("SETTING_SQUID_ACCL", 0, CONST_CS | CONST_PERSISTENT);
#endif
	chdir(old_pwd);
	return SUCCESS;
}

static ZEND_MSHUTDOWN_FUNCTION(bbs_module_shutdown)
{
	return SUCCESS;
}

static ZEND_RINIT_FUNCTION(bbs_request_init)
{
	getcwd(old_pwd,1023);
	old_pwd[1023]=0;
	return SUCCESS;
}

static ZEND_RSHUTDOWN_FUNCTION(bbs_request_shutdown)
{
	chdir(old_pwd);
	return SUCCESS;
}
