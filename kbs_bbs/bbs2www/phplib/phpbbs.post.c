#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_kbs_bbs.h"  

#include "bbs.h"
#include "bbslib.h"


int getattachtmppath(char *buf, size_t buf_len)
{
#if USE_TMPFS==1 && ! defined(FREE)
    /* setcachehomefile() 不接受 buf_len 参数，先直接这么写吧 */
    snprintf(buf,buf_len,"%s/home/%c/%s/%d/upload",TMPFSROOT,toupper(getCurrentUser()->userid[0]),
			getCurrentUser()->userid, getSession()->utmpent);
#else
    snprintf(buf,buf_len,"%s/%s_%d",ATTACHTMPPATH,getCurrentUser()->userid,  getSession()->utmpent);
#endif
    buf[buf_len-1] = '\0';
    return 0;
}


int check_last_post_time(struct user_info *uinfo) {
    int lastpost = uinfo->lastpost;
    int now = time(0);
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
    getattachtmppath(buf, MAXPATH);
    RETURN_STRING(buf, 1);
}


PHP_FUNCTION(bbs_filteruploadfilename)
{
    char *filename;
    long flen;
    if (zend_parse_parameters(1 TSRMLS_CC, "s/", &filename, &flen) == FAILURE) {
        WRONG_PARAM_COUNT;
    }
    if (!flen) {
        RETURN_FALSE;
    }
    filename = filter_upload_filename(filename);
    RETURN_STRING(filename, 1);
}


PHP_FUNCTION(bbs_postarticle)
{
	char *boardName, *title, *content;
    char path[80],board[80];
	int blen, tlen, clen;
    long sig, mailback, is_tex;
	long reid;
    struct fileheader *oldx = NULL;
    boardheader_t *brd;
    long local_save, outgo, anony;
    struct fileheader post_file, oldxx;
    char filepath[MAXPATH];
    char buf[256];
    int fd, anonyboard, color;
	int retvalue;
    FILE *fp, *fp2;
    char attachdir[MAXPATH], attachfile[MAXPATH];


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
    filter_control_char(title);
    if (tlen == 0)
        RETURN_LONG(-3); //标题为NULL
    if (true == checkreadonly(board) || !haspostperm(getCurrentUser(), board))
        RETURN_LONG(-4); //此讨论区是唯读的, 或是您尚无权限在此发表文章.
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
        getattachtmppath(attachdir, MAXPATH);
        snprintf(attachfile, MAXPATH, "%s/.index", attachdir);
        if ((fp2 = fopen(attachfile, "r")) != NULL) {
            fputs("\n", fp);
            while (!feof(fp2)) {
                char *name;
                long begin = 0;
                unsigned int save_size;
                char *ptr;
                off_t size;

                fgets(buf, 256, fp2);
                name = strchr(buf, ' ');
                if (name == NULL)
                    continue;
                *name = 0;
                name++;
                ptr = strchr(name, '\n');
                if (ptr)
                    *ptr = 0;

                if (-1 == (fd = open(buf, O_RDONLY)))
                    continue;
                if (post_file.attachment == 0) {
                    /*
                     * log the attachment begin 
                     */
                    post_file.attachment = ftell(fp) + 1;
                }
                fwrite(ATTACHMENT_PAD, ATTACHMENT_SIZE, 1, fp);
                fwrite(name, strlen(name) + 1, 1, fp);
                BBS_TRY {
                    if (safe_mmapfile_handle(fd,  PROT_READ, MAP_SHARED, (void **) &ptr, & size) == 0) {
                        size = 0;
                        save_size = htonl(size);
                        fwrite(&save_size, sizeof(save_size), 1, fp);
                    } else {
                        save_size = htonl(size);
                        fwrite(&save_size, sizeof(save_size), 1, fp);
                        begin = ftell(fp);
                        fwrite(ptr, size, 1, fp);
                    }
                }
                BBS_CATCH {
                    ftruncate(fileno(fp), begin + size);
                    fseek(fp, begin + size, SEEK_SET);
                }
                BBS_END end_mmapfile((void *) ptr, size, -1);

                close(fd);
            }
			fclose(fp2);
        }
        f_rm(attachdir);
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

    if (retvalue == 0) {
#ifdef WWW_GENERATE_STATIC
        generate_static(DIR_MODE_NORMAL,&post_file,board,oldx);
#endif
    }
    if (!junkboard(board)) {
        getCurrentUser()->numposts++;
    }
    RETURN_LONG(retvalue);
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
PHP_FUNCTION(bbs_delfile)
{
	FILE *fp;
    boardheader_t *brd;
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

	u = getCurrentUser();
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
                        if(del_post(num + 1, &f, dir, brd->filename) != 0)
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
    boardheader_t *brd;
    struct fileheader f;
    struct userec *u = NULL;
    char dir[80];

    char* board;
    int ent,fd;
    long id;
    int board_len;

	int ac = ZEND_NUM_ARGS();

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sl", &board, &board_len,&id) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	u = getCurrentUser();
	brd = getbcache(board);

    if (brd == 0)
        RETURN_LONG(-2);
    if (!haspostperm(u, board))
        RETURN_LONG(-2);

	setbdir(DIR_MODE_NORMAL, dir, brd->filename);
    
    /* 用文件名定位太土了 改用ID -- pig2532 */
    fd = open(dir, O_RDWR, 0644);
    if(fd < 0)
    {
        RETURN_LONG(-2);
    }
    if(!get_records_from_id(fd, id, &f, 1, &ent))
    {
        close(fd);
	    RETURN_LONG(-2);
    }
    close(fd);
    if(del_post(ent, &f, dir, brd->filename) != 0)
    {
        RETURN_LONG(-2);
    }
    else
    {
        RETURN_LONG(0);
    }
}




/* function bbs_caneditfile(string board, string filename);
 * 判断当前用户是否有权编辑某文件
 */
PHP_FUNCTION(bbs_caneditfile)
{
    char *board,*filename;
    int boardLen,filenameLen;
	char path[512];
    struct fileheader x;
    boardheader_t *brd;


    if ((ZEND_NUM_ARGS() != 2) || (zend_parse_parameters(2 TSRMLS_CC, "ss", &board, &boardLen,&filename,&filenameLen) != SUCCESS)) {
		WRONG_PARAM_COUNT;
    } 
    brd = getbcache(board);
    if (brd == NULL) {
        RETURN_LONG(-1); //讨论区名称错误
    }
	if (getCurrentUser()==NULL)
		RETURN_FALSE;
    if (!strcmp(brd->filename, "syssecurity")
        || !strcmp(brd->filename, "junk")
        || !strcmp(brd->filename, "deleted"))   /* Leeward : 98.01.22 */
         RETURN_LONG(-2);  //本版不能修改文章
    if (checkreadonly(brd->filename) == true) {
		RETURN_LONG(-3); //本版已被设置只读
    }
    if (get_file_ent(brd->filename, filename, &x) == 0) {
        RETURN_LONG(-4); //无法取得文件记录
    }
	setbfile(path, brd->filename, filename);
    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP)     /* SYSOP、当前版主、原发信人 可以编辑 */
        &&!chk_currBM(brd->BM, getCurrentUser())) {
        if (!isowner(getCurrentUser(), &x)) {
            RETURN_LONG(-5); //不能修改他人文章!
        }
    }
    /* 版主禁止POST 检查 */
    if (deny_me(getCurrentUser()->userid, brd->filename) && !HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        RETURN_LONG(-7); //您的POST权被封
    }
    RETURN_LONG(0);
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
    boardheader_t *bp;
    time_t now;
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
    if (clen>0) fprintf(fout, "%s", content);
    now = time(0);
    fprintf(fout, "\033[36m※ 修改:·%s 於 %15.15s 修改本文·[FROM: %s]\033[m\n", getCurrentUser()->userid, ctime(&now) + 4, SHOW_USERIP(getCurrentUser(), getSession()->fromhost));
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
    f_mv(outfile, infile);
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
	struct userec *u = NULL;
	struct fileheader f;
	struct fileheader xfh;
	struct boardheader brd;
	int bid,ent,i=0;
	int fd;
	
	int ac = ZEND_NUM_ARGS();
	if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "sls/l", &board, &board_len, &id , &title, &title_len , &mode) == FAILURE) 
		WRONG_PARAM_COUNT;
	
	if ((mode>= DIR_MODE_THREAD) && (mode<= DIR_MODE_WEB_THREAD))
        	RETURN_LONG(-8);
	if (title_len == 0)
		RETURN_LONG(-9);
	bid = getboardnum(board, &brd);
	if (bid==0) 
		RETURN_LONG(-1); //版面名称错误
	if (brd.flag&BOARD_GROUP)
	        RETURN_LONG(-1); //二级目录版
	if (!strcmp(brd.filename, "syssecurity") || !strcmp(brd.filename, "junk") || !strcmp(brd.filename, "deleted"))  
		RETURN_LONG(-2); //不允许修改文章
	if (true == checkreadonly(brd.filename))
		RETURN_LONG(-3); //只读讨论区
	if ((u = getCurrentUser())==NULL)
		RETURN_LONG(-10); //无法获得当前登录用户
	
	if (mode == DIR_MODE_DIGEST)
		setbdir(DIR_MODE_DIGEST, dirpath, brd.filename);
	else
		setbdir(DIR_MODE_NORMAL, dirpath, brd.filename);
	
	if ((fd = open(dirpath, O_RDWR, 0644)) < 0)
		RETURN_LONG(-10);
	if (!get_records_from_id(fd,id,&f,1,&ent))
	{
		close(fd);
		RETURN_LONG(-4); //无法取得文件记录
	}
	close(fd);
	if (!HAS_PERM(u,PERM_SYSOP)) //权限检查
	{
		if (!haspostperm(u, brd.filename))
	        	RETURN_LONG(-6);
	        if (deny_me(u->userid, brd.filename))
	        	RETURN_LONG(-5);
	        if (!chk_currBM(brd.BM, u))
	        {
	        	if (!isowner(u, &f))
		            RETURN_LONG(-6); //他人文章
		}
	}
    if (title_len > 256) {
        title[256] = '\0';
    }
    filter_control_char(title);
	if (!strcmp(title,f.title)) //无需修改
		RETURN_LONG(0);
#ifdef FILTER
	if (check_badword_str(title, strlen(title), getSession()))
		RETURN_LONG(-7);
#endif
	setbfile(path, brd.filename, f.filename);
	if (add_edit_mark(path, 2, title, getSession()) != 1)
		RETURN_LONG(-10);
	/* update .DIR START */
	strnzhcpy(f.title, title, ARTICLE_TITLE_LEN);
	if (mode == DIR_MODE_ZHIDING)
	{
		setbdir(DIR_MODE_ZHIDING, dirpath, brd.filename);
		ent = get_num_records(dirpath,sizeof(struct fileheader));
        	fd = open(dirpath, O_RDONLY, 0);
        }
	else
	{
		if (mode == DIR_MODE_DIGEST)
			setbdir(DIR_MODE_DIGEST, dirpath, brd.filename);
		else
			setbdir(DIR_MODE_NORMAL, dirpath, brd.filename);
		fd = open(dirpath, O_RDONLY, 0);
	}
	if (fd!=-1) 
	{
		for (i = ent; i > 0; i--)
		{
			if (0 == get_record_handle(fd, &xfh, sizeof(xfh), i)) 
			{
                		if (0 == strcmp(xfh.filename, f.filename)) 
                		{
                			ent = i;
                			break;
                		}
                	}
		}
		if (mode == DIR_MODE_ZHIDING)
		{
                	if (i!=0) 
                    		substitute_record(dirpath, &f, sizeof(f), ent);
               		board_update_toptitle(bid, true);
        	}
        	else
        	{
        		if (i!=0) 
                		substitute_record(dirpath, &f, sizeof(f), ent);
		}
	}
	close(fd);
	if (0 == i)
            RETURN_LONG(-10);
        if(mode != DIR_MODE_ORIGIN && f.id == f.groupid)
        {
		if( setboardorigin(board, -1) )
		{
			board_regenspecial(brd.filename,DIR_MODE_ORIGIN,NULL);
		}
		else
		{
			char olddirect[PATHLEN];
	    		setbdir(DIR_MODE_ORIGIN, olddirect, brd.filename);
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
	setboardtitle(brd.filename, 1);	
	/* update .DIR END   */
	RETURN_LONG(0);
}


PHP_FUNCTION(bbs_doforward)
{
    char *board,*filename, *tit, *target;
    int board_len,filename_len,tit_len,target_len;
    boardheader_t bh;
	char fname[STRLEN];
	long big5,noansi;
    struct boardheader *bp;
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

    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(-9);
    }
    if (getboardnum(board, &bh) == 0)
        RETURN_LONG(-11); //"错误的讨论区";
    if (!check_read_perm(getCurrentUser(), &bh))
        RETURN_LONG(-11); //您无权阅读本版;

    setbfile(fname, bp->filename, filename);

    if( !file_exist(fname) )
        RETURN_LONG(-7);

    snprintf(title, 511, "%.50s(转寄)", tit);

    if( !strchr(target, '@') ){
        mail_file(getCurrentUser()->userid, fname, u->userid, title,0, NULL);
		RETURN_LONG(1);
	}else{
		if( big5 == 1)
			conv_init(getSession());
		if( bbs_sendmail(fname, title, target, 0, big5, noansi, getSession()) == 0){
			RETURN_LONG(1);
		}else
			RETURN_LONG(-10);
	}
}

/**
 * 转贴文章
 * int bbs_docross(string board,int id,string target,int out_go);
 * return  0 :seccess
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
    char *board,*target;
    int  board_len,target_len;
    long  id,out_go;
    struct boardheader *src_bp;
	struct boardheader *dst_bp;
	struct fileheader f;
    int  ent;
    int  fd;
    struct userec *u = NULL;
    char path[256],ispost[10];
    
    int ac = ZEND_NUM_ARGS();
    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "slsl", &board, &board_len, &id, &target, &target_len, &out_go) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	u = getCurrentUser();
	src_bp = getbcache(board);
	if (src_bp == NULL)
	    RETURN_LONG(-1);
	strcpy(board, src_bp->filename);
	if(!check_read_perm(u, src_bp))
		RETURN_LONG(-1);
    
    dst_bp = getbcache(target);
    if (dst_bp == NULL)
        RETURN_LONG(-2);
    strcpy(target, dst_bp->filename);

#ifndef NINE_BUILD    
    if (!strcmp(board,target))
        RETURN_LONG(-8);
#endif
    
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
	
	strcpy(ispost ,((dst_bp->flag & BOARD_OUTFLAG) && out_go)?"s":"l");
	setbfile(path, board, f.filename);
	if (post_cross(u, target, board, f.title, path, 0, 0, ispost[0], 0, getSession()) == -1)
	    RETURN_LONG(-10);
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
    struct boardheader *src_bp, *commend_bp;
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
	boardheader_t* bp;
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
    struct boardheader *pbh;
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
