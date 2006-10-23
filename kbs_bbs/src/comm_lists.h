/* etnlegend, 2006.10.23, ХыАн comm_lists ... */

#ifndef KBS_COMM_LISTS_H
#define KBS_COMM_LISTS_H

#include <dlfcn.h>

#define KBS_COMM_LISTS_FUNC(function)   int function(void)

#define KBS_COMM_LISTS_GENE(function,transfer,args)                     \
    KBS_COMM_LISTS_FUNC(function){                                      \
        return (transfer args);                                         \
    }

typedef int (*CMD_FUNC)();
typedef struct _cmd_list{
    const char *name;
    CMD_FUNC    func;
}
CMD_LIST;
typedef struct _cmd_menu{
    int         row;
    int         col;
    CMD_FUNC    func;
}
CMD_MENU;

KBS_COMM_LISTS_FUNC(a_edits);
KBS_COMM_LISTS_FUNC(AllBoard);
KBS_COMM_LISTS_FUNC(Announce);
KBS_COMM_LISTS_FUNC(badlist);
KBS_COMM_LISTS_FUNC(Boards);
KBS_COMM_LISTS_FUNC(clear_all_board_read_flag);
KBS_COMM_LISTS_FUNC(Conditions);
KBS_COMM_LISTS_FUNC(confirm_delete_id);
KBS_COMM_LISTS_FUNC(ConveyID);
KBS_COMM_LISTS_FUNC(d_board);
KBS_COMM_LISTS_FUNC(define_key);
KBS_COMM_LISTS_FUNC(display_publicshm);
KBS_COMM_LISTS_FUNC(d_user_nul);
KBS_COMM_LISTS_FUNC(edit_board_delete_read_perm);
KBS_COMM_LISTS_FUNC(EGroup);
KBS_COMM_LISTS_FUNC(ENewGroup);
KBS_COMM_LISTS_FUNC(ent_chat1);
KBS_COMM_LISTS_FUNC(ent_chat2);
KBS_COMM_LISTS_FUNC(FavBoard);
KBS_COMM_LISTS_FUNC(friend_wall);
KBS_COMM_LISTS_FUNC(get_favread);
KBS_COMM_LISTS_FUNC(giveupnet);
KBS_COMM_LISTS_FUNC(Goodbye);
KBS_COMM_LISTS_FUNC(g_send);
KBS_COMM_LISTS_FUNC(Info);
KBS_COMM_LISTS_FUNC(inn_reload);
KBS_COMM_LISTS_FUNC(inn_start);
KBS_COMM_LISTS_FUNC(inn_stop);
KBS_COMM_LISTS_FUNC(kick_all_user);
KBS_COMM_LISTS_FUNC(kick_user_menu);
KBS_COMM_LISTS_FUNC(lock_scr);
KBS_COMM_LISTS_FUNC(mailall);
KBS_COMM_LISTS_FUNC(MailProc);
KBS_COMM_LISTS_FUNC(m_editbrd);
KBS_COMM_LISTS_FUNC(m_info);
KBS_COMM_LISTS_FUNC(m_internet);
KBS_COMM_LISTS_FUNC(m_new);
KBS_COMM_LISTS_FUNC(m_newbrd);
KBS_COMM_LISTS_FUNC(modify_userinfo_admin);
KBS_COMM_LISTS_FUNC(modify_userinfo_current);
KBS_COMM_LISTS_FUNC(m_read);
KBS_COMM_LISTS_FUNC(m_register);
KBS_COMM_LISTS_FUNC(m_sendnull);
KBS_COMM_LISTS_FUNC(m_stoplogin);
KBS_COMM_LISTS_FUNC(m_trace);
KBS_COMM_LISTS_FUNC(m_vote);
KBS_COMM_LISTS_FUNC(New);
KBS_COMM_LISTS_FUNC(ov_send);
KBS_COMM_LISTS_FUNC(Post);
KBS_COMM_LISTS_FUNC(ProtectID);
KBS_COMM_LISTS_FUNC(query_bm);
KBS_COMM_LISTS_FUNC(ReadBoard);
KBS_COMM_LISTS_FUNC(search_ip);
KBS_COMM_LISTS_FUNC(searchtrace);
KBS_COMM_LISTS_FUNC(Select);
KBS_COMM_LISTS_FUNC(set_BM);
KBS_COMM_LISTS_FUNC(set_clock);
KBS_COMM_LISTS_FUNC(set_ip_acl);
KBS_COMM_LISTS_FUNC(setsystempasswd);
KBS_COMM_LISTS_FUNC(show_allmsgs);
KBS_COMM_LISTS_FUNC(shownotepad);
KBS_COMM_LISTS_FUNC(ShowWeather);
KBS_COMM_LISTS_FUNC(s_msg);
KBS_COMM_LISTS_FUNC(suicide);
KBS_COMM_LISTS_FUNC(switch_code);
KBS_COMM_LISTS_FUNC(t_friends);
KBS_COMM_LISTS_FUNC(t_list);
KBS_COMM_LISTS_FUNC(t_monitor);
KBS_COMM_LISTS_FUNC(t_override);
KBS_COMM_LISTS_FUNC(t_pager);
KBS_COMM_LISTS_FUNC(t_query_null);
KBS_COMM_LISTS_FUNC(t_rusers);
KBS_COMM_LISTS_FUNC(t_talk);
KBS_COMM_LISTS_FUNC(t_users);
KBS_COMM_LISTS_FUNC(t_utmp);
KBS_COMM_LISTS_FUNC(t_wwwguest);
KBS_COMM_LISTS_FUNC(Users);
KBS_COMM_LISTS_FUNC(wait_friend);
KBS_COMM_LISTS_FUNC(wall);
KBS_COMM_LISTS_FUNC(Welcome);
KBS_COMM_LISTS_FUNC(WwwBoard);
KBS_COMM_LISTS_FUNC(XCheckLevel);
KBS_COMM_LISTS_FUNC(x_cloak);
KBS_COMM_LISTS_FUNC(x_date);
KBS_COMM_LISTS_FUNC(Xdelipacl);
KBS_COMM_LISTS_FUNC(Xdeljunk);
KBS_COMM_LISTS_FUNC(x_deny);
KBS_COMM_LISTS_FUNC(x_edits);
KBS_COMM_LISTS_FUNC(x_fillform);
KBS_COMM_LISTS_FUNC(x_info);
KBS_COMM_LISTS_FUNC(x_level);
KBS_COMM_LISTS_FUNC(x_results);
KBS_COMM_LISTS_FUNC(x_userdefine);
KBS_COMM_LISTS_FUNC(x_userdefine1);
KBS_COMM_LISTS_FUNC(x_vote);

#if HAVE_MYSQL_SMTH+0 == 1
KBS_COMM_LISTS_FUNC(al_read);
#endif /* HAVE_MYSQL_SMTH+0 == 1 */

#ifdef PERSONAL_CORP
KBS_COMM_LISTS_FUNC(pc_add_user);
KBS_COMM_LISTS_FUNC(pc_choose_user);
#endif /* PERSONAL_CORP */

#ifdef ZIXIA
KBS_COMM_LISTS_FUNC(m_altar);
#endif /* ZIXIA */

#ifdef FB2KPC
KBS_COMM_LISTS_FUNC(AddPCorpus);
KBS_COMM_LISTS_FUNC(Personal);
#endif /* FB2KPC */

#ifdef SMS_SUPPORT
KBS_COMM_LISTS_FUNC(register_sms);
KBS_COMM_LISTS_FUNC(send_sms);
#if HAVE_MYSQL_SMTH+0 == 1
KBS_COMM_LISTS_FUNC(smsmsg_read);
#endif /* HAVE_MYSQL_SMTH+0 == 1 */
KBS_COMM_LISTS_FUNC(unregister_sms);
KBS_COMM_LISTS_FUNC(x_usersmsdef);
#endif /* SMS_SUPPORT */

#endif /* KBS_COMM_LISTS_H */

