#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_smth_bbs.h"  

#include "bbs.h"
#include "bbslib.h"

/**
 * Checking whether a user is a BM of a board or not.
 * prototype:
 * int bbs_is_bm(int brdnum, int usernum);
 *
 * @return one if the user is BM,
 *         zero if not.
 * @author flyriver
 */
PHP_FUNCTION(bbs_is_bm)
{
    long brdnum;
    long usernum;
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





/**
 * load deny users
 * function bbs_denyusers(char* board,array denyusers);
 *  @return the result
 *     0 : seccess
 *     -1: PHP error
 *     -2: board NOT exist
 *     -3: do NOT have permission
 *  @author: windinsn
 */
PHP_FUNCTION(bbs_denyusers)
{
    char *board;
    int  board_len;
    zval *element,*denyusers;
    
    struct boardheader brd;
    FILE *fp;
    char path[80], buf[256], buf2[100];
    char *id, *nick;
    int i;

    int ac = ZEND_NUM_ARGS();
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sa", &board, &board_len, &denyusers) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

    if(array_init(denyusers) != SUCCESS)
        RETURN_LONG(-1);
	
	if (getboardnum(board, &brd) == 0)
        RETURN_LONG(-2);
    if (!check_read_perm(getCurrentUser(), &brd))
        RETURN_LONG(-2);
    strcpy(board,brd.filename);
    if (!is_BM(&brd, getCurrentUser()))
        RETURN_LONG(-3);
    
    sprintf(path, "boards/%s/deny_users", board);
    
    if((fp=fopen(path,"r"))==NULL)
        RETURN_LONG(0);
    
    i = 0;
    while (fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == '\n') continue;
        MAKE_STD_ZVAL(element);
        array_init(element);
        
        id = strchr(buf, ' ');
        if (id != NULL)
            *id = '\0';
        
        strcpy(buf2, buf);
        add_assoc_string(element,"ID",buf2,1);
        strncpy(buf2, buf + 12, 30);
        buf2[30] = '\0';
        add_assoc_string(element,"EXP",buf2,1);
        
        nick = strrchr(buf + 13, '[');
        if (nick != NULL) {
            add_assoc_long(element,"FREETIME",atol(nick + 1));
            nick--;
            if (nick - buf > 43) {
                *nick = '\0';
                strcpy(buf2, buf + 43);
                add_assoc_string(element,"COMMENT",buf2,1);
            }
        }
        zend_hash_index_update(Z_ARRVAL_P(denyusers),i,(void*) &element, sizeof(zval*), NULL);
        i++;
    }
    fclose(fp);
    RETURN_LONG(0);
}

/**
 * add user to deny list
 * function bbs_denyadd(char* board,char* userid,char* exp,int denyday,int manual_deny);
 *  @return the result
 *     0 : seccess
 *     -1: board NOT exist
 *     -2: do NOT have permission
 *     -3: user NOT exist
 *     -4: already in list
 *     -5: time long error
 *     -6: need denymsg
 *  @author: windinsn
 */
PHP_FUNCTION(bbs_denyadd)
{
    char *board,*userid,*exp;
    int  board_len,userid_len,exp_len;
    long  denyday,manual_deny;
    int autofree;
    struct boardheader brd;
    struct userec *lookupuser;
    char buf[256];
    struct tm *tmtime;
    time_t now,undenytime;
    char path[STRLEN];
    
    int ac = ZEND_NUM_ARGS();
    if (ac != 5 || zend_parse_parameters(5 TSRMLS_CC, "sssll", &board, &board_len, &userid ,&userid_len ,&exp ,&exp_len ,&denyday ,&manual_deny) == FAILURE) 
		    WRONG_PARAM_COUNT;
    
    if (getboardnum(board, &brd) == 0)
        RETURN_LONG(-1);
    if (!check_read_perm(getCurrentUser(), &brd))
        RETURN_LONG(-1);
    strcpy(board,brd.filename);
    if (!is_BM(&brd, getCurrentUser()))
        RETURN_LONG(-2);
    if (getuser(userid,&lookupuser)==0)
        RETURN_LONG(-3);
    strcpy(userid,lookupuser->userid);
    if (deny_me(userid, board))
	    RETURN_LONG(-4);  

    if (exp_len >= 28) exp[27] = '\0';
    filter_control_char(exp);
	
	if (!*exp)
	    RETURN_LONG(-6);
	
	if (denyday < 1 || denyday > (HAS_PERM(getCurrentUser(), PERM_SYSOP)?70:14) )
	    RETURN_LONG(-5);

#ifdef MANUAL_DENY
    autofree = manual_deny ? false : true;
#else
    autofree = true;
#endif

#if 0
    brc_initial(getCurrentUser()->userid, board, getSession());
#endif

    now = time(0);
    undenytime = now + denyday * 24 * 60 * 60;
    tmtime = gmtime(&undenytime);
    
    if (autofree)
        sprintf(buf, "%-12.12s %-30.30s%-12.12s %2d月%2d日解\x1b[%lum", userid, exp, getCurrentUser()->userid, tmtime->tm_mon + 1, tmtime->tm_mday, undenytime);
    else
        sprintf(buf, "%-12.12s %-30.30s%-12.12s %2d月%2d日后\x1b[%lum", userid, exp, getCurrentUser()->userid, tmtime->tm_mon + 1, tmtime->tm_mday, undenytime);
    
    setbfile(path, board, "deny_users");
    if (addtofile(path, buf) == 1) {
        struct userec *saveptr;
        int my_flag = 0;        
        struct userec saveuser;
        FILE *fn;
        char buffer[STRLEN];
        
		gettmpfilename(path, "deny" );
        fn = fopen(path, "w+");
        memcpy(&saveuser, getCurrentUser(), sizeof(struct userec));
        saveptr = getCurrentUser();
        getCurrentUser() = &saveuser;
        sprintf(buffer, "%s被取消在%s版的发文权限", userid, board);

        if ((HAS_PERM(getCurrentUser(), PERM_SYSOP) || HAS_PERM(getCurrentUser(), PERM_OBOARDS)) && !chk_BM_instr(brd.BM, getCurrentUser()->userid)) {
            my_flag = 0;
            fprintf(fn, "寄信人: SYSOP (System Operator) \n");
            fprintf(fn, "标  题: %s\n", buffer);
            fprintf(fn, "发信站: %s (%24.24s)\n", BBS_FULL_NAME, ctime(&now));
            fprintf(fn, "来  源: %s\n", NAME_BBS_ENGLISH);
            fprintf(fn, "\n");
            fprintf(fn, "由于您在 \x1b[4m%s\x1b[m 版 \x1b[4m%s\x1b[m，我很遗憾地通知您， \n", board, exp);
            fprintf(fn, "您被暂时取消在该版的发文权力 \x1b[4m%ld\x1b[m 天", denyday);
            if (!autofree)
                fprintf(fn, "，到期后请回复\n此信申请恢复权限。\n");
            fprintf(fn, "\n");
            fprintf(fn, "                            %s" NAME_SYSOP_GROUP "值班站务：\x1b[4m%s\x1b[m\n", NAME_BBS_CHINESE, getCurrentUser()->userid);
            fprintf(fn, "                              %s\n", ctime(&now));
            strcpy(getCurrentUser()->userid, "SYSOP");
            strcpy(getCurrentUser()->username, NAME_SYSOP);
        } else {
            my_flag = 1;
            fprintf(fn, "寄信人: %s \n", getCurrentUser()->userid);
            fprintf(fn, "标  题: %s\n", buffer);
            fprintf(fn, "发信站: %s (%24.24s)\n", BBS_FULL_NAME, ctime(&now));
            fprintf(fn, "来  源: %s \n", SHOW_USERIP(getCurrentUser(), getSession()->fromhost));
            fprintf(fn, "\n");
            fprintf(fn, "由于您在 \x1b[4m%s\x1b[m 版 \x1b[4m%s\x1b[m，我很遗憾地通知您， \n", board, exp);
            fprintf(fn, "您被暂时取消在该版的发文权力 \x1b[4m%ld\x1b[m 天", denyday);
            if (!autofree)
                fprintf(fn, "，到期后请回复\n此信申请恢复权限。\n");
            fprintf(fn, "\n");
            fprintf(fn, "                              " NAME_BM ":\x1b[4m%s\x1b[m\n", getCurrentUser()->userid);
            fprintf(fn, "                              %s\n", ctime(&now));
        }
        fclose(fn);
        mail_file(getCurrentUser()->userid, path, userid, buffer, 0, NULL);
        fn = fopen(path, "w+");
        fprintf(fn, "由于 \x1b[4m%s\x1b[m 在 \x1b[4m%s\x1b[m 版的 \x1b[4m%s\x1b[m 行为，\n", userid, board, exp);
        fprintf(fn, "被暂时取消在本版的发文权力 \x1b[4m%ld\x1b[m 天。\n", denyday);
        
        if (my_flag == 0) {
            fprintf(fn, "                            %s" NAME_SYSOP_GROUP "值班站务：\x1b[4m%s\x1b[m\n", NAME_BBS_CHINESE, saveptr->userid);
        } else {
            fprintf(fn, "                              " NAME_BM ":\x1b[4m%s\x1b[m\n", getCurrentUser()->userid);
        }
        fprintf(fn, "                              %s\n", ctime(&now));
        fclose(fn);
        post_file(getCurrentUser(), "", path, board, buffer, 0, 2, getSession());
        getCurrentUser() = saveptr;

        sprintf(buffer, "%s 被 %s 封禁本版POST权", userid, getCurrentUser()->userid);

        if (PERM_BOARDS & lookupuser->userlevel)
            sprintf(buffer, "%s 封某版" NAME_BM " %s 在 %s", getCurrentUser()->userid, userid, board);
        else
            sprintf(buffer, "%s 封 %s 在 %s", getCurrentUser()->userid, userid, board);
        post_file(getCurrentUser(), "", path, "denypost", buffer, 0, 8, getSession());
        unlink(path);
        bmlog(getCurrentUser()->userid, board, 10, 1);
    }
    
	RETURN_LONG(0);
}

/**
 * remove user from deny list
 * function bbs_denydel(char* board,char* userid);
 *  @return the result
 *     0 : seccess
 *     -1: board NOT exist
 *     -2: do NOT have permission
 *     -3: NOT in list
 *  @author: windinsn
 */
PHP_FUNCTION(bbs_denydel)
{
    char *board,*userid;
    int  board_len,userid_len;
    struct boardheader brd;
   
    int ac = ZEND_NUM_ARGS();
    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "ss", &board, &board_len, &userid ,&userid_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
    
    if (getboardnum(board, &brd) == 0)
        RETURN_LONG(-1);
    if (!check_read_perm(getCurrentUser(), &brd))
        RETURN_LONG(-1);
    strcpy(board,brd.filename);
    if (!is_BM(&brd, getCurrentUser()))
        RETURN_LONG(-2);
        
    if (deny_me(userid, board)) {
        deldeny(getCurrentUser(), board, userid, 0, getSession());          
        RETURN_LONG(0);
    }
    else
        RETURN_LONG(-3);
}



/**
 * function bbs_bm_get_manageable_bids(string userid)
 * @return a string
 *         "A": can manage all visible boards
 *         empty string:  not a bm
 *         .-delimited bid list: all manageable board id list
 * @author atppp
 *
 */
PHP_FUNCTION(bbs_bm_get_manageable_bids)
{
#define MAX_MANAGEABLE_BIDS_LENGTH 1024 // note most browser set 4k as maximum bytes per cookie - atppp
    char *userid;
    int userid_len;
    struct userec *user;
    char buf[MAX_MANAGEABLE_BIDS_LENGTH];
    int ac = ZEND_NUM_ARGS();
    if (ac == 0) {
        user = getCurrentUser();
        if (user == NULL) {
            RETURN_EMPTY_STRING();
        }
    } else {
        if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &userid ,&userid_len) == FAILURE) {
		    WRONG_PARAM_COUNT;
    	}
        if (!getuser(userid, &user)) {
            RETURN_EMPTY_STRING();
        }
    }
    if (HAS_PERM(user, PERM_OBOARDS) || HAS_PERM(user, PERM_SYSOP)) {
        RETURN_STRING("A", 1);
    }
    if (!HAS_PERM(user, PERM_BOARDS)) {
        RETURN_EMPTY_STRING();
    }
    get_manageable_bids(user, buf, MAX_MANAGEABLE_BIDS_LENGTH);
    RETURN_STRING(buf, 1);
#undef MAX_MANAGEABLE_BIDS_LENGTH
}


