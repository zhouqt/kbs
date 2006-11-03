#include "php_kbs_bbs.h"  

int check_last_post_time(struct user_info *uinfo) {
    int lastpost, now;
    lastpost = uinfo->lastpost;
    now = time(0);
    uinfo->lastpost = now;
    if (abs(now - lastpost) < 6) {
        return 1;
    } else {
        return 0;
    }
}

PHP_FUNCTION(bbs_getattachtmppath)
{
    char buf[MAXPATH];
    if (getCurrentUser() == NULL) {
        RETURN_FALSE;
        //用户未初始化
    }
    getattachtmppath(buf, MAXPATH, getSession());
    RETURN_STRING(buf, 1);
}


PHP_FUNCTION(bbs_filteruploadfilename)
{
    char *filename;
    int flen;
    if (zend_parse_parameters(1 TSRMLS_CC, "s/", &filename, &flen) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (!flen) {
        RETURN_FALSE;
    }
    filename = filter_upload_filename(filename);
    RETURN_STRING(filename, 1);
}


static int dump_attachment_info(zval *ret, struct ea_attach_info *ai)
{
    zval *element;
    int count;

    if (array_init(ret) == FAILURE) {
        return -1;
    }
    
    for(count=0; count<MAXATTACHMENTCOUNT&&ai[count].name[0]; count++) {
        MAKE_STD_ZVAL(element);
        array_init(element);
        add_assoc_string(element, "name", ai[count].name, 1);
        add_assoc_long(element, "size", ai[count].size);
        add_assoc_long(element, "pos", ai[count].offset+ATTACHMENT_SIZE);
        zend_hash_index_update(Z_ARRVAL_P(ret), count, (void *) &element, sizeof(zval *), NULL);
    }
    return 0;
}

PHP_FUNCTION(bbs_upload_read_fileinfo)
{
    struct ea_attach_info ai[MAXATTACHMENTCOUNT];
    upload_read_fileinfo(ai, getSession());
    if (dump_attachment_info(return_value, ai)) {
        RETURN_FALSE;
    }
}

PHP_FUNCTION(bbs_upload_del_file)
{
    char *ofilename;
    int oflen, ret;
    if (zend_parse_parameters(1 TSRMLS_CC, "s", &ofilename, &oflen) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (!oflen) {
        RETURN_ERROR(GENERAL);
    }
    ret = upload_del_file(ofilename, getSession());
    switch(ret) {
        case -1:
            RETURN_ERROR(GENERAL);
            break;
        case -2:
            RETURN_ERROR(ATTACH_DELNONE);
            break;
        default:
            RETURN_LONG(0);
            break;
    }
}

PHP_FUNCTION(bbs_upload_add_file)
{
    char *filename, *ofilename;
    int flen, oflen, ret;
    if (zend_parse_parameters(2 TSRMLS_CC, "ss/", &filename, &flen, &ofilename, &oflen) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (!flen || !oflen) {
        RETURN_ERROR(GENERAL);
    }
    ret = upload_add_file(filename, ofilename, getSession());
    switch(ret) {
        case -1:
            RETURN_ERROR(GENERAL);
            break;
        case -2:
            RETURN_ERROR(ATTACH_CLIMIT);
            break;
        case -3:
            RETURN_ERROR(ATTACH_INVALIDNAME);
            break;
        case -4:
            RETURN_ERROR(ATTACH_DUPNAME);
            break;
        case -5:
            RETURN_ERROR(GENERAL);
            break;
        case -6:
            RETURN_ERROR(ATTACH_SLIMIT);
            break;
        default:
            RETURN_LONG(0);
            break;            
    }
}


static int update_index_attpos(const struct boardheader *bh, int ent, struct fileheader *fh, int attpos)
{
/* TODO: update .ORIGIN? */
    struct write_dir_arg dirarg;
    char dir[PATHLEN];
    int ret;

    setbdir(DIR_MODE_NORMAL, dir, bh->filename);
    init_write_dir_arg(&dirarg);
    dirarg.filename = dir;  
    dirarg.ent = ent;
    fh->attachment = attpos;
    if(change_post_flag(&dirarg,DIR_MODE_NORMAL, bh, fh, FILE_ATTACHPOS_FLAG, fh, false,getSession())!=0)
        ret = 1;
    else
        ret = 0;
    free_write_dir_arg(&dirarg);
    return(ret);
}

PHP_FUNCTION(bbs_attachment_add)
{
    struct ea_attach_info ai[MAXATTACHMENTCOUNT];
    const boardheader_t *brd;
    struct fileheader f;
    char dir[PATHLEN];

    char* board;
    int ent,fd,ret;
    long id;
    int board_len;
    char *filename, *ofilename;
    int flen, oflen;
    
	int ac = ZEND_NUM_ARGS();

    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "slss", &board, &board_len,&id, &filename, &flen, &ofilename, &oflen) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	brd = getbcache(board);
    if (!brd)
        RETURN_ERROR(BOARD_NONEXIST);

    if (!(brd->flag & BOARD_ATTACH) && !HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        RETURN_ERROR(ATTACH_ADDPERM);
    }
    ent = get_ent_from_id_ext(DIR_MODE_NORMAL, id, brd->filename, &f);
    if (ent < 0)
    {
        RETURN_ERROR(POST_NONEXIST);
    }
    ret = deny_modify_article(brd, &f, DIR_MODE_NORMAL, getSession());
    if (ret) {
        RETURN_ERROR(POST_MODPERM);
    }

    setbfile(dir, brd->filename, f.filename);
    add_edit_mark(dir, 0, f.title, getSession());
    
    fd = open(dir, O_RDWR);
    if (fd < 0)
        RETURN_ERROR(GENERAL);

    ret = ea_locate(fd, ai);
    if (ret>=0) {
        int count, size=0;
        struct stat st;
        for(count=0; count<MAXATTACHMENTCOUNT&&ai[count].name[0]; count++) {
            size += ai[count].size;
        }
        if(stat(filename,&st)||!S_ISREG(st.st_mode)){
            unlink(filename);
            ret = PHPBBS_ERROR_GENERAL;
        } else if((size+st.st_size)>MAXATTACHMENTSIZE && !HAS_PERM(getCurrentUser(), PERM_SYSOP)){
            unlink(filename);
            ret = PHPBBS_ERROR_ATTACH_SLIMIT;
        } else {
            ret = ea_append(fd, ai, filename, ofilename);
            if (ret < 0) ret = PHPBBS_ERROR_GENERAL;
        }
    }
    close(fd);

    if (ret>=0)
        ret = dump_attachment_info(return_value, ai);

    if (ret < 0) {
        RETURN_LONG(ret);
    } else {
        update_index_attpos(brd, ent, &f, ai[0].offset);
    }
}

PHP_FUNCTION(bbs_attachment_del)
{
    struct ea_attach_info ai[MAXATTACHMENTCOUNT];
    const boardheader_t *brd;
    struct fileheader f;
    char dir[PATHLEN];

    char* board;
    int ent,fd,ret;
    long id,pos;
    int board_len;

	int ac = ZEND_NUM_ARGS();

    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sll", &board, &board_len,&id,&pos) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	brd = getbcache(board);
    if (!brd)
        RETURN_ERROR(BOARD_NONEXIST);

    ent = get_ent_from_id_ext(DIR_MODE_NORMAL, id, brd->filename, &f);
    if (ent < 0)
    {
        RETURN_ERROR(POST_NONEXIST);
    }
    ret = deny_modify_article(brd, &f, DIR_MODE_NORMAL, getSession());
    if (ret) {
        RETURN_ERROR(POST_MODPERM);
    }

    setbfile(dir, brd->filename, f.filename);
    add_edit_mark(dir, 0, f.title, getSession());
    
    fd = open(dir, O_RDWR);
    if (fd < 0)
        RETURN_ERROR(GENERAL);

    ret = ea_locate(fd, ai);
    if (ret>=0) {
        ret = ea_delete(fd, ai, pos);
        if (ret < 0) ret = PHPBBS_ERROR_ATTACH_DELNONE;
    }
    close(fd);

    if (ret>=0)
        ret = dump_attachment_info(return_value, ai);

    if (ret < 0) {
        RETURN_LONG(ret);
    } else {
        update_index_attpos(brd, ent, &f, ai[0].offset);
    }
}

PHP_FUNCTION(bbs_attachment_list)
{
    struct ea_attach_info ai[MAXATTACHMENTCOUNT];
    const boardheader_t *brd;
    struct fileheader f;
    char dir[PATHLEN];

    char* board;
    int ent,fd,ret;
    long id;
    int board_len;

	int ac = ZEND_NUM_ARGS();

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &board, &board_len,&id) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	brd = getbcache(board);
    if (!brd)
        RETURN_ERROR(BOARD_NONEXIST);

    ent = get_ent_from_id_ext(DIR_MODE_NORMAL, id, brd->filename, &f);
    if (ent < 0)
    {
        RETURN_ERROR(POST_NONEXIST);
    }
    setbfile(dir, brd->filename, f.filename);
    fd = open(dir, O_RDONLY);
    if (fd < 0)
        RETURN_ERROR(GENERAL);

    ret = ea_locate(fd, ai);
    close(fd);

    if (ret<0 || dump_attachment_info(return_value, ai)) {
        RETURN_ERROR(GENERAL);
    }
}


PHP_FUNCTION(bbs_postarticle)
{
	char *boardName, *title, *content;
    char path[80],board[80];
	int blen, tlen, clen;
    long sig, mailback, is_tex;
	long reid;
    struct fileheader *oldx = NULL;
    const boardheader_t *brd;
    long local_save, outgo, anony;
    struct fileheader post_file, oldxx;
    char filepath[MAXPATH];
    int anonyboard, color;
	int retvalue;
    FILE *fp;


	int ac = ZEND_NUM_ARGS();

    /*
     * getting arguments 
     */
    
    if (ac == 7) {
        if (zend_parse_parameters(7 TSRMLS_CC, "ss/s/llll", &boardName, &blen, &title, &tlen, &content, &clen, &sig, &reid, &outgo,&anony) == FAILURE) {
            WRONG_PARAM_COUNT;
        }
        mailback = 0;
        is_tex = 0;
    } else if (ac == 9) {
        if (zend_parse_parameters(9 TSRMLS_CC, "ss/s/llllll", &boardName, &blen, &title, &tlen, &content, &clen, &sig, &reid, &outgo,&anony,&mailback,&is_tex) == FAILURE) {
            WRONG_PARAM_COUNT;
        }
    } else {
        WRONG_PARAM_COUNT;
    }

    brd = getbcache(boardName);
    if (getCurrentUser() == NULL) {
        RETURN_FALSE;
	} 
    if (brd == 0)
        RETURN_LONG(-1); //错误的讨论区名称
    if (brd->flag&BOARD_GROUP)
        RETURN_LONG(-2); //二级目录版
    strcpy(board, brd->filename);

    if (tlen > 256) title[256] = '\0';
    process_control_chars(title,NULL);
    if (tlen == 0)
        RETURN_LONG(-3); //标题为NULL
    if (true == checkreadonly(board) || !haspostperm(getCurrentUser(), board))
        RETURN_LONG(-4); //此讨论区是唯读的, 或是您尚无权限在此发表文章.

#ifdef NEWSMTH
    if(!check_score_level(getCurrentUser(),brd)){
        RETURN_LONG(-21);
    }
#endif /* NEWSMTH */

    if (deny_me(getCurrentUser()->userid, board) && !HAS_PERM(getCurrentUser(), PERM_SYSOP))
        RETURN_LONG(-5); //很抱歉, 你被版务人员停止了本版的post权利.

    if (check_last_post_time(getSession()->currentuinfo)) {
        RETURN_LONG(-6); // 两次发文间隔过密, 请休息几秒后再试
    }

    if(reid > 0){
        int pos;int fd;
		setbfile(path,board,DOT_DIR);
		fd = open(path,O_RDWR);
		if(fd < 0) RETURN_LONG(-7); //索引文件不存在
		get_records_from_id(fd,reid,&oldxx,1,&pos);

		close(fd);
        if (pos >= 0) {
            oldx = &oldxx;
            if (oldx->accessed[1] & FILE_READ) {
                RETURN_LONG(-8); //本文不能回复
            }
        }
    }
    local_save = (is_outgo_board(board) && outgo) ? 0 : 1;

    memset(&post_file, 0, sizeof(post_file));
    anonyboard = anonymousboard(board); /* 是否为匿名版 */

    /*
     * 自动生成 POST 文件名 
     */
    setbfile(filepath, board, "");
    if (GET_POSTFILENAME(post_file.filename, filepath) != 0) {
        RETURN_LONG(-9);
    }
    setbfile(filepath, board, post_file.filename);

    anony = anonyboard && anony;
    strncpy(post_file.owner, anony ? board : getCurrentUser()->userid, OWNER_LEN);
    post_file.owner[OWNER_LEN - 1] = 0;

    if ((!strcmp(board, "Announce")) && (!strcmp(post_file.owner, board)))
        strcpy(post_file.owner, "SYSOP");
    fp = fopen(filepath, "w");
    write_header(fp, getCurrentUser(), 0, board, title, anony, (local_save ? 1 : 2), getSession());
    if (clen > 0) {
        f_append(fp, unix_string(content));
    }
    if (!anony) {
        getCurrentUser()->signature = sig;
        if (sig < 0) {
            struct userdata ud;
            read_userdata(getCurrentUser()->userid, &ud);
            if (ud.signum > 0) {
                sig = 1 + (int) (((double)ud.signum) * rand() / (RAND_MAX + 1.0)); //(rand() % ud.signum) + 1;
            } else sig = 0;
        }

        addsignature(fp, getCurrentUser(), sig);
    }
    if (sig == 0) {
        fputs("\n--\n", fp);
    } else {
        fprintf(fp, "\n");
    }
    color = (getCurrentUser()->numlogins % 7) + 31; /* 颜色随机变化 */
    if (!strcmp(board, "Announce"))
        fprintf(fp, "\033[m\033[%2dm※ 来源:·%s http://%s·[FROM: %s]\033[m\n", color, BBS_FULL_NAME, BBS_FULL_NAME, BBS_FULL_NAME);
    else
        fprintf(fp, "\n\033[m\033[%2dm※ 来源:·%s http://%s·[FROM: %s]\033[m\n", color, BBS_FULL_NAME, NAME_BBS_ENGLISH, (anony) ? NAME_ANONYMOUS_FROM : SHOW_USERIP(getCurrentUser(), getSession()->fromhost));

    if (brd->flag&BOARD_ATTACH) {
        upload_post_append(fp, &post_file, getSession());
	}
    fclose(fp);
    post_file.eff_size = get_effsize(filepath);

    strnzhcpy(post_file.title, title, ARTICLE_TITLE_LEN);
    if (local_save) {      /* local save */
        post_file.innflag[1] = 'L';
        post_file.innflag[0] = 'L';
    } else {
        post_file.innflag[1] = 'S';
        post_file.innflag[0] = 'S';
        outgo_post(&post_file, board, title, getSession());
    }

    if (mailback) post_file.accessed[1] |= FILE_MAILBACK;
    if (is_tex) post_file.accessed[1] |= FILE_TEX;
    
    /*
     * 在boards版版主发文自动添加文章标记 Bigman:2000.8.12 
     */
    if (!strcmp(board, "Board") && !HAS_PERM(getCurrentUser(), PERM_OBOARDS)
        && HAS_PERM(getCurrentUser(), PERM_BOARDS)) {
        post_file.accessed[0] |= FILE_SIGN;
    }
#ifdef HAVE_BRC_CONTROL
    brc_initial(getCurrentUser()->userid, board, getSession());
#endif
	retvalue = after_post(getCurrentUser(), &post_file, board, oldx, !anony, getSession());

    if (retvalue > 0) {
#ifdef WWW_GENERATE_STATIC
        generate_static(DIR_MODE_NORMAL,&post_file,board,oldx);
#endif
    }
    if (!junkboard(board)) {
        getCurrentUser()->numposts++;
    }
    if (retvalue == -1) {
        RETURN_LONG(-9);
    } else if (retvalue == -2) {
        RETURN_LONG(-10);
    } else {
        RETURN_LONG(retvalue);
    }
}




/**
 * del board article
 * prototype:
 * int bbs_delpost(string board, int id);
 *
 *  @return the result
 *  	 0 -- success
 *      -1 -- no perm
 *  	-2 -- failed     
 *  @author pig2532
 */
PHP_FUNCTION(bbs_delpost)
{
    const boardheader_t *brd;
    struct fileheader f;

    char* board;
    int ent;
    long id;
    int board_len;

	int ac = ZEND_NUM_ARGS();

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &board, &board_len,&id) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	brd = getbcache(board);
    if (brd == 0)
        RETURN_LONG(-2);

    ent = get_ent_from_id_ext(DIR_MODE_NORMAL, id, brd->filename, &f);
    if(ent < 0)
    {
        RETURN_LONG(-2);
    }
    if(del_post(ent, &f, brd) != 0)
    {
        RETURN_LONG(-2);
    }
    else
    {
        RETURN_LONG(0);
    }
}


PHP_FUNCTION(bbs_article_deny_modify)
{
    const boardheader_t *brd;
    struct fileheader f;

    char* board;
    int ent,ret;
    long id;
    int board_len;

	int ac = ZEND_NUM_ARGS();

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &board, &board_len,&id) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	brd = getbcache(board);
    if (!brd)
        RETURN_ERROR(BOARD_NONEXIST);

    ent = get_ent_from_id_ext(DIR_MODE_NORMAL, id, brd->filename, &f);
    if (ent < 0)
    {
        RETURN_ERROR(POST_NONEXIST);
    }
    ret = deny_modify_article(brd, &f, DIR_MODE_NORMAL, getSession());
    if (!ret)
        RETURN_LONG(0);
    switch(ret) {
        case -1: RETURN_ERROR(GENERAL); break;
        case -2: RETURN_ERROR(BOARD_DENYPOST); break;
        case -3: RETURN_ERROR(GENERAL); break;
        case -4: RETURN_ERROR(GENERAL); break;
        case -5: RETURN_ERROR(BOARD_READONLY); break;
        case -6: RETURN_ERROR(BOARD_DENYMOD); break;
        default: RETURN_ERROR(GENERAL); break;
    }
}

PHP_FUNCTION(bbs_article_modify)
{
}



/*  function bbs_updatearticle(string boardName, string filename ,string text)  
 *  更新编辑文章
 *
 */
PHP_FUNCTION(bbs_updatearticle)
{
	char *boardName, *filename, *content;
	int blen, flen, clen;
    FILE *fin;
    FILE *fout;
    char infile[80], outfile[80];
    char buf2[256];
    int i;
    const boardheader_t *bp;
    int asize;
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

    if (clen == 0) content = "";
    else content = unix_string(content);

#ifdef FILTER
    if (check_badword_str(content, strlen(content),getSession())) {
        RETURN_LONG(-1); //修改文章失败，文章可能含有不恰当内容.
    }
#endif

    setbfile(infile, bp->filename, filename);
    sprintf(outfile, "tmp/%s.%d.editpost", getCurrentUser()->userid, getpid());
    if ((fin = fopen(infile, "r")) == NULL)
        RETURN_LONG(-10);
    if ((fout = fopen(outfile, "w")) == NULL) {
        fclose(fin);
        RETURN_LONG(-10);
    }
    for (i = 0; i < 4; i++) {
        fgets(buf2, sizeof(buf2), fin);
		if ((i==0) && (strncmp(buf2,"发信人",6)!=0)) {
			break;
		}
        fprintf(fout, "%s", buf2);
    }
    
    /* 解决很猪的问题之一 by pig2532 */
    if (clen>0) {
        fprintf(fout, "%s", content);
        if(content[clen-1] != '\n')
	    fprintf(fout, "\n");
    }
    
    while ((asize = -attach_fgets(buf2, sizeof(buf2), fin)) != 0) {
        if (asize <= 0) {
            if (Origin2(buf2)) {
                fprintf(fout, "%s", buf2);
            }
        } else {
            put_attach(fin, fout, asize);
        }
    }
    fclose(fin);
    fclose(fout);
    f_cp(outfile, infile, O_TRUNC);
    unlink(outfile);
    add_edit_mark(infile, 0, NULL, getSession());
    RETURN_LONG(0);
}



/*
 * function bbs_edittitle(string boardName , int id , string newTitle , int dirMode)
 * 修改文章标题
 * @author: windinsn apr 28,2004
 * return 0 : 成功
 *        -1: 版面错误
 *        -2: 该版不能修改文章
 *        -3: 只读讨论区
 *        -4: 文件错误
 *        -5: 封禁中
 *        -6: 无权修改
 *        -7: 被过滤掉
 *	  -8: 当前模式不能编辑标题
 *        -9: 标题过长或为空
 *        -10:system error
 */

PHP_FUNCTION(bbs_edittitle)
{
    char *board,*title;
    int  board_len,title_len;
    long  id , mode;
    char path[STRLEN];
    char dirpath[STRLEN];
    struct fileheader f;
    struct fileheader xfh;
    const struct boardheader *brd;
    int bid,ent,i=0;
    int fd,ret;
    bool find;
    
    int ac = ZEND_NUM_ARGS();
    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "sls/l", &board, &board_len, &id , &title, &title_len , &mode) == FAILURE) 
        WRONG_PARAM_COUNT;
    
    if (title_len == 0)
        RETURN_LONG(-9);
    bid = getbid(board, &brd);
    if (bid==0) 
        RETURN_LONG(-1); //版面名称错误
    if (brd->flag&BOARD_GROUP)
        RETURN_LONG(-1); //二级目录版

    if (mode != DIR_MODE_ZHIDING)
        mode = DIR_MODE_NORMAL;
    setbdir(mode, dirpath, brd->filename);
    
    if ((fd = open(dirpath, O_RDWR, 0644)) < 0)
        RETURN_LONG(-10);
    if (mode == DIR_MODE_ZHIDING)   /* find "zhiding" record, by pig2532 */
    {
        ent = 0;
        find = 0;
        while (1)
        {
            if (read(fd, &f, sizeof(struct fileheader)) <= 0)
                break;
            ent++;
            if(f.id == id)
            {
                find = 1;
                break;
            }
        }
        if (!find)
        {
            close(fd);
            RETURN_LONG(-4);
        }
    }
    else if (!get_records_from_id(fd,id,&f,1,&ent))
    {
        close(fd);
        RETURN_LONG(-4); //无法取得文件记录
    }
    close(fd);

    ret = deny_modify_article(brd, &f, mode, getSession());
    if (ret) {
        switch(ret) {
        case -1:
            RETURN_LONG(-10);
            break;
        case -4:
            RETURN_LONG(-8);
            break;
        case -3:
            RETURN_LONG(-2); //不允许修改文章
            break;
        case -5:
            RETURN_LONG(-3); //只读讨论区
            break;
        case -2:
            RETURN_LONG(-5);
            break;
        case -6:
            RETURN_LONG(-6); //他人文章
            break;
        default:
            RETURN_LONG(-10);
            break;
        }
    }
    if (title_len > 256) {
        title[256] = '\0';
    }
    process_control_chars(title,NULL);
    if (!strcmp(title,f.title)) //无需修改
        RETURN_LONG(0);
#ifdef FILTER
    if (check_badword_str(title, strlen(title), getSession()))
        RETURN_LONG(-7);
#endif
    setbfile(path, brd->filename, f.filename);
    if (add_edit_mark(path, 2, title, getSession()) != 1)
        RETURN_LONG(-10);
    
    /* update .DIR START */
    strnzhcpy(f.title, title, ARTICLE_TITLE_LEN);
    setbdir(mode, dirpath, brd->filename);
    fd = open(dirpath, O_RDONLY, 0);
    if (fd!=-1) {
        for (i = ent; i > 0; i--) {
            if (0 == get_record_handle(fd, &xfh, sizeof(xfh), i)) {
                if (0 == strcmp(xfh.filename, f.filename)) {
                    ent = i;
                    break;
                }
            }
        }
        if (i!=0) {
            substitute_record(dirpath, &f, sizeof(f), ent);
            if (mode == DIR_MODE_ZHIDING)
                board_update_toptitle(bid, true);
        }
        close(fd);
    }
    if (0 == i)
        RETURN_LONG(-10);

    if (f.id == f.groupid) {
        if( setboardorigin(board, -1) )
        {
            board_regenspecial(brd->filename,DIR_MODE_ORIGIN,NULL);
        }
        else
        {
            char olddirect[PATHLEN];
            setbdir(DIR_MODE_ORIGIN, olddirect, brd->filename);
            if ((fd = open(olddirect, O_RDWR, 0644)) >= 0)
            {
                struct fileheader tmpfh;
                if (get_records_from_id(fd, f.id, &tmpfh, 1, &ent) == 0)
                {
                    close(fd);
                }
                else
                {
                    close(fd);
                    substitute_record(olddirect, &f, sizeof(f), ent);
                }
            }
        }
    }
    setboardtitle(brd->filename, 1);    
    /* update .DIR END   */
    RETURN_LONG(0);
}


PHP_FUNCTION(bbs_doforward)
{
    char *board,*filename, *tit, *target;
    int board_len,filename_len,tit_len,target_len;
    const struct boardheader *bh;
	char fname[STRLEN];
	long big5,noansi;
	char title[512];
	struct userec *u;
    int ret;
    
	if (ZEND_NUM_ARGS() != 6 || zend_parse_parameters(6 TSRMLS_CC, "ssssll", &board, &board_len,&filename, &filename_len, &tit, &tit_len, &target, &target_len, &big5, &noansi) != SUCCESS) {
            WRONG_PARAM_COUNT;
    }

    if( target[0] == 0 )
        RETURN_LONG(-8);
    if( !strchr(target, '@') ){
        if( getuser(target,&u) == 0)
            RETURN_LONG(-8);
        ret = check_mail_perm(getCurrentUser(), u);
        if (ret) {
            RETURN_LONG(-ret);
        }
        big5=0;
        noansi=0;
    }

    if (getbid(board, &bh) == 0)
        RETURN_LONG(-11); //"错误的讨论区";
    if (!check_read_perm(getCurrentUser(), bh))
        RETURN_LONG(-11); //您无权阅读本版;

    setbfile(fname, bh->filename, filename);

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

/**
 * 转贴文章
 * int bbs_docross(string board,int id,string target,int out_go [,string filename, string title]);
 * return  0 :success
 *         -1:源版面不存在
 *         -2:目标版面不存在
 *         -3:目标版面只读
 *         -4:无发文权限
 *         -5:被封禁
 *         -6:文件记录不存在
 *         -7:已经被转载过了
 *         -8:不能在板内转载
 *         -9:目标版面不支持附件
 *         -10:system error
 * @author: windinsn
 */
PHP_FUNCTION(bbs_docross)
{
    char *board,*target,*filename,*title;
    int  board_len,target_len,filename_len,title_len;
    long  id,out_go;
    const struct boardheader *src_bp;
	const struct boardheader *dst_bp;
	struct fileheader f;
    int  ent;
    int  fd;
    struct userec *u = NULL;
    char path[256],ispost[10];
    
    int ac = ZEND_NUM_ARGS();
    if (ac == 4) {
        if (zend_parse_parameters(4 TSRMLS_CC, "slsl", &board, &board_len, &id, &target, &target_len, &out_go) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
    }
    else if (ac == 6) {    /* cross mail by pig2532 */
        if (zend_parse_parameters(6 TSRMLS_CC, "slslss", &board, &board_len, &id, &target, &target_len, &out_go, &filename, &filename_len, &title, &title_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
    }
    else {
        WRONG_PARAM_COUNT;
    }
	u = getCurrentUser();
    if (ac == 4) {
	src_bp = getbcache(board);
	if (src_bp == NULL)
	    RETURN_LONG(-1);
	strcpy(board, src_bp->filename);
	if(!check_read_perm(u, src_bp))
		RETURN_LONG(-1);
    }
    
    dst_bp = getbcache(target);
    if (dst_bp == NULL)
        RETURN_LONG(-2);
    strcpy(target, dst_bp->filename);

    if (!strcmp(board,target))
        RETURN_LONG(-8);

    strcpy(ispost ,((dst_bp->flag & BOARD_OUTFLAG) && out_go)?"s":"l");
    
    if(!check_read_perm(u, dst_bp))
		RETURN_LONG(-2);
    if (true == checkreadonly(target))
		RETURN_LONG(-3); //只读讨论区
    if (!HAS_PERM(u,PERM_SYSOP)) { //权限检查
	    if (!haspostperm(u, target))
	        	RETURN_LONG(-4);
	    if (deny_me(u->userid, target))
	        	RETURN_LONG(-5);
	}

    if (check_last_post_time(getSession()->currentuinfo)) {
        RETURN_LONG(-10);
    }

    if (ac == 4) {
	setbdir(DIR_MODE_NORMAL, path, board);
	if ((fd = open(path, O_RDWR, 0644)) < 0)
		RETURN_LONG(-10);
    if (!get_records_from_id(fd,id,&f,1,&ent)) {
		close(fd);
		RETURN_LONG(-6); //无法取得文件记录
	}
	close(fd);
	
#if 0 //disabled by atppp 20051228
    if ((f.accessed[0] & FILE_FORWARDED) && !HAS_PERM(u, PERM_SYSOP)) 
        RETURN_LONG(-7);
#endif	
	
	if ((f.attachment!=0)&&!(dst_bp->flag&BOARD_ATTACH)) 
        RETURN_LONG(-9);
	
	setbfile(path, board, f.filename);
	if (post_cross(u, dst_bp, board, f.title, path, 0, 0, ispost[0], 0, getSession()) == -1)
	    RETURN_LONG(-10);	
    }
    else if (ac == 6)
    {
        setmailfile(path, getCurrentUser()->userid, filename);
        if (post_cross(u, dst_bp, target, title, path, 0, 1, ispost[0], 0, getSession()) == -1)
            RETURN_LONG(-10);
    }
    RETURN_LONG(0);
}

/**
 * int bbs_docommend(string board, int id, int confirmed);
 *
 * @param confirmed: when set false, only test if can recommend
 *
 * return 0: no error
 *       -1: 无权限
 *       -2: 源版面不存在
 *       -3: 文件记录不存在
 *       -4: 本文章已经推荐过
 *       -5: 内部版面文章
 *       -6: 被停止了推荐的权力
 *       -7: 推荐出错
 *       -10: system err
 *
 * @author atppp
 */
PHP_FUNCTION(bbs_docommend)
{
#ifdef COMMEND_ARTICLE
    char *board;
    int  board_len;
    long  id,confirmed;
    struct userec *u;
    const struct boardheader *src_bp, *commend_bp;
    struct fileheader fileinfo;
    int  ent;
    int  fd;
    char path[256];

    int ac = ZEND_NUM_ARGS();
    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sll", &board, &board_len, &id, &confirmed) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    u = getCurrentUser();

    src_bp = getbcache(board);
    if (src_bp == NULL)
        RETURN_LONG(-1);
    strcpy(board, src_bp->filename);
    if(!check_read_perm(u, src_bp))
        RETURN_LONG(-2);

    setbdir(DIR_MODE_NORMAL, path, board);
    if ((fd = open(path, O_RDWR, 0644)) < 0)
        RETURN_LONG(-10);
    if (!get_records_from_id(fd,id,&fileinfo,1,&ent)) {
        close(fd);
        RETURN_LONG(-3); //无法取得文件记录
    }
    close(fd);

    commend_bp = getbcache(COMMEND_ARTICLE);
    if (commend_bp == NULL) {
        RETURN_LONG(-7);
    }
    if (!is_BM(commend_bp, u) && !is_BM(src_bp, u)) {
        if (strcmp(u->userid, fileinfo.owner))
            RETURN_LONG(-1);
    }
    if (!HAS_PERM(getCurrentUser(), PERM_LOGINOK)) {
        RETURN_LONG(-1);
    }
    if ((fileinfo.accessed[1] & FILE_COMMEND) && !HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        RETURN_LONG(-4);
    }
    if( ! normal_board(board) ){
        RETURN_LONG(-5);
    }
    if ( deny_me(u->userid, COMMEND_ARTICLE) ) {
        RETURN_LONG(-6);
    }
    if (confirmed) {
        if (post_commend(u, board, &fileinfo ) == -1) {
            RETURN_LONG(-7);
        } else {
            struct write_dir_arg dirarg;
            struct fileheader data;
            data.accessed[1] = FILE_COMMEND;
            init_write_dir_arg(&dirarg);
            dirarg.filename = path;  
            dirarg.ent = ent;
            change_post_flag(&dirarg,DIR_MODE_NORMAL,src_bp, &fileinfo, FILE_COMMEND_FLAG, &data,false,getSession());
            free_write_dir_arg(&dirarg);
        }
    }
    RETURN_LONG(0);
#else
    RETURN_LONG(-1);
#endif
}
















PHP_FUNCTION(bbs_brcaddread)
{
	char *board;
	int blen;
    long fid;
	const boardheader_t* bp;
    int bid;

    if (!strcmp(getCurrentUser()->userid, "guest")) {
        RETURN_NULL();
    }
    if (zend_parse_parameters(2 TSRMLS_CC, "sl", &board, &blen, &fid) != SUCCESS)
        WRONG_PARAM_COUNT;
	if ((bid=getbid(board, &bp))==0){
		RETURN_NULL();
	}
#ifdef HAVE_BRC_CONTROL
	brc_initial(getCurrentUser()->userid, bp->filename, getSession());
	brc_add_read(fid, bid, getSession());
#endif
    RETURN_NULL();
}

/**
 * 清除版面未读标记 
 * bbs_brcclear(string board)
 * windinsn
 * return true/false
 */
PHP_FUNCTION(bbs_brcclear)
{
    char *board;
    int  board_len;
    const struct boardheader *pbh;
    struct userec *u;
    int bid;
        
    int ac = ZEND_NUM_ARGS();
	
	if (ac != 1 || zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "s" , &board, &board_len) == FAILURE)
		WRONG_PARAM_COUNT;
		
    u = getCurrentUser();
    if (!u)
        RETURN_FALSE;

    bid = getbid(board, &pbh);
    if (bid == 0)
        RETURN_FALSE;
    if (!check_read_perm(u, pbh))
        RETURN_FALSE;
    if (!strcmp(u->userid,"guest"))
        RETURN_TRUE;
#ifdef HAVE_BRC_CONTROL
    brc_initial(u->userid, pbh->filename, getSession());
    brc_clear(bid, getSession());
#endif
    RETURN_TRUE;
}

#ifdef HAVE_BRC_CONTROL
static int brcdump(struct boardheader *bh, int bid, void* arg)
{
    char **dumpstr = (char **)arg;
    int n;
    unsigned int *pn;
    if (!public_board(bh)) return 0;
    sprintf(*dumpstr, "%4.4x", bid);
    *dumpstr += 4;
    brc_initial(getCurrentUser()->userid, bh->filename, getSession());
    pn = getSession()->brc_cache_entry[getSession()->brc_currcache].list;
    for (n = 0; n < BRC_MAXNUM; n++) {
        if (*pn == 0) break;
        pn++;
    }
    sprintf(*dumpstr, "%4.4x", n);
    *dumpstr += 4;
    pn = getSession()->brc_cache_entry[getSession()->brc_currcache].list;
    for (n = 0; n < BRC_MAXNUM; n++) {
        if (*pn == 0) break;
        sprintf(*dumpstr, "%8.8x", *pn);
        *dumpstr += 8;
        pn++;
    }
    return 0;
}
#endif

PHP_FUNCTION(bbs2_brcdump)
{
#ifdef HAVE_BRC_CONTROL
    char dumpstr[MAXBOARD * (BRC_MAXNUM + 1) * 8 + 1];
    char *dumpptr = dumpstr;
    *dumpptr = '\0';

    if (!strcmp(getCurrentUser()->userid, "guest")) {
        RETURN_NULL();
    }
    apply_bids(brcdump, &dumpptr);
    RETURN_STRING(dumpstr, 1);
#endif
}

PHP_FUNCTION(bbs2_brcsync)
{
#ifdef HAVE_BRC_CONTROL
    char *dumpstr;
    char *enddump;
    int dumpstr_len;
    int bid, j, id, n;
    int total = 0;
    const struct boardheader *bh;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s/" , &dumpstr, &dumpstr_len) == FAILURE)
		WRONG_PARAM_COUNT;

    if (!strcmp(getCurrentUser()->userid, "guest")) {
        RETURN_NULL();
    }
    enddump = dumpstr + dumpstr_len;

#define READ_NEXT(var, len) \
    do { \
        char tmp; \
        if (enddump - dumpstr < len) goto brcsync_faint; \
        tmp = *(dumpstr + len); \
        *(dumpstr + len) = 0; \
        var = (int)strtol(dumpstr, NULL, 16); \
        dumpstr += len; \
        *dumpstr = tmp; \
    } while(0)    

    while(dumpstr < enddump) {
        READ_NEXT(bid, 4);
        READ_NEXT(n, 4);
        if (n == 0) continue;
        if (n < 0 || n > BRC_MAXNUM) goto brcsync_faint;
        bh = getboard(bid);
        if (bh == NULL) goto brcsync_faint;
        if (!public_board(bh)) goto brcsync_faint;
        brc_initial(getCurrentUser()->userid, bh->filename, getSession());
        for (j=0; j<n; j++) {
            READ_NEXT(id, 8);
            brc_add_read(id, bid, getSession());
        }
        total += n;
    }
    RETURN_LONG(total);
brcsync_faint:
    RETURN_NULL();
#endif
}

