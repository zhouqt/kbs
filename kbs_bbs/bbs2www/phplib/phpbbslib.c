#include <php.h>
#include "bbs.h"
#include "bbslib.h"

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
static ZEND_FUNCTION(bbs_wwwlogoff);
static ZEND_FUNCTION(bbs_printansifile);
static ZEND_FUNCTION(bbs_getboard);
static ZEND_FUNCTION(bbs_checkreadperm);
static ZEND_FUNCTION(bbs_brcaddread);
static ZEND_FUNCTION(bbs_ann_traverse_check);
static ZEND_FUNCTION(bbs_ann_get_board);
static ZEND_FUNCTION(bbs_getboards);

static ZEND_MINIT_FUNCTION(bbs_module_init);
static ZEND_MSHUTDOWN_FUNCTION(bbs_module_shutdown);
static ZEND_RINIT_FUNCTION(bbs_request_init);
static ZEND_RSHUTDOWN_FUNCTION(bbs_request_shutdown);

static unsigned char a2_arg_force_ref[] = { 2, BYREF_NONE, BYREF_FORCE };

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
	ZEND_FE(bbs_wwwlogoff, NULL)
	ZEND_FE(bbs_printansifile, NULL)
	ZEND_FE(bbs_checkreadperm, NULL)
	ZEND_FE(bbs_brcaddread, NULL)
	ZEND_FE(bbs_getboard, NULL)
	ZEND_FE(bbs_ann_traverse_check, NULL)
	ZEND_FE(bbs_ann_get_board, NULL)
	/*ZEND_FE(bbs_getboards, a2_arg_force_ref)*/
	ZEND_FE(bbs_getboards, NULL)
	{NULL, NULL, NULL}
};

/*
 * This is the module entry structure, and some properties
 */

static zend_module_entry bbs_php_module_entry = {
    STANDARD_MODULE_HEADER,
    "bbs module",               /* extension name */
    bbs_php_functions,          /* extension function list */
    ZEND_MINIT(bbs_module_init),        /* extension-wide startup function */
    ZEND_MSHUTDOWN(bbs_module_shutdown),        /* extension-wide shutdown function */
    ZEND_RINIT(bbs_request_init),       /* per-request startup function */
    ZEND_RSHUTDOWN(bbs_request_shutdown),       /* per-request shutdown function */
    NULL,                       /* information function */
    "1.0",
    STANDARD_MODULE_PROPERTIES
};

/*
 * Here is the function require when the module loaded
 */
DLEXPORT zend_module_entry *get_module()
{
    return &bbs_php_module_entry;
};


static void setstrlen(pval * arg)
{
    arg->value.str.len = strlen(arg->value.str.val);
}

static void assign_user(zval * array, struct userec *user, int num)
{
    add_assoc_long(array, "index", num);
    add_assoc_string(array, "userid", user->userid, 1);
    add_assoc_long(array, "firstlogin", user->firstlogin);
    add_assoc_stringl(array, "lasthost", user->lasthost, IPLEN, 1);
    add_assoc_long(array, "numlogins", user->numlogins);
    add_assoc_long(array, "numposts", user->numposts);
    add_assoc_long(array, "flag1", user->flags[0]);
    add_assoc_long(array, "flag2", user->flags[1]);
    add_assoc_string(array, "username", user->username, 1);
    add_assoc_string(array, "ident", user->ident, 1);
    add_assoc_stringl(array, "md5passwd", (char *) user->md5passwd, 16, 1);
    add_assoc_string(array, "realemail", user->realemail, 1);
    add_assoc_long(array, "userlevel", user->userlevel);
    add_assoc_long(array, "lastlogin", user->lastlogin);
    add_assoc_long(array, "stay", user->stay);
    add_assoc_string(array, "realname", user->realname, 1);
    add_assoc_string(array, "address", user->address, 1);
    add_assoc_string(array, "email", user->email, 1);
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
    add_assoc_long(array, "freshtime", uinfo->freshtime);
    add_assoc_long(array, "utmpkey", uinfo->utmpkey);
    add_assoc_string(array, "userid", uinfo->userid, 1);
    add_assoc_string(array, "realname", uinfo->realname, 1);
    add_assoc_string(array, "username", uinfo->username, 1);
}

static void assign_board(zval * array, struct boardheader *board, int num)
{
    add_assoc_long(array, "index", num);
    add_assoc_string(array, "filename", board->filename, 1);
    add_assoc_string(array, "owner", board->owner, 1);
    add_assoc_string(array, "BM", board->BM, 1);
    add_assoc_long(array, "flag", board->flag);
    add_assoc_string(array, "title", board->title, 1);
    add_assoc_long(array, "level", board->level);
}

static int currentusernum;
static char fullfrom[255];
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
static ZEND_FUNCTION(bbs_setfromhost)
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
    RETURN_NULL();
}

static ZEND_FUNCTION(bbs_getuser)
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

static ZEND_FUNCTION(bbs_getonlineuser)
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

static ZEND_FUNCTION(bbs_getonlinenumber)
{
    RETURN_LONG(get_utmp_number());
}


static ZEND_FUNCTION(bbs_countuser)
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

static ZEND_FUNCTION(bbs_checkpasswd)
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
    if (!(unum = getuser(s, &user)))
        ret = 2;
    else {
        if (checkpasswd2(pw, user)) {
            ret = 0;
            setcurrentuser(user, unum);
        } else {
            ret = 1;
            logattempt(user->userid, fromhost);
        }
    }
    RETURN_LONG(ret);
}

static ZEND_FUNCTION(bbs_wwwlogin)
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
    ret = www_user_login(getcurrentuser(), getcurrentuser_num(), kick_multi, fromhost,
#ifdef SQUID_ACCL
                         fullfrom,
#else
                         fromhost,
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

static ZEND_FUNCTION(bbs_getcurrentuinfo)
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

static ZEND_FUNCTION(bbs_getcurrentuser)
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

static ZEND_FUNCTION(bbs_setonlineuser)
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

    MAKE_STD_ZVAL(user_array);
    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (zend_parse_parameters(4 TSRMLS_CC, "slla", &userid, &userid_len, &utmpnum, &utmpkey, &user_array) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
    if (userid_len > IDLEN)
        RETURN_LONG(1);
    if (utmpnum < 0 || utmpnum >= MAXACTIVE)
        RETURN_LONG(2);

    if ((ret = www_user_init(utmpnum, userid, utmpkey, &user, &pui)) == 0) {
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

static ZEND_FUNCTION(bbs_printansifile)
{
    char *filename;
    long filename_len;
    long linkmode;
    char *ptr;
    int fd;
    struct stat st;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (ZEND_NUM_ARGS() == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "s", &filename, &filename_len) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
        linkmode = 1;
    } else {
        if (zend_parse_parameters(2 TSRMLS_CC, "sl", &filename, &filename_len, &linkmode) != SUCCESS) {
            WRONG_PARAM_COUNT;
        }
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

    if (!sigsetjmp(bus_jump, 1)) {
        signal(SIGBUS, sigbus);
        signal(SIGSEGV, sigbus);
        {
            char *p;
            int mode = 0;
            char outbuf[4096];
            char *outp = outbuf;
            int ansicolor, cal;

            outbuf[sizeof(outbuf) - 1] = 0;
#define FLUSHBUF { *outp=0;zend_printf("%s",outbuf); outp=outbuf; }
#define OUTPUT(buf,len) { if ((outbuf-outp)<len) FLUSHBUF; strncpy(outp,buf,len); outp+=len; }
            for (p = ptr; (*p) && (p - ptr < st.st_size); p++) {
                // TODO: need detect link
                switch (mode) {
                case 0:
                    if (*p == 0x1b) {   //ESC
                        mode = 1;
                        continue;
                    }
                    if (*p == '&')
                        OUTPUT("&amp;", 5)
                            else
                    if (*p == '<')
                        OUTPUT("&lt;", 4)
                            else
                    if (*p == '>')
                        OUTPUT("&gt;", 4)
                            else
                        break;
                    continue;
                case 1:
                    if ((*p) != '[') {
                        if (!isalpha(*p)) {
                            mode = 4;
                            continue;
                        }
                        mode = 0;
                        continue;
                    }
                    mode = 2;
                    cal = 0;
                    continue;
                case 2:
                    // TODO: add more ansi colir support
                    if (*p == ';') {
                        if (cal <= 37 && cal >= 30)
                            ansicolor = cal;
                        continue;
                    }
                    if (*p == 'm') {
                        char ansibuf[30];

                        if (cal <= 37 && cal >= 30)
                            ansicolor = cal;
                        if (ansicolor <= 37 && ansicolor >= 30) {
                            sprintf(ansibuf, "<font class=\"c%d\">", ansicolor);
                            OUTPUT(ansibuf, strlen(ansibuf));
                            mode = 0;
                            continue;
                        }
                    }
                    if (isdigit(*p)) {
                        cal = cal * 10 + (*p) - '0';
                        continue;
                    }
                    /* strange ansi escape,ignore it */
                    if (!isalpha(*p)) {
                        mode = 4;
                        continue;
                    }
                    mode = 0;
                    continue;
                case 4:
                    if (!isalpha(*p))
                        continue;
                    mode = 0;
                    continue;
                }
                *outp = *p;
                outp++;
                if (outp - outbuf >= sizeof(outbuf) - 1)
                    FLUSHBUF;
            }
            if (outp != outbuf) {
                *outp = 0;
                zend_printf("%s", outbuf);
            }
        }
    } else {
    }
    munmap(ptr, st.st_size);
    signal(SIGBUS, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);
    RETURN_LONG(0);
}

static ZEND_FUNCTION(bbs_getboard)
{
    zval *array;
    char *boardname;
    int boardname_len;
    const struct boardheader *bh;
    int b_num;

    MAKE_STD_ZVAL(array); /* Is it necessary? */
    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
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

static int
bbs_cmpboard(const struct newpostdata *brd, const struct newpostdata *tmp)
{
	register int type = 0;

	if ( !(currentuser->flags[0] & BRDSORT_FLAG) )
	{
		type = brd->title[0] - tmp->title[0];
		if (type == 0)
			type = strncasecmp(brd->title+1, tmp->title+1,6);
	}
	if ( type == 0 )
		type = strcasecmp( brd->name, tmp->name );
	return type;
}

/* TODO: move this function into bbslib. */
static int 
check_newpost( struct newpostdata *ptr)
{
	struct BoardStatus* bptr;
	ptr->total = ptr->unread = 0;

	bptr = getbstatus(ptr->pos);
	if (bptr == NULL)
		return 0;
	ptr->total = bptr->total;

	if (!brc_initial(currentuser->userid,ptr->name))
	{
		ptr->unread = 1;
	}
	else
	{
		if (brc_unread(bptr->lastpost))
		{
			ptr->unread = 1;
		}
	}
	return 1;
}

#define BOARD_COLUMNS 7

char *brd_col_names[BOARD_COLUMNS] =
{
	"NAME",
	"DESC",
	"CLASS",
	"BM",
	"ARTCNT", /* article count */
	"UNREAD",
	"ZAPPED"
};

static void
bbs_make_board_columns(zval **columns)
{
	int i;

	for (i = 0; i < BOARD_COLUMNS; i++)
	{
		MAKE_STD_ZVAL(columns[i]);
		ZVAL_STRING(columns[i], brd_col_names[i], 1);
	}
}

static void
bbs_make_board_zval(zval *value, char *col_name, struct newpostdata *brd)
{
	int len = strlen(col_name);

	if (strncmp(col_name, "ARTCNT", len) == 0)
	{
		ZVAL_LONG(value, brd->total);
	}
	else if (strncmp(col_name, "UNREAD", len) == 0)
	{
		ZVAL_LONG(value, brd->unread);
	}
	else if (strncmp(col_name, "ZAPPED", len) == 0)
	{
		ZVAL_LONG(value, brd->zap);
	}
	else if (strncmp(col_name, "CLASS", len) == 0)
	{
		ZVAL_STRINGL(value, brd->title+1, 6, 1);
	}
	else if (strncmp(col_name, "DESC", len) == 0)
	{
		ZVAL_STRING(value, brd->title+13, 1);
	}
	else if (strncmp(col_name, "NAME", len) == 0)
	{
		ZVAL_STRING(value, brd->name, 1);
	}
	else if (strncmp(col_name, "BM", len) == 0)
	{
		ZVAL_STRING(value, brd->BM, 1);
	}
	else
	{
		ZVAL_EMPTY_STRING(value);
	}
}

extern int     brdnum;
extern int yank_flag;

/**
 * Fetch all boards which have given prefix into an array.
 * prototype:
 * int bbs_getboards(char *prefix, int yank, array &output);
 * by flyriver, 2002.8.6
 *
 * @return >=0 number of fetched boards
 *         <0  failure
 */
static ZEND_FUNCTION(bbs_getboards)
{
	/*
	 * TODO: The name of "yank" must be changed, this name is totally
	 * shit, but I don't know which name is better this time.
	 */
	char *prefix;
	int plen;
	int yank;
	zval *brdarrs;
	int rows = 0;
	struct newpostdata newpost_buffer[MAXBOARD];
	struct newpostdata *ptr;
	zval **columns;
	zval *element;
	int i;
	int j;
	int ac = ZEND_NUM_ARGS();

	/* getting arguments */
#if 0
	if (ac != 3 
		|| zend_parse_parameters(3 TSRMLS_CC, "sla", &prefix, &plen, &yank, &brdarrs) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}
#endif
	if (ac != 2 
		|| zend_parse_parameters(2 TSRMLS_CC, "sl", &prefix, &plen, &yank) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}
	/*array_init(brdarrs);*/
	if (array_init(return_value) == FAILURE)
	{
		RETURN_FALSE;
	}

	/* loading boards */
	/* handle some global variables: currentuser, yank, brdnum, 
	 * boardprefix, nbrd.
	 */
	/* NOTE: currentuser SHOULD had been set in funcs.php, 
	 * but we still check it. */
	if (currentuser == NULL)
	{
		/*RETURN_LONG(-1);*/
		RETURN_FALSE;
	}
	yank_flag = yank;
	if (strcmp(currentuser->userid, "guest") == 0)
		yank_flag = 1; /* see all boards including zapped boards. */
	if (yank_flag != 0)
		yank_flag = 1;
	nbrd = newpost_buffer;
	brdnum = 0;
	/* TODO: replace load_board() with a new one, without accessing
	 * global variables. */
	if (load_boards(prefix) < 0)
	{
		/*RETURN_LONG(-1);*/
		RETURN_FALSE;
	}
	qsort( nbrd, brdnum, sizeof( nbrd[0] ), 
		   (int (*)(const void *, const void *))bbs_cmpboard );
	rows = brdnum; /* number of loaded boards */

	/* fill data in output array. */
	/* setup column names */
	columns = emalloc(BOARD_COLUMNS * sizeof(zval*));
	for (i = 0; i < BOARD_COLUMNS; i++)
	{
		MAKE_STD_ZVAL(element);
		array_init(element);
		columns[i] = element;
		/*zend_hash_update(Z_ARRVAL_P(brdarrs), */
		zend_hash_update(Z_ARRVAL_P(return_value), 
				brd_col_names[i], strlen(brd_col_names[i])+1,
				(void *) &element, sizeof(zval *), NULL);
	}
	/* fill data for each column */
	for (i = 0; i < rows; i++)
	{
		ptr = &nbrd[i];
		check_newpost(ptr);
		for (j = 0; j < BOARD_COLUMNS; j++)
		{
			MAKE_STD_ZVAL(element);
			bbs_make_board_zval(element, brd_col_names[j], ptr);
			/*
			 * 首先，取得 outarrs[i]，为一个 zval** 型指针；
			 * 然后，施行 *(outarrs[i])，得到 zval* 型指针(实际上指向
			 *       一个哈希表，该哈希表元素的数据类型为 zval*)；
			 * 最后，通过 ->value.ht 得到该哈希表的起始地址。
			 * 由于哈希表中存放的是 zval* 型元素，所以必须传入 element
			 * 的指针和 element 这个指针本身所占用的空间长度，这就是
			 * &element 和 sizeof(zval *) 的来历。
			 */
			zend_hash_index_update(Z_ARRVAL_P(columns[j]), i,
				   	(void*) &element, sizeof(zval*), NULL);
		}
	}
	efree(columns);
	/*RETURN_LONG(rows);*/
}

static ZEND_FUNCTION(bbs_checkreadperm)
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
    RETURN_LONG(check_read_perm(user, boardnum));
}

static ZEND_FUNCTION(bbs_wwwlogoff)
{
    if (getcurrentuser()) {
        int ret = (www_user_logoff(getcurrentuser(), getcurrentuser_num(),
                                   getcurrentuinfo(), getcurrentuinfo_num()));

        RETURN_LONG(ret);
    } else
        RETURN_LONG(-1);
}

static ZEND_FUNCTION(bbs_brcaddread)
{
    long posttime, boardnum;

    getcwd(old_pwd, 1023);
    chdir(BBSHOME);
    old_pwd[1023] = 0;
    if (zend_parse_parameters(2 TSRMLS_CC, "ll", &boardnum, &posttime) != SUCCESS)
        WRONG_PARAM_COUNT;
    brc_addreaddirectly(getcurrentuser()->userid, boardnum, posttime);

    RETURN_NULL();
}

static ZEND_FUNCTION(bbs_ann_traverse_check)
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

static ZEND_FUNCTION(bbs_ann_get_board)
{
    char *path, *board;
    int path_len, board_len;

    if (zend_parse_parameters(1 TSRMLS_CC, "ss", &path, &path_len, &board, &board_len) != SUCCESS)
        WRONG_PARAM_COUNT;
    RETURN_LONG(ann_get_board(path, board, board_len));
}

static ZEND_MINIT_FUNCTION(bbs_module_init)
{
    zval *bbs_home;
    zval *bbs_full_name;

    MAKE_STD_ZVAL(bbs_home);
    ZVAL_STRING(bbs_home, BBSHOME, 1);
    MAKE_STD_ZVAL(bbs_full_name);
    ZVAL_STRING(bbs_full_name, BBS_FULL_NAME, 1);
    ZEND_SET_SYMBOL(&EG(symbol_table), "BBS_HOME", bbs_home);
    ZEND_SET_SYMBOL(&EG(symbol_table), "BBS_FULL_NAME", bbs_full_name);
    getcwd(old_pwd, 1023);
    old_pwd[1023] = 0;
    chdir(BBSHOME);
    resolve_ucache();
    resolve_utmp();
    resolve_boards();
    www_data_init();
#ifdef SQUID_ACCL
    REGISTER_MAIN_LONG_CONSTANT("SQUID_ACCL", 1, CONST_CS | CONST_PERSISTENT);
#else
    REGISTER_MAIN_LONG_CONSTANT("SQUID_ACCL", 0, CONST_CS | CONST_PERSISTENT);
#endif
    REGISTER_MAIN_LONG_CONSTANT("BBS_PERM_POSTMASK", PERM_POSTMASK, CONST_CS | CONST_PERSISTENT);
    REGISTER_MAIN_LONG_CONSTANT("BBS_PERM_NOZAP", PERM_NOZAP, CONST_CS | CONST_PERSISTENT);
    chdir(old_pwd);
#ifdef DEBUG
    zend_error(E_WARNING, "module init");
#endif
    return SUCCESS;
}

static ZEND_MSHUTDOWN_FUNCTION(bbs_module_shutdown)
{
    detach_utmp();
#ifdef DEBUG
    zend_error(E_WARNING, "module shutdown");
#endif
    return SUCCESS;
}

static ZEND_RINIT_FUNCTION(bbs_request_init)
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

static ZEND_RSHUTDOWN_FUNCTION(bbs_request_shutdown)
{
#ifdef DEBUG
    zend_error(E_WARNING, "request shutdown");
#endif
    chdir(old_pwd);
    currentuser = NULL;
    return SUCCESS;
}
