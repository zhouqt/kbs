#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_smth_bbs.h"  

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

    filter_control_char(title);
    if (tlen == 0)
        RETURN_LONG(-3); //标题为NULL
    if (true == checkreadonly(board) || !haspostperm(getCurrentUser(), board))
        RETURN_LONG(-4); //此讨论区是唯读的, 或是您尚无权限在此发表文章.
    if (deny_me(getCurrentUser()->userid, board) && !HAS_PERM(getCurrentUser(), PERM_SYSOP))
        RETURN_LONG(-5); //很抱歉, 你被版务人员停止了本版的post权利.

    if (abs(time(0) - getSession()->currentuinfo->lastpost) < 6) {
        getSession()->currentuinfo->lastpost = time(0);
        RETURN_LONG(-6); // 两次发文间隔过密, 请休息几秒后再试
    }
    getSession()->currentuinfo->lastpost = time(0);

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

    strncpy(post_file.title, title, ARTICLE_TITLE_LEN - 1);
	post_file.title[ARTICLE_TITLE_LEN - 1] = '\0';
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
#ifdef HAVE_BRC_CONTROL
    brc_update(getCurrentUser()->userid, getSession());
#endif
    if (!junkboard(board)) {
        getCurrentUser()->numposts++;
    }
    RETURN_LONG(retvalue);
}




