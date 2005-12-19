#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_smth_bbs.h"  

#include "bbs.h"
#include "bbslib.h"

static char fullfrom[255];

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

static void assign_user(zval * array, struct userec *user, int num)
{
    struct userdata ud;

    read_userdata(user->userid, &ud);
    add_assoc_long(array, "index", num);
    add_assoc_string(array, "userid", user->userid, 1);
    add_assoc_long(array, "firstlogin", user->firstlogin);
    add_assoc_long(array, "exittime", user->exittime);
//    add_assoc_stringl(array, "lasthost", user->lasthost, IPLEN, 1);
    add_assoc_string(array, "lasthost", (!strcmp(user->userid , getCurrentUser()->userid) || HAS_PERM(getCurrentUser(), PERM_SYSOP)) ? user->lasthost: SHOW_USERIP(user, user->lasthost), 1);
    add_assoc_long(array, "numlogins", user->numlogins);
    add_assoc_long(array, "numposts", user->numposts);
    add_assoc_long(array, "flag1", user->flags);
    add_assoc_long(array, "title", user->title);
    add_assoc_string(array, "username", user->username, 1);
    add_assoc_stringl(array, "md5passwd", (char *) user->md5passwd, 16, 1);
    add_assoc_string(array, "realemail", ud.realemail, 1);
    add_assoc_long(array, "userlevel", user->userlevel);
    add_assoc_long(array, "lastlogin", user->lastlogin);
    add_assoc_long(array, "stay", user->stay);
    add_assoc_string(array, "realname", ud.realname, 1);
    add_assoc_string(array, "address", ud.address, 1);
    add_assoc_string(array, "email", ud.email, 1);
    add_assoc_long(array, "signature", user->signature);
    add_assoc_long(array, "signum", ud.signum);
    add_assoc_long(array, "userdefine0", user->userdefine[0]);
    add_assoc_long(array, "userdefine1", user->userdefine[1]);

	#ifdef HAVE_BIRTHDAY
	add_assoc_long(array,"gender",ud.gender);
	add_assoc_long(array,"birthyear",ud.birthyear);
    add_assoc_long(array,"birthmonth",ud.birthmonth);
    add_assoc_long(array,"birthday", ud.birthday);
	#endif

    add_assoc_string(array,"reg_email",ud.reg_email,1);
    add_assoc_long(array,"mobilderegistered", ud.mobileregistered);
    add_assoc_string(array, "mobilenumber", ud.mobilenumber,1);

#ifdef HAVE_WFORUM
    add_assoc_string(array,"OICQ",ud.OICQ,1);
    add_assoc_string(array,"ICQ",ud.ICQ,1);
    add_assoc_string(array,"MSN", ud.MSN,1);
    add_assoc_string(array,"homepage",ud.homepage,1);
    add_assoc_long(array,"userface_img", ud.userface_img);
	add_assoc_string(array,"userface_url", ud.userface_url,1);
	add_assoc_long(array,"userface_width", ud.userface_width);
	add_assoc_long(array,"userface_height", ud.userface_height);
	add_assoc_long(array,"group",ud.group);
    add_assoc_string(array,"country", ud.country,1);
    add_assoc_string(array,"province", ud.province,1);
    add_assoc_string(array,"city",ud.city,1);
    add_assoc_long(array,"shengxiao",ud.shengxiao);
    add_assoc_long(array,"bloodtype", ud.bloodtype);
    add_assoc_long(array,"religion",ud.religion);
    add_assoc_long(array,"profession",ud.profession);
    add_assoc_long(array,"married",ud.married);
    add_assoc_long(array,"education", ud.education);
    add_assoc_string(array,"graduateschool",ud.graduateschool,1);
    add_assoc_long(array,"character", ud.character);
	add_assoc_string(array,"photo_url", ud.photo_url,1);
	add_assoc_string(array,"telephone", ud.telephone,1);
#endif

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


/* arguments: userid, username, ipaddr, operation */
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
    if (full_len > 80)
        fullfromhostptr[80] = 0;
    strcpy(fullfrom, fullfromhostptr);
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


PHP_FUNCTION(bbs_getuser)
{
    long v1;
    struct userec *lookupuser;
    char *s;
    int s_len;
    zval *user_array;

    MAKE_STD_ZVAL(user_array);
    if (zend_parse_parameters(2 TSRMLS_CC, "sa", &s, &s_len, &user_array) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }

    if (s_len > IDLEN)
        s[IDLEN] = 0;
    v1 = getuser(s, &lookupuser);

    if (v1 == 0)
        RETURN_LONG(0);

    if (array_init(user_array) != SUCCESS)
        RETURN_LONG(0);
    assign_user(user_array, lookupuser, v1);
/*        RETURN_STRING(retbuf, 1);
 *        */
    RETURN_LONG(v1);
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


PHP_FUNCTION(bbs_checkpasswd)
{
    char *s;
    int s_len;
    char *pw;
    int pw_len;
    long ret;
    int unum = 0;
    long ismd5 = 0;
    struct userec *user;
    int ac = ZEND_NUM_ARGS();

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ss", &s, &s_len, &pw, &pw_len) != SUCCESS) {
        if (ac!= 3 || zend_parse_parameters(3 TSRMLS_CC, "ssl", &s, &s_len, &pw, &pw_len, &ismd5) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
    }
    if (s_len > IDLEN)
        s[IDLEN] = 0;
    if (pw_len > PASSLEN)
        pw[PASSLEN] = 0;
    if (pw[0] == '\0')
        ret = 1;
    else if ((s[0] != 0) && !(unum = getuser(s, &user)))
        ret = 2;
    else {
        if (s[0] == 0)
            user = getCurrentUser();
        if (user) {
            if (ismd5) {
                ismd5 = !(memcmp(pw, user->md5passwd, MD5PASSLEN));
            } else {
                ismd5 = checkpasswd2(pw, user);
            }
            if (ismd5) {
                ret = 0;
                if (s[0] != 0)
                    setcurrentuser(user, unum);
            } else {
                ret = 1;
                logattempt(user->userid, getSession()->fromhost, "www");
            }
        } else {
            ret = 1;
        }
    }
    RETURN_LONG(ret);
}


PHP_FUNCTION(bbs_wwwlogin)
{
    long ret;
    long kick_multi = 0;
    struct user_info *pu = NULL;
    int utmpent;

    if (ZEND_NUM_ARGS() == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "l", &kick_multi) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
    } else if (ZEND_NUM_ARGS() != 0)
        WRONG_PARAM_COUNT;
    
    if (getCurrentUser() != NULL && strcasecmp(getCurrentUser()->userid, "guest") != 0) {
        if (check_ip_acl(getCurrentUser()->userid, getSession()->fromhost)) 
            RETURN_LONG(7);
    }
    
    ret = www_user_login(getCurrentUser(), getSession()->currentuid, kick_multi, getSession()->fromhost,
#ifdef SQUID_ACCL
                         fullfrom,
#else
                         getSession()->fromhost,
#endif
                         &pu, &utmpent);
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

PHP_FUNCTION(bbs_getwwwparameters)
{
	FILE* fn;
	char  buf[1024];
	
	sethomefile(buf,getCurrentUser()->userid,"www");
    if ((fn=fopen(buf,"r"))==NULL) {
        strcpy(buf, "0");
    } else {
        fgets(buf,1024,fn);
        fclose(fn);
    }
	RETURN_STRING(buf,1);
}

PHP_FUNCTION(bbs_setwwwparameters)
{
	char* wwwparameters;
	int   wwwparameters_len;
	FILE *fn;
	char  buf[201];
	
	int ac = ZEND_NUM_ARGS();
	
	if (ac != 1 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "s" , &wwwparameters , &wwwparameters_len) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}
	
	if(wwwparameters_len > 200)
		RETURN_LONG(-1);
	sethomefile(buf,getCurrentUser()->userid,"www");
	if ((fn=fopen(buf,"w"))==NULL)
		RETURN_LONG(-10);
	fprintf(fn,"%s",wwwparameters);
	fclose(fn);
	RETURN_LONG(0);
}

PHP_FUNCTION(bbs_getuserparam){//这个函数总有一天要被我杀掉！！ - atppp
	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}
	if (getCurrentUser()==NULL) {
		RETURN_FALSE;
	}
	RETURN_LONG(getCurrentUser()->userdefine[0]);
}

PHP_FUNCTION(bbs_setuserparam){
	long userparam0, userparam1, mailboxprop;
	if (ZEND_NUM_ARGS() != 3 || zend_parse_parameters(3 TSRMLS_CC, "lll", &userparam0, &userparam1, &mailboxprop) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}
	if (getCurrentUser()==NULL) {
		RETURN_LONG(-1);
	}
	getCurrentUser()->userdefine[0] = userparam0;
    getCurrentUser()->userdefine[1] = userparam1;
    getSession()->currentuinfo->mailbox_prop = update_mailbox_prop(getCurrentUser()->userid, mailboxprop);
    store_mailbox_prop(getCurrentUser()->userid);
	RETURN_LONG(0);
}

