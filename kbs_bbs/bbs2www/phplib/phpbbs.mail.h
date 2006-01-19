#ifndef PHP_BBS_MAIL_H
#define PHP_BBS_MAIL_H

/* 信件相关 */

PHP_FUNCTION(bbs_checknewmail);
PHP_FUNCTION(bbs_mail_get_num);
PHP_FUNCTION(bbs_getmailnum);
PHP_FUNCTION(bbs_getmailnum2);
PHP_FUNCTION(bbs_getmails);
PHP_FUNCTION(bbs_getmailusedspace);
PHP_FUNCTION(bbs_is_save2sent);
PHP_FUNCTION(bbs_can_send_mail);
PHP_FUNCTION(bbs_loadmaillist);
PHP_FUNCTION(bbs_changemaillist);

PHP_FUNCTION(bbs_postmail);
PHP_FUNCTION(bbs_mail_file);
PHP_FUNCTION(bbs_delmail);
PHP_FUNCTION(bbs_setmailreaded);
PHP_FUNCTION(bbs_domailforward);


#define PHP_BBS_MAIL_EXPORT_FUNCTIONS \
    PHP_FE(bbs_checknewmail, NULL) \
    PHP_FE(bbs_mail_get_num, NULL) \
    PHP_FE(bbs_getmailnum, third_arg_force_ref_011) \
    PHP_FE(bbs_getmailnum2, NULL) \
    PHP_FE(bbs_getmails, NULL) \
    PHP_FE(bbs_getmailusedspace, NULL) \
    PHP_FE(bbs_is_save2sent, NULL) \
    PHP_FE(bbs_can_send_mail, NULL) \
    PHP_FE(bbs_loadmaillist, NULL) \
    PHP_FE(bbs_changemaillist, NULL) \
    PHP_FE(bbs_postmail, NULL) \
    PHP_FE(bbs_mail_file, NULL) \
    PHP_FE(bbs_delmail,NULL) \
    PHP_FE(bbs_setmailreaded,NULL) \
    PHP_FE(bbs_domailforward, NULL)

#endif //PHP_BBS_MAIL_H

