#ifndef PHP_BBS_REG_H
#define PHP_BBS_REG_H

PHP_FUNCTION(bbs_saveuserdata);
PHP_FUNCTION(bbs_createregform);

#define PHP_BBS_REG_EXPORT_FUNCTIONS \
    PHP_FE(bbs_saveuserdata, NULL) \
    PHP_FE(bbs_createregform,NULL)

#endif //PHP_BBS_REG_H

