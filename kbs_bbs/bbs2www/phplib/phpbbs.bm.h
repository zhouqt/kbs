#ifndef PHP_BBS_BM_H
#define PHP_BBS_BM_H

/* 版主版面管理相关 */

PHP_FUNCTION(bbs_is_bm);
PHP_FUNCTION(bbs_bmmanage);
PHP_FUNCTION(bbs_denyusers);
PHP_FUNCTION(bbs_denyadd);
PHP_FUNCTION(bbs_denydel);
PHP_FUNCTION(bbs_bm_get_manageable_bids);
PHP_FUNCTION(bbs_club_read);
PHP_FUNCTION(bbs_club_flag);
PHP_FUNCTION(bbs_club_write);
PHP_FUNCTION(bbs_threads_bmfunc);


#define PHP_BBS_BM_EXPORT_FUNCTIONS \
    PHP_FE(bbs_is_bm, NULL) \
    PHP_FE(bbs_bmmanage,NULL) \
    PHP_FE(bbs_denyusers,two_arg_force_ref_01) \
    PHP_FE(bbs_denyadd,NULL) \
    PHP_FE(bbs_denydel,NULL) \
    PHP_FE(bbs_bm_get_manageable_bids,NULL) \
    PHP_FE(bbs_club_read,NULL) \
    PHP_FE(bbs_club_flag,NULL) \
    PHP_FE(bbs_club_write,NULL) \
    PHP_FE(bbs_threads_bmfunc,NULL)

#endif //PHP_BBS_BM_H

