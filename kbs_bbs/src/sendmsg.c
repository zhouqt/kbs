#include "bbs.h"
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "screen.h"
#define MAXMESSAGE 5

int RMSG = false;
extern int RUNSH;
extern struct screenline *big_picture;
extern char MsgDesUid[14];      /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong */
static int RMSGCount = 0;       /* Leeward 98.07.30 supporting msgX */
extern int i_timeout;


int get_msg(uid, msg, line)
char *msg, *uid;
int line;
{
    char genbuf[3];
    int i;

    move(line, 0);
    clrtoeol();
    prints("ËÍÒôĞÅ¸ø:%-12s    ÇëÊäÈëÒôĞÅÄÚÈİ£¬Ctrl+Q »»ĞĞ:", uid);
    memset(msg, 0, sizeof(msg));
    while (1) {
        i = multi_getdata(line+1, 0, 79, NULL, msg, MAX_MSG_SIZE, 11, false);
        if (msg[0] == '\0')
            return false;
        getdata(line + i + 1, 0, "È·¶¨ÒªËÍ³öÂğ(Y)ÊÇµÄ (N)²»Òª (E)ÔÙ±à¼­? [Y]: ", genbuf, 2, DOECHO, NULL, 1);
        if (genbuf[0] == 'e' || genbuf[0] == 'E')
            continue;
        if (genbuf[0] == 'n' || genbuf[0] == 'N')
            return false;
        if (genbuf[0] == 'G') {
            if (HAS_PERM(currentuser, PERM_SYSOP))
                return 2;
            else
                return true;
        } else
            return true;
    }
}

int s_msg()
{
    return do_sendmsg(NULL, NULL, 0);
}
extern char msgerr[255];

int do_sendmsg(uentp, msgstr, mode)
struct user_info *uentp;
const char msgstr[MAX_MSG_SIZE];
int mode;
{
    char uident[STRLEN];
    struct user_info *uin;
    char buf[MAX_MSG_SIZE];
    int Gmode = 0;
    int result;

    if ((mode == 0) || (mode == 3)) {
        modify_user_mode(MSG);
        move(2, 0);
        clrtobot();
    }
    if (uentp == NULL) {
        prints("<ÊäÈëÊ¹ÓÃÕß´úºÅ>\n");
        move(1, 0);
        clrtoeol();
        prints("ËÍÑ¶Ï¢¸ø: ");
        creat_list();
        namecomplete(NULL, uident);
        if (uident[0] == '\0') {
            clear();
            return 0;
        }
        uin = t_search(uident, false);
        if (uin == NULL) {
            move(2, 0);
            prints("¶Ô·½Ä¿Ç°²»ÔÚÏßÉÏ£¬»òÊÇÊ¹ÓÃÕß´úºÅÊäÈë´íÎó...\n");
            pressreturn();
            move(2, 0);
            clrtoeol();
            return -1;
        }
        if (!canmsg(currentuser, uin)) {
            move(2, 0);
            prints("¶Ô·½ÒÑ¾­¹Ø±Õ½ÓÊÜÑ¶Ï¢µÄºô½ĞÆ÷...\n");
            pressreturn();
            move(2, 0);
            clrtoeol();
            return -1;
        }
        /*
         * ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong 
         */
        strcpy(MsgDesUid, uident);
        /*
         * uentp = uin; 
         */

    } else {
        /*
         * if(!strcasecmp(uentp->userid,currentuser->userid))  rem by Haohmaru,ÕâÑù²Å¿ÉÒÔ×Ô¼º¸ø×Ô¼º·¢msg
         * return 0;    
         */ uin = uentp;
        strcpy(uident, uin->userid);
        /*
         * strcpy(MsgDesUid, uin->userid); change by KCN,is wrong 
         */
    }

    /*
     * try to send the msg 
     */
    result = sendmsgfunc(uin, msgstr, mode);

    switch (result) {
    case 1:                    /* success */
        if (mode == 0) {
            prints("\nÒÑËÍ³öÑ¶Ï¢....\n");
            pressreturn();
            clear();
        }
        return 1;
        break;
    case -1:                   /* failed, reason in msgerr */
        if (mode == 0) {
            move(2, 0);
            prints(msgerr);
            pressreturn();
            move(2, 0);
            clrtoeol();
        }
        return -1;
        break;
    case 0:                    /* message presending test ok, get the message and resend */
        if (mode == 4)
            return 0;
        Gmode = get_msg(uident, buf, 1);
        if (!Gmode) {
            move(1, 0);
            clrtoeol();
            move(2, 0);
            clrtoeol();
            return 0;
        }
        break;
    default:                   /* unknown reason */
        return result;
        break;
    }
    /*
     * resend the message 
     */
    result = sendmsgfunc(uin, buf, mode);

    switch (result) {
    case 1:                    /* success */
        if (mode == 0) {
            prints("\nÒÑËÍ³öÑ¶Ï¢....\n");
            pressreturn();
            clear();
        }
        return 1;
        break;
    case -1:                   /* failed, reason in msgerr */
        if (mode == 0) {
            move(2, 0);
            prints(msgerr);
            pressreturn();
            move(2, 0);
            clrtoeol();
        }
        return -1;
        break;
    default:                   /* unknown reason */
        return result;
        break;
    }
    return 1;
}



int show_allmsgs()
{
    char buf[MAX_MSG_SIZE], showmsg[MAX_MSG_SIZE*2], chk[STRLEN];
    int oldmode, count, i, j, page, ch, y, all=0, reload=0;
    char title[STRLEN];
    struct msghead head;
    time_t now;
    char fname[STRLEN];
    FILE* fn;

    if(!HAS_PERM(currentuser, PERM_PAGE)) return -1;
    oldmode = uinfo.mode;
    modify_user_mode(LOOKMSGS);
//    set_alarm(0, 0, NULL, NULL);

    page = 0;
    count = get_msgcount(0, currentuser->userid);
    while(1) {
        if(reload) {
            reload = 0;
            page = 0;
            count = get_msgcount(all?2:0, currentuser->userid);
        }
        clear();
        if(count==0) {
            good_move(5,30);
            prints("Ã»ÓĞÈÎºÎµÄÑ¶Ï¢´æÔÚ£¡£¡");
            i = 0;
        }
        else {
            y = 0;
            i = page;
            load_msghead(all?2:0, currentuser->userid, i, &head);
            load_msgtext(currentuser->userid, &head, buf);
            j = translate_msg(buf, &head, showmsg);
            while(y+j<=23) {
                y+=j; i++;
                prints("%s", showmsg);
                if(i>=count) break;
                load_msghead(all?2:0, currentuser->userid, i, &head);
                load_msgtext(currentuser->userid, &head, buf);
                j = translate_msg(buf, &head, showmsg);
            }
        }
        good_move(23,0);
        if(!all)
            prints("[1;44;32mÑ¶Ï¢ä¯ÀÀÆ÷   ±£Áô<[37mr[32m> Çå³ı<[37mc[32m> ¼Ä»ØĞÅÏä<[37mm[32m> ·¢Ñ¶ÈË<[37mi[32m> Ñ¶Ï¢ÄÚÈİ<[37ms[32m>        Ê£Óà:%4d ", count-i);
        else
            prints("[1;44;32mÑ¶Ï¢ä¯ÀÀÆ÷   ±£Áô<[37mr[32m> Çå³ı<[37mc[32m> ¼Ä»ØĞÅÏä<[37mm[32m> ·¢Ñ¶ÈË<[37mi[32m> Ñ¶Ï¢ÄÚÈİ<[37ms[32m> È«²¿<[37ma[32m>     %4d ", count-i);
        refresh();
reenter:
        ch = igetkey();
        switch(ch) {
            case 'r':
            case 'R':
            case 'q':
            case 'Q':
            case KEY_LEFT:
            case '\r':
            case '\n':
                goto outofhere;
            case KEY_UP:
                if(page>0) page--;
                break;
            case KEY_DOWN:
                if(page<count-1) page++;
                break;
            case KEY_PGDN:
            case ' ':
            case KEY_RIGHT:
                if(page<count-11) page+=10;
                else page=count-1;
                break;
            case KEY_PGUP:
                if(page>10) page-=10;
                else page=0;
                break;
            case KEY_HOME:
            case Ctrl('A'):
                page=0;
                break;
            case KEY_END:
            case Ctrl('E'):
                page=count-1;
                break;
            case 'i':
            case 'I':
            case 's':
            case 'S':
                reload = 1;
                count = get_msgcount(0, currentuser->userid);
                if(count==0) break;
                good_move(23, 0);
                clrtoeol();
                getdata(23, 0, "ÇëÊäÈë¹Ø¼ü×Ö:", chk, 50, true, NULL, true);
                if(chk[0]) {
                    int fd, fd2;
                    char fname[STRLEN], fname2[STRLEN];
                    size_t bm_search[256];
                    struct msghead head;
                    int i, j;
                    bool init=false;
                    sethomefile(fname, currentuser->userid, "msgindex");
                    sethomefile(fname2, currentuser->userid, "msgindex3");
                    fd = open(fname, O_RDONLY, 0644);
                    fd2 = open(fname2, O_WRONLY|O_CREAT, 0644);
                    write(fd2, &i, 4);
                    lseek(fd, 4, SEEK_SET);
                    for(i=0;i<count;i++) {
                        read(fd, &head, sizeof(struct msghead));
                        if(toupper(ch)=='S') load_msgtext(currentuser->userid, &head, buf);
                        if(toupper(ch)=='I'&&!strncasecmp(chk, head.id, IDLEN) ||
                            toupper(ch)=='S'&&bm_strcasestr_rp(buf, chk, bm_search, &init) != NULL)
                            write(fd2, &head, sizeof(struct msghead));
                    }
                    close(fd2);
                    close(fd);
                    all = 1;
                }
                break;
            case 'c':
            case 'C':
                clear_msg(currentuser->userid);
                goto outofhere;
            case 'a':
            case 'A':
                if(all) {
                    sethomefile(buf, currentuser->userid, "msgindex3");
                    unlink(buf);
                    all = 0;
                    reload = 1;
                }
                break;
            case 'm':
            case 'M':
                mail_msg(currentuser);
                goto outofhere;
            default:
                goto reenter;
        }
    }
outofhere:
    
    if(all) {
        sethomefile(buf, currentuser->userid, "msgindex3");
        unlink(buf);
    }
    clear();
    uinfo.mode = oldmode;
//    R_monitor(NULL);
    return 0;
}


int dowall(struct user_info *uin, char *buf2)
{
    if (!uin->active || !uin->pid)
        return -1;
    /*---	²»¸øµ±Ç°´°¿Ú·¢ÏûÏ¢ÁË	period	2000-11-13	---*/
    if (getpid() == uin->pid)
        return -1;

    move(1, 0);
    clrtoeol();
    prints("[32mÕı¶Ô %s ¹ã²¥.... Ctrl-D Í£Ö¹¶Ô´ËÎ» User ¹ã²¥¡£[m", uin->userid);
    refresh();
    if (strcmp(uin->userid, "guest")) { /* Leeward 98.06.19 */
        /*
         * ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong 
         */
        strcpy(MsgDesUid, uin->userid);

        do_sendmsg(uin, buf2, 3);       /* ¹ã²¥Ê±±ÜÃâ±»¹ı¶àµÄ guest ´ò¶Ï */
    }
    return 0;
}


int wall()
{
    char buf2[MAX_MSG_SIZE];

    if (check_systempasswd() == false)
        return 0;
    modify_user_mode(MSG);
    move(2, 0);
    clrtobot();
    if (!get_msg("ËùÓĞÊ¹ÓÃÕß", buf2, 1)) {
        move(1, 0);
        clrtoeol();
        move(2, 0);
        clrtoeol();
        return 0;
    }
    if (apply_ulist((APPLY_UTMP_FUNC) dowall, buf2) == -1) {
        move(2, 0);
        prints("Ã»ÓĞÈÎºÎÊ¹ÓÃÕßÉÏÏß\n");
        pressanykey();
    }
    sprintf(genbuf, "%s ¹ã²¥:%s", currentuser->userid, buf2);
    securityreport(genbuf, NULL, NULL);
    prints("\nÒÑ¾­¹ã²¥Íê±Ï....\n");
    pressanykey();
    return 0;
}

int msg_count;

void r_msg_sig(int signo)
{
    msg_count++;
    signal(SIGUSR2, r_msg_sig);
}

void r_msg()
{
    int y, x, ch, i, ox, oy, tmpansi, pid, oldi;
    char savebuffer[25][256];
    char buf[MAX_MSG_SIZE+100], outmsg[MAX_MSG_SIZE*2], buf2[STRLEN], uid[14];
    struct user_info * uin;
    struct msghead head;
    int now, count, canreply, first=1;
    int hasnewmsg;
    int savemode;

    savemode=uinfo.mode;
    modify_user_mode(MSG);
    good_getyx(&y, &x);
    tmpansi = showansi;
    showansi = 1;
    oldi = i_timeout;
    set_alarm(0, 0, NULL, NULL);
    RMSG = true;
    RMSGCount++;
    for(i=0;i<=23;i++)
        saveline(i, 0, savebuffer[i]);

    hasnewmsg=get_unreadcount(currentuser->userid);
    if ((uinfo.mode == POSTING || uinfo.mode == SMAIL) && !DEFINE(currentuser, DEF_LOGININFORM)) {      /*Haohmaru.99.12.16.·¢ÎÄÕÂÊ±²»»Ømsg */
        good_move(0, 0);
        clrtoeol();
        if (hasnewmsg) {
            prints("[1m[33mÄãÓĞĞÂµÄÑ¶Ï¢£¬Çë·¢±íÍêÎÄÕÂºó°´ Ctrl+Z »ØÑ¶Ï¢[m");
            good_move(y, x);
            refresh();
            sleep(1);
        } else {
            prints("[1mÃ»ÓĞÈÎºÎĞÂµÄÑ¶Ï¢´æÔÚ![m");
            good_move(y, x);
            refresh();
            sleep(1);
        }
        goto outhere;
    }
    count = get_msgcount(1, currentuser->userid);
    if (!count) {
        good_move(0, 0);
        clrtoeol();
        prints("[1mÃ»ÓĞÈÎºÎµÄÑ¶Ï¢´æÔÚ£¡£¡[m");
        good_move(y, x);
        refresh();
        sleep(1);
        goto outhere;
    }

    now = get_unreadmsg(currentuser->userid);
    if(now==-1) now = count-1;
    else {
        load_msghead(1, currentuser->userid, now, &head);
        while(head.topid!=getuinfopid()&&now<count-1){
            now = get_unreadmsg(currentuser->userid);
            load_msghead(1, currentuser->userid, now, &head);
        };
    }
    while(1){
        load_msghead(1, currentuser->userid, now, &head);
        load_msgtext(currentuser->userid, &head, buf);
        translate_msg(buf, &head, outmsg);
        
        if (hasnewmsg&&DEFINE(currentuser, DEF_SOUNDMSG))
            bell();
        good_move(0,0);
        clrtoeol();
        prints("%s", outmsg);

        if(first) {
            refresh();
            do{
                ch = igetkey();
            }while(!DEFINE(currentuser, DEF_IGNOREMSG)&&ch!=Ctrl('Z')&&ch!='r'&&ch!='R');
            first = 0;
        }
        
        strncpy(uid, head.id, IDLEN+2);
        pid = head.frompid;
        uin = t_search(uid, pid);
        if(head.mode==3||uin==NULL) canreply = 0;
        else canreply = 1;
        
        clrtoeol();
        if(canreply)
            prints("[m µÚ %d ÌõÏûÏ¢ / ¹² %d ÌõÏûÏ¢, »Ø¸´ %-12s\n", now+1, count, uid);
        else
            if(uin)
                prints("[m µÚ %d ÌõÏûÏ¢ / ¹² %d ÌõÏûÏ¢,¡ü¡ıÇĞ»»,Enter½áÊø, ¸ÃÏûÏ¢ÎŞ·¨»Ø¸´", now+1, count);
            else
                prints("[m µÚ %d ÌõÏûÏ¢ / ¹² %d ÌõÏûÏ¢,¡ü¡ıÇĞ»»,Enter½áÊø, ÓÃ»§%sÒÑÏÂÕ¾,ÎŞ·¨»Ø¸´", now+1, count, uid);
        good_getyx(&oy, &ox);
        
        if(canreply)
            ch = -multi_getdata(oy, ox, 79, NULL, buf, 1024, 11, true);
        else {
            refresh();
            do {
                ch = igetkey();
            } while(ch!=KEY_UP&&ch!=KEY_DOWN&&ch!='\r'&&ch!='\n');
        }
        for(i=0;i<=oy;i++)
            norefresh_saveline(i, 1, savebuffer[i]);
        switch(ch) {
            case KEY_UP:
                now--;
                if(now<0) now=count-1;
                break;
            case KEY_DOWN:
                now++;
                if(now>=count) now=0;
                break;
            default:
                if(canreply) {
                    if(buf[0]) {
                        strcpy(MsgDesUid, uid);
                        i = do_sendmsg(uin, buf, 4);
                        buf[0]=0;
                        if(i==1) strcpy(buf, "[1m°ïÄãËÍ³öÑ¶Ï¢ÁË[m");
                        else if(i==-1) strcpy(buf, "[1m¶Ô·½ÒÑ¾­ÀëÏßÁË...[m");
                        if(buf[0]) {
                            good_move(0,0);
                            clrtoeol();
                            prints("%s", buf);
                            refresh();
#ifdef NINE_BUILD
                            if(i!=1)
#endif
                            sleep(1);
                        }
                    }
                    ch = '\n';
                }
                break;
        }
        if (ch=='\r'||ch=='\n') break;
    }


outhere:
    for(i=0;i<=23;i++)
        norefresh_saveline(i, 1, savebuffer[i]);
    showansi = tmpansi;
    good_move(y,x);
    refresh();
    if(oldi)
        R_monitor(NULL);
    RMSGCount--;
    if (0 == RMSGCount)
        RMSG = false;
    modify_user_mode(savemode);
    return;
}

void r_lastmsg()
{
    r_msg();
}

int myfriend_wall(struct user_info *uin, char *buf, int i)
{
    if ((uin->pid - uinfo.pid == 0) || !uin->active || !uin->pid || !canmsg(currentuser, uin))
        return -1;
    if (myfriend(uin->uid, NULL)) {
        move(1, 0);
        clrtoeol();
        prints("\x1b[1;32mÕıÔÚËÍÑ¶Ï¢¸ø %s...  \x1b[m", uin->userid);
        refresh();
        strcpy(MsgDesUid, uin->userid);
        do_sendmsg(uin, buf, 5);
    }
    return 0;
}

int friend_wall()
{
    char buf[MAX_MSG_SIZE];

    if (uinfo.invisible) {
        move(2, 0);
        prints("±§Ç¸, ´Ë¹¦ÄÜÔÚÒşÉí×´Ì¬ÏÂ²»ÄÜÖ´ĞĞ...\n");
        pressreturn();
        return 0;
    }
    modify_user_mode(MSG);
    move(2, 0);
    clrtobot();
    if (!get_msg("ÎÒµÄºÃÅóÓÑ", buf, 1))
        return 0;
    if (apply_ulist(myfriend_wall, buf) == -1) {
        move(2, 0);
        prints("ÏßÉÏ¿ÕÎŞÒ»ÈË\n");
        pressanykey();
    }
    move(6, 0);
    prints("Ñ¶Ï¢´«ËÍÍê±Ï...");
    pressanykey();
    return 1;
}
