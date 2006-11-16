/* etnlegend, 2006.10.23, ХыАн comm_lists ... */

#ifndef KBS_COMM_LISTS_H
#define KBS_COMM_LISTS_H

#define KBS_COMM_LISTS_FUNC_V(function)     int function(void)
#define KBS_COMM_LISTS_FUNC_S(function)     int function(const char *_s)

#define KBS_COMM_LISTS_GENERATE(type,function,transfer,args)                \
    KBS_COMM_LISTS_FUNC_##type(function){                                   \
        (void)(transfer args);                                              \
        return 0;                                                           \
    }

typedef int (*CMD_FUNC)();
typedef struct _cmd_list{
    const char *name;
    CMD_FUNC    func;
    int         type;
}
CMD_LIST;
typedef struct _cmd_menu{
    CMD_FUNC    func;
    int         row;
    int         col;
    int         type;
}
CMD_MENU;

KBS_COMM_LISTS_FUNC_V(a_edits);
KBS_COMM_LISTS_FUNC_V(AllBoard);
KBS_COMM_LISTS_FUNC_V(Announce);
KBS_COMM_LISTS_FUNC_V(badlist);
KBS_COMM_LISTS_FUNC_V(Boards);
KBS_COMM_LISTS_FUNC_V(clear_all_board_read_flag);
KBS_COMM_LISTS_FUNC_V(Conditions);
KBS_COMM_LISTS_FUNC_V(confirm_delete_id);
KBS_COMM_LISTS_FUNC_V(ConveyID);
KBS_COMM_LISTS_FUNC_V(d_board);
KBS_COMM_LISTS_FUNC_V(define_key);
KBS_COMM_LISTS_FUNC_V(display_publicshm);
KBS_COMM_LISTS_FUNC_S(domenu);
KBS_COMM_LISTS_FUNC_V(d_user_nul);
KBS_COMM_LISTS_FUNC_V(edit_board_delete_read_perm);
KBS_COMM_LISTS_FUNC_S(EGroup);
KBS_COMM_LISTS_FUNC_S(ENewGroup);
KBS_COMM_LISTS_FUNC_V(ent_chat1);
KBS_COMM_LISTS_FUNC_V(ent_chat2);
KBS_COMM_LISTS_FUNC_S(exec_mbem);
KBS_COMM_LISTS_FUNC_V(FavBoard);
KBS_COMM_LISTS_FUNC_V(friend_wall);
KBS_COMM_LISTS_FUNC_V(get_favread);
KBS_COMM_LISTS_FUNC_V(giveupnet);
KBS_COMM_LISTS_FUNC_V(Goodbye);
KBS_COMM_LISTS_FUNC_V(g_send);
KBS_COMM_LISTS_FUNC_V(Info);
KBS_COMM_LISTS_FUNC_V(inn_reload);
KBS_COMM_LISTS_FUNC_V(inn_start);
KBS_COMM_LISTS_FUNC_V(inn_stop);
KBS_COMM_LISTS_FUNC_V(kick_all_user);
KBS_COMM_LISTS_FUNC_V(kick_user_menu);
KBS_COMM_LISTS_FUNC_V(lock_scr);
KBS_COMM_LISTS_FUNC_V(mailall);
KBS_COMM_LISTS_FUNC_V(MailProc);
KBS_COMM_LISTS_FUNC_V(m_editbrd);
KBS_COMM_LISTS_FUNC_V(m_info);
KBS_COMM_LISTS_FUNC_V(m_internet);
KBS_COMM_LISTS_FUNC_V(m_new);
KBS_COMM_LISTS_FUNC_V(m_newbrd);
KBS_COMM_LISTS_FUNC_V(modify_userinfo_admin);
KBS_COMM_LISTS_FUNC_V(modify_userinfo_current);
KBS_COMM_LISTS_FUNC_V(m_read);
KBS_COMM_LISTS_FUNC_V(m_register);
KBS_COMM_LISTS_FUNC_V(m_sendnull);
KBS_COMM_LISTS_FUNC_V(m_stoplogin);
KBS_COMM_LISTS_FUNC_V(m_trace);
KBS_COMM_LISTS_FUNC_V(m_vote);
KBS_COMM_LISTS_FUNC_V(New);
KBS_COMM_LISTS_FUNC_V(ov_send);
KBS_COMM_LISTS_FUNC_V(Post);
KBS_COMM_LISTS_FUNC_V(ProtectID);
KBS_COMM_LISTS_FUNC_V(query_bm);
KBS_COMM_LISTS_FUNC_V(ReadBoard);
KBS_COMM_LISTS_FUNC_V(search_ip);
KBS_COMM_LISTS_FUNC_V(searchtrace);
KBS_COMM_LISTS_FUNC_V(Select);
KBS_COMM_LISTS_FUNC_V(set_BM);
KBS_COMM_LISTS_FUNC_V(set_clock);
KBS_COMM_LISTS_FUNC_V(set_ip_acl);
KBS_COMM_LISTS_FUNC_V(setsystempasswd);
KBS_COMM_LISTS_FUNC_V(show_allmsgs);
KBS_COMM_LISTS_FUNC_V(shownotepad);
KBS_COMM_LISTS_FUNC_V(ShowWeather);
KBS_COMM_LISTS_FUNC_V(s_msg);
KBS_COMM_LISTS_FUNC_V(suicide);
KBS_COMM_LISTS_FUNC_V(switch_code);
KBS_COMM_LISTS_FUNC_V(t_friends);
KBS_COMM_LISTS_FUNC_V(t_list);
KBS_COMM_LISTS_FUNC_V(t_monitor);
KBS_COMM_LISTS_FUNC_V(t_override);
KBS_COMM_LISTS_FUNC_V(t_pager);
KBS_COMM_LISTS_FUNC_V(t_query_null);
KBS_COMM_LISTS_FUNC_V(t_rusers);
KBS_COMM_LISTS_FUNC_V(t_talk);
KBS_COMM_LISTS_FUNC_V(t_users);
KBS_COMM_LISTS_FUNC_V(t_utmp);
KBS_COMM_LISTS_FUNC_V(t_wwwguest);
KBS_COMM_LISTS_FUNC_V(Users);
KBS_COMM_LISTS_FUNC_V(wait_friend);
KBS_COMM_LISTS_FUNC_V(wall);
KBS_COMM_LISTS_FUNC_V(Welcome);
KBS_COMM_LISTS_FUNC_V(WwwBoard);
KBS_COMM_LISTS_FUNC_V(XCheckLevel);
KBS_COMM_LISTS_FUNC_V(x_cloak);
KBS_COMM_LISTS_FUNC_V(x_date);
KBS_COMM_LISTS_FUNC_V(Xdelipacl);
KBS_COMM_LISTS_FUNC_V(Xdeljunk);
KBS_COMM_LISTS_FUNC_V(x_deny);
KBS_COMM_LISTS_FUNC_V(x_edits);
KBS_COMM_LISTS_FUNC_V(x_fillform);
KBS_COMM_LISTS_FUNC_V(x_info);
KBS_COMM_LISTS_FUNC_V(x_level);
KBS_COMM_LISTS_FUNC_V(x_results);
KBS_COMM_LISTS_FUNC_V(x_userdefine);
KBS_COMM_LISTS_FUNC_V(x_userdefine1);
KBS_COMM_LISTS_FUNC_V(x_vote);

#if HAVE_MYSQL_SMTH+0 == 1
KBS_COMM_LISTS_FUNC_V(al_read);
#endif /* HAVE_MYSQL_SMTH+0 == 1 */

#ifdef PERSONAL_CORP
KBS_COMM_LISTS_FUNC_V(pc_add_user);
KBS_COMM_LISTS_FUNC_V(pc_choose_user);
#endif /* PERSONAL_CORP */

#ifdef ZIXIA
KBS_COMM_LISTS_FUNC_V(m_altar);
#endif /* ZIXIA */

#ifdef FB2KPC
KBS_COMM_LISTS_FUNC_V(AddPCorpus);
KBS_COMM_LISTS_FUNC_S(Personal);
#endif /* FB2KPC */

#ifdef SMS_SUPPORT
KBS_COMM_LISTS_FUNC_V(register_sms);
KBS_COMM_LISTS_FUNC_V(send_sms);
#if HAVE_MYSQL_SMTH+0 == 1
KBS_COMM_LISTS_FUNC_V(smsmsg_read);
#endif /* HAVE_MYSQL_SMTH+0 == 1 */
KBS_COMM_LISTS_FUNC_V(unregister_sms);
KBS_COMM_LISTS_FUNC_V(x_usersmsdef);
#endif /* SMS_SUPPORT */

#endif /* KBS_COMM_LISTS_H */

