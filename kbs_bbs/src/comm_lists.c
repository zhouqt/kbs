/* etnlegend, 2006.10.23, 整理 comm_lists ... */

#include "bbs.h"
#include "comm_lists.h"

KBS_COMM_LISTS_GENERATE(V,t_query_null,t_query,(NULL))
KBS_COMM_LISTS_GENERATE(V,d_user_nul,d_user,(""))

static const CMD_LIST CMD[]={
    {"EditSFiles",          a_edits,                                0},
    {"AllBoard",            AllBoard,                               0},
    {"Announce",            Announce,                               0},
    {"BadList",             badlist,                                0},
    {"BoardsAll",           Boards,                                 0},
    {"ClearAllNew",         clear_all_board_read_flag,              0},
    {"ShowLicense",         Conditions,                             0},
    {"ConfirmDelete",       confirm_delete_id,                      0},
    {"ConveyID",            ConveyID,                               0},
    {"DelBoard",            d_board,                                0},
    {"DefineKeys",          define_key,                             0},
    {"DisplayPublic",       display_publicshm,                      0},
    {"domenu",              domenu,                                 1},
    {"DelUser",             d_user_nul,                             0},
    {"DelView",             edit_board_delete_read_perm,            0},
    {"EGroups",             EGroup,                                 1},
    {"ENewGroups",          ENewGroup,                              1},
    {"EnterChat",           ent_chat1,                              0},
    {"EnterChat2",          ent_chat2,                              0},
    {"RunMBEM",             exec_mbem,                              1},
    {"FavBoard",            FavBoard,                               0},
    {"FriendWall",          friend_wall,                            0},
    {"GetFavRead",          get_favread,                            0},
    {"GiveupNet",           giveupnet,                              0},
    {"LeaveBBS",            Goodbye,                                0},
    {"GroupSend",           g_send,                                 0},
    {"ShowVersion",         Info,                                   0},
    {"InnReload",           inn_reload,                             0},
    {"InnStart",            inn_start,                              0},
    {"InnStop",             inn_stop,                               0},
    {"Kickalluser",         kick_all_user,                          0},
    {"KickUser",            kick_user_menu,                         0},
    {"LockScreen",          lock_scr,                               0},
    {"MailAll",             mailall,                                0},
    {"MailProc",            MailProc,                               0},
    {"EditBoard",           m_editbrd,                              0},
    {"ModifyInfo",          m_info,                                 0},
    {"SendNetMail",         m_internet,                             0},
    {"ReadNewMail",         m_new,                                  0},
    {"NewBoard",            m_newbrd,                               0},
    {"ModifyUserAdmin",     modify_userinfo_admin,                  0},
    {"ModifyUserCurrent",   modify_userinfo_current,                0},
    {"ReadMail",            m_read,                                 0},
    {"CheckForm",           m_register,                             0},
    {"SendMail",            m_sendnull,                             0},
    {"StopLogin",           m_stoplogin,                            0},
    {"SetTrace",            m_trace,                                0},
    {"OpenVote",            m_vote,                                 0},
    {"BoardsNew",           New,                                    0},
    {"OverrideSend",        ov_send,                                0},
    {"PostArticle",         Post,                                   0},
    {"ProtectID",           ProtectID,                              0},
    {"QueryBUser",          query_bm,                               0},
    {"ReadBoard",           ReadBoard,                              0},
    {"SearchIP",            search_ip,                              0},
    {"SearchTrace",         searchtrace,                            0},
    {"SelectBoard",         Select,                                 0},
    {"XBoardManager",       set_BM,                                 0},
    {"SetAlarm",            set_clock,                              0},
    {"SetACL",              set_ip_acl,                             0},
    {"Setsyspass",          setsystempasswd,                        0},
    {"ShowMsg",             show_allmsgs,                           0},
    {"Notepad",             shownotepad,                            0},
    {"ReadWeather",         ShowWeather,                            0},
    {"SendMsg",             s_msg,                                  0},
    {"OffLine",             suicide,                                0},
    {"ConvCode",            switch_code,                            0},
    {"ShowFriends",         t_friends,                              0},
    {"ListLogins",          t_list,                                 0},
    {"Monitor",             t_monitor,                              0},
    {"SetFriends",          t_override,                             0},
    {"SetPager",            t_pager,                                0},
    {"QueryUser",           t_query_null,                           0},
    {"RealLogins",          t_rusers,                               0},
    {"Talk",                t_talk,                                 0},
    {"ShowLogins",          t_users,                                0},
    {"ViewUTMP",            t_utmp,                                 0},
    {"ViewWWWguest",        t_wwwguest,                             0},
    {"SpecialUser",         Users,                                  0},
    {"WaitFriend",          wait_friend,                            0},
    {"Announceall",         wall,                                   0},
    {"ShowWelcome",         Welcome,                                0},
    {"WwwBoard",            WwwBoard,                               0},
    {"XCheckLevel",         XCheckLevel,                            0},
    {"SetCloak",            x_cloak,                                0},
    {"ShowDate",            x_date,                                 0},
    {"Xdelipacl",           Xdelipacl,                              0},
    {"Xdeljunk",            Xdeljunk,                               0},
    {"Xlevel",              x_deny,                                 0},
    {"EditUFiles",          x_edits,                                0},
    {"FillForm",            x_fillform,                             0},
    {"SetInfo",             x_info,                                 0},
    {"ModifyLevel",         x_level,                                0},
    {"VoteResult",          x_results,                              0},
    {"UserDefine",          x_userdefine,                           0},
    {"UDefine",             x_userdefine1,                          0},
    {"DoVote",              x_vote,                                 0},

#if HAVE_MYSQL_SMTH+0 == 1
    {"Addrlist",            al_read,                                0},
#endif /* HAVE_MYSQL_SMTH+0 == 1 */

#ifdef PERSONAL_CORP
    {"PcorpManage",         pc_add_user,                            0},
    {"Pcorp",               pc_choose_user,                         0},
#endif /* PERSONAL_CORP */

#ifdef ZIXIA
    {"ModifyAltar",         m_altar,                                0},
#endif /* ZIXIA */

#ifdef FB2KPC
    {"AddFBPC",             AddPCorpus,                             0},
    {"Personal",            Personal,                               1},
#endif /* FB2KPC */

#ifdef SMS_SUPPORT
    {"RegisterSMS",         register_sms,                           0},
    {"SendSMS",             send_sms,                               0},
#if HAVE_MYSQL_SMTH+0 == 1
    {"SMSmsg",              smsmsg_read,                            0},
#endif /* HAVE_MYSQL_SMTH+0 == 1 */
    {"UnRegisterSMS",       unregister_sms,                         0},
    {"SMSdef",              x_usersmsdef,                           0},
#endif /* SMS_SUPPORT */

    {NULL,                  NULL,                                  -1}
};

static CMD_MENU *menu;

static int decode_string(const char *s){
    register int i,j;
    char buf[4096];
    int val,count;
    for(i=0,j=0;(buf[i]=s[j]);j++){
        if(buf[i]!='\001'){
            i++;
            continue;
        }
        if((val=s[++j])&&(count=s[++j])){
            while(count-->0)
                buf[i++]=val;
        }
    }
    outs(buf);
    return 0;
}

static CMD_FUNC search_func(const char *name,int *type){
    register const CMD_LIST *p=CMD;
    while(p->name){
        if(!strcmp(name,p->name)){
            *type=p->type;
            return p->func;
        }
        p++;
    }
    *type=-1;
    return NULL;
}

int exec_mbem(const char *command){
#define EM_QUIT(msg)                                                    \
    do{                                                                 \
        if(orig_mode!=-1)                                               \
            modify_user_mode(orig_mode);                                \
        if(msg){                                                        \
            clear();                                                    \
            move(4,0);                                                  \
            prints("\033[1;33m\t%s\033[0;33m<Enter>\033[m",(msg));      \
            WAIT_RETURN;                                                \
        }                                                               \
        return (!msg);                                                  \
    }                                                                   \
    while(0)
    void (*func)(void);
    char buf[1024],*p,*q;
    int orig_mode=-1;
    void *dll;
    if(HAS_PERM(getCurrentUser(),PERM_DENYRELAX)&&!HAS_PERM(getCurrentUser(),(PERM_SYSOP|PERM_ADMIN)))
        EM_QUIT("您目前不具有休闲娱乐权限, 无法执行此项操作...");
    strcpy(buf,command);
    if(!(p=strstr(buf,"@mod")))
        return 0;
    orig_mode=modify_user_mode(SERVICES);
    if((p>&buf[1])&&(toupper(buf[1])==
#ifdef SSHBBS
        'T'
#else /* ! SSHBBS */
        'S'
#endif /* SSHBBS */
        )){
#ifdef SSHBBS
        EM_QUIT("当前功能仅在 Telnet 方式登录下有效...");
#else /* ! SSHBBS */
        EM_QUIT("当前功能仅在 SSH 方式登录下有效...");
#endif /* SSHBBS */
    }
    p+=5;
    if((q=strchr(p,'#')))
        *q++=0;
#if defined(CYGWIN) && defined(SSHBBS)
    do{
        char svc[STRLEN],*r;
        if((r=strrchr(p,'.')))
            *r=0;
        snprintf(svc,STRLEN,"%s.ssh%s",p,(!r?"":r));
        dll=dlopen(svc,RTLD_NOW);
    }
    while(0);
#else /* ! (CYGWIN && SSHBBS) */
    dll=dlopen(p,RTLD_NOW);
#endif /* CYGWIN && SSHBBS */
    if(!dll)
        EM_QUIT("模块载入失败, 操作终止...");
    if(!(func=dlsym(dll,(!q?"mod_main":q)))){
        dlclose(dll);
        EM_QUIT("入口函数装入失败, 操作终止...");
    }
    (*func)();
    dlclose(dll);
    EM_QUIT(NULL);
}

static int domenu_screen(const struct smenuitem *pm,const char *prompt){
    const char *name,*desc,*str;
    int row,col,count,n;
    clear();
    row=3;col=0;count=0;
    while(1){
        n=(pm-menuitem);
        switch(pm->level){
            case -1:
                return count;
            case -2:
                name=sysconf_relocate(pm->name);
                desc=sysconf_relocate(pm->desc);
                if(!strcmp(name,"title")){
                    docmdtitle(desc,prompt);
                    update_endline();
                }
                else if(!strcmp(name,"screen")
                    &&(str=sysconf_str(DEFINE(getCurrentUser(),DEF_SHOWSCREEN)?desc:"S_BLANK"))){
                    move(menu[n].row,menu[n].col);
                    decode_string(str);
                }
                break;
            default:
                if(!(menu[n].row<0)&&HAS_PERM(getCurrentUser(),pm->level)){
                    if(!(menu[n].row)){
                        menu[n].row=row;
                        menu[n].col=col;
                    }
                    else{
                        row=menu[n].row;
                        col=menu[n].col;
                    }
                    move(row++,col);
                    prints("  %s",sysconf_relocate(pm->desc));
                }
                else{
                    if(menu[n].row>0){
                        row=menu[n].row;
                        col=menu[n].col;
                    }
                    menu[n].row=-1;
                }
                break;
        }
        pm++;count++;
    }
}

static int dump_menu_pos(void){
    register int i;
    for(i=0;i<sysconf_menu;i++){
        menu[i].row=menuitem[i].line;
        menu[i].col=menuitem[i].col;
        menu[i].func=search_func(sysconf_relocate(menuitem[i].func_name),&(menu[i].type));
    }
    return 0;
}

int domenu(const char *name){
#ifndef FREE
    const char *prompt="目前选择: ";
    const int prompt_len=10;
#else /* FREE */
    const char *prompt=NULL;
    const int prompt_len=0;
#endif /* ! FREE */
    CMD_FUNC func;
    struct smenuitem *pm;
    int size,now,n,i,key;
    if(!menu){
        if(!(menu=(CMD_MENU*)malloc(sysconf_menu*sizeof(CMD_MENU))))
            return -1;
        dump_menu_pos();
    }
    pm=sysconf_getmenu(name);
    size=domenu_screen(pm,prompt);
    n=(pm-menuitem);now=0;
    if(!strcmp(name,"TOPMENU")&&chkmail()){
        for(i=0;i<size;i++){
            if((menu[n+i].row>0)&&(*sysconf_relocate(pm[i].name)=='M'))
                now=i;
        }
    }
    modify_user_mode(MMENU);
#ifdef NEW_HELP
    helpmode=0;
#endif /* NEW_HELP */
    if(nettyNN==1)
        R_monitor(NULL);
    while(1){
#ifndef FREE
        printacbar();
#endif /* FREE */
        while((pm[now].level<0)||!HAS_PERM(getCurrentUser(),pm[now].level)){
            now++;
            if(!(now<size))
                now=0;
        }
        move(menu[n+now].row,menu[n+now].col);
#ifndef FREE
        prints("%s","◆");
#else /* FREE */
        prints("%s","> ");
#endif /* ! FREE */
        if(prompt){
            move(1,prompt_len);
            clrtoeol();
            prints("[\033[7m%-12s\033[m]",sysconf_relocate(pm[now].name));
        }
        key=igetkey();
        move(menu[n+now].row,menu[n+now].col);
        prints("%s","  ");
        switch(key){
            case EOF:
                abort_bbs(0);
                break;
            case KEY_REFRESH:
                domenu_screen(pm,prompt);
                modify_user_mode(MMENU);
#ifdef NEW_HELP
                helpmode=0;
#endif /* NEW_HELP */
                if(nettyNN==1)
                    R_monitor(NULL);
                break;
            case Ctrl('Z'):
                r_lastmsg();
                break;
            case KEY_RIGHT:
                for(i=0;i<size;i++){
                    if((menu[n+i].row==menu[n+now].row)&&!(pm[i].level<0)&&(menu[n+i].col>menu[n+now].col)
                        &&HAS_PERM(getCurrentUser(),pm[i].level))
                        break;
                }
                if(i<size){
                    now=i;
                    break;
                }
            case '\n':
            case '\r':
                if(!strcmp(sysconf_relocate(pm[now].arg),".."))
                    return 0;
                if((func=menu[n+now].func)){
                    if(prompt){
                        move(1,prompt_len);
                        clrtoeol();
                    }
                    set_alarm(0,0,NULL,NULL);
                    if(menu[n+now].type)
                        (*func)(sysconf_relocate(pm[now].arg));
                    else
                        (*func)();
                    domenu_screen(pm,prompt);
                    modify_user_mode(MMENU);
#ifdef NEW_HELP
                    helpmode=0;
#endif /* NEW_HELP */
                    if(nettyNN==1)
                        R_monitor(NULL);
                }
                break;
            case KEY_LEFT:
                for(i=0;i<size;i++){
                    if((menu[n+i].row==menu[n+now].row)&&!(pm[i].level<0)&&(menu[n+i].col<menu[n+now].col)
                        &&HAS_PERM(getCurrentUser(),pm[i].level))
                        break;
                    if(menu[n+i].func==Goodbye)
                        break;
                }
                if(i<size){
                    now=i;
                    break;
                }
                return 0;
            case KEY_DOWN:
                now++;
                break;
            case KEY_UP:
                now--;
                while((pm[now].level<0)||!HAS_PERM(getCurrentUser(),pm[now].level)){
                    if(now>0)
                        now--;
                    else
                        now=(size-1);
                }
                break;
            case '~':
                if(!HAS_PERM(getCurrentUser(),PERM_SYSOP))
                    break;
                newbbslog(BBSLOG_USIES,"rebuild sysconf.img");
                build_sysconf("etc/sysconf.ini",NULL);
                newbbslog(BBSLOG_USIES,"reload sysconf.img");
                load_sysconf();
                free(menu);
                if(!(menu=(CMD_MENU*)malloc(sysconf_menu*sizeof(CMD_MENU))))
                    return -1;
                dump_menu_pos();
                pm=sysconf_getmenu(name);
                size=domenu_screen(pm,prompt);
                now=0;
                break;
            default:
                for(i=0;i<size;i++){
                    if((menu[n+i].row>0)&&(*sysconf_relocate(pm[i].name)==toupper(key))
                        &&HAS_PERM(getCurrentUser(),pm[i].level)){
                        now=i;
                        break;
                    }
                }
                break;
        }
    }
}

