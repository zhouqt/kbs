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
extern bool inremsg;

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

    inremsg = true;

    if ((mode == 0) || (mode == 3)) {
        modify_user_mode(MSG);
    }
    if (uentp == NULL) {
        prints("<ÊäÈëÊ¹ÓÃÕß´úºÅ>\n");
        move(1, 0);
        clrtobot();
        prints("ËÍÑ¶Ï¢¸ø: ");
        creat_list();
        namecomplete(NULL, uident);
        if (uident[0] == '\0') {
            clear();
            inremsg = false;
            return 0;
        }
        uin = t_search(uident, false);
        if (uin == NULL) {
            move(2, 0);
            prints("¶Ô·½Ä¿Ç°²»ÔÚÏßÉÏ£¬»òÊÇÊ¹ÓÃÕß´úºÅÊäÈë´íÎó...");
            pressreturn();
            move(2, 0);
            clrtoeol();
            inremsg = false;
            return -1;
        }
        if (!canmsg(currentuser, uin)) {
            move(2, 0);
            prints("¶Ô·½ÒÑ¾­¹Ø±Õ½ÓÊÜÑ¶Ï¢µÄºô½ĞÆ÷...");
            pressreturn();
            move(2, 0);
            clrtoeol();
            inremsg = false;
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
            clear();
        }
        inremsg = false;
        return 1;
        break;
    case -1:                   /* failed, reason in msgerr */
        if (mode == 0) {
            move(2, 0);
            clrtobot();
            prints(msgerr);
            pressreturn();
            move(2, 0);
            clrtoeol();
        }
        inremsg = false;
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
            inremsg = false;
            return 0;
        }
        break;
    default:                   /* unknown reason */
        inremsg = false;
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
            clear();
        }
        inremsg = false;
        return 1;
        break;
    case -1:                   /* failed, reason in msgerr */
        if (mode == 0) {
            move(2, 0);
            clrtobot();
            prints(msgerr);
            pressreturn();
            move(2, 0);
            clrtoeol();
        }
        inremsg = false;
        return -1;
        break;
    default:                   /* unknown reason */
        inremsg = false;
        return result;
        break;
    }
    inremsg = false;
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
            move(5,30);
            prints("[mÃ»ÓĞÈÎºÎµÄÑ¶Ï¢´æÔÚ£¡£¡");
            i = 0;
        }
        else {
            y = 0;
            i = page;
            load_msghead(all?2:0, currentuser->userid, i, &head);
            load_msgtext(currentuser->userid, &head, buf);
            j = translate_msg(buf, &head, showmsg);
            while(y<=t_lines-1) {
                y+=j; i++;
                prints("%s", showmsg);
                if(i>=count) break;
                load_msghead(all?2:0, currentuser->userid, i, &head);
                load_msgtext(currentuser->userid, &head, buf);
                j = translate_msg(buf, &head, showmsg);
            }
        }
        move(t_lines-1,0);
        if(!all)
            prints("[1;44;32mÑ¶Ï¢ä¯ÀÀÆ÷   ±£Áô<[37mr[32m> Çå³ı<[37mc[32m> ¼Ä»ØĞÅÏä<[37mm[32m> ·¢Ñ¶ÈË<[37mi[32m> Ñ¶Ï¢ÄÚÈİ<[37ms[32m>        Ê£Óà:%4d ", count-i);
        else
            prints("[1;44;32mÑ¶Ï¢ä¯ÀÀÆ÷   ±£Áô<[37mr[32m> Çå³ı<[37mc[32m> ¼Ä»ØĞÅÏä<[37mm[32m> ·¢Ñ¶ÈË<[37mi[32m> Ñ¶Ï¢ÄÚÈİ<[37ms[32m> È«²¿<[37ma[32m>     %4d ", count-i);
        clrtoeol();
        resetcolor();
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
                move(t_lines-1, 0);
                clrtoeol();
                getdata(t_lines-1, 0, "ÇëÊäÈë¹Ø¼ü×Ö:", chk, 50, true, NULL, true);
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
                if(count!=0)mail_msg(currentuser);
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
    char savebuffer[25][LINELEN*3];
    char buf[MAX_MSG_SIZE+100], outmsg[MAX_MSG_SIZE*2], buf2[STRLEN], uid[14];
    struct user_info * uin;
    struct msghead head;
    int now, count, canreply, first=1;
    int hasnewmsg;
    int savemode;

    noscroll();
    savemode=uinfo.mode;
    modify_user_mode(MSG);
    getyx(&y, &x);
    tmpansi = showansi;
    showansi = 1;
    oldi = i_timeout;
    set_alarm(0, 0, NULL, NULL);
    RMSG = true;
    RMSGCount++;
    for(i=0;i<=23;i++)
        saveline(i, 0, savebuffer[i]);

    hasnewmsg=get_unreadcount(currentuser->userid);
    if ((savemode == POSTING || savemode == SMAIL) && !DEFINE(currentuser, DEF_LOGININFORM)) {      /*Haohmaru.99.12.16.·¢ÎÄÕÂÊ±²»»Ømsg */
        move(0, 0);
        if (hasnewmsg) {
            prints("[1m[33mÄãÓĞĞÂµÄÑ¶Ï¢£¬Çë·¢±íÍêÎÄÕÂºó°´ Ctrl+Z »ØÑ¶Ï¢[m");
            move(y, x);
            refresh();
            sleep(1);
        } else {
            prints("[1mÃ»ÓĞÈÎºÎĞÂµÄÑ¶Ï¢´æÔÚ![m");
            move(y, x);
            refresh();
            sleep(1);
        }
        clrtoeol();
        goto outhere;
    }
    count = get_msgcount(1, currentuser->userid);
    if (!count) {
        move(0, 0);
        prints("[1mÃ»ÓĞÈÎºÎµÄÑ¶Ï¢´æÔÚ£¡£¡[m");
        clrtoeol();
        move(y, x);
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
        
        if (first&&hasnewmsg&&DEFINE(currentuser, DEF_SOUNDMSG))
            bell();
        move(0,0);
        prints("%s", outmsg);

        if(first) {
            int x,y;
            getyx(&y,&x);
            prints("[m µÚ %d ÌõÏûÏ¢ / ¹² %d ÌõÏûÏ¢", now+1, count);
            clrtoeol();
            do{
                ch = igetkey();
            }while(!DEFINE(currentuser, DEF_IGNOREMSG)&&ch!=Ctrl('Z')&&ch!='r'&&ch!='R');
            first = 0;
            move(y, x);
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
        getyx(&oy, &ox);
        
        if(canreply) {
            ch = -multi_getdata(oy, ox, 79, NULL, buf, 1024, 11, true);
            if(ch<0) oy-=ch+1;
        }
        else {
            do {
                ch = igetkey();
            } while(ch!=KEY_UP&&ch!=KEY_DOWN&&ch!='\r'&&ch!='\n');
        }
        switch(ch) {
            case Ctrl('Z'):
                ch = '\n';
                break;
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
                        pid = head.frompid;
                        uin = t_search(uid, pid);
                        if(uin==NULL) {
                            i=-1;
                            strcpy(msgerr, "¶Ô·½ÒÑ¾­ÀëÏß....");
                        }
                        else
                            i = sendmsgfunc(uin, buf, 4);
                        buf[0]=0;
                        if(i==1) strcpy(buf, "[1m°ïÄãËÍ³öÑ¶Ï¢ÁË[m");
                        else if(i!=0) strcpy(buf, msgerr);
                        if(buf[0]) {
                            int j=i;
                            if(i!=1&&i!=0) {
                                move(oy+1, 0);
                                prints("%s °´ÈÎÒâ¼ü¼ÌĞø", buf);
                                igetkey();
                                saveline(oy+1, 1, savebuffer[oy+1]);
                            }
                            else {
                                for(i=0;i<=oy;i++)
                                    saveline(i, 1, savebuffer[i]);
                                move(0,0);
                                clrtoeol();
                                prints("%s", buf);
                            }
#ifndef NINE_BUILD
                            if(j==1) {
                                refresh();
                                sleep(1);
                            }
#endif
                        }
                    }
                    ch = '\n';
                }
                break;
        }
        for(i=0;i<=oy;i++)
            saveline(i, 1, savebuffer[i]);
        if (ch=='\r'||ch=='\n') {
        	// make a tag for msg end
//        	prints("\x1b[mÒÑ·¢³öÏûÏ¢");
        	break;
        }
    }


outhere:
    for(i=0;i<=23;i++)
        saveline(i, 1, savebuffer[i]);
    showansi = tmpansi;
    move(y,x);
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

#ifdef SMS_SUPPORT

#ifndef byte
typedef unsigned char byte;
#endif

#define SMS_SHM_SIZE 1024*50

struct header{
  char Type;
  byte SerialNo[4];
  byte pid[4];
  byte BodyLength[4];   //×ÜPacket³¤¶È
};

#define CMD_LOGIN 1
#define CMD_OK 101
#define CMD_ERR 102
#define CMD_LOGOUT 2
#define CMD_REG 3
#define CMD_CHECK 4
#define CMD_UNREG 5
#define CMD_REQUIRE 6
#define CMD_REPLY 7
#define CMD_BBSSEND 8
#define CMD_GWSEND 9

#define MOBILE_NUMBER_LEN 17

struct RegMobileNoPacket { //Type=3
    char MobileNo[MOBILE_NUMBER_LEN];
};
struct CheckMobileNoPacket { //Type=4
    char MobileNo[MOBILE_NUMBER_LEN];
    char ValidateNo[MOBILE_NUMBER_LEN];
};
struct UnRegPacket { //Type=5
    char MobileNo[MOBILE_NUMBER_LEN];
};
struct BBSSendSMS { //Type=8
    byte UserID[4];
    char SrcMobileNo[MOBILE_NUMBER_LEN];
    char DstMobileNo[MOBILE_NUMBER_LEN];
    byte MsgTxtLen[4];
};

struct sms_shm_head {
    int sem;
    int total;
    int length;
} * head;
void * buf=NULL;
int result=0;

inline unsigned int byte2long(byte arg[4]) {
    unsigned int tmp;
    tmp=((long)arg[0]<<24)+((long)arg[1]<<16)+((long)arg[2]<<8)+(long)arg[3];
    return tmp;
}

inline void long2byte(unsigned int num, byte* arg) {
    (arg)[0]=num>>24;
    (arg)[1]=(num<<8)>>24;
    (arg)[2]=(num<<16)>>24;
    (arg)[3]=(num<<24)>>24;
}


int init_memory()
{
    void * p;
    int iscreate;
    if(buf) return 0;

    iscreate = 0;
    p = attach_shm("SMS_SHMKEY", 8914, SMS_SHM_SIZE+sizeof(struct sms_shm_head), &iscreate);
    head = (struct sms_shm_head *) p;
    buf = p+sizeof(struct sms_shm_head);
}

void sendtosms(void * n, int s)
{
    if(head->length+s>=SMS_SHM_SIZE) return;
    memcpy(buf+head->length, n, s);
    head->length+=s;
}

void SMS_request(int signo)
{
    char fn[80];
    struct stat st;
    sprintf(fn, "tmp/%d.res", uinfo.pid);
    if(stat(fn, &st)!=-1)
        result=1;
}

int wait_for_result()
{
    int count;
    char fn[80];
    FILE* fp;
    int i;
    signal(SIGUSR1, SMS_request);
    sprintf(fn, "tmp/%d.res", uinfo.pid);
    unlink(fn);
    result = 0;
    head->sem=0;

    count=0;
    while(!result) {
        move(t_lines-1, 0);
        clrtoeol();
        prints("·¢ËÍÖĞ....%d%%", count*100/30);
        refresh();
        sleep(1);
        count++;
        if(count>30) {
            move(t_lines-1, 0);
            clrtoeol();
            return -1;
        }
    }
    signal(SIGUSR1, talk_request);
    
    move(t_lines-1, 0);
    clrtoeol();
    fp=fopen(fn, "r");
    fscanf("%d", &i);
    fclose(fp);
    if(i==1) return 0;
    else return -1;
}

int DoReg(char * n)
{
    int count=0;
    struct header h;
    struct RegMobileNoPacket h1;
    h.Type = CMD_REG;
    long2byte(uinfo.pid, h.pid);
    long2byte(sizeof(h1), h.BodyLength);
    strcpy(h1.MobileNo, n);
    while(head->sem) {
        sleep(1);
        count++;
        if(count>=5) return -1;
    }
    head->sem=1;
    head->total++;
    sendtosms(&h, sizeof(h));
    sendtosms(&h1, sizeof(h1));
    return wait_for_result();
}

int DoUnReg(char * n)
{
    int count=0;
    struct header h;
    struct UnRegPacket h1;
    h.Type = CMD_REG;
    long2byte(uinfo.pid, h.pid);
    long2byte(sizeof(h1), h.BodyLength);
    strcpy(h1.MobileNo, n);
    while(head->sem) {
        sleep(1);
        count++;
        if(count>=5) return -1;
    }
    head->sem=1;
    head->total++;
    sendtosms(&h, sizeof(h));
    sendtosms(&h1, sizeof(h1));
    return wait_for_result();
}

int DoCheck(char * n, char * c)
{
    int count=0;
    struct header h;
    struct CheckMobileNoPacket h1;
    h.Type = CMD_CHECK;
    long2byte(uinfo.pid, h.pid);
    long2byte(sizeof(h1), h.BodyLength);
    strcpy(h1.MobileNo, n);
    strcpy(h1.ValidateNo, c);
    while(head->sem) {
        sleep(1);
        count++;
        if(count>=5) return -1;
    }
    head->sem=1;
    head->total++;
    sendtosms(&h, sizeof(h));
    sendtosms(&h1, sizeof(h1));
    return wait_for_result();
}

int DoSendSMS(char * n, char * d, char * c)
{
    int count=0;
    struct header h;
    struct BBSSendSMS h1;
    h.Type = CMD_BBSSEND;
    long2byte(uinfo.pid, h.pid);
    long2byte(sizeof(h1), h.BodyLength);
    long2byte(strlen(c)+1, h1.MsgTxtLen);
    long2byte(uinfo.uid, h1.UserID);
    strcpy(h1.SrcMobileNo, n);
    strcpy(h1.DstMobileNo, d);
    while(head->sem) {
        sleep(1);
        count++;
        if(count>=5) return -1;
    }
    head->sem=1;
    head->total++;
    sendtosms(&h, sizeof(h));
    sendtosms(&h1, sizeof(h1));
    sendtosms(c, strlen(c)+1);
    return wait_for_result();
}

int register_sms()
{
    char ans[4];
    char valid[20];
    char buf2[80];
    init_memory();
    clear();
    prints("×¢²áÊÖ»úºÅ\n\n×¢²áÄãµÄÊÖ»úºÅÖ®ºó£¬Äã¿ÉÔÚbbsÉÏ·¢ËÍºÍ½ÓÊÕÊÖ»ú¶ÌĞÅ\n");
    move(4,0);
    if(curruserdata.mobileregistered) {
        prints("ÄãÒÑ¾­×¢²áÊÖ»úºÅÁË¡£Ã¿Ò»¸öÕËºÅÖ»ÄÜ×¢²áÒ»¸öÊÖ»úºÅ¡£\n");
        pressreturn();
        shmdt(head);
        buf=NULL;
        return -1;
    }
    if(curruserdata.mobilenumber[0]&&strlen(curruserdata.mobilenumber)==11) {
        sprintf(buf2, "ÄãÊäÈëµÄÊÖ»úºÅÊÇ%s£¬ÊÇ·ñÖØĞÂ·¢ËÍ×¢²áÂë£¿[Y/n]", curruserdata.mobilenumber);
        getdata(3, 0, buf2, ans, 3, 1, 0, 1);
        if(toupper(ans[0])!='N') curruserdata.mobilenumber[0]=0;
    }
    if(!curruserdata.mobilenumber[0]||strlen(curruserdata.mobilenumber)!=11) {
        getdata(4, 0, "ÇëÊäÈëÊÖ»úºÅ: ", curruserdata.mobilenumber, 17, 1, 0, 1);
        if(!curruserdata.mobilenumber[0]||strlen(curruserdata.mobilenumber)!=11) {
            move(5, 0);
            prints("´íÎóµÄÊÖ»úºÅ");
            pressreturn();
            shmdt(head);
            buf=NULL;
            return -1;
        }
        if(DoReg(curruserdata.mobilenumber)) {
            move(5, 0);
            prints("·¢ËÍ×¢²áÂëÊ§°Ü");
            pressreturn();
            shmdt(head);
            buf=NULL;
            return -1;
        }
        move(5, 0);
        prints("·¢ËÍ×¢²áÂë³É¹¦");
    }
    getdata(6, 0, "ÇëÊäÈëÄãµÄ×¢²áÂë: ", valid, 11, 1, 0, 1);
    if(!valid[0]) return -1;
    if(DoCheck(curruserdata.mobilenumber, valid)) {
        move(7, 0);
        prints("×¢²áÂë¼ì²éÊ§°Ü");
        pressreturn();
        shmdt(head);
        buf=NULL;
        return -1;
    }
    curruserdata.mobileregistered = 1;
    move(7, 0);
    prints("ÊÖ»ú×¢²á³É¹¦£¡ Äã¿ÉÒÔÔÚbbsÉÏ·¢ËÍ¶ÌĞÅÀ²£¡");
    pressreturn();
    shmdt(head);
    buf=NULL;
}

int unregister_sms()
{
    char ans[4];
    char valid[20];
    char buf2[80];
    init_memory();
    clear();
    prints("È¡Ïû×¢²áÊÖ»úºÅ");
    move(4,0);
    if(!curruserdata.mobileregistered) {
        prints("ÄãÉĞÎ´×¢²áÊÖ»úºÅ");
        pressreturn();
        shmdt(head);
        buf=NULL;
        return -1;
    }
    sprintf(buf2, "ÄãÊäÈëµÄÊÖ»úºÅÊÇ%s£¬ÊÇ·ñÈ¡Ïû×¢²á£¿[y/N]", curruserdata.mobilenumber);
    getdata(3, 0, buf2, ans, 3, 1, 0, 1);
    if(toupper(ans[0])=='Y') {
        if(DoUnReg(curruserdata.mobilenumber)) {
            move(5, 0);
            prints("È¡Ïû×¢²áÊ§°Ü");
            pressreturn();
            shmdt(head);
            buf=NULL;
            return -1;
        }
        move(5, 0);
        prints("È¡Ïû×¢²á³É¹¦");
        curruserdata.mobilenumber[0]=0;
        curruserdata.mobileregistered = 0;
    }
    shmdt(head);
    buf=NULL;
}


#endif
