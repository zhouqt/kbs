#include "php_kbs_bbs.h"


PHP_FUNCTION(bbs_getwebmsgs)
{
    char buf[MAX_MSG_SIZE];
    int count,i;
    struct msghead head;
    zval *element;

    if (ZEND_NUM_ARGS()!=0) {
        WRONG_PARAM_COUNT;
    }


    if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
        RETURN_LONG(-1);
    if (array_init(return_value) == FAILURE) {
        RETURN_LONG(-2);
    }
    count = get_msgcount(0, getCurrentUser()->userid);
    if (count!=0) {
        for (i=0;i<count;i++) {
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

PHP_FUNCTION(bbs_mailwebmsgs)
{
    if (ZEND_NUM_ARGS()!=0) {
        WRONG_PARAM_COUNT;
    }
    mail_msg(0, getCurrentUser(), getSession());
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
    if (getSession()->currentuinfo==NULL || !(getSession()->currentuinfo->mailcheck & CHECK_MSG))
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

    if (getSession()->currentuinfo==NULL || !(getSession()->currentuinfo->mailcheck & CHECK_MSG))
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
    // 可以给自己发讯息 - pig2532
    /* if (!strcasecmp(destid, getCurrentUser()->userid)) {
        ZVAL_STRING(z_errmsg, "你不能给自己发讯息", 1);
        RETURN_FALSE;
    } */
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

static int web_send_sms(char *dest,char *msgstr)
{
    struct userdata ud;
    char uident[STRLEN];
    char destid[STRLEN];
    bool cansend = true;
    struct userec *ur;
    int ret;
    char buf[MAX_MSG_SIZE];

    read_userdata(getCurrentUser()->userid, &ud);
    if (!ud.mobileregistered)
        return -1;

    if (!msgstr || !msgstr[0])
        return -3;

    sms_init_memory(getSession());
    getSession()->smsuin = getSession()->currentuinfo;

    if (isdigit(dest[0])) {
        int i;
        cansend = cansend && (strlen(dest) == 11);
        for (i=0;i<strlen(dest);i++)
            cansend = cansend && (isdigit(dest[i]));
        if (cansend)
            strcpy(uident,dest);
    } else {
        struct userdata destud;
        return -2;
        getuser(dest, &ur);
        if (ur)
            strcpy(destid, ur->userid);
        if (read_userdata(destid, &destud))
            cansend = false;
        else
            cansend = destud.mobileregistered && (strlen(destud.mobilenumber)==11);
        if (cansend)
            strcpy(uident, destud.mobilenumber);
    }

    if (!cansend) {
        shmdt(getSession()->head);
        return -2;
    }

    strncpy(buf, msgstr, MAX_MSG_SIZE);
    buf[MAX_MSG_SIZE-1]=0;

    if (strlen(buf) + strlen(ud.smsprefix) + strlen(ud.smsend) < MAX_MSG_SIZE) {
        int i,i1,j;

        i=strlen(buf);
        i1=strlen(ud.smsprefix);
        for (j= i+i1; j>=i1; j--) {
            buf[j] = buf[j-i1];
        }
        for (j=0;j<i1;j++)
            buf[j] = ud.smsprefix[j];
        strcat(buf, ud.smsend);

    }

    ret = DoSendSMS(ud.mobilenumber, uident, buf,getSession());

    if (ret == CMD_ERR_SMS_VALIDATE_FAILED) {
        if (read_user_memo(getCurrentUser()->userid, &getSession()->currentmemo) <= 0) return -1;
        ud.mobilenumber[0]=0;
        ud.mobileregistered=0;
        memcpy(&(getSession()->currentmemo->ud), &ud, sizeof(ud));
        end_mmapfile(getSession()->currentmemo, sizeof(struct usermemo), -1);
        write_userdata(getCurrentUser()->userid, &ud);
    }

    if (ret) {
        shmdt(getSession()->head);
        return 1;
    } else {
        struct msghead h;
        struct user_info *uin;
        h.frompid = getSession()->currentuinfo->pid;
        h.topid = -1;
        if (!isdigit(dest[0])) {
            uin = t_search(destid, false);
            if (uin) h.topid = uin->pid;
            strcpy(h.id, destid);
        } else
            strcpy(h.id, uident);
        h.mode = 6;
        h.sent = 1;
        h.time = time(0);
        save_msgtext(getCurrentUser()->userid, &h, buf,getSession());
#if HAVE_MYSQL_SMTH == 1
        save_smsmsg(getCurrentUser()->userid, &h, buf, 1, getSession());
#endif
        if (!isdigit(dest[0])) {
            h.sent = 0;
            strcpy(h.id, getCurrentUser()->userid);
            save_msgtext(destid, &h, buf,getSession());
#if HAVE_MYSQL_SMTH == 1
            save_smsmsg(uident, &h, buf, 1, getSession());
#endif
            if (uin) kill(uin->pid, SIGUSR2);
        }
    }

    shmdt(getSession()->head);
    return 0;

}

static int web_register_sms_sendcheck(char *mnumber)
{
    char ans[4];
    char valid[20];
    char buf2[80];
    struct userdata ud;
    int i;

    if (read_user_memo(getCurrentUser()->userid, &getSession()->currentmemo) <= 0) return -1;
    memcpy(&ud, &(getSession()->currentmemo->ud), sizeof(ud));

    sms_init_memory(getSession());
    getSession()->smsuin = getSession()->currentuinfo;

    if (ud.mobileregistered) {
        shmdt(getSession()->head);
        return -1;
    }

    if (mnumber == NULL) {
        shmdt(getSession()->head);
        return -2;
    }

    if (strlen(mnumber) != 11) {
        shmdt(getSession()->head);
        return -3;
    }

    for (i=0;i <11; i++) {
        if (! isdigit(mnumber[i])) {
            shmdt(getSession()->head);
            return -4;
        }
    }

    if (DoReg(mnumber)) {
        shmdt(getSession()->head);
        return -5;
    }

    strcpy(ud.mobilenumber, mnumber);
    memcpy(&(getSession()->currentmemo->ud), &ud, sizeof(ud));
    end_mmapfile(getSession()->currentmemo, sizeof(struct usermemo), -1);
    write_userdata(getCurrentUser()->userid, &ud);

    shmdt(getSession()->head);
    return 0;
}

static int web_register_sms_docheck(char *valid)
{
    char ans[4];
    char buf2[80];
    struct userdata ud;

    if (read_user_memo(getCurrentUser()->userid, &getSession()->currentmemo) <= 0) return -1;
    memcpy(&ud, &(getSession()->currentmemo->ud), sizeof(ud));

    sms_init_memory(getSession());
    getSession()->smsuin = getSession()->currentuinfo;

    if (ud.mobileregistered) {
        shmdt(getSession()->head);
        return -1;
    }

    if (! ud.mobilenumber[0] || strlen(ud.mobilenumber)!=11) {
        shmdt(getSession()->head);
        return -2;
    }

    if (valid == NULL || !valid[0]) {
        shmdt(getSession()->head);
        return -3;
    }

    if (DoCheck(ud.mobilenumber, valid)) {
        shmdt(getSession()->head);
        return -4;
    }

    ud.mobileregistered = 1;
    memcpy(&(getSession()->currentmemo->ud), &ud, sizeof(ud));
    end_mmapfile(getSession()->currentmemo, sizeof(struct usermemo), -1);
    write_userdata(getCurrentUser()->userid, &ud);

    shmdt(getSession()->head);
    return 0;
}

static int web_unregister_sms()
{
    char ans[4];
    char valid[20];
    char buf2[80];
    int rr;

    if (read_user_memo(getCurrentUser()->userid, &getSession()->currentmemo) <= 0) return -1;
    sms_init_memory(getSession());
    getSession()->smsuin = getSession()->currentuinfo;

    if (!getSession()->currentmemo->ud.mobileregistered) {
        shmdt(getSession()->head);
        getSession()->smsbuf=NULL;
        return -1;
    }

    rr = DoUnReg(getSession()->currentmemo->ud.mobilenumber,getSession());
    if (rr&&rr!=CMD_ERR_NO_SUCHMOBILE) {
        shmdt(getSession()->head);
        getSession()->currentmemo->ud.mobileregistered = 0;
        write_userdata(getCurrentUser()->userid, &(getSession()->currentmemo->ud));
        end_mmapfile(getSession()->currentmemo, sizeof(struct usermemo), -1);
        getSession()->smsbuf=NULL;
        return -1;
    }

    getSession()->currentmemo->ud.mobilenumber[0]=0;
    getSession()->currentmemo->ud.mobileregistered = 0;
    write_userdata(getCurrentUser()->userid, &(getSession()->currentmemo->ud));
    end_mmapfile(getSession()->currentmemo, sizeof(struct usermemo), -1);

    shmdt(getSession()->head);
    getSession()->smsbuf=NULL;

    return 0;
}







PHP_FUNCTION(bbs_send_sms)
{
    int ac = ZEND_NUM_ARGS();
    char *dest,*msgstr;
    int dest_len,msgstr_len;
    int ret;

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ss", &dest, &dest_len, &msgstr, &msgstr_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    ret = web_send_sms(dest, msgstr);

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

    ret = web_register_sms_sendcheck(dest);

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

    ret = web_register_sms_docheck(dest);

    RETURN_LONG(ret);
}

PHP_FUNCTION(bbs_unregister_sms)
{
    int ret;

    ret = web_unregister_sms();

    RETURN_LONG(ret);
}

#endif


