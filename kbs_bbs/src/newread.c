/**
  *  新的read.c，使用select结构重写原来的i_read
  * 先实现mail
  */
#include "bbs.h"
#include "read.h"

extern unsigned int tmpuser;

//每一个模式上次阅读位置保存
struct _read_pos {
    int mode;
    char* key;
    int pos;
    struct _read_pos* next;
}static *read_pos_head=NULL;

static int read_getdata(struct _select_def *conf, int pos, int len);
/* 获得上一次阅读的位置
  * @param mode 阅读模式
  * @param direct dir的文件名
  * @return 上次位置，为0则失败
*/
int getPos(int mode,char* direct,struct boardheader* bh)
{
    struct _read_pos* ptr;
    char* key;
    
    ptr=read_pos_head;
    if (mode==DIR_MODE_MAIL)
        key=direct;
    else if (mode==DIR_MODE_FRIEND)
        key=NULL;
    else
        key=bh->filename;
    while (ptr!=NULL) {
        if (mode==ptr->mode) {
            if ((key==NULL)||((ptr->key!=NULL)&&(!strcmp(key,ptr->key))))
                return ptr->pos;
        }
        ptr=ptr->next;
    }
    return 0;
}

/* 保存阅读位置
  * @param mode 阅读模式
  * @param direct dir的文件名
  * @param pos 阅读位置
*/
void savePos(int mode,char* direct,int pos,struct boardheader* bh)
{
    struct _read_pos*ptr;
    char* key;
    
    ptr=read_pos_head;
    if (mode==DIR_MODE_MAIL)
        key=direct;
    else if (mode==DIR_MODE_FRIEND)
        key=NULL;
    else
        key=bh->filename;
    while (ptr!=NULL) {
        if (mode==ptr->mode) {
            if ((key==NULL)||((ptr->key!=NULL)&&(!strcmp(key,ptr->key))))
                break;
        }
        ptr=ptr->next;
    }
    if (ptr==NULL) { /*增加一个表项*/
        ptr=(struct _read_pos*)malloc(sizeof(struct _read_pos));
        ptr->next=read_pos_head;
        read_pos_head=ptr;
    }
    ptr->mode=mode;
    if (key!=NULL) {
    ptr->key=(char*)malloc(strlen(key)+1);
    strcpy(ptr->key,key);
    } else
        ptr->key=NULL;
    ptr->pos=pos;
}

static void read_setusermode(enum BBS_DIR_MODE cmdmode)
{
    if (cmdmode==DIR_MODE_MAIL) {
        modify_user_mode(RMAIL);
     }
    else {
        modify_user_mode(READING);
    } //todo: other mode 
}

static int read_search_articles(struct _select_def* conf, char *query, bool up, int aflag);
/* 寻找下一个未读文章，找到返回位置，否则返回0*/
int find_nextnew(struct _select_def* conf,int begin)
{
    struct read_arg* arg=conf->arg;
    char *data;
    struct fileheader *pFh,*nowFh;
    off_t size;
    bool found=false;
    int i=0;
    if (begin<=0)
        return 0;
    BBS_TRY {
        if (safe_mmapfile_handle(arg->fd, PROT_READ|PROT_WRITE, MAP_SHARED, &data, &size) ) {
            pFh = (struct fileheader*)data;
            nowFh=pFh+begin-1;
            found=false;
            for (i=begin-1;i<size/sizeof(struct fileheader);i++,nowFh++) {
                if (brc_unread(nowFh->id, getSession())) {
                    found=true;
                    break;
                }
            }
        }
    }
    BBS_CATCH {
    }
    BBS_END;
    if (data!=MAP_FAILED)
        end_mmapfile(data, size, -1);
    if (found)
        return i+1;
    return 0;
}

/*用于apply_record的回调函数*/
int fileheader_thread_read(struct _select_def* conf, struct fileheader* fh,int ent, void* extraarg)
{
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    POINTDIFF mode=(POINTDIFF)extraarg;
    switch (mode) {
        case SR_FIRSTNEWDOWNSEARCH:
        case SR_FIRSTNEW:
#ifdef HAVE_BRC_CONTROL
			if (read_arg->mode==DIR_MODE_MAIL) {
                if (!(fh->accessed[0] & FILE_READ)) {
                conf->new_pos=ent;
				if (mode==SR_FIRSTNEW)
                    return APPLY_CONTINUE;
			    if (mode==SR_FIRSTNEWDOWNSEARCH)
                    return APPLY_QUIT;
                }
            } else {
            if (brc_unread(fh->id, getSession())) {
                conf->new_pos=ent;
				if (mode==SR_FIRSTNEW)
                    return APPLY_CONTINUE;
			    if (mode==SR_FIRSTNEWDOWNSEARCH)
                    return APPLY_QUIT;
            }
            }
/* readed */
			if (mode==SR_FIRSTNEW)
                    return APPLY_CONTINUE;
            else
			if (mode==SR_FIRSTNEWDOWNSEARCH)
                    return APPLY_CONTINUE;
            break;
#endif
        case SR_FIRST:
        case SR_LAST:
            conf->new_pos=ent;
            return APPLY_CONTINUE;/*继续查到底*/
        case SR_NEXT:
        case SR_PREV:
            conf->new_pos=ent;
            return APPLY_QUIT;
    }
    return APPLY_QUIT;
}


static int read_key(struct _select_def *conf, int command)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    int i;
    int ret=SHOW_CONTINUE;
    int mode=DONOTHING;
    struct fileheader* currfh;

    switch (command) {    
        case 'L':
        case 'l':                  /* Luzi 1997.10.31 */
            if (uinfo.mode != LOOKMSGS) {
                show_allmsgs();
                mode=FULLUPDATE;
                break;
            }

            else
                return DONOTHING;
        case 'w':                  /* Luzi 1997.10.31 */
            if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
                break;
            s_msg();
            mode=FULLUPDATE;
            break;
        case 'u':                  /* Haohmaru. 99.11.29 */
            clear();
            modify_user_mode(QUERY);
            t_query(NULL);
            mode= FULLUPDATE;
            break;
		case 'U':		/* pig2532 2005.12.10 */
			board_query();
			mode=FULLUPDATE;
			break;
        case 'O':
        case 'o':                  /* Luzi 1997.10.31 */
            {                       /* Leeward 98.10.26 fix a bug by saving old mode */
                int savemode = uinfo.mode;

                if (!HAS_PERM(getCurrentUser(), PERM_BASIC))
                    break;
                t_friends();
                modify_user_mode(savemode);
                mode=FULLUPDATE;
                break;
            }
    }
    if (conf->pos!=0)
        currfh=(struct fileheader*)(arg->data+(conf->pos - conf->page_pos) * arg->ssize);
    else
	currfh=NULL;
    for (i = 0; arg->rcmdlist[i].fptr != NULL; i++) {
        if (arg->rcmdlist[i].key == command) {
            mode = (*(arg->rcmdlist[i].fptr)) (conf, currfh, arg->rcmdlist[i].arg);
            break;
        }
    }
    read_setusermode(arg->mode);
    switch (mode) {
        case CHANGEMODE:
            arg->returnvalue=CHANGEMODE;
            ret=SHOW_QUIT;
            break;
        case FULLUPDATE:/*要检查一下时间*/
	    {
		 struct stat st;
		 if (fstat(arg->fd,&st)!=-1) {
		  if (st.st_mtime!=arg->lastupdatetime) {
			  ret=SHOW_DIRCHANGE;
		      break;
		  }
	         }
	    }
        case PARTUPDATE:
            clear();
            ret=SHOW_REFRESH;
            break;
        case NEWDIRECT: {
                int newfd,lastpos;
                if ((newfd = open(arg->direct, O_RDWR, 0)) != -1) {
                    close(arg->fd);
                    arg->fd=newfd;
                }
/*其实这里有个问题，在savePos的时候，arg->direct其实已经被改动成新的direct了
  正确的做法是类似newmode的写法，在这里保存原来的。
  但是，因为返回NEWDIRECT的只有阅读的时候，而阅读的时候只用当前版面作为key,
  所以，其实arg->direct是无用的。
  另一个问题，在这里，conf->pos没法设定成filecount,因为此时还没有getdata
*/
                savePos(arg->mode,arg->direct,conf->pos,arg->board);
                arg->board=currboard;
                arg->bid=getboardnum(currboard->filename, NULL);
                arg->boardstatus=getbstatus(arg->bid);
                read_getdata(conf,-1,conf->item_per_page);
                lastpos=getPos(arg->newmode,arg->direct,currboard);
                if ((lastpos!=0)&&(lastpos<arg->filecount))
                    conf->pos = lastpos;
                else {
                    conf->pos = arg->filecount;
                }
                arg->mode=arg->newmode;
                arg->newmode=-1;
            }
        case DIRCHANGED:
        case NEWSCREEN:
            ret=SHOW_DIRCHANGE;
            break;
        case DOQUIT:
            ret=SHOW_QUIT;
            break;
        case READ_NEXT:
        case GOTO_NEXT:
            ret=SHOW_REFRESH;
            if (arg->readmode==READ_NEW) { //find next new article;
                int findpos=find_nextnew(conf,conf->pos);
                if (findpos) {
                    conf->new_pos=findpos;
                    list_select_add_key(conf, 'r');
                    ret=SHOW_SELCHANGE;
                }
            } else if (arg->readmode==READ_NORMAL) {
                if (conf->pos<conf->item_count) {
                    conf->new_pos = conf->pos + 1;
                    if (mode==READ_NEXT)
                        list_select_add_key(conf,'r'); //SEL change的下一条指令是read
                    ret=SHOW_SELCHANGE;
                }
            } else  if (arg->readmode==READ_THREAD) { /* 处理同主题阅读*/
                int findthread=apply_thread(conf,
                    currfh,
                    fileheader_thread_read,
                    false,
                    true,
                    (void*)SR_NEXT);
                if (findthread!=0) {
                    list_select_add_key(conf,'r'); //SEL change的下一条指令是read
                    ret=SHOW_SELCHANGE;
                }
            } else { //处理同作者阅读
                if (read_search_articles(conf,currfh->owner,false,1)==1) {
                    list_select_add_key(conf,'r'); //SEL change的下一条指令是read
                    ret=SHOW_SELCHANGE;
                }
            }
            
            if (ret==SHOW_REFRESH) {
                arg->readmode=READ_NORMAL;
		if (arg->oldpos!=0) {
                /*恢复到原来的位置*/
                list_select_add_key(conf,KEY_REFRESH); //先刷新一下
		conf->new_pos=arg->oldpos;
                ret=SHOW_SELCHANGE;
                arg->oldpos=0;
               }
            }
            break;
        case READ_PREV:
            if (arg->readmode==READ_NEW) {
                break;
            }
            if (arg->readmode==READ_NORMAL) {
                if (conf->pos>1) {
                    conf->new_pos = conf->pos - 1;
                    list_select_add_key(conf,'r'); //SEL change的下一条指令是read
                    ret=SHOW_SELCHANGE;
                } else ret= SHOW_REFRESH;
            }
            else if (arg->readmode==READ_THREAD) {/* 处理同主题阅读*/
                int findthread=apply_thread(conf,
                    (struct fileheader*)(arg->data+(conf->pos - conf->page_pos) * arg->ssize),
                    fileheader_thread_read,
                    false,
                    false,
                    (void*)SR_PREV);
                if (findthread!=0) {
                    list_select_add_key(conf,'r'); //SEL change的下一条指令是read
                    ret=SHOW_SELCHANGE;
                }
                else ret=SHOW_REFRESH;
            } else { //处理同作者阅读
                if (read_search_articles(conf,currfh->owner,true,1)==1) {
                    list_select_add_key(conf,'r'); //SEL change的下一条指令是read
                    ret=SHOW_SELCHANGE;
                }
            }
            if (ret==SHOW_REFRESH) {
                arg->readmode=READ_NORMAL;
		if (arg->oldpos!=0) {
                /*恢复到原来的位置*/
                list_select_add_key(conf,KEY_REFRESH); //先刷新一下
                ret=SHOW_SELCHANGE;
		conf->new_pos=arg->oldpos;
                arg->oldpos=0;
               }
            }
            break;
        case SELCHANGE:
            ret=SHOW_SELCHANGE;
            break;
    } 
    if ((ret!=SHOW_SELCHANGE)&&!list_select_has_key(conf)) /*返回非顺序阅读模式*/
        arg->readmode=READ_NORMAL;
    return ret;
}

static int read_onselect(struct _select_def *conf)
{
    return SHOW_CONTINUE;
}

static int read_getdata(struct _select_def *conf, int pos, int len)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    struct stat st;
    int count;

    if (arg->data==NULL)
        arg->data=calloc(conf->item_per_page,arg->ssize);
    
    if (fstat(arg->fd,&st)!=-1) {
        int entry=0;
        int dingcount=0;
        int n;
	arg->lastupdatetime=st.st_mtime;
        count=st.st_size/arg->ssize;
        arg->filecount=count;
        if ((arg->mode==DIR_MODE_NORMAL)||
            (arg->mode==DIR_MODE_MARK))
        { //需要检查置顶
            dingcount=arg->boardstatus->toptitle;
        }

	if (count+dingcount==0) {
		conf->item_count=0;
		return SHOW_CONTINUE;
        }
        if (pos>count+dingcount) {
            conf->new_pos=count+dingcount-conf->item_per_page+1;
	    if (conf->new_pos<=0) conf->new_pos=1;
            list_select_add_key(conf,KEY_DIRCHANGE);
	    newbbslog(BBSLOG_DEBUG,"%s pos %d count %d",arg->board->filename,pos,count+dingcount);
            return SHOW_SELCHANGE;
	}
	if ((pos!=-1)&&(pos<=count)) {
          if (lseek(arg->fd, arg->ssize * (pos - 1), SEEK_SET) != -1) {
            if ((n = read(arg->fd, arg->data, arg->ssize * len)) != -1) {
                entry=(n / arg->ssize);
            }
          }
	}
        
        /* 获得置顶的数据*/
        if (dingcount&&pos!=-1) {
            if (entry!=len) { //需要读入.DING
                n=0;
                if (pos>count) {
                    n=len;
                    if ((n+pos-count-1)>dingcount) {
                        n=dingcount-(pos-count-1);
                    }
                    if (n>0) {
                        memcpy(arg->data,&arg->boardstatus->topfh[pos-count-1],arg->ssize * n);
                        entry=pos-count;
                    }
                }  else {
                    n=len-entry;
                    if (n>dingcount)
                        n=dingcount;
                    memcpy(arg->data+arg->ssize*entry,&arg->boardstatus->topfh[0],arg->ssize * n);
                }
            }
            /*加上置顶个数*/
            count+=dingcount;
        }
        conf->item_count=count;
    } else
        return SHOW_QUIT;
    return SHOW_CONTINUE;
}

static int read_title(struct _select_def *conf)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    clear();
    (*arg->dotitle) (conf);
    return SHOW_CONTINUE;
}

extern int draw_content(char *fn, struct fileheader *fh);
static int read_showcontent(struct _select_def* conf,int newpos)
{
    char buf[256], *t;
    struct read_arg *arg = (struct read_arg *) conf->arg;
    struct fileheader* h;
    strcpy(buf, read_getcurrdirect(conf));
    if ((t = strrchr(buf, '/')) != NULL)
        *t = '\0';
    if ((newpos-conf->page_pos)>=0&&(newpos-conf->page_pos<conf->item_per_page)) {
    h = (struct fileheader*)(arg->data+(newpos-conf->page_pos) * arg->ssize);
    sprintf(genbuf, "%s/%s", buf, h->filename);
    draw_content(genbuf,h);
    outs("\x1b[m");
    }
    return SHOW_CONTINUE;
}


static int read_endline(struct _select_def *conf)
{
    if (conf->tmpnum==-1)
        update_endline();
    else if (DEFINE(getCurrentUser(), DEF_ENDLINE)) {
        extern time_t login_start_time;
        int allstay;
        char pntbuf[256], nullbuf[2] = " ";
        char lbuf[11];

#ifdef FLOWBANNER
	allstay = (DEFINE(getCurrentUser(), DEF_SHOWBANNER)) ? (time(0) % 3) : 0;
	if (allstay) {
		if (allstay & 1) {	//显示系统浮动信息
			struct public_data *publicshm = get_publicshm();
			if (publicshm->bannercount) 
				snprintf(pntbuf, 256, "\033[33;44m%s\033[m", publicshm->banners[(time(0)>>1)%publicshm->bannercount]);
			else allstay=0;
		} else {	//显示版面浮动信息
			if (currboard->bannercount) 
				snprintf(pntbuf, 256, "\033[33;44m%s\033[m", currboard->banners[(time(0)>>1)%currboard->bannercount]);
			else allstay=0;
		}
	}
	if (!allstay) {
#endif
        snprintf(lbuf,11,"%d",conf->tmpnum);

        allstay = (time(0) - login_start_time) / 60;
        snprintf(pntbuf, 256, "\033[33;44m转到∶[\033[36m%9.9s\033[33m]" "  呼叫器[好友:%3s∶一般:%3s] 使用者[\033[36m%.12s\033[33m]%s停留[%3d:%2d]\033[m", 
            lbuf, (!(uinfo.pager & FRIEND_PAGER)) ? "NO " : "YES", (uinfo.pager & ALL_PAGER) ? "YES" : "NO ", getCurrentUser()->userid,      /*13-strlen(getCurrentUser()->userid)
                                                                                                                                                                                                                                                                                         * TODO:这个地方有问题，他想对齐，但是代码不对
                                                                                                                                                                                                                                                                                         * , */ nullbuf,
                 (allstay / 60) % 1000, allstay % 60);
#ifdef FLOWBANNER
	}
#endif
        move(t_lines - 1, 0);
        prints(pntbuf);
        clrtoeol();
    }
    if (TDEFINE(TDEF_SPLITSCREEN))
	    read_showcontent(conf,conf->pos);
    return SHOW_CONTINUE;
}

static int read_prekey(struct _select_def *conf, int *command)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    switch (*command) {
		case 'j':
		{
			struct fileheader * currfh;
			int ent;
			*command = KEY_DOWN;

			if(arg->mode == DIR_MODE_DIGEST && HAS_PERM(getCurrentUser(),PERM_SYSOP)) {
    			if (conf->pos<=0)
					return SHOW_CONTINUE;
		        currfh=(struct fileheader*)(arg->data+(conf->pos - conf->page_pos) * arg->ssize);
				if (currfh->id <= 0)
					return SHOW_CONTINUE;
				ent = get_ent_from_id(DIR_MODE_NORMAL, currfh->id, arg->board->filename);
				if(ent <= 0){
                	del_post(conf,currfh,(void*)(ARG_NOPROMPT_FLAG)) ;
				}
			}
			return SHOW_CONTINUE;
		}
		case 'q':
		{
			struct fileheader * currfh;
			int ent;
			*command = KEY_LEFT;
			if(arg->mode == DIR_MODE_DIGEST ||
			   arg->mode == DIR_MODE_THREAD ||
			   arg->mode == DIR_MODE_MARK ||
			   arg->mode == DIR_MODE_ORIGIN ||
			   arg->mode == DIR_MODE_AUTHOR ||
			   arg->mode == DIR_MODE_TITLE ||
			   arg->mode == DIR_MODE_SUPERFITER){
    			if (conf->pos<=0)
					return SHOW_CONTINUE;
		        currfh=(struct fileheader*)(arg->data+(conf->pos - conf->page_pos) * arg->ssize);
				if (currfh->id <= 0)
					return SHOW_CONTINUE;
				ent = get_ent_from_id(DIR_MODE_NORMAL, currfh->id, arg->board->filename);
				if(ent <= 0)
					return SHOW_CONTINUE;
                savePos(DIR_MODE_NORMAL,NULL,ent,arg->board);
			}
			return SHOW_CONTINUE;
		}
        case KEY_END:
        case '$':
            conf->new_pos=arg->filecount;
            *command=KEY_INVALID;
            return SHOW_SELCHANGE;
        case 'P': //翻页控制使用arg->filecount，不到达置顶
        case KEY_PGUP:
        case Ctrl('B'):
  	    if (conf->pos-conf->item_per_page>0)
    	        conf->new_pos= conf->page_pos - conf->item_per_page;
	    else
    		conf->new_pos=1;
            *command=KEY_INVALID;
            return SHOW_SELCHANGE;
        case KEY_PGDN:
        case 'N':
        case Ctrl('F'):
        case ' ':
  	    if (conf->page_pos+conf->item_per_page<=conf->item_count)
    	        conf->new_pos= conf->page_pos + conf->item_per_page;
	    else
    		conf->new_pos=conf->item_count;
        *command=KEY_INVALID;
        return SHOW_SELCHANGE;
    }
    return SHOW_CONTINUE;
}

static int read_show(struct _select_def *conf, int pos)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    char foroutbuf[512];
    prints("%s",(*arg->doentry) (foroutbuf, pos, arg->data+(pos-conf->page_pos) * arg->ssize,arg->readdata,conf));
    clrtoeol();
    return SHOW_CONTINUE;
}

static int read_onsize(struct _select_def* conf)
{
    int i;
    struct read_arg *arg = (struct read_arg *) conf->arg;
    int per_page=conf->item_per_page;
    conf->item_per_page = TDEFINE(TDEF_SPLITSCREEN)?BBS_PAGESIZE/2:BBS_PAGESIZE;
    if (conf->item_pos!=NULL)
        free(conf->item_pos);
    conf->item_pos = (POINT *) calloc(conf->item_per_page,sizeof(POINT));

    for (i = 0; i < conf->item_per_page; i++) {
        conf->item_pos[i].x = 1;
        conf->item_pos[i].y = i + 3;
    };
    if (per_page!=conf->item_per_page) {
    if (arg->data!=NULL) {
        free(arg->data);
        arg->data=NULL;
    }
    if (TDEFINE(TDEF_SPLITSCREEN))
        conf->on_selchange=read_showcontent;
    else conf->on_selchange=NULL;
    return SHOW_DIRCHANGE;
    }
    return DONOTHING;
}

int new_i_read(enum BBS_DIR_MODE cmdmode, char *direct, void (*dotitle) (struct _select_def*), READ_ENT_FUNC doentry, struct key_command *rcmdlist, int ssize)
{
    struct _select_def read_conf;
    struct read_arg arg;
    int lastpos;
    const static struct key_translate ktab[]= {
            {'\n','r'},
            {'\r','r'},
            {KEY_RIGHT,'r'},
            {'$',KEY_END},
//            {'q',KEY_LEFT},
            {'e',KEY_LEFT},
            {'k',KEY_UP},
//            {'j',KEY_DOWN},
            {-1,-1}
    };


    read_setusermode(cmdmode);
    lastpos=getPos(cmdmode,direct,currboard);
    /* save argument */
    bzero(&arg,sizeof(struct read_arg));
    arg.mode=cmdmode;
    arg.direct=direct;
    arg.dotitle=dotitle;
    arg.doentry=doentry;
    arg.rcmdlist=rcmdlist;
    arg.ssize=ssize;
    arg.readmode=READ_NORMAL;
    arg.data=NULL;
    arg.readdata=NULL;
    arg.returnvalue=QUIT;
    arg.writearg=NULL;
    arg.board=currboard;
    arg.bid=getboardnum(currboard->filename, NULL);
    arg.boardstatus=getbstatus(arg.bid);
    if ((arg.mode==DIR_MODE_NORMAL)||
        ((arg.mode>=DIR_MODE_THREAD)&&(arg.mode<=DIR_MODE_WEB_THREAD))) {
        char ding_direct[PATHLEN];
        //设置置顶的.DIR direct TODO:用tmpfs
        sprintf(ding_direct,"boards/%s/%s",currboard->filename,DING_DIR);
        arg.dingdirect=malloc(strlen(ding_direct)+1);
        strcpy(arg.dingdirect,ding_direct);
    } else arg.dingdirect=NULL;

    clear();

    if ((arg.fd = open(arg.direct, O_RDWR, 0)) != -1) {
        bzero((char *) &read_conf, sizeof(struct _select_def));
        read_conf.item_per_page = TDEFINE(TDEF_SPLITSCREEN)?BBS_PAGESIZE/2:BBS_PAGESIZE;
        read_conf.flag = LF_NUMSEL | LF_VSCROLL | LF_BELL | LF_MULTIPAGE;     /*|LF_HILIGHTSEL;*/
        if (DEFINE(getCurrentUser(), DEF_CIRCLE))
            read_conf.flag |= LF_LOOP ;
        read_conf.prompt = ">";
        read_conf.arg = &arg;
        read_conf.title_pos.x = 0;
        read_conf.title_pos.y = 0;

        read_conf.get_data = read_getdata;

        read_conf.on_select = read_onselect;
        read_conf.show_data = read_show;
        read_conf.pre_key_command = read_prekey;
        read_conf.key_command = read_key;
        read_conf.show_title = read_title;
        read_conf.show_endline= read_endline;
        read_conf.on_size= read_onsize;
        read_conf.key_table = (struct key_translate *)ktab;

        read_getdata(&read_conf,-1,read_conf.item_per_page);
        if (TDEFINE(TDEF_SPLITSCREEN))
            read_conf.on_selchange= read_showcontent;
        if ((lastpos!=0)&&(lastpos<arg.filecount))
            read_conf.pos = lastpos;
        else {
            read_conf.pos = arg.filecount; 
        }
        read_conf.page_pos = ((read_conf.pos-1)/read_conf.item_per_page)*read_conf.item_per_page+1;

        list_select_loop(&read_conf);
        close(arg.fd);
        savePos(arg.mode,direct,read_conf.pos,arg.board);
        if (read_conf.item_pos!=NULL)
           free(read_conf.item_pos);
    } else {

       if (cmdmode == DIR_MODE_MAIL) {
            prints("没有任何信件...");
            pressreturn();
            clear();
        }

        else if (cmdmode == DIR_MODE_FRIEND) {
            getdata(t_lines - 1, 0, "没有任何好友 (A)新增好友 (Q)离开？[Q] ", genbuf, 4, DOECHO, NULL, true);
            if (genbuf[0] == 'a' || genbuf[0] == 'A')
                friend_add();
        }

        else {
            getdata(t_lines - 1, 0, "新版刚成立 (P)发表文章 (Q)离开？[Q] ", genbuf, 4, DOECHO, NULL, true);
            if (genbuf[0] == 'p' || genbuf[0] == 'P')
                do_post(NULL, NULL, NULL);
        }
        pressreturn();
        clear();
    }
    if (arg.data!=NULL)
        free(arg.data);    
    if (arg.readdata!=NULL)
        free(arg.readdata);
    if (arg.dingdirect!=NULL)
        free(arg.dingdirect);
    if (cmdmode!=arg.mode)
        arg.returnvalue=CHANGEMODE;
    return arg.returnvalue;
}


/* TODO: 滤掉附件... - atppp */
static int searchpattern(char *filename, char *query, int qlen)
{
    char *ptr;
    off_t size;
    int ret=0;

    BBS_TRY {
        if (safe_mmapfile(filename, O_RDONLY, PROT_READ, MAP_SHARED, &ptr, &size, NULL) == 0)
            BBS_RETURN(0);
        if (memmem(ptr, size, query, qlen)) ret=1;
    }
    BBS_CATCH {
    }
    BBS_END end_mmapfile((void *) ptr, size, -1);

    return ret;
}

static void get_upper_str(char *ptr2, char *ptr1)
{
    int ln, i;

    for (ln = 0; (ln < STRLEN) && (ptr1[ln] != 0); ln++);
    for (i = 0; i < ln; i++) {
        ptr2[i] = toupper(ptr1[i]);
        /******** 下面为Luzi添加 ************/
        if (ptr2[i] == '\0')
            ptr2[i] = '\1';
        /******** 以上为Luzi添加 ************/
    }
    ptr2[ln] = '\0';
}

/* COMMAN : use mmap to speed up searching */
/* add by stiger
 * return :   2 :  DIRCHANGED
 *            1 :  FULLUPDATE
 *            0 :  DONOTHING
 */
static int read_search_articles(struct _select_def* conf, char *query, bool up, int aflag)
{
    char ptr[STRLEN];
    int now, match = 0;
    char upper_query[STRLEN];
    int init;
    size_t bm_search[256];
    int qlen = strlen(query);

    char *data;
    struct fileheader *pFh, *pFh1;
    off_t size;
    struct read_arg *arg = (struct read_arg *) conf->arg;

    get_upper_str(upper_query, query);
    if (*query == '\0') {
        return 0;
    }

    /*
     * move(t_lines-1,0);
     * clrtoeol();
     * prints("\033[44m\033[33m搜寻中，请稍候....                                                             \033[m");
     */
    init=false;
    now = conf->pos;

/*    refresh();*/
    match = 0;
    BBS_TRY {
        if (safe_mmapfile_handle(arg->fd, PROT_READ, MAP_SHARED, &data, &size) == 0)
            BBS_RETURN(0);
        pFh = (struct fileheader*)data;
        arg->filecount=size/sizeof(struct fileheader);
        if(now > arg->filecount){
	/*在置顶文章前搜索*/
            now = arg->filecount;
        }
        if (now <= arg->filecount) {
            pFh1 = pFh + now - 1;
            while (1) {
                if (!up) {
                    if (++now > arg->filecount)
                        break;
                    pFh1++;
                } else {
                    if (--now < 1)
                        break;
                    pFh1--;
                }
                if (now > arg->filecount)
                    break;
                if (aflag == -1) { /*内容检索*/
                    char p_name[256];

                    if (arg->mode!=DIR_MODE_MAIL)
                        setbfile(p_name, currboard->filename, pFh1->filename);
                    else
                        setmailfile(p_name, getCurrentUser()->userid, pFh1->filename);
                    if (searchpattern(p_name, query, qlen)) {
                        match = 1;
                        break;
                    } else
                        continue;
                }
                if (aflag == 0) {
                    char *ptr2;
                    strncpy(ptr, pFh1->title, ARTICLE_TITLE_LEN - 1);
                    ptr[ARTICLE_TITLE_LEN - 1] = 0;
                    ptr2 = ptr;

                    if ((*ptr == 'R' || *ptr == 'r')

                        && (*(ptr + 1) == 'E' || *(ptr + 1) == 'e') && (*(ptr + 2) == ':')
                        && (*(ptr + 3) == ' ')) {
                        ptr2 = ptr + 4;
                    }
                    if (bm_strcasestr_rp(ptr2,query,bm_search,&init)) {
                        match = 1;
                        break;
                    }
                } else {
                    strncpy(ptr, pFh1->owner, STRLEN - 1);
                    ptr[STRLEN - 1] = 0;
                    /*
                     * 同作者查询改成完全匹配 by dong, 1998.9.12 
                     */
                    if (!strcasecmp(ptr, upper_query)) {
                        match = 1;
                        break;
                    }
                }
            }
        }
    }
    BBS_CATCH {
        match = 0;
    }
    BBS_END
    end_mmapfile(data, size, -1);
    move(t_lines - 1, 0);
    clrtoeol();
    if(match) {
        conf->new_pos=now;
        return 1;
    }
    return 0;
}

int super_filter(struct _select_def* conf,struct fileheader* curfh,void* extraarg)
{
    extern int scr_cols;
    struct read_arg* arg=(struct read_arg*)conf->arg;
    int count;
    char newdirect[PATHLEN];
    struct fileheader dummy_curfh;
    struct super_filter_query_arg q_arg;

    static char query[180]="";

    if (!strcmp(getCurrentUser()->userid, "guest")) {
        return FULLUPDATE;
    }
    
    clear();
    prints("                  超强文章选择\n\n");
    move(5,0);
    prints("变量: no(文章号) m(保留) g(文摘) b(m&&g) noreply(不可回复) sign(标记)\n"
           "      del(删除) \033[1;31ma(附件)\033[0m unread(未读) percent(百分号)\n"
           "      title(标题) author(作者)\n"
           "函数: sub(s1,s2)第一个字符串在第二个中的位置,如果不存在返回0\n"
           "      len(s)字符串长度\n"
           "举例: 我要查询所有bad写的标记是b的文章:\n"
           "              author=='bad'&&b                   作者是bad且b\n"
           "      我要查询所有不可回复并且未读的文章:\n"
           "              noreply&&unread                    不可回复且未读\n"
           "      我要查询所有1000-2000范围内带附件的文章:\n"
           "              (no>=1000)&&(no<=2000)&&\033[1;31ma\033[0m          文章号大等于1000 且 文章号小等于2000 且 附件\n"
           "      我要查询标题长度在5-10之间的文章:\n"
           "              len(title)>=5&&len(title)<=10      标题的长度大等于5 且 标题的长度小等于10\n"
           "      我要查询标题里含有faint的文章:\n"
           "              sub('faint',title)                 标题包含faint\n"
           "      我要查询标题里包含hehe并且位置在最后的文章:\n"
           "              sub('hehe',title)==len(title)-3    标题包含hehe是标题的长度减3\n"
           "      我要查询......自己动手查吧,hehe"
    );
    multi_getdata(2, 0, scr_cols-1, "请输入表达式: ", query, sizeof(query), 20, 0, 0);
    if(!query[0]) 
        return FULLUPDATE;
    if (arg->mode==DIR_MODE_AUTHOR||arg->mode==DIR_MODE_TITLE) {
        unlink(arg->direct);
    }
#if 0
    newbbslog(BBSLOG_USER, "SUPER_FILTER %s", query);
#endif
    setbdir(DIR_MODE_SUPERFITER, newdirect, currboard->filename);

    q_arg.array = NULL;
    q_arg.array_size = 0;
    q_arg.boardname = currboard->filename;
    if (curfh == NULL) {
        memset(&dummy_curfh, 0, sizeof(dummy_curfh));
        q_arg.curfh = &dummy_curfh;
    } else {
        q_arg.curfh = curfh;
    }
    q_arg.isbm = chk_currBM(currBM, getCurrentUser());
    q_arg.write_file = newdirect;
    q_arg.query = query;

    count = query_super_filter(-1, &q_arg);

    if (count == -1) {
        return FULLUPDATE;
    } else if (count < 0) {
        move(3, 0);
        clrtoeol();
        prints("表达式错误 [%d]", -count);
        refresh();
        sleep(1);
        return FULLUPDATE;
    }
    else if(count==0) {
        move(3, 0);
        clrtoeol();
        prints("一个都没有找到....");
        refresh();
        sleep(1);
        return FULLUPDATE;
    }
/*    else if (chk_currBM(currBM, getCurrentUser())) {
        char ans[4];
        int i,j,k;
        int fflag;
        int y,x;
        move(3, 0);
        clrtoeol();
        prints("找到 %d 篇文章(0-退出, 1-保留标记m, 2-删除标记t, 3-不可回复标记;) [0]", count);
        getyx(&y, &x);
        getdata(y, x, 0, ans, 3, 1, 0, 1);
        if(ans[0]>='1'&&ans[0]<='3') {
            struct fileheader f;
            k=ans[0]-'0';
            if(ans[0]=='1') fflag=FILE_MARK_FLAG;
            else if(ans[0]=='2') fflag=FILE_DELETE_FLAG;
            else if(ans[0]=='3') fflag=FILE_NOREPLY_FLAG;
            for(i=0;i<count;i++)
                change_post_flag(currBM, getCurrentUser(), digestmode, currboard, i+1, &f, currdirect, fflag, 0);
        }
    }*/
    strcpy(arg->direct, newdirect);
    arg->newmode=DIR_MODE_SUPERFITER;
    return NEWDIRECT;
}

static int jumpSuperFilter(struct _select_def* conf,struct fileheader *fileinfo, bool down, char* query)
{
    int now; // 1-based
    off_t size;
    struct read_arg *arg = (struct read_arg *) conf->arg;
    char *data;
    struct fileheader *pFh;
    struct fileheader_num fhn;
    int count;
    struct super_filter_query_arg q_arg;

    //if (arg->mode != DIR_MODE_NORMAL) return DONOTHING;
    if (*query == '\0') return DONOTHING;

    BBS_TRY {
        if (safe_mmapfile_handle(arg->fd, PROT_READ, MAP_SHARED, &data, &size) == 0)
            BBS_RETURN(0);
        pFh = (struct fileheader*)data;
        arg->filecount = size/sizeof(struct fileheader);
        now = conf->pos;

        q_arg.array = &fhn;
        q_arg.array_size = 1;
        q_arg.boardname = currboard->filename;
        q_arg.curfh = &pFh[now - 1];
        q_arg.detectmore = false;
        q_arg.isbm = chk_currBM(currBM, getCurrentUser());
        q_arg.query = query;
        q_arg.write_file = NULL;
        count = query_super_filter_mmap(pFh, now + (down ? 0 : -2), arg->filecount, down, &q_arg);
        if (count == 1) {
            now = fhn.num;
        } else {
            now = -1;
            if (count < 0) {
                move(t_lines - 1, 0);
                prints("表达式错误 [%d]", -count);
                clrtoeol();
                refresh();
                sleep(1);
            }
        }
    }
    BBS_CATCH {
        now = -1;
    }
    BBS_END
    end_mmapfile(data, size, -1);
    move(t_lines - 1, 0);
    clrtoeol();
    if(now > 0) {
        conf->new_pos = now;
        return SELCHANGE;
    }
    return DONOTHING;
}

int post_search(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    static char query[STRLEN];
    char ans[STRLEN], pmt[STRLEN];
    bool up=(bool)extraarg;

    strncpy(ans, query, STRLEN);
    snprintf(pmt, STRLEN, "%s搜寻内容: ", up ? "↑" : "↓");
    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, pmt, ans, STRLEN - 1, DOECHO, NULL, false);
    if (ans[0] != '\0') {
        strncpy(query, ans, STRLEN);
        if (query[0] == '$') {
            int ret = jumpSuperFilter(conf, fh, !up, query + 1);
            if (ret != DONOTHING)
                return ret;
        } else {
            if (read_search_articles(conf, query, up, -1) == 1)
			{
				conf->show_endline(conf);	/* add by pig2532 on 2005.12.4 */
			    return SELCHANGE;
			}
        }
    }
    conf->show_endline(conf);
    return DONOTHING;
}

int auth_search(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    static char author[IDLEN + 1];
    char ans[IDLEN + 1], pmt[STRLEN];
    char currauth[STRLEN];
    bool up=(bool)extraarg;
    
    if (fh==NULL) return DONOTHING;
    strncpy(currauth, fh->owner, STRLEN);
    snprintf(pmt, STRLEN, "%s的文章搜寻作者 [%s]: ", up ? "往先前" : "往后来", currauth);
    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, pmt, ans, IDLEN + 1, DOECHO, NULL, true);   /*Haohmaru.98.09.29.修正作者查找只能11位ID的错误 */
    if (ans[0] != '\0')
        strncpy(author, ans, IDLEN);

    else
        strcpy(author, currauth);
    switch (read_search_articles(conf, author, up, 1)) {
        case 1:
			conf->show_endline(conf);	/* add by pig2532 on 2005.12.4 */
            return SELCHANGE;
        default:
            conf->show_endline(conf);
    }
    return DONOTHING;
}

int title_search(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    static char title[STRLEN];
    char ans[STRLEN], pmt[STRLEN];
    bool up=(bool)extraarg;

    strncpy(ans, title, STRLEN);
    snprintf(pmt, STRLEN, "%s搜寻标题: ", up ? "↑" : "↓");
    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, pmt, ans, STRLEN - 1, DOECHO, NULL, false);
    if (*ans != '\0') {
        strcpy(title, ans);
        if (read_search_articles(conf, title, up, 0) == 1)
		{
			conf->show_endline(conf);	/* add by pig2532 on 2005.12.4 */
            return SELCHANGE;
		}
    }
    conf->show_endline(conf);
    return DONOTHING;
}

bool isThreadTitle(char* a,char* b)
{
    if (!strncasecmp(a,"re: ",4)) a+=4;
    if (!strncasecmp(b,"re: ",4)) b+=4;
    if (!strncmp(a, "├ ", 3)) a+=3;
    if (!strncmp(b, "├ ", 3)) b+=3;
    if (!strncmp(a, "└ ", 3)) a+=3;
    if (!strncmp(b, "└ ", 3)) b+=3;
    return strcmp(a,b)?0:1;
}

int apply_thread(struct _select_def* conf, struct fileheader* fh,APPLY_THREAD_FUNC func,bool applycurrent, bool down,void* arg)
{
    char *data;
    struct fileheader *pFh,*nowFh;
    off_t size;
    int now; /*当前扫描到的位置*/
    int count; /*计数器*/
    int recordcount; /*文章总数*/
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    /*是否需要flock,这个有个关键是如果lock中间有提示用户做什么
      的,就会导致死锁*/
    count=0;
    now = conf->pos;
    BBS_TRY {
        if (safe_mmapfile_handle(read_arg->fd, PROT_READ|PROT_WRITE, MAP_SHARED, &data, &size) ) {
            bool needmove;
            pFh = (struct fileheader*)data;
            recordcount=size/sizeof(struct fileheader);
            if (now>recordcount)
                /*在置顶文章前搜索*/
                now=recordcount;
            nowFh=pFh+now-1;
            needmove=!applycurrent;
            while (1) {
                int ret;
                /* 移动指针*/
                if (needmove) {
                    if (down) {
                        if (++now > recordcount)
                            break;
                        nowFh++;
                    } else {
                        if (--now < 1)
                            break;
                        nowFh--;
                    }
                }
                needmove=true;
                
                /* 判断是不是同一主题,不是直接continue*/
                if ((read_arg->mode==DIR_MODE_NORMAL)||
                     ((read_arg->mode>=DIR_MODE_THREAD)&&(read_arg->mode>=DIR_MODE_WEB_THREAD)))
                { /*使用groupid*/
                    if (fh->groupid!=nowFh->groupid)
                    continue;
                } else {
                    if (!isThreadTitle(fh->title,nowFh->title))
                        continue;
                }

                /* 是同一主题*/
                count++;
                if (func) {
                    ret=(*func)(conf,nowFh,now,arg);
                    if (ret==APPLY_QUIT) break;

                    /*在返回APPLY_REAPPLY的时候不需要移动指针*/
                    if (ret==APPLY_REAPPLY) {
                        recordcount--;
                        read_arg->filecount--;
                        needmove=false;
                    } else 
                        needmove=true;
                }
            }
        }
    }
    BBS_CATCH {
    }
    BBS_END;
    if (data!=MAP_FAILED)
        end_mmapfile(data, size, -1);
    return count;
}

int thread_read(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    POINTDIFF mode=(POINTDIFF)extraarg;
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    conf->new_pos=0;
    read_arg->oldpos=0;
    switch (mode) {
        case SR_READX:
            read_arg->oldpos=conf->pos;
        case SR_READ:
            read_arg->readmode=READ_THREAD;
            list_select_add_key(conf, 'r');
            return DONOTHING;
        case SR_FIRST:
        case SR_PREV:
            apply_thread(conf,fh,fileheader_thread_read,false,false,(void*)mode);
            break;
        case SR_LAST:
        case SR_NEXT:
            apply_thread(conf,fh,fileheader_thread_read,false,true,(void*)mode);
            break;
        case SR_FIRSTNEW:
            apply_thread(conf,fh,fileheader_thread_read,false,false,(void*)mode);
            if (conf->new_pos==0) {
                apply_thread(conf,fh,fileheader_thread_read,false,true,(void*)SR_FIRSTNEWDOWNSEARCH);
            }
            break;
    }
    if (conf->new_pos==0) return DONOTHING;
    if (mode==SR_FIRSTNEW) {
        if (conf->new_pos!=0) {
            list_select_add_key(conf,'r'); //SEL change的下一条指令是read
            read_arg->readmode=READ_THREAD;
        }
    }
    return SELCHANGE;
}

int author_read(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    POINTDIFF mode=(POINTDIFF)extraarg;
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    conf->new_pos=0;
    read_arg->oldpos=0;
    switch (mode) {
        case SR_READX:
            read_arg->oldpos=conf->pos;
        case SR_READ:
            read_arg->readmode=READ_AUTHOR;
            list_select_add_key(conf, 'r');
            return DONOTHING;
    }
    return SELCHANGE;
}

int read_sendmsgtoauthor(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    struct user_info *uin;

    if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
        return DONOTHING;
    clear();
    uin = (struct user_info *) t_search(fh->owner, false);
    if (!uin || !canmsg(getCurrentUser(), uin))
        do_sendmsg(NULL, NULL, 0);

    else {
        strncpy(getSession()->MsgDesUid, uin->userid, 20);
        do_sendmsg(uin, NULL, 0);
    }
    return FULLUPDATE;
}


int read_showauthor(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    t_query(fh->owner);
    return FULLUPDATE;
}

/*直接查作者资料*/
int read_showauthorinfo(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    struct userec uinfo;
    struct userec *lookupuser;
    int id;

    if (!HAS_PERM(getCurrentUser(), PERM_ADMIN)) {
        clear();
        move(2,0);
        prints("全文链接快捷键改拉 ! 现在用 Ctrl+Q ，谢谢适应 !");
        pressanykey();
        return FULLUPDATE;
    } else {
        if (0 == (id = getuser(fh->owner, &lookupuser))) {
            move(2, 0);
            prints("不正确的使用者代号");
            clrtoeol();
            return PARTUPDATE;
        }
        uinfo = *lookupuser;
        move(1, 0);
        clrtobot();
        disply_userinfo(&uinfo, 1);
        uinfo_query(&uinfo, 1, id);
    }
    return FULLUPDATE;
}

/* etnlegend, 2005.10.16, 查询版主更新 */

int read_showauthorBM(struct _select_def *conf,struct fileheader *fh,void *arg){
    if(!HAS_PERM(getCurrentUser(),PERM_SYSOP))
        return DONOTHING;
    return query_bm_core(fh->owner,false);
}
/* END - etnlegend, 2005.10.16, 查询版主更新 */

int read_addauthorfriend(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    if (!strcmp("guest", getCurrentUser()->userid))
        return DONOTHING;;

    clear();
    addtooverride(fh->owner);
    return FULLUPDATE;
}

extern int zsend_file(char *filename, char *title);
int read_zsend(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    char *t;
    char buf1[512];
    strcpy(buf1, read_getcurrdirect(conf));
    if ((t = strrchr(buf1, '/')) != NULL)
        *t = '\0';
    snprintf(genbuf, 512, "%s/%s", buf1, fh->filename);
    zsend_file(genbuf, fh->title);
    return FULLUPDATE;
}

#ifdef PERSONAL_CORP
int read_importpc(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    return import_to_pc(conf->pos, fh, read_arg->direct);
}
#endif

char* read_getcurrdirect(struct _select_def* conf)
{
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    if ((conf->pos>read_arg->filecount)&&(read_arg->dingdirect!=NULL)) {
        return read_arg->dingdirect;
    }
    return read_arg->direct;
}

void setreadpost(struct _select_def* conf,struct fileheader* fh)
{
    struct read_arg* arg=conf->arg;
    if (arg->readdata==NULL)
        arg->readdata=malloc(sizeof(struct fileheader));
    memcpy(arg->readdata,fh,sizeof(struct fileheader));
}


int read_splitscreen(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    if (TDEFINE(TDEF_SPLITSCREEN))
        tmpuser&=~TDEF_SPLITSCREEN;
    else
        tmpuser|=TDEF_SPLITSCREEN;
    list_select_add_key(conf, KEY_ONSIZE);
    return DONOTHING;
}

int read_callfunc0(struct _select_def* conf, void* data, void* extraarg)
{
    return (*(int(*)())extraarg)();
}
