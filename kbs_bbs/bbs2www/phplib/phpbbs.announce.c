#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_smth_bbs.h"  

#include "bbs.h"
#include "bbslib.h"



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
    struct boardheader *bp;

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
 * int bbs_read_ann_dir(string path,string board,string path2,array articles)
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
    ITEM *its;
    int i,j;
    char *id,*ptr;
    char buf[256];
    char r_title[STRLEN],r_path[256],r_bm[256];
    int  r_flag,r_time;
    struct stat st;
    
    int ac = ZEND_NUM_ARGS();
    if(ac != 4 || zend_parse_parameters(4 TSRMLS_CC,"szza",&path,&path_len,&board,&path2,&articles) ==FAILURE) {
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
    
    if ((its = ann_alloc_items(MAXITEMS)) == NULL)
        RETURN_LONG(-9);
    
    ZVAL_STRING(path2,pathbuf,1);
    ann_set_items(&me, its, MAXITEMS);
    me.path = pathbuf;
    if (ann_load_directory(&me, getSession()) == 0) {
        buf[0] = '\0';
        ann_get_board(pathbuf, buf, sizeof(buf));
        ZVAL_STRING(board,buf,1);
        if (me.num <= 0) 
            RETURN_LONG(-3);
        me.now = 0;
        j = 0;
        for (i = 0; i < me.num; i++) {
            strncpy(r_title, me.item[i]->title, sizeof(r_title) - 1);
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
            snprintf(buf, sizeof(buf), "%s/%s", me.path,me.item[i]->fname);
            ptr = strchr(me.path, '/');
            
            if (stat(buf, &st) == -1) {
                r_time = 0;
                r_flag = 0;
            } else {
                r_time = st.st_mtime;
                if (S_ISDIR(st.st_mode))
                    r_flag = 1;
                else 
                    r_flag = me.item[i]->attachpos?3:2;
            }
            
            snprintf(r_path, sizeof(r_path), "%s/%s", ptr == NULL ? "" : ptr, me.item[i]->fname);
            strncpy(r_bm,id[0]?id:"",sizeof(r_bm)-1);
            r_bm[sizeof(r_bm)-1] = '\0';
            
            if (strcmp(r_bm,"BMS") && strcmp(r_bm,"SYSOPS")) { // only display common articles
                MAKE_STD_ZVAL(element);
                array_init(element);
                add_assoc_string(element,"TITLE",r_title,1);
                add_assoc_string(element,"PATH",r_path,1);
                add_assoc_string(element,"BM",r_bm,1);
                add_assoc_long(element,"FLAG",r_flag);
                add_assoc_long(element,"TIME",r_time);
                zend_hash_index_update(Z_ARRVAL_P(articles),j,(void*) &element, sizeof(zval*), NULL);
                j ++;
            }
            me.now++;
        }
        ann_free_items(its, MAXITEMS);
        RETURN_LONG(0);
    }
    else
        ann_free_items(its, MAXITEMS);
    
    RETURN_LONG(-2);
}
