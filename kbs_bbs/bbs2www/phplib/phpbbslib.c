#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
    
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"  
#include "php_smth_bbs.h"  

#include "bbs.h"
#include "bbslib.h"

static unsigned char third_arg_force_ref_1111[] = { 4, BYREF_FORCE, BYREF_FORCE, BYREF_FORCE, BYREF_FORCE };
static unsigned char third_arg_force_ref_011[] = { 3, BYREF_NONE, BYREF_FORCE, BYREF_FORCE };
static unsigned char fourth_arg_force_ref_0001[] = { 4, BYREF_NONE, BYREF_NONE, BYREF_NONE, BYREF_FORCE };

static PHP_FUNCTION(bbs_getuser);
static PHP_FUNCTION(bbs_getonlineuser);
static PHP_FUNCTION(bbs_getonlinenumber);
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
static PHP_FUNCTION(bbs_brcaddread);
static PHP_FUNCTION(bbs_ann_traverse_check);
static PHP_FUNCTION(bbs_ann_get_board);
static PHP_FUNCTION(bbs_getboards);
static PHP_FUNCTION(bbs_getarticles);
static PHP_FUNCTION(bbs_get_records_from_id);
static PHP_FUNCTION(bbs_get_filename_from_num);
static PHP_FUNCTION(bbs_countarticles);
static PHP_FUNCTION(bbs_is_bm);
static PHP_FUNCTION(bbs_getannpath);
static PHP_FUNCTION(bbs_getmailnum);
static PHP_FUNCTION(bbs_getmailnum2);
static PHP_FUNCTION(bbs_getmails);
static PHP_FUNCTION(bbs_getmailusedspace);
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
static PHP_FUNCTION(bbs_delfile);
static PHP_FUNCTION(bbs_delmail);
static PHP_FUNCTION(bbs_normalboard);


/*
 * define what functions can be used in the PHP embedded script
 */
static function_entry smth_bbs_functions[] = {
        PHP_FE(bbs_getuser, NULL)
        PHP_FE(bbs_getonlineuser, NULL)
        PHP_FE(bbs_getonlinenumber, NULL)
        PHP_FE(bbs_countuser, NULL)
        PHP_FE(bbs_setfromhost, NULL)
        PHP_FE(bbs_checkpasswd, NULL)
        PHP_FE(bbs_getcurrentuser, NULL)
        PHP_FE(bbs_setonlineuser, NULL)
        PHP_FE(bbs_getcurrentuinfo, NULL)
        PHP_FE(bbs_wwwlogin, NULL)
        PHP_FE(bbs_wwwlogoff, NULL)
        PHP_FE(bbs_printansifile, NULL)
        PHP_FE(bbs_checkreadperm, NULL)
        PHP_FE(bbs_getbname, NULL)
        PHP_FE(bbs_checkpostperm, NULL)
        PHP_FE(bbs_brcaddread, NULL)
        PHP_FE(bbs_getboard, NULL)
        PHP_FE(bbs_ann_traverse_check, NULL)
        PHP_FE(bbs_ann_get_board, NULL)
        PHP_FE(bbs_getboards, NULL)
        PHP_FE(bbs_getarticles, NULL)
        PHP_FE(bbs_get_records_from_id, NULL)
        PHP_FE(bbs_get_filename_from_num, NULL)
        PHP_FE(bbs_countarticles, NULL)
        PHP_FE(bbs_is_bm, NULL)
        PHP_FE(bbs_getannpath, NULL)
        PHP_FE(bbs_getmailnum, third_arg_force_ref_011)
        PHP_FE(bbs_getmailnum2, NULL)
        PHP_FE(bbs_getmails, NULL)
        PHP_FE(bbs_getmailusedspace, NULL)
        PHP_FE(bbs_loadmaillist, NULL)
        PHP_FE(bbs_changemaillist, NULL)
        PHP_FE(bbs_getwebmsg, third_arg_force_ref_1111)
        PHP_FE(bbs_sendwebmsg, fourth_arg_force_ref_0001)
        PHP_FE(bbs_sethomefile, NULL)
        PHP_FE(bbs_setmailfile, NULL)
        PHP_FE(bbs_mail_file, NULL)
        PHP_FE(bbs_update_uinfo, NULL)
        PHP_FE(bbs_createnewid,NULL)
        PHP_FE(bbs_fillidinfo,NULL)
        PHP_FE(bbs_delfile,NULL)
        PHP_FE(bbs_delmail,NULL)
        PHP_FE(bbs_normalboard,NULL)
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
    add_assoc_long(array, "flag1", user->flags[0]);
    add_assoc_long(array, "flag2", user->flags[1]);
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

static void assign_board(zval * array, struct boardheader *board, int num)
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
    RETURN_LONG(ret);
}

static void flush_buffer(buffered_output_t *output)
{
	*(output->outp) = '\0'; 
	zend_printf("%s", output->buf);
	output->outp = output->buf;
}

static int buffered_output(char *buf, size_t buflen, void *arg)
{
	buffered_output_t *output = (buffered_output_t *)arg;
	if (output->buflen <= buflen)
	{
		output->flush(output);
		zend_printf("%s", buf);
		return 0;
	}
	if ((output->buflen - (output->outp - output->buf) - 1) <= buflen) 
		output->flush(output);
	strncpy(output->outp, buf, buflen); 
	output->outp += buflen;

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
    buffered_output_t out;
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
	if ((out.buf = (char *)emalloc(outbuf_len)) == NULL)
	{
		munmap(ptr, st.st_size);
        RETURN_LONG(2);
	}
	out.outp = out.buf;
	out.buflen = outbuf_len;
	out.output = buffered_output;
	out.flush = flush_buffer;

    if (!sigsetjmp(bus_jump, 1)) 
	{
        signal(SIGBUS, sigbus);
        signal(SIGSEGV, sigbus);
		output_ansi_html(ptr, st.st_size, &out,attachlink);
    } 
    munmap(ptr, st.st_size);
	efree(out.buf);
    signal(SIGBUS, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);
    RETURN_LONG(0);
}

static PHP_FUNCTION(bbs_getboard)
{
    zval *array;
    char *boardname;
    int boardname_len;
    const struct boardheader *bh;
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
    if (array) {
        if (array_init(array) != SUCCESS)
            WRONG_PARAM_COUNT;
        assign_board(array, (struct boardheader *) bh, b_num);
    }
    RETURN_LONG(b_num);
}

static int bbs_cmpboard(const struct newpostdata *brd, const struct newpostdata *tmp)
{
    register int type = 0;

    if (!(currentuser->flags[0] & BRDSORT_FLAG)) {
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
    if (!brc_initial(currentuser->userid, ptr->name)) {
        ptr->unread = 1;
    } else {
        if (brc_unread(bptr->lastpost)) {
            ptr->unread = 1;
        }
    }
    return 1;
}

#define BOARD_COLUMNS 7

char *brd_col_names[BOARD_COLUMNS] = {
    "NAME",
    "DESC",
    "CLASS",
    "BM",
    "ARTCNT",                   /* article count */
    "UNREAD",
    "ZAPPED"
};

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
    } else {
        ZVAL_EMPTY_STRING(value);
    }
}

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

    /*
     * getting arguments 
     */
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &prefix, &plen, &yank) == FAILURE) {
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
    if ((brdnum = load_boards(newpost_buffer, prefix, 1, MAXBOARD, 1, yank_flag, NULL)) <= 0) {
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
    setbdir(mode, dirpath, board);
    total = get_num_records(dirpath, sizeof(struct fileheader));
    /* add by stiger */
	if(mode == DIR_MODE_NORMAL){
    	sprintf(dirpath1,"boards/%s/" DING_DIR,board);
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
    brc_initial(currentuser->userid, board);
    articles = emalloc(num * sizeof(struct fileheader));
    /* modified by stiger */
	if(mode == DIR_MODE_NORMAL)
    	rows = read_get_records(dirpath, dirpath1, articles, sizeof(struct fileheader), start, num);
	else
    	rows = get_records(dirpath, articles, sizeof(struct fileheader), start, num);
    for (i = 0; i < rows; i++) {
        MAKE_STD_ZVAL(element);
        array_init(element);
        flags[0] = get_article_flag(articles + i, currentuser, board, is_bm);
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
    total = get_num_records(dirpath, sizeof(struct fileheader));
    /* add by stiger */
    sprintf(dirpath,"boards/%s/" DING_DIR,bp->filename);
    total += get_num_records(dirpath, sizeof(struct fileheader));
    /* add end */
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

	setbdir(mode, dirpath, board);
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

    /* check for parameter being passed by reference */
	if (currentuser == NULL)
	{
		RETURN_LONG(0);
	}
	if ((bp = getbcache(board)) == NULL)
	{
		RETURN_LONG(0);
	}
	/*if (array_init(return_value) == FAILURE)
	{
		RETURN_LONG(0);
	}*/
	setbdir(mode, dirpath, board);
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
	  if(articles[i].id){
		flags[0] = get_article_flag(articles + i, currentuser, board, is_bm);
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

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (zend_parse_parameters(2 TSRMLS_CC, "ll", &user_num, &boardnum) != SUCCESS)
        WRONG_PARAM_COUNT;
    user = getuserbynum(user_num);
    if (user == NULL)
        RETURN_LONG(0);
    RETURN_LONG(haspostperm(user, getboard(boardnum)));
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

static PHP_FUNCTION(bbs_brcaddread)
{
	char *board;
	int blen;
    long fid;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (zend_parse_parameters(2 TSRMLS_CC, "sl", &board, &blen, &fid) != SUCCESS)
        WRONG_PARAM_COUNT;
	brc_initial(currentuser->userid, board);
	brc_add_read(fid);
	brc_update(currentuser->userid);
    /*brc_addreaddirectly(getcurrentuser()->userid, boardnum, fid);*/

    RETURN_NULL();
}

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
            RETURN_LONG(MAILBOARDNUM);  //最大值了
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
        ZVAL_STRING(z_errmsg, "无法发送讯息", 1);
        RETURN_FALSE;
    }
    if (!strcasecmp(destid, currentuser->userid)) {
        ZVAL_STRING(z_errmsg, "你不能给自己发讯息", 1);
        RETURN_FALSE;
    }
    if ((result = send_msg(currentuser->userid, get_utmpent_num(u_info), destid, destutmp, msg)) == 1) {
        ZVAL_STRING(z_errmsg, "已经帮你送出讯息", 1);
        RETURN_TRUE;
    } else if (result == -1) {
        char buf[STRLEN];

        snprintf(buf, sizeof(buf), "发送讯息失败，%s", msgerr);
        ZVAL_STRING(z_errmsg, buf, 1);
        RETURN_FALSE;
    } else {
        ZVAL_STRING(z_errmsg, "发送讯息失败，此人目前不在线或者无法接收讯息", 1);
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
    getcwd(old_pwd, 1023);
    old_pwd[1023] = 0;
    chdir(BBSHOME);
    resolve_ucache();
    resolve_utmp();
    resolve_boards();
    init_bbslog();
    www_data_init();
#ifdef SQUID_ACCL
    REGISTER_LONG_CONSTANT("SQUID_ACCL", 1, CONST_CS | CONST_PERSISTENT);
#else
    REGISTER_LONG_CONSTANT("SQUID_ACCL", 0, CONST_CS | CONST_PERSISTENT);
#endif
    REGISTER_LONG_CONSTANT("BBS_PERM_POSTMASK", PERM_POSTMASK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_NOZAP", PERM_NOZAP, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_PERM_CLOAK", PERM_CLOAK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("BBS_BOARD_ATTACH", BOARD_ATTACH, CONST_CS | CONST_PERSISTENT);
    chdir(old_pwd);
#ifdef DEBUG
    zend_error(E_WARNING, "module init");
#endif
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(smth_bbs)
{
    www_data_detach();
    detach_utmp();
    detach_boards();
    detach_ucache();
    detach_publicshm();

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
#ifdef DEBUG
    zend_error(E_WARNING, "request init:%d %x", getpid(), getcurrentuinfo);
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
 * check ID is valid
 *  rototype:
 * int bbs_createNewID(string smthid,string passwd);
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
	char buf[1024];
	char tmpstr[30];
	struct stat lst;
	time_t lnow;
	struct userec newuser;
	int allocid;

    int ac = ZEND_NUM_ARGS();

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ss", &userid, &userid_len,&passbuf,&passbuf_len) == FAILURE) {
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
	//存在前人的目录,并且还在保存期限以内
	if (!stat(buf,&lst) && S_ISDIR(lst.st_mode) && (lnow-lst.st_ctime < SEC_DELETED_OLDHOME ))
	{
		//log?

		RETURN_LONG(10);
	}

	memset(&newuser,0,sizeof(newuser));
	strncpy(newuser.userid ,userid,IDLEN);
	newuser.firstlogin = newuser.lastlogin = time(NULL);

	setpasswd(passbuf,&newuser);

	newuser.userlevel = PERM_BASIC;
	newuser.userdefine = -1;
	newuser.userdefine &= ~DEF_NOTMSGFRIEND;
	newuser.notemode = -1;
	newuser.exittime = time(NULL) - 100;
	newuser.flags[0] = CURSOR_FLAG;
	newuser.flags[0] |= PAGER_FLAG;
	newuser.flags[1] = 0;

	//分配ID号
	allocid = getnewuserid2(newuser.userid);
	if (allocid > MAXUSERS || allocid <= 0) RETURN_LONG(10);

	//更新共享内存数据
	update_user(&newuser,allocid,1);

	if (!getuser(newuser.userid,&currentuser))RETURN_LONG(10);

	bbslog("user","%s","new account from tsinghua www");


	//检查是否有前人的信件
	sethomepath(tmpstr,userid);
	sprintf(buf,"/bin/mv -f %s " BBSHOME "/homeback/%s",tmpstr,userid);
	system(buf);
	setmailpath(tmpstr,userid);
	sprintf(buf,"/bin/mv -f %s " BBSHOME "/mailback/%s",tmpstr,userid);
	system(buf);

	//创建新目录
	sethomepath(tmpstr,userid);
	if(mkdir(tmpstr,0755) < 0)RETURN_LONG(10);

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
    if(read_userdata(userid,&ud) < 0)RETURN_LONG(-2);

    strncpy(ud.realname, realname, NAMELEN);
    strncpy(ud.address,dept,STRLEN);
    sprintf(genbuf,"%s#%s#%s#TH",realname,number,dept);
    if(strlen(genbuf) >= STRLEN - 16) //too long
		sprintf(genbuf,"%s#%s#TH",realname,number);//must < STRLEN - 16
    strncpy(ud.realemail,genbuf,STRLEN-16);

    if(write_userdata(userid,&ud) < 0)RETURN_LONG(-2);

    RETURN_LONG(0);
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

	setbdir(DIR_MODE_NORMAL, dir, board);
	setbfile(path, board, file);
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
			if(del_post(num + 1, &f, dir, board) == DONOTHING)
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

