#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_smth_bbs.h"  

#include "bbs.h"
#include "bbslib.h"



PHP_FUNCTION(bbs_getwebmsgs){
    char buf[MAX_MSG_SIZE];
    int count,i;
    struct msghead head;
	zval *element;

    if (ZEND_NUM_ARGS()!=0 ) {
        WRONG_PARAM_COUNT;
    }


    if(!HAS_PERM(getCurrentUser(), PERM_PAGE)) 
		RETURN_LONG(-1);
    if (array_init(return_value) == FAILURE) {
        RETURN_LONG(-2);
    }
    count = get_msgcount(0, getCurrentUser()->userid);
    if(count!=0) {
  	    for(i=0;i<count;i++) {
            load_msghead(0, getCurrentUser()->userid, i, &head);
            load_msgtext(getCurrentUser()->userid, &head, buf);
			MAKE_STD_ZVAL(element);
			array_init(element);
			add_assoc_string(element, "ID", head.id, 1);
			add_assoc_long(element, "TIME", head.time);
			add_assoc_string(element, "content", buf, 1);
			add_assoc_long(element, "MODE", head.mode);
			add_assoc_long(element, "SENT", head.sent?0:1);
			zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
        }
    }
}

PHP_FUNCTION(bbs_mailwebmsgs){
    if (ZEND_NUM_ARGS()!=0 ) {
        WRONG_PARAM_COUNT;
    }
	mail_msg(getCurrentUser(), getSession());
	RETURN_TRUE;
}


/**
 * check webmsg.
 * prototype:
 * bool bbs_checkwegmsg();
 *
 * @return TRUE if has any webmsg,
 *       FALSE otherwise.
 * @author flyriver
 */
PHP_FUNCTION(bbs_checkwebmsg)
{
	if( getSession()->currentuinfo==NULL || !(getSession()->currentuinfo->mailcheck & CHECK_MSG))
		RETURN_FALSE;

    RETURN_TRUE;
}

/**
 * receive webmsg.
 * prototype:
 * bool bbs_getwegmsg(string &srcid,string &buf,long &srcutmpent,long &sndtime);
 *
 * @return TRUE on success,
 *       FALSE on failure.
 *       and return total and unread in argument
 * @author KCN
 */
PHP_FUNCTION(bbs_getwebmsg)
{
    zval *retsrcid, *msgbuf, *srcutmpent, *z_sndtime;
    int ac = ZEND_NUM_ARGS();
    int srcpid;
	time_t sndtime;
    char buf[MSG_LEN + 1];
    char srcid[IDLEN + 1];

    if (ac != 4 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "zzzz", &retsrcid, &msgbuf, &srcutmpent, &z_sndtime) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    /*
     * check for parameter being passed by reference 
     */
    if (!PZVAL_IS_REF(retsrcid) || !PZVAL_IS_REF(msgbuf) || !PZVAL_IS_REF(srcutmpent)
		|| !PZVAL_IS_REF(z_sndtime)) {
        zend_error(E_WARNING, "Parameter wasn't passed by reference");
        RETURN_FALSE;
    }

	if( getSession()->currentuinfo==NULL || !(getSession()->currentuinfo->mailcheck & CHECK_MSG))
		RETURN_FALSE;

    if (receive_webmsg(getSession()->utmpent, getCurrentUser()->userid, &srcpid, srcid, &sndtime, buf) == 0) {
        ZVAL_STRING(retsrcid, srcid, 1);
        ZVAL_STRING(msgbuf, buf, 1);
        ZVAL_LONG(srcutmpent, srcpid);
        ZVAL_LONG(z_sndtime, sndtime);
        RETURN_TRUE;
    }
	getSession()->currentuinfo->mailcheck &= ~CHECK_MSG;
    /*
     * make changes to the parameter 
     */
    RETURN_FALSE;
}

/**
 * send web message.
 * prototype:
 * bool bbs_sendwegmsg(string destid,string buf,long destutmp,
 *                     string &errmsg);
 *
 * @return TRUE on success,
 *       FALSE on failure.
 * @author flyriver
 */
PHP_FUNCTION(bbs_sendwebmsg)
{
    char *destid;
    int destid_len;
    char *msg;
    int msg_len;
    long destutmp;
    zval *z_errmsg;
    int result;
    int ac = ZEND_NUM_ARGS();

    if (ac != 4 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "sslz", &destid, &destid_len, &msg, &msg_len, &destutmp, &z_errmsg) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    /*
     * check for parameter being passed by reference 
     */
    if (!PZVAL_IS_REF(z_errmsg)) {
        zend_error(E_WARNING, "Parameter wasn't passed by reference");
        RETURN_FALSE;
    }
    if (!msg_can_sendmsg(getSession()->currentuser, destid, destutmp)) {
        ZVAL_STRING(z_errmsg, "无法发送讯息", 1);
        RETURN_FALSE;
    }
    if (!strcasecmp(destid, getCurrentUser()->userid)) {
        ZVAL_STRING(z_errmsg, "你不能给自己发讯息", 1);
        RETURN_FALSE;
    }
    if ((result = send_msg(getCurrentUser()->userid, getSession()->utmpent, destid, destutmp, msg)) == 1) {
        ZVAL_STRING(z_errmsg, "已经帮你送出讯息", 1);
        RETURN_TRUE;
    } else if (result == -1) {
        char buf[STRLEN];

        snprintf(buf, sizeof(buf), "发送讯息失败，%s", getSession()->msgerr);
        ZVAL_STRING(z_errmsg, buf, 1);
        RETURN_FALSE;
    } else {
        ZVAL_STRING(z_errmsg, "发送讯息失败，此人目前不在线或者无法接收讯息", 1);
        RETURN_FALSE;
    }
}




#ifdef SMS_SUPPORT

PHP_FUNCTION(bbs_send_sms)
{
	int ac = ZEND_NUM_ARGS();
	char *dest,*msgstr;
	int dest_len,msgstr_len;
	int ret;

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ss", &dest, &dest_len, &msgstr, &msgstr_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	ret = web_send_sms( dest, msgstr );

	RETURN_LONG(ret);
}

PHP_FUNCTION(bbs_register_sms_sendcheck)
{
	int ac = ZEND_NUM_ARGS();
	char *dest;
	int dest_len;
	int ret;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &dest, &dest_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	ret = web_register_sms_sendcheck( dest );

	RETURN_LONG(ret);
}

PHP_FUNCTION(bbs_register_sms_docheck)
{
	int ac = ZEND_NUM_ARGS();
	char *dest;
	int dest_len;
	int ret;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &dest, &dest_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	ret = web_register_sms_docheck( dest );

	RETURN_LONG(ret);
}

PHP_FUNCTION(bbs_unregister_sms)
{
	int ret;

	ret = web_unregister_sms();

	RETURN_LONG(ret);
}

#endif


