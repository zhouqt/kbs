/*
        o­Ó{¦¡O¥Ñ Firebird BBS ©ÒM¥Îªº Editor AwªïU¦ì¨Ï¥Î
        pG¦³¥ô¦ó°ÝD½Ð Mail µ¹ SmallPig.bbs@bbs.cs.ccu.edu.tw
        ©ÎO¨ì 140.123.101.78(bbs.cs.ccu.edu.tw) Post °ÝDC
*/

#include "bbs.h"
#include "edit.h"

struct textline *firstline = NULL ;
struct textline *lastline = NULL ;

void vedit_key();
struct textline *currline = NULL ;
int first_mark_line;
int currpnt = 0 ;
extern int local_article;
extern char BoardName[];
char searchtext[80];
int        editansi = 0;
int        scrollen=2;
int        marknum;
int        moveln=0;
int        shifttmp=0;
int        ismsgline;
int        tmpline;
struct textline *top_of_win = NULL ;
int curr_window_line ,currln;
int redraw_everything ;
int insert_character = 1;
/* for copy/paste */
#define CLEAR_MARK()  mark_on = 0; mark_begin = mark_end = NULL;
struct textline *mark_begin, *mark_end;
int    mark_on;
/* copy/paste */

void
msgline()
{
    char buf[256],buf2[STRLEN*2];
    void display_buffer();
    extern int talkrequest;
    int tmpshow;
    time_t now;
    if(ismsgline<=0)
        return;
    now=time(0);
    tmpshow=showansi;
    showansi=1;
    if(talkrequest)
    {
        talkreply();
        clear();
        showansi=0;
        display_buffer();
        showansi=1;
    }
    strcpy(buf,"[1;33;44m");
    if(chkmail())
        strcat(buf,"i[5;32m¡ó[m[1;33;44mj");
    else
        strcat(buf,"i  j");

    strcat(buf,"O_¦³sH¥ó [31mCtrl-Z[33m D±Ï ");
    sprintf(buf2," ª¬A [[32m%s[33m][[32m%4.4d[33m,[32m%3.3d[33m]   ®É¶¡",insert_character ? "Ins":"Rep", currln+1, currpnt+1);
    strcat(buf,buf2);
    sprintf(buf2,"[1;33;44mi[1;32m%.16s[33mj[m",ctime(&now));
    strcat(buf,buf2);
    move(t_lines-1,0);
    clrtoeol();
    prints("%s",buf);
    showansi=tmpshow;
}

void
msg()
{
    int x,y;
    int tmpansi;

    tmpansi=showansi;
    showansi=1;
    getyx(&x,&y);
    msgline();

    signal(SIGALRM, msg);
    move(x,y);
    refresh();
    alarm(60);
    showansi=tmpansi;
    return;
}

void
indigestion(i)
int     i;
{
    fprintf(stderr,"SERIOUS INTERNAL INDIGESTION CLASS %d\n",i) ;
}

struct textline *
            back_line(pos,num)
            struct textline *pos ;
int     num;
{
    moveln=0;
    while(num-- > 0)
        if(pos && pos->prev){
            pos = pos->prev ;
            moveln++;}

    return pos ;
}

struct textline *
            forward_line(pos,num)
            struct textline *pos ;
int     num;
{
    moveln=0;
    while(num-- > 0)
        if(pos && pos->next)
        {
            pos = pos->next ;
            moveln++;
        }
    return pos ;
}

void
countline()
{
    struct textline *pos;
    pos=firstline;
    moveln=0;
    while(pos!=lastline)
    {
        pos = pos->next ;
        moveln++;
    }
}

int
getlineno()
{
    int cnt = 0 ;
    struct textline *p = currline ;

    while(p != top_of_win) {
        if(p == NULL)
            break ;
        cnt++ ;
        p = p->prev ;
    }
    return cnt ;
}
char *
killsp(s)
char *s ;
{
    while(*s == ' ')
        s++ ;
    return s ;
}

struct textline *
            alloc_line()
{
    register struct textline *p ;

    p = (struct textline *) malloc(sizeof(*p)) ;
    if(p == NULL) {
        indigestion(13) ;
        abort_bbs() ;
    }
    p->next = NULL ;
    p->prev = NULL ;
    p->data[0] = '\0' ;
    p->len = 0 ;
    p->attr = 0 ; /* for copy/paste */
    return p ;
}

/*
  Appends p after line in list.  keeps up with last line as well.
 */

void
goline(n)
int n;
{
    register struct textline *p=firstline;
    int count;

    if(n<0)
        n=1;
    if(n==0)
        return;
    for(count=1;count<n;count++)
    {
        if(p)
        {
            p=p->next;
            continue;
        }
        else
            break;
    }
    if(p)
    {
        currln=n-1;
        curr_window_line=0;
        top_of_win=p;
        currline=p;
    }else
    {
        top_of_win=lastline;
        currln=count-2;
        curr_window_line=0;
        currline=lastline;
    }
    if(Origin(currline))
    {
        currline=currline->prev;
        top_of_win=currline;
        curr_window_line=0;
        currln--;
    }
    if(Origin(currline->prev)){
        currline=currline->prev->prev;
        top_of_win=currline;
        curr_window_line=0;
        currln-=2;
    }


}

void
go()
{
    char tmp[8];
    int line;
    signal(SIGALRM, SIG_IGN);
    getdata( 23,0,"½Ð°Ýn¸õ¨ì²ÄX¦æ: ",tmp, 7, DOECHO, NULL );
    msg();
    if(tmp[0]=='\0')
        return;
    line=atoi(tmp);
    goline(line);
    return;
}


void
searchline(text)
char text[STRLEN];
{
    int tmpline;
    int addr;
    int tt;

    register struct textline *p=currline;
    int count=0;
    tmpline=currln;
    for(;;p=p->next)
    {
        count++;
        if(p)
        {
            if(count==1)
                tt=currpnt;
            else
                tt=0;
            if(strstr(p->data+tt,text))
            {
                addr=(int)(strstr(p->data+tt,text) - p->data)+strlen(text);
                currpnt=addr;
                break;
            }
        }
        else
            break;
    }
    if(p)
    {
        currln=currln+count-1;
        curr_window_line=0;
        top_of_win=p;
        currline=p;
    }else
    {
        goline(currln+1);
    }
    if(Origin(currline))
    {
        currline=currline->prev;
        top_of_win=currline;
        curr_window_line=0;
        currln--;
    }
    if(Origin(currline->prev)){
        currline=currline->prev->prev;
        top_of_win=currline;
        curr_window_line=0;
        currln-=2;
    }

}

void
search()
{
    char tmp[STRLEN];

    signal(SIGALRM, SIG_IGN);
    getdata( 23,0,"jMr¦ê: ",tmp, 65, DOECHO, NULL );
    msg();
    if(tmp[0]=='\0')
        return;
    else
        strcpy(searchtext,tmp);

    searchline(searchtext);
    return;
}


void
append(p,line)
register struct textline *p, *line ;
{
    p->next = line->next ;
    if(line->next)
        line->next->prev = p ;
    else
        lastline = p ;
    line->next = p ;
    p->prev = line ;
}

/*
  delete_line deletes 'line' from the list and maintains the lastline, and 
  firstline pointers.
 */

void
delete_line(line)
register struct textline *line ;
{
    /* if single line */
    if(!line->next && !line->prev) {
        line->data[0] = '\0' ;
        line->len = 0 ;
        CLEAR_MARK();
        return ;
    }

#define ADJUST_MARK(p, q) if(p == q) p = (q->next) ? q->next : q->prev

    ADJUST_MARK( mark_begin, line );
    ADJUST_MARK( mark_end, line );

    if(line->next)
        line->next->prev = line->prev ;
    else
        lastline = line->prev ;  /* if on last line */

    if(line->prev)
        line->prev->next = line->next ;
    else
        firstline = line->next ; /* if on first line */

    free(line) ;
}

/*
  split splits 'line' right before the character pos
 */

void
split(line,pos)
register struct textline *line ;
register int pos ;
{
    register struct textline *p = alloc_line() ;

    if(pos > line->len) {
        free(p) ;
        return ;
    }
    p->len = line->len - pos ;
    line->len = pos ;
    strcpy(p->data,(line->data + pos)) ;
    p->attr = line->attr;              /* for copy/paste */
    *(line->data + pos) = '\0' ;
    append(p,line) ;
    if(line == currline && pos <= currpnt) {
        currline = p ;
        currpnt -= pos ;
        curr_window_line++ ;
        currln++;
    }
    redraw_everything = YEA ;
}

/*
  join connects 'line' and the next line.  It returns true if:
  
  1) lines were joined and one was deleted
  2) lines could not be joined
  3) next line is empty

  returns false if:

  1) Some of the joined line wrapped
 */

int
join(line)
register struct textline *line ;
{
    register int ovfl ;
    if(!line->next)
        return YEA ;
    /*if(*killsp(line->next->data) == '\0')
      return YEA ;*/
    ovfl = line->len + line->next->len - WRAPMARGIN ;
    if(ovfl < 0) {
        strcat(line->data, line->next->data) ;
        line->len += line->next->len ;
        delete_line(line->next) ;
        return YEA ;
    } else {
        register char *s ;
        register struct textline *p = line->next ;

        s = p->data + p->len - ovfl -1 ;
        while(s!=p->data && *s == ' ')
            s-- ;
        while(s!=p->data && *s != ' ')
            s-- ;
        if(s == p->data)
            return YEA ;
        split(p,(s - p->data) + 1) ;
        if(line->len + p->len >= WRAPMARGIN) {
            indigestion(0) ;
            return YEA ;
        }
        join(line) ;
        p = line->next ;
        if(p->len >= 1 && p->len+1 < WRAPMARGIN) {
            if(p->data[p->len-1] != ' ') {
                strcat(p->data," ") ;
                p->len++ ;
            }
        }
        return NA ;
    }
}

void
insert_char(ch)
register int ch ;
{
    register int i ;
    register char *s ;
    register struct textline *p = currline ;
    int wordwrap = YEA ;

    if(currpnt > p->len) {
        indigestion(1) ;
        return ;
    }
    if( currpnt < p->len && !insert_character ) {
        p->data[currpnt++] = ch ;
    } else {
        for(i = p->len; i >= currpnt; i--)
            p->data[i+1] = p->data[i] ;
        p->data[currpnt] = ch ;
        p->len++ ;
        currpnt++ ;
    }
    if(p->len < WRAPMARGIN)
        return ;
    s = p->data + (p->len - 1) ;
    while(s!=p->data && *s == ' ')
        s-- ;
    while(s!=p->data && *s != ' ')
        s-- ;
    if(s==p->data) {
        wordwrap = NA ;
        s = p->data + (p->len -2) ;
    }
    split(p,(s - p->data) + 1) ;
    p = p->next ;
    if(wordwrap && p->len >= 1) {
        i = p->len ;
        if(p->data[i-1] != ' ') {
            p->data[i] = ' ' ;
            p->data[i+1] = '\0' ;
            p->len++ ;
        }{
        }
    }
    while(!join(p)) {
        p = p->next ;
        if(p == NULL) {
            indigestion(2) ;
            break ;
        }
    }
    if(Origin(currline)){
        currline=p->prev;
        curr_window_line--;
        currln--;
    }
}

void
ve_insert_str(str)
char    *str;
{
    while(*str) insert_char(*(str++));
}

void
delete_char()
{
    register int i ;

    if(currline->len == 0)
        return ;
    if(currpnt >= currline->len) {
        indigestion(1) ;
        return ;
    }
    for(i=currpnt;i!=currline->len;i++)
        currline->data[i] = currline->data[i+1] ;
    currline->len-- ;
}

void
vedit_init()
{
    register struct textline *p = alloc_line() ;
    first_mark_line=0;
    firstline = p ;
    lastline = p ;
    currline = p ;
    currpnt = 0 ;
    marknum=0;
    process_ESC_action('M', '0');
    top_of_win = p ;
    curr_window_line = 0 ;
    currln=0;
    redraw_everything = NA ;
    CLEAR_MARK();
}

void
insert_to_fp(fp)
FILE *fp;
{
    int ansi = 0;
    struct textline *p;

    for ( p = firstline; p; p = p->next )
        if ( p->data[0] ) {
            fprintf(fp,"%s\n",p->data) ;
            if (strchr(p->data, '\033')) ansi++;
        }
    if (ansi) fprintf( fp, "%s\n", ANSI_RESET );
}

void
insert_from_fp(fp)
FILE *fp;
{
    int ch;
    while((ch = getc(fp)) != EOF)
        if(isprint2(ch) || ch == 27) {
            if ( currpnt < 254)  insert_char(ch) ;
            else if (currpnt < 255) insert_char('.');
        } else if(ch == Ctrl('I')) {
            do { insert_char(' ');
            } while (currpnt & 0x7);
        } else if(ch == '\n')
            split(currline,currpnt) ;
}

void
read_file(filename)
char *filename ;
{
    FILE *fp ;

    if(currline == NULL)
        vedit_init() ;
    if((fp = fopen(filename,"r+")) == NULL) {
        if((fp = fopen(filename,"w+")) != NULL) {
            fclose(fp) ;
            return ;
        }
        indigestion(4) ;
        abort_bbs() ;
    }
    insert_from_fp( fp );
    fclose(fp);
}

char save_title[STRLEN] ;
char save_filename[4096] ;
int in_mail ;

#define KEEP_EDITING -2

void
valid_article( pmt, abort )
char    *pmt, *abort;
{
    struct textline *p = firstline ;
    char        ch;
    int         total, lines, len, sig, y;
    if(uinfo.mode == POSTING)
    {
        total = lines = len = sig = 0;
        while( p != NULL ) {
            if( !sig ) {
                ch = p->data[0];
                if( strcmp( p->data, "--" ) == 0 )
                    sig = 1;
                else if( ch != ':' && ch != '>' && ch != '=' ) {
                    lines++;
                    len += strlen( p->data );
                }
            }
            total++;
            p = p->next;
        }
        y = 2;
        if( total > 20 + lines * 3 ) {
            move( y, 0 );
            prints( "¥»g¤å³¹ªº¤Þ¨¥PÃ±WÀÉ¦æ¼Æ»·WL¥»¤åªø«×.\n" );
            y += 3;
        }
        if( len < 8 || lines < 1 ) {
            move( y, 0 );
            prints( "¥»g¤å³¹D`Â²u, t²Î{¬°Äé¤ô¤å³¹.\n" );
            y += 3;
        }
    }

    getdata( 0, 0, pmt, abort, 3, DOECHO, NULL ,YEA);
    switch(abort[0]){
case 'A': case 'a':     /* abort */
case 'E': case 'e':     /* keep editing */
        return;}

}

int
write_file(filename)
char *filename ;
{
    struct textline *p = firstline ;
    FILE        *fp ;
    char        abort[6];
    int         aborted = 0;

    valid_article( "(S)xs, (A)¨ú®ø, or (E)As¿è? [S]: ", abort );
    if(abort[0]!='A' && abort[0]!='a' &&abort[0]!='E' &&abort[0]!='e')
        abort[0]='s';
    if (abort[0] == 'a' || abort[0] == 'A') {
        struct stat stbuf;
        clear();
        prints("¨ú®ø...\n");
        refresh();
        sleep(1);
        if (stat(filename, &stbuf) || stbuf.st_size == 0)
            unlink(filename);
        aborted = -1;
    } else if (abort[0] == 'e' || abort[0] == 'E') {
        msg();
        return KEEP_EDITING;
    } else if (abort[0] == 't' || abort[0] == 'T' ) {
        char    buf[ STRLEN ];

        move(1,0);
        prints("ÂÂ¼ÐD: %s", save_title );
        getdata(2,0,"s¼ÐD: ",buf,STRLEN,DOECHO,NULL);
        if ( strcmp( save_title, buf ) )  local_article = 0;
        strncpy(save_title,buf,STRLEN) ;
    } else if (abort[0] == 's' || abort[0] == 'S' ) {
        local_article = 0;
    }
    firstline = NULL ;
    if (!aborted) {
        if((fp = fopen(filename,"w")) == NULL) {
            indigestion(5) ;
            abort_bbs() ;
        }
    }
    while(p != NULL) {
        struct textline *v = p->next ;
        if (!aborted) if(p->next != NULL || p->data[0] != '\0')
                fprintf(fp,"%s\n",p->data) ;
        free(p) ;
        p = v ;
    }
    if (!aborted) fclose(fp) ;
    currline = NULL ;
    lastline = NULL ;
    firstline = NULL ;
    return aborted;
}

keep_fail_post()
{
    char filename[STRLEN];
    struct textline *p = firstline ;
    FILE        *fp ;

    sprintf(filename,"home/%s/%s.deadve",currentuser.userid,currentuser.userid);
    if((fp = fopen(filename,"w")) == NULL) {
        indigestion(5) ;
        return;
    }
    while(p != NULL) {
        struct textline *v = p->next ;
        if(p->next != NULL || p->data[0] != '\0')
            fprintf(fp,"%s\n",p->data) ;
        free(p) ;
        p = v ;
    }
    return ;
}


void    strnput(str)
char    *str;
{
    int count=0;

    while ((*str != '\0') && (++count < STRLEN))
    {
        if (*str == KEY_ESC)
        {
            outc('*');
            str++;
            continue;
        }
        outc(*str++);
    }
}

void
cstrnput(str)
char    *str;
{
    int count=0;

    prints("%s",ANSI_REVERSE);
    while ((*str != '\0') && (++count < STRLEN)){
        if (*str == KEY_ESC){
            outc('*');
            str++;
            continue;
        }
        outc(*str++);
    }
    while(++count < STRLEN)
        outc(' ');
    clrtoeol();
    prints("%s",ANSI_RESET);
}


/*Function Add by SmallPig*/
int
Origin(text)
struct textline *text;
{
    char tmp[STRLEN];

    if(uinfo.mode!=EDIT)
        return 0;
    if(!text)
        return 0;
    sprintf(tmp,"¡° ¨Ó·½:E%s %sE[FROM:",BoardName,email_domain());
    if(strstr(text->data,tmp)&&*text->data!=':')
        return 1;
    else
        return 0;
}

void
display_buffer()
{
    register struct textline *p ;
    register int i ;
    int      shift;
    int      temp_showansi;

    temp_showansi = showansi;

    for(p=top_of_win,i=0;i < t_lines-1;i++) {
        move(i,0) ;
        if(p)
        {
            shift = (currpnt+2 > STRLEN) ?
                    (currpnt / (STRLEN - scrollen)) * (STRLEN - scrollen) : 0;
            if (editansi) {
                showansi = 1;
                prints( "%s", p->data );
            } else if ((p->attr & M_MARK)) {
                showansi = 1; clear_whole_line( i );
                cstrnput(p->data+shift);
            } else {
                if(p->len>= shift){
                    showansi = 0 ; strnput(p->data+shift); }
                else
                    clrtoeol();
            }
            p = p->next ;
        } else prints( "%s~", editansi ? ANSI_RESET : "");
        clrtoeol();
    }

    showansi = temp_showansi;
    msgline();
    return ;
}

int
vedit_process_ESC(arg)
int     arg;   /* ESC + x */
{
    int         ch2, action;

#define WHICH_ACTION_COLOR    \
"(M)°Ï¶ôBz (I/E)Åª¨ú/gJ°ÅKÃ¯ (C)¨Ï¥Îm¦â (F/B/R)e´º/I´º/ÁÙ­ì¦âm"
#define WHICH_ACTION_MONO    \
"(M)°Ï¶ôBz (I/E)Åª¨ú/gJ°ÅKÃ¯ (C)¨Ï¥Î³æ¦â (F/B/R)e´º/I´º/ÁÙ­ì¦âm"

#define CHOOSE_MARK     "(0)¨ú®ø¼ÐO (1)]w}Y (2)]wµ²§À (3)½Æs¼ÐO¤ºe "
#define FROM_WHICH_PAGE "Åª¨ú°ÅKÃ¯²ÄX­¶? (0-7) [w]¬° 0]"
#define SAVE_ALL_TO     "§â¾ãg¤å³¹gJ°ÅKÃ¯²ÄX­¶? (0-7) [w]¬° 0]"
#define SAVE_PART_TO    "§â¾ãg¤å³¹gJ°ÅKÃ¯²ÄX­¶? (0-7) [w]¬° 0]"
#define FROM_WHICH_SIG  "¨úXÃ±WÃ¯²ÄX­¶? (0-7) [w]¬° 0]"
#define CHOOSE_FG       "e´ºC¦â? 0)¶Â 1)¬õ 2)ºñ 3)¶À 4)`ÂÅ 5)¯»¬õ 6)LÂÅ 7)¥Õ "
#define CHOOSE_BG       "I´ºC¦â? 0)¶Â 1)¬õ 2)ºñ 3)¶À 4)`ÂÅ 5)¯»¬õ 6)LÂÅ 7)¥Õ "
#define CHOOSE_ERROR    "¿ï¶µ¿ù~"

    switch( arg ) {
case 'M': case 'm':
        ch2 = ask(CHOOSE_MARK);     action = 'M'; break;
case 'I': case 'i': /* import */
        ch2 = ask(FROM_WHICH_PAGE); action = 'I'; break;
case 'E': case 'e': /* export */
        ch2 = ask( mark_on ? SAVE_PART_TO : SAVE_ALL_TO );
        action = 'E'; break;
case 'S': case 's': /* signature */
        ch2 = '0';  action = 'S'; break;
case 'F': case 'f':
        ch2 = ask(CHOOSE_FG);       action = 'F'; break;
case 'B': case 'b':
        ch2 = ask(CHOOSE_BG);       action = 'B'; break;
case 'R': case 'r':
        ch2 = '0'; /* not used */   action = 'R'; break;
case 'D': case 'd':
        ch2='4';                    action = 'M'; break;
case 'N': case 'n':
        ch2='0';                    action = 'N'; break;
case 'G': case 'g':
        ch2='1';                    action = 'G'; break;
case 'L': case 'l':
        ch2='0';/* not used */      action = 'L'; break;
case 'C': case 'c':
        ch2 = '0'; /* not used */   action = 'C'; break;
    default:
        return;
    }

    if ( strchr( "IES", action) &&
            (ch2 == '\n' || ch2 == '\r')) ch2 = '0';

    if (ch2 >= '0' && ch2 <= '7') return process_ESC_action(action, ch2);
    else {
        return ask( CHOOSE_ERROR );
    }
}

int
mark_block()
{
    struct      textline *p;
    int         pass_mark = 0;

    first_mark_line=0;
    if ( mark_begin == NULL && mark_end == NULL ) return 0;
    if ( mark_begin == mark_end )
    {
        mark_begin->attr |= M_MARK;
        return 1;
    }
    if ( mark_begin == NULL || mark_end == NULL )
    {
        if(mark_begin!=NULL)
            mark_begin->attr |= M_MARK;
        else
            mark_end->attr |= M_MARK;
        return 1;
    }
    else {
        for(p = firstline; p != NULL; p = p->next) {
            if ( p == mark_begin || p == mark_end )
            {
                pass_mark++;
                p->attr |= M_MARK;
                continue;
            }
            if( pass_mark ==1 )  p->attr |= M_MARK;
        else {first_mark_line++; p->attr &= ~(M_MARK); }
            if(pass_mark==2)
                first_mark_line--;
        }
        return 1;
    }
}

void
process_MARK_action(arg, msg)
int    arg;    /* operation of MARK */
char   *msg;   /* message to return */
{
    struct textline *p;
    int dele_1line;

    switch( arg ) {
    case '0': /* cancel */
        for(p = firstline; p != NULL; p = p->next) p->attr &= ~(M_MARK);
        CLEAR_MARK();
        break;
    case '1': /* mark begin */
        mark_begin = currline;
        mark_on = mark_block();
        if (mark_on) strcpy( msg, "¼ÐOw]w§¹¦¨");
        else strcpy( msg,"w]w}Y¼ÐO, |Lµ²§À¼ÐO");
        break;
    case '2': /* mark end */
        mark_end = currline; mark_on = mark_block();
        if (mark_on) strcpy( msg, "¼ÐOw]w§¹¦¨");
        else strcpy( msg,"w]wµ²§À¼ÐO, |L}Y¼ÐO");
        break;
    case '3': /* copy mark */
        if (mark_on&&!(currline->attr & M_MARK))
        {
            for(p = firstline; p != NULL; p = p->next)
            {
                if (p->attr & M_MARK) {
                    ve_insert_str( p->data );
                    split(currline, currpnt) ;
                }
            }
        }else
            bell();
        strcpy( msg,"¼ÐO½Æs§¹¦¨");
        break;
    case '4': /* delete mark */
        dele_1line=0;
        if (mark_on&&(currline->attr & M_MARK))
            if(currline==firstline)
                dele_1line=1;
            else
                dele_1line=2;
        for(p = firstline; p != NULL; p = p->next)
        {
            if (p->attr & M_MARK)
            {
                currline=p;
                vedit_key( Ctrl('Y') );
            }
        }
        process_ESC_action('M', '0');
        marknum=0;
        if(dele_1line==0||dele_1line==2){
            if(first_mark_line==0)
                first_mark_line=1;
            goline(first_mark_line);
        }else
            goline(1);
        break;
    default:
        strcpy(msg, CHOOSE_ERROR );
    }
    strcpy(msg,"\0");
}

int
process_ESC_action(action, arg)
int     action, arg;
/* valid action are I/E/S/B/F/R/C */
/* valid arg are    '0' - '7' */
{
    int         newch = 0;
    char        msg[ 80 ], buf[ 80 ];
    char        filename[ 80 ];
    FILE        *fp;

    msg[ 0 ] = '\0';
    switch( action ) {
    case 'L':
        if(ismsgline>=1)
        {
            ismsgline=0;
            move(t_lines-1,0);
            clrtoeol();
            refresh();
        }
        else
            ismsgline=1;
        break;
    case 'M':
        process_MARK_action(arg, msg); break;
    case 'I':
        sprintf( filename, "/tmp/%s_clip_%c", currentuser.userid, arg);
        if( (fp = fopen( filename, "r" )) != NULL ) {
            insert_from_fp( fp ); fclose( fp );
            sprintf( msg, "w¨úX°ÅKÃ¯²Ä %c ­¶", arg );
        } else sprintf( msg, "Lk¨úX°ÅKÃ¯²Ä %c ­¶", arg );
        break;
    case 'G':
        go();
        redraw_everything = YEA;
        break;
    case 'E':
        sprintf( filename, "/tmp/%s_clip_%c", currentuser.userid, arg);
        if( (fp = fopen( filename, "w" )) != NULL ) {
            if ( mark_on ) {
                struct textline *p;
                for(p = firstline; p != NULL; p = p->next)
                    if (p->attr & M_MARK) fprintf(fp, "%s\n", p->data);
            } else insert_to_fp( fp );
            fclose( fp );
            sprintf( msg, "wK¦Ü°ÅKÃ¯²Ä %c ­¶", arg );
        } else sprintf( msg, "LkK¦Ü°ÅKÃ¯²Ä %c ­¶", arg );
        break;
    case 'N':
        searchline(searchtext);
        redraw_everything = YEA;
        break;
    case 'S':
        search();
        redraw_everything = YEA;
        break;
        /*
        #if 0
                    sprintf( filename, "home/%s/signatures_%c", 
                        currentuser.userid, arg);
                    if (fp = fopen( filename, "r" )) {
                        insert_from_fp( fp ); fclose( fp );
                        sprintf( msg, "w¨úX²Ä %c iÃ±W", arg );
                    } else sprintf( msg, "Lk¨úXÃ±WÃ¯²Ä %c ­¶", arg );
        #else
                    sprintf( msg, "¦¹\\¯à|¥¼}©ñ" );
        #endif
                    break;*/
    case 'F':
        sprintf( buf, "%c[3%cm", 27, arg ); ve_insert_str( buf ); break;
    case 'B':
        sprintf( buf, "%c[4%cm", 27, arg ); ve_insert_str( buf ); break;
    case 'R':
        ve_insert_str( ANSI_RESET );  break;
    case 'C':
        editansi = showansi = 1; redraw_everything = YEA;
        clear();  display_buffer(); redoscr();
        strcpy( msg, "wÅã¥Üm¦âs¿è¦¨GAYN¤Á^³æ¦â¼Ò¦¡" );
    }

    if (strchr("FBRCM", action)) redraw_everything = YEA;

    if (msg[ 0 ] != '\0') {
        if ( action == 'C' ) {   /* need redraw */
            move( t_lines-2, 0 ) ;
            clrtoeol() ;
            prints( "[1m%s%s%s[m", msg, ", ½Ð«ö¥ôNÁäªð^s¿èe­±...", ANSI_RESET );
            igetkey() ;
            newch = '\0';
            editansi = showansi = 0;
            clear(); display_buffer();
        } else newch = ask(strcat( msg, "A½Ð~Äòs¿èC"));
    } else newch = '\0';
    return newch;
}
#ifdef VEDITOR
void
show_helpmenu( helptext )
char *helptext[];
{
    char *str;
    int i;

    showansi=1;
    clear() ;
    for( i = 0; (str = helptext[i]) != NULL; i++ ) {
        if( *str == '\01' || *str == '\02' ) {
            prints("[1;44m");
            /*          if( *str == '\02' )
                            printdash( str + 1 );
                        else   
            */              prints( "%s\n", str + 1 );
            prints("[m");
        } else {
            prints( "%s\n", str );
        }
    }
    move( t_lines-1, 0 ) ;
    prints("Press any key to continue...");
    igetkey();
    clear() ;
}

char *vedithelp[] = {
    "\01s¿è\\¯à«üO»¡©ú",
    "",
    "\01@¯ëRO:",
    " ^W,^X    sÀÉ«áµ²§ôs¿è         ^L,^G    ­«Ã¸¿Ã¹õ",
    " ^Q       µ²§ôs¿è, ¤£sÀÉ       ^Z       Åã¥Ü»²Ue­±",
    "",
    "\01´å¼Ð²¾°Ê«üO:",
    " ^P,¡ô   VW    U ^N,¡õ   VU    | ^R,¡ö    V¥ª   | ^V,¡÷   Vk  ",
    " ^A,Home ¦Ü¦æ­º  U ^E, End ¦Ü¦æ¥½  | ^B,PgUp  W@­¶ | ^F,PgDn U@­¶",
    " ^S      ÀÉ®×}YU ^T      ÀÉ®×µ²§À| Esc-s    jM   | Esc-l    ~ÄòjM",
    " Esc-g   ¸õ¨ì##¦æx",
    "",
    "\01R°£RO:",
    " ^O,Ins ¤Á´«´¡J/m´«¼Ò¦¡ U ^H,BS   R°£¥ª¤èr¤¸  U ^Y    R°£@¦æ  ",
    " ^D,DEL R°£¥Øer¤¸      U ^K      R¦Ü¦æ¥½      U                 ",
    "",
    "\01°Ï¶ôBz:",
    " ^U   °Ï¶ô¼Ðw     ^C   ½Æs°Ï¶ô      Esc-d   M°£°Ï¶ô    ^Q   M°£¼Ðw",
    "",
    "\01S®íRO: (½Ð¥ý«ö ESC, A¿ï¾ÜHU¶µ¥Ø)",
    " (I)¶×JÀÉ®×     (E)¶×XÀÉ®×      (C)[¬Ým¦âs¿è¦¨G",
    " (B)]wI´º¦âm (F)]we´º¦âm  (R)ÁÙ­ì¦âm      ",
    " (ESC)m¦â±±¨îX (M)°Ï¶ôBz      (L)}©ÎÃöT®§Äæ",
    NULL } ;

#endif
void
vedit_key( ch )
int     ch;
{
    int i;
#define NO_ANSI_MODIFY  if(no_touch) { warn++; break; }

    static int lastindent = -1 ;
    int no_touch, warn,shift;

    if( ch == Ctrl('P') || ch == KEY_UP ||
            ch == Ctrl('N') || ch == KEY_DOWN ) {
        if( lastindent == -1 )
            lastindent = currpnt;
    } else
        lastindent = -1;

    no_touch = (editansi && strchr(currline->data, '\033')) ? 1 : 0;
    warn = 0;


    if(ch < 0x100 && isprint2(ch)) {
        if (no_touch) warn++; else insert_char(ch) ;
    } else switch(ch) {
        case Ctrl('I'):
                        NO_ANSI_MODIFY;
    do {  insert_char(' ');
            } while (currpnt & 0x7);
            break;
    case '\r': case '\n':
            NO_ANSI_MODIFY;
            split(currline,currpnt) ;
            break ;
        case Ctrl('G'): /* redraw screen */
                        clear() ;
            redraw_everything = YEA ;
            break ;
        case Ctrl('Z'): /* call help screen */
#ifndef VEDITOR
                        show_help( "help/edithelp" );
#else
show_helpmenu( vedithelp );
#endif
            redraw_everything = YEA ;
            break ;
    case Ctrl('R'): case KEY_LEFT: /* backward character */
    if(currpnt > 0 ){
                currpnt--;
            }
            else if( currline->prev) {
                curr_window_line-- ;
                currln--;
                currline = currline->prev ;
                currpnt = currline->len ;
            }
            break;
        case Ctrl('Q'):
                        process_ESC_action('M', '0');
            marknum=0;
            break;
        case Ctrl('C'):
                        process_ESC_action('M', '3');
            break;
        case Ctrl('U'):
                if(marknum==0){
                    marknum=1;
                    process_ESC_action('M', '1');
                }else
                    process_ESC_action('M', '2');
            clear();
            break;
    case Ctrl('V'): case KEY_RIGHT: /* forward character */
            if(currline->len != currpnt){
                currpnt++;
            }
            else if( currline->next ) {
                currpnt = 0 ;
                curr_window_line++ ;
                currln++;
                currline = currline->next ;
                if(Origin(currline))
                {
                    curr_window_line-- ;
                    currln--;
                    currline = currline->prev ;
                }
            }
            break ;
    case Ctrl('P'): case KEY_UP: /* Previous line */
            if( currline->prev ) {
                currln--;
                curr_window_line-- ;
                currline = currline->prev ;
                currpnt = (currline->len>lastindent)?lastindent:currline->len;
            }
            break;
    case Ctrl('N'): case KEY_DOWN: /* Next line */
            if( currline->next ) {
                currline = currline->next ;
                curr_window_line++ ;
                currln++;
                if(Origin(currline))
                {
                    currln--;
                    curr_window_line-- ;
                    currline = currline->prev;
                }
                currpnt = (currline->len>lastindent)?lastindent:currline->len;
            }
            break ;
    case Ctrl('B'): case KEY_PGUP: /* previous page */
            top_of_win = back_line( top_of_win,22 ) ;
            currline = back_line( currline, 22 ) ;
            currln-=moveln;
            curr_window_line = getlineno() ;
            if( currpnt > currline->len )  currpnt = currline->len;
            redraw_everything = YEA ;
            break ;
    case Ctrl('F'): case KEY_PGDN: /* next page */
            top_of_win = forward_line( top_of_win, 22 ) ;
            currline = forward_line( currline, 22 ) ;
            currln+=moveln;
            curr_window_line = getlineno() ;
            if( currpnt > currline->len )  currpnt = currline->len;
            if(Origin(currline->prev))
            {
                currln-=2;
                curr_window_line=0;
                currline=currline->prev->prev;
                top_of_win=lastline->prev->prev;
            }
            if(Origin(currline))
            {
                currln--;
                curr_window_line--;
                currline=currline->prev;
            }
            redraw_everything = YEA ;
            break ;
    case Ctrl('A'): case KEY_HOME: /* begin of line */
            currpnt = 0 ;
            break ;
    case Ctrl('E'): case KEY_END: /* end of line */
            currpnt = currline->len ;
            break ;
        case Ctrl('S'): /* start of file */
                        top_of_win = firstline ;
            currline = top_of_win ;
            currpnt = 0 ;
            curr_window_line = 0 ;
            currln=0;
            redraw_everything = YEA ;
            break ;
        case Ctrl('T'): /* tail of file */
                        top_of_win = back_line(lastline,22) ;
            countline();
            currln=moveln;
            currline = lastline ;
            curr_window_line = getlineno() ;
            currpnt = 0 ;
            if(Origin(currline->prev))
    {
                currline=currline->prev->prev;
                currln-=2;
                curr_window_line-=2;
            }
            redraw_everything = YEA ;
            break ;
    case Ctrl('O'): case KEY_INS: /* Toggle insert/overwrite */
            insert_character = !insert_character;
            /*move(0,73);
            prints( " [%s] ", insert_character ? "Ins" : "Rep" );*/
            break;
    case Ctrl('H'): case '\177':  /* backspace */
            NO_ANSI_MODIFY;
            if(currpnt == 0) {
                struct textline *p ;

                if(!currline->prev) {
                    break ;
                }
                currln--;
                curr_window_line-- ;
                currline = currline->prev ;
                currpnt = currline->len ;
                if(*killsp(currline->next->data) == '\0') {
                    delete_line(currline->next) ;
                    redraw_everything = YEA ;
                    break;
                }
                p = currline ;
                while(!join(p)) {
                    p = p->next ;
                    if(p == NULL) {
                        indigestion(2) ;
                        abort_bbs() ;
                    }
                }
                redraw_everything = YEA ;
                break ;
            }
            currpnt-- ;
            delete_char() ;
            break ;
    case Ctrl('D'): case KEY_DEL: /* delete current character */
            NO_ANSI_MODIFY;
            if(currline->len == currpnt) {
                struct textline *p = currline ;
                if(!Origin(currline->next))
                {
                    while(!join(p))
                    {
                        p = p->next ;
                        if(p == NULL)
                        {
                            indigestion(2) ;
                            abort_bbs() ;
                        }
                    }
                }else if(currpnt==0)
                    vedit_key( Ctrl('K') );
                redraw_everything = YEA ;
                break;
            }
            delete_char() ;
            break ;
        case Ctrl('Y'): /* delete current line */
                        /* STONGLY coupled with Ctrl-K */
                        no_touch = 0;  /* ANSI_MODIFY hack */
            currpnt = 0;
    if(currline->next){
                if(Origin(currline->next)&&!currline->prev){
                    currline->data[0]='\0';
                    currline->len=0;
                    break;
                }
            }else if(currline->prev!=NULL)
            {   currline->len=0;
            }else
            {
                currline->len = 0;
                currline->data[0]='\0';
                break;
            }
            currline->len = 0;
            vedit_key( Ctrl('K') );
            break;
        case Ctrl('K'): /* delete to end of line */
                        NO_ANSI_MODIFY;
            if(currline->prev==NULL&&currline->next==NULL)
    {
                currline->data[0]='\0';
                currpnt=0;
                break;
            }
            if(currline->next)
            {
                if(Origin(currline->next)&&currpnt==currline->len&&currpnt!=0)
                    break;
                if(Origin(currline->next)&&currline->prev==NULL)
                {
                    vedit_key(Ctrl('Y'));
                    break;
                }
            }
            if(currline->len == 0) {
                struct textline *p = currline->next ;
                if(!p) {
                    p = currline->prev ;
                    if(!p) {
                        break ;
                    }
                    if( curr_window_line > 0 )
                        curr_window_line-- ;
                    currln--;
                }
                if(currline == top_of_win)
                    top_of_win = p ;
                delete_line(currline) ;
                currline = p ;
                if(Origin(currline))
                {
                    currline=currline->prev;
                    curr_window_line--;
                    currln--;
                }
                redraw_everything = YEA ;
                break ;
            }
            if(currline->len == currpnt) {
                struct textline *p = currline ;

                while(!join(p)) {
                    p = p->next ;
                    if(p == NULL) {
                        indigestion(2) ;
                        abort_bbs() ;
                    }
                }
                redraw_everything = YEA ;
                break ;
            }
            currline->len = currpnt ;
            currline->data[currpnt] = '\0' ;
            break ;
        default:
            break ;
        }

    if(curr_window_line < 0) {
        curr_window_line = 0 ;
        if(!top_of_win->prev) {
            indigestion(6) ;
        } else {
            top_of_win = top_of_win->prev ;
            /*            redraw_everything = YEA ;
                        move(t_lines-2,0);
                        clrtoeol();
                        refresh();*/
            rscroll();
        }
    }
    if(curr_window_line >= t_lines-1) {
        for(i=curr_window_line-t_lines+1;i>=0;i--)
        {
            curr_window_line --;
            if(!top_of_win->next) {
                indigestion(7) ;
            } else {
                top_of_win = top_of_win->next ;
                /*          redraw_everything = YEA ;
                          move(t_lines-1,0);
                          clrtoeol();
                          refresh();*/
                scroll() ;
            }
        }
    }

    if ( editansi /*|| mark_on*/ ) redraw_everything = YEA;
    shift = (currpnt+2 > STRLEN) ?
            (currpnt/(STRLEN-scrollen))*(STRLEN-scrollen) : 0;
    msgline();
    if(shifttmp!=shift||redraw_everything ==YEA)
    {
        redraw_everything = YEA ;
        shifttmp=shift;
    }
    else
        redraw_everything = NA ;

    move(curr_window_line,0) ;
    if(currline->attr & M_MARK)
    {
        showansi=1;
        cstrnput(currline->data+shift);
        showansi=0;
    }
    else
        strnput(currline->data+shift) ;
    clrtoeol() ;
}

int
raw_vedit(filename)
char *filename ;
{
    int newch, ch=0, foo ,shift;

    read_file(filename) ;
    top_of_win = firstline ;
    currline = firstline ;
    curr_window_line = 0 ;
    currln=0;
    currpnt = 0 ;
    clear() ;
    display_buffer() ;
    msgline();
    while( ch != EOF ) {
        newch = '\0';
        switch( ch ) {
    case Ctrl('W'): case Ctrl('X'): /* Save and exit */
                            foo = write_file(filename);
            if( foo != KEEP_EDITING ) return foo;
            redraw_everything = YEA ;
            break;
        case KEY_ESC:
            if ( KEY_ESC_arg == KEY_ESC ) insert_char(KEY_ESC);
    else {
                newch = vedit_process_ESC(KEY_ESC_arg);
                clear();
            }
            redraw_everything = YEA;
            break;
        default:
            vedit_key( ch );
        }
        if(redraw_everything){
            display_buffer() ;
        }
        redraw_everything = NA ;
        shift = (currpnt+2> STRLEN) ?
                (currpnt/(STRLEN-scrollen))*(STRLEN-scrollen) : 0;
        move(curr_window_line,currpnt-shift) ;

        ch = (newch != '\0') ? newch : igetkey();
    }
    return 1;
}

int
veedit(filename)
char *filename ;
{
    int ans,t;

    t = showansi;
    showansi=0;
    init_alarm();
    ismsgline=(DEFINE(DEF_EDITMSG))?1:0;
    msg();
    ans = raw_vedit(filename);
    showansi = t;
    signal(SIGALRM, SIG_IGN);
    return ans;
}
