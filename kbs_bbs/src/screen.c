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

int scr_lns, scr_cols;
int cur_ln = 0, cur_col = 0;
int can_clrscr = 0;
int roll, scrollcnt;
int tc_col=0, tc_line=0;
int tc_mode=0, tc_color = 7;
int cur_mode=0, cur_color=7;
int offsetln = 0, minln=0;
struct screenline *big_picture = NULL;
static const char nullstr[] = "(null)";
static /*struct screenline old_line; */ char tmpbuffer[LINELEN*3];

void setfcolor(int i,int j)
{
    if(!DEFINE(currentuser, DEF_COLOR)) return;
    cur_color = i+(cur_color&0xf0);
    if(j) cur_mode|=SCREEN_BRIGHT;
    else cur_mode&=~SCREEN_BRIGHT;
}

void setbcolor(int i)
{
    if(!DEFINE(currentuser, DEF_COLOR)) return;
    if(i==0) i=8;
    cur_color = (cur_color&0x0f)+(i<<4);
}

void resetcolor()
{
    cur_mode = 0;
    cur_color = 7;
}

/* ÐÇ¿ÕÕ½¶·¼¦ added by Czz 020926 */
void clrnlines(int n)
{
	struct screenline *slp;
	int i, k;
	for (i = cur_ln; i < cur_ln + n; i++) {
		slp = &big_picture[(i + roll) % scr_lns];
		memset(slp->data, 32, LINELEN);
		memset(slp->mode, 0, LINELEN);
		memset(slp->color, 7, LINELEN);
		slp->changed = true;
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
    struct screenline *slp, *oslp;
    struct screenline *oldp = big_picture;
    int j, oldln = scr_lns;

    scr_lns = slns;
    scr_cols = Min(scols, LINELEN);
    
    big_picture = (struct screenline *) calloc(scr_lns, sizeof (struct screenline));
    for (slns = 0; slns < scr_lns; slns++) {
        slp = &big_picture[slns];
        memset(slp->data, 32, LINELEN);
        memset(slp->mode, 0, LINELEN);
        memset(slp->color, 7, LINELEN);
        memset(slp->ldata, 255, LINELEN);
    }
    roll = 0;
    if (!oldp)
        return;
    for (slns = 0; slns < Min(oldln, scr_lns); slns++) {
    	slp = &big_picture[slns];
    	oslp = &oldp[slns];
    	memcpy(slp->data, oslp->data, LINELEN);
    	memcpy(slp->mode, oslp->mode, LINELEN);
    	memcpy(slp->color, oslp->color, LINELEN);
       slp->changed = true;
    }
    free(oldp);
}

void clear()
{
    int i, j;
    struct screenline *slp;

    if (!scrint) {
        o_clear();
        return;
    }
    for (i = 0; i < scr_lns; i++) {
        slp = big_picture+i;
        memset(slp->data, 32, scr_cols);
        memset(slp->mode, 0, scr_cols);
        memset(slp->color, 7, scr_cols);
        slp->changed = true;
    }
    cur_color = 7;
    cur_mode = 0;
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
    if (new_ln >= t_lines || new_col > t_columns)
        return;
    if(new_col == t_columns) new_col--;
    if(was_col==new_col&&was_ln==new_ln) return;
    tc_col = new_col;
    tc_line = new_ln;
    if ((new_col == 0) && (new_ln == was_ln + 1)) {
        ochar('\n');
        if((tc_color&0x0f)!=7)
            tc_color = (tc_color&0xf0)+8;
        if (was_col != 0)
            ochar('\r');
        return;
    }
    if ((new_col == 0) && (new_ln == was_ln)) {
        ochar('\r');
        return;
    }
    if (new_col <= was_col - 1 && new_col>=was_col-5 && new_ln == was_ln) {
        char s[LINELEN];
        memset(s, Ctrl('H'), was_col-new_col);
        output(s, was_col-new_col);
        return;
    }
    if (new_ln == was_ln && new_col>=was_col+1&&new_col<=was_col+5) {
        int p=1, q=(new_ln+roll)%scr_lns;
        for(i=was_col;i<new_col;i++)
            p=p&&(bp[q].color[i]==tc_color)&&(bp[q].mode[i]==tc_mode);
        if(p) {
            output(bp[q].data+was_col, new_col-was_col);
            return;
        }
    }
    if (new_ln == was_ln && new_col>=was_col+1) {
        char ss[20];
        if(new_col==was_col+1)
            sprintf(ss, "\x1b[C");
        else
            sprintf(ss, "\x1b[%dC", new_col-was_col);
        output(ss, strlen(ss));
        return;
    }
    if (new_ln == was_ln && new_col<=was_col-1) {
        char ss[20];
        if(new_col==was_col-1)
            sprintf(ss, "\x1b[D");
        else
            sprintf(ss, "\x1b[%dD", was_col-new_col);
        output(ss, strlen(ss));
        return;
    }
    if ((new_col == was_col || new_col==0) && new_ln>=was_ln+1) {
        char ss[20];
        if((tc_color&0x0f)!=7)
            tc_color = (tc_color&0xf0)+8;
        if(new_ln==was_ln+1)
            sprintf(ss, "\x1b[B");
        else
            sprintf(ss, "\x1b[%dB", new_ln-was_ln);
        output(ss, strlen(ss));
        if(new_col==0&&was_col!=0)
            ochar('\r');
        return;
    }
    if ((new_col == was_col || new_col==0) && new_ln<=was_ln-1) {
        char ss[20];
        if((tc_color&0x0f)!=7)
            tc_color = (tc_color&0xf0)+8;
        if(new_ln==was_ln-1)
            sprintf(ss, "\x1b[A");
        else
            sprintf(ss, "\x1b[%dA", was_ln-new_ln);
        output(ss, strlen(ss));
        if(new_col==0&&was_col!=0)
            ochar('\r');
        return;
    }
    if(new_ln == was_ln+1 && new_col<=5) {
        int p=1, q=(new_ln+roll)%scr_lns;
        for(i=0;i<new_col;i++)
            p=p&&(bp[q].color[i]==tc_color)&&(bp[q].mode[i]==tc_mode);
        if(p) {
            ochar('\n');
            if((tc_color&0x0f)!=7)
                tc_color = (tc_color&0xf0)+8;
            if (was_col != 0)
                ochar('\r');
            output(bp[q].data, new_col);
            return;
        }
    }

    do_move(new_col, new_ln, ochar);
}

#define ndiff(i,j) (bp[i].data[j]==bp[i].ldata[j]&&bp[i].mode[j]==bp[i].lmode[j]&&bp[i].color[j]==bp[i].lcolor[j])


void refresh()
{
    int i, j, k, ii, p, s;
    struct screenline *bp = big_picture;
    int chc;
    bool flagc;
    int count=0;
    int stack[10],stackt=0;

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
        do_move(0, scr_lns-1, ochar);
        tc_col = 0; tc_line = scr_lns-1;
    }
/*    if(can_clrscr) {
        o_clear();
        can_clrscr = 0;
        for (i=0; i < scr_lns; i++) {
            memset(bp[i].ldata, 32, scr_cols);
            memset(bp[i].lmode, 0, scr_cols);
            memset(bp[i].lcolor, 7, scr_cols);
        }
    }*/
    
    for (i = 0; i < scr_lns; i++) {
        j = (i + roll)%scr_lns;
        if(!bp[j].changed) continue;

        bp[j].changed = false;

        ii=scr_cols-1;
        count = 0;
        while(ii>=0&&(bp[j].data[ii]==0||bp[j].data[ii]==32)&&(bp[j].color[ii]>>4)==(bp[j].color[scr_cols-1]>>4)&&((bp[j].mode[ii]&~SCREEN_BRIGHT)==(bp[j].mode[scr_cols-1]&~SCREEN_BRIGHT))) {
            if(count<3&&!ndiff(j,ii)) count++;
            ii--;
        }
        p=ii+1;

        chc = 0;
        flagc = false;
        for (k = 0; k < scr_cols; k++) {
            if(chc==1) chc=2;
            else if(bp[j].data[k]&0x80) chc=1;
            else chc=0;
            if(flagc||(chc==1)&&(k<scr_cols-1)&&(!ndiff(j,k+1))||!ndiff(j,k)&&(isprint2(bp[j].data[k]))||(k>=p&&(count>=3||count>0&&i==scr_lns-1))) {
                stackt=0;
                rel_move(tc_col, tc_line, k, i);
                s = bp[j].mode[k];
                if((!(s&SCREEN_BRIGHT)&&tc_mode&SCREEN_BRIGHT&&bp[j].data[k]!=' '||
                    !(s&SCREEN_LINE)&&tc_mode&SCREEN_LINE||
                    !(s&SCREEN_BLINK)&&tc_mode&SCREEN_BLINK&&bp[j].data[k]!=' '||
                    !(s&SCREEN_BACK)&&tc_mode&SCREEN_BACK)||((tc_color>>4)!=0&&(bp[j].color[k]>>4)==0)) {
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
                if((tc_color&0x0f)!=(bp[j].color[k]&0x0f)&&(bp[j].data[k]!=' '||bp[j].mode[k]&SCREEN_LINE||bp[j].mode[k]&SCREEN_BACK)) {
                    tc_color=(tc_color&0xf0)+(bp[j].color[k]&0x0f);
                    stack[stackt++]=30+(bp[j].color[k]&0x0f);
                }
                if((tc_color>>4)!=(bp[j].color[k]>>4)) {
                    tc_color=(bp[j].color[k]&0xf0)+(tc_color&0x0f);
                    if((bp[j].color[k]>>4)==8)
                        stack[stackt++]=40;
                    else
                        stack[stackt++]=40+(bp[j].color[k]>>4);
                }
                if(stackt>0) {
                    char buf[200],*p;
                    int pos = 2;
                    sprintf(buf, "\x1b[");
                    p=buf+2;
                    if(stackt!=1||stack[0]!=0)
                    for(ii=0;ii<stackt;ii++) {
                        pos++;
                        if(ii==0) sprintf(p, "%d", stack[ii]);
                        else {sprintf(p, ";%d", stack[ii]);pos++;}
                        if(stack[ii]>9) pos++;
                        p=buf+pos;
                    }
                    *p='m'; pos++;
                    output(buf, pos);
                    stackt=0; 
                }
                if(k>=p&&(p<=scr_cols-4||i==scr_lns-1)) {
                    memcpy(bp[j].ldata+k, bp[j].data+k, scr_cols-k);
                    memcpy(bp[j].lmode+k, bp[j].mode+k, scr_cols-k);
                    memcpy(bp[j].lcolor+k, bp[j].color+k, scr_cols-k);
                    o_cleol();
                    break;
                }
                if(chc==1&&(k==scr_cols-1||(bp[j].data[k+1]<0x40)))
                    ochar('?');
                else
                    ochar(bp[j].data[k]);
                if(chc==1) flagc=true;
                bp[j].ldata[k]=bp[j].data[k];
                bp[j].lmode[k]=bp[j].mode[k];
                bp[j].lcolor[k]=bp[j].color[k];
                tc_col++;
            }
        }
    }
    rel_move(tc_col, tc_line, cur_col , cur_ln);
    oflush();
}

void redoscr()
{
    int i, j, k;
    struct screenline *bp = big_picture;

    if (!scrint) {
        oflush();
        return;
    }
    for (i = 0; i < scr_lns; i++) {
        j = (i + roll)%scr_lns;
        memset(bp[j].ldata, 255, scr_cols);
        bp[j].changed = true;
    }
    do_move(tc_col, tc_line, ochar);
    refresh();
}

void move(int y, int x)
{
	if (!scrint) {
            if(x>=0&&y>=0&&x<=scr_cols&&y<=scr_lns)
	    do_move(x, y, ochar);
	    return;
	}
	if(x<0) cur_col = scr_cols+x;
	else cur_col = x;
	cur_ln = y;
	if(cur_col<0) cur_col=0;
	if(cur_col>scr_cols) cur_col=scr_cols;
	if(cur_ln<0) cur_ln=0;
	if(cur_ln>=scr_lns) cur_ln=scr_lns-1;
}

void getyx(int *y, int *x)
{
    *y = cur_ln;
    *x = cur_col;
}

void clear_whole_line(int i)
{
    struct screenline *slp = &big_picture[(i+roll)%scr_lns];
    int k;
    memset(slp->data, 32, scr_cols);
    memset(slp->mode, cur_mode, scr_cols);
    memset(slp->color, cur_color, scr_cols);
    slp->changed = true;
}

void clrtoeol()
{
    struct screenline *slp;
    int ln;
    int k;

    if (!scrint) {
        o_cleol();
        return;
    }
    ln = (cur_ln + roll)%scr_lns;
    slp = &big_picture[ln];
    memset(slp->data+cur_col, 32, scr_cols-cur_col);
    memset(slp->mode+cur_col, cur_mode, scr_cols-cur_col);
    memset(slp->color+cur_col, cur_color, scr_cols-cur_col);
    slp->changed = true;
}

void clrtobot()
{
    struct screenline *slp;
    int i, k, ln;

    for (i = cur_ln; i < scr_lns; i++) {
        ln = (i + roll)%scr_lns;
        slp = &big_picture[ln];
        if(i==cur_ln) k=cur_col;
        else k=0;
        memset(slp->data+k, 32, scr_cols-k);
        memset(slp->mode+k, cur_mode, scr_cols-k);
        memset(slp->color+k, cur_color, scr_cols-k);
        slp->changed = true;
    }
}

void outc(unsigned char c)
{
    struct screenline *slp;
    unsigned int i,j,reg_col;

    if (!scrint) {
        if (c == '\n')
            ochar('\r');
        ochar(c);
        return;
    }
    slp = &big_picture[(cur_ln+roll)%scr_lns];
    if (!isprint2(c)) {
        if (c == '\r') return;
        if (c == '\n') {	/* do the newline thing */
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
        slp->changed = true;
        cur_col++;
    }
}

int savey=-1, savex=-1;
bool disable_move = false;
bool disable_color = false;

void outns(const char*str, int n)
{
    int i,j,k;
    char ch;
    struct screenline *slp = NULL;
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
                if(*(str+j)==';'&&j<=4&&j>=3&&i-j>=2&&i-j<=4) {
                    char s1[5],s2[5],x,y;
                    memcpy(s1,str+2,j-2);
                    s1[j-2]=0;
                    memcpy(s2,str+j+1,i-j-1);
                    s2[i-j-1]=0;
                    y=atoi(s1)-1+offsetln;
                    x=atoi(s2)-1;
                    if (x<-1) x = scr_cols+x+1;
                    if(DEFINE(currentuser, DEF_COLOR))
                    if(y>=0&&y<scr_lns&&x>=0&&x<=scr_cols&&!disable_move) {
                        cur_col=x; cur_ln=y;
                        if(cur_ln<minln) cur_ln=minln;
                    }
                    str+=i+1;
                    continue;
                }
                else if((*str+j)!=';') {
                    if(!disable_move) {
                        if(minln) {
                            move(minln, 0);
                            clrtobot();
                        }
                        else
                            clear();
                    }
                    str+=i+1;
                    continue;
                }
             }
             else if((*(str+i)=='A'||*(str+i)=='B'||*(str+i)=='C'||*(str+i)=='D')&&i<=5) {
                char s1[5];
                s1[i-2]=0;
                memcpy(s1,str+2,i-2);
                if(s1[0]) k=atoi(s1);
                else k=1;

                if(DEFINE(currentuser, DEF_COLOR))
                if(!disable_move) {
                
                    if(*(str+i)=='A') {
                        if(cur_ln>=k) cur_ln-=k;
                        else cur_ln=0;
                    }
                    else if(*(str+i)=='B') {
                        if(cur_ln<scr_lns-k) cur_ln+=k;
                        else cur_ln = scr_lns-1;
                    }
                    else if(*(str+i)=='C') {
                        if(cur_col<scr_cols-k) cur_col+=k;
                        else cur_col=scr_cols;
                    }
                    else if(*(str+i)=='D') {
                        if(cur_col>=k) cur_col-=k;
                        else cur_col=0;
                    }

                    if(cur_col<0) cur_col=0;
                    if(cur_col>=scr_cols) cur_col=scr_cols;
                    if(cur_ln>=scr_lns) cur_ln=scr_lns-1;
                    if(cur_ln<minln) cur_ln=minln;
                }

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
                if(DEFINE(currentuser, DEF_COLOR))
                if(savey!=-1&&savex!=-1&&!disable_move) {
                    cur_ln=savey; cur_col=savex;
                    if(cur_ln<minln) cur_ln=minln;
                    continue;
                }
             }
             else if(*(str+i)=='K' && i==2) {
                str+=3;
                clrtoeol();
                continue;
             }
             else if(*(str+i)=='J') {
                str+=i+1;
                if(DEFINE(currentuser, DEF_COLOR))
                if(!disable_move) {
                    if(minln) {
                        move(minln, 0);
                        clrtobot();
                    }
                    else
                        clear();
                }
                continue;
             }
             else if(*(str+i)=='m') {
                j=1;
                if(DEFINE(currentuser, DEF_COLOR)&&!disable_color)
                while(*(str+j)!='m') {
                    int m;
                    char s[100];
                    j++;
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
                            cur_color = m-30+(cur_color&0xf0);
                        else if(m>=40&&m<=47) {
                            if(m==40) m=48;
                            cur_color = ((m-40)<<4)+(cur_color&0x0f);
                        }
                    }
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
                if(DEFINE(currentuser, DEF_COLOR))
                if(!disable_move)
                if(k) {
                    refresh();
                    output(str, i+1);
                }
                str+=i+1;
                continue;
             }
             else if(isalpha(*(str+i))) {
                str+=i+1;
                continue;
             }
      }
        if (*str == '\r')  {
            str++;
            continue;    
        }
        if (*str == '\n') {
            if(cur_ln<scr_lns-1)
                cur_ln++;
            cur_col=0;
            str++;
            continue;
        }
        if(*str==9) {  //´¦Àítab
            ch=32;
            j=(cur_col/8+1)*8-cur_col;
            for(i=0;i<j;i++) {
                if(cur_col<scr_cols)
                {
                    slp->data[cur_col]=ch;
                    slp->mode[cur_col]=cur_mode;
                    slp->color[cur_col]=cur_color;
                    slp->changed = true;
                    cur_col++;
                }
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
            slp->changed = true;
            cur_col++;
        }
        str++;
    }        
}

void outs(const char*str)
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
	char *fmt;
	const char *bp;
	int i, count, hd, indx;
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
					int slen = strlen(bp);

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
					int slen;

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
					int slen;

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

void outline(char *s)
{
    int y,x;
    while(*s) {
        if(*s=='\n') {
            getyx(&y, &x);
            if(y+1>=scr_lns) {
                scroll();
                move(y, 0);
            }
            else
                move(y+1, 0);
            s++;
            continue;
        }
        outc(*s);
        s++;
        getyx(&y, &x);
        if(x>=scr_cols) {
            if(y+1>=scr_lns) {
                scroll();
                move(y, 0);
            }
            else
                move(y+1, 0);
        }
    }
}


void scroll()
{
    int ln,k;
    struct screenline *slp;
    scrollcnt++;
    roll++;
    if (roll >= scr_lns)
        roll -= scr_lns;
    move(scr_lns - 1, 0);
    ln = (cur_ln + roll)%scr_lns;
    slp = &big_picture[ln];
    memset(slp->ldata, 255, scr_cols);
    memset(slp->data, 32, scr_cols);
    memset(slp->mode, 0, scr_cols);
    memset(slp->color, 7, scr_cols);
}

void rscroll()
{
    int ln,k;
    struct screenline *slp;
    scrollcnt--;
    if (roll > 0) roll--;
    else roll = scr_lns - 1;
    move(0, 0);
    ln = (cur_ln + roll)%scr_lns;
    slp = &big_picture[ln];
    memset(slp->ldata, 255, scr_cols);
    memset(slp->data, 32, scr_cols);
    memset(slp->mode, 0, scr_cols);
    memset(slp->color, 7, scr_cols);
}

void noscroll()
{
    int i;
    struct screenline bp[LINEHEIGHT];
    for(i=0;i<scr_lns;i++)
        memcpy(bp+i,big_picture+(i+roll)%scr_lns,sizeof(struct screenline));
    for(i=0;i<scr_lns;i++)
        memcpy(big_picture+i,bp+i,sizeof(struct screenline));
    roll = 0;
}

int check_ch(int c1, int c2)
{
    if(c1>=0xb0&&c1<=0xd8&&c2>=0xa1&&c2<=0xfe)
        return 11;
    else if(c1>=0xa1&&c1<0xb0&&c2>=0xa1&&c2<=0xfe)
        return 6;
    else if(c1>=0xd9&&c1<=0xf7&&c2>=0xa1&&c2<=0xfe)
        return 5;
    else if(c1>0x80&&c2>=0x80)
        return 2;
    else if(c1>0x80&&c2>=0x40)
        return 1;
    else
        return 0;
}

void auto_chinese()
{
    struct screenline *bp = big_picture;
    int i,j,k,l;
    int a[LINELEN],b[LINELEN];
    for(i=0;i<scr_lns;i++) {
        j = (i + roll)%scr_lns;
        a[0]=0; b[0]=0;
        a[1]=check_ch(bp[j].data[0],bp[j].data[1]);
        if(a[1]) b[1]=1; else b[1]=0;
        for(k=2;k<scr_cols;k++) {
            l = check_ch(bp[j].data[k-1],bp[j].data[k]);
            if(l&&l+a[k-2]>a[k-1]) {
                a[k]=l+a[k-2];
                b[k]=1;
            }
            else {
                a[k]=a[k-1];
                b[k]=0;
            }
        }
        k=scr_cols-1;
        while(k>=0) {
            if(b[k]) k-=2;
            else {
                k--;
                if(bp[j].data[k+1]>=0x80) bp[j].data[k+1]='?';
            }
        }
    }
    redoscr();
}

void saveline(int line, int mode, char* buffer)	/* 0 : save, 1 : restore */
{
    struct screenline *bp = big_picture;
    char *tmp = tmpbuffer;
    int i;

    if (buffer)
        tmp = buffer;
    line=(line+roll)%scr_lns;
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
            bp[line].changed = true;
            break;
    }
};

void do_naws(int ln, int col)
{
    t_lines = ln;
    t_columns = col;
    if(t_lines<24)
        t_lines=24;
    if(t_lines>LINEHEIGHT)
        t_lines=LINEHEIGHT;
    if(t_columns<80)
        t_columns = 80;
    if(t_columns>LINELEN)
        t_columns = LINELEN;
    initscr();
}
