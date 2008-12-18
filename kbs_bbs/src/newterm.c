/***********************************************
 term.c for BBS without tty use
    Copyright (C) 1999 KCN,Lin Zhou,kcn@cic.tsinghua.edu.cn

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
/*#include <varargs.h>*/
#include <arpa/telnet.h>
int t_lines = 24;
int t_columns = 80;
int automargins = 1;
void term_init()
{
    extern int obufsize, ibufsize, icurrchar;

    obufsize = 0;
    ibufsize = 0;
    icurrchar = 0;
}
void do_move(int destcol, int destline, void (*outc)(char))
{
    char buf[30];
    char *p;

    snprintf(buf,29, "\033[%d;%dH", destline + 1, destcol + 1);
    for (p = buf; *p != 0; p++)
        (*outc)(*p);
}
