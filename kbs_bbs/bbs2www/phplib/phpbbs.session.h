#ifndef PHP_BBS_SESSION_H
#define PHP_BBS_SESSION_H

/* 所有和用户登录状态有关的 */

PHP_FUNCTION(bbs_setfromhost);
PHP_FUNCTION(bbs_getonline_user_list);
PHP_FUNCTION(bbs_getonlineuser);
PHP_FUNCTION(bbs_getonlinenumber);
PHP_FUNCTION(bbs_getonlineusernumber);
PHP_FUNCTION(bbs_getwwwguestnumber);
PHP_FUNCTION(bbs_countuser);

PHP_FUNCTION(bbs_getcurrentuinfo);
PHP_FUNCTION(bbs_getcurrentuser);
PHP_FUNCTION(bbs_setonlineuser);
PHP_FUNCTION(bbs_wwwlogin);
PHP_FUNCTION(bbs_setguest_nologin);
PHP_FUNCTION(bbs_wwwlogoff);
PHP_FUNCTION(bbs_update_uinfo);
PHP_FUNCTION(bbs_session_modify_user_mode);

PHP_FUNCTION(bbs_getusermode);
PHP_FUNCTION(bbs_isonline);


#define PHP_BBS_SESSION_EXPORT_FUNCTIONS \
    PHP_FE(bbs_setfromhost, NULL) \
    PHP_FE(bbs_getonline_user_list, NULL) \
    PHP_FE(bbs_getonlineuser, NULL) \
    PHP_FE(bbs_getonlinenumber, NULL) \
    PHP_FE(bbs_getonlineusernumber,NULL) \
    PHP_FE(bbs_getwwwguestnumber,NULL) \
    PHP_FE(bbs_countuser, NULL) \
    PHP_FE(bbs_getcurrentuinfo, NULL) \
    PHP_FE(bbs_getcurrentuser, one_arg_force_ref_1) \
    PHP_FE(bbs_setonlineuser, fifth_arg_force_ref_00011) \
    PHP_FE(bbs_wwwlogin, NULL) \
    PHP_FE(bbs_setguest_nologin, NULL) \
    PHP_FE(bbs_wwwlogoff, NULL) \
    PHP_FE(bbs_update_uinfo, NULL) \
    PHP_FE(bbs_getusermode, NULL) \
    PHP_FE(bbs_isonline, NULL) \
    PHP_FE(bbs_session_modify_user_mode, NULL) \

#endif //PHP_BBS_SESSION_H

