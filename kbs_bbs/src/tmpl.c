#include "bbs.h"
#include "read.h"

/***************************************************************************
 * add by stiger, 新文章模板
 ***************************************************************************/

struct a_template * ptemplate = NULL ;
int template_num = 0;
int t_now = 0;


int tmpl_init(int mode){

	int newmode=0;
	int ret;

	if(mode==1 || chk_currBM(currBM, getCurrentUser())) newmode = 1;

	ret = orig_tmpl_init(currboard->filename, newmode, & ptemplate);

	if(ret >= 0) template_num = ret;

    if (ret == -2) {
		clear();
		move(3,0);
		prints("模板程序已经更新过，请您重新登录，谢谢");
		pressreturn();
    }
	return ret;

}

void tmpl_free(){
	orig_tmpl_free( & ptemplate, template_num );
	template_num = 0;
}

int tmpl_save(){

	return orig_tmpl_save(ptemplate, template_num, currboard->filename);

}

int tmpl_add(){

	char buf[60];
	struct s_template tmpl;

	if( template_num >= MAX_TEMPLATE )
		return -1;

	bzero(&tmpl, sizeof(struct s_template));

	clear();
	buf[0]='\0';
	getdata(t_lines - 1, 0, "模板标题: ", buf, 50, DOECHO, NULL, false);
	if( buf[0]=='\0' || buf[0]=='\n' ){
		return -1;
	}
	strncpy(tmpl.title, buf, 50);
	tmpl.title[49] = '\0';

	ptemplate[template_num].tmpl = (struct s_template *) malloc( sizeof(struct s_template) );
	memcpy( ptemplate[template_num].tmpl, &tmpl, sizeof(struct s_template) );
	template_num ++;

	tmpl_save();

	return 0;
}

int content_add(){

	struct s_content ct;
	char buf[60];

	if( ptemplate[t_now].tmpl->content_num >= MAX_CONTENT )
		return -1;

	bzero(&ct, sizeof(struct s_content));
	clear();
	buf[0]='\0';
	getdata(t_lines - 1, 0, "选项标题: ", buf, 50, DOECHO, NULL, false);
	if( buf[0]=='\0' || buf[0]=='\n' ){
		return -1;
	}
	strncpy(ct.text, buf, 50);
	ct.text[49]='\0';

	buf[0]='\0';
	getdata(t_lines - 1, 0, "选项长度: ", buf, 5, DOECHO, NULL, false);
	ct.length = atoi(buf);
	if(ct.length <= 0 || ct.length > MAX_CONTENT_LENGTH )
		return -1;

	ptemplate[t_now].cont = (struct s_content *) realloc( ptemplate[t_now].cont, sizeof(struct s_content) * (ptemplate[t_now].tmpl->content_num+1));
	memcpy( &( ptemplate[t_now].cont[ptemplate[t_now].tmpl->content_num] ), &ct, sizeof(struct s_content) );
	ptemplate[t_now].tmpl->content_num ++;

	tmpl_save();

	return 0;

}


static int tmpl_show(struct _select_def *conf, int i)
{
	prints(" %2d %s%-60s %3d", i, ptemplate[i-1].tmpl->flag & TMPL_BM_FLAG ? "\033[1;31mB\033[m":" ", ptemplate[i-1].tmpl->title, ptemplate[i-1].tmpl->content_num);
	return SHOW_CONTINUE;
}

static int content_show(struct _select_def *conf, int i)
{
	prints(" %2d     %-50s  %3d", i,ptemplate[t_now].cont[i-1].text, ptemplate[t_now].cont[i-1].length);
	return SHOW_CONTINUE;
}

static int tmpl_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
    case 'q':
	case 'e':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case 'N':
        *key = KEY_PGDN;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    case ' ':
		*key = '\n';
		break;
    }
    return SHOW_CONTINUE;
}

static int tmpl_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[版面模板设置]",
               "添加[\x1b[1;32ma\x1b[0;37m] 删除[\x1b[1;32md\x1b[0;37m]\x1b[m 改名[\033[1;32mt\033[0;37m] \033[1;33m查看\033[m 标题[\033[1;32mx\033[m] 正文[\033[1;32ms\033[m] \033[1;33m修改\033[m 标题[\033[1;32mi\033[0;37m] 正文[\033[1;32mf\033[0;37m]");
    move(2, 0);
    prints("\033[0;1;37;44m %4s %-60s %8s", "序号", "模板名称","问题个数");
    clrtoeol();
    update_endline();
    return SHOW_CONTINUE;
}

static int content_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[版面模板问题设置]",
               "添加[\x1b[1;32ma\x1b[0;37m] 删除[\x1b[1;32md\x1b[0;37m]\x1b[m 修改问题名称[\033[1;32mt\033[0;37m] 修改回答长度[\033[1;32ml\033[0;37m]");
    move(2, 0);
    prints("\033[0;1;37;44m %4s     %-50s  %8s", "序号", "问题名称","回答长度");
    clrtoeol();
    update_endline();
    return SHOW_CONTINUE;
}

static int tmpl_getdata(struct _select_def *conf, int pos, int len)
{
    conf->item_count = template_num;
    return SHOW_CONTINUE;
}

static int content_getdata(struct _select_def *conf, int pos, int len)
{
    conf->item_count = ptemplate[t_now].tmpl->content_num;
    return SHOW_CONTINUE;
}

static int content_key(struct _select_def *conf, int key)
{
	int oldmode;
	switch (key) {
	case 'm':
		{
			char ans[5];
			int newm;
			getdata(t_lines-1, 0, "移动到新次序:", ans, 4, DOECHO, NULL, true);
			if( ans[0]=='\0' || ans[0]=='\n' || ans[0]=='\r' )
				return SHOW_REFRESH;
			newm=atoi(ans);

			if(newm <= 0 || newm > ptemplate[t_now].tmpl->content_num)
				return SHOW_REFRESH;

			if( newm > conf->pos ){
				int i;
				struct s_content sc;
				memcpy(&sc, &ptemplate[t_now].cont[conf->pos-1], sizeof(struct s_content));
				for(i=conf->pos;i<newm;i++)
					memcpy(& ptemplate[t_now].cont[i-1], & ptemplate[t_now].cont[i], sizeof(struct s_content));
				memcpy(&ptemplate[t_now].cont[newm-1], &sc, sizeof(struct s_content));

				tmpl_save();

				return SHOW_REFRESH;
			}else if(newm < conf->pos){
				int i;
				struct s_content sc;
				memcpy(&sc, &ptemplate[t_now].cont[conf->pos-1], sizeof(struct s_content));
				for(i=conf->pos;i>newm;i--)
					memcpy(& ptemplate[t_now].cont[i-1], & ptemplate[t_now].cont[i-2], sizeof(struct s_content));
				memcpy(&ptemplate[t_now].cont[newm-1], &sc, sizeof(struct s_content));

				tmpl_save();

				return SHOW_REFRESH;
			}else
				return SHOW_REFRESH;

		}
	case 'a':
		if( ptemplate[t_now].tmpl->content_num >= MAX_CONTENT ){
			char ans[STRLEN];
			move(t_lines - 1, 0);
			clrtoeol();
			a_prompt(-1, "选项已满，按回车继续 << ", ans);
			move(t_lines - 1, 0);
			clrtoeol();
			return SHOW_CONTINUE;
		}
		content_add();
		return SHOW_DIRCHANGE;
		break;
	case 'd':
		{
            char ans[3];

            getdata(t_lines - 1, 0, "确实要删除吗(Y/N)? [N]: ", ans, sizeof(ans), DOECHO, NULL, true);
            if (ans[0] == 'Y' || ans[0] == 'y') {
                int i=0;
				struct s_content *ct;

				if( ptemplate[t_now].tmpl->content_num == 1){
					ptemplate[t_now].tmpl->content_num = 0;
					free(ptemplate[t_now].cont);

					tmpl_save();
					return SHOW_QUIT;
				}

				ct = (struct s_content *) malloc( sizeof(struct s_content) * (ptemplate[t_now].tmpl->content_num-1));

				memcpy(ct+i,&(ptemplate[t_now].cont[i]),sizeof(struct s_content) * (conf->pos-1));
                for(i=conf->pos-1;i<ptemplate[t_now].tmpl->content_num-1;i++)
                    memcpy(ct+i, &(ptemplate[t_now].cont[i+1]), sizeof(struct s_content));

				free(ptemplate[t_now].cont);
				ptemplate[t_now].cont = ct;

				ptemplate[t_now].tmpl->content_num --;

				tmpl_save();
				return SHOW_DIRCHANGE;
			}
			return SHOW_REFRESH;
		}
	case 't' :
		{
			char newtitle[60];

			strcpy(newtitle, ptemplate[t_now].cont[conf->pos-1].text);
            getdata(t_lines - 1, 0, "新标题: ", newtitle, 50, DOECHO, NULL, false);

			if( newtitle[0] == '\0' || newtitle[0]=='\n' || ! strcmp(newtitle,ptemplate[t_now].cont[conf->pos-1].text) )
				return SHOW_REFRESH;

			strncpy(ptemplate[t_now].cont[conf->pos-1].text, newtitle, 50);
			ptemplate[t_now].cont[conf->pos-1].text[49]='\0';

			tmpl_save();

			return SHOW_REFRESH;
		}
		break;
	case 'l' :
		{
			char newlen[10];
			int nl;

			sprintf(newlen,"%-3d",ptemplate[t_now].cont[conf->pos-1].length);
			getdata(t_lines-1, 0, "新长度: ", newlen, 5, DOECHO, NULL, false);

			nl = atoi(newlen);

			if( nl <= 0 || nl > MAX_CONTENT_LENGTH || nl == ptemplate[t_now].cont[conf->pos-1].length )
				return SHOW_REFRESH;

			ptemplate[t_now].cont[conf->pos-1].length = nl;

			tmpl_save();

			return SHOW_REFRESH;
		}
		break;
	case 'w':                  /* by pig2532 on 2005-12-1 */
		oldmode = uinfo.mode;
       	 if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
       	     break;
        	s_msg();
		modify_user_mode(oldmode);
       	return SHOW_REFRESH;
    case 'u':                  /* by pig2532 on 2005-12-1 */
        clear();
        oldmode = uinfo.mode;
        modify_user_mode(QUERY);
        t_query(NULL);
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
	case 'U':		/* pig2532 2005.12.10 */
		board_query();
        return SHOW_REFRESH;
	default:
		break;
	}

	return SHOW_CONTINUE;
}

static int tmpl_key(struct _select_def *conf, int key)
{
	int oldmode;
	switch (key) {
	case 'a' :
		if( template_num >= MAX_TEMPLATE ){
			char ans[STRLEN];
			move(t_lines - 1, 0);
			clrtoeol();
			a_prompt(-1, "模板已满，按回车继续 << ", ans);
			move(t_lines - 1, 0);
			clrtoeol();
			return SHOW_CONTINUE;
		}
		tmpl_add();
		return SHOW_DIRCHANGE;
		break;
	case 'd' :
		{
            char ans[3];

            getdata(t_lines - 1, 0, "确实要删除吗(Y/N)? [N]: ", ans, sizeof(ans), DOECHO, NULL, true);
            if (ans[0] == 'Y' || ans[0] == 'y') {
                int i;
				char filepath[STRLEN];

				if( ptemplate[conf->pos-1].tmpl->filename[0] ){
					setbfile(filepath,currboard->filename, ptemplate[conf->pos-1].tmpl->filename);
					if(dashf(filepath))
						my_unlink(filepath);
				}

				if( ptemplate[conf->pos-1].tmpl != NULL)
					free(ptemplate[conf->pos-1].tmpl);
				if( ptemplate[conf->pos-1].cont != NULL)
					free(ptemplate[conf->pos-1].cont);

                template_num--;
                for(i=conf->pos-1;i<template_num;i++)
                    memcpy(ptemplate+i, ptemplate+i+1, sizeof(struct a_template));
				ptemplate[template_num].tmpl = NULL;
				ptemplate[template_num].cont = NULL;

				tmpl_save();
            }
			if(template_num > 0)
            	return SHOW_DIRCHANGE;
			else
				return SHOW_QUIT;
        }
        break;
    /* etnlegend, 2006.05.19, move templates... */
    case 'm':
        do{
            struct a_template temp;
            char ans[4];
            int i,pos;
            getdata(t_lines-1,0,"请输入希望移动到的位置序号: ",ans,4,DOECHO,NULL,true);
            trimstr(ans);
            if(!isdigit(ans[0]))
                break;
            pos=atoi(ans);
            pos=((pos<1)?1:((pos>template_num)?template_num:pos));
            if(pos==conf->pos)
                break;
            memcpy(&temp,&ptemplate[conf->pos-1],sizeof(struct a_template));
            if(pos>conf->pos){
                for(i=(conf->pos-1);i<(pos-1);i++)
                    memcpy(&ptemplate[i],&ptemplate[i+1],sizeof(struct a_template));
            }
            else{
                for(i=(conf->pos-1);i>(pos-1);i--)
                    memcpy(&ptemplate[i],&ptemplate[i-1],sizeof(struct a_template));
            }
            memcpy(&ptemplate[pos-1],&temp,sizeof(struct a_template));
            tmpl_save();
        }
        while(0);
        return SHOW_DIRCHANGE;
    /* END - etnlegend, 2006.05.19, move templates ... */
	case 't' :
		{
			char newtitle[60];

			strcpy(newtitle, ptemplate[conf->pos-1].tmpl->title);
            getdata(t_lines - 1, 0, "新名称: ", newtitle, 50, DOECHO, NULL, false);

			if( newtitle[0] == '\0' || newtitle[0]=='\n' || ! strcmp(newtitle,ptemplate[conf->pos-1].tmpl->title) )
				return SHOW_REFRESH;

			strncpy(ptemplate[conf->pos-1].tmpl->title, newtitle, 50);
			ptemplate[conf->pos-1].tmpl->title[49]='\0';

			tmpl_save();

			return SHOW_REFRESH;
		}
		break;/*
	case 'z' :
		{
			char newtitle[30];

			strcpy(newtitle, ptemplate[conf->pos-1].tmpl->title_prefix);
            getdata(t_lines - 1, 0, "请输入此模板的文章标题前缀: ", newtitle, 20, DOECHO, NULL, false);

			if( newtitle[0] == '\0' || newtitle[0]=='\n' || ! strcmp(newtitle,ptemplate[conf->pos-1].tmpl->title_prefix) )
				return SHOW_REFRESH;

			strncpy(ptemplate[conf->pos-1].tmpl->title_prefix, newtitle, 20);
			ptemplate[conf->pos-1].tmpl->title_prefix[19]='\0';

			tmpl_save();

			return SHOW_REFRESH;
		}*/
	case 'f' :
		{
			char filepath[STRLEN];
			int oldmode;

        oldmode = uinfo.mode;
        modify_user_mode(EDITUFILE);

			if( ptemplate[conf->pos-1].tmpl->filename[0] == '\0' ){

	    		setbfile(filepath, currboard->filename, "");
   	 			if ( GET_POSTFILENAME(ptemplate[conf->pos-1].tmpl->filename, filepath) != 0) {
					clear();
					move(3,0);
					prints("创建模板文件失败!");
					pressanykey();
					return SHOW_REFRESH;
				}
				tmpl_save();
			}

			setbfile(filepath, currboard->filename, ptemplate[conf->pos-1].tmpl->filename);

			vedit(filepath,0,NULL,NULL,0);
        modify_user_mode(oldmode);

			return SHOW_REFRESH;
		}
	case 's' :
		{
			char filepath[STRLEN];
			setbfile(filepath, currboard->filename, ptemplate[conf->pos-1].tmpl->filename);
			clear();
			ansimore(filepath,1);
			return SHOW_REFRESH;
		}
	case 'b' :
		{
			if( ptemplate[conf->pos-1].tmpl->flag & TMPL_BM_FLAG )
				ptemplate[conf->pos-1].tmpl->flag &= ~TMPL_BM_FLAG ;
			else
				ptemplate[conf->pos-1].tmpl->flag |= TMPL_BM_FLAG;

			tmpl_save();

			return SHOW_REFRESH;
		}
	case 'i' :
		{
			char newtitle[STRLEN];

			strcpy(newtitle, ptemplate[conf->pos-1].tmpl->title_tmpl);
            getdata(t_lines - 1, 0, "新文章标题: ", newtitle, STRLEN, DOECHO, NULL, false);

			if( newtitle[0] == '\0' || newtitle[0]=='\n' || ! strcmp(newtitle,ptemplate[conf->pos-1].tmpl->title_tmpl) )
				return SHOW_REFRESH;

			strncpy(ptemplate[conf->pos-1].tmpl->title_tmpl, newtitle, STRLEN);
			ptemplate[conf->pos-1].tmpl->title_tmpl[STRLEN-1]='\0';

			tmpl_save();

			return SHOW_REFRESH;
		}
	case 'x' :
		{
			clear();
			move(2,0);
			prints("此模版的标题设置为");
			move(4,0);
			prints("%s",ptemplate[conf->pos-1].tmpl->title_tmpl);
			pressanykey();
			return SHOW_REFRESH;
		}
		/*
	case 'h':
		{
			clear();
			move(1,0);
			prints("  x  :  查看标题格式\n");
			prints("  i  :  修改标题格式");
			pressanykey();
			return SHOW_REFRESH;
		}
		*/
	case 'l':	/* by pig2532 on 2005.12.01 */
        oldmode = uinfo.mode;
        show_allmsgs();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
	case 'w':                  /* by pig2532 on 2005-12-1 */
		oldmode = uinfo.mode;
       	 if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
       	     break;
        	s_msg();
		modify_user_mode(oldmode);
       	return SHOW_REFRESH;
	case 'u':                  /* by pig2532 on 2005-12-1 */
        clear();
        oldmode = uinfo.mode;
        modify_user_mode(QUERY);
        t_query(NULL);
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
	case 'U':		/* pig2532 2005.12.10 */
		board_query();
        return SHOW_REFRESH;
	default :
		break;
	}

	return SHOW_CONTINUE;
}

static int tmpl_select(struct _select_def *conf)
{

	int i;
	POINT *pts;
    struct _select_def grouplist_conf;

	clear();
	t_now = conf->pos - 1;
	
	if( ptemplate[t_now].tmpl->content_num == 0 ){
		char ans[3];
		clear();
        getdata(t_lines - 1, 0, "本模板现在没有内容，需要现在增加吗(Y/N)? [N]: ", ans, sizeof(ans), DOECHO, NULL, true);
        if (ans[0] != 'Y' && ans[0] != 'y'){
			return SHOW_REFRESH;
		}
		if( content_add() < 0 ){
			return SHOW_REFRESH;
		}
	}

    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    bzero(&grouplist_conf, sizeof(struct _select_def));

    grouplist_conf.item_count = ptemplate[t_now].tmpl->content_num;
    grouplist_conf.item_per_page = BBS_PAGESIZE;
    /*
     * 加上 LF_VSCROLL 才能用 LEFT 键退出 
     */
    grouplist_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    grouplist_conf.prompt = "◆";
    grouplist_conf.item_pos = pts;
    grouplist_conf.title_pos.x = 0;
    grouplist_conf.title_pos.y = 0;
    grouplist_conf.pos = 1;     /* initialize cursor on the first mailgroup */
    grouplist_conf.page_pos = 1;        /* initialize page to the first one */

    grouplist_conf.show_data = content_show;
    grouplist_conf.pre_key_command = tmpl_prekey;
    grouplist_conf.key_command = content_key;
    grouplist_conf.show_title = content_refresh;
    grouplist_conf.get_data = content_getdata;

    list_select_loop(&grouplist_conf);

    free(pts);

	t_now = 0;

	return SHOW_REFRESH;
}

int m_template()
{
	int i;
	POINT *pts;
    struct _select_def grouplist_conf;

	if (!chk_currBM(currBM, getCurrentUser())) {
		return DONOTHING;
	}

	if( tmpl_init(1) < 0 )
		return FULLUPDATE;

	if( template_num == 0 ){
		char ans[3];
		clear();
        getdata(t_lines - 1, 0, "本版现在没有模板，需要现在增加吗(Y/N)? [N]: ", ans, sizeof(ans), DOECHO, NULL, true);
        if (ans[0] != 'Y' && ans[0] != 'y'){
			tmpl_free();
			return FULLUPDATE;
		}
		if( tmpl_add() < 0 ){
			tmpl_free();
			return FULLUPDATE;
		}
	}

    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    bzero(&grouplist_conf, sizeof(struct _select_def));

    grouplist_conf.item_count = template_num;
    grouplist_conf.item_per_page = BBS_PAGESIZE;
    /*
     * 加上 LF_VSCROLL 才能用 LEFT 键退出 
     */
    grouplist_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    grouplist_conf.prompt = "◆";
    grouplist_conf.item_pos = pts;
    grouplist_conf.title_pos.x = 0;
    grouplist_conf.title_pos.y = 0;
    grouplist_conf.pos = 1;     /* initialize cursor on the first mailgroup */
    grouplist_conf.page_pos = 1;        /* initialize page to the first one */

    grouplist_conf.show_data = tmpl_show;
    grouplist_conf.pre_key_command = tmpl_prekey;
    grouplist_conf.key_command = tmpl_key;
    grouplist_conf.show_title = tmpl_refresh;
    grouplist_conf.get_data = tmpl_getdata;
	grouplist_conf.on_select = tmpl_select;

    list_select_loop(&grouplist_conf);

    free(pts);
	tmpl_free();

    return FULLUPDATE;

}

/*********************************
 * 模块选择方面
 ***************************/
static int choose_tmpl_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[版面模板选择]",
               "退出[\x1b[1;32m←\x1b[0;37m] 选择[\x1b[1;32m↑\x1b[0;37m,\x1b[1;32m↓\x1b[0;37m] 使用[\x1b[1;32mSPACE\x1b[0;37m] 查看正文[\033[1;32ms\033[0;37m] 查看问题[\033[1;32mw\033[0;37m] 查看标题[\033[1;32mx\033[m]");
    move(2, 0);
    prints("\033[0;1;37;44m %4s %-60s %8s", "序号", "名称","问题个数");
    clrtoeol();
    update_endline();
    return SHOW_CONTINUE;
}

static int choose_tmpl_getdata(struct _select_def *conf, int pos, int len)
{
    conf->item_count = template_num;
    return SHOW_CONTINUE;
}

static int choose_tmpl_select(struct _select_def *conf)
{
	t_now = conf->pos;
	return SHOW_QUIT;
}

static int choose_tmpl_post(char * title, char *fname){

	FILE *fp;
	FILE *fpsrc;
	char filepath[STRLEN];
	int i, ret=1;
	int write_ok = 0;
	char * tmp[ MAX_CONTENT ];
	char newtitle[STRLEN];
	int oldmode = uinfo.mode;
    bool modifying=false, loop=true;

	if(t_now <= 0 || t_now > MAX_TEMPLATE )
		return -1;

	if( ptemplate[t_now-1].tmpl->content_num <= 0 )
		return -1;

    while(loop) {

	if((fp = fopen(fname, "w"))==NULL){
		return -1;
	}

    modify_user_mode(POSTTMPL);
	for(i=0; i< ptemplate[t_now-1].tmpl->content_num; i++){
		char *ans;

        if(modifying)
            ans = tmp[i];
        else
        {
            ans = (char *)malloc(ptemplate[t_now-1].cont[i].length + 2);
            ans[0] = '\0';
        }
		if( ans == NULL ){
    		modify_user_mode(oldmode);
			fclose(fp);
			return -1;
		}
		clear();
		move(1,0);
		prints("Ctrl+Q 换行, ENTER 发送");
		move(3,0);
		prints("模板问题:%s",ptemplate[t_now-1].cont[i].text);
		move(4,0);
		prints("模板回答(最长%d字符):",ptemplate[t_now-1].cont[i].length);
        multi_getdata(6, 0, 79, NULL, ans, ptemplate[t_now-1].cont[i].length+1, 11, false, 0);
		tmp[i] = ans;
	}
    modify_user_mode(oldmode);

	if( ptemplate[t_now-1].tmpl->filename[0] ){
    	struct stat st;
		setbfile( filepath,currboard->filename, ptemplate[t_now-1].tmpl->filename);
		if( stat(filepath, &st) == 0 && S_ISREG(st.st_mode) && st.st_size>2){
			if((fpsrc = fopen(filepath,"r"))!=NULL){
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
							if( l<=0 || l > ptemplate[t_now-1].tmpl->content_num ){
								fputc('[', fp);
								continue;
							}
							fprintf(fp,"%s",tmp[l-1]);
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
		for(i=0; i< ptemplate[t_now-1].tmpl->content_num; i++)
			fprintf(fp,"\033[1;32m%s:\033[m\n%s\n\n",ptemplate[t_now-1].cont[i].text, tmp[i]);
	}
	fclose(fp);

	if( ptemplate[t_now-1].tmpl->title_tmpl[0] ){
		char *pn,*pe;
		char *buf;
		int l;
		int newl = 0;

		newtitle[0]='\0';
		buf = ptemplate[t_now-1].tmpl->title_tmpl;

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
				if( l<0 || l > ptemplate[t_now-1].tmpl->content_num ){
					if( newl < STRLEN - 1 ){
						newtitle[newl] = *pn ;
						newtitle[newl+1]='\0';
						newl ++;
					}
					continue;
				}
				if( l == 0 ){
					int ti;
					for( ti=0; title[ti]!='\0' && newl < STRLEN - 1; ti++, newl++ ){
						newtitle[newl] = title[ti] ;
						newtitle[newl+1]='\0';
					}
				}else{
					int ti;
					for( ti=0; tmp[l-1][ti]!='\0' && tmp[l-1][ti]!='\n' && tmp[l-1][ti]!='\r' && newl < STRLEN - 1; ti++, newl++ ){
						newtitle[newl] = tmp[l-1][ti] ;
						newtitle[newl+1]='\0';
					}
				}
				pn = pe;
				continue;
			}
		}
		strncpy(title, newtitle, STRLEN);
		title[STRLEN-1]='\0';
	}

    do{
        char ans[4];
        clear();
        ansimore2(fname,false,0,19);
        move(21,0);
        clrtobot();
        prints("标题: %s",title);
        getdata(t_lines-1,0,"确认发表 (Y)发表 (N)退出 (E)重新编辑 [Y]: ",ans,2,DOECHO,NULL,true);
        switch(toupper(ans[0])){
            case 'N':
                loop=false;
                ret=-1;
                break;
            case 'E':
                modifying=true;
                break;
            default:
                loop=false;
                ret=1;
                break;
        }
    }
    while(0);

    }

	for(i=0; i< ptemplate[t_now-1].tmpl->content_num; i++)
		free( tmp[i] );

	return ret;
}

static int choose_tmpl_key(struct _select_def *conf, int key)
{
	switch (key) {
	case 's' :
	{
		char filepath[STRLEN];

		if( conf->pos > template_num )
			return SHOW_CONTINUE;

		if( ptemplate[conf->pos-1].tmpl->filename[0] ){
			clear();
			setbfile(filepath,currboard->filename, ptemplate[conf->pos-1].tmpl->filename);

			ansimore(filepath, 1);
			return SHOW_REFRESH;
		}

		return SHOW_CONTINUE;
	}
		break;
	case 'w':
		{
			clear();
			if( ptemplate[conf->pos-1].tmpl->content_num <= 0 ){
				move(5,0);
				prints("斑竹还没有设置问题，本模板暂不可用\n");
			}else{
				int i;
				for(i=0;i<ptemplate[conf->pos-1].tmpl->content_num;i++){
					move(i+2,0);
					prints("\033[1;32m问题 %d\033[m:%s  \033[1;32m最长回答\033[m%d\033[1;32m字节\033[m", i+1, ptemplate[conf->pos-1].cont[i].text, ptemplate[conf->pos-1].cont[i].length);
				}
			}
			pressanykey();

			return SHOW_REFRESH;
		}
	case 'x' :
		{
			clear();
			move(2,0);
			prints("此模版的标题设置为");
			move(4,0);
			prints("%s",ptemplate[conf->pos-1].tmpl->title_tmpl);
			pressanykey();
			return SHOW_REFRESH;
		}
	default:
		break;
	}

	return SHOW_CONTINUE;
}

int choose_tmpl(char *title, char *fname)
{
	POINT *pts;
    struct _select_def grouplist_conf;
    int i;

	if( tmpl_init(0) < 0 )
		return  -1;

	if( template_num == 0 ){
		clear();
		move(3,0);
		prints("本版没有模板可供使用");
		pressanykey();
		tmpl_free();
		return  -1;
	}

	t_now = 0;

    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    bzero(&grouplist_conf, sizeof(struct _select_def));

	grouplist_conf.arg = fname;
    grouplist_conf.item_count = template_num;
    grouplist_conf.item_per_page = BBS_PAGESIZE;
    grouplist_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    grouplist_conf.prompt = "◆";
    grouplist_conf.item_pos = pts;
    grouplist_conf.title_pos.x = 0;
    grouplist_conf.title_pos.y = 0;
    grouplist_conf.pos = 1;
    grouplist_conf.page_pos = 1;

    grouplist_conf.show_data = tmpl_show;
    grouplist_conf.pre_key_command = tmpl_prekey;
    grouplist_conf.key_command = choose_tmpl_key;
    grouplist_conf.show_title = choose_tmpl_refresh;
    grouplist_conf.get_data = choose_tmpl_getdata;
	grouplist_conf.on_select = choose_tmpl_select;

    list_select_loop(&grouplist_conf);

	if(t_now > 0){
		if( choose_tmpl_post(title, fname) < 0 )
			t_now = 0;
	}

	free(pts);
	tmpl_free();

	if( t_now > 0){
		t_now = 0;
		return 0;
	}
	return -1;
}

