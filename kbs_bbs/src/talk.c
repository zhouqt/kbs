/*    Pirate Bulletin Board System
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
#ifdef lint
#include <sys/uio.h>
#endif                          /* 
                                 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define M_INT 8                 /* monitor mode update interval */
#define P_INT 20                /* interval to check for page req. in talk/chat */
extern int numf, friendmode;
int talkidletime = 0;
int ulistpage;
int friend_query();
int friend_mail();
int friend_dele();
int friend_add();
int friend_edit();
int friend_help();
int badlist();                  /* Bigman 2000.12.26 */
static int do_talk(int fd);

/* Bigman 2000.9.15 TalkµÄ¼ÇÂ¼ */
#ifdef TALK_LOG
void do_log();
int talkrec = -1;
char partner[IDLEN + 1];
#endif                          /* 
                                 */
struct one_key friend_list[] = {
    {'r', friend_query},
    {'m', friend_mail},
    {'M', friend_mail},
    {'a', friend_add},
    {'A', friend_add},
    {'d', friend_dele},
    {'D', friend_dele},
    {'E', friend_edit},
    {'h', friend_help},
    {'H', friend_help},
    {'\0', NULL},
};
struct talk_win {

    int curcol, curln;

    int sline, eline;
};
extern int t_columns;
char *talk_uent_buf;

/* begin - jjyang */
char save_page_requestor[STRLEN];

/* end - jjyang */

/*---	changed to isidhidden by period	2000-10-20	---*
int
ishidden(user)
char *user;
{
    int tuid;
    struct user_info uin;
    if (!(tuid = getuser(user))) return 0;
    search_ulist( &uin, t_cmpuids, tuid );
    return( uin.invisible );
}
---*/
char canpage(friend, pager)
int friend, pager;
{
    if ((pager & ALL_PAGER) || HAS_PERM(currentuser, PERM_SYSOP))
        return true;
    if ((pager & FRIEND_PAGER)) {
        if (friend)
            return true;
    }
    return false;
}
int listcuent(struct user_info *uentp, char *arg, int pos)
{
    if (uentp == NULL) {
        CreateNameList();
        return 0;
    }
    /*
     * if(uentp->uid == usernum)             rem by Haohmaru,00.5.26,ÕâÑù²ÅÄÜ¸ø×Ô¼º·¢msg
     * return 0;
     */
    if (!uentp->active || !uentp->pid)
        return 0;
    if (uentp->mode == ULDL)
        return 0;
    if (!HAS_PERM(currentuser, PERM_SEECLOAK) && uentp->invisible)
        return 0;
    AddNameList(uentp->userid);
    return 0;
}

void creat_list()
{
    listcuent(NULL, 0, 0);
    apply_ulist_addr((APPLY_UTMP_FUNC) listcuent, 0);
}

int t_pager()
{
    if (uinfo.pager & ALL_PAGER) {
        uinfo.pager &= ~ALL_PAGER;
        if (DEFINE(currentuser, DEF_FRIENDCALL))
            uinfo.pager |= FRIEND_PAGER;
        else
            uinfo.pager &= ~FRIEND_PAGER;
    } else {
        uinfo.pager |= ALL_PAGER;
        uinfo.pager |= FRIEND_PAGER;
    }
    if (!uinfo.in_chat && uinfo.mode != TALK) {
        move(1, 0);
        prints("ÄúµÄºô½ÐÆ÷ (pager) ÒÑ¾­[1m%s[mÁË!", (uinfo.pager & ALL_PAGER) ? "´ò¿ª" : "¹Ø±Õ");
        pressreturn();
    }
    UPDATE_UTMP(pager, uinfo);
    return 0;
}

/*Add by SmallPig*/
/*´Ëº¯ÊýÖ»¸ºÔðÁÐÓ¡ËµÃ÷µµ£¬²¢²»¹ÜÇå³ý»ò¶¨Î»µÄÎÊÌâ¡£*/
extern int minln; //added by bad

int show_user_plan(userid)
char userid[IDLEN];
{
    int i;
    char pfile[STRLEN], pbuf[256];
    FILE *pf;

    sethomefile(pfile, userid, "plans");
    if ((pf = fopen(pfile, "r")) == NULL) {
        prints("[36mÃ»ÓÐ¸öÈËËµÃ÷µµ[m\n");
/*fclose(pf); *//*
 * * * Leeward 98.04.20 
 */
        return false;
    } else {
        prints("[36m¸öÈËËµÃ÷µµÈçÏÂ£º[m\n");//added by bad
        getyx(&minln, &i);
        for (i = 1; i <= scr_lns-8/*MAXQUERYLINES*/; i++) {
            if (fgets(pbuf, sizeof(pbuf), pf))
                prints("%s", pbuf);
            else
                break;
        }
        fclose(pf);
        minln=0;
        return true;
    }
}
int t_printstatus(struct user_info *uentp, int *arg, int pos)
{
    char buf[80],buf2[20];
    char* p;
    if (uentp->invisible == 1) {
        if (!HAS_PERM(currentuser, PERM_SEECLOAK))
            return COUNT;
    }
    (*arg)++;
    if (*arg == 1)
        strcpy(genbuf, "Ä¿Ç°ÔÚÕ¾ÉÏ£¬×´Ì¬ÈçÏÂ£º\n");

    p = idle_str(uentp);
    if(p[0]==' '&&p[1]==' ') buf2[0]=0;
    else sprintf(buf2, "[%s]", p);
    sprintf(buf, "%s[1m%s[m%s ", uentp->invisible?"[32m":"", modestring(uentp->mode, uentp->destuid, 0,   /* 1->0 ²»ÏÔÊ¾ÁÄÌì¶ÔÏóµÈ modified by dong 1996.10.26 */
                                          (uentp->in_chat ? uentp->chatid : NULL)), buf2);
    strcat(genbuf, buf);

    if ((*arg) % 8 == 0)
        strcat(genbuf, "\n");
    UNUSED_ARG(pos);
    return COUNT;
}

/* Modified By Excellent*/
struct _tag_talk_showstatus {
    int count;
    int pos[20];
};
int talk_showstatus(struct user_info *uentp, struct _tag_talk_showstatus *arg, int pos)
{
    char buf[80];

    if (uentp->invisible && !HAS_PERM(currentuser, PERM_SEECLOAK))
        return 0;
    arg->pos[arg->count++] = pos;
    sprintf(buf, "(%d) Ä¿Ç°×´Ì¬: %s, À´×Ô: %s \n", arg->count, modestring(uentp->mode, uentp->destuid, 0,       /* 1->0 ²»ÏÔÊ¾ÁÄÌì¶ÔÏóµÈ modified by dong 1996.10.26 */
                                                                          uentp->in_chat ? uentp->chatid : NULL), uentp->from);
    strcat(genbuf, buf);
    return COUNT;
}

int t_cmpuids(int uid, struct user_info *up)
{
    return (up->active && uid == up->uid);
}

extern char MsgDesUid[20];

int t_query(q_id)
char q_id[IDLEN];
{
    char uident[STRLEN], *newline;
    int tuid = 0;
    int exp, perf;              /*Add by SmallPig */
    char qry_mail_dir[STRLEN];
    char planid[IDLEN + 2];
    char permstr[USER_TITLE_LEN];
    char exittime[40];
    time_t exit_time, temp /*Haohmaru.98.12.04 */ ;
    int logincount, seecount, oldmode;
    struct userec *lookupuser;

    if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS && uinfo.mode != FRIEND && uinfo.mode != READING && uinfo.mode != MAIL && uinfo.mode != RMAIL && uinfo.mode != GMENU) {
        /*
         * count = shortulist(NULL); 
         */
        oldmode = uinfo.mode;
        modify_user_mode(QUERY);
        move(2, 0);
        clrtobot();
        prints("<ÊäÈëÊ¹ÓÃÕß´úºÅ, °´¿Õ°×¼ü¿ÉÁÐ³ö·ûºÏ×Ö´®>\n");
        move(1, 0);
        clrtoeol();
        prints("²éÑ¯Ë­: ");
        usercomplete(NULL, uident);
        if (uident[0] == '\0') {
            clear();
            modify_user_mode(oldmode);
            return 0;
        }
        modify_user_mode(oldmode);
    } else {
        char *p;

        for (p = q_id; *p; p++)
            if (*p == ' ') {
                *p = 0;
                break;
            };
        strcpy(uident, q_id);
    }
    oldmode = uinfo.mode;
    modify_user_mode(QUERY);
    if (!(tuid = getuser(uident, &lookupuser))) {
        move(2, 0);
        clrtoeol();
        prints("[1m²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ[m\n");
        pressanykey();
        move(2, 0);
        clrtoeol();
        modify_user_mode(oldmode);
        return -1;
    }
    uinfo.destuid = tuid;
/*    UPDATE_UTMP(destuid,uinfo);  I think it is not very importance.KCN*/
/*    search_ulist( &uin, t_cmpuids, tuid );*/
    move(1, 0);
    clrtobot();
    setmailfile(qry_mail_dir, lookupuser->userid, DOT_DIR);
    exp = countexp(lookupuser);
    perf = countperf(lookupuser);
    /*---	modified by period	2000-11-02	hide posts/logins	---*/
#ifndef _DETAIL_UINFO_
    if ((!HAS_PERM(currentuser, PERM_ADMINMENU)) && strcmp(lookupuser->userid, currentuser->userid))
        prints("%s (%s)", lookupuser->userid, lookupuser->username);
    else
#endif                          /* 
                                 */
        prints("%s (%s) ¹²ÉÏÕ¾ %d ´Î£¬·¢±í¹ý %d ÆªÎÄÕÂ", lookupuser->userid, lookupuser->username, lookupuser->numlogins, lookupuser->numposts);
    strcpy(planid, lookupuser->userid);
    if ((newline = strchr(genbuf, '\n')) != NULL)
        *newline = '\0';
    seecount = 0;
    logincount = apply_utmp((APPLY_UTMP_FUNC) t_printstatus, 10, lookupuser->userid, &seecount);
    /*
     * »ñµÃÀëÏßÊ±¼ä Luzi 1998/10/23 
     */
    exit_time = get_exit_time(lookupuser->userid, exittime);
    if ((newline = strchr(exittime, '\n')) != NULL)
        *newline = '\0';
    if (exit_time <= lookupuser->lastlogin) {
        if (logincount != seecount) {
            temp = lookupuser->lastlogin + ((lookupuser->numlogins + lookupuser->numposts) % 100) + 60;
            strcpy(exittime, ctime(&temp));     /*Haohmaru.98.12.04.ÈÃÒþÉíÓÃ»§¿´ÉÏÈ¥ÀëÏßÊ±¼ä±ÈÉÏÏßÊ±¼äÍí60µ½160ÃëÖÓ */
            if ((newline = strchr(exittime, '\n')) != NULL)
                *newline = '\0';
        } else
            strcpy(exittime, "ÒòÔÚÏßÉÏ»ò·Ç³£¶ÏÏß²»Ïê");
    }
#ifdef NINE_BUILD
    prints("\nÉÏ´ÎÔÚ  [%s] ´Ó [%s] µ½±¾Õ¾Ò»ÓÎ¡£", Ctime(lookupuser->lastlogin), ((lookupuser->lasthost[0] == '\0') ? "(²»Ïê)" : lookupuser->lasthost));
#else
    prints("\nÉÏ´ÎÔÚ  [%s] ´Ó [%s] µ½±¾Õ¾Ò»ÓÎ¡£\nÀëÏßÊ±¼ä[%s] ", Ctime(lookupuser->lastlogin), ((lookupuser->lasthost[0] == '\0') /*|| DEFINE(currentuser,DEF_HIDEIP) */ ? "(²»Ïê)" : lookupuser->lasthost),    /*Haohmaru.99.12.18. hide ip */
           exittime);
#endif
#ifdef NINE_BUILD
     prints("\nÐÅÏä£º[[5m%2s[m]£¬¾­ÑéÖµ£º[%d](%s) ±íÏÖÖµ£º[%d](%s) ÉúÃüÁ¦£º[%d]%s\n"
       ,(check_query_mail(qry_mail_dir)==1)? "ÐÅ":"  ",exp,cexp(exp),perf,
       cperf(perf),compute_user_value(lookupuser),
       (lookupuser->userlevel & PERM_SUICIDE)?" (×ÔÉ±ÖÐ)":" ");
#else
    uleveltochar(permstr, lookupuser);
    prints("ÐÅÏä£º[[5m%2s[m] ÉúÃüÁ¦£º[%d] Éí·Ý: [%s]%s\n",
           (check_query_mail(qry_mail_dir) == 1) ? "ÐÅ" : "  ", compute_user_value(lookupuser), permstr, (lookupuser->userlevel & PERM_SUICIDE) ? " (×ÔÉ±ÖÐ)" : "¡£");
#endif
#if defined(QUERY_REALNAMES)
    if (HAS_PERM(currentuser, PERM_BASIC))
        prints("Real Name: %s \n", lookupuser->realname);
#endif                          /* 
                                 */
    if ((genbuf[0]) && seecount) {
        prints(genbuf);
        prints("\n");
    }
    show_user_plan(planid);
    if (oldmode != LUSERS && oldmode != LAUSERS && oldmode != FRIEND && oldmode != GMENU) {
        int ch, tuid, ucount, unum;
        char buf[STRLEN];
        struct user_info *uin;
        move(t_lines - 1, 0);
        if ((genbuf[0]) && seecount) {
            if (DEFINE(currentuser, DEF_HIGHCOLOR))
                prints("\x1b[m\x1b[44mÁÄÌì[\x1b[1;32mt\x1b[m\x1b[0;44m] ¼ÄÐÅ[\x1b[1;32mm\x1b[m\x1b[0;44m] ËÍÑ¶Ï¢[\x1b[1;32ms\x1b[m\x1b[0;44m] ¼Ó,¼õÅóÓÑ[\x1b[1;32mo\x1b[m\x1b[0;44m,\x1b[1;32md\x1b[m\x1b[0;44m] ÆäËü¼ü¼ÌÐø");
            else
                prints("\x1b[44mÁÄÌì[t] ¼ÄÐÅ[m] ËÍÑ¶Ï¢[s] ¼Ó,¼õÅóÓÑ[o,d] ÆäËü¼ü¼ÌÐø");
        }
        else {
            if (DEFINE(currentuser, DEF_HIGHCOLOR))
                prints("\x1b[m\x1b[44m        ¼ÄÐÅ[\x1b[1;32mm\x1b[m\x1b[0;44m]           ¼Ó,¼õÅóÓÑ[\x1b[1;32mo\x1b[m\x1b[0;44m,\x1b[1;32md\x1b[m\x1b[0;44m] ÆäËü¼ü¼ÌÐø");
            else
                prints("\x1b[44m        ¼ÄÐÅ[m]           ¼Ó,¼õÅóÓÑ[o,d] ÆäËü¼ü¼ÌÐø");
        }
        clrtoeol();
        resetcolor();
        ch = igetkey();
        switch(toupper(ch)) {
            case 'T':
                if (!((genbuf[0]) && seecount)) break;
                if (strcmp(uident, "guest") && !HAS_PERM(currentuser, PERM_PAGE))
                    break;
                uin = t_search(uident, false);
                if(uin==NULL) break;
                ttt_talk(uin);
                break;
            case 'S':
#ifdef SMS_SUPPORT
		if(ch=='S')
		    do_send_sms_func(uident, NULL);
		else
#endif
		{
		    if (!((genbuf[0]) && seecount)) break;
		    if (strcmp(uident, "guest") && !HAS_PERM(currentuser, PERM_PAGE))
	                break;
		    uin = t_search(uident, false);
		    if (!canmsg(currentuser, uin)) {
			sprintf(buf, "%s ÒÑ¾­¹Ø±ÕÑ¶Ï¢ºô½ÐÆ÷", uident);
			break;
		    }
		    strcpy(MsgDesUid, uident);
                    do_sendmsg(uin, NULL, 0);
		}
                break;
            case 'M':
                if (HAS_PERM(currentuser, PERM_DENYMAIL)
                	||!HAS_PERM(currentuser, PERM_LOGINOK))
                    break;
                m_send(uident);
                break;
            case 'O':
                if (!strcmp("guest", currentuser->userid))
                    break;
                if (addtooverride(uident) == -1)
                    sprintf(buf, "%s ÒÑÔÚÅóÓÑÃûµ¥", uident);
                else
                    sprintf(buf, "%s ÁÐÈëÅóÓÑÃûµ¥", uident);
                move(BBS_PAGESIZE + 3, 0);
                clrtoeol();
                prints("%s", buf);
                refresh();
                sleep(1);
                break;
            case 'D':
                if (!strcmp("guest", currentuser->userid))
                    break;
                sprintf(buf, "ÄãÒª°Ñ %s ´ÓÅóÓÑÃûµ¥ÒÆ³ýÂð (Y/N) [N]: ", uident);
                move(BBS_PAGESIZE + 3, 0);
                clrtoeol();
                getdata(BBS_PAGESIZE + 3, 0, buf, genbuf, 4, DOECHO, NULL, true);
                move(BBS_PAGESIZE + 3, 0);
                clrtoeol();
                if (genbuf[0] != 'Y' && genbuf[0] != 'y') break;
                if (deleteoverride(uident) == -1)
                    sprintf(buf, "%s ±¾À´¾Í²»ÔÚÅóÓÑÃûµ¥ÖÐ", uident);
                else
                    sprintf(buf, "%s ÒÑ´ÓÅóÓÑÃûµ¥ÒÆ³ý", uident);
                move(BBS_PAGESIZE + 3, 0);
                clrtoeol();
                prints("%s", buf);
                refresh();
                sleep(1);
                break;
        }
    }
    uinfo.destuid = 0;
    modify_user_mode(oldmode);
    return 0;
}
int count_visible_active(struct user_info *uentp, int *count, int pos)
{
    if (!uentp->active || !uentp->pid)
        return 0;
    if (!(!HAS_PERM(currentuser, PERM_SEECLOAK) && uentp->invisible))
        (*count)++;
    return 1;
}
int alcounter(struct user_info *uentp, char *arg, int pos)
{
    int canseecloak;

    if (!uentp->active || !uentp->pid)
        return 0;
    canseecloak = (!HAS_PERM(currentuser, PERM_SEECLOAK) && uentp->invisible) ? 0 : 1;
    if (myfriend(uentp->uid, NULL)) {
        count_friends++;
        if (!canseecloak)
            count_friends--;
    }
    count_users++;
    if (!canseecloak)
        count_users--;
    return 1;
}

int num_alcounter()
{
    count_friends = 0;
    count_users = 0;
    apply_ulist_addr((APPLY_UTMP_FUNC) alcounter, 0);
    return count_users;
}
int num_user_logins(char *uid)
{
    strcpy(save_page_requestor, uid);
    return apply_utmp(NULL, 0, uid, 0);
}

int num_visible_users()
{
    int count;

    count = 0;
    apply_ulist_addr((APPLY_UTMP_FUNC) count_visible_active, (char *) &count);
    return count;
}
int ttt_talk(struct user_info *userinfo)
{
    char uident[STRLEN];
    char test[STRLEN];
    int tuid, ucount, unum, tmp;
    struct user_info uin;
    struct _tag_talk_showstatus ts;

    move(1, 0);
    clrtobot();
    if (!HAS_PERM(currentuser,PERM_PAGE))
        return 0;
    if (userinfo == NULL) {
        move(2, 0);
        prints("<ÊäÈëÊ¹ÓÃÕß´úºÅ>\n");
        move(1, 0);
        clrtoeol();
        prints("¸úË­ÁÄÌì: ");
        creat_list();
        namecomplete(NULL, uident);
        if (uident[0] == '\0') {
            clear();
            return 0;
        }
        if (!(tuid = searchuser(uident)) || tuid == usernum) {
            move(2, 0);
            prints("´íÎó´úºÅ\n");
            pressreturn();
            move(2, 0);
            clrtoeol();
            return -1;
        }
        genbuf[0] = 0;
        ts.count = 0;
        ucount = apply_utmp((APPLY_UTMP_FUNC) talk_showstatus, 20, uident, &ts);
        move(3, 0);
        prints("Ä¿Ç° %s µÄ %d logins ÈçÏÂ: \n", uident, ucount);
        clrtobot();
        if (ucount > 1) {
            char buf[6];

          list:move(5, 0);
            prints("(0) ËãÁËËãÁË£¬²»ÁÄÁË¡£\n");
            prints(genbuf);
            clrtobot();
            tmp = ucount + 8;
            getdata(tmp, 0, "ÇëÑ¡Ò»¸öÄã¿´µÄ±È½ÏË³ÑÛµÄ [0]: ", buf, 4, DOECHO, NULL, true);
            unum = atoi(buf);
            if (unum == 0) {
                clear();
                return 0;
            }
            if (unum > ucount || unum < 0) {
                move(tmp, 0);
                prints("±¿±¿£¡ÄãÑ¡´íÁËÀ²£¡\n");
                clrtobot();
                pressreturn();
                goto list;
            }
            uin = utmpshm->uinfo[ts.pos[unum - 1]];
        } else
            search_ulist(&uin, t_cmpuids, tuid);
    } else {
        /*
         * memcpy(&uin,userinfo,sizeof(uin)); 
         */
        uin = *userinfo;
        tuid = uin.uid;
        strcpy(uident, uin.userid);
        move(1, 0);
        clrtoeol();
        prints("¸úË­ÁÄÌì: %s", uin.userid);
    }
    /*
     * check if pager on/off       --gtv 
     */
    if (!canpage(hisfriend(usernum,&uin), uin.pager)) {
        move(2, 0);
        prints("¶Ô·½ºô½ÐÆ÷ÒÑ¹Ø±Õ.\n");
        pressreturn();
        move(2, 0);
        clrtoeol();
        return -1;
    }
    /*
     * modified by Excellent 
     */
    if (uin.mode == ULDL || uin.mode == IRCCHAT || uin.mode == BBSNET || uin.mode == FOURM || uin.mode == EXCE_BIG2 || uin.mode == EXCE_MJ || uin.mode == EXCE_CHESS) {
        move(2, 0);
        prints("Ä¿Ç°ÎÞ·¨ºô½Ð.\n");
        pressreturn();
        move(2, 0);
        clrtoeol();
        return -1;
    }
    if (LOCKSCREEN == uin.mode) {       /* Leeward 98.02.28 */
        move(2, 0);
        prints("¶Ô·½ÒÑ¾­Ëø¶¨ÆÁÄ»£¬ÇëÉÔºòÔÙºô½ÐËû(Ëý)ÁÄÌì...\n");
        clrtobot();
        pressreturn();
        move(2, 0);
        clrtoeol();
        return -1;
    }
    if (!uin.active || (kill(uin.pid, 0) == -1)) {
        move(2, 0);
        prints("¶Ô·½ÒÑÀë¿ª\n");
        pressreturn();
        move(2, 0);
        clrtoeol();
        return -1;
    }
    if (false == canIsend2(currentuser,uin.userid)) {       /*Haohmaru.99.6.6.¼ì²éÊÇ·ñ±»ignore */
        move(2, 0);
        prints("¶Ô·½¾Ü¾øºÍÄãÁÄÌì\n");
        pressreturn();
        move(2, 0);
        clrtoeol();
        return -1;
    } else {
        int sock, msgsock;
        int length;
        struct sockaddr_in server;
        char c;
        char buf[512];

        move(3, 0);
        clrtobot();
        show_user_plan(uident);
        getdata(2, 0, "È·¶¨ÒªºÍËû/ËýÁÄÌìÂð? (Y/N) [N]: ", genbuf, 4, DOECHO, NULL, true);
        if (*genbuf != 'y' && *genbuf != 'Y') {
            clear();
            return 0;
        }
        sprintf(buf, "Talk to '%s'", uident);
        bbslog("user","%s",buf);
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("socket err\n");
            return -1;
        }
		bzero(&server, sizeof(server));
        server.sin_family = AF_INET;
/*        server.sin_addr.s_addr = INADDR_ANY ;
ÎÒÏëÓ¦¸ÃÓÃINADDR_LOOPBACK±È½ÏºÃ KCN
*/
        server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        server.sin_port = 0;
        if (bind(sock, (struct sockaddr *) &server, sizeof server) < 0) {
            perror("bind err");
            return -1;
        }
        length = sizeof server;
        if (getsockname(sock, (struct sockaddr *) &server, (socklen_t *) & length) < 0) {
            perror("socket name err");
            return -1;
        }
        uinfo.sockactive = true;
        uinfo.sockaddr = server.sin_port;
        uinfo.destuid = tuid;
        UPDATE_UTMP(sockactive, uinfo);
        UPDATE_UTMP(sockaddr, uinfo);
        UPDATE_UTMP(destuid, uinfo);
        modify_user_mode(PAGE);
        kill(uin.pid, SIGUSR1);
        clear();
        prints("ºô½Ð %s ÖÐ...\n\nÊäÈë Ctrl-D ½áÊø\n", uident);  /* modified by dong , 1999.1.27 */
        listen(sock, 1);
        add_io(sock, 20);
        while (true) {
            int ch;

            ch = igetkey();
            if (ch == I_TIMEOUT) {
                move(0, 0);
                prints("ÔÙ´Îºô½Ð.\n");
                bell();
                if (kill(uin.pid, SIGUSR1) == -1) {
                    move(0, 0);
                    prints("¶Ô·½ÒÑÀëÏß\n");
                    pressreturn();
                    /*
                     * Add by SmallPig 2 lines 
                     */
                    uinfo.sockactive = false;
                    uinfo.destuid = 0;
                    return -1;
                }
                continue;
            }
            if (ch == I_OTHERDATA)
                break;
            if (ch == '\004') {
                add_io(0, 0);
                close(sock);
                uinfo.sockactive = false;
                uinfo.destuid = 0;
                UPDATE_UTMP(sockactive, uinfo);
                UPDATE_UTMP(destuid, uinfo);
                clear();
                return 0;
            }
        }
        msgsock = accept(sock, (struct sockaddr *) 0, (socklen_t *) 0);
        if (msgsock == -1) {
            perror("accept");
            return -1;
        }
        add_io(0, 0);
        close(sock);
        uinfo.sockactive = false;
        uinfo.destuid = 0;
        UPDATE_UTMP(sockactive, uinfo);
        UPDATE_UTMP(destuid, uinfo);
        /*
         * uinfo.destuid = 0 ; 
         */
        read(msgsock, &c, sizeof c);
        clear();
        if (c == 'y' || c == 'Y') {
            sprintf(save_page_requestor, "%s (%s)", uin.userid, uin.username);
            /*
             * Bigman 2000.9.15 Ôö¼ÓTalk¼ÇÂ¼ 
             */
#ifdef TALK_LOG
            strcpy(partner, uin.userid);
#endif                          /* 
                                 */
            do_talk(msgsock);
            /*
             * Add by SmallPig 
             */
        } else if (c == 'n' || c == 'N') {
            prints("%s (%s)Ëµ£º±§Ç¸£¬ÎÒÏÖÔÚºÜÃ¦£¬²»ÄÜ¸úÄãÁÄ¡£\n", uin.userid, uin.username);
            pressreturn();
        } else if (c == 'b' || c == 'B') {
            prints("%s (%s)Ëµ£ºÎÒÏÖÔÚºÜ·³£¬²»Ïë¸ú±ðÈËÁÄÌì¡£\n", uin.userid, uin.username);
            pressreturn();
        } else if (c == 'c' || c == 'C') {
            prints("%s (%s)Ëµ£ºÎÒÓÐ¼±ÊÂ£¬ÎÒµÈÒ»ÏÂÔÙ Call Äã¡£\n", uin.userid, uin.username);
            pressreturn();
        } else if (c == 'd' || c == 'D') {
            prints("%s (%s)Ëµ£ºÇëÄã²»ÒªÔÙ Page£¬ÎÒ²»Ïë¸úÄãÁÄ¡£\n", uin.userid, uin.username);
            pressreturn();
        } else if (c == 'e' || c == 'E') {
            prints("%s (%s)Ëµ£ºÎÒÒªÀë¿ªÁË£¬ÏÂ´ÎÔÚÁÄ°É¡£\n", uin.userid, uin.username);
            pressreturn();
        } else if (c == 'F' || c == 'f') {
            prints("%s (%s)Ëµ£ºÇëÄã¼ÄÒ»·âÐÅ¸øÎÒ£¬ÎÒÏÖÔÚÃ»¿Õ¡£\n", uin.userid, uin.username);
            pressreturn();
        } else if (c == 'M' || c == 'm') {
            read(msgsock, test, sizeof test);
            prints("%s (%s)Ëµ£º%s\n", uin.userid, uin.username, test);
            pressreturn();
        } else {
            sprintf(save_page_requestor, "%s (%s)", uin.userid, uin.username);
            /*
             * Bigman 2000.9.15 Ôö¼ÓTalk¼ÇÂ¼ 
             */
#ifdef TALK_LOG
            strcpy(partner, uin.userid);
#endif                          /* 
                                 */
            do_talk(msgsock);
        }
	if (talkrequest)
	    talkreply();
        close(msgsock);
        clear();
    }
    return 0;
}

int t_talk()
{
    int netty_talk;

    netty_talk = ttt_talk(NULL);
    clear();
    return (netty_talk);
}
struct user_info ui;
char page_requestor[STRLEN];
char page_requestorid[STRLEN];
int cmpunums(unum, up)
int unum;
struct user_info *up;
{
    if (!up->active)
        return 0;
    return (unum == up->destuid);
}

int cmpmsgnum(unum, up)
int unum;
struct user_info *up;
{
    if (!up->active)
        return 0;
    return (unum == up->destuid && up->sockactive == 2);
}

int setpagerequest(mode)
int mode;
{
    int tuid;

    if (mode == 0)
        tuid = search_ulist(&ui, cmpunums, usernum);
    else
        tuid = search_ulist(&ui, cmpmsgnum, usernum);
    if (tuid == 0)
        return 1;
    if (!ui.sockactive)
        return 1;
    uinfo.destuid = ui.uid;
    sprintf(page_requestor, "%s (%s)", ui.userid, ui.username);
    strcpy(page_requestorid, ui.userid);
    return 0;
}
int servicepage(int line, char *mesg)
{
    static time_t last_check;
    time_t now;
    char buf[STRLEN];
    int tuid = search_ulist(&ui, cmpunums, usernum);

    if (tuid == 0 || !ui.sockactive)
        talkrequest = false;
    if (!talkrequest) {
        if (page_requestor[0]) {
            switch (uinfo.mode) {
            case TALK:
                move(line, 0);
                printdash(mesg);
                break;
            default:           /* a chat mode */
                sprintf(mesg, "** %s ÒÑÍ£Ö¹ºô½Ð.", page_requestor);
            }
            memset(page_requestor, 0, STRLEN);
            last_check = 0;
        }
        return false;
    } else {
        now = time(0);
        if (now - last_check > P_INT) {
            last_check = now;
            if (!page_requestor[0] && setpagerequest(0 /*For Talk */ ))
                return false;
            else
                switch (uinfo.mode) {
                case TALK:
                    move(line, 0);
                    sprintf(buf, "** %s ÕýÔÚºô½ÐÄã", page_requestor);
                    printdash(buf);
                    break;
                default:       /* chat */
                    sprintf(mesg, "** %s ÕýÔÚºô½ÐÄã", page_requestor);
                }
        }
    }
    return true;
}

int talkreply()
{
    int a;
    char buf[512];
    char reason[51];
    struct sockaddr_in sin;
    char inbuf[STRLEN * 2];

    talkrequest = false;
    if (setpagerequest(0 /*For Talk */ ))
        return 0;
    /*
     * added by netty  
     */
    set_alarm(0, 0, NULL, NULL);
    clear();
    /*
     * to show plan -cuteyu 
     */
    move(5, 0);
    clrtobot();
    show_user_plan(page_requestorid);
    /*
     * Add by SmallPig 
     */
    move(1, 0);
    prints("(N)¡¾±§Ç¸£¬ÎÒÏÖÔÚºÜÃ¦£¬²»ÄÜ¸úÄãÁÄ¡£¡¿(B)¡¾ÎÒÏÖÔÚºÜ·³£¬²»Ïë¸ú±ðÈËÁÄÌì¡£ ¡¿\n");
    prints("(C)¡¾ÎÒÓÐ¼±ÊÂ£¬ÎÒµÈÒ»ÏÂÔÙ Call Äã¡£¡¿(D)¡¾Çë²»ÒªÔÙ Page£¬ÎÒ²»Ïë¸úÄãÁÄ¡£¡¿\n");
    prints("(E)¡¾ÎÒÒªÀë¿ªÁË£¬ÏÂ´ÎÔÚÁÄ°É¡£      ¡¿(F)¡¾Çë¼ÄÒ»·âÐÅ¸øÎÒ£¬ÎÒÏÖÔÚÃ»¿Õ¡£ ¡¿\n");
    prints("(M)¡¾ÁôÑÔ¸ø %-12s           ¡¿\n", page_requestorid);
    sprintf(inbuf, "ÄãÏë¸ú %s ÁÄÁÄÌìÂð? (Y N B C D E F)[Y]: ", page_requestor);
    strcpy(save_page_requestor, page_requestor);
    /*
     * 2000.9.15 Bigman Ìí¼ÓTalk¼ÇÂ¼ 
     */
#ifdef TALK_LOG
    strcpy(partner, page_requestorid);
#endif                          /* 
                                 */
    memset(page_requestor, 0, sizeof(page_requestor));
    memset(page_requestorid, 0, sizeof(page_requestorid));
    getdata(0, 0, inbuf, buf, STRLEN, DOECHO, NULL, true);
    /*
     * gethostname(hostname,STRLEN) ;
     * if(!(h = gethostbyname(hostname))) {
     * perror("gethostbyname") ;
     * return -1 ;
     * }
     * memcpy( &sin.sin_addr, h->h_addr, h->h_length) ;
     * ÎÒÏëÓ¦¸ÃÓÃINADDR_LOOPBACK±È½ÏºÃ KCN
     */
    memset(&sin, 0, sizeof sin);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sin.sin_port = ui.sockaddr;
    a = socket(sin.sin_family, SOCK_STREAM, 0);
    if ((connect(a, (struct sockaddr *) &sin, sizeof sin))) {
        prints("connect err");
        return 0;
    }
    if (buf[0] != 'n' && buf[0] != 'N' && buf[0] != 'B' && buf[0] != 'b'
        && buf[0] != 'C' && buf[0] != 'c' && buf[0] != 'D' && buf[0] != 'd' && buf[0] != 'e' && buf[0] != 'E' && buf[0] != 'f' && buf[0] != 'F' && buf[0] != 'm' && buf[0] != 'M')
        buf[0] = 'y';
    if (buf[0] == 'M' || buf[0] == 'm') {
        move(1, 0);
        clrtobot();
        getdata(1, 0, "Áô»°£º", reason, 50, DOECHO, NULL, true);
    }
    write(a, buf, 1);
    if (buf[0] == 'M' || buf[0] == 'm')
        write(a, reason, sizeof reason);
    if (buf[0] != 'y') {
        close(a);
        bbslog("user","%s","page refused");
        clear();
        return 0;
    }
    bbslog("user","%s","page accepted");
    clear();
    do_talk(a);
    close(a);
    clear();
    return 0;
}

int dotalkent(uentp, buf)
struct user_info *uentp;
char *buf;
{
    char mch;

    if (!uentp->active || !uentp->pid)
        return -1;
    if (!HAS_PERM(currentuser, PERM_SEECLOAK) && uentp->invisible)
        return -1;
    switch (uentp->mode) {
    case ULDL:
        mch = 'U';
        break;
    case TALK:
        mch = 'T';
        break;
    case CHAT1:
    case CHAT2:
    case CHAT3:
    case CHAT4:
        mch = 'C';
        break;
    case IRCCHAT:
        mch = 'I';
        break;
    case FOURM:
        mch = '4';
        break;
    case BBSNET:
        mch = 'B';
        break;
    case READNEW:
    case READING:
        mch = 'R';
        break;
    case POSTING:
        mch = 'P';
        break;
    case SMAIL:
    case RMAIL:
    case MAIL:
        mch = 'M';
        break;
    default:
        mch = '-';
    }
    sprintf(buf, "%s%s(%c), ", uentp->invisible ? "*" : "", uentp->userid, mch);
    return 0;
}
int dotalkuent(struct user_info *uentp, char *arg, int pos)
{
    char buf[STRLEN];

    if (dotalkent(uentp, buf) != -1) {
        strcpy(talk_uent_buf, buf);
        talk_uent_buf += strlen(buf);
    }
    return 0;
}

void do_talk_nextline(twin)
struct talk_win *twin;
{
    twin->curln = twin->curln + 1;
    if (twin->curln > twin->eline)
        twin->curln = twin->sline;
    /*
     * if( curln != twin->eline ) {
     * move( curln + 1, 0 );
     * clrtoeol();
     * } 
     */
    move(twin->curln, 0);
    clrtoeol();
    twin->curcol = 0;
}

void do_talk_char(twin, ch)
struct talk_win *twin;
int ch;
{
    if (isprint2(ch)) {
        if (twin->curcol < 79) {
            move(twin->curln, (twin->curcol)++);
            prints("%c", ch);
            return;
        }
        do_talk_nextline(twin);
        twin->curcol++;
        prints("%c", ch);
        return;
    }
    switch (ch) {
    case Ctrl('H'):
    case '\177':
        if (twin->curcol == 0) {
            return;
        }
        (twin->curcol)--;
        move(twin->curln, twin->curcol);
        prints(" ");
        move(twin->curln, twin->curcol);
        return;
    case Ctrl('M'):
    case Ctrl('J'):
        do_talk_nextline(twin);
        return;
    case Ctrl('G'):
        bell();
        return;
    default:
        break;
    }
    return;
}

void do_talk_string(twin, str)
struct talk_win *twin;
char *str;
{
    while (*str) {
        do_talk_char(twin, *str++);
    }
}
void dotalkuserlist(twin)
struct talk_win *twin;
{
    char bigbuf[USHM_SIZE * 20];        /* change MAXACTIVE->USHM_SIZE, dong, 1999.9.15 */
    int savecolumns;

    do_talk_string(twin, "\n*** ÉÏÏßÍøÓÑ ***\n");
    savecolumns = (t_columns > STRLEN ? t_columns : 0);
    talk_uent_buf = bigbuf;
    if (apply_ulist_addr((APPLY_UTMP_FUNC) dotalkuent, 0) == -1) {
        strcpy(bigbuf, "Ã»ÓÐÈÎºÎÊ¹ÓÃÕßÉÏÏß\n");
    }
    strcpy(talk_uent_buf, "\n");
    do_talk_string(twin, bigbuf);
    if (savecolumns)
        t_columns = savecolumns;
}

char talkobuf[80];
int talkobuflen;
int talkflushfd;
void talkflush()
{
    if (talkobuflen)
        write(talkflushfd, talkobuf, talkobuflen);
    talkobuflen = 0;
}
static void moveto(int mode, struct talk_win *twin)
{
    if (mode == 1)
        twin->curln--;
    if (mode == 2)
        twin->curln++;
    if (mode == 3)
        twin->curcol++;
    if (mode == 4)
        twin->curcol--;
    if (twin->curcol < 0) {
        twin->curln--;
        twin->curcol = 0;
    } else if (twin->curcol > 79) {
        twin->curln++;
        twin->curcol = 0;
    }
    if (twin->curln < twin->sline) {
        twin->curln = twin->eline;
    }
    if (twin->curln > twin->eline) {
        twin->curln = twin->sline;
    }
    move(twin->curln, twin->curcol);
}
void endmsg(void *data)
{
    int x, y;
    int tmpansi;

    tmpansi = showansi;
    showansi = 1;
    talkidletime += 60;
    if (talkidletime >= IDLE_TIMEOUT)
        kill(getpid(), SIGHUP);
    if (uinfo.in_chat == true)
        return;
    getyx(&x, &y);
    update_endline();
    move(x, y);
    refresh();
    set_alarm(60, 0, endmsg, NULL);
    showansi = tmpansi;
    return;
}
static int do_talk(int fd)
{
    struct talk_win mywin, itswin;
    char mid_line[256];
    int page_pending = false;
    int i, i2;
    int previous_mode;

#ifdef TALK_LOG
    char mywords[80], itswords[80], buf[80];
    int mlen = 0, ilen = 0;
    time_t now;

    mywords[0] = itswords[0] = '\0';
#endif                          /* 
                                 */
    endmsg(NULL);
    previous_mode = uinfo.mode;
    modify_user_mode(TALK);
    sprintf(mid_line, " %s (%s) ºÍ %s ÕýÔÚ³©Ì¸ÖÐ", currentuser->userid, currentuser->username, save_page_requestor);
    memset(&mywin, 0, sizeof(mywin));
    memset(&itswin, 0, sizeof(itswin));
    i = (t_lines - 1) / 2;
    mywin.eline = i - 1;
    itswin.curln = itswin.sline = i + 1;
    itswin.eline = t_lines - 2;
    move(i, 0);
    printdash(mid_line);
    move(0, 0);
    talkobuflen = 0;
    talkflushfd = fd;
    add_io(fd, 0);
    add_flush(talkflush);
    while (true) {
        int ch;

        if (talkrequest)
            page_pending = servicepage((t_lines - 1) / 2, mid_line);
        ch = igetkey();
        if (ch>255&&ch<0)
        	continue;
        talkidletime = 0;
        if (ch == I_OTHERDATA) {
            char data[80];
            int datac;
            register int i;

            datac = read(fd, data, 80);
            if (datac <= 0)
                break;
            for (i = 0; i < datac; i++) {
                if (data[i] >= 1 && data[i] <= 4) {
                    moveto(data[i] - '\0', &itswin);
                    continue;
                }
#ifdef TALK_LOG
                /*
                 * Bigman 2000.9.15 Ìí¼ÓTALK¼ÇÂ¼
                 */
                /*
                 * existing do_log() overflow problem       
                 */
                else if (isprint2(data[i])) {
                    if (ilen >= 80) {
                        itswords[80] = '\0';
                        (void) do_log(itswords, 2);
                        ilen = 0;
                    } else {
                        itswords[ilen] = data[i];
                        ilen++;
                    }
                } else if ((data[i] == Ctrl('H') || data[i] == '\177') && !ilen) {
                    itswords[ilen--] = '\0';
                } else if (data[i] == Ctrl('M') || data[i] == '\r' || data[i] == '\n') {
                    itswords[ilen] = '\0';
                    (void) do_log(itswords, 2);
                    ilen = 0;
                }
#endif
                do_talk_char(&itswin, data[i]);
            }
        } else {
            if (ch == Ctrl('D') || ch == Ctrl('C'))
                break;
            if (isprint2(ch) || ch == Ctrl('H') || ch == '\177' || ch == Ctrl('G') /* || ch == Ctrl('M') */ ) {
                talkobuf[talkobuflen++] = ch;
                if (talkobuflen == 80)
                    talkflush();
#ifdef TALK_LOG
                if (mlen < 80) {
                    if ((ch == Ctrl('H') || ch == '\177') && mlen != 0) {
                        mywords[mlen--] = '\0';
                    } else {
                        mywords[mlen] = ch;
                        mlen++;
                    }
                } else if (mlen >= 80) {
                    mywords[80] = '\0';
                    (void) do_log(mywords, 1);
                    mlen = 0;
                }
#endif                          /* 
                                 */
                do_talk_char(&mywin, ch);
                /*
                 * } else if (ch == '\n') {   Bigman 2000.9.15 
                 */
            } else if (ch == '\n' || ch == Ctrl('M') || ch == '\r') {
#ifdef TALK_LOG
                if (mywords[0] != '\0') {
                    mywords[mlen++] = '\0';
                    (void) do_log(mywords, 1);
                    mlen = 0;
                }
#endif                          /* 
                                 */
                talkobuf[talkobuflen++] = '\r';
                talkflush();
                do_talk_char(&mywin, '\r');
                /*
                 * } else if (ch == Ctrl('T')) {
                 * now=time(0);
                 * strcpy(ct,ctime(&now));
                 * do_talk_string( &mywin, ct);
                 * } else if (ch == Ctrl('U') || ch == Ctrl('W')) {
                 * dotalkuserlist( &mywin ); 
                 */
            } else if (ch >= KEY_UP && ch <= KEY_LEFT) {
                moveto(ch - KEY_UP + 1, &mywin);
                talkobuf[talkobuflen++] = ch - KEY_UP + 1;
                if (talkobuflen == 80)
                    talkflush();
            } else if (ch == Ctrl('E')) {
                for (i2 = 0; i2 <= 10; i2++) {
                    talkobuf[talkobuflen++] = '\r';
                    talkflush();
                    do_talk_char(&mywin, '\r');
                }
            } else if (ch == Ctrl('P') && HAS_PERM(currentuser, PERM_BASIC)) {
                t_pager();
                update_endline();
            } else if (Ctrl('Z') == ch)
                r_lastmsg();    /* Leeward 98.07.30 support msgX */
        }
    }
    add_io(0, 0);
    talkflush();
    set_alarm(0, 0, NULL, NULL);
    add_flush(NULL);
    modify_user_mode(previous_mode);
#ifdef TALK_LOG
    /*
     * 2000.9.15 Bigman Ìí¼ÓTalk¼ÇÂ¼ 
     */
    mywords[mlen] = '\0';
    itswords[ilen] = '\0';
    if (mywords[0] != '\0')
        do_log(mywords, 1);
    if (itswords[0] != '\0')
        do_log(itswords, 2);
    now = time(0);
    sprintf(buf, "\n\033[1;34mÍ¨»°½áÊø, Ê±¼ä: %s \033[m\n", Cdate(now));
    write(talkrec, buf, strlen(buf));
    close(talkrec);
    /*---	Õâ¾äÓÐÓÃÂð?	commented by period	---*/
    /*
     * sethomefile(genbuf, currentuser->userid, "talklog");  
     */
    /*---	changed by period	2000-09-18	---*/
    *genbuf = 0;
    move(t_lines - 1, 0);
    if (askyn("ÊÇ·ñ¼Ä»ØÁÄÌì¼ÍÂ¼ ", false) == true) {
        /*---						---*
            getdata(23, 0, "ÊÇ·ñ¼Ä»ØÁÄÌì¼ÍÂ¼ [Y/n]: ", genbuf, 2, DOECHO, NULL, true); 
            if (genbuf[0] != 'N' || genbuf[0] != 'n')  {
         *---	also '||' used above is wrong...	---*/
        sethomefile(buf, currentuser->userid, "talklog");
        sprintf(mywords, "¸ú %s µÄÁÄÌì¼ÇÂ¼ [%12.12s]", partner, Ctime(now) + 6);
        mail_file(currentuser->userid, buf, currentuser->userid, mywords, 0, NULL);
    }
    sethomefile(buf, currentuser->userid, "talklog");
    unlink(buf);
#endif                          /* 
                                 */
    return 0;
}

int shortulist(uentp)
struct user_info *uentp;
{
    int i;
    int pageusers = 60;
    extern struct user_info *user_record[];
    extern int range;

    fill_userlist();
    if (ulistpage > ((range - 1) / pageusers))
        ulistpage = 0;
    if (ulistpage < 0)
        ulistpage = (range - 1) / pageusers;
    move(1, 0);
    clrtoeol();
    prints("Ã¿¸ô %d Ãë¸üÐÂÒ»´Î£¬Ctrl-C »ò Ctrl-D Àë¿ª£¬[F]¸ü»»Ä£Ê½[¡ü¡ý]ÉÏ¡¢ÏÂÒ»Ò³  µÚ%1dÒ³", M_INT, ulistpage + 1);
    clrtoeol();
    move(3, 0);
    clrtobot();
    for (i = ulistpage * pageusers; i < (ulistpage + 1) * pageusers && i < range; i++) {
        char ubuf[STRLEN];
        int ovv;

        if (i < numf || friendmode)
            ovv = true;
        else
            ovv = false;
        sprintf(ubuf, "%s%-12.12s %s%-10.10s[m", (ovv) ? "[32m£®" : "  ", user_record[i]->userid, (user_record[i]->invisible == true) ? "[34m" : "",
                modestring(user_record[i]->mode, user_record[i]->destuid, 0, NULL));
        prints("%s", ubuf);
        if ((i + 1) % 3 == 0)
            prints("\n");
        else
            prints(" |");
    }
    return range;
}

int do_list(modestr)
char *modestr;
{
    char buf[STRLEN];
    int count;
    extern int RMSG;
    int chkmailflag = 0;

    if (RMSG != true) {         /*Èç¹ûÊÕµ½ Msg µÚÒ»ÐÐ²»ÏÔÊ¾¡£ */
        move(0, 0);
        clrtoeol();
        chkmailflag = chkmail();
        if (chkmailflag == 2)
            /*
             * Haohmaru.99.4.4.¶ÔÊÕÐÅÒ²¼ÓÏÞÖÆ 
             */
            showtitle(modestr, "[ÄúµÄÐÅÏä³¬¹ýÈÝÁ¿,²»ÄÜÔÙÊÕÐÅ!]");
        else if (chkmailflag)
            showtitle(modestr, "[ÄúÓÐÐÅ¼þ]");
        else
            showtitle(modestr, BBS_FULL_NAME);
	update_endline();
    }
    move(2, 0);
    clrtoeol();
    sprintf(buf, "  %-12s %-10s", "Ê¹ÓÃÕß´úºÅ", "Ä¿Ç°¶¯Ì¬");
    prints("[33m[44m%s |%s |%s[m", buf, buf, buf);
    /*
     * if(apply_ulist( shortulist ) == -1) {
     * prints("No Users Exist\n") ;
     * return 0;
     * }
     * count = shortulist(NULL); 
     */
    count = shortulist(NULL);
    if (uinfo.mode == MONITOR) {
        time_t thetime = time(0);

        move(t_lines - 1, 0);
        prints("[44m[33mÄ¿Ç°ÓÐ %3d %6sÉÏÏß, Ê±¼ä: %s , Ä¿Ç°×´Ì¬£º%10s   [m", count, friendmode ? "ºÃÅóÓÑ" : "Ê¹ÓÃÕß", Ctime(thetime), friendmode ? "ÄãµÄºÃÅóÓÑ" : "ËùÓÐÊ¹ÓÃÕß");
    }
    return 0;
}

int t_list()
{
    modify_user_mode(LUSERS);
    bbslog("user","%s","t_list");
    do_list("Ê¹ÓÃÕß×´Ì¬");
    pressreturn();
    clear();
    return 0;
}
void sig_catcher(void *data)
{
    ulistpage++;
    if (uinfo.mode != MONITOR) {
        set_alarm(0, 0, NULL, NULL);
        return;
    }
    do_list("Ì½ÊÓÃñÇé");
    idle_count++;
    set_alarm(M_INT * idle_count, 0, sig_catcher, NULL);
    UNUSED_ARG(data);
}

int t_monitor()
{
    int i;

    set_alarm(0, 0, NULL, NULL);
    /*
     * idle_monitor_time = 0; 
     */
    bbslog("user","%s","monitor");
    modify_user_mode(MONITOR);
    ulistpage = 0;
    do_list("Ì½ÊÓÃñÇé");
    set_alarm(M_INT * idle_count, 0, sig_catcher, NULL);
    while (true) {
        i = igetkey();
        if (i==KEY_REFRESH)
            do_list("Ì½ÊÓÃñÇé");
        else
        if (Ctrl('Z') == i)
            r_lastmsg();        /* Leeward 98.07.30 support msgX */
        else
        if (i == 'f' || i == 'F' ) {
            if (friendmode == true)
                friendmode = false;
            else
                friendmode = true;
            do_list("Ì½ÊÓÃñÇé");
        }
        else
        if (i == KEY_DOWN) {
            ulistpage++;
            do_list("Ì½ÊÓÃñÇé");
        }
        else
        if (i == KEY_UP) {
            ulistpage--;
            do_list("Ì½ÊÓÃñÇé");
        }
        else
        if (i == Ctrl('D') || i == Ctrl('C') || i == KEY_LEFT)
            break;
        /*
         * else if (i == -1) {
         * if (errno != EINTR) { perror("read"); exit(1); }
         * } else idle_monitor_time = 0; 
         */
    }
    move(2, 0);
    clrtoeol();
    clear();
    return 0;
}

#ifdef IRC
int count_useshell(struct user_info *uentp, int *count, int pos)
{
    if (!uentp->active || !uentp->pid)
        return 0;
    if (uentp->mode == WWW || uentp->mode == CSIE_TIN || uentp->mode == CSIE_GOPHER || uentp->mode == EXCE_CHESS || uentp->mode == EXCE_BIG2 || uentp->mode == EXCE_MJ || uentp->mode == IRCCHAT)
        (*count)++;
    return 1;
}

int num_useshell()
{
    int count;

    count = 0;
    apply_ulist((APPLY_UTMP_FUNC) count_useshell, (char *) &count);
    return count;
}

void exec_cmd(umode, pager, cmdfile)
int umode, pager;
char *cmdfile;
{
    char buf[STRLEN * 2];
    char userhome[STRLEN];
    int save_pager;
    extern int RUNSH;

    if (num_useshell() >= 15) {
        clear();
        prints("Ì«¶àÈËÊ¹ÓÃÍâ²¿³ÌÊ½ÁË£¬ÄãµÈÒ»ÏÂÔÚÓÃ...");
        pressanykey();
        return;
    }
    if (!dashf(cmdfile)) {
        move(2, 0);
        prints("no %s\n", cmdfile);
        pressreturn();
        return;
    }
    save_pager = uinfo.pager;
    if (pager == false) {
        uinfo.pager = 0;
    }
    modify_user_mode(umode);
    /***** modified by netty, March 15,1995
        sprintf( buf, "/bin/sh %s", cmdfile );
    ******/
    sethomepath(userhome, currentuser->userid);
    sprintf(buf, "/bin/sh %s %s %s %s", cmdfile, userhome, currentuser->userid, currentuser->username);
    RUNSH = true;
    do_exec(buf, NULL);
    RUNSH = false;
    uinfo.pager = save_pager;
    clear();
}

void t_irc()
{
    exec_cmd(IRCCHAT, false, "bin/irc.sh");
}
#endif                          /* IRC */
/*
void
t_announce() {
    exec_cmd( CSIE_ANNOUNCE, true, "bin/faq.sh" );
}
void
t_tin() {
    exec_cmd( CSIE_TIN, true, "bin/tin.sh" );
}
void
t_gopher() {
    exec_cmd( CSIE_GOPHER, true, "bin/gopher.sh" );
}
void
t_www() {
    exec_cmd( WWW, true, "bin/www.sh" );
}*/
/*Add By Excellent*/
/*void
x_excemj() {
    clear();
    exec_cmd( EXCE_MJ, true, "bin/excemj.sh" );
        }
*/
/*Add By Excellent */
/*void
x_excebig2() {
    clear();
    exec_cmd( EXCE_BIG2, true, "bin/excebig2.sh" );
        }
*/
/* Add By Excellent */
/*void
x_excechess() {
    clear();
    exec_cmd( EXCE_CHESS, true, "bin/excechess.sh" );
        }        
*/
int listfilecontent(fname)
char *fname;
{
    FILE *fp;
    int x = 0, y = 3, cnt = 0, max = 0, len;
    char u_buf[20], line[STRLEN], *nick;

    move(y, x);
    CreateNameList();
    strcpy(genbuf, fname);
    if ((fp = fopen(genbuf, "r")) == NULL) {
        prints("(none)\n");
        return 0;
    }
    while (fgets(genbuf, STRLEN, fp) != NULL) {
        strtok(genbuf, " \n\r\t");
        strcpy(u_buf, genbuf);
        AddNameList(u_buf);
        nick = (char *) strtok(NULL, "\n\r\t");
        if (nick != NULL) {
            while (*nick == ' ')
                nick++;
            if (*nick == '\0')
                nick = NULL;
        }
        if (nick == NULL) {
            strcpy(line, u_buf);
        } else {
            sprintf(line, "%-12s%s", u_buf, nick);
        }
        if ((len = strlen(line)) > max)
            max = len;
        if (x + len > 78)
            line[78 - x] = '\0';
        prints("%s", line);
        cnt++;
        if ((++y) >= t_lines - 1) {
            y = 3;
            x += max + 2;
            max = 0;
            if (x > 70)
                break;
        }
        move(y, x);
    }
    fclose(fp);
    if (cnt == 0)
        prints("(none)\n");
    return cnt;
}

int addtooverride(uident)
char *uident;
{
    struct friends tmp;
    int n;
    char buf[STRLEN];

    memset(&tmp, 0, sizeof(tmp));
    sethomefile(buf, currentuser->userid, "friends");
    if ((!HAS_PERM(currentuser, PERM_ACCOUNTS) && !HAS_PERM(currentuser, PERM_SYSOP)) && (get_num_records(buf, sizeof(struct friends)) >= MAXFRIENDS)) {
        move(t_lines - 2, 0);
        clrtoeol();
        prints("±§Ç¸£¬±¾Õ¾Ä¿Ç°½ö¿ÉÒÔÉè¶¨ %d ¸öºÃÓÑ, Çë°´ÈÎºÎ¼ü¼ÌÐø...", MAXFRIENDS);
        igetkey();
        /* TODO: add KEY_REFRESH support */
        move(t_lines - 2, 0);
        clrtoeol();
        return -1;
    }
    if (myfriend(searchuser(uident), NULL))
        return -1;
    if (uinfo.mode != LUSERS && uinfo.mode != LAUSERS && uinfo.mode != FRIEND) {
        strcpy(tmp.id, uident);
        move(2, 0);
        clrtoeol();
        sprintf(genbuf, "ÇëÊäÈë¸øºÃÓÑ¡¾%s¡¿µÄËµÃ÷: ", tmp.id);
        getdata(2, 0, genbuf, tmp.exp, 15, DOECHO, NULL, true);
    } else {
        move(t_lines - 2, 0);
        clrtoeol();
        strcpy(tmp.id, uident);
        sprintf(genbuf, "ÇëÊäÈë¸øºÃÓÑ¡¾%s¡¿µÄËµÃ÷: ", tmp.id);
        getdata(t_lines - 2, 0, genbuf, tmp.exp, 15, DOECHO, NULL, true);
    }
    sethomefile(genbuf, currentuser->userid, "friends");
    n = append_record(genbuf, &tmp, sizeof(struct friends));
    if (n != -1)
        getfriendstr(currentuser,get_utmpent(utmpent));
    else
        bbslog("user","%s","append friendfile error");
    return n;
}
int deleteoverride(char *uident)
{
    int deleted;
    struct friends fh;

    sethomefile(genbuf, currentuser->userid, "friends");
    deleted = search_record(genbuf, &fh, sizeof(fh), (RECORD_FUNC_ARG) cmpfnames, uident);
    if (deleted > 0) {
        if (delete_record(genbuf, sizeof(fh), deleted, NULL, NULL) == 0)
            getfriendstr(currentuser,get_utmpent(utmpent));
        else {
            deleted = -1;
            bbslog("user","%s","delete friend error");
        }
    }
    return (deleted > 0) ? 1 : -1;
}

void friend_title()
{
    int chkmailflag = 0;

    chkmailflag = chkmail();
    if (chkmailflag == 2)
        /*
         * Haohmaru.99.4.4.¶ÔÊÕÐÅÒ²¼ÓÏÞÖÆ 
         */
        strcpy(genbuf, "[ÄúµÄÐÅÏä³¬¹ýÈÝÁ¿,²»ÄÜÔÙÊÕÐÅ!]");
    else if (chkmailflag)
        strcpy(genbuf, "[ÄúÓÐÐÅ¼þ]");
    else
        strcpy(genbuf, BBS_FULL_NAME);
    showtitle("[±à¼­ºÃÓÑÃûµ¥]", genbuf);
    prints(" [¡û,e] Àë¿ª [h] ÇóÖú [¡ú,r] ºÃÓÑËµÃ÷µµ [¡ü,¡ý] Ñ¡Ôñ [a] Ôö¼ÓºÃÓÑ [d] É¾³ýºÃÓÑ\n");
    prints("[44m ±àºÅ  ºÃÓÑÁÐ±í      ´úºÅËµÃ÷                                                   [m\n");
}
char *friend_doentry(char *buf, int ent, struct friends *fh)
{
    sprintf(buf, " %4d  %-12.12s  %s", ent, fh->id, fh->exp);
    return buf;
}

int friend_edit(ent, fh, direc)
int ent;
struct friends *fh;
char *direc;
{
    struct friends nh;
    char buf[STRLEN / 2];
    int pos;

    pos = search_record(direc, &nh, sizeof(nh), (RECORD_FUNC_ARG) cmpfnames, fh->id);
    move(t_lines - 2, 0);
    clrtoeol();
    if (pos > 0) {
        sprintf(buf, "ÇëÊäÈë %s µÄÐÂºÃÓÑËµÃ÷: ", fh->id);
        getdata(t_lines - 2, 0, buf, nh.exp, 15, DOECHO, NULL, false);
    }
    if (substitute_record(direc, &nh, sizeof(nh), pos) < 0)
        bbslog("user","%s","Friend files subs err");
    move(t_lines - 2, 0);
    clrtoeol();
    return NEWDIRECT;
}

int friend_help()
{
    show_help("help/friendshelp");
    return FULLUPDATE;
}

int friend_add(ent, fh, direct)
int ent;
struct friends *fh;
char *direct;
{
    char uident[13];

    clear();
    move(1, 0);
    usercomplete("ÇëÊäÈëÒªÔö¼ÓµÄ´úºÅ: ", uident);
    if (uident[0] != '\0') {
        if (searchuser(uident) <= 0) {
            move(2, 0);
            prints(MSG_ERR_USERID);
            pressanykey();
        } else
            addtooverride(uident);
    }
    return FULLUPDATE;
}

int friend_dele(ent, fh, direct)
int ent;
struct friends *fh;
char *direct;
{
    char buf[STRLEN];
    int deleted = false;

    saveline(t_lines - 2, 0, NULL);
    move(t_lines - 2, 0);
    clrtoeol();
    sprintf(buf, "ÊÇ·ñ°Ñ¡¾%s¡¿´ÓºÃÓÑÃûµ¥ÖÐÈ¥³ý", fh->id);
    if (askyn(buf, false) == true) {
        move(t_lines - 2, 0);
        clrtoeol();
        if (deleteoverride(fh->id) == 1) {
            prints("ÒÑ´ÓºÃÓÑÃûµ¥ÖÐÒÆ³ý¡¾%s¡¿,°´ÈÎºÎ¼ü¼ÌÐø...", fh->id);
            deleted = true;
        } else
            prints("ÕÒ²»µ½¡¾%s¡¿,°´ÈÎºÎ¼ü¼ÌÐø...", fh->id);
    } else {
        move(t_lines - 2, 0);
        clrtoeol();
        prints("È¡ÏûÉ¾³ýºÃÓÑ...");
    }
    igetkey();
    /* TODO: add KEY_REFRESH support */
    move(t_lines - 2, 0);
    clrtoeol();
    saveline(t_lines - 2, 1, NULL);
    return (deleted) ? FULLUPDATE : DONOTHING;
}

int friend_mail(ent, fh, direct)
int ent;
struct friends *fh;
char *direct;
{
    if (!HAS_PERM(currentuser, PERM_POST))
        return DONOTHING;
    m_send(fh->id);
    return FULLUPDATE;
}

int friend_query(ent, fh, direct)
int ent;
struct friends *fh;
char *direct;
{
    int ch;

    if (t_query(fh->id) == -1)
        return FULLUPDATE;
    move(t_lines - 1, 0);
    clrtoeol();
    prints("[44m[31m[¶ÁÈ¡ºÃÓÑËµÃ÷µµ][33m ¼ÄÐÅ¸øºÃÓÑ m ©¦ ½áÊø Q,¡û ©¦ÉÏÒ»Î» ¡ü©¦ÏÂÒ»Î» <Space>,¡ý      [m");
    ch = igetkey();
    switch (ch) {
    case Ctrl('Z'):
        r_lastmsg();            /* Leeward 98.07.30 support msgX */
        break;
    case KEY_REFRESH:
    case 'N':
    case 'Q':
    case 'n':
    case 'q':
    case KEY_LEFT:
        break;
    case 'm':
    case 'M':
        m_send(fh->id);
        break;
    case ' ':
    case 'j':
    case KEY_RIGHT:
    case KEY_DOWN:
    case KEY_PGDN:
        return READ_NEXT;
    case KEY_UP:
    case KEY_PGUP:
        return READ_PREV;
    default:
        break;
    }
    return FULLUPDATE;
}

void t_override()
{
    sethomefile(genbuf, currentuser->userid, "friends");
    i_read(GMENU, genbuf, friend_title, (READ_FUNC) friend_doentry, friend_list, sizeof(struct friends));
    clear();
    return;
}

int cmpfuid(a, b)
struct friends *a, *b;
{
    return strncasecmp(a->id, b->id, IDLEN);
}

int wait_friend()
{
    FILE *fp;
    int tuid;
    char buf[STRLEN];
    char uid[13];

    modify_user_mode(WFRIEND);
    clear();
    move(1, 0);
    usercomplete("ÇëÊäÈëÊ¹ÓÃÕß´úºÅÒÔ¼ÓÈëÏµÍ³µÄÑ°ÈËÃû²á: ", uid);
    if (uid[0] == '\0') {
        clear();
        return 0;
    }
    if (!(tuid = getuser(uid, NULL))) {
        move(2, 0);
        prints("[1m²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ[m\n");
        pressanykey();
        clear();
        return -1;
    }
    sprintf(buf, "ÄãÈ·¶¨Òª°Ñ %s ¼ÓÈëÏµÍ³Ñ°ÈËÃûµ¥ÖÐ", uid);
    move(2, 0);
    if (askyn(buf, true) == false) {
        clear();
        return -1;
    }
    if ((fp = fopen("friendbook", "a")) == NULL) {
        prints("ÏµÍ³µÄÑ°ÈËÃû²áÎÞ·¨¿ªÆô£¬ÇëÍ¨ÖªÕ¾³¤...\n");
        pressanykey();
        return -1;
    }
    sprintf(buf, "%d@%s", tuid, currentuser->userid);
    if (!seek_in_file("friendbook", buf))
        fprintf(fp, "%s\n", buf);
    fclose(fp);
    move(3, 0);
    prints("ÒÑ¾­°ïÄã¼ÓÈëÑ°ÈËÃû²áÖÐ£¬%s ÉÏÕ¾ÏµÍ³Ò»¶¨»áÍ¨ÖªÄã...\n", uid);
    pressanykey();
    clear();
    return 0;
}

/* »µÈËÃûµ¥:Bigman 2000.12.26 */
int list_ignore(fname)
char *fname;
{
    FILE *fp;
    int x = 0, y = 4, nIdx = 0;
    char buf[IDLEN + 1], buf2[80];

    clear();
    move(y, x);
    /*
     * clrtoeol(); 
     */
    if ((fp = fopen(fname, "r")) == NULL) {
        prints("\033[1;33m*** ÉÐÎ´Éè¶¨ºÚÃûµ¥ ***\033[m");
        return (0);
    } else {
        strcpy(buf2, "\033[1;32m¡¼ºÚÃûµ¥ÉÏµÄÓÃ»§IDÁÐ±í¡½\033[m");
        while (fread(buf, IDLEN + 1, 1, fp) > 0) {
            if (nIdx % 4 == 0) {
                prints(buf2);
                memset(buf2, 0, IDLEN + 1);
                y++;
                move(y, x);
            }
            nIdx++;
            sprintf(buf2 + strlen(buf2), "  %-13s", buf);
        }
        if (nIdx > 0) {
            prints(buf2);
        } else {
            prints("\033[1;32m*** ÉÐÎ´Éè¶¨ºÚÃûµ¥ ***\033[m");
        }
        y++;
        move(y, x);
        clrtoeol();
        fclose(fp);
        return (nIdx);
    }
}
void clear_press()
{                               /* 2000.12.28 Bigman ÖØ¸´ÊäÈëµÄ»Ø³µ£¬Çå³ý */
    pressreturn();
    move(1, 0);
    clrtoeol();
    move(2, 0);
    clrtoeol();
}

int badlist()
{
    char userid[IDLEN + 1], tmp[3];
    int cnt, nIdx;
    char ignoreuser[IDLEN + 1], path[40];
    int search_record(), append_record();
    int usercomplete(), namecomplete();

    modify_user_mode(EDITUFILE);
    clear();
    sethomefile(path, currentuser->userid, "/ignores");
    while (1) {
        cnt = list_ignore(path);
        if (cnt >= MAX_IGNORE) {
            move(1, 0);
            prints("ÒÑ¾­µ½´ïºÚÃûµ¥×î´óÈËÊýÏÞÖÆ");
            getdata(0, 0, "(D)É¾³ý (C)Çå³ý (Q)·µ»Ø? [Q]£º ", tmp, 2, DOECHO, NULL, true);
        } else if (cnt <= 0)
            getdata(0, 0, "(A)Ôö¼Ó (Q)·µ»Ø? [Q]£º ", tmp, 2, DOECHO, NULL, true);
        else
            getdata(0, 0, "(A)Ôö¼Ó (D)É¾³ý (C)Çå³ý (Q)·µ»Ø? [Q]£º ", tmp, 2, DOECHO, NULL, true);
        if (tmp[0] == 'Q' || tmp[0] == 'q' || tmp[0] == '\0') {
            break;
        }
        if (((tmp[0] == 'a' || tmp[0] == 'A') && (cnt < MAX_IGNORE)) || ((tmp[0] == 'd' || tmp[0] == 'D') && (cnt > 0))) {
            usercomplete("ÇëÊäÈëÊ¹ÓÃÕß´úºÅ(Ö»°´ ENTER ½áÊøÊäÈë): ", userid);
            if (userid[0] == '\0') {
                move(1, 0);
                clrtoeol();
                continue;
            }
            if (!searchuser(userid)) {
                prints("Õâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n");
                clear_press();
            } else if (!strcasecmp(userid, currentuser->userid)) {
                prints("²»ÄÜÊÇ×Ô¼ºµÄ´úºÅ\n");
                clear_press();
            } else {
                nIdx = search_record(path, ignoreuser, IDLEN + 1, (RECORD_FUNC_ARG) cmpinames, userid);
                if (tmp[0] == 'a' || tmp[0] == 'A') {
                    if (nIdx > 0) {
                        prints("¸ÃIDÒÑ¾­ÔÚºÚÃûµ¥ÉÏ£¡");
                        clear_press();
                    } else {
                        if (append_record(path, userid, IDLEN + 1) == 0) {
                            /*
                             * prints("ÒÑ¾­³É¹¦Ìí¼Óµ½ºÚÃûµ¥ÖÐ"); 
                             */
                            /*
                             * cnt=list_ignore(path); 
                             */
                        } else {
                            prints("*** ÏµÍ³´íÎó , ÇëÓëSYSOPÁªÏµ***");
                            clear_press();
                        }
                    }
                } else {
                    if (nIdx <= 0) {
                        prints("¸ÃIDÃ»ÓÐÔÚºÚÃûµ¥ÉÏ£¡");
                        clear_press();
                    } else {
                        if (delete_record(path, IDLEN + 1, nIdx, NULL, NULL) == 0) {
                            ;
                            /*
                             * prints("ÒÑ¾­³É¹¦´ÓºÚÃûµ¥ÖÐÉ¾³ý"); 
                             */
                            /*
                             * cnt=list_ignore(path); 
                             */
                        } else {
                            prints("*** ÏµÍ³´íÎó , ÇëÓëSYSOPÁªÏµ***");
                            clear_press();
                        }
                    }
                }
            }
        } else if ((tmp[0] == 'c' || tmp[0] == 'C') && (cnt > 0)) {
            getdata(1, 0, "È·¶¨É¾³ýºÚÃûµ¥? (Y/N) [N]:", tmp, 2, DOECHO, NULL, true);
            if (tmp[0] == 'y' || tmp[0] == 'Y') {
                unlink(path);
                /*
                 * cnt=list_ignore(path); 
                 */
            } else {
                move(1, 0);
                clrtoeol();
            }
        }
    }
    pressreturn();
    return (cnt);
}

#ifdef TALK_LOG
/* Bigman 2000.9.15 ·Ö±ðÎªÁ½Î»ÁÄÌìµÄÈË×÷¼ÍÂ¼ */
/* -=> ×Ô¼ºËµµÄ»° */
/* --> ¶Ô·½ËµµÄ»° */
void do_log(char *msg, int who)
{
    time_t now;
    char buf[100];

    now = time(0);
    if (msg[strlen(msg)] == '\n')
        msg[strlen(msg)] = '\0';
    if (strlen(msg) < 1 || msg[0] == '\r' || msg[0] == '\n')
        return;
    /*
     * Ö»°ï×Ô¼º×ö 
     */
    sethomefile(buf, currentuser->userid, "talklog");
    if (!dashf(buf) || talkrec == -1) {
        talkrec = open(buf, O_RDWR | O_CREAT | O_TRUNC, 0644);
        sprintf(buf, "\033[1;32mÓë %s µÄÁÄÌì¼ÇÂ¼, ÈÕÆÚ: %s \033[m\n", save_page_requestor, Cdate(now));
        write(talkrec, buf, strlen(buf));
        sprintf(buf, "\tÑÕÉ«·Ö±ð´ú±í: \033[1;33m%s\033[m \033[1;36m%s\033[m \n\n", currentuser->userid, partner);
        write(talkrec, buf, strlen(buf));
    }
    if (who == 1) {             /* ×Ô¼ºËµµÄ»° */
        sprintf(buf, "\033[1;33m%s \033[m\n", msg);
        write(talkrec, buf, strlen(buf));
    } else if (who == 2) {      /* ±ðÈËËµµÄ»° */
        sprintf(buf, "\033[1;36m%s \033[m\n", msg);
        write(talkrec, buf, strlen(buf));
    }
}
#endif
