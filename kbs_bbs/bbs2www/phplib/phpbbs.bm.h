#ifndef PHP_BBS_BM_H
#define PHP_BBS_BM_H

PHP_FUNCTION(bbs_is_bm);
PHP_FUNCTION(bbs_denyusers);
PHP_FUNCTION(bbs_denyadd);
PHP_FUNCTION(bbs_denydel);


#define PHP_BBS_BM_EXPORT_FUNCTIONS \
    PHP_FE(bbs_is_bm, NULL) \
    PHP_FE(bbs_denyusers,two_arg_force_ref_01) \
    PHP_FE(bbs_denyadd,NULL) \
    PHP_FE(bbs_denydel,NULL)

#endif //PHP_BBS_BM_H

