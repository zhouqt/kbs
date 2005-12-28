#ifndef PHP_BBS_FRIEND_H
#define PHP_BBS_FRIEND_H

/* ∫√”—œ‡πÿ */

PHP_FUNCTION(bbs_getfriends);
PHP_FUNCTION(bbs_countfriends);
PHP_FUNCTION(bbs_delete_friend);
PHP_FUNCTION(bbs_add_friend);
PHP_FUNCTION(bbs_getonlinefriends);

#define PHP_BBS_FRIEND_EXPORT_FUNCTIONS \
    PHP_FE(bbs_getfriends, NULL) \
    PHP_FE(bbs_countfriends, NULL) \
    PHP_FE(bbs_delete_friend, NULL) \
    PHP_FE(bbs_add_friend, NULL) \
    PHP_FE(bbs_getonlinefriends,NULL)


#endif //PHP_BBS_FRIEND_H

