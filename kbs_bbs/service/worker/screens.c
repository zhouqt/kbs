#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/time.h>
#include<unistd.h>

#include"gamedefs.h"
#include"gametty.h"
#include"screens.h"

#define RECTFILL1 "\x1b[0;47;30m┬"
#define RECTFILL2 "\x1b[0;47;30m┴"
#define RECTBOX  "\x1b[1;46;33m▓"
#define RECTBOXTARGET  "\x1b[1;46;31m▓"
#define TARGET   "\x1b[1;46;32m·"
#define WORKER   "\x1b[1;46;31m♀"

void _mvaddnstr(int y, int x, char * ptr, int len)
{
    move(y,x); prints(ptr); move(y,x);
}

void target(int x, int y)
{
    _mvaddnstr(y, x, TARGET, 2);
}

void worker(int x, int y)
{
    _mvaddnstr(y, x, WORKER, 2);
}

void rectwall(int x, int y)
{
    if ((x/2+y)&1)
        _mvaddnstr(y, x, RECTFILL1, 2);
    else
        _mvaddnstr(y, x, RECTFILL2, 2);
}

void rectbox(int x, int y)
{
    _mvaddnstr(y, x, RECTBOX, 2);
}

void rectboxtarget(int x, int y)
{
    _mvaddnstr(y, x, RECTBOXTARGET, 2);
}

void background(int x, int y)
{
    _mvaddnstr(y, x, "\x1b[1;46m  ", 2);
}


#if 0
void gcharbox(int x, int y, int wid, int hei)
{
    static char buf[256];
    int i, j;

    if (x < 0 || x+wid >= 80) { x = 0; wid=80; }
    if (y < 0 || y+hei > 24) { x = 0; hei=24; }
    if (wid%2 != 0) wid--;

    for (j=0;j<wid;j++) {
        if (0 == j%2)
            sprintf(buf+j, "%s", RECTFILL);
    }
    buf[wid] = 0;

    for (i=y;i<y+hei;i++) {
        move(i, x);
        addstr(buf);
    }
}

void gcharrect(int x, int y, int wid, int hei)
{
    static char buf[256];
    int i, j;

    if (x < 0 || x+wid >= 80) { x = 0; wid=80; }
    if (y < 0 || y+hei > 24) { x = 0; hei=24; }
    if (wid%2 != 0) wid--;

    for (j=0;j<wid;j++) {
        if (0 == j%2)
            sprintf(buf+j, "%s", RECTFILL);
    }
    buf[wid] = 0;
    move(y, x);
    addstr(buf);
    move(y+hei-1, x);
    addstr(buf);

    for (j=2;j<wid-2;j++) buf[j] = ' ';
    for (i=y+1;i<y+hei-1;i++) {
        move(i, x);
        addstr(buf);
    }
}

void FoolWorker(void)
{
    move(10, 5);
    addstr("这么久了还没成功！ *(#&$)@(#*&$@(");
    sleep(1);
}
#endif
