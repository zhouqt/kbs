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

#define o_clear() {output("\x1b[2J",4); tc_mode=0; tc_color=7;  tc_col=0; tc_line=0; }
#define o_cleol() output("\x1b[K",3)

unsigned char scr_lns, scr_cols;
unsigned char cur_ln = 0, cur_col = 0;
int roll, scrollcnt;
int tc_col=0, tc_line=0;
int tc_mode=0, tc_color = 7;
int cur_mode=0, cur_color=7;
struct screenline *big_picture = NULL;
static const char nullstr[] = "(null)";
static /*struct screenline old_line; */ char tmpbuffer[256*3];

/* –«ø’’Ω∂∑º¶ added by Czz 020926 */
void clrnlines(int n)
{
	register struct screenline *slp;
	register int i, k;
	for (i = cur_ln; i < cur_ln + n; i++) {
		slp = &big_picture[(i + roll) % scr_lns];
		for (k = 0; k < LINELEN; k++) {
			slp->data[k] = 0;
			slp->mode[k] = 0;
                     slp->color[k] = 7;
		}
	}
}
/* added end */

int num_noans_chr(char* str)
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
                    if (isalpha(str[i])) ansi=false;
                    ansinum++;
                    continue;
		}
	}
	return len - ansinum;
}

void init_screen(int slns, int scols)
{
    register struct screenline *slp;
    register int j;

	scr_lns = slns;
	scr_cols = Min(scols, LINELEN);
	if (!big_picture)
		big_picture =
		    (struct screenline *) calloc(scr_lns,
						 sizeof (struct screenline));
	for (slns = 0; slns < scr_lns; slns++) {
		slp = &big_picture[slns];
              for(j=0;j<LINELEN;j++)
                { slp->data[j]=0; slp->mode[j]=0; slp->color[j]=7; }
	}
	roll = 0;
}

void clear()
{
    register int i, j;
    register struct screenline *slp;

    if (!scrint) {
        o_clear();
        return;
    }
    roll = 0;
    for (i = 0; i < scr_lns; i++) {
        slp = big_picture;
        for(j=0; j<scr_cols;j++) {
            if((slp[i].data[j]==0||slp[i].data[j]==' ')&&slp[i].mode[j]&~SCREEN_MODIFIED==0&&slp[i].color[j]/16==0)
                slp[i].mode[j]=0;
            else
                slp[i].mode[j]=SCREEN_MODIFIED;
            slp[i].data[j]=0;
            slp[i].color[j]=7;
        }
    }
    move(0, 0);
}

void initscr()
{
//	if (!big_picture)
//		t_columns = WRAPMARGIN;
//	init_screen(t_lines, WRAPMARGIN);
    init_screen(t_lines, t_columns);
    clear();
    tc_col = 0;
    tc_line = t_lines-1;
}

void rel_move(int was_col, int was_ln, int new_col, int new_ln)
{
	if (new_ln >= t_lines || new_col >= t_columns)
		return;
       if(was_col==new_col&&was_ln==new_ln) return;
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

#define push(x) stack[stackt++]=x
#define outstack() if(stackt>0) {\
    char buf[200],*p;\
    sprintf(buf, "\x1b[");\
    p=buf+2;\
    for(ii=0;ii<stackt;ii++) {\
        if(ii==0) sprintf(p, "%d", stack[ii]); \
        else sprintf(p, ";%d", stack[ii]); \
        p+=strlen(p);\
    }\
    sprintf(p, "m");\
    output(buf, strlen(buf)); \
    stackt=0; }
    
void refresh()
{
    register int i, j, k, ii;
    register struct screenline *bp = big_picture;
    int count=0;
    int stack[100],stackt=0;

    if (!scrint) {
        oflush();
        return;
    }

    if (num_in_buf() != 0)
        return;
    if (scrollcnt < 0) {
        char buf[10];
        rel_move(tc_col, tc_line, 0, 0);
        sprintf(buf, "\033[%dL", -scrollcnt);
        output(buf, strlen(buf));
        scrollcnt = 0;
    }
    if (scrollcnt > 0) {
        do_move(0, 1024, ochar);
        while (scrollcnt > 0) {
            ochar('\n');
            scrollcnt--;
        }
        do_move(0, t_lines - 2, ochar);
        tc_col = 0; tc_line = t_lines-2;
    }
    for (i=0; i<scr_lns; i++)
        for(k=0; k<scr_cols; k++)
            if((bp[i].data[k]==0||bp[i].data[k]==' ')&&bp[i].mode[k]==SCREEN_MODIFIED&&bp[i].color[k]/16==0)
                count++;
    if(count>scr_lns*scr_cols/2) {
        o_clear();
        clear();
    }

    for (i = 0; i < scr_lns; i++) {
        j = (i + roll)%scr_lns;
        for (k = 0; k < scr_cols; k++)
        if((bp[j].mode[k]&SCREEN_MODIFIED)&&(isprint2(bp[j].data[k])||bp[j].data[k]==0)) {
            stackt=0;
            rel_move(tc_col, tc_line, k, i);
            bp[j].mode[k]&=~SCREEN_MODIFIED;
            if(~(bp[j].mode[k])&tc_mode!=0) {
                tc_mode = 0;
                tc_color = 7;
                push(0);
            }
            if(!(tc_mode&SCREEN_BRIGHT)&&bp[j].mode[k]&SCREEN_BRIGHT) {
                tc_mode|=SCREEN_BRIGHT;
                push(1);
            }
            if(!(tc_mode&SCREEN_LINE)&&bp[j].mode[k]&SCREEN_LINE) {
                tc_mode|=SCREEN_LINE;
                push(4);
            }
            if(!(tc_mode&SCREEN_BLINK)&&bp[j].mode[k]&SCREEN_BLINK) {
                tc_mode|=SCREEN_BLINK;
                push(5);
            }
            if(!(tc_mode&SCREEN_BACK)&&bp[j].mode[k]&SCREEN_BACK) {
                tc_mode|=SCREEN_BACK;
                push(7);
            }
            if(tc_color%16!=bp[j].color[k]%16) {
                tc_color=tc_color/16*16+bp[j].color[k]%16;
                push(30+bp[j].color[k]%16);
            }
            if(tc_color/16!=bp[j].color[k]/16) {
                tc_color=bp[j].color[k]/16*16+tc_color%16;
                push(40+bp[j].color[k]/16);
            }
            outstack();
            if(k<scr_cols-3&&(bp[j].data[k]==0||bp[j].data[k]==32)&&(bp[j].data[k+1]==0||bp[j].data[k+1]==32)) {
                int p=1;
                for(ii=k+1;ii<scr_cols;ii++)
                    p=p&&((bp[j].data[ii]==0||bp[j].data[ii]==32)&&(bp[j].color[ii]/16)==(bp[j].color[k]/16)&&((bp[j].mode[ii]&~SCREEN_MODIFIED)==(bp[j].mode[k]&~SCREEN_MODIFIED)));
                if(p) {
                    for(ii=k;ii<scr_cols;ii++)
                        bp[j].mode[ii]&=~SCREEN_MODIFIED;
                    o_cleol();
                    continue;
                }
            }
            if(bp[j].data[k]==0) ochar(' ');
            else ochar(bp[j].data[k]);
            tc_col++;
        }
    }
    rel_move(tc_col, tc_line, cur_col , cur_ln);
    oflush();
}

void redoscr()
{
    register int i, j, k;
    register struct screenline *bp = big_picture;

    if (!scrint) {
        oflush();
        return;
    }
    o_clear();
    for (i = 0; i < scr_lns; i++) {
        j = (i + roll)%scr_lns;
        for (k=0;k < scr_cols; k++)
            bp[j].mode[k]|=SCREEN_MODIFIED;
    }
    refresh();
}

void move(int y, int x)
{
	if (!scrint) {
	    do_move(x, y, ochar);
	    return;
	}
	cur_col = x /*+c_shift(y,x) */ ;
	cur_ln = y;
}

void good_move(int y, int x)
{
    move(y, x);
}

void getyx(int *y, int *x)
{
	*y = cur_ln;
	*x = cur_col /*-c_shift(y,x)*/ ;
}

void good_getyx(int *y, int *x)
{
    getyx(y,x);
}

void clear_whole_line(int i)
{
    register struct screenline *slp = &big_picture[(i+roll)%scr_lns];
    register int k;
    for(k=0;k<scr_cols;k++) {
        if((slp->data[k]==32||slp->data[k]==0)&&slp->mode[k]==cur_mode&&slp->color[k]/16==cur_color/16)
            slp->mode[k]=SCREEN_MODIFIED;
        else
            slp->mode[k]=0;
        slp->data[k]=0;
        slp->color[k]=0;
    }
}

void clrtoeol()
{
    register struct screenline *slp;
    register int ln;
    register int k;

    if (!scrint) {
        o_cleol();
        return;
    }
    ln = (cur_ln + roll)%scr_lns;
    slp = &big_picture[ln];
    for(k=cur_col;k<t_columns;k++) {
        if((slp->data[k]==32||slp->data[k]==0)&&slp->mode[k]==cur_mode&&slp->color[k]/16==cur_color/16)
            slp->mode[k]=SCREEN_MODIFIED;
        else
            slp->mode[k]=0;
        slp->data[k]=0;
        slp->mode[k]|=cur_mode;
        slp->color[k]=cur_color;
    }
}

void clrtobot()
{
    register struct screenline *slp;
    register int i, k, ln;

    for (i = cur_ln; i < scr_lns; i++) {
        ln = (i + roll)%scr_lns;
        slp = &big_picture[ln];
        for(k=0;k<t_columns;k++) 
        if(i!=cur_ln||k>=cur_col)
        {
            if((slp->data[k]==32||slp->data[k]==0)&&slp->mode[k]==cur_mode&&slp->color[k]/16==cur_color/16)
                slp->mode[k]=SCREEN_MODIFIED;
            else
                slp->mode[k]=0;
            slp->data[k]=0;
            slp->mode[k]|=cur_mode;
            slp->color[k]=cur_color;
        }
    }
}

void outc(unsigned char c)
{
    register struct screenline *slp;
    register unsigned int i,j,reg_col;

    if (!scrint) {
        if (c == '\n')
            ochar('\r');
        ochar(c);
        return;
    }
    slp = &big_picture[(cur_ln+roll)%scr_lns];
    if (!isprint2(c)) {
        if (c == '\n' || c == '\r') {	/* do the newline thing */
            clrtoeol();
            cur_col=0; 
            if(cur_ln<scr_lns-1) cur_ln++;
        }
        return;
    }
    if(slp->data[cur_col]!=c||slp->mode[cur_col]!=cur_mode||slp->color[cur_col]!=cur_color) {
        slp->mode[cur_col]=SCREEN_MODIFIED|cur_mode;
        slp->color[cur_col]=cur_color;
        slp->data[cur_col]=c;
        cur_col++;
    }
}

int savey=-1, savex=-1;

void outns(const char*str, int n)
{
    register int i,j,k;
    register char ch;
    register struct screenline *slp = NULL;
    const char *begin_str = str;

    if (!scrint) {
        i=0;
        for (; *begin_str && (i < n); i++, begin_str++)
            outc(*begin_str);
        return;
    }
    while ((str - begin_str < n) && *str) {
        slp = &big_picture[(cur_ln + roll)%scr_lns];

        if (*str == ''&&*(str+1)=='[') {
             i=1;
             while(!isalpha(*(str+i))&&(*(str+i)!='')&&*(str+i)) i++;
             if(*(str+i)=='H') {
                j=0;
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
                        cur_col=x; cur_ln=y;
                    }
                    str+=i+1;
                    continue;
                }
                else if((*str+j)!=';') {
                    clear();
                    str+=i+1;
                    continue;
                }
             }
             else if((*(str+i)=='A'||*(str+i)=='B'||*(str+i)=='C'||*(str+i)=='D')&&i<=4) {
                char s1[5];
                s1[i-2]=0;
                memcpy(s1,str+2,i-2);
                if(s1[0]) k=atoi(s1);
                else k=1;
                
                if(*(str+i)=='A') cur_ln-=k;
                else if(*(str+i)=='B') cur_ln+=k;
                else if(*(str+i)=='C') cur_col+=k;
                else if(*(str+i)=='D') cur_col-=k;

                if(cur_col<0) cur_col=0;
                if(cur_col>=scr_cols) cur_col=scr_cols;
                if(cur_ln<0) cur_ln=0;
                if(cur_ln>=scr_lns) cur_ln=scr_lns-1;

                str+=i+1;
                continue;
             }
             else if(*(str+i)=='s' && i==2) {
                str+=3;
                savey=cur_ln; savex=cur_col;
                continue;
             }
             else if(*(str+i)=='u' && i==2) {
                str+=3;
                if(savey!=-1&&savex!=-1) {
                    cur_ln=savey; cur_col=savex;
                    continue;
                }
             }
             else if(*(str+i)=='J') {
                str+=i+1;
                clear();
                continue;
             }
             else if(*(str+i)=='m') {
                j=2;
                while(*(str+j)!='m') {
                    int m;
                    char s[100];
                    k=j;
                    while(*(str+j)!='m'&&*(str+j)!=';'&&*(str+j)>='0'&&*(str+j)<='9') j++;
                    if(*(str+j)!='m'&&*(str+j)!=';') break;
                    memcpy(s, str+k, j-k);
                    s[j-k]=0;
                    if(s[0]) {
                        m=atoi(s);
                        if(m==0) {
                            cur_mode=0;
                            cur_color=7;
                        } else if(m==1)
                            cur_mode|=SCREEN_BRIGHT;
                        else if(m==4)
                            cur_mode|=SCREEN_LINE;
                        else if(m==5)
                            cur_mode|=SCREEN_BLINK;
                        else if(m==7)
                            cur_mode|=SCREEN_BACK;
                        else if(m>=30&&m<=37)
                            cur_color = m-30+cur_color/16*16;
                        else if(m>=40&&m<=47)
                            cur_color = (m-40)*16+cur_color%16;
                    }
                    j++;
                }
                if(i==2) {
                    cur_mode=0;
                    cur_color=7;
                }
                str+=i+1;
                continue;
             }
             else if(isalpha(*(str+i))) {
                str+=i+1;
                continue;
             }
      }
              
        if (*str == '\n' || *str == '\r') {
            if(cur_ln<scr_lns-1)
                cur_ln++;
            cur_col=0;
            str++;
            continue;
        }
        if (!isprint2(*str)) ch=(unsigned char) '*';
        else ch=*str;
        if(ch!=slp->data[cur_col]||cur_mode!=slp->mode[cur_col]||cur_color!=slp->color[cur_col]) {
            slp->data[cur_col]=ch;
            slp->mode[cur_col]=SCREEN_MODIFIED|cur_mode;
            slp->color[cur_col]=cur_color;
        }
        cur_col++;
        str++;
    }        
}

void outs(register const char*str)
{
    outns(str, 4096);
}

int dec[] =
    { 1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10,
	1
};

void prints(char *format, ...)
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

void scroll()
{
	scrollcnt++;
	roll++;
	if (roll >= scr_lns)
		roll -= scr_lns;
	move(scr_lns - 1, 0);
	clrtoeol();
}

void rscroll()
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

void saveline(int line, int mode, char* buffer)	/* 0 : save, 1 : restore */
{
    register struct screenline *bp = big_picture;
    char *tmp = tmpbuffer;
    int i;

    if (buffer)
        tmp = buffer;
    switch (mode) {
        case 0:
            strncpy(tmp, bp[line].data, LINELEN);
            strncpy(tmp+LINELEN, bp[line].mode, LINELEN);
            strncpy(tmp+LINELEN*2, bp[line].color, LINELEN);
            break;
        case 1:
            strncpy(bp[line].data, tmp, LINELEN);
            strncpy(bp[line].mode, tmp+LINELEN, LINELEN);
            strncpy(bp[line].color, tmp+LINELEN*2, LINELEN);
            for(i=0;i<scr_lns;i++)
                bp[line].mode[i]|=SCREEN_MODIFIED;
            refresh();
            break;
    }
};

void norefresh_saveline(int line, int mode, char*buffer)	/* 0 : save, 1 : restore */
{
    register struct screenline *bp = big_picture;
    char *tmp = tmpbuffer;
    int i;

    if (buffer)
        tmp = buffer;
    switch (mode) {
        case 0:
            strncpy(tmp, bp[line].data, LINELEN);
            strncpy(tmp+LINELEN, bp[line].mode, LINELEN);
            strncpy(tmp+LINELEN*2, bp[line].color, LINELEN);
            break;
        case 1:
            strncpy(bp[line].data, tmp, LINELEN);
            strncpy(bp[line].mode, tmp+LINELEN, LINELEN);
            strncpy(bp[line].color, tmp+LINELEN*2, LINELEN);
            for(i=0;i<scr_lns;i++)
                bp[line].mode[i]|=SCREEN_MODIFIED;
            break;
    }
};

