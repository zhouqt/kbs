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
extern char MsgDesUid[14];      /* ±£¥ÊÀ˘∑¢msgµƒƒøµƒuid 1998.7.5 by dong */
int t_users();
int Show_Users();
int print_title()
{
    if (DEFINE(currentuser, DEF_HIGHCOLOR))
        docmdtitle((friendmode) ? "[∫√≈Û”—¡–±Ì]" : "[ π”√’ﬂ¡–±Ì]",
                   " ¡ƒÃÏ[\x1b[1;32mt\x1b[m] ºƒ–≈[\x1b[1;32mm\x1b[m] ÀÕ—∂œ¢[\x1b[1;32ms\x1b[m] º”,ºı≈Û”—[\x1b[1;32mo\x1b[m,\x1b[1;32md\x1b[m] ø¥Àµ√˜µµ[\x1b[1;32m°˙\x1b[m,\x1b[1;32mr\x1b[m] «–ªªƒ£ Ω [\x1b[1;32mf\x1b[m] «Ûæ»[\x1b[1;32mh\x1b[m]");
    else
        docmdtitle((friendmode) ? "[∫√≈Û”—¡–±Ì]" : "[ π”√’ﬂ¡–±Ì]", " ¡ƒÃÏ[t] ºƒ–≈[m] ÀÕ—∂œ¢[s] º”,ºı≈Û”—[o,d] ø¥Àµ√˜µµ[°˙,r] «–ªªƒ£ Ω [f] «Ûæ»[h]");
    update_endline();
    return 0;
}

int print_title2()
{
    if (DEFINE(currentuser, DEF_HIGHCOLOR))
        docmdtitle((friendmode) ? "[∫√≈Û”—¡–±Ì]" : "[ π”√’ﬂ¡–±Ì]",
                   "          ºƒ–≈[\x1b[1;32mm\x1b[m] º”,ºı≈Û”—[\x1b[1;32mo\x1b[m,\x1b[1;32md\x1b[m] ø¥Àµ√˜µµ[\x1b[1;32m°˙\x1b[m,\x1b[1;32mr\x1b[m] —°‘Ò[\x1b[1;32m°¸\x1b[m,\x1b[1;32m°˝\x1b[m] «Ûæ»[\x1b[1;32mh\x1b[m]");
    else
        docmdtitle((friendmode) ? "[∫√≈Û”—¡–±Ì]" : "[ π”√’ﬂ¡–±Ì]", "          ºƒ–≈[m] º”,ºı≈Û”—[o,d] ø¥Àµ√˜µµ[°˙,r] —°‘Ò[°¸,°˝] «Ûæ»[h]");
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
         * ∑Ò‘Ú‘⁄÷¥––“ª–©◊”≥Ã Ω ±£¨ª≠√Ê“≤ª·∏¸–¬ 
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
    field_2 = " π”√’ﬂÍ«≥∆";
    if (real_user_names)
        field_2 = "’Ê µ–’√˚  ";
    sprintf(title_str,
             /*---	modified by period	2000-10-21	‘⁄œﬂ”√ªß ˝ø…“‘¥Û”⁄1000µƒ
                     "[44m%s%-12.12s %-16.16s %-16.16s %c %c %-16.16s %5s[m\n",
             ---*/
            "[44m %s%-12.12s %-16.16s %-16.16s %c %c %-15.15s %5s", "±‡∫≈  ", " π”√’ﬂ¥˙∫≈", (showexplain == 1) ? "∫√”—Àµ√˜ªÚ¥˙∫≈" : field_2, "¿¥◊‘", 'P',
            /*
             * (HAS_PERM(currentuser,PERM_SYSOP) ? 'C' : ' ') 
             */ 'M', "∂ØÃ¨",
#ifdef SHOW_IDLE_TIME
            " ±:∑÷");

#else                           /*  */
            "");

#endif                          /*  */
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
        prints("[1m%s[m", msg);
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
    if (!HAS_PERM(currentuser, PERM_SEECLOAK) && uentp->invisible && strcmp(uentp->userid, currentuser->userid)) {      /*Haohmaru.99.4.24.»√“˛…Ì’ﬂƒ‹ø¥º˚◊‘º∫ */
        return 0;
    }
    if (friendmode && !myfriend(uentp->uid, NULL)) {
        return 0;
    }
    user_record[*count] = uentp;
    (*count)++;
    return COUNT;
}

#ifdef NINE_BUILD

//‘ˆº”∞¥ip≈≈–Ú£¨shiyao 2003.5.31

int SortBy = 0;

void sort_user_record(left, right)
int left, right;
{
    int i, last;

    if (left >= right)
        return;
    swap_user_record(left, (left + right) / 2);
    last = left;
    if (SortBy == 0) {
    	for (i = left + 1; i <= right; i++)
       	 if (strcasecmp(user_record[i]->userid, user_record[left]->userid) < 0)
            		swap_user_record(++last, i);
    } else 
    	for (i = left + 1; i <= right; i++)
       	 if (ntohl(inet_addr(user_record[i]->from)) < ntohl(inet_addr(user_record[left]->from)))
            		swap_user_record(++last, i);
    swap_user_record(left, last);
    sort_user_record(left, last - 1);
    sort_user_record(last + 1, right);
}
#endif
int fill_userlist()
{
    static int i, i2;


    /*
     * struct      user_info *not_good; 
     */
    i2 = 0;
    if (!friendmode) {
        apply_ulist_addr((APPLY_UTMP_FUNC) full_utmp, (char *) &i2);
#ifdef NINE_BUILD
//changed by shiyao, 2003.6.1
        numf = 0;
        for (i = 0; i < i2; i++) {
            if (myfriend(user_record[i]->uid, NULL)) {
                swap_user_record(numf++, i);
            }
        }
	sort_user_record(0, numf-1);
	sort_user_record(numf, i2-1);
#endif
    } else {
        struct user_info* u;
	u=get_utmpent(utmpent);
        for (i = 0; i < u->friendsnum; i++) {
            if (u->friends_uid[i])
                apply_utmpuid((APPLY_UTMP_FUNC) full_utmp, u->friends_uid[i], (char *) &i2);
    }
 #ifdef NINE_BUILD
 	if (i2>0) sort_user_record(0, i2-1);
 #endif
   } range = i2;
    return i2 == 0 ? -1 : 1;
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
        *isfriend = hisfriend(usernum, uin);
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
    int i,j,y,x;
    char user_info_str[256 /*STRLEN*2 */ ], pagec;
    char tbuf[80];
    int override;
    extern bool disable_move;
    extern bool disable_color;
    char fexp[30];
    struct user_info uentp;
#ifdef NINE_BUILD
  #define FRIENDSIG "°ı"
    char *p;
    int padding_count;
  #define FROMSTR uentp.from   
#else    
  #define FRIENDSIG "£Æ"
  #define FROMSTR "*"  
#endif    

    /*
     * _SHOW_ONLINE_USER 
     */
    /*
     * to print on line user to a file 
     */
    /*
     * char online_users[STRLEN+10];
     * 
     * if(!strcmp(currentuser->userid,"guest")){
     * fd=open("onlineulist",O_RDWR|O_TRUNC, 0600);
     * if(fd!=-1)
     * {
     * flock(fd,LOCK_EX);
     * for(i=0; i<range ; i++)
     * {
     * uentp=user_record[i];
     * len = sprintf(online_users, " %3d %-12.12s %-24.24s %-20.20s %-17.17s %5.5s\n", i+1,uentp->userid,uentp->username,uentp->from,modestring(uentp->mode, uentp->destuid, 0, (uentp->in_chat ? uentp->chatid : NULL)),uentp->invisible? "#":" ");
     * write(fd,online_users,len);
     * }
     * flock(fd,LOCK_UN);
     * close(fd);
     * }
     * }
     */
    /*
     * end of this insertion 
     */

    /*
     * end of this insertion 
     */
    move(3, 0);
    print_user_info_title();
    for (i = 0; i < BBS_PAGESIZE && i + page < range; i++) {
        int isfriend;

        isfriend = -1;
        if (user_record[i + page] == NULL) {
            clear();
            prints("[1;31mœµÕ≥≥ˆœ÷Bug,«ÎµΩSysop∞Ê±®∏Ê£¨–ª–ª![m");
            oflush();
            sleep(10);
            exit(0);
        }
        uentp = *(user_record[i + page]);
        if (!uentp.active || !uentp.pid) {
            prints(" %4d ∞°,Œ“∏’◊ﬂ\n", i + 1 + page);
            continue;
        }
        if (!showexplain)
            override = (i + page < numf) || friendmode;

        else {
#ifdef NINE_BUILD
            override = myfriend(uentp.uid, fexp);
#else
            if ((i + page < numf) || friendmode)
                override = myfriend(uentp.uid, fexp);
            else
                override = false;
#endif
        }
        if (readplan == true) {
            return 0;
        }
        pagec = pagerchar(usernum,&uentp, uentp.pager, &isfriend);
        strncpy(tbuf, (real_user_names) ? uentp.realname : (showexplain && override) ? fexp : uentp.username, 80);
//Í«≥∆‘⁄¡–±Ì÷–◊Ó∫Û“ª◊÷œ˚≥˝¬“¬Î£¨shiyao  2003.6.1
//	j = 15;
//	while (j>=0 && tbuf[j]<0) j--;
//	if ((15-j)%2)  tbuf[15] = 0;
        tbuf[80]=0;
        resetcolor();
        clrtoeol();
        getyx(&y, &x);
        move(y, 20);
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
                i + 1 + page, (override) ? (uentp.invisible ? "££" : FRIENDSIG) : (uentp.invisible ? "£™" : ""),
                (override) ? "[1;32m" : "", uentp.userid, (override) ? "[m" : "", 
                (override && showexplain) ? "[1;31m" : "");
        prints("%s", user_info_str);
        resetcolor();
        move(y, 36);
        sprintf(user_info_str, " %-16.16s %c %c %s%-16.16s[m%5.5s\n",  
                (((pagec == ' ' || pagec == 'O')) || HAS_PERM(currentuser, PERM_SYSOP)) ? uentp.from : FROMSTR,
                pagec, msgchar(&uentp, &isfriend), 
                (uentp.invisible == true)? "[34m" : "", modestring(uentp.mode, uentp.destuid, 0,        /* 1->0 ≤ªœ‘ æ¡ƒÃÏ∂‘œÛµ» modified by dong 1996.10.26 */
                                           (uentp.in_chat ? uentp.chatid : NULL)),            
#ifdef SHOW_IDLE_TIME
                idle_str(&uentp));

#else                           /*  */
                "");

#endif                          /*  */
        prints("%s", user_info_str);
        resetcolor();
    }
#undef FROMSTR    
#undef FRIENDSIG    
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
        prints("√ª”– π”√’ﬂ£®≈Û”—£©‘⁄¡–±Ì÷–...\n");
        clrtobot();
        if (friendmode) {
            getdata(BBS_PAGESIZE + 3, 0, " «∑Ò◊™ªª≥… π”√’ﬂƒ£ Ω (Y/N)[Y]: ", genbuf, 4, DOECHO, NULL, true);
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

void t_rusers()
{
    real_user_names = 1;
    t_users();
    real_user_names = 0;
}


int deal_key(ch, allnum, pagenum)
char ch;
int allnum, pagenum;
{
    char buf[STRLEN], genbuf[5];
    extern bool enableESC;
    int i, buflen;

    switch (ch) {
#ifdef HAVE_TEMPORARY_NICK
    case UL_CHANGE_NICK_LOWER:
    case UL_CHANGE_NICK_UPPER:
       if(!strcmp(currentuser->userid,"guest")) break;
       strncpy(buf,uinfo.username,NAMELEN);
       enableESC = true;
       getdata( BBS_PAGESIZE+3, 0, "±‰ªªÍ«≥∆: ",buf,NAMELEN,DOECHO,NULL,false);
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
#endif
#ifdef NINE_BUILD
	case 'i' : SortBy = 1-SortBy;
	           update_time = 0;
	           break;
#endif
    case 'k':
    case 'K':
        if (!HAS_PERM(currentuser, PERM_SYSOP) && strcmp(currentuser->userid, user_record[allnum]->userid))
            return 1;
        if (!strcmp(currentuser->userid, "guest"))
            return 1;           /* Leeward 98.04.13 */
        sprintf(buf, "ƒ„“™∞— %s Ãﬂ≥ˆ’æÕ‚¬ (Yes/No) [N]: ", user_record[allnum]->userid);
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        getdata(BBS_PAGESIZE + 3, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (genbuf[0] != 'Y' && genbuf[0] != 'y') {
            return 1;
        }
        if (kick_user(user_record[allnum]) == 1) {
            sprintf(buf, "%s “—±ªÃﬂ≥ˆ’æÕ‚", user_record[allnum]->userid);
        }

        else {
            sprintf(buf, "%s Œﬁ∑®Ãﬂ≥ˆ’æÕ‚", user_record[allnum]->userid);
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
        if (!HAS_PERM(currentuser, PERM_PAGE))
            return 1;
        if (strcmp(currentuser->userid, user_record[allnum]->userid))
            ttt_talk(user_record[allnum]);

        else
            return 1;
        break;
    case 'm':
    case 'M':
        if (HAS_PERM(currentuser, PERM_DENYMAIL)
        	||!HAS_PERM(currentuser, PERM_LOGINOK))
            return 1;
        m_send(user_record[allnum]->userid);
        break;
    case UL_SWITCH_FRIEND_LOWER:
    case UL_SWITCH_FRIEND_UPPER:
        if(!strcmp(currentuser->userid,"guest")) break;
        if (friendmode)
            friendmode = false;

        else
            friendmode = true;
        update_time = 0;
        break;
    case 's':
    case 'S':
        if (strcmp(user_record[allnum]->userid, "guest") && !HAS_PERM(currentuser, PERM_PAGE))
            return 1;
        if (!canmsg(currentuser, user_record[allnum])) {
            sprintf(buf, "%s “—æ≠πÿ±’—∂œ¢∫ÙΩ–∆˜", user_record[allnum]->userid);
            show_message(buf);
            break;
        }

        /*
         * ±£¥ÊÀ˘∑¢msgµƒƒøµƒuid 1998.7.5 by dong 
         */
        strcpy(MsgDesUid, user_record[allnum]->userid);
#ifdef SMS_SUPPORT
	if(ch=='S')
	    do_send_sms_func(user_record[allnum]->userid, NULL);
	else
#endif
        do_sendmsg(user_record[allnum], NULL, 0);
        break;
    case 'o':
    case 'O':
        if (!strcmp("guest", currentuser->userid))
            return 0;
        if (addtooverride(user_record[allnum]->userid) == -1) {
            sprintf(buf, "%s “—‘⁄≈Û”—√˚µ•", user_record[allnum]->userid);
        }

        else {
            sprintf(buf, "%s ¡–»Î≈Û”—√˚µ•", user_record[allnum]->userid);
        }
        show_message(buf);
        break;
    case 'd':
    case 'D':
        if (!strcmp("guest", currentuser->userid))
            return 0;

        /*
         * Leeward: 97.12.19: confirm removing operation 
         */
        sprintf(buf, "ƒ„“™∞— %s ¥”≈Û”—√˚µ•“∆≥˝¬ (Y/N) [N]: ", user_record[allnum]->userid);
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        getdata(BBS_PAGESIZE + 3, 0, buf, genbuf, 4, DOECHO, NULL, true);
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        if (genbuf[0] != 'Y' && genbuf[0] != 'y')
            return 0;
        if (deleteoverride(user_record[allnum]->userid) == -1) {
            sprintf(buf, "%s ±æ¿¥æÕ≤ª‘⁄≈Û”—√˚µ•÷–", user_record[allnum]->userid);
        }

        else {
            sprintf(buf, "%s “—¥”≈Û”—√˚µ•“∆≥˝", user_record[allnum]->userid);
        }
        show_message(buf);
        break;
    case '/':
        strcpy(buf, " ‰»ÎÀ˘“™≤È’“µƒID: ");
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
        if (!HAS_PERM(currentuser, PERM_POST))
            return 1;
        m_send(user_data[allnum - pagenum].userid);
        break;
    case 'o':
    case 'O':
        if (!strcmp("guest", currentuser->userid))
            return 0;
        if (addtooverride(user_data[allnum - pagenum].userid) == -1) {
            sprintf(buf, "%s “—‘⁄≈Û”—√˚µ•", user_data[allnum - pagenum].userid);
            show_message(buf);
        }

        else {
            sprintf(buf, "%s ¡–»Î≈Û”—√˚µ•", user_data[allnum - pagenum].userid);
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
        if (!strcmp("guest", currentuser->userid))
            return 0;

        /*
         * Leeward: 97.12.19: confirm removing operation 
         */
        sprintf(buf, "ƒ„“™∞— %s ¥”≈Û”—√˚µ•“∆≥˝¬ (Y/N) [N]: ", user_data[allnum - pagenum].userid);
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        getdata(BBS_PAGESIZE + 3, 0, buf, genbuf, 4, DOECHO, NULL, true);
        move(BBS_PAGESIZE + 3, 0);
        clrtoeol();
        if (genbuf[0] != 'Y' && genbuf[0] != 'y')
            return 0;
        if (deleteoverride(user_data[allnum - pagenum].userid) == -1) {
            sprintf(buf, "%s ±æ¿¥æÕ≤ª‘⁄≈Û”—√˚µ•÷–", user_data[allnum - pagenum].userid);
            show_message(buf);
        }

        else {
            sprintf(buf, "%s “—¥”≈Û”—√˚µ•“∆≥˝", user_data[allnum - pagenum].userid);
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
int printuent(struct userec *uentp, char *arg)
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
    override = myfriend(searchuser(uentp->userid), fexp);

    /*---	modified by period	2000-11-02	hide posts/logins	---*/
#ifdef _DETAIL_UINFO_
    prints(" %5d%2s%s%-14s%s %s%-19s%s  %5d %5d %4s   %-16s\n", i + 1,
#else                           /*  */
    if (HAS_PERM(currentuser, PERM_ADMINMENU))
        sprintf(buf, "%5d %5d", uentp->numlogins, uentp->numposts);
    prints(" %5d%2s%s%-14s%s %s%-19s%s  %11s %4s   %-16s\n", i + 1,
#endif                          /*  */
           (override) ? "£Æ" : "", (override) ? "[32m" : "", uentp->userid, (override) ? "[m" : "", (override && showexplain) ? "[31m" : "",
#if defined(ACTS_REALNAMES)
           uentp->realname,
#else                           /*  */
           (override && showexplain) ? fexp : uentp->username,
#endif                          /*  */
           (override && showexplain) ? "[m" : "",
#ifdef _DETAIL_UINFO_
           uentp->numlogins, uentp->numposts,
#else                           /*  */
           buf,
#endif                          /*  */
           permstr, Ctime(uentp->lastlogin));
    i++;
    usercounter++;
    return 0;
}
int countusers(struct userec *uentp, char *arg)
{
    if (uentp->userid[0]!=0)
        return COUNT;
    return 0;
}

int allusers()
{
    int count;

    if ((count = apply_users(countusers, 0)) <= 0) {
        return 0;
    }
    return count;
}

int Show_Users()
{
    usercounter = 0;
    modify_user_mode(LAUSERS);
    printuent((struct userec *) NULL, 0);
    apply_users(printuent, 0);
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
        prints("[44m∏√”√ªß“—¿Îœﬂ[m");
    } else {
        t_query(user_record[curr]->userid);
        move(t_lines - 1, 0);
        if (DEFINE(currentuser, DEF_HIGHCOLOR))
            prints
                ("\x1b[m\x1b[44m¡ƒÃÏ[\x1b[1;32mt\x1b[m\x1b[0;44m] ºƒ–≈[\x1b[1;32mm\x1b[m\x1b[0;44m] ÀÕ—∂œ¢[\x1b[1;32ms\x1b[m\x1b[0;44m] º”,ºı≈Û”—[\x1b[1;32mo\x1b[m\x1b[0;44m,\x1b[1;32md\x1b[m\x1b[0;44m] —°‘Ò π”√’ﬂ[\x1b[1;32m°¸\x1b[m\x1b[0;44m,\x1b[1;32m°˝\x1b[m\x1b[0;44m] «–ªªƒ£ Ω [\x1b[1;32mf\x1b[m\x1b[0;44m] «Ûæ»[\x1b[1;32mh\x1b[m\x1b[0;44m]");
        else
            prints("\x1b[44m¡ƒÃÏ[t] ºƒ–≈[m] ÀÕ—∂œ¢[s] º”,ºı≈Û”—[o,d] —°‘Ò π”√’ﬂ[°¸,°˝] «–ªªƒ£ Ω [f] «Ûæ»[h]");
        clrtoeol();
        resetcolor();
    }
    return 0;
}
int do_query2(int star, int curr)
{
    t_query(user_data[curr - star].userid);
    move(t_lines - 1, 0);
    prints("[44m           ºƒ–≈[m] º”,ºı≈Û”—[o,d] ø¥Àµ√˜µµ[°˙,r] —°‘Ò[°¸,°˝] «Ûæ»[h]           [m");
    return 0;
}

void Users()
{
    range = allusers();
    modify_user_mode(LAUSERS);
    clear();
    user_data = (struct userec *) calloc(sizeof(struct userec), BBS_PAGESIZE);
    choose(false, 0, print_title2, deal_key2, Show_Users, do_query2);
    clear();
    free(user_data);
    return;
}

int t_friends()
{
    FILE *fp;
    char genbuf[STRLEN];

    modify_user_mode(FRIEND);
    friendmode = true;
    if (get_utmpent(utmpent)->friendsnum==0) {
        move(1, 0);
        clrtobot();
        prints("ƒ„…–Œ¥¿˚”√ Talk -> Override …Ë∂®∫√”—√˚µ•£¨À˘“‘...\n");
        pressanykey();
        return 0;
    }
    num_alcounter();
    range = count_friends;
    if (range == 0) {
        move(2, 0);
        clrtobot();
        prints("ƒø«∞Œﬁ∫√”—…œœﬂ\n");
        getdata(BBS_PAGESIZE + 3, 0, " «∑Ò◊™ªª≥… π”√’ﬂƒ£ Ω (Y/N)[N]: ", genbuf, 4, DOECHO, NULL, true);
        move(BBS_PAGESIZE + 3, 0);
        clrtobot();
        if (genbuf[0] == 'Y' || genbuf[0] == 'y') {
            range = num_visible_users();
            page = -1;
            friendmode = false;
            update_time = 0;
            choose(true, 0, print_title, deal_key, show_userlist, do_query);
            clear();
            return 0;
        }
    } else {
        update_time = 0;
        choose(true, 0, print_title, deal_key, show_userlist, do_query);
    }
    clear();
    friendmode = false;
    return 0;
}

int t_users()
{
    friendmode = false;
    modify_user_mode(LUSERS);
    range = num_visible_users();
    if (range == 0) {
        move(3, 0);
        clrtobot();
        prints("ƒø«∞Œﬁ π”√’ﬂ…œœﬂ\n");
    }
    update_time = 0;
    choose(true, 0, print_title, deal_key, show_userlist, do_query);
    clear();
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
