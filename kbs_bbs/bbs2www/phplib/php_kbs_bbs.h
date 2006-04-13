/*
  +----------------------------------------------------------------------+
  | PHP Version 4                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2002 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.02 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available at through the world-wide-web at                           |
  | http://www.php.net/license/2_02.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+

  $Id$ 
*/

#ifndef PHP_KBS_BBS_H
#define PHP_KBS_BBS_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include "php.h"
#include "php_ini.h"

extern zend_module_entry kbs_bbs_module_entry;
#define phpext_kbs_bbs_ptr &kbs_bbs_module_entry

#ifdef PHP_WIN32
#define PHP_KBS_BBS_API __declspec(dllexport)
#else
#define PHP_KBS_BBS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(kbs_bbs);
PHP_MSHUTDOWN_FUNCTION(kbs_bbs);
PHP_RINIT_FUNCTION(kbs_bbs);
PHP_RSHUTDOWN_FUNCTION(kbs_bbs);
PHP_MINFO_FUNCTION(kbs_bbs);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(kbs_bbs)
	int   global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(kbs_bbs)
*/

/* In every utility function you add that needs to use variables 
   in php_kbs_bbs_globals, call TSRM_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as KBS_BBS_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define KBS_BBS_G(v) TSRMG(kbs_bbs_globals_id, zend_kbs_bbs_globals *, v)
#else
#define KBS_BBS_G(v) (kbs_bbs_globals.v)
#endif


/* 简单粗暴法去掉重复定义 Warning */
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef MYSQL_UNIX_ADDR

#include "bbs.h"
#include "bbslib.h"

#include "phpbbs.errorno.h"


/* phpbbs.article.c */
void reset_output_buffer();
void bbs_make_article_array(zval * array, struct fileheader *fh, char *flags, size_t flags_len);
void make_article_flag_array(char flags[5], struct fileheader *ent, struct userec *user, const char *boardname, int is_bm);

/* phpbbs.post.c */
int check_last_post_time(struct user_info *uinfo);

/* phpbbs.user.c */
void assign_user(zval * array, struct userec *user, int num);

/* phpbbs.session.c */
void setcurrentuinfo(struct user_info *uinfo, int uinfonum);
void setcurrentuser(struct userec *user, int usernum);

#endif	/* PHP_KBS_BBS_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
