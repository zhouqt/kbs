#ifndef PHP_BBS_REG_H
#define PHP_BBS_REG_H

/* 用户注册相关，包括用户注册资料读取和写入 */

PHP_FUNCTION(bbs_saveuserdata);
PHP_FUNCTION(bbs_createregform);
PHP_FUNCTION(bbs_fillidinfo);
PHP_FUNCTION(bbs_modify_info);

PHP_FUNCTION(bbs_createnewid);
PHP_FUNCTION(bbs_is_invalid_id);

PHP_FUNCTION(bbs_setactivation);
PHP_FUNCTION(bbs_getactivation);
PHP_FUNCTION(bbs_doactivation);
PHP_FUNCTION(bbs_sendactivation);

#define PHP_BBS_REG_EXPORT_FUNCTIONS \
    PHP_FE(bbs_saveuserdata, NULL) \
    PHP_FE(bbs_createregform,NULL) \
    PHP_FE(bbs_fillidinfo,NULL) \
    PHP_FE(bbs_modify_info,NULL) \
    PHP_FE(bbs_createnewid,NULL) \
    PHP_FE(bbs_is_invalid_id,NULL) \
    PHP_FE(bbs_setactivation,NULL) \
    PHP_FE(bbs_doactivation,NULL) \
    PHP_FE(bbs_sendactivation,NULL) \
    PHP_FE(bbs_getactivation,two_arg_force_ref_01) \

#endif //PHP_BBS_REG_H

