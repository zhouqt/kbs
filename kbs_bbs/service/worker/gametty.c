#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>

#include"gametty.h"

#ifdef ENABLE_DL

void _prints(const char * s)
{
    output(s,strlen(s));
}

void println(const char * s)
{
    _prints(s);
    oflush();
}

void outs(char * msg)
{
	println(msg);
	oflush();
}

void _move(int y1, int x1)
{
    char tmp[200];
    sprintf(tmp, "\033[%d;%dH", y1+3, x1+1);
    _prints(tmp);
}

void color(int c)
{
    static int lastc = -1;
    char tmp[200];
    if(c == lastc)return;
    sprintf(tmp, "\033[%d;%dm", c/8, c%8+30);
    _prints(tmp);
}

void _clear (void)
{
    char tmp[200];
    sprintf(tmp,"\033[H\033[J");
    _prints(tmp);
}

void _cleartoeol(void)
{
	_prints("\033[K");
}

void _mvaddstr(int x, int y, char * ptr)
{
    _move(x, y); _prints(ptr); _move(x, y); 
}
void initialize(void)
{
}

void finalize(void)
{
}
int getch(void)
{
    return igetkey();
}
#else

#ifndef _USE_NCURSE_
#include<termios.h>

char buf[8192];
int bufip=0;

void oflush (void)
{
    write(1,buf,bufip);
    bufip=0;
}

void prints(const char * s)
{
    sprintf(buf+bufip, "%s", s);
    bufip+=strlen(s);
}

void println(const char * s)
{
    prints(s);
    oflush();
}

void outs(char * msg)
{
	println(msg);
	oflush();
}

void _move(int y1, int x1)
{
    char tmp[200];
    sprintf(tmp, "\033[%d;%dH", y1+1, x1+1);
    prints(tmp);
}

void color(int c)
{
    static int lastc = -1;
    char tmp[200];
    if(c == lastc)return;
    sprintf(tmp, "\033[%d;%dm", c/8, c%8+30);
    prints(tmp);
}

void _clear (void)
{
    char tmp[200];
    sprintf(tmp,"\033[H\033[J");
    prints(tmp);
}

void _cleartoeol(void)
{
	prints("\033[K");
}

void _mvaddstr(int x, int y, char * ptr)
{
    _move(x, y); prints(ptr); _move(x, y); oflush();
}

struct termios tty_state, tty_new ;

int
get_tty (void)
{
    if(tcgetattr(1,&tty_state) < 0)
        return 0;
    return 1 ;
}

void init_tty (void)
{
    long vdisable;

    memcpy( &tty_new, &tty_state, sizeof(tty_new)) ;
    tty_new.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ISIG);
    tty_new.c_cflag &= ~CSIZE;
    tty_new.c_cflag |= CS8;
    tty_new.c_cc[ VMIN ] = 1;
    tty_new.c_cc[ VTIME ] = 0;
    if ((vdisable = fpathconf(STDIN_FILENO, _PC_VDISABLE)) >= 0) {
        tty_new.c_cc[VSTART] = vdisable;
        tty_new.c_cc[VSTOP]  = vdisable;
        tty_new.c_cc[VLNEXT] = vdisable;
    }
    tcsetattr(1, TCSANOW, &tty_new);
}

void reset_tty (void)
{
    tcsetattr(1, TCSANOW, &tty_state) ;
}

void initialize(void)
{
    get_tty();
    init_tty();
}

void finalize(void)
{
    _move(0, 0);
    reset_tty();
    color(7);
    fflush(stdout);
    oflush();
}

int kbhit(void)
{
    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 50000;
    return select(1, &rfds, NULL, NULL, &tv);
}

int getch(void)
{
    int c,d,e;
    if(kbhit()) {
        c=getchar();
        if(c!=27)return c;  /*---   ESCAPE = PAUSE  ---*/
        if('[' == (d=getchar())) {
            e=getchar();
            if(e=='A') return KEY_UP;
            if(e=='B') return KEY_DOWN;
            if(e=='C') return KEY_RIGHT;
            if(e=='D') return KEY_LEFT;
        }
        return 0;
    }
    else
        return 0;
}

#else

#include<curses.h>

void initialize(void)
{
	initscr();
	cbreak();
	nonl();
	noecho();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	raw();
	meta(stdscr, TRUE);
	attrset(A_NORMAL);
	refresh();
}

void finalize(void)
{
	refresh();
	echo();
	endwin();
}

#endif
#endif
