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
#ifdef AIX
#include <sys/select.h>
#endif

#define OBUFSIZE  (4096)
#define IBUFSIZE  (256)

#define INPUT_ACTIVE 0
#define INPUT_IDLE 1

extern int dumb_term ;

char outbuf[OBUFSIZE] ;
int obufsize = 0 ;

char inbuf[IBUFSIZE] ;
int ibufsize = 0 ;
int icurrchar = 0 ;
int KEY_ESC_arg;

static int i_mode = INPUT_ACTIVE;

void
hit_alarm_clock()
{
    if (HAS_PERM(PERM_NOTIMEOUT))
        return;
    if(i_mode == INPUT_IDLE) {
        clear();
        fprintf(stderr,"Idle timeout exceeded! Booting...\n") ;
        kill(getpid(),SIGHUP) ;
    }
    i_mode = INPUT_IDLE ;
    alarm(IDLE_TIMEOUT) ;
}

void
init_alarm()
{
    signal(SIGALRM,hit_alarm_clock) ;
    alarm(IDLE_TIMEOUT) ;
}

void
oflush()
{
    if(obufsize)
      write(1,outbuf,obufsize) ;
    obufsize = 0 ;
}

void
output(s,len)
char    *s;
int     len;
{
    /* Invalid if len >= OBUFSIZE */

    if(obufsize+len > OBUFSIZE) {  /* doin a oflush */
        write(1,outbuf,obufsize) ;
        obufsize = 0 ;
    }
    memcpy(outbuf+obufsize, s, len) ;
    obufsize+=len ;
}

void
ochar(c)
int     c;
{
    if(obufsize > OBUFSIZE-1) {  /* doin a oflush */
        write(1,outbuf,obufsize) ;
        obufsize = 0 ;
    }
    outbuf[obufsize++] = c ;
}

int i_newfd  = 0 ;
struct timeval i_to, *i_top = NULL ;
int (*flushf)() = NULL ;

void
add_io(fd,timeout)
int fd ;
int timeout ;
{
    i_newfd = fd ;
    if(timeout) {
        i_to.tv_sec = timeout ;
        i_to.tv_usec = 0 ;
        i_top = &i_to ;
    } else i_top = NULL ;
}

void
add_flush(flushfunc)
int (*flushfunc)() ;
{
    flushf = flushfunc ;
}

int
num_in_buf()
{
/*---	Modified accoding to zhch's article	period	2000-11-21	---*/
/*    return icurrchar - ibufsize ;*/
    int n;
    if((n = icurrchar - ibufsize) < 0) n = 0;
    return n;
}

int
igetch()
{
  igetagain:
    if(ibufsize == icurrchar) {
        fd_set readfds ;
        struct timeval to ;
        int sr ;

        to.tv_sec = 0 ;
        to.tv_usec = 0 ;
        FD_ZERO(&readfds) ;
        FD_SET(0,&readfds) ;
        if(i_newfd)
          FD_SET(i_newfd,&readfds) ;
        if((sr = select(FD_SETSIZE,&readfds, NULL, NULL, &to)) <= 0) {
            if(flushf)
              (*flushf)() ;
            if(dumb_term)
              oflush() ;
            else
              refresh() ;
            FD_ZERO(&readfds) ;
            FD_SET(0,&readfds) ;
            if(i_newfd)
              FD_SET(i_newfd,&readfds) ;
            while((sr = select(FD_SETSIZE,&readfds, NULL, NULL, i_top)) <0) {
                if(errno == EINTR)
                  continue ;
                else {
                    perror("select") ;
                    fprintf(stderr,"abnormal select conditions\n") ;
                    return -1 ;
                }
            }
            if(sr == 0)
              return I_TIMEOUT ;
        }
        if(i_newfd && FD_ISSET(i_newfd,&readfds))
          return I_OTHERDATA ;
        while((ibufsize = read(0,inbuf,IBUFSIZE)) <= 0) {
            if(ibufsize == 0)
              longjmp(byebye,-1) ;
            if(ibufsize < 0 && errno != EINTR)
              longjmp(byebye,-1) ;
        }
        icurrchar = 0 ;
    }
    i_mode = INPUT_ACTIVE;
    switch(inbuf[icurrchar]) {
      case Ctrl('L'):
        redoscr() ;
        icurrchar++ ;
        goto igetagain ;
      default:
        break ;
    }
    return inbuf[icurrchar++] ;
}

int
igetkey()
{
    int  mode;
    int  ch, last;
    extern int RMSG;

    mode = last = 0;
    while( 1 ) {
        if((uinfo.mode==CHAT1||uinfo.mode==TALK||uinfo.mode==PAGE) && RMSG==YEA)
        {
                char a;

                read(0,&a,1);
                ch=(int) a;
        }
        else
                ch = igetch();
        if( mode == 0 ) {
            if( ch == KEY_ESC ) mode = 1;
            else  return ch;    /* Normal Key */
        } else if( mode == 1 ) {  /* Escape sequence */
            if( ch == '[' || ch == 'O' )  mode = 2;
            else if( ch == '1' || ch == '4' )  mode = 3;
            else { KEY_ESC_arg=ch; return KEY_ESC; }
        } else if( mode == 2 ) {  /* Cursor key */
            if( ch >= 'A' && ch <= 'D' )
                return KEY_UP + (ch - 'A');
            else if( ch >= '1' && ch <= '6' )  mode = 3;
            else  return ch;
        } else if( mode == 3 ) {  /* Ins Del Home End PgUp PgDn */
            if( ch == '~' )
                return KEY_HOME + (last - '1');
            else  return ch;
        }
        last = ch;
    }
}

void 
top_show( prompt )
char    *prompt;
{
    if (editansi) { prints(ANSI_RESET); refresh(); }
    move(0,0) ;
    clrtoeol() ;
    standout() ;
    prints("%s", prompt) ;
    standend() ;
}

int
ask( prompt )
char *prompt;
{
    int         ch;

    top_show( prompt );
    ch = igetkey() ;
    move(0,0) ;
    clrtoeol() ;
    return( ch );
}

int
getdata(line, col, prompt, buf, len, echo,nouse,clearlabel)
int     line,   col,    len,    echo, clearlabel;
int     nouse;
char    *prompt,        *buf;
{
        int     ch,clen = 0,curr = 0,x,y;
        char    tmp[STRLEN];
        extern unsigned char scr_cols ;
        extern int RMSG;

        if (clearlabel==YEA)
        {
                memset(buf,0, sizeof(buf));
        }
        move(line, col);
        if (prompt)
                prints("%s", prompt);
        y = line;
        col+= (prompt == NULL) ? 0 : strlen(prompt);
        x = col;
        clen = strlen(buf);
        curr = (clen >= len) ? len-1: clen;
        buf[curr]='\0';
        prints("%s", buf);

        if (dumb_term||echo==NA)
        {
                while ((ch = igetkey()) != '\r')
                {
                        if (ch == '\n')
                                break;
                        if (ch == '\177' || ch == Ctrl('H'))
                        {
                                if (clen == 0)
                                {
                                        continue;
                                }
                                clen--;
                                ochar(Ctrl('H'));
                                ochar(' ');
                                ochar(Ctrl('H'));
                                move(line, col + clen); /* Leeward 98.02.23 */
                                refresh();
                                continue;
                        }
                        if (!isprint2(ch))
                        {
                                continue;
                        }
                        if (clen >= len-1)
                        {
                                continue;
                        }
                        buf[clen++] = ch;
                        move(line, col + clen); /* Leeward 98.02.23 */
                        if (echo)
                                ochar(ch);
                        else
                                ochar('*');
                }
                buf[clen] = '\0';
                prints("\n");
                oflush();
                return clen;
        }
        clrtoeol();
        while (1)
        {
                if((uinfo.mode==CHAT1||uinfo.mode==TALK )&& RMSG==YEA)
                {
                        refresh();
                }
                ch = igetkey();

                if (YEA == RMSG && (KEY_UP == ch || KEY_DOWN == ch))
                  return - ch; /* Leeward 98.07.30 supporting msgX */

                if (ch == '\n'||ch == '\r')
                        break;
                if (ch == '\177' || ch == Ctrl('H'))
                {
                        if (curr == 0)
                        {
                                continue;
                        }
                        strcpy(tmp, &buf[curr]);
                        buf[--curr] = '\0';
                        (void)strcat(buf, tmp);
                        clen--;
                        move(y, x);
                        prints("%s", buf);
                        clrtoeol();
                        move(y, x + curr);
                        continue;
                }
                if (ch == KEY_DEL)
                {
                        if (curr >= clen)
                        {
                                curr = clen;
                                continue;
                        }
                        strcpy(tmp, &buf[curr+1]);
                        buf[curr] = '\0';
                        (void)strcat(buf, tmp);
                        clen--;
                        move(y, x);
                        prints("%s", buf);
                        clrtoeol();
                        move(y, x + curr);
                        continue;
                }
                if (ch == KEY_LEFT)
                {
                        if (curr == 0)
                        {
                                continue;
                        }
                        curr--;
                        move(y, x + curr);
                        continue;
                }
                if(ch==Ctrl('E') || ch==KEY_END)
                {
                        curr = clen;
                        move(y, x + curr);
                        continue;
                }
                if(ch==Ctrl('A') || ch== KEY_HOME)
                {
                        curr = 0;
                        move(y, x + curr);
                        continue;
                }
                if (ch == KEY_RIGHT)
                {
                        if (curr >= clen)
                        {
                                curr = clen;
                                continue;
                        }
                        curr++;
                        move(y, x + curr);
                        continue;
                }
                if (!isprint2(ch))
                {
                        continue;
                }

                if (x+clen >= scr_cols || clen >= len-1)
                {
                        continue;
                }

                if (!buf[curr])
                {
                        buf[curr + 1] = '\0';
                        buf[curr] = ch;
                }
                else
                {
                        strncpy(tmp, &buf[curr], len);
                        buf[curr] = ch;
                        buf[curr + 1] = '\0';
                        strncat(buf, tmp, len - curr);
                }
                curr++;
                clen++;
                move(y, x);
                prints("%s", buf);
                move(y, x + curr);
        }
        buf[clen] = '\0';
        if(echo)
        {
                move(y, x);
                prints("%s", buf);
        }
        prints("\n");
        refresh();
        return clen;
}
