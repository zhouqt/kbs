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

/* Lots o' modes! */

#define IDLE            0       /* Modes a user can be in */
#define ULDL            1       /* see mode in struct user_info in bbs.h */
#define TALK            2
#define NEW             3
#define CHAT1           4
#define READNEW         5
#define POSTING         6
#define MAIL            7
#define CHAT2           8
#define CHAT4           9
#define CHAT3           10
#define LAUSERS         11
#define LUSERS          12
#define SMAIL           13
#define RMAIL           14
#define MMENU           15
#define TMENU           16
#define XMENU           17
#define READING         18
#define PAGE            19
#define ADMIN           20
#define READBRD         21
#define SELECT          22
#define LOGIN           23
#define MONITOR         24
#define EDITWELC        25
#define ZAP             26
#define EDITUFILE       27
#define EDITSFILE       28
#define QUERY           29
#define CNTBRDS         30
#define VOTING          31
#define VISIT           32
#define IRCCHAT         33
#define BBSNET          34
#define FOURM           35
#define CSIE_GOPHER     36
#define CSIE_TIN        37
#define CSIE_ANNOUNCE   38
#define FRIEND          39
#define YANKING         40
#define EXCE_MJ         41
#define GMENU           42
#define EXCE_BIG2       43
#define EXCE_CHESS      44
#define NOTEPAD         45
#define MSGING          46
#define USERDEF         47
#define EDIT            48
#define OFFLINE         49
#define EDITANN         50
//#define WWW             51
//#define WEBEXPLORE	WWW
#define WEBEXPLORE	51
#define CCUGOPHER       52
#define LOOKMSGS        53
#define WFRIEND         54
#define LOCKSCREEN      55
#define IMAIL		56
#define EDITSIG         57
#define EDITPLAN        58
#define GIVEUPNET	59
#define SERVICES	60
#define FRIENDTEST  61
#define	CHICKEN		62
#define QUIZ 63
#define KILLER 64
#define CALENDAR 65
#define CALENEDIT 66
#define DICT 67
#define CALC 68
#define SETACL 69
#define EDITOR 70
#define HELP 71
#define POSTTMPL 72
#define TETRIS 73
#define WINMINE 74
#define PC 75
#define QUERYBOARD 76
#define BLOG 77
#define POSTCROSS 78

