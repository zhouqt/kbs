#ifndef PHP_BBS_WFORUM_H
#define PHP_BBS_WFORUM_H

/* wForum ÌØÓÐº¯Êý */

PHP_FUNCTION(bbs_searchtitle);
PHP_FUNCTION(bbs_getthreadnum);
PHP_FUNCTION(bbs_get_today_article_num);
PHP_FUNCTION(bbs_getthreads);

#define PHP_BBS_WFORUM_EXPORT_FUNCTIONS \
    PHP_FE(bbs_searchtitle, NULL) \
    PHP_FE(bbs_get_today_article_num, NULL) \
    PHP_FE(bbs_getthreadnum, NULL) \
    PHP_FE(bbs_getthreads, NULL) \

#endif //PHP_BBS_WFORUM_H

