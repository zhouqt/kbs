/*
		scan complete for global variable
*/

#include "bbs.h"

extern time_t login_start_time;
extern int *zapbuf;
extern int zapbuf_changed;
static int yank_flag=0;
struct favbrd_struct {
    int flag;
    char *title;
    int father;
};
extern struct favbrd_struct favbrd_list[FAVBOARDNUM];
extern int favbrd_list_t, favnow;
static int check_newpost(struct newpostdata *ptr);

void EGroup(cmd)
char *cmd;
{
    char buf[STRLEN];
    char *boardprefix;

    sprintf(buf, "EGROUP%c", *cmd);
    boardprefix = sysconf_str(buf);
    choose_board(DEFINE(currentuser, DEF_NEWPOST) ? 1 : 0, boardprefix,0,0);
}

void ENewGroup(cmd)
char *cmd;
{
	choose_board(DEFINE(currentuser,DEF_NEWPOST) ? 1 : 0, NULL, -2, 0);
}

static int clear_all_board_read_flag_func(struct boardheader *bh,void* arg)
{
#ifdef HAVE_BRC_CONTROL
    if (brc_initial(currentuser->userid, bh->filename) != 0)
        brc_clear();
#endif
}

int clear_all_board_read_flag()
{
    char ans[4];
    struct boardheader* save_board;
    int bid;

    getdata(t_lines - 1, 0, "Çå³ýËùÓÐµÄÎ´¶Á±ê¼ÇÃ´(Y/N)? [N]: ", ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y') {

        bid=currboardent;
        save_board=currboard;
        apply_boards(clear_all_board_read_flag_func,NULL);
        currboard=save_board;
        currboardent=bid;
    }
    return 0;
}

void Boards()
{
    choose_board(0, NULL,0,0);
}

void New()
{
    choose_board(1, NULL,0,0);
}

int unread_position(dirfile, ptr)
char *dirfile;
struct newpostdata *ptr;
{
#ifdef HAVE_BRC_CONTROL
    struct fileheader fh;
    int id;
    int fd, offset, step, num;

    num = ptr->total + 1;
    if (ptr->unread && (fd = open(dirfile, O_RDWR)) > 0) {
        if (!brc_initial(currentuser->userid, ptr->name)) {
            num = 1;
        } else {
            offset = (int) ((char *) &(fh.id) - (char *) &(fh));
            num = ptr->total - 1;
            step = 4;
            while (num > 0) {
                lseek(fd, offset + num * sizeof(fh), SEEK_SET);
                if (read(fd, &id, sizeof(unsigned int)) <= 0 || !brc_unread(id))
                    break;
                num -= step;
                if (step < 32)
                    step += step / 2;
            }
            if (num < 0)
                num = 0;
            while (num < ptr->total) {
                lseek(fd, offset + num * sizeof(fh), SEEK_SET);
                if (read(fd, &id, sizeof(unsigned int)) <= 0 || brc_unread(id))
                    break;
                num++;
            }
        }
        close(fd);
    }
    if (num < 0)
        num = 0;
    return num;
#else
    return 0;
#endif
}

int show_authorBM(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
    struct boardheader *bptr;
    int tuid = 0;
    int n;

    if (!HAS_PERM(currentuser, PERM_ACCOUNTS) || !strcmp(fileinfo->owner, "Anonymous") || !strcmp(fileinfo->owner, "deliver"))
        return DONOTHING;
    else {
        struct userec *lookupuser;

        if (!(tuid = getuser(fileinfo->owner, &lookupuser))) {
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

/* inserted by cityhunter to query BM */
int query_bm()
{
    const struct boardheader *bptr;
    int n;
    char uident[STRLEN];
    int tuid = 0;
    struct userec *lookupuser;

    modify_user_mode(QUERY);
    move(2, 0);
    clrtobot();
    prints("<ÊäÈëÊ¹ÓÃÕß´úºÅ, °´¿Õ°×¼ü¿ÉÁÐ³ö·ûºÏ×Ö´®>\n");
    move(1, 0);
    clrtoeol();
    prints("²éÑ¯Ë­: ");
    usercomplete(NULL, uident);
    if (uident[0] == '\0') {
        clear();
        return FULLUPDATE;
    }
    if (!(tuid = getuser(uident, &lookupuser))) {
        move(2, 0);
        clrtoeol();
        prints("[1m²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ[m\n");
        pressanykey();
        move(2, 0);
        clrtoeol();
        return FULLUPDATE;
    }

    move(3, 0);
    if (!(lookupuser->userlevel & PERM_BOARDS)) {
        prints("ÓÃ»§%s²»ÊÇ°æÖ÷!\n", lookupuser->userid);
        pressanykey();
        move(2, 0);
        clrtoeol();
        return FULLUPDATE;
    }
    prints("ÓÃ»§%sÎªÒÔÏÂ°æµÄ°æÖ÷\n\n", lookupuser->userid);

    prints("©³©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©×©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©·\n");
    prints("©§            °æÓ¢ÎÄÃû            ©§            °æÖÐÎÄÃû            ©§\n");

    for (n = 0; n < get_boardcount(); n++) {
        bptr = getboard(n + 1);
        if (chk_BM_instr(bptr->BM, lookupuser->userid) == true) {
            prints("©Ç©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©ï©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©Ï\n");
            prints("©§%-32s©§%-32s©§\n", bptr->filename, bptr->title + 12);
        }
    }
    prints("©»©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©ß©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¥©¿\n");
    pressanykey();
    move(2, 0);
    clrtoeol();
    return FULLUPDATE;
}

/* end of insertion */

extern int load_mboards();
extern void mailtitle();
extern char *maildoent(char *buf, int num, struct fileheader *ent);

static int check_newpost(struct newpostdata *ptr)
{
    struct BoardStatus *bptr;

    if (ptr->dir)
        return 0;

    ptr->total = ptr->unread = 0;

    bptr = getbstatus(ptr->pos+1);
    if (bptr == NULL)
        return 0;
    ptr->total = bptr->total;
    ptr->currentusers = bptr->currentusers;

#ifdef HAVE_BRC_CONTROL
    if (!brc_initial(currentuser->userid, ptr->name)) {
        ptr->unread = 1;
    } else {
        if (brc_unread(bptr->lastpost)) {
            ptr->unread = 1;
        }
    }
#endif
    return 1;
}


enum board_mode {
    BOARD_BOARD,
    BOARD_BOARDALL,
    BOARD_FAV
};

struct favboard_proc_arg {
    struct newpostdata *nbrd;
    int favmode;
    int newflag;
    int tmpnum;
    enum board_mode yank_flag;
    int father; /*±£´æ¸¸½áµã£¬Èç¹ûÊÇÊÕ²Ø¼Ð£¬ÊÇfav_father,
    Èç¹ûÊÇ°æÃæÄ¿Â¼£¬ÊÇgroup±àºÅ*/
    bool reloaddata;
    bool select_group; //Ñ¡ÔñÁËÒ»¸öÄ¿Â¼

    char* boardprefix;
    /*ÓÃÓÚsearch_boardµÃÊ±ºò»º´æ*/
    int loop_mode;
    int find;
    char bname[BOARDNAMELEN + 1];
    int bname_len;
    char** namelist;
};

static int search_board(int *num, struct _select_def *conf, int key)
{
    struct favboard_proc_arg *arg = (struct favboard_proc_arg *) conf->arg;
    int n, ch, tmpn = false;
    struct newpostdata* buffer;

    *num=0;
    if (arg->find == true) {
        bzero(arg->bname, BOARDNAMELEN);
        arg->find = false;
        arg->bname_len = 0;
    }
    if (arg->namelist==NULL) {
    	arg->namelist=(char**)malloc(MAXBOARD*sizeof(char*));
    	conf->get_data(conf,-1,-1);
    }
    while (1) {
        move(t_lines - 1, 0);
        clrtoeol();
        prints("ÇëÊäÈëÒªÕÒÑ°µÄ board Ãû³Æ£º%s", arg->bname);
        if (key == -1)
            ch = igetkey();
        else {
            ch = key;
            key = -1;
        }

        if (ch == KEY_REFRESH)
            break;
        if (isprint2(ch)) {
            arg->bname[arg->bname_len++] = ch;
            for (n = 0; n < conf->item_count; n++) {
                if ((!strncasecmp(arg->namelist[n], arg->bname, arg->bname_len))&&*num==0) {
                    tmpn = true;
                    *num = n;
                }
                if (!strcmp(arg->namelist[n], arg->bname))
                    return 1 /*ÕÒµ½ÀàËÆµÄ°æ£¬»­ÃæÖØ»­ */ ;
            }
            if (tmpn)
                return 1;
            if (arg->find == false) {
                arg->bname[--arg->bname_len] = '\0';
            }
            continue;
        } else if (ch == Ctrl('H') || ch == KEY_LEFT || ch == KEY_DEL || ch == '\177') {
            arg->bname_len--;
            if (arg->bname_len< 0) {
                arg->find = true;
                break;
            } else {
                arg->bname[arg->bname_len]=0;
                continue;
            }
        } else if (ch == '\t') {
            arg->find = true;
            break;
        } else if (Ctrl('Z') == ch) {
            r_lastmsg();        /* Leeward 98.07.30 support msgX */
            break;
        } else if (ch == '\n' || ch == '\r' || ch == KEY_RIGHT) {
            arg->find = true;
            break;
        }
        bell();
    }
    if (arg->find) {
        move(t_lines - 1, 0);
        clrtoeol();
        return 2 /*½áÊøÁË */ ;
    }
    return 1;
}

static int fav_show(struct _select_def *conf, int pos)
{
    struct favboard_proc_arg *arg = (struct favboard_proc_arg *) conf->arg;
    struct newpostdata *ptr;
    char buf[LENGTH_SCREEN_LINE];

    ptr = &arg->nbrd[pos-(conf->page_pos)];
    if ((ptr->dir == 1)&&arg->favmode) {        /* added by bad 2002.8.3*/
        if (ptr->tag < 0)
            prints("       ");
        else if (!arg->newflag)
            prints(" %4d  £«  <Ä¿Â¼>  ", pos);
        else
            prints(" %4d  £«  <Ä¿Â¼>  ", ptr->total);
    } else {
	if ((ptr->dir==1)||(ptr->flag&BOARD_GROUP)) {
            prints(" %4d  £« ", ptr->total);
	} else {
        if (!arg->newflag){
			check_newpost(ptr);
            prints(" %4d  %s ", pos, ptr->unread?"¡ô" : "¡ó");     /*zap±êÖ¾ */
		}
        else if (ptr->zap && !(ptr->flag & BOARD_NOZAPFLAG)) {
            /*
             * ptr->total = ptr->unread = 0;
             * prints( "    -    -" ); 
             */
            /*
             * Leeward: 97.12.15: extended display 
             */
            check_newpost(ptr);
            prints(" %4d%s%s ", ptr->total, ptr->total > 9999 ? " " : "  ", ptr->unread ? "¡ô" : "¡ó"); /*ÊÇ·ñÎ´¶Á */
        } else {
            if (ptr->total == -1) {
                /*refresh();*/
                check_newpost(ptr);
            }
            prints(" %4d%s%s ", ptr->total, ptr->total > 9999 ? " " : "  ", ptr->unread ? "¡ô" : "¡ó"); /*ÊÇ·ñÎ´¶Á */
        }
      }
    }
    /*
     * Leeward 98.03.28 Displaying whether a board is READONLY or not 
     */
    if (ptr->dir == 2)
        sprintf(buf, "%s(%d)", ptr->title, ptr->total);
    else if ((ptr->dir >= 1)&&arg->favmode)
        sprintf(buf, "%s", ptr->title); /* added by bad 2002.8.3*/
    else if (true == checkreadonly(ptr->name))
        sprintf(buf, "[Ö»¶Á] %s", ptr->title + 8);
    else
        sprintf(buf, "%s", ptr->title + 1);

    if ((ptr->dir >= 1)&&arg->favmode)          /* added by bad 2002.8.3*/
        prints("%-50s", buf);
    else {
          char flag[20];
          char f;
          char tmpBM[BM_LEN + 1];

          strncpy(tmpBM, ptr->BM, BM_LEN);
          tmpBM[BM_LEN] = 0;
  
          if ((ptr->flag & BOARD_CLUB_READ) && (ptr->flag & BOARD_CLUB_WRITE))
                 f='A';
          else if (ptr->flag & BOARD_CLUB_READ)
                f = 'c';
          else if (ptr->flag & BOARD_CLUB_WRITE)
                f = 'p';
          else
                f = ' ';
          if (ptr->flag & BOARD_CLUB_HIDE) {
	            sprintf(flag,"\x1b[1;31m%c\x1b[m",f);
	       } else if (f!=' ') {
	           sprintf(flag,"\x1b[1;33m%c\x1b[m",f);
          } else sprintf(flag,"%c",f);
          prints("%c%-16s %s%s%-36s %-12s", ((ptr->zap && !(ptr->flag & BOARD_NOZAPFLAG)) ? '*' : ' '), ptr->name, (ptr->flag & BOARD_VOTEFLAG) ? "[31;1mV[m" : " ", flag, buf, ptr->BM[0] <= ' ' ? "³ÏÕ÷°æÖ÷ÖÐ" : strtok(tmpBM, " ")); /*µÚÒ»¸ö°æÖ÷ */
#ifdef BOARD_SHOW_ONLINE
        if(scr_cols>=80+5) {
            int x,y;
            getyx(&y, &x);
            move(y, 81);
            prints("%4d", ptr->currentusers);
        }
#endif
    }
    prints("\n");
    return SHOW_CONTINUE;
}

static int fav_prekey(struct _select_def *conf, int *command)
{
    struct favboard_proc_arg *arg = (struct favboard_proc_arg *) conf->arg;
    struct newpostdata *ptr;

    if (arg->loop_mode) {
        int tmp, num;
/* search_boardÓÐÎÊÌâ£¬Ä¿Ç°ÎÒÃÇÖ»ÓÐÒ»Ò³µÄÊý¾Ý£¬Ö»ÄÜsearch
Ò»Ò³*/
        tmp = search_board(&num, conf, *command);
        if (tmp == 1) {
            conf->new_pos = num + 1;
            arg->loop_mode = 1;
            move(t_lines - 1, 0);
            clrtoeol();
            prints("ÇëÊäÈëÒªÕÒÑ°µÄ board Ãû³Æ£º%s", arg->bname);
            return SHOW_SELCHANGE;
        } else {
            arg->find = true;
            arg->bname_len = 0;
            arg->loop_mode = 0;
            conf->new_pos = num + 1;
            return SHOW_REFRESH;
        }
        return SHOW_REFRESH;
    }
    if ((*command == '\r' || *command == '\n') && (arg->tmpnum != 0)) {
        /* Ö±½ÓÊäÈëÊý×ÖÌø×ª*/
        conf->new_pos = arg->tmpnum;
        arg->tmpnum = 0;
        return SHOW_SELCHANGE;
    }

    if (!isdigit(*command))
        arg->tmpnum = 0;

    if (!arg->loop_mode) {
        int y,x;
        getyx(&y, &x);
        update_endline();
        move(y, x);
    }
    ptr = &arg->nbrd[conf->pos - conf->page_pos];
    switch (*command) {
    case 'e':
    case 'q':
        *command = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *command = KEY_UP;
        break;
    case ' ':
    case 'N':
        *command = KEY_PGDN;
        break;
    case 'n':
    case 'j':
        *command = KEY_DOWN;
        break;
    };
    return SHOW_CONTINUE;
}

static int fav_gotonextnew(struct _select_def *conf)
{
    struct favboard_proc_arg *arg = (struct favboard_proc_arg *) conf->arg;
    int tmp, num,page_pos=conf->page_pos;

        /*ËÑÑ°ÏÂÒ»¸öÎ´¶ÁµÄ°æÃæ*/
    if (arg->newflag) {
      num = tmp = conf->pos;
      while(num<=conf->item_count) {
          while ((num <= page_pos+conf->item_per_page-1)&&num<=conf->item_count) {
              struct newpostdata *ptr;
  
              ptr = &arg->nbrd[num - page_pos];
              if ((ptr->total == -1) && (ptr->dir == 0))
                  check_newpost(ptr);
              if (ptr->unread)
                  break;
                  num++;
          }
          if ((num <= page_pos+conf->item_per_page-1)&&num<=conf->item_count) {
              conf->pos = num;
              conf->page_pos=page_pos;
  	     return SHOW_DIRCHANGE;
  	 }
          page_pos+=conf->item_per_page;
          num=page_pos;
          (*conf->get_data)(conf, page_pos, conf->item_per_page);
      }
      if (page_pos!=conf->page_pos)
        (*conf->get_data)(conf, conf->page_pos, conf->item_per_page);
    }
    return SHOW_REFRESH;
}

static int fav_onselect(struct _select_def *conf)
{
    struct favboard_proc_arg *arg = (struct favboard_proc_arg *) conf->arg;
    char buf[STRLEN];
    struct newpostdata *ptr;

    ptr = &arg->nbrd[conf->pos - conf->page_pos];

    if (arg->select_group) return SHOW_SELECT; //select a group
    arg->select_group=false;
    if ((ptr->dir == 1)||((arg->favmode==1)&&(ptr->flag&BOARD_GROUP))) {        /* added by bad 2002.8.3*/
        return SHOW_SELECT;
    } else {
        struct boardheader bh;
        int tmp, page;

        if (getboardnum(ptr->name, &bh) != 0 && check_read_perm(currentuser, &bh)) {
            int bid;
	    int returnmode;
            bid = getbnum(ptr->name);

            currboardent=bid;
            currboard=(struct boardheader*)getboard(bid);

#ifdef HAVE_BRC_CONTROL
            brc_initial(currentuser->userid, ptr->name);
#endif
            memcpy(currBM, ptr->BM, BM_LEN - 1);
            if (DEFINE(currentuser, DEF_FIRSTNEW)) {
                setbdir(digestmode, buf, currboard->filename);
                tmp = unread_position(buf, ptr);
                page = tmp - t_lines / 2;
                getkeep(buf, page > 1 ? page : 1, tmp + 1);
            }
            while (1) {
                returnmode=Read();
                if (returnmode==CHANGEMODE) { //select another board
                    if (currboard->flag&BOARD_GROUP) {
			arg->select_group=true;
                        return SHOW_SELECT;
                    }
                } else break;
            }
            (*conf->get_data)(conf, conf->page_pos, conf->item_per_page);
            modify_user_mode(SELECT);
            if (arg->newflag) { /* Èç¹ûÊÇreadnewµÄ»°£¬ÔòÌøµ½ÏÂÒ»¸öÎ´¶Á°æ */
                return fav_gotonextnew(conf);
            }
        }
        return SHOW_REFRESH;
    }
    return SHOW_CONTINUE;
}

static int fav_key(struct _select_def *conf, int command)
{
    struct favboard_proc_arg *arg = (struct favboard_proc_arg *) conf->arg;
    struct newpostdata *ptr;

    ptr = &arg->nbrd[conf->pos - conf->page_pos];
    if (command >= '0' && command <= '9') {
        arg->tmpnum = arg->tmpnum * 10 + (command - '0');
        return SHOW_CONTINUE;
    }
    switch (command) {
    case Ctrl('Z'):
        r_lastmsg();            /* Leeward 98.07.30 support msgX */
        break;
    case 'X':                  /* Leeward 98.03.28 Set a board READONLY */
        {
            char buf[STRLEN];

            if (!HAS_PERM(currentuser, PERM_SYSOP) && !HAS_PERM(currentuser, PERM_OBOARDS))
                break;
            if (!strcmp(ptr->name, "syssecurity")
                || !strcmp(ptr->name, "Filter")
                || !strcmp(ptr->name, "junk")
                || !strcmp(ptr->name, "deleted"))
                break;          /* Leeward 98.04.01 */
            if (ptr->dir)
                break;

            if (strlen(ptr->name)) {
                board_setreadonly(ptr->name, 1);

                /*
                 * Bigman 2000.12.11:ÏµÍ³¼ÇÂ¼ 
                 */
                sprintf(genbuf, "Ö»¶ÁÌÖÂÛÇø %s ", ptr->name);
                securityreport(genbuf, NULL, NULL);
                sprintf(genbuf, " readonly board %s", ptr->name);
                bbslog("1user", "%s", genbuf);

                return SHOW_REFRESHSELECT;
            }
            break;
        }
    case 'Y':                  /* Leeward 98.03.28 Set a board READABLE */
        {
            char buf[STRLEN];

            if (!HAS_PERM(currentuser, PERM_SYSOP) && !HAS_PERM(currentuser, PERM_OBOARDS))
                break;
            if (ptr->dir)
                break;

            board_setreadonly(ptr->name, 0);

            /*
             * Bigman 2000.12.11:ÏµÍ³¼ÇÂ¼ 
             */
            sprintf(genbuf, "½â¿ªÖ»¶ÁÌÖÂÛÇø %s ", ptr->name);
            securityreport(genbuf, NULL, NULL);
            sprintf(genbuf, " readable board %s", ptr->name);
            bbslog("1user", "%s", genbuf);

            return SHOW_REFRESHSELECT;
        }
        break;
    case 'L':
    case 'l':                  /* Luzi 1997.10.31 */
        if (uinfo.mode != LOOKMSGS) {
            show_allmsgs();
            return SHOW_REFRESH;
        }
        break;
    case 'W':
    case 'w':                  /* Luzi 1997.10.31 */
        if (!HAS_PERM(currentuser, PERM_PAGE))
            break;
        s_msg();
        return SHOW_REFRESH;
    case 'u':                  /*Haohmaru.99.11.29 */
        {
            int oldmode = uinfo.mode;

            clear();
            modify_user_mode(QUERY);
            t_query(NULL);
            modify_user_mode(oldmode);
            return SHOW_REFRESH;
        }
	/*add by stiger */
    case 'H':
	{
		read_hot_info(0,NULL,NULL);
    	return SHOW_REFRESH;
	}
	/* add end */
    case '!':
        Goodbye();
        return SHOW_REFRESH;
    case 'O':
    case 'o':                  /* Luzi 1997.10.31 */
#ifdef NINE_BUILD
    case 'C':
    case 'c':
#endif
        {                       /* Leeward 98.10.26 fix a bug by saving old mode */
            int savemode;

            savemode = uinfo.mode;

            if (!HAS_PERM(currentuser, PERM_BASIC))
                break;
            t_friends();
            modify_user_mode(savemode);
            return SHOW_REFRESH;
        }
#ifdef NINE_BUILD
    case 'F':
    case 'f':
#else
    case 'C':
    case 'c':                  /*ÔÄ¶ÁÄ£Ê½ */
#endif
        if (arg->newflag == 1)
            arg->newflag = 0;
        else
            arg->newflag = 1;
        return SHOW_REFRESH;
    case 'h':
        show_help("help/boardreadhelp");
        return SHOW_REFRESH;
    case '/':                  /*ËÑË÷board */
        {
            int tmp, num;

            tmp = search_board(&num, conf , -1);
            if (tmp == 1) {
                conf->new_pos = num + 1;
                arg->loop_mode = 1;
                move(t_lines - 1, 0);
                clrtoeol();
                prints("ÇëÊäÈëÒªÕÒÑ°µÄ board Ãû³Æ£º%s", arg->bname);
                return SHOW_SELCHANGE;
            } else {
                arg->find = true;
                arg->bname_len = 0;
                arg->loop_mode = 0;
                conf->new_pos = num + 1;
                return SHOW_REFRESH;
            }
            return SHOW_REFRESH;
        }
    case 'S':
        currentuser->flags ^= BRDSORT_FLAG;  /*ÅÅÐò·½Ê½ */
        return SHOW_DIRCHANGE;
    case 's':                  /* sort/unsort -mfchen */
        if (do_select(0, NULL, genbuf) == CHANGEMODE) {
            if (!(currboard->flag&BOARD_GROUP)) {
                while (1) {
                    int returnmode;
                    returnmode=Read();
                    if (returnmode==CHANGEMODE) { //select another board
                        if (currboard->flag&BOARD_GROUP) {
                            arg->select_group=true;
                            return SHOW_SELECT;
                        }
                    } else break;
                }
            }
            else {
		arg->select_group=true;
                return SHOW_SELECT;
            }
        }
        modify_user_mode(arg->newflag ? READNEW : READBRD);
        return SHOW_REFRESH;
    case Ctrl('O'):
    case 'a':
        {
            char bname[STRLEN];
            int i = 0;

            if (favbrd_list_t >= FAVBOARDNUM) {
                move(2, 0);
                clrtoeol();
                prints("¸öÈËÈÈÃÅ°æÊýÒÑ¾­´ïÉÏÏÞ(%d)£¡", FAVBOARDNUM);
                pressreturn();
                return SHOW_REFRESH;
            }
            if (BOARD_FAV == arg->yank_flag) {
                move(0, 0);
                clrtoeol();
                prints("ÊäÈëÌÖÂÛÇøÓ¢ÎÄÃû (´óÐ¡Ð´½Ô¿É£¬°´¿Õ°×¼ü×Ô¶¯ËÑÑ°): ");
                clrtoeol();

                make_blist();
                namecomplete((char *) NULL, bname);
                CreateNameList();   /*  free list memory. */
                if (*bname)
                    i = getbnum(bname);
                if (i==0)
                    return SHOW_REFRESH;
            } else {
                struct boardheader bh;
                int total=0,j,k;
                if (favbrd_list_t == -1)
                    load_favboard(0);
                i=getboardnum(ptr->name, &bh);
                if (i<=0)
                    return SHOW_REFRESH;
                for(j=0;j<favbrd_list_t;j++) {
                    if(favbrd_list[j].flag==-1) total++;
                }
                if(total==0) {
                    SetFav(-1);
                    if (IsFavBoard(i - 1)) {
                        move(2, 0); 
                        clrtoeol();
                        prints("ÒÑ´æÔÚ¸ÃÌÖÂÛÇø.");
                        clrtoeol();
                        pressreturn();
                        return SHOW_REFRESH;
                    }
                    move(2,0);
                    if (askyn("¼ÓÈë¸öÈË¶¨ÖÆÇø£¿",0)!=1)
                        return SHOW_REFRESH;
                }
                else {
                    struct _select_item *sel;
                    char root[18]="¶¨ÖÆÇøÖ÷Ä¿Â¼";
                    clear();
                    move(3, 3);
                    prints("ÇëÑ¡Ôñ¼ÓÈëµ½¶¨ÖÆÇøÄÄ¸öÄ¿Â¼");
                    sel = (struct _select_item *) malloc(sizeof(struct _select_item) * (total+2));
                    sel[0].x = 3;
                    sel[0].y = 6;
                    sel[0].hotkey = '0';
                    sel[0].type = SIT_SELECT;
                    sel[0].data = root;
                    k=1;
                    for(j=0;j<favbrd_list_t;j++)
                        if(favbrd_list[j].flag==-1) {
                            sel[k].x = 3;
                            sel[k].y = 6+k;
                            sel[k].hotkey = '0'+k;
                            sel[k].type = SIT_SELECT;
                            sel[k].data = favbrd_list[j].title;
                            k++;
                        }
                    sel[k].x = -1;
                    sel[k].y = -1;
                    sel[k].hotkey = -1;
                    sel[k].type = 0;
                    sel[k].data = NULL;
                    k = simple_select_loop(sel, SIF_NUMBERKEY | SIF_SINGLE | SIF_ESCQUIT, 0, 6, NULL) - 1;
                    free(sel);
                    if(k>=0&&k<total+1) {
                        if(k==0) SetFav(-1);
                        else {
                            for(j=0;j<favbrd_list_t;j++)
                                if(favbrd_list[j].flag==-1) {
                                    k--;
                                    if(k==0) {
                                        SetFav(j);
                                        break;
                                    }
                                }
                        }
                    }
                    else
                        return SHOW_REFRESH;
                }
            }
            if (i > 0 && !IsFavBoard(i - 1)) {
                addFavBoard(i - 1);
                save_favboard();
                arg->reloaddata=true;
                if (BOARD_FAV == arg->yank_flag)
                    return SHOW_DIRCHANGE;
                else
                    return SHOW_REFRESH;
            } else if (IsFavBoard(i - 1)) {
                move(2, 0);
                prints("ÒÑ´æÔÚ¸ÃÌÖÂÛÇø.");
                clrtoeol();
                pressreturn();
                return SHOW_REFRESH;
            } else {
                move(2, 0);
                prints("²»ÕýÈ·µÄÌÖÂÛÇø.");
                clrtoeol();
                pressreturn();
                return SHOW_REFRESH;
            }
        }
        break;
    case 'A':                  /* added by bad 2002.8.3*/
        if (BOARD_FAV == arg->yank_flag) {
            char bname[STRLEN];
            int i = 0;

            if (favbrd_list_t >= FAVBOARDNUM) {
                move(2, 0);
                clrtoeol();
                prints("¸öÈËÈÈÃÅ°æÊýÒÑ¾­´ïÉÏÏÞ(%d)£¡", FAVBOARDNUM);
                pressreturn();
                return SHOW_REFRESH;
            }
            move(0, 0);
            clrtoeol();
            getdata(0, 0, "ÊäÈëÌÖÂÛÇøÄ¿Â¼Ãû: ", bname, 22, DOECHO, NULL, true);
            if (bname[0]) {
                addFavBoardDir(i, bname);
                save_favboard();
                arg->reloaddata=true;
                return SHOW_DIRCHANGE;
            }
        }
        break;
    case 'T':                  /* added by bad 2002.8.3*/
        if (BOARD_FAV == arg->yank_flag) {
            char bname[STRLEN];
            int i = 0;

            if (ptr->dir == 1 && ptr->tag >= 0) {
                move(0, 0);
                clrtoeol();
                getdata(0, 0, "ÊäÈëÌÖÂÛÇøÄ¿Â¼Ãû: ", bname, 22, DOECHO, NULL, true);
                if (bname[0]) {
                    changeFavBoardDir(ptr->tag, bname);
                    save_favboard();
                    return SHOW_REFRESH;
                }
            }
        }
        break;
    case 'm':
        if (arg->yank_flag == BOARD_FAV) {
            if (currentuser->flags & BRDSORT_FLAG) {
                move(0, 0);
                prints("ÅÅÐòÄ£Ê½ÏÂ²»ÄÜÒÆ¶¯£¬ÇëÓÃ'S'¼üÇÐ»»!");
                pressreturn();
            } else {
                if (ptr->tag >= 0) {
                    int p, q;
                    char ans[5];

                    p = ptr->tag;
                    move(0, 0);
                    clrtoeol();
                    getdata(0, 0, "ÇëÊäÈëÒÆ¶¯µ½µÄÎ»ÖÃ:", ans, 4, DOECHO, NULL, true);
                    q = atoi(ans) - 1;
                    if (q < 0 || q >= conf->item_count) {
                        move(2, 0);
                        clrtoeol();
                        prints("·Ç·¨µÄÒÆ¶¯Î»ÖÃ£¡");
                        pressreturn();
                    } else {
                        arg->father=MoveFavBoard(p, q, arg->father);
                        save_favboard();
                        arg->reloaddata=true;
                        return SHOW_DIRCHANGE;
                    }
                }
            }
            return SHOW_REFRESH;
        }
        break;
    case 'd':
        if (BOARD_FAV == arg->yank_flag) {
            int p = 1;

            if (ptr->tag < 0)
                p = 0;
            if (p) {
                char ans[2];

                move(0, 0);
                clrtoeol();
                p = askyn("È·ÈÏÉ¾³ýÂð£¿", 0);
            }
            if (p) {
                DelFavBoard(ptr->tag);
                save_favboard();
                arg->father=favnow;
                arg->reloaddata=true;
                return SHOW_DIRCHANGE;
            }
            return SHOW_REFRESH;
        }
    case 'y':
        if (arg->yank_flag < BOARD_FAV) {
                                /*--- Modified 4 FavBoard 2000-09-11	---*/
            arg->yank_flag = 1 - arg->yank_flag;
            arg->reloaddata=true;
            return SHOW_DIRCHANGE;
        }
        break;
    case 'z':                  /* Zap */
        if (arg->yank_flag < BOARD_FAV) {
                                /*--- Modified 4 FavBoard 2000-09-11	---*/
            if (HAS_PERM(currentuser, PERM_BASIC) && !(ptr->flag & BOARD_NOZAPFLAG)) {
                ptr->zap = !ptr->zap;
                ptr->total = -1;
                zapbuf[ptr->pos] = (ptr->zap ? 0 : login_start_time);
                zapbuf_changed = 1;
                return SHOW_REFRESHSELECT;
            }
        }
        break;
    case 'v':                  /*Haohmaru.2000.4.26 */
        if(!strcmp(currentuser->userid, "guest") || !HAS_PERM(currentuser, PERM_READMAIL)) return SHOW_CONTINUE;
        clear();
		if (HAS_MAILBOX_PROP(&uinfo, MBP_MAILBOXSHORTCUT))
			MailProc();
		else
        	m_read();
        return SHOW_REFRESH;
    }
    return SHOW_CONTINUE;
}

static void fav_refresh(struct _select_def *conf)
{
    struct favboard_proc_arg *arg = (struct favboard_proc_arg *) conf->arg;
    struct newpostdata *ptr;

    clear();
    ptr = &arg->nbrd[conf->pos - conf->page_pos];
    if (DEFINE(currentuser, DEF_HIGHCOLOR)) {
        if (arg->yank_flag == BOARD_FAV)
            docmdtitle("[¸öÈË¶¨ÖÆÇø]",
                       "  [mÖ÷Ñ¡µ¥[\x1b[1;32m¡û\x1b[m,\x1b[1;32me\x1b[m] ÔÄ¶Á[\x1b[1;32m¡ú\x1b[m,\x1b[1;32mr\x1b[m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[m,\x1b[1;32m¡ý\x1b[m] Ìí¼Ó[\x1b[1;32ma\x1b[m,\x1b[1;32mA\x1b[m] ÒÆ¶¯[\x1b[1;32mm\x1b[m] É¾³ý[\x1b[1;32md\x1b[m] ÅÅÐò[\x1b[1;32mS\x1b[m] ÇóÖú[\x1b[1;32mh\x1b[m]");
        else
            docmdtitle("[ÌÖÂÛÇøÁÐ±í]",
                       "  [mÖ÷Ñ¡µ¥[\x1b[1;32m¡û\x1b[m,\x1b[1;32me\x1b[m] ÔÄ¶Á[\x1b[1;32m¡ú\x1b[m,\x1b[1;32mr\x1b[m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[m,\x1b[1;32m¡ý\x1b[m] ÁÐ³ö[\x1b[1;32my\x1b[m] ÅÅÐò[\x1b[1;32mS\x1b[m] ËÑÑ°[\x1b[1;32m/\x1b[m] ÇÐ»»[\x1b[1;32mc\x1b[m] ÇóÖú[\x1b[1;32mh\x1b[m]");
    } else {
        if (arg->yank_flag == BOARD_FAV)
            docmdtitle("[¸öÈË¶¨ÖÆÇø]", "  [mÖ÷Ñ¡µ¥[¡û,e] ÔÄ¶Á[¡ú,r] Ñ¡Ôñ[¡ü,¡ý] Ìí¼Ó[a,A] ÒÆ¶¯[m] É¾³ý[d] ÅÅÐò[S] ÇóÖú[h]");
        else
            docmdtitle("[ÌÖÂÛÇøÁÐ±í]", "  [mÖ÷Ñ¡µ¥[¡û,e] ÔÄ¶Á[¡ú,r] Ñ¡Ôñ[¡ü,¡ý] ÁÐ³ö[y] ÅÅÐò[S] ËÑÑ°[/] ÇÐ»»[c] ÇóÖú[h]");
    }
    move(2, 0);
    setfcolor(WHITE, DEFINE(currentuser, DEF_HIGHCOLOR));
    setbcolor(BLUE);
    clrtoeol();
    prints("  %s ÌÖÂÛÇøÃû³Æ        V Àà±ð ×ªÐÅ  %-24s °æ  Ö÷     ", arg->newflag ? "È«²¿ Î´¶Á" : "±àºÅ Î´¶Á", "ÖÐ  ÎÄ  Ðð  Êö");
#ifdef BOARD_SHOW_ONLINE
    if(scr_cols>=80+5) {
        move(2, 81);
        prints("ÔÚÏß");
    }
#endif
    resetcolor();
    if (!arg->loop_mode)
        update_endline();
    else {
            move(t_lines - 1, 0);
            clrtoeol();
            prints("ÇëÊäÈëÒªÕÒÑ°µÄ board Ãû³Æ£º%s", arg->bname);
    }

}

static int fav_getdata(struct _select_def *conf, int pos, int len)
{
    struct favboard_proc_arg *arg = (struct favboard_proc_arg *) conf->arg;

    if (arg->reloaddata) {
    	arg->reloaddata=false;
    	if (arg->namelist) 	{
    		free(arg->namelist);
    		arg->namelist=NULL;
    	}
    }
    if (pos==-1) 
        fav_loaddata(NULL, arg->father,1, conf->item_count,currentuser->flags & BRDSORT_FLAG,arg->namelist);
    else
        conf->item_count = fav_loaddata(arg->nbrd, arg->father,pos, len,currentuser->flags & BRDSORT_FLAG,NULL);
    return SHOW_CONTINUE;
}

static int boards_getdata(struct _select_def *conf, int pos, int len)
{
    struct favboard_proc_arg *arg = (struct favboard_proc_arg *) conf->arg;

    if (arg->reloaddata) {
    	arg->reloaddata=false;
    	if (arg->namelist) 	{
    		free(arg->namelist);
    		arg->namelist=NULL;
    	}
    }
    if (pos==-1) 
         load_boards(NULL, arg->boardprefix,arg->father,1, conf->item_count,currentuser->flags & BRDSORT_FLAG,arg->yank_flag,arg->namelist);
    else
         conf->item_count = load_boards(arg->nbrd, arg->boardprefix,arg->father,pos, len,currentuser->flags & BRDSORT_FLAG,arg->yank_flag,NULL);
    return SHOW_CONTINUE;
}
int choose_board(int newflag, char *boardprefix,int group,int favmode)
{
/* Ñ¡Ôñ °æ£¬ readnew»òreadboard */
    struct _select_def favboard_conf;
    struct favboard_proc_arg arg;
    POINT *pts;
    int i;
    int y;
    struct newpostdata *nbrd;
    int favlevel = 0;           /*µ±Ç°²ãÊý */
    int favlist[FAVBOARDNUM];   /* ±£´æµ±Ç°µÄgroup ÐÅÏ¢ºÍÊÕ²Ø¼ÐÐÅÏ¢*/
    int sellist[FAVBOARDNUM];   /*±£´æÄ¿Â¼µÝ¹éÐÅÏ¢ */
    int oldmode;
    int changelevel=-1; /*±£´æÔÚÄÇÒ»¼¶µÄÄ¿Â¼×ª»»ÁËmode,¾ÍÊÇ´ÓÊÕ²Ø¼Ð½øÈëÁË°æÃæÄ¿Â¼*/
    int selectlevel=-1; /*±£´æÔÚÄÄÒ»¼¶½øÈëÁËsÄ¿Â¼*/

    oldmode = uinfo.mode;
    modify_user_mode(SELECT);
    clear();
    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);

    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 1;
        pts[i].y = i + 3;
    };

    nbrd = (struct newpostdata *) malloc(sizeof(*nbrd) * BBS_PAGESIZE);
    arg.nbrd = nbrd;
    sellist[0] = 1;
    arg.favmode = favmode;
    if (favmode)
        favlist[0] = -1;
    else
        favlist[0] = group;
    arg.namelist=NULL;
    while (1) {
        bzero((char *) &favboard_conf, sizeof(struct _select_def));
        arg.tmpnum = 0;
        arg.father = favlist[favlevel];
        if (favmode) {
            arg.newflag = 1;
            arg.yank_flag = BOARD_FAV;
        } else {
            arg.newflag = newflag;
            arg.yank_flag = yank_flag;
        }
        if (favmode) {
            favnow = favlist[favlevel];
        } else {
            arg.boardprefix=boardprefix;
	 }
        arg.find = true;
        arg.loop_mode = 0;
	arg.select_group =false;

        favboard_conf.item_per_page = BBS_PAGESIZE;
        favboard_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;     /*|LF_HILIGHTSEL;*/
        favboard_conf.prompt = ">";
        favboard_conf.item_pos = pts;
        favboard_conf.arg = &arg;
        favboard_conf.title_pos.x = 0;
        favboard_conf.title_pos.y = 0;
        favboard_conf.pos = sellist[favlevel];
        favboard_conf.page_pos = ((sellist[favlevel]-1)/BBS_PAGESIZE)*BBS_PAGESIZE+1;
        if (arg.namelist) {
        	free(arg.namelist);
        	arg.namelist=NULL;
        }
        arg.reloaddata=false;

        if (favmode)        
            favboard_conf.get_data = fav_getdata;
        else
            favboard_conf.get_data = boards_getdata;
        (*favboard_conf.get_data)(&favboard_conf, favboard_conf.page_pos, BBS_PAGESIZE);
        if (favboard_conf.item_count==0)
            if (arg.yank_flag == BOARD_FAV || arg.yank_flag == BOARD_BOARDALL)
                break;
	    else {
                char ans[3];
                getdata(t_lines - 1, 0, "¸ÃÌÖÂÛÇø×éµÄ°æÃæÒÑ¾­±»ÄãÈ«²¿È¡ÏûÁË£¬ÊÇ·ñ²é¿´ËùÓÐÌÖÂÛÇø£¿(Y/N)[N]", ans, 2, DOECHO, NULL, true);
                if (toupper(ans[0]) != 'Y')
                    break;
		arg.yank_flag=BOARD_BOARDALL;
                (*favboard_conf.get_data)(&favboard_conf, favboard_conf.page_pos, BBS_PAGESIZE);
                if (favboard_conf.item_count==0)
		    break;
	    }
        fav_gotonextnew(&favboard_conf);
        favboard_conf.on_select = fav_onselect;
        favboard_conf.show_data = fav_show;
        favboard_conf.pre_key_command = fav_prekey;
        favboard_conf.key_command = fav_key;
        favboard_conf.show_title = fav_refresh;

        update_endline();
        if (list_select_loop(&favboard_conf) == SHOW_QUIT) {
            /*ÍË³öÒ»²ãÄ¿Â¼*/
            favlevel--;
            if (favlevel == -1)
                break;
            if (favlevel==changelevel) //´Ó°æÃæÄ¿Â¼·µ»ØÊÕ²Ø¼Ð
                favmode=1;
        } else {
            /*Ñ¡ÔñÁËÒ»¸öÄ¿Â¼,SHOW_SELECT£¬×¢ÒâÓÐ¸ö¼ÙÉè£¬Ä¿Â¼µÄÉî¶È
            ²»»á´óÓÚFAVBOARDNUM£¬·ñÔòselist»áÒç³ö
            ×¢Òâ£¬arg->select_group±»ÓÃÀ´±íÊ¾ÊÇselectÁË°æÃæ»¹ÊÇÓÃsÌø×ªµÄ¡£
            Èç¹ûÊÇsÌø×ª£¬arg->select_group=true,·ñÔòarg->select_group=false;
            TODO: ¶¯Ì¬Ôö³¤sellist
            */
            sellist[favlevel] = favboard_conf.pos;
            if ((selectlevel==-1)||(!arg.select_group))
                favlevel++;
	    else
		favlevel=selectlevel; /*ÍË»Øµ½¡ÏÒ»´ÎµÄÄ¿Â¼*/
            if (favmode) {
                if (arg.select_group||(nbrd[favboard_conf.pos - favboard_conf.page_pos].flag!=-1)) {
                    //½øÈë°æÃæÄ¿Â¼
                    if (arg.select_group) //select½øÈëµÄ 
                        favlist[favlevel] = currboardent;
                    else
                        favlist[favlevel] = nbrd[favboard_conf.pos - favboard_conf.page_pos].pos+1;
                    changelevel=favlevel-1;
                    favmode=0;
                } else
                    favlist[favlevel] = nbrd[favboard_conf.pos - favboard_conf.page_pos].tag;
            }
            else {
                if (arg.select_group) //select½øÈëµÄ
                    favlist[favlevel] = currboardent;
                else
                favlist[favlevel] = nbrd[favboard_conf.pos - favboard_conf.page_pos].pos+1;
            }
            if (arg.select_group) //select½øÈëµÄ
		    selectlevel=favlevel;
            sellist[favlevel] = 1;
        };
        clear();
    }
    free(nbrd);
    free(pts);
    if (arg.namelist) {
    	free(arg.namelist);
    	arg.namelist=NULL;
    }
    save_zapbuf();
    modify_user_mode(oldmode);
}

void FavBoard()
{
    if (favbrd_list_t == -1)
        load_favboard(1);
    choose_board(1, NULL,0,1);
}
