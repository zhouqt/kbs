#ifndef PHP_BBS_USER_H
#define PHP_BBS_USER_H

PHP_FUNCTION(bbs_getuserparam);
PHP_FUNCTION(bbs_setuserparam);

#define PHP_BBS_USER_EXPORT_FUNCTIONS \
    PHP_FE(bbs_getuserparam, NULL) \
    PHP_FE(bbs_setuserparam, NULL)

#endif //PHP_BBS_USER_H
