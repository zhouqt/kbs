#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
    
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"  
#include "php_smth_bbs.h"  

#include "bbs.h"
#include "bbslib.h"
#include "vote.h"

static unsigned char third_arg_force_ref_1111[] = { 4, BYREF_FORCE, BYREF_FORCE, BYREF_FORCE, BYREF_FORCE };
static unsigned char third_arg_force_ref_011[] = { 3, BYREF_NONE, BYREF_FORCE, BYREF_FORCE };
static unsigned char fourth_arg_force_ref_0001[] = { 4, BYREF_NONE, BYREF_NONE, BYREF_NONE, BYREF_FORCE };

#ifdef HAVE_WFORUM
static PHP_FUNCTION(bbs_get_article);
static PHP_FUNCTION(bbs_is_yank);
static PHP_FUNCTION(bbs_alter_yank); 
#endif
static PHP_FUNCTION(bbs_getonline_user_list);
static PHP_FUNCTION(bbs_get_elite_num);
static PHP_FUNCTION(bbs_get_elite_list);

#ifdef HAVE_USERMONEY
static PHP_FUNCTION(bbs_getusermoney);
static PHP_FUNCTION(bbs_setusermoney);
static PHP_FUNCTION(bbs_addusermoney);
static PHP_FUNCTION(bbs_getuserscore);
static PHP_FUNCTION(bbs_setuserscore);
static PHP_FUNCTION(bbs_adduserscore);
#endif

static PHP_FUNCTION(bbs_saveuserdata);
static PHP_FUNCTION(bbs_checkuserpasswd);
static PHP_FUNCTION(bbs_setuserpasswd);

static PHP_FUNCTION(bbs_getuserparam);
static PHP_FUNCTION(bbs_setuserparam);

static PHP_FUNCTION(bbs_getuserlevel);
static PHP_FUNCTION(bbs_get_today_article_num);
static PHP_FUNCTION(bbs_searchtitle);
static PHP_FUNCTION(bbs_getnumofsig);
static PHP_FUNCTION(bbs_postmail);
static PHP_FUNCTION(bbs_mailwebmsgs);
static PHP_FUNCTION(bbs_getwebmsgs);
static PHP_FUNCTION(bbs_get_thread_article_num);
static PHP_FUNCTION(bbs_get_thread_articles);
static PHP_FUNCTION(bbs_getuser);
static PHP_FUNCTION(bbs_getonlineuser);
static PHP_FUNCTION(bbs_getonlinenumber);
static PHP_FUNCTION(bbs_getonlineusernumber);
static PHP_FUNCTION(bbs_getwwwguestnumber);
static PHP_FUNCTION(bbs_countuser);
static PHP_FUNCTION(bbs_setfromhost);
static PHP_FUNCTION(bbs_checkpasswd);
static PHP_FUNCTION(bbs_getcurrentuser);
static PHP_FUNCTION(bbs_setonlineuser);
static PHP_FUNCTION(bbs_getcurrentuinfo);
static PHP_FUNCTION(bbs_wwwlogin);
static PHP_FUNCTION(bbs_wwwlogoff);
static PHP_FUNCTION(bbs_printansifile);
static PHP_FUNCTION(bbs_getboard);
static PHP_FUNCTION(bbs_checkreadperm);
static PHP_FUNCTION(bbs_getbname);
static PHP_FUNCTION(bbs_checkpostperm);
static PHP_FUNCTION(bbs_postarticle);
#ifdef HAVE_BRC_CONTROL
static PHP_FUNCTION(bbs_brcaddread);
#endif
static PHP_FUNCTION(bbs_ann_traverse_check);
static PHP_FUNCTION(bbs_ann_get_board);
static PHP_FUNCTION(bbs_getboards);
static PHP_FUNCTION(bbs_getarticles);
static PHP_FUNCTION(bbs_get_records_from_id);
static PHP_FUNCTION(bbs_get_records_from_num);
static PHP_FUNCTION(bbs_get_filename_from_num);
static PHP_FUNCTION(bbs_get_threads_from_id);
static PHP_FUNCTION(bbs_countarticles);
static PHP_FUNCTION(bbs_is_bm);
static PHP_FUNCTION(bbs_getannpath);
static PHP_FUNCTION(bbs_getmailnum);
static PHP_FUNCTION(bbs_getmailnum2);
static PHP_FUNCTION(bbs_getmails);
static PHP_FUNCTION(bbs_getmailusedspace);
static PHP_FUNCTION(bbs_valid_filename);
static PHP_FUNCTION(bbs_can_send_mail);
static PHP_FUNCTION(bbs_loadmaillist);
static PHP_FUNCTION(bbs_changemaillist);
static PHP_FUNCTION(bbs_getwebmsg);
static PHP_FUNCTION(bbs_sendwebmsg);
static PHP_FUNCTION(bbs_sethomefile);
static PHP_FUNCTION(bbs_setmailfile);
static PHP_FUNCTION(bbs_mail_file);
static PHP_FUNCTION(bbs_update_uinfo);
static PHP_FUNCTION(bbs_createnewid);
static PHP_FUNCTION(bbs_fillidinfo);
static PHP_FUNCTION(bbs_createregform);
static PHP_FUNCTION(bbs_findpwd_check);
static PHP_FUNCTION(bbs_delfile);
static PHP_FUNCTION(bbs_delmail);
static PHP_FUNCTION(bbs_normalboard);
static PHP_FUNCTION(bbs_setmailreaded);
static PHP_FUNCTION(bbs_add_import_path);
static PHP_FUNCTION(bbs_get_import_path);
static PHP_FUNCTION(bbs_new_board);
static PHP_FUNCTION(bbs_set_onboard);
static PHP_FUNCTION(bbs_get_votes);
static PHP_FUNCTION(bbs_get_vote_from_num);
static PHP_FUNCTION(bbs_vote_num);
static PHP_FUNCTION(bbs_get_explain);
static PHP_FUNCTION(bbs_start_vote);
/* favboard operation. by caltary  */
static PHP_FUNCTION(bbs_load_favboard);
static PHP_FUNCTION(bbs_fav_boards);
static PHP_FUNCTION(bbs_release_favboard);
static PHP_FUNCTION(bbs_is_favboard);
static PHP_FUNCTION(bbs_add_favboarddir);
static PHP_FUNCTION(bbs_add_favboard);
static PHP_FUNCTION(bbs_del_favboard);
static PHP_FUNCTION(bbs_sysconf_str);
static PHP_FUNCTION(bbs_get_tmpls);
static PHP_FUNCTION(bbs_get_tmpl_from_num);
static PHP_FUNCTION(bbs_make_tmpl_file);
#ifdef SMS_SUPPORT
static PHP_FUNCTION(bbs_send_sms);
static PHP_FUNCTION(bbs_register_sms_sendcheck);
static PHP_FUNCTION(bbs_register_sms_docheck);
static PHP_FUNCTION(bbs_unregister_sms);
#endif
#if HAVE_MYSQL == 1
static PHP_FUNCTION(bbs_csv_to_al);
#endif
static PHP_FUNCTION(bbs_printoriginfile);
static PHP_FUNCTION(bbs_caneditfile);
static PHP_FUNCTION(bbs_updatearticle);
static PHP_FUNCTION(bbs_getthreadnum);
static PHP_FUNCTION(bbs_getthreads);
static PHP_FUNCTION(bbs_ext_initialized);
static PHP_FUNCTION(bbs_init_ext);
static PHP_FUNCTION(bbs_x_search);

/*
 * define what functions can be used in the PHP embedded script
 */
static function_entry smth_bbs_functions[] = {
#ifdef HAVE_WFORUM
		PHP_FE(bbs_get_article, NULL)
		PHP_FE(bbs_is_yank, NULL)
		PHP_FE(bbs_alter_yank, NULL)
#endif
		PHP_FE(bbs_getonline_user_list, NULL)
		PHP_FE(bbs_get_elite_num, NULL)
		PHP_FE(bbs_get_elite_list, NULL)
#ifdef HAVE_USERMONEY
		PHP_FE(bbs_getusermoney, NULL)
		PHP_FE(bbs_setusermoney, NULL)
		PHP_FE(bbs_addusermoney, NULL)
		PHP_FE(bbs_getuserscore, NULL)
		PHP_FE(bbs_setuserscore, NULL)
		PHP_FE(bbs_adduserscore, NULL)
		PHP_FE(bbs_saveuserdata, NULL)
#endif
		PHP_FE(bbs_getuserparam, NULL)
		PHP_FE(bbs_setuserparam, NULL)

		PHP_FE(bbs_checkuserpasswd, NULL)
		PHP_FE(bbs_setuserpasswd, NULL)
		PHP_FE(bbs_getuserlevel, NULL)
		PHP_FE(bbs_get_today_article_num, NULL)
		PHP_FE(bbs_searchtitle, NULL)
	    PHP_FE(bbs_getnumofsig, NULL)
		PHP_FE(bbs_postmail, NULL)
		PHP_FE(bbs_mailwebmsgs, NULL)
		PHP_FE(bbs_getwebmsgs, NULL)
		PHP_FE(bbs_get_thread_article_num,NULL)
		PHP_FE(bbs_get_thread_articles, NULL)
		PHP_FE(bbs_getthreads, NULL)
		PHP_FE(bbs_getthreadnum, NULL)
        PHP_FE(bbs_getuser, NULL)
        PHP_FE(bbs_getonlineuser, NULL)
        PHP_FE(bbs_getonlinenumber, NULL)
	PHP_FE(bbs_getonlineusernumber,NULL)
	PHP_FE(bbs_getwwwguestnumber,NULL)
        PHP_FE(bbs_countuser, NULL)
        PHP_FE(bbs_setfromhost, NULL)
        PHP_FE(bbs_checkpasswd, NULL)
        PHP_FE(bbs_getcurrentuser, NULL)
        PHP_FE(bbs_setonlineuser, NULL)
        PHP_FE(bbs_getcurrentuinfo, NULL)
        PHP_FE(bbs_wwwlogin, NULL)
        PHP_FE(bbs_wwwlogoff, NULL)
        PHP_FE(bbs_printansifile, NULL)
		PHP_FE(bbs_printoriginfile, NULL)
		PHP_FE(bbs_caneditfile,NULL)
        PHP_FE(bbs_checkreadperm, NULL)
        PHP_FE(bbs_getbname, NULL)
        PHP_FE(bbs_checkpostperm, NULL)
		PHP_FE(bbs_updatearticle, NULL)
#ifdef HAVE_BRC_CONTROL
        PHP_FE(bbs_brcaddread, NULL)
#endif
        PHP_FE(bbs_getboard, NULL)
		PHP_FE(bbs_postarticle,NULL)
        PHP_FE(bbs_ann_traverse_check, NULL)
        PHP_FE(bbs_ann_get_board, NULL)
        PHP_FE(bbs_getboards, NULL)
        PHP_FE(bbs_getarticles, NULL)
        PHP_FE(bbs_get_records_from_id, NULL)
        PHP_FE(bbs_get_records_from_num, NULL)
        PHP_FE(bbs_get_filename_from_num, NULL)
        PHP_FE(bbs_get_threads_from_id, NULL)
        PHP_FE(bbs_countarticles, NULL)
        PHP_FE(bbs_is_bm, NULL)
        PHP_FE(bbs_getannpath, NULL)
        PHP_FE(bbs_getmailnum, third_arg_force_ref_011)
        PHP_FE(bbs_getmailnum2, NULL)
        PHP_FE(bbs_getmails, NULL)
        PHP_FE(bbs_getmailusedspace, NULL)
        PHP_FE(bbs_valid_filename, NULL)
        PHP_FE(bbs_can_send_mail, NULL)
        PHP_FE(bbs_loadmaillist, NULL)
        PHP_FE(bbs_changemaillist, NULL)
        PHP_FE(bbs_getwebmsg, third_arg_force_ref_1111)
        PHP_FE(bbs_sendwebmsg, fourth_arg_force_ref_0001)
        PHP_FE(bbs_sethomefile, NULL)
        PHP_FE(bbs_setmailfile, NULL)
        PHP_FE(bbs_mail_file, NULL)
        PHP_FE(bbs_update_uinfo, NULL)
        PHP_FE(bbs_createnewid,NULL)
	PHP_FE(bbs_createregform,NULL)
	PHP_FE(bbs_findpwd_check,NULL)
        PHP_FE(bbs_fillidinfo,NULL)
        PHP_FE(bbs_delfile,NULL)
        PHP_FE(bbs_delmail,NULL)
        PHP_FE(bbs_normalboard,NULL)
        PHP_FE(bbs_setmailreaded,NULL)
	PHP_FE(bbs_add_import_path,NULL)
	PHP_FE(bbs_get_import_path,NULL)
	PHP_FE(bbs_new_board,NULL)
	PHP_FE(bbs_set_onboard,NULL)
	PHP_FE(bbs_get_votes,NULL)
	PHP_FE(bbs_get_vote_from_num,NULL)
	PHP_FE(bbs_vote_num,NULL)
	PHP_FE(bbs_get_explain,NULL)
	PHP_FE(bbs_start_vote,NULL)
	/* favboard operation. by caltary  */
	PHP_FE(bbs_load_favboard,NULL)
	PHP_FE(bbs_fav_boards,NULL)
	PHP_FE(bbs_release_favboard,NULL)
	PHP_FE(bbs_is_favboard,NULL)
	PHP_FE(bbs_add_favboarddir,NULL)
	PHP_FE(bbs_add_favboard,NULL)
	PHP_FE(bbs_del_favboard,NULL)
       PHP_FE(bbs_sysconf_str,NULL)
		PHP_FE(bbs_get_tmpls,NULL)
		PHP_FE(bbs_get_tmpl_from_num,NULL)
		PHP_FE(bbs_make_tmpl_file,NULL)
#ifdef SMS_SUPPORT
		PHP_FE(bbs_send_sms,NULL)
		PHP_FE(bbs_register_sms_sendcheck,NULL)
		PHP_FE(bbs_unregister_sms,NULL)
		PHP_FE(bbs_register_sms_docheck,NULL)
#endif
#if HAVE_MYSQL == 1
		PHP_FE(bbs_csv_to_al, NULL)
#endif
		PHP_FE(bbs_ext_initialized, NULL)
		PHP_FE(bbs_init_ext, NULL)
	PHP_FE(bbs_x_search,NULL)
        {NULL, NULL, NULL}
};

/*
 * This is the module entry structure, and some properties
 */
zend_module_entry smth_bbs_module_entry = {
    STANDARD_MODULE_HEADER,
    "smth_bbs",
    smth_bbs_functions,
    PHP_MINIT(smth_bbs),
    PHP_MSHUTDOWN(smth_bbs),
    PHP_RINIT(smth_bbs),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(smth_bbs),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(smth_bbs),
    "1.0", /* Replace with version number for your extension */
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SMTH_BBS
ZEND_GET_MODULE(smth_bbs)
#endif

/*
 * Here is the function require when the module loaded
DLEXPORT zend_module_entry *get_module()
{
    return &smth_bbs_module_entry;
};


static void setstrlen(pval * arg)
{
    arg->value.str.len = strlen(arg->value.str.val);
}
 */

static void assign_user(zval * array, struct userec *user, int num)
{
    struct userdata ud;

    read_userdata(user->userid, &ud);
    add_assoc_long(array, "index", num);
    add_assoc_string(array, "userid", user->userid, 1);
    add_assoc_long(array, "firstlogin", user->firstlogin);
    add_assoc_stringl(array, "lasthost", user->lasthost, IPLEN, 1);
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
    add_assoc_long(array, "userdefine", user->userdefine);
    add_assoc_long(array, "notedate", user->notedate);
    add_assoc_long(array, "noteline", user->noteline);
    add_assoc_long(array, "notemode", user->notemode);
	#ifdef HAVE_USERMONEY
	add_assoc_long(array,"money", user->money);
	add_assoc_long(array,"score", user->score);
	#endif

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
static int foundInArray(unsigned int content, unsigned int array[], unsigned int len){
	int i;
	for (i=0;i<len;i++){
		if (array[i]==content)	{
			return i;
		}
	}
	return -1;
}
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
    add_assoc_string(array, "userid", uinfo->userid, 1);
    add_assoc_string(array, "realname", uinfo->realname, 1);
    add_assoc_string(array, "username", uinfo->username, 1);
}

//char* maillist, 40 bytes long, 30 bytes for the mailbox name,10 bytes for the mailbox path file name.
static void asssign_maillist(zval * array, char *boxname, char *pathname)
{
    add_assoc_string(array, "boxname", boxname, 1);
    add_assoc_string(array, "pathname", pathname, 1);
}

static void assign_board(zval * array, const struct boardheader *board, const struct BoardStatus* bstatus, int num)
{
    add_assoc_long(array, "NUM", num);
    add_assoc_string(array, "NAME", board->filename, 1);
    /*
     * add_assoc_string(array, "OWNER", board->owner, 1);
     */
    add_assoc_string(array, "BM", board->BM, 1);
    add_assoc_long(array, "FLAG", board->flag);
    add_assoc_string(array, "DESC", board->title + 13, 1);
    add_assoc_stringl(array, "CLASS", board->title + 1, 6, 1);
    add_assoc_stringl(array, "SECNUM", board->title, 1, 1);
    add_assoc_long(array, "LEVEL", board->level);
    add_assoc_long(array, "CURRENTUSERS", bstatus->currentusers);
    add_assoc_long(array, "TOTAL", bstatus->total);
}

static int currentusernum;
static char fullfrom[255];
static char php_fromhost[IPLEN + 1];
static struct user_info *currentuinfo;
static int currentuinfonum;

static inline struct userec *getcurrentuser()
{
    return currentuser;
}

static inline struct user_info *getcurrentuinfo()
{
    return currentuinfo;
}

static inline void setcurrentuinfo(struct user_info *uinfo, int uinfonum)
{
    currentuinfo = uinfo;
    currentuinfonum = uinfonum;
}

static inline void setcurrentuser(struct userec *user, int usernum)
{
    currentuser = user;
    currentusernum = usernum;
}

static inline int getcurrentuser_num()
{
    return currentusernum;
}

#ifdef HAVE_WFORUM
static PHP_FUNCTION(bbs_is_yank){
	RETURN_LONG(currentuinfo->yank);
}
static PHP_FUNCTION(bbs_alter_yank){
	currentuinfo->yank=currentuinfo->yank?0:1;
	RETURN_LONG(currentuinfo->yank);
};
#endif
static inline int getcurrentuinfo_num()
{
    return currentuinfonum;
}

/*
 * Here goes the real functions
 */

/* arguments: userid, username, ipaddr, operation */
static char old_pwd[1024];
static PHP_FUNCTION(bbs_setfromhost)
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
    strcpy(fromhost, s);
    strcpy(php_fromhost, s);
    RETURN_NULL();
}


static PHP_FUNCTION(bbs_getuser)
{
    long v1;
    struct userec *lookupuser;
    char *s;
    int s_len;
    zval *user_array;

    MAKE_STD_ZVAL(user_array);
    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
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

static PHP_FUNCTION(bbs_getonlineuser)
{
    long idx, ret;
    struct user_info *uinfo;
    zval *user_array;

    MAKE_STD_ZVAL(user_array);
    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
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

static PHP_FUNCTION(bbs_getonlinenumber)
{
    RETURN_LONG(get_utmp_number() + getwwwguestcount());
}

static PHP_FUNCTION(bbs_getonlineusernumber)
{
    RETURN_LONG(get_utmp_number());
}

static PHP_FUNCTION(bbs_getwwwguestnumber)
{
    RETURN_LONG(getwwwguestcount());
}


static PHP_FUNCTION(bbs_countuser)
{
    long idx;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (zend_parse_parameters(2 TSRMLS_CC, "l", &idx) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    RETURN_LONG(apply_utmpuid(NULL, idx, 0));
}

static PHP_FUNCTION(bbs_getnumofsig){
    FILE *fp;
    char tmp[256];
    int count = 0;
	int sigln;
    char signame[STRLEN];
	int numofsig;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
	if (ZEND_NUM_ARGS()!=0) {
		WRONG_PARAM_COUNT;
	}
	if (currentuser==NULL) 
		RETURN_LONG(-1);
    sethomefile(signame, currentuser->userid, "signatures");

    if ((fp = fopen(signame, "r")) == NULL)
        RETURN_LONG(0);
    while (fgets(tmp, sizeof(tmp), fp) != NULL)
        count++;
    fclose(fp);
    sigln = count;
    numofsig = sigln / 6;
    if ((sigln % 6) != 0)
        numofsig += 1;
	RETURN_LONG(numofsig);
}

static PHP_FUNCTION(bbs_setuserpasswd){
    char *s;
    int s_len;
    char *pw;
    int pw_len;
    long ret;
    int unum;
    struct userec *user;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
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

static PHP_FUNCTION(bbs_checkuserpasswd){
    char *s;
    int s_len;
    char *pw;
    int pw_len;
    long ret;
    int unum;
    struct userec *user;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
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

static PHP_FUNCTION(bbs_checkpasswd)
{
    char *s;
    int s_len;
    char *pw;
    int pw_len;
    long ret;
    int unum;
    struct userec *user;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (zend_parse_parameters(2 TSRMLS_CC, "ss", &s, &s_len, &pw, &pw_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    if (s_len > IDLEN)
        s[IDLEN] = 0;
    if (pw_len > PASSLEN)
        pw[PASSLEN] = 0;
    if ((s[0] != 0) && !(unum = getuser(s, &user)))
        ret = 2;
    else {
        if (s[0] == 0)
            user = currentuser;
        if (checkpasswd2(pw, user)) {
            ret = 0;
            if (s[0] != 0)
                setcurrentuser(user, unum);
        } else {
            ret = 1;
            logattempt(user->userid, php_fromhost);
        }
    }
    RETURN_LONG(ret);
}

static PHP_FUNCTION(bbs_getuserparam){
	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}
	if (currentuser==NULL) {
		RETURN_FALSE;
	}
	RETURN_LONG(currentuser->userdefine);
}

static PHP_FUNCTION(bbs_setuserparam){
	int userparam;
	if (ZEND_NUM_ARGS() != 1 || zend_parse_parameters(1 TSRMLS_CC, "l", &userparam) != SUCCESS) {
		WRONG_PARAM_COUNT;
	}
	if (currentuser==NULL) {
		RETURN_LONG(-1);
	}
	currentuser->userdefine=userparam;
	RETURN_LONG(0);
}

static PHP_FUNCTION(bbs_wwwlogin)
{
    long ret;
    long kick_multi = 0;
    struct user_info *pu = NULL;
    int utmpent;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (ZEND_NUM_ARGS() == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "l", &kick_multi) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
    } else if (ZEND_NUM_ARGS() != 0)
        WRONG_PARAM_COUNT;
    ret = www_user_login(getcurrentuser(), getcurrentuser_num(), kick_multi, php_fromhost,
#ifdef SQUID_ACCL
                         fullfrom,
#else
                         php_fromhost,
#endif
                         &pu, &utmpent);
    if (getcurrentuser() == NULL) {
        struct userec *user;
        int num;

        num = getuser("guest", &user);
        setcurrentuser(user, num);
    }
    setcurrentuinfo(pu, utmpent);
    RETURN_LONG(ret);
}
static PHP_FUNCTION(bbs_getuserlevel){
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
	title[9]=0;
	RETURN_STRINGL(title,strlen(title),1);
}
#ifdef HAVE_USERMONEY
static PHP_FUNCTION(bbs_getusermoney){
    struct userec* u;
	char* user;
	int uLen;
    if (ZEND_NUM_ARGS() != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &user, &uLen) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }
    if (getuser(user, &u)==0) {
		RETURN_LONG(-1);
	}
	RETURN_LONG(get_money(u));
}

static PHP_FUNCTION(bbs_setusermoney){
    struct userec* u;
	char* user;
	int uLen,money;
    if (ZEND_NUM_ARGS() != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &user, &uLen, &money) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }
    if (getuser(user, &u)==0) {
		RETURN_LONG(-1);
	}
	if (money<0) {
		RETURN_LONG(-2);
	}
	RETURN_LONG(set_money(u,money));
}

static PHP_FUNCTION(bbs_addusermoney){
    struct userec* u;
	char* user;
	int uLen,money;
    if (ZEND_NUM_ARGS() != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &user, &uLen, &money) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }
    if (getuser(user, &u)==0) {
		RETURN_LONG(-1);
	}
	if (money<0) {
		RETURN_LONG(-2);
	}
	RETURN_LONG(add_money(u,money));
}

static PHP_FUNCTION(bbs_getuserscore){
    struct userec* u;
	char* user;
	int uLen;
    if (ZEND_NUM_ARGS() != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &user, &uLen) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }
    if (getuser(user, &u)==0) {
		RETURN_LONG(-1);
	}
	RETURN_LONG(get_score(u));
}

static PHP_FUNCTION(bbs_setuserscore){
    struct userec* u;
	char* user;
	int uLen,score;
    if (ZEND_NUM_ARGS() != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &user, &uLen, &score) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }
    if (getuser(user, &u)==0) {
		RETURN_LONG(-1);
	}
	if (score<0) {
		RETURN_LONG(-2);
	}
	RETURN_LONG(set_score(u,score));
}
static PHP_FUNCTION(bbs_adduserscore){
    struct userec* u;
	char* user;
	int uLen,score;
    if (ZEND_NUM_ARGS() != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &user, &uLen, &score) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }
    if (getuser(user, &u)==0) {
		RETURN_LONG(-1);
	}
	if (score<0) {
		RETURN_LONG(-2);
	}
	RETURN_LONG(add_score(u,score));
}
#endif
static PHP_FUNCTION(bbs_getcurrentuinfo)
{
    zval *user_array;
    long ret = 1;

    MAKE_STD_ZVAL(user_array);
    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (ZEND_NUM_ARGS() == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "a", &user_array) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        if (array_init(user_array) != SUCCESS) {
            ret = 0;
        } else {
            if (getcurrentuinfo()) {
                assign_userinfo(user_array, getcurrentuinfo(), getcurrentuinfo_num());
            } else
                ret = 0;
        }
    } else if (ZEND_NUM_ARGS() != 0)
        WRONG_PARAM_COUNT;
    if (ret)
        ret = getcurrentuinfo_num();

    RETURN_LONG(ret);
}

static PHP_FUNCTION(bbs_getcurrentuser)
{
    zval *user_array;
    long ret;
    MAKE_STD_ZVAL(user_array);
    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (zend_parse_parameters(1 TSRMLS_CC, "a", &user_array) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }

    if (array_init(user_array) != SUCCESS) {
        ret = 0;
    } else {
        if (getcurrentuser()) {
            assign_user(user_array, getcurrentuser(), getcurrentuser_num());
            ret = getcurrentuser_num();
        } else
            ret = 0;
    }
    RETURN_LONG(ret);
}

static PHP_FUNCTION(bbs_setonlineuser)
{
    zval *user_array;
    char *userid;
    int userid_len;
    long utmpnum;
    long utmpkey;
    long ret;
    struct user_info *pui;
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
    if (utmpnum < 0 || utmpnum >= MAXACTIVE)
        RETURN_LONG(2);

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
    u_info=pui;
    RETURN_LONG(ret);
}

static char* output_buffer=NULL;
static int output_buffer_len=0;
static int output_buffer_size=0;

static void output_printf(const char* buf, size_t len)
{
	int bufLen;
	int n,newsize;
	char * newbuf;
	if (output_buffer==NULL) {
		output_buffer=(char* )emalloc(51200); //first 50k
		if (output_buffer==NULL) {
			return;
		}
		output_buffer_size=51200;
	}
	bufLen=strlen(buf);
	if (bufLen>len) {
		bufLen=len;
	}
	n=1+output_buffer_len+bufLen-output_buffer_size;
	if (n>=0) {
		newsize=output_buffer_size+((n/102400)+1)*102400; //n*100k every time
		newbuf=(char*)erealloc(output_buffer,newsize);
		if (newbuf==NULL){
			return;
		}
		output_buffer=newbuf;
		output_buffer_size=newsize;
	}
	memcpy(output_buffer+output_buffer_len,buf,bufLen);
	output_buffer_len+=bufLen;
}

static char* get_output_buffer(){
	return output_buffer;
}

static int get_output_buffer_len(){
	int len=output_buffer_len;
	output_buffer_len=0;
	return len;
}

static int new_buffered_output(char *buf, size_t buflen, void *arg)
{
	output_printf(buf,buflen);
	return 0;
}

static void new_flush_buffer(buffered_output_t *output){
}

static int new_write(const char *buf, size_t buflen)
{
	output_printf(buf, buflen);
	return 0;
}

static PHP_FUNCTION(bbs_printansifile)
{
    char *filename;
    long filename_len;
    long linkmode;
    char *ptr;
    int fd;
    struct stat st;
    const int outbuf_len = 4096;
    buffered_output_t *out;
    char* attachlink;
    long attachlink_len;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (ZEND_NUM_ARGS() == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "s", &filename, &filename_len) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        linkmode = 1;
        attachlink=NULL;
    } else if (ZEND_NUM_ARGS() == 2) {
        if (zend_parse_parameters(2 TSRMLS_CC, "sl", &filename, &filename_len, &linkmode) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        attachlink=NULL;
    } else 
        if (zend_parse_parameters(3 TSRMLS_CC, "sls", &filename, &filename_len, &linkmode,&attachlink,&attachlink_len) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
    fd = open(filename, O_RDONLY);
    if (fd < 0)
        RETURN_LONG(2);
    if (fstat(fd, &st) < 0) {
        close(fd);
        RETURN_LONG(2);
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        RETURN_LONG(2);
    }
    if (st.st_size <= 0) {
        close(fd);
        RETURN_LONG(2);
    }

    ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if (ptr == NULL)
        RETURN_LONG(-1);
	if ((out = alloc_output(outbuf_len)) == NULL)
	{
		munmap(ptr, st.st_size);
        RETURN_LONG(2);
	}
/*
	override_default_output(out, buffered_output);
	override_default_flush(out, flush_buffer);
*/
	/*override_default_output(out, new_buffered_output);
	override_default_flush(out, new_flush_buffer);*/
	override_default_write(out, new_write);

    if (!sigsetjmp(bus_jump, 1)) 
	{
        signal(SIGBUS, sigbus);
        signal(SIGSEGV, sigbus);
		output_ansi_html(ptr, st.st_size, out, attachlink);
		free_output(out);
    }
    signal(SIGBUS, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);
    munmap(ptr, st.st_size);
	RETURN_STRINGL(get_output_buffer(), get_output_buffer_len(),1);
}

static void bbs_make_article_array(zval * array, struct fileheader *fh, char *flags, size_t flags_len)
{
    add_assoc_string(array, "FILENAME", fh->filename, 1);
    add_assoc_long(array, "ID", fh->id);
    add_assoc_long(array, "GROUPID", fh->groupid);
    add_assoc_long(array, "REID", fh->reid);
    add_assoc_long(array, "POSTTIME", get_posttime(fh));
    add_assoc_stringl(array, "INNFLAG", fh->innflag, sizeof(fh->innflag), 1);
    add_assoc_string(array, "OWNER", fh->owner, 1);
    add_assoc_string(array, "TITLE", fh->title, 1);
    add_assoc_long(array, "LEVEL", fh->level);
    add_assoc_stringl(array, "FLAGS", flags, flags_len, 1);
    add_assoc_long(array, "ATTACHPOS", fh->attachment);
}

static PHP_FUNCTION(bbs_searchtitle)
{
    char *board,*title, *title2, *title3,*author;
    long bLen,tLen,tLen2,tLen3,aLen;
    long date,mmode,origin,attach;
    bcache_t bh;
	char dirpath[STRLEN];
	int fd;
	struct stat buf;
	struct flock ldata;
	struct fileheader *ptr1;
	char* ptr;
	long int threadsFounded;
	unsigned int *IDList;
	unsigned int *IDList2;
	unsigned int *index;
	long int threads;
	int total,i;
	zval * element;
	int is_bm;
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
    struct boardheader *bp;
	int found;


    if (ZEND_NUM_ARGS() != 9 || zend_parse_parameters(9 TSRMLS_CC, "sssssllll", &board, &bLen,&title,&tLen, &title2, &tLen2, &title3, &tLen3,&author, &aLen, &date,&mmode,&attach,&origin) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }
    if (date <= 0)
        date = 9999;
    if (date > 9999)
        date = 9999;
    if ((bp = getbcache(board)) == NULL) {
        RETURN_FALSE;
    }
    is_bm = is_BM(bp, currentuser);
    if (getboardnum(board, &bh) == 0)
        RETURN_LONG(-1); //"´íÎóµÄÌÖÂÛÇø";
    if (!check_read_perm(currentuser, &bh))
        RETURN_LONG(-2); //ÄúÎÞÈ¨ÔÄ¶Á±¾°æ;
    setbdir(DIR_MODE_NORMAL, dirpath, bh.filename);
    if ((fd = open(dirpath, O_RDONLY, 0)) == -1)
        RETURN_LONG(-3);   
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata)== -1) {
		close(fd);
		RETURN_LONG(-200);
	}
	if (fstat(fd, &buf) == -1) {
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
		RETURN_LONG(-201);
	}
    total = buf.st_size / sizeof(struct fileheader);

    if ((i = safe_mmapfile_handle(fd, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, (size_t*)&buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        RETURN_LONG(-4);
    }
    /*
     * fetching articles 
     */
    if (array_init(return_value) == FAILURE) {
        RETURN_LONG(-210);
    }
#ifdef HAVE_BRC_CONTROL
    brc_initial(currentuser->userid, board);
#endif
	IDList	= emalloc((1000)*sizeof(long int));
	if (IDList==NULL) {
		RETURN_LONG(-211);
	}
	IDList2	= emalloc((50000)*sizeof(long int));
	if (IDList2==NULL) {
		RETURN_LONG(-212);
	}
	index	= emalloc((50000)*sizeof(long int));
	if (index==NULL) {
		RETURN_LONG(-213);
	}
	threadsFounded=0;
	threads=0;
    ptr1 = (struct fileheader *) ptr;

	for (i=total-1;i>=0;i--) {
		if (foundInArray(ptr1[i].groupid,IDList2,threads)==-1)	{
			found=Search_Bin(ptr,ptr1[i].groupid,0,total-1);
			if (found>=0) {
				IDList2[threads]=ptr1[i].groupid;
				index[threads]=found;
				threads++;
			}
		} else {
			break;
		}
		if (threads>10000) 
			break;
		if (title[0] && !strcasestr(ptr1[i].title, title))
	        continue;
	    if (title2[0] && !strcasestr(ptr1[i].title, title2))
	        continue;
	    if (author[0] && strcasecmp(ptr1[i].owner, author))
	        continue;
		if (title3[0] && strcasestr(ptr1[i].title, title3))
			continue;
		if (abs(time(0) - get_posttime(ptr1+i)) > date * 86400)
			break;
		if (mmode && !(ptr1[i].accessed[0] & FILE_MARKED) && !(ptr1[i].accessed[0] & FILE_DIGEST))
			continue;
		if (origin && (ptr1[i].groupid!=ptr1[i].id) )
			continue;
		if (origin && ptr1[i].attachment==0)
			continue;
		if (foundInArray(ptr1[i].groupid,IDList,threadsFounded)==-1)	{
			int found,tmp=foundInArray(ptr1[i].groupid,IDList2,threads);
			found=index[tmp];
			IDList[threadsFounded]=ptr1[i].groupid;
			threadsFounded++;
			MAKE_STD_ZVAL(element);
			array_init(element);
			flags[0] = get_article_flag(ptr1+found, currentuser, board, is_bm);
			if (is_bm && (ptr1[found].accessed[0] & FILE_IMPORTED))
				flags[1] = 'y';
			else
				flags[1] = 'n';
			if (ptr1[found].accessed[1] & FILE_READ)
				flags[2] = 'y';
			else
				flags[2] = 'n';
			if (ptr1[found].attachment)
				flags[3] = '@';
			else
				flags[3] = ' ';
			bbs_make_article_array(element, ptr1+found, flags, sizeof(flags));
			add_assoc_long(element, "threadsnum",tmp);
			zend_hash_index_update(Z_ARRVAL_P(return_value),threadsFounded, (void *) &element, sizeof(zval *), NULL);
			if (threadsFounded>=999){
				break;
			}
		}
	}
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* ÍË³ö»¥³âÇøÓò*/
    close(fd);
    efree(IDList);
    efree(IDList2);
	efree(index);
}


/* function bbs_caneditfile(string board, string filename);
 * ÅÐ¶Ïµ±Ç°ÓÃ»§ÊÇ·ñÓÐÈ¨±à¼­Ä³ÎÄ¼þ
 */
 static PHP_FUNCTION(bbs_caneditfile)
{
    char *board,*filename;
    long boardLen,filenameLen;
	char path[512];
    struct fileheader x;
    bcache_t *brd;


    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if ((ZEND_NUM_ARGS() != 2) || (zend_parse_parameters(2 TSRMLS_CC, "ss", &board, &boardLen,&filename,&filenameLen) != SUCCESS)) {
		WRONG_PARAM_COUNT;
    } 
    brd = getbcache(board);
    if (brd == NULL) {
        RETURN_LONG(-1); //ÌÖÂÛÇøÃû³Æ´íÎó
    }
	if (currentuser==NULL)
		RETURN_FALSE;
    if (!strcmp(brd->filename, "syssecurity")
        || !strcmp(brd->filename, "junk")
        || !strcmp(brd->filename, "deleted"))   /* Leeward : 98.01.22 */
         RETURN_LONG(-2);  //±¾°æ²»ÄÜÐÞ¸ÄÎÄÕÂ
    if (checkreadonly(brd->filename) == true) {
		RETURN_LONG(-3); //±¾°æÒÑ±»ÉèÖÃÖ»¶Á
    }
    if (get_file_ent(brd->filename, filename, &x) == 0) {
        RETURN_LONG(-4); //ÎÞ·¨È¡µÃÎÄ¼þ¼ÇÂ¼
    }
	setbfile(path, brd->filename, filename);
    if (!HAS_PERM(currentuser, PERM_SYSOP)     /* SYSOP¡¢µ±Ç°°æÖ÷¡¢Ô­·¢ÐÅÈË ¿ÉÒÔ±à¼­ */
        &&!chk_currBM(brd->BM, currentuser)) {
        if (!isowner(currentuser, &x)) {
            RETURN_LONG(-5); //²»ÄÜÐÞ¸ÄËûÈËÎÄÕÂ!
        }
        else if (file_time(path) < currentuser->firstlogin) {
            RETURN_LONG(-6); //Í¬ÃûID²»ÄÜÐÞ¸ÄÀÏIDµÄÎÄÕÂ
        }
    }
    /* °æÖ÷½ûÖ¹POST ¼ì²é */
    if (deny_me(currentuser->userid, brd->filename) && !HAS_PERM(currentuser, PERM_SYSOP)) {
        RETURN_LONG(-7); //ÄúµÄPOSTÈ¨±»·â
    }
    RETURN_LONG(0);
}


/* function bbs_printoriginfile(string board, string filename);
 * Êä³öÔ­ÎÄÄÚÈÝ¹©±à¼­
 */
static PHP_FUNCTION(bbs_printoriginfile)
{
    char *board,*filename;
    long boardLen,filenameLen;
    FILE* fp;
    const int outbuf_len = 4096;
	char buf[512],path[512];
    buffered_output_t *out;
	int i;
	int skip;
	bcache_t* bp;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if ((ZEND_NUM_ARGS() != 2) || (zend_parse_parameters(2 TSRMLS_CC, "ss", &board,&boardLen, &filename,&filenameLen) != SUCCESS)) {
		WRONG_PARAM_COUNT;
    } 
	if ( (bp=getbcache(board))==0) {
		RETURN_LONG(-1);
	}
	setbfile(path, bp->filename, filename);
    fp = fopen(path, "r");
    if (fp == 0)
        RETURN_LONG(-1); //ÎÄ¼þÎÞ·¨¶ÁÈ¡
	if ((out = alloc_output(outbuf_len)) == NULL)
	{
        RETURN_LONG(-2);
	}
	override_default_write(out, zend_write);
	/*override_default_output(out, buffered_output);
	override_default_flush(out, flush_buffer);*/
	
	i=0;    
	skip=0;
    while (skip_attach_fgets(buf, sizeof(buf), fp) != 0) {
        char tmp[256];
		i++;
        if (Origin2(buf))
            break;
		if ((i==1) && (strncmp(buf,"·¢ÐÅÈË",6)==0)) {
			skip=1;
		}
		if ((skip) && (i<=4) ){
			continue;
		}
        if (!strcasestr(buf, "</textarea>"))
		{
			int len = strlen(buf);
            BUFFERED_OUTPUT(out, buf, len);
		}
    }
	BUFFERED_FLUSH(out);
	free_output(out);
    RETURN_LONG(0);
}

static PHP_FUNCTION(bbs_getboard)
{
    zval *array;
    char *boardname;
    int boardname_len;
    const struct boardheader *bh;
    const struct BoardStatus *bs;
    int b_num;

    if (ZEND_NUM_ARGS() == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "s", &boardname, &boardname_len) != SUCCESS)
            WRONG_PARAM_COUNT;
        array = NULL;
    } else {
        if (ZEND_NUM_ARGS() == 2) {
            if (zend_parse_parameters(2 TSRMLS_CC, "sa", &boardname, &boardname_len, &array) != SUCCESS)
                WRONG_PARAM_COUNT;
        } else
            WRONG_PARAM_COUNT;
    }
    if (boardname_len > BOARDNAMELEN)
        boardname[BOARDNAMELEN] = 0;
    b_num = getbnum(boardname);
    if (b_num == 0)
        RETURN_LONG(0);
    bh = getboard(b_num);
    bs = getbstatus(b_num);
    if (array) {
        if (array_init(array) != SUCCESS)
            WRONG_PARAM_COUNT;
        assign_board(array, bh, bs, b_num);
    }
    RETURN_LONG(b_num);
}

static int bbs_cmpboard(const struct newpostdata *brd, const struct newpostdata *tmp)
{
    register int type = 0;

    if (!(currentuser->flags & BRDSORT_FLAG)) {
        type = brd->title[0] - tmp->title[0];
        if (type == 0)
            type = strncasecmp(brd->title + 1, tmp->title + 1, 6);
    }
    if (type == 0)
        type = strcasecmp(brd->name, tmp->name);
    return type;
}

/* TODO: move this function into bbslib. */
static int check_newpost(struct newpostdata *ptr)
{
    struct BoardStatus *bptr;

    ptr->total = ptr->unread = 0;

    bptr = getbstatus(ptr->pos+1);
    if (bptr == NULL)
        return 0;
    ptr->total = bptr->total;

    if (!strcmp(currentuser->userid, "guest")) {
        ptr->unread = 1;
        return 1;
    }

#ifdef HAVE_BRC_CONTROL
    if (!brc_initial(currentuser->userid, ptr->name)) {
        ptr->unread = 1;
    } else {
        if (brc_unread(bptr->lastpost)) {
            ptr->unread = 1;
        }
    }
#else
    ptr->unread = 0;
#endif
    return 1;
}

#define BOARD_COLUMNS 10

char *brd_col_names[BOARD_COLUMNS] = {
    "NAME",
    "DESC",
    "CLASS",
    "BM",
    "ARTCNT",                   /* article count */
    "UNREAD",
    "ZAPPED",
    "BID",
    "POSITION",                  /* added by caltary */
    "FLAG"           /* is group ?*/
};

/* added by caltary */
struct favbrd_struct
{
  int flag;
  char *title;
  int father;
};
extern struct favbrd_struct favbrd_list[FAVBOARDNUM];
extern int favbrd_list_t;
extern int favnow;

static void bbs_make_board_columns(zval ** columns)
{
    int i;

    for (i = 0; i < BOARD_COLUMNS; i++) {
        MAKE_STD_ZVAL(columns[i]);
        ZVAL_STRING(columns[i], brd_col_names[i], 1);
    }
}

static void bbs_make_board_zval(zval * value, char *col_name, struct newpostdata *brd)
{
    int len = strlen(col_name);

    if (strncmp(col_name, "ARTCNT", len) == 0) {
        ZVAL_LONG(value, brd->total);
    } else if (strncmp(col_name, "UNREAD", len) == 0) {
        ZVAL_LONG(value, brd->unread);
    } else if (strncmp(col_name, "ZAPPED", len) == 0) {
        ZVAL_LONG(value, brd->zap);
    } else if (strncmp(col_name, "CLASS", len) == 0) {
        ZVAL_STRINGL(value, brd->title + 1, 6, 1);
    } else if (strncmp(col_name, "DESC", len) == 0) {
        ZVAL_STRING(value, brd->title + 13, 1);
    } else if (strncmp(col_name, "NAME", len) == 0) {
        ZVAL_STRING(value, brd->name, 1);
    } else if (strncmp(col_name, "BM", len) == 0) {
        ZVAL_STRING(value, brd->BM, 1);
    /* added by caltary */
    } else if (strncmp(col_name, "POSITION", len) == 0){
        ZVAL_LONG(value, brd->pos);/*added end */
    } else if (strncmp(col_name, "FLAG", len) == 0){
        ZVAL_LONG(value, brd->flag);/*added end */
    } else if (strncmp(col_name, "BID", len) == 0){
        ZVAL_LONG(value, brd->pos+1);/*added end */
    } else {
        ZVAL_EMPTY_STRING(value);
    }
}

#ifdef HAVE_WFORUM

unsigned int * zapbuf;
/**
 * Fetch all boards which have given prefix into an array.
 * prototype:
 * array bbs_getboards(char *prefix, int group, int yank);
 *
 * @return array of loaded boards on success,
 *         FALSE on failure.
 * @author roy 
 */
static PHP_FUNCTION(bbs_getboards)
{
    /*
     * TODO: The name of "yank" must be changed, this name is totally
     * shit, but I don't know which name is better this time.
     */
    char *prefix;
    int plen;
    int yank;
    int rows = 0;
    struct newpostdata newpost_buffer;
    struct newpostdata *ptr;
    zval **columns;
    zval *element;
    int i;
    int j;
    int ac = ZEND_NUM_ARGS();
    int brdnum, yank_flag;
    int group;
	int total;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);



    /*
     * getting arguments 
     */
    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sll", &prefix, &plen, &group,&yank) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

	if (plen=0) {
		RETURN_FALSE;
	}
    if (currentuser == NULL) {
        RETURN_FALSE;
    }
    /*
     * setup column names 
     */
    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }
    columns = emalloc(BOARD_COLUMNS * sizeof(zval *));
	if (columns==NULL) {
		RETURN_FALSE;
	}
    for (i = 0; i < BOARD_COLUMNS; i++) {
        MAKE_STD_ZVAL(element);
        array_init(element);
        columns[i] = element;
        zend_hash_update(Z_ARRVAL_P(return_value), brd_col_names[i], strlen(brd_col_names[i]) + 1, (void *) &element, sizeof(zval *), NULL);
    }

	total=get_boardcount();
    
	yank_flag = yank;
    if (yank_flag !=1 )
	yank_flag =0 ;

    if  (zapbuf==NULL)  {
		char fname[STRLEN];
		int fd, size;

		size = total* sizeof(int);
   		zapbuf = (int *) emalloc(size);
		if (zapbuf==NULL) {
			RETURN_FALSE;
		}
    	for (i = 0; i < total; i++)
        	zapbuf[i] = 1;
	   	sethomefile(fname, currentuser->userid, ".lastread");       /*userµÄ.lastread£¬ zapÐÅÏ¢ */
        if ((fd = open(fname, O_RDONLY, 0600)) != -1) {
	        size = total * sizeof(int);
	        read(fd, zapbuf, size);
	   	    close(fd);
	    } 
    }
   		
    brdnum = 0;
    {
	    int n;
	    struct boardheader const *bptr;
	    char** namelist;
        int* indexlist;
		time_t tnow;

		tnow = time(0);
        namelist=(char**)emalloc(sizeof(char**)*(total));
		if (namelist==NULL) {
			RETURN_FALSE;
		}
	    indexlist=(int*)emalloc(sizeof(int*)*(total));
		if (indexlist==NULL) {
			RETURN_FALSE;
		}
	    for (n = 0; n < total; n++) {
	        bptr = getboard(n + 1);
	        if (!bptr)
	            continue;
	        if (*(bptr->filename)==0)
	            continue;
			if ( group == -2 ){
				if( ( tnow - bptr->createtime ) > 86400*30 || ( bptr->flag & BOARD_GROUP ) )
					continue;
			}else if (bptr->group!=group)
	            continue;
	        if (!check_see_perm(currentuser,bptr)) {
	            continue;
	        }
	        if ((group==0)&&( strchr(prefix, bptr->title[0]) == NULL && prefix[0] != '*'))
	            continue;
	        if (yank_flag || zapbuf[n] != 0 || (bptr->level & PERM_NOZAP)) {
	            /*¶¼ÒªÅÅÐò*/
	            for (i=0;i<brdnum;i++) {
				    if ( strcasecmp(namelist[i], bptr->filename)>0) 
						break;
				}
				for (j=brdnum;j>i;j--) {
						namelist[j]=namelist[j-1];
					   	indexlist[j]=indexlist[j-1];
				}
			   	namelist[i]=bptr->filename;
			   	indexlist[i]=n;
			   	brdnum++;
		   	}
	   	}
		for (i=0;i<brdnum;i++) {
		  	ptr=&newpost_buffer;
		   	bptr = getboard(indexlist[i]+1);
		   	ptr->dir = bptr->flag&BOARD_GROUP?1:0;
		   	ptr->name = bptr->filename;
		   	ptr->title = bptr->title;
		   	ptr->BM = bptr->BM;
		   	ptr->flag = bptr->flag | ((bptr->level & PERM_NOZAP) ? BOARD_NOZAPFLAG : 0);
		   	ptr->pos = indexlist[i];
		   	if (bptr->flag&BOARD_GROUP) {
			   	ptr->total = bptr->board_data.group_total;
		   	} else ptr->total=-1;
		   	ptr->zap = (zapbuf[indexlist[i]] == 0);
   			check_newpost(ptr);
	        for (j = 0; j < BOARD_COLUMNS; j++) {
       		    MAKE_STD_ZVAL(element);
	            bbs_make_board_zval(element, brd_col_names[j], ptr);
	            zend_hash_index_update(Z_ARRVAL_P(columns[j]), i, (void *) &element, sizeof(zval *), NULL);
	        }
		}
		efree(namelist);
	   	efree(indexlist);
    }

    efree(columns);
}

#else 

extern int brdnum;

/**
 * Fetch all boards which have given prefix into an array.
 * prototype:
 * array bbs_getboards(char *prefix, int yank);
 *
 * @return array of loaded boards on success,
 *         FALSE on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_getboards)
{
    /*
     * TODO: The name of "yank" must be changed, this name is totally
     * shit, but I don't know which name is better this time.
     */
    char *prefix;
    int plen;
    int yank;
    int rows = 0;
    struct newpostdata newpost_buffer[MAXBOARD];
    struct newpostdata *ptr;
    zval **columns;
    zval *element;
    int i;
    int j;
    int ac = ZEND_NUM_ARGS();
    int brdnum, yank_flag;
    int group;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);

    /*
     * getting arguments 
     */
    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sll", &prefix, &plen, &group,&yank) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    /*
     * loading boards 
     */
    /*
     * handle some global variables: currentuser, yank, brdnum, 
     * * nbrd.
     */
    /*
     * NOTE: currentuser SHOULD had been set in funcs.php, 
     * * but we still check it. 
     */
    if (currentuser == NULL) {
        RETURN_FALSE;
    }
    yank_flag = yank;
    if (strcmp(currentuser->userid, "guest") == 0)
        yank_flag = 1;          /* see all boards including zapped boards. */
    if (yank_flag != 0)
        yank_flag = 1;
    brdnum = 0;

    /*
     * TODO: replace load_board() with a new one, without accessing
     * * global variables. 
     */
    if ((brdnum = load_boards(newpost_buffer, prefix, group, 1, MAXBOARD, 1, yank_flag, NULL)) <= 0) {
        RETURN_FALSE;
    }
    /*
     * qsort( nbrd, brdnum, sizeof( nbrd[0] ), 
     * (int (*)(const void *, const void *))bbs_cmpboard );
     */
    rows = brdnum;              /* number of loaded boards */

    /*
     * fill data in output array. 
     */
    /*
     * setup column names 
     */
    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }
    columns = emalloc(BOARD_COLUMNS * sizeof(zval *));
	if (columns==NULL) {
		RETURN_FALSE;
	}
    for (i = 0; i < BOARD_COLUMNS; i++) {
        MAKE_STD_ZVAL(element);
        array_init(element);
        columns[i] = element;
        zend_hash_update(Z_ARRVAL_P(return_value), brd_col_names[i], strlen(brd_col_names[i]) + 1, (void *) &element, sizeof(zval *), NULL);
    }
    /*
     * fill data for each column 
     */
    for (i = 0; i < rows; i++) {
        ptr = &newpost_buffer[i];
        check_newpost(ptr);
        for (j = 0; j < BOARD_COLUMNS; j++) {
            MAKE_STD_ZVAL(element);
            bbs_make_board_zval(element, brd_col_names[j], ptr);
            zend_hash_index_update(Z_ARRVAL_P(columns[j]), i, (void *) &element, sizeof(zval *), NULL);
        }
    }
    efree(columns);
}
#endif
 

/**
 * Fetch a list of articles in a board into an array.
 * prototype:
 * array bbs_getarticles(char *board, int start, int num, int mode);
 *
 * @return array of loaded articles on success,
 *         FALSE on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_getarticles)
{
    char *board;
    int blen;
    int start;
    int num;
    int mode;
    char dirpath[STRLEN];
    char dirpath1[STRLEN];	/* add by stiger */
    int total;
    struct fileheader *articles;
    struct boardheader *bp;
    int rows;
    int i;
    zval *element;
    int is_bm;
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
    int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments 
     */
    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "slll", &board, &blen, &start, &num, &mode) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    /*
     * checking arguments 
     */
    if (currentuser == NULL) {
        RETURN_FALSE;
    }
    if ((bp = getbcache(board)) == NULL) {
        RETURN_FALSE;
    }
    is_bm = is_BM(bp, currentuser);

    setbdir(mode, dirpath, bp->filename);
    total = get_num_records(dirpath, sizeof(struct fileheader));
    /* add by stiger */
	if(mode == DIR_MODE_NORMAL){
    	sprintf(dirpath1,"boards/%s/" DING_DIR,bp->filename);
    	total += get_num_records(dirpath1, sizeof(struct fileheader));
	}
    /* add end */
    if (start > (total - num + 1))
        start = (total - num + 1);
    if (start <= 0)
        start = 1;

    /*
     * fetching articles 
     */
    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }
#ifdef HAVE_BRC_CONTROL
    brc_initial(currentuser->userid, bp->filename);
#endif
    articles = emalloc(num * sizeof(struct fileheader));
	if (articles==NULL) {
		RETURN_FALSE;
	}
    /* modified by stiger */
	if(mode == DIR_MODE_NORMAL)
    	rows = read_get_records(dirpath, dirpath1, articles, sizeof(struct fileheader), start, num);
	else
    	rows = get_records(dirpath, articles, sizeof(struct fileheader), start, num);
    for (i = 0; i < rows; i++) {
        MAKE_STD_ZVAL(element);
        array_init(element);
        flags[0] = get_article_flag(articles + i, currentuser, bp->filename, is_bm);
        if (is_bm && (articles[i].accessed[0] & FILE_IMPORTED))
            flags[1] = 'y';
        else
            flags[1] = 'n';
        if (articles[i].accessed[1] & FILE_READ)
            flags[2] = 'y';
        else
            flags[2] = 'n';
        if (articles[i].attachment)
            flags[3] = '@';
        else
            flags[3] = ' ';
        bbs_make_article_array(element, articles + i, flags, sizeof(flags));
        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
    }
    efree(articles);
}

static PHP_FUNCTION(bbs_getwebmsgs){
    char *user;
    int user_len;
    char buf[MAX_MSG_SIZE], showmsg[MAX_MSG_SIZE*2], chk[STRLEN];
    int count,i;
    char title[STRLEN];
    struct msghead head;
    char fname[STRLEN];
	zval *element;

    if (ZEND_NUM_ARGS()!=0 ) {
        WRONG_PARAM_COUNT;
    }


    if(!HAS_PERM(currentuser, PERM_PAGE)) 
		RETURN_LONG(-1);
    if (array_init(return_value) == FAILURE) {
        RETURN_LONG(-2);
    }
    count = get_msgcount(0, currentuser->userid);
    if(count!=0) {
  	    for(i=0;i<count;i++) {
            load_msghead(0, currentuser->userid, i, &head);
            load_msgtext(currentuser->userid, &head, buf);
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

static PHP_FUNCTION(bbs_mailwebmsgs){
    if (ZEND_NUM_ARGS()!=0 ) {
        WRONG_PARAM_COUNT;
    }
	mail_msg(currentuser);
	RETURN_TRUE;
}

/**
 * »ñÈ¡°æÃæ¾«»ªÖ÷ÌâÊýÁ¿
 * prototype:
 * int bbs_get_elite_num(char *board);
 *
 * @return elite top number
 *         < 0 on failure.
 * @author roy
 */
static PHP_FUNCTION(bbs_get_elite_num){
	RETURN_FALSE;
}
/**
 * »ñÈ¡´Óstart¿ªÊ¼µÄnum¸ö¾«»ªÖ÷Ìâ
 * prototype:
 * array bbs_get_elite_list(char *board, int start, int num);
 *
 * @return array of loaded articles on success,
 *         FALSE on failure.
 * @author roy
 */
static PHP_FUNCTION(bbs_get_elite_list){
	RETURN_FALSE;
}

/**
 * »ñÈ¡´Óstart¿ªÊ¼µÄnum¸ö°æÃæÖ÷Ìâ
 * prototype:
 * array bbs_getthreads(char *board, int start, int num,int includeTop);
 *
 * @return array of loaded articles on success,
 *         FALSE on failure.
 * @author roy
 */
static PHP_FUNCTION(bbs_getthreads)
{
    char *board;
    int blen;
    int start,num;
    int total;
    struct boardheader *bp=NULL;
	char dirpath[STRLEN];
    int i,j;
    zval *element;
    int is_bm;
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
	int fd;
	struct stat buf;
	struct flock ldata;
	struct wwwthreadheader *ptr1=NULL;
	char* ptr;
	unsigned int long found;
	int includeTop;
	int skip;
    int ac = ZEND_NUM_ARGS();
	int begin,end;
	zval* columns[3];
	char* thread_col_names[]={"origin","lastreply","articlenum"};



    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    /*
     * getting arguments 
     */
    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "slll", &board, &blen, &start, &num, &includeTop) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

	if (start<0){
		RETURN_FALSE;
	}
	if (num<0){
		RETURN_FALSE;
	}
    /*
     * checking arguments 
     */
    if (currentuser == NULL) {
        RETURN_FALSE;
    }
    if ((bp = getbcache(board)) == NULL) {
        RETURN_FALSE;
    }

    is_bm = is_BM(bp, currentuser);

    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }
#ifdef HAVE_BRC_CONTROL
    brc_initial(currentuser->userid, bp->filename);
#endif


    setbdir(DIR_MODE_WEB_THREAD, dirpath, bp->filename);

    if ((fd = open(dirpath, O_RDONLY, 0)) == -1) {
        RETURN_LONG(-1);   
	}
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata)==-1) {
		close(fd);
		RETURN_LONG(-200);
	}
	if (fstat(fd, &buf)==-1) {
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
		RETURN_LONG(-201);
	}
    total = buf.st_size / sizeof(struct wwwthreadheader);

    if ((i = safe_mmapfile_handle(fd, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, (size_t*)&buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        RETURN_LONG(-2);
    }


    ptr1 = (struct fileheader *) ptr;
    /*
     * fetching articles 
     */
	 total--;
	if (!includeTop) {
		for (i=total;i>=0;i--) {
			if (!( ptr1[i].flags & FILE_ON_TOP )) 
				break;
		}
		total=i;
	} 
	begin=total-start;
	end=total-start-num+1;
	if (end<0)
		end=0;

	for (i=begin;i>=end;i--) {
		MAKE_STD_ZVAL(element);
		array_init(element);
		for (j = 0; j < 3; j++) {
			MAKE_STD_ZVAL(columns[j] );
			zend_hash_update(Z_ARRVAL_P(element), thread_col_names[j], strlen(thread_col_names[j]) + 1, (void *) &columns[j] , sizeof(zval *), NULL);
		}
		flags[0] = get_article_flag(&(ptr1[i].origin), currentuser, bp->filename, is_bm);
		if (is_bm && (ptr1[i].origin.accessed[0] & FILE_IMPORTED))
			flags[1] = 'y';
		else
			flags[1] = 'n';
		if (ptr1[i].origin.accessed[1] & FILE_READ)
			flags[2] = 'y';
		else
			flags[2] = 'n';
		if (ptr1[i].origin.attachment)
			flags[3] = '@';
		else
			flags[3] = ' ';
		array_init(columns[0] );
		bbs_make_article_array(columns[0], &(ptr1[i].origin), flags, sizeof(flags));
		flags[0] = get_article_flag(&(ptr1[i].lastreply), currentuser, bp->filename, is_bm);
		if (is_bm && (ptr1[i].lastreply.accessed[0] & FILE_IMPORTED))
			flags[1] = 'y';
		else
			flags[1] = 'n';
		if (ptr1[i].lastreply.accessed[1] & FILE_READ)
			flags[2] = 'y';
		else
			flags[2] = 'n';
		if (ptr1[i].lastreply.attachment)
			flags[3] = '@';
		else
			flags[3] = ' ';
		array_init(columns[1] );
		bbs_make_article_array(columns[1], &(ptr1[i].lastreply), flags, sizeof(flags));
		ZVAL_LONG(columns[2],ptr1[i].articlecount);

		zend_hash_index_update(Z_ARRVAL_P(return_value), begin-i, (void *) &element, sizeof(zval *), NULL);
	}
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* ÍË³ö»¥³âÇøÓò*/
    close(fd);
}

#ifdef HAVE_WFORUM
static PHP_FUNCTION(bbs_get_article)
{
    char *board;
    int blen;
    int groupid;
    int total;
    struct fileheader *articles;
    struct boardheader *bp;
	char dirpath[STRLEN];
    zval *element;
    int is_bm;
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
    int ac = ZEND_NUM_ARGS();
	unsigned int articlesFounded;
	int fd;
	int i;

	struct stat buf;
	struct flock ldata;
	struct fileheader *ptr1;
	char* ptr;
	unsigned int found;


    /*
     * getting arguments 
     */
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &board, &blen, &groupid) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

	if (groupid<0){
		RETURN_LONG(-3);
	}
    /*
     * checking arguments 
     */
    if (currentuser == NULL) {
        RETURN_LONG(-4);
    }
    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(-5);
    }
    is_bm = is_BM(bp, currentuser);
    setbdir(DIR_MODE_NORMAL, dirpath, bp->filename);

    if ((fd = open(dirpath, O_RDONLY, 0)) == -1)
        RETURN_LONG(-6);   
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata)==-1) {
		close(fd);
		RETURN_LONG(-200);
	}
	if (fstat(fd, &buf)==-1) {
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
		RETURN_LONG(-201);
	}
    total = buf.st_size / sizeof(struct fileheader);

    if ((i = safe_mmapfile_handle(fd, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, (size_t*)&buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        RETURN_LONG(-7);
    }
    ptr1 = (struct fileheader *) ptr;
    /*
     * fetching articles 
     */
    if (array_init(return_value) == FAILURE) {
        RETURN_LONG(-8);
    }
#ifdef HAVE_BRC_CONTROL
    brc_initial(currentuser->userid, bp->filename);
#endif

	articlesFounded=0;

	if ( (found=Search_Bin(ptr,groupid,0,total-1))>=0) {
		MAKE_STD_ZVAL(element);
		array_init(element);
		flags[0] = get_article_flag(ptr1+found, currentuser, bp->filename, is_bm);
		if (is_bm && (ptr1[found].accessed[0] & FILE_IMPORTED))
			flags[1] = 'y';
		else
			flags[1] = 'n';
		if (ptr1[found].accessed[1] & FILE_READ)
			flags[2] = 'y';
		else
			flags[2] = 'n';
		if (ptr1[found].attachment)
			flags[3] = '@';
		else
			flags[3] = ' ';
		bbs_make_article_array(element, ptr1+found, flags, sizeof(flags));
		zend_hash_index_update(Z_ARRVAL_P(return_value), 0 , (void *) &element, sizeof(zval *), NULL);
	}else 
		RETURN_FALSE;
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* ÍË³ö»¥³âÇøÓò*/
    close(fd);
}
#endif

/**
 * ·´Ðò»ñÈ¡´Óstart¿ªÊ¼µÄnum¸öÍ¬Ö÷ÌâÎÄÕÂ
 * prototype:
 * array bbs_get_thread_articles(char *board, int groupID, int start, int num);
 *
 * @return array of loaded articles on success,
 *         FALSE on failure.
 * @author roy
 */
static PHP_FUNCTION(bbs_get_thread_articles)
{
    char *board;
    int blen;
    int start,num,groupid;
    int total;
    struct fileheader *articles;
    struct boardheader *bp;
	char dirpath[STRLEN];
    int i;
    zval *element;
    int is_bm;
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
    int ac = ZEND_NUM_ARGS();
	unsigned int articlesFounded;
	int fd;
	struct stat buf;
	struct flock ldata;
	struct fileheader *ptr1;
	char* ptr;
	unsigned int found;


    /*
     * getting arguments 
     */
    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "slll", &board, &blen, &groupid, &start, &num) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

	if (start<0){
		RETURN_LONG(-1);
	}
	if (num<0){
		RETURN_LONG(-2);
	}
	if (groupid<0){
		RETURN_LONG(-3);
	}
    /*
     * checking arguments 
     */
    if (currentuser == NULL) {
        RETURN_LONG(-4);
    }
    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(-5);
    }
    is_bm = is_BM(bp, currentuser);
    setbdir(DIR_MODE_NORMAL, dirpath, bp->filename);

    if ((fd = open(dirpath, O_RDONLY, 0)) == -1)
        RETURN_LONG(-6);   
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata)==-1) {
		close(fd);
		RETURN_LONG(-200);
	}
	if (fstat(fd, &buf)==-1) {
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
		RETURN_LONG(-200);
	}
    total = buf.st_size / sizeof(struct fileheader);

    if ((i = safe_mmapfile_handle(fd, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, (size_t*)&buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        RETURN_LONG(-7);
    }
    ptr1 = (struct fileheader *) ptr;
    /*
     * fetching articles 
     */
    if (array_init(return_value) == FAILURE) {
        RETURN_LONG(-8);
    }
#ifdef HAVE_BRC_CONTROL
    brc_initial(currentuser->userid, bp->filename);
#endif

	articlesFounded=0;

	for (i=total-1;i>=0;i--) {
		if (ptr1[i].id<=groupid)
			break;
		if (ptr1[i].groupid==groupid)	{
			articlesFounded++;
			if ((articlesFounded-1)>=start){
				MAKE_STD_ZVAL(element);
				array_init(element);
				flags[0] = get_article_flag(ptr1+i, currentuser, bp->filename, is_bm);
				if (is_bm && (ptr1[i].accessed[0] & FILE_IMPORTED))
					flags[1] = 'y';
				else
					flags[1] = 'n';
				if (ptr1[i].accessed[1] & FILE_READ)
					flags[2] = 'y';
				else
					flags[2] = 'n';
				if (ptr1[i].attachment)
					flags[3] = '@';
				else
					flags[3] = ' ';
				bbs_make_article_array(element, ptr1+i, flags, sizeof(flags));
				zend_hash_index_update(Z_ARRVAL_P(return_value), articlesFounded-1-start, (void *) &element, sizeof(zval *), NULL);
				if (articlesFounded>=num+start){
					break;
				}
			}
		}
	}
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* ÍË³ö»¥³âÇøÓò*/
    close(fd);
}

static PHP_FUNCTION(bbs_get_today_article_num){
    char *board;
    int blen;
    int total;
    struct boardheader *bp;
	char dirpath[STRLEN];
    int i;
    zval *element;
    int is_bm;
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
    int ac = ZEND_NUM_ARGS();
	unsigned int articleNums;
	int fd;
	struct stat buf;
	struct flock ldata;
	struct fileheader *ptr1;
	char* ptr;
	time_t now;
	struct tm nowtm;

    /*
     * getting arguments 
     */
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &board, &blen) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    /*
     * checking arguments 
     */
    if (currentuser == NULL) {
        RETURN_LONG(-2);
    }
    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(-3);
    }
    is_bm = is_BM(bp, currentuser);
    setbdir(DIR_MODE_NORMAL, dirpath, bp->filename);

    if ((fd = open(dirpath, O_RDONLY, 0)) == -1)
        RETURN_LONG(-4);   
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata)==-1) {
		close(fd);
		RETURN_LONG(-200);
	}
	if (fstat(fd, &buf) == -1 ){
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
		RETURN_LONG(-201);
	}
    total = buf.st_size / sizeof(struct fileheader);

    if ((i = safe_mmapfile_handle(fd, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, (size_t*)&buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        RETURN_LONG(-5);
    }
    ptr1 = (struct fileheader *) ptr;
#ifdef HAVE_BRC_CONTROL
    brc_initial(currentuser->userid, bp->filename);
#endif

	articleNums=0;

	now=time(NULL);
	gmtime_r(&now,&nowtm);
	nowtm.tm_sec=0;
	nowtm.tm_min=0;
	nowtm.tm_hour=0;
	now=mktime(&nowtm);

	for (i=total-1;i>=0;i--) {
		if (get_posttime(ptr1+i)<now)
			break;
		articleNums++;
	}
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* ÍË³ö»¥³âÇøÓò*/
    close(fd);
	RETURN_LONG(articleNums);
}

/**
 * »ñÈ¡Í¬Ö÷ÌâµÄÎÄÕÂ¸öÊý
 * prototype:
 * array bbs_get_thread_article_num(char *board,  int groupID);
 *
 * @return array of loaded articles on success,
 *         FALSE on failure.
 * @author roy
 */
static PHP_FUNCTION(bbs_get_thread_article_num)
{
    char *board;
    int blen;
    unsigned int groupid;
    int total;
    struct boardheader *bp;
	char dirpath[STRLEN];
    int i;
    zval *element;
    int is_bm;
    char flags[4];              /* flags[0]: flag character
                                 * flags[1]: imported flag
                                 * flags[2]: no reply flag
                                 * flags[3]: attach flag
                                 */
    int ac = ZEND_NUM_ARGS();
	unsigned int articleNums;
	int fd;
	struct stat buf;
	struct flock ldata;
	struct fileheader *ptr1;
	char* ptr;
	unsigned int found;

    /*
     * getting arguments 
     */
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &board, &blen, &groupid) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

	if (groupid<0){
		RETURN_LONG(-1);
	}
    /*
     * checking arguments 
     */
    if (currentuser == NULL) {
        RETURN_LONG(-2);
    }
    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(-3);
    }
    is_bm = is_BM(bp, currentuser);
    setbdir(DIR_MODE_NORMAL, dirpath, bp->filename);

    if ((fd = open(dirpath, O_RDONLY, 0)) == -1)
        RETURN_LONG(-4);   
    ldata.l_type = F_RDLCK;
    ldata.l_whence = 0;
    ldata.l_len = 0;
    ldata.l_start = 0;
    if (fcntl(fd, F_SETLKW, &ldata) == -1) {
		close(fd);
		RETURN_LONG(-200);
	}
	if (fstat(fd, &buf)==-1) {
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
		RETURN_LONG(-201);
	}
    total = buf.st_size / sizeof(struct fileheader);

    if ((i = safe_mmapfile_handle(fd, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, (size_t*)&buf.st_size)) != 1) {
        if (i == 2)
            end_mmapfile((void *) ptr, buf.st_size, -1);
        ldata.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &ldata);
        close(fd);
        RETURN_LONG(-5);
    }
    ptr1 = (struct fileheader *) ptr;
    /*
     * fetching articles 
     */
    if (array_init(return_value) == FAILURE) {
        RETURN_LONG(-6);
    }
#ifdef HAVE_BRC_CONTROL
    brc_initial(currentuser->userid, bp->filename);
#endif

	articleNums=0;

	for (i=total-1;i>=0;i--) {
		if (ptr1[i].id<=groupid)
			break;
		if (ptr1[i].groupid==groupid)
			articleNums++;
	}
    end_mmapfile((void *) ptr, buf.st_size, -1);
    ldata.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &ldata);        /* ÍË³ö»¥³âÇøÓò*/
    close(fd);
	RETURN_LONG(articleNums);
}
/**
 * Count articles in a board with specific .DIR mode.
 * prototype:
 * int bbs_countarticles(int brdnum, int mode);
 *
 * @return non-negative value on success,
 *         negative value on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_countarticles)
{
    int brdnum;
    int mode;
    const struct boardheader *bp = NULL;
    char dirpath[STRLEN];
    int total;
    int ac = ZEND_NUM_ARGS();

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    /*
     * getting arguments 
     */
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ll", &brdnum, &mode) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if ((bp = getboard(brdnum)) == NULL) {
        RETURN_LONG(-1);
    }
    setbdir(mode, dirpath, bp->filename);

	if(mode == DIR_MODE_THREAD){
//		if(setboardtitle(board, -1)){
			gen_title(bp->filename);
//		}
	}

    total = get_num_records(dirpath, sizeof(struct fileheader));
    /* add by stiger */
    sprintf(dirpath,"boards/%s/%s",bp->filename, DING_DIR);
    total += get_num_records(dirpath, sizeof(struct fileheader));
    /* add end */
    RETURN_LONG(total);
}


/* long bbs_getthreadnum(long boardNum)
 * get number of threads
 */
static PHP_FUNCTION(bbs_getthreadnum)
{
    int brdnum;
    int mode;
    const struct boardheader *bp = NULL;
    char dirpath[STRLEN];
    int total;
    int ac = ZEND_NUM_ARGS();
	struct stat normalStat,originStat;
	char dirpath1[STRLEN];

    /*
     * getting arguments 
     */
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l", &brdnum) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if ((bp = getboard(brdnum)) == NULL) {
        RETURN_LONG(-1);
    }
    setbdir(DIR_MODE_WEB_THREAD, dirpath, bp->filename);
	if (!stat(dirpath,&originStat))	{
		setbdir(DIR_MODE_NORMAL,dirpath1,bp->filename);
		if (!stat(dirpath1,&normalStat)){
			if (normalStat.st_mtime>originStat.st_mtime){
				www_generateOriginIndex(bp->filename);
			}
		} else {
			www_generateOriginIndex(bp->filename);
		}
	} else {
		www_generateOriginIndex(bp->filename);
	}
   total = get_num_records(dirpath, sizeof(struct wwwthreadheader));


    RETURN_LONG(total);
}

/**
 * Get filename from num in the DIR
 * @param sll
 * 		s: board
 * 		l: num
 * 		l: mode
 * @return error: 0
 * 		   success: s: filename
 * @author: stiger
 */
static PHP_FUNCTION(bbs_get_filename_from_num)
{

	char *board;
	int blen;
	int num;
	int mode;
	struct boardheader *bp;
	char dirpath[STRLEN];
	fileheader_t fh;
	FILE *fp;

    int ac = ZEND_NUM_ARGS();
    if (ac != 3
        ||zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll", &board, &blen, &num, &mode) == FAILURE)
    {
        WRONG_PARAM_COUNT;
    }
    /* check for parameter being passed by reference */
	if (currentuser == NULL)
	{
		RETURN_LONG(0);
	}
	if ((bp = getbcache(board)) == NULL)
	{
		RETURN_LONG(0);
	}

	setbdir(mode, dirpath, bp->filename);
	if ((fp=fopen(dirpath,"r"))==NULL)
	{
		RETURN_LONG(0);
	}
	fseek(fp, sizeof(fh) * (num-1), SEEK_SET);
	if( fread(&fh, sizeof(fh), 1, fp) < 1 )
	{
		fclose(fp);
		RETURN_LONG(0);
	}
	fclose(fp);

	RETURN_STRING(fh.filename,1);
}

/**
 * Get a article records from the article num.
 * prototype:
 * int bbs_get_records_from_num(string dirpath, long num, arrary articles);
 *
 * @return Record index on success,
 *       0 on failure.
 * @author stiger
 */
static PHP_FUNCTION(bbs_get_records_from_num)
{
	int num;
	FILE *fp;
	char *dirpath;
	int dlen;
	fileheader_t articles;
	int i;
	zval *element,*articlearray;
	char flags[3]; /* flags[0]: flag character
					* flags[1]: imported flag
					* flags[2]: no reply flag
					*/
    int ac = ZEND_NUM_ARGS();
    int retnum;
    if (ac != 3
        ||zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sla", &dirpath, &dlen, &num, &articlearray) == FAILURE)
    {
        WRONG_PARAM_COUNT;
    }

	if (currentuser == NULL)
	{
		RETURN_LONG(0);
	}

	if ((fp = fopen(dirpath, "r")) == NULL )
	{
		RETURN_LONG(0);
	}
	fseek(fp, sizeof(articles) * num, SEEK_SET);
	if( fread(&articles, sizeof(articles), 1, fp) < 1 )
	{
		fclose(fp);
		RETURN_LONG(0);
	}
	fclose(fp);

	if(array_init(articlearray) != SUCCESS)
	{
                RETURN_LONG(0);
	}

	MAKE_STD_ZVAL(element);
	array_init(element);
	flags[0]=0;
	flags[1]=0;
	flags[2]=0;
	bbs_make_article_array(element, &articles, flags, sizeof(flags));
	zend_hash_index_update(Z_ARRVAL_P(articlearray), 0,
				(void*) &element, sizeof(zval*), NULL);

	RETURN_LONG(1);
}

/**
 * Get some article records from the article id.
 * prototype:
 * int bbs_get_records_from_id(string board, long id, long mode, arrary articles);
 *
 * @return Record index on success,
 *       0 on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_get_records_from_id)
{
	char *board;
	int blen;
	int id;
	int num;
	int mode;
	int fd;
	char dirpath[STRLEN];
#define record_cnt 3
	fileheader_t articles[record_cnt];
	struct boardheader *bp;
	int i;
	zval *element,*articlearray;
	int is_bm;
	char flags[3]; /* flags[0]: flag character
					* flags[1]: imported flag
					* flags[2]: no reply flag
					*/
    int ac = ZEND_NUM_ARGS();
    int retnum;

    if (ac != 4
        ||zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slla", &board, &blen, &id, &mode, &articlearray) == FAILURE)
    {
        WRONG_PARAM_COUNT;
    }

    /* check for parameter being passed by reference 
	if (currentuser == NULL)
	{
		RETURN_LONG(0);
	}
	*/
	if ((bp = getbcache(board)) == NULL)
	{
		RETURN_LONG(0);
	}
	/*if (array_init(return_value) == FAILURE)
	{
		RETURN_LONG(0);
	}*/
	setbdir(mode, dirpath, bp->filename);
  if(mode == DIR_MODE_ZHIDING){
		num = search_record(dirpath, articles+1, sizeof(struct fileheader), (RECORD_FUNC_ARG) cmpfileid, &id);
		if(num == 0) RETURN_LONG(0);
		memset(articles,0,sizeof(struct fileheader));
		memset(articles+2,0,sizeof(struct fileheader));
  }else{

	if ((fd = open(dirpath, O_RDWR, 0644)) < 0)
	{
		RETURN_LONG(0);
	}
	if ((retnum=get_records_from_id(fd, id, articles, record_cnt, &num)) == 0)
	{
		close(fd);
		RETURN_LONG(0);
	}
	close(fd);
  }
	//MAKE_STD_ZVAL(articlearray);
	if(array_init(articlearray) != SUCCESS)
	{
                RETURN_LONG(0);
	}
	for (i = 0; i < record_cnt; i++)
	{
		MAKE_STD_ZVAL(element);
		array_init(element);
	  if(articles[i].id && currentuser ){
		flags[0] = get_article_flag(articles + i, currentuser, bp->filename, is_bm);
		if (is_bm && (articles[i].accessed[0] & FILE_IMPORTED))
			flags[1] = 'y';
		else
			flags[1] = 'n';
		if (articles[i].accessed[1] & FILE_READ)
			flags[2] = 'y';
		else
			flags[2] = 'n';
	  }else{
		flags[0]=0;
		flags[1]=0;
		flags[2]=0;
	  }
		bbs_make_article_array(element, articles + i, flags, sizeof(flags));
		zend_hash_index_update(Z_ARRVAL_P(articlearray), i,
				(void*) &element, sizeof(zval*), NULL);
	}
	RETURN_LONG(num);
}

/**
 * Get some thread records from an article id.
 * prototype:
 * int bbs_get_threads_from_id(long boardid, long id, long mode, long num);
 *
 * @return Records on success,
 *         FALSE on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_get_threads_from_id)
{
	int bid;
	int id;
	int num;
	int mode;
	char dirpath[STRLEN];
	fileheader_t *articles;
	const struct boardheader *bp;
	int rc;
	int i;
	zval *element;
	char flags[3] = {0x00}; /* flags[0]: flag character
							 * flags[1]: imported flag
							 * flags[2]: no reply flag
							 */
    int ac = ZEND_NUM_ARGS();
    int retnum;

    if (ac != 4
        ||zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &bid, &id, &mode, &num) == FAILURE)
    {
        WRONG_PARAM_COUNT;
    }

    /* check for parameter being passed by reference 
	*/
	if ((bp = getboard(bid)) == NULL)
	{
		RETURN_FALSE;
	}
	if (num == 0)
	{
		RETURN_FALSE;
	}
	if (num < 0)
	{
		if ((articles = emalloc((-num)*sizeof(fileheader_t))) == NULL)
		{
			RETURN_FALSE;
		}
	}
	else
	{
		if ((articles = emalloc(num*sizeof(fileheader_t))) == NULL)
		{
			RETURN_FALSE;
		}
	}
	if (array_init(return_value) == FAILURE)
	{
		retnum = 0;
		goto threads_error;
	}
	setbdir(mode, dirpath, bp->filename);
	if ((rc = get_threads_from_id(dirpath, id, articles, num)) == 0)
	{
		retnum = 0;
		goto threads_error;
	}
	for (i = 0; i < rc; i++)
	{
		MAKE_STD_ZVAL(element);
		array_init(element);
		bbs_make_article_array(element, articles + i, flags, sizeof(flags));
		zend_hash_index_update(Z_ARRVAL_P(return_value), i,
				(void*) &element, sizeof(zval*), NULL);
	}
threads_error:
	efree(articles);
	if (retnum == 0)
		RETURN_FALSE;
}

/**
 * Checking whether a user is a BM of a board or not.
 * prototype:
 * int bbs_is_bm(int brdnum, int usernum);
 *
 * @return one if the user is BM,
 *         zero if not.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_is_bm)
{
    int brdnum;
    int usernum;
    const struct boardheader *bp = NULL;
    const struct userec *up = NULL;
    int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments 
     */
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ll", &brdnum, &usernum) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if ((bp = getboard(brdnum)) == NULL) {
        RETURN_LONG(0);
    }
    if ((up = getuserbynum(usernum)) == NULL) {
        RETURN_LONG(0);
    }
    RETURN_LONG(is_BM(bp, up));
}

static PHP_FUNCTION(bbs_getbname)
{
	int brdnum;
	struct boardheader *bp=NULL;
    int ac = ZEND_NUM_ARGS();

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l", &brdnum) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if ((bp = getboard(brdnum)) == NULL) {
        RETURN_LONG(0);
    }
	RETURN_STRING(bp->filename,1);
}

static PHP_FUNCTION(bbs_checkreadperm)
{
    long user_num, boardnum;
    struct userec *user;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (zend_parse_parameters(2 TSRMLS_CC, "ll", &user_num, &boardnum) != SUCCESS)
        WRONG_PARAM_COUNT;
    user = getuserbynum(user_num);
    if (user == NULL)
        RETURN_LONG(0);
    RETURN_LONG(check_read_perm(user, getboard(boardnum)));
}

static PHP_FUNCTION(bbs_checkpostperm)
{
    long user_num, boardnum;
    struct userec *user;
    const struct boardheader *bh;

	getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (zend_parse_parameters(2 TSRMLS_CC, "ll", &user_num, &boardnum) != SUCCESS)
        WRONG_PARAM_COUNT;
    user = getuserbynum(user_num);
    if (user == NULL)
        RETURN_LONG(0);
	bh=getboard(boardnum);
	if (bh==0) {
		RETURN_LONG(0);
	}
    RETURN_LONG(haspostperm(user, bh->filename));
}


/*  function bbs_postarticle(string boardName, string title,string text, long signature, long reid, long outgo,long anony)  
 *
 *
 */
static PHP_FUNCTION(bbs_postarticle)
{
	FILE *fp;
	char *boardName, *title, *content;
    char filename[80], dir[80], buf[80], buf2[80],path[80],board[80];
	int blen, tlen, clen;
    int r, i, sig;
	int reid;
    struct fileheader x, *oldx;
    bcache_t *brd;
    int local, anony;
    /*int filtered = 0;*/

	int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments 
     */
    
	if (ac != 7 || zend_parse_parameters(7 TSRMLS_CC, "ss/s/llll", &boardName, &blen, &title, &tlen, &content, &clen, &sig, &reid, &local,&anony) == FAILURE) {
		WRONG_PARAM_COUNT;

	} 

    brd = getbcache(boardName);
    if (currentuser == NULL) {

		RETURN_FALSE;
  //ÓÃ»§Î´³õÊ¼»¯
	} 
    if (brd == 0)
        RETURN_LONG(-1); //´íÎóµÄÌÖÂÛÇøÃû³Æ
    if (brd->flag&BOARD_GROUP)
        RETURN_LONG(-2); //¶þ¼¶Ä¿Â¼°æ
    strcpy(board, brd->filename);
    
    for (i = 0; i < tlen; i++) {
        if (title[i] <= 27 && title[i] >= -1)
            title[i] = ' ';
    }
    local = local ? 0 : 1;
    anony = anony ? 1 : 0;
    if (tlen == 0)
        RETURN_LONG(-3); //±êÌâÎªNULL
    sprintf(dir, "boards/%s/.DIR", board);
    if (true == checkreadonly(board) || !haspostperm(currentuser, board))
        RETURN_LONG(-4); //´ËÌÖÂÛÇøÊÇÎ¨¶ÁµÄ, »òÊÇÄúÉÐÎÞÈ¨ÏÞÔÚ´Ë·¢±íÎÄÕÂ.
    if (deny_me(currentuser->userid, board) && !HAS_PERM(currentuser, PERM_SYSOP))
        RETURN_LONG(-5); //ºÜ±§Ç¸, Äã±»°æÎñÈËÔ±Í£Ö¹ÁË±¾°æµÄpostÈ¨Àû.
    if (abs(time(0) - *(int *) (u_info->from + 36)) < 6) {
        *(int *) (u_info->from + 36) = time(0);
        RETURN_LONG(-6); // Á½´Î·¢ÎÄ¼ä¸ô¹ýÃÜ, ÇëÐÝÏ¢¼¸ÃëºóÔÙÊÔ
    }
    *(int *) (u_info->from + 36) = time(0);
    sprintf(filename, "tmp/%s.%d.tmp", getcurruserid(), getpid());
    if (!sigsetjmp(bus_jump, 1)) {
        signal(SIGBUS, sigbus);
        signal(SIGSEGV, sigbus);
    	f_append(filename, unix_string(content));
    } else {
		RETURN_LONG(-9);
	}
    signal(SIGBUS, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);

    if(reid > 0){
        int pos;int fd;
        oldx = (struct fileheader*)emalloc(sizeof(struct fileheader));

		if (oldx==NULL) {
			RETURN_FALSE;
		}	

		setbfile(path,board,DOT_DIR);
		fd =open(path,O_RDWR);
		if(fd < 0) RETURN_LONG(-7); //Ë÷ÒýÎÄ¼þ²»´æÔÚ
		get_records_from_id(fd,reid,oldx,1,&pos);

		close(fd);
        if (pos < 0) {
    		efree(oldx);
    		oldx = NULL;
        }
        else
        if (oldx->accessed[1] & FILE_READ)
           RETURN_LONG(-8); //±¾ÎÄ²»ÄÜ»Ø¸´
    }
    else {
        oldx = NULL;
    }
#ifdef HAVE_BRC_CONTROL
    brc_initial(currentuser->userid, board);
#endif
    if (is_outgo_board(board) && local == 0)
        local = 0;
    else
        local = 1;
    if (brd->flag&BOARD_ATTACH) {
#if USE_TMPFS==1
        snprintf(buf,MAXPATH,"%s/home/%c/%s/%d/upload",TMPFSROOT,toupper(currentuser->userid[0]),
			currentuser->userid,getcurrentuinfo_num());
#else
        snprintf(buf,MAXPATH,"%s/%s_%d",ATTACHTMPPATH,currentuser->userid,getcurrentuinfo_num());
#endif
        if (!sigsetjmp(bus_jump, 1)) {
            signal(SIGBUS, sigbus);
            signal(SIGSEGV, sigbus);
        	r = post_article(board, title, filename, currentuser, fromhost, sig, local, anony, oldx,buf);
        } else {
			RETURN_LONG(-9);
		}
		signal(SIGBUS, SIG_IGN);
    	signal(SIGSEGV, SIG_IGN);
        f_rm(buf);
	} else {
        if (!sigsetjmp(bus_jump, 1)) {
            signal(SIGBUS, sigbus);
            signal(SIGSEGV, sigbus);
        	r = post_article(board, title, filename, currentuser, fromhost, sig, local, anony, oldx,NULL);
        } else {
			RETURN_LONG(-9);
		}
    
		signal(SIGBUS, SIG_IGN);
    	signal(SIGSEGV, SIG_IGN);
    }
    if (r < 0)
        RETURN_LONG(-9) ; //"ÄÚ²¿´íÎó£¬ÎÞ·¨·¢ÎÄ";
#ifdef HAVE_BRC_CONTROL
    brc_update(currentuser->userid);
#endif
    if(oldx)
    	efree(oldx);
    unlink(filename);
    if (!junkboard(board)) {
        currentuser->numposts++;
        if (!sigsetjmp(bus_jump, 1)) {
            signal(SIGBUS, sigbus);
            signal(SIGSEGV, sigbus);
			/*
stiger: ÔÚ post_article Àï´¦Àí
        	write_posts(currentuser->userid, board, title);
			*/
        }
        signal(SIGBUS, SIG_IGN);
        signal(SIGSEGV, SIG_IGN);
    }
    RETURN_LONG(0);
}

/*  function bbs_updatearticle(string boardName, string filename ,string text)  
 *  ¸üÐÂ±à¼­ÎÄÕÂ
 *
 */
static PHP_FUNCTION(bbs_updatearticle)
{
	FILE *fp;
	char *boardName, *filename, *content;
	int blen, flen, clen;
    FILE *fin;
    FILE *fout;
    char infile[80], outfile[80];
    char buf2[256];
    int i;
    bcache_t *bp;
    /*int filtered = 0;*/

	int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments 
     */
    
	if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sss/", &boardName, &blen, &filename, &flen, &content, &clen) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
   if ( (bp=getbcache(boardName))==0) {
   		RETURN_LONG(-1);
	}		

    setbfile(infile, bp->filename, filename);
    sprintf(outfile, "tmp/%s.%d.editpost", getcurruserid(), getpid());
    if ((fin = fopen(infile, "r")) == NULL)
        RETURN_LONG(-10);
    if ((fout = fopen(outfile, "w")) == NULL) {
        fclose(fin);
        RETURN_LONG(-10);
    }
    for (i = 0; i < 4; i++) {
        fgets(buf2, sizeof(buf2), fin);
		if ((i==0) && (strncmp(buf2,"·¢ÐÅÈË",6)!=0)) {
			break;
		}
        fprintf(fout, "%s", buf2);
    }
    fprintf(fout, "%s", unix_string(content));
#ifndef RAW_ARTICLE
    fprintf(fout, "[36m¡ù ÐÞ¸Ä:¡¤%s ì¶ %s ÐÞ¸Ä±¾ÎÄ¡¤[FROM: %s][m\n", currentuser->userid, wwwCTime(time(0)) + 4, fromhost);
    while (fgets(buf2, sizeof(buf2), fin) != NULL) {
        if (Origin2(buf2)) {
            fprintf(fout, "%s", buf2);
            break;
        }
    }
#endif
    fclose(fin);
    fclose(fout);
#ifdef FILTER
    if (check_badword(outfile) !=0) {
		unlink(outfile);
        RETURN_LONG(-1); //ÐÞ¸ÄÎÄÕÂÊ§°Ü£¬ÎÄÕÂ¿ÉÄÜº¬ÓÐ²»Ç¡µ±ÄÚÈÝ.
    }
    else {
#endif
        if (!sigsetjmp(bus_jump, 1)) {
            signal(SIGBUS, sigbus);
            signal(SIGSEGV, sigbus);
       		f_mv(outfile, infile);
        } else {
			RETURN_LONG(-1);
		}
        signal(SIGBUS, SIG_IGN);
        signal(SIGSEGV, SIG_IGN);
#ifdef FILTER
    }
#endif
    RETURN_LONG(0);
}


static PHP_FUNCTION(bbs_wwwlogoff)
{
    if (getcurrentuser()) {
        int ret = (www_user_logoff(getcurrentuser(), getcurrentuser_num(),
                                   getcurrentuinfo(), getcurrentuinfo_num()));

        RETURN_LONG(ret);
    } else
        RETURN_LONG(-1);
}

#ifdef HAVE_BRC_CONTROL
static PHP_FUNCTION(bbs_brcaddread)
{
	char *board;
	int blen;
    long fid;
	bcache_t* bp;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (zend_parse_parameters(2 TSRMLS_CC, "sl", &board, &blen, &fid) != SUCCESS)
        WRONG_PARAM_COUNT;
	if ((bp=getbcache(board))==0){
		RETURN_NULL();
	}
	brc_initial(currentuser->userid, bp->filename);
	brc_add_read(fid);
	brc_update(currentuser->userid);
    /*brc_addreaddirectly(getcurrentuser()->userid, boardnum, fid);*/

    RETURN_NULL();
}
#endif

static PHP_FUNCTION(bbs_ann_traverse_check)
{
    char *path;
    int path_len;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (zend_parse_parameters(1 TSRMLS_CC, "s", &path, &path_len) != SUCCESS)
        WRONG_PARAM_COUNT;
    RETURN_LONG(ann_traverse_check(path, getcurrentuser()));
}

static PHP_FUNCTION(bbs_ann_get_board)
{
    char *path, *board;
    int path_len, board_len;

    if (zend_parse_parameters(1 TSRMLS_CC, "ss", &path, &path_len, &board, &board_len) != SUCCESS)
        WRONG_PARAM_COUNT;
    RETURN_LONG(ann_get_board(path, board, board_len));
}

/**
 * Fetching the announce path from a board name.
 * prototype:
 * string bbs_getannpath(char *board);
 *
 * @return a string of the announce path on success,
 *         FALSE on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_getannpath)
{
    char *board;
    int board_len;
    char buf[256];
    int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments 
     */
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &board, &board_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (ann_get_path(board, buf, sizeof(buf)) < 0) {
        RETURN_FALSE;
    }
    RETURN_STRING(buf, 1);
}

/**
 * get the number of one user's mail.
 * prototype:
 * bool bbs_getmailnum(string userid,long &total,long &unread);
 *
 * @return TRUE on success,
 *       FALSE on failure.
 *       and return total and unread in argument
 * @author KCN
 */
static PHP_FUNCTION(bbs_getmailnum)
{
    zval *total, *unread;
    char *userid;
    int userid_len;
    struct fileheader x;
    char path[80];
    int totalcount = 0, unreadcount = 0;
    int ac = ZEND_NUM_ARGS();
    int fd;

    if (ac != 3 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "szz", &userid, &userid_len, &total, &unread) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (userid_len > IDLEN)
        WRONG_PARAM_COUNT;

    /*
     * check for parameter being passed by reference 
     */
    if (!PZVAL_IS_REF(total) || !PZVAL_IS_REF(unread)) {
        zend_error(E_WARNING, "Parameter wasn't passed by reference");
        RETURN_FALSE;
    }

    setmailfile(path, userid, DOT_DIR);
    fd = open(path, O_RDONLY);
    if (fd == -1)
        RETURN_FALSE;
    while (read(fd, &x, sizeof(x)) > 0) {
        totalcount++;
        if (!(x.accessed[0] & FILE_READ))
            unreadcount++;
    }
    close(fd);
    /*
     * make changes to the parameter 
     */
    ZVAL_LONG(total, totalcount);
    ZVAL_LONG(unread, unreadcount);
    RETURN_TRUE;
}

/**
 * get the number of one user's mail path.
 * prototype:
 * int bbs_getmailnum2(string path);
 *
 * @return the number
 * @author binxun
 */
static PHP_FUNCTION(bbs_getmailnum2)
{
    char *path;
    int path_len;

    int ac = ZEND_NUM_ARGS();

    if (ac != 1 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "s", &path, &path_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    RETURN_LONG(getmailnum(path));
}

/**
 * Get mail used space
 * @author stiger
 */
static PHP_FUNCTION(bbs_getmailusedspace)
{
	RETURN_LONG(get_mailusedspace(currentuser,1)/1024);
}

/**
 * Fetch a list of mails in one user's mail path file into an array.
 * prototype:
 * array bbs_getmails(char *filename,int start,int num);
 *
 * start - 0 based
 * @return array of loaded mails on success,
 *         -1  no mail
 *         FALSE on failure.
 * @author binxun
 */
static PHP_FUNCTION(bbs_getmails)
{
    char *mailpath;
    int mailpath_len;
    int total, rows, i,j ;
	long start,num;

    struct fileheader *mails;
    zval *element;
    char flags[2];              /* flags[0]: status
                                 * flags[1]: reply status
                                 */
    int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments
     */
    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sll", &mailpath, &mailpath_len,&start,&num) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    total = getmailnum(mailpath);
    if (!total)
        RETURN_LONG(-1);

	if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }

    if(start >= total)RETURN_FALSE;
	if(start + num > total)num = total - start;

	mails = emalloc(num * sizeof(struct fileheader));
	if (!mails)
		RETURN_FALSE;
	rows = get_records(mailpath, mails, sizeof(struct fileheader), start+1, num);//it is 1 -based
	if (rows == -1)
		RETURN_FALSE;
	for (i = 0; i < rows; i++) {
		MAKE_STD_ZVAL(element);
		array_init(element);
		if (mails[i].accessed[0] & FILE_READ) {
			if (mails[i].accessed[0] & FILE_MARKED)
				flags[0] = 'm';
			else
				flags[0] = ' ';
		} else {
			if (mails[i].accessed[0] & FILE_MARKED)
				flags[0] = 'M';
			else
				flags[0] = 'N';
		}
		if (mails[i].accessed[0] & FILE_REPLIED) {
			if (mails[i].accessed[0] & FILE_FORWARDED)
				flags[1] = 'A';
			else
				flags[1] = 'R';
		} else {
			if (mails[i].accessed[0] & FILE_FORWARDED)
				flags[1] = 'F';
			else
				flags[1] = ' ';
		}
		bbs_make_article_array(element, mails + i, flags, sizeof(flags));
		zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
	}
    efree(mails);
}

/**
 * load mail list. user custom mailboxs.
 * prototype:
 * array bbs_loadmaillist(char *userid);
 *
 * @return array of loaded mails on success,
 *         -1 no mailbox
 *         FALSE on failure.
 * @author binxun
 */
static PHP_FUNCTION(bbs_loadmaillist)
{
    char *userid;
    int userid_len;
    char buf[10];
    struct _mail_list maillist;

    struct userec *user;
    int i;
    zval *element;

    int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments 
     */
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (userid_len > IDLEN)
        RETURN_FALSE;

    if (!getuser(userid, &user))
        RETURN_FALSE;
    load_mail_list(user, &maillist);

    if (maillist.mail_list_t < 0 || maillist.mail_list_t > MAILBOARDNUM)        //no custom mail box
    {
        RETURN_FALSE;
    }

    if (!maillist.mail_list_t)
        RETURN_LONG(-1);

    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }

    for (i = 0; i < maillist.mail_list_t; i++) {
        MAKE_STD_ZVAL(element);
        array_init(element);
        sprintf(buf, ".%s", maillist.mail_list[i] + 30);
        //assign_maillist(element,maillist.mail_list[i],buf);
        add_assoc_string(element, "boxname", maillist.mail_list[i], 1);
        add_assoc_string(element, "pathname", buf, 1);
        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);

    }
}

/**
 * change mail list and save new for user custom mailboxs.
 * prototype:
 * int bbs_changemaillist(bool bAdd,char* userid,char* newboxname,int index); index--0 based
 *
 * @return
 *         0 ---- fail
 *         -1 ---- success
 *         >0 --- reach to max number!
 * @author binxun
 */
static PHP_FUNCTION(bbs_changemaillist)
{
    char *boxname;
    int boxname_len;
    char *userid;
    int userid_len;
    zend_bool bAdd;
    int index;

    struct _mail_list maillist;
    char buf[10], path[PATHLEN];

    struct userec *user;
    int i;
    struct stat st;
    zval *element;

    int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments
     */
    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "bssl", &bAdd, &userid, &userid_len, &boxname, &boxname_len, &index) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (userid_len > IDLEN)
        RETURN_LONG(0);
    if (boxname_len > 29)
        boxname[29] = '\0';

    if (!getuser(userid, &user))
        RETURN_LONG(0);
    load_mail_list(user, &maillist);

    if (maillist.mail_list_t < 0 || maillist.mail_list_t > MAILBOARDNUM)        //no custom mail box
    {
        RETURN_LONG(0);
    }

    if (bAdd)                   //add
    {
        if (maillist.mail_list_t == MAILBOARDNUM)
            RETURN_LONG(MAILBOARDNUM);  //×î´óÖµÁË
        i = 0;
        while (1)               //search for new mailbox path name
        {
            i++;
            sprintf(buf, ".MAILBOX%d", i);
            setmailfile(path, currentuser->userid, buf);
            if (stat(path, &st) == -1)
                break;
        }
        sprintf(buf, "MAILBOX%d", i);
        strcpy(maillist.mail_list[maillist.mail_list_t], boxname);
        strcpy(maillist.mail_list[maillist.mail_list_t] + 30, buf);
        maillist.mail_list_t += 1;
        save_mail_list(&maillist);
    } else                      //delete
    {
        if (index < 0 || index > maillist.mail_list_t - 1)
            RETURN_LONG(-1);
        maillist.mail_list_t -= 1;
        if (index != maillist.mail_list_t - 1)  //it is not the last one
        {
            strncpy(maillist.mail_list[index], maillist.mail_list[index + 1], 30);
            strncpy(maillist.mail_list[index] + 30, maillist.mail_list[index + 1] + 30, 10);
        }
        save_mail_list(&maillist);
    }
    RETURN_LONG(-1);
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
static PHP_FUNCTION(bbs_getwebmsg)
{
    zval *retsrcid, *msgbuf, *srcutmpent, *z_sndtime;
    int ac = ZEND_NUM_ARGS();
    int srcutmp;
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

    if (receive_webmsg(currentuinfonum, currentuser->userid, &srcutmp, srcid, &sndtime, buf) == 0) {
        ZVAL_STRING(retsrcid, srcid, 1);
        ZVAL_STRING(msgbuf, buf, 1);
        ZVAL_LONG(srcutmpent, srcutmp);
        ZVAL_LONG(z_sndtime, sndtime);
        RETURN_TRUE;
    }
    /*
     * make changes to the parameter 
     */
    RETURN_FALSE;
}

extern char msgerr[255];

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
static PHP_FUNCTION(bbs_sendwebmsg)
{
    char *destid;
    int destid_len;
    char *msg;
    int msg_len;
    int destutmp;
    zval *z_errmsg;
    int result;
    int i;
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
    if (!msg_can_sendmsg(destid, destutmp)) {
        ZVAL_STRING(z_errmsg, "ÎÞ·¨·¢ËÍÑ¶Ï¢", 1);
        RETURN_FALSE;
    }
    if (!strcasecmp(destid, currentuser->userid)) {
        ZVAL_STRING(z_errmsg, "Äã²»ÄÜ¸ø×Ô¼º·¢Ñ¶Ï¢", 1);
        RETURN_FALSE;
    }
    if ((result = send_msg(currentuser->userid, get_utmpent_num(u_info), destid, destutmp, msg)) == 1) {
        ZVAL_STRING(z_errmsg, "ÒÑ¾­°ïÄãËÍ³öÑ¶Ï¢", 1);
        RETURN_TRUE;
    } else if (result == -1) {
        char buf[STRLEN];

        snprintf(buf, sizeof(buf), "·¢ËÍÑ¶Ï¢Ê§°Ü£¬%s", msgerr);
        ZVAL_STRING(z_errmsg, buf, 1);
        RETURN_FALSE;
    } else {
        ZVAL_STRING(z_errmsg, "·¢ËÍÑ¶Ï¢Ê§°Ü£¬´ËÈËÄ¿Ç°²»ÔÚÏß»òÕßÎÞ·¨½ÓÊÕÑ¶Ï¢", 1);
        RETURN_FALSE;
    }
}

/**
 * get the user dir or file.
 * prototype:
 * string bbs_sethomefile(string userid[,string filename])
 *
 * @return TRUE on success,
 *       FALSE on failure.
 *       and return total and unread in argument
 * @author KCN
 */
static PHP_FUNCTION(bbs_sethomefile)
{
    char *userid, *file;
    int userid_len, file_len = 0;
    char buf[60];
    int ac = ZEND_NUM_ARGS();

    if (ac == 2) {
        if (zend_parse_parameters(2 TSRMLS_CC, "ss", &userid, &userid_len, &file, &file_len) != SUCCESS)
            WRONG_PARAM_COUNT;
    } else if (ac == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) != SUCCESS)
            WRONG_PARAM_COUNT;
    } else
        WRONG_PARAM_COUNT;
    if (file_len != 0)
        sethomefile(buf, userid, file);
    else
        sethomepath(buf, userid);
    RETURN_STRING(buf, 1);
}

/**
 * get the user mail dir or file.
 * prototype:
 * string bbs_setmailfile(string userid[,string filename])
 *
 * @return path string
 * @author binxun
 */
static PHP_FUNCTION(bbs_setmailfile)
{
    char *userid, *file;
    int userid_len, file_len = 0;
    char buf[60];
    int ac = ZEND_NUM_ARGS();

    if (ac == 2) {
        if (zend_parse_parameters(2 TSRMLS_CC, "ss", &userid, &userid_len, &file, &file_len) != SUCCESS)
            WRONG_PARAM_COUNT;
    } else if (ac == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) != SUCCESS)
            WRONG_PARAM_COUNT;
    } else
        WRONG_PARAM_COUNT;
    if (file_len != 0)
        setmailfile(buf, userid, file);
    else
        setmailpath(buf, userid);
    RETURN_STRING(buf, 1);
}


/**
 * mail a file from a user to another user.
 * prototype:
 * string bbs_mail_file(string srcid, string filename, string destid,
 *                        string title, int is_move)
 *
 * @return TRUE on success,
 *       FALSE on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_mail_file)
{
    char *srcid;
    int srcid_len;
    char *filename;
    int filename_len;
    char *destid;
    int destid_len;
    char *title;
    int title_len;
    int is_move;
    int ac = ZEND_NUM_ARGS();

    if (ac != 5 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "ssssl", &srcid, &srcid_len, &filename, &filename_len, &destid, &destid_len, &title, &title_len, &is_move) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (mail_file(srcid, filename, destid, title, is_move, NULL) < 0)
        RETURN_FALSE;
    RETURN_TRUE;
}

/**
 * set currentuinfo for user.
 * prototype:
 * string bbs_update_uinfo(string field, value)
 * @return TRUE on success,
 *       FALSE on failure.
 * @author kcn
 */
static PHP_FUNCTION(bbs_update_uinfo)
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
        currentuinfo->invisible = Z_LVAL_P(value);
    }
    RETURN_LONG(0);
}

/**
 * set password for user.
 * prototype:
 * string bbs_setpassword(string userid, string password)
 * @if userid=="" then user=currentuser
 * @return TRUE on success,
 *       FALSE on failure.
 * @author kcn
 */
static PHP_FUNCTION(bbs_setpassword)
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
        user = currentuser;
    if (setpasswd(password, user) != 1)
        RETURN_FALSE;
    RETURN_TRUE;
}

static int ext_init = 0; /* Don't access this variable directly, 
						  use the following helper routines */

static int get_initialized()
{
	return ext_init;
}

static int initialize_ext()
{
    getcwd(old_pwd, 1023);
    old_pwd[1023] = 0;
    chdir(BBSHOME);
    resolve_ucache();
    resolve_utmp();
    resolve_boards();
    init_bbslog();
    www_data_init();
    chdir(old_pwd);
	ext_init = 1;

	return ext_init;
}

/**
 * return status flag of initialization of the extension.
 * prototype:
 * bool bbs_ext_initialized()
 *
 * @return TRUE if the extension had been initialized,
 *       FALSE otherwise.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_ext_initialized)
{
	if (!get_initialized())
        RETURN_FALSE;
    RETURN_TRUE;
}

/**
 * initialize the extension.
 * prototype:
 * bool bbs_init_ext()
 *
 * @return TRUE on success,
 *       FALSE on failure.
 * @author flyriver
 */
static PHP_FUNCTION(bbs_init_ext)
{
	int ret = 0;
	if (!get_initialized())
        ret = initialize_ext();
	if (ret)
	{
    	RETURN_TRUE;
	}
	else
	{
		RETURN_FALSE;
	}
}

PHP_MINIT_FUNCTION(smth_bbs)
{
    /*
    zval *bbs_home;
    zval *bbs_full_name;

    MAKE_STD_ZVAL(bbs_home);
    ZVAL_STRING(bbs_home, BBSHOME, 1);
    MAKE_STD_ZVAL(bbs_full_name);
    ZVAL_STRING(bbs_full_name, BBS_FULL_NAME, 1);
    PHP_SET_SYMBOL(&EG(symbol_table), "BBS_HOME", bbs_home);
    PHP_SET_SYMBOL(&EG(symbol_table), "BBS_FULL_NAME", bbs_full_name);
    */
    REGISTER_STRINGL_CONSTANT("BBS_HOME",BBSHOME,strlen(BBSHOME),CONST_CS | CONST_PERSISTENT);
    REGISTER_STRINGL_CONSTANT("BBS_FULL_NAME",BBS_FULL_NAME,strlen(BBS_FULL_NAME),CONST_CS | CONST_PERSISTENT);
#ifdef SQUID_ACCL
    REGISTER_LONG_CONSTANT("SQUID_ACCL", 1, CONST_CS | CONST_PERSISTENT);
#else
    REGISTER_LONG_CONSTANT("SQUID_ACCL", 0, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef BUILD_PHP_EXTENSION
    REGISTER_LONG_CONSTANT("BUILD_PHP_EXTENSION", 1, CONST_CS | CONST_PERSISTENT);
#else
    REGISTER_LONG_CONSTANT("BUILD_PHP_EXTENSION", 0, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef HAVE_WFORUM
    REGISTER_LONG_CONSTANT("BBS_WFORUM", 1, CONST_CS | CONST_PERSISTENT);
#else
    REGISTER_LONG_CONSTANT("BBS_WFORUM", 0, CONST_CS | CONST_PERSISTENT);
#endif
    REGISTER_LONG_CONSTANT("BBS_PERM_POSTMASK", PERM_POSTMASK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_NOZAP", PERM_NOZAP, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_BOARDS", PERM_BOARDS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_CLOAK", PERM_CLOAK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_BMAMANGER", PERM_BMAMANGER, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_ATTACH", BOARD_ATTACH, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_ANNONY", BOARD_ANNONY, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_JUNK", BOARD_JUNK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_OUTFLAG", BOARD_OUTFLAG, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_CLUB_READ", BOARD_CLUB_READ, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_CLUB_WRITE", BOARD_CLUB_WRITE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_CLUB_HIDE", BOARD_CLUB_HIDE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_GROUP", BOARD_GROUP, CONST_CS | CONST_PERSISTENT);
#ifdef DEBUG
    zend_error(E_WARNING, "module init");
#endif
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(smth_bbs)
{
	if (get_initialized())
	{
		www_data_detach();
		detach_utmp();
		detach_boards();
		detach_ucache();
		detach_publicshm();
	}

#ifdef DEBUG
    zend_error(E_WARNING, "module shutdown");
#endif
    return SUCCESS;
}

PHP_RINIT_FUNCTION(smth_bbs)
{
    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    currentuser = NULL;
    currentusernum = 0;
#ifdef DEBUG
    zend_error(E_WARNING, "request init:%d %x", getpid(), getcurrentuinfo);
#endif
#ifdef HAVE_WFORUM
	zapbuf=NULL;
#endif
	output_buffer=NULL;
	output_buffer_size=0;
	output_buffer_len=0;
#ifdef SMS_SUPPORT
	smsbuf=NULL;
	smsresult=0;
#endif
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(smth_bbs)
{
#ifdef DEBUG
    zend_error(E_WARNING, "request shutdown");
#endif
    chdir(old_pwd);
    currentuser = NULL;
    return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION
 *  */
PHP_MINFO_FUNCTION(smth_bbs)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "smth_bbs support", "enabled");
    php_info_print_table_end();
}
/**
 * Function: post a new mail
 *  rototype:
 * int bbs_postmail(string targetid,string title,string content,long sig, long backup);
 *
 *  @return the result
 *  	0 -- success
 *		<0 error
 *  @author roy
 */
static PHP_FUNCTION(bbs_postmail){
	char* targetID, *title, *content;
	int  idLen, tLen,cLen, backup,sig;
	int ac = ZEND_NUM_ARGS();
	char filename[STRLEN+1];
	char title2[80],title3[80];
	int ret;
	struct userec *u = NULL;
	int i;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (ac != 5 || zend_parse_parameters(5 TSRMLS_CC, "ss/s/ll", &targetID, &idLen,&title,&tLen,&content,&cLen,&sig,&backup) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}
	getuser(targetID, &u);
	if (u==NULL) 
		RETURN_LONG(-100);//can't find user
    for (i = 0; i < strlen(title); i++)
        if (title[i] < 27 && title[i] >= -1)
            title[i] = ' ';
	if (title[0] == 0)
        strcpy(title3,"Ã»Ö÷Ìâ");
	else 
		strncpy(title3,title,79);
	title3[79]=0;
	snprintf(filename, STRLEN, "tmp/%s.%d.tmp", targetID, getpid());
	filename[STRLEN]=0;
    if (f_append(filename, unix_string(content)) < 0)
        RETURN_LONG(-1); //"ÎÞ·¨´´½¨ÁÙÊ±ÎÄ¼þ";
    snprintf(title2,79, "{%s} %s", targetID, title);
    title2[79] = 0;
    
    if ((ret=post_mail(targetID, title3, filename, currentuser->userid, currentuser->username, fromhost, sig))!=0)
    {
		RETURN_LONG(ret-1);
		/*
        case -1:
        	http_fatal("·¢ÐÅÊ§°Ü:ÎÞ·¨´´½¨ÎÄ¼þ");
        case -2:
        	http_fatal("·¢ÐÅÊ§°Ü:¶Ô·½¾ÜÊÕÄãµÄÓÊ¼þ");
        case -3:
        	http_fatal("·¢ÐÅÊ§°Ü:¶Ô·½ÐÅÏäÂú");
        default:
        	http_fatal("·¢ÐÅÊ§°Ü");
        }
		*/
    }
    if (backup)
        post_mail(currentuser->userid, title2, filename, currentuser->userid, currentuser->username, fromhost, sig);
    unlink(filename);
	RETURN_LONG(6);
}

/**
 * Function: Create a new user id
 *  rototype:
 * int bbs_createNewID(string smthid,string passwd,string nickname);
 *
 *  @return the result
 *  	0 -- success, 1 -- specail char or first char not alpha
 *  	2 -- at least two chars 3 -- system name or bad name
 *  	4 -- have been used 5 -- length > IDLEN
 *  	6 -- passwd is too long > 39
 *  .	7 -- IP is baned
 *  	10 -- system error
 *  @author binxun
 */
static PHP_FUNCTION(bbs_createnewid)
{
	char* userid;
	int userid_len;
	char* passbuf;
	int passbuf_len;
	char* nickname;
	int nickname_len;
	char buf[1024];
	char tmpstr[30];
	struct stat lst;
	time_t lnow;
	struct userec newuser;
	int allocid;

    int ac = ZEND_NUM_ARGS();

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sss", &userid, &userid_len,&passbuf,&passbuf_len,&nickname,&nickname_len) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}
	if (userid_len > IDLEN)RETURN_LONG(5);
	if (passbuf_len > 39)RETURN_LONG(6);

	if (id_invalid(userid) == 1) RETURN_LONG(1);
	if (strlen(userid) < 2) RETURN_LONG(2);
	if (bad_user_id(userid)) RETURN_LONG(3);
	if (searchuser(userid)) RETURN_LONG(4);

	//if(check_ban_IP(fromhost,buf) < 0)RETURN_LONG(7);

	lnow = time(NULL);
	sethomepath(buf,userid);
	//´æÔÚÇ°ÈËµÄÄ¿Â¼,²¢ÇÒ»¹ÔÚ±£´æÆÚÏÞÒÔÄÚ
	if (!stat(buf,&lst) && S_ISDIR(lst.st_mode) && (lnow-lst.st_ctime < SEC_DELETED_OLDHOME ))
	{
		//log?

		RETURN_LONG(10);
	}

	memset(&newuser,0,sizeof(newuser));
	strncpy(newuser.userid ,userid,IDLEN);
	strncpy(newuser.username,nickname,NAMELEN-1);
	newuser.username[NAMELEN-1] = '\0';

	newuser.firstlogin = newuser.lastlogin = time(NULL);

	setpasswd(passbuf,&newuser);

	newuser.userlevel = PERM_AUTOSET;
	newuser.userdefine = -1;
	newuser.userdefine &= ~DEF_NOTMSGFRIEND;
#ifdef HAVE_WFORUM
	newuser.userdefine &= ~DEF_SHOWREALUSERDATA;
#endif
	newuser.notemode = -1;
	newuser.exittime = time(NULL) - 100;
	newuser.flags = CURSOR_FLAG;
	newuser.flags |= PAGER_FLAG;
	newuser.title = 0;

	//·ÖÅäIDºÅ
	allocid = getnewuserid2(newuser.userid);
	if (allocid > MAXUSERS || allocid <= 0) RETURN_LONG(10);

	//¸üÐÂ¹²ÏíÄÚ´æÊý¾Ý
	update_user(&newuser,allocid,1);

	if (!getuser(newuser.userid,&currentuser))RETURN_LONG(10);

	newbbslog("user","%s","new account from www");

	//¼ì²éÊÇ·ñÓÐÇ°ÈËµÄÐÅ¼þ
	sethomepath(tmpstr,userid);
	sprintf(buf,"/bin/mv -f %s " BBSHOME "/homeback/%s",tmpstr,userid);
	system(buf);
	setmailpath(tmpstr,userid);
	sprintf(buf,"/bin/mv -f %s " BBSHOME "/mailback/%s",tmpstr,userid);
	system(buf);

	//´´½¨ÐÂÄ¿Â¼
	sethomepath(tmpstr,userid);
	if(mkdir(tmpstr,0755) < 0) {
	    bbslog("3error","create id %s home dir error:%s",userid,strerror(errno));
	    RETURN_LONG(10);
	}

	RETURN_LONG(0);
}
/**
 * fill infomation of ID ,name, NO. dept, for tsinghua
 * prototype:
 * int bbs_fillIDInfo(string smthid,string name,string number,string dept);
 *
 *  @return the result
 *  	0 -- success, -1 -- Invalid parameter
 *  	-2 -- error
 *  @author binxun
 */
static PHP_FUNCTION(bbs_fillidinfo)
{
    char* userid;
    int userid_len;
    char* realname;
    int realname_len;
    char* number;
    int number_len;
    char* dept;
    int dept_len;
    char genbuf[STRLEN];

    struct userdata ud;

    int ac = ZEND_NUM_ARGS();


    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "ssss", &userid, &userid_len,&realname,&realname_len,&number,&number_len,&dept,&dept_len) == FAILURE)
    {
            WRONG_PARAM_COUNT;
    }

    if(userid_len > IDLEN || realname_len > NAMELEN || dept_len > STRLEN)
       RETURN_LONG(-1);

    memset(&ud,0,sizeof(ud));
	if( read_user_memo(userid, &currentmemo) <= 0) RETURN_LONG(-2);

    if(read_userdata(userid,&ud) < 0)RETURN_LONG(-2);

    strncpy(ud.realname, realname, NAMELEN);
    strncpy(ud.address,dept,STRLEN);
    sprintf(genbuf,"%s#%s#%s#TH",realname,number,dept);
    if(strlen(genbuf) >= STRLEN - 16) //too long
		sprintf(genbuf,"%s#%s#TH",realname,number);//must < STRLEN - 16
    strncpy(ud.realemail,genbuf,STRLEN-16);

	memcpy(&(currentmemo->ud), &ud, sizeof(ud));
	end_mmapfile(currentmemo, sizeof(struct usermemo), -1);

    if(write_userdata(userid,&ud) < 0)RETURN_LONG(-2);

	bbslog("user","%s","new account from tsinghua www");

    RETURN_LONG(0);
}

#ifdef HAVE_WFORUM

static PHP_FUNCTION(bbs_saveuserdata)
{
    char*   userid,
	    *   realname,
        *   photo_url,
        *   address,
		*	email,
		*	phone,
		*   mobile_phone,
		* OICQ, 
		* ICQ, 
		* MSN, 
		* homepage,
		* userface_url,
		* country,
		* province,
		* city,
		* graduate_school;
    int     userid_len,
	        realname_len,
	        photo_url_len,
			address_len,
			email_len,
			phone_len,
			mobile_phone_len,
			OICQ_len,
			ICQ_len,
			MSN_len,
			homepage_len,
			userface_url_len,
			country_len,
			province_len,
			city_len,
			graduate_school_len,
			gender,
	        year,
	        month,
			day,
			userface_img,
			userface_width,
			userface_height, 
			group,
			shengxiao,
			bloodtype,
			religion ,
			profession, 
			married, 
			education,
			character;
    zend_bool   bAuto;
	struct  userdata ud;
	struct  userec* uc;
	FILE*   fn;
	char    genbuf[STRLEN+1];
	char*   ptr;
	int     usernum;
	long    now;

    int ac = ZEND_NUM_ARGS();


	if (ac != 32 || zend_parse_parameters(32 TSRMLS_CC, "sssllllssssssslslllsssllllllslsb", &userid,&userid_len,&realname,&realname_len,
	    &address,&address_len,&gender,&year,&month,&day,&email,&email_len,&phone,&phone_len,&mobile_phone,&mobile_phone_len,
		&OICQ, &OICQ_len, &ICQ, &ICQ_len, &MSN, &MSN_len, &homepage, &homepage_len, &userface_img,
		&userface_url, &userface_url_len, &userface_width, &userface_height, &group, &country, &country_len,
		&province, &province_len, &city, &city_len, &shengxiao, &bloodtype, &religion, &profession,
		&married, &education, &graduate_school, &graduate_school_len, &character, &photo_url, &photo_url_len,&bAuto) == FAILURE)
    {
		WRONG_PARAM_COUNT;
	}

	if(userid_len > IDLEN)RETURN_LONG(2);

    usernum = getusernum(userid);
	if(0 == usernum)RETURN_LONG(3);

	if ( (userface_width<0) || (userface_width>120) ){
		RETURN_LONG(-1);
	}
	if ( (userface_height<0) || (userface_height>120) ){
		RETURN_LONG(-2);
	}
	if (userface_url_len!=0) {
		userface_img=-1;
	}

	if( read_user_memo(userid, &currentmemo) <= 0) RETURN_LONG(-2);
	read_userdata(userid, &ud);
    strncpy(ud.realname, realname, NAMELEN);
    strncpy(ud.address, address, STRLEN);
	strncpy(ud.reg_email,email,STRLEN);
	strncpy(ud.OICQ,OICQ,STRLEN);
	strncpy(ud.ICQ,ICQ,STRLEN);
	strncpy(ud.MSN,MSN,STRLEN);
	strncpy(ud.homepage,homepage,STRLEN);
	strncpy(ud.userface_url,userface_url,STRLEN);
	strncpy(ud.country,country,STRLEN);
	strncpy(ud.province,province,STRLEN);
	strncpy(ud.city,city,STRLEN);
	strncpy(ud.graduateschool,graduate_school,STRLEN);
	strncpy(ud.telephone,phone,STRLEN);
	strncpy(ud.photo_url,photo_url,STRLEN);
	ud.photo_url[STRLEN-1]=0;
	ud.telephone[STRLEN-1]=0;
	ud.OICQ[STRLEN-1]=0;
	ud.ICQ[STRLEN-1]=0;
	ud.MSN[STRLEN-1]=0;
	ud.homepage[STRLEN-1]=0;
	ud.userface_url[STRLEN-1]=0;
	ud.country[STRLEN-1]=0;
	ud.province[STRLEN-1]=0;
	ud.city[STRLEN-1]=0;
	ud.graduateschool[STRLEN-1]=0;
    ud.realname[NAMELEN-1] = '\0';
	ud.address[STRLEN-1] = '\0';
	ud.reg_email[STRLEN-1] = '\0';

    
#ifdef HAVE_BIRTHDAY
    ud.birthyear=(year > 1900 && year < 2050)?(year-1900):0;
	ud.birthmonth=(month >=1 && month <=12)?month:0;
	ud.birthday=(day>=1 && day <=31)?day:0;
	if(gender==1)ud.gender='M';
	else
	    ud.gender='F';
#endif
	ud.userface_img=userface_img;
	ud.userface_width=userface_width;
	ud.userface_height=userface_height;
	ud.group=group;
	ud.shengxiao=shengxiao;
	ud.bloodtype=bloodtype;
	ud.religion=religion;
	ud.profession=profession;
	ud.married=married;
	ud.education=education;
	ud.character=character;
	memcpy(&(currentmemo->ud), &ud, sizeof(ud));
	end_mmapfile(currentmemo, sizeof(struct usermemo), -1);
	write_userdata(userid, &ud);
    RETURN_LONG(0);

}

#endif

#ifdef HAVE_WFORUM
/**
 * Function: Create a registry form
 *  rototype:
 * int bbs_createregform(string userid ,string realname,string dept,string address,int gender,int year,int month,int day,
    string email,string phone,string mobile_phone,string OICQ, string ICQ, string MSN, string homepage, int userface_img,
	string userface_url,int userface_width, int userface_height, int group, string country ,string province, string city,
	int shengxiao, int bloodtype, int religion , int profession, int married, int education, string graduate_school,
	int character,	bool bAuto)
 *
 *  bAuto : true -- ×Ô¶¯Éú³É×¢²áµ¥,false -- ÊÖ¹¤.
 *  @return the result
 *  	0 -- success,
 *      1 -- ×¢²áµ¥ÉÐÎ´´¦Àí
 *      2 -- ²ÎÊý´íÎó
 *      3 -- ÓÃ»§²»´æÔÚ
 *      4 -- ÓÃ»§ÒÑ¾­Í¨¹ý×¢²á  5 -- ²»µ½Ê±¼ä
 *  	10 -- system error
 *  @author binxun 2003.5
 */
static PHP_FUNCTION(bbs_createregform)
{
    char*   userid,
	    *   realname,
        *   dept,
        *   address,
		*	email,
		*	phone,
		*   mobile_phone,
		* OICQ, 
		* ICQ, 
		* MSN, 
		* homepage,
		* userface_url,
		* country,
		* province,
		* city,
		* graduate_school;
    int     userid_len,
	        realname_len,
	        dept_len,
			address_len,
			email_len,
			phone_len,
			mobile_phone_len,
			OICQ_len,
			ICQ_len,
			MSN_len,
			homepage_len,
			userface_url_len,
			country_len,
			province_len,
			city_len,
			graduate_school_len,
			gender,
	        year,
	        month,
			day,
			userface_img,
			userface_width,
			userface_height, 
			group,
			shengxiao,
			bloodtype,
			religion ,
			profession, 
			married, 
			education,
			character;
    zend_bool   bAuto;
	struct  userdata ud;
	struct  userec* uc;
	FILE*   fn;
	char    genbuf[STRLEN+1];
	char*   ptr;
	int     usernum;
	long    now;

    int ac = ZEND_NUM_ARGS();


	if (ac != 32 || zend_parse_parameters(32 TSRMLS_CC, "ssssllllssssssslslllsssllllllslb", &userid,&userid_len,&realname,&realname_len,&dept,&dept_len,
	    &address,&address_len,&gender,&year,&month,&day,&email,&email_len,&phone,&phone_len,&mobile_phone,&mobile_phone_len,
		&OICQ, &OICQ_len, &ICQ, &ICQ_len, &MSN, &MSN_len, &homepage, &homepage_len, &userface_img,
		&userface_url, &userface_url_len, &userface_width, &userface_height, &group, &country, &country_len,
		&province, &province_len, &city, &city_len, &shengxiao, &bloodtype, &religion, &profession,
		&married, &education, &graduate_school, &graduate_school_len, &character,&bAuto) == FAILURE)
    {
		WRONG_PARAM_COUNT;
	}

	if(userid_len > IDLEN)RETURN_LONG(2);

    usernum = getusernum(userid);
	if(0 == usernum)RETURN_LONG(3);

	if ( (userface_width<0) || (userface_width>120) ){
		RETURN_LONG(-1);
	}
	if ( (userface_height<0) || (userface_height>120) ){
		RETURN_LONG(-2);
	}
	if (userface_url_len!=0) {
		userface_img=-1;
	}

	if(!bAuto)
	{
        //¼ì²éÓÃ»§ÊÇ·ñÒÑ¾­Í¨¹ý×¢²á»òÕß»¹²»µ½Ê±¼ä(ÏÈ·Åµ½ÕâÀï,×îºÃ·Åµ½phpÀïÃæ)
	    if(getuser(userid,&uc) == 0)RETURN_LONG(3);
		if(HAS_PERM(uc,PERM_LOGINOK))RETURN_LONG(4);
		/* remed by roy 2003.7.17 
		if(time(NULL) - uc->firstlogin < REGISTER_WAIT_TIME)RETURN_LONG(5);
		*/
	    //¼ì²éÊÇ·ñµ¥×ÓÒÑ¾­Ìî¹ýÁË
		if ((fn = fopen("new_register", "r")) != NULL) {
			while (fgets(genbuf, STRLEN, fn) != NULL) {
				if ((ptr = strchr(genbuf, '\n')) != NULL)
					*ptr = '\0';
				if (strncmp(genbuf, "userid: ", 8) == 0 && strcmp(genbuf + 8, userid) == 0) {
					fclose(fn);
					RETURN_LONG(1);
				}
			}
			fclose(fn);
		}
    }
	if( read_user_memo(userid, &currentmemo) <= 0) RETURN_LONG(-2);
	read_userdata(userid, &ud);
    strncpy(ud.realname, realname, NAMELEN);
    strncpy(ud.address, address, STRLEN);
	strncpy(ud.reg_email,email,STRLEN);
	strncpy(ud.OICQ,OICQ,STRLEN);
	strncpy(ud.ICQ,ICQ,STRLEN);
	strncpy(ud.MSN,MSN,STRLEN);
	strncpy(ud.homepage,homepage,STRLEN);
	strncpy(ud.userface_url,userface_url,STRLEN);
	strncpy(ud.country,country,STRLEN);
	strncpy(ud.province,province,STRLEN);
	strncpy(ud.city,city,STRLEN);
	strncpy(ud.graduateschool,graduate_school,STRLEN);
	strncpy(ud.telephone,phone,STRLEN);
	ud.telephone[STRLEN-1]=0;
	ud.OICQ[STRLEN-1]=0;
	ud.ICQ[STRLEN-1]=0;
	ud.MSN[STRLEN-1]=0;
	ud.homepage[STRLEN-1]=0;
	ud.userface_url[STRLEN-1]=0;
	ud.country[STRLEN-1]=0;
	ud.province[STRLEN-1]=0;
	ud.city[STRLEN-1]=0;
	ud.graduateschool[STRLEN-1]=0;
    ud.realname[NAMELEN-1] = '\0';
	ud.address[STRLEN-1] = '\0';
	ud.reg_email[STRLEN-1] = '\0';

    if(strcmp(mobile_phone,"")){
	    ud.mobileregistered = false;
		strncpy(ud.mobilenumber,mobile_phone,MOBILE_NUMBER_LEN);
		ud.mobilenumber[MOBILE_NUMBER_LEN-1] = '\0';
	}
    else{
    	ud.mobileregistered = false;
    	}
    
#ifdef HAVE_BIRTHDAY
    ud.birthyear=(year > 1900 && year < 2050)?(year-1900):0;
	ud.birthmonth=(month >=1 && month <=12)?month:0;
	ud.birthday=(day>=1 && day <=31)?day:0;
	if(gender==1)ud.gender='M';
	else
	    ud.gender='F';
#endif
	ud.userface_img=userface_img;
	ud.userface_width=userface_width;
	ud.userface_height=userface_height;
	ud.group=group;
	ud.shengxiao=shengxiao;
	ud.bloodtype=bloodtype;
	ud.religion=religion;
	ud.profession=profession;
	ud.married=married;
	ud.education=education;
	ud.character=character;
	memcpy(&(currentmemo->ud), &ud, sizeof(ud));
	end_mmapfile(currentmemo, sizeof(struct usermemo), -1);
	write_userdata(userid, &ud);

#ifdef NEW_COMERS
	{
	FILE *fout;
	char buf2[STRLEN],buf[STRLEN];
	sprintf(buf, "tmp/newcomer.%s",uc->userid);
		if ((fout = fopen(buf, "w")) != NULL)
		{
			fprintf(fout, "´ó¼ÒºÃ,\n\n");
			fprintf(fout, "ÎÒÊÇ %s (%s), À´×Ô %s\n", uc->userid,
					uc->username, fromhost);
			fprintf(fout, "½ñÌì%s³õÀ´´ËÕ¾±¨µ½, Çë´ó¼Ò¶à¶àÖ¸½Ì¡£\n",
#ifdef HAVE_BIRTHDAY
					(ud.gender == 'M') ? "Ð¡µÜ" : "Ð¡Å®×Ó");
#else
                                        "Ð¡µÜ");
#endif
			fprintf(fout, "\n\nÎÒÊÇwww×¢²áÓÃ»§~~~\n\n");
			fclose(fout);
			sprintf(buf2, "ÐÂÊÖÉÏÂ·: %s", uc->username);
			post_file(uc, "", buf, "newcomers", buf2, 0, 2);
			unlink(buf);
		}
	}
#endif

	sprintf(genbuf,"%d.%d.%d",year,month,day);
	if(bAuto)
        fn = fopen("pre_register", "a");
	else
	    fn = fopen("new_register", "a");

    if (fn) {
        now = time(NULL);
        flock(fileno(fn),LOCK_EX);
        fprintf(fn, "usernum: %d, %s", usernum, ctime(&now));
        fprintf(fn, "userid: %s\n", userid);
        fprintf(fn, "realname: %s\n", realname);
        fprintf(fn, "career: %s\n", dept);
        fprintf(fn, "addr: %s\n", address);
        fprintf(fn, "phone: %s\n", phone);
        fprintf(fn, "birth: %s\n", genbuf);
        fprintf(fn, "----\n");
        flock(fileno(fn),LOCK_UN);
        fclose(fn);
        RETURN_LONG(0);
    }
	else
        RETURN_LONG(10);
}

#else

/**
 * Function: Create a registry form
 *  rototype:
 * int bbs_createregform(string userid ,string realname,string dept,string address,int gender,int year,int month,int day,
    string email,string phone,string mobile_phone,bool bAuto)
 *
 *  bAuto : true -- ×Ô¶¯Éú³É×¢²áµ¥,false -- ÊÖ¹¤.
 *  @return the result
 *  	0 -- success,
 *      1 -- ×¢²áµ¥ÉÐÎ´´¦Àí
 *      2 -- ²ÎÊý´íÎó
 *      3 -- ÓÃ»§²»´æÔÚ
 *      4 -- ÓÃ»§ÒÑ¾­Í¨¹ý×¢²á  5 -- ²»µ½Ê±¼ä
 *  	10 -- system error
 *  @author binxun 2003.5
 */
static PHP_FUNCTION(bbs_createregform)
{
    char*   userid,
	    *   realname,
        *   dept,
        *   address,
		*	email,
		*	phone,
		*   mobile_phone;
    int     userid_len,
	        realname_len,
	        dept_len,
			address_len,
			email_len,
			phone_len,
			mobile_phone_len,
			gender,
	        year,
	        month,
			day;
    zend_bool   bAuto;
	struct  userdata ud;
	struct  userec* uc;
	FILE*   fn;
	char    genbuf[STRLEN+1];
	char*   ptr;
	int     usernum;
	long    now;

    int ac = ZEND_NUM_ARGS();

    if (ac != 12 || zend_parse_parameters(12 TSRMLS_CC, "ssssllllsssb", &userid,&userid_len,&realname,&realname_len,&dept,&dept_len,
	    &address,&address_len,&gender,&year,&month,&day,&email,&email_len,&phone,&phone_len,&mobile_phone,&mobile_phone_len,&bAuto) == FAILURE)
    {
		WRONG_PARAM_COUNT;
	}

	if(userid_len > IDLEN)RETURN_LONG(2);

    usernum = getusernum(userid);
	if(0 == usernum)RETURN_LONG(3);


	if(!bAuto)
	{
        //¼ì²éÓÃ»§ÊÇ·ñÒÑ¾­Í¨¹ý×¢²á»òÕß»¹²»µ½Ê±¼ä(ÏÈ·Åµ½ÕâÀï,×îºÃ·Åµ½phpÀïÃæ)
	    if(getuser(userid,&uc) == 0)RETURN_LONG(3);
		if(HAS_PERM(uc,PERM_LOGINOK))RETURN_LONG(4);
		/* remed by roy 2003.7.17 
		if(time(NULL) - uc->firstlogin < REGISTER_WAIT_TIME)RETURN_LONG(5);
		*/
	    //¼ì²éÊÇ·ñµ¥×ÓÒÑ¾­Ìî¹ýÁË
		if ((fn = fopen("new_register", "r")) != NULL) {
			while (fgets(genbuf, STRLEN, fn) != NULL) {
				if ((ptr = strchr(genbuf, '\n')) != NULL)
					*ptr = '\0';
				if (strncmp(genbuf, "userid: ", 8) == 0 && strcmp(genbuf + 8, userid) == 0) {
					fclose(fn);
					RETURN_LONG(1);
				}
			}
			fclose(fn);
		}
    }
	if( read_user_memo(userid, &currentmemo) <= 0) RETURN_LONG(-2);
	read_userdata(userid, &ud);
    strncpy(ud.realname, realname, NAMELEN);
    strncpy(ud.address, address, STRLEN);
	strncpy(ud.reg_email,email,STRLEN);
    ud.realname[NAMELEN-1] = '\0';
	ud.address[STRLEN-1] = '\0';
	ud.reg_email[STRLEN-1] = '\0';

    if(strcmp(mobile_phone,"")){
	    ud.mobileregistered = true;
		strncpy(ud.mobilenumber,mobile_phone,MOBILE_NUMBER_LEN);
		ud.mobilenumber[MOBILE_NUMBER_LEN-1] = '\0';
	}
    else{
    	ud.mobileregistered = false;
    	}
    
#ifdef HAVE_BIRTHDAY
    ud.birthyear=(year > 1900 && year < 2050)?(year-1900):0;
	ud.birthmonth=(month >=1 && month <=12)?month:0;
	ud.birthday=(day>=1 && day <=31)?day:0;
	if(gender==1)ud.gender='M';
	else
	    ud.gender='F';
#endif
	memcpy(&(currentmemo->ud), &ud, sizeof(ud));
	end_mmapfile(currentmemo, sizeof(struct usermemo), -1);
	write_userdata(userid, &ud);

#ifdef NEW_COMERS
	{
	FILE *fout;
	char buf2[STRLEN],buf[STRLEN];
	sprintf(buf, "tmp/newcomer.%s",uc->userid);
		if ((fout = fopen(buf, "w")) != NULL)
		{
			fprintf(fout, "´ó¼ÒºÃ,\n\n");
			fprintf(fout, "ÎÒÊÇ %s (%s), À´×Ô %s\n", uc->userid,
					uc->username, fromhost);
			fprintf(fout, "½ñÌì%s³õÀ´´ËÕ¾±¨µ½, Çë´ó¼Ò¶à¶àÖ¸½Ì¡£\n",
#ifdef HAVE_BIRTHDAY
					(ud.gender == 'M') ? "Ð¡µÜ" : "Ð¡Å®×Ó");
#else
                                        "Ð¡µÜ");
#endif
			fprintf(fout, "\n\nÎÒÊÇwww×¢²áÓÃ»§~~~\n\n");
			fclose(fout);
			sprintf(buf2, "ÐÂÊÖÉÏÂ·: %s", uc->username);
			post_file(uc, "", buf, "newcomers", buf2, 0, 2);
			unlink(buf);
		}
	}
#endif

	sprintf(genbuf,"%d.%d.%d",year,month,day);
	if(bAuto)
        fn = fopen("pre_register", "a");
	else
	    fn = fopen("new_register", "a");

    if (fn) {
        now = time(NULL);
        flock(fileno(fn),LOCK_EX);
        fprintf(fn, "usernum: %d, %s", usernum, ctime(&now));
        fprintf(fn, "userid: %s\n", userid);
        fprintf(fn, "realname: %s\n", realname);
        fprintf(fn, "career: %s\n", dept);
        fprintf(fn, "addr: %s\n", address);
        fprintf(fn, "phone: %s\n", phone);
        fprintf(fn, "birth: %s\n", genbuf);
        fprintf(fn, "----\n");
        flock(fileno(fn),LOCK_UN);
        fclose(fn);
        RETURN_LONG(0);
    }
	else
        RETURN_LONG(10);
}
#endif

/**
 *  Function: ¸ù¾Ý×¢²áÐÕÃûºÍemailÉú³ÉÐÂµÄÃÜÂë.Èç¹ûÓÃ»§ÃûÎª¿Õ,ÔòÉú³ÉÒ»¸öÃÜÂë.
 *   string bbs_findpwd_check(string userid,string realname,string email);
 *
 *   if failed. reaturn NULL string; or return new password.
 *              by binxun
 */
static PHP_FUNCTION(bbs_findpwd_check)
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

	//Éú³ÉÐÂÃÜÂë
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

/**
 * del board article
 * prototype:
 * int bbs_delfile(char* board, char* filename);
 *
 *  @return the result
 *  	0 -- success, -1 -- no perm
 *  	-2 -- wrong parameter
 *  @author binxun
 */
static PHP_FUNCTION(bbs_delfile)
{
	FILE *fp;
    bcache_t *brd;
    struct fileheader f;
    struct userec *u = NULL;
    char dir[80], path[80];
	long result = 0;

	char* board;
	char* file;
	int board_len,file_len;
    int num = 0;

	int ac = ZEND_NUM_ARGS();

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ss", &board, &board_len,&file,&file_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	u = getcurrentuser();
	brd = getbcache(board);

	if (VALID_FILENAME(file) < 0)
        RETURN_LONG(-2);
    if (brd == 0)
        RETURN_LONG(-2);
    if (!haspostperm(u, board))
        RETURN_LONG(-2);

	setbdir(DIR_MODE_NORMAL, dir, brd->filename);
	setbfile(path, brd->filename, file);
	/*
	 * TODO: Improve the following block of codes.
	 */
    fp = fopen(dir, "r");
    if (fp == 0)
        RETURN_LONG(-2);
	while (1) {
		if (fread(&f, sizeof(struct fileheader), 1, fp) <= 0)
			break;
		if (!strcmp(f.filename, file)) {
			if(del_post(num + 1, &f, dir, brd->filename) == DONOTHING)
				result = -1;
			else
				result = 0;
			break;
		}
		num++;
    }
    fclose(fp);

	RETURN_LONG(result);
}

/**
 * del mail
 * prototype:
 * int bbs_delmail(char* path,char* filename);
 *
 *  @return the result
 *  	0 -- success, -1 -- mail don't exist
 *  	-2 -- wrong parameter
 *  @author binxun
 */
static PHP_FUNCTION(bbs_delmail)
{
	FILE *fp;
    struct fileheader f;
    struct userec *u = NULL;
    char dir[80];
	long result = 0;

	char* path;
	char* filename;
	int path_len,filename_len;
    int num = 0;

	int ac = ZEND_NUM_ARGS();

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ss", &path, &path_len,&filename,&filename_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
    if (strncmp(filename, "M.", 2) || strstr(filename, ".."))
        RETURN_LONG(-2);

	u = getcurrentuser();

    sprintf(dir, "mail/%c/%s/%s", toupper(u->userid[0]),u->userid,path);
    fp = fopen(dir, "r");
    if (fp == 0)
        RETURN_LONG(-2);

	while (1) {
		if (fread(&f, sizeof(struct fileheader), 1, fp) <= 0)
			break;
		if (!strcmp(f.filename, filename)) {
			del_mail(num + 1, &f, dir);
			break;
		}
		num++;
    }
    fclose(fp);

	RETURN_LONG(result);
}

/**
 * check a board is normal board
 * prototype:
 * int bbs_normal(char* boardname);
 *
 *  @return the result
 *  	1 -- normal board
 *  	0 -- no
 *  @author kcn
 */
static PHP_FUNCTION(bbs_normalboard)
{
    int ac = ZEND_NUM_ARGS();
    char* boardname;
    int name_len;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &boardname, &name_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	RETURN_LONG(normal_board(boardname));
}

/*
 * valid_filename()
 * @author stiger
 */
static PHP_FUNCTION(bbs_valid_filename)
{
    int ac = ZEND_NUM_ARGS();
	char * filename;
	int name_len;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &filename, &name_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	RETURN_LONG(VALID_FILENAME(filename));
}

/*
 * bbs_can_send_mail ()
 * @author stiger
 */
static PHP_FUNCTION(bbs_can_send_mail)
{
	RETURN_LONG(can_send_mail());
}

/*
 * set a mail had readed
 */
static PHP_FUNCTION(bbs_setmailreaded)
{
	int ac = ZEND_NUM_ARGS();
	int num;
	char * dirname;
	int dirname_len;
	int total;
	struct fileheader fh;
	FILE *fp;

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &dirname, &dirname_len, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	total = file_size(dirname) / sizeof(fh) ;

	if(total <= 0)
		RETURN_LONG(0);

	if(num >=0 && num < total){
		if((fp=fopen(dirname,"r+"))==NULL)
			RETURN_LONG(0);
		fseek(fp,sizeof(fh) * num,SEEK_SET);
		if(fread(&fh,sizeof(fh),1,fp) > 0){
			if(fh.accessed[0] & FILE_READ){
				fclose(fp);
				RETURN_LONG(0);
			}
			else{
				fh.accessed[0] |= FILE_READ;
				fseek(fp,sizeof(fh)*num,SEEK_SET);
				fwrite(&fh,sizeof(fh),1,fp);
				fclose(fp);
				RETURN_LONG(1);
			}
		}
		fclose(fp);
	}
	RETURN_LONG(0);
}

/*
 * add import path,     annnounce.c
 * @author stiger
 */
static PHP_FUNCTION(bbs_add_import_path)
{
    int ac = ZEND_NUM_ARGS();
	char * path;
	int path_len;
	char * title;
	int title_len;
	int num;
	char *im_path[ANNPATH_NUM];
	char *im_title[ANNPATH_NUM];
	int im_time=0;
	int im_select=0;
	char buf[MAXPATH];
	char board[MAXPATH];
	char *c;
    struct boardheader *bp;

	if(ac == 2){
    	if ( zend_parse_parameters(2 TSRMLS_CC, "sl", &path, &path_len, &num) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		title = NULL;
	}else if(ac == 3){
    	if ( zend_parse_parameters(3 TSRMLS_CC, "ssl", &path, &path_len, &title, &title_len, &num) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
	}else
		WRONG_PARAM_COUNT;

	/* first ,check the path */
	if(strstr(path,"..") || strstr(path,"SYSHome") ) /* SYSHome? from bbs0an.c */
		RETURN_LONG(0);
	if(path[0] == '\0')
		RETURN_LONG(0);
	path_len = strlen(path);
	if(path[path_len-1]=='/')
		path[path_len-1]='\0';
	if(path[0]=='/')
		snprintf(buf, sizeof(buf), "0Announce%s", path);
	else if(strncmp(path,"0Announce",9))
		snprintf(buf, sizeof(buf), "0Announce/%s", path);
	else
		snprintf(buf, sizeof(buf), "%s", path);

	if(strncmp(buf,"0Announce/groups/",17))
		RETURN_LONG(0);

	if((c=strchr(buf+17,'/'))==NULL)
		RETURN_LONG(0);
	strcpy(board,c+1);
	if((c=strchr(board,'/'))!=NULL) *c='\0';
    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(0);
    }
    if(! is_BM(bp, currentuser))
        RETURN_LONG(0);

	if (ann_traverse_check(buf, currentuser) < 0)
		RETURN_LONG(0);

	if(num < 0 || num >= ANNPATH_NUM)
		RETURN_LONG(0);

	load_import_path(im_path,im_title,&im_time,&im_select);

	efree(im_path[num]);
	if ( (im_path[num] = emalloc(strlen(buf)+1)) ==NULL) {
		RETURN_LONG(0);
	}
	
	strcpy(im_path[num],buf);

	if(title == NULL || title[0]==0 ){
		MENU pm;

		bzero(&pm,sizeof(pm));
		pm.path = im_path[num];
		a_loadnames(&pm);
		strncpy(buf, pm.mtitle, MAXPATH - 1);
		buf[MAXPATH - 1]=0;
		a_freenames(&pm);
	}else{
		strncpy(buf, title, MAXPATH - 1);
		buf[MAXPATH - 1]=0;
	}

	efree(im_title[num]);
	if ( (im_title[num] = emalloc(strlen(buf)+1)) ==NULL) {
		RETURN_LONG(0);
	}
	strcpy(im_title[num],buf);

	save_import_path(im_path,im_title,&im_time);

	free_import_path(im_path,im_title,&im_time);

	RETURN_LONG(1);
}

static PHP_FUNCTION(bbs_get_import_path)
{
    zval *element,*ret_path;
	char *im_path[ANNPATH_NUM];
	char *im_title[ANNPATH_NUM];
	int im_time=0;
	int im_select=0;
	FILE *fp;
	int i;

    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }

	load_import_path(im_path,im_title,&im_time,&im_select);

	for(i=0;i<ANNPATH_NUM;i++){
        MAKE_STD_ZVAL(element);
        array_init(element);
    	add_assoc_string(element, "PATH", im_path[i], 1);
    	add_assoc_string(element, "TITLE", im_title[i], 1);
        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
    }

	free_import_path(im_path,im_title,&im_time);

}

static int bbs_bm_change(char *board, struct boardheader *newbh, struct boardheader *oldbh)
{
	int id,m,brd_num,n,i;
	int flag;
	struct userec *lookupuser;
	unsigned int newlevel;
	char *p;
	char oldbm[BM_LEN];
	char obm[10][IDLEN+2];
	int obmnum=0,nbmnum=0;
	char nbm[10][IDLEN+2];
	char buf[200];

	for(i=0;i<10;i++){
		obm[i][0]='\0';
		nbm[i][0]='\0';
	}

	if(oldbh){
		for(p = strtok(oldbh->BM, " "),obmnum=0; p && obmnum < 10; p=strtok(NULL," "),obmnum++){
			strncpy(obm[obmnum], p, IDLEN+2);
			obm[obmnum][IDLEN+1]='\0';
		}
	}

	for(p = strtok(newbh->BM, " "),nbmnum=0; p && nbmnum < 10; p=strtok(NULL," "),nbmnum++){
		strncpy(nbm[nbmnum], p, IDLEN+2);
		nbm[nbmnum][IDLEN+1]='\0';
	}

	newbh->BM[0]='\0';

	for( i=0; i<obmnum; i++ ){
		flag = 2;

		if(!(id = getuser(obm[i],&lookupuser))){
			continue;
		}

		for(m=0;m<nbmnum;m++){
			if(!strcmp(obm[i],nbm[m]))
				flag = 0;
		}
		if(flag == 0) continue;

		newlevel = lookupuser->userlevel;

		brd_num = 0;

		if( lookupuser->userlevel & PERM_BOARDS ){
			for(n = 0; n < get_boardcount(); n++){
				if(chk_BM_instr(getboard(n+1)->BM, lookupuser->userid) == true )
					brd_num ++;
			}
		}

		if( brd_num == 1){
			newlevel &= ~PERM_BOARDS;
			newlevel &= ~PERM_CLOAK;
		}

		sprintf(buf,"ÃâÈ¥ %s µÄ°ßÖñ %s", board, lookupuser->userid);
		//securityreport(buf, lookupuser, NULL);
		lookupuser->userlevel = newlevel;
	}

	for( i=0; i<nbmnum; i++ ){
		flag = 1;

		if(!(id = getuser(nbm[i],&lookupuser))){
			continue;
		}

		if( strlen(newbh->BM) + strlen(lookupuser->userid) >= BM_LEN - 2 )
			continue;

		for(m=0;m<obmnum;m++){
			if(!strcmp(nbm[i],obm[m])){
				flag = 0;
				if( newbh->BM[0] != '\0' )
					strcat(newbh->BM, " ");
				strcat(newbh->BM, lookupuser->userid);
			}
		}
		if(flag == 0) continue;

		newlevel = lookupuser->userlevel;

		if( newbh->BM[0] != '\0' )
			strcat( newbh->BM, " " );
		strcat(newbh->BM, lookupuser->userid);

		newlevel |= PERM_BOARDS;
		mail_file(currentuser->userid, "etc/forbm", lookupuser->userid, "ÐÂÈÎ°ßÖñ±Ø¶Á", BBSPOST_LINK, NULL);

		sprintf(buf,"ÈÎÃü %s µÄ°ßÖñ %s", board, lookupuser->userid);
		//securityreport(buf, lookupuser, NULL);
		lookupuser->userlevel = newlevel;
	}
}


/*
 * new a board
 * ÐÞ¸Ä°æÃæ²»ÔÊÐíÖØÐÂÐÞ¸Ä¾«»ªÇøÂ·¾¶
 */
static PHP_FUNCTION(bbs_new_board)
{
	int ac = ZEND_NUM_ARGS();

	char *bname;
	int bname_len;
	char *btitle;
	int btitle_len;
	char *bbm;
	int bbm_len;
	char *section;
	int section_len;
	char *desp;
	int desp_len;
	int blevel;
	int banony;
	int bjunk;
	int bout;
	int battach;
	int oldbnum;
	int bclubread;
	int bclubwrite;
	int bclubhide;

	char* bgroup;
	int bgroup_len;
	char explainn[100];
	
	int i;
	struct boardheader newboard;
	char vbuf[100];

    if (ac != 15 || zend_parse_parameters(15 TSRMLS_CC, "lsssssllllsllll", &oldbnum, &bname, &bname_len, &section, &section_len, &desp, &desp_len, &btitle, &btitle_len, &bbm, &bbm_len, &blevel, &banony, &bjunk, &bout, &bgroup, &bgroup_len, &battach, &bclubread, &bclubwrite, &bclubhide) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	bzero(&newboard,sizeof(newboard));
	if(bname_len >= 18)
		RETURN_LONG(-1);

	if(! HAS_PERM(currentuser, PERM_SYSOP) )
		RETURN_LONG(-7);

	strncpy(newboard.filename,bname,18);
	newboard.filename[17]='\0';

	strncpy(newboard.title+13,btitle,47);
	newboard.title[59]='\0';

	if(section[0]=='\0' || ! isprint(section[0]))
		RETURN_LONG(-10);
	newboard.title[0]=section[0];

	if(desp[0]=='\0')
		RETURN_LONG(-11);

	if(desp[0]=='['){
		strncpy(newboard.title+1,desp,11);
		for(i=desp_len+1; i<13; i++)
			newboard.title[i]=' ';
	}else{
		newboard.title[1]='[';
		strncpy(newboard.title+2,desp,10);
		if(desp_len < 10){
			newboard.title[2+desp_len]=']';
			for(i=2+desp_len+1; i<13; i++)
				newboard.title[i]=' ';
		}
		else
			newboard.title[12]=']';
	}

	if( ! valid_brdname(newboard.filename) )
		RETURN_LONG(-2);

	strncpy(newboard.BM, bbm, BM_LEN - 1);
	newboard.BM[BM_LEN-1] = '\0';

	newboard.level = blevel;

	if( banony )
		newboard.flag |= BOARD_ANNONY;

	if( bjunk )
		newboard.flag |= BOARD_JUNK;

	if( bout )
		newboard.flag |= BOARD_OUTFLAG;

	if( battach )
		newboard.flag |= BOARD_ATTACH;

	if(oldbnum < 0)
		RETURN_LONG(-14);

	if(oldbnum > 0)
		if( getboard(oldbnum) == 0)
			RETURN_LONG(-21);

	if( (oldbnum==0 && getbnum(newboard.filename) > 0))
		RETURN_LONG(-3);
	if( oldbnum && getbnum(newboard.filename)!=oldbnum && getbnum(newboard.filename)>0 )
		RETURN_LONG(-23);

	if( oldbnum ){	//¸ü¸Ä°æÃæÊôÐÔ
		struct boardheader oldboard;

		memcpy(&oldboard, getboard(oldbnum), sizeof(oldboard) );
		if(oldboard.filename[0]=='\0')
			RETURN_LONG(-22);

		if(strcmp(oldboard.filename,newboard.filename)){
			char old[256],tar[256];
			
			setbpath(old, oldboard.filename);
			setbpath(tar, newboard.filename);
			f_mv(old,tar);
			sprintf(old, "vote/%s", oldboard.filename);
			sprintf(tar, "vote/%s", newboard.filename);
			f_mv(old,tar);
		}

		bbs_bm_change(newboard.filename, &newboard, &oldboard);

		if(newboard.BM[0]!='\0' && strcmp(oldboard.BM, newboard.BM)){
			if(newboard.BM[0] != '\0'){
				if(strlen(newboard.BM) <= 30)
					sprintf(vbuf,"%-38.38s(BM: %s)", newboard.title + 13, newboard.BM);
				else
					snprintf(vbuf, STRLEN, "%-28.28s(BM: %s)", newboard.title + 13, newboard.BM);
			}else
				sprintf(vbuf,"%-38.38s", newboard.title+13);

			//edit_grp(oldboard.filename, oldboard.title + 13, vbuf);
			//¾«»ªÇø¸ü¸Ä
		}

		strncpy(newboard.ann_path, oldboard.ann_path, 128);
		newboard.ann_path[127]='\0';
		//¾«»ªÇøÒÆ¶¯
		
		if(oldboard.flag | BOARD_ANNONY)
			del_from_file("etc/anonymous",oldboard.filename);
		if(newboard.flag | BOARD_ANNONY)
			addtofile("etc/anonymous",newboard.filename);

		if( bclubread )
			newboard.flag |= BOARD_CLUB_READ;
		
		if( bclubwrite )
			newboard.flag |= BOARD_CLUB_WRITE;
		
		if( bclubhide )
			newboard.flag |= BOARD_CLUB_HIDE;
		
		set_board(oldbnum, &newboard, &oldboard);
		sprintf(vbuf, "¸ü¸ÄÌÖÂÛÇø %s µÄ×ÊÁÏ --> %s", oldboard.filename, newboard.filename);
		bbslog("user","%s",vbuf);

	}else{	//Ôö¼Ó°æÃæ

		if(newboard.flag | BOARD_ANNONY)
			addtofile("etc/anonymous",newboard.filename);

		sprintf(vbuf,"vote/%s",newboard.filename);
		if( mkdir(vbuf,0755) == -1 )
			RETURN_LONG( -4);

		sprintf(vbuf,"boards/%s",newboard.filename);
		if( mkdir(vbuf,0755) == -1 )
			RETURN_LONG( -5);

		bbs_bm_change(newboard.filename, &newboard, NULL);

		if(bgroup && bgroup_len > 0){
			for(i=0; groups[i] && explain[i]; i++){
				if( ! strncmp(groups[i], bgroup, strlen(groups[i])) )
					break;
			}

			if( groups[i]==NULL || explain[i]==NULL )
				RETURN_LONG( -13);

			if(newboard.BM[0] != '\0'){
				if(strlen(newboard.BM) <= 30)
					sprintf(vbuf,"%-38.38s(BM: %s)", newboard.title + 13, newboard.BM);
				else
					snprintf(vbuf, STRLEN, "%-28.28s(BM: %s)", newboard.title + 13, newboard.BM);
			}else
				sprintf(vbuf,"%-38.38s", newboard.title+13);

			if(add_grp(bgroup, newboard.filename, vbuf, explain[i]) == -1)
				RETURN_LONG( -12);

			snprintf(newboard.ann_path,127,"%s/%s",bgroup, newboard.filename);
			newboard.ann_path[127]=0;
		}

		if( add_board( &newboard ) == -1 )
			RETURN_LONG( -6);
	}

	RETURN_LONG(1);
}

static PHP_FUNCTION(bbs_set_onboard)
{
	int ac = ZEND_NUM_ARGS();
	long boardnum,count;
	int oldboard;
        struct WWW_GUEST_S *guestinfo;

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ll", &boardnum, &count) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
    if (currentuser==NULL) RETURN_FALSE;
    if (currentuinfo==NULL) RETURN_FALSE;
    if (!strcmp(currentuser->userid,"guest")) {
        guestinfo=www_get_guest_entry(currentuinfonum);
        oldboard=guestinfo->currentboard;
    } else
        oldboard=currentuinfo->currentboard;
    if (oldboard)
        board_setcurrentuser(oldboard, -1);
    
    board_setcurrentuser(boardnum, count);
    if (!strcmp(currentuser->userid,"guest")) {
        if (count>0)
            guestinfo->currentboard = boardnum;
        else
            guestinfo->currentboard = 0;
    }
    else {
        if (count>0)
            currentuinfo->currentboard = boardnum;
        else
            currentuinfo->currentboard = 0;
    }
    RETURN_TRUE;
}

static int bbs_can_access_vote(char *limitpath)
{
	struct votelimit userlimit;

	userlimit.numlogins=0;
	userlimit.numposts=0;
	userlimit.stay=0;
	userlimit.day=0;

	get_record(limitpath,&userlimit,sizeof(userlimit),1);

    if ((currentuser->numposts < userlimit.numposts
         || currentuser->numlogins < userlimit.numlogins
         || currentuser->stay < userlimit.stay * 60 * 60
         || (time(NULL) - currentuser->firstlogin) <
         userlimit.day * 24 * 60 * 60)) {

		return 0;
	}

	return 1;
}

static void bbs_make_vote_array(zval * array, struct votebal *vbal)
{
    add_assoc_string(array, "USERID", vbal->userid, 1);
    add_assoc_string(array, "TITLE", vbal->title, 1);
    add_assoc_long(array, "DATE", vbal->opendate);
    if(vbal->type <= 5 && vbal->type >= 1) add_assoc_string(array, "TYPE", vote_type[vbal->type-1], 1);
    add_assoc_long(array, "MAXDAY", vbal->maxdays);
}

static PHP_FUNCTION(bbs_get_votes)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	struct votebal vbal;
	char controlfile[STRLEN];
	struct boardheader *bp=NULL;
	FILE *fp;
	int vnum,i;
	zval *element,*retarray;

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sa", &bname, &bname_len, &retarray) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(-2);

	if( ! HAS_PERM(currentuser, PERM_LOGINOK) )
		RETURN_LONG(-1);

	sprintf(controlfile,"vote/%s/control",bname);

	if(array_init(retarray) != SUCCESS)
	{
                RETURN_LONG(-5);
	}

	vnum = get_num_records(controlfile,sizeof(struct votebal));

	if(vnum <= 0)
		RETURN_LONG(-4);

	if((fp=fopen(controlfile,"r"))==NULL)
		RETURN_LONG(-3);

	for(i=0; i < vnum; i++){
		if(fread(&vbal, sizeof(vbal), 1, fp) < 1)
			break;
		MAKE_STD_ZVAL(element);
		array_init(element);

		bbs_make_vote_array(element, &vbal);
		zend_hash_index_update(Z_ARRVAL_P(retarray), i,
				(void*) &element, sizeof(zval*), NULL);
	}

	fclose(fp);

	RETURN_LONG(i);
}

static void bbs_make_detail_vote_array(zval * array, struct votebal *vbal)
{
	int i;
	char tmp[10];

    add_assoc_string(array, "USERID", vbal->userid, 1);
    add_assoc_string(array, "TITLE", vbal->title, 1);
    add_assoc_long(array, "DATE", vbal->opendate);
    if(vbal->type <= 5 && vbal->type >= 1) add_assoc_string(array, "TYPE", vote_type[vbal->type-1], 1);
    add_assoc_long(array, "MAXDAY", vbal->maxdays);
    add_assoc_long(array, "MAXTKT", vbal->maxtkt);
    add_assoc_long(array, "TOTALITEMS", vbal->totalitems);

	for(i=0; i < vbal->totalitems; i++){
		sprintf(tmp,"ITEM%d",i+1);
		add_assoc_string(array, tmp, vbal->items[i], 1);
	}

}

static void bbs_make_user_vote_array(zval * array, struct ballot *vbal)
{
	int i;
	char tmp[10];

	if(vbal && vbal->uid[0]){
	    add_assoc_string(array, "USERID", vbal->uid, 1);
    	add_assoc_long(array, "VOTED", vbal->voted);
	    add_assoc_string(array, "MSG1", vbal->msg[0], 1);
	    add_assoc_string(array, "MSG2", vbal->msg[1], 1);
	    add_assoc_string(array, "MSG3", vbal->msg[2], 1);
	}else{
	    add_assoc_string(array, "USERID", "", 1);
	}
}

static int cmpvuid(char *userid, struct ballot *uv)
{
	return !strncmp(userid, uv->uid,IDLEN);
}

static PHP_FUNCTION(bbs_get_vote_from_num)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	struct votebal vbal;
	struct ballot uservote;
	char controlfile[STRLEN];
	struct boardheader *bp=NULL;
	FILE *fp;
	int vnum;
	zval *element,*retarray,*uservotearray;
	int ent;
	int pos;

    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "sala", &bname, &bname_len, &retarray, &ent, &uservotearray) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(-2);

	if( ! HAS_PERM(currentuser, PERM_LOGINOK) )
		RETURN_LONG(-1);

	sprintf(controlfile,"vote/%s/control",bname);

	if(array_init(retarray) != SUCCESS)
	{
                RETURN_LONG(-5);
	}

	if(array_init(uservotearray) != SUCCESS)
	{
                RETURN_LONG(-9);
	}

	vnum = get_num_records(controlfile,sizeof(struct votebal));

	if(vnum <= 0)
		RETURN_LONG(-4);

	if(ent <= 0 || ent > vnum)
		RETURN_LONG(-6);

	if((fp=fopen(controlfile,"r"))==NULL)
		RETURN_LONG(-3);

	fseek(fp,sizeof(vbal) * (ent-1), SEEK_SET);

	if(fread(&vbal, sizeof(vbal), 1, fp) < 1){
		fclose(fp);
		RETURN_LONG(-7);
	}
	fclose(fp);

	sprintf(controlfile,"vote/%s/limit.%lu",bname,vbal.opendate);
	if(! bbs_can_access_vote(controlfile))
		RETURN_LONG(-8);

	MAKE_STD_ZVAL(element);
	array_init(element);

	bbs_make_detail_vote_array(element, &vbal);
	zend_hash_index_update(Z_ARRVAL_P(retarray), 0,
				(void*) &element, sizeof(zval*), NULL);

	MAKE_STD_ZVAL(element);
	array_init(element);

	sprintf(controlfile,"vote/%s/flag.%lu",bname,vbal.opendate);
	if((pos = search_record(controlfile, &uservote, sizeof(uservote),
							(RECORD_FUNC_ARG) cmpvuid, currentuser->userid))<=0){
		bbs_make_user_vote_array(element, NULL);
	}
	else{
		bbs_make_user_vote_array(element, &uservote);
	}

	zend_hash_index_update(Z_ARRVAL_P(uservotearray), 0,
				(void*) &element, sizeof(zval*), NULL);

	RETURN_LONG(ent);
}

static PHP_FUNCTION(bbs_vote_num)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	char *msg;
	int msg_len;
	int ent;
	unsigned int votevalue;
	struct votebal vbal;
	struct ballot uservote;
	struct ballot tmpball;
	char controlfile[STRLEN];
	struct boardheader *bp=NULL;
	FILE *fp;
	int vnum,i,pos;
	char lmsg[3][STRLEN];
	char *c,*cc;
	int llen;

    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "slls", &bname, &bname_len, &ent, &votevalue, &msg, &msg_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	llen = msg_len+1;
	lmsg[0][0]='\0';
	lmsg[1][0]='\0';
	lmsg[2][0]='\0';
	strncpy(lmsg[0],msg,(llen > STRLEN)?STRLEN:llen);
	lmsg[0][STRLEN-1]='\0';
	if((c=strchr(msg,'\n'))!=NULL){
		c++;
		llen -= (c-msg);
		strncpy(lmsg[1],c,(llen > STRLEN)?STRLEN:llen);
		lmsg[1][STRLEN-1]='\0';
		if((cc=strchr(c,'\n'))!=NULL){
			cc++;
			llen -= (cc-c);
			strncpy(lmsg[2],cc,(llen > STRLEN)?STRLEN:llen);
			lmsg[2][STRLEN-1]='\0';
		}
	}
	for(llen=0;llen<3;llen++){
		if((c=strchr(lmsg[llen],'\n'))!=NULL)
			*c = '\0';
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(-2);

	if( ! HAS_PERM(currentuser, PERM_LOGINOK) )
		RETURN_LONG(-1);

	sprintf(controlfile,"vote/%s/control",bname);

	vnum = get_num_records(controlfile,sizeof(struct votebal));

	if(vnum <= 0)
		RETURN_LONG(-4);

	if(ent <= 0 || ent > vnum)
		RETURN_LONG(-6);

	if((fp=fopen(controlfile,"r"))==NULL)
		RETURN_LONG(-3);

	fseek(fp,sizeof(vbal) * (ent-1), SEEK_SET);

	if(fread(&vbal, sizeof(vbal), 1, fp) < 1){
		fclose(fp);
		RETURN_LONG(-7);
	}
	fclose(fp);

	sprintf(controlfile,"vote/%s/limit.%lu",bname,vbal.opendate);
	if(! bbs_can_access_vote(controlfile))
		RETURN_LONG(-8);

	if(vbal.type == 4 && votevalue > vbal.maxtkt)
		RETURN_LONG(-12);

	bzero( &uservote, sizeof(uservote) );
	strcpy(uservote.uid,currentuser->userid);
	uservote.voted = votevalue;
	strncpy(uservote.msg[0],lmsg[0],STRLEN);
	strncpy(uservote.msg[1],lmsg[1],STRLEN);
	strncpy(uservote.msg[2],lmsg[2],STRLEN);

	sprintf(controlfile,"vote/%s/flag.%lu",bname,vbal.opendate);
	if((pos = search_record(controlfile, &tmpball, sizeof(tmpball),
							(RECORD_FUNC_ARG) cmpvuid, currentuser->userid))>0){
		substitute_record(controlfile, &uservote, sizeof(uservote), pos); 
	}
	else{
		if(append_record(controlfile, &uservote, sizeof(uservote)) == -1)
			RETURN_LONG(-11);
	}

	RETURN_LONG(ent);
}

static PHP_FUNCTION(bbs_get_explain)
{
	int ac = ZEND_NUM_ARGS();
	zval *element,*retarray;
	int i;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "a", &retarray) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	for(i=0; secname[i][0] && groups[i]; i++){

		MAKE_STD_ZVAL(element);
		array_init(element);

	    add_assoc_string(element, "EXPLAIN", secname[i][0], 1);
	    add_assoc_string(element, "GROUPS", groups[i], 1);
		zend_hash_index_update(Z_ARRVAL_P(retarray), i,
				(void*) &element, sizeof(zval*), NULL);

	}

	RETURN_LONG(i);
}

static PHP_FUNCTION(bbs_start_vote)
{

	int ac = ZEND_NUM_ARGS();
	//zval *items;
	char *items[32];
	int items_len[32];
	char *board;
	int board_len;
	char *title;
	int title_len;
	char *desp;
	int desp_len;
	int type,ball_maxdays;
	int ball_maxtkt;
	int ball_totalitems;
	int i,pos;
	int numlogin,numpost,numstay,numday;
	struct boardheader *bp;
	struct boardheader fh;
	struct votebal ball;
	struct votelimit vlimit;
	char buf[PATHLEN];
	char buff[PATHLEN];
	FILE *fp;

    if (ac != 21 || zend_parse_parameters(21 TSRMLS_CC, "slllllsslllssssssssss", &board, &board_len, &type, &numlogin, &numpost, &numstay, &numday, &title, &title_len, &desp, &desp_len, &ball_maxdays, &ball_maxtkt, &ball_totalitems, &items[0], &items_len[0], &items[1], &items_len[1], &items[2], &items_len[2], &items[3], &items_len[3], &items[4], &items_len[4], &items[5], &items_len[5], &items[6], &items_len[6], &items[7], &items_len[7], &items[8], &items_len[8], &items[9], &items_len[9]) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(-1);
    }
    if(! is_BM(bp, currentuser) && !HAS_PERM(currentuser,PERM_SYSOP) )
        RETURN_LONG(-2);

	if(type < 1 || type > 5)
		RETURN_LONG(-3);

	if(ball_totalitems < 1 || ball_totalitems > 10)
		RETURN_LONG(-4);

	if(ball_maxdays <= 0)
		ball_maxdays = 1;

	if(strlen(title) <=0 )
		RETURN_LONG(-8);

	bzero(&ball,sizeof(ball));
	strncpy(ball.title,title,STRLEN);
	ball.title[STRLEN-1]='\0';

	ball.opendate = time(0);
	ball.type = type;
	ball.maxdays = 1;

	if(type == 1){
		ball.maxtkt = 1;
		ball.totalitems = 3;
		strcpy(ball.items[0], "ÔÞ³É  £¨ÊÇµÄ£©");
		strcpy(ball.items[1], "²»ÔÞ³É£¨²»ÊÇ£©");
		strcpy(ball.items[2], "Ã»Òâ¼û£¨²»Çå³þ£©");
	}else if(type == 2 || type == 3){
		if(type == 2) ball.maxtkt = 1;
		else ball.maxtkt = ball_maxtkt;

		ball.totalitems = ball_totalitems;
		for(i=0; i<ball.totalitems; i++){
			/*
			strncpy(ball.items[i], items[i], STRLEN);
			ball.items[i][STRLEN-1]='\0';*/
			/*overflow.      modified by binxun . */
			strncpy(ball.items[i], items[i], 38);
			ball.items[i][37]='\0';
		}
	}else if(type == 4){
		ball.maxtkt = ball_maxtkt;
		if(ball.maxtkt <= 0)
			ball.maxtkt = 100;
	}else if(type == 5){
		ball.maxtkt = 0;
		ball.totalitems = 0;
	}

	//setvoteflag
    pos = getboardnum(bp->filename, &fh);
    if (pos) {
        fh.flag = fh.flag | BOARD_VOTEFLAG;
        set_board(pos, &fh,NULL);
    }

	strcpy(ball.userid, currentuser->userid);

	sprintf(buf, "vote/%s/control", bp->filename);
	if(append_record(buf,&ball,sizeof(ball)) == -1)
		RETURN_LONG(-7);

	sprintf(buf,"%s OPEN VOTE",bp->filename);
	bbslog("user","%s",buf);

	sprintf(buf, "vote/%s/desc.%lu",bp->filename, ball.opendate );
	if((fp=fopen(buf,"w"))!=NULL){
		fputs(desp,fp);
		fclose(fp);
	}

	if(numlogin < 0) numlogin = 0;
	vlimit.numlogins = numlogin;

	if(numpost < 0) numpost = 0;
	vlimit.numposts = numpost;

	if(numstay < 0) numstay = 0;
	vlimit.stay = numstay;

	if(numday < 0) numday = 0;
	vlimit.day = numday;

	sprintf(buf,"vote/%s/limit.%lu",bp->filename, ball.opendate);
	append_record(buf, &vlimit, sizeof(vlimit));

	sprintf(buf,"tmp/votetmp.%d",getpid());
	if((fp=fopen(buf,"w"))==NULL){
		sprintf(buff,"[Í¨Öª] %s ¾Ù°ìÍ¶Æ±: %s",bp->filename,ball.title);
		fprintf(fp,"%s",buff);
		fclose(fp);
#ifdef NINE_BUILD
		post_file(currentuser, "", buf, bp->filename, buff, 0, 1);
		post_file(currentuser, "", buf, "vote", buff, 0, 1);
#else
		if( !normal_board(bp->filename) ){
			post_file(currentuser, "", buf, bp->filename, buff, 0,1);
		}else{
			post_file(currentuser, "", buf, "vote", buff, 0,1);
		}
#endif
		unlink(buf);
	}

	RETURN_LONG(1);
}


/*
  * bbs_load_favboard()

*/
static PHP_FUNCTION(bbs_load_favboard)
{
        int ac = ZEND_NUM_ARGS();
        int dohelp;
        int select;
        if(ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ll", &dohelp,&select) ==FAILURE) {
                WRONG_PARAM_COUNT;
        }
        load_favboard(dohelp);
        if(select<favbrd_list_t)
        {
                SetFav(select);
                RETURN_LONG(0);
        }
        else 
                RETURN_LONG(-1);
}

static PHP_FUNCTION(bbs_is_favboard)
{
        int ac = ZEND_NUM_ARGS();
        int position;
        if(ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l" ,&position) == FAILURE){
                WRONG_PARAM_COUNT;
        }
        RETURN_LONG(IsFavBoard(position));
}

static PHP_FUNCTION(bbs_del_favboard)
{
        int ac = ZEND_NUM_ARGS();
        int position;
        if(ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l" , &position) == FAILURE){
                WRONG_PARAM_COUNT;
        }
        DelFavBoard(position);
        save_favboard();
}
//add fav dir
static PHP_FUNCTION(bbs_add_favboarddir)
{
        int ac = ZEND_NUM_ARGS();
        int char_len;   
        char *char_dname;
        if(ac != 1 || zend_parse_parameters(1 TSRMLS_CC,"s",&char_dname,&char_len) ==FAILURE){
                WRONG_PARAM_COUNT;
        }
        if(char_len <= 20)
        {
                addFavBoardDir(0,char_dname);
                save_favboard();
        }
        RETURN_LONG(char_len);
}

static PHP_FUNCTION(bbs_add_favboard)
{
        int ac = ZEND_NUM_ARGS();
        int char_len;
        char *char_bname;
        int i;
        if(ac !=1 || zend_parse_parameters(1 TSRMLS_CC,"s",&char_bname,&char_len) ==FAILURE){
                WRONG_PARAM_COUNT;
        }
        i=getbnum(char_bname);
        if(i >0 && ! IsFavBoard(i - 1))
        {
                addFavBoard(i - 1);
                save_favboard();
        }
}

/**
 * Fetch all fav boards which have given prefix into an array.
 * prototype:
 * array bbs_fav_boards(char *prefix, int yank);
 *
 * @return array of loaded fav boards on success,
 *         FALSE on failure.
 * @
 */


static PHP_FUNCTION(bbs_fav_boards)
{
    int select;
    int yank;
    int rows = 0;
    struct newpostdata newpost_buffer[FAVBOARDNUM];
    struct newpostdata *ptr;
    zval **columns;
    zval *element;
    int i;
    int j;
    int ac = ZEND_NUM_ARGS();
    int brdnum, yank_flag;
    /*
     * getting arguments 
     */
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ll", &select, &yank) == FAILURE) {
        WRONG_PARAM_COUNT;
    }


    /*
     * loading boards 
     */
    /*
     * handle some global variables: currentuser, yank, brdnum, 
     * * nbrd.
     */
    /*
     * NOTE: currentuser SHOULD had been set in funcs.php, 
     * * but we still check it. 
     */
    if (currentuser == NULL) {
        RETURN_FALSE;
    }
    yank_flag = yank;
    if (strcmp(currentuser->userid, "guest") == 0)
        yank_flag = 1;          /* see all boards including zapped boards. */
    if (yank_flag != 0)
        yank_flag = 1;
    brdnum = 0;
    
    if ((brdnum = fav_loaddata(newpost_buffer, select, 1, FAVBOARDNUM, 1, NULL)) <= -1) {
        RETURN_FALSE;
    }
    /*
     * fill data in output array. 
     */
    /*
     * setup column names 
     */
    rows=brdnum;
    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }
    columns = emalloc(BOARD_COLUMNS * sizeof(zval *));

	if (columns==NULL){
		RETURN_FALSE;
	}
    for (i = 0; i < BOARD_COLUMNS; i++) {
        MAKE_STD_ZVAL(element);
        array_init(element);
        columns[i] = element;
        zend_hash_update(Z_ARRVAL_P(return_value), brd_col_names[i], strlen(brd_col_names[i]) + 1, (void *) &element, sizeof(zval *), NULL);
    }
   /*
     * fill data for each column 
     */
   for (i = 0; i < rows; i++) {
        ptr = &newpost_buffer[i];
        check_newpost(ptr);
        for (j = 0; j < BOARD_COLUMNS; j++) {
            MAKE_STD_ZVAL(element);
            bbs_make_board_zval(element, brd_col_names[j], ptr);
            zend_hash_index_update(Z_ARRVAL_P(columns[j]), i, (void *) &element, sizeof(zval *), NULL);
        }       
    }
        
    efree(columns);
    
}

/*
 * bbs_release_favboard()

*/
static PHP_FUNCTION(bbs_release_favboard)
{
        release_favboard();
}


/*
 * bbs_sysconf_str
 »ñÈ¡ÏµÍ³²ÎÊý
*/
static PHP_FUNCTION(bbs_sysconf_str)
{
         int ac = ZEND_NUM_ARGS();
        int char_len;
        char *char_conf;
        char *char_result;
        if(ac !=1 || zend_parse_parameters(1 TSRMLS_CC,"s",&char_conf,&char_len) ==FAILURE){
                WRONG_PARAM_COUNT;
        }
        char_result=sysconf_str(char_conf);//»ñÈ¡ÅäÖÆ²ÎÊý
        RETURN_STRING(char_result,1);
}

/****
 * add by stiger, template, Ãþ°å
 */
static void bbs_make_tmpl_array(zval * array, struct a_template * ptemp, char *board)
{
    add_assoc_string(array, "TITLE", ptemp->tmpl->title, 1);
    add_assoc_string(array, "TITLE_TMPL", ptemp->tmpl->title_tmpl, 1);
    add_assoc_long(array, "CONT_NUM", ptemp->tmpl->content_num);
	if( ptemp->tmpl->filename[0] ){
		char path[STRLEN];
		setbfile( path, board, ptemp->tmpl->filename );
    	add_assoc_string(array, "FILENAME", path,1);
	}else
 		add_assoc_string(array, "FILENAME", ptemp->tmpl->filename,1);
}

static PHP_FUNCTION(bbs_get_tmpls)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	char path[STRLEN];
	struct a_template * ptemp = NULL;
	int mode,tmpl_num,i;
	struct boardheader *bp=NULL;
	zval *element,*retarray;

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sa", &bname, &bname_len, &retarray) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(-2);

	setbfile(path, bname, TEMPLATE_DIR);

	if(array_init(retarray) != SUCCESS)
	{
                RETURN_LONG(-5);
	}

    if(is_BM(bp, currentuser))
		mode = 1;
	else
		mode = 0;

	tmpl_num = orig_tmpl_init(bname, mode, & ptemp);
	
	if(tmpl_num < 0)
		RETURN_LONG(-6);

	if(tmpl_num == 0)
		RETURN_LONG(0);

	for(i=0; i < tmpl_num; i++){
		MAKE_STD_ZVAL(element);
		array_init(element);

		bbs_make_tmpl_array(element, ptemp + i,bname);
		zend_hash_index_update(Z_ARRVAL_P(retarray), i,
				(void*) &element, sizeof(zval*), NULL);
	}

	orig_tmpl_free( & ptemp, tmpl_num );

	RETURN_LONG(i);
}

/**********
 * get a detail tmpl, stiger
 */

static void bbs_make_detail_tmpl_array(zval * array, struct s_content * cont)
{

    add_assoc_string(array, "TEXT", cont->text, 1);
    add_assoc_long(array, "LENGTH", cont->length);

}

static PHP_FUNCTION(bbs_get_tmpl_from_num)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	char path[STRLEN];
	struct boardheader *bp=NULL;
	struct a_template * ptemp = NULL;
	zval *element,*retarray;
	int ent,tmpl_num,i,mode;

    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sla", &bname, &bname_len, &ent, &retarray) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(-2);

	setbfile(path, bname, TEMPLATE_DIR);

	if(array_init(retarray) != SUCCESS)
	{
                RETURN_LONG(-5);
	}

    if(is_BM(bp, currentuser))
		mode = 1;
	else
		mode = 0;

	tmpl_num = orig_tmpl_init(bname, mode, & ptemp);
	
	if(tmpl_num < 0)
		RETURN_LONG(-6);

	if(tmpl_num == 0)
		RETURN_LONG(0);

	if(ent <= 0 || ent > tmpl_num){
		orig_tmpl_free( & ptemp, tmpl_num );
		RETURN_LONG(-8);
	}

	MAKE_STD_ZVAL(element);
	array_init(element);

	bbs_make_tmpl_array(element, ptemp+ent-1, bname);
	zend_hash_index_update(Z_ARRVAL_P(retarray), 0,
				(void*) &element, sizeof(zval*), NULL);

	for(i=0; i < ptemp[ent-1].tmpl->content_num; i++){

		MAKE_STD_ZVAL(element);
		array_init(element);

		bbs_make_detail_tmpl_array(element, ptemp[ent-1].cont+i);
		zend_hash_index_update(Z_ARRVAL_P(retarray), i+1,
				(void*) &element, sizeof(zval*), NULL);
	}

	orig_tmpl_free( & ptemp, tmpl_num );

	RETURN_LONG(ent);
}

static PHP_FUNCTION(bbs_make_tmpl_file)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	char tmpfname[STRLEN];
	char path[STRLEN];
	struct boardheader *bp=NULL;
	struct a_template * ptemp = NULL;
	FILE *fp,*fpsrc;
	int write_ok=0;
	int ent,tmpl_num,i,mode;
	char newtitle[STRLEN];

	char *text[21];
	int t_len[21];

    if (ac != 23 || zend_parse_parameters(23 TSRMLS_CC, "slsssssssssssssssssssss", &bname, &bname_len, &ent, &text[0],&t_len[0],&text[1],&t_len[1],&text[2],&t_len[2],&text[3],&t_len[3],&text[4],&t_len[4],&text[5],&t_len[5],&text[6],&t_len[6],&text[7],&t_len[7],&text[8],&t_len[8],&text[9],&t_len[9],&text[10],&t_len[10],&text[11],&t_len[11],&text[12],&t_len[12],&text[13],&t_len[13],&text[14],&t_len[14],&text[15],&t_len[15],&text[16],&t_len[16],&text[17],&t_len[17],&text[18],&t_len[18],&text[19],&t_len[19],&text[20],&t_len[20]) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(0);

	if( currentuser == NULL )
		RETURN_LONG(0);

	setbfile(path, bname, TEMPLATE_DIR);

    if(is_BM(bp, currentuser))
		mode = 1;
	else
		mode = 0;

	tmpl_num = orig_tmpl_init(bname, mode, & ptemp);
	
	if(tmpl_num < 0)
		RETURN_LONG(0);

	if(tmpl_num == 0)
		RETURN_LONG(0);

	if(ent <= 0 || ent > tmpl_num){
		orig_tmpl_free( & ptemp, tmpl_num );
		RETURN_LONG(0);
	}

	sprintf(tmpfname, "tmp/%s.tmpl.tmp", currentuser->userid);
	if((fp = fopen(tmpfname, "w"))==NULL){
		RETURN_LONG(0);
	}

	if( ptemp[ent-1].tmpl->filename[0] ){
		setbfile( path,bname, ptemp[ent-1].tmpl->filename);
		if( dashf( path )){
			if((fpsrc = fopen(path,"r"))!=NULL){
				char buf[256];

				while(fgets(buf,255,fpsrc)){
					int l;
					int linex = 0;
					int ischinese=0;
					char *pn,*pe;

					for(pn = buf; *pn!='\0'; pn++){
						if( *pn != '[' || *(pn+1)!='$' ){
							fputc(*pn, fp);
							linex++;
						}else{
							pe = strchr(pn,']');
							if(pe == NULL){
								fputc(*pn, fp);
								continue;
							}
							l = atoi(pn+2);
							if( l<=0 || l > ptemp[ent-1].tmpl->content_num || l > 20){
								fputc('[', fp);
								continue;
							}
							fprintf(fp,"%s",text[l]);
							pn = pe;
							continue;
						}
					}
				}
				fclose(fpsrc);

				write_ok = 1;
			}
		}
	}
	if(write_ok == 0){
		for(i=0; i< ptemp[ent-1].tmpl->content_num && i<20; i++)
			fprintf(fp,"[1;32m%s:[m\n%s\n\n",ptemp[ent-1].cont[i].text, text[i+1]);
	}
	fclose(fp);

	if( ptemp[ent-1].tmpl->title_tmpl[0] ){
		char *pn,*pe;
		char *buf;
		int l;
		int newl = 0;

		newtitle[0]='\0';
		buf = ptemp[ent-1].tmpl->title_tmpl;

		for(pn = buf; *pn!='\0' && newl < STRLEN-1; pn++){
			if( *pn != '[' || *(pn+1)!='$' ){
				if( newl < STRLEN - 1 ){
					newtitle[newl] = *pn ;
					newtitle[newl+1]='\0';
					newl ++;
				}
			}else{
				pe = strchr(pn,']');
				if(pe == NULL){
					if( newl < STRLEN - 1 ){
						newtitle[newl] = *pn ;
						newtitle[newl+1]='\0';
						newl ++;
					}
					continue;
				}
				l = atoi(pn+2);
				if( l<0 || l > ptemp[ent-1].tmpl->content_num || l > 20){
					if( newl < STRLEN - 1 ){
						newtitle[newl] = *pn ;
						newtitle[newl+1]='\0';
						newl ++;
					}
					continue;
				}
				if( l == 0 ){
					int ti;
					for( ti=0; text[0][ti]!='\0' && ti < t_len[0] && newl < STRLEN - 1; ti++, newl++ ){
						newtitle[newl] = text[0][ti] ;
						newtitle[newl+1]='\0';
					}
				}else{
					int ti;
					for( ti=0; text[l][ti]!='\0' && ti < t_len[l] && text[l][ti]!='\n' && text[l][ti]!='\r' && newl < STRLEN - 1; ti++, newl++ ){
						newtitle[newl] = text[l][ti] ;
						newtitle[newl+1]='\0';
					}
				}
				pn = pe;
				continue;
			}
		}
	}else{
		strncpy(newtitle, text[0], STRLEN);
		newtitle[STRLEN-1]='\0';
	}

	orig_tmpl_free( & ptemp, tmpl_num );

	//RETURN_LONG(1);
	RETURN_STRING(newtitle,1);
}

#ifdef SMS_SUPPORT

static PHP_FUNCTION(bbs_send_sms)
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

static PHP_FUNCTION(bbs_register_sms_sendcheck)
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

static PHP_FUNCTION(bbs_register_sms_docheck)
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

static PHP_FUNCTION(bbs_unregister_sms)
{
	int ret;

	ret = web_unregister_sms();

	RETURN_LONG(ret);
}

#endif

#if HAVE_MYSQL == 1
static PHP_FUNCTION(bbs_csv_to_al)
{
	int ac = ZEND_NUM_ARGS();
	char *dest;
	int dest_len;
	int ret;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &dest, &dest_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	ret = conv_csv_to_al( dest );

	RETURN_LONG(ret);
}
#endif

/*
 *  bbs_getonline_user_list
 *  »ñÈ¡ÔÙÏßÓÃ»§ÁÐ±í
 *
 *  Ã»ÓÐ²ÎÊý
 *  ·µ»ØÔÚÏßÓÃ»§Êý×é
 *  ¸ñÊ½£º(invisible,isfriend,userid,username,userfrom,mode,idle(Ãî))
 */
static PHP_FUNCTION(bbs_getonline_user_list)
{

	int i;
	uinfo_t **user;

	zval* element;
	
	int total = 0;

    if ( 0!=ZEND_NUM_ARGS() )
		WRONG_PARAM_COUNT;

   	if (array_init(return_value) == FAILURE)
      	 RETURN_FALSE;

	apply_ulist_addr((APPLY_UTMP_FUNC) full_utmp, (char *) &total);

	user = get_ulist_addr();

	for ( i=0; i < total; i++ ) {
		MAKE_STD_ZVAL ( element );
		array_init ( element );

		add_assoc_bool ( element, "invisible", user[i]->invisible );
		add_assoc_bool ( element, "isfriend", isfriend(user[i]->userid) );
		add_assoc_string ( element, "userid", user[i]->userid, 1 );
		add_assoc_string ( element, "username", user[i]->username, 1 );
		add_assoc_string ( element, "userfrom", user[i]->from, 1 );
		add_assoc_string ( element, "mode", ModeType(user[i]->mode), 1 );
		add_assoc_long ( element, "idle", (time(0) - get_idle_time(user[i])) );

		zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
	}
}

int get_pos(char * s)
{
    struct stat st;
    FILE* fp;
    char buf[240],buf2[100],tt[100];
    int i,j,k;
    if(stat(s, &st)==-1) return -1;
    strcpy(buf, s);
    i=strlen(buf)-1;
    while(buf[i]!='/') i--;
    i++;
    strcpy(buf2, buf+i);
    strcpy(buf+i, ".Names");
    fp=fopen(buf, "r");
    if(fp==NULL) return -1;
    tt[0]=0;
    j=0;
    while(!feof(fp))
    {
	if(!fgets(buf, 240, fp)) {
	    fclose(fp);
	    return -1;
	}
	if(buf[0]) buf[strlen(buf)-1]=0;
	if(!strncmp(buf, "Name=", 5)) {
	    strcpy(tt, buf+5);
	}
	if(!strncmp(buf, "Path=~/", 7)) {
	    j++;
	    if(!strcmp(buf+7, buf2)) {
		fclose(fp);
		return j;
	    }
	}
    }
    fclose(fp);
    return -1;
}

static PHP_FUNCTION(bbs_x_search)
{
    int toomany, res_total;
    int ac = ZEND_NUM_ARGS();
    int char_len;   
    int pos;
    char *qn;
    zval* element;
    struct sockaddr_in addr;
    FILE* sockfp;
    int sockfd, i, j, k, ttt;
    char buf[256];
    char ip[20], s1[30], s2[30], *pp;
    #define MAX_KEEP 100
    char res_title[MAX_KEEP][80],res_filename[MAX_KEEP][200],res_path[MAX_KEEP][200],res_content[MAX_KEEP][1024];
    int res_flag[MAX_KEEP];

    if(ac != 2 || zend_parse_parameters(2 TSRMLS_CC,"sl",&qn,&char_len,&pos) ==FAILURE){
        WRONG_PARAM_COUNT;
    }
    if (array_init(return_value) == FAILURE)
        RETURN_FALSE;

    strcpy(ip, sysconf_str("QUERY_SERVER"));
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))==-1) return;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family=AF_INET;    
    addr.sin_addr.s_addr=inet_addr(ip);
    addr.sin_port=htons(4875);
    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr))<0) return;
    sockfp=fdopen(sockfd, "r+");
    fprintf(sockfp, "\n%d\n%s\n", pos, qn);
    fflush(sockfp);
    fscanf(sockfp, "%d %d %d\n", &toomany, &i, &res_total);
    for(i=0;i<res_total;i++) {
        fgets(buf, 256, sockfp);
        if(buf[0]&&buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]=0;
        strncpy(res_title[i], buf, 80);
        res_title[i][79] = 0;

        fgets(buf, 256, sockfp);
        if(buf[0]&&buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]=0;
        strncpy(res_filename[i], buf, 200);
        res_filename[i][199] = 0;
    }
    fclose(sockfp);
    close(sockfd);
    for(i=0;i<res_total;i++)
	if(get_pos(res_filename[i])==-1) {
	    strcpy(res_path[i], "ÎÞÐ§ÎÄÕÂ");
	    res_flag[i]=1;
	}
        else {
	    char buf[200],buf2[200];
	    res_flag[i]=0;
	    if(!strncmp(res_filename[i], "0Announce/groups/", 17)) {
		j=17;
		while(res_filename[i][j]!='/') j++;
		j++;
		k=j;
		while(res_filename[i][j]!='/') j++;
		strcpy(buf,res_filename[i]+k);
		buf[j-k]=0;
		strcpy(res_path[i],buf);
		k=strlen(res_path[i]);
		while(1) {
		    j++;
		    while(res_filename[i][j]!='/'&&res_filename[i][j]) j++;
		    strcpy(buf2, res_filename[i]);
		    buf2[j] = 0;
		    sprintf(res_path[i]+k, "-%d", get_pos(buf2));
		    k = strlen(res_path[i]);
		    if(!res_filename[i][j]) break;
		}
	    }
	    else {
		j=10;
		strcpy(res_path[i], "¾«»ªÇø");
		k=strlen(res_path[i]);
		while(1) {
		    j++;
		    while(res_filename[i][j]!='/'&&res_filename[i][j]) j++;
		    strcpy(buf2, res_filename[i]);
		    buf2[j] = 0;
		    sprintf(res_path[i]+k, "-%d", get_pos(buf2));
		    k = strlen(res_path[i]);
		    if(!res_filename[i][j]) break;
		}
	    }
	}
    for(i=0;i<res_total;i++) 
    if(res_title[i][0]){
        if(strlen(res_title[i])>30){
	    j=30;
	    while(res_title[i][j]!=' '&&res_title[i][j])j++;
	    res_title[i][j]=0;
        }
	j=strlen(res_title[i])-1;
	if((j>=0)&&res_title[i][j]==' ')j--;
	j++;
	res_title[i][j]=0;
    }
    for(i=0;i<res_total;i++) {
	pp=res_filename[i]+strlen(res_filename[i])-1;
	while(*pp!='/') pp--;
	strcpy(s1, pp+1);
	if(strlen(s1)>7)
	for(j=i+1;j<res_total;j++) {
	    pp=res_filename[j]+strlen(res_filename[j])-1;
	    while(*pp!='/') pp--;
	    strcpy(s2, pp+1);
	    if(!strcmp(s1,s2)) {
		res_flag[j]=1;
		strcpy(res_path[j], "ÖØ¸´ÎÄÕÂ");
	    }
	}
    }

    for (ttt=0; ttt < res_total; ttt++) {
        char buf[10*1024],out[10*1024],out2[10*1024];
        FILE* fp;
        int i,j,k,l,fsize=0,t=0,p=0,inc=0;
        res_content[ttt][0] = 0;
        fp = fopen(res_filename[ttt], "rb");
        if(!fp) continue;
        fsize = fread(buf, 1, 10*1024, fp);
        fclose(fp);
        memset(out, 0, fsize);
        for(i=0;i<fsize;i++) {
            if(buf[i]==0x1b) {
                j=i;
                while(!(buf[j]>='a'&&buf[j]<='z'||buf[j]>='A'&&buf[j]<='Z')&&j<fsize) {
                    buf[j] = 0;
                    j++;
                }
                if(j<fsize)
                    buf[j] = 0;
            }
        }
        i=0;
        if(qn[i]=='=') {
            while(i<strlen(qn)&&qn[i]!=' ') i++;
            if(i>=strlen(qn)) i=0;
        }
        while(i<strlen(qn)) {
            if(qn[i]>='a'&&qn[i]<='z'||qn[i]>='A'&&qn[i]<='Z') {
                j=i;
                while(qn[j]>='a'&&qn[j]<='z'||qn[j]>='A'&&qn[j]<='Z'||qn[j]>='0'&&qn[j]<='9') j++;
                for(k=0;k<fsize-(j-i)+1;k++)
                    if(!strncasecmp(qn+i,buf+k,j-i)&&(k==0||!(buf[k-1]>='a'&&buf[k-1]<='z'||buf[k-1]>='A'&&buf[k-1]<='Z'))&&
                        (k+j-i==fsize||!(buf[k+j-i]>='a'&&buf[k+j-i]<='z'||buf[k+j-i]>='A'&&buf[k+j-i]<='Z')))
                        for(l=0;l<j-i;l++) if(!out[k+l]){out[k+l]=1;t++;}
                i=j-1;
            }
            if(qn[i]>='0'&&qn[i]<='9') {
                j=i;
                while(qn[j]>='0'&&qn[j]<='9') j++;
                for(k=0;k<fsize-(j-i)+1;k++)
                    if(!strncmp(qn+i,buf+k,j-i)&&(k==0||!(buf[k-1]>='0'&&buf[k-1]<='9'))&&
                        (k+j-i==fsize||!(buf[k+j-i]>='0'&&buf[k+j-i]<='9')))
                        for(l=0;l<j-i;l++) if(!out[k+l]){out[k+l]=1;t++;}
                i=j-1;
            }
            if(qn[i]<0&&qn[i+1]<0) {
                j=i+2;
                for(k=0;k<fsize-(j-i)+1;k++)
                    if(!strncmp(qn+i,buf+k,j-i))
                        for(l=0;l<j-i;l++) if(!out[k+l]){out[k+l]=1;t++;}
                i=j-1;
            }
            i++;
        }
        if(t>=20) {
            for(k=0;k<fsize-4;k++) {
                if(out[k]==0&&out[k+1]==1&&out[k+2]==0) {
                    out[k+1]=0;
                    t--;
                }
                if(out[k]==0&&out[k+1]==1&&out[k+2]==1&&out[k+3]==0) {
                    out[k+1]=0;
                    out[k+2]=0;
                    t-=2;
                }
                if(t<10) break;
            }
        }
        if(t==0) {
            continue;
        }
        while(t<240&&t<fsize) {
            memset(out2, 0, fsize);
            t=0;
            for(k=0;k<fsize;k++) 
            if(!out2[k]) {
                if(out[k]||k>0&&out[k-1]||k<fsize-1&&out[k+1]) {
                    if(out[k]==1)
                        out2[k]=1;
                    else {
                        out2[k]=2;
                        if(!out[k]&&buf[k]<0) {
                            if(k>0&&out[k-1]&&k<fsize-1) out2[k+1]=2;
                            if(k>0&&k<fsize-1&&out[k+1]) out2[k-1]=2;
                            t++;
                        }
                    }
                    t++;
                }
            }
            memcpy(out,out2,fsize);
        }
        pp = res_content[ttt];
        j=0; t = 0;
        for(i=0;i<fsize;i++)
        if(out[i]) {
            if(i>0&&out[i-1]==0) {
                sprintf(pp, "...");
                pp+=3;
                j+=3;
            }
            if(out[i]==1&&!inc) {
                sprintf(pp, "<font class=\"f001\">");
                pp += 19;
                inc = 1;
            }
            else if(out[i]!=1&&inc) {
                sprintf(pp, "</font>");
                pp += 7;
                inc = 0;
            }
            if(buf[i]=='\n') *(pp++) = ' ';
            else if(buf[i]) *(pp++) = buf[i];
            if(p) p=0;
            else if(buf[i]<0) p=1;
            j++;
            if(j>=84&&p==0) {
                t++;
                if(t>=3) break;
                j=0;
            }
        }
        if(inc) {
            sprintf(pp, "</font>");
            pp += 7;
        }
        *pp = 0;
    }


    for ( i=0; i < res_total; i++ ) {
        MAKE_STD_ZVAL ( element );
        array_init ( element );

        add_assoc_string ( element, "title", res_title[i], 1 );
        add_assoc_string ( element, "filename", res_filename[i], 1 );
        add_assoc_string ( element, "path", res_path[i], 1 );
        add_assoc_string ( element, "content", res_content[i], 1 );

        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
    }
}

