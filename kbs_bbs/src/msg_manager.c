#include "bbs.h"
#include "mysql.h"

struct smsmsg * s_m;
int sm_num=0;
int sm_start=0;
char sm_dest[13];
int sm_type=-1;
char sm_msgtxt[30];

static int set_smsg_select(struct _select_def *conf)
{
	clear();
	move(0,0);
	prints("%sĞÅÈË:%s   Ê±¼ä:%s", s_m[conf->pos-1].type?"·¢":"ÊÕ", s_m[conf->pos-1].dest, s_m[conf->pos-1].time);
	prints("\nÄÚÈİ:\n%s", s_m[conf->pos-1].context);
	pressanykey();

	if( ! s_m[conf->pos-1].readed ){
		if( sign_smsmsg_read( s_m[conf->pos-1].id )){
			s_m[conf->pos-1].readed = 1;
		}
	}

	return SHOW_REFRESH;
}

static int set_smsg_show(struct _select_def *conf, int i)
{
	char title[41];
	char *c;

	if(strlen(s_m[i-1].context) > 40){
		strncpy(title, s_m[i-1].context, 37);
		title[37]='.';
		title[38]='.';
		title[39]='.';
	}else{
		strcpy(title, s_m[i-1].context);
	}
	title[40]=0;

	if((c = strchr(title, '\n') )!= NULL) *c=0;
	if((c = strchr(title, '\r') )!= NULL) *c=0;

	prints(" %s%-3d %-13s %-14s %s %-40s%s",s_m[i-1].readed?"":"[1m",sm_start + i, s_m[i-1].dest, s_m[i-1].time, s_m[i-1].type?"[1;32m·¢[m":"[1;33mÊÕ[m", title, s_m[i-1].readed?"":"[m");
	return SHOW_CONTINUE;
}

static int set_smsg_prekey(struct _select_def *conf, int *key)
{
	switch (*key) {
	case KEY_PGDN:
	case KEY_DOWN:
	{
		if( *key == KEY_DOWN ){
			if( conf->pos != BBS_PAGESIZE )
				break;
		}
		if( sm_num >= BBS_PAGESIZE ){
			sm_start += sm_num - 1;
			if( *key == KEY_DOWN )
				conf->pos = 1;
			return SHOW_DIRCHANGE;
		}
		else if( conf->pos != sm_num ){
			conf->new_pos = sm_num;
			return SHOW_SELCHANGE;
		}
		break;
	}
	case KEY_PGUP:
	case KEY_UP:
	{
		if( *key == KEY_UP ){
			if( conf->pos != 1)
				break;
		}
		if( sm_start > 0 ){
			sm_start -= BBS_PAGESIZE;
			if(sm_start < 0)
				sm_start = 0;
			if( *key == KEY_UP )
				conf->pos = BBS_PAGESIZE;
			return SHOW_DIRCHANGE;
		}else if( conf->pos != 1){
			conf->new_pos = 1;
			return SHOW_SELCHANGE;
		}
		break;
	}
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

static int set_smsg_refresh(struct _select_def *conf)
{
	clear();
	docmdtitle("[¶ÌĞÅ¹ÜÀí]","ÍË³ö[[1;32mq[m] É¾³ı[[1;32md[m] ³¬¼¶Ñ¡Ôñ[[1;32ms[m] ÏÔÊ¾ËùÓĞ¶ÌĞÅ[[1;32ma[m]");
	move(2,0);
	prints("[0;1;37;44m  %-4s %-13s %-14s %-2s %-40s[m","ĞòºÅ","¶ÔÏó","Ê±¼ä", "Àà", "ÄÚÈİ");
	update_endline();
	return SHOW_CONTINUE;
}

static int set_smsg_getdata(struct _select_def *conf,int pos,int len)
{
	int i;

	for(i=0;i<sm_num;i++){
		if( s_m[i].context ) free(s_m[i].context);
	}
	bzero( s_m, sizeof(struct smsmsg) * BBS_PAGESIZE );
	sm_num = get_sql_smsmsg(s_m, currentuser->userid, sm_dest, 0, 0, sm_type, 0, sm_start, BBS_PAGESIZE,sm_msgtxt);

	conf->item_count = sm_num;

	if( sm_num <= 0){

		sm_start = 0;
		sm_dest[0]=0;
		sm_type = -1;
		sm_msgtxt[0]=0;
		
		sm_num = get_sql_smsmsg(s_m, currentuser->userid, sm_dest, 0, 0, sm_type, 0, sm_start, BBS_PAGESIZE,sm_msgtxt);

		conf->item_count = sm_num;

		if(sm_num <= 0)
			return SHOW_QUIT;
	}

	return SHOW_CONTINUE;
}


static int set_smsg_key(struct _select_def *conf, int key)
{
	switch(key){
	case 'd':
	{
		char sql[100];
		MYSQL s;
		char ans[4];

        move(2,0);
		clrtoeol();
		ans[0]=0;
        getdata(2, 0, "É¾³ıÕâÌõÏûÏ¢(Y/N) [N]: ", ans, 3, DOECHO, NULL, true);
		if(ans[0] != 'y' && ans[0]!='Y')
			return SHOW_REFRESH;

		mysql_init(&s);
		if (! my_connect_mysql(&s) ){
			clear();
			prints("%s\n",mysql_error(&s));
			pressanykey();
			mysql_close(&s);
			return SHOW_REFRESH;
		}

		sprintf(sql,"DELETE FROM smsmsg WHERE id=%d;",s_m[conf->pos-1].id);

		if( mysql_real_query( &s, sql, strlen(sql) ) ){
			clear();
			prints("%s\n",mysql_error(&s));
			pressanykey();
			mysql_close(&s);
			return SHOW_REFRESH;
		}

		mysql_close(&s);

		return SHOW_DIRCHANGE;
	}
	case 's':
	{
		char ans[40];

		clear();
		move(0,0);
		ans[0]=0;
		prints("                                 ³¬¼¶¶ÌĞÅÑ¡Ôñ\n");
		prints("[1;31m------------------------------------------------------------------------[m\n");
        getdata(2, 0, "Ñ¡ÔñÈ«²¿¶ÌĞÅÇë°´[1;32m1[m,ÊäÈëÌõ¼şÑ¡ÔñÇë°´[1;32m2[m,È¡ÏûÖ±½Ó»Ø³µ(1/2/0) [0]: ", ans, 3, DOECHO, NULL, true);
		if( ans[0] == '1' ){
			sm_start = 0;
			sm_dest[0]=0;
			sm_type = -1;
			sm_msgtxt[0]=0;
			return SHOW_DIRCHANGE;
		}else if(ans[0] == '2'){
			move(3,0);
			getdata(3,0,"ÇëÊäÈëÒªÑ¡ÔñµÄ¶ÌĞÅÀ´Ô´(»Ø³µÑ¡ÔñËùÓĞ):",ans,15, DOECHO,NULL,true);
			strncpy(sm_dest, ans, 13);
			sm_dest[12]=0;

			move(4,0);
			getdata(4,0,"ÇëÊäÈëÒªÑ¡ÔñµÄ¶ÌĞÅÀà±ğ(1±íÊ¾·¢,2±íÊ¾ÊÕ,»Ø³µËùÓĞ) [0]:",ans,3, DOECHO,NULL,true);
			if( ans[0] == '1' )
				sm_type = 1;
			else if(ans[0] == '2')
				sm_type = 0;
			else sm_type = -1;

			move(5,0);
			getdata(5,0,"ÇëÊäÈë¿ªÊ¼ÏÔÊ¾µÄ¶ÌĞÅĞòºÅ [0]:",ans,5, DOECHO,NULL,true);
			sm_start = atoi(ans);

			move(6,0);
			getdata(6,0,"ÇëÊäÈëÒª¶ÌĞÅÄÚÈİ°üº¬ÎÄ×Ö(»Ø³µÑ¡ÔñËùÓĞ):",ans,21, DOECHO,NULL,true);
			strncpy(sm_msgtxt, ans, 21);
			sm_msgtxt[20]=0;

			return SHOW_DIRCHANGE;
		}else{
			return SHOW_REFRESH;
		}
		break;
	}
	case 'a':
	{
		sm_start = 0;
		sm_dest[0]=0;
		sm_type = -1;
		sm_msgtxt[0]=0;
		return SHOW_DIRCHANGE;
	}
	default:
		break;
	}
	return SHOW_CONTINUE;
}

int smsmsg_read()
{

	struct _select_def group_conf;
	int i;
	POINT *pts;
	
	sm_dest[0]=0;

	s_m = (struct smsmsg *) malloc( sizeof(struct smsmsg) * BBS_PAGESIZE );
	if( s_m == NULL)
		return -1;

	bzero(&group_conf,sizeof(struct _select_def));

    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
	group_conf.item_per_page = BBS_PAGESIZE;
    group_conf.flag = LF_VSCROLL | LF_BELL;
    group_conf.prompt = "¡ô";
    group_conf.item_pos = pts;
	group_conf.title_pos.x = 0;
	group_conf.title_pos.y = 0;
	group_conf.pos=1;
	group_conf.page_pos=1;

	group_conf.on_select = set_smsg_select;
	group_conf.show_data = set_smsg_show;
	group_conf.pre_key_command = set_smsg_prekey;
	group_conf.show_title = set_smsg_refresh;
	group_conf.get_data = set_smsg_getdata;
	group_conf.key_command = set_smsg_key;


	bzero( s_m, sizeof(struct smsmsg) * BBS_PAGESIZE );
	sm_num = get_sql_smsmsg(s_m, currentuser->userid, NULL, 0, 0, -1, 0, sm_start, BBS_PAGESIZE, NULL);
	
	if(sm_num <= 0) return -1;

	group_conf.item_count = sm_num;
		
	clear();
	list_select_loop(&group_conf);

	for(i=0;i<sm_num;i++){
		if( s_m[i].context ) free(s_m[i].context);
	}

	free(pts);
	free(s_m);
	s_m = NULL;
	sm_num = 0;

	chk_smsmsg(1);

	return 1;
}
