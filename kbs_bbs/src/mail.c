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
#define         INTERNET_PRIVATE_EMAIL

/*For read.c*/
int auth_search_down();
int auth_search_up();
int do_cross();
int t_search_down();
int t_search_up();
int thread_up();
int thread_down();
int deny_user();
int show_author();
int show_authorinfo();          /*Haohmaru.98.12.19 */
int show_authorBM();            /*cityhunter 00.10.18 */
int SR_first_new();
int SR_last();
int SR_first();
int SR_read();
int SR_readX();                 /* Leeward 98.10.03 */
int SR_author();
int SR_authorX();               /* Leeward 98.10.03 */
int G_SENDMODE = false;
extern int add_author_friend();
int cmpinames();                /* added by Leeward 98.04.10 */

extern int numofsig;
extern char quote_user[];
char *sysconf_str();
//char currmaildir[STRLEN];
extern char currdirect[];

#define maxrecp 300

static int mail_reply(int ent, struct fileheader *fileinfo, char *direct);
static int mail_del(int ent, struct fileheader *fileinfo, char *direct);
static int do_gsend(char *userid[], char *title, int num);

int chkmail()
{
    static time_t lasttime = 0;
    static int ismail = 0;
    struct fileheader fh;
    struct stat st;
    int fd;
    int i, offset;
    long numfiles;
    unsigned char ch;
    char curmaildir[STRLEN];

    setmailfile(curmaildir, currentuser->userid, DOT_DIR);

    if (!HAS_PERM(currentuser, PERM_BASIC)) {
        return 0;
    }
    /*
     * ylsdd 2001.4.23: ¼ì²âÎÄ¼ş×´Ì¬Ó¦¸ÃÔÚget_mailnum£¬get_sum_recordsÖ®Ç°£¬·ñÔòÆñ²»ÊÇ
     * Òª×ö´óÁ¿ÎŞÓÃµÄÏµÍ³µ÷ÓÃ. ÔÚÕâ¸ö¸Ä¶¯ÖĞÒ²°Ñfstat¸ÄÎªstatÁË£¬½ÚÊ¡Ò»¸öopen&close 
     */
    if (stat(curmaildir, &st) < 0)
        return (ismail = 0);
    if (lasttime >= st.st_mtime)
        return ismail;


    if (chkusermail(currentuser))
        return (ismail = 2);
    offset = (int) ((char *) &(fh.accessed[0]) - (char *) &(fh));
    if ((fd = open(curmaildir, O_RDONLY)) < 0)
        return (ismail = 0);
    lasttime = st.st_mtime;
    numfiles = st.st_size;
    numfiles = numfiles / sizeof(fh);
    if (numfiles <= 0) {
        close(fd);
        return (ismail = 0);
    }
    lseek(fd, (st.st_size - (sizeof(fh) - offset)), SEEK_SET);
    for (i = 0; i < numfiles; i++) {
        read(fd, &ch, 1);
        if (!(ch & FILE_READ)) {
            close(fd);
            return (ismail = 1);
        }
        lseek(fd, -sizeof(fh) - 1, SEEK_CUR);
    }
    close(fd);
    return (ismail = 0);
}

int get_mailnum()
{
    struct fileheader fh;
    struct stat st;
    int fd;
    register int numfiles;

    if ((fd = open(currdirect, O_RDONLY)) < 0)
        return 0;
    fstat(fd, &st);
    numfiles = st.st_size;
    numfiles = numfiles / sizeof(fh);
    close(fd);
    return numfiles;
}


static int mailto(struct userec *uentp, char *arg)
{
    char filename[STRLEN];
    int mailmode = (int) arg;

    sprintf(filename, "etc/%s.mailtoall", currentuser->userid);
    if ((uentp->userlevel == PERM_BASIC && mailmode == 1) ||
        (!HAS_PERM(uentp, PERM_DENYMAIL) && mailmode == 2) || (uentp->userlevel & PERM_BOARDS && mailmode == 3) || (uentp->userlevel & PERM_CHATCLOAK && mailmode == 4)) {
        mail_file(currentuser->userid, filename, uentp->userid, save_title, 0, NULL);
    }
    return 1;
}

static int mailtoall(int mode)
{

    return apply_users(mailto, (char *) mode);
}

int mailall()
{
    char ans[4], ans4[4], ans2[4], fname[STRLEN], title[STRLEN];
    char doc[4][STRLEN], buf[STRLEN];
    char buf2[STRLEN], include_mode = 'Y';
    char buf3[STRLEN], buf4[STRLEN];
    int i, replymode = 0;       /* Post New UI */

    strcpy(title, "Ã»Ö÷Ìâ");
    buf4[0] = '\0';
    modify_user_mode(SMAIL);
    clear();
    move(0, 0);
    sprintf(fname, "etc/%s.mailtoall", currentuser->userid);
    prints("ÄãÒª¼Ä¸øËùÓĞµÄ£º\n");
    prints("(0) ·ÅÆú\n");
    strcpy(doc[0], "(1) Î´ÈÏÖ¤Éí·İÕß");
    strcpy(doc[1], "(2) ÒÑÈÏÖ¤Éí·İÕß");
    strcpy(doc[2], "(3) ÓĞ°æÖ÷È¨ÏŞÕß");
    strcpy(doc[3], "(4) ÖÇÄÒÍÅ³ÉÔ±");
    for (i = 0; i < 4; i++)
        prints("%s\n", doc[i]);
    while (1) {
        getdata(8, 0, "ÇëÊäÈëÄ£Ê½ (0~4)? [0]: ", ans4, 2, DOECHO, NULL, true);

        move(9,0);
        clrtoeol();
        if (!askyn("ÕæµÄÒªÈºĞÅÃ´", 0)) {
            clear();
            return -1;
        }
        if (ans4[0] - '0' >= 1 && ans4[0] - '0' <= 4) {
            sprintf(buf, "ÊÇ·ñÈ·¶¨¼Ä¸ø%s (Y/N)? [N]: ", doc[ans4[0] - '0' - 1]);
            getdata(9, 0, buf, ans2, 2, DOECHO, NULL, true);
            if (ans2[0] != 'Y' && ans2[0] != 'y') {
                return -1;
            }
            in_mail = true;
            /*
             * Leeward 98.01.17 Prompt whom you are writing to 
             */
            /*
             * strcpy(currentlookupuser->userid, doc[ans4[0]-'0'-1] + 4); 
             */

            if (currentuser->signature > numofsig)
                currentuser->signature = 1;
            while (1) {
                sprintf(buf3, "ÒıÑÔÄ£Ê½ [[1m%c[m]", include_mode);
                move(t_lines - 4, 0);
                clrtoeol();
                prints("ÊÕĞÅÈË: [1m%s[m\n", doc[ans4[0] - '0' - 1]);
                clrtoeol();
                prints("Ê¹ÓÃ±êÌâ: [1m%-50s[m\n", (title[0] == '\0') ? "[ÕıÔÚÉè¶¨±êÌâ]" : title);
                clrtoeol();
                if (currentuser->signature < 0)
                    prints("Ê¹ÓÃËæ»úÇ©Ãûµµ     %s", (replymode) ? buf3 : "");
                else
                    prints("Ê¹ÓÃµÚ [1m%d[m ¸öÇ©Ãûµµ     %s", currentuser->signature, (replymode) ? buf3 : "");

                if (buf4[0] == '\0' || buf4[0] == '\n') {
                    move(t_lines - 1, 0);
                    clrtoeol();
                    getdata(t_lines - 1, 0, "±êÌâ: ", buf4, 50, DOECHO, NULL, true);
                    if ((buf4[0] == '\0' || buf4[0] == '\n')) {
                        buf4[0] = ' ';
                        continue;
                    }
                    strcpy(title, buf4);
                    continue;
                }
                move(t_lines - 1, 0);
                clrtoeol();
                /*
                 * Leeward 98.09.24 add: viewing signature(s) while setting post head 
                 */
                sprintf(buf2, "°´[1;32m0[m~[1;32m%d/V/L[mÑ¡/¿´/Ëæ»úÇ©Ãûµµ%s£¬[1;32mT[m¸Ä±êÌâ£¬[1;32mEnter[m½ÓÊÜËùÓĞÉè¶¨: ", numofsig,
                        (replymode) ? "£¬[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m¸ÄÒıÑÔÄ£Ê½" : "");
                getdata(t_lines - 1, 0, buf2, ans, 3, DOECHO, NULL, true);
                ans[0] = toupper(ans[0]);       /* Leeward 98.09.24 add; delete below toupper */
                if ((ans[0] - '0') >= 0 && ans[0] - '0' <= 9) {
                    if (atoi(ans) <= numofsig)
                        currentuser->signature = atoi(ans);
                } else if ((ans[0] == 'Y' || ans[0] == 'N' || ans[0] == 'A' || ans[0] == 'R') && replymode) {
                    include_mode = ans[0];
                } else if (ans[0] == 'T') {
                    buf4[0] = '\0';
                } else if (ans[0] == 'V') {     /* Leeward 98.09.24 add: viewing signature(s) while setting post head */
                    sethomefile(buf2, currentuser->userid, "signatures");
                    move(t_lines - 1, 0);
                    if (askyn("Ô¤ÉèÏÔÊ¾Ç°Èı¸öÇ©Ãûµµ, ÒªÏÔÊ¾È«²¿Âğ", false) == true)
                        ansimore(buf2, 0);
                    else {
                        clear();
                        ansimore2(buf2, false, 0, 18);
                    }
                } else if (ans[0] == 'L') {
                    currentuser->signature = -1;
                } else {
                    extern char quote_title[120];

                    strncpy(save_title, title, STRLEN);
                    strncpy(quote_title, title, STRLEN);
                    break;
                }
            }
            do_quote(fname, include_mode, "", quote_user);
            if (vedit(fname, true, NULL, NULL) == -1) {
                in_mail = false;
                unlink(fname);
                clear();
                return -2;
            }
            move(t_lines - 1, 0);
            clrtoeol();
            prints("[32m[44mÕıÔÚ¼ÄĞÅ¼şÖĞ£¬ÇëÉÔºò.....                                                        [m");
            mailtoall(ans4[0] - '0');
            move(t_lines - 1, 0);
            clrtoeol();
            unlink(fname);
            in_mail = false;
            return 0;
        } else
            in_mail = false;
        return 0;
    }
    return -1;
}

void m_internet()
{
    char receiver[STRLEN], title[STRLEN];

    modify_user_mode(SMAIL);
    getdata(1, 0, "ÊÕĞÅÈË: ", receiver, 70, DOECHO, NULL, true);
    getdata(2, 0, "Ö÷Ìâ  : ", title, 70, DOECHO, NULL, true);
    if (!invalidaddr(receiver) && strchr(receiver, '@') && strlen(title) > 0) {
        clear();                /* Leeward 98.09.24fix a bug */
        switch (do_send(receiver, title, "")) { /* Leeward 98.05.11 adds "switch" */
        case -1:
            prints("ÊÕĞÅÕß²»ÕıÈ·\n");
            break;
        case -2:
            prints("È¡Ïû·¢ĞÅ\n");
            break;
        case -3:
            prints("'%s' ÎŞ·¨ÊÕĞÅ\n", receiver);
            break;
        case -4:
            clear();
            move(1, 0);
            prints("%s ĞÅÏäÒÑÂú,ÎŞ·¨ÊÕĞÅ\n", receiver);
            break;              /*Haohmaru.4.5.ÊÕĞÅÏŞÖÆ */
        case -5:
            clear();
            move(1, 0);
            prints("%s ×ÔÉ±ÖĞ£¬²»ÄÜÊÕĞÅ\n", receiver);
            break;              /*Haohmaru.99.10.26.×ÔÉ±Õß²»ÄÜÊÕĞÅ */
        case -552:
            prints("\n[1m[33mĞÅ¼ş³¬³¤£¨±¾Õ¾ÏŞ¶¨ĞÅ¼ş³¤¶ÈÉÏÏŞÎª %d ×Ö½Ú£©£¬È¡Ïû·¢ĞÅ²Ù×÷[m[m\n", MAXMAILSIZE);
            break;
        default:
            prints("ĞÅ¼şÒÑ¼Ä³ö\n");
        }
        pressreturn();

    } else {
        move(3, 0);
        prints("ÊÕĞÅÈË»òÖ÷Ìâ²»ÕıÈ·, ÇëÖØĞÂÑ¡È¡Ö¸Áî\n");
        pressreturn();
    }
    clear();
}


/* ·µ»ØÖµ¶¨Òå: 
  *   -1  ÊÕĞÅÕß²»´æÔÚ;  -2 È¡Ïû·¢ĞÅ;
  *   -3  ÎŞ·¨ÊÕĞÅ   -4 ĞÅÏäÒÑÂú
  *   -5  ×ÔÉ±ÖĞÎŞ·¨ÊÕĞÅ    -552 ĞÅ¼ş³¬³¤?
  *    (´íÎó´úÂëÍ³Ò»¶¨Òå²ÅºÃ£¬²»È»Ì«»ìÂÒ£¬Î¬»¤ºÜ²»Ë¬¡£:(  )
  */
int do_send(char *userid, char *title, char *q_file)
{
    struct fileheader newmessage;
    struct stat st;
    char filepath[STRLEN], fname[STRLEN], *ip;
    int sum, sumlimit;
    char buf2[256], buf3[STRLEN], buf4[STRLEN];
    int replymode = 1;          /* Post New UI */
    char ans[4], include_mode = 'Y';

    int internet_mail = 0;
    char tmp_fname[256];
    int noansi;
    struct userec *user;
    extern char quote_title[120];
    int ret;
    char* upload = NULL;

    if (HAS_PERM(currentuser, PERM_DENYMAIL)) {
        prints("[1m[33mºÜ±§Ç¸¡ÃÄúÎŞ·¨¸ø %s ·¢ĞÅ£®ÒòÎª Äú±»·â½ûÁËMailÈ¨ÏŞ¡£\n[m");
        return -2;
    }
    if (!strchr(userid, '@')) {
        if (getuser(userid, &user) == 0)
            return -1;
        ret = chkreceiver(currentuser, user);

	if (false == canIsend2(currentuser, userid)) {  /* Leeward 98.04.10 */
            prints("[1m[33mºÜ±§Ç¸¡ÃÄúÎŞ·¨¸ø %s ·¢ĞÅ£®ÒòÎª %s ¾Ü¾ø½ÓÊÕÄúµÄĞÅ¼ş£®[m[m\n\n", userid,userid);
            return -2;
        }

        if (ret == 1)
            return -3;
        /*
         * SYSOPÒ²ÄÜ¸ø×ÔÉ±µÄÈË·¢ĞÅ
         */


        if (ret == 2) {
            move(1, 0);
            prints("ÄãµÄĞÅÏäÈİÁ¿³¬³öÉÏÏŞ, ÎŞ·¨·¢ËÍĞÅ¼ş¡£", sum, sumlimit);
            pressreturn();
            return -2;
        }
        if (ret == 3)
            return -4;
    }
#ifdef INTERNET_PRIVATE_EMAIL
    /*
     * I hate go to , but I use it again for the noodle code :-)
     */
    else {
        /*
         * if(!strstr(userid,"edu.tw")){
         * if(strstr(userid,"@bbs.ee.nthu."))
         * strcat(userid,"edu.tw");
         * else
         * strcat(userid,".edu.tw");}
         */
        if (chkusermail(currentuser)) {
            move(1, 0);
            prints("ÄãµÄĞÅÏäÈİÁ¿³¬³öÉÏÏŞ, ÎŞ·¨·¢ËÍĞÅ¼ş¡£", sum, sumlimit);
            pressreturn();
            return -2;
        }
        internet_mail = 1;
        modify_user_mode(IMAIL);
        buf4[0] = ' ';
		gettmpfilename(tmp_fname, "bbs-internet-gw" );
        //sprintf(tmp_fname, "tmp/bbs-internet-gw/%05d", getpid());
        strcpy(filepath, tmp_fname);
        goto edit_mail_file;
    }
    /*
     * end of kludge for internet mail
     */
#endif

    setmailpath(filepath, userid);
    if (stat(filepath, &st) == -1) {
        if (mkdir(filepath, 0755) == -1)
            return -1;
    } else {
        if (!(st.st_mode & S_IFDIR))
            return -1;
    }

    memset(&newmessage, 0, sizeof(newmessage));
    GET_MAILFILENAME(fname, filepath);
    strcpy(newmessage.filename, fname);


    /*
     * strncpy(newmessage.title,title,STRLEN) ;
     */
    in_mail = true;
#if defined(MAIL_REALNAMES)
    sprintf(genbuf, "%s (%s)", currentuser->userid, currentuser->realname);
#else
/*sprintf(genbuf,"%s (%s)",currentuser->userid,currentuser->username) ;*/
    strcpy(genbuf, currentuser->userid);        /* Leeward 98.04.14 */
#endif
    strncpy(newmessage.owner, genbuf, OWNER_LEN);
    newmessage.owner[OWNER_LEN - 1] = 0;

    setmailfile(filepath, userid, fname);

#ifdef INTERNET_PRIVATE_EMAIL
  edit_mail_file:
#endif
    if (!title) {
        replymode = 0;
        title = "Ã»Ö÷Ìâ";
        buf4[0] = '\0';
    } else
        buf4[0] = ' ';

    if (currentuser->signature > numofsig)
        currentuser->signature = 1;
    while (1) {
        sprintf(buf3, "ÒıÑÔÄ£Ê½ [[1m%c[m]", include_mode);
        move(t_lines - 4, 0);
        clrtoeol();
        prints("ÊÕĞÅÈË: [1m%s[m\n", userid);
        clrtoeol();
        prints("Ê¹ÓÃ±êÌâ: [1m%-50s[m\n", (title[0] == '\0') ? "[ÕıÔÚÉè¶¨±êÌâ]" : title);
        clrtoeol();
        if (currentuser->signature < 0)
            prints("Ê¹ÓÃËæ»úÇ©Ãûµµ     %s", (replymode) ? buf3 : "");
        else
            prints("Ê¹ÓÃµÚ [1m%d[m ¸öÇ©Ãûµµ     %s", currentuser->signature, (replymode) ? buf3 : "");

        if (buf4[0] == '\0' || buf4[0] == '\n') {
            move(t_lines - 1, 0);
            clrtoeol();
            getdata(t_lines - 1, 0, "±êÌâ: ", buf4, 50, DOECHO, NULL, true);
            if ((buf4[0] == '\0' || buf4[0] == '\n')) {
                buf4[0] = ' ';
                continue;
            }
            title = buf4;
            continue;
        }
        move(t_lines - 1, 0);
        clrtoeol();
        /*
         * Leeward 98.09.24 add: viewing signature(s) while setting post head 
         */
        sprintf(buf2, "°´ [1;32m0[m~[1;32m%d/V/L[mÑ¡/¿´/Ëæ»úÇ©Ãûµµ%s£¬[1;32mT[m¸Ä±êÌâ£¬[1;32mEnter[m½ÓÊÜËùÓĞÉè¶¨: ", numofsig,
                (replymode) ? "£¬[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m¸ÄÒıÑÔÄ£Ê½" : "");
        getdata(t_lines - 1, 0, buf2, ans, 3, DOECHO, NULL, true);
        ans[0] = toupper(ans[0]);       /* Leeward 98.09.24 add; delete below toupper */
        if ((ans[0] - '0') >= 0 && ans[0] - '0' <= 9) {
            if (atoi(ans) <= numofsig)
                currentuser->signature = atoi(ans);
        } else if ((ans[0] == 'Y' || ans[0] == 'N' || ans[0] == 'A' || ans[0] == 'R') && replymode) {
            include_mode = ans[0];
        } else if (ans[0] == 'T') {
            buf4[0] = '\0';
        } else if (ans[0] == 'L') {
            currentuser->signature = -1;
        } else if (ans[0] == 'V') {     /* Leeward 98.09.24 add: viewing signature(s) while setting post head */
            sethomefile(buf2, currentuser->userid, "signatures");
            move(t_lines - 1, 0);
            if (askyn("Ô¤ÉèÏÔÊ¾Ç°Èı¸öÇ©Ãûµµ, ÒªÏÔÊ¾È«²¿Âğ", false) == true)
                ansimore(buf2, 0);
            else {
                clear();
                ansimore2(buf2, false, 0, 18);
            }
        } 
        else if (ans[0] == 'U'&&HAS_PERM(currentuser, PERM_SYSOP)) {
            int i;
            chdir("tmp");
            upload = bbs_zrecvfile();
            chdir("..");
         }
        else {
            strncpy(newmessage.title, title, STRLEN);
            newmessage.title[STRLEN-1] = 0;
            strncpy(save_title, newmessage.title, STRLEN);
            save_title[STRLEN-1] = 0;
            break;
        }
    }

    do_quote(filepath, include_mode, q_file, quote_user);
    strncpy(quote_title, newmessage.title, STRLEN);
    quote_title[STRLEN-1] = 0;

#ifdef INTERNET_PRIVATE_EMAIL
    if (internet_mail) {
        int res, ch;

        if (vedit(filepath, false, NULL, NULL) == -1) {
            unlink(filepath);
            clear();
            return -2;
        }
        clear();
      redo:
        prints("ĞÅ¼ş¼´½«¼Ä¸ø %s \n", userid);
        prints("±êÌâÎª£º %s \n", title);
        prints("È·¶¨Òª¼Ä³öÂğ? (Y/N) [Y]");
        ch = igetkey();
        switch (ch) {
        case KEY_REFRESH:
            move(3, 0);
            goto redo;
        case 'N':
        case 'n':
            prints("%c\n", 'N');
            prints("\nĞÅ¼şÒÑÈ¡Ïû...\n");
            res = -2;
            break;
        default:
            {
                /*
                 * uuencode or convert to big5 option -- Add by ming, 96.10.9 
                 */
                char data[3];
                int isuu, isbig5;

                prints("%c\n", 'Y');
                if (askyn("ÊÇ·ñ±¸·İ¸ø×Ô¼º", false) == true)
                    mail_file(currentuser->userid, tmp_fname, currentuser->userid, save_title, 0, NULL);

                prints("ÈôÄúÒª×ª¼ÄµÄµØÖ·ÎŞ·¨´¦ÀíÖĞÎÄÇëÊäÈë Y »ò y\n");
                getdata(5, 0, "Uuencode? [N]: ", data, 2, DOECHO, 0, 0);
                if (data[0] == 'y' || data[0] == 'Y')
                    isuu = 1;
                else
                    isuu = 0;

                prints("ÈôÄúÒª½«ĞÅ¼ş×ª¼Äµ½Ì¨ÍåÇëÊäÈë Y »ò y\n");
                getdata(7, 0, "×ª³ÉBIG5Âë? [N]: ", data, 2, DOECHO, 0, 0);
                if (data[0] == 'y' || data[0] == 'Y')
                    isbig5 = 1;
                else
                    isbig5 = 0;

                getdata(8, 0, "¹ıÂËANSI¿ØÖÆ·û„1¤7? [N]: ", data, 2, DOECHO, 0, 0);
                if (data[0] == 'y' || data[0] == 'Y')
                    noansi = 1;
                else
                    noansi = 0;

                prints("ÇëÉÔºò, ĞÅ¼ş´«µİÖĞ...\n");
                /*
                 * res = bbs_sendmail( tmp_fname, title, userid );  
                 */
                res = bbs_sendmail(tmp_fname, title, userid, isuu, isbig5, noansi);

                newbbslog(BBSLOG_USER, "mailed %s %s", userid, title);
                break;
            }
        }
        unlink(tmp_fname);
        return res;
    } else
#endif
    {
        if (vedit(filepath, true, NULL, NULL) == -1) {
            unlink(filepath);
            clear();
            return -2;
        }

    if(upload) {
        char sbuf[PATHLEN];
        strcpy(sbuf,"tmp/");
        strcpy(sbuf+strlen(sbuf), upload);
        newmessage.attachment = add_attach(filepath, sbuf, upload);
    }
    
        clear();
        /*
         * if(!chkreceiver(userid))
         * {
         * prints("%s ĞÅÏäÒÑÂú,ÎŞ·¨ÊÕĞÅ",userid);
         * return -4;
         * }
         */

        if (false == canIsend2(currentuser, userid)) {  /* Leeward 98.04.10 */
            prints("[1m[33mºÜ±§Ç¸¡ÃÏµÍ³ÎŞ·¨·¢³ö´ËĞÅ£®ÒòÎª %s ¾Ü¾ø½ÓÊÕÄúµÄĞÅ¼ş£®[m[m\n\n", userid);
            sprintf(save_title, "ÍËĞÅ¡Ã %s ¾Ü¾ø½ÓÊÕÄúµÄĞÅ¼ş£®", userid);
            mail_file(currentuser->userid, filepath, currentuser->userid, save_title, BBSPOST_MOVE, NULL);
            return -2;
        }
        /*
         * ¼ÓÉÏ±£´æµ½·¢¼şÏäµÄÈ·ÈÏ£¬by flyriver, 2002.9.23 
         */
        /*
         * Disabled by flyriver, 2003.1.5
         * Using the newly introduced mailbox properties.
         buf2[0] = '\0';
         getdata(1, 0, "±£´æĞÅ¼şµ½·¢¼şÏä? [N]: ", buf2, 2, DOECHO, 0, 0);
         if (buf2[0] == 'y' || buf2[0] == 'Y')
         */
        if (HAS_MAILBOX_PROP(&uinfo, MBP_SAVESENTMAIL)) {
            /*
             * backup mail to sent folder 
             */
            mail_file_sent(userid, filepath, currentuser->userid, save_title, 0);
        }
        if (askyn("È·¶¨¼Ä³ö£¿", true) == false)
            return -2;

        setmailfile(genbuf, userid, DOT_DIR);
        if (append_record(genbuf, &newmessage, sizeof(newmessage)) == -1)
            return -1;

        if (stat(filepath, &st) != -1) {
            user->usedspace += st.st_size;
            /*
             * Removed by flyriver, 2002.9.23 
             */
            /*
             * currentuser->usedspace += st.st_size;
             *//*
             * ÕâÀï¶à¼ÆËãÁËÒ»´Î 
             */
        }

        newbbslog(BBSLOG_USER, "mailed %s %s", userid, save_title);
        if (!strcasecmp(userid, "SYSOP"))
            updatelastpost(SYSMAIL_BOARD);
        return 0;
    }
}

int m_send(char *userid)
{
    char uident[STRLEN];
    int oldmode;

    /*
     * ·â½ûMail Bigman:2000.8.22 
     */
    if (HAS_PERM(currentuser, PERM_DENYMAIL))
        return DONOTHING;

    oldmode = uinfo.mode;
    modify_user_mode(SMAIL);
    if (userid == NULL) {
        move(1, 0);
        clrtoeol();
        usercomplete("ÊÕĞÅÈË£º ", uident);
        if (uident[0] == '\0') {
            clear();
            modify_user_mode(oldmode);
            return 0;
        }
    } else
        strcpy(uident, userid);
    clear();
    switch (do_send(uident, NULL, "")) {
    case -1:
        prints("ÊÕĞÅÕß²»ÕıÈ·\n");
        break;
    case -2:
        prints("È¡Ïû·¢ĞÅ\n");
        break;
    case -3:
        prints("'%s' ÎŞ·¨ÊÕĞÅ\n", uident);
        break;
    case -4:
        clear();
        move(1, 0);
        prints("%s ĞÅÏäÒÑÂú,ÎŞ·¨ÊÕĞÅ\n", uident);
        break;                  /*Haohmaru.4.5.ÊÕĞÅÏŞÖÆ */
    case -5:
        clear();
        move(1, 0);
        prints("%s ×ÔÉ±ÖĞ£¬²»ÄÜÊÕĞÅ\n", uident);
        break;                  /*Haohmaru.99.10.26.×ÔÉ±Õß²»ÄÜÊÕĞÅ */
    case -552:
        prints("\n[1m[33mĞÅ¼ş³¬³¤£¨±¾Õ¾ÏŞ¶¨ĞÅ¼ş³¤¶ÈÉÏÏŞÎª %d ×Ö½Ú£©£¬È¡Ïû·¢ĞÅ²Ù×÷[m[m\n", MAXMAILSIZE);
        break;
    default:
        prints("ĞÅ¼şÒÑ¼Ä³ö\n");
    }
    pressreturn();
    modify_user_mode(oldmode);
    return 0;
}

int read_mail(struct fileheader *fptr)
{
    setmailfile(genbuf, currentuser->userid, fptr->filename);
    ansimore_withzmodem(genbuf, false, fptr->title);
    fptr->accessed[0] |= FILE_READ;
    return 0;
}

int del_mail(int ent, struct fileheader *fh, char *direct)
{
    char buf[PATHLEN];
    char *t;
    struct stat st;

    if (strstr(direct, ".DELETED")
        || HAS_MAILBOX_PROP(&uinfo, MBP_FORCEDELETEMAIL)) {
        strcpy(buf, direct);
        t = strrchr(buf, '/') + 1;
        strcpy(t, fh->filename);
		if (lstat(buf, &st) == 0 && S_ISREG(st.st_mode) && st.st_nlink == 1)
            currentuser->usedspace -= st.st_size;
    }

    strcpy(buf, direct);
    if ((t = strrchr(buf, '/')) != NULL)
        *t = '\0';
    if (!delete_record(direct, sizeof(*fh), ent, (RECORD_FUNC_ARG) cmpname, fh->filename)) {
        sprintf(genbuf, "%s/%s", buf, fh->filename);
        if (strstr(direct, ".DELETED")
            || HAS_MAILBOX_PROP(&uinfo, MBP_FORCEDELETEMAIL))
            unlink(genbuf);
        else {
            strcpy(buf, direct);
            t = strrchr(buf, '/') + 1;
            strcpy(t, ".DELETED");
            append_record(buf, fh, sizeof(*fh));
        }
        return 0;
    }
    return 1;
}

int mrd;
int delete_new_mail(struct fileheader *fptr, int idc, void *arg)
{
    if (fptr->accessed[1] & FILE_DEL) {
        del_mail(idc, fptr, currdirect);
        return 1;
    }
    return 0;
}

int read_new_mail(struct fileheader *fptr, int idc, void *arg)
{
    char done = false, delete_it;
    char fname[256];

    if (fptr->accessed[0])
        return 0;
    prints("¶ÁÈ¡ %s ¼ÄÀ´µÄ '%s' ?\n", fptr->owner, fptr->title);
    prints("(Yes, or No): ");
    getdata(1, 0, "(Y)¶ÁÈ¡ (N)²»¶Á (Q)Àë¿ª [Y]: ", genbuf, 3, DOECHO, NULL, true);
    if (genbuf[0] == 'q' || genbuf[0] == 'Q') {
        clear();
        return QUIT;
    }
    if (genbuf[0] != 'y' && genbuf[0] != 'Y' && genbuf[0] != '\0') {
        clear();
        return 0;
    }
    read_mail(fptr);
    strcpy(fname, genbuf);
    mrd = 1;
    delete_it = false;
    while (!done) {
        move(t_lines - 1, 0);
        prints("(R)»ØĞÅ, (D)É¾³ı, (G)¼ÌĞø ? [G]: ");
        switch (igetkey()) {
        case Ctrl('Y'):
            zsend_post(idc, fptr, currdirect);
            break;
        case 'R':
        case 'r':

            /*
             * ·â½ûMail Bigman:2000.8.22 
             */
            if (HAS_PERM(currentuser, PERM_DENYMAIL)) {
                clear();
                move(3, 10);
                prints("ºÜ±§Ç¸,ÄúÄ¿Ç°Ã»ÓĞMailÈ¨ÏŞ!");
                pressreturn();
                break;
            }
            mail_reply(idc, fptr, currdirect);
            /*
             * substitute_record(currmaildir, fptr, sizeof(*fptr), dc);
             */
            break;
        case 'D':
        case 'd':
            delete_it = true;
        default:
            done = true;
        }
        if (!done)
            ansimore(fname, false);     /* re-read */
    }
    if (delete_it) {
        clear();
        prints("Delete Message '%s' ", fptr->title);
        getdata(1, 0, "(Yes, or No) [N]: ", genbuf, 3, DOECHO, NULL, true);
        if (genbuf[0] == 'Y' || genbuf[0] == 'y') {     /* if not yes quit */
            fptr->accessed[1] |= FILE_DEL;
        }
    }
    if (substitute_record(currdirect, fptr, sizeof(*fptr), idc))
        return -1;
    clear();
    return 0;
}

int m_new()
{
    clear();
    mrd = 0;
    modify_user_mode(RMAIL);
    setmailfile(currdirect, currentuser->userid, ".DIR");
    if (apply_record(currdirect, (APPLY_FUNC_ARG) read_new_mail, sizeof(struct fileheader), NULL, 1, false) == -1) {
        clear();
        move(0, 0);
        prints("No new messages\n\n\n");
        return -1;
    }
    apply_record(currdirect, (APPLY_FUNC_ARG) delete_new_mail, sizeof(struct fileheader), NULL, 1, true);
/*    	
    if (delcnt) {
        while (delcnt--)
            delete_record(currmaildir, sizeof(struct fileheader), delmsgs[delcnt], NULL, NULL);
    }
*/
    clear();
    move(0, 0);
    if (mrd)
        prints("No more messages.\n\n\n");
    else
        prints("No new messages.\n\n\n");
    return -1;
}

void mailtitle()
{
    /*
     * Leeward 98.01.19 adds below codes for statistics 
     */
    int MailSpace, numlimit;
    int UsedSpace = get_mailusedspace(currentuser, 0) / 1024;

    get_mail_limit(currentuser, &MailSpace, &numlimit);
    showtitle("ÓÊ¼şÑ¡µ¥    ", BBS_FULL_NAME);
    update_endline();
    move(1, 0);
    prints("Àë¿ª[¡û,e]  Ñ¡Ôñ[¡ü,¡ı]  ÔÄ¶ÁĞÅ¼ş[¡ú,r]  »ØĞÅ[R]  ¿³ĞÅ£¯Çå³ı¾ÉĞÅ[d,D]  ÇóÖú[h][m\n");
    /*
     * prints("[44m±àºÅ    %-20s %-49s[m\n","·¢ĞÅÕß","±ê  Ìâ") ; 
     */
    if (0 != get_mailnum() && 0 == UsedSpace)
        UsedSpace = 1;
    else if (UsedSpace < 0)
        UsedSpace = 0;
    prints("[44m±àºÅ    %-12s %6s  %-13sÄúµÄĞÅÏäÉÏÏŞÈİÁ¿%4dK£¬µ±Ç°ÒÑÓÃ%4dK ", (strstr(currdirect, ".SENT")) ? "ÊÕĞÅÕß" : "·¢ĞÅÕß", "ÈÕ  ÆÚ", "±ê  Ìâ", MailSpace, UsedSpace);    /* modified by dong , 1998.9.19 */
    clrtoeol();
    prints("\n");
    resetcolor();
}

char *maildoent(char *buf, int num, struct fileheader *ent)
{
    time_t filetime;
    char *date;
    char b2[512];
    char status, reply_status;
    char *t;
    extern char ReadPost[];
    extern char ReplyPost[];
    char c1[8];
    char c2[8];
    int same = false;

    filetime = get_posttime(ent);       /* ÓÉÎÄ¼şÃûÈ¡µÃÊ±¼ä */
    if (filetime > 740000000)
        date = ctime(&filetime) + 4;    /* Ê±¼ä -> Ó¢ÎÄ */
    else
        /*
         * date = ""; char *ÀàĞÍ±äÁ¿, ¿ÉÄÜ´íÎó, modified by dong, 1998.9.19 
         */
    {
        date = ctime(&filetime) + 4;
        date = "";
    }

    if (DEFINE(currentuser, DEF_HIGHCOLOR)) {
        strcpy(c1, "[1;33m");
        strcpy(c2, "[1;36m");
    } else {
        strcpy(c1, "[33m");
        strcpy(c2, "[36m");
    }
    if (!strncmp(ReadPost, ent->title, STRLEN) || !strncmp(ReplyPost, ent->title, STRLEN))
        same = true;
    strncpy(b2, ent->owner, OWNER_LEN);
    ent->owner[OWNER_LEN - 1] = 0;
    if ((t = strchr(b2, ' ')) != NULL)
        *t = '\0';
    if (ent->accessed[0] & FILE_READ) {
        if (ent->accessed[0] & FILE_MARKED)
            status = 'm';
        else
            status = ' ';
    } else {
        if (ent->accessed[0] & FILE_MARKED)
            status = 'M';
        else
            status = 'N';
    }
    if (ent->accessed[0] & FILE_REPLIED) {
        if (ent->accessed[0] & FILE_FORWARDED)
            reply_status = 'A';
        else
            reply_status = 'R';
    } else {
        if (ent->accessed[0] & FILE_FORWARDED)
            reply_status = 'F';
        else
            reply_status = ' ';
    }
    /*
     * if (ent->accessed[0] & FILE_REPLIED)
     * reply_status = 'R';
     * else
     * reply_status = ' '; 
     *//*
     * * * * * added by alex, 96.9.7 
     */
    if (!strncmp("Re:", ent->title, 3)) {
        sprintf(buf, " %s%3d[m %c%c %-12.12s %6.6s  %s%.50s[m", same ? c1 : "", num, reply_status, status, b2, date, same ? c1 : "", ent->title);
    } /* modified by dong, 1998.9.19 */
    else {
        sprintf(buf, " %s%3d[m %c%c %-12.12s %6.6s  ¡ï %s%.49s[m", same ? c2 : "", num, reply_status, status, b2, date, same ? c2 : "", ent->title);
    }                           /* modified by dong, 1998.9.19 */
    return buf;
}

#ifdef POSTBUG
extern int bug_possible;
#endif

int mail_read(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
    char buf[512], notgenbuf[128];
    char *t;
    int readnext,readprev;
    char done = false, delete_it, replied;

    clear();
    readnext = false;
    readprev = false;
    setqtitle(fileinfo->title);
    strcpy(buf, direct);
    if ((t = strrchr(buf, '/')) != NULL)
        *t = '\0';
    sprintf(notgenbuf, "%s/%s", buf, fileinfo->filename);
    delete_it = replied = false;
    while (!done) {
        ansimore(notgenbuf, false);
        move(t_lines - 1, 0);
        prints("(R)»ØĞÅ, (D)É¾³ı, (G)¼ÌĞø? [G]: ");
        switch (igetkey()) {
        case Ctrl('Y'):
            zsend_post(ent, fileinfo, direct);
	    break;
        case 'R':
        case 'r':

            /*
             * ·â½ûMail Bigman:2000.8.22 
             */
            if (HAS_PERM(currentuser, PERM_DENYMAIL)) {
                clear();
                move(3, 10);
                prints("ºÜ±§Ç¸,ÄúÄ¿Ç°Ã»ÓĞMailÈ¨ÏŞ!");
                pressreturn();
                break;
            }
            replied = true;
            mail_reply(ent, fileinfo, direct);
            break;
        case ' ':
        case 'j':
        case KEY_RIGHT:
        case KEY_DOWN:
        case KEY_PGDN:
            done = true;
            readnext = true;
            break;
        /* read prev mail  .  binxun 2003.5*/
        case KEY_UP:
        	done = true;
        	readprev = true;
        	break;
        	
        case Ctrl('D'):
            zsend_attach(ent, fileinfo, direct);
            done=true;
            break;
        case 'D':
        case 'd':
            delete_it = true;
        default:
            done = true;
            
        }
    }
    if (delete_it)
        return mail_del(ent, fileinfo, direct);
    else if ((fileinfo->accessed[0] & FILE_READ) != FILE_READ)
	{
        fileinfo->accessed[0] |= FILE_READ;
        substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
	}
    if (readnext == true)
        return READ_NEXT;

    /* read prev mail  .  binxun 2003.5*/
    if(readprev == true)
	return READ_PREV;
    
    return FULLUPDATE;
}

 /*ARGSUSED*/ static int mail_reply(int ent, struct fileheader *fileinfo, char *direct)
{
    char uid[STRLEN];
    char title[STRLEN];
    char q_file[STRLEN];
    char *t;

    clear();
    modify_user_mode(SMAIL);
    strncpy(uid, fileinfo->owner, OWNER_LEN);
    uid[OWNER_LEN - 1] = 0;
    if ((t = strchr(uid, ' ')) != NULL)
        *t = '\0';
    if (toupper(fileinfo->title[0]) != 'R' || fileinfo->title[1] != 'e' || fileinfo->title[2] != ':')
        strcpy(title, "Re: ");
    else
        title[0] = '\0';
    strncat(title, fileinfo->title, STRLEN - 5);

    setmailfile(q_file, currentuser->userid, fileinfo->filename);
    strncpy(quote_user, fileinfo->owner, IDLEN);
    quote_user[IDLEN] = 0;
    switch (do_send(uid, title, q_file)) {
    case -1:
        prints("ÎŞ·¨Í¶µİ\n");
        break;
    case -2:
        prints("È¡Ïû»ØĞÅ\n");
        break;
    case -3:
        prints("'%s' ÎŞ·¨ÊÕĞÅ\n", uid);
        break;
    case -4:
        clear();
        move(1, 0);
        prints("%s ĞÅÏäÒÑÂú,ÎŞ·¨ÊÕĞÅ\n", uid);
        break;                  /*Haohmaru.4.5.ÊÕĞÅÏŞÖÆ */
    case -5:
        clear();
        move(1, 0);
        prints("%s ×ÔÉ±ÖĞ£¬²»ÄÜÊÕĞÅ\n", uid);
        break;                  /*Haohmaru.99.10.26.×ÔÉ±Õß²»ÄÜÊÕĞÅ */
    default:
        prints("ĞÅ¼şÒÑ¼Ä³ö\n");
        fileinfo->accessed[0] |= FILE_REPLIED;  /*added by alex, 96.9.7 */
        substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
    }
    pressreturn();
    return FULLUPDATE;
}

static int mail_del(int ent, struct fileheader *fileinfo, char *direct)
{
    clear();
    prints("É¾³ı´ËĞÅ¼ş '%s' ", fileinfo->title);
    getdata(1, 0, "(Yes, or No) [N]: ", genbuf, 2, DOECHO, NULL, true);
    if (genbuf[0] != 'Y' && genbuf[0] != 'y') { /* if not yes quit */
        move(2, 0);
        prints("È¡ÏûÉ¾³ı\n");
        pressreturn();
        clear();
        return FULLUPDATE;
    }

    if (del_mail(ent, fileinfo, direct) == 0) {
        return DIRCHANGED;
    }
    move(2, 0);
    prints("É¾³ıÊ§°Ü\n");
    pressreturn();
    clear();
    return FULLUPDATE;
}

/*added by bad 03-2-10*/
static int mail_edit(int ent, struct fileheader *fileinfo, char *direct)
{
    char buf[512];
    char *t;
    long eff_size;
    long attachpos;
    struct stat st;
    int before = 0;

    strcpy(buf, direct);
    if ((t = strrchr(buf, '/')) != NULL)
        *t = '\0';

    sprintf(genbuf, "%s/%s", buf, fileinfo->filename);
    if(stat(genbuf,&st) != -1)
	{
		mode_t rwmode = S_IRUSR | S_IWUSR;
		if ((st.st_mode & rwmode) != rwmode)
			return DONOTHING;
		before = st.st_size;
	}
	else
		return DONOTHING;

    clear();
	attachpos = fileinfo->attachment;
    if (vedit_post(genbuf, false, &eff_size,&attachpos) != -1) {
        fileinfo->eff_size = eff_size;
        if (ADD_EDITMARK)
            add_edit_mark(genbuf, 1, /*NULL*/ fileinfo->title);
        if (attachpos!=fileinfo->attachment) {
            fileinfo->attachment=attachpos;
            substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
        }
    }
    if(stat(genbuf,&st) != -1) currentuser->usedspace -= (before - st.st_size);

    newbbslog(BBSLOG_USER, "edited mail '%s' ", fileinfo->title);
    return FULLUPDATE;
}

static int mail_edit_title(int ent, struct fileheader *fileinfo, char *direct)
{
	char buf[STRLEN];
	char tmp[STRLEN*2];
	char genbuf[1024];
	char * t = NULL;
	unsigned int i;

	strcpy(buf,fileinfo->title);
	getdata(t_lines-1,0,"ĞÂĞÅ¼ş±êÌâ:",buf,50,DOECHO,NULL,false);

	if(strcmp(buf,fileinfo->title))
	{
		for(i = 0; (i < strlen(buf)) && (i < STRLEN -1); i++)  /* disable color title */
			if(buf[i] == 0x1b)
				fileinfo->title[i]=' ';
			else
				fileinfo->title[i] = buf[i];
		fileinfo->title[i] = 0;

		strcpy(tmp,direct);
		if((t = strrchr(tmp,'/')) != NULL)*t='\0';
		sprintf(genbuf,"%s/%s",tmp,fileinfo->filename);
		add_edit_mark(genbuf,3,buf); /* 3 means edit mail and title */
		substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
	    newbbslog(BBSLOG_USER, "edited mail '%s' ", fileinfo->title);
	}
	return FULLUPDATE;
}

/** Added by netty to handle mail to 0Announce */
int mail_to_tmp(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
    char buf[STRLEN];
    char *p;
    char fname[STRLEN];
    char board[STRLEN];
    char ans[STRLEN];

    if (!HAS_PERM(currentuser, PERM_BOARDS)) {
        return DONOTHING;
    }
    strncpy(buf, direct, sizeof(buf));
    if ((p = strrchr(buf, '/')) != NULL)
        *p = '\0';
    clear();
    sprintf(fname, "%s/%s", buf, fileinfo->filename);
    sprintf(genbuf, "½«--%s--´æÈëÔİ´æµµ,È·¶¨Âğ?(Y/N) [N]: ", fileinfo->title);
    a_prompt(-1, genbuf, ans);
    if (ans[0] == 'Y' || ans[0] == 'y') {
        sprintf(board, "tmp/bm.%s", currentuser->userid);
        if (dashf(board)) {
            sprintf(genbuf, "Òª¸½¼ÓÔÚ¾ÉÔİ´æµµÖ®ºóÂğ?(Y/N) [N]: ");
            a_prompt(-1, genbuf, ans);
            if (ans[0] == 'Y' || ans[0] == 'y') {
                sprintf(genbuf, "/bin/cat %s >> tmp/bm.%s", fname, currentuser->userid);
                system(genbuf);
            } else {
                /*
                 * sprintf( genbuf, "/bin/cp -r %s  tmp/bm.%s", fname , currentuser->userid );
                 */
                sprintf(genbuf, "tmp/bm.%s", currentuser->userid);
                f_cp(fname, genbuf, 0);
            }
        } else {
            sprintf(genbuf, "tmp/bm.%s", currentuser->userid);
            f_cp(fname, genbuf, 0);
        }
        sprintf(genbuf, " ÒÑ½«¸ÃÎÄÕÂ´æÈëÔİ´æµµ, Çë°´ÈÎºÎ¼üÒÔ¼ÌĞø << ");
        a_prompt(-1, genbuf, ans);
    }
    clear();
    return FULLUPDATE;
}


#ifdef INTERNET_EMAIL
int mail_forward_internal(int ent, struct fileheader *fileinfo, char *direct, int isuu)
{
    char buf[STRLEN];
    char *p;

    if (strcmp("guest", currentuser->userid) == 0) {
        clear();
        move(3, 10);
        prints("ºÜ±§Ç¸,Ïë×ª¼ÄÎÄÕÂÇëÉêÇëÕıÊ½ID!");
        pressreturn();
        return FULLUPDATE;
    }

    /*
     * ·â½ûMail Bigman:2000.8.22
     */
    if (HAS_PERM(currentuser, PERM_DENYMAIL)) {
        clear();
        move(3, 10);
        prints("ºÜ±§Ç¸,ÄúÄ¿Ç°Ã»ÓĞMailÈ¨ÏŞ!");
        pressreturn();
        return FULLUPDATE;
    }

    if (!HAS_PERM(currentuser, PERM_FORWARD) || !HAS_PERM(currentuser,PERM_LOGINOK)) {
        return DONOTHING;
    }
    strncpy(buf, direct, sizeof(buf));
    if ((p = strrchr(buf, '/')) != NULL)
        *p = '\0';
    clear();
    switch (doforward(buf, fileinfo, isuu)) {
    case 0:
        prints("ÎÄÕÂ×ª¼ÄÍê³É!\n");
        fileinfo->accessed[0] |= FILE_FORWARDED;        /*added by alex, 96.9.7 */
        break;
    case -1:
        prints("Forward failed: system error.\n");
        break;
    case -2:
        prints("Forward failed: missing or invalid address.\n");
        break;
    case -552:
        prints
            ("\n[1m[33mĞÅ¼ş³¬³¤£¨±¾Õ¾ÏŞ¶¨ĞÅ¼ş³¤¶ÈÉÏÏŞÎª %d ×Ö½Ú£©£¬È¡Ïû×ª¼Ä²Ù×÷[m[m\n\nÇë¸æÖªÊÕĞÅÈË£¨Ò²Ğí¾ÍÊÇÄú×Ô¼º°É:PP£©£º\n\n*1* Ê¹ÓÃ [1m[33mWWW[m[m ·½Ê½·ÃÎÊ±¾Õ¾£¬ËæÊ±¿ÉÒÔ±£´æÈÎÒâ³¤¶ÈµÄÎÄÕÂµ½×Ô¼ºµÄ¼ÆËã»ú£»\n*2* Ê¹ÓÃ [1m[33mpop3[m[m ·½Ê½´Ó±¾Õ¾ÓÃ»§µÄĞÅÏäÈ¡ĞÅ£¬Ã»ÓĞÈÎºÎ³¤¶ÈÏŞÖÆ¡£\n*3* Èç¹û²»ÊìÏ¤±¾Õ¾µÄ WWW »ò pop3 ·şÎñ£¬ÇëÔÄ¶Á [1m[33mAnnounce[m[m °æÓĞ¹Ø¹«¸æ¡£\n",
             MAXMAILSIZE);
        break;
    default:
        prints("È¡Ïû×ª¼Ä...\n");
    }
    pressreturn();
    clear();
    return FULLUPDATE;
}

int mail_uforward(int ent, struct fileheader *fileinfo, char *direct)
{
    return mail_forward_internal(ent, fileinfo, direct, 1);
}

int mail_forward(int ent, struct fileheader *fileinfo, char *direct)
{
    return mail_forward_internal(ent, fileinfo, direct, 0);
}

#endif

int mail_del_range(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
    int ret;

    ret = (del_range(ent, fileinfo, direct, 0));        /*Haohmaru.99.5.14.ĞŞ¸ÄÒ»¸öbug,
                                                         * * ·ñÔò¿ÉÄÜ»áÒòÎªÉ¾ĞÅ¼şµÄ.tmpfile¶ø´íÉ¾°æÃæµÄ.tmpfile */
    if (!strcmp(direct, ".DELETED"))
        get_mailusedspace(currentuser, 1);
    return ret;
}

int mail_mark(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
    if (fileinfo->accessed[0] & FILE_MARKED)
        fileinfo->accessed[0] &= ~FILE_MARKED;
    else
        fileinfo->accessed[0] |= FILE_MARKED;
    substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
    return (PARTUPDATE);
}


int mail_move(int ent, struct fileheader *fileinfo, char *direct)
{
    struct _select_item *sel;
    int i, j;
    char buf[PATHLEN];
    char *t;
    char menu_char[3][10] = { "I) ÊÕ¼şÏä", "J) À¬»øÏä", "Q) ÍË³ö" };

    clear();
    move(5, 3);
    prints("ÇëÑ¡ÔñÒÆ¶¯µ½ÄÄ¸öÓÊÏä");
    sel = (struct _select_item *) malloc(sizeof(struct _select_item) * (user_mail_list.mail_list_t + 4));
    sel[0].x = 3;
    sel[0].y = 6;
    sel[0].hotkey = 'I';
    sel[0].type = SIT_SELECT;
    sel[0].data = menu_char[0];
    sel[1].x = 3;
    sel[1].y = 7;
    sel[1].hotkey = 'J';
    sel[1].type = SIT_SELECT;
    sel[1].data = menu_char[1];
    for (i = 0; i < user_mail_list.mail_list_t; i++) {
        sel[i + 2].x = 3;
        sel[i + 2].y = i + 8;
        sel[i + 2].hotkey = user_mail_list.mail_list[i][0];
        sel[i + 2].type = SIT_SELECT;
        sel[i + 2].data = (void *) user_mail_list.mail_list[i];
    }
    sel[user_mail_list.mail_list_t + 2].x = 3;
    sel[user_mail_list.mail_list_t + 2].y = user_mail_list.mail_list_t + 8;
    sel[user_mail_list.mail_list_t + 2].hotkey = 'Q';
    sel[user_mail_list.mail_list_t + 2].type = SIT_SELECT;
    sel[user_mail_list.mail_list_t + 2].data = menu_char[2];
    sel[user_mail_list.mail_list_t + 3].x = -1;
    sel[user_mail_list.mail_list_t + 3].y = -1;
    sel[user_mail_list.mail_list_t + 3].hotkey = -1;
    sel[user_mail_list.mail_list_t + 3].type = 0;
    sel[user_mail_list.mail_list_t + 3].data = NULL;
    i = simple_select_loop(sel, SIF_NUMBERKEY | SIF_SINGLE | SIF_ESCQUIT, 0, 6, NULL) - 1;
    if (i >= 0 && i < user_mail_list.mail_list_t + 2) {
        strcpy(buf, direct);
        t = strrchr(buf, '/') + 1;
        *t = '.';
        t++;
        if (i >= 2)
            strcpy(t, user_mail_list.mail_list[i - 2] + 30);
        else if (i == 0)
            strcpy(t, "DIR");
        else if (i == 1)
            strcpy(t, "DELETED");
        if (strcmp(buf, direct))
            if (!delete_record(direct, sizeof(*fileinfo), ent, (RECORD_FUNC_ARG) cmpname, fileinfo->filename)) {
                append_record(buf, fileinfo, sizeof(*fileinfo));
            }
    }
    free(sel);
    return (DIRCHANGED);
}

extern int mailreadhelp();

struct one_key mail_comms[] = {
    {'d', mail_del},
    {'D', mail_del_range},
//added by bad 03-2-10
    {'E', mail_edit},
	{'T', mail_edit_title},
    {'r', mail_read},
    {'R', mail_reply},
    {'m', mail_mark},
    {'M', mail_move},
    {'i', mail_to_tmp},
#ifdef INTERNET_EMAIL
    {'F', mail_forward},
    {'U', mail_uforward},
#endif
    /*
     * Added by ming, 96.10.9
     */
    {'a', auth_search_down},
    {'A', auth_search_up},
    {'/', t_search_down},
    {'?', t_search_up},
    {']', thread_down},
    {'[', thread_up},
    {Ctrl('A'), show_author},
    {Ctrl('Q'), show_authorinfo},       /*Haohmaru.98.12.19 */
    {Ctrl('W'), show_authorBM}, /*cityhunter 00.10.18 */
    {Ctrl('N'), SR_first_new},
    {'\\', SR_last},
    {'=', SR_first},
    {Ctrl('C'), do_cross},
/*    {Ctrl('S'), SR_read},
    {'n', SR_first_new},
    {'p', SR_read},
    {Ctrl('X'), SR_readX},   */   /* Leeward 98.10.03 */
/*    {Ctrl('U'), SR_author},
    {Ctrl('H'), SR_authorX}, */   /* Leeward 98.10.03 */
    {'h', mailreadhelp},
    {Ctrl('J'), mailreadhelp},
    {Ctrl('O'), add_author_friend},
    {Ctrl('Y'), zsend_post},    /* COMMAN 2002 */    
    {'\0', NULL},
};

int m_read()
{
    char curmaildir[STRLEN];

    setmailfile(curmaildir, currentuser->userid, DOT_DIR);
    in_mail = true;
    i_read(RMAIL, curmaildir, mailtitle, (READ_FUNC) maildoent, &mail_comms[0], sizeof(struct fileheader));
    in_mail = false;
    return FULLUPDATE /* 0 */ ;
}

#ifdef INTERNET_EMAIL

#include <netdb.h>
#include <pwd.h>
#include <time.h>
#define BBSMAILDIR "/usr/spool/mqueue"
int invalidaddr(char *addr)
{
    if (*addr == '\0')
        return 1;               /* blank */
    while (*addr) {
        if (!isalnum(*addr) && strchr("[].%!@:-_", *addr) == NULL)
            return 1;
        addr++;
    }
    return 0;
}

void spacestozeros(s)
char *s;
{
    while (*s) {
        if (*s == ' ')
            *s = '0';
        s++;
    }
}

int getqsuffix(s)
char *s;
{
    struct stat stbuf;
    char qbuf[STRLEN], dbuf[STRLEN];
    char c1 = 'A', c2 = 'A';
    int pos = strlen(BBSMAILDIR) + 3;

    sprintf(dbuf, "%s/dfAA%5d", BBSMAILDIR, getpid());
    sprintf(qbuf, "%s/qfAA%5d", BBSMAILDIR, getpid());
    spacestozeros(dbuf);
    spacestozeros(qbuf);
    while (1) {
        if (stat(dbuf, &stbuf) && stat(qbuf, &stbuf))
            break;
        if (c2 == 'Z') {
            c2 = 'A';
            if (c1 == 'Z')
                return -1;
            else
                c1++;
            dbuf[pos] = c1;
            qbuf[pos] = c1;
        } else
            c2++;
        dbuf[pos + 1] = c2;
        qbuf[pos + 1] = c2;
    }
    strcpy(s, &(qbuf[pos]));
    return 0;
}

int g_send()
{
    char uident[13], tmp[3];
    int cnt, i, n, fmode = false;
    char maillists[STRLEN];
    struct userec *lookupuser;
    struct user_info *u;

    /*
     * ·â½ûMail Bigman:2000.8.22 
     */
    if (HAS_PERM(currentuser, PERM_DENYMAIL))
        return DONOTHING;

    modify_user_mode(SMAIL);
    clear();
    sethomefile(maillists, currentuser->userid, "maillist");
    cnt = listfilecontent(maillists);
    while (1) {
        if (cnt > maxrecp - 10) {
            move(2, 0);
            prints("Ä¿Ç°ÏŞÖÆ¼ÄĞÅ¸ø [1m%d[m ÈË", maxrecp);
        }
        getdata(0, 0, "(A)Ôö¼Ó (D)É¾³ı (I)ÒıÈëºÃÓÑ (C)Çå³ıÄ¿Ç°Ãûµ¥ (E)·ÅÆú (S)¼Ä³ö? [S]£º ", tmp, 2, DOECHO, NULL, true);
        if (tmp[0] == '\n' || tmp[0] == '\0' || tmp[0] == 's' || tmp[0] == 'S') {
            break;
        }
        if (tmp[0] == 'a' || tmp[0] == 'd' || tmp[0] == 'A' || tmp[0] == 'D') {
            move(1, 0);
            if (tmp[0] == 'a' || tmp[0] == 'A')
                usercomplete("ÇëÒÀ´ÎÊäÈëÊ¹ÓÃÕß´úºÅ(Ö»°´ ENTER ½áÊøÊäÈë): ", uident);
            else
                namecomplete("ÇëÒÀ´ÎÊäÈëÊ¹ÓÃÕß´úºÅ(Ö»°´ ENTER ½áÊøÊäÈë): ", uident);
            move(1, 0);
            clrtoeol();
            if (uident[0] == '\0')
                continue;
            if (!getuser(uident, &lookupuser)) {
                move(2, 0);
                prints("Õâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n");
                continue;
            } else
                strcpy(uident, lookupuser->userid);
        }
        switch (tmp[0]) {
        case 'A':
        case 'a':
            if (!(lookupuser->userlevel & PERM_READMAIL)) {
                move(2, 0);
                prints("ĞÅ¼şÎŞ·¨±»¼Ä¸ø: [1m%s[m\n", lookupuser->userid);
                break;
            } else if (seek_in_file(maillists, uident)) {
                move(2, 0);
                prints("ÒÑ¾­ÁĞÎªÊÕ¼şÈËÖ®Ò» \n");
                break;
            }
            addtofile(maillists, uident);
            cnt++;
            break;
        case 'E':
        case 'e':
            cnt = 0;
            break;
        case 'D':
        case 'd':
            {
                if (seek_in_file(maillists, uident)) {
                    del_from_file(maillists, uident);
                    cnt--;
                }
                break;
            }
        case 'I':
        case 'i':
            n = 0;
            clear();
            u = get_utmpent(utmpent);
            for (i = cnt; i < maxrecp && n < u->friendsnum; i++) {
                int key;

                move(2, 0);
                prints("%s\n", getuserid2(u->friends_uid[n]));
                move(4, 0);
                clrtoeol();
                move(3, 0);
                n++;
                if (!fmode) {
                    prints("(A)Ê£ÏÂµÄÈ«²¿¼ÓÈë (Y)¼ÓÈë (N)²»¼ÓÈë (Q)½áÊø? [Y]:");
                    /*
                     * TODO: add KEY_REFRESH support
                     */
                    key = igetkey();
                } else
                    key = 'Y';
                if (key == 'q' || key == 'Q')
                    break;
                if (key == 'A' || key == 'a') {
                    fmode = true;
                    key = 'Y';
                }
                if (key == '\0' || key == '\n' || key == 'y' || key == 'Y' || '\r' == key) {
                    struct userec *lookupuser;
                    char *errstr;
                    char *touserid = getuserid2(u->friends_uid[n - 1]);

                    errstr = NULL;
                    if (!touserid) {
                        errstr = "Õâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n";
                    } else {
                        strcpy(uident, getuserid2(u->friends_uid[n - 1]));
                        if (!getuser(uident, &lookupuser)) {
                            errstr = "Õâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n";
                        } else if (!(lookupuser->userlevel & PERM_READMAIL)) {
                            errstr = "ĞÅ¼şÎŞ·¨±»¼Ä¸øËû\n";
                        } else if (seek_in_file(maillists, uident)) {
                            i--;
                            continue;
                        }
                    }
                    if (errstr) {
                        if (fmode != true) {
                            move(4, 0);
                            prints(errstr);
                            pressreturn();
                        }
                        i--;
                        continue;
                    }
                    addtofile(maillists, uident);
                    cnt++;
                }
            }
            fmode = false;
            clear();
            break;
        case 'C':
        case 'c':
            unlink(maillists);
            cnt = 0;
            break;
        }
        if (tmp[0] == 'e' || tmp[0] == 'E')
            break;
        move(5, 0);
        clrtobot();
        if (cnt > maxrecp)
            cnt = maxrecp;
        move(3, 0);
        clrtobot();
        listfilecontent(maillists);
    }
    if (cnt > 0) {
        G_SENDMODE = 2;
        switch (do_gsend(NULL, NULL, cnt)) {
        case -1:
            prints("ĞÅ¼şÄ¿Â¼´íÎó\n");
            break;
        case -2:
            prints("È¡Ïû·¢ĞÅ\n");
            break;
        case -4:
            prints("ĞÅÏäÒÑ¾­³¬³öÏŞ¶î\n");
            break;
        default:
            prints("ĞÅ¼şÒÑ¼Ä³ö\n");
        }
        G_SENDMODE = 0;
        pressreturn();
    }
    return 0;
}

/*Add by SmallPig*/

static int do_gsend(char *userid[], char *title, int num)
{
    struct stat st;
    char buf2[256], buf3[STRLEN], buf4[STRLEN];
    int replymode = 1;          /* Post New UI */
    char ans[4], include_mode = 'Y';
    char filepath[STRLEN], tmpfile[STRLEN];
    int cnt;
    FILE *mp;
    extern char quote_title[120];

    /*
     * Ìí¼ÓÔÚºÃÓÑ¼ÄĞÅÊ±µÄ·¢ĞÅÉÏÏŞÏŞÖÆ Bigman 2000.12.11 
     */
    if (chkusermail(currentuser)) {
        move(1, 0);
        prints("ÄãµÄĞÅÏäÒÑ¾­³¬³öÏŞ¶î£¬ÎŞ·¨×ª¼ÄĞÅ¼ş¡£\n");
        pressreturn();
        return -4;
    }

    in_mail = true;
#if defined(MAIL_REALNAMES)
    sprintf(genbuf, "%s (%s)", currentuser->userid, currentuser->realname);
#else
    /*
     * sprintf(genbuf,"%s (%s)",currentuser->userid,currentuser->username) ; 
     */
    strcpy(genbuf, currentuser->userid);        /* Leeward 98.04.14 */
#endif
    move(1, 0);
    clrtoeol();
    if (!title) {
        replymode = 0;
        title = "Ã»Ö÷Ìâ";
        buf4[0] = '\0';
    } else
        buf4[0] = ' ';

	gettmpfilename(tmpfile, "bbs-gsend");
    //sprintf(tmpfile, "tmp/bbs-gsend/%05d", getpid());
    /*
     * Leeward 98.01.17 Prompt whom you are writing to 
     * if (1 == G_SENDMODE)
     * strcpy(lookupuser->userid, "ºÃÓÑÃûµ¥");
     * else if (2 == G_SENDMODE)
     * strcpy(lookupuser->userid, "¼ÄĞÅÃûµ¥");
     * else
     * strcpy(lookupuser->userid, "¶àÎ»ÍøÓÑ");
     */

    if (currentuser->signature > numofsig)
        currentuser->signature = 1;
    while (1) {
        sprintf(buf3, "ÒıÑÔÄ£Ê½ [[1m%c[m]", include_mode);
        move(t_lines - 3, 0);
        clrtoeol();
        prints("Ê¹ÓÃ±êÌâ: [1m%-50s[m\n", (title[0] == '\0') ? "[ÕıÔÚÉè¶¨±êÌâ]" : title);
        clrtoeol();
        if (currentuser->signature < 0)
            prints("Ê¹ÓÃËæ»úÇ©Ãûµµ     %s", (replymode) ? buf3 : "");
        else
            prints("Ê¹ÓÃµÚ [1m%d[m ¸öÇ©Ãûµµ     %s", currentuser->signature, (replymode) ? buf3 : "");

        if (buf4[0] == '\0' || buf4[0] == '\n') {
            move(t_lines - 1, 0);
            clrtoeol();
            getdata(t_lines - 1, 0, "±êÌâ: ", buf4, 50, DOECHO, NULL, true);
            if ((buf4[0] == '\0' || buf4[0] == '\n')) {
                buf4[0] = ' ';
                continue;
            }
            title = buf4;
            continue;
        }
        move(t_lines - 1, 0);
        clrtoeol();
        /*
         * Leeward 98.09.24 add: viewing signature(s) while setting post head 
         */
        sprintf(buf2, "°´[1;32m0[m~[1;32m%d/V/L[mÑ¡/¿´/Ëæ»úÇ©Ãûµµ%s£¬[1;32mT[m¸Ä±êÌâ£¬[1;32mEnter[m½ÓÊÜËùÓĞÉè¶¨: ", numofsig,
                (replymode) ? "£¬[1;32mY[m/[1;32mN[m/[1;32mR[m/[1;32mA[m¸ÄÒıÑÔÄ£Ê½" : "");
        getdata(t_lines - 1, 0, buf2, ans, 3, DOECHO, NULL, true);
        ans[0] = toupper(ans[0]);       /* Leeward 98.09.24 add; delete below toupper */
        if ((ans[0] - '0') >= 0 && ans[0] - '0' <= 9) {
            if (atoi(ans) <= numofsig)
                currentuser->signature = atoi(ans);
        } else if ((ans[0] == 'Y' || ans[0] == 'N' || ans[0] == 'A' || ans[0] == 'R') && replymode) {
            include_mode = ans[0];
        } else if (ans[0] == 'T') {
            buf4[0] = '\0';
        } else if (ans[0] == 'V') {     /* Leeward 98.09.24 add: viewing signature(s) while setting post head */
            sethomefile(buf2, currentuser->userid, "signatures");
            move(t_lines - 1, 0);
            if (askyn("Ô¤ÉèÏÔÊ¾Ç°Èı¸öÇ©Ãûµµ, ÒªÏÔÊ¾È«²¿Âğ", false) == true)
                ansimore(buf2, 0);
            else {
                clear();
                ansimore2(buf2, false, 0, 18);
            }
        } else if (ans[0] == 'L') {
            currentuser->signature = -1;
        } else {
            strncpy(save_title, title, STRLEN);
            break;
        }
    }

    /*
     * Bigman:2000.8.13 ÈºÌå·¢ĞÅÎªÊ²Ã´ÒªÒıÓÃÎÄÕÂÄØ 
     */
    /*
     * do_quote( tmpfile,include_mode ); 
     */

    strcpy(quote_title, save_title);
    if (vedit(tmpfile, true, NULL, NULL) == -1) {
        unlink(tmpfile);
        clear();
        return -2;
    }
    clear();
    if (G_SENDMODE == 2) {
        char maillists[STRLEN];

        sethomefile(maillists, currentuser->userid, "maillist");
        if ((mp = fopen(maillists, "r")) == NULL) {
            return -3;
        }
    }

    for (cnt = 0; cnt < num; cnt++) {
        char uid[13];
        char buf[STRLEN];
        struct userec *user;

        if (G_SENDMODE == 1)
            getuserid(uid, get_utmpent(utmpent)->friends_uid[cnt]);
        else if (G_SENDMODE == 2) {
            if (fgets(buf, STRLEN, mp) != NULL) {
                if (strtok(buf, " \n\r\t") != NULL)
                    strcpy(uid, buf);
                else
                    continue;
            } else {
                cnt = num;
                continue;
            }
        } else
            strcpy(uid, userid[cnt]);
        setmailpath(filepath, uid);
        if (stat(filepath, &st) == -1) {
            if (mkdir(filepath, 0755) == -1) {
                if (G_SENDMODE == 2)
                    fclose(mp);
                return -1;
            }
        } else {
            if (!(st.st_mode & S_IFDIR)) {
                if (G_SENDMODE == 2)
                    fclose(mp);
                return -1;
            }
        }

        if (getuser(uid, &user) == 0) {
            prints("ÕÒ²»µ½ÓÃ»§%s,Çë°´ Enter ¼ü¼ÌĞøÏòÆäËûÈË·¢ĞÅ...", uid);
            pressreturn();
            clear();
        } else if (user->userlevel & PERM_SUICIDE) {
            prints("%s ×ÔÉ±ÖĞ£¬²»ÄÜÊÕĞÅ£¬Çë°´ Enter ¼ü¼ÌĞøÏòÆäËûÈË·¢ĞÅ...", uid);
            pressreturn();
            clear();
        } else if (!(user->userlevel & PERM_READMAIL)) {
            prints("%s Ã»ÓĞÊÕĞÅµÄÈ¨Á¦£¬²»ÄÜÊÕĞÅ£¬Çë°´ Enter ¼ü¼ÌĞøÏòÆäËûÈË·¢ĞÅ...", uid);
            pressreturn();
            clear();
        } else if (chkusermail(user)) { /*Haohamru.99.4.05 */
            prints("%s ĞÅÏäÒÑÂú,ÎŞ·¨ÊÕĞÅ,Çë°´ Enter ¼ü¼ÌĞøÏòÆäËûÈË·¢ĞÅ...", uid);
            pressreturn();
            clear();
        } else /* ĞŞÕıºÃÓÑ·¢ĞÅµÄ´íÎó Bigman 2000.9.8 */ if (false == canIsend2(currentuser, uid)) {     /* Leeward 98.04.10 */
            char tmp_title[STRLEN], save_title_bak[STRLEN];

            prints("[1m[33mºÜ±§Ç¸¡ÃÏµÍ³ÎŞ·¨Ïò %s ·¢³ö´ËĞÅ£®ÒòÎª %s ¾Ü¾ø½ÓÊÕÄúµÄĞÅ¼ş£®\n\nÇë°´ Enter ¼ü¼ÌĞøÏòÆäËûÈË·¢ĞÅ...[m[m\n\n", uid, uid);
            pressreturn();
            clear();
            strcpy(save_title_bak, save_title);
            sprintf(tmp_title, "ÍËĞÅ¡Ã %s ¾Ü¾ø½ÓÊÕÄúµÄĞÅ¼ş£®", uid);
            mail_file(currentuser->userid, tmpfile, currentuser->userid, tmp_title, 0, NULL);
            strcpy(save_title, save_title_bak);
        } else {
            mail_file(currentuser->userid, tmpfile, uid, save_title, 0, NULL);
        }
    }
    mail_file_sent(".group", tmpfile, currentuser->userid, save_title, 0);
    unlink(tmpfile);
    if (G_SENDMODE == 2)
        fclose(mp);
    return 0;
}

/*Add by SmallPig*/
int ov_send()
{
    int all, i;
    struct user_info *u;

    /*
     * ·â½ûMail Bigman:2000.8.22 
     */
    if (HAS_PERM(currentuser, PERM_DENYMAIL))
        return DONOTHING;

    modify_user_mode(SMAIL);
    move(1, 0);
    clrtobot();
    move(2, 0);
    u = get_utmpent(utmpent);
    prints("¼ÄĞÅ¸øºÃÓÑÃûµ¥ÖĞµÄÈË£¬Ä¿Ç°±¾Õ¾ÏŞÖÆ½ö¿ÉÒÔ¼Ä¸ø [1m%d[m Î»¡£\n", maxrecp);
    if (u->friendsnum <= 0) {
        prints("Äã²¢Ã»ÓĞÉè¶¨ºÃÓÑ¡£\n");
        pressanykey();
        clear();
        return 0;
    } else {
        prints("Ãûµ¥ÈçÏÂ£º\n");
    }
    G_SENDMODE = 1;
    all = (u->friendsnum >= maxrecp) ? maxrecp : u->friendsnum;
    for (i = 0; i < all; i++) {
        char *userid;

        userid = getuserid2(u->friends_uid[i]);
        if (!userid)
            prints("\x1b[1;32m%-12s\x1b[m ", u->friends_uid[i]);
        else
            prints("%-12s ", userid);
        if ((i + 1) % 6 == 0)
            prints("\n");
    }
    pressanykey();
    switch (do_gsend(NULL, NULL, all)) {
    case -1:
        prints("ĞÅ¼şÄ¿Â¼´íÎó\n");
        break;
    case -2:
        prints("ĞÅ¼şÈ¡Ïû\n");
        break;
    case -4:
        prints("ĞÅÏäÒÑ¾­³¬³öÏŞ¶î\n");
        break;
    default:
        prints("ĞÅ¼şÒÑ¼Ä³ö\n");
    }
    pressreturn();
    G_SENDMODE = 0;
    return 0;
}

int in_group(uident, cnt)
char uident[maxrecp][STRLEN];
int cnt;
{
    int i;

    for (i = 0; i < cnt; i++)
        if (!strcmp(uident[i], uident[cnt])) {
            return i + 1;
        }
    return 0;
}

int doforward(char *direct, struct fileheader *fh, int isuu)
{
    static char address[STRLEN];
    char fname[STRLEN];
    char receiver[STRLEN];
    char title[STRLEN];
    int return_no;
    char tmp_buf[200];
    int y = 5;
    int noansi;

    clear();
    if (address[0] == '\0') {
        strncpy(address, curruserdata.email, STRLEN);
        if (strstr(curruserdata.email, "@" MAIL_BBSDOMAIN) || strlen(curruserdata.email) == 0) {
            strcpy(address, currentuser->userid);
        }
    }

    if (chkusermail(currentuser)) {
        move(1, 0);
        prints("ÄãµÄĞÅÏäÒÑ¾­³¬³öÏŞ¶î£¬ÎŞ·¨×ª¼ÄĞÅ¼ş¡£\n");
        pressreturn();
        return -4;
    }

    prints("ÇëÖ±½Ó°´ Enter ½ÓÊÜÀ¨ºÅÄÚÌáÊ¾µÄµØÖ·, »òÕßÊäÈëÆäËûµØÖ·\n");
    prints("(ÈçÒª×ªĞÅµ½×Ô¼ºµÄBBSĞÅÏä,ÇëÖ±½ÓÊäÈëÄãµÄID×÷ÎªµØÖ·¼´¿É)\n");
    prints("°Ñ %s µÄ¡¶%s¡·×ª¼Ä¸ø:", fh->owner, fh->title);
    sprintf(genbuf, "[%s]: ", address);
    getdata(3, 0, genbuf, receiver, 70, DOECHO, NULL, true);
    if (receiver[0] == '\0') {
        sprintf(genbuf, "È·¶¨½«ÎÄÕÂ¼Ä¸ø %s Âğ? (Y/N) [Y]: ", address);
        getdata(3, 0, genbuf, receiver, 3, DOECHO, NULL, true);
        if (receiver[0] == 'n' || receiver[0] == 'N')
            return 1;
        strncpy(receiver, address, STRLEN);
    } else {
        strncpy(address, receiver, STRLEN);
        /*
         * È·ÈÏµØÖ·ÊÇ·ñÕıÈ· added by dong, 1998.10.1
         */
        sprintf(genbuf, "È·¶¨½«ÎÄÕÂ¼Ä¸ø %s Âğ? (Y/N) [Y]: ", address);
        getdata(3, 0, genbuf, receiver, 3, DOECHO, NULL, true);
        if (receiver[0] == 'n' || receiver[0] == 'N')
            return 1;
        strncpy(receiver, address, STRLEN);
    }
    if (invalidaddr(receiver))
        return -2;
    if (HAS_PERM(currentuser, PERM_DENYMAIL))
        if (!strstr(receiver, "@") && !strstr(receiver, ".")) {
            prints("ÄãÉĞÎŞÈ¨ÏŞ×ª¼ÄĞÅ¼ş¸øÕ¾ÄÚÆäËüÓÃ»§¡£");
            pressreturn();
            return -22;
        }

	gettmpfilename(fname, "forward");
    //sprintf(fname, "tmp/forward/%s.%05d", currentuser->userid, getpid());
    /*
     * sprintf( tmp_buf, "cp %s/%s %s",
     * direct, fh->filename, fname);
     */
    sprintf(tmp_buf, "%s/%s", direct, fh->filename);
    f_cp(tmp_buf, fname, 0);
    sprintf(title, "%.50s(×ª¼Ä)", fh->title);   /*Haohmaru.00.05.01,moved here */
    if (askyn("ÊÇ·ñĞŞ¸ÄÎÄÕÂÄÚÈİ", 0) == 1) {
        if (vedit(fname, false, NULL, &fh->attachment) != -1) {
            if (ADD_EDITMARK)
                add_edit_mark(fname, 1, fh->title);
        }
        y = 2;
        newbbslog(BBSLOG_USER, "ĞŞ¸Ä±»×ªÌùµÄÎÄÕÂ»òĞÅ¼ş: %s", title);    /*Haohmaru.00.05.01 */
        /*
         * clear();
         */
    }


    {                           /* Leeward 98.04.27: better:-) */

        char *ptrX;

        /*
         * ptrX = strstr(receiver, ".bbs@smth.org");
         * @smth.org @zixia.net È¡µ½Ç°ÃæµÄÓÃ»§¼´¿É
         */
        ptrX = strstr(receiver, (const char *) email_domain());

        /*
         * disable by KCN      if (!ptrX) ptrX = strstr(receiver, ".bbs@");
         */
        if (ptrX && '@' == *(ptrX - 1))
            *(ptrX - 1) = 0;
    }

    if (!strstr(receiver, "@") && !strstr(receiver, ".")) {     /* sending local file need not uuencode or convert to big5... */
        struct userec *lookupuser;

        prints("×ª¼ÄĞÅ¼ş¸ø %s, ÇëÉÔºò....\n", receiver);

        return_no = getuser(receiver, &lookupuser);
        if (return_no == 0) {
            return_no = 1;
            prints("Ê¹ÓÃÕßÕÒ²»µ½...\n");
        } else {                /* ²éÍêºóÓ¦¸ÃÊ¹ÓÃlookupuserÖĞµÄÄÚÈİ,±£Ö¤´óĞ¡Ğ´ÕıÈ·  period 2000-12-13 */
            strncpy(receiver, lookupuser->userid, IDLEN + 1);
            receiver[IDLEN] = 0;

            /*
             * if(!chkreceiver(receiver,NULL))Haohamru.99.4.05
             * FIXME NULL -> lookupuser£¬ÔÚ zixia.net ÉÏÊÇÕâÃ´¸ÄµÄ... ÓĞÃ»ÓĞÎÊÌâ£¿ 
             */
            if (!HAS_PERM(currentuser, PERM_SYSOP) && lookupuser->userlevel & PERM_SUICIDE) {
                prints("%s ×ÔÉ±ÖĞ£¬²»ÄÜÊÕĞÅ\n", receiver);
                return -5;
            }
            if (!HAS_PERM(currentuser, PERM_SYSOP) && !(lookupuser->userlevel & PERM_READMAIL)) {
                prints("%s Ã»ÓĞÊÕĞÅµÄÈ¨Á¦£¬²»ÄÜÊÕĞÅ\n", receiver);
                return -5;
            }


            if (!HAS_PERM(currentuser, PERM_SYSOP) && chkusermail(lookupuser)) {        /*Haohamru.99.4.05 */
                prints("%s ĞÅÏäÒÑÂú,ÎŞ·¨ÊÕĞÅ\n", receiver);
                return -4;
            }

            if (false == canIsend2(currentuser, receiver)) {    /* Leeward 98.04.10 */
                prints("[1m[33mºÜ±§Ç¸¡ÃÏµÍ³ÎŞ·¨×ª¼Ä´ËĞÅ£®ÒòÎª %s ¾Ü¾ø½ÓÊÕÄúµÄĞÅ¼ş£®[m[m\n\n", receiver);
                sprintf(title, "ÍËĞÅ¡Ã %s ¾Ü¾ø½ÓÊÕÄúµÄĞÅ¼ş£®", receiver);
                mail_file(currentuser->userid, fname, currentuser->userid, title, 0, NULL);
                return -4;
            }
            return_no = mail_file(currentuser->userid, fname, lookupuser->userid, title, 0, fh);
        }
    } else {
        /*
         * Add by ming, 96.10.9 
         */
        char data[3];
        int isbig5;

        data[0] = 0;
        prints("ÈôÄúÒª½«ĞÅ¼ş×ª¼Äµ½Ì¨ÍåÇëÊäÈë Y »ò y\n");
        getdata(7, 0, "×ª³ÉBIG5Âë? [N]: ", data, 2, DOECHO, 0, 0);
        if (data[0] == 'y' || data[0] == 'Y')
            isbig5 = 1;
        else
            isbig5 = 0;

        getdata(8, 0, "¹ıÂËANSI¿ØÖÆ·û„1¤7? [N]: ", data, 2, DOECHO, 0, 0);
        if (data[0] == 'y' || data[0] == 'Y')
            noansi = 1;
        else
            noansi = 0;

        prints("×ª¼ÄĞÅ¼ş¸ø %s, ÇëÉÔºò....\n", receiver);

        /*
         * return_no = bbs_sendmail(fname, title, receiver); 
         */

        return_no = bbs_sendmail(fname, title, receiver, isuu, isbig5, noansi);
    }
    if (return_no==0)
        newbbslog(BBSLOG_USER, "forwarded file to %s", receiver);
    unlink(fname);
    return (return_no);
}

#endif


struct command_def {
    char *prompt;
    int permission;
    int (*func) ();
    void *arg;
};

void t_override();

const static char *mail_sysbox[] = {
    ".DIR",
    ".SENT",
    ".DELETED"
};

const static char *mail_sysboxtitle[] = {
    "I)ÊÕ¼şÏä",
    "P)·¢¼şÏä",
    "J)À¬»øÏä",
};

static int m_clean()
{
    char buf[40];
    int num;
    int savemode = uinfo.mode;

    move(0, 0);
    uinfo.mode = RMAIL;
    setmailfile(buf, currentuser->userid, mail_sysbox[1]);
    num = get_num_records(buf, sizeof(struct fileheader));
    if (num && askyn("Çå³ı·¢¼şÏäÃ´?", 0))
        delete_range(buf, 1, num, 2);
    move(0, 0);
    setmailfile(buf, currentuser->userid, mail_sysbox[2]);
    num = get_num_records(buf, sizeof(struct fileheader));
    if (num && askyn("Çå³ıÀ¬»øÏäÃ´?", 0))
        delete_range(buf, 1, num, 2);
    if (user_mail_list.mail_list_t) {
        int i;

        for (i = 0; i < user_mail_list.mail_list_t; i++) {
            char filebuf[20];

            move(0, 0);
            sprintf(filebuf, ".%s", user_mail_list.mail_list[i] + 30);
            setmailfile(buf, currentuser->userid, filebuf);
            num = get_num_records(buf, sizeof(struct fileheader));
            if (num) {
                char prompt[80];

                sprintf(prompt, "Çå³ı×Ô¶¨ÒåÓÊÏä %s Ã´?", user_mail_list.mail_list[i]);
                if (askyn(prompt, 0))
                    delete_range(buf, 1, num, 2);
            }
        }
    }
    uinfo.mode = savemode;
}

int m_sendnull()
{
    m_send(NULL);
}

const static struct command_def mail_cmds[] = {
    {"N) ÀÀÔÄĞÂĞÅ¼ş", 0, m_new, NULL},
    {"R) ÀÀÔÄÈ«²¿ĞÅ¼ş", 0, m_read, NULL},
    {"S) ¼ÄĞÅ", PERM_LOGINOK, m_sendnull, NULL},
#ifdef MAILOUT
    {"I) ·¢ËÍÕ¾ÍâĞÅ¼ş", PERM_LOGINOK, m_internet, NULL},
#endif
    {"G) ÈºÌåĞÅ¼şÑ¡µ¥", PERM_LOGINOK, set_mailgroup_list, NULL},
    /*
     * {"O)©°Éè¶¨ºÃÓÑÃûµ¥", 0, t_override, NULL},
     */
    {"F) ¼ÄĞÅ¸øËùÓĞºÃÓÑ", PERM_LOGINOK, ov_send, NULL},
    {"C) Çå¿Õ±¸·İµÄÓÊÏä", 0, m_clean, NULL},
    {"X) ÉèÖÃÓÊÏäÑ¡Ïî", 0, set_mailbox_prop, NULL},
    {"M) ¼ÄĞÅ¸øËùÓĞÈË", PERM_SYSOP, mailall, NULL},
};

struct mail_proc_arg {
    int leftpos;
    int rightpos;
    int cmdnum;
    int sysboxnum;
    int numbers;
    int tmpnum;
    int flag;

    int cmdptr[sizeof(mail_cmds) / sizeof(struct command_def)];
};

static void maillist_refresh(struct _select_def *conf)
{
    int i;

    clear();
    docmdtitle("[´¦ÀíĞÅ¼ãÑ¡µ¥]",
               "Ö÷Ñ¡µ¥[\x1b[1;32m¡û\x1b[0;37m,\x1b[1;32me\x1b[0;37m] ½øÈë[\x1b[1;32mEnter\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ı\x1b[0;37m] ×óÓÒÇĞ»»[\x1b[1;32mTab\x1b[m] Ìí¼Ó[\x1b[1;32ma\x1b[0;37m] ¸ÄÃû[\x1b[1;32mT\x1b[0;37m] É¾³ı[\x1b[1;32md\x1b[0;37m]\x1b[m");
    update_endline();

    move(2, 0);
    prints("%s", "\x1b[1;44;37m©¤©¤¹¦ÄÜÑ¡µ¥©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ğ©¤©¤©¤©¤×Ô¶¨ÒåÓÊÏä©¤©¤©¤©¤©¤©¤©¤");
    for(i=0;i<scr_cols/2-36;i++)
        prints("©¤");
    for (i = 3; i < scr_lns - 1; i++) {
        move(i, 38);
        prints("%s", "\x1b[1;44;37m©¦\x1b[m");
    }
    move(17, 0);
    prints("%s", "\x1b[1;44;37m©¤©¤ÏµÍ³Ô¤¶¨ÒåÓÊÏä©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©È\x1b[m");

    if (user_mail_list.mail_list_t == 0) {
        move(14, 46);
        prints("%s", "ÎŞ×Ô¶¨ÒåÓÊÏä");
    }
}
static int maillist_show(struct _select_def *conf, int pos)
{
    struct mail_proc_arg *arg = (struct mail_proc_arg *) conf->arg;
    char buf[80];

    if (pos <= arg->cmdnum) {
        /*
         * ÓÊÏäÃüÁî
         */
        outs(mail_cmds[arg->cmdptr[pos - 1]].prompt);
    } else if (pos <= arg->sysboxnum + arg->cmdnum) {
        int sel;

        sel = pos - arg->cmdnum - 1;
	if (arg->flag)
            outs(mail_sysbox[sel]+1);
	else
            outs(mail_sysboxtitle[sel]);

        setmailfile(buf, currentuser->userid, mail_sysbox[sel]);
        prints("(%d)", getmailnum(buf));
    } else {
        /*
         * ×Ô¶¨ÒåÓÊÏä
         */
        int sel;
        char dirbstr[60];

        sel = pos - arg->cmdnum - arg->sysboxnum;
        if (sel < 10)
            outc(' ');
        sprintf(dirbstr, ".%s", user_mail_list.mail_list[sel - 1] + 30);
	if (arg->flag)
            prints("%d) %s", sel, user_mail_list.mail_list[sel - 1] + 30);
	else
            prints("%d) %s", sel, user_mail_list.mail_list[sel - 1]);
        setmailfile(buf, currentuser->userid, dirbstr);
        prints("(%d)", getmailnum(buf));
    }
    return SHOW_CONTINUE;
}

static int maillist_onselect(struct _select_def *conf)
{
    struct mail_proc_arg *arg = (struct mail_proc_arg *) conf->arg;
    char buf[20];
    char curmaildir[STRLEN];

    if (conf->pos <= arg->cmdnum) {
        /*
         * ÓÊÏäÃüÁî
         */
        (*mail_cmds[arg->cmdptr[conf->pos - 1]].func) ();
    } else if (conf->pos <= arg->sysboxnum + arg->cmdnum) {
        int sel;

        sel = conf->pos - arg->cmdnum - 1;
        setmailfile(curmaildir,currentuser->userid, mail_sysbox[sel]);
        in_mail = true;
        i_read(RMAIL, curmaildir, mailtitle, (READ_FUNC) maildoent, &mail_comms[0], sizeof(struct fileheader));
        in_mail = false;
        /*
         * ÏµÍ³ÓÊÏä
         */
    } else {
        /*
         * ×Ô¶¨ÒåÓÊÏä
         */
        int sel;

        sel = conf->pos - arg->sysboxnum - arg->cmdnum - 1;
        sprintf(buf, ".%s", user_mail_list.mail_list[sel] + 30);
        setmailfile(curmaildir, currentuser->userid, buf);
        in_mail = true;
        i_read(RMAIL, curmaildir, mailtitle, (READ_FUNC) maildoent, &mail_comms[0], sizeof(struct fileheader));
        in_mail = false;
    }
    modify_user_mode(MAIL);
    return SHOW_REFRESH;
}

static int maillist_prekey(struct _select_def *conf, int *command)
{
    struct mail_proc_arg *arg = (struct mail_proc_arg *) conf->arg;

    /*
     * Èç¹ûÊÇ×ó¼ü²¢ÇÒµ½ÁË×ó±ß
     */
    if (*command == KEY_RIGHT) {
        if ((user_mail_list.mail_list_t == 0) || (conf->pos > arg->cmdnum + arg->sysboxnum))
            *command = '\n';
        else
            *command = '\t';
    }
    if (*command == KEY_LEFT) {
        if ((conf->pos <= arg->cmdnum + arg->sysboxnum))
            return SHOW_QUIT;
        else {
            *command = '\t';
            return SHOW_CONTINUE;
        }
    }

    if (*command == 'e')
        return SHOW_QUIT;
    update_endline();
    if (!isdigit(*command))
        arg->tmpnum = -1;
    return SHOW_CONTINUE;
}

static int maillist_key(struct _select_def *conf, int command)
{
    struct mail_proc_arg *arg = (struct mail_proc_arg *) conf->arg;
    int i;

    if (command == '\t') {
        if (conf->pos <= arg->cmdnum + arg->sysboxnum) {
            /*
             * ×ó±ß
             */
            if (!user_mail_list.mail_list_t)
                return SHOW_CONTINUE;
            arg->leftpos = conf->pos;
            conf->new_pos = arg->rightpos;
        } else {
            arg->rightpos = conf->pos;
            conf->new_pos = arg->leftpos;
        }
        return SHOW_SELCHANGE;
    }
    if (toupper(command) == 'H') {
        mailreadhelp();
        return SHOW_REFRESH;
    }

    if (toupper(command) == 'Z') {
	arg->flag=!arg->flag;
        return SHOW_REFRESH;
    }
    if (toupper(command) == 'A') {
        char bname[STRLEN], buf[PATHLEN];
        int i = 0, y, x;
        struct stat st;

        if (!HAS_PERM(currentuser, PERM_LOGINOK))
            return SHOW_CONTINUE;
        if (user_mail_list.mail_list_t >= MAILBOARDNUM) {
            move(2, 0);
            clrtoeol();
            prints("ÓÊÏäÊıÒÑ¾­´ïÉÏÏŞ(%d)£¡", MAILBOARDNUM);
            pressreturn();
            return SHOW_REFRESH;
        }
        move(0, 0);
        clrtoeol();
        getdata(0, 0, "ÊäÈë×Ô¶¨ÒåÓÊÏäÏÔÊ¾ÖĞÎÄÃû: ", buf, 30, DOECHO, NULL, true);
        if (buf[0] == 0) {
            return SHOW_REFRESH;
        }
        strncpy(user_mail_list.mail_list[user_mail_list.mail_list_t], buf, 29);
        move(0, 0);
        clrtoeol();
        while (1) {
            i++;
            sprintf(bname, ".MAILBOX%d", i);
            setmailfile(buf, currentuser->userid, bname);
            if (stat(buf, &st) == -1)
                break;
        }
        sprintf(bname, "MAILBOX%d", i);
        f_touch(buf);
        strncpy(user_mail_list.mail_list[user_mail_list.mail_list_t] + 30, bname, 9);
        user_mail_list.mail_list_t++;
        save_mail_list(&user_mail_list);
        x = 0;

        y = 3 + (20 - user_mail_list.mail_list_t) / 2;
        arg->numbers++;
        conf->item_count = arg->numbers;
        conf->item_per_page = arg->numbers;
        for (i = arg->cmdnum + arg->sysboxnum; i < arg->cmdnum + arg->sysboxnum + user_mail_list.mail_list_t; i++) {
            conf->item_pos[i].x = 44;
            conf->item_pos[i].y = y + i - arg->cmdnum - arg->sysboxnum;
        }
        return SHOW_REFRESH;
    }
    if (toupper(command) == 'D') {
        int p = 1, i, j;
        char ans[2];
        int num, y;

        if (!HAS_PERM(currentuser, PERM_LOGINOK))
            return SHOW_CONTINUE;
        if (conf->pos <= arg->cmdnum + arg->sysboxnum)
            return SHOW_CONTINUE;
        move(0, 0);
        clrtoeol();
        getdata(0, 0, "È·ÈÏÉ¾³ıÕû¸öÄ¿Â¼£¿(y/N)", ans, 2, DOECHO, NULL, true);
        p = ans[0] == 'Y' || ans[0] == 'y';
        if (p) {
            p = conf->pos - arg->cmdnum - arg->sysboxnum - 1;
            for (j = p; j < user_mail_list.mail_list_t - 1; j++)
                memcpy(user_mail_list.mail_list[j], user_mail_list.mail_list[j + 1], sizeof(user_mail_list.mail_list[j]));
            user_mail_list.mail_list_t--;
            save_mail_list(&user_mail_list);
            y = 3 + (20 - user_mail_list.mail_list_t) / 2;
            arg->numbers--;
            conf->item_count = arg->numbers;
            conf->item_per_page = arg->numbers;
            if (conf->pos > arg->numbers)
                conf->pos = arg->numbers;
            for (i = arg->cmdnum + arg->sysboxnum; i < arg->cmdnum + arg->sysboxnum + user_mail_list.mail_list_t; i++) {
                conf->item_pos[i].x = 44;
                conf->item_pos[i].y = y + i - arg->cmdnum - arg->sysboxnum;
            }
            return SHOW_REFRESH;
        }
        return SHOW_REFRESH;
    }
    if (command == 'T') {
        int p = 1, i, j;
        char bname[STRLEN];
        int num;
        char ans[2];

        if (!HAS_PERM(currentuser, PERM_LOGINOK))
            return SHOW_CONTINUE;
        if (conf->pos <= arg->cmdnum + arg->sysboxnum)
            return SHOW_CONTINUE;
        move(0, 0);
        clrtoeol();
        i = conf->pos - arg->cmdnum - arg->sysboxnum - 1;
        strcpy(bname, user_mail_list.mail_list[i]);
        getdata(0, 0, "ÊäÈëĞÅÏäÖĞÎÄÃû: ", bname, 30, DOECHO, NULL, false);
        if (bname[0]) {
            strcpy(user_mail_list.mail_list[i], bname);
            save_mail_list(&user_mail_list);
            return SHOW_REFRESH;
        }
        return SHOW_REFRESH;
    }

    for (i = 0; i < arg->cmdnum; i++)
        if (toupper(command) == mail_cmds[i].prompt[0]) {
            conf->new_pos = i + 1;
            return SHOW_SELCHANGE;
        }
    for (i = 0; i < arg->sysboxnum; i++)
        if (toupper(command) == mail_sysboxtitle[i][0]) {
            conf->new_pos = i + arg->cmdnum + 1;
            return SHOW_SELCHANGE;
        }
    if (isdigit(command)) {
        int num;

        num = command - '0';
        if ((arg->tmpnum == -1) || (num + arg->tmpnum * 10 > user_mail_list.mail_list_t)) {
            arg->tmpnum = command - '0';
            if (arg->tmpnum <= user_mail_list.mail_list_t) {
                conf->new_pos = arg->tmpnum + arg->cmdnum + arg->sysboxnum;
                return SHOW_SELCHANGE;
            }
        } else {
            conf->new_pos = arg->tmpnum * 10 + arg->cmdnum + arg->sysboxnum + num;
            arg->tmpnum = command - '0';
            return SHOW_SELCHANGE;
        }
    }
    return SHOW_CONTINUE;
}

int MailProc()
{
    struct _select_def maillist_conf;
    struct mail_proc_arg arg;
    POINT *pts;
    int i;
    int y;
    int oldmode;

    oldmode = uinfo.mode;
    modify_user_mode(MAIL);
    clear();
    bzero(&arg,sizeof(arg));
    arg.tmpnum = -1;
    arg.cmdnum = 0;
    for (i = 0; i < sizeof(mail_cmds) / sizeof(struct command_def); i++) {
        if (HAS_PERM(currentuser, mail_cmds[i].permission)) {
            arg.cmdptr[arg.cmdnum] = i;
            arg.cmdnum++;
        }
    }
    arg.sysboxnum = sizeof(mail_sysbox) / sizeof(char *);
    arg.numbers = user_mail_list.mail_list_t + arg.cmdnum + arg.sysboxnum;
    arg.leftpos = 2;
    arg.rightpos = arg.cmdnum + arg.sysboxnum + 1;
    pts = (POINT *) malloc(sizeof(POINT) * (arg.numbers + MAILBOARDNUM));

    /*
     * ¼ÆËãÓÊÏäÃüÁîµØÎ»ÖÃ
     */
    y = 2 + (17 - arg.cmdnum) / 2;
    for (i = 0; i < arg.cmdnum; i++) {
        pts[i].x = 6;
        pts[i].y = y + i;
    }

    y = 18 + (5 - arg.sysboxnum) / 2;
    /*
     * ¼ÆËãÏµÍ³ÓÊÏäµØÎ»ÖÃ
     */
    for (; i < arg.cmdnum + arg.sysboxnum; i++) {
        pts[i].x = 6;
        pts[i].y = y + i - arg.cmdnum;
    }
    /*
     * ¼ÆËã×Ô¶¨ÒåÓÊÏäµØÎ»ÖÃ
     */

    y = 3 + (20 - user_mail_list.mail_list_t) / 2;
    for (; i < arg.cmdnum + arg.sysboxnum + user_mail_list.mail_list_t; i++) {
        pts[i].x = 44;
        pts[i].y = y + i - arg.cmdnum - arg.sysboxnum;
    }
    bzero((char *) &maillist_conf, sizeof(struct _select_def));
    maillist_conf.item_count = arg.numbers;
    maillist_conf.item_per_page = arg.numbers;
    maillist_conf.flag = LF_BELL | LF_LOOP;     /*|LF_HILIGHTSEL; */
    maillist_conf.prompt = "¡ô";
    maillist_conf.item_pos = pts;
    maillist_conf.arg = &arg;
    maillist_conf.title_pos.x = 1;
    maillist_conf.title_pos.y = 6;
    maillist_conf.pos = 2;

    maillist_conf.on_select = maillist_onselect;
    maillist_conf.show_data = maillist_show;
    maillist_conf.pre_key_command = maillist_prekey;
    maillist_conf.key_command = maillist_key;
    maillist_conf.show_title = maillist_refresh;

    list_select_loop(&maillist_conf);
    free(pts);
    modify_user_mode(oldmode);
}

typedef struct {
    unsigned int prop;
    unsigned int oldprop;
} mailbox_prop_arg;

static int set_mailbox_prop_select(struct _select_def *conf)
{
    mailbox_prop_arg *arg = (mailbox_prop_arg *) conf->arg;

    if (conf->pos == conf->item_count)
        return SHOW_QUIT;
    arg->prop ^= (1 << (conf->pos - 1));
    return SHOW_REFRESHSELECT;
}

static int set_mailbox_prop_show(struct _select_def *conf, int i)
{
    mailbox_prop_arg *arg = (mailbox_prop_arg *) conf->arg;

    i = i - 1;
    if (i == conf->item_count - 1) {
        prints("%c. ÍË³ö ", 'A' + i);
    } else {
        if ((arg->prop & (1 << i)) != (arg->oldprop & (1 << i)))
            prints("%c. %-50s [31;1m%3s[m", 'A' + i, mailbox_prop_str[i], ((arg->prop >> i) & 1 ? "ON" : "OFF"));
        else
            prints("%c. %-50s [37;0m%3s[m", 'A' + i, mailbox_prop_str[i], ((arg->prop >> i) & 1 ? "ON" : "OFF"));
    }
    return SHOW_CONTINUE;
}

static int set_mailbox_prop_key(struct _select_def *conf, int key)
{
    int sel;

    if (key == Ctrl('Q'))
        return SHOW_QUIT;
    if (key == Ctrl('A')) {
        mailbox_prop_arg *arg = (mailbox_prop_arg *) conf->arg;

        arg->prop = arg->oldprop;
        return SHOW_QUIT;
    }
    if (key <= 'z' && key >= 'a')
        sel = key - 'a';
    else
        sel = key - 'A';
    if (sel >= 0 && sel < (conf->item_count)) {
        conf->new_pos = sel + 1;
        return SHOW_SELCHANGE;
    }
    return SHOW_CONTINUE;
}

/**
 * Setting currentuser's mailbox properties.
 *
 * @authur flyriver
 */
int set_mailbox_prop()
{
    struct _select_def proplist_conf;
    mailbox_prop_arg arg;
    POINT *pts;
    int i;

    clear();
    move(0, 0);
    prints("Éè¶¨ÓÊÏäÊôĞÔ£¬[1;32mCtrl+Q[mÍË³ö£¬[1;32mCtrl+A[m·ÅÆúĞŞ¸ÄÍË³ö.\n");
    arg.prop = load_mailbox_prop(currentuser->userid);
    arg.oldprop = arg.prop;
    pts = (POINT *) malloc(sizeof(POINT) * (MBP_NUMS + 1));
    for (i = 0; i < MBP_NUMS + 1; i++) {
        pts[i].x = 2;
        pts[i].y = i + 2;
    }
    bzero(&proplist_conf, sizeof(struct _select_def));
    proplist_conf.item_count = MBP_NUMS + 1;
    proplist_conf.item_per_page = MBP_NUMS + 1;
    proplist_conf.flag = LF_BELL | LF_LOOP;
    proplist_conf.prompt = "¡ô";
    proplist_conf.item_pos = pts;
    proplist_conf.arg = &arg;
    proplist_conf.title_pos.x = 1;
    proplist_conf.title_pos.y = 2;
    proplist_conf.pos = MBP_NUMS + 1;

    proplist_conf.on_select = set_mailbox_prop_select;
    proplist_conf.show_data = set_mailbox_prop_show;
    proplist_conf.key_command = set_mailbox_prop_key;

    list_select_loop(&proplist_conf);
    free(pts);
    uinfo.mailbox_prop = update_mailbox_prop(currentuser->userid, arg.prop);
    store_mailbox_prop(currentuser->userid);

    return 0;
}

typedef struct {
    int tmpnum;
    mailgroup_list_t *mgl;
    int entry;
    mailgroup_t *users;
} mailgroup_arg;

static int set_mailgroup_select(struct _select_def *conf)
{
    mailgroup_arg *arg = (mailgroup_arg *) conf->arg;
    int oldmode;

    oldmode = uinfo.mode;
    t_query(arg->users[conf->pos - 1].id);
    modify_user_mode(oldmode);

    return SHOW_REFRESH;
}

static int set_mailgroup_show(struct _select_def *conf, int i)
{
    mailgroup_arg *arg = (mailgroup_arg *) conf->arg;

    prints(" %3d  %-12s  %-14s", i, arg->users[i - 1].id, arg->users[i - 1].exp);
    return SHOW_CONTINUE;
}

static int set_mailgroup_prekey(struct _select_def *conf, int *key)
{
    mailgroup_arg *arg = (mailgroup_arg *) conf->arg;

    if ((*key == '\r' || *key == '\n') && (arg->tmpnum != 0)) {
        conf->new_pos = arg->tmpnum;
        arg->tmpnum = 0;
        return SHOW_SELCHANGE;
    }

    if (!isdigit(*key))
        arg->tmpnum = 0;

    switch (*key) {
    case 'e':
    case 'q':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case ' ':
    case 'N':
        *key = KEY_PGDN;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int set_mailgroup_key(struct _select_def *conf, int key)
{
    mailgroup_arg *arg = (mailgroup_arg *) conf->arg;
    int oldmode;

    if (key >= '0' && key <= '9') {
        arg->tmpnum = arg->tmpnum * 10 + (key - '0');
        return SHOW_CONTINUE;
    }
    switch (key) {
    case 'a':                  /* add new user */
        if (arg->mgl->groups[arg->entry].users_num < MAX_MAILGROUP_USERS) {
            mailgroup_t user;

            bzero(&user, sizeof(user));
            clear();
            move(1, 0);
            usercomplete("ÇëÊäÈëÒªÔö¼ÓµÄÓÃ»§´úºÅ: ", user.id);
            if (user.id[0] != '\0') {
                if (searchuser(user.id) <= 0) {
                    move(2, 0);
                    prints(MSG_ERR_USERID);
                    pressanykey();
                } else {
                    move(2, 0);
                    getdata(2, 0, "ÇëÊäÈëÓÃ»§ËµÃ÷: ", user.exp, sizeof(user.exp), DOECHO, NULL, true);
                    add_mailgroup_user(arg->mgl, arg->entry, arg->users, &user);
                }
            }
            return SHOW_DIRCHANGE;
        }
        break;
    case 'd':                  /* delete existed user */
        if (arg->mgl->groups[arg->entry].users_num > 0) {
            char ans[3];

            getdata(t_lines - 1, 0, "È·ÊµÒª´Ó×éÖĞÉ¾³ı¸ÃÓÃ»§Âğ(Y/N)? [N]: ", ans, sizeof(ans), DOECHO, NULL, true);
            if (ans[0] == 'Y' || ans[0] == 'y') {
                delete_mailgroup_user(arg->mgl, arg->entry, arg->users, conf->pos - 1);
            }
            return SHOW_DIRCHANGE;
        }
        break;
    case 'T':                  /* modify existed user */
        if (arg->mgl->groups[arg->entry].users_num > 0) {
            mailgroup_t user;

            memcpy(&user, &(arg->users[conf->pos - 1]), sizeof(user));
            getdata(0, 0, "ÇëÊäÈëĞÂÓÃ»§ËµÃ÷: ", user.exp, sizeof(user.exp), DOECHO, NULL, true);
            if (strlen(user.exp) > 0)
                modify_mailgroup_user(arg->users, conf->pos - 1, &user);
            return SHOW_DIRCHANGE;
        }
        break;
    case 'm':                  /* send mail to a user */
        if (arg->mgl->groups[arg->entry].users_num > 0) {
            oldmode = uinfo.mode;
            modify_user_mode(FRIEND);   /* FIXME: A temporary workaround for 
                                         * the buggy m_send() function. */
            m_send(arg->users[conf->pos - 1].id);
            modify_user_mode(oldmode);
            return SHOW_REFRESH;
        }
        break;
    case 'z':                  /* send message to a user */
        if (arg->mgl->groups[arg->entry].users_num > 0) {
            struct user_info *uin;
            extern char MsgDesUid[];

            if (!HAS_PERM(currentuser, PERM_PAGE))
                break;
            oldmode = uinfo.mode;
            clear();
            uin = (struct user_info *) t_search(arg->users[conf->pos - 1].id, 0);
            if (!uin || !canmsg(currentuser, uin))
                do_sendmsg(NULL, NULL, 0);
            else {
                strcpy(MsgDesUid, uin->userid);
                do_sendmsg(uin, NULL, 0);
            }
            modify_user_mode(oldmode);
            return SHOW_REFRESH;
        }
        break;
    case Ctrl('Z'):
        oldmode = uinfo.mode;
        r_lastmsg();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'L':
    case 'l':
        oldmode = uinfo.mode;
        show_allmsgs();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'W':
    case 'w':
        oldmode = uinfo.mode;
        if (!HAS_PERM(currentuser, PERM_PAGE))
            break;
        s_msg();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'u':
        oldmode = uinfo.mode;
        clear();
        modify_user_mode(QUERY);
        t_query(NULL);
        modify_user_mode(oldmode);
        clear();
        return SHOW_REFRESH;
    }

    return SHOW_CONTINUE;
}

static int set_mailgroup_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[ÉèÖÃÈºÌåĞÅ¼ş×é]",
               "ÍË³ö[\x1b[1;32m¡û\x1b[0;37m,\x1b[1;32me\x1b[0;37m] ½øÈë[\x1b[1;32mEnter\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ı\x1b[0;37m] Ìí¼Ó[\x1b[1;32ma\x1b[0;37m] ĞŞ¸ÄËµÃ÷[\x1b[1;32mT\x1b[0;37m] É¾³ı[\x1b[1;32md\x1b[0;37m]\x1b[m ·¢ĞÅ[\x1b[1;32mm\x1b[0;37m]\x1b[m");
    move(2, 0);
    prints("[0;1;37;44m  %4s  %-12s  %-58s", "±àºÅ", "ÓÃ»§´úºÅ", "ÓÃ»§ËµÃ÷");
    clrtoeol();
    update_endline();
    return SHOW_CONTINUE;
}

static int set_mailgroup_getdata(struct _select_def *conf, int pos, int len)
{
    mailgroup_arg *arg = (mailgroup_arg *) conf->arg;

    conf->item_count = arg->mgl->groups[arg->entry].users_num;

    return SHOW_CONTINUE;
}

static int init_mailgroup(mailgroup_list_t * mgl, int entry, mailgroup_t * users)
{
    mailgroup_t user;
    int ret = 0;

    clear();
    move(0, 0);
    prints("³õÊ¼»¯ÈºÌåĞÅ¼ş×éÓÃ»§Ïòµ¼\n");
    bzero(&user, sizeof(user));
    move(1, 0);
    usercomplete("ÇëÊäÈëÒªÔö¼ÓµÄÓÃ»§´úºÅ: ", user.id);
    if (user.id[0] != '\0') {
        if (searchuser(user.id) <= 0) {
            move(2, 0);
            prints(MSG_ERR_USERID);
            pressanykey();
        } else {
            move(2, 0);
            getdata(2, 0, "ÇëÊäÈëÓÃ»§ËµÃ÷: ", user.exp, sizeof(user.exp), DOECHO, NULL, true);
            add_mailgroup_user(mgl, entry, users, &user);
            move(3, 0);
            prints("³õÊ¼»¯Íê³É!\n");
            pressanykey();
            ret = 1;
        }
    }
    return ret;
}

/**
 * Setting currentuser's mailgroup.
 *
 * @authur flyriver
 */
int set_mailgroup(mailgroup_list_t * mgl, int entry, mailgroup_t * users)
{
    struct _select_def group_conf;
    mailgroup_arg arg;
    POINT *pts;
    int i;

    arg.mgl = mgl;
    arg.entry = entry;
    arg.users = users;
    arg.tmpnum = 0;

    bzero(&group_conf, sizeof(struct _select_def));
    group_conf.item_count = load_mailgroup(currentuser->userid, mgl->groups[entry].group_name, users, mgl->groups[entry].users_num);
    if (group_conf.item_count == 0) {
        group_conf.item_count = init_mailgroup(mgl, entry, users);
        if (group_conf.item_count == 0)
            return -1;
    }

    clear();
    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    group_conf.item_per_page = BBS_PAGESIZE;
    /*
     * ¼ÓÉÏ LF_VSCROLL ²ÅÄÜÓÃ LEFT ¼üÍË³ö 
     */
    group_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    group_conf.prompt = "¡ô";
    group_conf.item_pos = pts;
    group_conf.arg = &arg;
    group_conf.title_pos.x = 0;
    group_conf.title_pos.y = 0;
    group_conf.pos = 1;         /* initialize cursor on the first mailgroup */
    group_conf.page_pos = 1;    /* initialize page to the first one */

    group_conf.on_select = set_mailgroup_select;
    group_conf.show_data = set_mailgroup_show;
    group_conf.pre_key_command = set_mailgroup_prekey;
    group_conf.key_command = set_mailgroup_key;
    group_conf.show_title = set_mailgroup_refresh;
    group_conf.get_data = set_mailgroup_getdata;

    list_select_loop(&group_conf);
    store_mailgroup(currentuser->userid, mgl->groups[entry].group_name, users, mgl->groups[entry].users_num);
    free(pts);

    return 0;
}

typedef struct {
    int tmpnum;
    mailgroup_list_t mail_group;
    mailgroup_t users[MAX_MAILGROUP_USERS];
} mailgroup_list_arg;

static int set_mailgroup_list_select(struct _select_def *conf)
{
    mailgroup_list_arg *arg = (mailgroup_list_arg *) conf->arg;

    bzero(arg->users, sizeof(mailgroup_t) * MAX_MAILGROUP_USERS);
    set_mailgroup(&(arg->mail_group), conf->pos - 1, arg->users);

    return SHOW_REFRESH;
}

static int set_mailgroup_list_show(struct _select_def *conf, int i)
{
    mailgroup_list_arg *arg = (mailgroup_list_arg *) conf->arg;

    prints("  %2d  %-40s  %3d", i, arg->mail_group.groups[i - 1].group_desc, arg->mail_group.groups[i - 1].users_num);
    return SHOW_CONTINUE;
}

static int set_mailgroup_list_prekey(struct _select_def *conf, int *key)
{
    mailgroup_list_arg *arg = (mailgroup_list_arg *) conf->arg;

    if ((*key == '\r' || *key == '\n') && (arg->tmpnum != 0)) {
        conf->new_pos = arg->tmpnum;
        arg->tmpnum = 0;
        return SHOW_SELCHANGE;
    }

    if (!isdigit(*key))
        arg->tmpnum = 0;

    switch (*key) {
    case 'e':
    case 'q':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case ' ':
    case 'N':
        *key = KEY_PGDN;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int set_mailgroup_list_key(struct _select_def *conf, int key)
{
    mailgroup_list_arg *arg = (mailgroup_list_arg *) conf->arg;
    int oldmode;

    if (key >= '0' && key <= '9') {
        arg->tmpnum = arg->tmpnum * 10 + (key - '0');
        return SHOW_CONTINUE;
    }
    switch (key) {
    case 'a':                  /* add new mailgroup */
        if (arg->mail_group.groups_num < MAX_MAILGROUP_NUM) {
            mailgroup_list_item item;
            char ans[3];
            char filename[STRLEN];
            int y = 0;
            int initialized = 0;

            clear();
            sethomefile(filename, currentuser->userid, "friends");
            if (dashf(filename)) {
                getdata(y, 0, "ÊÇ·ñµ¼ÈëºÃÓÑÃûµ¥(Y/N)? [Y]: ", ans, sizeof(ans), DOECHO, NULL, true);
                y++;
                if (ans[0] == '\0' || ans[0] == 'Y' || ans[0] == 'y') {
                    move(y, 0);
                    prints("µ¼ÈëºÃÓÑÃûµ¥... ");
                    import_friends_mailgroup(currentuser->userid, &(arg->mail_group));
                    initialized++;
                    prints("[[0;1;32m³É¹¦[m]\n");
                    y++;
                }
            }
            if (initialized == 0) {
                bzero(&item, sizeof(item));
                getdata(y, 0, "ÇëÊäÈëĞÂÈºÌåĞÅ¼ş×éµÄÃû³Æ: ", item.group_desc, sizeof(item.group_desc), DOECHO, NULL, true);
                add_mailgroup_item(currentuser->userid, &(arg->mail_group), &item);
            }
            pressanykey();
            return SHOW_DIRCHANGE;
        }
        break;
    case 'd':                  /* delete existed mailgroup */
        if (arg->mail_group.groups_num > 0) {
            char ans[3];

            getdata(t_lines - 1, 0, "È·ÊµÒªÉ¾³ı¸ÃÈºÌåĞÅ¼ş×éÂğ(Y/N)? [N]: ", ans, sizeof(ans), DOECHO, NULL, true);
            if (ans[0] == 'Y' || ans[0] == 'y') {
                delete_mailgroup_item(currentuser->userid, &(arg->mail_group), conf->pos - 1);
                if (conf->item_count == 0) {
                    add_default_mailgroup_item(currentuser->userid, &(arg->mail_group));
                }
            }
            return SHOW_DIRCHANGE;
        }
        break;
    case 'T':                  /* modify existed mailgroup */
        if (arg->mail_group.groups_num > 0) {
            mailgroup_list_item item;

            memcpy(&item, &(arg->mail_group.groups[conf->pos - 1]), sizeof(item));
            getdata(0, 0, "ÇëÊäÈëĞÂÈºÌåĞÅ¼ş×éµÄÃû³Æ: ", item.group_desc, sizeof(item.group_desc), DOECHO, NULL, true);
            if (strlen(item.group_desc) > 0)
                modify_mailgroup_item(currentuser->userid, &(arg->mail_group), conf->pos - 1, &item);
            return SHOW_DIRCHANGE;
        }
        break;
    case 'm':
        if (arg->mail_group.groups_num > 0 && arg->mail_group.groups[conf->pos - 1].users_num > 0) {
            char **mg_users;
            int cnt;
            int i;

            cnt = arg->mail_group.groups[conf->pos - 1].users_num;
            mg_users = (char **) malloc(cnt * sizeof(char *));
            if (mg_users == NULL)
                break;
            load_mailgroup(currentuser->userid, arg->mail_group.groups[conf->pos - 1].group_name, arg->users, cnt);
            for (i = 0; i < cnt; i++)
                mg_users[i] = arg->users[i].id;
            clear();
            G_SENDMODE = 0;
            switch (do_gsend(mg_users, NULL, cnt)) {
            case -1:
                prints("ĞÅ¼şÄ¿Â¼´íÎó\n");
                break;
            case -2:
                prints("È¡Ïû·¢ĞÅ\n");
                break;
            case -4:
                prints("ĞÅÏäÒÑ¾­³¬³öÏŞ¶î\n");
                break;
            default:
                prints("ĞÅ¼şÒÑ¼Ä³ö\n");
            }
            free(mg_users);
            pressreturn();
            return SHOW_REFRESH;
        }
        break;
    case Ctrl('Z'):
        oldmode = uinfo.mode;
        r_lastmsg();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'L':
    case 'l':
        oldmode = uinfo.mode;
        show_allmsgs();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'W':
    case 'w':
        oldmode = uinfo.mode;
        if (!HAS_PERM(currentuser, PERM_PAGE))
            break;
        s_msg();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'u':
        oldmode = uinfo.mode;
        clear();
        modify_user_mode(QUERY);
        t_query(NULL);
        modify_user_mode(oldmode);
        clear();
        return SHOW_REFRESH;
    }

    return SHOW_CONTINUE;
}

static int set_mailgroup_list_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[ÈºÌåĞÅ¼şÑ¡µ¥]",
               "ÍË³ö[\x1b[1;32m¡û\x1b[0;37m,\x1b[1;32me\x1b[0;37m] ½øÈë[\x1b[1;32mEnter\x1b[0;37m] Ñ¡Ôñ[\x1b[1;32m¡ü\x1b[0;37m,\x1b[1;32m¡ı\x1b[0;37m] Ìí¼Ó[\x1b[1;32ma\x1b[0;37m] ¸ÄÃû[\x1b[1;32mT\x1b[0;37m] É¾³ı[\x1b[1;32md\x1b[0;37m]\x1b[m ·¢ÈºÌåĞÅ[\x1b[1;32mm\x1b[0;37m]\x1b[m");
    move(2, 0);
    prints("[0;1;37;44m  %4s  %-40s %-31s", "±àºÅ", "ÈºÌåĞÅ¼ş×éÃû³Æ", "ÈËÊı");
    clrtoeol();
    update_endline();
    return SHOW_CONTINUE;
}

static int set_mailgroup_list_getdata(struct _select_def *conf, int pos, int len)
{
    mailgroup_list_arg *arg = (mailgroup_list_arg *) conf->arg;

    conf->item_count = arg->mail_group.groups_num;

    return SHOW_CONTINUE;
}

static int init_mailgroup_list(mailgroup_list_t * mgl)
{
    char filename[STRLEN];
    char ans[3];
    int y = 2;
    int initialized = 0;

    move(0, 0);
    prints("³õÊ¼»¯ÈºÌåĞÅ¼ş·Ö×éÏòµ¼\n");
    sethomefile(filename, currentuser->userid, "maillist");
    if (dashf(filename)) {
        getdata(y, 0, "ÊÇ·ñµ¼ÈëÀÏ°æ±¾µÄÈºÌåĞÅ¼ş×é(Y/N)? [Y]: ", ans, sizeof(ans), DOECHO, NULL, true);
        if (ans[0] == '\0' || ans[0] == 'Y' || ans[0] == 'y') {
            y++;
            move(y, 0);
            prints("µ¼ÈëÀÏ°æ±¾µÄÈºÌåĞÅ¼ş×é... ");
            import_old_mailgroup(currentuser->userid, mgl);
            unlink(filename);
            initialized++;
            prints("[[0;1;32m³É¹¦[m]\n");
            y++;
        }
    }
    sethomefile(filename, currentuser->userid, "friends");
    if (dashf(filename)) {
        getdata(y, 0, "ÊÇ·ñµ¼ÈëºÃÓÑÃûµ¥(Y/N)? [Y]: ", ans, sizeof(ans), DOECHO, NULL, true);
        if (ans[0] == '\0' || ans[0] == 'Y' || ans[0] == 'y') {
            y++;
            move(y, 0);
            prints("µ¼ÈëºÃÓÑÃûµ¥... ");
            import_friends_mailgroup(currentuser->userid, mgl);
            initialized++;
            prints("[[0;1;32m³É¹¦[m]\n");
            y++;
        }
    }
    if (initialized == 0) {
        add_default_mailgroup_item(currentuser->userid, mgl);
        initialized++;
    }
    move(y, 0);
    prints("³õÊ¼»¯Íê³É£¡\n");
    pressanykey();
    return initialized;
}

/**
 * Setting currentuser's mailgroup lists.
 *
 * @authur flyriver
 */
int set_mailgroup_list()
{
    struct _select_def grouplist_conf;
    mailgroup_list_arg *arg;
    POINT *pts;
    int i;
    int oldmode;

    clear();
    arg = (mailgroup_list_arg *) malloc(sizeof(mailgroup_list_arg));
    if (arg == NULL)
        return -1;
    oldmode = uinfo.mode;
    modify_user_mode(MAIL);
    arg->tmpnum = 0;
    pts = (POINT *) malloc(sizeof(POINT) * BBS_PAGESIZE);
    for (i = 0; i < BBS_PAGESIZE; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    bzero(&grouplist_conf, sizeof(struct _select_def));
    grouplist_conf.item_count = load_mailgroup_list(currentuser->userid, &(arg->mail_group));
    if (grouplist_conf.item_count == 0) {
        grouplist_conf.item_count = init_mailgroup_list(&(arg->mail_group));
        clear();
    }
    grouplist_conf.item_per_page = BBS_PAGESIZE;
    /*
     * ¼ÓÉÏ LF_VSCROLL ²ÅÄÜÓÃ LEFT ¼üÍË³ö 
     */
    grouplist_conf.flag = LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    grouplist_conf.prompt = "¡ô";
    grouplist_conf.item_pos = pts;
    grouplist_conf.arg = arg;
    grouplist_conf.title_pos.x = 0;
    grouplist_conf.title_pos.y = 0;
    grouplist_conf.pos = 1;     /* initialize cursor on the first mailgroup */
    grouplist_conf.page_pos = 1;        /* initialize page to the first one */

    grouplist_conf.on_select = set_mailgroup_list_select;
    grouplist_conf.show_data = set_mailgroup_list_show;
    grouplist_conf.pre_key_command = set_mailgroup_list_prekey;
    grouplist_conf.key_command = set_mailgroup_list_key;
    grouplist_conf.show_title = set_mailgroup_list_refresh;
    grouplist_conf.get_data = set_mailgroup_list_getdata;

    list_select_loop(&grouplist_conf);
    store_mailgroup_list(currentuser->userid, &(arg->mail_group));
    free(arg);
    free(pts);
    modify_user_mode(oldmode);

    return 0;
}
