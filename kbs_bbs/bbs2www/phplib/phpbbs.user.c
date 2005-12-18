#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_smth_bbs.h"  

#include "phpbbs.user.h"

#include "bbs.h"
#include "bbslib.h"


PHP_FUNCTION(bbs_getuserparam){//这个函数总有一天要被我杀掉！！ - atppp
	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}
	if (getCurrentUser()==NULL) {
		RETURN_FALSE;
	}
	RETURN_LONG(getCurrentUser()->userdefine[0]);
}

PHP_FUNCTION(bbs_setuserparam){
	long userparam0, userparam1, mailboxprop;
	if (ZEND_NUM_ARGS() != 3 || zend_parse_parameters(3 TSRMLS_CC, "lll", &userparam0, &userparam1, &mailboxprop) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}
	if (getCurrentUser()==NULL) {
		RETURN_LONG(-1);
	}
	getCurrentUser()->userdefine[0] = userparam0;
    getCurrentUser()->userdefine[1] = userparam1;
    getSession()->currentuinfo->mailbox_prop = update_mailbox_prop(getCurrentUser()->userid, mailboxprop);
    store_mailbox_prop(getCurrentUser()->userid);
	RETURN_LONG(0);
}


