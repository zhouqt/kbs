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
extern int editansi;

extern int automargins ;

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
int tc_col, tc_line ;
struct screenline *big_picture = NULL ;
static const char nullstr[] = "(null)" ;

extern int ochar() ;

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
    if( !big_picture)
        t_columns=WRAPMARGIN;
    init_screen(t_lines,WRAPMARGIN) ;
    iscolor = YEA;
}

void
rel_move(was_col,was_ln,new_col,new_ln)
int     was_col,was_ln,new_col,new_ln;
{
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
    if (!scrint) {
	oflush();
	return;
    }

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
    for(i=0;i<scr_lns;i++)
        big_picture[i].mode &= ~(STANDOUT) ;
}

void
outc(c)
register unsigned char c ;
{
    register struct screenline *slp ;
    register unsigned char reg_col;

#ifndef BIT8
    c &= 0x7f ;
#endif
    if (!scrint) {
	if (c=='\n') ochar('\r');
	ochar(c);
	return;
    }
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
    {
        register int    reg_line  = cur_ln+roll;

        while( reg_line >= scr_lns )  reg_line -= scr_lns;
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
        if(!(slp->mode & MODIFIED)) {
            slp->smod = (slp->emod = reg_col) ;
	        slp->mode |= MODIFIED ;
        }
        else {
            if(reg_col > slp->emod)
                slp->emod = reg_col ;
            else if(reg_col < slp->smod)
                slp->smod = reg_col ;
        }
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
outns(str,n)
unsigned char *str ;
int n ;
{
    register int reg_col=0;
    register struct screenline *slp=NULL ;
    register len=0;
    unsigned char* begin_str=str;
    int begincol=0;

#define DO_MODIFY { if (slp->smod > begincol) slp->smod=begincol; \
                    if (slp->emod < reg_col) slp->emod=reg_col; \
                    if(standing && slp->mode&STANDOUT) { \
                        standing = NA ; \
                        slp->eso = Max(slp->eso,reg_col) ; \
		    } \
		    if(!(slp->mode & MODIFIED)) { \
		    	slp->mode |= MODIFIED ; \
		    } \
	          }

#define DO_CRLF   {	slp->len=reg_col; \
                        cur_col = 0 ; \
                        if(cur_ln < scr_lns) \
                            cur_ln++ ; \
			reg_col=begincol; \
                  }
	if (!scrint) {
		for (;*begin_str&&(reg_col<n);reg_col++,begin_str++) outc(*begin_str);
	return;
    };
    while ((str-begin_str<n)&&*str) {
        reg_col=cur_col;
        begincol=cur_col;
        {
            register int    reg_line  = cur_ln+roll;
            while( reg_line >= scr_lns )  reg_line -= scr_lns;
            slp = &big_picture[ reg_line ] ;
        }
            
        if(cur_col >= slp->len) {
            register int i ;
            for(i=slp->len;i<=cur_col;i++)
                slp->data[i] = ' ' ;
        }
        while((str-begin_str<n)&&*str){
			if (*str=='\n'||*str=='\r') {
		    	DO_MODIFY;
                DO_CRLF;
		    	str++;
                break;
			}
            if(*str=='') {
            	if (!iscolor){
	                while(*str&&(*str!='m'))
	                    str++;
	                if(*str) str++;
	                continue;
            	} else
	            	slp->data[reg_col++]=(unsigned char)'';
            } else if (!isprint2(*str)) 
                slp->data[reg_col++]=(unsigned char)'*';
            else 
            	slp->data[reg_col++]=*(str);
		str++;
            if(reg_col >= scr_cols) {
		DO_MODIFY;
                DO_CRLF;
                break;
            }
        } /* while (*str) */
    } /* while (1) */
    if (slp&&(begincol!=reg_col)) {
	if (slp->len<reg_col) slp->len=reg_col;
	DO_MODIFY;
	cur_col=reg_col;
    }
}

void
outs(str)
register char *str ;
{
    outns(str,4096);
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
    char* begin;

    va_start(ap) ;
    fmt = va_arg(ap, char *) ;
    begin = fmt;
    while(*fmt != '\0')
    {
        if(*fmt==''&&!iscolor){
                while(*fmt!='m')
                    fmt++;
                fmt++;continue;}
        if(*fmt == '%')
        {
            int sgn = 1 ;
            int val = 0 ;
            int len,negi ;

            if (fmt-begin) outns(begin,fmt-begin);
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
            begin=fmt;
            continue ;
        }

        fmt++ ;
    }
    if (*begin) outs(begin);
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
    scrollcnt++ ;
    roll++;
    if( roll >= scr_lns )  roll -= scr_lns;
    move(scr_lns-1,0) ;
    clrtoeol() ;
}

void
rscroll()
{
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

    if( !strtstandoutlen)
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

    if( !strtstandoutlen)
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


int
lock_scr() /* Leeward 98.02.22 */
{
    char passbuf[STRLEN];


    if (!strcmp(currentuser->userid, "guest"))
        return;

    modify_user_mode(LOCKSCREEN);
    clear();
    /*lock_monitor();*/
    while(1)
    {
        move(19,32);
        clrtobot();
        prints("[1m[32mBBS "NAME_BBS_CHINESE"’æ[m");
        move(21,0);
        clrtobot();
        getdata(21, 0, "∆¡ƒªœ÷‘⁄“—æ≠À¯∂®£¨“™Ω‚≥˝À¯∂®£¨«Î ‰»Î√‹¬Î£∫", passbuf, 39, NOECHO, NULL ,YEA);
        move(22,32);
        if( !checkpasswd2( passbuf,currentuser)) {
            prints( "[1m[31m√‹¬Î ‰»Î¥ÌŒÛ...[m\n" );
            pressanykey();
        }
        else
        {
            prints( "[1m[31m∆¡ƒªœ÷‘⁄“—æ≠Ω‚≥˝À¯∂®[m\n" );
            /*pressanykey();*/
            break;
        }
    }
}
