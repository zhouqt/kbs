#include "php_kbs_bbs.h"  

static void assign_userinfo(zval * array, struct user_info *uinfo, int num)
{
    add_assoc_long(array, "index", num);
    add_assoc_long(array, "active", uinfo->active);
    add_assoc_long(array, "uid", uinfo->uid);
    add_assoc_long(array, "pid", uinfo->pid);
    add_assoc_long(array, "invisible", uinfo->invisible);
    add_assoc_long(array, "sockactive", uinfo->sockactive);
    add_assoc_long(array, "sockaddr", uinfo->sockaddr);
    add_assoc_long(array, "destuid", uinfo->destuid);
    add_assoc_long(array, "mode", uinfo->mode);
    add_assoc_long(array, "pager", uinfo->pager);
    add_assoc_long(array, "in_chat", uinfo->in_chat);
    add_assoc_string(array, "chatid", uinfo->chatid, 1);
    add_assoc_string(array, "from", uinfo->from, 1);
    add_assoc_long(array, "logintime", uinfo->logintime);
    add_assoc_long(array, "freshtime", uinfo->freshtime);
    add_assoc_long(array, "utmpkey", uinfo->utmpkey);
    add_assoc_long(array, "mailbox_prop", uinfo->mailbox_prop);
    add_assoc_string(array, "userid", uinfo->userid, 1);
    add_assoc_string(array, "realname", uinfo->realname, 1);
    add_assoc_string(array, "username", uinfo->username, 1);
}

inline void setcurrentuinfo(struct user_info *uinfo, int uinfonum)
{
    getSession()->currentuinfo = uinfo;
    getSession()->utmpent = uinfonum;
}

inline void setcurrentuser(struct userec *user, int usernum)
{
	setCurrentUser(user);
    getSession()->currentuid = usernum;
}


PHP_FUNCTION(bbs_setfromhost)
{
    char *s;
    int s_len;
    int full_len;
    char *fullfromhostptr;

    if (zend_parse_parameters(2 TSRMLS_CC, "ss", &s, &s_len, &fullfromhostptr, &full_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    if (s_len > IPLEN)
        s[IPLEN] = 0;
    strcpy(getSession()->fromhost, s);
    RETURN_NULL();
}


struct fulluserlistarg{
    long start;
    long num;
    zval* return_value;
};

static int full_user_list(struct user_info *uentp, struct fulluserlistarg* arg,int count)
{
    struct user_info userinfo=*uentp;
    struct userec *lookupuser;
    zval* element;
    if (!userinfo.active || !userinfo.pid) {
        return 0;
    }
    if (!HAS_PERM(getCurrentUser(), PERM_SEECLOAK) && userinfo.invisible && strcmp(userinfo.userid, getCurrentUser()->userid)) {
        /*Haohmaru.99.4.24.让隐身者能看见自己 */
        return 0;
    }
    if (count+1<arg->start)
        return COUNT;
    if (count+1-arg->start>=arg->num)
        return QUIT;
    MAKE_STD_ZVAL ( element );
    array_init ( element );

    add_assoc_bool ( element, "invisible", userinfo.invisible );
    add_assoc_long ( element, "pid", userinfo.pid );
    add_assoc_bool ( element, "isfriend", isfriend(userinfo.userid) );
    add_assoc_string ( element, "userid", userinfo.userid, 1 );
    add_assoc_string ( element, "username", userinfo.username, 1 );
    if( getuser(userinfo.userid, &lookupuser) == 0 ) lookupuser=NULL;
    add_assoc_string ( element, "userfrom", HAS_PERM(getCurrentUser(), PERM_SYSOP)? userinfo.from: SHOW_USERIP(lookupuser, userinfo.from), 1 );
    add_assoc_string ( element, "mode", ModeType(userinfo.mode), 1 );
    add_assoc_long ( element, "idle", (long)(time(0) - userinfo.freshtime)/60 );
    
    zend_hash_index_update(Z_ARRVAL_P(arg->return_value), count+1-arg->start, (void *) &element, sizeof(zval *), NULL);
    return COUNT;
}

/*
 *  bbs_getonline_user_list
 *  获取再线用户列表
 *  @param start 开始位置
 *  @param num 获取的个数
 *  @return 在线用户数组
 *      格式：(invisible,isfriend,userid,username,userfrom,mode,idle(妙))
 */
PHP_FUNCTION(bbs_getonline_user_list)
{
    struct fulluserlistarg arg;
    int ac = ZEND_NUM_ARGS();

    if(ac != 2 || zend_parse_parameters(2 TSRMLS_CC,"ll",&arg.start,&arg.num) ==FAILURE){
        WRONG_PARAM_COUNT;
    }
    if (arg.start < 0 || arg.start > USHM_SIZE) RETURN_FALSE;
    if (array_init(return_value) == FAILURE)
        RETURN_FALSE;

    arg.return_value=return_value;
    apply_ulist_addr((APPLY_UTMP_FUNC) full_user_list, &arg);
}

PHP_FUNCTION(bbs_getonlineuser)
{
    long idx, ret;
    struct user_info *uinfo;
    zval *user_array;

    MAKE_STD_ZVAL(user_array);
    if (zend_parse_parameters(2 TSRMLS_CC, "la", &idx, &user_array) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    uinfo = get_utmpent(idx);
    if (uinfo == NULL)
        ret = 0;
    else {
        if (array_init(user_array) != SUCCESS)
            ret = 0;
        else {
            assign_userinfo(user_array, uinfo, idx);
            ret = idx;
        }
    }
    RETURN_LONG(ret);
}

PHP_FUNCTION(bbs_getonlinenumber)
{
    RETURN_LONG(get_utmp_number() + getwwwguestcount());
}

PHP_FUNCTION(bbs_getonlineusernumber)
{
    RETURN_LONG(get_utmp_number());
}

PHP_FUNCTION(bbs_getwwwguestnumber)
{
    RETURN_LONG(getwwwguestcount());
}


PHP_FUNCTION(bbs_countuser)
{
    long idx;

    if (zend_parse_parameters(2 TSRMLS_CC, "l", &idx) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    RETURN_LONG(apply_utmpuid(NULL, idx, 0));
}



PHP_FUNCTION(bbs_getcurrentuinfo)
{
    zval *user_array;
    long ret = 1;

    MAKE_STD_ZVAL(user_array);
    if (ZEND_NUM_ARGS() == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "a", &user_array) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        if (array_init(user_array) != SUCCESS) {
            ret = 0;
        } else {
            if (getSession()->currentuinfo) {
                assign_userinfo(user_array, getSession()->currentuinfo, getSession()->utmpent);
            } else
                ret = 0;
        }
    } else if (ZEND_NUM_ARGS() != 0)
        WRONG_PARAM_COUNT;
    if (ret)
        ret = getSession()->utmpent;

    RETURN_LONG(ret);
}

PHP_FUNCTION(bbs_getcurrentuser)
{
    zval *user_array;
    long ret;
    MAKE_STD_ZVAL(user_array);
    if (zend_parse_parameters(1 TSRMLS_CC, "a", &user_array) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }

    if (array_init(user_array) != SUCCESS) {
        ret = 0;
    } else {
        if (getCurrentUser()) {
            assign_user(user_array, getCurrentUser(), getSession()->currentuid);
            ret = getSession()->currentuid;
        } else
            ret = 0;
    }
    RETURN_LONG(ret);
}


PHP_FUNCTION(bbs_setonlineuser)
{
    zval *user_array;
    char *userid;
    int userid_len;
    long utmpnum;
    long utmpkey;
    long ret;
    struct user_info *pui = NULL;
    int idx;
    struct userec *user;
    long compat_telnet;

    MAKE_STD_ZVAL(user_array);
    if (ZEND_NUM_ARGS() == 4) {
        if (zend_parse_parameters(4 TSRMLS_CC, "slla", &userid, &userid_len, &utmpnum, &utmpkey, &user_array) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        compat_telnet=false;
    } else {
        if (zend_parse_parameters(5 TSRMLS_CC, "sllal", &userid, &userid_len, &utmpnum, &utmpkey, &user_array,&compat_telnet) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
    }
    if (userid_len > IDLEN)
        RETURN_LONG(1);
	/*
    if (utmpnum < 0 || utmpnum >= MAXACTIVE)
        RETURN_LONG(2);
		*/
    if (userid_len==0)
        userid=NULL;
    if ((ret = www_user_init(utmpnum, userid, utmpkey, &user, &pui, compat_telnet)) == 0) {
        setcurrentuinfo(pui, utmpnum);
        idx = getuser(pui->userid, &user);
        setcurrentuser(user, idx);
        if (user == NULL)
            RETURN_LONG(6);
        if (array_init(user_array) != SUCCESS)
            ret = 7;
        else {
            assign_userinfo(user_array, pui, idx);
            ret = 0;
        }
    }
    getSession()->currentuinfo=pui;
    RETURN_LONG(ret);
}


PHP_FUNCTION(bbs_wwwlogin)
{
    long ret;
    long kick_multi = 0;
    struct user_info *pu = NULL;
    int utmpent;
    char *fromhost, *fullfrom;
    int fromhostlen, fullfromlen;
    int ac = ZEND_NUM_ARGS();

    if (ac == 3) {
        if (zend_parse_parameters(3 TSRMLS_CC, "lss", &kick_multi, &fromhost, &fromhostlen, &fullfrom, &fullfromlen) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        if (fullfromlen > 256) fullfrom[256] = '\0';
        if (fromhostlen > IPLEN) fromhost[IPLEN] = '\0';
        strcpy(getSession()->fromhost, fromhost);
    } else if (ac == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "l", &kick_multi) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        fromhost = fullfrom = getSession()->fromhost;
    } else {
        WRONG_PARAM_COUNT;
    }
    
    if (getCurrentUser() != NULL && strcasecmp(getCurrentUser()->userid, "guest") != 0) {
        if (check_ip_acl(getCurrentUser()->userid, getSession()->fromhost)) 
            RETURN_LONG(7);
    }
    
    ret = www_user_login(getCurrentUser(), getSession()->currentuid, kick_multi, fromhost, fullfrom, &pu, &utmpent);
    if (getCurrentUser() == NULL) {
        struct userec *user;
        int num;

        num = getuser("guest", &user);
        setcurrentuser(user, num);
    }
    setcurrentuinfo(pu, utmpent);
    RETURN_LONG(ret);
}

/*
 * 本函数设置 currentuser 为 guest 但不登录，这样做非常的危险!
 * 除非你完全确定你自己在干什么，否则绝对不要调用这个函数! 目前只有极特殊的地方 wForum 需要调用这个函数
 */
PHP_FUNCTION(bbs_setguest_nologin)
{
    struct userec *user;
    int num;

    num = getuser("guest", &user);
    setcurrentuser(user, num);
}


PHP_FUNCTION(bbs_wwwlogoff)
{
    if (getCurrentUser()) {
        int ret = (www_user_logoff(getCurrentUser(), getSession()->currentuid,
                                   getSession()->currentuinfo, getSession()->utmpent));

        RETURN_LONG(ret);
    } else
        RETURN_LONG(-1);
}


/**
 * set currentuinfo for user.
 * prototype:
 * string bbs_update_uinfo(string field, value)
 * @return TRUE on success,
 *       FALSE on failure.
 * @author kcn
 */
PHP_FUNCTION(bbs_update_uinfo)
{
    zval *value;
    char *field;
    int field_len;
    int ac = ZEND_NUM_ARGS();

    if (ac != 2 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "sz", &field, &field_len, &value) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    if (!strcmp(field, "invisible")) {
/*	    conver_to_boolean_ex(&value);
 *	    */
        getSession()->currentuinfo->invisible = Z_LVAL_P(value);
    }
    RETURN_LONG(0);
}




PHP_FUNCTION(bbs_session_modify_user_mode)
{
    long mode;
    int ac = ZEND_NUM_ARGS();
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l", &mode) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (getSession() && getSession()->currentuinfo) {
        getSession()->currentuinfo->mode = mode;
        RETURN_LONG(0);
    }
    RETURN_LONG(-1);
}



static int printstatusstr(struct user_info *uentp, char *arg, int pos)
{
    if (uentp->invisible == 1) {
		arg[0] = '1';
        if (!HAS_PERM(getCurrentUser(), PERM_SEECLOAK))
            return COUNT;
    }
    if (arg[1]==0)
        strcat(arg, "目前在站上，状态如下：\n");
    if (uentp->invisible)
        strcat(arg, "<font class=\"c32\">隐身中</font>   ");
    else {
        char buf[80];
        if (uentp->pid == 1) {
            sprintf(buf, "<span class='blue'>%s</span> ", ModeType(uentp->mode));
        } else {
            sprintf(buf, "%s ", ModeType(uentp->mode));
        }
        strcat(arg, buf);
    }
    UNUSED_ARG(pos);
    return COUNT;
}

/* stiger: 获得一个用户的在线状态string */
static int get_userstatusstr(char *userid, char *buf)
{
	int tuid=0;
	struct userec *lookupuser;

	if(!(tuid = getuser(userid, &lookupuser)))
		return 0;

	buf[0]='0';
	buf[1]=0;

	return apply_utmp((APPLY_UTMP_FUNC) printstatusstr, 10, lookupuser->userid, buf);
}


PHP_FUNCTION(bbs_getusermode)
{
	char *userid;
	int userid_len;
	char buf[1024];

    if (zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
	
	if( userid_len > IDLEN )
		userid[IDLEN]=0;

	if( get_userstatusstr(userid, buf) == 0 )
		RETURN_LONG(0);

	RETURN_STRING(buf,1);
}



static int count_online(struct user_info *uentp, int *arg, int pos)
{
     if (uentp->invisible == 1) {
         return COUNT;
     }
     (*arg) = 1;
     UNUSED_ARG(pos);
     return QUIT;
}
/*
 * bbs_isonline(userid), return if this userid is online. If cloak, return false
 * 这个函数慎用，目前仅 wForum 用到 - atppp
 */
PHP_FUNCTION(bbs_isonline)
{
    int can_see = 0;
    struct userec *lookupuser;
    char *s;
    int s_len;

    if (ZEND_NUM_ARGS() != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &s, &s_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    if( ! getuser(s,&lookupuser) ) {
        RETURN_FALSE;
    }
    apply_utmp((APPLY_UTMP_FUNC) count_online, 0, lookupuser->userid, &can_see);
    if (can_see) RETURN_TRUE;
    RETURN_FALSE;
}

