/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	only too int global: bmonly a_fmode
*/

/* ¾«»ªÇø Ïà¹Ø º¯Êý */

#include "bbs.h"

#define A_PAGESIZE      (t_lines - 5)

#define ADDITEM         0
#define ADDGROUP        1
#define ADDMAIL         2
#define ADDGOPHER       3

#define ANNPATH_NUM	40
int bmonly = 0;
int a_fmode = 1;
char *email_domain();
void a_menu();
void a_report();                /*Haohmaru.99.12.06.°æÖ÷¾«»ªÇø²Ù×÷¼ÇÂ¼£¬×÷Îª¿¼²é¹¤×÷µÄÒÀ¾Ý */

extern void a_prompt();         /* added by netty */
int t_search_down();
int t_search_up();
int a_loadnames(MENU * pm);     /* ×°Èë .Names */
#define BLK_SIZ 10240

static char *import_path[ANNPATH_NUM];  /*¶àË¿Â· */
static char *import_title[ANNPATH_NUM];
static int import_path_select = 0;
static time_t import_path_time = 0;

static void a_freenames(MENU * pm)
{
    int i;

    for (i = 0; i < pm->num; i++)
        free(pm->item[i]);
}

void a_prompt(bot, pmt, buf)    /* ¾«»ªÇø×´Ì¬ÏÂ ÊäÈë */
int bot;
char *pmt, *buf;
{
    move(t_lines + bot, 0);
    clrtoeol();
    getdata(t_lines + bot, 0, pmt, buf, 39, DOECHO, NULL, true);
}

void a_prompt2(int bot, char *pmt, char *buf)
{                               /* ¾«»ªÇø×´Ì¬ÏÂ ÊäÈë ,°üº¬Ô­À´µÄÄÚÈÝ */
    move(t_lines + bot, 0);
    clrtoeol();
    getdata(t_lines + bot, 0, pmt, buf, 39, DOECHO, NULL, false);
}

void a_report(s)                /* Haohmaru.99.12.06 */
char *s;
{
    /*
     * disable it because of none using it , KCN,2002.07.31 
     */
    return;
/*	
    if((fd = open("a_trace",O_WRONLY|O_CREAT,0644)) != -1 ) {
        char buf[512] ;
        char timestr[24], *thetime;
        time_t dtime;
        time(&dtime);
        thetime = ctime(&dtime);
        strncpy(timestr, thetime, 20);
        timestr[20] = '\0';
        flock(fd,LOCK_EX) ;
        lseek(fd,0,SEEK_END) ;
        sprintf(buf,"%s %s %s\n",currentuser->userid, timestr, s) ;
        write(fd,buf,strlen(buf)) ;
        flock(fd,LOCK_UN) ;
        close(fd) ;
        return ;
    }
*/
}

static void free_import_path()
{
    int i;

    for (i = 0; i < ANNPATH_NUM; i++) {
        if (import_path[i] != NULL) {
            free(import_path[i]);
            import_path[i] = NULL;
        }
        if (import_title[i] != NULL) {
            free(import_title[i]);
            import_title[i] = NULL;
        }
    }
    import_path_time = 0;
}
static int save_import_path()
{
    FILE *fn;
    int i;
    char buf[MAXPATH];

    sethomefile(buf, currentuser->userid, "BMpath");
    fn = fopen(buf, "wt");
    if (fn) {
        struct stat st;

        for (i = 0; i < ANNPATH_NUM; i++) {
            fputs(import_path[i], fn);
            fputs("\n", fn);
            fputs(import_title[i], fn);
            fputs("\n", fn);
        }
        fstat(fileno(fn), &st);
        fclose(fn);
        import_path_time = st.st_mtime;
        return 0;
    }
    return -1;
}

static void load_import_path()
{
    FILE *fn;
    char buf[MAXPATH];
    int i;
    struct stat st;

    sethomefile(buf, currentuser->userid, "BMpath");
    if (stat(buf, &st) != -1)
        if (st.st_mtime == import_path_time)
            return;
    if (import_path_select != 0)
        free_import_path();
    fn = fopen(buf, "rt");
    if (fn) {
        import_path_time = st.st_mtime;
        for (i = 0; i < ANNPATH_NUM; i++) {
            if (!feof(fn)) {
                fgets(buf, MAXPATH - 1, fn);
                if (buf[strlen(buf) - 1] == '\n')
                    buf[strlen(buf) - 1] = 0;
            } else
                buf[0] = 0;
            /*
             * TODO: access check need complete!
             * if (buf[0]!=0&&(ann_traverse_check(buf, currentuser)!=0))
             * buf[0]=0;  can't access 
             */

            import_path[i] = (char *) malloc(strlen(buf) + 1);
            strcpy(import_path[i], buf);
            if (!feof(fn)) {
                fgets(buf, MAXPATH - 1, fn);
                if (buf[strlen(buf) - 1] == '\n')
                    buf[strlen(buf) - 1] = 0;
            } else {            //get the title of pm
                buf[0] = 0;
                if (import_path[i][0]) {
                    MENU pm;

                    bzero(&pm, sizeof(pm));
                    pm.path = import_path[i];
                    a_loadnames(&pm);
                    strncpy(buf, pm.mtitle, MAXPATH - 1);
                    buf[MAXPATH - 1] = 0;
                    a_freenames(&pm);
                }
            }
            if (import_path[i][0] == 0) /* if invalid path,then let the title empty */
                buf[0] = 0;
            import_title[i] = (char *) malloc(strlen(buf) + 1);
            strcpy(import_title[i], buf);
        }
        fclose(fn);
    } else {
        for (i = 0; i < ANNPATH_NUM; i++) {
            import_path[i] = (char *) malloc(1);
            import_path[i][0] = 0;
            import_title[i] = (char *) malloc(1);
            import_title[i][0] = 0;
        }
        save_import_path();
    }
    import_path_select = 1;
}


typedef struct {
    bool save_mode;             /* in save mode,path need valid */
    int tmpnum;
    bool show_path;
} a_select_path_arg;

static int a_select_path_onselect(struct _select_def *conf)
{
    a_select_path_arg *arg = (a_select_path_arg *) conf->arg;

    if (arg->save_mode) {       /* check valid path */
        if (import_title[conf->pos - 1][0] == 0 || import_path[conf->pos - 1][0] == 0) {
            bell();
            return SHOW_CONTINUE;
        }
    } else {                    /* confirm replace */
        if (import_title[conf->pos - 1][0] != 0 && import_path[conf->pos - 1][0] != 0) {
            char ans[STRLEN];

            a_prompt(-2, "Òª¸²¸ÇÒÑÓÐµÄË¿Â·Ã´£¿(Y/N) [N]", ans);
            if (toupper(ans[0]) != 'Y')
                return SHOW_REFRESH;
        }
    }
    return SHOW_SELECT;
}

static int a_select_path_show(struct _select_def *conf, int i)
{
    a_select_path_arg *arg = (a_select_path_arg *) conf->arg;

    if (import_title[i - 1][0] != 0)
        if (arg->show_path)
            prints(" %2d   %s", i, import_path[i - 1]);
        else
            prints(" %2d   %s", i, import_title[i - 1]);
    else
        prints(" %2d   \x1b[32m<ÉÐÎ´Éè¶¨>\x1b[m", i);
    return SHOW_CONTINUE;
}

static int a_select_path_prekey(struct _select_def *conf, int *key)
{
    a_select_path_arg *arg = (a_select_path_arg *) conf->arg;

    if ((*key == '\r' || *key == '\n') && (arg->tmpnum != 0)) {
        conf->new_pos = arg->tmpnum;
        arg->tmpnum = 0;
        return SHOW_SELCHANGE;
    }

    if (!isdigit(*key))
        arg->tmpnum = 0;

    switch (*key) {
    case 'e':
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

static int a_select_path_key(struct _select_def *conf, int key)
{
    a_select_path_arg *arg = (a_select_path_arg *) conf->arg;
    int oldmode;

    if (key >= '0' && key <= '9') {
        arg->tmpnum = arg->tmpnum * 10 + (key - '0');
        return SHOW_CONTINUE;
    }

    switch (key) {
    case 'h':
    case 'H':
        show_help("help/import_announcehelp");
        return SHOW_REFRESH;
    case 'R':
    case 'r':
        free_import_path();
        load_import_path();
        return SHOW_DIRCHANGE;
    case 'a':
    case 'A':
        arg->show_path = !arg->show_path;
        return SHOW_DIRCHANGE;
    case 'T':
    case 't':
        if (import_path[conf->pos - 1][0] != 0) {
            char new_title[STRLEN];

            strcpy(new_title, import_title[conf->pos - 1]);
            a_prompt2(-2, "ÐÂÃû³Æ: ", new_title);
            if (new_title[0] != 0) {
                free(import_title[conf->pos - 1]);
                new_title[STRLEN - 1] = 0;
                import_title[conf->pos - 1] = (char *) malloc(strlen(new_title) + 1);
                strcpy(import_title[conf->pos - 1], new_title);
                save_import_path();
                return SHOW_DIRCHANGE;
            }
            return SHOW_REFRESH;
        }
        break;
    case 'D':
    case 'd':
        if (import_title[conf->pos - 1][0] != 0) {
            char ans[STRLEN];

            a_prompt(-2, "ÒªÉ¾³ýÕâ¸öË¿Â·£¿(Y/N)[N]", ans);
            if (toupper(ans[0]) == 'Y') {
                free(import_title[conf->pos - 1]);
                import_title[conf->pos - 1] = (char *) malloc(1);
                import_title[conf->pos - 1][0] = 0;
                save_import_path();
                return SHOW_DIRCHANGE;
            }
            return SHOW_REFRESH;
        }
        break;
    case 'M':
    case 'm':
        {
            char ans[STRLEN];

            a_prompt(-2, "ÊäÈëÒªÒÆ¶¯µ½µÄÎ»ÖÃ£º", ans);
            if ((ans[0] != 0) && isdigit(ans[0])) {
                int new_pos;

                new_pos = atoi(ans);
                if ((new_pos >= 1) && (new_pos <= ANNPATH_NUM) && (new_pos != conf->pos)) {
                    char *tmp;

                    tmp = import_title[conf->pos - 1];
                    import_title[conf->pos - 1] = import_title[new_pos - 1];
                    import_title[new_pos - 1] = tmp;
                    tmp = import_path[conf->pos - 1];
                    import_path[conf->pos - 1] = import_path[new_pos - 1];
                    import_path[new_pos - 1] = tmp;
                    save_import_path();
                    conf->pos = new_pos;
                    return SHOW_DIRCHANGE;
                }
            }
            return SHOW_REFRESH;
        }
        break;
    case Ctrl('Z'):
        oldmode = uinfo.mode;
        r_lastmsg();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'L':
        oldmode = uinfo.mode;
        show_allmsgs();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'W':
        oldmode = uinfo.mode;
        if (!HAS_PERM(currentuser, PERM_PAGE))
            break;
        s_msg();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'U':
        oldmode = uinfo.mode;
        clear();
        modify_user_mode(QUERY);
        t_query(NULL);
        modify_user_mode(oldmode);
        clear();
        return SHOW_REFRESH;
    }
    return SHOW_CONTINUE;
}

static int a_select_path_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[Ë¿Â·Ñ¡Ôñ²Ëµ¥]",
               "ÍË³ö[\x1b[1;32m¡û\x1b[0;37m,\x1b[1;32me\x1b[0;37m] ½øÈë[\x1b[1;32mEnter\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ý\x1b[0;37m] Ìí¼Ó[\x1b[1;32ma\x1b[0;37m] ¸ÄÃû[\x1b[1;32mT\x1b[0;37m] É¾³ý[\x1b[1;32md\x1b[0;37m]\x1b[m ÒÆ¶¯[\x1b[1;32mm\x1b[0;37m]°ïÖú[\x1b[1;32mh\x1b[0;37m]\x1b[m");
    move(2, 0);
    prints("[0;1;37;44m %4s   %-64s[m", "±àºÅ", "Ë¿Â·Ãû");
    update_endline();
    return SHOW_CONTINUE;
}

/* select a announce path
     return the index in import_path
     		1 base,0=error
     author: KCN
     */
static int a_select_path(bool save_mode)
{
    int i;
    struct _select_def pathlist_conf;
    POINT *pts;
    a_select_path_arg arg;

    clear();
    load_import_path();
    arg.save_mode = save_mode;
    arg.show_path = false;
    arg.tmpnum = 0;
    pts = (POINT *) malloc(sizeof(POINT) * ANNPATH_NUM);
    for (i = 0; i < 20; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    bzero(&pathlist_conf, sizeof(struct _select_def));
    pathlist_conf.item_count = ANNPATH_NUM;
    pathlist_conf.item_per_page = 20;
    /*
     * ¼ÓÉÏ LF_VSCROLL ²ÅÄÜÓÃ LEFT ¼üÍË³ö 
     */
    pathlist_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    pathlist_conf.prompt = "¡ô";
    pathlist_conf.item_pos = pts;
    pathlist_conf.arg = &arg;
    pathlist_conf.title_pos.x = 0;
    pathlist_conf.title_pos.y = 0;
    pathlist_conf.pos = import_path_select;
    pathlist_conf.page_pos = 1; /* initialize page to the first one */

    pathlist_conf.on_select = a_select_path_onselect;
    pathlist_conf.show_data = a_select_path_show;
    pathlist_conf.pre_key_command = a_select_path_prekey;
    pathlist_conf.key_command = a_select_path_key;
    pathlist_conf.show_title = a_select_path_refresh;
    pathlist_conf.get_data = NULL;

    i = list_select_loop(&pathlist_conf);
    free(pts);
    if (i == SHOW_SELECT)
        return pathlist_conf.pos;
    else
        return 0;
}

int valid_fname(str)
char *str;
{
    char ch;

    while ((ch = *str++) != '\0') {
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || strchr("0123456789@[]-._", ch) != NULL) {
            ;
        } else {
            return 0;
        }
    }
    return 1;
}

void a_showmenu(pm)             /* ¾«»ªÇø ²Ëµ¥ ×´Ì¬ */
MENU *pm;
{
    struct stat st;
    struct tm *pt;
    char title[MAXPATH], kind[20];
    char fname[STRLEN];
    char ch;
    char buf[MAXPATH], genbuf[MAXPATH];
    time_t mtime;
    int n;
    int chkmailflag = 0;

    clear();
    chkmailflag = chkmail();

    if (chkmailflag == 2) {     /*Haohmaru.99.4.4.¶ÔÊÕÐÅÒ²¼ÓÏÞÖÆ */
        prints("[5m");
        sprintf(genbuf, "[ÄúµÄÐÅÏä³¬¹ýÈÝÁ¿,²»ÄÜÔÙÊÕÐÅ!]");
    } else if (chkmailflag) {
        prints("[5m");
        sprintf(genbuf, "[ÄúÓÐÐÅ¼þ]");
    } else
        strncpy(genbuf, pm->mtitle, MAXPATH);
    if (strlen(genbuf) <= 80)
        sprintf(buf, "%*s", (80 - strlen(genbuf)) / 2, "");
    else
        strcpy(buf, "");
    prints("[44m%s%s%s[m\n", buf, genbuf, buf);
    prints("            F ¼Ä»Ø×Ô¼ºµÄÐÅÏä©§¡ü¡ý ÒÆ¶¯©§¡ú <Enter> ¶ÁÈ¡©§¡û,q Àë¿ª[m\n");
    prints("[44m[37m ±àºÅ  %-45s Õû  Àí           %8s [m", "[Àà±ð] ±ê    Ìâ", a_fmode == 2 ? "µµ°¸Ãû³Æ" : "±à¼­ÈÕÆÚ");
    prints("\n");
    if (pm->num == 0)
        prints("      << Ä¿Ç°Ã»ÓÐÎÄÕÂ >>\n");
    for (n = pm->page; n < pm->page + 19 && n < pm->num; n++) {
        strncpy(title, pm->item[n]->title, STRLEN * 2 - 1);
        snprintf(fname, STRLEN, "%s", pm->item[n]->fname);
        snprintf(genbuf, MAXPATH, "%s/%s", pm->path, fname);
        if (a_fmode == 2) {
            ch = (dashf(genbuf) ? ' ' : (dashd(genbuf) ? '/' : ' '));
            fname[10] = '\0';
        } else {
            if (dashf(genbuf) || dashd(genbuf)) {
                stat(genbuf, &st);
                mtime = st.st_mtime;
            } else
                mtime = time(0);

            pt = localtime(&mtime);
            sprintf(fname, "[1m%04d[m.[1m%02d[m.[1m%02d[m", pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday);
            ch = ' ';
        }
        if (pm->item[n]->host != NULL) {
            strcpy(kind, "[[33mÁ¬Ïß[m]");
        } else if (dashf(genbuf)) {
            strcpy(kind, "[[36mÎÄ¼þ[m]");
        } else if (dashd(genbuf)) {
            strcpy(kind, "[Ä¿Â¼]");
        } else {
            strcpy(kind, "[[32m´íÎó[m]");
        }
        if (!strncmp(title, "[Ä¿Â¼] ", 7) || !strncmp(title, "[ÎÄ¼þ] ", 7)
            || !strncmp(title, "[Á¬Ïß] ", 7))
            sprintf(genbuf, "%-s %-55.55s%-s%c", kind, title + 7, fname, ch);
        else
            sprintf(genbuf, "%-s %-55.55s%-s%c", kind, title, fname, ch);
        strncpy(title, genbuf, STRLEN * 2 - 1);
        if (pm->item[n]->attachpos)
            prints("  %3d @%s\n", n + 1, title);
        else
            prints("  %3d  %s\n", n + 1, title);
    }
    clrtobot();
    move(t_lines - 1, 0);
    prints("%s", (pm->level & PERM_BOARDS) ?
           "[31m[44m[°æ  Ö÷]  [33mËµÃ÷ h ©¦ Àë¿ª q,¡û ©¦ ÐÂÔöÎÄÕÂ a ©¦ ÐÂÔöÄ¿Â¼ g ©¦ ÐÞ¸Äµµ°¸ e        [m" :
           "[31m[44m[¹¦ÄÜ¼ü] [33m ËµÃ÷ h ©¦ Àë¿ª q,¡û ©¦ ÒÆ¶¯ÓÎ±ê k,¡ü,j,¡ý ©¦ ¶ÁÈ¡×ÊÁÏ Rtn,¡ú         [m");
}

void a_additem(MENU* pm,char* title,char* fname,char* host,int port,long attachpos)    /* ²úÉúITEM object,²¢³õÊ¼»¯ */
{
    ITEM *newitem;

    if (pm->num < MAXITEMS) {
        newitem = (ITEM *) malloc(sizeof(ITEM));
        strncpy(newitem->title, title, sizeof(newitem->title) - 1);
        if (host != NULL) {
            newitem->host = (char *) malloc(sizeof(char) * (strlen(host) + 1));
            strcpy(newitem->host, host);
        } else
            newitem->host = host;
        newitem->port = port;
        newitem->attachpos = attachpos;
        strncpy(newitem->fname, fname, sizeof(newitem->fname) - 1);
        pm->item[(pm->num)++] = newitem;
    }
}

int a_loadnames(pm)             /* ×°Èë .Names */
MENU *pm;
{
    FILE *fn;
    ITEM litem;
    char buf[PATHLEN], *ptr;
    char hostname[STRLEN];
    struct stat st;

    a_freenames(pm);
    pm->num = 0;
    sprintf(buf, "%s/.Names", pm->path);        /*.Names¼ÇÂ¼²Ëµ¥ÐÅÏ¢ */
    if ((fn = fopen(buf, "r")) == NULL)
        return 0;
    if (fstat(fileno(fn), &st) != -1)
        pm->modified_time = st.st_mtime;
    hostname[0] = '\0';
    while (fgets(buf, sizeof(buf), fn) != NULL) {
        if ((ptr = strchr(buf, '\n')) != NULL)
            *ptr = '\0';
        if (strncmp(buf, "Name=", 5) == 0) {
            strncpy(litem.title, buf + 5, sizeof(litem.title));
            litem.attachpos = 0;
        } else if (strncmp(buf, "Path=", 5) == 0) {
            if (strncmp(buf, "Path=~/", 7) == 0)
                strncpy(litem.fname, buf + 7, sizeof(litem.fname));
            else
                strncpy(litem.fname, buf + 5, sizeof(litem.fname));
            if ((!strstr(litem.title, "(BM: BMS)") || HAS_PERM(currentuser, PERM_BOARDS)) &&
                (!strstr(litem.title, "(BM: SYSOPS)") || HAS_PERM(currentuser, PERM_SYSOP)) && (!strstr(litem.title, "(BM: ZIXIAs)") || HAS_PERM(currentuser, PERM_SECANC))) {
                if (strstr(litem.fname, "!@#$%")) {     /*È¡ host & port */
                    char *ptr1, *ptr2, gtmp[STRLEN];

                    strncpy(gtmp, litem.fname, STRLEN - 1);
                    ptr1 = strtok(gtmp, "!#$%@");
                    strcpy(hostname, ptr1);
                    ptr2 = strtok(NULL, "@");
                    strncpy(litem.fname, ptr2, sizeof(litem.fname) - 1);
                    litem.port = atoi(strtok(NULL, "@"));
                }
                a_additem(pm, litem.title, litem.fname, (strlen(hostname) == 0) ?       /*²úÉúITEM */
                          NULL : hostname, litem.port, litem.attachpos);
            }
            hostname[0] = '\0';
        } else if (strncmp(buf, "# Title=", 8) == 0) {
            if (pm->mtitle[0] == '\0')
                strncpy(pm->mtitle, buf + 8, STRLEN);
        } else if (strncmp(buf, "Host=", 5) == 0) {
            strcpy(hostname, buf + 5);
        } else if (strncmp(buf, "Port=", 5) == 0) {
            litem.port = atoi(buf + 5);
        } else if (strncmp(buf, "Attach=", 7) == 0) {
            litem.attachpos= atoi(buf + 7);
        }
    }
    fclose(fn);
    return 1;
}

int a_savenames(pm)             /*±£´æµ±Ç°MENUµ½ .Names */
MENU *pm;
{
    FILE *fn;
    ITEM *item;
    char fpath[PATHLEN];
    int n;
    struct stat st;

    sprintf(fpath, "%s/.Names", pm->path);
    if (stat(fpath, &st) != -1) {
        if (st.st_mtime != pm->modified_time)
            return -3;
    }
    if ((fn = fopen(fpath, "w")) == NULL)
        return -1;
    fprintf(fn, "#\n");
    if (!strncmp(pm->mtitle, "[Ä¿Â¼] ", 7) || !strncmp(pm->mtitle, "[ÎÄ¼þ] ", 7)
        || !strncmp(pm->mtitle, "[Á¬Ïß] ", 7)) {
        fprintf(fn, "# Title=%s\n", pm->mtitle + 7);
    } else {
        fprintf(fn, "# Title=%s\n", pm->mtitle);
    }
    fprintf(fn, "#\n");
    for (n = 0; n < pm->num; n++) {
        item = pm->item[n];
        if (!strncmp(item->title, "[Ä¿Â¼] ", 7) || !strncmp(item->title, "[ÎÄ¼þ] ", 7)
            || !strncmp(item->title, "[Á¬Ïß] ", 7)) {
            fprintf(fn, "Name=%s\n", item->title + 7);
        } else
            fprintf(fn, "Name=%s\n", item->title);
        fprintf(fn, "Attach=%ld\n", item->attachpos);
        if (item->host != NULL) {
            fprintf(fn, "Host=%s\n", item->host);
            fprintf(fn, "Port=%d\n", item->port);
            fprintf(fn, "Type=1\n");
            fprintf(fn, "Path=%s\n", item->fname);
        } else
            fprintf(fn, "Path=~/%s\n", item->fname);
        fprintf(fn, "Numb=%d\n", n + 1);
        fprintf(fn, "#\n");
    }
    fclose(fn);
    if (stat(fpath, &st) != -1)
        pm->modified_time = st.st_mtime;
    chmod(fpath, 0644);
    return 0;
}

/* a_SeSave ÓÃÀ´É¾³ý´æµ½ÔÝ´æµµÊ±µÄÎÄ¼þÍ·ºÍÎ² Life 1997.4.6 */
int a_SeSave(char *path, char *key, struct fileheader *fileinfo, int nomsg, char *direct, int ent)
{

    char ans[STRLEN];
    FILE *inf, *outf;
    char qfile[STRLEN], filepath[STRLEN];
    char buf[256];
    bool findattach=false;
    struct fileheader savefileheader;

    sprintf(filepath, "tmp/bm.%s", currentuser->userid);
    ans[0]='N';
    if (dashf(filepath)) {
    	if (!nomsg) {
            sprintf(buf, "Òª¸½¼ÓÔÚ¾ÉÔÝ´æµµÖ®ºóÂð?(Y/N/C) [Y]: ");
            a_prompt(-1, buf, ans);
    	}
        if ((ans[0] == 'N' || ans[0] == 'n') && (!nomsg)) {
            ans[0]='N';
        } else if (((ans[0] == 'C' || ans[0] == 'c')) && (!nomsg))
            return 1;
        else {
            ans[0]='Y';
        }
    }
    sprintf(qfile, "boards/%s/%s", key, fileinfo->filename);
    sprintf(filepath, "tmp/se.%s", currentuser->userid);
    outf = fopen(filepath, "w");
    if (*qfile != '\0' && (inf = fopen(qfile, "r")) != NULL) {
        fgets(buf, 256, inf);
        fprintf(outf, "%s", buf);
        fprintf(outf, "\n");
        while (fgets(buf, 256, inf) != NULL)
            if (buf[0] == '\n')
                break;

        while (fgets(buf, 256, inf) != NULL) {
            if (strcmp(buf, "--\n") == 0)
                break;
            if (fileinfo->attachment&&
                !memcmp(buf,ATTACHMMENT_PAD,sizeof(ATTACHMMENT_PAD)-1)) {
                findattach=true;
                break;
            }
            if (buf[250] != '\0')
                strcpy(buf + 250, "\n");
            fprintf(outf, "%s", buf);
        }
        fprintf(outf, "\n\n");
        fclose(inf);
    }

    fclose(outf);
    memcpy(&savefileheader,fileinfo,sizeof(savefileheader));
    savefileheader.attachment=0;
    if (fileinfo->attachment) {
        int fsrc,fdst;
        char *src = (char *) malloc(BLK_SIZ);
        if ((fsrc = open(qfile, O_RDONLY)) >= 0) {
            lseek(fsrc,fileinfo->attachment-1,SEEK_SET);
            sprintf(genbuf,"tmp/bm.%s.attach",currentuser->userid);
            if ((fdst=open(genbuf,O_WRONLY | O_CREAT | O_APPEND, 0600)) >= 0) {
                long ret;
                do {
                    ret = read(fsrc, src, BLK_SIZ);
                    if (ret <= 0)
                        break;
                } while (write(fdst, src, ret) > 0);
                close(fdst);
            }
            close(fsrc);
        }
        free(src);
    }
    if (ans[0]!='N') /*Èç¹ûÐèÒª¸½¼ÓÔÝ´æµµ£¬µ÷ÓÃa_SaveÈ¥±£´æÕýÎÄ¡£*/
    	a_Save(filepath, key, &savefileheader, true,NULL,ent);
    change_post_flag(currBM, currentuser, digestmode, currboard, ent, fileinfo, direct, FILE_IMPORT_FLAG, 0);
    return 1;
}


/* added by netty to handle post saving into (0)Announce */
int a_Save(char *path, char *key, struct fileheader *fileinfo, int nomsg, char *direct, int ent)
{
    char board[STRLEN];
    char ans[STRLEN];
    char buf[256];
    char* filepath;

    sprintf(board, "tmp/bm.%s", currentuser->userid);
    ans[0]='N';
    if (dashf(board)) {
        if (!nomsg) {
            sprintf(buf, "Òª¸½¼ÓÔÚ¾ÉÔÝ´æµµÖ®ºóÂð?(Y/N/C) [Y]: ");
            a_prompt(-1, buf, ans);
        }
        if ((ans[0] == 'N' || ans[0] == 'n') && (!nomsg)) {
            ans[0]='N';
        } else if (((ans[0] == 'C' || ans[0] == 'c')) && (!nomsg))
            return 1;
        else {
            ans[0]='Y';
        }
    } 
    if ((ans[0]=='N')||(ans[0]=='Y'))
    {
        int mode;
        int fsrc,fdst1,fdst2;
        if (ans[0]=='Y')
            mode=O_APPEND;
        else
            mode=O_TRUNC;
        if (path==NULL) {
            sprintf(buf, "boards/%s/%s", key, fileinfo->filename);
            filepath=buf;
        } else filepath=path;
        sprintf(board, "tmp/bm.%s", currentuser->userid);
        if ((fsrc = open(filepath, O_RDONLY)) >= 0) {
            sprintf(genbuf,"tmp/bm.%s.attach",currentuser->userid);
            if ((fdst2=open(board,O_WRONLY | O_CREAT | mode, 0600)) >= 0) {
                int ret;
                char *src = (char *) malloc(BLK_SIZ);
                long saved=0,needsave;
                if ((fdst1=open(genbuf,O_WRONLY | O_CREAT | mode, 0600)) >= 0) {
                    do {
                        /* read content and save (fileinfo->attachment) bytes */
                        ret = read(fsrc, src, BLK_SIZ);
                        if (ret <= 0)
                            break;
                        if (fileinfo->attachment) {
                            if (saved+ret>fileinfo->attachment-1) {
                                needsave=fileinfo->attachment-1-saved;
                            } else needsave=ret;
                        } else needsave=ret;
                        saved+=needsave;
                    } while ((write(fdst2, src, needsave) > 0)&&(needsave==ret));
                    close(fdst2);
                }
                if ((needsave!=ret)&&(ret>0))
                    write(fdst1, src+needsave, ret-needsave);
                if (fileinfo->attachment)
                    /* save attachment */
                    do {
                        ret = read(fsrc, src, BLK_SIZ);
                        if (ret <= 0)
                            break;
                    } while (write(fdst1, src, ret) > 0);
                free(src);
                close(fdst1);
            }
            close(fsrc);
        }
    }
    sprintf(buf, "½« boards/%s/%s ´æÈëÔÝ´æµµ", key, fileinfo->filename);
    if (direct!=NULL)
    change_post_flag(currBM, currentuser, digestmode, currboard, ent, fileinfo, direct, FILE_IMPORT_FLAG, 0);
    a_report(buf);
    if (!nomsg) {
        sprintf(buf, " ÒÑ½«¸ÃÎÄÕÂ´æÈëÔÝ´æµµ, Çë°´ÈÎºÎ¼üÒÔ¼ÌÐø << ");
        a_prompt(-1, buf, ans);
    }
    return 1;
}

/* added by netty to handle post saving into (0)Announce */
int a_Import(path, key, fileinfo, nomsg, direct, ent)
char *path, *key;
struct fileheader *fileinfo;
int nomsg;
char *direct;                   /* Leeward 98.04.15 */
int ent;
{

    char fname[STRLEN], bname[PATHLEN];
    char buf[PATHLEN];
    int ch;
    MENU pm;
    char ans[STRLEN];
    char importpath[MAXPATH];
    int ret;

    ret = 0;
    modify_user_mode(CSIE_ANNOUNCE);
    if (ann_get_path(key, buf, sizeof(buf)) == 0) {
        int i;

        bzero(&pm, sizeof(pm));
        if ((path == NULL) || (path[0] == 0)) {
            i = a_select_path(true);
            if (i == 0)
                return 1;
            import_path_select = i;
            i--;
            if (import_path[i][0] != '\0') {
                pm.path = import_path[i];
                if (path)
                    strcpy(path, import_path[i]);
            } else {
                strcpy(importpath, buf);
                pm.path = importpath;
            }
        } else
            pm.path = path;
        /*
         * if (!nomsg) {
         * sprintf(buf, "½«¸ÃÎÄÕÂ·Å½ø %s,È·¶¨Âð?(Y/N) [N]: ", pm.path);
         * a_prompt(-1, buf, ans);
         * if (ans[0] != 'Y' && ans[0] != 'y')
         * return 2;
         * }
         */
        a_loadnames(&pm);
        ann_get_postfilename(fname, fileinfo, &pm);
        sprintf(bname, "%s/%s", pm.path, fname);
        sprintf(buf, "%-38.38s %s ", fileinfo->title, currentuser->userid);
        a_additem(&pm, buf, fname, NULL, 0, fileinfo->attachment);
        if (a_savenames(&pm) == 0) {
            sprintf(buf, "boards/%s/%s", key, fileinfo->filename);
            f_cp(buf, bname, 0);

            /*
             * Leeward 98.04.15 add below FILE_IMPORTED 
             */
            change_post_flag(currBM, currentuser, digestmode, currboard, ent, fileinfo, direct, FILE_IMPORT_FLAG, 0);
            bmlog(currentuser->userid, currboard, 12, 1);
        } else {
            sprintf(buf, " ÊÕÈë¾«»ªÇøÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
            if (!nomsg)
                a_prompt(-1, buf, ans);
            ret = 3;
        }
        for (ch = 0; ch < pm.num; ch++)
            free(pm.item[ch]);
        return ret;
    }
    return 1;
}

int a_menusearch(path, key, level)
char *path, *key;
int level;
{
    FILE *fn;
    char bname[STRLEN], bpath[STRLEN];
    char buf[PATHLEN], *ptr;
    int len;
    struct stat st;
    struct boardheader fhdr;
    int num;

    if (key == NULL) {
        key = bname;
        a_prompt(-1, "ÊäÈëÓûËÑÑ°Ö®ÌÖÂÛÇøÃû³Æ: ", key);
    }

    setbpath(bpath, key);
    if ((*key == '\0') || (stat(bpath, &st) == -1))     /* ÅÐ¶ÏboardÊÇ·ñ´æÔÚ */
        return 0;
    if (!(st.st_mode & S_IFDIR))
        return 0;
    if ((num = getboardnum(key, &fhdr)) == 0)
        return 0;


    if (check_read_perm(currentuser, &fhdr) == 0)
        return 0;


    len = strlen(key);
    sprintf(buf, "%s/.Search", path);
    if (len > 0 && (fn = fopen(buf, "r")) != NULL) {
        while (fgets(buf, sizeof(buf), fn) != NULL) {
            if (strncasecmp(buf, key, len) == 0 && buf[len] == ':' && (ptr = strtok(&buf[len + 1], " \t\n")) != NULL) {
                sprintf(bname, "%s/%s", path, ptr);
                fclose(fn);
                a_menu("", bname, level, 0);
                return 1;
            }
        }
        /*---	added by period	2000-09-21	---*/
        fclose(fn);
        /*---	---*/
    }
    return 0;
}

void a_forward(path, pitem, mode)
char *path;
ITEM *pitem;
int mode;
{
    struct fileheader fhdr;
    char fname[PATHLEN], *mesg;

    sprintf(fname, "%s/%s", path, pitem->fname);
    if (dashf(fname)) {
        strncpy(fhdr.title, pitem->title, STRLEN);
        strncpy(fhdr.filename, pitem->fname, FILENAME_LEN);
        switch (doforward(path, &fhdr, mode)) {
        case 0:
            mesg = "ÎÄÕÂ×ª¼ÄÍê³É!\n";
            break;
        case -1:
            mesg = "system error!!.\n";
            break;
        case -2:
            mesg = "invalid address.\n";
            break;
        case -552:
            prints
                ("\n[1m[33mÐÅ¼þ³¬³¤£¨±¾Õ¾ÏÞ¶¨ÐÅ¼þ³¤¶ÈÉÏÏÞÎª %d ×Ö½Ú£©£¬È¡Ïû×ª¼Ä²Ù×÷[m[m\n\nÇë¸æÖªÊÕÐÅÈË£¨Ò²Ðí¾ÍÊÇÄú×Ô¼º°É:PP£©£º\n\n*1* Ê¹ÓÃ [1m[33mWWW[m[m ·½Ê½·ÃÎÊ±¾Õ¾£¬ËæÊ±¿ÉÒÔ±£´æÈÎÒâ³¤¶ÈµÄÎÄÕÂµ½×Ô¼ºµÄ¼ÆËã»ú£»\n*2* Ê¹ÓÃ [1m[33mpop3[m[m ·½Ê½´Ó±¾Õ¾ÓÃ»§µÄÐÅÏäÈ¡ÐÅ£¬Ã»ÓÐÈÎºÎ³¤¶ÈÏÞÖÆ¡£\n*3* Èç¹û²»ÊìÏ¤±¾Õ¾µÄ WWW »ò pop3 ·þÎñ£¬ÇëÔÄ¶Á [1m[33mAnnounce[m °æÓÐ¹Ø¹«¸æ¡£\n",
                 MAXMAILSIZE);
            break;
        default:
            mesg = "È¡Ïû×ª¼Ä¶¯×÷.\n";
        }
        prints(mesg);
    } else {
        move(t_lines - 1, 0);
        prints("ÎÞ·¨×ª¼Ä´ËÏîÄ¿.\n");
    }
    pressanykey();
}

void a_newitem(pm, mode)        /* ÓÃ»§´´½¨ÐÂµÄ ITEM */
MENU *pm;
int mode;
{
    char uident[STRLEN];
    char board[STRLEN], title[STRLEN];
    char fname[STRLEN], fpath[PATHLEN], fpath2[PATHLEN];
    char *mesg;
    FILE *pn;
    char ans[STRLEN];
    char buf[255];
    long attachpos=0;

    pm->page = 9999;
    switch (mode) {
    case ADDITEM:
        mesg = "ÇëÊäÈëÐÂÎÄ¼þÖ®Ó¢ÃûÃû³Æ(¿Éº¬Êý×Ö)£º";
        break;
    case ADDGROUP:
        mesg = "ÇëÊäÈëÐÂÄ¿Â¼Ö®Ó¢ÃûÃû³Æ(¿Éº¬Êý×Ö)£º";
        break;
    case ADDMAIL:
        sprintf(board, "tmp/bm.%s", currentuser->userid);
        if (!dashf(board)) {
            sprintf(buf, "°¥Ñ½!! ÇëÏÈÖÁ¸Ã°æ(ÌÖÂÛÇø)½«ÎÄÕÂ´æÈëÔÝ´æµµ! << ");
            a_prompt(-1, buf, ans);
            return;
        }
        mesg = "ÇëÊäÈëÎÄ¼þÖ®Ó¢ÎÄÃû³Æ(¿Éº¬Êý×Ö)£º";
        break;
    }
    a_prompt(-2, mesg, fname);
    if (*fname == '\0')
        return;
    sprintf(fpath, "%s/%s", pm->path, fname);
    if (!valid_fname(fname)) {
        sprintf(buf, "°¥Ñ½!! Ãû³ÆÖ»ÄÜ°üº¬Ó¢ÎÄ¼°Êý×Ö! << ");
        a_prompt(-1, buf, ans);
    } else if (dashf(fpath) || dashd(fpath)) {
        sprintf(buf, "°¥Ñ½!! ÏµÍ³ÄÚÒÑ¾­ÓÐ %s Õâ¸öÎÄ¼þ´æÔÚÁË! << ", fname);
        a_prompt(-1, buf, ans);
    } else {
        mesg = "ÇëÊäÈëÎÄ¼þ»òÄ¿Â¼Ö®ÖÐÎÄÃû³Æ <<  ";
        a_prompt(-1, mesg, title);
        if (*title == '\0')
            return;
        sprintf(buf, "´´½¨ÐÂÎÄ¼þ»òÄ¿Â¼ %s (±êÌâ: %s)", fpath + 17, title);
        a_report(buf);
        switch (mode) {
        case ADDITEM:
            if (-1 == vedit(fpath, 0, NULL))
                return;         /* Leeward 98.06.12 fixes bug */
            chmod(fpath, 0644);
            break;
        case ADDGROUP:
            mkdir(fpath, 0755);
            chmod(fpath, 0755);
            break;
        case ADDMAIL:
            /*
             * sprintf( genbuf, "mv -f %s %s",board, fpath );
             */
            {
                struct stat st;
                int fsrc,fdst;
                f_mv(board, fpath);
                sprintf(fpath2, "tmp/bm.%s.attach", currentuser->userid);
                if ((fsrc = open(fpath2, O_RDONLY)) != NULL) {
                    if ((fdst = open(fpath, O_RDWR , 0600)) >= 0) {
                        char *src = (char *) malloc(BLK_SIZ);
                        long ret;
                        fstat(fdst,&st);
                        if (st.st_size>1) {
                            lseek(fdst,st.st_size-1,SEEK_SET);
			    read(fdst,src,1);
			    if (src[0]!='\n')
                                write(fdst, "\n", 1);
                        }
                        do {
                            ret = read(fsrc, src, BLK_SIZ);
                            if (ret <= 0)
                                break;
                        } while (write(fdst, src, ret) > 0);
                        close(fdst);
                        attachpos=st.st_size;
                        free(src);
                    }
                    close(fsrc);
                }
                unlink(fpath2);
            }
            break;
        }
        if (mode != ADDGROUP)
            sprintf(buf, "%-38.38s %s ", title, currentuser->userid);
        else {
            /*
             * Add by SmallPig 
             */
            if (HAS_PERM(currentuser, PERM_SYSOP || HAS_PERM(currentuser, PERM_ANNOUNCE))) {
                move(1, 0);
                clrtoeol();
                /*
                 * $$$$$$$$ Multi-BM Input, Modified By Excellent $$$$$$$ 
                 */
                getdata(1, 0, "°æÖ÷: ", uident, STRLEN - 1, DOECHO, NULL, true);
                if (uident[0] != '\0')
                    sprintf(buf, "%-38.38s(BM: %s)", title, uident);
                else
                    sprintf(buf, "%-38.38s", title);
            } else
                sprintf(buf, "%-38.38s", title);
        }
        a_additem(pm, buf, fname, NULL, 0, attachpos);
        if (a_savenames(pm) == 0) {
            if (mode == ADDGROUP) {
                sprintf(fpath2, "%s/%s/.Names", pm->path, fname);
                if ((pn = fopen(fpath2, "w")) != NULL) {
                    fprintf(pn, "#\n");
                    fprintf(pn, "# Title=%s\n", buf);
                    fprintf(pn, "#\n");
                    fclose(pn);
                }
            }
            if (mode == ADDMAIL)
                bmlog(currentuser->userid, currboard, 12, 1);
            else
                bmlog(currentuser->userid, currboard, 13, 1);
        } else {
            //retry
            a_loadnames(pm);
            a_additem(pm, buf, fname, NULL, 0, attachpos);
            if (a_savenames(pm) != 0) {
                sprintf(buf, " ÕûÀí¾«»ªÇøÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
                a_prompt(-1, buf, ans);
            }
            a_loadnames(pm);
        }
    }
}

void a_moveitem(pm)             /*¸Ä±ä ITEM ´ÎÐò */
MENU *pm;
{
    ITEM *tmp;
    char newnum[STRLEN];
    int num, n, temp;

    sprintf(genbuf, "ÇëÊäÈëµÚ %d ÏîµÄÐÂ´ÎÐò: ", pm->now + 1);
    temp = pm->now + 1;
    a_prompt(-2, genbuf, newnum);
    num = (newnum[0] == '$') ? 9999 : atoi(newnum) - 1;
    if (num >= pm->num)
        num = pm->num - 1;
    else if (num < 0)
        return;
    tmp = pm->item[pm->now];
    if (num > pm->now) {
        for (n = pm->now; n < num; n++)
            pm->item[n] = pm->item[n + 1];
    } else {
        for (n = pm->now; n > num; n--)
            pm->item[n] = pm->item[n - 1];
    }
    pm->item[num] = tmp;
    pm->now = num;
    if (a_savenames(pm) == 0) {
        sprintf(genbuf, "¸Ä±ä %s ÏÂµÚ %d ÏîµÄ´ÎÐòµ½µÚ %d Ïî", pm->path + 17, temp, pm->now + 1);
        bmlog(currentuser->userid, currboard, 13, 1);
        a_report(genbuf);
    } else {
        char buf[80], ans[40];

        sprintf(buf, " ÕûÀí¾«»ªÇøÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
        a_prompt(-1, buf, ans);
        a_loadnames(pm);
    }
}

void a_copypaste(pm, paste)
MENU *pm;
int paste;
{
    /*
     * KCN 2002.03.22,ÏÂÃæ±äÁ¿µÄstaticÓ¦¸Ã¿ÉÒÔÈ¥µô 
     */
    char title[STRLEN], filename[STRLEN], fpath[PATHLEN];
    ITEM *item;
    char ans[STRLEN], newpath[PATHLEN];
    FILE *fn;                   /* Leeward 98.02.19 */
    long attachpos=0;

    move(t_lines - 2, 0);
    if (paste == 0) {
        item = pm->item[pm->now];
        strncpy(title, item->title, STRLEN);
        strncpy(filename, item->fname, FILENAME_LEN);
        sprintf(genbuf, "%s/%s", pm->path, filename);
        strncpy(fpath, genbuf, PATHLEN);
        prints("¿½±´±êÊ¶Íê³É¡£×¢Òâ£ºÕ³ÌùÎÄÕÂáá²ÅÄÜÓÃ d ÃüÁî½«ÎÄÕÂÉ¾³ý! -- Çë°´ÈÎÒâ¼ü¼ÌÐø << ");
        pressanykey();

        /*
         * Leeward: 98.02.19: ¶Ô°æÖ÷µÄ¶à¸ö´°¿ÚÍ¬²½ C/P ²Ù×÷ 
         */
        sprintf(genbuf, "home/%c/%s/.CP", toupper(currentuser->userid[0]), currentuser->userid);
        fn = fopen(genbuf, "wt");
        if (fn) {
            fputs("0\n", fn);
            fputs(title, fn);
            fputs("\n", fn);
            fputs(filename, fn);
            fputs("\n", fn);
            fputs(fpath, fn);
            fputs("\n", fn);
            fprintf(fn,"%ld\n",item->attachpos);
            fclose(fn);
        } else {
            prints("File open ERROR -- please report SYSOP");
            pressanykey();
        }
    } else if (paste == 1) {    // cut and paste, modified by bad, 03-2-10
        item = pm->item[pm->now];
        strncpy(title, item->title, STRLEN);
        strncpy(filename, item->fname, FILENAME_LEN);
        sprintf(genbuf, "%s/%s", pm->path, filename);
        strncpy(fpath, genbuf, PATHLEN);
        pressanykey();

        /*
         * Leeward: 98.02.19: ¶Ô°æÖ÷µÄ¶à¸ö´°¿ÚÍ¬²½ C/P ²Ù×÷ 
         */
        sprintf(genbuf, "home/%c/%s/.CP", toupper(currentuser->userid[0]), currentuser->userid);
        fn = fopen(genbuf, "wt");
        if (fn) {
            fputs("1\n", fn);
            fputs(title, fn);
            fputs("\n", fn);
            fputs(filename, fn);
            fputs("\n", fn);
            fputs(fpath, fn);
            fputs("\n", fn);
            fprintf(fn,"%ld\n",item->attachpos);
            fclose(fn);
        } else {
            prints("File open ERROR -- please report SYSOP");
            pressanykey();
        }
    } else if (paste == 2) {
        /*
         * Leeward: 98.02.19: ¶Ô°æÖ÷µÄ¶à¸ö´°¿ÚÍ¬²½ C/P ²Ù×÷ 
         */
        int iscut;

        sprintf(genbuf, "home/%c/%s/.CP", toupper(currentuser->userid[0]), currentuser->userid);
        title[0] = 0;
        fn = fopen(genbuf, "rt");
        if (fn) {
            fgets(title, STRLEN, fn);
            if ('\n' == title[strlen(title) - 1])
                title[strlen(title) - 1] = 0;
            iscut = (title[0] == '1');
            fgets(title, STRLEN, fn);
            if ('\n' == title[strlen(title) - 1])
                title[strlen(title) - 1] = 0;
            fgets(filename, STRLEN, fn);
            if ('\n' == filename[strlen(filename) - 1])
                filename[strlen(filename) - 1] = 0;
            fgets(fpath, /*STRLEN*/ PATHLEN, fn);       /* Leeward 98.04.15 */
            if ('\n' == fpath[strlen(fpath) - 1])
                fpath[strlen(fpath) - 1] = 0;
            attachpos=0;
            fgets(ans, STRLEN, fn);       /* Leeward 98.04.15 */
            if ('\n' == ans[strlen(ans) - 1])
                ans[strlen(ans) - 1] = 0;
            attachpos=atol(ans);
            fclose(fn);
        }

        sprintf(newpath, "%s/%s", pm->path, filename);
        if (*title == '\0' || *filename == '\0') {
            prints("ÇëÏÈÊ¹ÓÃ copy/cut ÃüÁîÔÙÊ¹ÓÃ paste ÃüÁî. ");
            pressanykey();
        } else if (dashf(newpath) || dashd(newpath)) {
            prints("%s %s ÒÑ¾­´æÔÚ. ", (dashd(newpath) ? "Ä¿Â¼" : "ÎÄ¼þ"), filename);
            pressanykey();
        } else if (strstr(newpath, fpath) != NULL) {
            prints("ÎÞ·¨½«Ò»¸öÄ¿Â¼°á½ø×Ô¼ºµÄ×ÓÄ¿Â¼ÖÐ, »áÔì³ÉËÀÑ­»·. ");
            pressanykey();
        } else {
            /*
             * modified by cityhunter to simplify annouce c/p 
             */
            if (!iscut)
                sprintf(genbuf, "ÄúÈ·¶¨ÒªÕ³Ìù%s %s Âð? (C/L/N)CÎª¸´ÖÆ·½Ê½ LÎªÁ´½Ó·½Ê½ [N]: ", (dashd(fpath) ? "Ä¿Â¼" : "ÎÄ¼þ"), filename);
            else
                sprintf(genbuf, "ÄúÈ·¶¨Òª¼ôÇÐ%s %s Âð? (Y/N) [N]: ", (dashd(fpath) ? "Ä¿Â¼" : "ÎÄ¼þ"), filename);
            a_prompt(-2, genbuf, ans);
            if ((ans[0] == 'C' || ans[0] == 'c') && (iscut == 0) || (ans[0] == 'Y' || ans[0] == 'y') && (iscut == 1)) {
                char buf[256];

                if (dashd(fpath)) {     /* ÊÇÄ¿Â¼ */
                    sprintf(genbuf, "/bin/cp -rp %s %s", fpath, newpath);
                    system(genbuf);
                } else {        /* ÊÇÎÄ¼þ 
                                 * sprintf( genbuf, "/bin/cp -p %s %s", fpath, newpath ); */
                    f_cp(fpath, newpath, 0);
                }
                if (iscut) {
                    ITEM *item;
                    char uppath[PATHLEN], *pnt;
                    MENU pm2;
                    int n, k;

                    sprintf(genbuf, "home/%c/%s/.CP", toupper(currentuser->userid[0]), currentuser->userid);
                    unlink(genbuf);
                    bzero(&pm2, sizeof(pm2));

                    strncpy(uppath, fpath, PATHLEN);
                    pnt = uppath + strlen(uppath) - 1;
                    while (*pnt != '/')
                        pnt--;
                    *pnt = 0;
                    pnt++;

                    if (dashf(fpath)) {
                        unlink(fpath);
                    } else if (dashd(fpath)) {
                        f_rm(fpath);
                    }

                    pm2.path = uppath;
                    a_loadnames(&pm2);

                    for (k = 0; k < pm2.num; k++)
                        if (!strcmp(pm2.item[k]->fname, filename))
                            break;
                    item = pm2.item[k];
                    free(item);
                    (pm2.num)--;
                    for (n = k; n < pm2.num; n++)
                        pm2.item[n] = pm2.item[n + 1];
                    if (a_savenames(&pm2) == 0) {
                        sprintf(genbuf, "É¾³ýÎÄ¼þ»òÄ¿Â¼: %s", fpath + 17);
                        bmlog(currentuser->userid, currboard, 13, 1);
                        a_report(genbuf);
                    } else {
                        char buf[80], ans[40];

                        sprintf(buf, " É¾³ýÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
                        a_prompt(-1, buf, ans);
                    }
                    a_freenames(&pm2);

                }
                a_additem(pm, title, filename, NULL, 0, attachpos);
                if (a_savenames(pm) == 0) {
                    sprintf(buf, "¸´ÖÆ¾«»ªÇøÎÄ¼þ»òÄ¿Â¼: %s", genbuf);
                    a_report(buf);
                } else {
                    //retry
                    a_loadnames(pm);
                    a_additem(pm, title, filename, NULL, 0, attachpos);
                    if (a_savenames(pm) != 0) {
                        sprintf(buf, " ÕûÀí¾«»ªÇøÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
                        a_prompt(-1, buf, ans);
                    }
                    a_loadnames(pm);
                }
            } else if ((ans[0] == 'L' || ans[0] == 'l') && (iscut == 0)) {
                char buf[256];

                if (dashd(fpath)) {     /* ÊÇÄ¿Â¼ */
                    sprintf(genbuf, "/bin/cp -rp %s %s", fpath, newpath);
                    system(genbuf);
                } else {        /* ÊÇÎÄ¼þ 
                                 * sprintf( genbuf, "/bin/ln %s %s", fpath, newpath ); */
                    f_ln(fpath, newpath);
                }
                a_additem(pm, title, filename, NULL, 0, attachpos);
                if (a_savenames(pm) == 0) {
                    sprintf(buf, "¸´ÖÆ¾«»ªÇøÎÄ¼þ»òÄ¿Â¼: %s", genbuf);
                    a_report(buf);
                } else {
                    //retry
                    a_loadnames(pm);
                    a_additem(pm, title, filename, NULL, 0, attachpos);
                    if (a_savenames(pm) != 0) {
                        sprintf(buf, " ÕûÀí¾«»ªÇøÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
                        a_prompt(-1, buf, ans);
                    }
                    a_loadnames(pm);
                }
            }
        }
        /*
         * sprintf( genbuf, "ÄúÈ·¶¨ÒªÕ³Ìù%s %s Âð? (Y/N) [N]: ", (dashd(fpath) ? "Ä¿Â¼" : "ÎÄ¼þ"), filename);
         * a_prompt( -2, genbuf, ans );
         * if( ans[0] == 'Y' || ans[0] == 'y' ) {
         * if (dashd(fpath))
         * { 
         * sprintf( genbuf, "/bin/cp -rp %s %s", fpath, newpath );
         * }
         * else
         * { 
         * sprintf( genbuf, "Ê¹ÓÃÁ´½Ó·½Ê½(L)»¹ÊÇ¸´ÖÆ·½Ê½(C)£¿Ç°ÕßÄÜ´ó´ó½ÚÊ¡´ÅÅÌ¿Õ¼ä (L/C) [L]: ", filename ); 
         * a_prompt( -2, genbuf, ans );
         * if( ans[0] == 'C' || ans[0] == 'c' ) 
         * sprintf( genbuf, "/bin/cp -p %s %s", fpath, newpath );
         * else
         * sprintf( genbuf, "/bin/ln %s %s", fpath, newpath );
         * }
         * system( genbuf );
         * a_additem( pm, title, filename  ,NULL,0);
         * a_savenames( pm );
         * sprintf(genbuf,"¸´ÖÆ¾«»ªÇøÎÄ¼þ»òÄ¿Â¼: %s",genbuf);
         * a_report(genbuf);
         * }
         * }
         */
    }
    pm->page = 9999;
}

void a_delete(pm)
MENU *pm;
{
    ITEM *item;
    char fpath[PATHLEN];
    char ans[STRLEN];
    int n;

    item = pm->item[pm->now];
    move(t_lines - 2, 0);
    prints("%5d  %-50s\n", pm->now + 1, item->title);
    sprintf(fpath, "%s/%s", pm->path, item->fname);
    if (dashf(fpath)) {
        a_prompt(-1, "É¾³ý´ËÎÄ¼þ, È·¶¨Âð?(Y/N) [N]£º", ans);
        if (ans[0] != 'Y' && ans[0] != 'y')
            return;
        unlink(fpath);
    } else if (dashd(fpath)) {
        a_prompt(-1, "É¾³ýÕû¸ö×ÓÄ¿Â¼, ±ð¿ªÍæÐ¦Å¶, È·¶¨Âð?(Y/N) [N]: ", ans);
        if (ans[0] != 'Y' && ans[0] != 'y')
            return;
        /*
         * sprintf( genbuf, "/bin/rm -rf %s", fpath ); 
         */
        f_rm(fpath);
    }
    free(item);
    (pm->num)--;
    for (n = pm->now; n < pm->num; n++)
        pm->item[n] = pm->item[n + 1];
    if (a_savenames(pm) == 0) {
        sprintf(genbuf, "É¾³ýÎÄ¼þ»òÄ¿Â¼: %s", fpath + 17);
        bmlog(currentuser->userid, currboard, 13, 1);
        a_report(genbuf);
    } else {
        char buf[80], ans[40];

        sprintf(buf, " É¾³ýÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
        a_prompt(-1, buf, ans);
        a_loadnames(pm);
    }
}

void a_newname(pm)
MENU *pm;
{
    ITEM *item;
    char fname[STRLEN];
    char fpath[PATHLEN];
    char *mesg;

    item = pm->item[pm->now];
    a_prompt(-2, "ÐÂÎÄ¼þÃû: ", fname);
    if (*fname == '\0')
        return;
    sprintf(fpath, "%s/%s", pm->path, fname);
    if (!valid_fname(fname)) {
        mesg = "²»ºÏ·¨ÎÄ¼þÃû³Æ.";
    } else if (dashf(fpath) || dashd(fpath)) {
        mesg = "ÏµÍ³ÖÐÒÑÓÐ´ËÎÄ¼þ´æÔÚÁË.";
    } else {
        sprintf(genbuf, "%s/%s", pm->path, item->fname);
        strcpy(item->fname, fname);
        if (a_savenames(pm) == 0) {
            if (f_mv(genbuf, fpath) == 0) {
                char r_buf[256];

                sprintf(r_buf, "¸ü¸ÄÎÄ¼þÃû: %s -> %s", genbuf + 17, fpath + 17);
                a_report(r_buf);
                return;
            }
        } else {
            char buf[80], ans[40];

            sprintf(buf, "ÕûÀí¾«»ªÇøÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
            a_prompt(-1, buf, ans);
            a_loadnames(pm);
        }
        mesg = "ÎÄ¼þÃû¸ü¸ÄÊ§°Ü !!";
    }
    prints(mesg);
    pressanykey();
}

void a_manager(pm, ch)
MENU *pm;
int ch;
{
    char uident[STRLEN];
    ITEM *item;
    char fpath[PATHLEN], changed_T[STRLEN], ans[STRLEN];

    if (pm->num > 0) {
        item = pm->item[pm->now];
        sprintf(fpath, "%s/%s", pm->path, item->fname);
    }
    switch (ch) {
    case 'a':
        a_newitem(pm, ADDITEM);
        break;
    case 'g':
        a_newitem(pm, ADDGROUP);
        break;
    case 'i':
        a_newitem(pm, ADDMAIL);
        break;
        /*
         * case 'G':  a_newitem( pm, ADDGOPHER );    break; 
         */
    case 'p':
        a_copypaste(pm, 2);
        break;
    case 'f':{
            int i;

            pm->page = 9999;
            i = a_select_path(false);
            if (i == 0)
                break;

            import_path_select = i;
            i--;
            free(import_path[i]);
            import_path[i] = malloc(strlen(pm->path) + 1);
            strcpy(import_path[i], pm->path);
            free(import_title[i]);
            strcpy(ans, pm->mtitle);
            move(t_lines - 2, 0);
            clrtoeol();
            getdata(t_lines - 2, 0, "Éè¶¨Ë¿Â·Ãû:", ans, STRLEN - 1, DOECHO, NULL, false);
            import_title[i] = malloc(strlen(ans) + 1);
            strcpy(import_title[i], ans);
            save_import_path();
        }
        break;
    }
    if (pm->num > 0)
        switch (ch) {
        case 's':
            if (++a_fmode >= 3)
                a_fmode = 1;
            pm->page = 9999;
            break;
        case 'm':
            a_moveitem(pm);
            pm->page = 9999;
            break;
        case 'd':
            a_delete(pm);
            pm->page = 9999;
            break;
        case 'V':
        case 'v':
            if (HAS_PERM(currentuser, PERM_SYSOP)) {
                if (ch == 'v')
                    sprintf(fpath, "%s/.Names", pm->path);
                else
                    sprintf(fpath, "0Announce/.Search");        /*.Search¿ØÖÆ¸÷°æ¶ÔÓ¦µÄ¾«»ªÇø */

                if (dashf(fpath)) {
                    modify_user_mode(EDITANN);
                    vedit(fpath, 0, NULL);
                    modify_user_mode(CSIE_ANNOUNCE);
                }
                pm->page = 9999;
            }
            break;
        case 't':
            strncpy(changed_T, item->title, 39);
            changed_T[39] = 0;
            {
                char *p;

                p = changed_T + strlen(changed_T) - 1;
                for (; p >= changed_T; p--) {
                    if (*p == ' ')
                        *p = 0;
                    else
                        break;
                };
            }
            a_prompt2(-2, "ÐÂ±êÌâ: ", changed_T);
            /*
             * modified by netty to properly handle title change,add bm by SmallPig 
             */
            if (*changed_T) {
                if (dashf(fpath)) {
                    sprintf(genbuf, "%-38.38s %s ", changed_T, currentuser->userid);
                    strcpy(item->title, genbuf);
                    sprintf(genbuf, "¸Ä±äÎÄ¼þ %s µÄ±êÌâ", fpath + 17);
                    a_report(genbuf);
                } else if (dashd(fpath)) {
                    if (HAS_PERM(currentuser, PERM_SYSOP || HAS_PERM(currentuser, PERM_ANNOUNCE))) {
                        move(1, 0);
                        clrtoeol();
                        /*
                         * usercomplete("°æÖ÷: ",uident) ; 
                         */
                        /*
                         * $$$$$$$$ Multi-BM Input, Modified By Excellent $$$$$$$ 
                         */
                        getdata(1, 0, "°æÖ÷: ", uident, STRLEN - 1, DOECHO, NULL, true);
                        if (uident[0] != '\0')
                            sprintf(genbuf, "%-38.38s(BM: %s)", changed_T, uident);
                        else
                            sprintf(genbuf, "%-38.38s", changed_T);
                    } else
                        sprintf(genbuf, "%-38.38s", changed_T);

                    strcpy(item->title, genbuf);
                    sprintf(genbuf, "¸Ä±äÄ¿Â¼ %s µÄ±êÌâ", fpath + 17);
                    a_report(genbuf);
                }
                if (a_savenames(pm) != 0) {
                    char buf[80], ans[40];

                    sprintf(buf, "ÕûÀí¾«»ªÇøÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
                    a_prompt(-1, buf, ans);
                    a_loadnames(pm);
                }
            }
            pm->page = 9999;
            break;
        case 'e':
            if (dashf(fpath)) {
                modify_user_mode(EDITANN);
                vedit(fpath, 0, NULL);
                modify_user_mode(CSIE_ANNOUNCE);
                sprintf(genbuf, "ÐÞ¸ÄÎÄÕÂ %s µÄÄÚÈÝ", pm->path + 17);
                a_report(genbuf);
            }
            pm->page = 9999;
            break;
        case 'n':
            a_newname(pm);
            pm->page = 9999;
            break;
        case 'c':
            a_copypaste(pm, 0);
            break;
        case 'x':              //added by bad 03-2-10
            a_copypaste(pm, 1);
            break;
/*  do not support thread read in announce: COMMAN 2002.7
        case '=':  t_search_down();     break;
        case '+':  t_search_up();       break;
*/
        }
}

void a_menu(maintitle, path, lastlevel, lastbmonly)
char *maintitle, *path;
int lastlevel, lastbmonly;
{
    MENU me;
    char fname[PATHLEN], tmp[STRLEN];
    int ch;
    char *bmstr;
    char buf[STRLEN];
    int bmonly;
    int number = 0;

    bzero(&me, sizeof(me));
    modify_user_mode(CSIE_ANNOUNCE);
    me.path = path;
    strcpy(me.mtitle, maintitle);
    me.level = lastlevel;
    bmonly = lastbmonly;
    a_loadnames(&me);           /* Load .Names */

    strcpy(buf, me.mtitle);
    bmstr = strstr(buf, "(BM:");
    if (bmstr != NULL) {
        if (chk_currBM(bmstr + 4, currentuser) || HAS_PERM(currentuser, PERM_SYSOP))
            me.level |= PERM_BOARDS;
        else if (bmonly == 1 && !(me.level & PERM_BOARDS))
            return;
    }

    if (strstr(me.mtitle, "(BM: BMS)") || strstr(me.mtitle, "(BM: SECRET)") || strstr(me.mtitle, "(BM: SYSOPS)"))
        bmonly = 1;

    strcpy(buf, me.mtitle);
    bmstr = strstr(buf, "(BM:");

    me.page = 9999;
    me.now = 0;
    while (1) {
        if (me.now >= me.num && me.num > 0) {
            me.now = me.num - 1;
        } else if (me.now < 0) {
            me.now = 0;
        }
        if (me.now < me.page || me.now >= me.page + A_PAGESIZE) {
            me.page = me.now - (me.now % A_PAGESIZE);
            a_showmenu(&me);
        }
        move(3 + me.now - me.page, 0);
        prints("->");
        ch = igetkey();
        move(3 + me.now - me.page, 0);
        prints("  ");
        if (ch == 'Q' || ch == 'q' || ch == KEY_LEFT || ch == EOF)
            break;
      EXPRESS:                 /* Leeward 98.09.13 */
        switch (ch) {
        case KEY_REFRESH:
            a_showmenu(&me);
            break;
        case Ctrl('Z'):
            r_lastmsg();        /* Leeward 98.07.30 support msgX */
            break;
        case KEY_UP:
        case 'K':
        case 'k':
            if (--me.now < 0)
                me.now = me.num - 1;
            break;
        case KEY_DOWN:
        case 'J':
        case 'j':
            if (++me.now >= me.num)
                me.now = 0;
            break;
        case KEY_PGUP:
        case Ctrl('B'):
            if (me.now >= A_PAGESIZE)
                me.now -= A_PAGESIZE;
            else if (me.now > 0)
                me.now = 0;
            else
                me.now = me.num - 1;
            break;
        case KEY_PGDN:
        case Ctrl('F'):
        case ' ':
            if (me.now < me.num - A_PAGESIZE)
                me.now += A_PAGESIZE;
            else if (me.now < me.num - 1)
                me.now = me.num - 1;
            else
                me.now = 0;
            break;
        case Ctrl('C'):
        case Ctrl('P'):
            if (!HAS_PERM(currentuser, PERM_POST))
                break;
            if (!me.item[me.now])
                break;
            sprintf(fname, "%s/%s", path, me.item[me.now]->fname);
            if (!dashf(fname))
                break;
            if (me.now < me.num) {
                char bname[30];

                clear();
                move(1, 0);
                if (get_a_boardname(bname, "ÇëÊäÈëÒª×ªÌùµÄÌÖÂÛÇøÃû³Æ: ")) {
                    move(1, 0);
                    clrtoeol();
                    strcpy(tmp, currboard);
                    strcpy(currboard, bname);
                    if (deny_me(currentuser->userid, currboard)) {
                        prints("¶Ô²»Æð£¬ÄãÔÚ %s °æ±»Í£Ö¹·¢±íÎÄÕÂµÄÈ¨Á¦", bname);
                        pressreturn();
                        strcpy(currboard, tmp);
                        me.page = 9999;
                        break;
                    }
                    if (!haspostperm(currentuser, currboard)) {
                        move(1, 0);
                        prints("ÄúÉÐÎÞÈ¨ÏÞÔÚ %s ·¢±íÎÄÕÂ.\n", currboard);
                        prints("Èç¹ûÄúÉÐÎ´×¢²á£¬ÇëÔÚ¸öÈË¹¤¾ßÏäÄÚÏêÏ¸×¢²áÉí·Ý\n");
                        prints("Î´Í¨¹ýÉí·Ý×¢²áÈÏÖ¤µÄÓÃ»§£¬Ã»ÓÐ·¢±íÎÄÕÂµÄÈ¨ÏÞ¡£\n");
                        prints("Ð»Ð»ºÏ×÷£¡ :-) \n");
                        pressreturn();
                        strcpy(currboard, tmp);
                        me.page = 9999;
                        break;
                    }
                    if (check_readonly(currboard)) {
                        strcpy(currboard, tmp);
                        me.page = 9999;
                        break;
                    }
                    strcpy(currboard, tmp);
                    sprintf(tmp, "ÄãÈ·¶¨Òª×ªÌùµ½ %s °æÂð", bname);
                    if (askyn(tmp, 0) == 1) {
                        post_file(currentuser, "", fname, bname, me.item[me.now]->title, 0, 2);
                        move(2, 0);
                        sprintf(tmp, "[1mÒÑ¾­°ïÄã×ªÌùÖÁ %s °æÁË[m", bname);
                        prints(tmp);
                        refresh();
                        sleep(1);
                    }
                }
                me.page = 9999;
            }
            show_message(NULL);
            break;
        case 'h':
            show_help("help/announcereadhelp");
            me.page = 9999;
            break;
        case '\n':
        case '\r':
            if (number > 0) {
                me.now = number - 1;
                number = 0;
                continue;
            }
        case 'R':
        case 'r':
        case KEY_RIGHT:
            if (me.now < me.num) {
                if (me.item[me.now]->host != NULL) {
                    /*
                     * gopher(me.item[ me.now ]->host,me.item[ me.now ]->fname,
                     * me.item[ me.now ]->port,me.item[ me.now ]->title); 
                     */
                    me.page = 9999;
                    break;
                } else
                    sprintf(fname, "%s/%s", path, me.item[me.now]->fname);
                if (dashf(fname)) {
                    /*
                     * ansimore( fname, true ); 
                     */
                    /*
                     * Leeward 98.09.13 ÐÂÌí¹¦ÄÜ¡Ã
                     * £¬ÓÃÉÏ£¯ÏÂ¼ýÍ·Ö±½ÓÌø×ªµ½Ç°£¯ºóÒ»Ïî 
                     */
                    ansimore_withzmodem(fname, false, me.item[me.now]->title);
                    prints("[1m[44m[31m[ÔÄ¶Á¾«»ªÇø×ÊÁÏ]  [33m½áÊø Q,¡û ©¦ ÉÏÒ»Ïî×ÊÁÏ U,¡ü©¦ ÏÂÒ»Ïî×ÊÁÏ <Enter>,<Space>,¡ý [m");
                    switch (ch = igetkey()) {
                    case KEY_DOWN:
                    case ' ':
                    case '\n':
                        if (++me.now >= me.num)
                            me.now = 0;
                        ch = KEY_RIGHT;
                        goto EXPRESS;
                    case KEY_UP:
                        if (--me.now < 0)
                            me.now = me.num - 1;
                        ch = KEY_RIGHT;
                        goto EXPRESS;
                    case Ctrl('Y'):
                        zsend_file(fname, me.item[me.now]->title);
                        break;
                    case Ctrl('Z'):
                    case 'h':
                        goto EXPRESS;
                    default:
                        break;
                    }
                } else if (dashd(fname)) {
                    a_menu(me.item[me.now]->title, fname, me.level, bmonly);
                    a_loadnames(&me);   /* added by bad 03-2-10 */
                }
                me.page = 9999;
            }
            break;
        case '/':
            if (a_menusearch(path, NULL, me.level))
                me.page = 9999;
            break;
        case 'F':
        case 'U':
            if (me.now < me.num && HAS_PERM(currentuser, PERM_BASIC)) {
                a_forward(path, me.item[me.now], ch == 'U');
                me.page = 9999;
            }
            break;
        case 'o':
            t_friends();
            me.page = 9999;
            break;              /*Haohmaru 98.09.22 */
        case 'u':
            clear();
            modify_user_mode(QUERY);
            t_query(NULL);
            modify_user_mode(CSIE_ANNOUNCE);
            me.page = 9999;
            break;              /*Haohmaru.99.11.29 */
        case '!':
            Goodbye();
            me.page = 9999;
            break;              /*Haohmaru 98.09.24 */
            /*
             * case 'Z':
             * if( me.now < me.num && HAS_PERM(currentuser, PERM_BASIC ) ) {
             * sprintf( fname, "%s/%s", path, me.item[ me.now ]->fname );
             * a_download( fname );
             * me.page = 9999;
             * }
             * break;
             */
        case Ctrl('Y'):
            if (me.now < me.num) {
                if (me.item[me.now]->host != NULL) {
                    /*
                     * gopher(me.item[ me.now ]->host,me.item[ me.now ]->fname,
                     * me.item[ me.now ]->port,me.item[ me.now ]->title); 
                     */
                    me.page = 9999;
                    break;
                } else
                    sprintf(fname, "%s/%s", path, me.item[me.now]->fname);
                if (dashf(fname)) {
                    zsend_file(fname, me.item[me.now]->title);
                    me.page = 9999;
                }
            }
            break;
        }
        if (ch >= '0' && ch <= '9') {
            number = number * 10 + (ch - '0');
            ch = '\0';
        } else {
            number = 0;
        }
        if (me.level & PERM_BOARDS)
            a_manager(&me, ch);
    }
    for (ch = 0; ch < me.num; ch++)
        free(me.item[ch]);
}

int linkto(char *path, char *fname, char *title)
{
    MENU pm;

    pm.path = path;

    strcpy(pm.mtitle, title);
    a_loadnames(&pm);
    a_additem(&pm, title, fname, NULL, 0, 0);
    if (a_savenames(&pm) != 0) {
        char buf[80], ans[40];

        sprintf(buf, "ÕûÀí¾«»ªÇøÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
        a_prompt(-1, buf, ans);
    }
    a_freenames(&pm);
    return 0;
}

/*
 * ²ÎÊýËµÃ÷:
 *     group    groups[]ÖÐµÄÄ³Ïî
 *     bname    °æµÄÓ¢ÎÄÃû
 *     title    °æ¾«»ªÇøµÄÖÐÎÄÃû
 *     gname    Óë group ¶ÔÓ¦µÄÖÐÎÄÃû
 */
int add_grp(char group[STRLEN], char bname[STRLEN], char title[STRLEN], char gname[STRLEN])
        /*
         * ¾«»ªÇø ¼Ó Ä¿Â¼ 
         */
{
    FILE *fn;
    char buf[PATHLEN];
    char searchname[STRLEN];
    char gpath[STRLEN * 2];
    char bpath[STRLEN * 2];

    sprintf(buf, "0Announce/.Search");
    sprintf(searchname, "%s: groups/%s/%s", bname, group, bname);
    sprintf(gpath, "0Announce/groups/%s", group);
    sprintf(bpath, "%s/%s", gpath, bname);
    if (!dashd("0Announce")) {
        mkdir("0Announce", 0755);
        chmod("0Announce", 0755);
        if ((fn = fopen("0Announce/.Names", "w")) == NULL)
            return -1;
        fprintf(fn, "#\n");
        fprintf(fn, "# Title=%s ¾«»ªÇø¹«²¼À¸\n", BBS_FULL_NAME);
        fprintf(fn, "#\n");
        fclose(fn);
    }
    if (!dashd("0Announce/groups")) {
        mkdir("0Announce/groups", 0755);
        chmod("0Announce/groups", 0755);

        linkto("0Announce", "groups", "ÌÖÂÛÇø¾«»ª");
    }
    if (!dashd(gpath)) {
        mkdir(gpath, 0755);
        chmod(gpath, 0755);
        linkto("0Announce/groups", group, gname);
    }
    if (!dashd(bpath)) {
        mkdir(bpath, 0755);
        chmod(bpath, 0755);
        linkto(gpath, bname, title);
        sprintf(buf, "%s/.Names", bpath);
        if ((fn = fopen(buf, "w")) == NULL) {
            return -1;
        }
        fprintf(fn, "#\n");
        fprintf(fn, "# Title=%s\n", title);
        fprintf(fn, "#\n");
        fclose(fn);
    }
    return 0;
}

int del_grp(bname, title)
char bname[STRLEN], title[STRLEN];
{
    char buf2[STRLEN];
    char gpath[STRLEN * 2];
    char bpath[STRLEN * 2];
    char check[30];
    char *ptr;
    int i, n;
    MENU pm;

    bzero(&pm, sizeof(pm));
    /*
     * »ñÈ¡¸Ã°æÔÚ¾«»ªÇøÖÐµÄÂ·¾¶ 
     */
    if (ann_get_path(bname, gpath, sizeof(gpath)) < 0)
        return 0;
    snprintf(bpath, sizeof(bpath), "0Announce/%s", gpath);
    strcpy(gpath, bpath);
    /*
     * »ñÈ¡¸Ã°æ¶ÔÓ¦ group µÄÂ·¾¶ 
     */
    if ((ptr = strrchr(gpath, '/')) == NULL)
        return 0;
    if (strncmp(bname, ptr + 1, strlen(bname)) != 0)
        return 0;
    *ptr = '\0';

    f_rm(bpath);

    pm.path = gpath;
    a_loadnames(&pm);
    for (i = 0; i < pm.num; i++) {
        strcpy(buf2, pm.item[i]->title);
        strcpy(check, strtok(pm.item[i]->fname, "/~\n\b"));
        if (strstr(buf2, title) && !strcmp(check, bname)) {
            free(pm.item[i]);
            (pm.num)--;
            for (n = i; n < pm.num; n++)
                pm.item[n] = pm.item[n + 1];
            if (a_savenames(&pm) != 0) {
                char buf[80], ans[40];

                sprintf(buf, "ÕûÀí¾«»ªÇøÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
                a_prompt(-1, buf, ans);
                a_loadnames(&pm);
            }
            break;
        }
    }
    a_freenames(&pm);
    return 0;                   /* FIXME: return value */
}

int edit_grp(char bname[STRLEN], char title[STRLEN], char newtitle[100])
{
    char buf2[STRLEN];
    char gpath[STRLEN * 2];
    char bpath[STRLEN * 2];
    char *ptr;
    int i;
    MENU pm;

    bzero(&pm, sizeof(pm));
    /*
     * »ñÈ¡¸Ã°æÔÚ¾«»ªÇøÖÐµÄÂ·¾¶ 
     */
    if (ann_get_path(bname, gpath, sizeof(gpath)) < 0)
        return 0;
    snprintf(bpath, sizeof(bpath), "0Announce/%s", gpath);
    strcpy(gpath, bpath);
    /*
     * »ñÈ¡¸Ã°æ¶ÔÓ¦ group µÄÂ·¾¶ 
     */
    if ((ptr = strrchr(gpath, '/')) == NULL)
        return 0;
    if (strncmp(bname, ptr + 1, strlen(bname)) != 0)
        return 0;
    *ptr = '\0';

    pm.path = gpath;
    a_loadnames(&pm);
    for (i = 0; i < pm.num; i++) {
        strcpy(buf2, pm.item[i]->title);
        if (strstr(buf2, title) && strstr(pm.item[i]->fname, bname)) {
            strcpy(pm.item[i]->title, newtitle);
            break;
        }
    }
    if (a_savenames(&pm) != 0) {
        char buf[80], ans[40];

        sprintf(buf, "ÕûÀí¾«»ªÇøÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
        a_prompt(-1, buf, ans);
        a_loadnames(&pm);
    }
    pm.path = bpath;
    a_loadnames(&pm);
    strncpy(pm.mtitle, newtitle, STRLEN);
    if (a_savenames(&pm) != 0) {
        char buf[80], ans[40];

        sprintf(buf, "ÕûÀí¾«»ªÇøÊ§°Ü£¬¿ÉÄÜÓÐÆäËû°æÖ÷ÔÚ´¦ÀíÍ¬Ò»Ä¿Â¼£¬°´ Enter ¼ÌÐø ");
        a_prompt(-1, buf, ans);
        a_loadnames(&pm);
    }

    a_freenames(&pm);
    return 0;                   /* FIXME: return value */
}

void Announce()
{
    sprintf(genbuf, "%s ¾«»ªÇø¹«²¼À¸", BBS_FULL_NAME);
    a_menu(genbuf, "0Announce", HAS_PERM(currentuser, PERM_ANNOUNCE) ? PERM_BOARDS : 0, 0);
    clear();
}
