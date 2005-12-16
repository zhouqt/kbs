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
#include "read.h"
void show_help(fname)
    char *fname;
{
    /*---	Modified by period	2000-10-26	according to ylsdd's warning	---*/
    static short int cnt;

    if (cnt >= 2) {
        return;
    }
    ++cnt;
    clear();
    ansimore(fname, true);
    clear();
    --cnt;
}

/*void
standhelp( mesg )
char    *mesg;
{
    prints("\033[1;32;44m");
    prints( mesg ) ;
    prints("\033[m");
}*/

int mainreadhelp(struct _select_def* conf,struct fileheader *fileinfo,void* extraarg)
{
    show_help("help/mainreadhelp");
    return FULLUPDATE;
}

