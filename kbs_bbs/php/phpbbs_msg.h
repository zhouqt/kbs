#ifndef PHP_BBS_MSG_H
#define PHP_BBS_MSG_H

/* 讯息及手机短信相关 */

PHP_FUNCTION(bbs_getwebmsgs);
PHP_FUNCTION(bbs_mailwebmsgs);
PHP_FUNCTION(bbs_checkwebmsg);
PHP_FUNCTION(bbs_getwebmsg);
PHP_FUNCTION(bbs_sendwebmsg);

#ifdef SMS_SUPPORT
PHP_FUNCTION(bbs_send_sms);
PHP_FUNCTION(bbs_register_sms_sendcheck);
PHP_FUNCTION(bbs_register_sms_docheck);
PHP_FUNCTION(bbs_unregister_sms);
#endif

#ifdef SMS_SUPPORT
#define PHP_BBS_SMS_EXPORT_FUNCTION \
    PHP_FE(bbs_send_sms,NULL) \
    PHP_FE(bbs_register_sms_sendcheck,NULL) \
    PHP_FE(bbs_unregister_sms,NULL) \
    PHP_FE(bbs_register_sms_docheck,NULL)
#else
#define PHP_BBS_SMS_EXPORT_FUNCTION
#endif

#define PHP_BBS_MSG_EXPORT_FUNCTIONS \
    PHP_FE(bbs_mailwebmsgs, NULL) \
    PHP_FE(bbs_getwebmsgs, NULL) \
    PHP_FE(bbs_checkwebmsg, NULL) \
    PHP_FE(bbs_getwebmsg, third_arg_force_ref_1111) \
    PHP_FE(bbs_sendwebmsg, fourth_arg_force_ref_0001) \
    PHP_BBS_SMS_EXPORT_FUNCTION

#endif //PHP_BBS_MSG_H

