#ifndef PHP_BBS_BOARD_H
#define PHP_BBS_BOARD_H

PHP_FUNCTION(bbs_getboard);
PHP_FUNCTION(bbs_getboards);

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
    PHP_FE(bbs_getboards, NULL) \
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

