/* etnlegend, 2006.10.23, 整理 comm_lists ... */

#include "bbs.h"
#include "comm_lists.h"

KBS_COMM_LISTS_GENE(t_query_null,t_query,(NULL))
KBS_COMM_LISTS_GENE(d_user_nul,d_user,(""))

static const CMD_LIST CMD[]={
    {"EditSFiles",a_edits},
    {"AllBoard",AllBoard},
    {"Announce",Announce},
    {"BadList",badlist},
    {"BoardsAll",Boards},
    {"ClearAllNew",clear_all_board_read_flag},
    {"ShowLicense",Conditions},
    {"ConfirmDelete",confirm_delete_id},
    {"ConveyID",ConveyID},
    {"DelBoard",d_board},
    {"DefineKeys",define_key},
    {"DisplayPublic",display_publicshm},
    {"DelUser",d_user_nul},
    {"DelView",edit_board_delete_read_perm},
    {"EGroups",EGroup},
    {"ENewGroups",ENewGroup},
    {"EnterChat",ent_chat1},
    {"EnterChat2",ent_chat2},
    {"FavBoard",FavBoard},
    {"FriendWall",friend_wall},
    {"GetFavRead",get_favread},
    {"GiveupNet",giveupnet},
    {"LeaveBBS",Goodbye},
    {"GroupSend",g_send},
    {"ShowVersion",Info},
    {"InnReload",inn_reload},
    {"InnStart",inn_start},
    {"InnStop",inn_stop},
    {"Kickalluser",kick_all_user},
    {"KickUser",kick_user_menu},
    {"LockScreen",lock_scr},
    {"MailAll",mailall},
    {"MailProc",MailProc},
    {"EditBoard",m_editbrd},
    {"ModifyInfo",m_info},
    {"SendNetMail",m_internet},
    {"ReadNewMail",m_new},
    {"NewBoard",m_newbrd},
    {"ModifyUserAdmin",modify_userinfo_admin},
    {"ModifyUserCurrent",modify_userinfo_current},
    {"ReadMail",m_read},
    {"CheckForm",m_register},
    {"SendMail",m_sendnull},
    {"StopLogin",m_stoplogin},
    {"SetTrace",m_trace},
    {"OpenVote",m_vote},
    {"BoardsNew",New},
    {"OverrideSend",ov_send},
    {"PostArticle",Post},
    {"ProtectID",ProtectID},
    {"QueryBUser",query_bm},
    {"ReadBoard",ReadBoard},
    {"SearchIP",search_ip},
    {"SearchTrace",searchtrace},
    {"SelectBoard",Select},
    {"XBoardManager",set_BM},
    {"SetAlarm",set_clock},
    {"SetACL",set_ip_acl},
    {"Setsyspass",setsystempasswd},
    {"ShowMsg",show_allmsgs},
    {"Notepad",shownotepad},
    {"ReadWeather",ShowWeather},
    {"SendMsg",s_msg},
    {"OffLine",suicide},
    {"ConvCode",switch_code},
    {"ShowFriends",t_friends},
    {"ListLogins",t_list},
    {"Monitor",t_monitor},
    {"SetFriends",t_override},
    {"SetPager",t_pager},
    {"QueryUser",t_query_null},
    {"RealLogins",t_rusers},
    {"Talk",t_talk},
    {"ShowLogins",t_users},
    {"ViewUTMP",t_utmp},
    {"ViewWWWguest",t_wwwguest},
    {"SpecialUser",Users},
    {"WaitFriend",wait_friend},
    {"Announceall",wall},
    {"ShowWelcome",Welcome},
    {"WwwBoard",WwwBoard},
    {"XCheckLevel",XCheckLevel},
    {"SetCloak",x_cloak},
    {"ShowDate",x_date},
    {"Xdelipacl",Xdelipacl},
    {"Xdeljunk",Xdeljunk},
    {"Xlevel",x_deny},
    {"EditUFiles",x_edits},
    {"FillForm",x_fillform},
    {"SetInfo",x_info},
    {"ModifyLevel",x_level},
    {"VoteResult",x_results},
    {"UserDefine",x_userdefine},
    {"UDefine",x_userdefine1},
    {"DoVote",x_vote},

#if HAVE_MYSQL_SMTH+0 == 1
    {"Addrlist",al_read},
#endif /* HAVE_MYSQL_SMTH+0 == 1 */

#ifdef PERSONAL_CORP
    {"PcorpManage",pc_add_user},
    {"Pcorp",pc_choose_user},
#endif /* PERSONAL_CORP */

#ifdef ZIXIA
    {"ModifyAltar",m_altar},
#endif /* ZIXIA */

#ifdef FB2KPC
    {"AddFBPC",AddPCorpus},
    {"Personal",Personal},
#endif /* FB2KPC */

#ifdef SMS_SUPPORT
    {"RegisterSMS",register_sms},
    {"SendSMS",send_sms},
#if HAVE_MYSQL_SMTH+0 == 1
    {"SMSmsg",smsmsg_read},
#endif /* HAVE_MYSQL_SMTH+0 == 1 */
    {"UnRegisterSMS",unregister_sms},
    {"SMSdef",x_usersmsdef},
#endif /* SMS_SUPPORT */

    {"RunMBEM",exec_mbem},
    {"domenu",domenu},

    {NULL,NULL}
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

static CMD_FUNC search_func(const char *name){
    register const CMD_LIST *p=CMD;
    while(p->name){
        if(!strcmp(name,p->name))
            return p->func;
        p++;
    }
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
        ))
        EM_QUIT(
#ifdef SSHBBS
            "当前功能仅在 Telnet 方式登录下有效..."
#else /* ! SSHBBS */
            "当前功能仅在 SSH 方式登录下有效..."
#endif /* SSHBBS */
            );
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

static int dump_menu_pos(){
    register int i;
    for(i=0;i<sysconf_menu;i++){
        menu[i].row=menuitem[i].line;
        menu[i].col=menuitem[i].col;
        menu[i].func=search_func(sysconf_relocate(menuitem[i].func_name));
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
            if((menu[n].row>0)&&(*sysconf_relocate(pm[i].name)=='M'))
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
                    (*func)(sysconf_relocate(pm[now].arg));
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

