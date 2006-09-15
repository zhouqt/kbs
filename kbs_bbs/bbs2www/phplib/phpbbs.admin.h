#ifndef PHP_BBS_ADMIN_H
#define PHP_BBS_ADMIN_H


PHP_FUNCTION(bbs_admin_getnewreg);


#define PHP_BBS_ADMIN_EXPORT_FUNCTIONS \
    PHP_FE(bbs_admin_getnewreg, NULL)


#endif /* PHP_BBS_ADMIN_H */

