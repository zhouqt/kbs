#ifndef PHP_BBS_ADMIN_H
#define PHP_BBS_ADMIN_H

/* 管理功能函数 */

PHP_FUNCTION(bbs_admin_getnewreg);
PHP_FUNCTION(bbs_admin_getuserinfo);
PHP_FUNCTION(bbs_admin_setuserinfo);


#define PHP_BBS_ADMIN_EXPORT_FUNCTIONS \
    PHP_FE(bbs_admin_getnewreg, NULL) \
    PHP_FE(bbs_admin_getuserinfo, NULL) \
    PHP_FE(bbs_admin_setuserinfo, NULL)


#endif /* PHP_BBS_ADMIN_H */

