#include "bbs.h"

#define refreshtime     (30)
extern time_t login_start_time;
int (*func_list_show) ();

time_t update_time = 0;
int showexplain = 0, freshmode = 0;
int numf;
int friendmode = 0;
int usercounter, real_user_names = 0;
int range, page, readplan, num;
struct user_info *user_record[USHM_SIZE];
struct userec *user_data;
struct WWW_GUEST_S *wg_record[MAX_WWW_GUEST];
int t_users(void);
int Show_Users();
int print_title()
{
    if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
        docmdtitle((friendmode) ? "[好朋友列表]" : "[使用者列表]",
                   " 聊天[\x1b[1;32mt\x1b[m] 寄信[\x1b[1;32mm\x1b[m] 送讯息[\x1b[1;32ms\x1b[m] 加,减朋友[\x1b[1;32mo\x1b[m,\x1b[1;32md\x1b[m] 看说明档[\x1b[1;32m→\x1b[m,\x1b[1;32mr\x1b[m] 切换模式 [\x1b[1;32mf\x1b[m] 求救[\x1b[1;32mh\x1b[m]");
    else
        docmdtitle((friendmode) ? "[好朋友列表]" : "[使用者列表]", " 聊天[t] 寄信[m] 送讯息[s] 加,减朋友[o,d] 看说明档[→,r] 切换模式 [f] 求救[h]");
    update_endline();
    return 0;
}

int utmp_title()
{
    docmdtitle("[活动进程列表]", "  该功能在测试中，有任何问题请与\033[1;32mpig2532\033[m联系，谢谢。");
    update_endline();
    return 0;
}

int wwwguest_title()
{
    docmdtitle("[www guest 列表]", "  该功能在测试中，有任何问题请与\033[1;32mpig2532\033[m联系，谢谢。");
    update_endline();
    return 0;
}

int print_title2()
{
    if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
        docmdtitle((friendmode) ? "[好朋友列表]" : "[使用者列表]",
                   "          寄信[\x1b[1;32mm\x1b[m] 加,减朋友[\x1b[1;32mo\x1b[m,\x1b[1;32md\x1b[m] 看说明档[\x1b[1;32m→\x1b[m,\x1b[1;32mr\x1b[m] 选择[\x1b[1;32m↑\x1b[m,\x1b[1;32m↓\x1b[m] 求救[\x1b[1;32mh\x1b[m]");
    else
        docmdtitle((friendmode) ? "[好朋友列表]" : "[使用者列表]", "          寄信[m] 加,减朋友[o,d] 看说明档[→,r] 选择[↑,↓] 求救[h]");
    update_endline();
    return 0;
}
void update_data(void *data)
{
    if (readplan == true)
        return;
    idle_count++;
    if (time(0) >= update_time + refreshtime * idle_count - 1) {
        freshmode = 1;

        /*
         * Take out by SmallPig 
         */
        /*
         * 否则在执行一些子程式时，画面也会更新 
         */
        /*
         * (*func_list_show)();
         * update_endline();
         * move( 3+num-page,0 ); prints( ">");
         * refresh(); 
         */
    }
    set_alarm(refreshtime * idle_count, 0, update_data, NULL);
    UNUSED_ARG(data);
    return;
}

int print_user_info_title()
{
    char title_str[512];
    char *field_2;

    move(2, 0);
    clrtoeol();
    field_2 = "使用者昵称";
    if (real_user_names)
        field_2 = "真实姓名  ";
    sprintf(title_str,
             /*---	modified by period	2000-10-21	在线用户数可以大于1000的
                     "\033[44m%s%-12.12s %-16.16s %-16.16s %c %c %-16.16s %5s\033[m\n",
             ---*/
            "\033[44m %s%-12.12s %-22.22s %-16.16s %c %c %-10.10s%5s", "编号  ", "使用者代号", (showexplain == 1) ? "好友说明或代号" : field_2, "来自", 'P',
            /*
             * (HAS_PERM(getCurrentUser(),PERM_SYSOP) ? 'C' : ' ') 
             */ 'M', "动态",
            "时:分");
    prints("%s", title_str);
    clrtoeol();
    prints("\n");
    return 0;
}
void show_message(char *msg)
{
    move(BBS_PAGESIZE + 3, 0);
    clrtoeol();
    if (msg != NULL) {
        prints("\033[1m%s\033[m", msg);
        refresh();
        sleep(1);
    }
    update_endline();
}

void swap_user_record(a, b)
int a, b;
{
    struct user_info *c;

    c = user_record[a];
    user_record[a] = user_record[b];
    user_record[b] = c;
}
int full_utmp(struct user_info *uentp, int *count)
{
    if (!uentp->active || !uentp->pid) {
        return 0;
    }
    if (!HAS_PERM(getCurrentUser(), PERM_SEECLOAK) && uentp->invisible && strcmp(uentp->userid, getCurrentUser()->userid)) {      /*Haohmaru.99.4.24.让隐身者能看见自己 */
        return 0;
    }
    if (friendmode && !myfriend(uentp->uid, NULL, getSession())) {
        return 0;
    }
    user_record[*count] = uentp;
    (*count)++;
    return COUNT;
}

int fill_userlist()
{
    static int i, i2;


    /*
     * struct      user_info *not_good; 
     */
    i2 = 0;
    if (!friendmode) {
        apply_ulist_addr((APPLY_UTMP_FUNC) full_utmp, (char *) &i2);
    } else {
        struct user_info* u;
	u=get_utmpent(getSession()->utmpent);
        for (i = 0; i < u->friendsnum; i++) {
            if (u->friends_uid[i])
                apply_utmpuid((APPLY_UTMP_FUNC) full_utmp, u->friends_uid[i], (char *) &i2);
    }
   } range = i2;
    return i2 == 0 ? -1 : 1;
}

// load www guest list, pig2532 2006-9
int fill_wglist()
{
    int count, i;

    resolve_guest_table();
    count = 0;
    for(i=0; i<MAX_WWW_GUEST; i++)
    {
        if(wwwguest_shm->guest_entry[i].key > 0) {
            wg_record[count] = &(wwwguest_shm->guest_entry[i]);
            count++;
        }
    }
    return 0;
}

char pagerchar(int usernum, struct user_info *user, int pager, int *isfriend)
{
    if (pager & ALL_PAGER)
        return ' ';
    if (*isfriend == -1)
        *isfriend = hisfriend(usernum, user);
    if (*isfriend) {
        if (pager & FRIEND_PAGER)
            return 'O';

        else
            return '#';
    }
    return '*';
}
char msgchar(struct user_info *uin, int *isfriend)
{
    if ((uin->pager & ALLMSG_PAGER))
        return ' ';
    if (*isfriend == -1)
        *isfriend = hisfriend(getSession()->currentuid, uin);
    if (*isfriend) {
        if ((uin->pager & FRIENDMSG_PAGER))
            return 'O';

        else
            return '#';
    }
    return '*';
}

static bool showcolor=true;

int do_userlist()
{
    int i,y,x;
    char user_info_str[256 /*STRLEN*2 */ ], pagec;
    char tbuf[80];
	char modebuf[80],idlebuf[10];
    int override;
    extern bool disable_move;
    extern bool disable_color;
    char fexp[30];
    struct user_info uentp;
	struct userec *lookupuser;

    move(3, 0);
    print_user_info_title();
    for (i = 0; i < BBS_PAGESIZE && i + page < range; i++) {
        int isfriend;

        isfriend = -1;
        if (user_record[i + page] == NULL) {
            clear();
            prints("\033[1;31m系统出现Bug,请到Sysop版报告，谢谢!\033[m");
            oflush();
            sleep(10);
            exit(0);
        }
        uentp = *(user_record[i + page]);
        if (!uentp.active || !uentp.pid || uentp.logintime > update_time) {
            clrtoeol();
            prints(" %4d 啊,我刚走\n", i + 1 + page);
            continue;
        }
		if( getuser(uentp.userid, &lookupuser) == 0 ){
			prints(" error");
			continue;
		}
        if (!showexplain)
            override = (i + page < numf) || friendmode;

        else {
            if ((i + page < numf) || friendmode)
                override = myfriend(uentp.uid, fexp, getSession());
            else
                override = false;
        }
        if (readplan == true) {
            return 0;
        }
        pagec = pagerchar(getSession()->currentuid,&uentp, uentp.pager, &isfriend);
        strncpy(tbuf, (real_user_names) ? uentp.realname : (showexplain && override) ? fexp : uentp.username, sizeof(tbuf));
//昵称在列表中最后一字消除乱码，shiyao  2003.6.1
//	j = 15;
//	while (j>=0 && tbuf[j]<0) j--;
//	if ((15-j)%2)  tbuf[15] = 0;
        tbuf[sizeof(tbuf) - 1]=0;
        resetcolor();
        clrtoeol();
        getyx(&y, &x);
        move(y, (i+page>9998)?21:20);
        if(!showcolor)
            disable_color = true;
        disable_move = true;
        prints("%s", tbuf);
        disable_move = false;
        if(!showcolor)
            disable_color = false;
        resetcolor();
        move(y, x);
        sprintf(user_info_str,
                " %4d%2s%s%-12.12s%s%s ", 
                i + 1 + page, (override) ? (uentp.invisible ? "＃" : "．") : (uentp.invisible ? "＊" : ""),
                (override) ? "\033[1;32m" : "", uentp.userid, (override) ? "\033[m" : "", 
                (override && showexplain) ? "\033[1;31m" : "");
        prints("%s", user_info_str);
        resetcolor();
        move(y, 42);
        sprintf(user_info_str, " %-16.16s %c %c %s%-10.10s\033[m%5.5s\n",  
                (HAS_PERM(getCurrentUser(), PERM_SYSOP))? uentp.from : ( (pagec == ' ' || pagec == 'O')  ? SHOW_USERIP(lookupuser, uentp.from) : "*" ),
                pagec, msgchar(&uentp, &isfriend), 
                (uentp.invisible == true)? (uentp.pid==1?"\033[33m":"\033[34m") : (uentp.pid==1?"\033[1;36m":""), 
				modestring(modebuf,uentp.mode, uentp.destuid, 0,        /* 1->0 不显示聊天对象等 modified by dong 1996.10.26 */
                                           (uentp.in_chat ? uentp.chatid : NULL)),            
                idle_str(idlebuf,&uentp));
        prints("%s", user_info_str);
        resetcolor();
    }
    return 0;
}

// show all utmp information, pig2532 2006-9
int show_utmplist()
{
    int i, rn;
    struct user_info uent;
	struct boardheader *bp = NULL;
    char stbuf[STRLEN];
    
    if (update_time + refreshtime < time(0)) {
        fill_userlist();
        update_time = time(0);
    }
    move(2, 0);
	prints("\033[1;37;44m  序号 用户名        进程号 IP地址          当前状态");
	clrtoeol();
	prints("\033[m");
    rn = 3;
    for(i=0; i<BBS_PAGESIZE && i+page<range; i++) {
        uent = *(user_record[i+page]);
        move(rn, 0);
        rn++;
        if(!uent.active || !uent.pid || uent.logintime > update_time)
            prints("    -- null --\n");
        else {
            bp = (struct boardheader *)getboard(uent.currentboard);
            prints("  %4d %s%-12s\033[m %7d %-15s", i+page, uent.invisible?"\033[1;34m":"", uent.userid?uent.userid:"", uent.pid, uent.from);
            if(bp)
                prints(" %s", bp->filename);
            prints(" %s", modestring(stbuf, uent.mode, uent.destuid, 0, ""));
        }
        clrtoeol();
    }
    clrtobot();
    return 0;
}

// show all www guest, pig2532 2006-9
int show_wwwguestlist()
{
    int i, rn;
    struct WWW_GUEST_S *wgentp;
    unsigned char ip[4];
    char ips[16];
    struct boardheader *bp = NULL;

    fill_wglist();
    move(2, 0);
    prints("\033[1;37;44m    序号 IP地址          登录时间                 所在版面");
    clrtoeol();
    prints("\033[m");
    rn = 3;
    for(i=0; i<BBS_PAGE_SIZE && i+page<range; i++) {
        move(rn, 0);
        rn++;
        wgentp = wg_record[i+page];
        memcpy(ip, &(wgentp->fromip), 4);
        sprintf(ips, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        bp = (struct boardheader *)getboard(wgentp->currentboard);
        prints("   %5d %-15s %-24s %s", i+page, ips, ctime(&(wgentp->logintime)), bp?bp->filename:"");
        clrtoeol();
    }
    clrtobot();
    return 0;
}

int show_userlist()
{
    char genbuf[5];


    /*
     * num_alcounter(); 
     */
    /*
     * if(!friendmode)
     * range=count_users;
     * else
     * range=count_friends; 
     */
    if (update_time + refreshtime < time(0)) {
        fill_userlist();
        update_time = time(0);
    }
    if (range == 0 /*||fill_userlist() == 0 */ ) {
        move(2, 0);
        prints("没有使用者（朋友）在列表中...\n");
        clrtobot();
        if (friendmode) {
            getdata(BBS_PAGESIZE + 3, 0, "是否转换成使用者模式 (Y/N)[Y]: ", genbuf, 4, DOECHO, NULL, true);
            move(BBS_PAGESIZE + 3, 0);
            clrtobot();
            if (genbuf[0] != 'N' && genbuf[0] != 'n') {
                range = num_visible_users();
                page = -1;
                friendmode = false;
                return 1;
            }
        } else
            pressanykey();
        return -1;
    }
    do_userlist();
    clrtobot();
    return 1;
}

int t_rusers(void){
    real_user_names = 1;
    t_users();
    real_user_names = 0;
    return 0;
}

int deal_key(ch, allnum, pagenum)
char ch;
int allnum, pagenum;
{
    char buf[STRLEN], genbuf[5], tmpid[20];
    int i, buflen;

    switch (ch) {
    case UL_CHANGE_NICK_UPPER:
        if(!strcmp(getCurrentUser()->userid,"guest"))
            break;
        strncpy(buf,getCurrentUser()->username,NAMELEN);
        getdata(t_lines-1,0,"永久变换昵称: ",buf,NAMELEN,DOECHO,NULL,false);
        if(!buf[0]||!strcmp(buf,getCurrentUser()->username))
            break;
        update_username(getCurrentUser()->userid,getCurrentUser()->username,buf);
        strncpy(getCurrentUser()->username,buf,NAMELEN);
        strncpy(uinfo.username,buf,NAMELEN);
        break;
    case UL_CHANGE_NICK_LOWER:
       if(!strcmp(getCurrentUser()->userid,"guest")) break;
       strncpy(buf,uinfo.username,NAMELEN);
       enableESC = true;
       getdata( BBS_PAGESIZE+3, 0, "临时变换昵称: ",buf,NAMELEN,DOECHO,NULL,false);
       enableESC = false;
       if(buf[0]!='\0')
       {	       
	    strncpy(uinfo.username,buf,NAMELEN);
	    UPDATE_UTMP_STR(username,uinfo);
       }	    
        break;	    
    case Ctrl('T'):
        showcolor = !showcolor;
        break;
    case 'k':
    case 'K':
        if (user_record[allnum]->logintime > update_time)
            return 1;
        strncpy(tmpid, user_record[allnum]->userid, sizeof(tmpid));
        tmpid[sizeof(tmpid) - 1] = '\0';
        if (!HAS_PERM(getCurrentUser(), PERM_SYSOP) && strcmp(getCurrentUser()->userid, tmpid))
            return 1;
        if (!strcmp(getCurrentUser()->userid, "guest"))
            return 1;           /* Leeward 98.04.13 */
        sprintf(buf, "你要把 %s 踢出站外吗 (Yes/No) [N]: ", tmpid);
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        getdata(BBS_PAGESIZE + 3, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (genbuf[0] != 'Y' && genbuf[0] != 'y') {
            update_endline();
            return 1;
        }
        if (user_record[allnum]->logintime > update_time || strcmp(user_record[allnum]->userid, tmpid) || !user_record[allnum]->active) {
            sprintf(buf, "%s 已经下线", tmpid);
        } else if (kick_user(user_record[allnum]->uid, user_record[allnum]->userid, user_record[allnum]) == 1) {
            sprintf(buf, "%s 已被踢出站外", tmpid);
        } else {
            sprintf(buf, "%s 无法踢出站外", tmpid);
        }
        show_message(buf);
        break;
    case 'h':
    case 'H':
        show_help("help/userlisthelp");
        break;
    case 'W':
    case 'w':
        if (showexplain == 1)
            showexplain = 0;

        else
            showexplain = 1;
        break;
    case 't':
    case 'T':
        if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
            return 1;
        if (strcmp(getCurrentUser()->userid, user_record[allnum]->userid))
            ttt_talk(user_record[allnum]);

        else
            return 1;
        break;
    case 'm':
    case 'M':
        if (HAS_PERM(getCurrentUser(), PERM_DENYMAIL)
        	||!HAS_PERM(getCurrentUser(), PERM_LOGINOK))
            return 1;
        m_send(user_record[allnum]->userid);
        break;
    case UL_SWITCH_FRIEND_LOWER:
    case UL_SWITCH_FRIEND_UPPER:
        if(!strcmp(getCurrentUser()->userid,"guest")) break;
        if (friendmode)
            friendmode = false;

        else
            friendmode = true;
        update_time = 0;
        break;
    case 's':
    case 'S':
        if (strcmp(user_record[allnum]->userid, "guest") && !HAS_PERM(getCurrentUser(), PERM_PAGE))
            return 1;
        if (!canmsg(getCurrentUser(), user_record[allnum])) {
            sprintf(buf, "%s 已经关闭讯息呼叫器", user_record[allnum]->userid);
            show_message(buf);
            break;
        }

        /*
         * 保存所发msg的目的uid 1998.7.5 by dong 
         */
        strcpy(getSession()->MsgDesUid, user_record[allnum]->userid);
#ifdef SMS_SUPPORT
	/*
	if(ch=='S')
	    do_send_sms_func(user_record[allnum]->userid, NULL);
	else
	*/
#endif
        do_sendmsg(user_record[allnum], NULL, 0);
        break;
#ifdef PERSONAL_CORP
    case 'x':
    case 'X':
        {
            struct userec *u;
            if (getuser(user_record[allnum]->userid, &u)) {
                if (u->flags & PCORP_FLAG) {
                    int oldmode = uinfo.mode;
                    modify_user_mode(PC);
                    pc_read(u->userid);
                    modify_user_mode(oldmode);
                }
            }
        }
        break;
#endif
    case 'o':
    case 'O':
		if (friendmode)
            return 0;
        if (!strcmp("guest", getCurrentUser()->userid))
            return 0;
        if (addtooverride(user_record[allnum]->userid) == -1) {
            sprintf(buf, "%s 已在朋友名单", user_record[allnum]->userid);
        }

        else {
            sprintf(buf, "%s 列入朋友名单", user_record[allnum]->userid);
        }
        show_message(buf);
        break;
    case 'd':
    case 'D':
        if (!strcmp("guest", getCurrentUser()->userid))
            return 0;

        /*
         * Leeward: 97.12.19: confirm removing operation 
         */
        sprintf(buf, "你要把 %s 从朋友名单移除吗 (Y/N) [N]: ", user_record[allnum]->userid);
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        getdata(BBS_PAGESIZE + 3, 0, buf, genbuf, 4, DOECHO, NULL, true);
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        if (genbuf[0] != 'Y' && genbuf[0] != 'y')
            return 0;
        if (deleteoverride(user_record[allnum]->userid) == -1) {
            sprintf(buf, "%s 本来就不在朋友名单中", user_record[allnum]->userid);
        }

        else {
            sprintf(buf, "%s 已从朋友名单移除", user_record[allnum]->userid);
        }
        show_message(buf);
        break;
    case '/':
        strcpy(buf, "输入所要查找的ID: ");
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        getdata(BBS_PAGESIZE + 3, 0, buf, genbuf, IDLEN+2, DOECHO, NULL, true);
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        buflen = strlen(genbuf);
        if (buflen>0)
        	for (i=0; i<range; i++)
        		if (strncasecmp(genbuf, user_record[i]->userid, buflen) == 0) {
        			num = i;
        			update_time = 0;
        			break;
        		}
    	break;
    default:
        return 0;
    }
    if (friendmode)
        modify_user_mode(FRIEND);
    else
        modify_user_mode(LUSERS);
    if (readplan == false) {
        print_title();
        clrtobot();
        if (show_userlist() == -1)
            return -1;
        update_endline();
    }
    return 1;
}

int utmp_key(ch, allnum, pagenum)
char ch;
int allnum, pagenum;
{
    return 0;
}

int wwwguest_key(ch, allnum, pagenum)
char ch;
int allnum, pagenum;
{
    return 0;
}

int deal_key2(ch, allnum, pagenum)
char ch;
int allnum, pagenum;


{
    char buf[STRLEN];

    switch (ch) {
    case 'h':
    case 'H':
        show_help("help/usershelp");
        break;
    case 'm':
    case 'M':
        if (!HAS_PERM(getCurrentUser(), PERM_POST))
            return 1;
        m_send(user_data[allnum - pagenum].userid);
        break;
    case 'o':
    case 'O':
        if (!strcmp("guest", getCurrentUser()->userid))
            return 0;
        if (addtooverride(user_data[allnum - pagenum].userid) == -1) {
            sprintf(buf, "%s 已在朋友名单", user_data[allnum - pagenum].userid);
            show_message(buf);
        }

        else {
            sprintf(buf, "%s 列入朋友名单", user_data[allnum - pagenum].userid);
            show_message(buf);
        }
        if (!friendmode)
            return 1;
        break;
    case 'W':
    case 'w':
        if (showexplain == 1)
            showexplain = 0;

        else
            showexplain = 1;
        break;
    case 'd':
    case 'D':
        if (!strcmp("guest", getCurrentUser()->userid))
            return 0;

        /*
         * Leeward: 97.12.19: confirm removing operation 
         */
        sprintf(buf, "你要把 %s 从朋友名单移除吗 (Y/N) [N]: ", user_data[allnum - pagenum].userid);
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        getdata(BBS_PAGESIZE + 3, 0, buf, genbuf, 4, DOECHO, NULL, true);
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        if (genbuf[0] != 'Y' && genbuf[0] != 'y')
            return 0;
        if (deleteoverride(user_data[allnum - pagenum].userid) == -1) {
            sprintf(buf, "%s 本来就不在朋友名单中", user_data[allnum - pagenum].userid);
            show_message(buf);
        }

        else {
            sprintf(buf, "%s 已从朋友名单移除", user_data[allnum - pagenum].userid);
            show_message(buf);
        }
        if (!friendmode)
            return 1;
        break;
    default:
        return 0;
    }
    modify_user_mode(LAUSERS);
    if (readplan == false) {
        print_title2();
        move(3, 0);
        clrtobot();
        if (Show_Users() == -1)
            return -1;
    }
    return 1;
}
int printuent(struct userec *uentp, void *arg)
{
    static int i;
    char permstr[USER_TITLE_LEN];
    int override;
    char fexp[30];

    if (uentp == NULL) {
        move(3, 0);
        printutitle();
        i = 0;
        return 0;
    }
    /*
    if (uentp->numlogins == 0 || uleveltochar(permstr, uentp) == 0)
        return 0;
	*/
    if (uentp->userid[0]==0)
	return 0;
    if (i < page || i >= page + BBS_PAGESIZE || i >= range) {
        i++;
        if (i >= page + BBS_PAGESIZE || i >= range)
            return QUIT;

        else
            return 0;
    }
    uleveltochar(permstr, uentp);
    user_data[i - page] = *uentp;
    override = myfriend(searchuser(uentp->userid), fexp, getSession());

    /*---	modified by period	2000-11-02	hide posts/logins	---*/
    prints(" %5d%2s%s%-14s%s %s%-19s%s  %5d %5d %4s   %-16s\n", i + 1,
           (override) ? "．" : "", (override) ? "\033[32m" : "", uentp->userid, (override) ? "\033[m" : "", (override && showexplain) ? "\033[31m" : "",
#if defined(ACTS_REALNAMES)
           uentp->realname,
#else                           /*  */
           (override && showexplain) ? fexp : uentp->username,
#endif                          /*  */
           (override && showexplain) ? "\033[m" : "",
           uentp->numlogins, uentp->numposts,
           permstr, Ctime(uentp->lastlogin));
    i++;
    usercounter++;
    return 0;
}
int countusers(struct userec *uentp, void *arg)
{
    if (uentp->userid[0]!=0)
        return COUNT;
    return 0;
}

int allusers()
{
    int count;

    if ((count = apply_users(countusers, NULL)) <= 0) {
        return 0;
    }
    return count;
}

int Show_Users()
{
    usercounter = 0;
    modify_user_mode(LAUSERS);
    printuent((struct userec *) NULL, 0);
    apply_users(printuent, NULL);
    clrtobot();
    return 0;
}
void setlistrange(int i)
{
    range = i;
}
int do_query(int star, int curr)
{
    clear();
    if (!user_record[curr]->active) {
        move(t_lines - 1, 0);
        prints("\033[44m该用户已离线\033[m");
    } else {
        t_query(user_record[curr]->userid);
        move(t_lines - 1, 0);
        if (DEFINE(getCurrentUser(), DEF_HIGHCOLOR))
            prints
                ("\x1b[m\x1b[44m聊天[\x1b[1;32mt\x1b[m\x1b[0;44m] 寄信[\x1b[1;32mm\x1b[m\x1b[0;44m] 送讯息[\x1b[1;32ms\x1b[m\x1b[0;44m] 加,减朋友[\x1b[1;32mo\x1b[m\x1b[0;44m,\x1b[1;32md\x1b[m\x1b[0;44m] 选择使用者[\x1b[1;32m↑\x1b[m\x1b[0;44m,\x1b[1;32m↓\x1b[m\x1b[0;44m] 切换模式 [\x1b[1;32mf\x1b[m\x1b[0;44m] 求救[\x1b[1;32mh\x1b[m\x1b[0;44m]");
        else
            prints("\x1b[44m聊天[t] 寄信[m] 送讯息[s] 加,减朋友[o,d] 选择使用者[↑,↓] 切换模式 [f] 求救[h]");
        clrtoeol();
        resetcolor();
    }
    return 0;
}

int utmp_query(int star, int curr)
{
    struct user_info *uentp;
    struct boardheader *bp = NULL;
    
    uentp = user_record[curr];
    clear();
    move(1, 0);
    bp = (struct boardheader *)getboard(uentp->currentboard);
    prints("%15s = %d\n", "用户号", uentp->uid);
    prints("%15s = %s\n", "用户名", uentp->userid);
    prints("%15s = %s\n", "真实姓名", uentp->realname);
    prints("%15s = %d %s\n", "进程号", uentp->pid, (uentp->pid==1)?"(歪脖)":"");
    prints("%15s = %s\n", "隐身状态", uentp->invisible?"ON":"OFF");
    prints("%15s = %s\n", "呼叫器状态", uentp->pager?"ON":"OFF");
    prints("%15s = %s\n", "聊天状态", uentp->in_chat?"正在聊天":"没有");
    prints("%15s = %s\n", "当前状态", ModeType(uentp->mode));
    prints("%15s = %s\n", "客户端IP地址", uentp->from);
    prints("%15s = %s", "登录时间", ctime(&(uentp->logintime)));
    prints("%15s = %s", "刷新时间", ctime(&(uentp->freshtime)));
    prints("%15s = %s\n\n\n", "所在版面", bp?bp->filename:"没有");
    clrtoeol();
    return 0;
}

int wwwguest_query(int star, int curr)
{
    clear();
    move(10, 10);
    prints("没有什么可供查询的信息。");
    return 0;
}

int do_query2(int star, int curr)
{
    t_query(user_data[curr - star].userid);
    move(t_lines - 1, 0);
    prints("\033[44m           寄信[m] 加,减朋友[o,d] 看说明档[→,r] 选择[↑,↓] 求救[h]           \033[m");
    return 0;
}

int Users(void){
#ifdef NEW_HELP
	int oldhelpmode=helpmode;
#endif
    range = allusers();
    modify_user_mode(LAUSERS);
    clear();
    user_data = (struct userec *) calloc(sizeof(struct userec), BBS_PAGESIZE);
#ifdef NEW_HELP
	helpmode=HELP_FRIEND;
#endif
    choose(false, 0, print_title2, deal_key2, Show_Users, do_query2);
#ifdef NEW_HELP
	helpmode=oldhelpmode;
#endif
    clear();
    free(user_data);
    return 0;
}

int t_friends(void){
    char genbuf[STRLEN];
#ifdef NEW_HELP
	int oldhelpmode=helpmode;
#endif

    modify_user_mode(FRIEND);
    friendmode = true;
    if (get_utmpent(getSession()->utmpent)->friendsnum==0) {
        move(1, 0);
        clrtobot();
        prints("你尚未利用 Talk -> Override 设定好友名单，所以...\n");
        pressanykey();
        return 0;
    }
    num_alcounter();
    range = count_friends;
    if (range == 0) {
        move(2, 0);
        clrtobot();
        prints("目前无好友上线\n");
        getdata(BBS_PAGESIZE + 3, 0, "是否转换成使用者模式 (Y/N)[N]: ", genbuf, 4, DOECHO, NULL, true);
        move(BBS_PAGESIZE + 3, 0);
        clrtobot();
        if (genbuf[0] == 'Y' || genbuf[0] == 'y') {
            range = num_visible_users();
            page = -1;
            friendmode = false;
            update_time = 0;
#ifdef NEW_HELP
			helpmode=HELP_FRIEND;
#endif
            choose(true, 0, print_title, deal_key, show_userlist, do_query);
#ifdef NEW_HELP
	helpmode=oldhelpmode;
#endif
            clear();
            return 0;
        }
    } else {
#ifdef NEW_HELP
		helpmode=HELP_FRIEND;
#endif
        update_time = 0;
        choose(true, 0, print_title, deal_key, show_userlist, do_query);
#ifdef NEW_HELP
	helpmode=oldhelpmode;
#endif
    }
    clear();
    friendmode = false;
    return 0;
}

int t_users(void){
#ifdef NEW_HELP
	int oldhelpmode=helpmode;
#endif
    friendmode = false;
    modify_user_mode(LUSERS);
    range = num_visible_users();
    if (range == 0) {
        move(3, 0);
        clrtobot();
        prints("目前无使用者上线\n");
    }
    update_time = 0;
#ifdef NEW_HELP
	helpmode=HELP_FRIEND;
#endif
    choose(true, 0, print_title, deal_key, show_userlist, do_query);
#ifdef NEW_HELP
	helpmode=oldhelpmode;
#endif
    clear();
    return 0;
}

int t_utmp(void){
    if(!HAS_PERM(getCurrentUser(), PERM_SEECLOAK)) {
        prints("您不具有查看该项目的权限（可见隐身）。");
        pressanykey();
    }
    range = num_visible_users();
    choose(true, 0, utmp_title, utmp_key, show_utmplist, utmp_query);
    return 0;
}

int t_wwwguest(void){
   struct public_data *publicshm;
   publicshm = get_publicshm();
   range = publicshm->www_guest_count;
   if(range == 0) {
       clear();
       move(5, 6);
       prints("目前没有www guest在线。");
       pressanykey();
   }
   else
       choose(true, 0, wwwguest_title, wwwguest_key, show_wwwguestlist, wwwguest_query);
   return 0;
}

int choose(int update, int defaultn, int (*title_show) (), int (*key_deal) (), int (*list_show) (), int (*read) ())
{
    int ch, number, deal;

    readplan = false;
    (*title_show) ();
    func_list_show = list_show;
    set_alarm(0, 0, NULL, NULL);
    if (update == 1)
        update_data(NULL);
    page = -1;
    number = 0;
    num = defaultn;
    while (1) {
        if (num <= 0)
            num = 0;
        if (num >= range)
            num = range - 1;
        if (page < 0 || freshmode == 1) {
            freshmode = 0;
            page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
            move(3, 0);
            clrtobot();
            if ((*list_show) () == -1)
                return -1;
            update_endline();
        }
        if (num < page || num >= page + BBS_PAGESIZE) {
            page = (num / BBS_PAGESIZE) * BBS_PAGESIZE;
            if ((*list_show) () == -1)
                return -1;
            update_endline();
            continue;
        }
        if (readplan == true) {
            if ((*read) (page, num) == -1)
                return num;
        }

        else {
            move(3 + num - page, 0);
            prints(">", number);
        }
        ch = igetkey();
        if (readplan == false)
            move(3 + num - page, 0);
        prints(" ");
        if (ch == 'q' || ch == 'e' || ch == KEY_LEFT || ch == EOF || ch == KEY_REFRESH) {
            if (readplan == true) {
                readplan = false;
                move(1, 0);
                clrtobot();
                if ((*list_show) () == -1)
                    return -1;
                (*title_show) ();
                continue;
            }
            break;
        }
        deal = (*key_deal) (ch, num, page);
        if (range == 0)
            break;
        if (deal == 1)
            continue;

        else if (deal == -1)
            break;
        switch (ch) {
        case Ctrl('Z'):
            r_lastmsg();        /* Leeward 98.07.30 support msgX */
            break;
        case 'P':
        case 'b':
        case Ctrl('B'):
        case KEY_PGUP:
            if (num == 0)
                num = range - 1;

            else
                num -= BBS_PAGESIZE;
            break;
        case ' ':
            if (readplan == true) {
                if (++num >= range)
                    num = 0;
                break;
            }
        case 'N':
        case Ctrl('F'):
        case KEY_PGDN:
            if (num == range - 1)
                num = 0;

            else
                num += BBS_PAGESIZE;
            break;
        case 'p':
        case 'l':
        case KEY_UP:
            if (num-- <= 0)
                num = range - 1;
            break;
        case 'n':
        case 'j':
        case KEY_DOWN:
            if (++num >= range)
                num = 0;
            break;
        case '$':
        case KEY_END:
            num = range - 1;
            break;
        case KEY_HOME:
            num = 0;
            break;
        case '\n':
        case '\r':
            if (number > 0) {
                num = number - 1;
                break;
            }

            /*
             * fall through 
             */
        case 'r':
        case KEY_RIGHT:
            {
                if (readplan == true) {
                    if (++num >= range)
                        num = 0;
                }

                else
                    readplan = true;
                break;
            }
        default:
            ;
        }
        if (ch >= '0' && ch <= '9') {
            number = number * 10 + (ch - '0');
            ch = '\0';
        } else {
            number = 0;
        }
    }
    set_alarm(0, 0, NULL, NULL);
    return -1;
}

int display_publicshm(void){
    struct public_data *publicshm;
    publicshm = get_publicshm();
    clear();
    move(1, 0);
    prints("  \033[1;33m当前系统状态\033[m\n\n");
    prints("    系统时间 : \033[1;32m%s\033[m", ctime(&(publicshm->nowtime)));
    prints("    系统缓存版本 : \033[1;32m%d\033[m\n", publicshm->sysconfimg_version);
    prints("    www guest数量 : \033[1;32m%d\033[m\n", publicshm->www_guest_count);
    prints("    最高用户数 : \033[1;32m%d\033[m\n", publicshm->max_user);
    prints("    最高www guest数 : \033[1;32m%d\033[m\n", publicshm->max_wwwguest);
    prints("    上站计数 : \033[1;32m%d\033[m\n", publicshm->logincount);
    prints("    离站计数 : \033[1;32m%d\033[m\n", publicshm->logoutcount);
    prints("    www上站计数 : \033[1;32m%d\033[m\n", publicshm->wwwlogincount);
    prints("    www离站计数 : \033[1;32m%d\033[m\n", publicshm->wwwlogoutcount);
    prints("    www guest上站计数 : \033[1;32m%d\033[m\n", publicshm->wwwguestlogincount);
    prints("    www guest离站计数 : \033[1;32m%d\033[m\n", publicshm->wwwguestlogoutcount);
    pressanykey();
    return 0;
}

