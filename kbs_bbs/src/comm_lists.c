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
#include <dlfcn.h>

extern int nettyNN;

int domenu();

/* Add By Excellent */
/* int     t_announce(), t_tin(), t_gopher(), x_excemj(),x_excebig2(),x_excechess();*/
int Announce();
int Boards();
int EGroup();
int ENewGroup();
int Info();
int Goodbye();
int Help();
int New();
int Post();
int ReadBoard();
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
int ShowWeather();              /* 2001.6.12 */
int x_results();
int ConveyID();             /* Added by binxun 2003.5.3 */

int search_ip();
int kick_all_user();
#ifdef CAN_EXEC
int ent_bnet();
#endif
int a_edits();
int x_edits();
int x_date();
int x_userdefine();
int confirm_delete_id();        /* Added by Bigman 2001.7.14 */

/*Add by SmallPig*/
int m_new(), m_read(), m_sendnull(), g_send();
int ov_send(), m_internet(), mailall(), suicide();


int t_users(), t_friends(), t_rusers(), t_list(), t_monitor();
int wait_friend();
int t_query(), t_talk(), t_pager(), t_override(), x_cloak();

#ifdef IRC
int t_irc()
#endif
int kick_user();
int x_deny();
int set_BM();

/* add by KCN */
/* Modified by sanshao, sb KCN */
int ent_chat1(), ent_chat2();
int switch_code();              /* add by KCN */


int x_level(), XCheckLevel(), m_info(), d_user(), m_register(), m_newbrd(), setsystempasswd();
int d_board(), m_editbrd(), m_trace(), m_vote(), wall();
/*
int m_mclean();*/
/* inserted by cityhunter */
int query_bm();

/* end of insertion */
int lock_scr();                 /* Leeward 98.02.22 */
int FavBoard();                 /* period  2000-09-11 */
int MailProc();                 /* bad 2002.9.4 */
int searchtrace();              /* stephen 2000.12.15 */
int badlist();                  /* Bigman 2000.12.26 */
int giveupnet();                /* bad 2002.7.5 */
int m_stoplogin();
int inn_start();                /* czz 2002.01.15 */
int inn_reload();               /* czz 2002.01.15 */
int inn_stop();                 /* czz 2002.01.15 */
int clear_all_board_read_flag();        /* kcn 2002.07.18 */
int friend_wall();
int define_key();
int set_clock();
int set_ip_acl();

int exec_mbem(char *s);
#ifdef SMS_SUPPORT
int register_sms();
int unregister_sms();
int send_sms();
int smsmsg_read();
#endif
int al_read();

/*Add By Excellent */
struct scommandlist {
    char *name;
    int (*fptr) ();
};
static const struct scommandlist sysconf_cmdlist[] = {
    {"domenu", domenu},
    {"EGroups", EGroup},
    {"ENewGroups", ENewGroup},
    {"BoardsAll", Boards},
    {"BoardsNew", New},
    {"LeaveBBS", Goodbye},
    {"Announce", Announce},
    /*
     * {"ExecTin",      t_tin},
     * {"ExecGopher",   t_gopher}, 
     */
    {"SelectBoard", Select},
    {"ReadBoard", ReadBoard},
    {"PostArticle", Post},
    {"SetHelp", Help},
    {"SetAlarm", set_clock},
    {"MailAll", mailall},
    /*
     * {"ExecMJ",       x_excemj},
     * {"ExecBIG2",     x_excebig2},
     * {"ExecCHESS",    x_excechess},
     * {"WWW",          t_www},        
     */
    {"RunMBEM",exec_mbem},
    {"MailProc", MailProc},
    {"OffLine", suicide},
    {"ReadNewMail", m_new},
    {"ReadMail", m_read},
    {"SendMail", m_sendnull},
    {"GroupSend", g_send},
    {"OverrideSend", ov_send},
    {"SendNetMail", m_internet},
    {"UserDefine", x_userdefine},
    {"ShowFriends", t_friends},
    {"ShowLogins", t_users},
    {"QueryUser", t_query},
    {"WaitFriend", wait_friend},
    {"Talk", t_talk},
    {"SetPager", t_pager},
    {"SetCloak", x_cloak},
    {"SendMsg", s_msg},
    {"FriendWall",friend_wall},
    {"ShowMsg", show_allmsgs},
    {"SetFriends", t_override},
    {"EnterChat", ent_chat1},
    {"EnterChat2", ent_chat2},
#ifdef IRC
    {"ExecIrc", t_irc},
#endif
    {"ListLogins", t_list},
    {"Monitor", t_monitor},
    {"RealLogins", t_rusers},
    {"FillForm", x_fillform},
    {"SetInfo", x_info},
    {"EditUFiles", x_edits},
    {"ShowLicense", Conditions},
    {"ShowVersion", Info},
    {"Notepad", shownotepad},
    {"ShowDate", x_date},
    {"DoVote", x_vote},
    {"VoteResult", x_results},
#ifdef CAN_EXEC
    {"ExecBBSNet", ent_bnet},
#endif
    {"ShowWelcome", Welcome},
    {"ReadWeather", ShowWeather},       /*Bigman 2001.6.12 */
    {"SpecialUser", Users},
    /*
     * {"ExecViewer",   t_announce}, 
     */
    {"LockScreen", lock_scr},   /* Leeward 98.02.22 */
    {"ConvCode", switch_code},  /* KCN 99.09.03 */
    {"FavBoard", FavBoard},     /* added period 2000-09-11 */
    {"BadList", badlist},       /* added Bigman 2000.12.26 */
    {"GiveupNet", giveupnet},   /* added Bad 2002.7.5 */
    {"ClearAllNew", clear_all_board_read_flag}, /* kcn 2002.07.18 */
    {"CheckForm", m_register},
    {"ModifyInfo", m_info},
    {"ModifyLevel", x_level},
    {"QueryBUser", query_bm},
    {"ConveyID", ConveyID},    /* added by binxun 2003.5.3 */
    /*
     * end of addin 
     */
    {"XCheckLevel", XCheckLevel},
    {"KickUser", kick_user},
    {"DelUser", d_user},
    {"OpenVote", m_vote},
    {"NewBoard", m_newbrd},
    {"EditBoard", m_editbrd},
    {"DelBoard", d_board},
    {"SetTrace", m_trace},
    {"Xlevel", x_deny},
    {"XBoardManager", set_BM},
/*    
    {"CleanMail", m_mclean},
    */
    {"EditSFiles", a_edits},
    {"Announceall", wall},
    {"Setsyspass", setsystempasswd},
    {"SearchTrace", searchtrace},       /*stephen 2000.12.15 */
    {"StopLogin", m_stoplogin}, /*stephen 2000.12.15 */
    {"ConfirmDelete", confirm_delete_id},       /*Bigman 2001.7.14 */
    {"InnStart", inn_start},    /* czz 2002.01.15 */
    {"InnReload", inn_reload},  /* czz 2002.01.15 */
    {"InnStop", inn_stop},      /* czz 2002.01.15 */
    {"SearchIP",search_ip},
    {"Kickalluser",kick_all_user},	/* stiger, 2003.7.30 */
    {"DefineKeys",define_key},
    {"SetACL",set_ip_acl},
#ifdef SMS_SUPPORT
    {"RegisterSMS",register_sms},
    {"UnRegisterSMS",unregister_sms},
    {"SendSMS",send_sms},
	{"SMSmsg",smsmsg_read},
#endif
	{"Addrlist", al_read},
    {NULL, NULL},
};

void decodestr(register char *str)
{
    register char ch;
    int n, i=0;
    char buf[4096];

    while ((ch = *str++) != '\0')
        if (ch != '\01')
            buf[i++]=ch;
        else if (*str != '\0' && str[1] != '\0') {
            ch = *str++;
            n = *str++;
            while (--n >= 0)
                buf[i++]=ch;
        }
    buf[i]=0;
    outs(buf);
}

typedef int (*CMD_FUNC) ();
static CMD_FUNC cmdlist_funcptr(char *func_name)
{
    int n = 0;
    char *str;

    while ((str = sysconf_cmdlist[n].name) != NULL) {
        if (strcmp(func_name, str) == 0)
            return sysconf_cmdlist[n].fptr;
        n++;
    }
    return NULL;
}

extern int sysconf_menu;
struct _menupos {
    int line, col;
    CMD_FUNC fptr;
};

extern struct smenuitem *menuitem;
static struct _menupos *menupos = NULL;

int exec_mbem(char *s)
{
    void *hdll;   
    int (*func)();   
    char *c;   
    char buf[1024];   
    int oldmode;
    
    if (HAS_PERM(currentuser,PERM_DENYRELAX)) {
	clear();
        move(4,0);
	prints("你被封禁了休闲娱乐权限或者自己戒了休闲娱乐功能！");
	pressanykey();
	return 0;
    }
    oldmode = uinfo.mode;
    modify_user_mode(SERVICES);
    strcpy(buf,s);   
    s=strstr(buf,"@mod:");   
    if(s)   
    {      
        c=strstr(s+5,"#");      
        if(c)      
        { 
            *c=0;        
            c++; 
        }      
        hdll=dlopen(s+5,RTLD_LAZY);      
        if(hdll)      
        {         
	    char* error;
            if(func=dlsym(hdll,c ? c : "mod_main"))             
            func();         
	    else
            if ((error = dlerror()) != NULL)  {
	        clear();
	        prints ("%s\n", error);
                pressanykey();
            }   
            dlclose(hdll);      
        }      
        else 
        {      
            clear();      
            prints("，模块 [%s] 载入失败!!\n\n",s+5);      
            prints("失败原因:%s",dlerror());      
            pressanykey();
        }   
    }
    modify_user_mode(oldmode);
}

static int domenu_screen(struct smenuitem *dopm, char *cmdprompt)
{
    char *str;
    int help, line, col, num;
    struct smenuitem pm;

    int n;

/*    if(!DEFINE(currentuser,DEF_NORMALSCR))
 */
    clear();
    help = (currentuser->flags & CURSOR_FLAG);
    line = 3;
    col = 0;
    num = 0;
    while (1) {
        pm = *dopm;
        n = ((char *) dopm - (char *) menuitem) / sizeof(struct smenuitem);
        /*
         * 这个获得n的写法太丑陋了，但是.....
         * 先不管了KCN 
         */

        switch (pm.level) {
        case -1:
            return (num);
        case -2:
            if (strcmp(sysconf_relocate(pm.name), "title") == 0) {
                docmdtitle(sysconf_relocate(pm.desc), cmdprompt);
		update_endline();
            } else if (strcmp(sysconf_relocate(pm.name), "screen") == 0) {
                if (DEFINE(currentuser, DEF_SHOWSCREEN)) {
                    if (help && (str = sysconf_str(sysconf_relocate(pm.desc))) != NULL) {
                        move(menupos[n].line, menupos[n].col);
                        decodestr(str);
                    }
                } else {
                    if (help && (str = sysconf_str("S_BLANK")) != NULL) {
                        move(menupos[n].line, menupos[n].col);
                        decodestr(str);
                    }
                }
            }
            break;
        default:
            if (menupos[n].line >= 0 && HAS_PERM(currentuser, pm.level)) {
                if (menupos[n].line == 0) {
                    menupos[n].line = line;
                    menupos[n].col = col;
                } else {
                    line = menupos[n].line;
                    col = menupos[n].col;
                }
                if (help) {
                    move(line, col);
                    prints("  %s", sysconf_relocate(pm.desc));
                }
                line++;
            } else {
                if (menupos[n].line > 0) {
                    line = menupos[n].line;
                    col = menupos[n].col;
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

    for (n = 0; n < sysconf_menu; n++) {
        menupos[n].line = menuitem[n].line;
        menupos[n].col = menuitem[n].col;
        menupos[n].fptr = cmdlist_funcptr(sysconf_relocate(menuitem[n].func_name));
    }
}

int domenu(menu_name)
char *menu_name;
{
    struct smenuitem *pm;
    char *cmdprompt = "目前选择：";
    int size, now;
    int cmdplen, cmd, i, base;

    /*
     * if( sysconf_menu <= 0 ) {
     * return -1;
     * }
     */

    /*
     * disable it,因为不知道为何会core dump
     * if (check_sysconf()) {
     * free(menupos);
     * menupos=NULL;
     * }
     */
    if (menupos == NULL) {
        menupos = (struct _menupos *) malloc(sizeof(struct _menupos) * sysconf_menu);
        copymenupos();
    }
    pm = sysconf_getmenu(menu_name);
    size = domenu_screen(pm, cmdprompt);
    base = ((char *) pm - (char *) menuitem) / sizeof(struct smenuitem);
    /*
     * 这个获得base的写法太丑陋了，但是.....
     * 先不管了KCN 
     */
    cmdplen = strlen(cmdprompt);
    now = 0;
    if (strcmp(menu_name, "TOPMENU") == 0 && chkmail()) {
        for (i = 0; i < size; i++)
            if (menupos[base + i].line > 0 && (*sysconf_relocate(pm[i].name)) == 'M')
                now = i;

    }
    modify_user_mode(MMENU);
    /*
     * added by netty  
     */
    if (nettyNN == 1) {
        R_monitor(NULL);
    }
    while (1) {
        int (*fptr) ();

        printacbar();
        while (pm[now].level < 0 || !HAS_PERM(currentuser, pm[now].level)) {
            now++;
            if (now >= size)
                now = 0;
        }
        if (currentuser->flags & CURSOR_FLAG) {
            move(menupos[base + now].line, menupos[base + now].col);
            prints("◆");
        }
        move(1, cmdplen);
        clrtoeol();
        prints("[");
        prints("\x1b[7m%-12s\x1b[m", sysconf_relocate(pm[now].name));
        prints("]");
        clrtoeol();
        cmd = igetkey();
        if (currentuser->flags & CURSOR_FLAG) {
            move(menupos[base + now].line, menupos[base + now].col);
            prints("  ");
        }
        switch (cmd) {
        case Ctrl('Z'):
            r_lastmsg();        /* Leeward 98.07.30 support msgX */
            break;
        case EOF:
                abort_bbs(0);
                break;
		case KEY_REFRESH:
            domenu_screen(pm, cmdprompt);
            modify_user_mode(MMENU);
            /*
             * Modify to showout ActiveBoard After talking 
             */
            if (nettyNN == 1) {
                R_monitor(NULL);
            }
            break;
        case KEY_RIGHT:
            for (i = 0; i < size; i++) {
                if (menupos[base + i].line == menupos[base + now].line && pm[i].level >= 0 && menupos[base + i].col > menupos[base + now].col && HAS_PERM(currentuser, pm[i].level))
                    break;
            }
            if (i < size) {
                now = i;
                break;
            }
        case '\n':
        case '\r':
            if (strcmp(sysconf_relocate(pm[now].arg), "..") == 0) {
                return 0;
            }
            fptr = menupos[base + now].fptr;
            if (fptr != NULL) {
                move(1, cmdplen);
                clrtoeol();
                set_alarm(0, 0, NULL, NULL);
                (*fptr) (sysconf_relocate(pm[now].arg));
                if (fptr == Select) {
                    now++;
                }
                domenu_screen(pm, cmdprompt);
                modify_user_mode(MMENU);
                if (nettyNN) {
                    R_monitor(NULL);
                }
            }
            break;
        case KEY_LEFT:
            for (i = 0; i < size; i++) {
                if (menupos[base + i].line == menupos[base + now].line && pm[i].level >= 0 && menupos[base + i].col < menupos[base + now].col && HAS_PERM(currentuser, pm[i].level))
                    break;
                if (menupos[base + i].fptr == Goodbye)
                    break;
            }
            if (i < size) {
                now = i;
                break;
            }
            return 0;
        case KEY_DOWN:
            now++;
            break;
        case KEY_UP:
            now--;
            while (pm[now].level < 0 || !HAS_PERM(currentuser, pm[now].level)) {
                if (now > 0)
                    now--;
                else
                    now = size - 1;
            }
            break;
        case '~':
            if (!HAS_PERM(currentuser, PERM_SYSOP)) {
                break;
            }
            newbbslog(BBSLOG_USIES, "rebuild sysconf.img");
            build_sysconf("etc/sysconf.ini", NULL);
            bbslog("user","%s","reload sysconf.img");
            load_sysconf();

            free(menupos);
            menupos = (struct _menupos *) malloc(sizeof(struct _menupos) * sysconf_menu);
            copymenupos();
            pm = sysconf_getmenu(menu_name);
            size = domenu_screen(pm, cmdprompt);
            now = 0;
            break;
        default:
            if (cmd >= 'a' && cmd <= 'z')
                cmd = cmd - 'a' + 'A';
            for (i = 0; i < size; i++) {
                if (menupos[base + i].line > 0 && cmd == *sysconf_relocate(pm[i].name) && HAS_PERM(currentuser, pm[i].level)) {
                    now = i;
                    break;
                }

            }
        }
    }
}
