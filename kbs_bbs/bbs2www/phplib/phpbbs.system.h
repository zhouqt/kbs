#ifndef PHP_BBS_SYSTEM_H
#define PHP_BBS_SYSTEM_H

PHP_FUNCTION(bbs_get_explain);
PHP_FUNCTION(bbs_sysconf_str);
PHP_FUNCTION(bbs_new_board);


#define PHP_BBS_SYSTEM_EXPORT_FUNCTIONS \
    PHP_FE(bbs_get_explain,NULL) \
    PHP_FE(bbs_sysconf_str,NULL) \
    PHP_FE(bbs_new_board,NULL)

#endif //PHP_BBS_SYSTEM_H

