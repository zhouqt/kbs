#include "php_kbs_bbs.h"

/* kbs admin function for web
 * pig2532
 */


/* bbs_admin_getnewreg(array reglist);
 */
PHP_FUNCTION(bbs_admin_getnewreg) {

    int ac;
    long i;
    zval *reglist, *element;
    FILE *fp;
    char buf[256], *p;

    ac = ZEND_NUM_ARGS();
    if(ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "a", &reglist) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if(array_init(reglist) != SUCCESS) {
        RETURN_LONG(-1);
    }

    if((fp = fopen("new_register", "r")) == NULL) {
        RETURN_LONG(0);
    }
    MAKE_STD_ZVAL(element);
    array_init(element);
    i = 0;
    while(fgets(buf, 256, fp) != NULL) {
        if(strncmp(buf, "----", 4) == 0) {
            zend_hash_index_update(Z_ARRVAL_P(reglist), i, (void*)&element, sizeof(zval*), NULL);
            i++;
            MAKE_STD_ZVAL(element);
            array_init(element);
        }
        else {
            p = strchr(buf, ':');
            if(p == NULL)
                continue;
            *p = '\0';
            add_assoc_string(element, buf, p + 1, 1);
        }
    }
    fclose(fp);
    RETURN_LONG(i);
}


/* bbs_admin_getuserinfo(string userid, array userinfo);
 */
PHP_FUNCTION(bbs_admin_getuserinfo) {
    int ac, userid_len, uid;
    char *userid;
    zval *userinfo;
    struct userec *user;

    ac = ZEND_NUM_ARGS();
    if(ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sa", &userid, &userid_len, &userinfo) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    uid = getuser(userid, &user);
    if(!uid) {
        RETURN_LONG(-1);
    }
    if(array_init(userinfo) != SUCCESS) {
        RETURN_LONG(0);
    }
    assign_user(userinfo, user, uid);
    RETURN_LONG((long)uid);
}


/* bbs_admin_setuserinfo(string userid, string username, string realname, string address, string email, long gender, long birthyear, long birthmonth, long birthday, long title, string realemail, long numlogins, long numposts, long firstlogin, long lastlogin);
 */
PHP_FUNCTION(bbs_admin_setuserinfo) {
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

    ac = ZEND_NUM_ARGS();
    if(ac != 15 || zend_parse_parameters(15 TSRMLS_CC, "ssssslllllsllll", &userid, &userid_len, &username, &username_len, &realname, &realname_len, &address, &address_len, &email, &email_len, &gender, &birthyear, &birthmonth, &birthday, &title, &realemail, &realemail_len, &numlogins, &numposts, &firstlogin, &lastlogin) == FAILURE) {
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
    if(gender == 77)
        ud.gender = 'M';
    else
        ud.gender = 'F';
    if(birthyear < 1900) {
        RETURN_LONG(-1);
    }
    birthyear -= 1900;
    if(birthyear < tmnow->tm_year - 98 || birthyear > tmnow->tm_year - 3) {
        RETURN_LONG(-1);    // birthyear invalid
    }
    ud.birthyear = (int)birthyear;
    if(birthmonth < 1 || birthmonth > 12) {
        RETURN_LONG(-2);    // birthmonth invalid
    }
    ud.birthmonth = (int)birthmonth;
    if(birthday < 1 || birthday > 31) {
        RETURN_LONG(-3);    // birthday invalid
    }
    ud.birthday = (int)birthday;
    if(title < 0 || title > 255) {
        RETURN_LONG(-4);    // title number invalid
    }
    newinfo.title = (unsigned char)title;
    strncpy(ud.realemail, realemail, STRLEN - 16);
    newinfo.numlogins = (int)numlogins;
    newinfo.numposts = (int)numposts;
    if(firstlogin)
        newinfo.firstlogin -= REGISTER_WAIT_TIME;
    if(lastlogin)
        newinfo.lastlogin = time(0);
    update_user(&newinfo, uid, 1);
    memcpy(&(um->ud), &ud, sizeof(struct userdata));
    write_userdata(newinfo.userid, &ud);
    end_mmapfile(um, sizeof(struct usermemo), -1);
    
    RETURN_LONG(0);
}

