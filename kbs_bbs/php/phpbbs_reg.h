#ifndef PHP_BBS_REG_H
#define PHP_BBS_REG_H

/* 用户注册相关，包括用户注册资料读取和写入 */

PHP_FUNCTION(bbs_saveuserdata);
PHP_FUNCTION(bbs_createregform);
PHP_FUNCTION(bbs_fillidinfo);
PHP_FUNCTION(bbs_modify_info);

PHP_FUNCTION(bbs_createnewid);
PHP_FUNCTION(bbs_is_invalid_id);

#ifdef HAVE_ACTIVATION
PHP_FUNCTION(bbs_setactivation);
PHP_FUNCTION(bbs_getactivation);
PHP_FUNCTION(bbs_doactivation);
PHP_FUNCTION(bbs_sendactivation);
#endif
#ifdef NEWSMTH
PHP_FUNCTION(bbs_getinvite_email);
PHP_FUNCTION(bbs_cleaninvite);
PHP_FUNCTION(bbs_invite);
#endif

#define PHP_BBS_REG_EXPORT_FUNCTIONS1 \
    PHP_FE(bbs_saveuserdata, NULL) \
    PHP_FE(bbs_createregform,NULL) \
    PHP_FE(bbs_fillidinfo,NULL) \
    PHP_FE(bbs_modify_info,NULL) \
    PHP_FE(bbs_createnewid,NULL) \
    PHP_FE(bbs_is_invalid_id,NULL)

#ifdef HAVE_ACTIVATION
#define PHP_BBS_REG_EXPORT_FUNCTIONS2 \
    PHP_FE(bbs_setactivation,NULL) \
    PHP_FE(bbs_doactivation,NULL) \
    PHP_FE(bbs_sendactivation,NULL) \
    PHP_FE(bbs_getactivation,two_arg_force_ref_01)
#else
#define PHP_BBS_REG_EXPORT_FUNCTIONS2
#endif

#if defined(NEWSMTH) && defined(HAVE_ACTIVATION)
#define PHP_BBS_REG_EXPORT_FUNCTIONS3 \
    PHP_FE(bbs_getinvite_email, NULL) \
    PHP_FE(bbs_cleaninvite, NULL) \
    PHP_FE(bbs_invite, NULL)
#else
#define PHP_BBS_REG_EXPORT_FUNCTIONS3
#endif

#define PHP_BBS_REG_EXPORT_FUNCTIONS \
    PHP_BBS_REG_EXPORT_FUNCTIONS1 \
    PHP_BBS_REG_EXPORT_FUNCTIONS2 \
    PHP_BBS_REG_EXPORT_FUNCTIONS3


#endif //PHP_BBS_REG_H

