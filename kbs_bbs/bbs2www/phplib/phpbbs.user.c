#include "php_kbs_bbs.h"  

//暂时放这里
#define MANAGERSHIP(user) (getCurrentUser() && (HAS_PERM(getCurrentUser(), PERM_SYSOP) || !strcmp((user)->userid , getCurrentUser()->userid)))

void assign_user(zval * array, struct userec *user, int num)
{
    struct userdata ud;

    read_userdata(user->userid, &ud);
    add_assoc_long(array, "index", num);
    add_assoc_string(array, "userid", user->userid, 1);
    add_assoc_long(array, "firstlogin", user->firstlogin);
    add_assoc_long(array, "exittime", user->exittime);
//    add_assoc_stringl(array, "lasthost", user->lasthost, IPLEN, 1);
    add_assoc_string(array, "lasthost", MANAGERSHIP(user) ? user->lasthost: SHOW_USERIP(user, user->lasthost), 1);
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

#ifdef NEWSMTH
    if (MANAGERSHIP(user)) add_assoc_long(array,"score_user",user->score_user);
#endif
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

PHP_FUNCTION(bbs_checkuserpasswd){
    char *s;
    int s_len;
    char *pw;
    int pw_len;
    int unum;
    struct userec *user;

    if (zend_parse_parameters(2 TSRMLS_CC, "ss", &s, &s_len, &pw, &pw_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    if (s_len > IDLEN)
        s[IDLEN] = 0;
    if (pw_len > PASSLEN)
        pw[PASSLEN] = 0;
	if (pw_len < 2) {
		RETURN_LONG(-1);
	}
    if ( !(unum = getuser(s, &user))) {
        RETURN_LONG(-2);
    }
	   if ( !checkpasswd2(pw, user)) {
        RETURN_LONG(-3);
    }
    RETURN_LONG(0);
}


PHP_FUNCTION(bbs_setuserpasswd){
    char *s;
    int s_len;
    char *pw;
    int pw_len;
    int unum;
    struct userec *user;

    if (zend_parse_parameters(2 TSRMLS_CC, "ss", &s, &s_len, &pw, &pw_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    if (s_len > IDLEN)
        s[IDLEN] = 0;
    if (pw_len > PASSLEN)
        pw[PASSLEN] = 0;
	if (pw_len < 2) {
		RETURN_LONG(-1);
	}
    if ( !(unum = getuser(s, &user))) {
        RETURN_LONG(-2);
    }
	setpasswd(pw, user);
    RETURN_LONG(0);
}


/**
 * set password for user.
 * prototype:
 * string bbs_setpassword(string userid, string password)
 * @if userid=="" then user=getCurrentUser()
 * @return TRUE on success,
 *       FALSE on failure.
 * @author kcn
 */
PHP_FUNCTION(bbs_setpassword)
{
    char *userid;
    int userid_len;
    char *password;
    int password_len;
    int ac = ZEND_NUM_ARGS();
    struct userec *user;

    if (ac != 2 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "ss", &userid, &userid_len, &password, &password_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (userid != 0) {
        if (getuser(userid, &user) == 0)
            RETURN_FALSE;
    } else
        user = getCurrentUser();
    if (setpasswd(password, user) != 1)
        RETURN_FALSE;
    RETURN_TRUE;
}



/**
 *  Function: 根据注册姓名和email生成新的密码.如果用户名为空,则生成一个密码.
 *   string bbs_findpwd_check(string userid,string realname,string email);
 *
 *   if failed. reaturn NULL string; or return new password.
 *              by binxun
 */
PHP_FUNCTION(bbs_findpwd_check)
{
    char*   userid,
	        *realname,
            *email;
	int     userid_len,
	        realname_len,
			email_len;
	char    pwd[30];
    struct userdata ud;
	struct userec* uc;

	int ac = ZEND_NUM_ARGS();
    chdir(BBSHOME);

    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sss", &userid,&userid_len,&realname,&realname_len,&email,&email_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	pwd[0] = 0;
    if(userid_len > IDLEN)RETURN_LONG(1);

	//生成新密码
	srand(time(NULL));
	sprintf(pwd,"%d",rand());

	if(userid_len > 0){
        if(getuser(userid,&uc) == 0)RETURN_LONG(3);
	    if(read_userdata(userid,&ud)<0)RETURN_LONG(4);

	    if(!strncmp(userid,ud.userid,IDLEN) && !strncmp(email,ud.email,STRLEN))
	    {
		    setpasswd(pwd,uc);
	    }
	    else
	        RETURN_LONG(5);
	}

    RETURN_STRING(pwd,1);
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




PHP_FUNCTION(bbs_user_setflag)
{
    struct userec *lookupuser;
    char *s;
    int s_len;
    long flag, set;

    if (ZEND_NUM_ARGS() != 3 || zend_parse_parameters(3 TSRMLS_CC, "sll", &s, &s_len, &flag, &set) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    if( ! getuser(s,&lookupuser) ) {
        RETURN_FALSE;
    }
    if (set) {
        lookupuser->flags |= flag;
    } else {
        lookupuser->flags &= ~flag;
    }
    RETURN_TRUE;
}




PHP_FUNCTION(bbs_recalc_sig)
{
	struct userec newinfo;
	int unum;
	int sign;

    if( (unum = searchuser(getCurrentUser()->userid))==0)
		RETURN_LONG(-1);
	memcpy(&newinfo, getCurrentUser(), sizeof(struct userec));
    
	if( read_user_memo(getCurrentUser()->userid, &(getSession()->currentmemo)) <= 0) RETURN_LONG(-2);

    (getSession()->currentmemo)->ud.signum = calc_numofsig(getCurrentUser()->userid);
	sign = (getSession()->currentmemo)->ud.signum;

    write_userdata(getCurrentUser()->userid,&((getSession()->currentmemo)->ud) );
	end_mmapfile((getSession()->currentmemo), sizeof(struct usermemo), -1);


	if(sign>0 && newinfo.signature>0)
		RETURN_LONG(1);

	if(sign<=0 && newinfo.signature==0)
		RETURN_LONG(2);

	if(sign > 0)
    	newinfo.signature = 1;
	else
		newinfo.signature = 0;

	update_user(&newinfo, unum, 1);

	RETURN_LONG(3);

}



PHP_FUNCTION(bbs_modify_nick)
{
    struct userec newinfo;
    int unum;

    char* username;
    int username_len;
    int ac = ZEND_NUM_ARGS();
    long bTmp = 1;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &username, &username_len) == FAILURE)
        if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &username, &username_len, &bTmp) == FAILURE)
    {
            WRONG_PARAM_COUNT;
    }
    process_control_chars(username,NULL);
	if( username_len >= NAMELEN)
       RETURN_LONG(-1);

    if (!bTmp) {
        if( (unum = searchuser(getCurrentUser()->userid))==0)
            RETURN_LONG(-1);
        memcpy(&newinfo, getCurrentUser(), sizeof(struct userec));
        if (strcmp(newinfo.username, username)) {
            strcpy(newinfo.username, username);
        	update_user(&newinfo, unum, 1);
        }
    }

	strcpy(getSession()->currentuinfo->username, username);
    UPDATE_UTMP_STR(username, (*(getSession()->currentuinfo)));

	RETURN_LONG(0);
}






PHP_FUNCTION(bbs_getuserlevel){
    struct userec* u;
	char* user;
	int uLen;
	char title[USER_TITLE_LEN];
    if (ZEND_NUM_ARGS() != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &user, &uLen) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }
    if (getuser(user, &u)==0) {
		RETURN_LONG(-1);
	}
	uleveltochar(title,u);
	title[USER_TITLE_LEN-1]=0;
	RETURN_STRINGL(title,strlen(title),1);
}


PHP_FUNCTION(bbs_compute_user_value)
{
    struct userec *user;
	char *userid;
	int userid_len;

    if(zend_parse_parameters(1 TSRMLS_CC,"s",&userid,&userid_len)!=SUCCESS){
        WRONG_PARAM_COUNT;
    }
	if(userid_len>IDLEN)
        userid[IDLEN]=0;
	if(!getuser(userid,&user))
		RETURN_LONG(0);
	RETURN_LONG(compute_user_value(user));
}

PHP_FUNCTION(bbs_user_level_char)
{
	char *userid;
	int userid_len;
    struct userec *lookupuser;
	char permstr[USER_TITLE_LEN];

    if (zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
	
	if( userid_len > IDLEN )
		userid[IDLEN]=0;

	if( getuser(userid, &lookupuser) == 0 )
		RETURN_LONG(0);

	permstr[0]=0;
	uleveltochar(permstr, lookupuser);
	RETURN_STRING(permstr, 1);

}

