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

extern  int  nettyNN;

int     domenu();
/* Add By Excellent */
/* int     t_announce(), t_tin(), t_gopher(), x_excemj(),x_excebig2(),x_excechess();*/
int     Announce();
int Boards();
int EGroup();
int Info();
int Goodbye();
int Help();
int New();
int Post();
int Read();
int Select();
int Users();
int Welcome();
int t_www();
int setcalltime();
int show_allmsgs();
int Conditions();
int x_cloak();
int t_users();
int x_info();
int x_fillform();
int x_vote();
int ShowWeather();	/* 2001.6.12 */
int x_results();
int ent_bnet();
int a_edits();
int x_edits();
int x_date();
int shownotepad();
int x_userdefine();
int confirm_delete_id();	/* Added by Bigman 2001.7.14 */
/*Add by SmallPig*/
int     m_new(), m_read(), m_send(), g_send();
int     ov_send(), m_internet(),s_msg(),mailall(),suicide();


int     t_users(), t_friends(), t_rusers(), t_list(), t_monitor();
int     wait_friend();
int     t_query(), t_talk(), t_pager(), t_override(), x_cloak();
#ifdef IRC
int     t_irc()
#endif
int kick_user();
/* add by KCN */
/* Modified by sanshao, sb KCN */
int	ent_chat1(), ent_chat2();
int     switch_code(); /* add by KCN */


int     x_level(), XCheckLevel(), m_info(), d_user(), m_register(), m_newbrd(),setsystempasswd();
int     d_board(), m_editbrd(), m_mclean(), m_trace(), m_vote(),wall();
/* inserted by cityhunter */
int	query_bm();
int	p_level();
/* end of insertion */
int     lock_scr(); /* Leeward 98.02.22 */
int     FavBoard(); /* period  2000-09-11 */
int 	searchtrace(); 	/* stephen 2000.12.15 */
int	badlist();	/* Bigman 2000.12.26 */
int	m_stoplogin();
int	inn_start();	/* czz 2002.01.15 */
int	inn_reload();	/* czz 2002.01.15 */
int	inn_stop();	/* czz 2002.01.15 */

/*Add By Excellent */
struct scommandlist {
    char        *name;
    int         (*fptr)();
} ;
static const struct scommandlist sysconf_cmdlist[] = {
    "domenu",       domenu,
    "EGroups",      EGroup,
    "BoardsAll",    Boards,
    "BoardsNew",    New,
    "LeaveBBS",     Goodbye,
    "Announce",     Announce,
    /*        "ExecTin",      t_tin,
            "ExecGopher",   t_gopher,*/
    "SelectBoard",  Select,
    "ReadBoard",    Read,
    "PostArticle",  Post,
    "SetHelp",      Help,
    "SetAlarm",     setcalltime,
    "MailAll",     mailall,
    /*"ExecMJ",       x_excemj,
    "ExecBIG2",     x_excebig2,
    "ExecCHESS",    x_excechess,
    "WWW",          t_www,        */
    "OffLine",      suicide,
    "ReadNewMail",  m_new,
    "ReadMail",     m_read,
    "SendMail",     m_send,
    "GroupSend",    g_send,
    "OverrideSend", ov_send,
    "SendNetMail",  m_internet,
    "UserDefine",   x_userdefine,
    "ShowFriends",  t_friends,
    "ShowLogins",   t_users,
    "QueryUser",    t_query,
    "WaitFriend",   wait_friend,
    "Talk",         t_talk,
    "SetPager",     t_pager,
    "SetCloak",     x_cloak,
    "SendMsg",      s_msg,
    "ShowMsg",      show_allmsgs,
    "SetFriends",   t_override,
    "EnterChat",    ent_chat1,
    "EnterChat2",    ent_chat2,
#ifdef IRC
    "ExecIrc",      t_irc,
#endif
    "ListLogins",   t_list,
    "Monitor",      t_monitor,
    "RealLogins",   t_rusers,
    "FillForm",     x_fillform,
    "SetInfo",      x_info,
    "EditUFiles",   x_edits,
    "ShowLicense",  Conditions,
    "ShowVersion",  Info,
    "Notepad",      shownotepad,
    "ShowDate",     x_date,
    "DoVote",       x_vote,
    "VoteResult",   x_results,
    "ExecBBSNet",   ent_bnet,
    "ShowWelcome",  Welcome,
    "ReadWeather",	ShowWeather, /*Bigman 2001.6.12 */
    "SpecialUser",  Users,
    /*"ExecViewer",   t_announce,*/
    "LockScreen",   lock_scr, /* Leeward 98.02.22 */
    "ConvCode",     switch_code, /* KCN 99.09.03 */
    "FavBoard",	FavBoard,	/* added period 2000-09-11 */
    "BadList",	badlist,	/* added Bigman 2000.12.26 */
    "CheckForm",    m_register,
    "ModifyInfo",   m_info,
    "ModifyLevel",  x_level,
    /*added by cityhunter */
    "ModifypLevel", p_level,
    "QueryBUser",   query_bm,
    /* end of addin */
    "XCheckLevel",  XCheckLevel,
    "KickUser",     kick_user,
    "DelUser",      d_user,
    "OpenVote",     m_vote,
    "NewBoard",     m_newbrd,
    "EditBoard",    m_editbrd,
    "DelBoard",     d_board,
    "SetTrace",     m_trace,
    "CleanMail",    m_mclean,
    "EditSFiles",   a_edits,
    "Announceall",  wall,
    "Setsyspass",   setsystempasswd,
    "SearchTrace",	searchtrace,	/*stephen 2000.12.15*/
    "StopLogin",	m_stoplogin,	/*stephen 2000.12.15*/
    "ConfirmDelete",	confirm_delete_id, /*Bigman 2001.7.14 */
    "InnStart",		inn_start,	/* czz 2002.01.15 */
    "InnReload",	inn_reload,	/* czz 2002.01.15 */
    "InnStop",		inn_stop,	/* czz 2002.01.15 */
    NULL,           NULL
};

void
encodestr( str )
register char *str;
{
    register char       ch, *buf;
    int                 n;

    buf = str;
    while( (ch = *str++) != '\0' ) {
        if( *str == ch && str[1] == ch && str[2] == ch ) {
            n = 4;
            str += 3;
            while( *str == ch && n < 100 ) {
                str++;
                n++;
            }
            *buf++ = '\01';
            *buf++ = ch;
            *buf++ = n;
        } else
            *buf++ = ch;
    }
    *buf = '\0';
}

void
decodestr( str )
register char *str;
{
    register char       ch;
    int         n;

    while( (ch = *str++) != '\0' )
        if( ch != '\01' )
            outc( ch );
        else if( *str != '\0' && str[1] != '\0' ) {
            ch = *str++;
            n =  *str++;
            while( --n >= 0 )
                outc( ch );
        }
}

typedef int (*CMD_FUNC)();
CMD_FUNC cmdlist_funcptr(char *func_name)
{
    int         n = 0;
    char        *str;

    while( (str = sysconf_cmdlist[n].name) != NULL ) {
        if( strcmp( func_name, str ) == 0 )
            return sysconf_cmdlist[n].fptr;
        n++;
    }
    return NULL;
}

extern int sysconf_menu;
struct _menupos {
	int line,col;
};
extern struct smenuitem* menuitem;
static struct _menupos *menupos=NULL;

static int domenu_screen(struct smenuitem *dopm,char*     cmdprompt)
{
    char        *str;
    int         help, line, col, num;
    struct smenuitem pm;

    int n;

    if(!DEFINE(currentuser,DEF_NORMALSCR))
        clear();
    help = (currentuser->flags[0] & CURSOR_FLAG);
    line = 3;
    col  = 0;
    num  = 0;
    while( 1 ) {
    	pm=*dopm;
    	n=((char*)dopm-(char*)menuitem)/sizeof(struct smenuitem);
    	/* 这个获得n的写法太丑陋了，但是.....
    	  先不管了KCN*/
    	  
        switch( pm.level ) {
        case -1:
            return( num );
        case -2:
            if( strcmp( sysconf_relocate(pm.name), "title" ) == 0 ) {
                docmdtitle( sysconf_relocate(pm.desc), cmdprompt );
            } else if( strcmp( sysconf_relocate(pm.name), "screen" ) == 0 ) {
                if (DEFINE(currentuser,DEF_SHOWSCREEN))
                {
                    if( help && (str = sysconf_str( sysconf_relocate(pm.desc) )) != NULL ) {
                        move( menupos[n].line, menupos[n].col );
                        decodestr( str );
                    }
                }
                else
                {
                    if( help && (str = sysconf_str( "S_BLANK" )) != NULL ) {
                        move( menupos[n].line, menupos[n].col );
                        decodestr( str );
                    }
                }
            }
            break;
        default:
            if( menupos[n].line >= 0 && HAS_PERM(currentuser, pm.level ) ) {
                if( menupos[n].line == 0 ) {
                    menupos[n].line = line;  menupos[n].col = col;
                } else {
                    line = menupos[n].line;  col = menupos[n].col;
                }
                if( help ) {
                    move( line, col );
                    prints( "  %s", sysconf_relocate(pm.desc) );
                }
                line++;
            } else {
                if( menupos[n].line > 0 ) {
                    line = menupos[n].line;  col = menupos[n].col;
                }
                menupos[n].line = -1;
            }
        }
        num++;
        dopm++;
    }
}

static void copymenupos()
{
	int n;
    for( n = 0; n < sysconf_menu; n++ ) {
        menupos[n].line=menuitem[n].line;
        menupos[n].col=menuitem[n].col;
    }
}

int domenu( menu_name )
char    *menu_name;
{
    extern int          refscreen;
    struct smenuitem    *pm;
    char        *cmdprompt = "目前选择：";
    int         size, now;
    int         cmdplen, cmd, i,base;

    /*
    if( sysconf_menu <= 0 ) {
        return -1;
    }
    */

	/* disable it,因为不知道为何会core dump
	if (check_sysconf()) {
		free(menupos);
		menupos=NULL;
	}
	*/
    if (menupos==NULL) {
    	menupos=(struct _menupos*)malloc(sizeof(struct _menupos)*sysconf_menu);
    	copymenupos();
    }
    pm = sysconf_getmenu(menu_name);
    size = domenu_screen( pm, cmdprompt );
    base=((char*)pm-(char*)menuitem)/sizeof(struct smenuitem);
    	/* 这个获得base的写法太丑陋了，但是.....
    	  先不管了KCN*/
    cmdplen = strlen( cmdprompt );
    now = 0;
    if( strcmp( menu_name, "TOPMENU" ) == 0 && chkmail() ) {
        for( i = 0; i < size; i++ )
            if( menupos[base+i].line > 0 && (*sysconf_relocate(pm[i].name)) == 'M' )
                now = i;

    }
    modify_user_mode( MMENU );
    /* added by netty  */
if (nettyNN ==1) { R_monitor(NULL);}
    while( 1 ) {
    	int (*fptr)();
        printacbar();
        while( pm[now].level < 0 || !HAS_PERM(currentuser, pm[now].level ) ) {
            now++;
            if( now >= size )  now = 0;
        }
        if( currentuser->flags[0] & CURSOR_FLAG ) {
            move( menupos[base+now].line, menupos[base+now].col );
            prints( "◆" );
        }
        move(1,cmdplen);
        clrtoeol();
        prints("[");
        standout();
        prints( "%-12s", sysconf_relocate(pm[now].name) );
        standend();
        prints("]");
        clrtoeol();
        cmd = egetch();
        if( currentuser->flags[0] & CURSOR_FLAG ) {
            move( menupos[base+now].line, menupos[base+now].col );
            prints( "  " );
        }
        switch( cmd ) {
        case Ctrl('Z'): r_lastmsg(); /* Leeward 98.07.30 support msgX */
            break;
        case EOF:
            if( ! refscreen ) {
                abort_bbs();
            }
            domenu_screen( pm, cmdprompt );
            modify_user_mode( MMENU );
            /*Modify to showout ActiveBoard After talking*/
            if (nettyNN ==1) { R_monitor();}
            break;
        case KEY_RIGHT:
            for( i = 0; i < size; i++ ) {
                if( menupos[base+i].line == menupos[base+now].line && pm[i].level >= 0 &&
                        menupos[base+i].col > menupos[base+now].col && HAS_PERM(currentuser, pm[i].level ) )
                    break;
            }
            if( i < size ) {
                now = i;
                break;
            }
    case '\n': case '\r':
            if( strcmp( sysconf_relocate(pm[now].arg), ".." ) == 0 ) {
                return 0;
            }
            fptr=cmdlist_funcptr(sysconf_relocate(pm[now].func_name));
            if( fptr != NULL ) {
                move( 1, cmdplen );
                clrtoeol();
                (*fptr)( sysconf_relocate(pm[now].arg) );
                if( fptr == Select ) {
                    now++;
                }
                domenu_screen( pm, cmdprompt );
                modify_user_mode( MMENU );
                if (nettyNN ) { R_monitor();}
            }
            break;
      case KEY_LEFT:
            for( i = 0; i < size; i++ ) {
                if( menupos[base+i].line == menupos[base+now].line && pm[i].level >= 0 &&
                        menupos[base+i].col < menupos[base+now].col && HAS_PERM(currentuser, pm[i].level ) )
                    break;
                if( cmdlist_funcptr(sysconf_relocate(pm[i].func_name)) == Goodbye )
                    break;
            }
            if( i < size ) {
                now = i;
                break;
            }
            return 0;
       case KEY_DOWN:
            now++;
            break;
        case KEY_UP:
            now--;
            while( pm[now].level < 0 || !HAS_PERM(currentuser, pm[now].level ) ) {
                if( now > 0 )  now--;
                else  now = size - 1;
            }
            break;
        case '~':
            if(!HAS_PERM(currentuser,PERM_SYSOP)) {
                break;
            }
            bbslog("1bbs", "rebuild sysconf.img" );
            build_sysconf( "etc/sysconf.ini", NULL);
            report( "reload sysconf.img" );
            load_sysconf();

            free(menupos);
    		menupos=(struct _menupos*)malloc(sizeof(struct _menupos)*sysconf_menu);
    		copymenupos();
            pm = sysconf_getmenu(menu_name);
            size = domenu_screen( pm, cmdprompt );
            now = 0;
            break;
        default:
            if( cmd >= 'a' && cmd <= 'z' )
                cmd = cmd - 'a' + 'A';
            for( i = 0; i < size; i++ ) {
                if( menupos[base+i].line > 0 && cmd == *sysconf_relocate(pm[i].name) &&
                        HAS_PERM(currentuser, pm[i].level ) ) {
                    now = i;
                    break;
                }

            }
        }
    }
}

