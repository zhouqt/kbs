#ifndef PHP_BBS_BOARD_H
#define PHP_BBS_BOARD_H

/* 版面读取和查询 */

PHP_FUNCTION(bbs_getboard);
PHP_FUNCTION(bbs_safe_getboard);
PHP_FUNCTION(bbs_getboards);

PHP_FUNCTION(bbs_checkorigin);
PHP_FUNCTION(bbs_checkmark);
PHP_FUNCTION(bbs_getbname);
PHP_FUNCTION(bbs_getbdes);
PHP_FUNCTION(bbs_checkreadperm);
PHP_FUNCTION(bbs_checkpostperm);
PHP_FUNCTION(bbs_normalboard);
PHP_FUNCTION(bbs_searchboard);

PHP_FUNCTION(bbs_useronboard);
PHP_FUNCTION(bbs_set_onboard);


/* favboard operation. by caltary  */
PHP_FUNCTION(bbs_load_favboard);
PHP_FUNCTION(bbs_fav_boards);
PHP_FUNCTION(bbs_is_favboard);
PHP_FUNCTION(bbs_add_favboarddir);
PHP_FUNCTION(bbs_add_favboard);
PHP_FUNCTION(bbs_del_favboard);
PHP_FUNCTION(bbs_get_father);
PHP_FUNCTION(bbs_get_dirname);
PHP_FUNCTION(bbs_del_favboarddir);


#define PHP_BBS_BOARD_EXPORT_FUNCTIONS \
    PHP_FE(bbs_getboard, NULL) \
    PHP_FE(bbs_safe_getboard, NULL) \
    PHP_FE(bbs_getboards, NULL) \
    PHP_FE(bbs_checkorigin, NULL) \
    PHP_FE(bbs_checkmark, NULL) \
    PHP_FE(bbs_getbname, NULL) \
    PHP_FE(bbs_getbdes, NULL) \
    PHP_FE(bbs_checkreadperm, NULL) \
    PHP_FE(bbs_checkpostperm, NULL) \
    PHP_FE(bbs_normalboard,NULL) \
    PHP_FE(bbs_searchboard,third_arg_force_ref_001) \
    PHP_FE(bbs_useronboard,two_arg_force_ref_01) \
    PHP_FE(bbs_set_onboard,NULL) \
    PHP_FE(bbs_load_favboard,NULL) \
    PHP_FE(bbs_fav_boards,NULL) \
    PHP_FE(bbs_is_favboard,NULL) \
    PHP_FE(bbs_add_favboarddir,NULL) \
    PHP_FE(bbs_add_favboard,NULL) \
    PHP_FE(bbs_del_favboard,NULL) \
    PHP_FE(bbs_get_father,NULL) \
    PHP_FE(bbs_get_dirname,NULL) \
    PHP_FE(bbs_del_favboarddir,NULL)

#endif //PHP_BBS_BOARD_H

