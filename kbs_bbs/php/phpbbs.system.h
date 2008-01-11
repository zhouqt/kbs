#ifndef PHP_BBS_SYSTEM_H
#define PHP_BBS_SYSTEM_H

/* 一些系统函数 */

PHP_FUNCTION(bbs_get_explain);
PHP_FUNCTION(bbs_sysconf_str);
PHP_FUNCTION(bbs_new_board);
PHP_FUNCTION(bbs_valid_filename);
PHP_FUNCTION(bbs_sethomefile);
PHP_FUNCTION(bbs_setmailfile);
PHP_FUNCTION(bbs_checkbadword);

PHP_FUNCTION(bbs_error_get_desc);


#define PHP_BBS_SYSTEM_EXPORT_FUNCTIONS \
    PHP_FE(bbs_get_explain,NULL) \
    PHP_FE(bbs_sysconf_str,NULL) \
    PHP_FE(bbs_new_board,NULL) \
    PHP_FE(bbs_valid_filename, NULL) \
    PHP_FE(bbs_sethomefile, NULL) \
    PHP_FE(bbs_setmailfile, NULL) \
    PHP_FE(bbs_checkbadword, NULL) \
    PHP_FE(bbs_error_get_desc, NULL)

#endif //PHP_BBS_SYSTEM_H

