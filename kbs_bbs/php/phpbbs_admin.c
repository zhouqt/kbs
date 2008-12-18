#include "php_kbs_bbs.h"

/* kbs admin function for web
 * pig2532
 */

#define WWW_SECURITY_REPORT_SUFFIX "(Web)"

/* long bbs_admin_getnewreg(array reglist);
 */
PHP_FUNCTION(bbs_admin_getnewreg)
{

    int ac;
    long i;
    zval *reglist, *element;
    FILE *fp;
    char buf[256], *p;

    ac = ZEND_NUM_ARGS();
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "a", &reglist) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (array_init(reglist) != SUCCESS) {
        RETURN_LONG(-1);
    }

    if ((fp = fopen("new_register", "r")) == NULL) {
        RETURN_LONG(0);
    }
    MAKE_STD_ZVAL(element);
    array_init(element);
    i = 0;
    while (fgets(buf, 256, fp) != NULL) {
        if (strncmp(buf, "----", 4) == 0) {
            zend_hash_index_update(Z_ARRVAL_P(reglist), i, (void*)&element, sizeof(zval*), NULL);
            i++;
            MAKE_STD_ZVAL(element);
            array_init(element);
        } else {
            p = strchr(buf, ':');
            if (p == NULL)
                continue;
            *p = '\0';
            add_assoc_string(element, buf, p + 1, 1);
        }
    }
    fclose(fp);
    RETURN_LONG(i);
}


/* long bbs_admin_getuserinfo(string userid, array userinfo);
 */
PHP_FUNCTION(bbs_admin_getuserinfo)
{
    int ac, userid_len, uid;
    char *userid;
    zval *userinfo;
    struct userec *user;

    ac = ZEND_NUM_ARGS();
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sa", &userid, &userid_len, &userinfo) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    uid = getuser(userid, &user);
    if (!uid) {
        RETURN_LONG(-1);
    }
    if (array_init(userinfo) != SUCCESS) {
        RETURN_LONG(0);
    }
    assign_user(userinfo, user, uid);
    RETURN_LONG((long)uid);
}


/* long bbs_admin_setuserinfo(string userid, string username, string realname, string address, string email, long gender, long birthyear, long birthmonth, long birthday, long title, string realemail, long numlogins, long numposts, long firstlogin, long lastlogin);
 */
PHP_FUNCTION(bbs_admin_setuserinfo)
{
    int ac;
    char *userid, *username, *realname, *address, *email, *realemail;
    int userid_len, username_len, realname_len, address_len, email_len, realemail_len;
    long gender, birthyear, birthmonth, birthday, title, numlogins, numposts, firstlogin, lastlogin;
    int uid;
    struct userec *user, newinfo;
    struct userdata ud;
    struct usermemo *um;
    time_t now;
    struct tm *tmnow;
    char secustr[256];

    ac = ZEND_NUM_ARGS();
    if (ac != 15 || zend_parse_parameters(15 TSRMLS_CC, "ssssslllllsllll", &userid, &userid_len, &username, &username_len, &realname, &realname_len, &address, &address_len, &email, &email_len, &gender, &birthyear, &birthmonth, &birthday, &title, &realemail, &realemail_len, &numlogins, &numposts, &firstlogin, &lastlogin) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    now = time(0);
    tmnow = localtime(&now);

    uid = getuser(userid, &user);
    memcpy(&newinfo, user, sizeof(struct userec));
    read_userdata(user->userid, &ud);
    read_user_memo(user->userid, &um);

    strncpy(newinfo.username, username, NAMELEN);
    strncpy(ud.realname, realname, NAMELEN);
    strncpy(ud.address, address, NAMELEN);
    strncpy(ud.email, email, STRLEN);
    if (gender == 77)
        ud.gender = 'M';
    else
        ud.gender = 'F';
    if (birthyear < 1900) {
        RETURN_LONG(-1);
    }
    birthyear -= 1900;
    if (birthyear < tmnow->tm_year - 98 || birthyear > tmnow->tm_year - 3) {
        RETURN_LONG(-1);    // birthyear invalid
    }
    ud.birthyear = (int)birthyear;
    if (birthmonth < 1 || birthmonth > 12) {
        RETURN_LONG(-2);    // birthmonth invalid
    }
    ud.birthmonth = (int)birthmonth;
    if (birthday < 1 || birthday > 31) {
        RETURN_LONG(-3);    // birthday invalid
    }
    ud.birthday = (int)birthday;
    if (title < 0 || title > 255) {
        RETURN_LONG(-4);    // title number invalid
    }
    newinfo.title = (unsigned char)title;
    strncpy(ud.realemail, realemail, STRLEN - 16);
    newinfo.numlogins = (int)numlogins;
    newinfo.numposts = (int)numposts;
    if (firstlogin)
        newinfo.firstlogin -= REGISTER_WAIT_TIME;
    if (lastlogin)
        newinfo.lastlogin = time(0);
    update_user(&newinfo, uid, 1);
    memcpy(&(um->ud), &ud, sizeof(struct userdata));
    write_userdata(newinfo.userid, &ud);
    end_mmapfile(um, sizeof(struct usermemo), -1);

    sprintf(secustr, "修改 %s 的基本资料" WWW_SECURITY_REPORT_SUFFIX, user->userid);
    securityreport(secustr, &newinfo, NULL, getSession());

    RETURN_LONG(0);
}

/* string bbs_admin_getusertitle(long num);
 */
PHP_FUNCTION(bbs_admin_getusertitle)
{
    int ac;
    long num;
    char *user_title;

    ac = ZEND_NUM_ARGS();
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l", &num) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    if ((num < 1) || (num > 255)) {
        RETURN_STRING("", 1);
    }
    user_title = get_user_title((unsigned char)num);
    RETURN_STRING(user_title, 1);
}

/* long bbs_admin_getuserperm(string userid);
 */
PHP_FUNCTION(bbs_admin_getuserperm)
{
    int ac;
    char *userid;
    int userid_len, uid;
    struct userec *user;

    ac = ZEND_NUM_ARGS();
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    uid = getuser(userid, &user);
    if (!uid) {
        RETURN_LONG(-1);
    } else {
        RETURN_LONG(user->userlevel);
    }
}

/* long bbs_admin_getgiveupperm(string userid);
 */
PHP_FUNCTION(bbs_admin_getgiveupperm)
{
    int ac;
    char *userid;
    int userid_len, uid, s[GIVEUPINFO_PERM_COUNT];
    struct userec *user;
    long giveupperm;

    ac = ZEND_NUM_ARGS();
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    uid = getuser(userid, &user);
    if (!uid) {
        RETURN_LONG(-1);
    }
    giveupperm = get_giveupinfo(user, s);
    RETURN_LONG(giveupperm);
}

/* long bbs admin_setuserperm(string userid, long perm);
 */
PHP_FUNCTION(bbs_admin_setuserperm)
{
    int ac;
    char *userid;
    int userid_len, uid, s[GIVEUPINFO_PERM_COUNT];
    struct userec *user;
    long perm;
    int giveupperm;
    bool flag1, flag2;
    char secustr[256];

    ac = ZEND_NUM_ARGS();
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &userid, &userid_len, &perm) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    uid = getuser(userid, &user);
    if (!uid) {
        RETURN_LONG(-1);
    } else {
        giveupperm = get_giveupinfo(user, s);
        flag1 = (user->userlevel & PERM_CLOAK) == 0;
        flag2 = (user->userlevel & PERM_XEMPT) == 0;
        user->userlevel = (unsigned int)perm;
        if ((user->userlevel & PERM_CLOAK) && flag1)
            mail_file(getCurrentUser()->userid, "etc/forcloak", user->userid, NAME_SYSOP_GROUP "授予您隐身权限", BBSPOST_LINK, NULL);
        if ((user->userlevel & PERM_XEMPT) && flag2)
            mail_file(getCurrentUser()->userid, "etc/forlongid", user->userid, NAME_SYSOP_GROUP "授予您长期帐号权限", BBSPOST_LINK, NULL);
        save_giveupinfo(user, s);

        sprintf(secustr, "修改 %s 的权限" WWW_SECURITY_REPORT_SUFFIX, user->userid);
        securityreport(secustr, user, NULL, getSession());
        sprintf(secustr, "changed permissions for %s", user->userid);
        bbslog("user", "%s", secustr);

        RETURN_LONG(0);
    }
}

/* long bbs_admin_newboard(string boardname);
 */
PHP_FUNCTION(bbs_admin_newboard)
{
    int ac, bid;
    char *boardname, vpath[PATHLEN], bpath[PATHLEN], secustr[STRLEN];
    int boardname_len;
    struct boardheader newbh;

    ac = ZEND_NUM_ARGS();
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &boardname, &boardname_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    memset(&newbh, 0, sizeof(struct boardheader));
    if (!boardname) {
        RETURN_LONG(-1);
    }
    if (boardname[0] == '\0') {
        RETURN_LONG(-1);
    }
    if (!valid_brdname(boardname)) {
        RETURN_LONG(-2);
    }
    strncpy(newbh.filename, boardname, BOARDNAMELEN);
    strcpy(newbh.title, "0[待定]      版面中文名称待定");
    sprintf(vpath, "vote/%s", boardname);
    setbpath(bpath, boardname);
    if (getbid(boardname, NULL) > 0) {
        RETURN_LONG(-3);
    }
    if ((mkdir(bpath, 0755) == -1) || (mkdir(vpath, 0755) == -1)) {
        RETURN_LONG(-4);
    }
#ifndef PERM_NEWBOARD
    newbh.level = PERM_SYSOP;
#else
    newbh.level = PERM_NEWBOARD;
#endif
    if (!HAS_PERM(getCurrentUser(), newbh.level))
        newbh.level = 0;
    build_board_structure(boardname);
    snprintf(newbh.ann_path, 127, "%s/%s", groups[0], boardname);
    newbh.ann_path[127]=0;
    if (add_board(&newbh) == -1) {
        RETURN_LONG(-5);
    }
    bid = getbid(boardname, NULL);
    if (bid == 0) {
        RETURN_LONG(-6);
    }
    edit_group(NULL, &newbh);
    sprintf(secustr, "add brd %s", boardname);
    bbslog("user", "%s", secustr);
    sprintf(secustr, "成立新版：%s" WWW_SECURITY_REPORT_SUFFIX, boardname);
    securityreport(secustr, NULL, NULL, getSession());
    RETURN_LONG(0);
}

/* long bbs_admin_getboardparam(string boardname, array boardparams);
 */
PHP_FUNCTION(bbs_admin_getboardparam)
{
    int ac, bid;
    char *boardname;
    int boardname_len, section;
    zval *boardparams;
    struct boardheader *bp = NULL;
    unsigned int annstat;

    ac = ZEND_NUM_ARGS();
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sa", &boardname, &boardname_len, &boardparams) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    bid = getbnum_safe(boardname, getSession(), 1);
    if (!bid) {
        RETURN_LONG(-1);
    }
    bp = (struct boardheader *)getboard(bid);
    if (bp == NULL) {
        RETURN_LONG(-2);
    }

    if (array_init(boardparams) != SUCCESS) {
        RETURN_LONG(-3);
    }
    add_assoc_long(boardparams, "BID", bid);
    add_assoc_string(boardparams, "FILENAME", bp->filename, 1);
    add_assoc_string(boardparams, "BM", bp->BM, 1);
    add_assoc_string(boardparams, "TITLE", bp->title, 1);
    add_assoc_long(boardparams, "LEVEL", bp->level);
    add_assoc_long(boardparams, "CLUBNUM", bp->clubnum);
    add_assoc_long(boardparams, "FLAG", bp->flag);
    add_assoc_long(boardparams, "SCORELEVEL", bp->score_level);
    add_assoc_string(boardparams, "ANNPATH", bp->ann_path, 1);
    annstat = check_ann(bp);
    section = (annstat & 0x020000) ? -1 : (annstat & 0xFFFF);
    add_assoc_long(boardparams, "ANNPATH_SECTION", section);
    add_assoc_long(boardparams, "ANNPATH_STATUS", !(annstat&~0xFFFF) ? 0 : ((annstat & 0x040000) ? 1 : 2));
    add_assoc_long(boardparams, "GROUP", bp->group);
    add_assoc_long(boardparams, "TITLELEVEL", bp->title_level);
    add_assoc_string(boardparams, "DES", bp->des, 1);
    RETURN_LONG(0);
}

/* long bbs_admin_setboardparam(string boardname, string filename, string bm, string title, string des, long flag, string parentbname, long annpath_section, long level, long title_level);
 */
PHP_FUNCTION(bbs_admin_setboardparam)
{
    int ac, boardname_len, filename_len, bm_len, title_len, des_len, parentbname_len;
    char *boardname, *filename, *bm, *title, *des, *parentbname;
    long flag, annpath_section, level, title_level;
    struct boardheader *bp, newbh;
    int bid, parentbid;

    ac = ZEND_NUM_ARGS();
    if (ac != 10 || zend_parse_parameters(10 TSRMLS_CC, "ssssslslll", &boardname, &boardname_len,
                                          &filename, &filename_len, &bm, &bm_len, &title, &title_len, &des, &des_len, &flag,
                                          &parentbname, &parentbname_len, &annpath_section, &level, &title_level) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    bid = getbid(boardname, (const struct boardheader **)&bp);
    if (!bid) {
        // 所修改的版面不存在
        RETURN_LONG(-1);
    }
    memcpy(&newbh, bp, sizeof(struct boardheader));

    if (strcasecmp(boardname, filename) && (getbid(filename, NULL) > 0)) {
        // 有同名版面
        RETURN_LONG(-2);
    }
    if (strchr(filename, '/') || strchr(filename, ' ')) {
        // 版面名称非法
        RETURN_LONG(-3);
    }
    strcpy(newbh.filename, filename);
    strncpy(newbh.BM, bm, BM_LEN - 1);
    strncpy(newbh.title, title, STRLEN - 1);
    strncpy(newbh.des, des, 194);
    newbh.flag = (unsigned int)flag;
    if (parentbname[0] != '\0') {
        parentbid = getbnum_safe(parentbname, getSession(), 1);
        if (!parentbid) {
            // 所属目录不存在
            RETURN_LONG(-4);
        } else if (!(getboard(parentbid)->flag & BOARD_GROUP)) {
            // 所属目录不是目录讨论区
            RETURN_LONG(-5);
        }
        newbh.group = parentbid;
    } else
        newbh.group = 0;
    if ((annpath_section < 0) || (annpath_section >= SECNUM)) {
        // 精华区分区不存在
        RETURN_LONG(-6);
    }
    snprintf(newbh.ann_path, 127, "%s/%s", groups[annpath_section], newbh.filename);
    newbh.level = level;
    if ((title_level < 0) || (title_level > 255)) {
        // 身份不存在
        RETURN_LONG(-7);
    }
    if (title_level != 0)
        if (!*get_user_title((unsigned int)title_level)) {
            // 身份不存在
            RETURN_LONG(-7);
        }
    newbh.title_level = title_level;

    // 版面名称修改过
    if (strcmp(bp->filename, newbh.filename)) {
        char spath[PATHLEN], dpath[PATHLEN];
        sprintf(spath, "boards/%s", bp->filename);
        sprintf(dpath, "boards/%s", newbh.filename);
        if (dashd(dpath))
            my_f_rm(dpath);
        if (dashd(spath))
            rename(spath, dpath);
        else {
            mkdir(dpath, 0755);
            build_board_structure(newbh.filename);
        }
        sprintf(spath, "vote/%s", bp->filename);
        sprintf(dpath, "vote/%s", newbh.filename);
        if (dashd(dpath))
            my_f_rm(dpath);
        if (dashd(spath))
            rename(spath, dpath);
    }
    edit_group(bp, &newbh);
    set_board(bid, &newbh, bp);

    // TODO: auto post syssecurity log

    newbbslog(BBSLOG_USER, "edit_board: %s <%4.4d>", bp->filename, bid);
    // 修改成功
    RETURN_LONG(0);
}

