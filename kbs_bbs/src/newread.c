/**
  *  ÐÂµÄread.c£¬Ê¹ÓÃselect½á¹¹ÖØÐ´Ô­À´µÄi_read
  * ÏÈÊµÏÖmail
  */
#include "bbs.h"
#include "read.h"

extern char MsgDesUid[14];

/*ÓÃÓÚapply_recordµÄ»Øµ÷º¯Êý*/
static int fileheader_thread_read(struct _select_def* conf, struct fileheader* fh,int ent, void* extraarg)
{
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    int mode=(int)extraarg;
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
            if (brc_unread(fh->id)) {
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
            return APPLY_CONTINUE;/*¼ÌÐø²éµ½µ×*/
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
            if (!HAS_PERM(currentuser, PERM_PAGE))
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
        case 'O':
        case 'o':                  /* Luzi 1997.10.31 */
            {                       /* Leeward 98.10.26 fix a bug by saving old mode */
                int savemode = uinfo.mode;

                if (!HAS_PERM(currentuser, PERM_BASIC))
                    break;
                t_friends();
                modify_user_mode(savemode);
                mode=FULLUPDATE;
                break;
            }
    }
    for (i = 0; arg->rcmdlist[i].fptr != NULL; i++) {
        if (arg->rcmdlist[i].key == command) {
            mode = (*(arg->rcmdlist[i].fptr)) (conf, arg->data+(conf->pos - conf->page_pos) * arg->ssize, arg->rcmdlist[i].arg);
            break;
        }
    }
    switch (mode) {
        case FULLUPDATE:
        case PARTUPDATE:
            clear();
            ret=SHOW_REFRESH;
            break;
        case DIRCHANGED:
        case NEWDIRECT:
            ret=SHOW_DIRCHANGE;
            break;
        case DOQUIT:
        case CHANGEMODE:
            ret=SHOW_QUIT;
            break;
        case READ_NEXT:
            if (arg->readmode==READ_NORMAL) {
                if (conf->pos<conf->item_count) {
                    conf->new_pos = conf->pos + 1;
                    list_select_add_key(conf,'r'); //SEL changeµÄÏÂÒ»ÌõÖ¸ÁîÊÇread
                    ret=SHOW_SELCHANGE;
                } else ret=SHOW_REFRESH;
            } else  { /* ´¦ÀíÍ¬Ö÷ÌâÔÄ¶Á*/
                int findthread=apply_thread(conf,
                    arg->data+(conf->pos - conf->page_pos) * arg->ssize,
                    fileheader_thread_read,
                    true,
                    (void*)SR_NEXT);
                if (findthread!=0) {
                    list_select_add_key(conf,'r'); //SEL changeµÄÏÂÒ»ÌõÖ¸ÁîÊÇread
                    ret=SHOW_SELCHANGE;
                }
                else ret=SHOW_REFRESH;
            }
            break;
        case READ_PREV:
            if (arg->readmode==READ_NORMAL) {
                if (conf->pos>1) {
                    conf->new_pos = conf->pos - 1;
                    list_select_add_key(conf,'r'); //SEL changeµÄÏÂÒ»ÌõÖ¸ÁîÊÇread
                    ret=SHOW_SELCHANGE;
                } else ret= SHOW_REFRESH;
            } else { /* ´¦ÀíÍ¬Ö÷ÌâÔÄ¶Á*/
                int findthread=apply_thread(conf,
                    arg->data+(conf->pos - conf->page_pos) * arg->ssize,
                    fileheader_thread_read,
                    false,
                    (void*)SR_PREV);
                if (findthread!=0) {
                    list_select_add_key(conf,'r'); //SEL changeµÄÏÂÒ»ÌõÖ¸ÁîÊÇread
                    ret=SHOW_SELCHANGE;
                }
                else ret=SHOW_REFRESH;
            }
            break;
        case SELCHANGE:
            ret=SHOW_SELCHANGE;
            break;
    } 
    if (ret!=SHOW_SELCHANGE) /*·µ»Ø·ÇË³ÐòÔÄ¶ÁÄ£Ê½*/
        arg->readmode=READ_NORMAL;
    return ret;
}

static int read_onselect(struct _select_def *conf)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    return SHOW_CONTINUE;
}

static int read_getdata(struct _select_def *conf, int pos, int len)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    int n;
    struct stat st;
    int entry=0;
    int count;

    if (arg->data==NULL)
        arg->data=calloc(BBS_PAGESIZE,arg->ssize);
    
    if (fstat(arg->fd,&st)!=-1) {
        count=st.st_size/arg->ssize;
        if (count!=conf->item_count) {
            //need refresh
            conf->item_count=count;
            arg->filecount=count;//TODO,ÖÃ¶¥µÄÊ±ºò£¬filecountºÍitem_count²»Ò»Ñù£¬ÉÙ¸öÖÃ¶¥
            return SHOW_DIRCHANGE;
        }
        
        if (lseek(arg->fd, arg->ssize * (pos - 1), SEEK_SET) != -1) {
            if ((n = read(arg->fd, arg->data, arg->ssize * len)) != -1) {
                entry=(n / arg->ssize);
            }
        }
    } else
        return SHOW_QUIT;
    return SHOW_CONTINUE;
}

static int read_title(struct _select_def *conf)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    clear();
    (*arg->dotitle) ();
    return SHOW_CONTINUE;
}

static int read_endline(struct _select_def *conf)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;

    if (!conf->tmpnum)
        update_endline();
    else if (DEFINE(currentuser, DEF_ENDLINE)) {
        extern time_t login_start_time;
        int allstay;
        char pntbuf[256], nullbuf[2] = " ";
        char lbuf[11];

        snprintf(lbuf,11,"%d",conf->tmpnum);

        allstay = (time(0) - login_start_time) / 60;
        snprintf(pntbuf, 256, "\033[33;44m×ªµ½¡Ã[\033[36m%9.9s\033[33m]" "  ºô½ÐÆ÷[ºÃÓÑ:%3s¡ÃÒ»°ã:%3s] Ê¹ÓÃÕß[\033[36m%.12s\033[33m]%sÍ£Áô[%3d:%2d]\033[m", 
            lbuf, (!(uinfo.pager & FRIEND_PAGER)) ? "NO " : "YES", (uinfo.pager & ALL_PAGER) ? "YES" : "NO ", currentuser->userid,      /*13-strlen(currentuser->userid)
                                                                                                                                                                                                                                                                                         * TODO:Õâ¸öµØ·½ÓÐÎÊÌâ£¬ËûÏë¶ÔÆë£¬µ«ÊÇ´úÂë²»¶Ô
                                                                                                                                                                                                                                                                                         * , */ nullbuf,
                 (allstay / 60) % 1000, allstay % 60);
        move(t_lines - 1, 0);
        prints(pntbuf);
        clrtoeol();
    }
    return SHOW_CONTINUE;
}

static int read_prekey(struct _select_def *conf, int *command)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    return SHOW_CONTINUE;
}

static int read_show(struct _select_def *conf, int pos)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    char foroutbuf[512];
    prints("%s",(*arg->doentry) (foroutbuf, pos, arg->data+(pos-conf->page_pos) * arg->ssize));
    clrtoeol();
    return SHOW_CONTINUE;
}

static int read_onsize(struct _select_def* conf)
{
    int i;
    struct read_arg *arg = (struct read_arg *) conf->arg;
    if (conf->item_pos!=NULL)
        free(conf->item_pos);
    conf->item_pos = (POINT *) calloc(BBS_PAGESIZE,sizeof(POINT));

    for (i = 0; i < BBS_PAGESIZE; i++) {
        conf->item_pos[i].x = 1;
        conf->item_pos[i].y = i + 3;
    };
    if (arg->data!=NULL) {
        free(arg->data);
        arg->data=NULL;
    }
    conf->item_per_page = BBS_PAGESIZE;
    return SHOW_DIRCHANGE;
}

int new_i_read(enum BBS_DIR_MODE cmdmode, char *direct, void (*dotitle) (), READ_FUNC doentry, struct key_command *rcmdlist, int ssize)
{
    struct _select_def read_conf;
    struct read_arg arg;
    int i;
    const static struct key_translate ktab[]= {
            {'\n','r'},
            {'\r','r'},
            {KEY_RIGHT,'r'},
            {'$',KEY_END},
            {'q',KEY_LEFT},
            {'e',KEY_LEFT},
            {'k',KEY_UP},
            {'j',KEY_DOWN},
            {'N',KEY_PGDN},
            {Ctrl('F'),KEY_PGDN},
            {' ',KEY_PGDN},
            {'p',KEY_PGDN},
            {Ctrl('B'),KEY_PGUP},
            {-1,-1}
    };

    if (cmdmode==DIR_MODE_MAIL)
        modify_user_mode(RMAIL);
    else //todo: other mode
        modify_user_mode(READBRD);

    /* save argument */
    arg.mode=cmdmode;
    arg.direct=direct;
    arg.dotitle=dotitle;
    arg.doentry=doentry;
    arg.rcmdlist=rcmdlist;
    arg.ssize=ssize;
    arg.readmode=READ_NORMAL;
    arg.data=NULL;

    clear();

    if ((arg.fd = open(arg.direct, O_RDWR, 0)) != -1) {
        bzero((char *) &read_conf, sizeof(struct _select_def));
        read_conf.item_per_page = BBS_PAGESIZE;
        read_conf.flag = LF_NUMSEL | LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;     /*|LF_HILIGHTSEL;*/
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
        read_conf.key_table = &ktab[0];

        read_conf.pos = 1; //TODO: get last position
        read_getdata(&read_conf,read_conf.pos,read_conf.item_per_page);
        read_conf.pos = read_conf.item_count; //TODO: get last position
        read_conf.page_pos = ((read_conf.pos-1)/BBS_PAGESIZE)*BBS_PAGESIZE+1; //TODO

        list_select_loop(&read_conf);
        if (arg.data!=NULL)
            free(arg.data);    
        if (read_conf.item_pos!=NULL)
            free(read_conf.item_pos);
        close(arg.fd);
    } else {
            prints("Ã»ÓÐÈÎºÎÐÅ¼þ...");
            pressreturn();
            clear();
    }
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
    int complete_search;
    char upper_query[STRLEN];
    bool init;
    size_t bm_search[256];

/*	int mmap_offset,mmap_length; */
    struct fileheader *pFh, *pFh1;
    int size;
    struct read_arg *arg = (struct read_arg *) conf->arg;

    get_upper_str(upper_query, query);
    if (aflag >= 2) {
        complete_search = 1;
        aflag -= 2;
    } else {
        complete_search = 0;
    }
    if (*query == '\0') {
        return 0;
    }

    /*
     * move(t_lines-1,0);
     * clrtoeol();
     * prints("[44m[33mËÑÑ°ÖÐ£¬ÇëÉÔºò....                                                             [m");
     */
    init=false;
    now = conf->pos;

/*    refresh();*/
    match = 0;
    BBS_TRY {
        if (safe_mmapfile_handle(arg->fd, PROT_READ, MAP_SHARED, (void **) &pFh, &size) == 0)
            BBS_RETURN(0);
        if(now > arg->filecount){
	/*ÔÚÖÃ¶¥ÎÄÕÂÇ°ËÑË÷*/
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
                if (aflag == -1) { /*ÄÚÈÝ¼ìË÷*/
                    char p_name[256];

                    if (uinfo.mode != RMAIL)
                        setbfile(p_name, currboard->filename, pFh1->filename);
                    else
                        setmailfile(p_name, currentuser->userid, pFh1->filename);
                    if (searchpattern(p_name, query)) {
                        match = 1;
                        break;
                    } else
                        continue;
                }
                strncpy(ptr, aflag ? pFh1->owner : pFh1->title, STRLEN - 1);
                ptr[STRLEN - 1] = 0;
                if (complete_search == 1) {
                    char *ptr2 = ptr;

                    if ((*ptr == 'R' || *ptr == 'r')

                        && (*(ptr + 1) == 'E' || *(ptr + 1) == 'e') && (*(ptr + 2) == ':')
                        && (*(ptr + 3) == ' ')) {
                        ptr2 = ptr + 4;
                    }
                    if (!strcmp(ptr2, query)) {
                        match = 1;
                        break;
                    }
                } else {
                    /*
                     * Í¬×÷Õß²éÑ¯¸Ä³ÉÍêÈ«Æ¥Åä by dong, 1998.9.12 
                     */
                    if (aflag == 1) {   /* ½øÐÐÍ¬×÷Õß²éÑ¯ */
                        if (!strcasecmp(ptr, upper_query)) {
                            match = 1;
                            break;
                        }
                    }

                    else if (bm_strcasestr_rp(ptr, upper_query,bm_search,&init) != NULL) {
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
    end_mmapfile((void *) pFh, size, -1);
    move(t_lines - 1, 0);
    clrtoeol();
    if(match) {
        conf->pos=now;
        return 1;
    }
    return 0;
}


int auth_search(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    static char author[IDLEN + 1];
    char ans[IDLEN + 1], pmt[STRLEN];
    char currauth[STRLEN];
    bool up=(bool)extraarg;
    
    strncpy(currauth, fh->owner, STRLEN);
    snprintf(pmt, STRLEN, "%sµÄÎÄÕÂËÑÑ°×÷Õß [%s]: ", up ? "ÍùÏÈÇ°" : "ÍùºóÀ´", currauth);
    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, pmt, ans, IDLEN + 1, DOECHO, NULL, true);   /*Haohmaru.98.09.29.ÐÞÕý×÷Õß²éÕÒÖ»ÄÜ11Î»IDµÄ´íÎó */
    if (ans[0] != '\0')
        strncpy(author, ans, IDLEN);

    else
        strcpy(author, currauth);
    switch (read_search_articles(conf, author, up, 1)) {
        case 1:
            return PARTUPDATE;
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
    snprintf(pmt, STRLEN, "%sËÑÑ°±êÌâ [%s]: ", up ? "ÍùÇ°" : "Íùºó", ans);
    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, pmt, ans, STRLEN - 1, DOECHO, NULL, true);
    if (*ans != '\0')
        strcpy(title, ans);
    switch (read_search_articles(conf, title, up, 0)) {
        case 1:
            return PARTUPDATE;
        default:
            conf->show_endline(conf);
    }
    return DONOTHING;
}

bool isThreadTitle(char* a,char* b)
{
  if (!strncasecmp(a,"re: ",4)) a+=4;
  if (!strncasecmp(b,"re: ",4)) b+=4;
  return strcmp(a,b)?0:1;
}

int apply_thread(struct _select_def* conf, struct fileheader* fh,APPLY_THREAD_FUNC func, bool down,void* arg)
{
    struct fileheader *pFh,*nowFh;
    int size;
    int now; /*µ±Ç°É¨Ãèµ½µÄÎ»ÖÃ*/
    int count; /*¼ÆÊýÆ÷*/
    int recordcount; /*ÎÄÕÂ×ÜÊý*/
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    /*ÊÇ·ñÐèÒªflock,Õâ¸öÓÐ¸ö¹Ø¼üÊÇÈç¹ûlockÖÐ¼äÓÐÌáÊ¾ÓÃ»§×öÊ²Ã´
      µÄ,¾Í»áµ¼ÖÂËÀËø*/
    count=0;
    now = conf->pos;
    BBS_TRY {
        if (safe_mmapfile_handle(read_arg->fd, PROT_READ|PROT_WRITE, MAP_SHARED, (void **) &pFh, &size) ) {
            bool needmove;
            if(now > read_arg->filecount){
            /*ÔÚÖÃ¶¥ÎÄÕÂÇ°ËÑË÷*/
                now = read_arg->filecount;
            }
            recordcount=size/sizeof(struct fileheader);
            if (now>recordcount)
                now=recordcount;
            nowFh=pFh+now-1;
            needmove=true;
            while (1) {
                int ret;
                /* ÒÆ¶¯Ö¸Õë*/
                if (needmove) {
                    if (down) {
                        if (++now > read_arg->filecount)
                            break;
                        nowFh++;
                    } else {
                        if (--now < 1)
                            break;
                        nowFh--;
                    }
                }
                
                /* ÅÐ¶ÏÊÇ²»ÊÇÍ¬Ò»Ö÷Ìâ,²»ÊÇÖ±½Ócontinue*/
                if (read_arg->mode==DIR_MODE_NORMAL) { /*ÅÐ¶ÏÊÇ·ñÊ¹ÓÃgroupid*/
                    if (fh->groupid!=nowFh->groupid)
                    continue;
                } else {
                    if (!isThreadTitle(fh->title,nowFh->title))
                        continue;
                }

                /* ÊÇÍ¬Ò»Ö÷Ìâ*/
                count++;
                if (func) {
                    ret=(*func)(conf,nowFh,now,arg);
                    if (ret==APPLY_QUIT) break;

                    /*ÔÚ·µ»ØAPPLY_REAPPLYµÄÊ±ºò²»ÐèÒªÒÆ¶¯Ö¸Õë*/
                    needmove=(ret!=APPLY_REAPPLY);
                }
            }
        }
    }
    BBS_CATCH {
    }
    BBS_END;
    if (pFh!=MAP_FAILED)
        end_mmapfile((void *) pFh, size, -1);
    return count;
}

int thread_search(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
//TODO: ÔÚ.DIRµÄÊ±ºò£¬ÓÃgroupidÀ´ËÑ
    bool up=(bool)extraarg;
    char* title=fh->title;
    if (title[0] == 'R' && (title[1] == 'e' || title[1] == 'E')
        && title[2] == ':')
        title += 4;
    setqtitle(title);
    switch (read_search_articles(conf, title, up, 2)) {
        case 1:
            return PARTUPDATE;
        default:
            conf->show_endline(conf);
    }
    return DONOTHING;
}

int thread_read(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    int mode=(int)extraarg;
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    conf->new_pos=0;
    switch (mode) {
        case SR_FIRST:
            apply_thread(conf,fh,fileheader_thread_read,false,(void*)mode);
            break;
        case SR_LAST:
            apply_thread(conf,fh,fileheader_thread_read,true,(void*)mode);
            break;
        case SR_FIRSTNEW:
            apply_thread(conf,fh,fileheader_thread_read,false,(void*)mode);
            if (conf->new_pos==0) {
                apply_thread(conf,fh,fileheader_thread_read,true,(void*)SR_FIRSTNEWDOWNSEARCH);
            }
            break;
    }
    if (conf->new_pos==0) return DONOTHING;
    if (mode==SR_FIRSTNEW) {
        if (conf->new_pos!=0) {
            list_select_add_key(conf,'r'); //SEL changeµÄÏÂÒ»ÌõÖ¸ÁîÊÇread
            read_arg->readmode=READ_THREAD;
        }
    }
    return SELCHANGE;
}


int read_sendmsgtoauthor(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    struct user_info *uin;

    if (!HAS_PERM(currentuser, PERM_PAGE))
        return DONOTHING;
    clear();
    uin = (struct user_info *) t_search(fh->owner, false);
    if (!uin || !canmsg(currentuser, uin))
        do_sendmsg(NULL, NULL, 0);

    else {
        strncpy(MsgDesUid, uin->userid, 20);
        do_sendmsg(uin, NULL, 0);
    }
    return FULLUPDATE;
}


int read_showauthor(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    if ( /*strchr(fileinfo->owner,'.')|| */ !strcmp(fh->owner, "Anonymous") || !strcmp(fh->owner, "deliver"))       /* Leeward 98.04.14 */
        return DONOTHING;

    else
        t_query(fh->owner);
    return FULLUPDATE;
}

/*Ö±½Ó²é×÷Õß×ÊÁÏ*/
int read_showauthorinfo(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    struct userec uinfo;
    struct userec *lookupuser;
    int id;

    if (!HAS_PERM(currentuser, PERM_ACCOUNTS)
        || !strcmp(fh->owner, "Anonymous")
        || !strcmp(fh->owner, "deliver"))
        return DONOTHING;

    else {
        if (0 == (id = getuser(fh->owner, &lookupuser))) {
            move(2, 0);
            prints("²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ");
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

int read_showauthorBM(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    struct boardheader *bptr;
    int tuid = 0;
    int n;

    if (!HAS_PERM(currentuser, PERM_ACCOUNTS) || !strcmp(fh->owner, "Anonymous") || !strcmp(fh->owner, "deliver"))
        return DONOTHING;
    else {
        struct userec *lookupuser;

        if (!(tuid = getuser(fh->owner, &lookupuser))) {
            clrtobot();
            prints("²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ\n");
            pressanykey();
            move(2, 0);
            clrtobot();
            return FULLUPDATE;
        }

        move(3, 0);
        if (!(lookupuser->userlevel & PERM_BOARDS)) {
            clrtobot();
            prints("ÓÃ»§%s²»ÊÇ°æÖ÷!\n", lookupuser->userid);
            pressanykey();
            move(2, 0);
            clrtobot();
            return FULLUPDATE;
        }
        clrtobot();
        prints("ÓÃ»§%sÎªÒÔÏÂ°æµÄ°æÖ÷\n\n", lookupuser->userid);

        prints("©³©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©×©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©·\n");
        prints("©§            °æÓ¢ÎÄÃû            ©§            °æÖÐÎÄÃû            ©§\n");

        for (n = 0; n < get_boardcount(); n++) {
            bptr = (struct boardheader *) getboard(n + 1);
            if (chk_BM_instr(bptr->BM, lookupuser->userid) == true) {
                prints("©Ç©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©ï©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©Ï\n");
                prints("©§%-32s©§%-32s©§\n", bptr->filename, bptr->title + 12);
            }
        }
        prints("©»©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©ß©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¿\n");
        pressanykey();
        move(2, 0);
        clrtobot();
        return FULLUPDATE;
    }
}


int read_addauthorfriend(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    if (!strcmp("guest", currentuser->userid))
        return DONOTHING;;

    if (!strcmp(fh->owner, "Anonymous") || !strcmp(fh->owner, "deliver"))
        return DONOTHING;
    else {
        clear();
        addtooverride(fh->owner);
    }
    return FULLUPDATE;
}

int read_zsend(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    return zsend_post(conf->pos, fh, read_arg->direct);
}

int read_cross(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    return do_cross(conf->pos, fh, read_arg->direct);
}

#ifdef PERSONAL_CORP
int read_importpc(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    struct read_arg *read_arg = (struct read_arg *) conf->arg;
    return import_to_pc(conf->pos, fh, read_arg->direct);
}
#endif

