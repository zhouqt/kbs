#include "php_kbs_bbs.h"  
#include "vote.h"

static int bbs_can_access_vote(char *limitpath)
{
	struct votelimit userlimit;

	userlimit.numlogins=0;
	userlimit.numposts=0;
	userlimit.stay=0;
	userlimit.day=0;

	get_record(limitpath,&userlimit,sizeof(userlimit),1);

    if ((getCurrentUser()->numposts < userlimit.numposts
         || getCurrentUser()->numlogins < userlimit.numlogins
         || getCurrentUser()->stay < userlimit.stay * 60 * 60
         || (time(NULL) - getCurrentUser()->firstlogin) <
         userlimit.day * 24 * 60 * 60)) {

		return 0;
	}

	return 1;
}

static void bbs_make_vote_array(zval * array, struct votebal *vbal)
{
    add_assoc_string(array, "USERID", vbal->userid, 1);
    add_assoc_string(array, "TITLE", vbal->title, 1);
    add_assoc_long(array, "DATE", vbal->opendate);
    if(vbal->type <= 5 && vbal->type >= 1) add_assoc_string(array, "TYPE", vote_type[vbal->type-1], 1);
    add_assoc_long(array, "MAXDAY", vbal->maxdays);
}

PHP_FUNCTION(bbs_get_votes)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	struct votebal vbal;
	char controlfile[STRLEN];
	const struct boardheader *bp=NULL;
	FILE *fp;
	int vnum,i;
	zval *element,*retarray;

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sa", &bname, &bname_len, &retarray) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(-2);

	if( ! HAS_PERM(getCurrentUser(), PERM_LOGINOK) )
		RETURN_LONG(-1);

	sprintf(controlfile,"vote/%s/control",bname);

	if(array_init(retarray) != SUCCESS)
	{
                RETURN_LONG(-5);
	}

	vnum = get_num_records(controlfile,sizeof(struct votebal));

	if(vnum <= 0)
		RETURN_LONG(-4);

	if((fp=fopen(controlfile,"r"))==NULL)
		RETURN_LONG(-3);

	for(i=0; i < vnum; i++){
		if(fread(&vbal, sizeof(vbal), 1, fp) < 1)
			break;
		MAKE_STD_ZVAL(element);
		array_init(element);

		bbs_make_vote_array(element, &vbal);
		zend_hash_index_update(Z_ARRVAL_P(retarray), i,
				(void*) &element, sizeof(zval*), NULL);
	}

	fclose(fp);

	RETURN_LONG(i);
}

static void bbs_make_detail_vote_array(zval * array, struct votebal *vbal)
{
	int i;
	char tmp[10];

    add_assoc_string(array, "USERID", vbal->userid, 1);
    add_assoc_string(array, "TITLE", vbal->title, 1);
    add_assoc_long(array, "DATE", vbal->opendate);
    if(vbal->type <= 5 && vbal->type >= 1) add_assoc_string(array, "TYPE", vote_type[vbal->type-1], 1);
    add_assoc_long(array, "MAXDAY", vbal->maxdays);
    add_assoc_long(array, "MAXTKT", vbal->maxtkt);
    add_assoc_long(array, "TOTALITEMS", vbal->totalitems);

	for(i=0; i < vbal->totalitems; i++){
		sprintf(tmp,"ITEM%d",i+1);
		add_assoc_string(array, tmp, vbal->items[i], 1);
	}

}

static void bbs_make_user_vote_array(zval * array, struct ballot *vbal)
{
	if(vbal && vbal->uid[0]){
	    add_assoc_string(array, "USERID", vbal->uid, 1);
    	add_assoc_long(array, "VOTED", vbal->voted);
	    add_assoc_string(array, "MSG1", vbal->msg[0], 1);
	    add_assoc_string(array, "MSG2", vbal->msg[1], 1);
	    add_assoc_string(array, "MSG3", vbal->msg[2], 1);
	}else{
	    add_assoc_string(array, "USERID", "", 1);
	}
}

static int cmpvuid(char *userid, struct ballot *uv)
{
	return !strncmp(userid, uv->uid,IDLEN);
}

PHP_FUNCTION(bbs_get_vote_from_num)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	struct votebal vbal;
	struct ballot uservote;
	char controlfile[STRLEN];
	const struct boardheader *bp=NULL;
	FILE *fp;
	int vnum;
	zval *element,*retarray,*uservotearray;
	long ent;
	int pos;

    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "sala", &bname, &bname_len, &retarray, &ent, &uservotearray) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(-2);

	if( ! HAS_PERM(getCurrentUser(), PERM_LOGINOK) )
		RETURN_LONG(-1);

	sprintf(controlfile,"vote/%s/control",bname);

	if(array_init(retarray) != SUCCESS)
	{
                RETURN_LONG(-5);
	}

	if(array_init(uservotearray) != SUCCESS)
	{
                RETURN_LONG(-9);
	}

	vnum = get_num_records(controlfile,sizeof(struct votebal));

	if(vnum <= 0)
		RETURN_LONG(-4);

	if(ent <= 0 || ent > vnum)
		RETURN_LONG(-6);

	if((fp=fopen(controlfile,"r"))==NULL)
		RETURN_LONG(-3);

	fseek(fp,sizeof(vbal) * (ent-1), SEEK_SET);

	if(fread(&vbal, sizeof(vbal), 1, fp) < 1){
		fclose(fp);
		RETURN_LONG(-7);
	}
	fclose(fp);

	sprintf(controlfile,"vote/%s/limit.%lu",bname,vbal.opendate);
	if(! bbs_can_access_vote(controlfile))
		RETURN_LONG(-8);

	MAKE_STD_ZVAL(element);
	array_init(element);

	bbs_make_detail_vote_array(element, &vbal);
	zend_hash_index_update(Z_ARRVAL_P(retarray), 0,
				(void*) &element, sizeof(zval*), NULL);

	MAKE_STD_ZVAL(element);
	array_init(element);

	sprintf(controlfile,"vote/%s/flag.%lu",bname,vbal.opendate);
	if((pos = search_record(controlfile, &uservote, sizeof(uservote),
							(RECORD_FUNC_ARG) cmpvuid, getCurrentUser()->userid))<=0){
		bbs_make_user_vote_array(element, NULL);
	}
	else{
		bbs_make_user_vote_array(element, &uservote);
	}

	zend_hash_index_update(Z_ARRVAL_P(uservotearray), 0,
				(void*) &element, sizeof(zval*), NULL);

	RETURN_LONG(ent);
}

PHP_FUNCTION(bbs_vote_num)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	char *msg;
	int msg_len;
	long ent;
	long votevalue;
	struct votebal vbal;
	struct ballot uservote;
	struct ballot tmpball;
	char controlfile[STRLEN];
	const struct boardheader *bp=NULL;
	FILE *fp;
	int vnum,pos;
	char lmsg[3][STRLEN];
	char *c,*cc;
	int llen;

    if (ac != 4 || zend_parse_parameters(4 TSRMLS_CC, "slls", &bname, &bname_len, &ent, &votevalue, &msg, &msg_len) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	llen = msg_len+1;
	lmsg[0][0]='\0';
	lmsg[1][0]='\0';
	lmsg[2][0]='\0';
	strncpy(lmsg[0],msg,(llen > STRLEN)?STRLEN:llen);
	lmsg[0][STRLEN-1]='\0';
	if((c=strchr(msg,'\n'))!=NULL){
		c++;
		llen -= (c-msg);
		strncpy(lmsg[1],c,(llen > STRLEN)?STRLEN:llen);
		lmsg[1][STRLEN-1]='\0';
		if((cc=strchr(c,'\n'))!=NULL){
			cc++;
			llen -= (cc-c);
			strncpy(lmsg[2],cc,(llen > STRLEN)?STRLEN:llen);
			lmsg[2][STRLEN-1]='\0';
		}
	}
	for(llen=0;llen<3;llen++){
		if((c=strchr(lmsg[llen],'\n'))!=NULL)
			*c = '\0';
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(-2);

	if( ! HAS_PERM(getCurrentUser(), PERM_LOGINOK) )
		RETURN_LONG(-1);

	sprintf(controlfile,"vote/%s/control",bname);

	vnum = get_num_records(controlfile,sizeof(struct votebal));

	if(vnum <= 0)
		RETURN_LONG(-4);

	if(ent <= 0 || ent > vnum)
		RETURN_LONG(-6);

	if((fp=fopen(controlfile,"r"))==NULL)
		RETURN_LONG(-3);

	fseek(fp,sizeof(vbal) * (ent-1), SEEK_SET);

	if(fread(&vbal, sizeof(vbal), 1, fp) < 1){
		fclose(fp);
		RETURN_LONG(-7);
	}
	fclose(fp);

	sprintf(controlfile,"vote/%s/limit.%lu",bname,vbal.opendate);
	if(! bbs_can_access_vote(controlfile))
		RETURN_LONG(-8);

	if(vbal.type == 4 && votevalue > vbal.maxtkt)
		RETURN_LONG(-12);

	bzero( &uservote, sizeof(uservote) );
	strcpy(uservote.uid,getCurrentUser()->userid);
	uservote.voted = votevalue;
	strncpy(uservote.msg[0],lmsg[0],STRLEN);
	strncpy(uservote.msg[1],lmsg[1],STRLEN);
	strncpy(uservote.msg[2],lmsg[2],STRLEN);

	sprintf(controlfile,"vote/%s/flag.%lu",bname,vbal.opendate);
	if((pos = search_record(controlfile, &tmpball, sizeof(tmpball),
							(RECORD_FUNC_ARG) cmpvuid, getCurrentUser()->userid))>0){
		substitute_record(controlfile, &uservote, sizeof(uservote), pos); 
	}
	else{
		if(append_record(controlfile, &uservote, sizeof(uservote)) == -1)
			RETURN_LONG(-11);
	}

	RETURN_LONG(ent);
}

PHP_FUNCTION(bbs_start_vote)
{

	int ac = ZEND_NUM_ARGS();
	//zval *items;
	char *items[32];
	int items_len[32];
	char *board;
	int board_len;
	char *title;
	int title_len;
	char *desp;
	int desp_len;
	long type,ball_maxdays;
	long ball_maxtkt;
	long ball_totalitems;
	int i,pos;
	long numlogin,numpost,numstay,numday;
	const struct boardheader *bp;
	struct boardheader fh;
	struct votebal ball;
	struct votelimit vlimit;
	char buf[PATHLEN];
	char buff[PATHLEN];
	FILE *fp;

    if (ac != 21 || zend_parse_parameters(21 TSRMLS_CC, "slllllsslllssssssssss", &board, &board_len, &type, &numlogin, &numpost,
        &numstay, &numday, &title, &title_len, &desp, &desp_len, &ball_maxdays, &ball_maxtkt, &ball_totalitems, &items[0],
        &items_len[0], &items[1], &items_len[1], &items[2], &items_len[2], &items[3], &items_len[3], &items[4], &items_len[4],
        &items[5], &items_len[5], &items[6], &items_len[6], &items[7], &items_len[7], &items[8], &items_len[8], &items[9],
        &items_len[9]) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

    if ((bp = getbcache(board)) == NULL) {
        RETURN_LONG(-1);
    }
    if(! is_BM(bp, getCurrentUser()) && !HAS_PERM(getCurrentUser(),PERM_SYSOP) )
        RETURN_LONG(-2);

	if(type < 1 || type > 5)
		RETURN_LONG(-3);

	if(ball_totalitems < 1 || ball_totalitems > 10)
		RETURN_LONG(-4);

	if(ball_maxdays <= 0)
		ball_maxdays = 1;

	if(strlen(title) <=0 )
		RETURN_LONG(-8);

	bzero(&ball,sizeof(ball));
	strncpy(ball.title,title,STRLEN);
	ball.title[STRLEN-1]='\0';

	ball.opendate = time(0);
	ball.type = type;
	ball.maxdays = 1;

	if(type == 1){
		ball.maxtkt = 1;
		ball.totalitems = 3;
		strcpy(ball.items[0], "赞成  （是的）");
		strcpy(ball.items[1], "不赞成（不是）");
		strcpy(ball.items[2], "没意见（不清楚）");
	}else if(type == 2 || type == 3){
		if(type == 2) ball.maxtkt = 1;
		else ball.maxtkt = ball_maxtkt;

		ball.totalitems = ball_totalitems;
		for(i=0; i<ball.totalitems; i++){
			/*
			strncpy(ball.items[i], items[i], STRLEN);
			ball.items[i][STRLEN-1]='\0';*/
			/*overflow.      modified by binxun . */
			strncpy(ball.items[i], items[i], 38);
			ball.items[i][37]='\0';
		}
	}else if(type == 4){
		ball.maxtkt = ball_maxtkt;
		if(ball.maxtkt <= 0)
			ball.maxtkt = 100;
	}else if(type == 5){
		ball.maxtkt = 0;
		ball.totalitems = 0;
	}

	//setvoteflag
    pos = getboardnum(bp->filename, &fh);
    if (pos) {
        fh.flag = fh.flag | BOARD_VOTEFLAG;
        set_board(pos, &fh,NULL);
    }

	strcpy(ball.userid, getCurrentUser()->userid);

	sprintf(buf, "vote/%s/control", bp->filename);
	if(append_record(buf,&ball,sizeof(ball)) == -1)
		RETURN_LONG(-7);

	sprintf(buf,"%s OPEN VOTE",bp->filename);
	bbslog("user","%s",buf);

	sprintf(buf, "vote/%s/desc.%lu",bp->filename, ball.opendate );
	if((fp=fopen(buf,"w"))!=NULL){
		fputs(desp,fp);
		fclose(fp);
	}

	if(numlogin < 0) numlogin = 0;
	vlimit.numlogins = numlogin;

	if(numpost < 0) numpost = 0;
	vlimit.numposts = numpost;

	if(numstay < 0) numstay = 0;
	vlimit.stay = numstay;

	if(numday < 0) numday = 0;
	vlimit.day = numday;

	sprintf(buf,"vote/%s/limit.%lu",bp->filename, ball.opendate);
	append_record(buf, &vlimit, sizeof(vlimit));

	sprintf(buf,"tmp/votetmp.%d",getpid());
	if((fp=fopen(buf,"w"))==NULL){
		sprintf(buff,"[通知] %s 举办投票: %s",bp->filename,ball.title);
		fprintf(fp,"%s",buff);
		fclose(fp);
		if( !normal_board(bp->filename) ){
			post_file(getCurrentUser(), "", buf, bp->filename, buff, 0,1, getSession());
		}else{
			post_file(getCurrentUser(), "", buf, "vote", buff, 0,1, getSession());
		}
		unlink(buf);
	}

	RETURN_LONG(1);
}






/****
 * add by stiger, template, 摸板
 */
static void bbs_make_tmpl_array(zval * array, struct a_template * ptemp, char *board)
{
    add_assoc_string(array, "TITLE", ptemp->tmpl->title, 1);
    add_assoc_string(array, "TITLE_TMPL", ptemp->tmpl->title_tmpl, 1);
    add_assoc_long(array, "CONT_NUM", ptemp->tmpl->content_num);
	if( ptemp->tmpl->filename[0] ){
		char path[STRLEN];
		setbfile( path, board, ptemp->tmpl->filename );
    	add_assoc_string(array, "FILENAME", path,1);
	}else
 		add_assoc_string(array, "FILENAME", ptemp->tmpl->filename,1);
}

PHP_FUNCTION(bbs_get_tmpls)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	char path[STRLEN];
	struct a_template * ptemp = NULL;
	int mode,tmpl_num,i;
	const struct boardheader *bp=NULL;
	zval *element,*retarray;

    if (ac != 2 || zend_parse_parameters(2 TSRMLS_CC, "sa", &bname, &bname_len, &retarray) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(-2);

	setbfile(path, bname, TEMPLATE_DIR);

	if(array_init(retarray) != SUCCESS)
	{
                RETURN_LONG(-5);
	}

    if(is_BM(bp, getCurrentUser()))
		mode = 1;
	else
		mode = 0;

	tmpl_num = orig_tmpl_init(bname, mode, & ptemp);
	
	if(tmpl_num < 0)
		RETURN_LONG(-6);

	if(tmpl_num == 0)
		RETURN_LONG(0);

	for(i=0; i < tmpl_num; i++){
		MAKE_STD_ZVAL(element);
		array_init(element);

		bbs_make_tmpl_array(element, ptemp + i,bname);
		zend_hash_index_update(Z_ARRVAL_P(retarray), i,
				(void*) &element, sizeof(zval*), NULL);
	}

	orig_tmpl_free( & ptemp, tmpl_num );

	RETURN_LONG(i);
}

/**********
 * get a detail tmpl, stiger
 */

static void bbs_make_detail_tmpl_array(zval * array, struct s_content * cont)
{

    add_assoc_string(array, "TEXT", cont->text, 1);
    add_assoc_long(array, "LENGTH", cont->length);

}

PHP_FUNCTION(bbs_get_tmpl_from_num)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	char path[STRLEN];
	const struct boardheader *bp=NULL;
	struct a_template * ptemp = NULL;
	zval *element,*retarray;
	long ent;
    int tmpl_num,i,mode;

    if (ac != 3 || zend_parse_parameters(3 TSRMLS_CC, "sla", &bname, &bname_len, &ent, &retarray) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(-2);

	setbfile(path, bname, TEMPLATE_DIR);

	if(array_init(retarray) != SUCCESS)
	{
                RETURN_LONG(-5);
	}

    if(is_BM(bp, getCurrentUser()))
		mode = 1;
	else
		mode = 0;

	tmpl_num = orig_tmpl_init(bname, mode, & ptemp);
	
	if(tmpl_num < 0)
		RETURN_LONG(-6);

	if(tmpl_num == 0)
		RETURN_LONG(0);

	if(ent <= 0 || ent > tmpl_num){
		orig_tmpl_free( & ptemp, tmpl_num );
		RETURN_LONG(-8);
	}

	MAKE_STD_ZVAL(element);
	array_init(element);

	bbs_make_tmpl_array(element, ptemp+ent-1, bname);
	zend_hash_index_update(Z_ARRVAL_P(retarray), 0,
				(void*) &element, sizeof(zval*), NULL);

	for(i=0; i < ptemp[ent-1].tmpl->content_num; i++){

		MAKE_STD_ZVAL(element);
		array_init(element);

		bbs_make_detail_tmpl_array(element, ptemp[ent-1].cont+i);
		zend_hash_index_update(Z_ARRVAL_P(retarray), i+1,
				(void*) &element, sizeof(zval*), NULL);
	}

	orig_tmpl_free( & ptemp, tmpl_num );

	RETURN_LONG(ent);
}

PHP_FUNCTION(bbs_make_tmpl_file)
{
	int ac = ZEND_NUM_ARGS();
	char *bname;
	int bname_len;
	char tmpfname[STRLEN];
	char path[STRLEN];
	const struct boardheader *bp=NULL;
	struct a_template * ptemp = NULL;
	FILE *fp,*fpsrc;
	int write_ok=0;
	long ent;
    int tmpl_num,i,mode;
	char newtitle[STRLEN];

	char *text[21];
	int t_len[21];

    if (ac != 23 || zend_parse_parameters(23 TSRMLS_CC, "slsssssssssssssssssssss", &bname, &bname_len, &ent, &text[0],&t_len[0],
        &text[1],&t_len[1],&text[2],&t_len[2],&text[3],&t_len[3],&text[4],&t_len[4],&text[5],&t_len[5],&text[6],&t_len[6],&text[7],
        &t_len[7],&text[8],&t_len[8],&text[9],&t_len[9],&text[10],&t_len[10],&text[11],&t_len[11],&text[12],&t_len[12],&text[13],
        &t_len[13],&text[14],&t_len[14],&text[15],&t_len[15],&text[16],&t_len[16],&text[17],&t_len[17],&text[18],&t_len[18],&text[19],
        &t_len[19],&text[20],&t_len[20]) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if((bp=getbcache(bname))==NULL)
		RETURN_LONG(0);

	if( getCurrentUser() == NULL )
		RETURN_LONG(0);

	setbfile(path, bname, TEMPLATE_DIR);

    if(is_BM(bp, getCurrentUser()))
		mode = 1;
	else
		mode = 0;

	tmpl_num = orig_tmpl_init(bname, mode, & ptemp);
	
	if(tmpl_num < 0)
		RETURN_LONG(0);

	if(tmpl_num == 0)
		RETURN_LONG(0);

	if(ent <= 0 || ent > tmpl_num){
		orig_tmpl_free( & ptemp, tmpl_num );
		RETURN_LONG(0);
	}

	sprintf(tmpfname, "tmp/%s.tmpl.tmp", getCurrentUser()->userid);
	if((fp = fopen(tmpfname, "w"))==NULL){
		RETURN_LONG(0);
	}

	if( ptemp[ent-1].tmpl->filename[0] ){
		setbfile( path,bname, ptemp[ent-1].tmpl->filename);
		if( dashf( path )){
			if((fpsrc = fopen(path,"r"))!=NULL){
				char buf[256];

				while(fgets(buf,255,fpsrc)){
					int l;
					int linex = 0;
					char *pn,*pe;

					for(pn = buf; *pn!='\0'; pn++){
						if( *pn != '[' || *(pn+1)!='$' ){
							fputc(*pn, fp);
							linex++;
						}else{
							pe = strchr(pn,']');
							if(pe == NULL){
								fputc(*pn, fp);
								continue;
							}
							l = atoi(pn+2);
							if( l<=0 || l > ptemp[ent-1].tmpl->content_num || l > 20){
								fputc('[', fp);
								continue;
							}
							fprintf(fp,"%s",text[l]);
							pn = pe;
							continue;
						}
					}
				}
				fclose(fpsrc);

				write_ok = 1;
			}
		}
	}
	if(write_ok == 0){
		for(i=0; i< ptemp[ent-1].tmpl->content_num && i<20; i++)
			fprintf(fp,"\033[1;32m%s:\033[m\n%s\n\n",ptemp[ent-1].cont[i].text, text[i+1]);
	}
	fclose(fp);

	if( ptemp[ent-1].tmpl->title_tmpl[0] ){
		char *pn,*pe;
		char *buf;
		int l;
		int newl = 0;

		newtitle[0]='\0';
		buf = ptemp[ent-1].tmpl->title_tmpl;

		for(pn = buf; *pn!='\0' && newl < STRLEN-1; pn++){
			if( *pn != '[' || *(pn+1)!='$' ){
				if( newl < STRLEN - 1 ){
					newtitle[newl] = *pn ;
					newtitle[newl+1]='\0';
					newl ++;
				}
			}else{
				pe = strchr(pn,']');
				if(pe == NULL){
					if( newl < STRLEN - 1 ){
						newtitle[newl] = *pn ;
						newtitle[newl+1]='\0';
						newl ++;
					}
					continue;
				}
				l = atoi(pn+2);
				if( l<0 || l > ptemp[ent-1].tmpl->content_num || l > 20){
					if( newl < STRLEN - 1 ){
						newtitle[newl] = *pn ;
						newtitle[newl+1]='\0';
						newl ++;
					}
					continue;
				}
				if( l == 0 ){
					int ti;
					for( ti=0; text[0][ti]!='\0' && ti < t_len[0] && newl < ARTICLE_TITLE_LEN - 1; ti++, newl++ ){
						newtitle[newl] = text[0][ti] ;
						newtitle[newl+1]='\0';
					}
				}else{
					int ti;
					for( ti=0; text[l][ti]!='\0' && ti < t_len[l] && text[l][ti]!='\n' && text[l][ti]!='\r' && newl < STRLEN - 1; ti++, newl++ ){
						newtitle[newl] = text[l][ti] ;
						newtitle[newl+1]='\0';
					}
				}
				pn = pe;
				continue;
			}
		}
	}else{
		strncpy(newtitle, text[0], ARTICLE_TITLE_LEN );
		newtitle[ARTICLE_TITLE_LEN -1]='\0';
	}

	orig_tmpl_free( & ptemp, tmpl_num );

	//RETURN_LONG(1);
	RETURN_STRING(newtitle,1);
}


