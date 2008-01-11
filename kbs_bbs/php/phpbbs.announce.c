#include "php_kbs_bbs.h"  

PHP_FUNCTION(bbs_ann_num2path)
{
    char *path,*userid;
    int path_len,userid_len;
    struct userec *user;
	char buf[256];

    if (zend_parse_parameters(2 TSRMLS_CC, "ss", &path, &path_len, &userid, &userid_len) != SUCCESS)
        WRONG_PARAM_COUNT;
    
    if (userid != 0) {
        if (getuser(userid, &user) == 0)
            RETURN_FALSE;
    } else
        user = getCurrentUser();

	buf[0]='\0';
	if(ann_numtopath(buf, path, user)==NULL){
		RETURN_FALSE;
	}else{
	    RETURN_STRING(buf, 1);
	}
}

PHP_FUNCTION(bbs_ann_traverse_check)
{
    char *path,*userid;
    int path_len,userid_len;
    struct userec *user;

    if (zend_parse_parameters(2 TSRMLS_CC, "ss", &path, &path_len, &userid, &userid_len) != SUCCESS)
        WRONG_PARAM_COUNT;
    
    if (userid != 0) {
        if (getuser(userid, &user) == 0)
            RETURN_FALSE;
    } else
        user = getCurrentUser();
    
    RETURN_LONG(ann_traverse_check(path, user));
}

PHP_FUNCTION(bbs_ann_get_board)
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
PHP_FUNCTION(bbs_getannpath)
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



/*
 * add import path,     annnounce.c
 * @author stiger
 */
PHP_FUNCTION(bbs_add_import_path)
{
    int ac = ZEND_NUM_ARGS();
	char * path;
	int path_len;
	char * title;
	int title_len;
	long num;
	char *im_path[ANNPATH_NUM];
	char *im_title[ANNPATH_NUM];
	time_t im_time=0;
	int im_select=0;
	char buf[MAXPATH];
	char board[MAXPATH];
	char *c;
    const struct boardheader *bp;

	if(ac == 2){
    	if ( zend_parse_parameters(2 TSRMLS_CC, "sl", &path, &path_len, &num) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		title = NULL;
	}else if(ac == 3){
    	if ( zend_parse_parameters(3 TSRMLS_CC, "ssl", &path, &path_len, &title, &title_len, &num) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
	}else
		WRONG_PARAM_COUNT;

	/* first ,check the path */
	if(strstr(path,"..") || strstr(path,"SYSHome") ) /* SYSHome? from bbs0an.c */
		RETURN_LONG(0);
	if(path[0] == '\0')
		RETURN_LONG(0);
	path_len = strlen(path);
	if(path[path_len-1]=='/')
		path[path_len-1]='\0';
	if(path[0]=='/')
		snprintf(buf, sizeof(buf), "0Announce%s", path);
	else if(strncmp(path,"0Announce",9))
		snprintf(buf, sizeof(buf), "0Announce/%s", path);
	else
		snprintf(buf, sizeof(buf), "%s", path);

	if(strncmp(buf,"0Announce/groups/",17))
		RETURN_LONG(0);

	if((c=strchr(buf+17,'/'))==NULL)
		RETURN_LONG(0);
	strcpy(board,c+1);
	if((c=strchr(board,'/'))!=NULL) *c='\0';
    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(0);
    }
    if(! is_BM(bp, getCurrentUser()))
        RETURN_LONG(0);

	if (ann_traverse_check(buf, getCurrentUser()) < 0)
		RETURN_LONG(0);

	if(num < 0 || num >= ANNPATH_NUM)
		RETURN_LONG(0);

	load_import_path(im_path,im_title,&im_time,&im_select, getSession());

	efree(im_path[num]);
	if ( (im_path[num] = emalloc(strlen(buf)+1)) ==NULL) {
		RETURN_LONG(0);
	}
	
	strcpy(im_path[num],buf);

	if(title == NULL || title[0]==0 ){
		MENU pm;

		bzero(&pm,sizeof(pm));
		pm.path = im_path[num];
		a_loadnames(&pm, getSession());
		strncpy(buf, pm.mtitle, MAXPATH - 1);
		buf[MAXPATH - 1]=0;
		a_freenames(&pm);
	}else{
		strncpy(buf, title, MAXPATH - 1);
		buf[MAXPATH - 1]=0;
	}

	efree(im_title[num]);
	if ( (im_title[num] = emalloc(strlen(buf)+1)) ==NULL) {
		RETURN_LONG(0);
	}
	strcpy(im_title[num],buf);

	save_import_path(im_path,im_title,&im_time, getSession());

	free_import_path(im_path,im_title,&im_time);

	RETURN_LONG(1);
}

PHP_FUNCTION(bbs_get_import_path)
{
    zval *element;
	char *im_path[ANNPATH_NUM];
	char *im_title[ANNPATH_NUM];
	time_t im_time=0;
	int im_select=0;
	int i;

    if (array_init(return_value) == FAILURE) {
        RETURN_FALSE;
    }

	load_import_path(im_path,im_title,&im_time,&im_select, getSession());

	for(i=0;i<ANNPATH_NUM;i++){
        MAKE_STD_ZVAL(element);
        array_init(element);
    	add_assoc_string(element, "PATH", im_path[i], 1);
    	add_assoc_string(element, "TITLE", im_title[i], 1);
        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
    }

	free_import_path(im_path,im_title,&im_time);

}

static int get_pos(char * s)
{
    struct stat st;
    FILE* fp;
    char buf[240],buf2[100],tt[100];
    int i,j;
    if(stat(s, &st)==-1) return -1;
    strcpy(buf, s);
    i=strlen(buf)-1;
    while(buf[i]!='/') i--;
    i++;
    strcpy(buf2, buf+i);
    strcpy(buf+i, ".Names");
    fp=fopen(buf, "r");
    if(fp==NULL) return -1;
    tt[0]=0;
    j=0;
    while(!feof(fp))
    {
	if(!fgets(buf, 240, fp)) {
	    fclose(fp);
	    return -1;
	}
	if(buf[0]) buf[strlen(buf)-1]=0;
	if(!strncmp(buf, "Name=", 5)) {
	    strcpy(tt, buf+5);
	}
	if(!strncmp(buf, "Path=~/", 7)) {
	    j++;
	    if(!strcmp(buf+7, buf2)) {
		fclose(fp);
		return j;
	    }
	}
    }
    fclose(fp);
    return -1;
}

PHP_FUNCTION(bbs_x_search)
{
    int toomany, res_total;
    int ac = ZEND_NUM_ARGS();
    int char_len;   
    long pos;
    char *qn;
    zval* element;
    zval *total_records;
    struct sockaddr_in addr;
    FILE* sockfp;
    int sockfd, i, j, k, ttt;
    char buf[256];
    char s1[30], s2[30], *pp;
    const char *ip;
    #define MAX_KEEP 100
    char res_title[MAX_KEEP][80],res_filename[MAX_KEEP][200],res_path[MAX_KEEP][200],res_content[MAX_KEEP][1024];
    int res_flag[MAX_KEEP];

    if(ac != 3 || zend_parse_parameters(3 TSRMLS_CC,"slz",&qn,&char_len,&pos,&total_records) ==FAILURE){
        WRONG_PARAM_COUNT;
    }

    /*
     * check for parameter being passed by reference 
     */
    if (!PZVAL_IS_REF(total_records)) {
        zend_error(E_WARNING, "Parameter wasn't passed by reference");
        RETURN_FALSE;
    }

    if (array_init(return_value) == FAILURE)
        RETURN_FALSE;

    if ((ip = sysconf_str("QUERY_SERVER")) == NULL) return;
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))==-1) return;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family=AF_INET;    
    addr.sin_addr.s_addr=inet_addr(ip);
    addr.sin_port=htons(4875);
    if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr))<0) return;
    sockfp=fdopen(sockfd, "r+");
    fprintf(sockfp, "\n%ld\n%s\n", pos, qn);
    fflush(sockfp);
    fscanf(sockfp, "%d %d %d\n", &toomany, &i, &res_total);
    for(i=0;i<res_total;i++) {
        fgets(buf, 256, sockfp);
        if(buf[0]&&buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]=0;
        strncpy(res_title[i], buf, ARTICLE_TITLE_LEN);
        res_title[i][ARTICLE_TITLE_LEN-1] = 0;

        fgets(buf, 256, sockfp);
        if(buf[0]&&buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]=0;
        strncpy(res_filename[i], buf, 200);
        res_filename[i][199] = 0;
    }
    fclose(sockfp);
    close(sockfd);
    for(i=0;i<res_total;i++)
	if(get_pos(res_filename[i])==-1) {
	    strcpy(res_path[i], "无效文章");
	    res_flag[i]=1;
	}
        else {
	    char buf[200],buf2[200];
	    res_flag[i]=0;
	    if(!strncmp(res_filename[i], "0Announce/groups/", 17)) {
		j=17;
		while(res_filename[i][j]!='/') j++;
		j++;
		k=j;
		while(res_filename[i][j]!='/') j++;
		strcpy(buf,res_filename[i]+k);
		buf[j-k]=0;
		strcpy(res_path[i],buf);
		k=strlen(res_path[i]);
		while(1) {
		    j++;
		    while(res_filename[i][j]!='/'&&res_filename[i][j]) j++;
		    strcpy(buf2, res_filename[i]);
		    buf2[j] = 0;
		    sprintf(res_path[i]+k, "-%d", get_pos(buf2));
		    k = strlen(res_path[i]);
		    if(!res_filename[i][j]) break;
		}
	    }
	    else {
		j=10;
		strcpy(res_path[i], "精华区");
		k=strlen(res_path[i]);
		while(1) {
		    j++;
		    while(res_filename[i][j]!='/'&&res_filename[i][j]) j++;
		    strcpy(buf2, res_filename[i]);
		    buf2[j] = 0;
		    sprintf(res_path[i]+k, "-%d", get_pos(buf2));
		    k = strlen(res_path[i]);
		    if(!res_filename[i][j]) break;
		}
	    }
	}
    for(i=0;i<res_total;i++) 
    if(res_title[i][0]){
        if(strlen(res_title[i])>30){
	    j=30;
	    while(res_title[i][j]!=' '&&res_title[i][j])j++;
	    res_title[i][j]=0;
        }
	j=strlen(res_title[i])-1;
	if((j>=0)&&res_title[i][j]==' ')j--;
	j++;
	res_title[i][j]=0;
    }
    for(i=0;i<res_total;i++) {
	pp=res_filename[i]+strlen(res_filename[i])-1;
	while(*pp!='/') pp--;
	strcpy(s1, pp+1);
	if(strlen(s1)>7)
	for(j=i+1;j<res_total;j++) {
	    pp=res_filename[j]+strlen(res_filename[j])-1;
	    while(*pp!='/') pp--;
	    strcpy(s2, pp+1);
	    if(!strcmp(s1,s2)) {
		res_flag[j]=1;
		strcpy(res_path[j], "重复文章");
	    }
	}
    }

    for (ttt=0; ttt < res_total; ttt++) {
        char buf[10*1024],out[10*1024],out2[10*1024];
        FILE* fp;
        int i,j,k,l,fsize=0,t=0,p=0,inc=0;
        res_content[ttt][0] = 0;
        fp = fopen(res_filename[ttt], "rb");
        if(!fp) continue;
        fsize = fread(buf, 1, 10*1024, fp);
        fclose(fp);
        memset(out, 0, fsize);
        for(i=0;i<fsize;i++) {
            if(buf[i]==0x1b) {
                j=i;
                while(!((buf[j]>='a'&&buf[j]<='z')||(buf[j]>='A'&&buf[j]<='Z'))&&j<fsize) {
                    buf[j] = 0;
                    j++;
                }
                if(j<fsize)
                    buf[j] = 0;
            }
        }
        i=0;
        if(qn[i]=='=') {
            while(i<strlen(qn)&&qn[i]!=' ') i++;
            if(i>=strlen(qn)) i=0;
        }
        while(i<strlen(qn)) {
            if((qn[i]>='a'&&qn[i]<='z')||(qn[i]>='A'&&qn[i]<='Z')) {
                j=i;
                while((qn[j]>='a'&&qn[j]<='z')||(qn[j]>='A'&&qn[j]<='Z')||(qn[j]>='0'&&qn[j]<='9')) j++;
                for(k=0;k<fsize-(j-i)+1;k++)
                    if(!strncasecmp(qn+i,buf+k,j-i)&&(k==0||!((buf[k-1]>='a'&&buf[k-1]<='z')||(buf[k-1]>='A'&&buf[k-1]<='Z')))&&
                        (k+j-i==fsize||!((buf[k+j-i]>='a'&&buf[k+j-i]<='z')||(buf[k+j-i]>='A'&&buf[k+j-i]<='Z'))))
                        for(l=0;l<j-i;l++) if(!out[k+l]){out[k+l]=1;t++;}
                i=j-1;
            }
            if(qn[i]>='0'&&qn[i]<='9') {
                j=i;
                while(qn[j]>='0'&&qn[j]<='9') j++;
                for(k=0;k<fsize-(j-i)+1;k++)
                    if(!strncmp(qn+i,buf+k,j-i)&&(k==0||!(buf[k-1]>='0'&&buf[k-1]<='9'))&&
                        (k+j-i==fsize||!(buf[k+j-i]>='0'&&buf[k+j-i]<='9')))
                        for(l=0;l<j-i;l++) if(!out[k+l]){out[k+l]=1;t++;}
                i=j-1;
            }
            if(qn[i]<0&&qn[i+1]<0) {
                j=i+2;
                for(k=0;k<fsize-(j-i)+1;k++)
                    if(!strncmp(qn+i,buf+k,j-i))
                        for(l=0;l<j-i;l++) if(!out[k+l]){out[k+l]=1;t++;}
                i=j-1;
            }
            i++;
        }
        if(t>=20) {
            for(k=0;k<fsize-4;k++) {
                if(out[k]==0&&out[k+1]==1&&out[k+2]==0) {
                    out[k+1]=0;
                    t--;
                }
                if(out[k]==0&&out[k+1]==1&&out[k+2]==1&&out[k+3]==0) {
                    out[k+1]=0;
                    out[k+2]=0;
                    t-=2;
                }
                if(t<10) break;
            }
        }
        if(t==0) {
            continue;
        }
        while(t<240&&t<fsize) {
            memset(out2, 0, fsize);
            t=0;
            for(k=0;k<fsize;k++) 
            if(!out2[k]) {
                if(out[k]||(k>0&&out[k-1])||(k<fsize-1&&out[k+1])) {
                    if(out[k]==1)
                        out2[k]=1;
                    else {
                        out2[k]=2;
                        if(!out[k]&&buf[k]<0) {
                            if(k>0&&out[k-1]&&k<fsize-1) out2[k+1]=2;
                            if(k>0&&k<fsize-1&&out[k+1]) out2[k-1]=2;
                            t++;
                        }
                    }
                    t++;
                }
            }
            memcpy(out,out2,fsize);
        }
        pp = res_content[ttt];
        j=0; t = 0;
        for(i=0;i<fsize;i++)
        if(out[i]) {
            if(i>0&&out[i-1]==0) {
                sprintf(pp, "...");
                pp+=3;
                j+=3;
            }
            if(out[i]==1&&!inc) {
                sprintf(pp, "<font class=\"f001\">");
                pp += 19;
                inc = 1;
            }
            else if(out[i]!=1&&inc) {
                sprintf(pp, "</font>");
                pp += 7;
                inc = 0;
            }
            if(buf[i]=='\n') *(pp++) = ' ';
            else if(buf[i]) *(pp++) = buf[i];
            if(p) p=0;
            else if(buf[i]<0) p=1;
            j++;
            if(j>=84&&p==0) {
                t++;
                if(t>=3) break;
                sprintf(pp, "<br/>");
                pp += 5;
                j=0;
            }
        }
        if(inc) {
            sprintf(pp, "</font>");
            pp += 7;
        }
        *pp = 0;
    }


    for ( i=0; i < res_total; i++ ) {
        MAKE_STD_ZVAL ( element );
        array_init ( element );

        add_assoc_string ( element, "title", res_title[i], 1 );
        add_assoc_string ( element, "filename", res_filename[i], 1 );
        add_assoc_string ( element, "path", res_path[i], 1 );
        add_assoc_string ( element, "content", res_content[i], 1 );

        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void *) &element, sizeof(zval *), NULL);
    }
    ZVAL_LONG(total_records, toomany);
}

/**
 * int bbs_read_ann_dir(string path,string board,string path2,array articles[,long seespecial])
 * $articles is the articles/sub-directories in this directory except BMS/SYSOPS.
 * $articles = array(
 *               int 'FLAG',  // 0: error;1: dir;2: file;3: file with attach
 *               string 'TITLE',
 *               string 'PATH',
 *               string 'BM',
 *               int 'TIME'
 *               );
 * return 0 :seccess;
 *        -1:dir NOT exist
 *        -2:can NOT find .Names file
 *        -3:No article here
 *        -9:system error;
 */
PHP_FUNCTION(bbs_read_ann_dir)
{
    char  *path;
    int   path_len;
    zval *board,*path2,*element,*articles;
    
    struct userec *u;
    char pathbuf[256];
    int len;
    MENU me;
    int i,j;
    char *id,*ptr;
    char buf[256];
    char r_title[STRLEN],r_path[256],r_bm[256];
    int  r_flag,r_time;
    struct stat st;
    bool cansee;
    long seespecial=0;
    

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"szza|l",&path,&path_len,&board,&path2,&articles,&seespecial) == FAILURE) {
        WRONG_PARAM_COUNT;
    }

    if (!PZVAL_IS_REF(board) || !PZVAL_IS_REF(path2)) {
       	zend_error(E_WARNING, "Parameter wasn't passed by reference");
       	RETURN_FALSE;
    }
    
    if(array_init(articles) != SUCCESS)
        RETURN_LONG(-9);
    
    u = getCurrentUser();
    if (!u)
        RETURN_LONG(-9);
    
    if (strstr(path, "..") || strstr(path, "SYSHome"))  
		RETURN_LONG(-1);
    if (path[0] != '\0') {
        len = strlen(path);
        if (path[len - 1] == '/')
            path[len - 1] = '\0';
        if (path[0] == '/')
            snprintf(pathbuf, 255, "0Announce%s", path);
        else
            snprintf(pathbuf, 255, "0Announce/%s", path);
        if (ann_traverse_check(pathbuf, u) < 0)
			RETURN_LONG(-1);
    } else
        strcpy(pathbuf, "0Announce");

    ZVAL_STRING(path2,pathbuf,1);

    bzero(&me,sizeof(MENU));
    me.path = pathbuf;
    if(a_loadnames(&me,NULL)==-1)
        RETURN_LONG(-2);

    buf[0] = '\0';
    ann_get_board(pathbuf, buf, sizeof(buf));
    ZVAL_STRING(board,buf,1);
    if (me.num <= 0) 
        RETURN_LONG(-3);

    me.now = 0;
    j = 0;
    for (i = 0; i < me.num; i++) {
        strncpy(r_title, M_ITEM(&me,i)->title, sizeof(r_title) - 1);
        r_title[sizeof(r_title) - 1] = '\0';
        if (strlen(r_title) <= 39) {
            id = "";
        } else {
            if ((ptr = strchr(r_title + 38, '(')) != NULL) {
                *ptr = '\0';
                id = ptr + 1;
                if (strncmp(id, "BM: ", 4) == 0)
                    id += 4;
                if ((ptr = strchr(id, ')')) != NULL)
                    *ptr = '\0';
            } else if ((ptr = strchr(r_title + 38, ' ')) != NULL) {
                *ptr = '\0';
                id = ptr + 1;
            } else
                id = "";
        }
        snprintf(buf, sizeof(buf), "%s/%s", me.path, M_ITEM(&me,i)->fname);
        ptr = strchr(me.path, '/');
        
        if (stat(buf, &st) == -1) {
            r_time = 0;
            r_flag = 0;
        } else {
            r_time = st.st_mtime;
            if (S_ISDIR(st.st_mode))
                r_flag = 1;
            else 
                r_flag = M_ITEM(&me,i)->attachpos?3:2;
        }
        
        snprintf(r_path, sizeof(r_path), "%s/%s", ptr == NULL ? "" : ptr, M_ITEM(&me,i)->fname);
        strncpy(r_bm,id[0]?id:"",sizeof(r_bm)-1);
        r_bm[sizeof(r_bm)-1] = '\0';
        
        cansee = false;
        if (strcmp(r_bm, "BMS")==0) {
            if (HAS_PERM(getCurrentUser(), PERM_BOARDS) && (seespecial!=0))
                cansee = true;
        }
        else if (strcmp(r_bm, "SYSOPS")==0) {
            if (HAS_PERM(getCurrentUser(), PERM_SYSOP) && (seespecial!=0))
                cansee = true;
        }
        else
            cansee = true;
        if (cansee) {
            MAKE_STD_ZVAL(element);
            array_init(element);
            add_assoc_string(element,"TITLE",r_title,1);
            add_assoc_string(element,"PATH",r_path,1);
            add_assoc_string(element,"BM",r_bm,1);
            add_assoc_long(element,"FLAG",r_flag);
            add_assoc_long(element,"TIME",r_time);
            add_assoc_string(element,"FNAME",M_ITEM(&me,i)->fname,1);
            zend_hash_index_update(Z_ARRVAL_P(articles),j,(void*) &element, sizeof(zval*), NULL);
            j ++;
        }
        me.now++;
    }
    a_freenames(&me);
    RETURN_LONG(0);
}

/* get ann dir's title from parent .Names, pig2532 2006.3
string bbs_ann_get_title(string path);
*/
PHP_FUNCTION(bbs_ann_get_title)
{
    int ac, path_len, i;
    char *path, *fname, title[STRLEN], fpath[PATHLEN];
    MENU me;
    bool find=false;

    ac = ZEND_NUM_ARGS();
    if((ac != 1) || (zend_parse_parameters(1 TSRMLS_CC, "s", &path, &path_len) != SUCCESS))
    {
        WRONG_PARAM_COUNT;
    }

    strncpy(fpath, path, PATHLEN);
    fpath[PATHLEN - 1] = '\0';
    if(strcmp(fpath, "0Announce") == 0)
    {
        RETURN_STRING("精华公布栏", 1);
    }
    if(!(dashd(fpath) || dashf(fpath)))
    {
        RETURN_STRING("精华区目录不存在", 1);
    }
    fname = strrchr(fpath, '/');
    if(fname == NULL)
    {
        RETURN_STRING("找不到精华区目录", 1);
    }
    *fname = '\0';
    fname++;

    bzero(&me, sizeof(me));
    me.path = fpath;
    me.level = PERM_BOARDS;
    a_loadnames(&me, getSession());
    for(i=0; i<me.num; i++)
    {
        if(strcmp(fname, M_ITEM(&me,i)->fname) == 0)
        {
            find = true;
            break;
        }
    }
    if(!find)
    {
        a_freenames(&me);
        RETURN_STRING("找不到精华区项目", 1);
    }
    strncpy(title, M_ITEM(&me,i)->title, STRLEN);
    title[STRLEN - 1] = '\0';
    a_freenames(&me);
    RETURN_STRING(title, 1);
}


/* BM functions in announce below
 * by pig2532@newsmth
 * NOTICE: These functions do not check BM perm,
 *         please use bbs_ann_traverse_check to check perm first in php.
 */

static void ann_write_bmlog(char *path)
{
    char *ptr, board[STRLEN];
    if((ptr = strstr(path, "groups/")) != NULL)
        ann_get_board(ptr, board, sizeof(board));
    if(board[0] != '\0')
        bmlog(getCurrentUser()->userid, board, 13, 1);
}

/* create directory
bbs_ann_mkdir(string path, string fname, string title, string bm);
return:
     0: success
    -1: path not found
    -2: filename not valid
    -3: file already exists
    -4: a_savenames failed
*/
PHP_FUNCTION(bbs_ann_mkdir)
{
    int ac;
    char *path, *sfname, *title, *bm, fname[80];
    int path_len, fname_len, title_len, bm_len;
    char fpath[PATHLEN], buf[STRLEN];
    MENU me;
    FILE *fp;

    ac = ZEND_NUM_ARGS();
    if((ac != 4) || (zend_parse_parameters(4 TSRMLS_CC, "ssss", &path, &path_len, &sfname, &fname_len, &title, &title_len, &bm, &bm_len) != SUCCESS))
    {
        WRONG_PARAM_COUNT;
    }
    
    if(!dashd(path))
    {
        RETURN_LONG(-1);
    }
#ifdef ANN_AUTONAME
    sprintf(fname, "D%X", time(0) + rand());
#else
    strncpy(fname, sfname, 80);
    fname[79] = '\0';
    if(!valid_fname(fname))
    {
        RETURN_LONG(-2);
    }
#endif
    sprintf(fpath, "%s/%s", path, fname);
    if(dashf(fpath) || dashd(fpath))
    {
        RETURN_LONG(-3);
    }

    mkdir(fpath, 0755);
    chmod(fpath, 0755);

    bzero(&me, sizeof(me));
    me.path = path;
    me.level = PERM_BOARDS;
    a_loadnames(&me, getSession());
    if(bm[0] == '\0')
        sprintf(buf, "%-38.38s", title);
    else
        sprintf(buf, "%-38.38s(BM: %s)", title, bm);
    a_additem(&me, buf, fname, NULL, 0, 0);
    if(a_savenames(&me) == 0)
    {
        sprintf(fpath, "%s/%s/.Names", path, fname);
        if((fp = fopen(fpath, "w")) != NULL)
        {
            fprintf(fp, "#\n");
            fprintf(fp, "# Title=%s", buf);
            fprintf(fp, "#\n");
            fclose(fp);
        }
    }
    else
    {
        a_freenames(&me);
        RETURN_LONG(-4);
    }

    a_freenames(&me);
    ann_write_bmlog(path);
    RETURN_LONG(0);
}

/* create file
bbs_ann_mkfile(string path, string fname, string title, string content[, long import]);
 notice: set import=1 to make file from import file.
return:
     0: success
    -1: path not found
    -2: fname not valid
    -3: file already exists
    -4: error to write file
    -5: a_savenames failed
*/
PHP_FUNCTION(bbs_ann_mkfile)
{
    int ac;
    char *path, *sfname, *title, *content, fname[80];
    int path_len, fname_len, title_len, content_len;
    FILE *fp;
    MENU me;
    long import=0;
    char fpath[PATHLEN], buf[STRLEN];

    ac = ZEND_NUM_ARGS();
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssss/|l", &path, &path_len, &sfname, &fname_len, &title, &title_len, &content, &content_len, &import) != SUCCESS)
    {
        WRONG_PARAM_COUNT;
    }

    if(!dashd(path))
    {
        RETURN_LONG(-1);
    }
#ifdef ANN_AUTONAME
    sprintf(fname, "A%X", time(0) + rand());
#else
    strncpy(fname, sfname, 80);
    fname[79] = '\0';
    if(!valid_fname(fname))
    {
        RETURN_LONG(-2);
    }
#endif
    sprintf(fpath, "%s/%s", path, fname);
    if(dashf(fpath) || dashd(fpath))
    {
        RETURN_LONG(-3);
    }

    if(import)
    {
        char importpath[PATHLEN], attachpath[PATHLEN];
        sprintf(importpath, "tmp/bm.%s", getCurrentUser()->userid);
        if(!dashf(importpath))
        {
            RETURN_LONG(0);
        }
        sprintf(attachpath, "tmp/bm.%s.attach", getCurrentUser()->userid);
        if(dashf(attachpath))
        {
            a_append_attachment(importpath, attachpath);
            my_unlink(attachpath);
        }
        f_mv(importpath, fpath);
        chmod(fpath, 0644);
    }
    else
    {
        if((fp = fopen(fpath, "w")) == NULL)
        {
            RETURN_LONG(-4);
        }
        fwrite(content, content_len, 1, fp);
        fclose(fp);
        chmod(fpath, 0644);
    }

    bzero(&me, sizeof(me));
    me.path = path;
    me.level = PERM_BOARDS;
    a_loadnames(&me, getSession());
    sprintf(buf, "%-38.38s %s ", title, getCurrentUser()->userid);
    a_additem(&me, buf, fname, NULL, 0, 0);
    if(a_savenames(&me) != 0)
    {
        a_freenames(&me);
        RETURN_LONG(-5);
    }

    a_freenames(&me);
    ann_write_bmlog(path);
    RETURN_LONG(0);

}

/* modify directory
bbs_ann_editdir(string path, string fname, string newfname, string title, string bm);
return:
     0: success
    -1: dir not found
    -2: newfname not valid
    -3: newfname already exists
    -4: dir record not found in .Names
    -5: a_savenames failed
*/
PHP_FUNCTION(bbs_ann_editdir)
{
    int ac, i;
    char *path, *fname, *newfname, *title, *bm;
    int path_len, fname_len, newfname_len, title_len, bm_len;
    MENU me;
    char fpath[PATHLEN], newfpath[PATHLEN];
    bool find = false;

    ac = ZEND_NUM_ARGS();
    if((ac != 5) || (zend_parse_parameters(5 TSRMLS_CC, "sssss", &path, &path_len, &fname, &fname_len, &newfname, &newfname_len, &title, &title_len, &bm, &bm_len) != SUCCESS))
    {
        WRONG_PARAM_COUNT;
    }

    snprintf(fpath, PATHLEN, "%s/%s", path, fname);
    if(!dashd(fpath))
    {
        RETURN_LONG(-1);
    }
    if(!valid_fname(newfname))
    {
        RETURN_LONG(-2);
    }
    if(strcmp(fname, newfname))
    {
        snprintf(newfpath, PATHLEN, "%s/%s", path, newfname);
        if(dashf(newfpath) || dashd(newfpath))
        {
            RETURN_LONG(-3);
        }
        f_mv(fpath, newfpath);
    }
    bzero(&me, sizeof(me));
    me.path = path;
    me.level = PERM_BOARDS;
    a_loadnames(&me, getSession());
    for(i=0; i<me.num; i++)
    {
        if(strcmp(fname, M_ITEM(&me,i)->fname) == 0)
        {
            strncpy(M_ITEM(&me,i)->fname, newfname, 80);
            M_ITEM(&me,i)->fname[79] = '\0';
            if(bm[0] == '\0')
                sprintf(M_ITEM(&me,i)->title, "%-38.38s", title);
            else
                sprintf(M_ITEM(&me,i)->title, "%-38.38s(BM: %s)", title, bm);
            find = true;
            break;
        }
    }
    if(!find)
    {
        a_freenames(&me);
        RETURN_LONG(-4);
    }
    if(a_savenames(&me) != 0)
    {
        a_freenames(&me);
        RETURN_LONG(-5);
    }

    a_freenames(&me);
    RETURN_LONG(0);

}

/* return file content for editing
bbs_ann_originfile(string path);
*/
PHP_FUNCTION(bbs_ann_originfile)
{
    int ac, path_len;
    char *path;
    FILE* fp;
	char buf[512];
    char *content, *ptr;
    int chunk_size=51200, calen, clen, buflen;

    ac = ZEND_NUM_ARGS();
    if((ac != 1) || (zend_parse_parameters(1 TSRMLS_CC, "s", &path, &path_len) != SUCCESS))
    {
        WRONG_PARAM_COUNT;
    }

	if (!dashf(path))
    {
		RETURN_LONG(-1);
	}
    fp = fopen(path, "r");
    if (fp == NULL)
    {
        RETURN_LONG(-1);
    }
	
    calen = chunk_size;
    content = (char *)emalloc(calen);
    clen = 0;
    ptr = content;
    while (skip_attach_fgets(buf, sizeof(buf), fp) != 0) {
        buflen = strlen(buf);
        if((clen + buflen) >= (calen + 1))
        {
            calen += chunk_size;
            content = (char *)erealloc(content, calen);
            ptr = content + clen;
        }
        memcpy(ptr, buf, buflen);
        clen += buflen;
        ptr += buflen;
    }
    fclose(fp);
    content[clen] = '\0';
    RETURN_STRINGL(content, clen + 1, 0);
}

/* edit file
bbs_ann_editfile(string path, string fname, string newfname, string title, string content);
return:
     0: success
    -1: file not found
    -2: newfname not valid
    -3: newfname already exists
    -4: failed to open file
    -5: file not found in .Names
    -6: a_savenames failed
*/
PHP_FUNCTION(bbs_ann_editfile)
{
    int ac, asize, i;
    char *path, *fname, *newfname, *title, *content;
    int path_len, fname_len, newfname_len, title_len, content_len;
    char fpath[PATHLEN], newfpath[PATHLEN], tmpfpath[PATHLEN], buf[256];
    FILE *fin, *fout;
    bool find = false;
    MENU me;

    ac = ZEND_NUM_ARGS();
    if((ac != 5) || (zend_parse_parameters(5 TSRMLS_CC, "sssss/", &path, &path_len, &fname, &fname_len, &newfname, &newfname_len, &title, &title_len, &content, &content_len) != SUCCESS))
    {
        WRONG_PARAM_COUNT;
    }

    snprintf(fpath, PATHLEN, "%s/%s", path, fname);
    if(!dashf(fpath))
    {
        RETURN_LONG(-1);
    }
    if(!valid_fname(newfname))
    {
        RETURN_LONG(-2);
    }

    if((fin = fopen(fpath, "r")) == NULL)
    {
        RETURN_LONG(-4);
    }
    snprintf(tmpfpath, PATHLEN, "tmp/%s.%d.editpost", getCurrentUser()->userid, getpid());
    if((fout = fopen(tmpfpath, "w")) == NULL)
    {
        RETURN_LONG(-4);
    }
    fwrite(content, content_len, 1, fout);
    while((asize = -attach_fgets(buf, sizeof(buf), fin)) != 0)
    {
        if(asize > 0)
        {
            put_attach(fin, fout, asize);
        }
    }
    fclose(fin);
    fclose(fout);
    f_cp(tmpfpath, fpath, O_TRUNC);
    unlink(tmpfpath);

    if(strcmp(fname, newfname))
    {
        snprintf(newfpath, PATHLEN, "%s/%s", path, newfname);
        if(dashf(newfpath) || dashd(newfpath))
        {
            RETURN_LONG(-3);
        }
        f_mv(fpath, newfpath);
    }

    bzero(&me, sizeof(me));
    me.path = path;
    me.level = PERM_BOARDS;
    a_loadnames(&me, getSession());
    for(i=0; i<me.num; i++)
    {
        if(strcmp(fname, M_ITEM(&me,i)->fname) == 0)
        {
            strncpy(M_ITEM(&me,i)->fname, newfname, 80);
            M_ITEM(&me,i)->fname[79] = '\0';
            sprintf(M_ITEM(&me,i)->title, "%-38.38s %s ", title, getCurrentUser()->userid);
            find = true;
            break;
        }
    }
    if(!find)
    {
        a_freenames(&me);
        RETURN_LONG(-5);
    }
    if(a_savenames(&me) != 0)
    {
        a_freenames(&me);
        RETURN_LONG(-6);
    }

    a_freenames(&me);
    RETURN_LONG(0);

}

/* delete
bbs_ann_delete(string path, string fname);
return:
     0: success
    -1: path not found
    -2: record not found in .Names
    -3: a_savenames failed
*/
PHP_FUNCTION(bbs_ann_delete)
{
    int ac, i;
    char *path, *fname, fpath[PATHLEN];
    int path_len, fname_len;
    MENU me;
    bool find = false;

    ac = ZEND_NUM_ARGS();
    if((ac != 2) || (zend_parse_parameters(2 TSRMLS_CC, "ss", &path, &path_len, &fname, &fname_len) != SUCCESS))
    {
        WRONG_PARAM_COUNT;
    }

    if(!dashd(path))
    {
        RETURN_LONG(-1);
    }

    snprintf(fpath, PATHLEN, "%s/%s", path, fname);
    if(dashd(fpath))
    {
        my_f_rm(fpath);
    }
    else if(dashf(fpath))
    {
        my_unlink(fpath);
    }

    bzero(&me, sizeof(me));
    me.path = path;
    me.level = PERM_BOARDS;
    a_loadnames(&me, getSession());
    for(i=0; i<me.num; i++)
    {
        if(strcmp(fname, M_ITEM(&me,i)->fname) == 0)
        {
            find = true;
            break;
        }
    }
    if(!find)
    {
        a_freenames(&me);
        RETURN_LONG(-2);
    }
    a_delitem(&me,i);
    if(a_savenames(&me) != 0)
    {
        a_freenames(&me);
        RETURN_LONG(-3);
    }

    a_freenames(&me);
    ann_write_bmlog(path);
    RETURN_LONG(0);
}

/* copy to clipboard
bbs_ann_copy(string path, string fnames, long delsource);
note: if fnames contains more than one filenames, use "," to seperate.
      delsource=0 means to copy, else means to cut.
return:
     0: success
    -1: cannot write clipboard file
*/
PHP_FUNCTION(bbs_ann_copy)
{
    int ac;
    char *path, *fnames, clipfile[PATHLEN];
    int path_len, fnames_len;
    long delsource;
    FILE *fp;
    char *ptr;

    ac = ZEND_NUM_ARGS();
    if((ac != 3) || (zend_parse_parameters(3 TSRMLS_CC, "ssl", &path, &path_len, &fnames, &fnames_len, &delsource) != SUCCESS))
    {
        WRONG_PARAM_COUNT;
    }

    snprintf(clipfile, PATHLEN, "tmp/clip/%s.announce", getCurrentUser()->userid);
    if((fp = fopen(clipfile, "w")) == NULL)
    {
        RETURN_LONG(-1);
    }
    fprintf(fp, "DelSource=%ld\n", delsource);
    fprintf(fp, "Path=%s\n", path);
    ptr = fnames;
    while(*fnames != '\0')
    {
        ptr = strchr(fnames, ',');
        if(ptr != NULL)
        {
            *ptr = '\0';
            fprintf(fp, "Filename=%s\n", fnames);
            fnames = ptr + 1;
        }
        else
        {
            break;
        }
    }
    fclose(fp);
    RETURN_LONG(0);
}

/* paste from clipboard
bbs_ann_paste(string path);
note: path is the destination dir
return:
     0: success
    -1: path not found
    -2: source path not found
    -3: board not found
    -4: no bm perm in source board
    -5: error writing .Names
*/
PHP_FUNCTION(bbs_ann_paste)
{
    int ac, i;
    char *dpath, clipfile[PATHLEN], buf[PATHLEN], spath[PATHLEN], sfpath[PATHLEN], dfpath[PATHLEN];
    char fname[STRLEN], newfname[STRLEN], title[STRLEN], *ptr;
    int dpath_len;
    bool delsource = false, find, err = false;
    bool inann = true;
    FILE *fp;
    MENU sme, dme;
    char bname[STRLEN];
    const struct boardheader *bp;
    struct fileheader fh;

    ac = ZEND_NUM_ARGS();
    if((ac != 1) || (zend_parse_parameters(1 TSRMLS_CC, "s", &dpath, &dpath_len) != SUCCESS))
    {
        WRONG_PARAM_COUNT;
    }

    if(!dashd(dpath))
    {
        RETURN_LONG(-1);
    }

    snprintf(clipfile, PATHLEN, "tmp/clip/%s.announce", getCurrentUser()->userid);
    if((fp = fopen(clipfile, "r")) == NULL)
    {
        RETURN_LONG(0);
    }
    spath[0] = '\0';
    bname[0] = '\0';
    while(fgets(buf, sizeof(buf), fp) != NULL)
    {
        if((ptr = strchr(buf, '\n')) != NULL)
            *ptr = '\0';
        if(strncmp(buf, "DelSource=OnBoard", 17) == 0)
        {
            inann = false;    /* the files in clipboard is from a borad, not a announce dir */
            continue;
        }
        else if(strncmp(buf, "DelSource=1", 11) == 0)
        {
            delsource = true;
            continue;
        }

        if(inann)       /* clipboard contains files from another announce dir */
        {
            if(strncmp(buf, "Path=", 5) == 0)
            {
                strncpy(spath, buf + 5, PATHLEN);
                if(dashd(spath))
                {
                    if(strcmp(spath, dpath) == 0)
                    {
                        fclose(fp);
                        unlink(clipfile);
                        RETURN_LONG(-1);
                    }
                }
                else
                {
                    fclose(fp);
                    unlink(clipfile);
                    RETURN_LONG(-2);
                }
                bzero(&sme, sizeof(sme));
                sme.path = spath;
                sme.level = PERM_BOARDS;
                a_loadnames(&sme, getSession());
                bzero(&dme, sizeof(dme));
                dme.path = dpath;
                dme.level = PERM_BOARDS;
                a_loadnames(&dme, getSession());
            }
            else if(spath[0] != '\0')
            {
                if(strncmp(buf, "Filename=", 9) == 0)
                {
                    strncpy(fname, buf + 9, STRLEN);
                    snprintf(sfpath, PATHLEN, "%s/%s", spath, fname);
                    if(!(dashf(sfpath)||dashd(sfpath)))
                        continue;
                    snprintf(dfpath, PATHLEN, "%s/%s", dpath, fname);
                    if(dashf(dfpath)||dashd(dfpath))
                        continue;
                    if(delsource)
                        f_mv(sfpath, dfpath);
                    else
                        f_cp(sfpath, dfpath, O_TRUNC);
                    find = false;
                    for(i=0; i<sme.num; i++)
                    {
                        if(strcmp(fname, M_ITEM(&sme,i)->fname) == 0)
                        {
                            find = true;
                            break;
                        }
                    }
                    if(find)
                    {
                        strncpy(title, M_ITEM(&sme,i)->title, STRLEN);
                        if(delsource)
                            a_delitem(&sme,i);
                    }
                    a_additem(&dme, title, fname, NULL, 0, 0);
                }
            }
        }
        else       /* clipboard contains files from a board */
        {
            if(strncmp(buf, "Board=", 6) == 0)
            {
                strncpy(bname, buf + 6, STRLEN);
                if((bp = getbcache(bname)) == NULL)
                {
                    fclose(fp);
                    unlink(clipfile);
                    RETURN_LONG(-3);
                }
                strcpy(bname, bp->filename);
                if(!is_BM(bp, getCurrentUser()))
                {
                    fclose(fp);
                    unlink(clipfile);
                    RETURN_LONG(-4);
                }
                bzero(&dme, sizeof(dme));
                dme.path = dpath;
                dme.level = PERM_BOARDS;
                a_loadnames(&dme, getSession());
                strcpy(title, "没有名字？");
            }
            else if(bname[0] != '\0')
            {
                if(strncmp(buf, "Title=", 6) == 0)
                {
                    snprintf(title, STRLEN, "%-38.38s %s ", buf + 6, getCurrentUser()->userid);
                }
                else if(strncmp(buf, "Filename=", 9) == 0)
                {
                    strncpy(fname, buf + 9, STRLEN);
                    setbfile(sfpath, bname, fname);
                    if(!dashf(sfpath))
                        continue;
                    strcpy(fh.filename, fname);
                    ann_get_postfilename(newfname, &fh, &dme);
                    snprintf(dfpath, PATHLEN, "%s/%s", dpath, newfname);
                    f_cp(sfpath, dfpath, O_TRUNC);
                    a_additem(&dme, title, newfname, NULL, 0, 0);
                }
            }
        }
    }
    fclose(fp);
    if(delsource && inann)
    {
        if(a_savenames(&sme) != 0)
            err = true;
    }
    if(a_savenames(&dme) != 0)
        err = true;
    if(inann)
        a_freenames(&sme);
    a_freenames(&dme);
    unlink(clipfile);
    if(err)
    {
        RETURN_LONG(-3);
    }
    else
    {
        ann_write_bmlog(dpath);
        RETURN_LONG(0);
    }
}

/* move item
bbs_ann_move(string path, long oldnum, long newnum);
return:
     0: success
    -1: path not found
    -2: item not found
    -3: a_savesnames failed
*/
PHP_FUNCTION(bbs_ann_move)
{
    int ac;
    char *path;
    int path_len, i;
    long oldnum, newnum;
    MENU me;
    ITEM *tmp;

    ac = ZEND_NUM_ARGS();
    if((ac != 3) || (zend_parse_parameters(3 TSRMLS_CC, "sll", &path, &path_len, &oldnum, &newnum) != SUCCESS))
    {
        WRONG_PARAM_COUNT;
    }

    if(oldnum == newnum)
    {
        RETURN_LONG(0);
    }
    if(!dashd(path))
    {
        RETURN_LONG(-1);
    }

    bzero(&me, sizeof(me));
    me.path = path;
    me.level = PERM_BOARDS;
    a_loadnames(&me, getSession());
    if((oldnum > me.num) || (oldnum <= 0))
    {
        RETURN_LONG(-2);
    }
    if(newnum <= 0)
        newnum = 1;
    if(newnum > me.num)
        newnum = me.num;
    oldnum--;
    newnum--;
    tmp = M_ITEM(&me,oldnum);
    if(oldnum > newnum)
    {
        for(i=oldnum; i>newnum; i--)
            M_ITEM(&me,i) = M_ITEM(&me,i-1);
    }
    else
    {
        for(i=oldnum; i<newnum; i++)
            M_ITEM(&me,i) = M_ITEM(&me,i+1);
    }
    M_ITEM(&me,newnum)=tmp;

    if(a_savenames(&me) != 0)
    {
        a_freenames(&me);
        RETURN_LONG(-3);
    }
    a_freenames(&me);
    ann_write_bmlog(path);
    RETURN_LONG(0);
}

/* BM functions of announce end here */


/* Import path functions below
 * by pig2532@newsmth
 */

/* read import path list
array bbs_ipath_list();
return:
    array of import path list when success
    false when failed
*/
PHP_FUNCTION(bbs_ipath_list)
{
    zval *item;
    char *ipath[ANNPATH_NUM], *ititle[ANNPATH_NUM];
    int ipath_select = 0, i;
    time_t ipath_time = 0;

    if(ZEND_NUM_ARGS() != 0)
    {
        WRONG_PARAM_COUNT;
    }
    if(!HAS_PERM(getCurrentUser(), PERM_BOARDS))
    {
        RETURN_FALSE;
    }
    load_import_path(ipath, ititle, &ipath_time, &ipath_select, getSession());
    array_init(return_value);
    for(i=0; i<ANNPATH_NUM; i++)
    {
        MAKE_STD_ZVAL(item);
        array_init(item);
        add_assoc_string(item, "PATH", ipath[i], 1);
        add_assoc_string(item, "TITLE", ititle[i], 1);
        zend_hash_index_update(Z_ARRVAL_P(return_value), i, (void*)&item, sizeof(zval*), NULL);
    }
    free_import_path(ipath, ititle, &ipath_time);
}

/* modify import path
bbs_ipath_modify(long num, string title, string path);
return:
     0: success
    -1: num not correct
    -2: no bm perm in that path
*/
PHP_FUNCTION(bbs_ipath_modify)
{
    long num;
    char *title, *path;
    int title_len, path_len;
    char *ipath[ANNPATH_NUM], *ititle[ANNPATH_NUM];
    int ipath_select = 0;
    time_t ipath_time = 0;

    path = NULL;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls|s", &num, &title, &title_len, &path, &path_len) != SUCCESS)
    {
        WRONG_PARAM_COUNT;
    }

    if(!HAS_PERM(getCurrentUser(), PERM_BOARDS))
    {
        RETURN_LONG(-2);
    }
    if((num < 1) || (num > ANNPATH_NUM))
    {
        RETURN_LONG(-1);
    }
    if(path)
        if(path[0] != '\0')
            if(ann_traverse_check(path, getCurrentUser()) != 1)
            {
                RETURN_LONG(-2);
            }
    num--;
    load_import_path(ipath, ititle, &ipath_time, &ipath_select, getSession());
    free(ititle[num]);
    ititle[num] = (char *)malloc(title_len + 1);
    strcpy(ititle[num], title);
    ititle[num][title_len] = '\0';
    if(path)
    {
        free(ipath[num]);
        ipath[num] = (char *)malloc(path_len + 1);
        strcpy(ipath[num], path);
        ipath[num][path_len] = '\0';
    }
    save_import_path(ipath, ititle, &ipath_time, getSession());
    free_import_path(ipath, ititle, &ipath_time);
    RETURN_LONG(0);
}

