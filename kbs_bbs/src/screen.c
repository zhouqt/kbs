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
#include "screen.h"
#include "edit.h"
#include <sys/param.h>
#include <varargs.h>

extern char clearbuf[] ;
extern char cleolbuf[] ;
extern char strtstandout[] ;
extern char endstandout[] ;
extern int iscolor ;
extern int clearbuflen ;
extern int cleolbuflen ;
extern int strtstandoutlen ;
extern int endstandoutlen ;
#ifndef VEDITOR
#endif
extern int editansi;

extern int automargins ;
extern int dumb_term ;

#define o_clear()     output(clearbuf,clearbuflen)
#define o_cleol()     output(cleolbuf,cleolbuflen) 
#define o_standup()   output(strtstandout,strtstandoutlen)
#define o_standdown() output(endstandout,endstandoutlen)

unsigned char   scr_lns, scr_cols ;
unsigned char   cur_ln = 0, cur_col = 0 ;
int             roll, scrollcnt ;
unsigned char   docls ;
unsigned char   downfrom ;
int             standing = NA ;
int             inansi=NA;

struct screenline *big_picture = NULL ;

int
num_noans_chr(str)
char *str;
{
    int len,i,ansinum,ansi;

    ansinum=0;
    ansi=NA;
    len=strlen(str);
    for (i=0; i < len; i++)
    {
        if (str[i] == KEY_ESC)
        {
            ansi = YEA;
            ansinum++;
            continue;
        }
        if (ansi)
        {
            if (strchr("[0123456789; ", str[i]))
            {
                ansinum++;
                continue;
            }
            else if (isalpha(str[i]))
            {
                ansinum++;
                ansi = NA;
                continue;
            }
            else
                break;
        }
    }
    return len-ansinum;
}


void
init_screen(slns,scols)
int     slns, scols;
{
    register struct screenline *slp;

    scr_lns = slns ;
    scr_cols = Min(scols, LINELEN) ;
    if (!big_picture)
        big_picture = (struct screenline *) calloc(scr_lns,
                      sizeof(struct screenline)) ;
    for(slns=0;slns<scr_lns;slns++) {
        slp = &big_picture[ slns ];
        slp->mode = 0 ;
        slp->len = 0 ;
        slp->oldlen = 0 ;
    }
    docls = YEA ;
    downfrom = 0 ;
    roll = 0 ;
}

void
initscr()
{
    if(!dumb_term && !big_picture)
        t_columns=WRAPMARGIN;
    init_screen(t_lines,WRAPMARGIN) ;
}

int tc_col, tc_line ;
extern int ochar() ;

void
rel_move(was_col,was_ln,new_col,new_ln)
int     was_col,was_ln,new_col,new_ln;
{
    extern char *BC ;

    if(new_ln >= t_lines  || new_col >= t_columns)
        return ;
    tc_col = new_col ;
    tc_line = new_ln ;
    if((new_col == 0) && (new_ln == was_ln+1)) {
        ochar('\n') ;
        if(was_col != 0)
            ochar('\r') ;
        return ;
    }
    if((new_col == 0) && (new_ln == was_ln)) {
        if(was_col != 0)
            ochar('\r') ;
        return ;
    }
    if(was_col == new_col && was_ln == new_ln)
        return ;
    if(new_col == was_col - 1 && new_ln == was_ln) {
        ochar(Ctrl('H')) ;
        return ;
    }

    do_move(new_col,new_ln,ochar) ;
}

void
standoutput(buf,ds,de,sso,eso)
char *buf ;
int ds,de,sso,eso ;
{
    int st_start, st_end ;

    if(eso <= ds || sso >= de) {
        output(buf+ds,de-ds) ;
        return ;
    }
    st_start = Max(sso,ds) ;
    st_end = Min(eso,de) ;
    if(sso > ds)
        output(buf+ds,sso-ds) ;
    o_standup() ;
    output(buf+st_start,st_end-st_start) ;
    o_standdown() ;
    if(de > eso)
        output(buf+eso,de-eso) ;
}

void
redoscr()
{
    register int i,j ;
    int ochar() ;
    register struct screenline *bp = big_picture ;

    if(dumb_term)
        return ;
    o_clear() ;
    tc_col = 0 ;
    tc_line = 0 ;
    for(i=0;i<scr_lns;i++) {
        j = i + roll;
        while( j >= scr_lns )  j -= scr_lns;
        if(bp[j].len == 0)
            continue ;
        rel_move(tc_col,tc_line,0,i) ;
        if(bp[j].mode&STANDOUT)
            standoutput(bp[j].data,0,bp[j].len,bp[j].sso,bp[j].eso) ;
        else
            output(bp[j].data,bp[j].len) ;
        tc_col+=bp[j].len ;
        if(tc_col >= t_columns) {
            if(!automargins) {
                tc_col -= t_columns ;
                tc_line++ ;
                if(tc_line >= t_lines)
                    tc_line = t_lines - 1 ;
            }  else
                tc_col = t_columns-1 ;
        }
        bp[j].mode &= ~(MODIFIED) ;
        bp[j].oldlen = bp[j].len ;
    }
    rel_move(tc_col,tc_line,cur_col,cur_ln) ;
    docls = NA ;
    scrollcnt = 0 ;
    oflush() ;
}

void
refresh()
{
    register int i,j ;
    register struct screenline *bp = big_picture ;
    extern int automargins ;

    if(dumb_term)
        return ;
    if(num_in_buf() != 0)
        return ;
    if((docls) || (abs(scrollcnt) >= (scr_lns-3)) ) {
        redoscr() ;
        return ;
    }
    /*    if(scrollcnt < 0) {
            if(!scrollrevlen) {
                redoscr() ;
                return ;
            }
            rel_move(tc_col,tc_line,0,0) ;
            while(scrollcnt < 0) {
                o_scrollrev() ;
                scrollcnt++ ;
            }
        }
        if(scrollcnt > 0) {
            rel_move(tc_col,tc_line,0,t_lines-1) ;
            while(scrollcnt > 0) {
                ochar('\n') ;
                scrollcnt-- ;
            }
        }
    */
    if(scrollcnt < 0) {
        char buf[10];
        rel_move(tc_col,tc_line,0,0) ;
        sprintf(buf,"\033[%dL",-scrollcnt);
        output(buf,strlen(buf));
        scrollcnt=0;
    }
    if(scrollcnt > 0) {
        /*        rel_move(tc_col,tc_line,0,t_lines-1) ;
        */
        do_move(0,1024,ochar);
        while(scrollcnt > 0) {
            ochar('\n') ;
            scrollcnt-- ;
        }
        do_move(0,t_lines-2,ochar);
    }

    for(i=0;i<scr_lns;i++) {
        j = i + roll;
        while( j >= scr_lns )  j -= scr_lns;
        if(bp[j].mode&MODIFIED && bp[j].smod < bp[j].len) {
            bp[j].mode &= ~(MODIFIED) ;
            if(bp[j].emod >= bp[j].len)
                bp[j].emod = bp[j].len - 1 ;
            rel_move(tc_col,tc_line,bp[j].smod,i) ;
            if(bp[j].mode&STANDOUT)
                standoutput(bp[j].data,bp[j].smod,bp[j].emod+1,
                            bp[j].sso,bp[j].eso) ;
            else
                output(&bp[j].data[bp[j].smod],bp[j].emod-bp[j].smod+1) ;
            tc_col = bp[j].emod+1 ;
            if(tc_col >= t_columns) {
                if(automargins) {
                    tc_col -= t_columns ;
                    tc_line++ ;
                    if(tc_line >= t_lines)
                        tc_line = t_lines - 1 ;
                } else
                    tc_col = t_columns-1 ;
            }
        }
        if(bp[j].oldlen > bp[j].len) {
            rel_move(tc_col,tc_line,bp[j].len,i) ;
            o_cleol() ;
        }
        bp[j].oldlen = bp[j].len ;
    }
    rel_move(tc_col,tc_line,cur_col,cur_ln) ;
    oflush() ;
}

void
move(y,x)
int     y,x;
{
    cur_col = x/*+c_shift(y,x)*/ ;
    cur_ln = y ;
}

void
getyx(y,x)
int *y,*x ;
{
    *y = cur_ln ;
    *x = cur_col/*-c_shift(y,x)*/ ;
}

void
clear()
{
    register int i ;
    register struct screenline *slp;

    if(dumb_term)
        return ;
    roll = 0 ;
    docls = YEA ;
    downfrom = 0 ;
    for(i=0 ;i<scr_lns;i++) {
        slp = &big_picture[ i ];
        slp->mode = 0 ;
        slp->len = 0 ;
        slp->oldlen = 0 ;
    }
    move(0,0);
}

void
clear_whole_line(i)
int i;
{
    register struct screenline *slp = &big_picture[ i ];
    slp->mode = slp->len = 0;
    slp->oldlen = 79;
}

void
clrtoeol()
{
    register struct screenline *slp ;
    register int        ln;

    if(dumb_term)
        return ;
    standing = NA ;
    ln = cur_ln + roll;
    while( ln >= scr_lns )  ln -= scr_lns;
    slp = &big_picture[ ln ];
    if(cur_col <= slp->sso)
        slp->mode &= ~STANDOUT ;
    if(cur_col > slp->oldlen) {
        register int i ;
        for(i=slp->len;i<=cur_col;i++)
            slp->data[i] = ' ' ;
    }
    slp->len = cur_col ;
}

void
clrtobot()
{
    register struct screenline *slp ;
    register int        i, j;

    if(dumb_term)
        return ;
    for(i=cur_ln; i<scr_lns;i++) {
        j = i + roll;
        while( j >= scr_lns )  j -= scr_lns;
        slp = &big_picture[ j ];
        slp->mode = 0 ;
        slp->len = 0 ;
        if( slp->oldlen > 0 )  slp->oldlen = 255 ;
    }
}

void
clrstandout()
{
    register int i ;
    if(dumb_term)
        return ;
    for(i=0;i<scr_lns;i++)
        big_picture[i].mode &= ~(STANDOUT) ;
}

static char nullstr[] = "(null)" ;

void
outc(c)
register unsigned char c ;
{
    register struct screenline *slp ;
    register unsigned char reg_col;

#ifndef BIT8
    c &= 0x7f ;
#endif
    if(inansi==1)
    {
        if(c=='m')
        {
            inansi=0;
            return;
        }
        return;
    }
    if(c==KEY_ESC&&iscolor==NA)
    {
        inansi=1;
        return;
    }
    if(dumb_term) {
        if( !isprint2(c)) {
            if(c == '\n') {
                ochar('\r') ;
            } else if( c != KEY_ESC || !showansi ) {
                c = '*';
            }
        }
        ochar(c) ;
        return ;
    }
    if( 1 ) {
        register int    reg_line  = cur_ln+roll;
        register int    reg_scrln = scr_lns;

        while( reg_line >= reg_scrln )  reg_line -= reg_scrln;
        slp = &big_picture[ reg_line ] ;
    }
    reg_col = cur_col;
    /* deal with non-printables */
    if( !isprint2(c) ) {
        if(c == '\n' || c =='\r') {  /* do the newline thing */
            if(standing) {
                slp->eso = Max(slp->eso,reg_col) ;
                standing = NA ;
            }
            if(reg_col > slp->len) {
                register int i ;
                for(i=slp->len;i<=reg_col;i++)
                    slp->data[i] = ' ' ;
            }
            slp->len = reg_col ;
            cur_col = 0 ;       /* reset cur_col */
            if(cur_ln < scr_lns)
                cur_ln++ ;
            return ;
        } else if( c != KEY_ESC || !showansi ) {
            c = '*' ;  /* else substitute a '*' for non-printable */
        }
    }
    if(reg_col >= slp->len) {
        register int i ;
        for(i=slp->len;i<reg_col;i++)
            slp->data[i] = ' ';
        slp->data[reg_col] = '\0' ;
        slp->len = reg_col+1 ;
    }
    if(slp->data[reg_col] != c) {
        if((slp->mode & MODIFIED) != MODIFIED)
            slp->smod = (slp->emod = reg_col) ;
        else {
            if(reg_col > slp->emod)
                slp->emod = reg_col ;
            if(reg_col < slp->smod)
                slp->smod = reg_col ;
        }
        slp->mode |= MODIFIED ;
    }
    slp->data[ reg_col ] = c ;
    reg_col++;
    if(reg_col >= scr_cols) {
        if(standing && slp->mode&STANDOUT) {
            standing = NA ;
            slp->eso = Max(slp->eso,reg_col) ;
        }
        reg_col = 0 ;
        if(cur_ln < scr_lns)
            cur_ln++ ;
    }
    cur_col = reg_col;  /* store cur_col back */
}

void
RemoveAnsiX(char *szStr, int n) /* Leeward 98.01.12 */
{
    int nLen = (n > 0 ? n : strlen(szStr));
    char *ptrHead, *ptrTail, *ptr;
    int nIndex;

    for (nIndex = 0; nIndex < nLen;)
    {
        switch (szStr[nIndex])
        {
        case 27:
            nIndex ++;
            if (nIndex < nLen)
            {
                if (27 == szStr[nIndex])
                    szStr[nIndex - 1] = ' '; /* remove extra ESC */
            }
            break;

        case '(':
            if (nIndex - 1 >= 0)
            {
                if (27 == szStr[nIndex - 1])
                {
                    ptrHead = szStr + nIndex - 1;
                    nIndex ++;
                    if (nIndex < nLen)
                    {
                        while ((szStr[nIndex] >= '0' && szStr[nIndex] <= '9')
                                ||(szStr[nIndex] >= 'A' && szStr[nIndex] <= 'Z'))
                        {
                            nIndex ++;
                            if (nIndex >= nLen)
                                break;
                        }
                    }
                    ptrTail = szStr + nIndex - 1;
                    nIndex = ptrTail - szStr + 1;
                    ptrTail = (ptrTail > ptrHead) ? ptrTail : ptrHead;
                    memset(ptrHead, ' ', ptrTail - ptrHead + 1);
                    break;
                }
            }
            nIndex ++;
            break;

        case '[':
        case ']':
            nIndex ++;
            if (nIndex < nLen)
            {
                ptrHead = szStr + nIndex - 1;
                if (nIndex - 2 >= 0)
                {
                    if (27 == szStr[nIndex - 2])
                        ptrHead = szStr + nIndex - 2;
                }

                switch (szStr[nIndex])
                {
                case '[':
                case ']': /* AnsiX executes client-side applications */
                    if (27 != *ptrHead
                            && !('[' == szStr[nIndex - 1] && ']' == szStr[nIndex]))
                    {
                        nIndex ++;
                        break;
                    }

                    ptrTail = strchr(ptrHead + 1, 27);
                    if (ptrTail)
                    {
                        if (ptr = strchr(ptrTail, '*'))
                            ptrTail = ptr;
                    }
                    else
                        /*  ptrTail = szStr + nIndex;*/
                    {
                        nIndex ++;
                        break;
                    }

                    nIndex = ptrTail - szStr + 1;
                    ptrTail = (ptrTail > ptrHead) ? ptrTail : ptrHead;
                    /*---	modified by period	2000-11-22	---*/
                    memset(ptrHead, ' ', ptrTail - ptrHead /* + 1 */);
                    break;

                case '=': /* AnsiX modifys client information */
                    nIndex ++;
                    if (nIndex < nLen)
                    {
                        switch (szStr[nIndex])
                        {
                        case 'm':
                        case 'M':
                            ptrTail = strchr(ptrHead + 1, 27);
                            if (ptrTail)
                            {
                                if (ptr = strchr(ptrTail, 'S'))
                                    ptrTail = ptr;
                                else if (ptr = strchr(ptrTail, '='))
                                    ptrTail = ptr;
                            }
                            else
                                ptrTail = szStr + nIndex;
                            break;
                        case 'k':
                        case 'K':
                            /*break;*/
                        default:
                            ptrTail = szStr - nIndex;
                        }
                    }
                    else
                        ptrTail = szStr - nIndex - 1;

                    nIndex = ptrTail - szStr + 1;
                    ptrTail = (ptrTail > ptrHead) ? ptrTail : ptrHead;
                    memset(ptrHead, ' ', ptrTail - ptrHead + 1);
                    break;

                default:
                    while (szStr[nIndex] >= '0' && szStr[nIndex] <= '9')
                    {
                        nIndex ++;
                        if (nIndex >= nLen)
                            break;
                    }
                    if (nIndex < nLen)
                    {
                        if (!(szStr[nIndex - 1] >= '0' && szStr[nIndex - 1] <= '9'))
                        {
                            nIndex ++;
                            break;
                        }

                        ptrTail = 0;
                        switch (szStr[nIndex])
                        {
                        case 'i':  /* AnsiX has client-side disk/port access */
                            ptrTail = szStr + nIndex;
                            break;

                        case 'q':  /* AnsiX modifys client information */
                            ptrTail = szStr + nIndex;
                            break;

                        case ';':
                            nIndex ++;
                            while (szStr[nIndex] >= '0' && szStr[nIndex] <= '9')
                            {
                                nIndex ++;
                                if (nIndex >= nLen)
                                    break;
                            }
                            if (nIndex < nLen)
                            {
                                if ('h' == szStr[nIndex]) /* AnsiX changes screen size */
                                    ptrTail = szStr + nIndex;
                            }
                            break;

                        case 'm':
                            if (nIndex - 3 >= 0)
                            {
                                if ('1' == szStr[nIndex - 3] && '3' == szStr[nIndex - 2] && '2' == szStr[nIndex - 1]) /* AnsiX modifys display size */
                                    ptrTail = szStr + nIndex;
                            }
                            break;

                        default: ptrTail = 0;
                        }

                        if (ptrTail)
                        {
                            nIndex = ptrTail - szStr + 1;
                            ptrTail = (ptrTail > ptrHead) ? ptrTail : ptrHead;
                            memset(ptrHead, ' ', ptrTail - ptrHead + 1);
                        }
                        else nIndex ++;
                    } /* End if (nIndex < nLen) */
                } /* End switch (szStr[nIndex]) */
            } /* End if (nIndex < nLen) */
            break;

        default: nIndex ++;
        } /* End switch (szStr[nIndex]) */
    } /* End for (nIndex = 0; nIndex < nLen;) */
}

void
outs(str)
register char *str ;
{
    if (DEFINE(DEF_IGNOREANSIX))
        RemoveAnsiX(str, 0);

    while(*str != '\0'){
#ifndef VEDITOR
        if(*str==''&&!iscolor){
            while(*str!='m'){
                str++;}
            str++;}else
#endif
            outc(*str++) ;}
}

void
outns(str,n)
register char *str ;
register int n ;
{
    if (DEFINE(DEF_IGNOREANSIX))
        RemoveAnsiX(str, n);

    for(;n>0;n--){
#ifndef VEDITOR
        if(*str==''&&!iscolor){
            while(*str!='m'){
                str++;n--;}
            str++;n--;}else
#endif
            outc(*str++) ;}
}


int dec[] = {1000000000,100000000,10000000,1000000,100000,10000,1000,100,10,1};

void
prints(va_alist)
va_dcl
{
    va_list ap ;
    register char *fmt ;
    char *bp ;
    register int i, count, hd, indx ;

    va_start(ap) ;
    fmt = va_arg(ap, char *) ;
    while(*fmt != '\0')
    {
#ifndef VEDITOR
    if(*fmt==''&&!iscolor){
            while(*fmt!='m')
                fmt++;
            fmt++;continue;}
#endif
        if(*fmt == '%')
        {
            int sgn = 1 ;
            int val = 0 ;
            int len,negi ;

            fmt++ ;
            while(*fmt == '-') {
                sgn *= -1 ;
                fmt++ ;
            }
            while(isdigit(*fmt)) {
                val *= 10 ;
                val += *fmt - '0' ;
                fmt++ ;
            }
            switch(*fmt)
            {
            case 's':
                bp = va_arg(ap, char *) ;
                if(bp == NULL)
                    bp = nullstr ;
                if(val) {
                    register int slen = strlen(bp) ;
                    if(val <= slen)
                        outns(bp,val) ;
                    else if(sgn > 0) {
                        for(slen=val-slen;slen > 0; slen--)
                            outc(' ') ;
                        outs(bp) ;
                    } else {
                        outs(bp) ;
                        for(slen=val-slen;slen > 0; slen--)
                            outc(' ') ;
                    }
                } else outs(bp) ;
                break ;
            case 'd':
                i = va_arg(ap, int) ;

                negi = NA ;
                if(i < 0)
                {
                    negi = YEA ;
                    i *= -1 ;
                }
                for(indx=0;indx < 10;indx++)
                    if(i >= dec[indx])
                        break ;
                if(i == 0)
                    len = 1 ;
                else
                    len = 10 - indx ;
                if(negi)
                    len++ ;
                if(val >= len && sgn > 0) {
                    register int slen ;
                    for(slen = val-len;slen>0;slen--)
                        outc(' ') ;
                }
                if(negi)
                    outc('-') ;
                hd = 1, indx = 0;
                while(indx < 10)
                {
                    count = 0 ;
                    while(i >= dec[indx])
                    {
                        count++ ;
                        i -= dec[indx] ;
                    }
                    indx++ ;
                    if(indx == 10)
                        hd = 0 ;
                    if(hd && !count)
                        continue ;
                    hd = 0 ;
                    outc('0'+count) ;
                }
                if(val >= len && sgn < 0) {
                    register int slen ;
                    for(slen = val-len;slen>0;slen--)
                        outc(' ') ;
                }
                break ;
            case 'c':
                i = va_arg(ap, int) ;
                outc(i) ;
                break ;
            case '\0':
                goto endprint ;
            default:
                outc(*fmt) ;
                break ;
            }
            fmt++ ;
            continue ;
        }

        outc(*fmt) ;
        fmt++ ;
    }
    endprint:
    return ;
}

void
addch(ch)
int     ch;
{
    outc(ch) ;
}

void
scroll()
{
    if(dumb_term) {
        prints("\n") ;
        return ;
    }
    scrollcnt++ ;
    roll++;
    if( roll >= scr_lns )  roll -= scr_lns;
    move(scr_lns-1,0) ;
    clrtoeol() ;
}

void
rscroll()
{
    if(dumb_term) {
        prints("\n\n") ;
        return ;
    }
    scrollcnt-- ;
    if( roll > 0 )  roll--;
    else  roll = scr_lns - 1;
    move(0,0) ;
    clrtoeol() ;
}

void
standout()
{
    register struct screenline *slp ;
    register int        ln;

    if(dumb_term  || !strtstandoutlen)
        return ;
    if(!standing) {
        ln = cur_ln + roll;
        while( ln >= scr_lns )  ln -= scr_lns;
        slp = &big_picture[ ln ] ;
        standing = YEA ;
        slp->sso = cur_col ;
        slp->eso = cur_col ;
        slp->mode |= STANDOUT ;
    }
}

void
standend()
{
    register struct screenline *slp ;
    register int        ln;

    if(dumb_term || !strtstandoutlen)
        return ;
    if(standing) {
        ln = cur_ln + roll;
        while( ln >= scr_lns )  ln -= scr_lns;
        slp = &big_picture[ ln ] ;
        standing= NA ;
        slp->eso = Max(slp->eso,cur_col) ;
    }
}

saveline(line, mode, buffer)  /* 0 : save, 1 : restore */
int line, mode;
char *buffer;
{
    register struct screenline *bp = big_picture ;
    static /*struct screenline old_line;*/char tmpbuffer[256];
    char *tmp=tmpbuffer;
    int x,y;
    if (buffer) tmp=buffer;
    switch (mode) {
    case 0 :
        strncpy(tmp/*old_line.data*/, bp[line].data, LINELEN) ;
        tmp[bp[line].len]='\0';
        break;
    case 1 :
        getyx(&x,&y);
        move(line,0);
        clrtoeol();
        prints("%s",tmp);
        move(x,y);
        refresh();
    }
} ;


#ifdef CANCELLED_BY_LEEWARD /* Õâ¸ö´úÂë¿ªÏú¸Ð¾õÌ«´óÁË£¬·Ï³ýÖ® */
void
lock_monitor() /* Leeward 98.03.01 */
{
    static int xx = - 1, yy = - 1; /* Leeward:  for animated text:-) */
    static int dx = 2, dy = 1;
    int /*x,y,*/c;
    int cy,cx;
    time_t tm;

    getyx(&cy,&cx);
    move(yy, xx);
    clrtoeol(yy, xx);
    alarm(0);
    tm=time(0);
    if (- 1 == xx && - 1 == yy)  xx = tm % 60 + 4, yy = tm % 20 + 1;
    if (xx >= 64 || xx <= 1) dx = - dx; /* Careful: -1, 60, 4, 20, 1, 64, 0 */
    if (yy >= 20 || yy <= 0) dy = - dy; /* 21: top prompt line for password */
    xx += dx;
    yy += dy;
    c=tm%7+1;
    move(yy, xx);
    /*if(tm%2==0)
      prints("[3%dmÎÒÊÇ %12s[m",c,currentuser.userid);
    else
      prints("[3%dmÎÒÊÇ %12s[m",c,currentuser.username);*/
    prints("[1m[3%dmBBS Ë®Ä¾Çå»ªÕ¾[0m[0m", c);
    signal(SIGALRM,lock_monitor);
    alarm(1);
    move(cy,cx);
    refresh();
}
#endif

int
lock_scr() /* Leeward 98.02.22 */
{
    char passbuf[STRLEN];


    if (!strcmp(currentuser.userid, "guest"))
        return;

    modify_user_mode(LOCKSCREEN);
    clear();
    /*lock_monitor();*/
    while(1)
    {
        move(19,32);
        clrtobot();
        prints("[1m[32mBBSË®Ä¾Çå»ªÕ¾[m");
        move(21,0);
        clrtobot();
        getdata(21, 0, "ÆÁÄ»ÏÖÔÚÒÑ¾­Ëø¶¨£¬Òª½â³ýËø¶¨£¬ÇëÊäÈëÃÜÂë£º", passbuf, PASSLEN, NOECHO, NULL ,YEA);
        passbuf[8] = '\0';
        move(22,32);
        if( !checkpasswd( currentuser.passwd, passbuf )) {
            prints( "[1m[31mÃÜÂëÊäÈë´íÎó...[m\n" );
            pressanykey();
        }
        else
        {
            prints( "[1m[31mÆÁÄ»ÏÖÔÚÒÑ¾­½â³ýËø¶¨[m\n" );
            /*pressanykey();*/
            break;
        }
    }
}
