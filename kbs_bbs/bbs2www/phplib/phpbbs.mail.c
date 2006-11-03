#include "php_kbs_bbs.h"  

PHP_FUNCTION(bbs_checknewmail)
{
	char *userid;
	int userid_len;
	char qry_mail_dir[STRLEN];

    if (zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
	
	if( userid_len > IDLEN )
		userid[IDLEN]=0;

	setmailfile(qry_mail_dir, userid, DOT_DIR);

	RETURN_LONG( check_query_mail(qry_mail_dir, NULL) );

}

PHP_FUNCTION(bbs_mail_get_num)
{
	char *userid;
	int userid_len, total, newmail;
	char qry_mail_dir[STRLEN];

    if (zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) != SUCCESS) {
        WRONG_PARAM_COUNT;
    }
	
	if( userid_len > IDLEN )
		userid[IDLEN]=0;

	setmailfile(qry_mail_dir, userid, DOT_DIR);

	newmail = check_query_mail(qry_mail_dir, &total);
    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }

    add_assoc_long(return_value, "total", total);
    add_assoc_bool(return_value, "newmail", newmail);
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
PHP_FUNCTION(bbs_getmailnum)
{
    zval *total, *unread;
    char *userid;
    int userid_len;
    struct fileheader x;
    char path[80];
    int totalcount = 0, unreadcount = 0;
    int ac = ZEND_NUM_ARGS();
    int fd;
	long oldtotal,oldunread;

    if (ac != 5 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "szzll", &userid, &userid_len, &total, &unread, &oldtotal, &oldunread) == FAILURE) {
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

	if( !strcmp(userid, getCurrentUser()->userid) && oldtotal && getSession()->currentuinfo && !(getSession()->currentuinfo->mailcheck & CHECK_MAIL) ){
		totalcount = oldtotal;
		unreadcount = oldunread;
    	ZVAL_LONG(total, totalcount);
    	ZVAL_LONG(unread, unreadcount);
    	RETURN_TRUE;
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
	if( getSession()->currentuinfo )
		getSession()->currentuinfo->mailcheck |= CHECK_MAIL;
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
PHP_FUNCTION(bbs_getmailnum2)
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
PHP_FUNCTION(bbs_getmails)
{
    char *mailpath;
    int mailpath_len;
    int total, rows, i;
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

	if( getSession()->currentuinfo )
		getSession()->currentuinfo->mailcheck &= ~CHECK_MAIL;
}


/**
 * Get mail used space
 * @author stiger
 */
PHP_FUNCTION(bbs_getmailusedspace)
{
	RETURN_LONG(get_mailusedspace(getCurrentUser(),1)/1024);
}

/**
 * Whether save to sent box
 * @author atppp
 */
PHP_FUNCTION(bbs_is_save2sent)
{
	RETURN_LONG(HAS_MAILBOX_PROP(getSession()->currentuinfo, MBP_SAVESENTMAIL));
}




/*
 * bbs_can_send_mail ()
 * @author stiger
 */
PHP_FUNCTION(bbs_can_send_mail)
{
    long is_reply = 0;
    int ret;
    int ac = ZEND_NUM_ARGS();
    if (ac == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "l", &is_reply) == FAILURE) {
            WRONG_PARAM_COUNT;
        }
    } else {
        if (ac != 0) {
            WRONG_PARAM_COUNT;
        }
    }
    ret = check_mail_perm(getCurrentUser(), NULL);
    if (ret > 0) {
        if (is_reply) {
            RETURN_LONG(ret == 6 ? 1 : 0);
        } else {
            RETURN_LONG(0);
        }
    } else {
        RETURN_LONG(1);
    }
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
PHP_FUNCTION(bbs_loadmaillist)
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
PHP_FUNCTION(bbs_changemaillist)
{
    char *boxname;
    int boxname_len;
    char *userid;
    int userid_len;
    zend_bool bAdd;
    long index;

    struct _mail_list maillist;
    char buf[10], path[PATHLEN];

    struct userec *user;
    int i;
    struct stat st;

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
            setmailfile(path, getCurrentUser()->userid, buf);
            if (stat(path, &st) == -1)
                break;
        }
        f_touch(path);
        sprintf(buf, "MAILBOX%d", i);
        strcpy(maillist.mail_list[maillist.mail_list_t], boxname);
        strcpy(maillist.mail_list[maillist.mail_list_t] + 30, buf);
        maillist.mail_list_t += 1;
        save_mail_list(&maillist, getSession());
    } else                      //delete
    {
        if (index < 0 || index > maillist.mail_list_t - 1)
            RETURN_LONG(-1);
        sprintf(buf, ".%s", maillist.mail_list[index] + 30);
        setmailfile(path, getCurrentUser()->userid, buf);
        if (get_num_records(path, sizeof(struct fileheader)) != 0)
            RETURN_LONG(0);
        f_rm(path);
        for (i = index; i < maillist.mail_list_t - 1; i++)
            memcpy(maillist.mail_list[i], maillist.mail_list[i + 1], sizeof(maillist.mail_list[i]));
        maillist.mail_list_t--;
        save_mail_list(&maillist, getSession());
    }
	if( getSession()->currentuinfo )
		getSession()->currentuinfo->mailcheck &= ~CHECK_MAIL;
    RETURN_LONG(-1);
}





/**
 * Function: post a new mail
 *  rototype:
 * int bbs_postmail(string targetid,string title,string content,long sig, long backup);
 *
 *  @return the result
 *  	0 -- success
 *		-1   index file failed to open
 *      -2   file/dir creation failed
 *      -3   receiver refuses
 *      -4   receiver reaches mail limit
 *      -5   send too frequently
 *      -6   receiver index append failed
 *      -7   sender index append failed
 *      -8   invalid renum
 *      -9   sender no permission
 *      -100 invalid user
 *  @author roy
 */
PHP_FUNCTION(bbs_postmail){
	char *recvID, *title, *content;
	char targetID[IDLEN+1];
	int  idLen, tLen,cLen;
    long backup,sig,renum;
	int ac = ZEND_NUM_ARGS();
	char mail_title[80];
    FILE *fp;
    char fname[PATHLEN], filepath[PATHLEN], sent_filepath[PATHLEN];
    struct fileheader header;
    struct stat st;
    struct userec *touser;      /*peregrine for updating used space */
	char *refname,*dirfname;
	int find=-1,fhcount=0,refname_len,dirfname_len,ret;

    if(ac == 5)		/* use this to send a new mail */
	{
		if(zend_parse_parameters(5 TSRMLS_CC, "ss/s/ll", &recvID, &idLen,&title,&tLen,&content,&cLen,&sig,&backup) == FAILURE)
		{
			WRONG_PARAM_COUNT;
		}
		strncpy(targetID, recvID, sizeof(targetID));
		targetID[sizeof(targetID)-1] = '\0';
	}
    else if(ac == 7)		/* use this to reply a mail */
	{
		if(zend_parse_parameters(7 TSRMLS_CC, "ssls/s/ll", &dirfname, &dirfname_len, &refname, &refname_len, &renum, &title, &tLen, &content, &cLen, &sig, &backup) == FAILURE)
		{
			WRONG_PARAM_COUNT;
		}
	}
	else
	{
		WRONG_PARAM_COUNT;
	}

    if (check_last_post_time(getSession()->currentuinfo)) {
        RETURN_LONG(-5); // 两次发文间隔过密, 请休息几秒后再试
    }

	/* read receiver's id from mail when replying, by pig2532 */
	if(ac == 7)
	{
		if(stat(dirfname, &st)==-1)
        {
            RETURN_LONG(-1);    /* error reading stat */
        }
        if((renum<0)||(renum>=(st.st_size/sizeof(fileheader))))
        {
            RETURN_LONG(-8);    /* no such mail to reply */
        }
		if((fp = fopen(dirfname, "r+")) == NULL)
		{
			RETURN_LONG(-1);		/* error openning .DIR */
		}
		fseek(fp, sizeof(header) * renum, SEEK_SET);
		if(fread(&header, sizeof(header), 1, fp) > 0 )	/* read fileheader by renum */
		{
			if(strcmp(header.filename, refname) == 0)
			{
				find = renum;
			}
		}
		if(find == -1)
		{
			rewind(fp);
			while(true)		/* find the fileheader */
			{
				if(fread(&header, sizeof(header), 1, fp) <= 0)
				{
					break;
				}
				if(strcmp(header.filename, refname) == 0)
				{
					find = fhcount;
					break;
				}
				fhcount++;
			}
		}
		if(find == -1)
		{	/* file not found */
			fclose(fp);
			RETURN_LONG(-8);
		}
		else
		{
			strncpy(targetID, header.owner, sizeof(targetID));
			targetID[sizeof(targetID)-1] = '\0';
			if(!(header.accessed[0] & FILE_REPLIED))
			{	/* set the replied flag */
				header.accessed[0] |= FILE_REPLIED;
				fseek(fp, sizeof(header) * find, SEEK_SET);
				fwrite(&header, sizeof(header), 1, fp);
			}
			fclose(fp);
		}
	}

    getuser(targetID, &touser);
    if (touser == NULL) 
		RETURN_LONG(-100);//can't find user

    ret = check_mail_perm(getCurrentUser(), touser);
    switch(ret) {
        case 1:
            RETURN_LONG(-4);
            break;
        case 2:
            RETURN_LONG(-9);
            break;
        case 3:
            RETURN_LONG(-4);
            break;
        case 4:
            RETURN_LONG(-3);
            break;
        case 5:
            RETURN_LONG(-9);
            break;
        case 6:
            if (ac != 7) {
                RETURN_LONG(-9);
            }
            break;
        default:
            if (ret > 0) {
                RETURN_LONG(-10);
            }            
    }

	strncpy(targetID, touser->userid, sizeof(targetID));
	targetID[sizeof(targetID)-1] = '\0';
    process_control_chars(title,NULL);
	if (title[0] == 0)
        strcpy(mail_title,"没主题");
	else 
		strncpy(mail_title,title,79);
	mail_title[79]=0;
    
    bzero(&header, sizeof(header));
    strcpy(header.owner, getCurrentUser()->userid);
    strnzhcpy(header.title, mail_title, ARTICLE_TITLE_LEN);
    setmailpath(filepath, targetID);
    if (stat(filepath, &st) == -1) {
        if (mkdir(filepath, 0755) == -1)
            RETURN_LONG(-2);
    } else {
        if (!(st.st_mode & S_IFDIR))
            RETURN_LONG(-2);
    }
    if (GET_MAILFILENAME(fname, filepath) < 0)
        RETURN_LONG(-2);
    strcpy(header.filename, fname);
    setmailfile(filepath, targetID, fname);

    fp = fopen(filepath, "w");
    if (fp == NULL)
        RETURN_LONG(-2);
    write_header(fp, getCurrentUser(), 1, NULL, mail_title, 0, 0, getSession());
    if (cLen>0) {
        f_append(fp, unix_string(content));
    }
    getCurrentUser()->signature = sig;
    if (sig < 0) {
        struct userdata ud;
        read_userdata(getCurrentUser()->userid, &ud);
        if (ud.signum > 0) {
            sig = 1 + (int) (((double)ud.signum) * rand() / (RAND_MAX + 1.0)); //(rand() % ud.signum) + 1;
        } else sig = 0;
    }
    addsignature(fp, getCurrentUser(), sig);
    fputc('\n', fp);
    fclose(fp);
    
    if (stat(filepath, &st) != -1)
        header.eff_size = st.st_size;
    setmailfile(fname, targetID, ".DIR");
    if (append_record(fname, &header, sizeof(header)) == -1)
        RETURN_LONG(-6);
    touser->usedspace += header.eff_size;
	setmailcheck(targetID);
	    
   /* 添加Log Bigman: 2003.4.7 */
    newbbslog(BBSLOG_USER, "mailed(www) %s %s", targetID, mail_title);

    if (backup) {
        strcpy(header.owner, targetID);
        setmailpath(sent_filepath, getCurrentUser()->userid);
        if (GET_MAILFILENAME(fname, sent_filepath) < 0) {
            RETURN_LONG(-7);
        }
        strcpy(header.filename, fname);
        setmailfile(sent_filepath, getCurrentUser()->userid, fname);

        f_cp(filepath, sent_filepath, 0);
        if (stat(sent_filepath, &st) != -1) {
            getCurrentUser()->usedspace += st.st_size;
            header.eff_size = st.st_size;
        } else {
            RETURN_LONG(-7);
        }
        header.accessed[0] |= FILE_READ;
        setmailfile(fname, getCurrentUser()->userid, ".SENT");
        if (append_record(fname, &header, sizeof(header)) == -1)
            RETURN_LONG(-7);
        newbbslog(BBSLOG_USER, "mailed(www) %s ", getCurrentUser()->userid);
    }
	RETURN_LONG(0);
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
PHP_FUNCTION(bbs_mail_file)
{
    char *srcid;
    int srcid_len;
    char *filename;
    int filename_len;
    char *destid;
    int destid_len;
    char *title;
    int title_len;
    long is_move;
    int ac = ZEND_NUM_ARGS();

    if (ac != 5 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "ssssl", &srcid, &srcid_len, &filename, &filename_len, &destid, &destid_len, &title, &title_len, &is_move) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (mail_file(srcid, filename, destid, title, is_move, NULL) < 0)
        RETURN_FALSE;
    RETURN_TRUE;
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
PHP_FUNCTION(bbs_delmail)
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

	u = getCurrentUser();

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


/*
 * set a mail had readed
 */
PHP_FUNCTION(bbs_setmailreaded)
{
	int ac = ZEND_NUM_ARGS();
	long num;
	char * dirname;
	int dirname_len;
	int total;
	struct fileheader fh;
	FILE *fp;

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &dirname, &dirname_len, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	total = get_num_records(dirname, sizeof(fh));

	if(total <= 0)
		RETURN_LONG(0);

	if( getSession()->currentuinfo )
		setmailcheck(getSession()->currentuinfo->userid);

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


PHP_FUNCTION(bbs_domailforward)
{
    char *fname, *tit, *target1;
	char target[128];
    int ret,filename_len,tit_len,target_len;
	long big5,noansi;
	char title[512];
	struct userec *u;
	char mail_domain[STRLEN];
    
	if (ZEND_NUM_ARGS() != 5 || zend_parse_parameters(5 TSRMLS_CC, "sssll", &fname, &filename_len, &tit, &tit_len, &target1, &target_len, &big5, &noansi) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }

	strncpy(target, target1, 128);
	target[127]=0;

    if( target[0] == 0 ) {
        RETURN_ERROR(USER_NONEXIST);
    }
        

	snprintf(mail_domain, sizeof(mail_domain), "@%s", MAIL_BBSDOMAIN);
	if( strstr(target, mail_domain) )
		strcpy(target, getCurrentUser()->userid);
    if( !strchr(target, '@') ){
        if( getuser(target,&u) == 0) {
            RETURN_ERROR(USER_NONEXIST);
        }
        ret = check_mail_perm(getCurrentUser(), u);
        if (ret) {
            RETURN_LONG(-ret);
        }
        big5=0;
        noansi=0;
    }

    if( !file_exist(fname) )
        RETURN_LONG(-7);

    snprintf(title, 511, "%.50s(转寄)", tit);

    if( !strchr(target, '@') ){
        mail_file(getCurrentUser()->userid, fname, u->userid, title,0, NULL);
		RETURN_LONG(1);
	}else{
		if( big5 == 1)
			conv_init(getSession());
		if( bbs_sendmail(fname, title, target, big5, noansi, getSession()) == 0){
			RETURN_LONG(1);
		}else
			RETURN_LONG(-10);
	}
}
