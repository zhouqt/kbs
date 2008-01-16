#ifndef PHP_BBS_USER_H
#define PHP_BBS_USER_H

/* 用户基本信息和参数 */

PHP_FUNCTION(bbs_getuser);

PHP_FUNCTION(bbs_checkpasswd);
PHP_FUNCTION(bbs_checkuserpasswd);
PHP_FUNCTION(bbs_setuserpasswd);
PHP_FUNCTION(bbs_setpassword);
PHP_FUNCTION(bbs_findpwd_check);

PHP_FUNCTION(bbs_getwwwparameters);
PHP_FUNCTION(bbs_setwwwparameters);
PHP_FUNCTION(bbs_getuserparam);
PHP_FUNCTION(bbs_setuserparam);

PHP_FUNCTION(bbs_user_setflag);
PHP_FUNCTION(bbs_recalc_sig);
PHP_FUNCTION(bbs_modify_nick);

PHP_FUNCTION(bbs_getuserlevel);
PHP_FUNCTION(bbs_compute_user_value);
PHP_FUNCTION(bbs_user_level_char);



#define PHP_BBS_USER_EXPORT_FUNCTIONS \
    PHP_FE(bbs_getuser, NULL) \
    PHP_FE(bbs_checkpasswd, NULL) \
    PHP_FE(bbs_checkuserpasswd, NULL) \
    PHP_FE(bbs_setuserpasswd, NULL) \
    PHP_FE(bbs_setpassword,NULL) \
    PHP_FE(bbs_findpwd_check,NULL) \
    PHP_FE(bbs_getwwwparameters,one_arg_force_ref_1) \
    PHP_FE(bbs_setwwwparameters,NULL) \
    PHP_FE(bbs_getuserparam, NULL) \
    PHP_FE(bbs_setuserparam, NULL) \
    PHP_FE(bbs_user_setflag, NULL) \
    PHP_FE(bbs_recalc_sig,NULL) \
    PHP_FE(bbs_modify_nick,NULL) \
    PHP_FE(bbs_getuserlevel, NULL) \
    PHP_FE(bbs_compute_user_value, NULL) \
    PHP_FE(bbs_user_level_char, NULL) \


#endif //PHP_BBS_USER_H
