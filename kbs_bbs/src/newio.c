/*
   Pirate Bulletin Board System
   Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
   Eagles Bulletin Board System
   Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                       Guy Vega, gtvega@seabass.st.usm.edu
                       Dominic Tynes, dbtynes@seabass.st.usm.edu
   Copyright (C) 1999, Lin Zhou, kcn@cic.tsinghua.edu.cn

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
#include <arpa/telnet.h>

#ifdef AIX
#include <sys/select.h>
#endif

#define OBUFSIZE  (4096)
#define IBUFSIZE  (256)

#define INPUT_ACTIVE 0
#define INPUT_IDLE 1

extern temp_numposts;

void abort_bbs();

unsigned char outbuffer[OBUFSIZE+1] ;
unsigned char* outbuf=outbuffer+1;
int obufsize = 0 ;

unsigned char inbuffer[IBUFSIZE+1] ;
unsigned char* inbuf=inbuffer+1 ;
int ibufsize = 0 ;

int icurrchar = 0 ;
int KEY_ESC_arg;

int idle_count=0;

static time_t old;
static time_t lasttime = 0;

extern int convcode;
extern char* big2gb(char*,int*,int);
extern char* gb2big(char*,int*,int);

void
oflush()
{
    if(obufsize) {
        if (convcode) {
            char* out;
            out=gb2big(outbuf,&obufsize,0);
#ifdef SSHBBS
            if (ssh_write(0,out,obufsize)<0)
#else
            if (write(0,out,obufsize)<0)
#endif
                abort_bbs() ;
        }
        else
#ifdef SSHBBS
            if (ssh_write(0,outbuf,obufsize)<0)
#else
            if (write(0,outbuf,obufsize)<0)
#endif
                abort_bbs() ;
    }
    obufsize = 0 ;
}

void
ochar(c)
int     c;
{
    if(obufsize > OBUFSIZE-1) {  /* doin a oflush */
        oflush();
    }
    outbuf[obufsize++] = c ;
    /* need to change IAC to IAC IAC */
    if (c==IAC) {
        if(obufsize > OBUFSIZE-1) {  /* doin a oflush */
            oflush();
        }
        outbuf[obufsize++] = c ;
    }
}

void
output(s,len)
char    *s;
int     len;
{
    /* need to change IAC to IAC IAC
    if(obufsize+len > OBUFSIZE) {
#ifdef SSHBBS
        ssh_write(0,outbuf,obufsize) ;
#else
        write(0,outbuf,obufsize) ;
#endif
        obufsize = 0 ;
    }
    memcpy(outbuf+obufsize, s, len) ;
    obufsize+=len ; */
    int i;
    for (i=0;i<len;i++)
        ochar(s[i]);
}


int i_newfd  = 0 ;
int (*flushf)() = NULL ;

static int i_timeout=0;
static time_t i_begintimeout;
static void (*i_timeout_func)(void*);
static struct timeval i_to, *i_top = NULL ;
static void *timeout_data;

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

void set_alarm(int set_timeout,void (*timeout_func)(void*),void* data)
{
	i_timeout=set_timeout;
	i_begintimeout=time(0);
	i_timeout_func=timeout_func;
	timeout_data=data;
}

int
num_in_buf()
{
    /*---   Modified according to zhch@dii	period	2000-11-21	---*/
    /*    return icurrchar - ibufsize ;*/
    int n;
    if((n = icurrchar - ibufsize) < 0) n = 0;
    return n;
}

int telnet_state=0;
char lastch;

int telnet_machine(ch)
unsigned char ch;
{
    switch (telnet_state) {
    case 255:	/* after the first IAC */
        switch(ch) {
        case DO:
        case DONT:
        case WILL:
        case WONT:
            telnet_state=1;
            break;
        case SB:			/* loop forever looking for the SE */
            telnet_state=2;
            break;
        case IAC:
            return IAC;
        default:
            telnet_state=0;		/* other telnet command*/
        }
        break;
    case 1: /* Get the DO,DONT,WILL,WONT */
        telnet_state=0;  /* the ch is the telnet option */
        break;
    case 2: /* the telnet suboption */
        if (ch==IAC)
            telnet_state=3;	/* wait for SE */
        break;
    case 3:	/* wait for se */
        if (ch==SE)
            telnet_state=0;
        else telnet_state=2;
        break;
    }
    return 0;
}

int filter_telnet(unsigned char* s,int* len)
{
    unsigned char* p1,*p2,*pend;
    int newlen;
    newlen=0;
    for (p1=s,p2=s,pend=s+(*len);p1!=pend;p1++) {
        if (telnet_state) {
            int ch=0;
            ch=telnet_machine(*p1);
            if (ch==IAC) { /* Á½¸öIAC */
                *p2=IAC;
                p2++;
                newlen++;
            }
        } else {
            if (*p1==IAC)
                telnet_state=255;
            else {
                *p2=*p1;
                p2++;
                newlen++;
            }
        }
    }
    return (*len=newlen);
}

int
igetch()
{
    time_t now;
    char c;
    int hasaddio=1;
    extern int RMSG;


    if((uinfo.mode==CHAT1||uinfo.mode==TALK||uinfo.mode==PAGE) && RMSG==YEA)
        hasaddio=0;

igetagain:
    if(ibufsize == icurrchar) {
        fd_set readfds ,xds;
        struct timeval to ;
        int sr ,hifd;

        to.tv_sec = 0 ;
        to.tv_usec = 0 ;
        hifd=1;
        FD_ZERO(&readfds) ;
        FD_SET(0,&readfds) ;
        if(hasaddio&&(i_newfd)) {
            FD_SET(i_newfd,&readfds) ;
            if(hifd<=i_newfd)hifd=i_newfd+1;
        }
        sr = select(hifd,&readfds,NULL,NULL,&to);
        if( sr<0 && errno!=EINTR ) abort_bbs();
		if( sr==0 ){
            if(flushf) (*flushf)() ;
            refresh() ;

            while(1){
            	int alarm_timeout;
                hifd=1;
                FD_ZERO(&xds);
                FD_SET(0,&xds);
                FD_ZERO(&readfds) ;
                FD_SET(0,&readfds) ;
                if(hasaddio&&(i_newfd)) {
                    FD_SET(i_newfd,&readfds) ;
                    if(hifd<=i_newfd)hifd=i_newfd+1;
                }
            	alarm_timeout=0;
                if(i_top) 
                	to=*i_top;
                else {
                	while (i_timeout!=0) {
                		to.tv_sec=i_timeout-(time(0)-i_begintimeout);
                		if (to.tv_sec<=0) {
                			i_timeout=0;
                			(*i_timeout_func)(timeout_data);
                			continue;
                		};
                		alarm_timeout=1;
                		break;
                	};
                	if (!alarm_timeout)
                		to.tv_sec=IDLE_TIMEOUT;
                }
                sr = select(hifd,&readfds, NULL,&xds, &to);
                if (sr==0&&alarm_timeout) {
                	i_timeout=0;
                	(*i_timeout_func)(timeout_data);
                	continue;
                }
                if( sr>=0 )break;
                if(errno == EINTR) continue ;
                else abort_bbs();

            }
            if ((sr == 0)&&(!i_top)) abort_bbs();
            if(sr == 0) return I_TIMEOUT ;
            if(FD_ISSET(0,&xds)) abort_bbs();
        }
        if(hasaddio&&(i_newfd && FD_ISSET(i_newfd,&readfds)))
            return I_OTHERDATA ;
#ifdef SSHBBS
        while((ibufsize = ssh_read(0,inbuffer+1,IBUFSIZE)) <= 0) {
#else
        while((ibufsize = read(0,inbuffer+1,IBUFSIZE)) <= 0) {
#endif
            if(ibufsize == 0)
                longjmp(byebye,-1) ;
            if(ibufsize < 0 && errno != EINTR)
                longjmp(byebye,-1) ;
        }
        if (!filter_telnet(inbuffer+1,&ibufsize))
        {
            icurrchar = 0;
            ibufsize = 0;
            goto igetagain;
        }

        /* add by KCN for GB/BIG5 encode */
        if (convcode) {
            inbuf=big2gb(inbuffer+1,&ibufsize,0);
            if (ibufsize==0) {
                icurrchar = 0 ;
                goto igetagain;
            }
        }
        else inbuf=inbuffer+1;
        /* end */
        icurrchar = 0 ;
        if (ibufsize>IBUFSIZE)
        {
            ibufsize=0;
            goto igetagain;
        }
    }

    if (icurrchar>=ibufsize) {
        ibufsize=icurrchar;
        goto igetagain;
    }
    if (((inbuf[icurrchar]=='\n')&&(lastch=='\r'))
            ||((inbuf[icurrchar]=='\r')&&(lastch=='\n')))
    {
        lastch=0;
        goto igetagain;
    }

    else if (icurrchar!=ibufsize) {
        if (((inbuf[icurrchar]=='\n')&&(inbuf[icurrchar+1]=='\r'))
                ||((inbuf[icurrchar]=='\r')&&(inbuf[icurrchar+1]=='\n'))) {
            icurrchar++;
            lastch=0;
        }
    }
    else     lastch=inbuf[icurrchar];

    idle_count=0;
    c=inbuf[icurrchar];
    switch(c) {
    case Ctrl('L'):
                    redoscr() ;
        icurrchar++ ;
        goto igetagain ;
    default:
        break ;
    }
    icurrchar++;
    while((icurrchar!=ibufsize)&&(inbuf[icurrchar]==0)) icurrchar++;
    now=time(0);
    /*---	Ctrl-T disabled as anti-idle key	period	2000-12-05	---*/
    if(Ctrl('T') != c)
        uinfo.freshtime=now;
    /* add by KCN , decrease temp_numposts*/
	if (lasttime + 60*60*8 < now) {
        lasttime = now;
        if (temp_numposts>0)
            temp_numposts--;
    }
    if (now-old>60) {
    	UPDATE_UTMP(freshtime,uinfo);
    	old=now;
    }
    return c;
}

int
igetkey()
{
    int  mode;
    int  ch, last;
    extern int RMSG;

    mode = last = 0;
    while( 1 ) {
        /*        if((uinfo.mode==CHAT1||uinfo.mode==TALK||uinfo.mode==PAGE) && RMSG==YEA)
                {
                        char a;
#ifdef SSHBBS
                        ssh_read(0,&a,1);
#else
                        read(0,&a,1);
#endif
                        ch=(int) a;
                }
        	else 
        */
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

    if (!scrint||(echo==NA))
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
/*
removed by wwj, just use oflush , 2001/5/8 

#ifndef DEBUG
--- strange code, maybe should call move() after refresh() 
                move(line, col + clen);  Leeward 98.02.23 
                refresh();  

#endif
*/
                oflush();
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
            /* move(line, col + clen);  Leeward 98.02.23  -- removed by wwj 2001/5/8*/
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

