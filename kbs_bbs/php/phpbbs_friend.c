#include "php_kbs_bbs.h"  

/*
 * stiger: getfriends
 */
PHP_FUNCTION(bbs_getfriends)
{
    char *userid;
    int userid_len;
	struct friends fr;
    int ac = ZEND_NUM_ARGS();
	long start;
	int fd;
	int i=0;
	char fpath[STRLEN];
    zval *element;

    if (ac != 2 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "sl", &userid, &userid_len, &start) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (userid_len > IDLEN)
        WRONG_PARAM_COUNT;

    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }

	i=0;
	sethomefile(fpath, userid, "friends");

	if( (fd=open(fpath, O_RDONLY)) < 0 )
        RETURN_FALSE;
	lseek(fd, sizeof(struct friends)*start, SEEK_CUR);
    while (read(fd, &fr, sizeof(fr)) > 0) {

        MAKE_STD_ZVAL(element);
        array_init(element);

    	add_assoc_string(element, "ID", fr.id, 1);
    	add_assoc_string(element, "EXP", fr.exp, 1);

        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);

		i++;
		if( i>=20)
			break;
    }
    close(fd);
}




 
/*
 * stiger: countfriends
 */
PHP_FUNCTION(bbs_countfriends)
{
    char *userid;
    int userid_len;
    int ac = ZEND_NUM_ARGS();
	char fpath[STRLEN];
	struct stat st;

    if (ac != 1 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "s", &userid, &userid_len) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (userid_len > IDLEN)
        WRONG_PARAM_COUNT;

	sethomefile(fpath, userid, "friends");

    if (stat(fpath, &st) < 0)
        RETURN_FALSE;

	RETURN_LONG(st.st_size / sizeof(struct friends));
}

static int cmpfnames2(char *userid, struct friends *uv)
{
    return !strcasecmp(userid, uv->id);
}

PHP_FUNCTION(bbs_delete_friend)
{
    char *userid;
    int userid_len;
    int ac = ZEND_NUM_ARGS();
    char buf[STRLEN];
    struct friends fh;
	int deleted;

    if (ac != 1 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "s", &userid, &userid_len) == FAILURE) {
        WRONG_PARAM_COUNT;
	}

    sethomefile(buf, getCurrentUser()->userid, "friends");

    deleted = search_record(buf, &fh, sizeof(fh), (RECORD_FUNC_ARG)cmpfnames2, userid);

    if (deleted > 0) {
        if (delete_record(buf, sizeof(fh), deleted, NULL, NULL) != -1){
			getfriendstr(getCurrentUser(),getSession()->currentuinfo,getSession());
			RETURN_LONG(0);
		} else {
			RETURN_LONG(3);
        }
    } else{
		RETURN_LONG(2);
	}
}



PHP_FUNCTION(bbs_add_friend)
{
    char *userid;
    int userid_len;
	char *exp;
	int exp_len;
    int ac = ZEND_NUM_ARGS();
    char buf[STRLEN];
    struct friends fh;
	struct userec *lookupuser;
	int n;

    if (ac != 2 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "ss", &userid, &userid_len, &exp, &exp_len) == FAILURE) {
        WRONG_PARAM_COUNT;
	}

    if (userid_len == 0) RETURN_LONG(-4);

    memset(&fh, 0, sizeof(fh));
    sethomefile(buf, getCurrentUser()->userid, "friends");

    if ((!HAS_PERM(getCurrentUser(), PERM_SYSOP))
        && (get_num_records(buf, sizeof(struct friends)) >= MAXFRIENDS)) {
		RETURN_LONG(-1);
    }

	if(! getuser(userid,&lookupuser))
		RETURN_LONG(-4);

    n = search_record(buf, &fh, sizeof(fh), (RECORD_FUNC_ARG)cmpfnames2, lookupuser->userid);
    if (n > 0)
		RETURN_LONG(-2);

    strcpy(fh.id, lookupuser->userid);
    strncpy(fh.exp, exp, sizeof(fh.exp)-1);
    fh.exp[sizeof(fh.exp)-1] = '\0';

    n = append_record(buf, &fh, sizeof(friends_t));
    getfriendstr(getCurrentUser(),getSession()->currentuinfo,getSession());
    if (n != -1)
		RETURN_LONG(0);

	RETURN_LONG(-3);
}





static int cmpuser(const void *a1, const void *b1)
{
    char id1[80], id2[80];
    uinfo_t *a, *b;
    a = (uinfo_t *)a1;
    b = (uinfo_t *)b1;
    
    sprintf(id1, "%d%s", !isfriend(a->userid), a->userid);
    sprintf(id2, "%d%s", !isfriend(b->userid), b->userid);
    return strcasecmp(id1, id2);
}

typedef struct _frienduserlistarg{
    int count;
    uinfo_t** user_record;
} frienduserlistarg;

static int full_utmp_friend(struct user_info *uentp, frienduserlistarg *pful)
{
    if (!uentp->active || !uentp->pid) {
        return 0;
    }
    if (!HAS_PERM(getCurrentUser(), PERM_SEECLOAK) && uentp->invisible && strcmp(uentp->userid, getCurrentUser()->userid)) {  /*Haohmaru.99.4.24.让隐身者能看见自己 */
        return 0;
    }
    if (!myfriend(uentp->uid, NULL, getSession())) {
        return 0;
    }
    if (pful->count < MAXFRIENDS) {
        pful->user_record[pful->count] = uentp;
        pful->count++;
    }
    return COUNT;
}

static int fill_friendlist(int* range, uinfo_t** user_record)
{
    int i;
    frienduserlistarg ful;
    struct user_info *u;

    ful.count = 0;
    ful.user_record = user_record;
    u = getSession()->currentuinfo;
    for (i = 0; i < u->friendsnum; i++) {
        if (u->friends_uid[i])
            apply_utmpuid((APPLY_UTMP_FUNC) full_utmp_friend, u->friends_uid[i], &ful);
    }
    *range = ful.count;
    return ful.count == 0 ? -1 : 1;
}

/**
 *  Function: 返回当前在线好友名单
 *   user_info bbs_getonlinefriends();
 *
 *  Return: user_info 结构数组 
 *  by binxun
 */
PHP_FUNCTION(bbs_getonlinefriends)
{
    int i = 0,total = 0;
    uinfo_t* x;
    uinfo_t user[MAXFRIENDS]; //local copy from shm, for sorting, etc.
    zval* element;
    int range;
    uinfo_t *usr[MAXFRIENDS]; //指向共享内存内容
    struct userec *lookupuser;

    int ac = ZEND_NUM_ARGS();

    if (ac != 0) {
        WRONG_PARAM_COUNT;
    }

    fill_friendlist(&range, usr);
    
    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }
    //if(!usr) RETURN_LONG(0);
    
    for (i = 0; i < range; i++) {
        x = usr[i];
        if (x == NULL)continue;
        if (x->active == 0) continue;
        if (x->invisible && !HAS_PERM(getCurrentUser(), PERM_SEECLOAK)) continue;
	
        memcpy(&user[total], x , sizeof(uinfo_t));
    	total++;
        if(total >= MAXFRIENDS) break;
    }
    if(total == 0) RETURN_LONG(0);
    
    qsort(user, total, sizeof(uinfo_t), cmpuser);	
	
    for (i = 0; i < total; i++) {
        MAKE_STD_ZVAL(element);
        array_init(element);
        add_assoc_bool ( element, "invisible", user[i].invisible );
        add_assoc_long ( element, "pid", user[i].pid );
        add_assoc_bool ( element, "isfriend", isfriend(user[i].userid) );
        add_assoc_long ( element, "idle", (long)(time(0) - user[i].freshtime)/60 );
        add_assoc_string ( element, "userid", user[i].userid, 1 );       
        add_assoc_string ( element, "username", user[i].username, 1 );   
        if( getuser(user[i].userid, &lookupuser) == 0 ) lookupuser=NULL;
        add_assoc_string ( element, "userfrom", HAS_PERM(getCurrentUser(), PERM_SYSOP)?user[i].from:SHOW_USERIP(lookupuser, user[i].from), 1 );
        add_assoc_string ( element, "mode", ModeType(user[i].mode), 1 );
        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
	}
}
