#include "php_kbs_bbs.h"  

PHP_FUNCTION(bbs_get_explain)
{
	int ac = ZEND_NUM_ARGS();
	zval *element,*retarray;
	int i;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "a", &retarray) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	for(i=0; secname[i][0] && groups[i]; i++){

		MAKE_STD_ZVAL(element);
		array_init(element);

	    add_assoc_string(element, "EXPLAIN", (char*)secname[i][0], 1);
	    add_assoc_string(element, "GROUPS", (char*)groups[i], 1);
		zend_hash_index_update(Z_ARRVAL_P(retarray), i,
				(void*) &element, sizeof(zval*), NULL);

	}

	RETURN_LONG(i);
}


/*
 * bbs_sysconf_str
 获取系统参数
*/
PHP_FUNCTION(bbs_sysconf_str)
{
         int ac = ZEND_NUM_ARGS();
        int char_len;
        char *char_conf;
        const char *char_result;
		const char *char_default="";
		int default_len;
        if(ac !=1 || zend_parse_parameters(1 TSRMLS_CC,"s",&char_conf,&char_len) ==FAILURE){
            if(ac !=2 || zend_parse_parameters(2 TSRMLS_CC,"ss",&char_conf,&char_len,&char_default,&default_len) ==FAILURE){
                WRONG_PARAM_COUNT;
			}
        }
        char_result=sysconf_str(char_conf);//获取配制参数
		if (char_result==NULL) char_result=char_default;
        RETURN_STRING((char*)char_result,1);
}



static int bbs_bm_change(char *board, struct boardheader *newbh, struct boardheader *oldbh)
{
	int id,m,brd_num,n,i;
	int flag;
	struct userec *lookupuser;
	unsigned int newlevel;
	char *p;
	char obm[10][IDLEN+2];
	int obmnum=0,nbmnum=0;
	char nbm[10][IDLEN+2];
	char buf[200];

	for(i=0;i<10;i++){
		obm[i][0]='\0';
		nbm[i][0]='\0';
	}

	if(oldbh){
		for(p = strtok(oldbh->BM, " "),obmnum=0; p && obmnum < 10; p=strtok(NULL," "),obmnum++){
			strncpy(obm[obmnum], p, IDLEN+2);
			obm[obmnum][IDLEN+1]='\0';
		}
	}

	for(p = strtok(newbh->BM, " "),nbmnum=0; p && nbmnum < 10; p=strtok(NULL," "),nbmnum++){
		strncpy(nbm[nbmnum], p, IDLEN+2);
		nbm[nbmnum][IDLEN+1]='\0';
	}

	newbh->BM[0]='\0';

	for( i=0; i<obmnum; i++ ){
		flag = 2;

		if(!(id = getuser(obm[i],&lookupuser))){
			continue;
		}

		for(m=0;m<nbmnum;m++){
			if(!strcmp(obm[i],nbm[m]))
				flag = 0;
		}
		if(flag == 0) continue;

		newlevel = lookupuser->userlevel;

		brd_num = 0;

		if( lookupuser->userlevel & PERM_BOARDS ){
			for(n = 0; n < get_boardcount(); n++){
				if(chk_BM_instr(getboard(n+1)->BM, lookupuser->userid) == true )
					brd_num ++;
			}
		}

		if( brd_num == 1){
			newlevel &= ~PERM_BOARDS;
			newlevel &= ~PERM_CLOAK;
		}

		sprintf(buf,"免去 %s 的斑竹 %s", board, lookupuser->userid);
		//securityreport(buf, lookupuser, NULL);
		lookupuser->userlevel = newlevel;
	}

	for( i=0; i<nbmnum; i++ ){
		flag = 1;

		if(!(id = getuser(nbm[i],&lookupuser))){
			continue;
		}

		if( strlen(newbh->BM) + strlen(lookupuser->userid) >= BM_LEN - 2 )
			continue;

		for(m=0;m<obmnum;m++){
			if(!strcmp(nbm[i],obm[m])){
				flag = 0;
				if( newbh->BM[0] != '\0' )
					strcat(newbh->BM, " ");
				strcat(newbh->BM, lookupuser->userid);
			}
		}
		if(flag == 0) continue;

		newlevel = lookupuser->userlevel;

		if( newbh->BM[0] != '\0' )
			strcat( newbh->BM, " " );
		strcat(newbh->BM, lookupuser->userid);

		newlevel |= PERM_BOARDS;
		mail_file(getCurrentUser()->userid, "etc/forbm", lookupuser->userid, "新任斑竹必读", BBSPOST_LINK, NULL);

		sprintf(buf,"任命 %s 的斑竹 %s", board, lookupuser->userid);
		//securityreport(buf, lookupuser, NULL);
		lookupuser->userlevel = newlevel;
	}
    return 0;
}


/*
 * new a board
 * 修改版面不允许重新修改精华区路径
 * - atppp 20050801 modify: 修改精华区路径通过编译了，不过我没测试！
 */
PHP_FUNCTION(bbs_new_board)
{
	int ac = ZEND_NUM_ARGS();

	char *bname;
	int bname_len;
	char *btitle;
	int btitle_len;
	char *bbm;
	int bbm_len;
	char *section;
	int section_len;
	char *desp;
	int desp_len;
	long blevel;
	long banony;
	long bjunk;
	long bout;
	long battach;
	long oldbnum;
	long bclubread;
	long bclubwrite;
	long bclubhide;

	char* bgroup;
	int bgroup_len;
	
	int i;
	struct boardheader newboard;
	char vbuf[100];

    if (ac != 15 || zend_parse_parameters(15 TSRMLS_CC, "lsssssllllsllll", &oldbnum, &bname, &bname_len, &section, &section_len, 
        &desp, &desp_len, &btitle, &btitle_len, &bbm, &bbm_len, &blevel, &banony, &bjunk, &bout, &bgroup, &bgroup_len, &battach,
        &bclubread, &bclubwrite, &bclubhide) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	bzero(&newboard,sizeof(newboard));
	if(bname_len >= 18)
		RETURN_LONG(-1);

	if(! HAS_PERM(getCurrentUser(), PERM_SYSOP) )
		RETURN_LONG(-7);

	strncpy(newboard.filename,bname,18);
	newboard.filename[17]='\0';

	strncpy(newboard.title+13,btitle,47);
	newboard.title[59]='\0';

	if(section[0]=='\0' || ! isprint(section[0]))
		RETURN_LONG(-10);
	newboard.title[0]=section[0];

	if(desp[0]=='\0')
		RETURN_LONG(-11);

	if(desp[0]=='['){
		strncpy(newboard.title+1,desp,11);
		for(i=desp_len+1; i<13; i++)
			newboard.title[i]=' ';
	}else{
		newboard.title[1]='[';
		strncpy(newboard.title+2,desp,10);
		if(desp_len < 10){
			newboard.title[2+desp_len]=']';
			for(i=2+desp_len+1; i<13; i++)
				newboard.title[i]=' ';
		}
		else
			newboard.title[12]=']';
	}

	if( ! valid_brdname(newboard.filename) )
		RETURN_LONG(-2);

	strncpy(newboard.BM, bbm, BM_LEN - 1);
	newboard.BM[BM_LEN-1] = '\0';

	newboard.level = blevel;

	if( banony )
		newboard.flag |= BOARD_ANNONY;

	if( bjunk )
		newboard.flag |= BOARD_JUNK;

	if( bout )
		newboard.flag |= BOARD_OUTFLAG;

	if( battach )
		newboard.flag |= BOARD_ATTACH;

	if(oldbnum < 0)
		RETURN_LONG(-14);

	if(oldbnum > 0)
		if( getboard(oldbnum) == 0)
			RETURN_LONG(-21);

	if( (oldbnum==0 && getbid(newboard.filename,NULL) > 0))
		RETURN_LONG(-3);
	if( oldbnum && getbid(newboard.filename,NULL)!=oldbnum && getbid(newboard.filename,NULL)>0 )
		RETURN_LONG(-23);

	if( oldbnum ){	//更改版面属性
		struct boardheader oldboard;

		memcpy(&oldboard, getboard(oldbnum), sizeof(oldboard) );
		if(oldboard.filename[0]=='\0')
			RETURN_LONG(-22);

		if(strcmp(oldboard.filename,newboard.filename)){
			char old[256],tar[256];
			
			setbpath(old, oldboard.filename);
			setbpath(tar, newboard.filename);
			f_mv(old,tar);
			sprintf(old, "vote/%s", oldboard.filename);
			sprintf(tar, "vote/%s", newboard.filename);
			f_mv(old,tar);
		}

		bbs_bm_change(newboard.filename, &newboard, &oldboard);

		strncpy(newboard.ann_path, oldboard.ann_path, 128);
		newboard.ann_path[127]='\0';
		edit_group(&oldboard, &newboard);
		//精华区移动
		
		if(oldboard.flag | BOARD_ANNONY)
			del_from_file("etc/anonymous",oldboard.filename);
		if(newboard.flag | BOARD_ANNONY)
			addtofile("etc/anonymous",newboard.filename);

		if( bclubread )
			newboard.flag |= BOARD_CLUB_READ;
		
		if( bclubwrite )
			newboard.flag |= BOARD_CLUB_WRITE;
		
		if( bclubhide )
			newboard.flag |= BOARD_CLUB_HIDE;
		
		set_board(oldbnum, &newboard, &oldboard);
		sprintf(vbuf, "更改讨论区 %s 的资料 --> %s", oldboard.filename, newboard.filename);
		bbslog("user","%s",vbuf);

	}else{	//增加版面

		if(newboard.flag | BOARD_ANNONY)
			addtofile("etc/anonymous",newboard.filename);

		sprintf(vbuf,"vote/%s",newboard.filename);
		if( mkdir(vbuf,0755) == -1 )
			RETURN_LONG( -4);

		sprintf(vbuf,"boards/%s",newboard.filename);
		if( mkdir(vbuf,0755) == -1 )
			RETURN_LONG( -5);

		bbs_bm_change(newboard.filename, &newboard, NULL);

		if(bgroup && bgroup_len > 0){
			snprintf(newboard.ann_path,127,"%s/%s",bgroup, newboard.filename);
			newboard.ann_path[127]=0;
			edit_group(NULL, &newboard);
		}

		if( add_board( &newboard ) == -1 )
			RETURN_LONG( -6);
	}

	RETURN_LONG(1);
}


/*
 * valid_filename()
 * @author stiger
 */
PHP_FUNCTION(bbs_valid_filename)
{
    int ac = ZEND_NUM_ARGS();
	char * filename;
	int name_len;

    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &filename, &name_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	RETURN_LONG(VALID_FILENAME(filename));
}


/**
 * get the user dir or file.
 * prototype:
 * string bbs_sethomefile(string userid[,string filename])
 *
 * @author KCN
 */
PHP_FUNCTION(bbs_sethomefile)
{
    char *userid, *file;
    int userid_len, file_len = 0;
    char buf[60];
    int ac = ZEND_NUM_ARGS();

    if (ac == 2) {
        if (zend_parse_parameters(2 TSRMLS_CC, "ss", &userid, &userid_len, &file, &file_len) != SUCCESS)
            WRONG_PARAM_COUNT;
    } else if (ac == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) != SUCCESS)
            WRONG_PARAM_COUNT;
    } else
        WRONG_PARAM_COUNT;
    if (file_len != 0)
        sethomefile(buf, userid, file);
    else
        sethomepath(buf, userid);
    RETURN_STRING(buf, 1);
}

/**
 * get the user mail dir or file.
 * prototype:
 * string bbs_setmailfile(string userid[,string filename])
 *
 * @return path string
 * @author binxun
 */
PHP_FUNCTION(bbs_setmailfile)
{
    char *userid, *file;
    int userid_len, file_len = 0;
    char buf[60];
    int ac = ZEND_NUM_ARGS();

    if (ac == 2) {
        if (zend_parse_parameters(2 TSRMLS_CC, "ss", &userid, &userid_len, &file, &file_len) != SUCCESS)
            WRONG_PARAM_COUNT;
    } else if (ac == 1) {
        if (zend_parse_parameters(1 TSRMLS_CC, "s", &userid, &userid_len) != SUCCESS)
            WRONG_PARAM_COUNT;
    } else
        WRONG_PARAM_COUNT;
    if (file_len != 0)
        setmailfile(buf, userid, file);
    else
        setmailpath(buf, userid);
    RETURN_STRING(buf, 1);
}


PHP_FUNCTION(bbs_checkbadword)
{
    char *str;
    int  str_len;
    
    int ac = ZEND_NUM_ARGS();
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
#ifdef FILTER
	if (check_badword_str(str, strlen(str),getSession()))
		RETURN_TRUE;
#endif    
    RETURN_FALSE;
}





PHP_FUNCTION(bbs_error_get_desc)
{
    long no;
    int ac = ZEND_NUM_ARGS();
    char *desc;
    if (ac != 1 || zend_parse_parameters(1 TSRMLS_CC, "l", &no) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
    switch(no) {
        ALL_PHPBBS_ERRORS
        default:
            desc = "未知错误";
            break;            
    }
    RETURN_STRING(desc, 1);
}
