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

    checked global variable
*/

#include "bbs.h"

static void mail_info()
{
    FILE *fn;
    time_t now;
    char filename[STRLEN];

    now = time(0);
    sprintf(filename, "etc/%s.tmp", currentuser->userid);
    fn = fopen(filename, "w");
    fprintf(fn, "[1m%s[m ÒÑ¾­ÔÚ [1m%24.24s[m ×ÔÉ±ÁË£¬ÒÔÏÂÊÇËû(Ëı)µÄ×ÊÁÏ£¬Çë±£Áô...", currentuser->userid, ctime(&now));
    getuinfo(fn, currentuser);
    fprintf(fn, "\n                      [1m ÏµÍ³×Ô¶¯·¢ĞÅÏµÍ³Áô[m\n");
    fclose(fn);
    mail_file(currentuser->userid, filename, "acmanager", "×ÔÉ±Í¨Öª....", BBSPOST_MOVE, NULL);
}


int d_board()
{
    char bname[STRLEN];
    char title[STRLEN];

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    clear();
    stand_title("É¾³ıÌÖÂÛÇø");
    make_blist();
    move(1, 0);
    namecomplete("ÇëÊäÈëÌÖÂÛÇø: ", genbuf);
    if (genbuf[0] == '\0')
        return 0;
    strcpy(bname, genbuf);
    if (delete_board(bname, title) != 0)
        return 0;
    if (seek_in_file("0Announce/.Search", bname)) {
#ifdef BBSMAIN
        getdata(3, 0, "ÒÆ³ı¾«»ªÇø (Yes, or No) [Y]: ", genbuf, 4, DOECHO, NULL, true);
        if (genbuf[0] != 'N' && genbuf[0] != 'n') {
#endif
            char lookgrp[30];

            del_grp(bname, title + 13);
        }
    }
    if (!bname[0]) {
        if (anonymousboard(bname))
            del_from_file("etc/anonymous", bname);
        sprintf(genbuf, "deleted board %s", bname);
        bbslog("user","%s",genbuf);
        /*
           sprintf(genbuf,"/bin/rm -fr boards/%s",bname) ;
           sprintf(genbuf,"/bin/rm -fr vote/%s",bname) ;
         */
        sprintf(genbuf, "boards/%s", bname);
        f_rm(bname);
        sprintf(genbuf, "vote/%s", bname);
        f_rm(bname);
    }

    move(4, 0);
    prints("±¾ÌÖÂÛÇøÒÑ¾­É¾³ı...\n");
    pressreturn();
    clear();
    return 0;
}

void suicide()
{
    char buf[STRLEN];
    FILE *fn;
    time_t now;
    char filename[STRLEN];

    char XPERM[48];
    int oldXPERM;
    int num;

    modify_user_mode(OFFLINE);
#ifndef NINE_BUILD
    if (HAS_PERM(currentuser, PERM_SYSOP) || HAS_PERM(currentuser, PERM_BOARDS) || HAS_PERM(currentuser, PERM_OBOARDS) || HAS_PERM(currentuser, PERM_ACCOUNTS)
        || HAS_PERM(currentuser, PERM_ANNOUNCE)
        || HAS_PERM(currentuser, PERM_JURY) || HAS_PERM(currentuser, PERM_SUICIDE) || HAS_PERM(currentuser, PERM_CHATOP) || (!HAS_PERM(currentuser, PERM_POST))
        || HAS_PERM(currentuser, PERM_DENYMAIL)
        || HAS_PERM(currentuser, PERM_DENYRELAX)) {
        clear();
        move(11, 28);
        prints("[1m[33mÄãÓĞÖØÈÎÔÚÉí£¬²»ÄÜ×ÔÉ±£¡[m");
        pressanykey();
        return;
    }

    clear();
    move(1, 0);
    prints("Ñ¡Ôñ×ÔÉ±½«Ê¹ÄúµÄÉúÃüÁ¦¼õÉÙµ½14Ìì£¬14ÌìºóÄúµÄÕÊºÅ×Ô¶¯ÏûÊ§¡£");
    move(3, 0);
    prints("ÔÚÕâ14ÌìÄÚÈô¸Ä±äÖ÷ÒâµÄ»°£¬Ôò¿ÉÒÔÍ¨¹ıµÇÂ¼±¾Õ¾Ò»´Î»Ö¸´Ô­ÉúÃüÁ¦");
    move(5, 0);
    prints("×ÔÉ±ÓÃ»§½«¶ªÊ§ËùÓĞ[33mÌØÊâÈ¨ÏŞ[m£¡£¡£¡");
    move(7, 0);
    /*
       clear();
       move(1,0);
       prints("Ñ¡Ôñ×ÔÉ±½«Ê¹ÄúµÄÕÊºÅ×Ô¶¯Á¢¼´ÏûÊ§!");
       move(3,0);
       prints("ÄúµÄÕÊºÅ½«ÂíÉÏ´ÓÏµÍ³ÖĞÉ¾³ı");
     */

    if (askyn("ÄãÈ·¶¨Òª×ÔÉ±Âğ£¿", 0) == 1) {
        clear();
        getdata(0, 0, "ÇëÊäÈëÔ­ÃÜÂë(ÊäÈëÕıÈ·µÄ»°»áÁ¢¿Ì¶ÏÏß): ", buf, 39, NOECHO, NULL, true);   /*Haohmaru,98.10.12,check the passwds */
        if (*buf == '\0' || !checkpasswd2(buf, currentuser)) {
            prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
            pressanykey();
            return;
        }

        oldXPERM = currentuser->userlevel;
        strcpy(XPERM, XPERMSTR);
        for (num = 0; num < (int) strlen(XPERM); num++)
            if (!(oldXPERM & (1 << num)))
                XPERM[num] = ' ';
        XPERM[num] = '\0';
        currentuser->userlevel &= 0x3F; /*Haohmaru,99.3.20.×ÔÉ±ÕßÖ»±£Áô»ù±¾È¨ÏŞ */
        currentuser->userlevel ^= PERM_SUICIDE;

        /*Haohmaru.99.3.20.×ÔÉ±Í¨Öª */
        now = time(0);
        sprintf(filename, "etc/%s.tmp", currentuser->userid);
        fn = fopen(filename, "w");
        fprintf(fn, "[1m%s[m ÒÑ¾­ÔÚ [1m%24.24s[m ×ÔÉ±ÁË£¬ÒÔÏÂÊÇËûµÄ×ÊÁÏ£¬Çë±£Áô...", currentuser->userid, ctime(&now));
        fprintf(fn, "\n\nÒÔÏÂÊÇ×ÔÉ±ÕßÔ­À´µÄÈ¨ÏŞ\n\033[1m\033[33m%s\n[m", XPERM);
        getuinfo(fn, currentuser);
        fprintf(fn, "\n                      [1m ÏµÍ³×Ô¶¯·¢ĞÅÏµÍ³Áô[m\n");
        fclose(fn);
        sprintf(buf, "%s µÄ×ÔÉ±Í¨Öª", currentuser->userid);
        post_file(currentuser, "", filename, "Goodbye", buf, 0, 1);
        unlink(filename);

        /*kick_user(&uinfo);
           exit(0); */
        abort_bbs(0);
    }
#else
    if (HAS_PERM(currentuser, PERM_SYSOP) || HAS_PERM(currentuser, PERM_BOARDS)) {
        clear();
        move(11, 28);
        prints("[1m[33mÄãÓĞÖØÈÎÔÚÉí£¬²»ÄÜ×ÔÉ±£¡[m");
        pressanykey();
        return;
    }

    clear();
    move(1, 0);
    prints("Ò»µ©×ÔÉ±£¬¾ÍÎŞ·¨Íì»Ø");
    move(3, 0);
    prints("ÕæµÄÏëÒªÒ»ËÀÁËÖ®Âğ? ");
    move(5, 0);

    if (askyn("ÄãÈ·¶¨Òª×ÔÉ±Âğ£¿", 0) == 1) {
        char buf2[STRLEN], tmpbuf[PATHLEN], genbuf[PATHLEN];
	 int id;
        clear();
	 getdata(0, 0, "ÇëÊäÈëÒ»¾ä¼ò¶ÌµÄ×ÔÉ±ÁôÑÔ: ", buf2, 75, DOECHO, NULL, true);
        getdata(0, 0, "ÇëÊäÈëÔ­ÃÜÂë(ÊäÈëÕıÈ·µÄ»°»áÁ¢¿Ì¶ÏÏß²¢ÇÒÎŞ·¨Íì»Ø): ", buf, 39, NOECHO, NULL, true);   /*Haohmaru,98.10.12,check the passwds */
        if (*buf == '\0' || !checkpasswd2(buf, currentuser)) {
            prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
            pressanykey();
            return;
        }

        now = time(0);
        sprintf(filename, "etc/%s.tmp", currentuser->userid);
        fn = fopen(filename, "w");
        fprintf(fn, "´ó¼ÒºÃ,\n\nÎÒÊÇ %s (%s)¡£ ÎÒÒÑ¾­Àë¿ªÕâÀïÁË¡£", currentuser->userid, currentuser->username);
        fprintf(fn, "\n\nÎÒ²»»á¸ü²»¿ÉÄÜÍü¼Ç×Ô %s", ctime(&(currentuser->firstlogin)));
        fprintf(fn, "ÒÔÀ´ÎÒÔÚ±¾Õ¾ %d ´Î login ÖĞ×Ü¹² %d ·ÖÖÓ¶ºÁôÆÚ¼äµÄµãµãµÎµÎ¡£", currentuser->numlogins, currentuser->stay/60);
        fprintf(fn, "\nÇëÎÒµÄºÃÓÑ°Ñ %s ´ÓÄãÃÇµÄºÃÓÑÃûµ¥ÖĞÄÃµô°É¡£", currentuser->userid);
        fprintf(fn, "\n\n»òĞíÓĞ³¯Ò»ÈÕÎÒ»á»ØÀ´µÄ¡£ ÕäÖØ!! ÔÙ¼û!!");
        fprintf(fn, "\n\n×ÔÉ±ÕßµÄ¼ò¶ÌÁôÑÔ: %s", buf2);
        fclose(fn);
        sprintf(buf, "%s µÄ×ÔÉ±Í¨Öª", currentuser->userid);
        post_file(currentuser, "", filename, "Goodbye", buf, 0, 2);
        unlink(filename);

    setmailpath(tmpbuf, currentuser->userid);
    f_rm(tmpbuf);
    sethomepath(tmpbuf, currentuser->userid);
    f_rm(tmpbuf);
    sprintf(genbuf, "tmp/email/%s", currentuser->userid);
    f_rm(genbuf);
    currentuser->userlevel = 0;
    id = searchuser(currentuser->userid);
    setuserid(id, "");
    /*strcpy(lookupuser->address, "");*/
    strcpy(currentuser->username, "");
    /*strcpy(lookupuser->realname, "");*/
	/*read_userdata(lookupuser->userid, &ud);
	strcpy(ud.address, "");
	strcpy(ud.realname, "");
	write_userdata(lookupuser->userid, &ud);*/
/*    lookupuser->userid[0] = '\0' ; */
        abort_bbs(0);
    }

#endif
}

int giveupnet()
{                               /* bad 2002.7.5 */
/*
PERM_BASIC   ÉÏÕ¾
PERM_POST    ·¢±í
PERM_CHAT    ÁÄÌì
PERM_PAGE    ºô½Ğ
PERM_DENYMAIL·¢ĞÅ
PERM_DENYRELAXÓéÀÖ
*/
    char buf[STRLEN], genbuf[PATHLEN];
    FILE *fn;
    char ans[3], day[10];
    int i, j, k, lcount, tcount;

    modify_user_mode(GIVEUPNET);
    if (!HAS_PERM(currentuser, PERM_LOGINOK)) {
        clear();
        move(11, 28);
        prints("[1m[33mÄãÓĞ»¹Ã»ÓĞ×¢²áÍ¨¹ı£¬²»ÄÜ½äÍø£¡[m");
        pressanykey();
        return;
    }

    if (HAS_PERM(currentuser, PERM_SYSOP) || HAS_PERM(currentuser, PERM_BOARDS) || HAS_PERM(currentuser, PERM_OBOARDS) || HAS_PERM(currentuser, PERM_ACCOUNTS)
        || HAS_PERM(currentuser, PERM_ANNOUNCE)
        || HAS_PERM(currentuser, PERM_JURY) || HAS_PERM(currentuser, PERM_SUICIDE) || HAS_PERM(currentuser, PERM_CHATOP)) {
        clear();
        move(11, 28);
        prints("[1m[33mÄãÓĞÖØÈÎÔÚÉí£¬²»ÄÜ½äÍø£¡[m");
        pressanykey();
        return;
    }

    lcount = 0;
    tcount = 0;
    sethomefile(genbuf, currentuser->userid, "giveup");
    fn = fopen(genbuf, "rt");
    if (fn) {
        clear();
        move(1, 0);
        prints("ÄãÏÖÔÚµÄ½äÍøÇé¿ö£º\n\n");
        while (!feof(fn)) {
            if (fscanf(fn, "%d %d", &i, &j) <= 0)
                break;
            switch (i) {
            case 1:
                prints("ÉÏÕ¾È¨ÏŞ");
                break;
            case 2:
                prints("·¢±íÈ¨ÏŞ");
                break;
            case 3:
                prints("ÁÄÌìÈ¨ÏŞ");
                break;
            case 4:
                prints("ºô½ĞÈ¨ÏŞ");
                break;
            case 5:
                prints("·¢ĞÅÈ¨ÏŞ");
                break;
            case 6:
                prints("ĞİÏĞÓéÀÖÈ¨ÏŞ");
                break;
            }
            sprintf(buf, "        »¹ÓĞ%dÌì\n", j - time(0) / 3600 / 24);
            prints(buf);
            lcount++;
        }
        fclose(fn);
        pressanykey();
    }


    clear();
    move(1, 0);
    prints("ÇëÑ¡Ôñ½äÍøÖÖÀà:");
    move(3, 0);
    prints("(0) - ½áÊø");
    move(4, 0);
    prints("(1) - ÉÏÕ¾È¨ÏŞ");
    move(5, 0);
    prints("(2) - ·¢±íÈ¨ÏŞ");
    move(6, 0);
    prints("(3) - ÁÄÌìÊÒÈ¨ÏŞ");
    move(7, 0);
    prints("(4) - ·¢ËÍÏûÏ¢ºÍºô½ĞÁÄÌìÈ¨ÏŞ");
    move(8, 0);
    prints("(5) - ·¢ĞÅÈ¨ÏŞ");
    move(9, 0);
    prints("(6) - ĞİÏĞÓéÀÖÈ¨ÏŞ");

    getdata(12, 0, "ÇëÑ¡Ôñ [0]", ans, 2, DOECHO, NULL, true);
    if (ans[0] < '1' || ans[0] > '6') {
        return;
    }
    k = 1;
    switch (ans[0]) {
    case '1':
        k = k && (currentuser->userlevel & PERM_BASIC);
        break;
    case '2':
        k = k && (currentuser->userlevel & PERM_POST);
        break;
    case '3':
        k = k && (currentuser->userlevel & PERM_CHAT);
        break;
    case '4':
        k = k && (currentuser->userlevel & PERM_PAGE);
        break;
    case '5':
        k = k && !(currentuser->userlevel & PERM_DENYMAIL);
        break;
    case '6':
        k = k && !(currentuser->userlevel & PERM_DENYRELAX);
        break;
    }

    if (!k) {
        prints("\n\nÄãÒÑ¾­Ã»ÓĞÁË¸ÃÈ¨ÏŞ");
        pressanykey();
        return;
    }

    getdata(11, 0, "ÇëÊäÈë½äÍøÌìÊı [0]", day, 4, DOECHO, NULL, true);
    i = 0;
    while (day[i]) {
        if (!isdigit(day[i]))
            return;
        i++;
    }
    j = atoi(day);
    if (j <= 0)
        return;

    if (compute_user_value(currentuser) <= j) {
        prints("\n\n¶Ô²»Æğ£¬ÌìÊı²»¿ÉÒÔ´óÓÚÉúÃüÁ¦...");
        pressanykey();
        return;
    }
    j = time(0) / 3600 / 24 + j;

    move(13, 0);

    if (askyn("ÄãÈ·¶¨Òª½äÍøÂğ\x1b[1;31m(×¢Òâ:½äÍøÖ®ºó²»ÄÜÌáÇ°½â¿ª!\x1b[m)£¿", 0) == 1) {
        getdata(15, 0, "ÇëÊäÈëÃÜÂë: ", buf, 39, NOECHO, NULL, true);
        if (*buf == '\0' || !checkpasswd2(buf, currentuser)) {
            prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
            pressanykey();
            return;
        }

        sethomefile(genbuf, currentuser->userid, "giveup");
        fn = fopen(genbuf, "at");
        if (!fn) {
            prints("\n\nÓÉÓÚÏµÍ³ÎÊÌâ£¬ÏÖÔÚÄã²»ÄÜ½äÍø");
            pressanykey();
            return;
        }
        fprintf(fn, "%d %d\n", ans[0] - 48, j);
        fclose(fn);

        switch (ans[0]) {
        case '1':
            currentuser->userlevel &= ~PERM_BASIC;
            break;
        case '2':
            currentuser->userlevel &= ~PERM_POST;
            break;
        case '3':
            currentuser->userlevel &= ~PERM_CHAT;
            break;
        case '4':
            currentuser->userlevel &= ~PERM_PAGE;
            break;
        case '5':
            currentuser->userlevel |= PERM_DENYMAIL;
            break;
        case '6':
            currentuser->userlevel |= PERM_DENYRELAX;
            break;
        }
        lcount++;

        if (currentuser->userlevel & PERM_BASIC)
            tcount++;
        if (currentuser->userlevel & PERM_POST)
            tcount++;
        if (currentuser->userlevel & PERM_CHAT)
            tcount++;
        if (currentuser->userlevel & PERM_PAGE)
            tcount++;
        if (!(currentuser->userlevel & PERM_DENYMAIL))
            tcount++;
        if (!(currentuser->userlevel & PERM_DENYRELAX))
            tcount++;

        if (lcount + tcount == 6)
            currentuser->flags |= GIVEUP_FLAG;
        else
            currentuser->flags &= ~GIVEUP_FLAG;

        prints("\n\nÄãÒÑ¾­¿ªÊ¼½äÍøÁË");
        pressanykey();
        if (ans[0] == '1')
            abort_bbs(0);
    }
}


void offline()
{
    modify_user_mode(OFFLINE);

    if (HAS_PERM(currentuser, PERM_SYSOP))
        return;
    clear();
    move(1, 0);
    prints("[32mºÃÄÑ¹ıà¸.....[m");
    move(3, 0);
    if (askyn("ÄãÈ·¶¨ÒªÀë¿ªÕâ¸ö´ó¼ÒÍ¥", 0) == 1) {
        clear();
        if (d_user(currentuser->userid) == 1) {
            mail_info();
            kick_user(&uinfo);
            exit(0);
        }
    }
}

int kickuser(struct user_info *uentp, char *arg, int count)
{
    kill(uentp->pid, SIGHUP);
    clear_utmp((uentp - utmpshm->uinfo) + 1, uentp->uid, uentp->pid);
    UNUSED_ARG(arg);
    UNUSED_ARG(count);
    return 0;
}

int d_user(cid)
    char cid[IDLEN];
{
    int id, fd;
    char tmpbuf[30];
    char userid[IDLEN + 2];
    struct userec *lookupuser;
       /* Ôö¼ÓÏÔÊ¾ÓÃ»§ĞÅÏ¢ Bigman:2003.5.11*/
        struct userec uinfo1;
	/*struct userdata ud;*/

    if (uinfo.mode != OFFLINE) {
        modify_user_mode(ADMIN);
        if (!check_systempasswd()) {
            return 0;
        }
        clear();
        stand_title("É¾³ıÊ¹ÓÃÕßÕÊºÅ");
        move(1, 0);
        usercomplete("ÇëÊäÈëÓûÉ¾³ıµÄÊ¹ÓÃÕß´úºÅ: ", userid);
        if (userid[0] == '\0') {
            clear();
            return 0;
        }
    } else
        strcpy(userid, cid);
    if (!(id = getuser(userid, &lookupuser))) {
        move(3, 0);
        prints(MSG_ERR_USERID);
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
     /* Ôö¼ÓÏÔÊ¾ÓÃ»§ĞÅÏ¢ Bigman:2003.5.11*/
    uinfo1 = *lookupuser;
    clrtobot();

    disply_userinfo(&uinfo1, 1);

    /*    if (!isalpha(lookupuser->userid[0])) return 0; */
    /* rrr - don't know how... */
    move(22, 0);
    if (uinfo.mode != OFFLINE)
        prints("É¾³ıÊ¹ÓÃÕß '%s'.", userid);
    else
        prints(" %s ½«Àë¿ªÕâÀï", cid);
/*    clrtoeol(); */
    
    getdata(24, 0, "(Yes, or No) [No](×¢ÒâÒªÊäÈëÈ«µ¥´ÊºÍ´óĞ¡Ğ´): ", genbuf, 5, DOECHO, NULL, true);
    if (strcmp(genbuf,"Yes")) { /* if not yes quit */
        move(24, 0);
        if (uinfo.mode != OFFLINE)
            prints("È¡ÏûÉ¾³ıÊ¹ÓÃÕß...\n");
        else
            prints("ÄãÖÕÓÚ»ØĞÄ×ªÒâÁË£¬ºÃ¸ßĞËà¸...");
        pressreturn();
        clear();
        return 0;
    }
    if (uinfo.mode != OFFLINE) {
        char secu[STRLEN];

        sprintf(secu, "É¾³ıÊ¹ÓÃÕß£º%s", lookupuser->userid);
        securityreport(secu, lookupuser, NULL);
    }
    sprintf(genbuf, "%s deleted user %s", currentuser->userid, lookupuser->userid);
    bbslog("user","%s",genbuf);
    /*Haohmaru.99.12.23.±»É¾IDÒ»¸öÔÂÄÚ²»µÃ×¢²á */
    if ((fd = open(".badname", O_WRONLY | O_CREAT, 0644)) != -1) {
        char buf[STRLEN];
        char thtime[40];
        time_t dtime;

        dtime = time(0);
        sprintf(thtime, "%lu", dtime);
        flock(fd, LOCK_EX);
        lseek(fd, 0, SEEK_END);
        sprintf(buf, "%-12.12s %-66.66s\n", lookupuser->userid, thtime);
        write(fd, buf, strlen(buf));
        flock(fd, LOCK_UN);
        close(fd);
    } else {
        printf("´íÎó£¬Çë±¨¸æSYSOP");
        pressanykey();
    }
    setmailpath(tmpbuf, lookupuser->userid);
    /*
       sprintf(genbuf,"/bin/rm -fr %s", tmpbuf) ;
     */
    f_rm(tmpbuf);
    sethomepath(tmpbuf, lookupuser->userid);
    /*
       sprintf(genbuf,"/bin/rm -fr %s", tmpbuf) ;
       sprintf(genbuf,"/bin/rm -fr tmp/email/%s", lookupuser->userid) ;
     */
    f_rm(tmpbuf);
    sprintf(genbuf, "tmp/email/%s", lookupuser->userid);
    f_rm(genbuf);
    apply_utmp((APPLY_UTMP_FUNC) kickuser, 0, userid, 0);
    setuserid(id, "");
    lookupuser->userlevel = 0;
    /*strcpy(lookupuser->address, "");*/
    strcpy(lookupuser->username, "");
    /*strcpy(lookupuser->realname, "");*/
	/*read_userdata(lookupuser->userid, &ud);
	strcpy(ud.address, "");
	strcpy(ud.realname, "");
	write_userdata(lookupuser->userid, &ud);*/
/*    lookupuser->userid[0] = '\0' ; */
    move(24, 0);
    prints("%s ÒÑ¾­ÒÑ¾­ºÍ±¾¼ÒÍ¥Ê§È¥ÁªÂç....\n", userid);
    pressreturn();

    clear();
    return 1;
}

/* to be Continue fix kick user problem */
int kick_user(struct user_info *userinfo)
{
    int id, ind;
    struct user_info uin;
    char userid[40];

    if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE && uinfo.mode != FRIEND) {
        modify_user_mode(ADMIN);
        stand_title("Kick User");
        move(1, 0);
        usercomplete("Enter userid to be kicked: ", userid);
        if (*userid == '\0') {
            clear();
            return 0;
        }
        if (!(id = searchuser(userid))) {       /* change getuser -> searchuser, by dong, 1999.10.26 */
            move(3, 0);
            prints("Invalid User Id");
            clrtoeol();
            pressreturn();
            clear();
            return 0;
        }
        move(1, 0);
        prints("Kick User '%s'.", userid);
        clrtoeol();
        getdata(2, 0, "(Yes, or No) [N]: ", genbuf, 4, DOECHO, NULL, true);
        if (genbuf[0] != 'Y' && genbuf[0] != 'y') {     /* if not yes quit */
            move(2, 0);
            prints("Aborting Kick User\n");
            pressreturn();
            clear();
            return 0;
        }
        return apply_utmp((APPLY_UTMP_FUNC) kickuser, 0, userid, 0);
    } else {
        uin = *userinfo;
        strcpy(userid, uin.userid);
        ind = true;
    }
    if (uin.mode == WEBEXPLORE)
        clear_utmp((userinfo - utmpshm->uinfo) + 1, uin.uid, uin.pid);
    if (!ind || !uin.active || (kill(uin.pid, 0) == -1)) {
        if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE && uinfo.mode != FRIEND) {
            move(3, 0);
            prints("User Has Logged Out");
            clrtoeol();
            pressreturn();
            clear();
        }
        return 0;
    }
    if (kill(uin.pid, SIGHUP) == -1) {
        clear_utmp((userinfo - utmpshm->uinfo) + 1, uin.uid, uin.pid);
    }
    newbbslog(BBSLOG_USER, "kicked %s", userid);
    /*sprintf( genbuf, "%s (%s)", kuinfo.userid, kuinfo.username );modified by dong, 1998.11.2 */
    /*bbslog( "1system", "KICK %s (%s)", uin.userid, uin.username ); */
    /*    uin.active = false;
       uin.pid = 0;
       uin.invisible = true;
       uin.sockactive = 0;
       uin.sockaddr = 0;
       uin.destuid = 0;
       update_ulist( &uin, ind ); ÎŞÒâÒå¶øÇÒ²ÎÊıÓĞ´í£¬ËùÒÔ×¢ÊÍµô dong 1998.7.7 */
    move(2, 0);
    if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE && uinfo.mode != FRIEND) {
        prints("User has been Kicked\n");
        pressreturn();
        clear();
    }
    return 1;
}
