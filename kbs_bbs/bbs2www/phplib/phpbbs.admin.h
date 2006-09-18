#ifndef PHP_BBS_ADMIN_H
#define PHP_BBS_ADMIN_H

/* 管理功能函数 */

PHP_FUNCTION(bbs_admin_getnewreg);
PHP_FUNCTION(bbs_admin_getuserinfo);
PHP_FUNCTION(bbs_admin_setuserinfo);
PHP_FUNCTION(bbs_admin_getuserperm);
PHP_FUNCTION(bbs_admin_getgiveupperm);
PHP_FUNCTION(bbs_admin_setuserperm);


#define PHP_BBS_ADMIN_EXPORT_FUNCTIONS \
    PHP_FE(bbs_admin_getnewreg, NULL) \
    PHP_FE(bbs_admin_getuserinfo, NULL) \
    PHP_FE(bbs_admin_setuserinfo, NULL) \
    PHP_FE(bbs_admin_getuserperm, NULL) \
    PHP_FE(bbs_admin_getgiveupperm, NULL) \
    PHP_FE(bbs_admin_setuserperm, NULL)


#endif /* PHP_BBS_ADMIN_H */

