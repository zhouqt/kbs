#include <php.h>
#include "bbs.h"

static ZEND_FUNCTION(bbs_getuser);
static ZEND_FUNCTION(bbs_getonlineuser);
static ZEND_FUNCTION(bbs_getonlinenumber);
static ZEND_FUNCTION(bbs_countuser);
static ZEND_FUNCTION(bbs_setfromhost);
static ZEND_FUNCTION(bbs_checkpasswd);
static ZEND_FUNCTION(bbs_getcurrentuser);
static ZEND_FUNCTION(bbs_setonlineuser);
static ZEND_FUNCTION(bbs_getcurrentuinfo);
static ZEND_FUNCTION(bbs_wwwlogin);

static ZEND_MINIT_FUNCTION(bbs_module_init);
static ZEND_MSHUTDOWN_FUNCTION(bbs_module_shutdown);
static ZEND_RINIT_FUNCTION(bbs_request_init);
static ZEND_RSHUTDOWN_FUNCTION(bbs_request_shutdown); 
/*
 * define what functions can be used in the PHP embedded script
 */
static function_entry bbs_php_functions[] = {
        ZEND_FE(bbs_getuser, NULL)
        ZEND_FE(bbs_getonlineuser, NULL)
        ZEND_FE(bbs_getonlinenumber, NULL)
        ZEND_FE(bbs_countuser, NULL)
        ZEND_FE(bbs_setfromhost, NULL)
        ZEND_FE(bbs_checkpasswd, NULL)
        ZEND_FE(bbs_getcurrentuser, NULL)
        ZEND_FE(bbs_setonlineuser, NULL)
	ZEND_FE(bbs_getcurrentuinfo, NULL)
	ZEND_FE(bbs_wwwlogin, NULL)
        {NULL,NULL,NULL}
};

/*
 * This is the module entry structure, and some properties
 */

static zend_module_entry bbs_php_module_entry = {
	STANDARD_MODULE_HEADER,
        "bbs module",				/* extension name */
        bbs_php_functions,				/* extension function list */
        ZEND_MINIT(bbs_module_init),						/* extension-wide startup function */
	ZEND_MSHUTDOWN(bbs_module_shutdown),						/* extension-wide shutdown function */
	ZEND_RINIT(bbs_request_init),						/* per-request startup function */
	ZEND_RSHUTDOWN(bbs_request_shutdown),						/* per-request shutdown function */
	NULL,						/* information function */
	"1.0",
        STANDARD_MODULE_PROPERTIES
};

/*
 * Here is the function require when the module loaded
 */
DLEXPORT zend_module_entry *get_module() {
        return &bbs_php_module_entry;
};


static void setstrlen(pval * arg)
{
	arg->value.str.len=strlen(arg->value.str.val);
}

static void assign_user(zval* array,struct userec* user)
{
	add_assoc_string(array,"userid",user->userid,1);
	add_assoc_long(array,"firstlogin",user->firstlogin);
	add_assoc_stringl(array,"lasthost",user->lasthost,IPLEN,1);
	add_assoc_long(array,"numlogins",user->numlogins);
	add_assoc_long(array,"numposts",user->numposts);
	add_assoc_long(array,"flag1",user->flags[0]);
	add_assoc_long(array,"flag2",user->flags[1]);
	add_assoc_string(array,"username",user->username,1);
	add_assoc_string(array,"ident",user->ident,1);
	add_assoc_stringl(array,"md5passwd",(char*)user->md5passwd,16,1);
	add_assoc_string(array,"realemail",user->realemail,1);
	add_assoc_long(array,"userlevel",user->userlevel);
	add_assoc_long(array,"lastlogin",user->lastlogin);
	add_assoc_long(array,"stay",user->stay);
	add_assoc_string(array,"realname",user->realname,1);
	add_assoc_string(array,"address",user->address,1);
	add_assoc_string(array,"email",user->email,1);
	add_assoc_long(array,"signature",user->signature);
	add_assoc_long(array,"userdefine",user->userdefine);
	add_assoc_long(array,"notedate",user->notedate);
	add_assoc_long(array,"noteline",user->noteline);
	add_assoc_long(array,"notemode",user->notemode);
}

static void assign_userinfo(zval* array,struct user_info* uinfo)
{
	add_assoc_long(array,"active",uinfo->active);
	add_assoc_long(array,"uid",uinfo->uid);
	add_assoc_long(array,"pid",uinfo->pid);
	add_assoc_long(array,"invisible",uinfo->invisible);
	add_assoc_long(array,"sockactive",uinfo->sockactive);
	add_assoc_long(array,"sockaddr",uinfo->sockaddr);
	add_assoc_long(array,"destuid",uinfo->destuid);
	add_assoc_long(array,"mode",uinfo->mode);
	add_assoc_long(array,"pager",uinfo->pager);
	add_assoc_long(array,"in_chat",uinfo->in_chat);
	add_assoc_string(array,"chatid",uinfo->chatid,1);
	add_assoc_string(array,"from",uinfo->from,1);
	add_assoc_long(array,"freshtime",uinfo->freshtime);
	add_assoc_long(array,"utmpkey",uinfo->utmpkey);
	add_assoc_string(array,"userid",uinfo->userid,1);
	add_assoc_string(array,"realname",uinfo->realname,1);
	add_assoc_string(array,"username",uinfo->username,1);
}

static int currentusernum;
static char* fullfrom;
static struct user_info *currentuinfo;
static int currentuinfonum;

static inline struct userec* getcurrentuser()
{
	return currentuser;
}

static inline struct user_info* getcurrentuinfo()
{
	return currentuinfo;
}

static inline void setcurrentuinfo(struct user_info* uinfo,int uinfonum)
{
	currentuinfo=uinfo;
	currentuinfonum=uinfonum;
}

static inline void setcurrentuser(struct userec* user,int usernum)
{
	currentuser=user;
	currentusernum=usernum;
}

static inline int getcurrentuser_num()
{
	return currentusernum;
}

static inline int getcurrentuinfo_num()
{
	return currentuinfonum;
}
time_t set_idle_time(struct user_info *uentp, time_t t)
{
	uentp->freshtime = t;
	return t;
}

/*
 * Here goes the real functions
 */

/* arguments: userid, username, ipaddr, operation */
static ZEND_FUNCTION(bbs_setfromhost)
{
	char* s;
	int s_len;
	int full_len;

        if (zend_parse_parameters(2 TSRMLS_CC, "ss" ,&s,&s_len,&fullfrom,&full_len) != SUCCESS) {
                WRONG_PARAM_COUNT;
        }
        if (s_len>IPLEN)
        	s[IPLEN]=0;
        if (full_len>80)
        	fullfrom[80]=0;
        strcpy(fromhost,s);
        RETURN_NULL();
}

static ZEND_FUNCTION(bbs_getuser)
{
        long                    v1;
	struct	userec *lookupuser;
	char* s;
	int s_len;
	zval* user_array;

        if (zend_parse_parameters(2 TSRMLS_CC, "sa" ,&s,&s_len, &user_array) != SUCCESS) {
                WRONG_PARAM_COUNT;
        }

	if (s_len>IDLEN)
		s[IDLEN]=0;
        v1=getuser(s, &lookupuser);

	if (v1==0)
		RETURN_LONG(0);

	if(array_init(user_array) != SUCCESS)
		RETURN_LONG(0);
	assign_user(user_array,lookupuser);
/*        RETURN_STRING(retbuf, 1);
 *        */
        RETURN_LONG(v1);
}

static ZEND_FUNCTION(bbs_getonlineuser)
{
	long idx,ret;
	struct user_info* uinfo;
	zval* user_array;

        if (zend_parse_parameters(2 TSRMLS_CC, "la" ,&idx, &user_array) != SUCCESS) {
                WRONG_PARAM_COUNT;
        }
	uinfo=get_utmpent(idx);
	if (uinfo==NULL) ret = 0;
	else {
	  if(array_init(user_array) != SUCCESS)
		ret=0;
	  else {
		  assign_userinfo(user_array,uinfo);
		  ret=idx;
	  }
	}
	RETURN_LONG(ret);
}

static ZEND_FUNCTION(bbs_getonlinenumber)
{
	RETURN_LONG(get_utmp_number());
}


static ZEND_FUNCTION(bbs_countuser)
{
	long idx;

        if (zend_parse_parameters(2 TSRMLS_CC, "l" ,&idx) != SUCCESS) {
                WRONG_PARAM_COUNT;
        }
    	RETURN_LONG(apply_utmpuid( NULL , idx,0));
}

static ZEND_FUNCTION(bbs_checkmulti)
{
    int count;
    if (!getcurrentuser())
    	RETURN_LONG(1);
    if (!getcurrentuser_num()==-1)
    	RETURN_LONG(1);
    count = apply_utmpuid( NULL , getcurrentuser_num(),0);
    if ((HAS_PERM(getcurrentuser(),PERM_BOARDS) || HAS_PERM(getcurrentuser(),PERM_CHATOP)|| HAS_PERM(currentuser,PERM_JURY) || HAS_PERM(currentuser,PERM_CHATCLOAK)) && count< 2)
        RETURN_LONG(0);
    if ( (get_utmp_number()<700)&&(count>=2) 
           || (get_utmp_number()>=700)&& (count>=1) ) /*user login limit*/
    {  
		RETURN_LONG(1);
    }
    RETURN_LONG(0);
}

static ZEND_FUNCTION(bbs_checkpasswd)
{
	char* s;
	int s_len;
	char* pw;
	int pw_len;
	long ret;
	int unum;
	struct userec* user;

       if (zend_parse_parameters(2 TSRMLS_CC, "ss" ,&s,&s_len, &pw,&pw_len) != SUCCESS) {
                WRONG_PARAM_COUNT;
       }
       if (s_len>IDLEN)
        	s[IDLEN]=0;
       if (pw_len>PASSLEN)
        	pw[PASSLEN]=0;
       if (!(unum=getuser( s,&user)))
       	ret=2;
       else {
		if (checkpasswd2(pw,user)) {
			ret=0;
			setcurrentuser(user,unum);
		}
		else {
			ret=1;
			logattempt(user->userid, fromhost);
		}
       }
	RETURN_LONG(ret);
}

static ZEND_FUNCTION(bbs_wwwlogin)
{
	char buf[255];
	long ret;
#ifdef SQUID_ACCL
	snprintf(buf, sizeof(buf), "ENTER ?@%s [www]", fullfrom);
#else
	snprintf(buf, sizeof(buf), "ENTER ?@%s [www]", fromhost);
#endif
	bbslog("1system", buf);
	if(strcasecmp(getcurrentuser()->userid, "guest")) {
		struct user_info ui;
		int utmpent;
                time_t t;

		if(!HAS_PERM(getcurrentuser(), PERM_BASIC))
			RETURN_LONG(3);
		if(!check_ban_IP(fromhost,buf))
			RETURN_LONG(4);
		t=getcurrentuser()->lastlogin;
		getcurrentuser()->lastlogin=time(0);
		if(abs(t-time(0))<5)
			RETURN_LONG(5);
		getcurrentuser()->numlogins++;
		strncpy(getcurrentuser()->lasthost, fromhost, IPLEN);
		if (!HAS_PERM(getcurrentuser(),PERM_LOGINOK) && !HAS_PERM(getcurrentuser(),PERM_SYSOP))
		{
			if (strchr(getcurrentuser()->realemail, '@')
				&& valid_ident(getcurrentuser()->realemail))
			{
				getcurrentuser()->userlevel |= PERM_DEFAULT;
				if (HAS_PERM(getcurrentuser(),PERM_DENYPOST)/* && !HAS_PERM(currentuser,PERM_SYSOP)*/)
					getcurrentuser()->userlevel &= ~PERM_POST;
			}
		}

		memset( &ui, 0, sizeof( struct user_info ) );
    		ui.active = YEA ;
		/* Bigman 2000.8.29 ÖÇÄÒÍÅÄÜ¹»ÒþÉí */
		if( (HAS_PERM(getcurrentuser(),PERM_CHATCLOAK)
			|| HAS_PERM(getcurrentuser(),PERM_CLOAK)) 
			&& (getcurrentuser()->flags[0] & CLOAK_FLAG))
		    	ui.invisible = YEA;
		ui.pager = 0;
		if(DEFINE(getcurrentuser(),DEF_FRIENDCALL))
		{
		    ui.pager|=FRIEND_PAGER;
		}
		if(getcurrentuser()->flags[0] & PAGER_FLAG)
		{
		    ui.pager|=ALL_PAGER;
		    ui.pager|=FRIEND_PAGER;
		}
		if(DEFINE(getcurrentuser(),DEF_FRIENDMSG))
		{
		    ui.pager|=FRIENDMSG_PAGER;
		}
		if(DEFINE(getcurrentuser(),DEF_ALLMSG))
		{
		    ui.pager|=ALLMSG_PAGER;
		    ui.pager|=FRIENDMSG_PAGER;
		}
		ui.uid=getcurrentuser_num();
		strncpy( ui.from, fromhost, IPLEN );
		ui.logintime=time(0);	/* for counting user's stay time */
										/* refer to bbsfoot.c for details */
		ui.freshtime = time(0);
		ui.mode = WEBEXPLORE;
		strncpy( ui.userid,   getcurrentuser()->userid,   20 );
		strncpy( ui.realname, getcurrentuser()->realname, 20 );
		strncpy( ui.username, getcurrentuser()->username, 40 );
		utmpent = getnewutmpent(&ui) ;
		if (utmpent == -1)
			ret=1;
		else {
			struct user_info* u;
			int tmp;
			u = get_utmpent(utmpent);
			u->pid = 1;
			tmp=rand()%100000000;
			u->utmpkey=tmp;
			if (addto_msglist(utmpent, getcurrentuser()->userid) < 0)
				ret=2;
			else {
				/*
				sprintf(buf, "%d", utmpent);
				setcookie("utmpnum", buf);
				sprintf(buf, "%d", tmp);
				setcookie("utmpkey", buf);
				setcookie("utmpuserid", getcurrentuser()->userid);
				set_my_cookie();
				*/
				setcurrentuinfo(u,utmpent);
				ret=0;
			}
		}
	} else /* guest */
		ret=0;
	RETURN_LONG(ret);
}

static ZEND_FUNCTION(bbs_getcurrentuinfo)
{
        zval* user_array;
        long ret;

        if (zend_parse_parameters(1 TSRMLS_CC, "a" , &user_array) != SUCCESS) {
                WRONG_PARAM_COUNT;
        }

        if(array_init(user_array) != SUCCESS) {
                ret=0;
        }
        else {
                if (getcurrentuinfo()) {
                  assign_userinfo(user_array,getcurrentuinfo());
                  ret=getcurrentuinfo_num();
                } else
                        ret=0;
        }
        RETURN_LONG(ret);
}

static ZEND_FUNCTION(bbs_getcurrentuser)
{
	zval* user_array;
	long ret;

        if (zend_parse_parameters(1 TSRMLS_CC, "a" , &user_array) != SUCCESS) {
                WRONG_PARAM_COUNT;
        }
	
	if(array_init(user_array) != SUCCESS) {
		ret=0;
	}
	else {
		if (getcurrentuser()) {
		  assign_user(user_array,getcurrentuser());
		  ret=getcurrentuser_num();
		} else
			ret=0;
	}
	RETURN_LONG(ret);
}

static ZEND_FUNCTION(bbs_setonlineuser)
{
	zval* user_array;
	char* userid;
	int userid_len;
	long utmpnum;
	long utmpkey;
	long ret;
	struct user_info* pui;
	int idx;
	struct userec* user;

        if (zend_parse_parameters(4 TSRMLS_CC, "slla" , &userid, &userid_len,
				&utmpnum, &utmpkey, &user_array) != SUCCESS) {
                WRONG_PARAM_COUNT;
        }
	if (userid_len>IDLEN) RETURN_LONG(1);
	if(utmpnum<1 || utmpnum>=MAXACTIVE)
		RETURN_LONG(2);

	pui = get_utmpent(utmpnum);
	if (strcasecmp(pui->userid,"guest")) {
		if (pui->utmpkey!=utmpkey)
			RETURN_LONG(3);
		if (pui->active==0)
			RETURN_LONG(4);
		if (strcmp(pui->userid,userid))
			RETURN_LONG(5);
	}
	setcurrentuinfo(pui,utmpnum);
	idx=getuser(pui->userid,&user);
	if (user==NULL)
		RETURN_LONG(6);
	setcurrentuser(user,idx);
	if(array_init(user_array) != SUCCESS)
		ret=7;
	else {
		assign_userinfo(user_array,pui);
		ret=0;
	}
	RETURN_LONG(ret);
}

static char old_pwd[1024];
static ZEND_MINIT_FUNCTION(bbs_module_init)
{
	getcwd(old_pwd,1023);
	old_pwd[1023]=0;
	chdir(BBSHOME);
	resolve_ucache();
	resolve_utmp();
	resolve_boards();
#ifdef SQUID_ACCL
	REGISTER_MAIN_LONG_CONSTANT("SETTING_SQUID_ACCL", 1, CONST_CS | CONST_PERSISTENT);
#else
	REGISTER_MAIN_LONG_CONSTANT("SETTING_SQUID_ACCL", 0, CONST_CS | CONST_PERSISTENT);
#endif
	chdir(old_pwd);
	return SUCCESS;
}

static ZEND_MSHUTDOWN_FUNCTION(bbs_module_shutdown)
{
	return SUCCESS;
}

static ZEND_RINIT_FUNCTION(bbs_request_init)
{
	getcwd(old_pwd,1023);
	old_pwd[1023]=0;
	return SUCCESS;
}

static ZEND_RSHUTDOWN_FUNCTION(bbs_request_shutdown)
{
	chdir(old_pwd);
	return SUCCESS;
}
