/*
        ’‚∏ˆ≥Ã Ω «”… Firebird BBS À˘◊®”√µƒ Editor £¨ª∂”≠∏˜Œª π”√
        »Áπ˚”–»Œ∫ŒŒ Ã‚«Î Mail ∏¯ SmallPig.bbs@bbs.cs.ccu.edu.tw
        ªÚ «µΩ 140.123.101.78(bbs.cs.ccu.edu.tw) Post Œ Ã‚°£
*/

#include "bbs.h"
#include "edit.h"

#define MAX_EDIT_LINE 20000
#define LOCAL_ARTICLE_DEFAULT 0

extern int temp_numposts;       /*Haohmaru.99.4.02.»√∞Æπ‡ÀÆµƒ»Àøﬁ»•∞…//grin */
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
static int top_of_line = 0;
static int curr_window_line, currln;
static int insert_character = 1;
/* for copy/paste */
static struct textline *mark_begin, *mark_end;
static int mark_on;

inline static void CLEAR_MARK() {
    mark_on = 0;
    mark_begin = mark_end = NULL;
}

int myy=-1, myx, outii, outjj;
bool show_eof=false;

void display_buffer()
{
    struct textline *p;
    int ii, i, j, ch, y;

    clear();
    y = 0; outii = -1;
    for (p = top_of_win, ii = 0; ii < t_lines - 1; ii++) {
        if (p) {
            if (editansi)
                prints("%s", p->data);
            else {
                if (p->attr & M_MARK) prints("%s", ANSI_REVERSE);
                j = 0; ch = 0;
                for (i = 0; i < p->len; i++) {
                    if (ch) ch = 0;
                    else if (p->data[i]<0) ch = 1;
                    if (p == currline && i == currpnt) {
                        myy = y; myx = j;
                    }
                    if (j >= scr_cols || j >= scr_cols-1 && ch) {
                        outc('\n');
                        y++;
                        j = 0;
                    }
                    if (y >= scr_lns-1) {
                        outii = ii; outjj = i;
                        break;
                    }
                    if (p->data[i]==27) {
                        setfcolor(YELLOW, 0);
                        outc('*');
                        resetcolor();
                    }
                    else outc(p->data[i]);
                    j++;
                }
                if (p == currline && i == currpnt) {
                    myy = y; myx = j;
                }
                if (show_eof) {
                    setfcolor(YELLOW, 0);
                    outc('~');
                }
                resetcolor();
            }
            p = p->next;
            if (show_eof&&!p) {
                setfcolor(GREEN, 0);
                outc('~');
            }
        } else
            prints("%s~", ANSI_RESET);
        outc('\n');
        y++;
        if (y >= scr_lns-1) {
            if (outii==-1) {
                outii = ii+1;
                outjj = 0;
            }
            break;
        }
    }

    msgline();
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

    extern int talkrequest;
    int tmpshow;
    time_t now;

    if (ismsgline <= 0)
        return;
    now = time(0);
    tmpshow = showansi;
    showansi = 1;
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
        strcat(buf, "°æ[32m–≈[33m°ø");
    else
        strcat(buf, "°æ  °ø");

    /* Leeward 98.07.30 Change hot key for msgX */
    /*strcat(buf," [31mCtrl-Z[33m «Ûæ»         "); */
    strcat(buf, " [31mCtrl-Q[33m «Ûæ»         ");
    sprintf(buf2, " ◊¥Ã¨ [%s][%d,%d]       ±º‰", insert_character ? "≤Â»Î" : "ÃÊªª", currln + 1, currpnt + 1);
    strcat(buf, buf2);
    sprintf(buf2, "[33m[44m°æ%.16s°ø", ctime(&now));
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

/* ≥ˆ¥Ì¡À*/
void indigestion(int i)
{
    prints("SERIOUS INTERNAL INDIGESTION CLASS %d\n", i);
    oflush();
}

/* œÚ«∞num ––,Õ¨ ±º∆À„’Ê µ“∆∂Ø–– ˝∑≈»Îmoveln*/
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

/* œÚ∫Ûnum ––,Õ¨ ±º∆À„’Ê µ“∆∂Ø–– ˝∑≈»Îmoveln*/
struct textline *forward_line(struct textline * pos, int num)
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

char *killsp(char * s)
{
    while (*s == ' ')
        s++;
    return s;
}

struct textline *alloc_line()
{
    struct textline *p;

    p = (struct textline *) malloc(sizeof(*p));
    if (p == NULL) {
        indigestion(13);
        abort_bbs(0);
    }
    p->next = NULL;
    p->prev = NULL;
    p->data = malloc(WRAPMARGIN+5);
    p->maxlen = WRAPMARGIN;
    p->data[0] = '\0';
    p->len = 0;
    p->attr = 0;                /* for copy/paste */
    return p;
}

struct textline *alloc_line_w(int w)
{
    struct textline *p;

    p = (struct textline *) malloc(sizeof(*p));
    if (p == NULL) {
        indigestion(13);
        abort_bbs(0);
    }
    p->next = NULL;
    p->prev = NULL;
    p->maxlen = (w/WRAPMARGIN+1)*WRAPMARGIN+5;
    if (p->maxlen<WRAPMARGIN) p->maxlen = WRAPMARGIN;
    p->data = malloc(p->maxlen+1);
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
    struct textline *p = firstline;
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
    getdata(t_lines-1, 0, "«ÎŒ “™Ã¯µΩµ⁄º∏––: ", tmp, 7, DOECHO, NULL, 1);
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

    struct textline *p = currline;
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
    getdata(t_lines-1, 0, "À——∞◊÷¥Æ: ", tmp, 65, DOECHO, NULL, 0);
    domsg();
    if (tmp[0] == '\0')
        return;
    else
        strcpy(searchtext, tmp);

    searchline(searchtext);
    return;
}


void append(struct textline * p, struct textline * line)
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

void delete_line(struct textline * line)
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
    free(line->data);
    free(line);
}

/*
  split splits 'line' right before the character pos
 */

void split(struct textline * line, int pos)
{
    struct textline *p;

    countline();
    if (moveln>MAX_EDIT_LINE) {
        return;
    }
        
    if (pos > line->len) {
        return;
    }
    p = alloc_line_w(line->len - pos);

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
}

/*
  join connects 'line' and the next line.  It returns true if:
  
  1) lines were joined and one was deleted
  2) lines could not be joined
  3) next line is empty

  returns false if:

  1) Some of the joined line wrapped
 */

int join(struct textline * line)
{
    int ovfl;

    if (!line->next)
        return true;
    if (line->maxlen<=line->len+line->next->len+5) {
        int ml;
        char *q;
        ml = ((line->len+line->next->len)/WRAPMARGIN+1)*WRAPMARGIN+5;
        if (ml<WRAPMARGIN) ml = WRAPMARGIN;
        q = (char *) malloc(ml+1);
        memcpy(q, line->data, line->len+1);
        free(line->data);
        line->data = q;
        line->maxlen = ml;
    }
    strcat(line->data, line->next->data);
    line->len += line->next->len;
    delete_line(line->next);
    return true;
}

void insert_char(int ch)
{
    int i;
    char *s;
    struct textline *p = currline;
    int wordwrap = true;

    if (currpnt > p->len) {
        indigestion(1);
        return;
    }
    if (currpnt > MAX_EDIT_LINE)
        return;
    if (currpnt < p->len && !insert_character) {
        p->data[currpnt++] = ch;
    } else {
        for (i = p->len; i >= currpnt; i--)
            p->data[i + 1] = p->data[i];
        p->data[currpnt] = ch;
        p->len++;
        currpnt++;
    }
    if (p->len >= p->maxlen-5) {
        char *q = (char *)malloc(p->maxlen+WRAPMARGIN+5);
        memcpy(q, p->data, p->len+1);
        free(p->data);
        p->data = q;
        p->maxlen += WRAPMARGIN;
    }
}

void ve_insert_str(char * str)
{
    while (*str)
        insert_char(*(str++));
}

void delete_char()
{
    int i;

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
    struct textline *p = alloc_line();

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
    CLEAR_MARK();
}

void insert_to_fp(FILE * fp)
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

void insertch_from_fp(int ch)
{
    if (isprint2(ch) || ch == 27)
        insert_char(ch);
    else if (ch == Ctrl('I'))
        do {
            insert_char(' ');
        } while (currpnt & 0x7);
    else if (ch == '\n')
        split(currline, currpnt);
}
long insert_from_fp(FILE *fp, long * attach_length)
{
    int ch;
    char attachpad[10];
    int matched;
    char* ptr;
    long size;
	long ret=0;

	if( attach_length ) *attach_length=0;
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
                        int d;
						long attsize;
						char *sstart = data;
                        data++; not++;
						if(ret == 0)
							ret = data - ptr - ATTACHMENT_SIZE + 1;
                        while(*data){
							data++;
							not++;
						}
                        data++;
                        not++;
                        memcpy(&d, data, 4);
                        attsize = htonl(d);
                        data+=4+attsize-1;
                        not+=4+attsize-1;
                        matched = 0;
						if( attach_length) *attach_length += data - sstart + ATTACHMENT_SIZE;
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

	if(ret <= 0) return 0;
    return ret;
}

long read_file(char *filename,long *attach_length)
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
    ret=insert_from_fp(fp, attach_length);
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
            prints("±æ∆™Œƒ’¬µƒ“˝—‘”Î«©√˚µµ–– ˝‘∂≥¨π˝±æŒƒ≥§∂».\n");
            y += 3;
        }
        if (total!=lines) 
            lines--; /*»Áπ˚ «reŒƒ∫Õ«©√˚µµ£¨”¶∏√ºıµÙ“ª––*/
        temp = 0;
        if (len < 8 || lines==0 || (lines<=2 && (len/lines) < 16)) {
            move(y, 0);
            prints("±æ∆™Œƒ’¬∑«≥£ºÚ∂Ã, œµÕ≥»œŒ™π‡ÀÆŒƒ’¬.\n");
            /*Haohmaru.99.4.02.»√∞Æπ‡ÀÆµƒ»Àøﬁ»•∞…//grin */
            y += 3;
            temp = 1;
        }

        // local_article √ª”–≥ı ºªØ by zixia
        local_article = LOCAL_ARTICLE_DEFAULT; // »± °µƒ◊™–≈…Ë÷√

        if (temp){
            local_article = 1; /* π‡ÀÆµƒŒƒ’¬æÕ≤ª“™◊™≥ˆ»•¡À∞… :P by flyriver */
        }

        if (local_article == 1)
            strcpy(pmt, "(L)’æƒ⁄, (S)◊™–≈, (F)◊‘∂Øªª––∑¢±Ì, (A)»°œ˚, (T)∏¸∏ƒ±ÍÃ‚ or (E)‘Ÿ±‡º≠? [L]: ");
        else
            strcpy(pmt, "(S)◊™–≈, (L)’æƒ⁄, (F)◊‘∂Øªª––∑¢±Ì, (A)»°œ˚, (T)∏¸∏ƒ±ÍÃ‚ or (E)‘Ÿ±‡º≠? [S]: ");
    }

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

int write_file(char* filename,int saveheader,long* effsize,long* pattachpos, long attach_length)
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

    char p_buf[100];

    set_alarm(0, 0, NULL, NULL);
    clear();

    if (uinfo.mode != CCUGOPHER) {
        /*
           if ( uinfo.mode == EDITUFILE||uinfo.mode==CSIE_ANNOUNCE||
           uinfo.mode == EDITSFILE)
           strcpy(p_buf,"(S)¥¢¥Êµµ∞∏, (A)∑≈∆˙±‡º≠, (E)ºÃ–¯±‡º≠? [S]: " );
           else if ( uinfo.mode == SMAIL )
           strcpy(p_buf,"(S)ºƒ≥ˆ, (A)»°œ˚, or (E)‘Ÿ±‡º≠? [S]: " );
           else if ( local_article == 1 )
           strcpy(p_buf,"(L)≤ª◊™–≈, (S)◊™–≈, (A)»°œ˚, (T)∏¸∏ƒ±ÍÃ‚ or (E)‘Ÿ±‡º≠? [L]: ");
           else
           strcpy(p_buf,"(S)◊™–≈, (L)≤ª◊™–≈, (A)»°œ˚, (T)∏¸∏ƒ±ÍÃ‚ or (E)‘Ÿ±‡º≠? [S]: ");
         */
        if (uinfo.mode == POSTING) {
            /*strcpy(p_buf,"(S)∑¢±Ì, (F)◊‘∂Øªª––∑¢±Ì, (A)»°œ˚, (T)∏¸∏ƒ±ÍÃ‚ or (E)‘Ÿ±‡º≠? [S]: "); */
#ifndef NINE_BUILD        
            move(4, 0);         /* Haohmaru 99.07.17 */
            prints
                ("«Î◊¢“‚£∫±æ’æ’æπÊπÊ∂®£∫Õ¨—˘ƒ⁄»›µƒŒƒ’¬—œΩ˚‘⁄ 5 (∫¨)∏ˆ“‘…œÃ÷¬€«¯ƒ⁄÷ÿ∏¥’≈Ã˘°£\n\nŒ•∑¥’ﬂ≥˝À˘Ã˘Œƒ’¬ª·±ª…æ≥˝÷ÆÕ‚£¨ªπΩ´±ª∞˛∂·ºÃ–¯∑¢±ÌŒƒ’¬µƒ»®¡¶°£œÍœ∏πÊ∂®«Î≤Œ’’£∫\n\n    Announce ∞Êµƒ’æπÊ£∫°∞πÿ”⁄◊™Ã˘∫Õ’≈Ã˘Œƒ’¬µƒπÊ∂®°±°£\n\n«Î¥Ûº“π≤Õ¨Œ¨ª§ BBS µƒª∑æ≥£¨Ω⁄ °œµÕ≥◊ ‘¥°£–ª–ª∫œ◊˜°£\n\n");
#endif
        } else if (uinfo.mode == SMAIL)
            strcpy(p_buf, "(S)ºƒ≥ˆ, (F)◊‘∂Øªª––ºƒ≥ˆ, (A)»°œ˚, or (E)‘Ÿ±‡º≠? [S]: ");
        else if (uinfo.mode == IMAIL)
            strcpy(p_buf, NAME_BBS_NICK " Internet –≈º„£∫(S)ºƒ≥ˆ, (F)◊‘∂Øªª––ºƒ≥ˆ, (A)»°œ˚, or (E)‘Ÿ±‡º≠? [S]: ");      /* Leeward 98.01.17 Prompt whom you are writing to */
        /*    sprintf(p_buf,"∏¯ %s µƒ–≈£∫(S)ºƒ≥ˆ, (F)◊‘∂Øªª––ºƒ≥ˆ, (A)»°œ˚, or (E)‘Ÿ±‡º≠? [S]: ", lookupuser->userid ); 
           Leeward 98.01.17 Prompt whom you are writing to */
        else
            strcpy(p_buf, "(S)¥¢¥Êµµ∞∏, (F)◊‘∂Øªª––¥Ê¥¢, (A)∑≈∆˙±‡º≠, (E)ºÃ–¯±‡º≠? [S]: ");
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
        if (uinfo.mode != CCUGOPHER) {
            prints("»°œ˚...\n");
            refresh();
            sleep(1);
        }
        if (stat(filename, &stbuf) || stbuf.st_size == 0)
            unlink(filename);
        aborted = -1;
    } else if (abort[0] == 'e' || abort[0] == 'E') {
#ifdef FILTER
        if (filter) {
            clear();
            move (3, 0);
            prints ("\n\n            ∫‹±ß«∏£¨±æŒƒø…ƒ‹∫¨”–≤ª  “Àƒ⁄»›£¨«Î÷ÿ–¬±‡º≠...\n");
            pressreturn();
        }
#endif
        domsg();
        return KEEP_EDITING;
    } else if ( (abort[0] == 't' || abort[0] == 'T') && uinfo.mode == POSTING) {
        char buf[STRLEN];

        buf[0] = 0;             /* Leeward 98.08.14 */
        move(1, 0);
        prints("æ…±ÍÃ‚: %s", save_title);
		strcpy(buf,save_title);
        getdata(2, 0, "–¬±ÍÃ‚: ", buf, STRLEN, DOECHO, NULL, 0);
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
        if (pattachpos && *pattachpos) {
            char buf[MAXPATH];
            int fsrc,fdst;
            snprintf(buf,MAXPATH,"%s.attach",filename);
            if ((fsrc = open(filename, O_RDONLY)) >=0) {
                if ((fdst = open(buf, O_WRONLY|O_CREAT , 0600)) >= 0) {
                    char* src=(char*)malloc(10240);
                    long ret;
					long lsize = attach_length;
					long ndread;
                    lseek(fsrc,*pattachpos-1,SEEK_SET);
                    do {
						if( lsize > 10240 )
							ndread = 10240;
						else
							ndread = lsize;
                        ret = read(fsrc, src, ndread);
                        if (ret <= 0)
                            break;
						lsize -= ret;
                    } while (write(fdst, src, ret) > 0 && lsize > 0);
                    close(fdst);
                    free(src);
                }
                close(fsrc);
            }
        }
        if ((fp = fopen(filename, "w")) == NULL) {
            indigestion(5);
            abort_bbs(0);
        }
    /* ‘ˆº”◊™–≈±Íº« czz 020819 */
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
    temp=0;  /*±£¥Ê «∑ÒΩ¯»Î«©√˚µ≥Œª÷√*/
    while (p != NULL) {
        struct textline *v = p->next;

        if( (!aborted)&&(aborted!=-1)) {
//            long_flag = (strlen(p->data) >= WRAPMARGIN -2 );
            long_flag = 0;
            if (p->next != NULL || p->data[0] != '\0') {
                if (!strcmp(p->data,"--"))
                    temp=1;
                if (effsize) {
                    if (!strcmp(p->data,"--")) {
/*◊¢“‚¥¶¿Ì
--
fsdfa
--
µƒ«Èøˆ*/
                        *effsize+=sign_size;
                        sign_size=2;
                    } else {
                    /*»Áπ˚≤ª ««©√˚µµ∑÷∏Ù∑˚*/
                        if (sign_size!=0) /*‘⁄ø…ƒ‹µƒ«©√˚µµ÷–*/
                            sign_size+=strlen(p->data);
                        else
                              *effsize+=strlen(p->data);
                    }
                }
                if (!temp&&(abort[0] == 'f' || abort[0] == 'F')) {       /* Leeward 98.07.27 ÷ß≥÷◊‘∂Øªª–– */
                    unsigned char *ppt = (unsigned char *) p->data;     /* ’€––¥¶ */
                    unsigned char *pp = (unsigned char *) ppt;  /* –– ◊ */
                    unsigned const int LINE_LEN=78;
                    unsigned int LLL = LINE_LEN;      /* ’€––Œª÷√ */
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
                        } while (ppx); //º∆À„—’…´◊÷∑˚£¨◊÷Ω⁄ ˝∑≈»Îlll
                        ppt += LLL + lll; //”¶∏√’€––µƒµÿ∑Ω

                        if ((*ppt) > 127) {     /* ±‹√‚‘⁄∫∫◊÷÷–º‰’€–– ,KCN:ø¥≤ª∂Æfaint*/
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

                        cc = *ppt; //±£¥Ê“ªœ¬◊÷∑˚
                        *ppt = 0;
                        if (':' == p->data[0] && ':' != *pp)
                            fprintf(fp, ": ");
                        fprintf(fp, "%s", pp);
                        if (cc) //ªª––¿≠
                            fprintf(fp, "\n");
                        *ppt = cc;
                        pp = ppt;
                        LLL = LINE_LEN;
                    }
                    LLL = LINE_LEN ;
                    if (':' == p->data[0] && ':' != *pp) //¥¶¿Ì £”‡µƒ◊÷∑˚
                        fprintf(fp, ": ");
                    fprintf(fp, "%s", pp);
                    if (long_flag){	/* »Áπ˚µ±«∞¥Æ «œµÕ≥◊‘∂ØΩÿ∂œµƒ“ª∏ˆ≥¨≥§¥Æ */
                    	    LLL = LLL - strlen(pp); //Ω´œ¬“ª––µƒ‘ –Ì≥§∂»ºı∂Ã
			}else{
			    fprintf(fp,"\n");
			}
                } else
                    fprintf(fp, "%s\n", p->data);
                }
            }
        free(p->data);
        free(p);
        p = v;
    }
    if (!aborted) {
        fclose(fp);
        if (pattachpos && *pattachpos) {
            char buf[MAXPATH];
            int fsrc,fdst;
            struct stat st;
            snprintf(buf,MAXPATH,"%s.attach",filename);
            stat(filename,&st);
            *pattachpos=st.st_size+1;
            f_catfile(buf,filename);
            f_rm(buf);
				/*
            struct stat st;
            stat(filename,&st);
            *pattachpos=st.st_size+1;
            f_catfile(tmpattachfile,filename);
            f_rm(tmpattachfile);
			*/
        }
    }
    currline = NULL;
    lastline = NULL;
    firstline = NULL;
    if (abort[0] == 'l' || abort[0] == 'L' || local_article == 1) {
        sprintf(genbuf, "local_article = %u", local_article);
        bbslog("user","%s",genbuf);
        local_article = 0;
        if (aborted != -1)
            aborted = 1;        /* aborted = 1 means local save */
    }
    if ((uinfo.mode == POSTING) && strcmp(currboard->filename, "test")) { /*Haohmaru.99.4.02.»√∞Æπ‡ÀÆµƒ»Àøﬁ»•∞…//grin */
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
        free(p->data);
        free(p);
        p = v;
    }
    return;
}


/*Function Add by SmallPig*/
static int Origin(struct textline *text)
{
    char tmp[STRLEN];

    if (uinfo.mode != EDIT)
        return 0;
    if (!text)
        return 0;
    sprintf(tmp, "°˘ ¿¥‘¥:°§%s ", BBS_FULL_NAME);
    if (strstr(text->data, tmp) && *text->data != ':')
        return 1;
    else
        return 0;
}

int vedit_process_ESC(arg)
    int arg;                    /* ESC + x */
{
    int ch2, action;

#define WHICH_ACTION_COLOR "(M)«¯øÈ¥¶¿Ì (I/E)∂¡»°/–¥»ÎºÙÃ˘≤æ (C) π”√≤ …´ (F/B/R)«∞æ∞/±≥æ∞/ªπ‘≠…´≤ "
#define WHICH_ACTION_MONO  "(M)«¯øÈ¥¶¿Ì (I/E)∂¡»°/–¥»ÎºÙÃ˘≤æ (C) π”√µ•…´ (F/B/R)«∞æ∞/±≥æ∞/ªπ‘≠…´≤ "

#define CHOOSE_MARK     "(0)»°œ˚±Íº« (1)…Ë∂®ø™Õ∑ (2)…Ë∂®Ω·Œ≤ (3)∏¥÷∆±Íº«ƒ⁄»› "
#define FROM_WHICH_PAGE "∂¡»°ºÙÃ˘≤æµ⁄º∏“≥? (0-7) [‘§…ËŒ™ 0]"
#define SAVE_ALL_TO     "∞—’˚∆™Œƒ’¬–¥»ÎºÙÃ˘≤æµ⁄º∏“≥? (0-7) [‘§…ËŒ™ 0]"
#define SAVE_PART_TO    "∞—’˚∆™Œƒ’¬–¥»ÎºÙÃ˘≤æµ⁄º∏“≥? (0-7) [‘§…ËŒ™ 0]"
#define FROM_WHICH_SIG  "»°≥ˆ«©√˚≤æµ⁄º∏“≥? (0-7) [‘§…ËŒ™ 0]"
#define CHOOSE_FG       "«∞æ∞—’…´? 0)∫⁄ 1)∫Ï 2)¬Ã 3)ª∆ 4)…Ó¿∂ 5)∑€∫Ï 6)«≥¿∂ 7)∞◊ "
#define CHOOSE_BG       "±≥æ∞—’…´? 0)∫⁄ 1)∫Ï 2)¬Ã 3)ª∆ 4)…Ó¿∂ 5)∑€∫Ï 6)«≥¿∂ 7)∞◊ "
#define CHOOSE_ERROR    "—°œÓ¥ÌŒÛ"

    switch (arg) {
    case 'A':
    case 'a':
        show_eof = !show_eof;
        break;
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
            strcpy(msg, "±Íº«“—…Ë∂®ÕÍ≥…");
        else
            strcpy(msg, "“—…Ë∂®ø™Õ∑±Íº«, …–ŒﬁΩ·Œ≤±Íº«");
        break;
    case '2':                  /* mark end */
        mark_end = currline;
        mark_on = mark_block();
        if (mark_on)
            strcpy(msg, "±Íº«“—…Ë∂®ÕÍ≥…");
        else
            strcpy(msg, "“—…Ë∂®Ω·Œ≤±Íº«, …–Œﬁø™Õ∑±Íº«");
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
        strcpy(msg, "±Íº«∏¥÷∆ÕÍ≥…");
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
            sprintf(msg, "“—»°≥ˆºÙÃ˘≤æµ⁄ %c “≥", arg);
        } else
            sprintf(msg, "Œﬁ∑®»°≥ˆºÙÃ˘≤æµ⁄ %c “≥", arg);
        break;
    case 'G':
        go();
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
            sprintf(msg, "“—Ã˘÷¡ºÙÃ˘≤æµ⁄ %c “≥", arg);
        } else
            sprintf(msg, "Œﬁ∑®Ã˘÷¡ºÙÃ˘≤æµ⁄ %c “≥", arg);
        break;
    case 'N':
        searchline(searchtext);
        break;
    case 'S':
        search();
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
        clear();
        display_buffer();
        strcpy(msg, "“—œ‘ æ≤ …´±‡º≠≥…π˚£¨º¥Ω´«–ªÿµ•…´ƒ£ Ω");
    }

    if (msg[0] != '\0') {
        if (action == 'C') {    /* need redraw */
            move(t_lines - 2, 0);
            clrtoeol();
            prints("[1m%s%s%s[m", msg, ", «Î∞¥»Œ“‚º¸∑µªÿ±‡º≠ª≠√Ê...", ANSI_RESET);
            pressanykey();
            newch = '\0';
            editansi = showansi = 0;
        } else
            newch = ask(strcat(msg, "£¨«ÎºÃ–¯±‡º≠°£"));
    } else
        newch = '\0';
    return newch;
}

void input_tools()
{
    char *msg =
        "ƒ⁄¬Î ‰»Îπ§æﬂ: 1.º”ºı≥À≥˝  2.“ª∂˛»˝Àƒ  3.£¡£¬£√£ƒ  4.∫· ˙–±ª° £ø[Q]";
    char *ansi1[7][10] = {
        {"£´", "£≠", "°¡", "°¬", "°¿", "°ﬂ", "°‡", "° ", "°‘", "°ÿ"},
        {"°∆", "°«", "°»", "°…", "°“", "°”", "°√", "°ƒ", "°≈", "°À"},
        {"°’", "°÷", "°◊", "°Ÿ", "°⁄", "°€", "°‹", "°›", "°ﬁ", "°œ"},
        {"°≤", "°≥", "£®", "£©", "°¥", "°µ", "°∂", "°∑", "°∏", "°π"},
        {"°∫", "°ª", "°º", "°Ω", "°æ", "°ø", "£€", "£›", "£˚", "£˝"},
        {"¶‡", "¶·", "¶‚", "¶„", "¶‰", "¶Â", "¶Ê", "¶Á", "¶Ë", "¶È"},
        {"¶Í", "¶Î", "¶Ó", "¶Ô", "¶", "¶Ò", "°Æ", "°Ø", "°∞", "°±"}
    };

    char *ansi2[7][10] = {
        {"¢±", "¢≤", "¢≥", "¢¥", "¢µ", "¢∂", "¢∑", "¢∏", "¢π", "¢∫"},
        {"¢ª", "¢º", "¢Ω", "¢æ", "¢ø", "¢¿", "¢¡", "¢¬", "¢√", "¢ƒ"},
        {"¢≈", "¢∆", "¢«", "¢»", "¢…", "¢ ", "¢À", "¢Ã", "¢Õ", "¢Œ"},
        {"¢Ÿ", "¢⁄", "¢€", "¢‹", "¢›", "¢ﬁ", "¢ﬂ", "¢‡", "¢·", "¢‚"},
        {"¢Â", "¢Ê", "¢Á", "¢Ë", "¢È", "¢Í", "¢Î", "¢Ï", "¢Ì", "¢Ó"},
        {"¢°", "¢¢", "¢£", "¢§", "¢•", "¢¶", "¢ß", "¢®", "¢©", "¢™"},
        {"¢Ò", "¢Ú", "¢Û", "¢Ù", "¢ı", "¢ˆ", "¢˜", "¢¯", "¢˘", "¢˙"},
    };

    char *ansi3[7][10] = {
        {"¶°", "¶¢", "¶£", "¶§", "¶•", "¶¶", "¶ß", "¶®", "¶©", "¶™"},
        {"¶´", "¶¨", "¶≠", "¶Æ", "¶Ø", "¶∞", "¶±", "¶≤", "¶≥", "¶¥"},
        {"¶µ", "¶∂", "¶∑", "¶∏", "¶¡", "¶¬", "¶√", "¶ƒ", "¶≈", "¶∆"},
        {"¶«", "¶»", "¶…", "¶ ", "¶À", "¶Ã", "¶Õ", "¶Œ", "¶œ", "¶–"},
        {"¶—", "¶“", "¶”", "¶‘", "¶’", "¶÷", "¶◊", "¶◊", "¶ÿ", "©J"},
        {"©Y", "©K", "©L", "©M", "©N", "©O", "©P", "©â", "©ä", "©ã"},
        {"©å", "©ç", "©é", "©è", "©ê", "©ë", "©í", "©ì", "©î", "©ï"}
    };

    char *ansi4[7][10] = {
        {"®x", "®y", "®z", "®{", "®|", "®}", "®~", "®Ä", "®Å", "®Ç"},
        {"®É", "®Ñ", "®Ö", "®Ü", "®á", "®à", "®u", "®v", "®w", "°˘"},
        {"©§", "©¶", "©∞", "©¥", "©∏", "©º", "©¿", "©»", "©–", "©ÿ"},
        {"©‡", "®I", "®J", "®K", "®L", "°˙", "°˚", "°¸", "°˝", "°Ã"},
        {"®ã", "®å", "®ç", "®é", "®ê", "®è", "®q", "®r", "®s", "®t"},
        {"°·", "°‚", "®ë", "®í", "©ñ", "°Ú", "°˛", "®H", "°Ê", "©I"},
        {"°Ó", "°Ô", "°Û", "°Ù", "°ı", "°ˆ", "°˜", "°¯", "°", "°Ò"}
    };

    char buf[128], tmp[5];
    char *(*show)[10];
    int ch, i, page;

    move(t_lines - 1, 0);
    clrtoeol();
    prints("%s", msg);
    ch = igetkey();

    if (ch < '1' || ch > '4')
        return;

    switch (ch) {
    case '1':
        show = ansi1;
        break;
    case '2':
        show = ansi2;
        break;
    case '3':
        show = ansi3;
        break;
    case '4':
    default:
        show = ansi4;
        break;
    }

    page = 0;
    for (;;) {
        buf[0] = '\0';

        sprintf(buf, "µ⁄%d“≥:", page + 1);
        for (i = 0; i < 10; i++) {
            sprintf(tmp, "%d%s%s ", i, ".", show[page][i]);
            strcat(buf, tmp);
        }
        strcat(buf, "(P:…œ  N:œ¬)[Q]\0");
        move(t_lines - 1, 0);
        clrtoeol();
        prints("%s", buf);
        ch = igetkey();

        if (tolower(ch) == 'p'||ch == KEY_UP) {
            if (page)
                page -= 1;
        } else if (tolower(ch) == 'n'||ch == KEY_DOWN) {
            if (page != 6)
                page += 1;
        } else if (ch < '0' || ch > '9') {
            buf[0] = '\0';
            break;
        } else {
            char *ptr = show[page][ch - '0'];
            while (*ptr) {
                insert_char(*ptr);
                ptr++;
            }
            break;
        }
        buf[0] = '\0';
    }
}
void vedit_key(int ch)
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
            break;
            /* Leeward 98.07.30 Change hot key for msgX */
            /*case Ctrl('Z'):  call help screen */
        case Ctrl('Q'):        /* call help screen */
            show_help("help/edithelp");
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
            break;
        case Ctrl('O'):
            input_tools();
            break;
        case KEY_INS:          /* Toggle insert/overwrite */
            insert_character = !insert_character;
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
        } else
            top_of_win = top_of_win->prev;
    }
    if (curr_window_line >= t_lines - 1 || outii!=-1&&curr_window_line>=outii) {
        if (outii!=-1&&curr_window_line>=outii) i = curr_window_line - outii;
        else i = curr_window_line - t_lines + 1;
        for (; i >= 0; i--) {
            curr_window_line--;
            if (!top_of_win->next) {
                indigestion(7);
            } else
                top_of_win = top_of_win->next;
        }
    }
}

extern int icurrchar, ibufsize;

static int raw_vedit(char *filename,int saveheader,int headlines,long* eff_size,long* pattachpos)
{
    int newch, ch = 0, foo, shift;
    struct textline *st_tmp, *st_tmp2;
	long attach_length;

    if (pattachpos != NULL && *pattachpos!=0) {
        *pattachpos=read_file(filename,&attach_length);
    } else
        // TODO: add zmodem upload
        read_file(filename,NULL);
    top_of_win = firstline;
    top_of_line = 0;
    for (newch = 0; newch < headlines; newch++)
        if (top_of_win->next)
            top_of_win = top_of_win->next;
    /* Ã¯π˝headlines ÷∏∂®–– ˝µƒÕ∑≤ø–≈œ¢ Luzi 1999/1/8 */
    currline = top_of_win;
    st_tmp2 = firstline;
    st_tmp = currline->prev;    /* ±£¥Ê¡¥±Ì÷∏’Î£¨≤¢–ﬁ∏ƒ±‡º≠µ⁄“ª––µƒµƒ÷∏’Î */
    currline->prev = NULL;
    firstline = currline;

    curr_window_line = 0;
    currln = 0;
    currpnt = 0;
    display_buffer();
    while (ch != EOF) {
        newch = '\0';
        switch (ch) {
        case Ctrl('W'):
        case Ctrl('X'):        /* Save and exit */
            if (headlines) {
                st_tmp->next = firstline;       /* ÕÀ≥ˆ ±∫Úª÷∏¥‘≠¡¥±Ì */
                firstline->prev = st_tmp;
                firstline = st_tmp2;
            }
            foo = write_file(filename, saveheader, eff_size,pattachpos,attach_length);
            if (foo != KEEP_EDITING)
                return foo;
            if (headlines) {
                firstline = st_tmp->next;       /* ºÃ–¯±‡º≠‘Ú‘Ÿ¥Œ–ﬁ∏ƒµ⁄“ª––µƒ÷∏’Î */
                firstline->prev = NULL;
            }
            break;
        case KEY_ESC:
            if (KEY_ESC_arg == KEY_ESC)
                insert_char(KEY_ESC);
            else {
                newch = vedit_process_ESC(KEY_ESC_arg);
                clear();
            }
            break;
        case KEY_REFRESH:
            break;
        default:
            vedit_key(ch);
        }
        if (newch==0&&ibufsize == icurrchar) {
            display_buffer();
            move(myy, myx);
        }

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
    ans = raw_vedit(filename, saveheader, 4, eff_size,pattachpos);   /*Haohmaru.99.5.5.”¶∏√±£¡Ù“ª∏ˆø’–– */
    showansi = t;
    return ans;
}
