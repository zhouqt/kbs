/*
        Õâ¸ö³ÌÊ½ÊÇÓÉ Firebird BBS Ëù×¨ÓÃµÄ Editor £¬»¶Ó­¸÷Î»Ê¹ÓÃ
        Èç¹ûÓÐÈÎºÎÎÊÌâÇë Mail ¸ø SmallPig.bbs@bbs.cs.ccu.edu.tw
        »òÊÇµ½ 140.123.101.78(bbs.cs.ccu.edu.tw) Post ÎÊÌâ¡£
*/

#include "bbs.h"
#include "edit.h"

#define MAX_EDIT_LINE 20000
#define LOCAL_ARTICLE_DEFAULT 0

extern int temp_numposts;       /*Haohmaru.99.4.02.ÈÃ°®¹àË®µÄÈË¿ÞÈ¥°É//grin */
extern int local_article;
static const int scrollen = 2;
char save_title[STRLEN];
int in_mail;

void vedit_key();
static struct textline *firstline = NULL;
static struct textline *lastline = NULL;
static struct textline *currline = NULL;
static int first_mark_line;
static int currpnt = 0;
static char searchtext[80];
static int editansi = 0;
static int marknum;
static int moveln = 0;
static int shifttmp = 0;
static int ismsgline;
static int tmpline;
static struct textline *top_of_win = NULL;
static int curr_window_line, currln;
static int redraw_everything;
static int insert_character = 1;
/* for copy/paste */
static struct textline *mark_begin, *mark_end;
static int mark_on;

inline static void CLEAR_MARK() {
    mark_on = 0;
    mark_begin = mark_end = NULL;
}

void top_show(char *prompt)
{
    if (editansi) {
        prints(ANSI_RESET);
    }
    move(0, 0);
    clrtoeol();
    prints("\x1b[7m%s\x1b[m", prompt);
}

int ask(char *prompt)
{
    int ch;

    top_show(prompt);
    ch = igetkey();
    move(0, 0);
    clrtoeol();
    return (ch);
}

static int Origin(struct textline *text);
static int process_ESC_action(int action, int arg);

void msgline()
{
    char buf[256], buf2[STRLEN * 2];
    void display_buffer();

#ifndef VEDITOR
    extern int talkrequest;
#else
    char ptr[6];
#endif
    int tmpshow;
    time_t now;

    if (ismsgline <= 0)
        return;
    now = time(0);
    tmpshow = showansi;
    showansi = 1;
#ifndef VEDITOR
    if (talkrequest) {
        talkreply();
        clear();
        showansi = 0;
        display_buffer();
        showansi = 1;
    }
    if (DEFINE(currentuser,DEF_HIGHCOLOR))
        strcpy(buf, "[1;33m[44m");
    else
        strcpy(buf, "[33m[44m");
    if (chkmail())
        strcat(buf, "¡¾[32mÐÅ[33m¡¿");
    else
        strcat(buf, "¡¾  ¡¿");

    /* Leeward 98.07.30 Change hot key for msgX */
    /*strcat(buf," [31mCtrl-Z[33m Çó¾È         "); */
    strcat(buf, " [31mCtrl-Q[33m Çó¾È         ");
    sprintf(buf2, " ×´Ì¬ [%s][%4.4d,%3.3d]      Ê±¼ä", insert_character ? "²åÈë" : "Ìæ»»", currln + 1, currpnt + 1);
    strcat(buf, buf2);
    sprintf(buf2, "[33m[44m¡¾%.16s¡¿", ctime(&now));
#else
    /*´ËÈíÌåËäÎª Freeware £¬µ«Çë²»ÒªÐÞ¸ÄÒÔÏÂµÄ×ÖÑù£¬Ð»Ð» */
    strcpy(buf, "[36m[44mÖÐÕý×Ê¹¤ BBS Éè¼Æ [31mCtrl-Z[33m Çó¾È");
    sprintf(buf2, " [%s][%6d,%3d]", insert_character ? "Ins" : "Rep", currln + 1, currpnt + 1);
    strcat(buf, buf2);
    sprintf(ptr, "%.5s", ctime(&now) + 11);
    sprintf(buf2, "[33m[44m %22.22s [m%.2s:%.2s][m", currfname, ptr, ptr + 3);
#endif
    strcat(buf, buf2);
    move(t_lines - 1, 0);
    prints("%s", buf);
    clrtoeol();
    resetcolor();
    showansi = tmpshow;
}

void domsg()
{
    int x, y;
    int tmpansi;

    tmpansi = showansi;
    showansi = 1;
    getyx(&x, &y);
    msgline();

    move(x, y);
    showansi = tmpansi;
    return;
}

/* ³ö´íÁË*/
void indigestion(int i)
{
    prints("SERIOUS INTERNAL INDIGESTION CLASS %d\n", i);
    oflush();
}

/* ÏòÇ°num ÐÐ,Í¬Ê±¼ÆËãÕæÊµÒÆ¶¯ÐÐÊý·ÅÈëmoveln*/
struct textline *back_line(struct textline *pos,int num)
{
    moveln = 0;
    while (num-- > 0)
        if (pos && pos->prev) {
            pos = pos->prev;
            moveln++;
        }

    return pos;
}

/* Ïòºónum ÐÐ,Í¬Ê±¼ÆËãÕæÊµÒÆ¶¯ÐÐÊý·ÅÈëmoveln*/
struct textline *forward_line(pos, num)
    struct textline *pos;
    int num;
{
    moveln = 0;
    while (num-- > 0)
        if (pos && pos->next) {
            pos = pos->next;
            moveln++;
        }
    return pos;
}

void countline()
{
    struct textline *pos;

    pos = firstline;
    moveln = 0;
    while (pos != lastline) {
        pos = pos->next;
        moveln++;
    }
}

int getlineno()
{
    int cnt = 0;
    struct textline *p = currline;

    while (p != top_of_win) {
        if (p == NULL)
            break;
        cnt++;
        p = p->prev;
    }
    return cnt;
}

char *killsp(s)
    char *s;
{
    while (*s == ' ')
        s++;
    return s;
}

struct textline *alloc_line()
{
    register struct textline *p;

    p = (struct textline *) malloc(sizeof(*p));
    if (p == NULL) {
        indigestion(13);
        abort_bbs(0);
    }
    p->next = NULL;
    p->prev = NULL;
    p->data[0] = '\0';
    p->len = 0;
    p->attr = 0;                /* for copy/paste */
    return p;
}

/*
  Appends p after line in list.  keeps up with last line as well.
 */

void goline(n)
    int n;
{
    register struct textline *p = firstline;
    int count;

    if (n < 0)
        n = 1;
    if (n == 0)
        return;
    for (count = 1; count < n; count++) {
        if (p) {
            p = p->next;
            continue;
        } else
            break;
    }
    if (p) {
        currln = n - 1;
        curr_window_line = 0;
        top_of_win = p;
        currline = p;
    } else {
        top_of_win = lastline;
        currln = count - 2;
        curr_window_line = 0;
        currline = lastline;
    }
    if (Origin(currline)) {
        currline = currline->prev;
        top_of_win = currline;
        curr_window_line = 0;
        currln--;
    }
    if (Origin(currline->prev)) {
        currline = currline->prev->prev;
        top_of_win = currline;
        curr_window_line = 0;
        currln -= 2;
    }


}

void go()
{
    char tmp[8];
    int line;

    set_alarm(0, 0, NULL, NULL);
    getdata(23, 0, "ÇëÎÊÒªÌøµ½µÚ¼¸ÐÐ: ", tmp, 7, DOECHO, NULL, 1);
    domsg();
    if (tmp[0] == '\0')
        return;
    line = atoi(tmp);
    goline(line);
    return;
}


void searchline(text)
    char text[STRLEN];
{
    int tmpline;
    int addr;
    int tt;

    register struct textline *p = currline;
    int count = 0;

    tmpline = currln;
    for (;; p = p->next) {
        count++;
        if (p) {
            if (count == 1)
                tt = currpnt;
            else
                tt = 0;
            if (strstr(p->data + tt, text)) {
                addr = (int) strstr(p->data + tt, text) - (int) p->data + strlen(text);
                currpnt = addr;
                break;
            }
        } else
            break;
    }
    if (p) {
        currln = currln + count - 1;
        curr_window_line = 0;
        top_of_win = p;
        currline = p;
    } else {
        goline(currln + 1);
    }
    if (Origin(currline)) {
        currline = currline->prev;
        top_of_win = currline;
        curr_window_line = 0;
        currln--;
    }
    if (Origin(currline->prev)) {
        currline = currline->prev->prev;
        top_of_win = currline;
        curr_window_line = 0;
        currln -= 2;
    }

}

void search()
{
    char tmp[STRLEN];

	tmp[0]='\0';
    set_alarm(0, 0, NULL, NULL);
    getdata(23, 0, "ËÑÑ°×Ö´®: ", tmp, 65, DOECHO, NULL, 0);
    domsg();
    if (tmp[0] == '\0')
        return;
    else
        strcpy(searchtext, tmp);

    searchline(searchtext);
    return;
}


void append(p, line)
    register struct textline *p, *line;
{
    p->next = line->next;
    if (line->next)
        line->next->prev = p;
    else
        lastline = p;
    line->next = p;
    p->prev = line;
}

/*
  delete_line deletes 'line' from the list and maintains the lastline, and 
  firstline pointers.
 */

void delete_line(line)
    register struct textline *line;
{
    /* if single line */
    if (!line->next && !line->prev) {
        line->data[0] = '\0';
        line->len = 0;
        CLEAR_MARK();
        return;
    }
#define ADJUST_MARK(p, q) if(p == q) p = (q->next) ? q->next : q->prev

    ADJUST_MARK(mark_begin, line);
    ADJUST_MARK(mark_end, line);

    if (line->next)
        line->next->prev = line->prev;
    else
        lastline = line->prev;  /* if on last line */

    if (line->prev)
        line->prev->next = line->next;
    else
        firstline = line->next; /* if on first line */

    free(line);
}

/*
  split splits 'line' right before the character pos
 */

void split(line, pos)
    register struct textline *line;
    register int pos;
{
    register struct textline *p;

    countline();
    if (moveln>MAX_EDIT_LINE) {
        return;
    }
        
    if (pos > line->len) {
        return;
    }
    p = alloc_line();

    p->len = line->len - pos;
    line->len = pos;
    strcpy(p->data, (line->data + pos));
    p->attr = line->attr;       /* for copy/paste */
    *(line->data + pos) = '\0';
    append(p, line);
    if (line == currline && pos <= currpnt) {
        currline = p;
        currpnt -= pos;
        curr_window_line++;
        currln++;
    }
    redraw_everything = true;
}

/*
  join connects 'line' and the next line.  It returns true if:
  
  1) lines were joined and one was deleted
  2) lines could not be joined
  3) next line is empty

  returns false if:

  1) Some of the joined line wrapped
 */

int join(line)
    register struct textline *line;
{
    register int ovfl;

    if (!line->next)
        return true;
    /*if(*killsp(line->next->data) == '\0')
       return true ; */
    ovfl = line->len + line->next->len - WRAPMARGIN;
    if (ovfl < 0) {
        strcat(line->data, line->next->data);
        line->len += line->next->len;
        delete_line(line->next);
        return true;
    } else {
        register char *s;
        register struct textline *p = line->next;

        s = p->data + p->len - ovfl - 1;
        while (s != p->data && *s == ' ')
            s--;
        while (s != p->data && *s != ' ')
            s--;
        if (s == p->data)
            return true;
        split(p, (s - p->data) + 1);
        if (line->len + p->len >= WRAPMARGIN) {
            indigestion(0);
            return true;
        }
        join(line);
        p = line->next;
        if (p->len >= 1 && p->len + 1 < WRAPMARGIN) {
            if (p->data[p->len - 1] != ' ') {
                strcat(p->data, " ");
                p->len++;
            }
        }
        return false;
    }
}

void insert_char(ch)
    register int ch;
{
    register int i;
    register char *s;
    register struct textline *p = currline;
    int wordwrap = true;

    if (currpnt > p->len) {
        indigestion(1);
        return;
    }
    if (currpnt < p->len && !insert_character) {
        p->data[currpnt++] = ch;
    } else {
        for (i = p->len; i >= currpnt; i--)
            p->data[i + 1] = p->data[i];
        p->data[currpnt] = ch;
        p->len++;
        currpnt++;
    }
    if (p->len < WRAPMARGIN)
        return;
    s = p->data + (p->len - 1);
    while (s != p->data && *s == ' ')
        s--;
    while (s != p->data && *s != ' ')
        s--;
    if (s == p->data) {
        wordwrap = false;
        s = p->data + (p->len - 2);
    }

    {                           /* Leeward 98.07.28 */
        int ich, lln;

        if (((unsigned char) *s) > 127) {       /* ±ÜÃâÔÚºº×ÖÖÐ¼äÕÛÐÐ */
            for (ich = 0, lln = s - p->data + 1; lln > 0; lln--)
                if (((unsigned char) p->data[lln - 1]) < 128)
                    break;
                else
                    ich++;
            if (ich % 2)
                s--;
        }
    }

    split(p, (s - p->data) + 1);
    p = p->next;
    if (wordwrap && p->len >= 1) {
        i = p->len;
        if (p->data[i - 1] != ' ') {
            p->data[i] = ' ';
            p->data[i + 1] = '\0';
            p->len++;
        }
        {
        }
    }
    /*while(!join(p)) { Leeward 98.07.29 ±ÜÃâÕÛÐÐºó¸²¸ÇÏÂÒ»ÐÐ£¬ÂÒÂë
       p = p->next ;
       if(p == NULL) {
       indigestion(2) ;
       break ;
       }
       } */
    if (Origin(currline)) {
        currline = p->prev;
        curr_window_line--;
        currln--;
    }
}

void ve_insert_str(str)
    char *str;
{
    while (*str)
        insert_char(*(str++));
}

void delete_char()
{
    register int i;

    if (currline->len == 0)
        return;
    if (currpnt >= currline->len) {
        indigestion(1);
        return;
    }
    for (i = currpnt; i != currline->len; i++)
        currline->data[i] = currline->data[i + 1];
    currline->len--;
}

void vedit_init()
{
    register struct textline *p = alloc_line();

    first_mark_line = 0;
    firstline = p;
    lastline = p;
    currline = p;
    currpnt = 0;
    marknum = 0;
    process_ESC_action('M', '0');
    top_of_win = p;
    curr_window_line = 0;
    currln = 0;
    redraw_everything = false;
    CLEAR_MARK();
}

void insert_to_fp(fp)
    FILE *fp;
{
    int ansi = 0;
    struct textline *p;

    for (p = firstline; p; p = p->next)
        if (p->data[0]) {
            fprintf(fp, "%s\n", p->data);
            if (strchr(p->data, '\033'))
                ansi++;
        }
    if (ansi)
        fprintf(fp, "%s\n", ANSI_RESET);
}

static void insertch_from_fp(int ch)
{
    if (isprint2(ch) || ch == 27) {
        if (currpnt < 254)
            insert_char(ch);
        else if (ch < 255)
            insert_char('.');
    } else if (ch == Ctrl('I')) {
        do {
            insert_char(' ');
        } while (currpnt & 0x7);
    } else if (ch == '\n') {
        split(currline, currpnt);
    }
}
static long insert_from_fp(FILE *fp, char *tmpattachfile)
{
    int ch;
    char attachpad[10];
    int matched;
    char* ptr;
    long size;
	int ret=0;
	int fdst=0;

	if( tmpattachfile && tmpattachfile[0] ){
        if ((fdst = open(tmpattachfile, O_WRONLY | O_CREAT, 0600)) == NULL) 
			ret = -1;
	}
	
    matched=0;
    BBS_TRY {
        if (safe_mmapfile_handle(fileno(fp), O_RDONLY, PROT_READ, MAP_SHARED, (void **) &ptr, (size_t *) & size) == 1) {
            char* data;
            long not;
            data=ptr;
            for (not=0;not<size;not++,data++) {
                if (*data==0) {
                    matched++;
                    if (matched==ATTACHMENT_SIZE) {
                        int d, size;
                        data++; not++;
						if( fdst != NULL ){
    						char o[8]={0,0,0,0,0,0,0,0};
							write(fdst, o, 8);
						}
						if(ret == 0)
							ret = data - ptr - ATTACHMENT_SIZE + 1;
                        while(*data){
							if( fdst != NULL ){
								write(fdst, data, 1);
							}
							data++;
							not++;
						}
						if( fdst != NULL ){
							write(fdst, data, 5);
						}
                        data++;
                        not++;
                        memcpy(&d, data, 4);
                        size = htonl(d);
						if( fdst != NULL ){
							char *atmp = data + 4;
							int i=size;
							int rd,wt;
							for(; i>0;){
								if( i > 10240 )
									rd = 10240;
								else
									rd = i;
                    			wt = write(fdst, atmp, rd);
								if( wt <= 0)
									break;
								atmp += wt;
								i -= wt;
							}
						}
                        data+=4+size-1;
                        not+=4+size-1;
                        matched = 0;
                    }
                    continue;
                }
                insertch_from_fp(*data);
            }
        }
	else
	{
		BBS_RETURN(-1);
	}
    }
    BBS_CATCH {
    }
    BBS_END end_mmapfile((void *) ptr, size, -1);

	if( fdst != NULL )
		close(fdst);

	if(ret <= 0) return 0;
    return ret;
}

long read_file(char *filename,char *tmpattachfile)
{
    FILE *fp;
    long ret;

    if (currline == NULL)
        vedit_init();
    if ((fp = fopen(filename, "r+b")) == NULL) {
        if ((fp = fopen(filename, "w+")) != NULL) {
            fclose(fp);
            return;
        }
        indigestion(4);
        abort_bbs(0);
    }
    ret=insert_from_fp(fp, tmpattachfile);
    fclose(fp);
    return ret;
}

#define KEEP_EDITING -2

int valid_article(pmt, abort)
    char *pmt, *abort;
{
    struct textline *p = firstline;
    char ch;
    int total, lines, len, sig, y;
    int temp;

#ifndef VEDITOR
    if (uinfo.mode == POSTING) {
        total = lines = len = sig = 0;
        while (p != NULL) {
            if (!sig) {
                ch = p->data[0];
                if (strcmp(p->data, "--") == 0)
                    sig = 1;
                else if ((ch != ':' && ch != '>' && ch != '=') && (strlen(p->data) > 2)) {
                    lines++;
                    len += strlen(p->data);
                }
            }
            total++;
            p = p->next;
        }
        y = 2;
        if (total > 20 + lines * 3) {
            move(y, 0);
            prints("±¾ÆªÎÄÕÂµÄÒýÑÔÓëÇ©ÃûµµÐÐÊýÔ¶³¬¹ý±¾ÎÄ³¤¶È.\n");
            y += 3;
        }
        if (total!=lines) 
            lines--; /*Èç¹ûÊÇreÎÄºÍÇ©Ãûµµ£¬Ó¦¸Ã¼õµôÒ»ÐÐ*/
        temp = 0;
        if (len < 8 || lines==0 || (lines<=2 && (len/lines) < 16)) {
            move(y, 0);
            prints("±¾ÆªÎÄÕÂ·Ç³£¼ò¶Ì, ÏµÍ³ÈÏÎª¹àË®ÎÄÕÂ.\n");
            /*Haohmaru.99.4.02.ÈÃ°®¹àË®µÄÈË¿ÞÈ¥°É//grin */
            y += 3;
            temp = 1;
        }

        // local_article Ã»ÓÐ³õÊ¼»¯ by zixia
        local_article = LOCAL_ARTICLE_DEFAULT; // È±Ê¡µÄ×ªÐÅÉèÖÃ

        if (temp){
            local_article = 1; /* ¹àË®µÄÎÄÕÂ¾Í²»Òª×ª³öÈ¥ÁË°É :P by flyriver */
        }

        if (local_article == 1)
            strcpy(pmt, "(L)Õ¾ÄÚ, (S)×ªÐÅ, (F)×Ô¶¯»»ÐÐ·¢±í, (A)È¡Ïû, (T)¸ü¸Ä±êÌâ or (E)ÔÙ±à¼­? [L]: ");
        else
            strcpy(pmt, "(S)×ªÐÅ, (L)Õ¾ÄÚ, (F)×Ô¶¯»»ÐÐ·¢±í, (A)È¡Ïû, (T)¸ü¸Ä±êÌâ or (E)ÔÙ±à¼­? [S]: ");
    }
#endif

    getdata(0, 0, pmt, abort, 3, DOECHO, NULL, true);
    switch (abort[0]) {
    case 'A':
    case 'a':                  /* abort */
    case 'E':
    case 'e':                  /* keep editing */
        return temp;
    }
    return temp;

}

static int write_file(char* filename,int saveheader,long* effsize,long* pattachpos, char *tmpattachfile)
{
    struct textline *p = firstline;
    FILE *fp;
    char abort[6];
    int aborted = 0;
    int temp;
    long sign_size;
    int ret;
    extern char quote_title[120], quote_board[120];
    extern int Anony;
    int long_flag; //for long line check
#ifdef FILTER
    int filter = 0;
#endif

#ifndef VEDITOR
    char p_buf[100];

    set_alarm(0, 0, NULL, NULL);
    clear();

    if (uinfo.mode != CCUGOPHER) {
        /*
           if ( uinfo.mode == EDITUFILE||uinfo.mode==CSIE_ANNOUNCE||
           uinfo.mode == EDITSFILE)
           strcpy(p_buf,"(S)´¢´æµµ°¸, (A)·ÅÆú±à¼­, (E)¼ÌÐø±à¼­? [S]: " );
           else if ( uinfo.mode == SMAIL )
           strcpy(p_buf,"(S)¼Ä³ö, (A)È¡Ïû, or (E)ÔÙ±à¼­? [S]: " );
           else if ( local_article == 1 )
           strcpy(p_buf,"(L)²»×ªÐÅ, (S)×ªÐÅ, (A)È¡Ïû, (T)¸ü¸Ä±êÌâ or (E)ÔÙ±à¼­? [L]: ");
           else
           strcpy(p_buf,"(S)×ªÐÅ, (L)²»×ªÐÅ, (A)È¡Ïû, (T)¸ü¸Ä±êÌâ or (E)ÔÙ±à¼­? [S]: ");
         */
        if (uinfo.mode == POSTING) {
            /*strcpy(p_buf,"(S)·¢±í, (F)×Ô¶¯»»ÐÐ·¢±í, (A)È¡Ïû, (T)¸ü¸Ä±êÌâ or (E)ÔÙ±à¼­? [S]: "); */
#ifndef NINE_BUILD        
            move(4, 0);         /* Haohmaru 99.07.17 */
            prints
                ("Çë×¢Òâ£º±¾Õ¾Õ¾¹æ¹æ¶¨£ºÍ¬ÑùÄÚÈÝµÄÎÄÕÂÑÏ½ûÔÚ 5 (º¬)¸öÒÔÉÏÌÖÂÛÇøÄÚÖØ¸´ÕÅÌù¡£\n\nÎ¥·´Õß³ýËùÌùÎÄÕÂ»á±»É¾³ýÖ®Íâ£¬»¹½«±»°þ¶á¼ÌÐø·¢±íÎÄÕÂµÄÈ¨Á¦¡£ÏêÏ¸¹æ¶¨Çë²ÎÕÕ£º\n\n    Announce °æµÄÕ¾¹æ£º¡°¹ØÓÚ×ªÌùºÍÕÅÌùÎÄÕÂµÄ¹æ¶¨¡±¡£\n\nÇë´ó¼Ò¹²Í¬Î¬»¤ BBS µÄ»·¾³£¬½ÚÊ¡ÏµÍ³×ÊÔ´¡£Ð»Ð»ºÏ×÷¡£\n\n");
#endif
        } else if (uinfo.mode == SMAIL)
            strcpy(p_buf, "(S)¼Ä³ö, (F)×Ô¶¯»»ÐÐ¼Ä³ö, (A)È¡Ïû, or (E)ÔÙ±à¼­? [S]: ");
        else if (uinfo.mode == IMAIL)
            strcpy(p_buf, NAME_BBS_NICK " Internet ÐÅ¼ã£º(S)¼Ä³ö, (F)×Ô¶¯»»ÐÐ¼Ä³ö, (A)È¡Ïû, or (E)ÔÙ±à¼­? [S]: ");      /* Leeward 98.01.17 Prompt whom you are writing to */
        /*    sprintf(p_buf,"¸ø %s µÄÐÅ£º(S)¼Ä³ö, (F)×Ô¶¯»»ÐÐ¼Ä³ö, (A)È¡Ïû, or (E)ÔÙ±à¼­? [S]: ", lookupuser->userid ); 
           Leeward 98.01.17 Prompt whom you are writing to */
        else
            strcpy(p_buf, "(S)´¢´æµµ°¸, (F)×Ô¶¯»»ÐÐ´æ´¢, (A)·ÅÆú±à¼­, (E)¼ÌÐø±à¼­? [S]: ");
        ret = valid_article(p_buf, abort);
        if (abort[0] == '\0') {
            if (local_article == 1)
                abort[0] = 'l';
            else
                abort[0] = 's';
        }
        /* Removed by flyriver, 2002.7.1, no need to modify abort[0] here. */
        /*if(abort[0]!='T' && abort[0]!='t' && abort[0]!='F' && abort[0]!='f' && abort[0]!='A' && abort[0]!='a' &&abort[0]!='E' &&abort[0]!='e' && abort[0] != 'L' && abort[0] != 'l')
           abort[0]='s'; */
    } else
        abort[0] = 'a';
#else                           /* use VEDITOR */
    valid_article("(S)´¢´æ, (A)È¡Ïû, or (E)ÔÙ±à¼­? [S]: ", abort);
    if (abort[0] != 'A' && abort[0] != 'a' && abort[0] != 'E' && abort[0] != 'e')
        abort[0] = 's';
#endif

#ifdef FILTER
    if (((abort[0] != 'a')&&(abort[0] != 'e'))&&
        (uinfo.mode==EDIT)) {
    while (p != NULL) {
        if(check_badword_str(p->data, strlen(p->data))) {
            abort[0] = 'e';
            filter = 1;
            break;
        }
        p = p->next;
    }
    p = firstline;
    }
#endif
    if (abort[0] == 'a' || abort[0] == 'A') {
        struct stat stbuf;

        clear();
#ifndef VEDITOR
        if (uinfo.mode != CCUGOPHER) {
#endif
            prints("È¡Ïû...\n");
            refresh();
            sleep(1);
#ifndef VEDITOR
        }
#endif
        if (stat(filename, &stbuf) || stbuf.st_size == 0)
            unlink(filename);
		if( pattachpos && *pattachpos )
			unlink(tmpattachfile);
        aborted = -1;
    } else if (abort[0] == 'e' || abort[0] == 'E') {
#ifdef FILTER
        if (filter) {
            clear();
            move (3, 0);
            prints ("\n\n            ºÜ±§Ç¸£¬±¾ÎÄ¿ÉÄÜº¬ÓÐ²»ÊÊÒËÄÚÈÝ£¬ÇëÖØÐÂ±à¼­...\n");
            pressreturn();
        }
#endif
        domsg();
        return KEEP_EDITING;
    } else if ( (abort[0] == 't' || abort[0] == 'T') && uinfo.mode == POSTING) {
        char buf[STRLEN];

        buf[0] = 0;             /* Leeward 98.08.14 */
        move(1, 0);
        prints("¾É±êÌâ: %s", save_title);
		strcpy(buf,save_title);
        getdata(2, 0, "ÐÂ±êÌâ: ", buf, STRLEN, DOECHO, NULL, 0);
    if (buf[0]!=0) {
            if (strcmp(save_title, buf))
                local_article = 0;
            strncpy(save_title, buf, STRLEN);
            strncpy(quote_title, buf, STRLEN);
    }
    } else if (abort[0] == 's' || abort[0] == 'S' || abort[0] == 'f' || abort[0] == 'F') {
        local_article = 0;
    } else {                    /* Added by flyriver, 2002.7.1, local save */
        abort[0] = 'l';
        local_article = 1;
    }
    firstline = NULL;
    if (!aborted) {
				/* old source */
				/*
        if (*pattachpos) {
            char buf[MAXPATH];
            int fsrc,fdst;
            snprintf(buf,MAXPATH,"%s.attach",filename);
            if ((fsrc = open(filename, O_RDONLY)) != NULL) {
                if ((fdst = open(buf, O_WRONLY|O_CREAT , 0600)) >= 0) {
                    char* src=(char*)malloc(10240);
                    long ret;
                    lseek(fsrc,*pattachpos-1,SEEK_SET);
                    do {
                        ret = read(fsrc, src, 10240);
                        if (ret <= 0)
                            break;
                    } while (write(fdst, src, ret) > 0);
                    close(fdst);
                    free(src);
                }
                close(fsrc);
            }
        }
				*/
				/* ols source end */
        if ((fp = fopen(filename, "w")) == NULL) {
            indigestion(5);
            abort_bbs(0);
        }
    /* Ôö¼Ó×ªÐÅ±ê¼Ç czz 020819 */
        if (saveheader) {
            if (local_article == 1)
                write_header(fp, currentuser, in_mail, quote_board, quote_title, Anony, 0);
            else
                write_header(fp, currentuser, in_mail, quote_board, quote_title, Anony, 2);
        }
    }
    if (effsize)
        *effsize=0;
    sign_size=0;
    temp=0;  /*±£´æÊÇ·ñ½øÈëÇ©Ãûµ³Î»ÖÃ*/
    while (p != NULL) {
        struct textline *v = p->next;

        if( (!aborted)&&(aborted!=-1)) {
            long_flag = (strlen(p->data) >= WRAPMARGIN -2 );
            if (p->next != NULL || p->data[0] != '\0') {
                if (!strcmp(p->data,"--"))
                    temp=1;
                if (effsize) {
                    if (!strcmp(p->data,"--")) {
/*×¢Òâ´¦Àí
--
fsdfa
--
µÄÇé¿ö*/
                        *effsize+=sign_size;
                        sign_size=2;
                    } else {
                    /*Èç¹û²»ÊÇÇ©Ãûµµ·Ö¸ô·û*/
                        if (sign_size!=0) /*ÔÚ¿ÉÄÜµÄÇ©ÃûµµÖÐ*/
                            sign_size+=strlen(p->data);
                        else
                              *effsize+=strlen(p->data);
                    }
                }
                if (!temp&&(abort[0] == 'f' || abort[0] == 'F')) {       /* Leeward 98.07.27 Ö§³Ö×Ô¶¯»»ÐÐ */
                    unsigned char *ppt = (unsigned char *) p->data;     /* ÕÛÐÐ´¦ */
                    unsigned char *pp = (unsigned char *) ppt;  /* ÐÐÊ× */
                    unsigned const int LINE_LEN=78;
                    unsigned int LLL = LINE_LEN;      /* ÕÛÐÐÎ»ÖÃ */
                    unsigned char *ppx, cc;
                    int ich, lll;

                    while (strlen((char *) pp) > LLL) {
                        lll = 0;
                        ppx = pp;
                        ich = 0;
                        do {
                            if ((ppx = (unsigned char *) strstr((char *) ppx, "[")) != NULL) {
                                int ich=0;
                                while (!isalpha(*(ppx+ich))&&(*(ppx+ich)!=0))
                                    ich++;
                                if (ich > 0)
                                    ich++;
                                else
                                    ich = 2;
                                lll += ich;
                                ppx += 2;
                                ich = 0;
                            }
                        } while (ppx); //¼ÆËãÑÕÉ«×Ö·û£¬×Ö½ÚÊý·ÅÈëlll
                        ppt += LLL + lll; //Ó¦¸ÃÕÛÐÐµÄµØ·½

                        if ((*ppt) > 127) {     /* ±ÜÃâÔÚºº×ÖÖÐ¼äÕÛÐÐ ,KCN:¿´²»¶®faint*/
                            for (ppx = ppt - 1, ich = 0; ppx >= pp; ppx--)
                                if ((*ppx) < 128)
                                    break;
                                else
                                    ich++;
                            if (ich % 2)
                                ppt--;
                        } else if (*ppt) {
                            for (ppx = ppt - 1, ich = 0; ppx >= pp; ppx--)
                                if ((*ppx) > 127 || ' ' == *ppx)
                                    break;
                                else
                                    ich++;
                            if (ppx > pp && ich < 16)
                                ppt -= ich;
                        }

                        cc = *ppt; //±£´æÒ»ÏÂ×Ö·û
                        *ppt = 0;
                        if (':' == p->data[0] && ':' != *pp)
                            fprintf(fp, ": ");
                        fprintf(fp, "%s", pp);
                        if (cc) //»»ÐÐÀ­
                            fprintf(fp, "\n");
                        *ppt = cc;
                        pp = ppt;
                        LLL = LINE_LEN;
                    }
                    LLL = LINE_LEN ;
                    if (':' == p->data[0] && ':' != *pp) //´¦ÀíÊ£ÓàµÄ×Ö·û
                        fprintf(fp, ": ");
                    fprintf(fp, "%s", pp);
                    if (long_flag){	/* Èç¹ûµ±Ç°´®ÊÇÏµÍ³×Ô¶¯½Ø¶ÏµÄÒ»¸ö³¬³¤´® */
                    	    LLL = LLL - strlen(pp); //½«ÏÂÒ»ÐÐµÄÔÊÐí³¤¶È¼õ¶Ì
			}else{
			    fprintf(fp,"\n");
			}
                } else
                    fprintf(fp, "%s\n", p->data);
                }
            }
        free(p);
        p = v;
    }
    if (!aborted) {
        fclose(fp);
        if (pattachpos && *pattachpos) {
				/* old source
            char buf[MAXPATH];
            int fsrc,fdst;
            struct stat st;
            snprintf(buf,MAXPATH,"%s.attach",filename);
            stat(filename,&st);
            *pattachpos=st.st_size+1;
            if (aborted!=-1)
                f_catfile(buf,filename);
            f_rm(buf);
				*/
            struct stat st;
            stat(filename,&st);
            *pattachpos=st.st_size+1;
            f_catfile(tmpattachfile,filename);
            f_rm(tmpattachfile);
        }
    }
    currline = NULL;
    lastline = NULL;
    firstline = NULL;
#ifndef VEDITOR
    if (abort[0] == 'l' || abort[0] == 'L' || local_article == 1) {
        sprintf(genbuf, "local_article = %u", local_article);
        bbslog("user","%s",genbuf);
        local_article = 0;
        if (aborted != -1)
            aborted = 1;        /* aborted = 1 means local save */
    }
#endif
    if ((uinfo.mode == POSTING) && strcmp(currboard->filename, "test")) { /*Haohmaru.99.4.02.ÈÃ°®¹àË®µÄÈË¿ÞÈ¥°É//grin */
        if (ret)
            temp_numposts++;
        if (temp_numposts > 20)
            Net_Sleep((temp_numposts - 20) * 1 + 1);
    }
    return aborted;
}

void keep_fail_post()
{
    char filename[STRLEN];
    char tmpbuf[30];
    struct textline *p = firstline;
    FILE *fp;

    sethomepath(tmpbuf, currentuser->userid);
    sprintf(filename, "%s/%s.deadve", tmpbuf, currentuser->userid);
    if ((fp = fopen(filename, "w")) == NULL) {
        indigestion(5);
        return;
    }
    while (p != NULL) {
        struct textline *v = p->next;

        if (p->next != NULL || p->data[0] != '\0')
            fprintf(fp, "%s\n", p->data);
        free(p);
        p = v;
    }
    return;
}


void strnput(str)
    char *str;
{
    int count = 0;

    while ((*str != '\0') && (++count < scr_cols)) {
        if (*str == KEY_ESC) {
            outc('*');
            str++;
            continue;
        }
        outc(*str++);
    }
}

void cstrnput(str)
    char *str;
{
    int count = 0;

    prints("%s", ANSI_REVERSE);
    while ((*str != '\0') && (++count < scr_cols)) {
        if (*str == KEY_ESC) {
            outc('*');
            str++;
            continue;
        }
        outc(*str++);
    }
    while (++count < scr_cols)
        outc(' ');
    clrtoeol();
    prints("%s", ANSI_RESET);
}


/*Function Add by SmallPig*/
static int Origin(struct textline *text)
{
#ifndef VEDITOR
    char tmp[STRLEN];

    if (uinfo.mode != EDIT)
        return 0;
    if (!text)
        return 0;
    sprintf(tmp, "¡ù À´Ô´:¡¤%s ", BBS_FULL_NAME);
    if (strstr(text->data, tmp) && *text->data != ':')
        return 1;
    else
        return 0;
#else
    return 0;
#endif
}

void display_buffer()
{
    register struct textline *p;
    register int i;
    int shift;
    int temp_showansi;

    temp_showansi = showansi;

    for (i=0; i<t_lines-1; i++) {
        move(i, 0); clrtoeol();
    }
    move(0, 0);
    for (p = top_of_win, i = 0; i < t_lines - 1; i++) {
        if (p) {
            shift = (currpnt + 2 > scr_cols) ? (currpnt / (scr_cols - scrollen)) * (scr_cols - scrollen) : 0;
            if (editansi) {
                showansi = 1;
                prints("%s", p->data);
            } else if ((p->attr & M_MARK)) {
                showansi = 1;
                clear_whole_line(i);
                cstrnput(p->data + shift);
            } else {
                if (p->len >= shift) {
                    showansi = 0;
                    strnput(p->data + shift);
                } 
//                else
//                    clrtoeol();
            }
            p = p->next;
        } else
            prints("%s~", editansi ? ANSI_RESET : "");
        prints("\n");
    }

    showansi = temp_showansi;
    msgline();
    return;
}

int vedit_process_ESC(arg)
    int arg;                    /* ESC + x */
{
    int ch2, action;

#define WHICH_ACTION_COLOR "(M)Çø¿é´¦Àí (I/E)¶ÁÈ¡/Ð´Èë¼ôÌù²¾ (C)Ê¹ÓÃ²ÊÉ« (F/B/R)Ç°¾°/±³¾°/»¹Ô­É«²Ê"
#define WHICH_ACTION_MONO  "(M)Çø¿é´¦Àí (I/E)¶ÁÈ¡/Ð´Èë¼ôÌù²¾ (C)Ê¹ÓÃµ¥É« (F/B/R)Ç°¾°/±³¾°/»¹Ô­É«²Ê"

#define CHOOSE_MARK     "(0)È¡Ïû±ê¼Ç (1)Éè¶¨¿ªÍ· (2)Éè¶¨½áÎ² (3)¸´ÖÆ±ê¼ÇÄÚÈÝ "
#define FROM_WHICH_PAGE "¶ÁÈ¡¼ôÌù²¾µÚ¼¸Ò³? (0-7) [Ô¤ÉèÎª 0]"
#define SAVE_ALL_TO     "°ÑÕûÆªÎÄÕÂÐ´Èë¼ôÌù²¾µÚ¼¸Ò³? (0-7) [Ô¤ÉèÎª 0]"
#define SAVE_PART_TO    "°ÑÕûÆªÎÄÕÂÐ´Èë¼ôÌù²¾µÚ¼¸Ò³? (0-7) [Ô¤ÉèÎª 0]"
#define FROM_WHICH_SIG  "È¡³öÇ©Ãû²¾µÚ¼¸Ò³? (0-7) [Ô¤ÉèÎª 0]"
#define CHOOSE_FG       "Ç°¾°ÑÕÉ«? 0)ºÚ 1)ºì 2)ÂÌ 3)»Æ 4)ÉîÀ¶ 5)·Ûºì 6)Ç³À¶ 7)°× "
#define CHOOSE_BG       "±³¾°ÑÕÉ«? 0)ºÚ 1)ºì 2)ÂÌ 3)»Æ 4)ÉîÀ¶ 5)·Ûºì 6)Ç³À¶ 7)°× "
#define CHOOSE_ERROR    "Ñ¡Ïî´íÎó"

    switch (arg) {
    case 'M':
    case 'm':
        ch2 = ask(CHOOSE_MARK);
        action = 'M';
        break;
    case 'I':
    case 'i':                  /* import */
        ch2 = ask(FROM_WHICH_PAGE);
        action = 'I';
        break;
    case 'E':
    case 'e':                  /* export */
        ch2 = ask(mark_on ? SAVE_PART_TO : SAVE_ALL_TO);
        action = 'E';
        break;
    case 'S':
    case 's':                  /* signature */
        ch2 = '0';
        action = 'S';
        break;
    case 'F':
    case 'f':
        ch2 = ask(CHOOSE_FG);
        action = 'F';
        break;
    case 'B':
    case 'b':
        ch2 = ask(CHOOSE_BG);
        action = 'B';
        break;
    case 'R':
    case 'r':
        ch2 = '0';              /* not used */
        action = 'R';
        break;
    case 'D':
    case 'd':
        ch2 = '4';
        action = 'M';
        break;
    case 'N':
    case 'n':
        ch2 = '0';
        action = 'N';
        break;
    case 'G':
    case 'g':
        ch2 = '1';
        action = 'G';
        break;
    case 'L':
    case 'l':
        ch2 = '0';              /* not used */
        action = 'L';
        break;
    case 'C':
    case 'c':
        ch2 = '0';              /* not used */
        action = 'C';
        break;
    case 'Q':
    case 'q':                  /* Leeward 98.07.30 Change hot key for msgX */
        marknum = 0;
        ch2 = '0';              /* not used */
        action = 'M';
        break;
    default:
        return 0;
    }

    if (strchr("IES", action) && (ch2 == '\n' || ch2 == '\r'))
        ch2 = '0';

    if (ch2 >= '0' && ch2 <= '7')
        return process_ESC_action(action, ch2);
    else {
        return ask(CHOOSE_ERROR);
    }

    return 0;
}

int mark_block()
{
    struct textline *p;
    int pass_mark = 0;

    first_mark_line = 0;
    if (mark_begin == NULL && mark_end == NULL)
        return 0;
    if (mark_begin == mark_end) {
        mark_begin->attr |= M_MARK;
        return 1;
    }
    if (mark_begin == NULL || mark_end == NULL) {
        if (mark_begin != NULL)
            mark_begin->attr |= M_MARK;
        else
            mark_end->attr |= M_MARK;
        return 1;
    } else {
        for (p = firstline; p != NULL; p = p->next) {
            if (p == mark_begin || p == mark_end) {
                pass_mark++;
                p->attr |= M_MARK;
                continue;
            }
            if (pass_mark == 1)
                p->attr |= M_MARK;
            else {
                first_mark_line++;
                p->attr &= ~(M_MARK);
            }
            if (pass_mark == 2)
                first_mark_line--;
        }
        return 1;
    }
}

void process_MARK_action(arg, msg)
    int arg;                    /* operation of MARK */
    char *msg;                  /* message to return */
{
    struct textline *p;
    int dele_1line;

    switch (arg) {
    case '0':                  /* cancel */
        for (p = firstline; p != NULL; p = p->next)
            p->attr &= ~(M_MARK);
        CLEAR_MARK();
        break;
    case '1':                  /* mark begin */
        mark_begin = currline;
        mark_on = mark_block();
        if (mark_on)
            strcpy(msg, "±ê¼ÇÒÑÉè¶¨Íê³É");
        else
            strcpy(msg, "ÒÑÉè¶¨¿ªÍ·±ê¼Ç, ÉÐÎÞ½áÎ²±ê¼Ç");
        break;
    case '2':                  /* mark end */
        mark_end = currline;
        mark_on = mark_block();
        if (mark_on)
            strcpy(msg, "±ê¼ÇÒÑÉè¶¨Íê³É");
        else
            strcpy(msg, "ÒÑÉè¶¨½áÎ²±ê¼Ç, ÉÐÎÞ¿ªÍ·±ê¼Ç");
        break;
    case '3':                  /* copy mark */
        if (mark_on && !(currline->attr & M_MARK)) {
            for (p = firstline; p != NULL; p = p->next) {
                if (p->attr & M_MARK) {
                    ve_insert_str(p->data);
                    split(currline, currpnt);
                }
            }
        } else
            bell();
        strcpy(msg, "±ê¼Ç¸´ÖÆÍê³É");
        break;
    case '4':                  /* delete mark */
        dele_1line = 0;
        if (mark_on && (currline->attr & M_MARK)) {
            if (currline == firstline)
                dele_1line = 1;
            else
                dele_1line = 2;
        }
        for (p = firstline; p != NULL; p = p->next) {
            if (p->attr & M_MARK) {
                currline = p;
                p=p->prev;
                vedit_key(Ctrl('Y'));
                if (p==NULL) {
                    p=firstline;
                    if (p==NULL);
                        break;
                }
            }
        }
        process_ESC_action('M', '0');
        marknum = 0;
        if (dele_1line == 0 || dele_1line == 2) {
            if (first_mark_line == 0)
                first_mark_line = 1;
            goline(first_mark_line);
        } else
            goline(1);
        break;
    default:
        strcpy(msg, CHOOSE_ERROR);
    }
    strcpy(msg, "\0");
}

static int process_ESC_action(int action, int arg)
/* valid action are I/E/S/B/F/R/C */
/* valid arg are    '0' - '7' */
{
    int newch = 0;
    char msg[80], buf[80];
    char filename[80];
    FILE *fp;

    msg[0] = '\0';
    switch (action) {
    case 'L':
        if (ismsgline >= 1) {
            ismsgline = 0;
            move(t_lines - 1, 0);
            clrtoeol();
        } else
            ismsgline = 1;
        break;
    case 'M':
        process_MARK_action(arg, msg);
        break;
    case 'I':
        sprintf(filename, "tmp/clip/%s.%c", currentuser->userid, arg);
        if ((fp = fopen(filename, "r")) != NULL) {
            insert_from_fp(fp,NULL);
            fclose(fp);
            sprintf(msg, "ÒÑÈ¡³ö¼ôÌù²¾µÚ %c Ò³", arg);
        } else
            sprintf(msg, "ÎÞ·¨È¡³ö¼ôÌù²¾µÚ %c Ò³", arg);
        break;
    case 'G':
        go();
        redraw_everything = true;
        break;
    case 'E':
        sprintf(filename, "tmp/clip/%s.%c", currentuser->userid, arg);
        if ((fp = fopen(filename, "w")) != NULL) {
            if (mark_on) {
                struct textline *p;

                for (p = firstline; p != NULL; p = p->next)
                    if (p->attr & M_MARK)
                        fprintf(fp, "%s\n", p->data);
            } else
                insert_to_fp(fp);
            fclose(fp);
            sprintf(msg, "ÒÑÌùÖÁ¼ôÌù²¾µÚ %c Ò³", arg);
        } else
            sprintf(msg, "ÎÞ·¨ÌùÖÁ¼ôÌù²¾µÚ %c Ò³", arg);
        break;
    case 'N':
        searchline(searchtext);
        redraw_everything = true;
        break;
    case 'S':
        search();
        redraw_everything = true;
        break;
    case 'F':
        sprintf(buf, "%c[3%cm", 27, arg);
        ve_insert_str(buf);
        break;
    case 'B':
        sprintf(buf, "%c[4%cm", 27, arg);
        ve_insert_str(buf);
        break;
    case 'R':
        ve_insert_str(ANSI_RESET);
        break;
    case 'C':
        editansi = showansi = 1;
        redraw_everything = true;
        clear();
        display_buffer();
        redoscr();
        strcpy(msg, "ÒÑÏÔÊ¾²ÊÉ«±à¼­³É¹û£¬¼´½«ÇÐ»Øµ¥É«Ä£Ê½");
    }

    if (strchr("FBRCM", action))
        redraw_everything = true;

    if (msg[0] != '\0') {
        if (action == 'C') {    /* need redraw */
            move(t_lines - 2, 0);
            clrtoeol();
            prints("[1m%s%s%s[m", msg, ", Çë°´ÈÎÒâ¼ü·µ»Ø±à¼­»­Ãæ...", ANSI_RESET);
            pressanykey();
            newch = '\0';
            editansi = showansi = 0;
            clear();
            display_buffer();
        } else
            newch = ask(strcat(msg, "£¬Çë¼ÌÐø±à¼­¡£"));
    } else
        newch = '\0';
    return newch;
}

void vedit_key(ch)
    int ch;
{
    int i;

#define NO_ANSI_MODIFY  if(no_touch) { warn++; break; }

    static int lastindent = -1;
    int no_touch, warn, shift;

    if (ch == Ctrl('P') || ch == KEY_UP || ch == Ctrl('N') || ch == KEY_DOWN) {
        if (lastindent == -1)
            lastindent = currpnt;
    } else
        lastindent = -1;

    no_touch = (editansi && strchr(currline->data, '\033')) ? 1 : 0;
    warn = 0;


    if (ch < 0x100 && isprint2(ch)) {
        if (no_touch)
            warn++;
        else
            insert_char(ch);
    } else
        switch (ch) {
        case Ctrl('Z'):
            r_lastmsg();        /* Leeward 98.07.30 support msgX */
            break;
        case Ctrl('I'):
            NO_ANSI_MODIFY;
            do {
                insert_char(' ');
            } while (currpnt & 0x7);
            break;
        case '\r':
        case '\n':
            NO_ANSI_MODIFY;
            split(currline, currpnt);
            break;
        case Ctrl('G'):        /* redraw screen */
            go();
            redraw_everything = true;
            break;
            /* Leeward 98.07.30 Change hot key for msgX */
            /*case Ctrl('Z'):  call help screen */
        case Ctrl('Q'):        /* call help screen */
#ifndef VEDITOR
            show_help("help/edithelp");
#else
            show_helpmenu(vedithelp);
#endif
            redraw_everything = true;
            break;
        case Ctrl('R'):
#ifdef CHINESE_CHARACTER
            currentuser->userdefine = currentuser->userdefine ^ DEF_CHCHAR;
            break;
#endif            
        case KEY_LEFT:         /* backward character */
            if (currpnt > 0) {
                currpnt--;
            } else if (currline->prev) {
                curr_window_line--;
                currln--;
                currline = currline->prev;
                currpnt = currline->len;
            }
#ifdef CHINESE_CHARACTER
            if (DEFINE(currentuser, DEF_CHCHAR)) {
                int i,j=0;
                for(i=0;i<currpnt;i++)
                    if(j) j=0;
                    else if(currline->data[i]<0) j=1;
                if(j)
                    if (currpnt > 0)
                        currpnt--;
            }
#endif
            break;
            /*case Ctrl('Q'):  Leeward 98.07.30 Change hot key for msgX
               process_ESC_action('M', '0');
               marknum=0;            
               break; */
        case Ctrl('C'):
            process_ESC_action('M', '3');
            break;
        case Ctrl('U'):
            if (marknum == 0) {
                marknum = 1;
                process_ESC_action('M', '1');
            } else
                process_ESC_action('M', '2');
            clear();
            break;
        case Ctrl('V'):
        case KEY_RIGHT:        /* forward character */
            if (currline->len != currpnt) {
                currpnt++;
            } else if (currline->next) {
                currpnt = 0;
                curr_window_line++;
                currln++;
                currline = currline->next;
                if (Origin(currline)) {
                    curr_window_line--;
                    currln--;
                    currline = currline->prev;
                }
            }
#ifdef CHINESE_CHARACTER
            if (DEFINE(currentuser, DEF_CHCHAR)) {
                int i,j=0;
                for(i=0;i<currpnt;i++)
                    if(j) j=0;
                    else if(currline->data[i]<0) j=1;
                if(j)
                    if (currline->len != currpnt)
                        currpnt++;
            }
#endif
            break;
        case Ctrl('P'):
        case KEY_UP:           /* Previous line */
            if (currline->prev) {
                currln--;
                curr_window_line--;
                currline = currline->prev;
                currpnt = (currline->len > lastindent) ? lastindent : currline->len;
            }
#ifdef CHINESE_CHARACTER
            if (DEFINE(currentuser, DEF_CHCHAR)) {
                int i,j=0;
                for(i=0;i<currpnt;i++)
                    if(j) j=0;
                    else if(currline->data[i]<0) j=1;
                if(j)
                    if (currpnt > 0)
                        currpnt--;
            }
#endif
            break;
        case Ctrl('N'):
        case KEY_DOWN:         /* Next line */
            if (currline->next) {
                currline = currline->next;
                curr_window_line++;
                currln++;
                if (Origin(currline)) {
                    currln--;
                    curr_window_line--;
                    currline = currline->prev;
                }
                currpnt = (currline->len > lastindent) ? lastindent : currline->len;
            }
#ifdef CHINESE_CHARACTER
            if (DEFINE(currentuser, DEF_CHCHAR)) {
                int i,j=0;
                for(i=0;i<currpnt;i++)
                    if(j) j=0;
                    else if(currline->data[i]<0) j=1;
                if(j)
                    if (currpnt > 0)
                        currpnt--;
            }
#endif
            break;
        case Ctrl('B'):
        case KEY_PGUP:         /* previous page */
            top_of_win = back_line(top_of_win, 22);
            currline = back_line(currline, 22);
            currln -= moveln;
            curr_window_line = getlineno();
            if (currpnt > currline->len)
                currpnt = currline->len;
            redraw_everything = true;
            break;
        case Ctrl('F'):
        case KEY_PGDN:         /* next page */
            top_of_win = forward_line(top_of_win, 22);
            currline = forward_line(currline, 22);
            currln += moveln;
            curr_window_line = getlineno();
            if (currpnt > currline->len)
                currpnt = currline->len;
            if (Origin(currline->prev)) {
                currln -= 2;
                curr_window_line = 0;
                currline = currline->prev->prev;
                top_of_win = lastline->prev->prev;
            }
            if (Origin(currline)) {
                currln--;
                curr_window_line--;
                currline = currline->prev;
            }
            redraw_everything = true;
            break;
        case Ctrl('A'):
        case KEY_HOME:         /* begin of line */
            currpnt = 0;
            break;
        case Ctrl('E'):
        case KEY_END:          /* end of line */
            currpnt = currline->len;
            break;
        case Ctrl('S'):        /* start of file */
            top_of_win = firstline;
            currline = top_of_win;
            currpnt = 0;
            curr_window_line = 0;
            currln = 0;
            redraw_everything = true;
            break;
        case Ctrl('T'):        /* tail of file */
            top_of_win = back_line(lastline, 22);
            countline();
            currln = moveln;
            currline = lastline;
            curr_window_line = getlineno();
            currpnt = 0;
            if (Origin(currline->prev)) {
                currline = currline->prev->prev;
                currln -= 2;
                curr_window_line -= 2;
            }
            redraw_everything = true;
            break;
        case Ctrl('O'):
        case KEY_INS:          /* Toggle insert/overwrite */
            insert_character = !insert_character;
            /*move(0,73);
               prints( " [%s] ", insert_character ? "Ins" : "Rep" ); */
            break;
        case Ctrl('H'):
        case '\177':           /* backspace */
            NO_ANSI_MODIFY;
            if (currpnt == 0) {
                struct textline *p;

                if (!currline->prev) {
                    break;
                }
                currln--;
                curr_window_line--;
                currline = currline->prev;
                currpnt = currline->len;

                /* Modified by cityhunter on 1999.10.22 */
                /* for the bug of edit two page article */

                if (curr_window_line < 0) {     /*top_of_win = top_of_win->next; */
                    top_of_win = currline;
                    curr_window_line = 0;
                }

                /* end of this modification             */
                if (*killsp(currline->next->data) == '\0') {
                    delete_line(currline->next);
                    redraw_everything = true;
                    break;
                }
                p = currline;
                while (!join(p)) {
                    p = p->next;
                    if (p == NULL) {
                        indigestion(2);
                        abort_bbs(0);
                    }
                }
                redraw_everything = true;
                break;
            }
            currpnt--;
            delete_char();
#ifdef CHINESE_CHARACTER
            if (DEFINE(currentuser, DEF_CHCHAR)) {
                int i,j=0;
                for(i=0;i<currpnt;i++)
                    if(j) j=0;
                    else if(currline->data[i]<0) j=1;
                if(j) {
                    currpnt--;
                    delete_char();
                }
            }
#endif
            break;
        case Ctrl('D'):
        case KEY_DEL:          /* delete current character */
            NO_ANSI_MODIFY;
            if (currline->len == currpnt) {
                struct textline *p = currline;

                if (!Origin(currline->next)) {
                    while (!join(p)) {
                        p = p->next;
                        if (p == NULL) {
                            indigestion(2);
                            abort_bbs(0);
                        }
                    }
                } else if (currpnt == 0)
                    vedit_key(Ctrl('K'));
                redraw_everything = true;
                break;
            }
#ifdef CHINESE_CHARACTER
            if (DEFINE(currentuser, DEF_CHCHAR)) {
                int i,j=0;
                for(i=0;i<currpnt+1;i++)
                    if(j) j=0;
                    else if(currline->data[i]<0) j=1;
                if(j)
                    delete_char();
            }
#endif
            delete_char();
            break;
        case Ctrl('Y'):        /* delete current line */
            /* STONGLY coupled with Ctrl-K */
            no_touch = 0;       /* ANSI_MODIFY hack */
            currpnt = 0;
            if (currline->next) {
                if (Origin(currline->next) && !currline->prev) {
                    currline->data[0] = '\0';
                    currline->len = 0;
                    break;
                }
            } else if (currline->prev != NULL) {
                currline->len = 0;
            } else {
                currline->len = 0;
                currline->data[0] = '\0';
                break;
            }
            currline->len = 0;
            vedit_key(Ctrl('K'));
            break;
        case Ctrl('K'):        /* delete to end of line */
            NO_ANSI_MODIFY;
            if (currline->prev == NULL && currline->next == NULL) {
                currline->data[0] = '\0';
                currpnt = 0;
                break;
            }
            if (currline->next) {
                if (Origin(currline->next) && currpnt == currline->len && currpnt != 0)
                    break;
                if (Origin(currline->next) && currline->prev == NULL) {
                    vedit_key(Ctrl('Y'));
                    break;
                }
            }
            if (currline->len == 0) {
                struct textline *p = currline->next;

                if (!p) {
                    p = currline->prev;
                    if (!p) {
                        break;
                    }
                    if (curr_window_line > 0)
                        curr_window_line--;
                    currln--;
                }
                if (currline == top_of_win)
                    top_of_win = p;
                delete_line(currline);
                currline = p;
                if (Origin(currline)) {
                    currline = currline->prev;
                    curr_window_line--;
                    currln--;
                }
                redraw_everything = true;
                break;
            }
            if (currline->len == currpnt) {
                struct textline *p = currline;

                while (!join(p)) {
                    p = p->next;
                    if (p == NULL) {
                        indigestion(2);
                        abort_bbs(0);
                    }
                }
                redraw_everything = true;
                break;
            }
            currline->len = currpnt;
            currline->data[currpnt] = '\0';
            break;
        default:
            break;
        }

    if (curr_window_line < 0) {
        curr_window_line = 0;
        if (!top_of_win->prev) {
            indigestion(6);
        } else {
            top_of_win = top_of_win->prev;
            /*            redraw_everything = true ;
               move(t_lines-2,0);
               clrtoeol();
               refresh(); */
            rscroll();
        }
    }
    if (curr_window_line >= t_lines - 1) {
        for (i = curr_window_line - t_lines + 1; i >= 0; i--) {
            curr_window_line--;
            if (!top_of_win->next) {
                indigestion(7);
            } else {
                top_of_win = top_of_win->next;
                /*          redraw_everything = true ;
                   move(t_lines-1,0);
                   clrtoeol();
                   refresh(); */
                scroll();
            }
        }
    }

    if (editansi /*|| mark_on */ )
        redraw_everything = true;
    shift = (currpnt + 2 > scr_cols) ? (currpnt / (scr_cols - scrollen)) * (scr_cols - scrollen) : 0;
    msgline();
    if (shifttmp != shift || redraw_everything == true) {
        redraw_everything = true;
        shifttmp = shift;
    } else
        redraw_everything = false;

    move(curr_window_line, 0);
    if (currline->attr & M_MARK) {
        showansi = 1;
        cstrnput(currline->data + shift);
        showansi = 0;
    } else
        strnput(currline->data + shift);
    clrtoeol();
}

static int raw_vedit(char *filename,int saveheader,int headlines,long* eff_size,long* pattachpos)
{
    int newch, ch = 0, foo, shift;
    struct textline *st_tmp, *st_tmp2;
	char tmpattachfile[STRLEN];

    if (pattachpos != NULL && *pattachpos!=0) {
		snprintf(tmpattachfile, STRLEN, "tmp/%s.%d.attedit.bak", currentuser->userid, getpid());
        *pattachpos=read_file(filename,tmpattachfile);
    } else
        // TODO: add zmodem upload
        read_file(filename,NULL);
    top_of_win = firstline;
    for (newch = 0; newch < headlines; newch++)
        if (top_of_win->next)
            top_of_win = top_of_win->next;
    /* Ìø¹ýheadlines Ö¸¶¨ÐÐÊýµÄÍ·²¿ÐÅÏ¢ Luzi 1999/1/8 */
    currline = top_of_win;
    st_tmp2 = firstline;
    st_tmp = currline->prev;    /* ±£´æÁ´±íÖ¸Õë£¬²¢ÐÞ¸Ä±à¼­µÚÒ»ÐÐµÄµÄÖ¸Õë */
    currline->prev = NULL;
    firstline = currline;

    curr_window_line = 0;
    currln = 0;
    currpnt = 0;
    clear();
    display_buffer();
    msgline();
    while (ch != EOF) {
        newch = '\0';
        switch (ch) {
        case Ctrl('W'):
        case Ctrl('X'):        /* Save and exit */
            if (headlines) {
                st_tmp->next = firstline;       /* ÍË³öÊ±ºò»Ö¸´Ô­Á´±í */
                firstline->prev = st_tmp;
                firstline = st_tmp2;
            }
            foo = write_file(filename, saveheader, eff_size,pattachpos,tmpattachfile);
            if (foo != KEEP_EDITING)
                return foo;
            if (headlines) {
                firstline = st_tmp->next;       /* ¼ÌÐø±à¼­ÔòÔÙ´ÎÐÞ¸ÄµÚÒ»ÐÐµÄÖ¸Õë */
                firstline->prev = NULL;
            }
            redraw_everything = true;
            break;
        case KEY_ESC:
            if (KEY_ESC_arg == KEY_ESC)
                insert_char(KEY_ESC);
            else {
                newch = vedit_process_ESC(KEY_ESC_arg);
                clear();
            }
            redraw_everything = true;
            break;
        case KEY_REFRESH:
            redraw_everything = true;
            break;
        default:
            vedit_key(ch);
        }
        if (redraw_everything) {
            display_buffer();
        }
        redraw_everything = false;
        shift = (currpnt + 2 > scr_cols) ? (currpnt / (scr_cols - scrollen)) * (scr_cols - scrollen) : 0;
        move(curr_window_line, currpnt - shift);

        ch = (newch != '\0') ? newch : igetkey();
    }
    return 1;
}

int vedit(char *filename,int saveheader,long* eff_size,long *pattachpos)
{
    int ans, t;
    long attachpos=0;

    t = showansi;
    showansi = 0;
    ismsgline = (DEFINE(currentuser, DEF_EDITMSG)) ? 1 : 0;
    domsg();
    ans = raw_vedit(filename, saveheader, 0,eff_size,pattachpos?pattachpos:&attachpos);
    showansi = t;
    return ans;
}

int vedit_post(char *filename,int saveheader,long* eff_size,long* pattachpos)
{
    int ans, t;

    t = showansi;
    showansi = 0;
    ismsgline = (DEFINE(currentuser, DEF_EDITMSG)) ? 1 : 0;
    domsg();
    ans = raw_vedit(filename, saveheader, 4, eff_size,pattachpos);   /*Haohmaru.99.5.5.Ó¦¸Ã±£ÁôÒ»¸ö¿ÕÐÐ */
    showansi = t;
    return ans;
}
