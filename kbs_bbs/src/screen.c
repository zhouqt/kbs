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

#define o_clear() {if(tc_color!=7||tc_mode!=0) output("\x1b[m",3);output("\x1b[H\x1b[J",6); tc_mode=0; tc_color=7;  tc_col=0; tc_line=0; }
#define o_cleol() output("\x1b[K",3)

unsigned char scr_lns, scr_cols;
unsigned char cur_ln = 0, cur_col = 0;
int can_clrscr = 0;
int roll, scrollcnt;
int tc_col=0, tc_line=0;
int tc_mode=0, tc_color = 7;
int cur_mode=0, cur_color=7;
int offsetln = 0;
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
			slp->data[k] = 32;
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
    
    if (big_picture)
        free(big_picture);
    big_picture = (struct screenline *) calloc(scr_lns, sizeof (struct screenline));
    for (slns = 0; slns < scr_lns; slns++) {
        slp = &big_picture[slns];
        for(j=0;j<LINELEN;j++)
        { slp->data[j]=32; slp->mode[j]=0; slp->color[j]=7; slp->ldata[j]=255;}
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
            slp[i].data[j]=32;
            slp[i].mode[j]=0;
            slp[i].color[j]=7;
        }
    }
    can_clrscr = 1;
    move(0, 0);
}

void initscr()
{
    init_screen(t_lines, t_columns);
}

void rel_move(int was_col, int was_ln, int new_col, int new_ln)
{
    int i;
    struct screenline *bp = big_picture;
    if (new_ln >= t_lines || new_col >= t_columns)
        return;
    if(was_col==new_col&&was_ln==new_ln) return;
    tc_col = new_col;
    tc_line = new_ln;
    if ((new_col == 0) && (new_ln == was_ln + 1)) {
        ochar('\n');
        if(tc_color%16!=7)
            tc_color = tc_color/16*16+8;
        if (was_col != 0)
            ochar('\r');
        return;
    }
    if ((new_col == 0) && (new_ln == was_ln)) {
        ochar('\r');
        return;
    }
    if (new_col <= was_col - 1 && new_col>=was_col-5 && new_ln == was_ln) {
        for(i=0;i<was_col-new_col;i++)
            ochar(Ctrl('H'));
        return;
    }
    if (new_ln == was_ln && new_col>=was_col+1&&new_col<=was_col+5) {
        int p=1, q=(new_ln+roll)%scr_lns;
        for(i=was_col;i<new_col;i++)
            p=p&&(bp[q].color[i]==tc_color)&&(bp[q].mode[i]==tc_mode);
        if(p) {
            for(i=was_col;i<new_col;i++)
                if(bp[q].data[i]==0) ochar(32);
                else ochar(bp[q].data[i]);
            return;
        }
    }
    if(new_ln == was_ln+1 && new_col<=5) {
        int p=1, q=(new_ln+roll)%scr_lns;
        for(i=0;i<new_col;i++)
            p=p&&(bp[q].color[i]==tc_color)&&(bp[q].mode[i]==tc_mode);
        if(p) {
            ochar('\n');
            if(tc_color%16!=7)
                tc_color = tc_color/16*16+8;
            if (was_col != 0)
                ochar('\r');
            for(i=0;i<new_col;i++)
                if(bp[q].data[i]==0) ochar(32);
                else ochar(bp[q].data[i]);
            return;
        }
    }

    do_move(new_col, new_ln, ochar);
}

#define ndiff(i,j) (bp[i].data[j]==bp[i].ldata[j]&&bp[i].mode[j]==bp[i].lmode[j]&&bp[i].color[j]==bp[i].lcolor[j])


void refresh()
{
    register int i, j, k, ii, p, s;
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
        do_move(0, t_lines - 1, ochar);
        while (scrollcnt > 0) {
            ochar('\n');
            scrollcnt--;
        }
        tc_col = 0; tc_line = t_lines-1;
    }
    count = 0;
    for (i=0; i < scr_lns; i++)
        for(j=0;j<scr_cols;j++) 
        if(!ndiff(i,j)) {
            if((bp[i].data[j]==0||bp[i].data[j]==32)&&(bp[i].color[j]/16==0))
                count++;
            else
                count--;
        }
    if(count>5&&can_clrscr) {
        o_clear();
        for (i=0; i < scr_lns; i++)
            for(j=0;j<scr_cols;j++) {
                bp[i].ldata[j] = 32;
                bp[i].lmode[j] = 0;
                bp[i].lcolor[j] = 7;
            }
    }
    can_clrscr = 0;
    
    for (i = 0; i < scr_lns; i++) {
        j = (i + roll)%scr_lns;

        ii=scr_cols-1;
        while(ii>=0&&(bp[j].data[ii]==0||bp[j].data[ii]==32)&&(bp[j].color[ii]/16)==(bp[j].color[scr_cols-1]/16)&&((bp[j].mode[ii]&~SCREEN_BRIGHT)==(bp[j].mode[scr_cols-1]&~SCREEN_BRIGHT))) ii--;
        p=ii+1;
        count = 0;
        for(ii=p;ii<scr_cols;ii++)
            if(!ndiff(j,ii)) count++;

        for (k = 0; k < scr_cols; k++)
        if(!ndiff(j,k)&&(isprint2(bp[j].data[k]))||(k>=p&&count>=5)) {
            stackt=0;
            rel_move(tc_col, tc_line, k, i);
            s = bp[j].mode[k];
            if((!(s&SCREEN_BRIGHT)&&tc_mode&SCREEN_BRIGHT&&bp[j].data[k]!=' '||
                !(s&SCREEN_LINE)&&tc_mode&SCREEN_LINE||
                !(s&SCREEN_BLINK)&&tc_mode&SCREEN_BLINK&&bp[j].data[k]!=' '||
                !(s&SCREEN_BACK)&&tc_mode&SCREEN_BACK)||(tc_color/16!=0&&bp[j].color[k]/16==0)) {
                char buf[10];
                tc_mode = 0;
                tc_color = 7;
                sprintf(buf, "\x1b[m");
                output(buf, strlen(buf));
            }
            if(!(tc_mode&SCREEN_BRIGHT)&&s&SCREEN_BRIGHT&&bp[j].data[k]!=' '&&bp[j].data[k]!=0) {
                tc_mode|=SCREEN_BRIGHT;
                stack[stackt++]=1;
            }
            if(!(tc_mode&SCREEN_LINE)&&s&SCREEN_LINE) {
                tc_mode|=SCREEN_LINE;
                stack[stackt++]=4;
            }
            if(!(tc_mode&SCREEN_BLINK)&&s&SCREEN_BLINK&&bp[j].data[k]!=' '&&bp[j].data[k]!=0) {
                tc_mode|=SCREEN_BLINK;
                stack[stackt++]=5;
            }
            if(!(tc_mode&SCREEN_BACK)&&s&SCREEN_BACK) {
                tc_mode|=SCREEN_BACK;
                stack[stackt++]=7;
            }
            if(tc_color%16!=bp[j].color[k]%16&&(bp[j].data[k]!=' '&&bp[j].data[k]!=0||bp[j].mode[k]&SCREEN_LINE)) {
                tc_color=tc_color/16*16+bp[j].color[k]%16;
                if(DEFINE(currentuser, DEF_COLOR))
                    stack[stackt++]=30+bp[j].color[k]%16;
            }
            if(tc_color/16!=bp[j].color[k]/16) {
                tc_color=bp[j].color[k]/16*16+tc_color%16;
                if(DEFINE(currentuser, DEF_COLOR)) {
                    if(bp[j].color[k]/16==8)
                        stack[stackt++]=40;
                    else
                        stack[stackt++]=40+bp[j].color[k]/16;
                }
            }
            if(stackt>0) {
                char buf[200],*p;
                sprintf(buf, "\x1b[");
                p=buf+2;
                if(stackt!=1||stack[0]!=0)
                for(ii=0;ii<stackt;ii++) {
                    if(ii==0) sprintf(p, "%d", stack[ii]);
                    else sprintf(p, ";%d", stack[ii]);
                    p+=strlen(p);
                }
                sprintf(p, "m");
                output(buf, strlen(buf));
                stackt=0; 
            }
            if(k>=p&&p<=scr_cols-5) {
                for(ii=k;ii<scr_cols;ii++) {
                    bp[j].ldata[ii]=bp[j].data[ii];
                    bp[j].lmode[ii]=bp[j].mode[ii];
                    bp[j].lcolor[ii]=bp[j].color[ii];
                }
                o_cleol();
                break;
            }
            if(bp[j].data[k]==0) ochar(' ');
            else ochar(bp[j].data[k]);
            bp[j].ldata[k]=bp[j].data[k];
            bp[j].lmode[k]=bp[j].mode[k];
            bp[j].lcolor[k]=bp[j].color[k];
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
    for (i = 0; i < scr_lns; i++) {
        j = (i + roll)%scr_lns;
        for (k=0;k < scr_cols; k++)
            bp[j].ldata[k]=255;
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
        slp->data[k]=32;
        slp->mode[k]=cur_mode;
        slp->color[k]=cur_color;
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
        slp->data[k]=32;
        slp->mode[k]=cur_mode;
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
        for(k=0;k<scr_cols;k++) 
        if(i>cur_ln||k>=cur_col)
        {
            slp->data[k]=32;
            slp->mode[k]=cur_mode;
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
    if(cur_col<scr_cols)
    {
        slp->mode[cur_col]=cur_mode;
        slp->color[cur_col]=cur_color;
        slp->data[cur_col]=c;
    }
    cur_col++;
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
                    y=atoi(s1)-1+offsetln;
                    x=atoi(s2)-1;
                    if(y>=0&&y<scr_lns&&x>=0&&x<scr_cols) {
                        cur_col=x; cur_ln=y;
                    }
                    str+=i+1;
                    continue;
                }
                else if((*str+j)!=';') {
                    if(offsetln==0)
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
                if(cur_ln<offsetln) cur_ln=offsetln;
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
                        else if(m>=40&&m<=47) {
                            if(m==40) m=48;
                            cur_color = (m-40)*16+cur_color%16;
                        }
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
             else if(*(str+i)=='M') {
                k=1;
                for(j=2;j<i;j++) k=k&&(*(str+j)>='0'&&*(str+j)<='9');
                if(k) {
                    refresh();
                    output(str, i+1);
                    oflush();
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
            clrtoeol();
            if(cur_ln<scr_lns-1)
                cur_ln++;
            cur_col=0;
            str++;
            continue;
        }
        if(*str==9) {  //¥¶¿Ìtab
            ch=32;
            j=(cur_col/8+1)*8-cur_col;
            for(i=0;i<j;i++) {
                if(cur_col<scr_cols)
                {
                    slp->data[cur_col]=ch;
                    slp->mode[cur_col]=cur_mode;
                    slp->color[cur_col]=cur_color;
                }
                cur_col++;
            }
            str++;
            continue;
        }
        else if (!isprint2(*str)) ch=(unsigned char) '*';
        else ch=*str;
        if(cur_col<scr_cols)
        {
            slp->data[cur_col]=ch;
            slp->mode[cur_col]=cur_mode;
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
    struct screenline bp[100];
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
            memcpy(tmp, bp[line].data, LINELEN);
            memcpy(tmp+LINELEN, bp[line].mode, LINELEN);
            memcpy(tmp+LINELEN*2, bp[line].color, LINELEN);
            break;
        case 1:
            memcpy(bp[line].data, tmp, LINELEN);
            memcpy(bp[line].mode, tmp+LINELEN, LINELEN);
            memcpy(bp[line].color, tmp+LINELEN*2, LINELEN);
            break;
    }
};

void do_naws(int ln, int col)
{
    t_lines = ln;
    t_columns = col;
    if(t_lines<24||t_lines>100)
        t_lines=24;
    if(t_columns<80||t_columns>240)
        t_columns = 80;
    initscr();
    refresh();
}
