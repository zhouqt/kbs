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
/*#include <varargs.h>*/
#include <stdarg.h>

extern int iscolor;
extern int editansi;

extern int automargins;
extern int dumb_term;

#define o_clear() output("\x1b[H\x1b[J",6)
#define o_cleol() output("\x1b[K",3)
#define o_standup() output("\x1b[7m",4)
#define o_standdown() output("\x1b[m",3)

unsigned char scr_lns, scr_cols;
unsigned char cur_ln = 0, cur_col = 0;
int roll, scrollcnt;
unsigned char docls;
unsigned char downfrom;
int standing = false;
int inansi = false;
int tc_col, tc_line;
struct screenline *big_picture = NULL;
static const char nullstr[] = "(null)";
static /*struct screenline old_line; */ char tmpbuffer[256];

/* –«ø’’Ω∂∑º¶ added by Czz 020926 */
void
clrnlines(int n)
{
	register struct screenline *slp;
	register int i, k;
/*	if (dumb_term)*/
/*		return;*/
	for (i = cur_ln; i < cur_ln + n; i++) {
		slp = &big_picture[(i + roll) % scr_lns];
		slp->mode = 0;
		slp->oldlen = 255;
		slp->len = 0;
		for (k = 0; k < LINELEN; k++)
			slp->data[k] = 0;
	}
}
/* added end */

int
num_noans_chr(str)
char *str;
{
	int len, i, ansinum, ansi;

	ansinum = 0;
	ansi = false;
	len = strlen(str);
	for (i = 0; i < len; i++) {
		if (str[i] == KEY_ESC) {
			ansi = true;
			ansinum++;
			continue;
		}
		if (ansi) {
			if (strchr("[0123456789; ", str[i])) {
				ansinum++;
				continue;
			} else if (isalpha(str[i])) {
				ansinum++;
				ansi = false;
				continue;
			} else
				break;
		}
	}
	return len - ansinum;
}

void
init_screen(slns, scols)
int slns, scols;
{
	register struct screenline *slp;

	scr_lns = slns;
	scr_cols = Min(scols, LINELEN);
	if (!big_picture)
		big_picture =
		    (struct screenline *) calloc(scr_lns,
						 sizeof (struct screenline));
	for (slns = 0; slns < scr_lns; slns++) {
		slp = &big_picture[slns];
		slp->mode = 0;
		slp->len = 0;
		slp->oldlen = 0;
	}
	docls = true;
	downfrom = 0;
	roll = 0;
}

void
initscr()
{
	if (!big_picture)
		t_columns = WRAPMARGIN;
	init_screen(t_lines, WRAPMARGIN);
}

void
rel_move(was_col, was_ln, new_col, new_ln)
int was_col, was_ln, new_col, new_ln;
{
	if (new_ln >= t_lines || new_col >= t_columns)
		return;
	tc_col = new_col;
	tc_line = new_ln;
	if ((new_col == 0) && (new_ln == was_ln + 1)) {
		ochar('\n');
		if (was_col != 0)
			ochar('\r');
		return;
	}
	if ((new_col == 0) && (new_ln == was_ln)) {
		if (was_col != 0)
			ochar('\r');
		return;
	}
	if (was_col == new_col && was_ln == new_ln)
		return;
	if (new_col == was_col - 1 && new_ln == was_ln) {
		ochar(Ctrl('H'));
		return;
	}

	do_move(new_col, new_ln, ochar);
}

static void
standoutput(buf, ds, de, sso, eso)
char *buf;
int ds, de, sso, eso;
{
	int st_start, st_end;

	if (eso <= ds || sso >= de) {
		output(buf + ds, de - ds);
		return;
	}
	st_start = Max(sso, ds);
	st_end = Min(eso, de);
	if (sso > ds)
		output(buf + ds, sso - ds);
	o_standup();
	output(buf + st_start, st_end - st_start);
	o_standdown();
	if (de > eso)
		output(buf + eso, de - eso);
}

void
redoscr()
{
	register int i, j;
	register struct screenline *bp = big_picture;

	if (!scrint) {
		oflush();
		return;
	}
	o_clear();
	tc_col = 0;
	tc_line = 0;
	for (i = 0; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)
			j -= scr_lns;
		if (bp[j].len == 0)
			continue;
		rel_move(tc_col, tc_line, 0, i);
		if (bp[j].mode & STANDOUT)
			standoutput(bp[j].data, 0, bp[j].len, bp[j].sso,
				    bp[j].eso);
		else
			output(bp[j].data, bp[j].len);
		tc_col += bp[j].len;
		if (tc_col >= t_columns) {
			if (!automargins) {
				tc_col -= t_columns;
				tc_line++;
				if (tc_line >= t_lines)
					tc_line = t_lines - 1;
			} else
				tc_col = t_columns - 1;
		}
		bp[j].mode &= ~(MODIFIED);
		bp[j].oldlen = bp[j].len;
	}
	rel_move(tc_col, tc_line, cur_col, cur_ln);
	docls = false;
	scrollcnt = 0;
	oflush();
}

void
refresh()
{
	int i, j;
	register struct screenline *bp = big_picture;
	extern int automargins;

	if (!scrint) {
		oflush();
		return;
	}

	if (num_in_buf() != 0)
		return;
	if ((docls) || (abs(scrollcnt) >= (scr_lns - 3))) {
		redoscr();
		return;
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
	if (scrollcnt < 0) {
		char buf[10];

		rel_move(tc_col, tc_line, 0, 0);
		sprintf(buf, "\033[%dL", -scrollcnt);
		output(buf, strlen(buf));
		scrollcnt = 0;
	}
	if (scrollcnt > 0) {
		/*        rel_move(tc_col,tc_line,0,t_lines-1) ;
		 */
		do_move(0, 1024, ochar);
		while (scrollcnt > 0) {
			ochar('\n');
			scrollcnt--;
		}
		do_move(0, t_lines - 2, ochar);
	}

	for (i = 0; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)
			j -= scr_lns;
		if (bp[j].mode & MODIFIED && bp[j].smod < bp[j].len) {
			bp[j].mode &= ~(MODIFIED);
			if (bp[j].emod >= bp[j].len)
				bp[j].emod = bp[j].len - 1;
			rel_move(tc_col, tc_line, bp[j].smod, i);
			if (bp[j].mode & STANDOUT)
				standoutput(bp[j].data, bp[j].smod,
					    bp[j].emod + 1, bp[j].sso,
					    bp[j].eso);
			else
				output(&bp[j].data[bp[j].smod],
				       bp[j].emod - bp[j].smod + 1);
			tc_col = bp[j].emod + 1;
			if (tc_col >= t_columns) {
				if (automargins) {
					tc_col -= t_columns;
					tc_line++;
					if (tc_line >= t_lines)
						tc_line = t_lines - 1;
				} else
					tc_col = t_columns - 1;
			}
		}
		if (bp[j].oldlen > bp[j].len) {
			rel_move(tc_col, tc_line, bp[j].len, i);
			o_cleol();
		}
		bp[j].oldlen = bp[j].len;
	}
	good_getyx(&i, &j);
	rel_move(tc_col, tc_line, j, i);
	oflush();
}

void
move(y, x)
int y, x;
{
	if (!scrint) {
	    do_move(x, y, ochar);
	    return;
	
	}
	cur_col = x /*+c_shift(y,x) */ ;
	cur_ln = y;
}

void
good_move(int y, int x)
{
	register struct screenline *slp;
	register int ln;
	int i, j = 0;
	int inansi = 0;

	if (!scrint) {
	    do_move(x, y, ochar);
	    return;
	
	}
	cur_ln = y;
	standing = false;
	ln = cur_ln + roll;
	while (ln >= scr_lns)
		ln -= scr_lns;
	slp = &big_picture[ln];
	for (i = 0; i < slp->len; i++) {
		if (inansi) {
			if ((slp->data[i] == KEY_ESC)
			    || (isalpha(slp->data[i])))
				inansi = 0;
		} else if (j >= x) {
			cur_col = i /*+c_shift(y,x) */ ;
			return;
		} else if (slp->data[i] == KEY_ESC)
			inansi = 1;
		else
			j++;
	}
	if (j <= x)
		cur_col = slp->len + (x - j);
}

void
getyx(int *y, int *x)
{
	*y = cur_ln;
	*x = cur_col /*-c_shift(y,x)*/ ;
}

void
good_getyx(int *y, int *x)
{
	register struct screenline *slp;
	register int ln;
	int i, j = 0;
	int inansi = 0;

	*y = cur_ln;
	ln = cur_ln + roll;
	while (ln >= scr_lns)
		ln -= scr_lns;
	slp = &big_picture[ln];

	for (i = 0; i < cur_col; i++) {
		if (inansi) {
			if ((slp->data[i] == KEY_ESC)
			    || (isalpha(slp->data[i])))
				inansi = 0;
		} else if (slp->data[i] == KEY_ESC)
			inansi = 1;
		else
			j++;
	}

	*x = j + cur_col - i;
}

void
clear()
{
	register int i;
	register struct screenline *slp;

        if (!scrint) {
            o_clear();
            return;
        }
	roll = 0;
	docls = true;
	downfrom = 0;
	for (i = 0; i < scr_lns; i++) {
		slp = &big_picture[i];
		slp->mode = 0;
		slp->len = 0;
		slp->oldlen = 0;
	}
	move(0, 0);
}

void
clear_whole_line(int i)
{
	register struct screenline *slp = &big_picture[i];

	slp->mode = slp->len = 0;
	slp->oldlen = 79;
}

void
clrtoeol()
{
	register struct screenline *slp;
	register int ln;

        if (!scrint) {
            o_cleol();
            return;
        }
	standing = false;
	ln = cur_ln + roll;
	while (ln >= scr_lns)
		ln -= scr_lns;
	slp = &big_picture[ln];
	if (cur_col <= slp->sso)
		slp->mode &= ~STANDOUT;
	if (cur_col > slp->oldlen) {
		register int i;

		for (i = slp->len; i <= cur_col; i++)
			slp->data[i] = ' ';
	}
	slp->len = cur_col;
}

void
clrtobot()
{
	register struct screenline *slp;
	register int i, j;

	for (i = cur_ln; i < scr_lns; i++) {
		j = i + roll;
		while (j >= scr_lns)
			j -= scr_lns;
		slp = &big_picture[j];
		slp->mode = 0;
		slp->len = 0;
		if (slp->oldlen > 0)
			slp->oldlen = 255;
	}
}

void
clrstandout()
{
	register int i;

	for (i = 0; i < scr_lns; i++)
		big_picture[i].mode &= ~(STANDOUT);
}

void
outc(unsigned char c)
{
	register struct screenline *slp;
	register unsigned char reg_col;

#ifndef BIT8
	c &= 0x7f;
#endif
	if (!scrint) {
		if (c == '\n')
			ochar('\r');
		ochar(c);
		return;
	}
	if (inansi == true) {
		if (isalpha(c) ) {
			inansi = false;
			return;
		} else if ((c=='\n') || (c=='\r'))
		    inansi=false;
		else
		    return;
	}
	{
		register int reg_line = cur_ln + roll;

		while (reg_line >= scr_lns)
			reg_line -= scr_lns;
		slp = &big_picture[reg_line];
	}
	reg_col = cur_col;
	/* deal with non-printables */
	if (!isprint2(c)) {
		if (c == '\n' || c == '\r') {	/* do the newline thing */
			if (standing) {
				slp->eso = Max(slp->eso, reg_col);
				standing = false;
			}
			if (reg_col > slp->len) {
				register int i;

				for (i = slp->len; i <= reg_col; i++)
					slp->data[i] = ' ';
				if (slp->smod > slp->len)
					slp->smod = slp->len;
				slp->mode |= MODIFIED;
			}
			slp->len = reg_col;
			cur_col = 0;	/* reset cur_col */
			if (cur_ln < scr_lns)
				cur_ln++;
			return;
		} else if (c != KEY_ESC || !showansi) {
			c = '*';	/* else substitute a '*' for non-printable */
		} if ((c==KEY_ESC)&&(!iscolor)) {
		    inansi=true;
		    return;
		}
	}
	if (reg_col >= slp->len) {
		register int i;

		for (i = slp->len; i < reg_col; i++)
			slp->data[i] = ' ';
		if (slp->smod > slp->len)
			slp->smod = slp->len;
		slp->mode |= MODIFIED;
		slp->data[reg_col] = '\0';
		slp->len = reg_col + 1;
	}
	if (slp->data[reg_col] != c) {
		if (!(slp->mode & MODIFIED)) {
			slp->smod = (slp->emod = reg_col);
			slp->mode |= MODIFIED;
		} else {
			if (reg_col > slp->emod)
				slp->emod = reg_col;
			else if (reg_col < slp->smod)
				slp->smod = reg_col;
		}
	}
	slp->data[reg_col] = c;
	reg_col++;
	if (reg_col >= scr_cols) {
		if (standing && slp->mode & STANDOUT) {
			standing = false;
			slp->eso = Max(slp->eso, reg_col);
		}
		reg_col = 0;
		if (cur_ln < scr_lns)
			cur_ln++;
	}
	cur_col = reg_col;	/* store cur_col back */
}

int savey=-1, savex=-1;

void
outns(str, n)
const char *str;
int n;
{
	register int reg_col = 0;
	register struct screenline *slp = NULL;
	const char *begin_str = str;
	int begincol = 0;

	inansi=false;

#define DO_MODIFY { if (slp->smod > begincol) slp->smod=begincol; \
                    if (slp->emod < reg_col) slp->emod=reg_col; \
                    if(standing && slp->mode&STANDOUT) { \
                        standing = false ; \
                        slp->eso = Max(slp->eso,reg_col) ; \
		    } \
		    if(!(slp->mode & MODIFIED)) { \
		    	slp->mode |= MODIFIED ; \
		    } \
	          }

#define DO_CRLF   {	slp->len=reg_col; \
                        if(cur_col < slp->sso) \
                             slp->mode&=~STANDOUT; \
                        cur_col = 0 ; \
                        cur_ln = (cur_ln+1)%scr_lns;\
			reg_col=begincol; \
                  }
	if (!scrint) {
		for (; *begin_str && (reg_col < n); reg_col++, begin_str++)
			outc(*begin_str);
		return;
	};
	while ((str - begin_str < n) && *str) {
		reg_col = cur_col;
		begincol = cur_col;
		slp = &big_picture[(cur_ln + roll)%scr_lns];

		if (cur_col >= slp->len) {
                     memset(slp->data+slp->len, ' ', cur_col-slp->len+1);
			slp->smod = Min(slp->smod, slp->len);
			slp->mode |= MODIFIED;
		}
		while ((str - begin_str < n) && *str) {
		    if (inansi) {
		        //filter ansi
		        if (isalpha(*str))
		            inansi=false;
		        if ((*str=='\n')||(*str=='\r')) {
		            inansi=false;
		            continue;
		        }
		        str++;
		        continue;
		    }
                      if (*str == ''&&*(str+1)=='[') {
                             register int i=1;
                             while(!isalpha(*(str+i))&&(*(str+i)!='')&&*(str+i)) i++;
                             if(*(str+i)=='H') {
                                register int j=0;
                                while(j<i&&*(str+j)!=';') j++;
                                if(*(str+j)==';'&&j<=4&&j>=3&&i-j>=2&&i-j<=3) {
                                    char s1[5],s2[5],x,y;
                                    memcpy(s1,str+2,j-2);
                                    s1[j-2]=0;
                                    memcpy(s2,str+j+1,i-j-1);
                                    s2[i-j-1]=0;
                                    y=atoi(s1)-1;
                                    x=atoi(s2)-1;
                                    if(y>=0&&y<scr_lns&&x>=0&&x<scr_cols) {
                                    	if (slp && (begincol != reg_col)) {
                                    		if (slp->len < reg_col)
                                    			slp->len = reg_col;
                                    		DO_MODIFY;
                                    	}
                                        good_move(y,x);
                                    }
                                    str+=i+1;
                                    break;
                                }
                             }
                      }
                      if(*str == ''&&*(str+1)=='['&&*(str+2)=='s') {
                        str+=3;
                        good_getyx(&savey, &savex);
                        continue;
                      }
                      else if(*str == ''&&*(str+1)=='['&&*(str+2)=='u') {
                        str+=3;
                        if(savey!=-1&&savex!=-1) {
                        	if (slp && (begincol != reg_col)) {
                        		if (slp->len < reg_col)
                        			slp->len = reg_col;
                        		DO_MODIFY;
                        	}
                            good_move(savey,savex);
                            break;
                        }
                        continue;
                      }
              
			if (*str == '\n' || *str == '\r') {
				DO_MODIFY;
				DO_CRLF;
				str++;
				break;
			}
			if (*str == '') {
			    if (iscolor) 
        		        slp->data[reg_col++] = (unsigned char) '';
			    else
        		        inansi=true;
    			} else if (!isprint2(*str))
    				slp->data[reg_col++] = (unsigned char) '*';
			else
				slp->data[reg_col++] = *(str);
			str++;
			if (reg_col >= scr_cols) {
				DO_MODIFY;
				DO_CRLF;
				break;
			}
		}		/* while (*str) */
	}			/* while (1) */
	if (slp && (begincol != reg_col)) {
		if (slp->len < reg_col)
			slp->len = reg_col;
		DO_MODIFY;
		cur_col = reg_col;
	}
}

void
outs(str)
register const char *str;
{
	outns(str, 4096);
}

int dec[] =
    { 1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10,
	1
};

void
prints(char *format, ...)
{
	va_list ap;
	register char *fmt;
	const char *bp;
	register int i, count, hd, indx;
	char *begin;

	va_start(ap, format);
	begin = fmt = format;
	while (*fmt != '\0') {
		if (*fmt == '%') {
			int sgn = 1;
			int val = 0;
			int len, negi;

			if (fmt - begin)
				outns(begin, fmt - begin);
			fmt++;
			while (*fmt == '-') {
				sgn *= -1;
				fmt++;
			}
			while (isdigit(*fmt)) {
				val *= 10;
				val += *fmt - '0';
				fmt++;
			}
			switch (*fmt) {
			case 's':
				bp = va_arg(ap, char *);

				if (bp == NULL)
					bp = nullstr;
				if (val) {
					register int slen = strlen(bp);

					if (val <= slen)
						outns(bp, val);
					else if (sgn > 0) {
						for (slen = val - slen;
						     slen > 0; slen--)
							outc(' ');
						outs(bp);
					} else {
						outs(bp);
						for (slen = val - slen;
						     slen > 0; slen--)
							outc(' ');
					}
				} else
					outs(bp);
				break;
			case 'd':
				i = va_arg(ap, int);

				negi = false;
				if (i < 0) {
					negi = true;
					i *= -1;
				}
				for (indx = 0; indx < 10; indx++)
					if (i >= dec[indx])
						break;
				if (i == 0)
					len = 1;
				else
					len = 10 - indx;
				if (negi)
					len++;
				if (val >= len && sgn > 0) {
					register int slen;

					for (slen = val - len; slen > 0; slen--)
						outc(' ');
				}
				if (negi)
					outc('-');
				hd = 1, indx = 0;
				while (indx < 10) {
					count = 0;
					while (i >= dec[indx]) {
						count++;
						i -= dec[indx];
					}
					indx++;
					if (indx == 10)
						hd = 0;
					if (hd && !count)
						continue;
					hd = 0;
					outc('0' + count);
				}
				if (val >= len && sgn < 0) {
					register int slen;

					for (slen = val - len; slen > 0; slen--)
						outc(' ');
				}
				break;
			case 'c':
				i = va_arg(ap, int);

				outc(i);
				break;
			case '\0':
				goto endprint;
			default:
				outc(*fmt);
				break;
			}
			fmt++;
			begin = fmt;
			continue;
		}

		fmt++;
	}
	if (*begin)
		outs(begin);
      endprint:
	va_end(ap);
	return;
}

void
scroll()
{
	scrollcnt++;
	roll++;
	if (roll >= scr_lns)
		roll -= scr_lns;
	move(scr_lns - 1, 0);
	clrtoeol();
}

void
rscroll()
{
	scrollcnt--;
	if (roll > 0)
		roll--;
	else
		roll = scr_lns - 1;
	move(0, 0);
	clrtoeol();
}

void noscroll()
{
    int i;
    struct screenline bp[30];
    for(i=0;i<scr_lns;i++)
        memcpy(bp+i,big_picture+(i+roll)%scr_lns,sizeof(struct screenline));
    for(i=0;i<scr_lns;i++)
        memcpy(big_picture+i,bp+i,sizeof(struct screenline));
    roll = 0;
}

void
standout()
{
	register struct screenline *slp;
	register int ln;

	if (!scrint) {
	    if (!standing)
		o_standup();
	    return;
	
	}
	if (!standing) {
		ln = cur_ln + roll;
		while (ln >= scr_lns)
			ln -= scr_lns;
		slp = &big_picture[ln];
		standing = true;
		slp->sso = cur_col;
		slp->eso = cur_col;
		slp->mode |= STANDOUT;
	}
}

void
standend()
{
	register struct screenline *slp;
	register int ln;

	if (!scrint) {
	    if (standing)
		o_standdown();
	    return;
	
	}
	if (standing) {
		ln = cur_ln + roll;
		while (ln >= scr_lns)
			ln -= scr_lns;
		slp = &big_picture[ln];
		standing = false;
		slp->eso = Max(slp->eso, cur_col);
	}
}

void
saveline(line, mode, buffer)	/* 0 : save, 1 : restore */
int line, mode;
char *buffer;
{
	register struct screenline *bp = big_picture;
	char *tmp = tmpbuffer;
	int x, y;

	if (buffer)
		tmp = buffer;
	switch (mode) {
	case 0:
		strncpy(tmp /*old_line.data */ , bp[line].data, LINELEN);
		tmp[bp[line].len] = '\0';
		break;
	case 1:
		good_getyx(&x, &y);
		move(line, 0);
		clrtoeol();
		prints("%s", tmp);
		good_move(x, y);
		refresh();
	}
};

void norefresh_saveline(line, mode, buffer)	/* 0 : save, 1 : restore */
int line, mode;
char *buffer;
{
	register struct screenline *bp = big_picture;
	char *tmp = tmpbuffer;
	int x, y;

	if (buffer)
		tmp = buffer;
	switch (mode) {
	case 0:
		strncpy(tmp /*old_line.data */ , bp[line].data, LINELEN);
		tmp[bp[line].len] = '\0';
		break;
	case 1:
		getyx(&x, &y);
		move(line, 0);
		clrtoeol();
		prints("%s", tmp);
		move(x, y);
	}
};

