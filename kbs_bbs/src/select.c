/*
	选择列表的通用接口
	KCN 2002.07.27
*/
#include "bbs.h"
#include "select.h"
static int check_valid(struct _select_def *conf)
{
    if (conf->item_count <= 0)
        return SHOW_QUIT;
    if (conf->page_pos > conf->item_count)
        conf->page_pos = conf->item_count;
    if (conf->page_pos <= 1)
        conf->page_pos = 1;
    if (conf->pos > conf->item_count)
        conf->pos = conf->item_count;
    if (conf->pos <= 1)
        conf->pos = 1;
    return SHOW_CONTINUE;
}
static void select_wrong(struct _select_def *conf)
{
    if (conf->flag & LF_BELL) {
        bell();
        oflush();
    }
}
static int show_item(struct _select_def *conf, int item, bool clear)
{
    int idx = item - conf->page_pos;

    if (conf->pos == item) {
        int pre_len = 0;

        if (conf->prompt) {
            pre_len = strlen(conf->prompt);
            if (conf->item_pos[idx].x > pre_len)
                good_move(conf->item_pos[idx].y, conf->item_pos[idx].x - pre_len);
            else
                good_move(conf->item_pos[idx].y, 0);
            outns(conf->prompt, pre_len);
        } else
            good_move(conf->item_pos[idx].y, conf->item_pos[idx].x);
        if (conf->flag & LF_HILIGHTSEL)
            outns("\x1b[1;45m", 7);
    } else {
        good_move(conf->item_pos[idx].y, conf->item_pos[idx].x);
    }
    if (clear) {
        if (conf->flag & LF_VSCROLL)
            clrtoeol();
        //TODO:
        //check LF_NEEDFORCECLEAN  to clean
    }
    if (conf->show_data)
        if ((*conf->show_data) (conf, item) == SHOW_QUIT)
            return SHOW_QUIT;
    if ((conf->flag & LF_HILIGHTSEL) && (conf->pos == item))
        outns("\x1b[m", 3);
    return SHOW_CONTINUE;
}
static int refresh_select(struct _select_def *conf)
{
    int i;

    //TODO:
    //目前应该清除的区域尚未定义，所以先清全部
    good_move(conf->title_pos.y, conf->title_pos.x);
    clrtobot();
    if (conf->show_title) {
        (*conf->show_title) (conf);
    }
    if (conf->show_endline) {
        good_move(conf->endline_pos.y, conf->endline_pos.x);
        (*conf->show_endline) (conf);
    }
    for (i = conf->page_pos; (i < conf->page_pos + conf->item_per_page)
         && (i <= conf->item_count); i++)
        if (show_item(conf, i, false) == SHOW_QUIT)
            return SHOW_QUIT;
    refresh();
    return SHOW_CONTINUE;
}
static int select_change(struct _select_def *conf, int new_pos)
{
    int ret = SHOW_CONTINUE;

    if (new_pos <= 0 || new_pos > conf->item_count) {
        if ((new_pos == 0) && (conf->flag & LF_LOOP)) {
            new_pos = conf->item_count;
        } else if ((new_pos == conf->item_count + 1) && (conf->flag & LF_LOOP)) {
            new_pos = 1;
        } else {
            select_wrong(conf);
            return SHOW_CONTINUE;
        }
    }
    if (conf->on_selchange) {
        ret = (*conf->on_selchange) (conf, new_pos);
        if (ret != SHOW_CONTINUE)
            return ret;
    }
    if (conf->flag & LF_MULTIPAGE) {
        //TODO multi page
    } else {
        if (conf->prompt) {
            int pre_len = strlen(conf->prompt);
            int idx = conf->pos - conf->page_pos;
            int newidx = new_pos - conf->page_pos;

            if (pre_len) {
                good_move(conf->item_pos[idx].y, conf->item_pos[idx].x - pre_len);
                outns("                                                               ", pre_len);
                if (conf->item_pos[newidx].x > pre_len)
                    good_move(conf->item_pos[newidx].y, conf->item_pos[newidx].x - pre_len);
                else
                    good_move(conf->item_pos[newidx].y, 0);
                outns(conf->prompt, pre_len);
            }
        }
        // 如果是高亮的选择方式，需要清除原来的行
        // 和重绘新行
        conf->pos = new_pos;
        if (conf->flag & LF_HILIGHTSEL) {
            conf->new_pos = conf->pos;
            show_item(conf, conf->pos, true);
            show_item(conf, conf->new_pos, true);
        }
        refresh();
    }
    conf->pos = new_pos;
    return ret;
}
static int do_select_internal(struct _select_def *conf, int key)
{
    int ret = SHOW_CONTINUE;

    if (!(conf->flag & LF_INITED)) {
        if (check_valid(conf) == SHOW_QUIT)
            return SHOW_QUIT;
        if (conf->init)
            (*conf->init) (conf);
        if (conf->page_init)
            (*conf->page_init) (conf);
        conf->flag |= LF_INITED;
        if (conf->item_count > conf->item_per_page)
            conf->flag |= LF_MULTIPAGE;
        if (refresh_select(conf) == SHOW_QUIT)
            return SHOW_QUIT;
    }
    if (key == KEY_INIT)
        return SHOW_CONTINUE;

    /*如果定义了预处理键,那么执行之*/
    if (conf->pre_key_command) {
    	ret=(*conf->pre_key_command)(conf,&key);
    	if (ret!=SHOW_CONTINUE)
    		return ret;
    }

    /* 查转换表*/
    if (conf->key_table) {
    	struct key_translate *p=conf->key_table;
    	while (p->ch!=-1) {
    		if (p->ch==key) {
    			key=p->command;
    			break;
    		}
    	}
    }
    switch (key) {
    case KEY_REFRESH:
        return refresh_select(conf);
    case KEY_UP:
        return select_change(conf, conf->pos - 1);
    case KEY_DOWN:
        return select_change(conf, conf->pos + 1);
    case KEY_SELECT:
    case '\n':
    case '\r':
        if (conf->on_select)
            return (*conf->on_select) (conf);
        return SHOW_SELECT;
    case KEY_LEFT:
        if (conf->flag & LF_VSCROLL)
            return SHOW_QUIT;
        {
            int i;
            int idx = conf->pos - conf->page_pos;
            int x = 0;
            int x2 = 1024;
            int findidx = -1;
            int findidx2 = -1;

            for (i = 0; i < conf->item_per_page; i++) {
                if (conf->item_pos[i].y == conf->item_pos[idx].y) {
                    if ((conf->item_pos[i].x < conf->item_pos[idx].x) && (conf->item_pos[i].x > x)) {
                        x = conf->item_pos[i].x;
                        findidx = i;
                    }
                    if ((conf->item_pos[i].x > conf->item_pos[idx].x) && (conf->item_pos[i].x < x2)) {
                        x2 = conf->item_pos[i].x;
                        findidx2 = i;
                    }
                }
            }
            if (findidx == -1) {
                if ((conf->flag & LF_LOOP) && (findidx2 != -1)) {
                    findidx = findidx2;
                } else {
                    select_wrong(conf);
                    return SHOW_CONTINUE;
                }
            }
            return select_change(conf, conf->page_pos + findidx);
        }
        break;
    case KEY_RIGHT:
        if (conf->flag & LF_VSCROLL)
            return SHOW_SELECT;
        {
            int i;
            int idx = conf->pos - conf->page_pos;
            int x = 1024;
            int x2 = 0;
            int findidx = -1;
            int findidx2 = -1;

            for (i = 0; i < conf->item_per_page; i++) {
                if (conf->item_pos[i].y == conf->item_pos[idx].y) {
                    if ((conf->item_pos[i].x > conf->item_pos[idx].x) && (conf->item_pos[i].x < x)) {
                        x = conf->item_pos[i].x;
                        findidx = i;
                    }
                    if ((conf->item_pos[i].x < conf->item_pos[idx].x) && (conf->item_pos[i].x > x2)) {
                        x2 = conf->item_pos[i].x;
                        findidx2 = i;
                    }
                }
            }
            if (findidx == -1) {
                if ((conf->flag & LF_LOOP) && (findidx2 != -1)) {
                    findidx = findidx2;
                } else {
                    select_wrong(conf);
                    return SHOW_CONTINUE;
                }
            }
            return select_change(conf, conf->page_pos + findidx);
        }
    }
    if (conf->key_command)
        return (*conf->key_command) (conf, key);
    return SHOW_CONTINUE;
}
int list_select(struct _select_def *conf, int key)
{
    int ret;

//      int old_page_num=conf->page_num;
    int old_pos = conf->pos;

//      int old_item_count = conf->item_count;
	ret=SHOW_QUIT;
	while (ret!=SHOW_CONTINUE) {
	    ret = do_select_internal(conf, key);
	    switch (ret) {
	    case SHOW_DIRCHANGE:
	        if (conf->get_data)
	            ret=(*conf->get_data) (conf, conf->page_pos, conf->item_per_page);
	        if (ret==SHOW_QUIT)
	        	return ret;
	        if (check_valid(conf) == SHOW_QUIT)
	            return SHOW_QUIT;
	    case SHOW_REFRESH:
	        ret=refresh_select(conf);
	        break;
	    case SHOW_SELCHANGE:
	        ret = select_change(conf, conf->new_pos);
	        if ((ret!=SHOW_SELECT)&&(ret!=SHOW_QUIT)) {
	        	key=ret;
	        	continue;
	        }
	    case SHOW_REFRESHSELECT:
	        show_item(conf, conf->pos, true);
	        ret=SHOW_CONTINUE;
	        break;
	    case SHOW_CONTINUE:
		return SHOW_CONTINUE;
	    case SHOW_SELECT:
	        ret = do_select_internal(conf, KEY_SELECT);
	        if (ret!=SHOW_SELECT) {
	        	key=ret;
	        	continue;
	        }
	    case SHOW_QUIT:
	    	if (conf->quit)
	    		(*conf->quit)(conf);
	        return ret;
	    }
	    key=ret;
	}
    return SHOW_CONTINUE;
}
int list_select_loop(struct _select_def *conf)
{
    int ch;

    list_select(conf, KEY_INIT);
    while (1) {
    	int ret;
        ch = igetkey();
        ret=list_select(conf, ch);
        if ((ret == SHOW_QUIT)||(ret == SHOW_SELECT))
            return ret;
    }
}

struct _simple_select_arg{
	struct _select_item* items;
	int flag;
};


static int simple_onselect(struct _select_def *conf)
{
    struct _simple_select_arg *arg = (struct _simple_select_arg *) conf->arg;
    return SHOW_CONTINUE;
}

static int simple_show(struct _select_def *conf, int i)
{
    struct _simple_select_arg *arg = (struct _simple_select_arg *) conf->arg;
    struct _select_item * item;

	i=i-1;
	item=&arg->items[i];
	if (item->type==SIT_SELECT) {
		outs((char*)item->data);
	} //TODO: Add other SIT support
    return SHOW_CONTINUE;
}

static int simple_key(struct _select_def *conf, int key)
{
    struct _simple_select_arg *arg = (struct _simple_select_arg *) conf->arg;
    int sel;

	if (arg->flag&SIF_NUMBERKEY) {
	    if (key <= '0'+conf->item_count && key >= '0') {
	        conf->new_pos = key-'0' +1;
	        return SHOW_SELCHANGE;
	    }
	} else
	if (arg->flag&SIF_ALPHAKEY) {
	    if (key <= 'z' && key >= 'a')
	        sel = key - 'a';
	    else
	        sel = key - 'A';
	    if (sel >= 0 && sel < (conf->item_count)) {
	        conf->new_pos = sel + 1;
	        return SHOW_SELCHANGE;
	    }
	}
    return SHOW_CONTINUE;
}


int simple_select_loop(struct _select_item* item_conf,int flag,int titlex,int titley,union _select_return_value* ret)
{
    struct _select_def simple_conf;
    struct _simple_select_arg arg;
    POINT *pts;
    int i;
    int numbers;

    pts = (POINT *) malloc(sizeof(POINT) * numbers);

    for (numbers = 0; item_conf[numbers].x!=-1; numbers++) {
        pts[numbers].x = item_conf[numbers].x;
        pts[numbers].y = item_conf[numbers].y;
    }

    arg.items=item_conf;
    arg.flag=flag;
    
    bzero((char *) &simple_conf, sizeof(struct _select_def));
    simple_conf.item_count = numbers;
    simple_conf.item_per_page = numbers;
    simple_conf.flag = LF_BELL | LF_LOOP; //|LF_HILIGHTSEL;
    simple_conf.prompt = "◆";
    simple_conf.item_pos = pts;
    simple_conf.arg = &arg;
    simple_conf.title_pos.x = titlex;
    simple_conf.title_pos.y = titley;
    simple_conf.pos = numbers;
    simple_conf.on_select = simple_onselect;
    simple_conf.show_data = simple_show;
    simple_conf.key_command = simple_key;

    i=list_select_loop(&simple_conf);
    free(pts);

    if ((flag&SIF_SINGLE)&&(i==SHOW_SELECT))
    	return simple_conf.pos;
    return 0;
}

