#ifndef PHP_BBS_REG_H
#define PHP_BBS_REG_H

PHP_FUNCTION(bbs_saveuserdata);
PHP_FUNCTION(bbs_createregform);

PHP_FUNCTION(bbs_createnewid);
PHP_FUNCTION(bbs_is_invalid_id);
PHP_FUNCTION(bbs_setactivation);
PHP_FUNCTION(bbs_getactivation);
PHP_FUNCTION(bbs_doactivation);
PHP_FUNCTION(bbs_sendactivation);
PHP_FUNCTION(bbs_fillidinfo);

#define PHP_BBS_REG_EXPORT_FUNCTIONS \
    PHP_FE(bbs_saveuserdata, NULL) \
    PHP_FE(bbs_createregform,NULL) \
    PHP_FE(bbs_createnewid,NULL) \
    PHP_FE(bbs_is_invalid_id,NULL) \
    PHP_FE(bbs_setactivation,NULL) \
    PHP_FE(bbs_doactivation,NULL) \
    PHP_FE(bbs_sendactivation,NULL) \
    PHP_FE(bbs_getactivation,two_arg_force_ref_01) \
    PHP_FE(bbs_fillidinfo,NULL)

#endif //PHP_BBS_REG_H

