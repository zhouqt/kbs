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

#ifndef PHP_SMTH_BBS_H
#define PHP_SMTH_BBS_H

extern zend_module_entry smth_bbs_module_entry;
#define phpext_smth_bbs_ptr &smth_bbs_module_entry

#ifdef PHP_WIN32
#define PHP_SMTH_BBS_API __declspec(dllexport)
#else
#define PHP_SMTH_BBS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(smth_bbs);
PHP_MSHUTDOWN_FUNCTION(smth_bbs);
PHP_RINIT_FUNCTION(smth_bbs);
PHP_RSHUTDOWN_FUNCTION(smth_bbs);
PHP_MINFO_FUNCTION(smth_bbs);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(smth_bbs)
	int   global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(smth_bbs)
*/

/* In every utility function you add that needs to use variables 
   in php_smth_bbs_globals, call TSRM_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as SMTH_BBS_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define SMTH_BBS_G(v) TSRMG(smth_bbs_globals_id, zend_smth_bbs_globals *, v)
#else
#define SMTH_BBS_G(v) (smth_bbs_globals.v)
#endif

#endif	/* PHP_SMTH_BBS_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
