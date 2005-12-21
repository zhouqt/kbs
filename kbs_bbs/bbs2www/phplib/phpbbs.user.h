#ifndef PHP_BBS_USER_H
#define PHP_BBS_USER_H

PHP_FUNCTION(bbs_setfromhost);
PHP_FUNCTION(bbs_getonline_user_list);
PHP_FUNCTION(bbs_getonlineuser);
PHP_FUNCTION(bbs_getonlinenumber);
PHP_FUNCTION(bbs_getonlineusernumber);
PHP_FUNCTION(bbs_getwwwguestnumber);
PHP_FUNCTION(bbs_countuser);
PHP_FUNCTION(bbs_getuser);
PHP_FUNCTION(bbs_getcurrentuinfo);
PHP_FUNCTION(bbs_setonlineuser);
PHP_FUNCTION(bbs_getcurrentuser);
PHP_FUNCTION(bbs_checkpasswd);
PHP_FUNCTION(bbs_wwwlogin);
PHP_FUNCTION(bbs_setguest_nologin);
PHP_FUNCTION(bbs_wwwlogoff);
PHP_FUNCTION(bbs_getwwwparameters);
PHP_FUNCTION(bbs_setwwwparameters);
PHP_FUNCTION(bbs_getuserparam);
PHP_FUNCTION(bbs_setuserparam);

#define PHP_BBS_USER_EXPORT_FUNCTIONS \
    PHP_FE(bbs_setfromhost, NULL) \
    PHP_FE(bbs_getonline_user_list, NULL) \
    PHP_FE(bbs_getonlineuser, NULL) \
    PHP_FE(bbs_getonlinenumber, NULL) \
    PHP_FE(bbs_getonlineusernumber,NULL) \
    PHP_FE(bbs_getwwwguestnumber,NULL) \
    PHP_FE(bbs_countuser, NULL) \
    PHP_FE(bbs_getuser, NULL) \
    PHP_FE(bbs_getcurrentuinfo, NULL) \
    PHP_FE(bbs_setonlineuser, fifth_arg_force_ref_00011) \
    PHP_FE(bbs_getcurrentuser, one_arg_force_ref_1) \
    PHP_FE(bbs_checkpasswd, NULL) \
    PHP_FE(bbs_wwwlogin, NULL) \
    PHP_FE(bbs_setguest_nologin, NULL) \
    PHP_FE(bbs_wwwlogoff, NULL) \
    PHP_FE(bbs_getwwwparameters,one_arg_force_ref_1) \
    PHP_FE(bbs_setwwwparameters,NULL) \
    PHP_FE(bbs_getuserparam, NULL) \
    PHP_FE(bbs_setuserparam, NULL)


#endif //PHP_BBS_USER_H
