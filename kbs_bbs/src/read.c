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
*/

#include "bbs.h"
/* COMMAN : use mmap to speed up searching */
#include <unistd.h>
#include <sys/mman.h>

struct fileheader SR_fptr;
int SR_BMDELFLAG = false;
int B_to_b = false;


/*---	Modified by period	2000-11-12	---*
char *pnt;
 *---	current code memory leak ---*/
extern char MsgDesUid[14];
extern unsigned int tmpuser;
struct keeploc {
    char *key;
    int top_line;
    int crs_line;
    struct keeploc *next;
};
inline static void PUTCURS(struct keeploc *locmem)
{
    move(3 + locmem->crs_line - locmem->top_line, 0);
    prints(">");
}

inline static void RMVCURS(struct keeploc *locmem)
{
    move(3 + locmem->crs_line - locmem->top_line, 0);
    prints(" ");
}

static int search_articles(struct keeploc *locmem, char *query, int offset, int aflag);
static int search_author(struct keeploc *locmem, int offset, char *powner);
static int search_post(struct keeploc *locmem, int offset);
static int search_title(struct keeploc *locmem, int offset);
static int i_read_key(int cmdmode, struct one_key *rcmdlist, struct keeploc *locmem, int ch, int ssize, char* pnt,char *ding_direct, char *direct);
static int cursor_pos(struct keeploc *locmem, int val, int from_top);
static int search_thread(struct keeploc *locmem, int offset, char *title);
static int search_threadid(struct keeploc *locmem, int offset, int groupid, int mode);


/*struct fileheader *files = NULL;*/
char currdirect[255];           /*COMMAN: increased directory length to MAX_PATH */
int screen_len;
int last_line;


int search_file(char *filename)
{
    char p_name[256];
    int i = 0;
    size_t size;
    struct fileheader *rptr, *rptr1;

    if (uinfo.mode != RMAIL)
        setbdir(digestmode, p_name, currboard->filename);
    else
        setmailfile(p_name, currentuser->userid, DOT_DIR);
    BBS_TRY {
        if (safe_mmapfile(p_name, O_RDONLY, PROT_READ, MAP_SHARED, (void *) &rptr, (size_t *) & size, NULL) == 0)
            BBS_RETURN(-1);
        for (i = 0, rptr1 = rptr; i < (int) (size / sizeof(struct fileheader)); i++, rptr1++)
            if (!strcmp(filename, rptr1->filename)) {
                end_mmapfile((void *) rptr, size, -1);
                BBS_RETURN(i);
            }
    }
    BBS_CATCH {
    }
    BBS_END end_mmapfile((void *) rptr, size, -1);
    return -1;
}

struct keeploc *getkeep(char *s, int def_topline, int def_cursline)
{
    static struct keeploc *keeplist = NULL;
    struct keeploc *p;

    for (p = keeplist; p != NULL; p = p->next) {
        if (!strcmp(s, p->key)) {
            if (p->crs_line < 1)
                p->crs_line = 1;        /* DAMMIT! - rrr */
            return p;
        }
    }
    p = (struct keeploc *) malloc(sizeof(*p));
    p->key = (char *) malloc(strlen(s) + 1);
    strcpy(p->key, s);
    p->top_line = def_topline;
    p->crs_line = def_cursline; /* this should be safe */
    p->next = keeplist;
    keeplist = p;
    return p;
}


void fixkeep(char *s, int first, int last)
{
    struct keeploc *k;

    k = getkeep(s, 1, 1);
    if (k->crs_line >= first) {
        k->crs_line = (first == 1 ? 1 : first - 1);
        k->top_line = (first < 11 ? 1 : first - 10);
    }
}


void modify_locmem(struct keeploc *locmem, int total)
{
    if (locmem->top_line > total) {
        locmem->crs_line = total;
        locmem->top_line = total - t_lines / 2;
        if (locmem->top_line < 1)
            locmem->top_line = 1;
    } else if (locmem->crs_line > total) {
        locmem->crs_line = total;
    }
}


int move_cursor_line(struct keeploc *locmem, int mode)
{
    int top, crs;
    int reload = 0;

    top = locmem->top_line;
    crs = locmem->crs_line;
    if (mode == READ_PREV) {
        if (crs <= top) {
            top -= screen_len - 1;
            if (top < 1)
                top = 1;
            reload = 1;
        }
        crs--;
        if (crs < 1) {
            crs = 1;
            reload = -1;
        }
    } else if (mode == READ_NEXT) {
        if (crs + 1 >= top + screen_len) {
            top += screen_len - 1;
            reload = 1;
        }
        crs++;
        if (crs > last_line) {
            crs = last_line;
            reload = -1;
        }
    }
    locmem->top_line = top;
    locmem->crs_line = crs;
    return reload;
}

static void draw_title(void (*dotitle) ())
{
    move(0, 0);
    (*dotitle) ();
}

/*---	Modified by period	2000-11-12	---*
void
draw_entry( doentry, locmem, num ,ssize)
char            *(*doentry)();
struct keeploc  *locmem;
int             num,ssize;
---*/
void draw_entry(READ_FUNC doentry, struct keeploc *locmem, int num, int ssize, char *pnt)
{
    char *str;
    int base, i;
    char foroutbuf[512];

    base = locmem->top_line;
    for (i = 0; i < num; i++) {
        move(i+3, 0);
        str = (*doentry) (foroutbuf, base + i, &pnt[i * ssize]);
	/*
        if (!check_stuffmode())
	*/
            prints("%s", str);
/*
        else
            showstuff(str);
	    */
        clrtoeol();
    }
    for(i=num+3; i<t_lines-1; i++) {
        move(i, 0);
        clrtoeol();
    }
    move(t_lines - 1, 0);
    update_endline();
}

static int search_author(struct keeploc *locmem, int offset, char *powner)
{
    static char author[IDLEN + 1];
    char ans[IDLEN + 1], pmt[STRLEN];
    char currauth[STRLEN];
    strncpy(currauth, powner, STRLEN);
    snprintf(pmt, STRLEN, "%sµÄÎÄÕÂËÑÑ°×÷Õß [%s]: ", offset > 0 ? "ÍùºóÀ´" : "ÍùÏÈÇ°", currauth);
    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, pmt, ans, IDLEN + 1, DOECHO, NULL, true);   /*Haohmaru.98.09.29.ÐÞÕý×÷Õß²éÕÒÖ»ÄÜ11Î»IDµÄ´íÎó */
    if (ans[0] != '\0')
        strncpy(author, ans, IDLEN);

    else
        strcpy(author, currauth);
    return search_articles(locmem, author, offset, 1);
}

int i_read(int cmdmode, char *direct, void (*dotitle) (), READ_FUNC doentry, struct one_key *rcmdlist, int ssize)
{
    char lbuf[11], lastfile[256];
    int num, entries, recbase;
    struct keeploc *locmem;

    int lbc, mode, ch;
    char *pnt;

    /* add by stiger*/
    char ding_direct[PATHLEN];
    /* add end */

    /* add by stiger */
    if ((cmdmode != RMAIL && cmdmode != GMENU)&&
        (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
  	  	sprintf(ding_direct,"boards/%s/%s",currboard->filename,DING_DIR);
	else
		ding_direct[0]='\0';
    /* add end */

    /*---	Moved from top of file	period	2000-11-12	---*/

    strncpy(currdirect, direct, 255);   /* COMMAN: strncpy */

    /*---	HERE:	---*/
    screen_len = t_lines - 4;
    if (TDEFINE(TDEF_SPLITSCREEN)&&cmdmode!=GMENU)
        screen_len = screen_len/2-1;
    modify_user_mode(cmdmode);
    pnt = calloc(t_lines, ssize);
    draw_title(dotitle);
    last_line = get_num_records(currdirect, ssize);
    /* add by stiger */
    if ((cmdmode != RMAIL && cmdmode != GMENU)&&
        (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    last_line += get_num_records(ding_direct,ssize);
    /* add end */
    if (last_line == 0) {
        if (cmdmode == RMAIL) {
            prints("Ã»ÓÐÈÎºÎÐÅ¼þ...");
            pressreturn();
            clear();
        }

        else if (cmdmode == GMENU) {
            getdata(t_lines - 1, 0, "Ã»ÓÐÈÎºÎºÃÓÑ (A)ÐÂÔöºÃÓÑ (Q)Àë¿ª£¿[Q] ", genbuf, 4, DOECHO, NULL, true);
            if (genbuf[0] == 'a' || genbuf[0] == 'A')
                friend_add(0, NULL, 0);
        }

        else {
	    if (digestmode!=DIR_MODE_NORMAL) 
		    digestmode=DIR_MODE_NORMAL;
	    else {
            getdata(t_lines - 1, 0, "ÐÂ°æ¸Õ³ÉÁ¢ (P)·¢±íÎÄÕÂ (Q)Àë¿ª£¿[Q] ", genbuf, 4, DOECHO, NULL, true);
            if (genbuf[0] == 'p' || genbuf[0] == 'P')
                do_post();
	    }
        }
        free(pnt);
        pnt = NULL;
        return;
    }
    num = last_line - screen_len + 2;
    locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);
    modify_locmem(locmem, last_line);

    if(locmem->crs_line-locmem->top_line>=screen_len) /*added by bad 2002.9.2*/
        locmem->crs_line = locmem->top_line;
    
    recbase = locmem->top_line;
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    entries = read_get_records(currdirect, ding_direct,pnt, ssize, recbase, screen_len);
    else
    /* add end */
    entries = get_records(currdirect, pnt, ssize, recbase, screen_len);

    /*---	Modofied by period	2000-11-12	---*
    draw_entry( doentry, locmem, entries ,ssize);
     *---			---*/
    draw_entry(doentry, locmem, entries, ssize, pnt);
    if (TDEFINE(TDEF_SPLITSCREEN)&&cmdmode!=GMENU){
        char buf[256], *t;
        struct fileheader* h;
        strcpy(buf, currdirect);
        if ((t = strrchr(buf, '/')) != NULL)
            *t = '\0';
        h = &pnt[(locmem->crs_line - locmem->top_line) * ssize];
        sprintf(genbuf, "%s/%s", buf, h->filename);
        strcpy(lastfile, genbuf);
        draw_content(genbuf,h);
        update_endline();
    }
    PUTCURS(locmem);
    lbc = 0;
    mode = DONOTHING;
    while ((ch = igetkey()) != EOF) {
#ifndef NINE_BUILD
    	if ((ch==KEY_TIMEOUT)&&(TDEFINE(TDEF_SPLITSCREEN)&&cmdmode!=GMENU)) {
            char buf[256], *t;
            struct fileheader* h;
            strcpy(buf, currdirect);
            if ((t = strrchr(buf, '/')) != NULL)
                *t = '\0';
            h = &pnt[(locmem->crs_line - locmem->top_line) * ssize];
            sprintf(genbuf, "%s/%s", buf, h->filename);
            if (strcmp(genbuf,lastfile)) {
            	draw_content(genbuf,h);
            update_endline();
            	strcpy(lastfile, genbuf);
            }
            move(0, 0);
            (*dotitle) ();
            PUTCURS(locmem);
	    continue;
    	} else 
#endif
    	if (ch == KEY_REFRESH) {
            mode = FULLUPDATE;

            /*
             * } else if( ch >= '0' && ch <= '9' ) {
             * if( lbc < 9 )
             * talkreply();
             * lbuf[ lbc++ ] = ch;
                                                                                                                                   *//*---	Modified by period	2000-09-11	---*/
        } else if ((ch >= '0' && ch <= '9')
                   || ((Ctrl('H') == ch || '\177' == ch) && (lbc > 0))) {
            if (Ctrl('H') == ch || '\177' == ch)
                lbuf[lbc--] = 0;

            else if (lbc < 9)
                lbuf[lbc++] = ch;
            lbuf[lbc] = 0;
            if (!lbc)
                update_endline();

            else if (DEFINE(currentuser, DEF_ENDLINE)) {
                extern time_t login_start_time;
                int allstay;
                char pntbuf[256], nullbuf[2] = " ";

                allstay = (time(0) - login_start_time) / 60;
                snprintf(pntbuf, 256, "\033[33;44m×ªµ½¡Ã[\033[36m%9.9s\033[33m]" "  ºô½ÐÆ÷[ºÃÓÑ:%3s¡ÃÒ»°ã:%3s] Ê¹ÓÃÕß[\033[36m%.12s\033[33m]%sÍ£Áô[%3d:%2d]\033[m", lbuf, (!(uinfo.pager & FRIEND_PAGER)) ? "NO " : "YES", (uinfo.pager & ALL_PAGER) ? "YES" : "NO ", currentuser->userid,      /*13-strlen(currentuser->userid)
                                                                                                                                                                                                                                                                                                 * TODO:Õâ¸öµØ·½ÓÐÎÊÌâ£¬ËûÏë¶ÔÆë£¬µ«ÊÇ´úÂë²»¶Ô
                                                                                                                                                                                                                                                                                                 * , */ nullbuf,
                         (allstay / 60) % 1000, allstay % 60);
                move(t_lines - 1, 0);
                prints(pntbuf);
                clrtoeol();
            }

            /*---		---*/
        } else if (lbc > 0 && (ch == '\n' || ch == '\r')) {

            /*---	2000-09-11	---*/
            update_endline();

            /*---	---*/
            lbuf[lbc] = '\0';
            lbc = atoi(lbuf);
            if (cursor_pos(locmem, lbc, screen_len/2))
                mode = PARTUPDATE;
            lbc = 0;
        } else {

            /*---	2000-09-11	---*/
            if (lbc)
                update_endline();

            /*---	---*/
            lbc = 0;

            /*---	Modified by period	2000-11-12	---*
                   mode = i_read_key( rcmdlist, locmem, ch ,ssize);
             *---		---*/
            mode = i_read_key(cmdmode, rcmdlist, locmem, ch, ssize, pnt,ding_direct,currdirect);
            while (mode == READ_NEXT || mode == READ_PREV) {
                int reload;

                reload = move_cursor_line(locmem, mode);
                if (reload == -1) {
                    mode = FULLUPDATE;
                    break;
                } else if (reload) {
                    recbase = locmem->top_line;
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    entries = read_get_records(currdirect,ding_direct, pnt, ssize, recbase, screen_len);
    else
    /* add end */
                    entries = get_records(currdirect, pnt, ssize, recbase, screen_len);
                    if (entries <= 0) {
                        last_line = -1;
                        break;
                    }
                }
                num = locmem->crs_line - locmem->top_line;

                /*---	Modified by period	2000-11-12	---*
                              mode = i_read_key( rcmdlist, locmem, ch ,ssize);
                 *---		---*/
                mode = i_read_key(cmdmode, rcmdlist, locmem, ch, ssize, pnt,ding_direct,currdirect);
            }
            modify_user_mode(cmdmode);
        }
        if ((mode == DOQUIT)||(mode == CHANGEMODE))
            break;
        if (mode == GOTO_NEXT) {
            cursor_pos(locmem, locmem->crs_line + 1, 1);
            mode = PARTUPDATE;
        }
        if (mode == NEWSCREEN) {
            last_line = get_num_records(currdirect, ssize);
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    last_line += get_num_records(ding_direct,ssize);
    /* add end */
            num = last_line - screen_len + 2;
            locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);
            modify_locmem(locmem, last_line);
            if(locmem->crs_line-locmem->top_line>=screen_len-1) /*added by bad 2002.9.2*/
                locmem->crs_line = locmem->top_line;
    
            recbase = locmem->top_line;
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    entries = read_get_records(currdirect,ding_direct, pnt, ssize, recbase, screen_len);
    else
    /* add end */
            entries = get_records(currdirect, pnt, ssize, recbase, screen_len);

            mode = FULLUPDATE;
        }
        switch (mode) {
        case NEWDIRECT:
        case DIRCHANGED:
    if ((cmdmode != RMAIL && cmdmode != GMENU)&&
        (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
    	sprintf(ding_direct,"boards/%s/%s",currboard->filename,DING_DIR);
	else
		ding_direct[0]='\0';
            recbase = -1;
            last_line = get_num_records(currdirect, ssize);
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    last_line += get_num_records(ding_direct,ssize);
    /* add end */
            if (last_line == 0 && digestmode > 0) {
                if (digestmode == 7 || digestmode == 8)
                    unlink(currdirect);
                digestmode = 0;
                setbdir(digestmode, currdirect, currboard->filename);
            }
            if (mode == NEWDIRECT) {
                num = last_line - screen_len + 1;
                locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);
            }
        case FULLUPDATE:
        case PARTUPDATE:
            draw_title(dotitle);
            if (last_line < locmem->top_line + screen_len) {
                num = get_num_records(currdirect, ssize);
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    num += get_num_records(ding_direct,ssize);
    /* add end */
                if (last_line != num) {
                    last_line = num;
                    recbase = -1;
                }
            }
            if (last_line == 0) {
                prints("No Messages\n");
                entries = 0;
            } else if (recbase != locmem->top_line) {
                recbase = locmem->top_line;
                if (recbase > last_line) {
                    recbase = last_line - screen_len / 2;
                    if (recbase < 1)
                        recbase = 1;
                    locmem->top_line = recbase;
                }
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    entries = read_get_records(currdirect,ding_direct, pnt, ssize, recbase, screen_len);
    else
    /* add end */
                entries = get_records(currdirect, pnt, ssize, recbase, screen_len);
            }
            if (locmem->crs_line > last_line)
                locmem->crs_line = last_line;

            move(3, 0);
            draw_entry(doentry, locmem, entries, ssize, pnt);
            if (TDEFINE(TDEF_SPLITSCREEN)&&cmdmode!=GMENU) {
#ifdef NINE_BUILD
            char buf[256], *t;
            struct fileheader* h;
            strcpy(buf, currdirect);
            if ((t = strrchr(buf, '/')) != NULL)
                *t = '\0';
            h = &pnt[(locmem->crs_line - locmem->top_line) * ssize];
            sprintf(genbuf, "%s/%s", buf, h->filename);
            draw_content(genbuf,h);
            update_endline();
#else
                set_alarm(0,300*1000,NULL,NULL);
		lastfile[0]=0;
#endif
	    }
            PUTCURS(locmem);
            break;

        default:
            if (TDEFINE(TDEF_SPLITSCREEN)&&cmdmode!=GMENU) /*added by bad 2002.9.2*/ {
#ifdef NINE_BUILD
                char buf[256], *t;
                struct fileheader* h;
                strcpy(buf, currdirect);
                if ((t = strrchr(buf, '/')) != NULL)
                    *t = '\0';
                h = &pnt[(locmem->crs_line - locmem->top_line) * ssize];
                sprintf(genbuf, "%s/%s", buf, h->filename);
                draw_content(genbuf,h);
                update_endline();
#else
                set_alarm(0,300*1000,NULL,NULL);
#endif
            }
            RMVCURS(locmem);
            PUTCURS(locmem);
            break;
        }
        mode = DONOTHING;
        if (entries == 0)
            break;
    }
    clear();
    free(pnt);
    pnt = NULL;
    return mode;
}


static int i_read_key(int cmdmode, struct one_key *rcmdlist, struct keeploc *locmem, int ch, int ssize, char* pnt, char* ding_direct,char *direct)
{
    int i, mode = DONOTHING;

    switch (ch) {
    case Ctrl('Z'):
        r_lastmsg();            /* Leeward 98.07.30 support msgX */
        break;

    case 'q':
    case 'e':
    case KEY_LEFT:
        if (digestmode > 0) {
            if (digestmode == 7 || digestmode == 8)
                unlink(currdirect);
            digestmode = 0;
            setbdir(digestmode, currdirect, currboard->filename);
            return NEWDIRECT;
        }
        else
            return DOQUIT;
    case Ctrl('L'):
        redoscr();
        break;
    case 'k':
    case KEY_UP:
        if (cursor_pos(locmem, locmem->crs_line - 1, screen_len - 2))
            return PARTUPDATE;
        break;
    case 'j':
    case KEY_DOWN:
        if (cursor_pos(locmem, locmem->crs_line + 1, 0))
            return PARTUPDATE;
        break;
    case 'N':
    case Ctrl('F'):
    case KEY_PGDN:
    case ' ':
        if (last_line >= locmem->top_line + screen_len) {
            locmem->top_line += screen_len - 1;
            locmem->crs_line = locmem->top_line;
            return PARTUPDATE;
        }
        RMVCURS(locmem);
        locmem->crs_line = last_line;
        PUTCURS(locmem);
        break;
    case 'P':
    case Ctrl('B'):
    case KEY_PGUP:
        if (locmem->top_line > 1) {
            locmem->top_line -= screen_len - 1;
            if (locmem->top_line <= 0)
                locmem->top_line = 1;
            locmem->crs_line = locmem->top_line;
            return PARTUPDATE;
        } else {
            RMVCURS(locmem);
            locmem->crs_line = locmem->top_line;
            PUTCURS(locmem);
        }
        break;
    case KEY_HOME:
        locmem->top_line = 1;
        locmem->crs_line = 1;
        return PARTUPDATE;
    case '$':
    case KEY_END:
        if (last_line >= locmem->top_line + screen_len) {
            locmem->top_line = last_line - screen_len + 1;
            if (locmem->top_line <= 0)
                locmem->top_line = 1;
	/*modified by stiger */
/*            locmem->crs_line = last_line; */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
            locmem->crs_line = last_line - get_num_records(ding_direct,ssize);
	else
            locmem->crs_line = last_line;
	/*modified by stiger end */
            return PARTUPDATE;
        }
        RMVCURS(locmem);
	/*modified by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
        locmem->crs_line = last_line - get_num_records(ding_direct,ssize);
	else
        locmem->crs_line = last_line;
	/*modified by stiger end */
		if( (locmem->crs_line - locmem->top_line) <= 0){
			locmem->top_line = last_line - screen_len + 1;
			return PARTUPDATE;
		}
        PUTCURS(locmem);
        break;
    case 'L':
    case 'l':                  /* Luzi 1997.10.31 */
        if (uinfo.mode != LOOKMSGS) {
            show_allmsgs();
            return FULLUPDATE;
            break;
        }

        else
            return DONOTHING;
    case 'w':                  /* Luzi 1997.10.31 */
        if (!HAS_PERM(currentuser, PERM_PAGE))
            break;
        s_msg();
        return FULLUPDATE;
        break;
    case 'u':                  /* Haohmaru. 99.11.29 */
        clear();
        modify_user_mode(QUERY);
        t_query(NULL);
        return FULLUPDATE;
        break;
    case 'O':
    case 'o':                  /* Luzi 1997.10.31 */
        {                       /* Leeward 98.10.26 fix a bug by saving old mode */
            int savemode = uinfo.mode;

            if (!HAS_PERM(currentuser, PERM_BASIC))
                break;
            t_friends();
            modify_user_mode(savemode);
            return FULLUPDATE;
            break;
        }
    case ',':
        if(uinfo.mode==GMENU) break;
        if (TDEFINE(TDEF_SPLITSCREEN))
        	tmpuser&=~TDEF_SPLITSCREEN;
        else
        	tmpuser|=TDEF_SPLITSCREEN;
        screen_len = t_lines - 4;
        if (TDEFINE(TDEF_SPLITSCREEN))
            screen_len = screen_len/2-1;

/*        num = last_line - screen_len + 2;
//        locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);
//        modify_locmem(locmem, last_line);*/

        return NEWSCREEN;
        break;
    case '!':                  /*Haohmaru 1998.09.24 */
        Goodbye();
        return FULLUPDATE;
        break;
    case '\n':
    case '\r':
    case KEY_RIGHT:
            ch = 'r';

        /*
         * lookup command table 
         */
    default:
        for (i = 0; rcmdlist[i].fptr != NULL; i++) {
            if (rcmdlist[i].key == ch) {
                if (cmdmode != RMAIL && cmdmode != GMENU
                     && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD)) {
		    /* add by stiger */
        		if( POSTFILE_BASENAME(((fileheader *)(pnt+(locmem->crs_line-locmem->top_line)*ssize))->filename)[0]=='Z' ){
        		    if(ch=='D' || ch=='b' || ch=='B') return DONOTHING;
        		    else if(ch=='s')
                            mode = (*(rcmdlist[i].fptr)) (locmem->crs_line - get_num_records(currdirect, ssize), &pnt[(locmem->crs_line - locmem->top_line) * ssize], direct );
        		    else
                            mode = (*(rcmdlist[i].fptr)) (locmem->crs_line - get_num_records(currdirect, ssize), &pnt[(locmem->crs_line - locmem->top_line) * ssize], ding_direct );
        		}
        		else
                            mode = (*(rcmdlist[i].fptr)) (locmem->crs_line, &pnt[(locmem->crs_line - locmem->top_line) * ssize], direct);
                }
                else
                      mode = (*(rcmdlist[i].fptr)) (locmem->crs_line, &pnt[(locmem->crs_line - locmem->top_line) * ssize], direct);
                break;
            }
        }
    }
    return mode;
}


int auth_search_down(int ent, struct fileheader *fileinfo, char *direct)
{
    struct keeploc *locmem;
	int i;

	if(strstr(direct,DING_DIR)) locmem=getkeep(currdirect,1,1);
	else locmem = getkeep(direct, 1, 1);

    i = search_author(locmem, 1, fileinfo->owner);
    if (i == 2)
		return DIRCHANGED;
	else if (i == 1)
        return PARTUPDATE;
    else
        update_endline();
    return DONOTHING;
}


int auth_search_up(int ent, struct fileheader *fileinfo, char *direct)
{
    struct keeploc *locmem;
	int i;

	if(strstr(direct,DING_DIR)) locmem=getkeep(currdirect,1,1);
	else locmem = getkeep(direct, 1, 1);

    i = search_author(locmem, -1, fileinfo->owner);
    if (i == 2)
		return DIRCHANGED;
	else if (i == 1)
        return PARTUPDATE;
    else
        update_endline();
    return DONOTHING;
}

int post_search_down(int ent, struct fileheader *fileinfo, char *direct)
{
    struct keeploc *locmem;
	int i;

	if(strstr(direct,DING_DIR)) locmem=getkeep(currdirect,1,1);
	else locmem = getkeep(direct, 1, 1);
    
	i = search_post(locmem, 1);
    if (i == 2)
		return NEWDIRECT;
	else if(i==1)
        return PARTUPDATE;
    else
        update_endline();
    return DONOTHING;
}

int post_search_up(int ent, struct fileheader *fileinfo, char *direct)
{
    struct keeploc *locmem;
	int i;

	if(strstr(direct,DING_DIR)) locmem=getkeep(currdirect,1,1);
	else locmem = getkeep(direct, 1, 1);

	i = search_post(locmem, -1);
    if (i == 2)
		return NEWDIRECT;
	else if(i==1)
        return PARTUPDATE;
    else
        update_endline();
    return DONOTHING;
}


/*Haohmaru.98.12.05.ÏµÍ³¹ÜÀíÔ±¿ÉÒÔÖ±½Ó²é×÷Õß×ÊÁÏ*/
int show_authorinfo(int ent, struct fileheader *fileinfo, char *direct)
{
    struct userec uinfo;
    struct userec *lookupuser;
    int id;

    if (!HAS_PERM(currentuser, PERM_ACCOUNTS)
        || !strcmp(fileinfo->owner, "Anonymous")
        || !strcmp(fileinfo->owner, "deliver"))
        return DONOTHING;

    else {
        if (0 == (id = getuser(fileinfo->owner, &lookupuser))) {
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

int sendmsgtoauthor(int ent, struct fileheader *fileinfo, char *direct)
{
    struct user_info *uin;

    if (!HAS_PERM(currentuser, PERM_PAGE))
        return DONOTHING;
    clear();
    uin = (struct user_info *) t_search(fileinfo->owner, false);
    if (!uin || !canmsg(currentuser, uin))
        do_sendmsg(NULL, NULL, 0);

    else {
        strncpy(MsgDesUid, uin->userid, 20);
        do_sendmsg(uin, NULL, 0);
    }
    return FULLUPDATE;
}


int show_author(int ent, struct fileheader *fileinfo, char *direct)
{
    if ( /*strchr(fileinfo->owner,'.')|| */ !strcmp(fileinfo->owner, "Anonymous") || !strcmp(fileinfo->owner, "deliver"))       /* Leeward 98.04.14 */
        return DONOTHING;

    else
        t_query(fileinfo->owner);
    return FULLUPDATE;
}


int SR_BMfunc(int ent, struct fileheader *fileinfo, char *direct)
{
    int i;
    char buf[256], ch[4], BMch;
    char *SR_BMitems[] = { "É¾³ý", "±£Áô", "ÎÄÕª", "·ÅÈë¾«»ªÇø", "·ÅÈëÔÝ´æµµ", "±ê¼ÇÉ¾³ý",
        "ÉèÎª²»¿É»Ø¸´", "×öºÏ¼¯"
    };
	const int item_num = 8;
	char filepath[80],title[STRLEN];

    char linebuffer[LINELEN*3];

    if (!chk_currBM(currBM, currentuser)) {
        return DONOTHING;
    }
    if (digestmode == 4 || digestmode == 5)     /* KCN:ÔÝ²»ÔÊÐí */
        return DONOTHING;
    if (digestmode >= 2)
    	return DONOTHING;
    saveline(t_lines - 3, 0, linebuffer);
    saveline(t_lines - 2, 0, NULL);
    move(t_lines - 3, 0);
    clrtoeol();
    strcpy(buf, "ÏàÍ¬Ö÷Ìâ (0)È¡Ïû  ");
    for (i = 0; i < item_num; i++) {
        char t[40];

        sprintf(t, "(%d)%s", i + 1, SR_BMitems[i]);
        strcat(buf, t);

/*        snprintf(buf,256,"%s(%d)%s  ",buf,i+1,SR_BMitems[i]);*/
    };
    strcat(buf, "? [0]: ");
    if (strlen(buf) > 76) {
        char savech = buf[76];

        buf[76] = 0;
        prints("%s", buf);
        buf[76] = savech;

/*        strcpy(buf,buf+76);*/
        getdata(t_lines - 2, 0, buf + 76, ch, 3, DOECHO, NULL, true);
    } else
        getdata(t_lines - 3, 0, buf, ch, 3, DOECHO, NULL, true);
    BMch = atoi(ch);
    if (BMch <= 0 || BMch > item_num) {
        saveline(t_lines - 2, 1, NULL);
        saveline(t_lines - 3, 1, linebuffer);
        return DONOTHING;
    }
    if (digestmode == 2 && BMch <= 3) {
        saveline(t_lines - 2, 1, NULL);
        saveline(t_lines - 3, 1, linebuffer);
        return DONOTHING;
    }
    move(t_lines - 2, 0);
    clrtoeol();
    move(t_lines - 3, 0);
    clrtoeol();

    /*
     * Leeward 98.04.16
     */
    snprintf(buf, 256, "ÊÇ·ñ´Ó´ËÖ÷ÌâµÚÒ»Æª¿ªÊ¼%s (Y)µÚÒ»Æª (N)Ä¿Ç°ÕâÆª (C)È¡Ïû (Y/N/C)? [Y]: ", SR_BMitems[BMch - 1]);
    getdata(t_lines - 3, 0, buf, ch, 3, DOECHO, NULL, true);
    switch (ch[0]) {
    default:
    case 'y':
    case 'Y':
        ent = sread(2, 0, ent, 0, fileinfo);    /* Ôö¼Ó·µ»ØÖµ,ÐÞ¸ÄÍ¬Ö÷Ìâ Bigman: 2000.8.20 */
        fileinfo = &SR_fptr;
        break;
    case 'n':
    case 'N':
        break;
    case 'c':
    case 'C':
        saveline(t_lines - 2, 1, NULL);
        saveline(t_lines - 3, 1, linebuffer);
        return DONOTHING;
    }
    bmlog(currentuser->userid, currboard->filename, 14, 1);

	if(SR_BMTOTAL == BMch + SR_BMBASE){
        snprintf(buf, 256, "ÊÇ·ñ±£ÁôÒýÎÄ(Y/N/C)? [Y]: ");
        getdata(t_lines - 2, 0, buf, ch, 3, DOECHO, NULL, true);
       sprintf(filepath,"tmp/bm.%s",currentuser->userid);
	    switch (ch[0]){
	    case 'y':
	    case 'Y':
	    default:
	    	if(dashf(filepath))unlink(filepath);
	       sread(BMch + SR_BMBASE - 3 , 0, ent, 0, fileinfo); //ÇÐ»»³ÉÔÝ´æµµ²Ù×÷
	 	break;
	    case 'n':
	    case 'N':
	    	if(dashf(filepath))unlink(filepath);
		sread(-(BMch + SR_BMBASE - 3), 0, ent, 0, fileinfo);
		break;
           case 'c':
	    case 'C':
		    saveline(t_lines - 2, 1, NULL);
            saveline(t_lines - 3, 1, linebuffer);
            return DONOTHING;
    	}
        //create new title
		strcpy(buf,"[ºÏ¼¯] ");
		if(!strncmp(fileinfo->title,"Re: ",4))strcpy(buf+7,fileinfo->title + 4);
		else
		    strcpy(buf+7,fileinfo->title);
        if(strlen(buf) >= STRLEN )buf[STRLEN-1] = 0;
		strcpy(title,buf);
        //post file to the board
		if(post_file(currentuser,"",filepath,currboard->filename,title,0,2) < 0) {//fail
            sprintf(buf,"·¢±íÎÄÕÂµ½°æÃæ³ö´í!Çë°´ÈÎÒâ¼üÍË³ö << ");
			a_prompt(-1,buf,filepath); //filepath no use
			saveline(t_lines - 2, 1, NULL);
            saveline(t_lines - 3, 1, linebuffer);
            return DONOTHING;
		}
        unlink(filepath);
		sprintf(filepath,"tmp/se.%s",currentuser->userid);
		unlink(filepath);
		return DIRCHANGED;
	}

	sread(BMch + SR_BMBASE, 0, ent, 0, fileinfo);

    return DIRCHANGED;
}


int SR_BMfuncX(int ent, struct fileheader *fileinfo, char *direct)
{
    int i;
    char buf[256], ch[4], BMch;
    char *SR_BMitems[] = { "É¾³ý", "±£Áô", "ÎÄÕª", "·ÅÈë¾«»ªÇø", "·ÅÈëÔÝ´æµµ", "±ê¼ÇÉ¾³ý",
        "ÉèÎª²»¿É»Ø¸´", "×öºÏ¼¯"
    };
	const int item_num = 8;
	char filepath[80],title[STRLEN];

    char linebuffer[LINELEN*3];

    if (!chk_currBM(currBM, currentuser)) {
        return DONOTHING;
    }
    if (digestmode == 4 || digestmode == 5)     /* KCN:²»ÔÊÐí */
        return DONOTHING;
    saveline(t_lines - 3, 0, linebuffer);
    saveline(t_lines - 2, 0, NULL);
    move(t_lines - 3, 0);
    clrtoeol();
    strcpy(buf, "ÏàÍ¬Ö÷Ìâ (0)È¡Ïû  ");
    for (i = 0; i < item_num; i++) {
        char t[40];

        sprintf(t, "(%d)%s", i + 1, SR_BMitems[i]);
        strcat(buf, t);
    } strcat(buf, "? [0]: ");
    if (strlen(buf) > 76) {
        char savech = buf[76];

        buf[76] = 0;
        prints("%s", buf);
        buf[76] = savech;

/*        strcpy(buf,buf+76);*/
        getdata(t_lines - 2, 0, buf + 76, ch, 3, DOECHO, NULL, true);
    } else
        getdata(t_lines - 3, 0, buf, ch, 3, DOECHO, NULL, true);
    BMch = atoi(ch);
    if (BMch <= 0 || BMch > item_num) {
        saveline(t_lines - 2, 1, NULL);
        saveline(t_lines - 3, 1, linebuffer);
        return DONOTHING;
    }
    if (digestmode == 2 && BMch <= 3) {
	saveline(t_lines - 2, 1, NULL);
	saveline(t_lines - 3, 1, linebuffer);
        return DONOTHING;
    }
    move(t_lines - 3, 0);
    clrtoeol();

    /*
     * Leeward 98.04.16
     */
    snprintf(buf, 256, "ÊÇ·ñ´Ó´ËÖ÷ÌâµÚÒ»Æª¿ªÊ¼%s (Y)µÚÒ»Æª (N)Ä¿Ç°ÕâÆª (C)È¡Ïû (Y/N/C)? [Y]: ", SR_BMitems[BMch - 1]);
    getdata(t_lines - 2, 0, buf, ch, 3, DOECHO, NULL, true);
    B_to_b = true;
    switch (ch[0]) {
    default:
    case 'y':
    case 'Y':
        ent = sread(2, 0, ent, 0, fileinfo);    /* Ôö¼Ó·µ»ØÖµ,ÐÞ¸ÄÍ¬Ö÷Ìâ Bigman: 2000.8.20 */
        fileinfo = &SR_fptr;
        break;
    case 'n':
    case 'N':
        break;
    case 'c':
    case 'C':
        saveline(t_lines - 2, 1, NULL);
        return DONOTHING;
    }

    bmlog(currentuser->userid, currboard->filename, 14, 1);

	if(SR_BMTOTAL == BMch + SR_BMBASE){
        snprintf(buf, 256, "ÊÇ·ñ±£ÁôÒýÎÄ(Y/N/C)? [Y]: ");
        getdata(t_lines - 2, 0, buf, ch, 3, DOECHO, NULL, true);
 	sprintf(filepath,"tmp/bm.%s",currentuser->userid);
	    switch (ch[0]){
	    case 'y':
	    case 'Y':
	    default:
		if(dashf(filepath))unlink(filepath);
		sread(BMch + SR_BMBASE - 3 , 0, ent, 0, fileinfo); //ÇÐ»»³ÉÔÝ´æµµ²Ù×÷
		break;
	    case 'n':
	    case 'N':
	    	if(dashf(filepath))unlink(filepath);
		sread(-(BMch + SR_BMBASE - 3), 0, ent, 0, fileinfo);
		break;
           case 'c':
	    case 'C':
		    saveline(t_lines - 2, 1, NULL);
            saveline(t_lines - 3, 1, linebuffer);
            return DONOTHING;
    	}
        //create new title
		strcpy(buf,"[ºÏ¼¯] ");
		if(!strncmp(fileinfo->title,"Re: ",4))strcpy(buf+7,fileinfo->title + 4);
		else
		    strcpy(buf+7,fileinfo->title);
        if(strlen(buf) >= STRLEN )buf[STRLEN-1] = 0;
		strcpy(title,buf);
        //post file to the board
		if(post_file(currentuser,"",filepath,currboard->filename,title,0,2) < 0) {//fail
            sprintf(buf,"·¢±íÎÄÕÂµ½°æÃæ³ö´í!Çë°´ÈÎÒâ¼üÍË³ö << ");
			a_prompt(-1,buf,filepath); //filepath no use
			saveline(t_lines - 2, 1, NULL);
            saveline(t_lines - 3, 1, linebuffer);
            return DONOTHING;
		}
        unlink(filepath);
		sprintf(filepath,"tmp/se.%s",currentuser->userid);
		unlink(filepath);
		return DIRCHANGED;
	}

    if (SR_BMTMP == BMch + SR_BMBASE)   /* Leeward 98.04.16 */
        sread(-(BMch + SR_BMBASE), 0, ent, 0, fileinfo);

    else
        sread(BMch + SR_BMBASE, 0, ent, 0, fileinfo);
    B_to_b = false;
    return DIRCHANGED;
}

int SR_first_new(int ent, struct fileheader *fileinfo, char *direct)
{
    sread(2, 0, ent, 0, fileinfo);
    if (sread(3, 0, NULL, 0, &SR_fptr) == -1) { /*Found The First One */
        sread(0, 1, NULL, 0, &SR_fptr);
        return FULLUPDATE;
    }
    return PARTUPDATE;
}


int SR_last(int ent, struct fileheader *fileinfo, char *direct)
{
    sread(1, 0, ent, 0, fileinfo);
    return PARTUPDATE;
}


int SR_first(int ent, struct fileheader *fileinfo, char *direct)
{
    sread(2, 0, ent, 0, fileinfo);
    return PARTUPDATE;
}

int SR_read(int ent, struct fileheader *fileinfo, char *direct)
{
    sread(0, 1, NULL, 0, fileinfo);
    return FULLUPDATE;
}


/* Leeward 98.10.03 Í¬Ö÷ÌâÔÄ¶ÁÍË³öÊ±ÎÄÕÂÎ»ÖÃ²»±ä */
int SR_readX(int ent, struct fileheader *fileinfo, char *direct)
{
    sread(-1003, 1, NULL, 0, fileinfo);
    return FULLUPDATE;
}

int SR_author(int ent, struct fileheader *fileinfo, char *direct)
{
    sread(0, 1, NULL, 1, fileinfo);
    return FULLUPDATE;
}

/* Leeward 98.10.03 Í¬×÷ÕßÔÄ¶ÁÍË³öÊ±ÎÄÕÂÎ»ÖÃ²»±ä */
int SR_authorX(int ent, struct fileheader *fileinfo, char *direct)
{
    sread(-1003, 1, NULL, 1, fileinfo);
    return FULLUPDATE;
}
/*
int auth_post_down(int ent, struct fileheader *fileinfo, char *direct)
{
    struct keeploc *locmem;

    locmem = getkeep(direct, 1, 1);
    if (search_author(locmem, 1, fileinfo->owner))
        return PARTUPDATE;
    else
        update_endline();
    return DONOTHING;
}

int auth_post_up(int ent, struct fileheader *fileinfo, char *direct)
{
    struct keeploc *locmem;

    locmem = getkeep(direct, 1, 1);
    if (search_author(locmem, -1, fileinfo->owner))
        return PARTUPDATE;
    else
        update_endline();
    return DONOTHING;
}
*/

int t_search_down(int ent, struct fileheader *fileinfo, char *direct)
{
    struct keeploc *locmem;
	int i;

	if(strstr(direct,DING_DIR)) locmem=getkeep(currdirect,1,1);
	else locmem = getkeep(direct, 1, 1);

	i = search_title(locmem, 1);
	if(i == 2)
		return NEWDIRECT;
	else if(i == 1)
        return PARTUPDATE;
    else
        update_endline();
    return DONOTHING;
}

int t_search_up(int ent, struct fileheader *fileinfo, char *direct)
{
    struct keeploc *locmem;
	int i;

	if(strstr(direct,DING_DIR)) locmem=getkeep(currdirect,1,1);
	else locmem = getkeep(direct, 1, 1);
    
	i = search_title(locmem, -1);
	if(i == 2)
		return NEWDIRECT;
	else if(i == 1)
        return PARTUPDATE;
    else
        update_endline();
    return DONOTHING;
}

int thread_up(int ent, struct fileheader *fileinfo, char *direct)
{
    struct keeploc *locmem;

    locmem = getkeep(direct, 1, 1);
    if (uinfo.mode != RMAIL) {
        if (search_threadid(locmem, -1, fileinfo->groupid, 0)) {
            update_endline();
            return PARTUPDATE;
        }
    } else if (search_thread(locmem, -1, fileinfo->title)) {
        update_endline();
        return PARTUPDATE;
    }
    update_endline();
    return DONOTHING;
}


int thread_down(int ent, struct fileheader *fileinfo, char *direct)
{
    struct keeploc *locmem;

    locmem = getkeep(direct, 1, 1);
    if (uinfo.mode != RMAIL) {
        if (search_threadid(locmem, 1, fileinfo->groupid, 0)) {
            update_endline();
            return PARTUPDATE;
        }
    } else if (search_thread(locmem, 1, fileinfo->title)) {
        update_endline();
        return PARTUPDATE;
    }
    update_endline();
    return DONOTHING;
}

static int search_post(struct keeploc *locmem, int offset)
{
    static char query[STRLEN];
    char ans[STRLEN], pmt[STRLEN];

    strncpy(ans, query, STRLEN);
    snprintf(pmt, STRLEN, "ËÑÑ°%sµÄÎÄÕÂ [%s]: ", offset > 0 ? "ÍùºóÀ´" : "ÍùÏÈÇ°", ans);
    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, pmt, ans, 50, DOECHO, NULL, true);
    if (ans[0] != '\0')
        strcpy(query, ans);
    return search_articles(locmem, query, offset, -1);
}


static int search_title(struct keeploc *locmem, int offset)
{
    static char title[STRLEN];
    char ans[STRLEN], pmt[STRLEN];

    strncpy(ans, title, STRLEN);
    snprintf(pmt, STRLEN, "%sËÑÑ°±êÌâ [%s]: ", offset > 0 ? "Íùºó" : "ÍùÇ°", ans);
    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, pmt, ans, STRLEN - 1, DOECHO, NULL, true);
    if (*ans != '\0')
        strcpy(title, ans);
    return search_articles(locmem, title, offset, 0);
}


static int search_thread(struct keeploc *locmem, int offset, char *title)
{
    if (title[0] == 'R' && (title[1] == 'e' || title[1] == 'E')
        && title[2] == ':')
        title += 4;
    setqtitle(title);
    return search_articles(locmem, title, offset, 2);
}


/*Add by SmallPig*/
int sread(int passonly, int readfirst, int pnum, int auser, struct fileheader *ptitle)
{
    struct keeploc *locmem;
    extern struct fileheader ReadPostHeader;
    int istest = 0, isstart = 0, isnext = 1;
    int previous;
    char genbuf[STRLEN], title[STRLEN],path[MAXPATH];
    int B;                      /* Leeward: ±íÊ¾°´µÄÊÇ B(-1) »¹ÊÇ b(+1) */
    int ori_top, ori_crs;       /* Leeward 98.10.03 add these 3 ori_...  and Xflag */
    char ori_file[STRLEN];
    int ret;

/*    int Xflag = (-1003 != passonly )? 0 : !(passonly = 0);Ææ¹Ö°¡KCN */
    int Xflag = (-1003 != passonly) ? 0 : (passonly = 0, 1);

	/* add by stiger*/
	if(POSTFILE_BASENAME(ptitle->filename)[0]=='Z') return -1;

    ret=0;
    path[0]=0;
    strncpy(ori_file, ptitle->filename, FILENAME_LEN);
    B = (passonly < 0 ? -1 : 1);        /* Leeward 98.04.16 */
    passonly *= B;
    RemoveAppendedSpace(ptitle->title); /* Leeward 98.02.13 */
    previous = pnum;
    if (passonly == 0) {
        if (readfirst)
            isstart = 1;
        else {
            isstart = 0;
            move(t_lines - 1, 0);
            prints("[1m[44m[31m[%8s] [33mÏÂÒ»·â ' ',<Enter>,¡ý©¦ÉÏÒ»·â ¡ü,U                                  [m", auser ? "ÏàÍ¬×÷Õß" : "Ö÷ÌâÔÄ¶Á");
            clrtoeol();
            switch (igetkey()) {
                /*
                 * TODO: add KEY_REFRESH support 
                 */
            case ' ':
            case '\n':
            case KEY_DOWN:
                isnext = 1;
                break;
            case KEY_UP:
            case 'u':
            case 'U':
                isnext = -1;
                break;
            default:
                break;
            }
        }
    } else if (passonly == 1 || passonly >= 3)
        isnext = 1;
    else
        isnext = -1;
    locmem = getkeep(currdirect, 1, 1);
    ori_top = locmem->top_line; /* Leeward 98.10.02 */
    ori_crs = locmem->crs_line;
    if (auser == 0) {
        strncpy(title, ptitle->title, STRLEN);
        setqtitle(title);
    } else {
        strncpy(title, ptitle->owner, OWNER_LEN);
        title[OWNER_LEN-1]=0;
        setqtitle(ptitle->title);
    }
    memcpy(&ReadPostHeader, ptitle, sizeof(struct fileheader));
    if (!strncmp(title, "Re: ", 4)) {
        strcpy(title, title + 4);
    }
    memcpy(&SR_fptr, ptitle, sizeof(SR_fptr));
    while (!istest) {
        switch (passonly) {
        case 0:
        case 1:
        case 2:
            break;
        case 3:
#ifdef HAVE_BRC_CONTROL
            if (brc_unread(SR_fptr.id))
                return -1;
            else
#endif
                break;
        case SR_BMDEL:
            if (digestmode)
                return -1;
            /*
             * Leeward 97.11.18: fix bugs: add "if" block 
             */
            /*
             * if (!( ptitle->accessed[0] & FILE_MARKED )) 
             */
            if (!(SR_fptr.accessed[0] & FILE_MARKED))

                /*
                 * Bigman 2000.8.20: ÐÞ¸ÄÍ¬Ö÷ÌâÉ¾³ý´íÎó.... LeewardÕâ¸öÔö¼ÓµÄ²»¶ÔÑ½,ÒÔºóµÄÄÚÈÝÃ»ÓÐ¶ÁÑ½ 
                 */
            {
                SR_BMDELFLAG = true;
                del_post(locmem->crs_line, &SR_fptr, currdirect);
                SR_BMDELFLAG = false;
                if (sysconf_eval("KEEP_DELETED_HEADER",0) <= 0) {
                    last_line--;
                    locmem->crs_line--;
                    previous = locmem->crs_line;
                }
            }
            break;
        case SR_BMMARKDEL:
            if (digestmode)
                return -1;
            /*
             * Leeward 97.11.18: fix bugs: add "if" block 
             */
            /*
             * if (!( ptitle->accessed[0] & FILE_MARKED )) 
             */
            if (!(SR_fptr.accessed[0] & FILE_MARKED))

                /*
                 * Bigman 2000.8.20: ÐÞ¸ÄÍ¬Ö÷ÌâÉ¾³ý´íÎó.... LeewardÕâ¸öÔö¼ÓµÄ²»¶ÔÑ½,ÒÔºóµÄÄÚÈÝÃ»ÓÐ¶ÁÑ½ 
                 */
            {
                set_delete_mark(locmem->crs_line, &SR_fptr, currdirect);
            }
            break;
        case SR_BMNOREPLY:
            if (digestmode)
                return -1;
            if (!(SR_fptr.accessed[1] & FILE_SIGN))

                /*
                 * Bigman 2000.8.20: ÐÞ¸ÄÍ¬Ö÷ÌâÉ¾³ý´íÎó.... LeewardÕâ¸öÔö¼ÓµÄ²»¶ÔÑ½,ÒÔºóµÄÄÚÈÝÃ»ÓÐ¶ÁÑ½ 
                 */
            {
                noreply_post_noprompt(locmem->crs_line, &SR_fptr, currdirect);
            }
            break;
        case SR_BMMARK:
            if (digestmode == 2)
                return -1;
            mark_post(locmem->crs_line, &SR_fptr, currdirect);
            break;
        case SR_BMDIGEST:
            if (digestmode == true || digestmode == 4 || digestmode == 5)
                return -1;
            digest_post(locmem->crs_line, &SR_fptr, currdirect);
            break;
        case SR_BMIMPORT:
            if (a_Import(path, currboard->filename, &SR_fptr, true, currdirect, locmem->crs_line)!=0);     /* Leeward 98.04.15 */
	        ret++;
            break;
        case SR_BMTMP:         /* Leeward 98.04.16 */
            if (-1 == B)
                a_SeSave("0Announce", currboard->filename, &SR_fptr, true,currdirect,locmem->crs_line,1);
            else
                /*a_Save(NULL, currboard->filename, &SR_fptr, true, currdirect, locmem->crs_line);*/
				a_SeSave("0Announce", currboard->filename, &SR_fptr, true,currdirect,locmem->crs_line,0);
            break;
        }
        if (!isstart) {
            if (uinfo.mode != RMAIL && auser == 0) {
                    search_threadid(locmem, isnext, ptitle->groupid, (passonly == 1)||(passonly == 2));
	    			if (passonly == 1 || passonly == 2) {//Ö±½ÓËÑµ½Î²ÁË
		    			previous = locmem->crs_line;
			    		break;
				    }
    		}
            else
                search_articles(locmem, title, isnext, auser + 2);
        }
        if (previous == locmem->crs_line) {
            break;
        }
        if (uinfo.mode != RMAIL)
            setbfile(genbuf, currboard->filename, SR_fptr.filename);
        else
            setmailfile(genbuf, currentuser->userid, SR_fptr.filename);
        previous = locmem->crs_line;
        if (passonly == 0) {
            int lch;            /* period 2000-09-11    ·½°¸1:                          *
                                 * * ½â¾ö:Í¬Ö÷ÌâÏòÉÏ²éÕÒ,ÎÄÕÂ´óÓÚÒ»ÆÁÊ±°´Ò»´ÎUP¼üÆÁÄ»ÎÞÄÚÈÝ *
                                 * * ·½°¸2: rawmore()º¯ÊýÖÐ, ÅÐ¶ÏKEY_UP==ch´¦²»Ó¦¸ÃÇåÆÁ */
            register_attach_link(board_attach_link, &SR_fptr);
            lch = ansimore_withzmodem(genbuf, false, SR_fptr.title);
            register_attach_link(NULL,NULL);

            /*
             * ansimore(genbuf,false) ;  
             */
#ifdef HAVE_BRC_CONTROL
            brc_add_read(SR_fptr.id);
#endif
            isstart = 0;
          redo:
            move(t_lines - 1, 0);
            prints("[1m[44m[31m[%8s] [33m»ØÐÅ R ©¦ ½áÊø Q,¡û ©¦ÏÂÒ»·â ¡ý,Enter©¦ÉÏÒ»·â ¡ü,U ©¦ ^R »Ø¸ø×÷Õß   \033[m", auser ? "ÏàÍ¬×÷Õß" : "Ö÷ÌâÔÄ¶Á");
            clrtoeol();
            /*
             * period 2000-09-11       Ô­ÒòÍ¬ÉÏ 
             */
            if (lch != KEY_UP && lch != KEY_DOWN
                    && (lch <= 0 || strchr("RrEexp", lch) == NULL))
                lch = igetkey();
            /*
             * TODO: add KEY_REFRESH support 
             */
            switch (lch) {
            case Ctrl('Y'):
                zsend_post(0, &SR_fptr, currdirect);
                clear();
                goto redo;
            case Ctrl('Z'):
                r_lastmsg();    /* Leeward 98.07.30 support msgX */
                break;
            case 'Q':
            case 'q':
            case KEY_LEFT:
                istest = 1;
                break;
            case 'Y':
            case 'R':
            case 'y':
            case 'r':
                do_reply(&SR_fptr);
            case ' ':
            case '\n':
            case 'j':
            case 'n':
            case KEY_DOWN:
                isnext = 1;
                break;
            case Ctrl('A'):
                clear();
                show_author(0, &SR_fptr, currdirect);
                isnext = 1;
                break;
            case Ctrl('Q'):    /*Haohmaru.98.12.05.ÏµÍ³¹ÜÀíÔ±Ö±½Ó²é×÷Õß×ÊÁÏ */
                clear();
                show_authorinfo(0, &SR_fptr, currdirect);
                isnext = 1;
                break;
            case Ctrl('W'):    /* cityhunter 00.10.18 °æÖ÷¹ÜÀíÖ±½Ó²ì¿´°æÖ÷ÐÅÏ¢ */
                clear();
                show_authorBM(0, &SR_fptr, currdirect);
                isnext = 1;
                break;
            case 'z':
            case 'Z':          /*Haohmaru.2000.5.19,Ö±½Ó¸ø×÷Õß·¢msg */
                if (!HAS_PERM(currentuser, PERM_PAGE))
                    break;
                sendmsgtoauthor(0, &SR_fptr, currdirect);
                isnext = 1;
                break;
            case 'k':
            case 'l':
            case KEY_UP:
            case 'u':
            case 'U':
                isnext = -1;
                break;
            case Ctrl('R'):
                post_reply(0, &SR_fptr, (char *) NULL);
                break;
            case 'g':
                digest_post(0, &SR_fptr, currdirect);
                break;
            case 'L':
                if (uinfo.mode == LOOKMSGS)
                    break;
                show_allmsgs();
                break;
            case 'H':          /* Luzi 1997.11.1 */
                r_lastmsg();
                break;
            case 'w':          /* Luzi 1997.11.1 */
                if (!HAS_PERM(currentuser, PERM_PAGE))
                    break;
                s_msg();
                break;
            case '!':          /*Haohmaru 1998.09.24 */
                Goodbye();
                break;
            case 'O':
            case 'o':          /* Luzi 1997.11.1 */
                if (!HAS_PERM(currentuser, PERM_BASIC))
                    break;
                t_friends();
                break;
            default:
                break;
            }
        }
    }

    /*
     * Leeward 98.10.02 add all below except last "return 1" 
     */
    if (ret!=0) {
	    move(t_lines-2,0);
	    prints("ÊÕÈë¹ý³ÌÖÐ¹²ÓÐ%dÆªÎÄÕÂ³öÏÖ´íÎó",ret);
	    pressanykey();
    };
    if (Xflag) {
/*        if (search_file(ori_file) != ori_crs)
            bell();
*/
        RMVCURS(locmem);
        locmem->top_line = ori_top;
        locmem->crs_line = ori_crs;
        PUTCURS(locmem);
    }
    if ((passonly == 2) && (readfirst == 0) && (auser == 0))    /*ÔÚÍ¬Ö÷ÌâÉ¾³ýÊ±,ÄÜ¹»·µ»ØµÚÒ»ÆªÎÄÕÂ Bigman:2000.8.20 */
        return previous;
    else
        return 1;
}

void get_upper_str(char *ptr2, char *ptr1)
{
    int ln, i;

    for (ln = 0; (ln < STRLEN) && (ptr1[ln] != 0); ln++);
    for (i = 0; i < ln; i++) {
        ptr2[i] = toupper(ptr1[i]);
        /******** ÏÂÃæÎªLuziÌí¼Ó ************/
        if (ptr2[i] == '\0')
            ptr2[i] = '\1';
        /******** ÒÔÉÏÎªLuziÌí¼Ó ************/
    }
    ptr2[ln] = '\0';
}

int searchpattern(char *filename, char *query)
{
    FILE *fp;
    char buf[256];

    if ((fp = fopen(filename, "r")) == NULL)

        return 0;
    while (fgets(buf, 256, fp) != NULL) {
        if (strstr(buf, query)) {
            fclose(fp);
            return true;
        }
    }
    fclose(fp);
    return false;
}


/* COMMAN : use mmap to speed up searching */
/* add by stiger
 * return :   2 :  DIRCHANGED
 *            1 :  FULLUPDATE
 *            0 :  DONOTHING
 */
static int search_articles(struct keeploc *locmem, char *query, int offset, int aflag)
{
    char ptr[STRLEN];
    int now, match = 0, retn;
    int complete_search;
    char upper_ptr[STRLEN], upper_query[STRLEN];
    bool init;
    size_t bm_search[256];

/*	int mmap_offset,mmap_length; */
    struct fileheader *pFh, *pFh1;
    int size;

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
    now = locmem->crs_line;

/*    refresh();*/
    memset(&SR_fptr, 0, sizeof(struct fileheader));
    match = 0;
	retn = 0;
    BBS_TRY {
        if (safe_mmapfile(currdirect, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &pFh, &size, NULL) == 0)
            BBS_RETURN(0);
        last_line = size / sizeof(struct fileheader);
		/*ÔÚÖÃ¶¥ÎÄÕÂÇ°ËÑË÷*/
		if(now > last_line){
			locmem->crs_line=last_line;
			if(locmem->top_line>locmem->crs_line) locmem->top_line=last_line;
			now = last_line;
			retn = 2;
		}
        if (now <= last_line) {
            pFh1 = pFh + now - 1;
            while (1) {
                if (offset > 0) {
                    if (++now > last_line)
                        break;
                    pFh1++;
                } else {
                    if (--now < 1)
                        break;
                    pFh1--;
                }
                if (now == locmem->crs_line)
                    break;
                if (aflag == -1) {
                    char p_name[256];

                    if (uinfo.mode != RMAIL)

                        setbfile(p_name, currboard->filename, pFh1->filename);
                    else
                        setmailfile(p_name, currentuser->userid, pFh1->filename);
                    if (searchpattern(p_name, query)) {
                        match = cursor_pos(locmem, now, screen_len/2);
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
                        match = cursor_pos(locmem, now, screen_len/2);
                        break;
                    }
                } else {
                    /*
                     * Í¬×÷Õß²éÑ¯¸Ä³ÉÍêÈ«Æ¥Åä by dong, 1998.9.12 
                     */
                    if (aflag == 1) {   /* ½øÐÐÍ¬×÷Õß²éÑ¯ */
                        if (!strcasecmp(ptr, upper_query)) {
                            match = cursor_pos(locmem, now, screen_len/2);
                            break;
                        }
                    }

                    else if (bm_strcasestr_rp(ptr, upper_query,bm_search,&init) != NULL) {
                        match = cursor_pos(locmem, now, screen_len/2);
                        break;
                    }
                }
            }
            memcpy(&SR_fptr, pFh + locmem->crs_line - 1, sizeof(struct fileheader));
        }
    }
    BBS_CATCH {
        memset(&SR_fptr, 0, sizeof(struct fileheader));
        match = 0;
    }
    BBS_END end_mmapfile((void *) pFh, size, -1);
    move(t_lines - 1, 0);
    clrtoeol();
	/*add by stiger,ÖÃ¶¥µÄ´¦Àí */
    if (uinfo.mode != RMAIL && uinfo.mode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD)){
		char dirtmp[256];
		char *dirp;
		strcpy(dirtmp,currdirect);
		dirp=strrchr(dirtmp,'/');
		if(dirp){
			*(dirp+1)='\0';
			strcat(dirtmp,DING_DIR);
			last_line += get_num_records(dirtmp,sizeof(struct fileheader));
		}
	}
	if(retn) return 2;
	if(match) return 1;
    return 0;
}

static int search_threadid(struct keeploc *locmem, int offset, int groupid, int mode)
{
//´Ó locmem Î»ÖÃÆð£¬Ïòoffset·½Ïò£¬ËÑË÷groupidÏàÍ¬µÄfileheader£¬
//mode == 0 ËÑË÷µÚÒ»¸ö¼´ÍË³ö£¬·ñÔòËÑË÷µ½×îºóÒ»¸ö·ûºÏÌõ¼þµÄ
//ÒÔÏÂ¿ªÊ¼³­Ï®ytht´úÂë              by yuhuan
    int now, match = 0, start, sorted, i;

/*	int mmap_offset,mmap_length; */
    struct fileheader *pFh, *pFh1;
    int size;

    now = locmem->crs_line;
	/* add by stiger */
	if(strstr(locmem->key,DING_DIR)) return 0;

/*    refresh();*/
    memset(&SR_fptr, 0, sizeof(struct fileheader));
	if (digestmode == 0 || digestmode == 3)  //Õâ¼¸ÖÖ .DIR ÊÇÅÅÐòµÄ
		sorted = 1;
	else 
		sorted = 0;
    match = 0;
    BBS_TRY {
        if (safe_mmapfile(currdirect, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &pFh, &size, NULL) == 0)
            BBS_RETURN(0);
        last_line = size / sizeof(struct fileheader);
        if (now <= last_line) {
			if (mode == 0) {           //°¤×ÅËÑ£¬ËÑµ½¾ÍÍ£
                pFh1 = pFh + now - 1;
                while (1) {
                    if (offset > 0) {
                        if (++now > last_line)
                            break;
                        pFh1++;
                    } else {
                        if (--now < 1)
                            break;
                        pFh1--;
						if (sorted && pFh1->id < groupid)  //ÒòÎªÅÅÐòÁË£¬ËÑµ½ÕâÀï¾Íµ½Í·ÁË
							break;
                    }
                    if (now == locmem->crs_line)
                        break;
                    if (pFh1->groupid == groupid) {
                        match = cursor_pos(locmem, now, screen_len/2);
                        break;
                    }
				}
            } else {
				if (sorted && offset == -1) {   //ÓÐÍûÁ½·Ö·¨¼ÓËÙ²¿·Ö
					start = Search_Bin(pFh, groupid, 0, now - 1);
					if (start >= 0) {   //ËÆºõÊÇÖ±½ÓÕÒµ½ÁËÅ¶
						match = cursor_pos(locmem, start + 1, screen_len / 2);
						goto END;
					}
					start = -(start + 1);
					if (start >= now)
						goto END;
					pFh1 = pFh + start;
					for (i = start; i < now; i++) {
						if (pFh1->groupid == groupid) {
							match = cursor_pos(locmem, i+1, screen_len / 2);
							break;
						}
						pFh1++;
					}
				} else {
					pFh1 = pFh + now - 1;
					while (1) {
						if (offset > 0) {
							if (++now > last_line)
								break;
							pFh1++;
						} else {
							if (--now < 1)
								break;
							pFh1--;
						}
						if (pFh1->groupid == groupid) {
							match = now;
						}
					}
					if (match)   //ÕÒµ½¹ý
						match = cursor_pos(locmem, match, screen_len/2);
				}
			}
			END: {}
            memcpy(&SR_fptr, pFh + locmem->crs_line - 1, sizeof(struct fileheader));
		}
    }
    BBS_CATCH {
        memset(&SR_fptr, 0, sizeof(struct fileheader));
        match = 0;
    }
    BBS_END end_mmapfile((void *) pFh, size, -1);
    move(t_lines - 1, 0);
    clrtoeol();
	/*add by stiger,ÖÃ¶¥µÄ´¦Àí */
    if (uinfo.mode != RMAIL && uinfo.mode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD)){
		char dirtmp[256];
		char *dirp;
		strcpy(dirtmp,currdirect);
		dirp=strrchr(dirtmp,'/');
		if(dirp){
			*(dirp+1)='\0';
			strcat(dirtmp,DING_DIR);
			last_line += get_num_records(dirtmp,sizeof(struct fileheader));
		}
	}
    return match;
}


/* calc cursor pos and show cursor correctly -cuteyu */
static int cursor_pos(struct keeploc *locmem, int val, int from_top)
{
    if (val > last_line) {
        val = DEFINE(currentuser, DEF_CIRCLE) ? 1 : last_line;
    }
    if (val <= 0) {
        val = DEFINE(currentuser, DEF_CIRCLE) ? last_line : 1;
    }
    if (val >= locmem->top_line && val < locmem->top_line + screen_len - 1) {
        RMVCURS(locmem);
        locmem->crs_line = val;
        PUTCURS(locmem);
        return 0;
    }
    locmem->top_line = val - from_top;
    if (locmem->top_line <= 0)
        locmem->top_line = 1;
    locmem->crs_line = val;
    return 1;
}
