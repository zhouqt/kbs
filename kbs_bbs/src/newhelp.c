#include "bbs.h"

#ifdef NEW_HELP

#include "mysql.h"

struct helps * s_help=NULL;
int help_num=0;
char help_search[21];

int count_help( int mode ){

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int i;
	char sql[600];
	char tmp[200];

	mysql_init(&s);

	if( !my_connect_mysql(&s) ){
		clear();
		prints("%s\n", mysql_error(&s));
		pressanykey();
		return -1;
	}

	sprintf(sql,"SELECT COUNT(*) FROM help WHERE modeid=%d",mode);

	if( help_search[0] ){
		char newsearch[50];
		mysql_escape_string( newsearch, help_search, strlen(help_search) );
		snprintf(tmp, 199, " And ( content LIKE \"%%%s%%\" OR func LIKE \"%%%s%%\") ", newsearch, newsearch);
		strcat(sql, tmp);
	}

	if( mysql_real_query(&s, sql, strlen(sql)) ){
		clear();
		prints("%s\n", mysql_error(&s));
		pressanykey();
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	if( row != NULL ){
		i=atoi(row[0]);
	}
	mysql_free_result(res);
	mysql_close(&s);

	return i;
}

int get_help( struct helps * h, int mode, int start, int num ){

	MYSQL s;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int i;
	char sql[600];
	char tmp[200];

	mysql_init(&s);

	if( !my_connect_mysql(&s) ){
		clear();
		prints("%s\n", mysql_error(&s));
		pressanykey();
		return -1;
	}

	sprintf(sql,"SELECT * FROM help WHERE modeid=%d",mode);

	if( help_search[0] ){
		char newsearch[50];
		mysql_escape_string( newsearch, help_search, strlen(help_search) );
		snprintf(tmp, 199, " And ( content LIKE \"%%%s%%\" OR func LIKE \"%%%s%%\") ", newsearch, newsearch);
		strcat(sql, tmp);
	}

	snprintf(tmp, 99, " ORDER BY prekey LIMIT %d,%d", start, num);
	strcat(sql, tmp);

	if( mysql_real_query(&s, sql, strlen(sql)) ){
		clear();
		prints("%s\n", mysql_error(&s));
		pressanykey();
		mysql_close(&s);
		return -1;
	}
	res = mysql_store_result(&s);
	row = mysql_fetch_row(res);

	i=0;
	while( row != NULL ){
		i++;
		if( i>num)
			break;
		h[i-1].id = atoi(row[0]);
		h[i-1].modeid = mode;
		strncpy(h[i-1].index, row[2], 10);
		h[i-1].index[10]=0;
		strncpy(h[i-1].desc, row[3], 40);
		h[i-1].desc[40]=0;
		if( row[4] && strlen(row[4]) > 0){
			h[i-1].content=(char *)malloc(strlen(row[4])+1);
			if( h[i-1].content)
				strcpy(h[i-1].content, row[4] );
		}
		row=mysql_fetch_row(res);
	}
	mysql_free_result(res);
	mysql_close(&s);

	return i;
}

static int del_help(unsigned int id)
{
	MYSQL s;
	char sql[100];

	clear();

	getdata(2,0,"È·¶¨É¾³ý? (y/N) [N]:",sql,2,DOECHO,NULL,true);
	if( sql[0] != 'y' && sql[0] != 'Y' )
		return -1;

	mysql_init(&s);
	if (! my_connect_mysql(&s) ){
		prints("\n%s\n",mysql_error(&s));
		pressanykey();
		return -1;
	}

	sprintf(sql,"DELETE FROM help WHERE id=%d;",id);
	
	if( mysql_real_query( &s, sql, strlen(sql) )){
		prints("\n%s\n",mysql_error(&s));
		pressanykey();
		mysql_close(&s);
		return -1;
	}

	mysql_close(&s);

	return 0;
}

static int add_new_help(int mode, struct helps *pn, char *body, int num)
{
	MYSQL s;
	char newindex[21];
	char newdesc[81];
	char newbody[1001];
	char sql[1200];

	newindex[0]=0;
	newdesc[0]=0;
	newbody[0]=0;

	mysql_init(&s);
	if (! my_connect_mysql(&s) ){
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
		return -1;
	}

	mysql_escape_string(newindex, pn->index, strlen(pn->index));
	mysql_escape_string(newdesc, pn->desc, strlen(pn->desc));
	mysql_escape_string(newbody, body, strlen(body));

	if( num == -1 ){
		sprintf(sql,"INSERT INTO help VALUES (NULL, %d, '%s', '%s', '%s');",mode, newindex, newdesc, newbody);
	}else{
		sprintf(sql,"UPDATE help SET prekey='%s', func='%s', content='%s' WHERE id=%d;", newindex, newdesc, newbody, s_help[num].id);
	}
	
	if( mysql_real_query( &s, sql, strlen(sql) )){
		clear();
		prints("%s\n",mysql_error(&s));
		pressanykey();
		mysql_close(&s);
		return -1;
	}

	mysql_close(&s);

	return 0;
}

int add_help(int mode, int num)
{
	struct helps pn;
	char ans[501];

	bzero( &pn, sizeof(pn) );

	clear();
	prints("%s°ïÖú,Ä£Ê½: %s\n", num==-1?"Ôö¼Ó":"ÐÞ¸Ä",helpmodestr[mode-1]);

	pn.modeid = mode;

	if( num == -1 )
		ans[0] = 0;
	else
		strcpy(ans, s_help[num].index);
	getdata(2,0,"Ë÷Òý(10×Ö½Ú×î¶à):",ans,11,DOECHO,NULL,false);
	if(! ans[0])
		return -1;
	strncpy(pn.index, ans, 10);
	pn.index[10]=0;

	if( num == -1 )
		ans[0] = 0;
	else
		strcpy(ans, s_help[num].desc);
	getdata(3,0,"¼òµ¥ÃèÊö(40×Ö½Ú×î¶à):",ans,41,DOECHO,NULL,false);
	if(! ans[0])
		return -1;
	strncpy(pn.desc, ans, 40);
	pn.desc[40]=0;

	if(num != -1 && s_help[num].content!=NULL) {
		strncpy(ans, s_help[num].content, 500);
		ans[500]=0;
	}else
		ans[0]=0;

	move(4,0);
	prints("ÇëÊäÈë¾ßÌåÃèÊö,15ÐÐ500×ÖÄÚ:");
	multi_getdata(5, 0, 79, NULL, ans, 500, 15, false, 0);

	if( add_new_help(mode, &pn, ans, num) == 0) {
		prints("\nÔö¼Ó³É¹¦\n");
		pressanykey();
		return 0;
	}else
		return -1;
}

static int help_select(struct _select_def *conf)
{
	clear();
	move(0,0);
	prints("Ä£Ê½: [1;33m%s[m\n\n", (*((int *)conf->arg) > 0)?helpmodestr[ *((int *)conf->arg) - 1]:"²Ëµ¥");
	prints("Ë÷Òý: %s\n", s_help[conf->pos-conf->page_pos].index);
	prints("¼ò½é: %s\n", s_help[conf->pos-conf->page_pos].desc);
	if(s_help[conf->pos-conf->page_pos].content)
		prints("\x1b[1;31m¾ßÌå½éÉÜ:\x1b[m\n%s", s_help[conf->pos-conf->page_pos].content);
	pressanykey();

	return SHOW_REFRESH;
}

static int help_show(struct _select_def *conf, int i)
{
	char title[41];
	char *c;

	strcpy(title, s_help[i-conf->page_pos].desc);
	if( (c=strchr(title, '\n')) != NULL) *c=0;
	if( (c=strchr(title, '\r')) != NULL) *c=0;
	prints(" %-3d %-10s %-40s", i, s_help[i-conf->page_pos].index, title);

	return SHOW_CONTINUE;
}

static int help_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
    case 'q':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case ' ':
    case 'N':
        *key = KEY_PGDN;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int help_refresh(struct _select_def *conf)
{
	int mode = *((int *)conf->arg);
	char searchstr[100];
	char helptitle[100];

	clear();
	move(0,0);
	if( mode == 0)
		strcpy(helptitle, "[È«²¿Ä£Ê½°ïÖú]");
	else snprintf(helptitle, 100, "[°ïÖú - %s]", helpmodestr[mode-1]);

	docmdtitle(helptitle, " [mÍË³ö[[1;32m¡û[m,[1;32mq[m] ÏêÏ¸[[1;32m¡ú[m,[1;32mRtn[m] ËÑË÷[[1;32ms[m] ÏÔÊ¾ËùÓÐ[[1;32mf[m]\n");

	if(help_search[0])
		sprintf(searchstr, "ËÑË÷:%s",help_search);
	else
		searchstr[0]=0;
	move(2,0);
	prints("[0;1;44m  %-4s %-10s %-15s%s%-45s[m","ÐòºÅ","Ë÷Òý","½éÉÜ", help_search[0]?"[33m":"",searchstr);
	return SHOW_CONTINUE;
}

static int help_getdata(struct _select_def *conf, int pos, int len)
{
	int i;
	int mode = *((int *)conf->arg);

	for(i=0;i<help_num;i++){
		if( s_help[i].content) free(s_help[i].content);
	}
	bzero( s_help, sizeof(struct helps)*BBS_PAGESIZE);

//	if( conf->item_count - conf->page_pos < BBS_PAGESIZE)
		conf->item_count = count_help(mode);

	i=get_help(s_help, mode , conf->page_pos -1, BBS_PAGESIZE);

	if( i<=0){
		conf->pos = 1;
		conf->page_pos = 1;
		help_search[0]=0;

		conf->item_count = count_help(mode);
		i=get_help(s_help, mode, 0, BBS_PAGESIZE);

		if( i<= 0)
			return SHOW_QUIT;
	}

	help_num = i;
	return SHOW_CONTINUE;
}

static int help_key(struct _select_def *conf, int key)
{

	switch(key){
	case 's':
	{
		clear();
		move(5,0);
		prints("[1;32mÌáÊ¾:[m ÔÚ°ïÖúÖ÷½çÃæÖÐ°´f¿ÉÒÔÈ¡ÏûËÑË÷ÏÔÊ¾µ±Ç°ËùÓÐµÄ°ïÖú");
		getdata(6,0, "ÇëÊäÈëËÑË÷ÄÚÈÝ:", help_search, 21, DOECHO, NULL, false);
		if( help_search[0] ){
			conf->pos = 1;
			conf->page_pos = 1;
			return SHOW_DIRCHANGE;
		}
		return SHOW_REFRESH;
	}
	case 'a':
		if( HAS_PERM(currentuser, PERM_SYSOP) ){
			if( add_help(*((int *)conf->arg), -1) < 0 )
				return SHOW_REFRESH;
			return SHOW_DIRCHANGE;
		}
	case 'e':
		if( HAS_PERM(currentuser, PERM_SYSOP) ){
			if( add_help(*((int *)conf->arg), conf->pos - conf->page_pos) < 0 )
				return SHOW_REFRESH;
			return SHOW_DIRCHANGE;
		}
	case 'd':
		if( HAS_PERM(currentuser, PERM_SYSOP) ){
			if( del_help(s_help[conf->pos-conf->page_pos].id) < 0 )
				return SHOW_REFRESH;
			return SHOW_DIRCHANGE;
		}
	case 'f':
		if( help_search[0] ){
			help_search[0]=0;
			return SHOW_DIRCHANGE;
		}
		return SHOW_CONTINUE;
	default:
		break;
	}

	return SHOW_CONTINUE;
}

int newhelp_loop(int mode){

	struct _select_def group_conf;
	int i;
	POINT *pts;

	if( mode < 0 || mode > NUMHELPMODE )
		return -1;

	help_search[0]=0;
	bzero(&group_conf,sizeof(struct _select_def));

	s_help = (struct helps *)malloc( sizeof(struct helps) * BBS_PAGESIZE);
	if( s_help == NULL )
		return -1;
	bzero( s_help, sizeof(struct helps)*BBS_PAGESIZE);

    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    group_conf.item_per_page = BBS_PAGESIZE;
	group_conf.arg = & mode;
    group_conf.flag = LF_VSCROLL | LF_BELL | LF_MULTIPAGE | LF_LOOP;
    group_conf.prompt = "¡ô";
    group_conf.item_pos = pts;
    group_conf.title_pos.x = 0;
    group_conf.title_pos.y = 0;
    group_conf.pos=1;
    group_conf.page_pos=1;

    group_conf.on_select = help_select;
    group_conf.show_data = help_show;
    group_conf.pre_key_command = help_prekey;
    group_conf.show_title = help_refresh;
    group_conf.get_data = help_getdata;
    group_conf.key_command = help_key;

    group_conf.item_count = count_help(mode);

	if( group_conf.item_count<=0 ){
		free(pts);
		free(s_help);
		if( HAS_PERM(currentuser, PERM_SYSOP) ){
			add_help(mode, -1);
		}else{
			clear();
			prints("ÏÖÔÚÃ»ÓÐ¸Ã×´Ì¬ÏÂ°ïÖú£¬»òÕßÏµÍ³³ö´í\n");
			pressanykey();
		}
		return -1;
	}

    i = get_help(s_help, mode, 0, BBS_PAGESIZE );

	if( i<=0 ){
		free(pts);
		free(s_help);
		clear();
		prints("ÏÖÔÚÃ»ÓÐ¸Ã×´Ì¬ÏÂ°ïÖú£¬»òÕßÏµÍ³³ö´í\n");
		pressanykey();
		return -1;
	}

	help_num = i;
	clear();
	list_select_loop(&group_conf);

	for( i=0; i< help_num; i++){
		if (s_help[i].content) free(s_help[i].content);
	}
	free(pts);
	free(s_help);
	s_help=NULL;
	help_num=0;
	
	return 1;

}

int newhelp(int mode){

	char save_scr[LINEHEIGHT][LINELEN*3];
	int save_y, save_x;
	int i;

    getyx(&save_y, &save_x);
    for(i=0;i<t_lines;i++)
        saveline(i, 0, save_scr[i]);

	newhelp_loop(mode);

    for(i=0;i<t_lines;i++)
        saveline(i, 1, save_scr[i]);
    move(save_y, save_x);

	return 1;
}
#endif
