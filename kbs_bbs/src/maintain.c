/*
 * Pirate Bulletin Board System Copyright (C) 1990, Edward Luke,
 * lush@Athena.EE.MsState.EDU Eagles Bulletin Board System Copyright (C)
 * 1992, Raymond Rocker, rocker@rock.b11.ingr.com Guy Vega,
 * gtvega@seabass.st.usm.edu Dominic Tynes, dbtynes@seabass.st.usm.edu
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 1, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675
 * Mass Ave, Cambridge, MA 02139, USA.
 */

#include "bbs.h"

char cexplain[STRLEN];
char *Ctime();
static int sysoppassed = 0;

/* modified by wwj, 2001/5/7, for new md5 passwd */
void igenpass(const char *passwd, const char *userid, unsigned char md5passwd[]);

int check_systempasswd()
{
    FILE *pass;
    char passbuf[40], prepass[STRLEN];

#ifdef NINE_BUILD
    return true;
#endif

    if ((sysoppassed) && (time(NULL) - sysoppassed < 60 * 60))
        return true;
    clear();
    if ((pass = fopen("etc/systempassword", "rb")) != NULL) {
        fgets(prepass, STRLEN, pass);
        prepass[strlen(prepass) - 1] = '\0';
        if (!strcmp(prepass, "md5")) {
            fread(&prepass[16], 1, 16, pass);
        }
        fclose(pass);

        getdata(1, 0, "ÇëÊäÈëÏµÍ³ÃÜÂë: ", passbuf, 39, NOECHO, NULL, true);
        if (passbuf[0] == '\0' || passbuf[0] == '\n')
            return false;


        if (!strcmp(prepass, "md5")) {
            igenpass(passbuf, "[system]", (unsigned char *) prepass);
            passbuf[0] = (char) !memcmp(prepass, &prepass[16], 16);
        } else {
            passbuf[0] = (char) checkpasswd(prepass, passbuf);
        }
        if (!passbuf[0]) {
            move(2, 0);
            prints("ÏµÍ³ÃÜÂëÊäÈë´íÎó...");
            securityreport("ÏµÍ³ÃÜÂëÊäÈë´íÎó...", NULL, NULL);
            pressanykey();
            return false;
        }
    }
    sysoppassed = time(NULL);
    return true;
}

int setsystempasswd()
{
    FILE *pass;
    char passbuf[40], prepass[40];

    modify_user_mode(ADMIN);
    if (strcmp(currentuser->userid, "SYSOP"))
        return -1;
    if (!check_systempasswd())
        return -1;
    getdata(2, 0, "ÇëÊäÈëÐÂµÄÏµÍ³ÃÜÂë: ", passbuf, 39, NOECHO, NULL, true);
    getdata(3, 0, "È·ÈÏÐÂµÄÏµÍ³ÃÜÂë: ", prepass, 39, NOECHO, NULL, true);
    if (strcmp(passbuf, prepass))
        return -1;
    if ((pass = fopen("etc/systempassword", "w")) == NULL) {
        move(4, 0);
        prints("ÏµÍ³ÃÜÂëÎÞ·¨Éè¶¨....");
        pressanykey();
        return -1;
    }
    fwrite("md5\n", 4, 1, pass);

    igenpass(passbuf, "[system]", (unsigned char *) prepass);
    fwrite(prepass, 16, 1, pass);

    fclose(pass);
    move(4, 0);
    prints("ÏµÍ³ÃÜÂëÉè¶¨Íê³É....");
    pressanykey();
    return 0;
}


void deliverreport(title, str)
char *title;
char *str;
{
    FILE *se;
    char fname[STRLEN];
    int savemode;

    savemode = uinfo.mode;
	gettmpfilename( fname, "deliver" );
    //sprintf(fname, "tmp/deliver.%s.%05d", currentuser->userid, uinfo.pid);
    if ((se = fopen(fname, "w")) != NULL) {
        fprintf(se, "%s\n", str);
        fclose(se);
        post_file(currentuser, "", fname, currboard->filename, title, 0, 2);
        unlink(fname);
        modify_user_mode(savemode);
    }
}


void securityreport(char *str, struct userec *lookupuser, char fdata[7][STRLEN])
{                               /* Leeward: 1997.12.02 */
    FILE *se;
    char fname[STRLEN];
    int savemode;
    char *ptr;

    savemode = uinfo.mode;
	gettmpfilename( fname, "security" );
    //sprintf(fname, "tmp/security.%d", getpid());
    if ((se = fopen(fname, "w")) != NULL) {
        if (lookupuser) {
            if (strstr(str, "ÈÃ") && strstr(str, "Í¨¹ýÉí·ÝÈ·ÈÏ")) {
                struct userdata ud;

                read_userdata(lookupuser->userid, &ud);
                fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\n[32mÔ­Òò£º%s[m\n", str);
                fprintf(se, "ÒÔÏÂÊÇÍ¨¹ýÕß¸öÈË×ÊÁÏ");
                /*
                 * getuinfo(se, lookupuser); 
                 */
                /*
                 * Haohmaru.99.4.15.°Ñ±»×¢²áµÄ×ÊÁÏÁÐµÃ¸üÏêÏ¸,Í¬Ê±È¥µô×¢²áÕßµÄ×ÊÁÏ 
                 */
                fprintf(se, "\n\nÄúµÄ´úºÅ     : %s\n", fdata[1]);
                fprintf(se, "ÄúµÄêÇ³Æ     : %s\n", lookupuser->username);
                fprintf(se, "ÕæÊµÐÕÃû     : %s\n", fdata[2]);
                fprintf(se, "µç×ÓÓÊ¼þÐÅÏä : %s\n", ud.email);
                fprintf(se, "ÕæÊµ E-mail  : %s$%s@%s\n", fdata[3], fdata[5], currentuser->userid);
                fprintf(se, "·þÎñµ¥Î»     : %s\n", fdata[3]);
                fprintf(se, "Ä¿Ç°×¡Ö·     : %s\n", fdata[4]);
                fprintf(se, "Á¬Âçµç»°     : %s\n", fdata[5]);
                fprintf(se, "×¢²áÈÕÆÚ     : %s", ctime(&lookupuser->firstlogin));
                fprintf(se, "×î½ü¹âÁÙÈÕÆÚ : %s", ctime(&lookupuser->lastlogin));
                fprintf(se, "×î½ü¹âÁÙ»úÆ÷ : %s\n", lookupuser->lasthost);
                fprintf(se, "ÉÏÕ¾´ÎÊý     : %d ´Î\n", lookupuser->numlogins);
                fprintf(se, "ÎÄÕÂÊýÄ¿     : %d(Board)\n", lookupuser->numposts);
                fprintf(se, "Éú    ÈÕ     : %s\n", fdata[6]);
                /*
                 * fprintf(se, "\n[33mÒÔÏÂÊÇÈÏÖ¤Õß¸öÈË×ÊÁÏ[35m");
                 * getuinfo(se, currentuser);rem by Haohmaru.99.4.16 
                 */
                fclose(se);
                post_file(currentuser, "", fname, "Registry", str, 0, 2);
            } else if (strstr(str, "É¾³ýÊ¹ÓÃÕß£º")) {
                fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\n[32mÔ­Òò£º%s[m\n", str);
                fprintf(se, "ÒÔÏÂÊÇ±»É¾Õß¸öÈË×ÊÁÏ");
                getuinfo(se, lookupuser);
                fprintf(se, "\nÒÔÏÂÊÇÉ¾³ýÕß¸öÈË×ÊÁÏ");
                getuinfo(se, currentuser);
                fclose(se);
                post_file(currentuser, "", fname, "syssecurity", str, 0, 2);
            } else if ((ptr = strstr(str, "µÄÈ¨ÏÞXPERM")) != NULL) {
                int oldXPERM, newXPERM;
                int num;
                char XPERM[48];

                sscanf(ptr + strlen("µÄÈ¨ÏÞXPERM"), "%d %d", &oldXPERM, &newXPERM);
                *(ptr + strlen("µÄÈ¨ÏÞ")) = 0;

                fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\n[32mÔ­Òò£º%s[m\n", str);

                strcpy(XPERM, XPERMSTR);
                for (num = 0; num < (int) strlen(XPERM); num++)
                    if (!(oldXPERM & (1 << num)))
                        XPERM[num] = ' ';
                XPERM[num] = '\0';
                fprintf(se, "ÒÔÏÂÊÇ±»¸ÄÕßÔ­À´µÄÈ¨ÏÞ\n\033[1m\033[33m%s", XPERM);

                strcpy(XPERM, XPERMSTR);
                for (num = 0; num < (int) strlen(XPERM); num++)
                    if (!(newXPERM & (1 << num)))
                        XPERM[num] = ' ';
                XPERM[num] = '\0';
                fprintf(se, "\n%s\033[m\nÒÔÉÏÊÇ±»¸ÄÕßÏÖÔÚµÄÈ¨ÏÞ\n", XPERM);

                fprintf(se, "\n"
                        "\033[1m\033[33mb\033[m»ù±¾È¨Á¦ \033[1m\033[33mT\033[m½øÁÄÌìÊÒ \033[1m\033[33mC\033[mºô½ÐÁÄÌì \033[1m\033[33mP\033[m·¢ÎÄÕÂ \033[1m\033[33mR\033[m×ÊÁÏÕýÈ· \033[1m\033[33mp\033[mÊµÏ°Õ¾Îñ \033[1m\033[33m#\033[m¿ÉÒþÉí \033[1m\033[33m@\033[m¿É¼ûÒþÉí\n"
                        "\033[1m\033[33mX\033[m³¤ÆÚÕÊºÅ \033[1m\033[33mW\033[m±à¼­ÏµÍ³µµ°¸ \033[1m\033[33mB\033[m°æÖ÷ \033[1m\033[33mA\033[mÕÊºÅ¹ÜÀí \033[1m\033[33m$\033[mÖÇÄÒÍÅ \033[1m\033[33mV\033[m·â½ûÓéÀÖ \033[1m\033[33mS\033[mÏµÍ³Î¬»¤\n"
                        "\033[1m\033[33m!\033[mRead/PostÏÞÖÆ \033[1m\033[33mD\033[m¾«»ªÇø×Ü¹Ü \033[1m\033[33mE\033[mÌÖÂÛÇø×Ü¹Ü \033[1m\033[33mM\033[m»î¶¯¿´°æ×Ü¹Ü \033[1m\033[33m1\033[m²»ÄÜZAP \033[1m\033[33m2\033[mÁÄÌìÊÒOP\n"
                        "\033[1m\033[33m3\033[mÏµÍ³×Ü¹ÜÀíÔ± \033[1m\033[33m4\033[mÈÙÓþÕÊºÅ \033[1m\033[33m5 7\033[m ÌØÊâÈ¨ÏÞ \033[1m\033[33m6\033[mÖÙ²Ã \033[1m\033[33m8\033[m×ÔÉ± \033[1m\033[33m9\033[m¼¯ÌåÕÊºÅ \033[1m\033[33m0\033[m¿´ÏµÍ³ÌÖÂÛ°æ\n"
			"\033[1m\033[33m%%\033[m·â½ûMail"
                        "\n");

                fprintf(se, "\nÒÔÏÂÊÇ±»¸ÄÕß¸öÈË×ÊÁÏ");
                getuinfo(se, lookupuser);
                fprintf(se, "\nÒÔÏÂÊÇÐÞ¸ÄÕß¸öÈË×ÊÁÏ");
                getuinfo(se, currentuser);
                fclose(se);
                post_file(currentuser, "", fname, "syssecurity", str, 0, 2);
            } else {            /* Modified for change id by Bigman 2001.5.25 */

                fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\0x1b[32mÔ­Òò£º%s\x1b[m\n", str);
                fprintf(se, "ÒÔÏÂÊÇ¸öÈË×ÊÁÏ");
                getuinfo(se, lookupuser);
                fclose(se);
                post_file(currentuser, "", fname, "syssecurity", str, 0, 2);
            }
        } else {
            fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\n[32mÔ­Òò£º%s[m\n", str);
            fprintf(se, "ÒÔÏÂÊÇ¸öÈË×ÊÁÏ");
            getuinfo(se, currentuser);
            fclose(se);
            if (strstr(str, "Éè¶¨Ê¹ÓÃÕß×¢²á×ÊÁÏ"))      /* Leeward 98.03.29 */
                post_file(currentuser, "", fname, "Registry", str, 0, 2);
            else
                post_file(currentuser, "", fname, "syssecurity", str, 0, 2);
        }
        unlink(fname);
        modify_user_mode(savemode);
    }
}

void stand_title(title)
char *title;
{
    clear();
    prints("\x1b[7m%s\x1b[m", title);
}

int m_info()
{
    struct userec uinfo;
    int id;
    struct userec *lookupuser;


    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {        /* Haohmaru.98.12.19 */
        return -1;
    }
    clear();
    stand_title("ÐÞ¸ÄÊ¹ÓÃÕß´úºÅ");
    move(1, 0);
    usercomplete("ÇëÊäÈëÊ¹ÓÃÕß´úºÅ: ", genbuf);
    if (*genbuf == '\0') {
        clear();
        return -1;
    }
    if (!(id = getuser(genbuf, &lookupuser))) {
        move(3, 0);
        prints(MSG_ERR_USERID);
        clrtoeol();
        pressreturn();
        clear();
        return -1;
    }
    uinfo = *lookupuser;

    move(1, 0);
    clrtobot();
    disply_userinfo(&uinfo, 1);
    uinfo_query(&uinfo, 1, id);
    return 0;
}

extern int cmpbnames();

char *chgrp()
{
    int i, ch;
    char buf[STRLEN], ans[6];

    clear();
    move(2, 0);
    prints("Ñ¡Ôñ¾«»ªÇøµÄÄ¿Â¼\n");
    oflush();

    for (i = 0;; i++) {
        if (secname[i][0] == NULL || groups[i] == NULL)
            break;
        prints("[32m%2d[m. %-20s%-20s\n", i, secname[i][0], groups[i]);
    }
    sprintf(buf, "ÇëÊäÈëÄãµÄÑ¡Ôñ(0~%d): ", i - 1);
    while (1) {
        getdata(i + 3, 0, buf, ans, 4, DOECHO, NULL, true);
        if (!isdigit(ans[0]))
            continue;
        ch = atoi(ans);
        if (ch < 0 || ch >= i || ans[0] == '\r' || ans[0] == '\0')
            continue;
        else
            break;
    }
    sprintf(cexplain, "%s", secname[ch][0]);

    return groups[ch];
}


int m_newbrd()
{
    struct boardheader newboard;
    char ans[5];
    char vbuf[100];
    char *group;


    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    clear();
    memset(&newboard, 0, sizeof(newboard));
    prints("¿ªÆôÐÂÌÖÂÛÇø:");
    while (1) {
        getdata(3, 0, "ÌÖÂÛÇøÃû³Æ:   ", newboard.filename, BOARDNAMELEN, DOECHO, NULL, true);
        if (newboard.filename[0] == '\0')
            return -1;
        if (valid_brdname(newboard.filename))
            break;
        prints("²»ºÏ·¨Ãû³Æ...");
    }
    getdata(4, 0, "ÌÖÂÛÇøËµÃ÷:   ", newboard.title, 60, DOECHO, NULL, true);
    strcpy(vbuf, "vote/");
    strcat(vbuf, newboard.filename);
    setbpath(genbuf, newboard.filename);
    if (getbnum(newboard.filename) > 0 || mkdir(genbuf, 0755) == -1 || mkdir(vbuf, 0755) == -1) {
        prints("\n´íÎó£º´íÎóµÄÌÖÂÛÇøÃû³Æ\n");
        pressreturn();
        clear();
        return -1;
    }
    newboard.flag = 0;
    getdata(5, 0, "ÌÖÂÛÇø¹ÜÀíÔ±: ", newboard.BM, BM_LEN - 1, DOECHO, NULL, true);
    getdata(6, 0, "ÊÇ·ñÏÞÖÆ´æÈ¡È¨Á¦ (Y/N)? [N]: ", ans, 4, DOECHO, NULL, true);
    if (*ans == 'y' || *ans == 'Y') {
        getdata(6, 0, "ÏÞÖÆ Read/Post? [R]: ", ans, 4, DOECHO, NULL, true);
        if (*ans == 'P' || *ans == 'p')
            newboard.level = PERM_POSTMASK;
        else
            newboard.level = 0;
        move(1, 0);
        clrtobot();
        move(2, 0);
        prints("Éè¶¨ %s È¨Á¦. ÌÖÂÛÇø: '%s'\n", (newboard.level & PERM_POSTMASK ? "POST" : "READ"), newboard.filename);
        newboard.level = setperms(newboard.level, 0, "È¨ÏÞ", NUMPERMS, showperminfo, NULL);
        clear();
    } else
        newboard.level = 0;
    getdata(7, 0, "ÊÇ·ñ¼ÓÈëÄäÃû°æ (Y/N)? [N]: ", ans, 4, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y') {
        newboard.flag |= BOARD_ANNONY;
        addtofile("etc/anonymous", newboard.filename);
    }
    getdata(8, 0, "ÊÇ·ñ²»¼ÇÎÄÕÂÊý(Y/N)? [N]: ", ans, 4, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y')
        newboard.flag |= BOARD_JUNK;
    getdata(9, 0, "ÊÇ·ñ¿ÉÏòÍâ×ªÐÅ(Y/N)? [N]: ", ans, 4, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y')
        newboard.flag |= BOARD_OUTFLAG;
    build_board_structure(newboard.filename);
    group = chgrp();
    if (group != NULL) {
        if (newboard.BM[0] != '\0')
            if (strlen(newboard.BM) <= 30)
                sprintf(vbuf, "%-38.38s(BM: %s)", newboard.title + 13, newboard.BM);
            else
                snprintf(vbuf, STRLEN, "%-28.28s(BM: %s)", newboard.title + 13, newboard.BM);
        else
            sprintf(vbuf, "%-38.38s", newboard.title + 13);

        if (add_grp(group, newboard.filename, vbuf, cexplain) == -1)
            prints("\n³ÉÁ¢¾«»ªÇøÊ§°Ü....\n");
        else
            prints("ÒÑ¾­ÖÃÈë¾«»ªÇø...\n");
        snprintf(newboard.ann_path,127,"%s/%s",group, newboard.filename);
        newboard.ann_path[127]=0;
    }
    if (add_board(&newboard) == -1) {
        move(t_lines - 1, 0);
        outs("¼ÓÈëÌÖÂÛÇøÊ§°Ü!\n");
        pressreturn();
        clear();
        return -1;
    }
    prints("\nÐÂÌÖÂÛÇø³ÉÁ¢\n");
    sprintf(genbuf, "add brd %s", newboard.filename);
    bbslog("user", "%s", genbuf);
    {
        char secu[STRLEN];

        sprintf(secu, "³ÉÁ¢ÐÂ°æ£º%s", newboard.filename);
        securityreport(secu, NULL, NULL);
    }
    pressreturn();
    clear();
    return 0;
}

int m_editbrd()
{
    char bname[STRLEN], buf[STRLEN], oldtitle[STRLEN], vbuf[256], *group;
    char oldpath[STRLEN], newpath[STRLEN];
    int pos, noidboard, a_mv;
    struct boardheader fh, newfh;
    int line;

    struct boardheader* bh=NULL;
    char* groupname="";

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    clear();
    stand_title("ÐÞ¸ÄÌÖÂÛÇø×ÊÑ¶");
    move(1, 0);
    make_blist();
    namecomplete("ÊäÈëÌÖÂÛÇøÃû³Æ: ", bname);
    if (*bname == '\0') {
        move(2, 0);
        prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
        pressreturn();
        clear();
        return -1;
    }
    pos = getboardnum(bname, &fh);
    if (!pos) {
        move(2, 0);
        prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
        pressreturn();
        clear();
        return -1;
    }
    noidboard = anonymousboard(bname);
    move(2, 0);
    memcpy(&newfh, &fh, sizeof(newfh));
    prints("ÌÖÂÛÇøÃû³Æ:   %s\n", fh.filename);
    prints("ÌÖÂÛÇøËµÃ÷:   %s\n", fh.title);
    prints("ÌÖÂÛÇø¹ÜÀíÔ±: %s\n", fh.BM);
    prints("ÄäÃûÌÖÂÛÇø:   %s ²»¼ÇÎÄÕÂÊý:   %s ÊÇ·ñÊÇÄ¿Â¼£º %s\n", 
        (noidboard) ? "Yes" : "No", (fh.flag & BOARD_JUNK) ? "Yes" : "No", (fh.flag & BOARD_GROUP) ? "Yes" : "No");
    if (newfh.group) {
        bh=getboard(newfh.group);
        if (bh) groupname=bh->filename;
    }
    prints("ËùÊôÄ¿Â¼£º%s\n",bh?groupname:"ÎÞ");
    prints("¿ÉÏòÍâ×ªÐÅ:   %s    ¿ÉÕ³Ìù¸½¼þ: %s    ÔÊÐí Email ·¢ÎÄ: %s\n", 
			(fh.flag & BOARD_OUTFLAG) ? "Yes" : "No",
			(fh.flag & BOARD_ATTACH) ? "Yes" : "No",
			(fh.flag & BOARD_EMAILPOST) ? "Yes" : "No");
    if (fh.flag & BOARD_CLUB_READ || fh.flag & BOARD_CLUB_WRITE)
        prints("¾ãÀÖ²¿:   %s %s %s  ÐòºÅ: %d\n", fh.flag & BOARD_CLUB_READ ? "ÔÄ¶ÁÏÞÖÆ" : "", fh.flag & BOARD_CLUB_WRITE ? "·¢±íÏÞÖÆ" : "", fh.flag & BOARD_CLUB_HIDE ? "Òþ²Ø" : "", fh.clubnum);
    else
        prints("%s", "¾ãÀÖ²¿:   ÎÞ\n");
    strcpy(oldtitle, fh.title);
    prints("ÏÞÖÆ %s È¨Á¦: %s      ÐèÒªµÄÓÃ»§Ö°Îñ: %s(%d)", 
        (fh.level & PERM_POSTMASK) ? "POST" : "READ", 
        (fh.level & ~PERM_POSTMASK) == 0 ? "²»ÉèÏÞ" : "ÓÐÉèÏÞ",
        fh.title_level? get_user_title(fh.title_level):"ÎÞ",fh.title_level);
    getdata(10, 0, "ÊÇ·ñ¸ü¸ÄÒÔÉÏ×ÊÑ¶? (Yes or No) [N]: ", genbuf, 4, DOECHO, NULL, true);
    if (*genbuf == 'y' || *genbuf == 'Y') {
        move(9, 0);
        prints("Ö±½Ó°´ <Return> ²»ÐÞ¸Ä´ËÀ¸×ÊÑ¶\n");
      enterbname:
        getdata(10, 0, "ÐÂÌÖÂÛÇøÃû³Æ: ", genbuf, BOARDNAMELEN, DOECHO, NULL, true);
        if (*genbuf != 0) {
            if (getboardnum(genbuf, NULL) > 0) {
                move(3, 0);
                prints("´íÎó! ´ËÌÖÂÛÇøÒÑ¾­´æÔÚ\n");
                move(11, 0);
                clrtobot();
                goto enterbname;
            }
            strncpy(newfh.filename, genbuf, sizeof(newfh.filename));
            strcpy(bname, genbuf);
        }
        line=11;
        getdata(line++, 0, "ÐÂÌÖÂÛÇøËµÃ÷: ", genbuf, 60, DOECHO, NULL, true);
        if (*genbuf != 0)
            strncpy(newfh.title, genbuf, sizeof(newfh.title));
        getdata(line++, 0, "ÌÖÂÛÇø¹ÜÀíÔ±: ", genbuf, 60, DOECHO, NULL, true);
        if (*genbuf != 0)
            strncpy(newfh.BM, genbuf, sizeof(newfh.BM));
        if (*genbuf == ' ')
            strncpy(newfh.BM, "\0", sizeof(newfh.BM));
        /*
         * newfh.BM[ BM_LEN - 1 ]=fh.BM[ BM_LEN - 1 ]; 
         */
        sprintf(buf, "ÄäÃû°æ (Y/N)? [%c]: ", (noidboard) ? 'Y' : 'N');
        getdata(line++, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'y' || *genbuf == 'Y' || *genbuf == 'N' || *genbuf == 'n') {
            if (*genbuf == 'y' || *genbuf == 'Y')
                noidboard = 1;
            else
                noidboard = 0;
        }
        sprintf(buf, "²»¼ÇÎÄÕÂÊý (Y/N)? [%c]: ", (newfh.flag & BOARD_JUNK) ? 'Y' : 'N');
        getdata(line++, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'y' || *genbuf == 'Y' || *genbuf == 'N' || *genbuf == 'n') {
            if (*genbuf == 'y' || *genbuf == 'Y')
                newfh.flag |= BOARD_JUNK;
            else
                newfh.flag &= ~BOARD_JUNK;
        };
        sprintf(buf, "¿ÉÏòÍâ×ªÐÅ (Y/N)? [%c]: ", (newfh.flag & BOARD_OUTFLAG) ? 'Y' : 'N');
        getdata(line++, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'y' || *genbuf == 'Y' || *genbuf == 'N' || *genbuf == 'n') {
            if (*genbuf == 'y' || *genbuf == 'Y')
                newfh.flag |= BOARD_OUTFLAG;
            else
                newfh.flag &= ~BOARD_OUTFLAG;
        };
        sprintf(buf, "¿ÉÕ³Ìù¸½¼þ (Y/N)? [%c]: ", (newfh.flag & BOARD_ATTACH) ? 'Y' : 'N');
        getdata(line++, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'y' || *genbuf == 'Y' || *genbuf == 'N' || *genbuf == 'n') {
            if (*genbuf == 'y' || *genbuf == 'Y')
                newfh.flag |= BOARD_ATTACH;
            else
                newfh.flag &= ~BOARD_ATTACH;
        };
        sprintf(buf, "ÔÊÐí Email ·¢ÎÄ (Y/N)? [%c]: ", (newfh.flag & BOARD_EMAILPOST) ? 'Y' : 'N');
        getdata(line++, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'y' || *genbuf == 'Y' || *genbuf == 'N' || *genbuf == 'n') {
            if (*genbuf == 'y' || *genbuf == 'Y')
                newfh.flag |= BOARD_EMAILPOST;
            else
                newfh.flag &= ~BOARD_EMAILPOST;
        };
        getdata(line++, 0, "ÊÇ·ñÒÆ¶¯¾«»ªÇøµÄÎ»ÖÃ (Y/N)? [N]: ", genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'Y' || *genbuf == 'y')
            a_mv = 2;           /* ±íÊ¾ÒÆ¶¯¾«»ªÇøÄ¿Â¼ */
        else
            a_mv = 0;
        sprintf(buf, "ÊÇ·ñÎª¶ÁÏÞÖÆ¾ãÀÖ²¿: (Y/N)? [%c]", (newfh.flag & BOARD_CLUB_READ) ? 'Y' : 'N');
        getdata(line++, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'Y' || *genbuf == 'y')
            newfh.flag |= BOARD_CLUB_READ;
        else if (*genbuf == 'N' || *genbuf == 'n')
            newfh.flag &= ~BOARD_CLUB_READ;
        sprintf(buf, "ÊÇ·ñÎª·¢±íÏÞÖÆ¾ãÀÖ²¿: (Y/N)? [%c]", (newfh.flag & BOARD_CLUB_WRITE) ? 'Y' : 'N');
        getdata(line++, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'Y' || *genbuf == 'y')
            newfh.flag |= BOARD_CLUB_WRITE;
        else if (*genbuf == 'N' || *genbuf == 'n')
            newfh.flag &= ~BOARD_CLUB_WRITE;
        if (newfh.flag & BOARD_CLUB_WRITE || newfh.flag & BOARD_CLUB_READ) {
            sprintf(buf, "ÊÇ·ñÎªÒþ²ØÏÞÖÆ¾ãÀÖ²¿: (Y/N)? [%c]", (newfh.flag & BOARD_CLUB_HIDE) ? 'Y' : 'N');
            getdata(line++, 0, buf, genbuf, 4, DOECHO, NULL, true);
            if (*genbuf == 'Y' || *genbuf == 'y')
                newfh.flag |= BOARD_CLUB_HIDE;
            else if (*genbuf == 'N' || *genbuf == 'n')
                newfh.flag &= ~BOARD_CLUB_HIDE;
        } else
            newfh.flag &= ~BOARD_CLUB_HIDE;
        
        sprintf(buf, "ÊÇ·ñÎªÄ¿Â¼ (Y/N)? [%c]", (newfh.flag & BOARD_GROUP) ? 'Y' : 'N');
        getdata(line++, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'Y' || *genbuf == 'y')
            newfh.flag |= BOARD_GROUP;
        else if (*genbuf == 'N' || *genbuf == 'n')
            newfh.flag &= ~BOARD_GROUP;

        while(1) {
            sprintf(buf, "Éè¶¨ËùÊôÄ¿Â¼[%s]", groupname);
            strcpy(genbuf,groupname);
            getdata(line, 0, buf, genbuf, BOARDNAMELEN, DOECHO, NULL, false);
            if (*genbuf == 0) {
                newfh.group = 0;
                break;
            }
            newfh.group=getbnum(genbuf);
            if (newfh.group) {
		if (!(getboard(newfh.group)->flag&BOARD_GROUP)) {
                    move(line+1,0);
                    prints("²»ÊÇÄ¿Â¼");
		} else break;
            }
        }
        
        line++;
        
        getdata(line++, 0, "ËùÐèÖ°Îñ: ", genbuf, 60, DOECHO, NULL, true); 
        if (*genbuf != 0)
            newfh.title_level=atoi(genbuf);

        getdata(line++, 0, "ÊÇ·ñ¸ü¸Ä´æÈ¡È¨ÏÞ (Y/N)? [N]: ", genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'Y' || *genbuf == 'y') {
            char ans[5];

            sprintf(genbuf, "ÏÞÖÆ (R)ÔÄ¶Á »ò (P)ÕÅÌù ÎÄÕÂ [%c]: ", (newfh.level & PERM_POSTMASK ? 'P' : 'R'));
            getdata(line++, 0, genbuf, ans, 4, DOECHO, NULL, true);
            if ((newfh.level & PERM_POSTMASK) && (*ans == 'R' || *ans == 'r'))
                newfh.level &= ~PERM_POSTMASK;
            else if (!(newfh.level & PERM_POSTMASK) && (*ans == 'P' || *ans == 'p'))
                newfh.level |= PERM_POSTMASK;
            move(1, 0);
            clrtobot();
            move(2, 0);
            prints("Éè¶¨ %s '%s' ÌÖÂÛÇøµÄÈ¨ÏÞ\n", newfh.level & PERM_POSTMASK ? "ÕÅÌù" : "ÔÄ¶Á", newfh.filename);
            newfh.level = setperms(newfh.level, 0, "È¨ÏÞ", NUMPERMS, showperminfo, NULL);
            clear();
            getdata(0, 0, "È·¶¨Òª¸ü¸ÄÂð? (Y/N) [N]: ", genbuf, 4, DOECHO, NULL, true);
        } else {
            getdata(line++, 0, "È·¶¨Òª¸ü¸ÄÂð? (Y/N) [N]: ", genbuf, 4, DOECHO, NULL, true);
        }
        if (*genbuf == 'Y' || *genbuf == 'y') {
            char lookgrp[30];
            int ret;

            {
                char secu[STRLEN];

                sprintf(secu, "ÐÞ¸ÄÌÖÂÛÇø£º%s(%s)", fh.filename, newfh.filename);
                securityreport(secu, NULL, NULL);
            }
            if (strcmp(fh.filename, newfh.filename)) {
                char old[256], tar[256];

                a_mv = 1;       /* ±íÊ¾°æÃû¸Ä±ä£¬ÐèÒª¸üÐÂ¾«»ªÇøÂ·¾¶ */
                setbpath(old, fh.filename);
                setbpath(tar, newfh.filename);
                f_mv(old, tar);
                sprintf(old, "vote/%s", fh.filename);
                sprintf(tar, "vote/%s", newfh.filename);
                f_mv(old, tar);
            }
            if (newfh.BM[0] != '\0')
                if (strlen(newfh.BM) <= 30)
                    sprintf(vbuf, "%-38.38s(BM: %s)", newfh.title + 13, newfh.BM);
                else
                    snprintf(vbuf, STRLEN, "%-28.28s(BM: %s)", newfh.title + 13, newfh.BM);
            else
                sprintf(vbuf, "%-38.38s", newfh.title + 13);
            edit_grp(fh.filename, oldtitle + 13, vbuf);
            if (a_mv >= 1) {
                group = chgrp();
                /*
                 * »ñÈ¡¸Ã°æ¶ÔÓ¦µÄ group 
                 */
                ann_get_path(fh.filename, newpath, sizeof(newpath));
                snprintf(oldpath, sizeof(oldpath), "0Announce/%s", newpath);
                sprintf(newpath, "0Announce/groups/%s/%s", group, newfh.filename);
                if (strcmp(oldpath, newpath) || a_mv != 2) {
                    if (group != NULL) {
                        if (newfh.BM[0] != '\0')
                            if (strlen(newfh.BM) <= 30)
                                sprintf(vbuf, "%-38.38s(BM: %s)", newfh.title + 13, newfh.BM);
                            else
                                sprintf(vbuf, "%-28.28s(BM: %s)", newfh.title + 13, newfh.BM);
                        else
                            sprintf(vbuf, "%-38.38s", newfh.title + 13);

                        if (add_grp(group, newfh.filename, vbuf, cexplain) == -1)
                            prints("\n³ÉÁ¢¾«»ªÇøÊ§°Ü....\n");
                        else
                            prints("ÒÑ¾­ÖÃÈë¾«»ªÇø...\n");
                        if (dashd(oldpath)) {
                            /*
                             * sprintf(genbuf, "/bin/rm -fr %s", newpath);
                             */
                            f_rm(newpath);
                        }
                        f_mv(oldpath, newpath);
                        del_grp(fh.filename, fh.title + 13);
                        snprintf(newfh.ann_path,127,"%s/%s",group, newfh.filename);
                        newfh.ann_path[127]=0;
                    }
                }
            }
            if (noidboard == 1 && !anonymousboard(newfh.filename)) {
                newfh.flag |= BOARD_ANNONY;
                addtofile("etc/anonymous", newfh.filename);
            } else if (noidboard == 0) {
                newfh.flag &= ~BOARD_ANNONY;
                del_from_file("etc/anonymous", newfh.filename);
            }
            set_board(pos, &newfh, &fh);
            sprintf(genbuf, "¸ü¸ÄÌÖÂÛÇø %s µÄ×ÊÁÏ --> %s", fh.filename, newfh.filename);
            bbslog("user", "%s", genbuf);
        }
    }
    clear();
    return 0;
}

int searchtrace()
{
    int id;
    char tmp_command[80], tmp_id[20];
    char buf[8192];
    struct userec *lookupuser;

    if (check_systempasswd() == false)
        return -1;
    modify_user_mode(ADMIN);
    clear();
    stand_title("²éÑ¯Ê¹ÓÃÕß·¢ÎÄ¼ÇÂ¼");
    move(1, 0);
    usercomplete("ÇëÊäÈëÊ¹ÓÃÕßÕÊºÅ:", genbuf);
    strcpy(tmp_id, genbuf);
    if (tmp_id[0] == '\0') {
        clear();
        return -1;
    }

    if (!(id = getuser(genbuf, &lookupuser))) {
        move(3, 0);
        prints("²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ\n");
        clrtoeol();
        pressreturn();
        clear();
        return -1;
    }

    sprintf(tmp_command, "grep -a -w %s user.log | grep posted > tmp/searchresult.%d", tmp_id, getpid());
    system(tmp_command);
    sprintf(tmp_command, "tmp/searchresult.%d", getpid());
    mail_file(currentuser->userid, tmp_command, currentuser->userid, "ÏµÍ³²éÑ¯½á¹û", BBSPOST_MOVE, NULL);

    sprintf(buf, "²éÑ¯ÓÃ»§ %s µÄ·¢ÎÄÇé¿ö", tmp_id);
    securityreport(buf, lookupuser, NULL);      /*Ð´Èësyssecurity°æ, stephen 2000.12.21 */
    sprintf(buf, "Search the posts by %s in the trace", tmp_id);
    bbslog("user", "%s", buf);  /*Ð´Èëtrace, stephen 2000.12.21 */

    move(3, 0);
    prints("²éÑ¯½á¹ûÒÑ¾­¼Äµ½ÄúµÄÐÅÏä£¡ \n");
    pressreturn();
    clear();
    return 0;
}                               /* stephen 2000.12.15 let sysop search in trace */


/*
char curruser[IDLEN + 2];
extern int delmsgs[];
extern int delcnt;

void domailclean(struct fileheader *fhdrp, char *arg)
{
    static int newcnt, savecnt, deleted, idc;
    char buf[STRLEN];

    if (fhdrp == NULL) {
        bbslog("clean", "new = %d, saved = %d, deleted = %d", newcnt, savecnt, deleted);
        newcnt = savecnt = deleted = idc = 0;
        if (delcnt) {
            setmailfile(buf, curruser, DOT_DIR);
            while (delcnt--)
                delete_record(buf, sizeof(struct fileheader), delmsgs[delcnt], NULL, NULL);
        }
        delcnt = 0;
        return;
    }
    idc++;
    if (!(fhdrp->accessed[0] & FILE_READ))
        newcnt++;
    else if (fhdrp->accessed[0] & FILE_MARKED)
        savecnt++;
    else {
        deleted++;
        setmailfile(buf, curruser, fhdrp->filename);
        unlink(buf);
        delmsgs[delcnt++] = idc;
    }
}

int cleanmail(struct userec *urec, char *arg)
{
    struct stat statb;

    if (urec->userid[0] == '\0' || !strcmp(urec->userid, "new"))
        return 0;
    setmailfile(genbuf, urec->userid, DOT_DIR);
    if (stat(genbuf, &statb) == -1) {
        bbslog("clean", "%s no mail", urec->userid);
    } else {
        if (statb.st_size == 0) {
            bbslog("clean", "%s no mail", urec->userid);
        } else {
            strcpy(curruser, urec->userid);
            delcnt = 0;
            apply_record(genbuf, (RECORD_FUNC_ARG) domailclean, sizeof(struct fileheader), 0, 1);
            domailclean(NULL, 0);
        }
    }
    return 0;
}

int m_mclean()
{
    char ans[5];

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    clear();
    stand_title("Çå³ýË½ÈËÐÅ¼þ");
    move(1, 0);
    prints("Çå³ýËùÓÐÒÑ¶ÁÇÒÎ´ mark µÄÐÅ¼þ\n");
    getdata(2, 0, "È·¶¨Âð (Y/N)? [N]: ", ans, 3, DOECHO, NULL, true);
    if (ans[0] != 'Y' && ans[0] != 'y') {
        clear();
        return 0;
    }
    {
        char secu[STRLEN];

        sprintf(secu, "Çå³ýËùÓÐÊ¹ÓÃÕßÒÑ¶ÁÐÅ¼þ¡£");
        securityreport(secu, NULL, NULL);
    }

    move(3, 0);
    prints("ÇëÄÍÐÄµÈºò.\n");
    refresh();
    apply_users(cleanmail, 0);
    move(4, 0);
    prints("Çå³ýÍê³É! Çë²é¿´ÈÕÖ¾ÎÄ¼þ.\n");
    bbslog("user","%s","Mail Clean");
    pressreturn();
    clear();
    return 0;
}
*/

void trace_state(flag, name, size)
int flag, size;
char *name;
{
    char buf[STRLEN];

    if (flag != -1) {
        sprintf(buf, "ON (size = %d)", size);
    } else {
        strcpy(buf, "OFF");
    }
    prints("%s¼ÇÂ¼ %s\n", name, buf);
}

int touchfile(filename)
char *filename;
{
    int fd;

    if ((fd = open(filename, O_RDWR | O_CREAT, 0600)) > 0) {
        close(fd);
    }
    return fd;
}

int m_trace()
{
    struct stat ostatb, cstatb;
    int otflag, ctflag, done = 0;
    char ans[3];
    char *msg;

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    clear();
    stand_title("Set Trace Options");
    while (!done) {
        move(2, 0);
        otflag = stat("trace", &ostatb);
        ctflag = stat("trace.chatd", &cstatb);
        prints("Ä¿Ç°Éè¶¨:\n");
        trace_state(otflag, "Ò»°ã", ostatb.st_size);
        trace_state(ctflag, "ÁÄÌì", cstatb.st_size);
        move(9, 0);
        prints("<1> ÇÐ»»Ò»°ã¼ÇÂ¼\n");
        prints("<2> ÇÐ»»ÁÄÌì¼ÇÂ¼\n");
        getdata(12, 0, "ÇëÑ¡Ôñ (1/2/Exit) [E]: ", ans, 2, DOECHO, NULL, true);

        switch (ans[0]) {
        case '1':
            if (otflag) {
                touchfile("trace");
                msg = "Ò»°ã¼ÇÂ¼ ON";
            } else {
                f_mv("trace", "trace.old");
                msg = "Ò»°ã¼ÇÂ¼ OFF";
            }
            break;
        case '2':
            if (ctflag) {
                touchfile("trace.chatd");
                msg = "ÁÄÌì¼ÇÂ¼ ON";
            } else {
                f_mv("trace.chatd", "trace.chatd.old");
                msg = "ÁÄÌì¼ÇÂ¼ OFF";
            }
            break;
        default:
            msg = NULL;
            done = 1;
        }
        move(t_lines - 2, 0);
        if (msg) {
            prints("%s\n", msg);
            bbslog("user", "%s", msg);
        }
    }
    clear();
    return 0;
}

int valid_userid(ident)         /* check the user has registed, added by dong, 1999.4.18 */
char *ident;
{
    if (strchr(ident, '@') && valid_ident(ident))
        return 1;
    return 0;
}

int check_proxy_IP(ip, buf)
                                /*
                                 * added for rejection of register from proxy,
                                 * Bigman, 2001.11.9 
                                 */
 /*
  * Óëbbsd_singleÀïÃæµÃlocal_check_ban_IP»ù±¾Ò»Ñù£¬¿ÉÒÔ¿¼ÂÇ¹²ÓÃ 
  */
char *ip;
char *buf;
{                               /* Leeward 98.07.31
                                 * RETURN:
                                 * - 1: No any banned IP is defined now
                                 * 0: The checked IP is not banned
                                 * other value over 0: The checked IP is banned, the reason is put in buf
                                 */
    FILE *Ban = fopen("etc/proxyIP", "r");
    char IPBan[64];
    int IPX = -1;
    char *ptr;

    if (!Ban)
        return IPX;
    else
        IPX++;

    while (fgets(IPBan, 64, Ban)) {
        if ((ptr = strchr(IPBan, '\n')) != NULL)
            *ptr = 0;
        if ((ptr = strchr(IPBan, ' ')) != NULL) {
            *ptr++ = 0;
            strcpy(buf, ptr);
        }
        IPX = strlen(ip);
        if (!strncmp(ip, IPBan, IPX))
            break;
        IPX = 0;
    }

    fclose(Ban);
    return IPX;
}

int apply_reg(regfile, fname, pid, num)
/* added by Bigman, 2002.5.31 */
/* ÉêÇëÖ¸¶¨ÌõÊý×¢²áµ¥ */
char *regfile, *fname;
long pid;
int num;
{
    FILE *in_fn, *out_fn, *tmp_fn;
    char fname1[STRLEN], fname2[STRLEN];
    int sum, fd;
    char *ptr;

    strcpy(fname1, "reg.ctrl");

    if ((in_fn = fopen(regfile, "r+")) == NULL) {
        move(2, 0);
        prints("ÏµÍ³´íÎó, ÎÞ·¨¶ÁÈ¡×¢²á×ÊÁÏµµ: %s\n", regfile);
        pressreturn();
        return -1;
    }

    fd = fileno(in_fn);
    flock(fd, LOCK_EX);

    if ((out_fn = fopen(fname, "w")) == NULL) {
        move(2, 0);
        flock(fd, LOCK_UN);
        fclose(in_fn);
        prints("ÏµÍ³´íÎó, ÎÞ·¨Ð´ÁÙÊ±×¢²á×ÊÁÏµµ: %s\n", fname);
        pressreturn();
        return -1;
    }
    sum = 0;

    while (fgets(genbuf, STRLEN, in_fn) != NULL) {
        if ((ptr = (char *) strstr(genbuf, "----")) != NULL)
            sum++;

        fputs(genbuf, out_fn);

        if (sum >= num)
            break;
    }
    fclose(out_fn);

    if (sum >= num) {
        sum = 0;

		gettmpfilename( fname2, "reg" );
        //sprintf(fname2, "tmp/reg.%ld", pid);

        if ((tmp_fn = fopen(fname2, "w")) == NULL) {
            prints("²»ÄÜ½¨Á¢ÁÙÊ±ÎÄ¼þ:%s\n", fname2);
            flock(fd, LOCK_UN);
            fclose(in_fn);
            pressreturn();
            return -1;
        }

        while (fgets(genbuf, STRLEN, in_fn) != NULL) {
            if ((ptr = (char *) strstr(genbuf, "userid")) != NULL)
                sum++;
            fputs(genbuf, tmp_fn);

        }

        flock(fd, LOCK_UN);

        fclose(in_fn);
        fclose(tmp_fn);

        if (sum > 0) {
            f_rm(regfile);
            f_mv(fname2, regfile);
        } else
            f_rm(regfile);

        f_rm(fname2);

    }

    else
        f_rm(regfile);

    if ((out_fn = fopen(fname1, "a")) == NULL) {
        move(2, 0);
        prints("ÏµÍ³´íÎó, ÎÞ·¨¸ü¸Ä×¢²á¿ØÖÆÎÄ¼þ: %s\n", fname1);
        pressreturn();
        return -1;
    }

    fd = fileno(out_fn);

    flock(fd, LOCK_UN);
    fprintf(out_fn, "%ld\n", pid);
    flock(fd, LOCK_UN);
    fclose(out_fn);

    return (0);
}

int check_reg(mod)
int mod;

/* added by Bigman, 2002.5.31 */
/* mod=0 ¼ì²éreg_controlÎÄ¼þ */
/* mod=1 Õý³£ÍË³öÉ¾³ý¸ÃÎÄ¼þ */
{
    FILE *fn1, *fn2;
    char fname1[STRLEN];
    char fname2[STRLEN];
    long myid;
    int flag = 0, fd;

    strcpy(fname1, "reg.ctrl");

    if ((fn1 = fopen(fname1, "r")) != NULL) {

        fd = fileno(fn1);
        flock(fd, LOCK_EX);

		gettmpfilename( fname2, "reg.c");
        //sprintf(fname2, "tmp/reg.c%ld", getpid());

        if ((fn2 = fopen(fname2, "w")) == NULL) {
            prints("²»ÄÜ½¨Á¢ÁÙÊ±ÎÄ¼þ:%s\n", fname2);
            flock(fd, LOCK_UN);
            fclose(fn1);
            pressreturn();
            return -1;
        } else {
            while (fgets(genbuf, STRLEN, fn1) != NULL) {

                myid = atol(genbuf);

                if (mod == 0) {
/*                    	if (myid==getpid())
                    {
                	prints("ÄãÖ»ÄÜÒ»¸ö½ø³Ì½øÐÐÉóÅúÕÊºÅ");
                	pressreturn();
                	return -1;
                    }
*/

                    if (kill(myid, 0) == -1) {  /*×¢²áÖÐ¼ä¶ÏÏßÁË£¬»Ö¸´ */
                        flag = 1;
                        restore_reg(myid);
                    } else {
                        fprintf(fn2, "%ld\n", myid);
                    }
                } else {
                    flag = 1;
                    if (myid != getpid())
                        fprintf(fn2, "%ld\n", myid);


                }

            }
            fclose(fn2);
        }
        flock(fd, LOCK_UN);
        fclose(fn1);

        if (flag == 1) {
            f_rm(fname1);
            f_mv(fname2, fname1);
        }
        f_rm(fname2);

    }

    return (0);
}

int restore_reg(pid)
long pid;

/* added by Bigman, 2002.5.31 */
/* »Ö¸´¶ÏÏßµÄ×¢²áÎÄ¼þ */
{
    FILE *fn, *freg;
    char *regfile, buf[STRLEN];
    int fd1, fd2;

    regfile = "new_register";

    sprintf(buf, "register.%ld", pid);

    if ((fn = fopen(buf, "r")) != NULL) {
        fd1 = fileno(fn);
        flock(fd1, LOCK_EX);

        if ((freg = fopen(regfile, "a")) != NULL) {
            fd2 = fileno(freg);
            flock(fd2, LOCK_EX);
            while (fgets(genbuf, STRLEN, fn) != NULL)
                fputs(genbuf, freg);
            flock(fd2, LOCK_UN);
            fclose(freg);

        }
        flock(fd1, LOCK_UN);
        fclose(fn);

        f_rm(buf);
    }

    return (0);
}
static const char *field[] = { "usernum", "userid", "realname", "career",
    "addr", "phone", "birth", NULL
};
static const char *reason[] = {
    "ÇëÊäÈëÕæÊµÐÕÃû(¹úÍâ¿ÉÓÃÆ´Òô).", "ÇëÏêÌîÑ§Ð£¿ÆÏµ»ò¹¤×÷µ¥Î».",
    "ÇëÌîÐ´ÍêÕûµÄ×¡Ö·×ÊÁÏ.", "ÇëÏêÌîÁ¬Âçµç»°(ÈôÎÞ¿ÉÓÃºô»ú»òEmailµØÖ·´úÌæ).",
    "ÇëÈ·Êµ¶øÏêÏ¸µÄÌîÐ´×¢²áÉêÇë±í.", "ÇëÓÃÖÐÎÄÌîÐ´ÉêÇëµ¥.",
    "²»ÔÊÐí´Ó´©Ëó×¢²á", "Í¬Ò»¸öÓÃ»§×¢²áÁË¹ý¶àID",
    NULL
};

int scan_register_form(logfile, regfile)
char *logfile, *regfile;
{
    static const char *finfo[] = { "ÕÊºÅÎ»ÖÃ", "ÉêÇë´úºÅ", "ÕæÊµÐÕÃû", "·þÎñµ¥Î»",
        "Ä¿Ç°×¡Ö·", "Á¬Âçµç»°", "Éú    ÈÕ", NULL
    };
    struct userec uinfo;
    FILE *fn, *fout, *freg;
    char fdata[7][STRLEN];
    char fname[STRLEN], buf[STRLEN], buff;

    /*
     * ^^^^^ Added by Marco 
     */
    char ans[5], *ptr, *uid;
    int n, unum, fd;
    int count, sum, total_num;  /*Haohmaru.2000.3.9.¼ÆËã»¹ÓÐ¶àÉÙµ¥×ÓÃ»´¦Àí */
    char result[256], ip[17];   /* Added for IP query by Bigman: 2002.8.20 */
    long pid;                   /* Added by Bigman: 2002.5.31 */

    uid = currentuser->userid;


    stand_title("ÒÀÐòÉè¶¨ËùÓÐÐÂ×¢²á×ÊÁÏ");
/*    sprintf(fname, "%s.tmp", regfile);*/

    pid = getpid();
    sprintf(fname, "register.%ld", pid);

    move(2, 0);
    if (dashf(fname)) {
/*        prints("[1mÆäËû SYSOP ÕýÔÚÊ¹ÓÃ telnet »ò WWW ²é¿´×¢²áÉêÇëµ¥£¬Çë¼ì²éÊ¹ÓÃÕß×´Ì¬¡£\n\n");
        prints("[33mÈç¹ûÃ»ÓÐÆäËû SYSOP ÕýÔÚ²é¿´×¢²áÉêÇëµ¥£¬ÔòÊÇÓÉÓÚ¶ÏÏßÔì³ÉµÄÎÞ·¨×¢²á¡£\n");
        prints("Çë½ø bbsroot ÕÊ»§ÔËÐÐÒ»´ÎÒÔÏÂÃüÁî£º\n");
        prints("                                   [32mcat new_register.tmp >> new_register[33m\n");
        prints("È·ÈÏÉÏÊöÃüÁîÔËÐÐ³É¹¦ºó£¬ÔÙÔËÐÐÒ»´ÎÒÔÏÂÃüÁî£º\n");
        prints("                                            [32mrm new_register.tmp\n[m");
        pressreturn();
        return -1;*/

        restore_reg(pid);       /* Bigman,2002.5.31:»Ö¸´¸ÃÎÄ¼þ */
    }
/*    f_mv(regfile, fname);*/
/*ÉêÇë×¢²áµ¥ added by Bigman, 2002.5.31*/

/*Í³¼Æ×ÜµÄ×¢²áµ¥Êý Bigman, 2002.6.2 */
    if ((fn = fopen(regfile, "r")) == NULL) {
        move(2, 0);
        prints("ÏµÍ³´íÎó, ÎÞ·¨¶ÁÈ¡×¢²á×ÊÁÏµµ: %s\n", fname);
        pressreturn();
        return -1;
    }

    fd = fileno(fn);
    flock(fd, LOCK_EX);

    total_num = 0;
    while (fgets(genbuf, STRLEN, fn) != NULL) {
        if ((ptr = (char *) strstr(genbuf, "userid")) != NULL)
            total_num++;
    }
    flock(fd, LOCK_UN);
    fclose(fn);

    apply_reg(regfile, fname, pid, 50);

    if ((fn = fopen(fname, "r")) == NULL) {
        move(2, 0);
        prints("ÏµÍ³´íÎó, ÎÞ·¨¶ÁÈ¡×¢²á×ÊÁÏµµ: %s\n", fname);
        pressreturn();
        return -1;
    }
    memset(fdata, 0, sizeof(fdata));
    /*
     * Haohmaru.2000.3.9.¼ÆËã¹²ÓÐ¶àÉÙµ¥×Ó 
     */
    sum = 0;
    while (fgets(genbuf, STRLEN, fn) != NULL) {
        if ((ptr = (char *) strstr(genbuf, "userid")) != NULL)
            sum++;
    }
    fseek(fn, 0, SEEK_SET);
    count = 1;
    while (fgets(genbuf, STRLEN, fn) != NULL) {
        struct userec *lookupuser;

        if ((ptr = (char *) strstr(genbuf, ": ")) != NULL) {
            *ptr = '\0';
            for (n = 0; field[n] != NULL; n++) {
                if (strcmp(genbuf, field[n]) == 0) {
                    strcpy(fdata[n], ptr + 2);
                    if ((ptr = (char *) strchr(fdata[n], '\n')) != NULL)
                        *ptr = '\0';
                }
            }
        } else if ((unum = getuser(fdata[1], &lookupuser)) == 0) {
            move(2, 0);
            clrtobot();
            prints("ÏµÍ³´íÎó, ²éÎÞ´ËÕÊºÅ.\n\n");
            for (n = 0; field[n] != NULL; n++)
                prints("%s     : %s\n", finfo[n], fdata[n]);
            pressreturn();
            memset(fdata, 0, sizeof(fdata));
        } else {
            struct userdata ud;

            uinfo = *lookupuser;
            move(1, 0);
            prints("ÕÊºÅÎ»ÖÃ     : %d   ¹²ÓÐ %d ÕÅ×¢²áµ¥£¬µ±Ç°ÎªµÚ %d ÕÅ£¬»¹Ê£ %d ÕÅ\n", unum, total_num, count++, sum - count + 1);    /*Haohmaru.2000.3.9.¼ÆËã»¹ÓÐ¶àÉÙµ¥×ÓÃ»´¦Àí */
            disply_userinfo(&uinfo, 1);
			
			read_userdata(lookupuser->userid, &ud);

/* Ìí¼Ó²éÑ¯IP, Bigman: 2002.8.20 */
            /*move(8, 20);*/
	     move(8,30); /* ÔÙÍùºóÅ²Å²µØ  binxun . 2003.5.30 */
            strncpy(ip, uinfo.lasthost, 17);
            find_ip(ip, 2, result);
            prints("\033[33m%s\033[m", result);

            move(15, 0);
            printdash(NULL);
            for (n = 0; field[n] != NULL; n++) {
                /*
                 * added for rejection of register from proxy
                 */
                /*
                 * Bigman, 2001.11.9
                 */
                 clrtoeol();
                if (n == 1) {
                    if (check_proxy_IP(uinfo.lasthost, buf) > 0)
                        prints("%s     : %s \033[33m%s\033[m\n", finfo[n], fdata[n], buf);
                    else
                        prints("%s     : %s\n", finfo[n], fdata[n]);
                } else
                    prints("%s     : %s\n", finfo[n], fdata[n]);
            }
            /*
             * if (uinfo.userlevel & PERM_LOGINOK) modified by dong, 1999.4.18 
             */
            if ((uinfo.userlevel & PERM_LOGINOK) || valid_userid(ud.realemail)) {
                move(t_lines - 1, 0);
                prints("´ËÕÊºÅ²»ÐèÔÙÌîÐ´×¢²áµ¥.\n");
                pressanykey();
                ans[0] = 'D';
            } else {
                getdata(t_lines - 1, 0, "ÊÇ·ñ½ÓÊÜ´Ë×ÊÁÏ (Y/N/Q/Del/Skip)? [S]: ", ans, 3, DOECHO, NULL, true);
            }
            move(2, 0);
            clrtobot();
            switch (ans[0]) {
            case 'D':
            case 'd':
                break;
            case 'Y':
            case 'y':
			{
				struct usermemo *um;

				read_user_memo(uinfo.userid, &um);

                prints("ÒÔÏÂÊ¹ÓÃÕß×ÊÁÏÒÑ¾­¸üÐÂ:\n");
                n = strlen(fdata[5]);
                if (n + strlen(fdata[3]) > 60) {
                    if (n > 40)
                        fdata[5][n = 40] = '\0';
                    fdata[3][60 - n] = '\0';
                }
                strncpy(ud.realname, fdata[2], NAMELEN);
                strncpy(ud.address, fdata[4], NAMELEN);
                sprintf(genbuf, "%s$%s@%s", fdata[3], fdata[5], uid);
		if(strlen(genbuf) >= STRLEN-16)
			sprintf(genbuf, "%s@%s",fdata[5],uid);
                strncpy(ud.realemail, genbuf, STRLEN - 16);
		ud.realemail[STRLEN - 16 - 1] = '\0';
                sprintf(buf, "tmp/email/%s", uinfo.userid);
                if ((fout = fopen(buf, "w")) != NULL) {
                    fprintf(fout, "%s\n", genbuf);
                    fclose(fout);
                }

                update_user(&uinfo, unum, 0);
                write_userdata(uinfo.userid, &ud);
				memcpy(&(um->ud), &ud, sizeof(ud));
				end_mmapfile(um, sizeof(struct usermemo), -1);

                mail_file(currentuser->userid, "etc/s_fill", uinfo.userid, "¹§ìûÄã£¬ÄãÒÑ¾­Íê³É×¢²á¡£", 0, NULL);
                sprintf(genbuf, "%s ÈÃ %s Í¨¹ýÉí·ÝÈ·ÈÏ.", uid, uinfo.userid);
                securityreport(genbuf, lookupuser, fdata);
                if ((fout = fopen(logfile, "a")) != NULL) {
                    time_t now;

                    for (n = 0; field[n] != NULL; n++)
                        fprintf(fout, "%s: %s\n", field[n], fdata[n]);
                    now = time(NULL);
                    fprintf(fout, "Date: %s\n", Ctime(now));
                    fprintf(fout, "Approved: %s\n", uid);
                    fprintf(fout, "----\n");
                    fclose(fout);
                }
                /*
                 * user_display( &uinfo, 1 ); 
                 */
                /*
                 * pressreturn(); 
                 */

                /*
                 * Ôö¼Ó×¢²áÐÅÏ¢¼ÇÂ¼ 2001.11.11 Bigman 
                 */
                sethomefile(buf, uinfo.userid, "/register");
                if ((fout = fopen(buf, "w")) != NULL) {
                    for (n = 0; field[n] != NULL; n++)
                        fprintf(fout, "%s     : %s\n", finfo[n], fdata[n]);
                    fprintf(fout, "ÄúµÄêÇ³Æ     : %s\n", uinfo.username);
                    fprintf(fout, "µç×ÓÓÊ¼þÐÅÏä : %s\n", ud.email);
                    fprintf(fout, "ÕæÊµ E-mail  : %s\n", ud.realemail);
                    fprintf(fout, "×¢²áÈÕÆÚ     : %s\n", ctime(&uinfo.firstlogin));
                    fprintf(fout, "×¢²áÊ±µÄ»úÆ÷ : %s\n", uinfo.lasthost);
                    fprintf(fout, "Approved: %s\n", uid);
                    fclose(fout);
                }

                break;
			}
            case 'Q':
            case 'q':
                if ((freg = fopen(regfile, "a")) != NULL) {
                    fd = fileno(freg);
                    flock(fd, LOCK_EX);

                    for (n = 0; field[n] != NULL; n++)
                        fprintf(freg, "%s: %s\n", field[n], fdata[n]);
                    fprintf(freg, "----\n");
                    while (fgets(genbuf, STRLEN, fn) != NULL)
                        fputs(genbuf, freg);

                    flock(fd, LOCK_UN);
                    fclose(freg);
                }

                break;
            case 'N':
            case 'n':
                for (n = 0; field[n] != NULL; n++)
                    prints("%s: %s\n", finfo[n], fdata[n]);
                move(9, 0);
                prints("ÇëÑ¡Ôñ/ÊäÈëÍË»ØÉêÇë±íÔ­Òò, °´ <enter> È¡Ïû.\n");
                for (n = 0; reason[n] != NULL; n++)
                    prints("%d) %s\n", n, reason[n]);
                getdata(10 + n, 0, "ÍË»ØÔ­Òò: ", buf, STRLEN, DOECHO, NULL, true);
                buff = buf[0];  /* Added by Marco */
                if (buf[0] != '\0') {
                    if (buf[0] >= '0' && buf[0] < '0' + n) {
                        strcpy(buf, reason[buf[0] - '0']);
                    }
                    sprintf(genbuf, "<×¢²áÊ§°Ü> - %s", buf);
                    strncpy(ud.address, genbuf, NAMELEN);
                    write_userdata(uinfo.userid, &ud);
                    update_user(&uinfo, unum, 0);

                    /*
                     * ------------------- Added by Marco 
                     */
                    switch (buff) {
                    case '0':
                        mail_file(currentuser->userid, "etc/f_fill.realname", uinfo.userid, ud.address, BBSPOST_LINK, NULL);
                        break;
                    case '1':
                        mail_file(currentuser->userid, "etc/f_fill.unit", uinfo.userid, ud.address, BBSPOST_LINK, NULL);
                        break;
                    case '2':
                        mail_file(currentuser->userid, "etc/f_fill.address", uinfo.userid, ud.address, BBSPOST_LINK, NULL);
                        break;
                    case '3':
                        mail_file(currentuser->userid, "etc/f_fill.telephone", uinfo.userid, ud.address, BBSPOST_LINK, NULL);
                        break;
                    case '4':
                        mail_file(currentuser->userid, "etc/f_fill.real", uinfo.userid, ud.address, BBSPOST_LINK, NULL);
                        break;
                    case '5':
                        mail_file(currentuser->userid, "etc/f_fill.chinese", uinfo.userid, ud.address, BBSPOST_LINK, NULL);
                        break;
                    case '6':
                        mail_file(currentuser->userid, "etc/f_fill.proxy", uinfo.userid, ud.address, BBSPOST_LINK, NULL);
                        break;
                    case '7':
                        mail_file(currentuser->userid, "etc/f_fill.toomany", uinfo.userid, ud.address, BBSPOST_LINK, NULL);
                        break;
                    default:
                        mail_file(currentuser->userid, "etc/f_fill", uinfo.userid, ud.address, BBSPOST_LINK, NULL);
                        break;
                    }
                    /*
                     * -------------------------------------------------------
                     */
                    /*
                     * user_display( &uinfo, 1 ); 
                     */
                    /*
                     * pressreturn(); 
                     */
                    break;
                }
                move(10, 0);
                clrtobot();
                prints("È¡ÏûÍË»Ø´Ë×¢²áÉêÇë±í.\n");
                /*
                 * run default -- put back to regfile 
                 */
            default:
                if ((freg = fopen(regfile, "a")) != NULL) {
                    fd = fileno(freg);
                    flock(fd, LOCK_EX);

                    for (n = 0; field[n] != NULL; n++)
                        fprintf(freg, "%s: %s\n", field[n], fdata[n]);
                    fprintf(freg, "----\n");

                    flock(fd, LOCK_UN);
                    fclose(freg);
                }
            }
            memset(fdata, 0, sizeof(fdata));
        }
    }                           /* while */

    check_reg(1);               /* Bigman:2002.5.31 */

    fclose(fn);
    unlink(fname);
    return (0);
}

int m_register()
{
    FILE *fn;
    char ans[3], *fname;
    int x, y, wid, len;

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    clear();

    if (check_reg(0) != 0)
        return (-1);            /* added by Bigman, 2002.5.31 */

    stand_title("Éè¶¨Ê¹ÓÃÕß×¢²á×ÊÁÏ");
    move(2, 0);

    fname = "new_register";

    if ((fn = fopen(fname, "r")) == NULL) {
        prints("Ä¿Ç°²¢ÎÞÐÂ×¢²á×ÊÁÏ.");
        pressreturn();
    } else {
        y = 2, x = wid = 0;
        while (fgets(genbuf, STRLEN, fn) != NULL && x < 65) {
            if (strncmp(genbuf, "userid: ", 8) == 0) {
                move(y++, x);
                prints(genbuf + 8);
                len = strlen(genbuf + 8);
                if (len > wid)
                    wid = len;
                if (y >= t_lines - 2) {
                    y = 2;
                    x += wid + 2;
                }
            }
        }
        fclose(fn);
        getdata(t_lines - 1, 0, "Éè¶¨×ÊÁÏÂð (Y/N)? [N]: ", ans, 2, DOECHO, NULL, true);
        if (ans[0] == 'Y' || ans[0] == 'y') {
            {
                char secu[STRLEN];

                sprintf(secu, "Éè¶¨Ê¹ÓÃÕß×¢²á×ÊÁÏ");
                securityreport(secu, NULL, NULL);
            }
            scan_register_form("register.list", fname);
        }
    }
    clear();
    return 0;
}

int m_stoplogin()
{
    char ans[4];

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    if (!HAS_PERM(currentuser, PERM_ADMIN))
        return -1;
    getdata(t_lines - 1, 0, "½ûÖ¹µÇÂ½Âð (Y/N)? [N]: ", ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y') {
        if (vedit("NOLOGIN", false, NULL, NULL) == -1)
            unlink("NOLOGIN");
    }
    return 0;
}

/* czz added 2002.01.15 */
int inn_start()
{
    char ans[4], tmp_command[80];

    getdata(t_lines - 1, 0, "Æô¶¯×ªÐÅÂð (Y/N)? [N]: ", ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y') {
        sprintf(tmp_command, "~bbs/innd/innbbsd");
        system(tmp_command);
    }
    return 0;
}

int inn_reload()
{
    char ans[4], tmp_command[80];

    getdata(t_lines - 1, 0, "ÖØ¶ÁÅäÖÃÂð (Y/N)? [N]: ", ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y') {
        sprintf(tmp_command, "~bbs/innd/ctlinnbbsd reload");
        system(tmp_command);
    }
    return 0;
}

int inn_stop()
{
    char ans[4], tmp_command[80];

    getdata(t_lines - 1, 0, "Í£Ö¹×ªÐÅÂð (Y/N)? [N]: ", ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y') {
        sprintf(tmp_command, "~bbs/innd/ctlinnbbsd shutdown");
        system(tmp_command);
    }
    return 0;
}

/* added end */
/* ·â½ûÈ¨ÏÞ¹ÜÀí*/
int x_deny()
{
    int sel;
    char userid[IDLEN + 2];
    struct userec *lookupuser;
    const int level[] = {
        PERM_BASIC,
        PERM_POST,
        PERM_DENYMAIL,
        PERM_CHAT,
        PERM_PAGE,
        PERM_DENYRELAX,
        -1
    };
    const int normal_level[] = {
        PERM_BASIC,
        PERM_POST,
        0,
        PERM_CHAT,
        PERM_PAGE,
        0,
        -1
    };

    const struct _select_item level_conf[] = {
        {3, 6, -1, SIT_SELECT, (void *) "1)µÇÂ¼È¨ÏÞ"},
        {3, 7, -1, SIT_SELECT, (void *) "2)·¢±íÎÄÕÂÈ¨ÏÞ"},
        {3, 8, -1, SIT_SELECT, (void *) "3)·¢ÐÅÈ¨ÏÞ"},
        {3, 9, -1, SIT_SELECT, (void *) "4)½øÈëÁÄÌìÊÒÈ¨ÏÞ"},
        {3, 10, -1, SIT_SELECT, (void *) "5)ºô½ÐÁÄÌìÈ¨ÏÞ"},
        {3, 11, -1, SIT_SELECT, (void *) "6)ÐÝÏÐÓéÀÖÈ¨ÏÞ"},
        {3, 12, -1, SIT_SELECT, (void *) "7)»»Ò»¸öID"},
        {3, 13, -1, SIT_SELECT, (void *) "8)ÍË³ö"},
        {-1, -1, -1, 0, NULL}
    };

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    move(0, 0);
    clear();

    while (1) {
        int i;
        int basicperm;
        int s[10][2];
        int lcount;

        move(1, 0);

        usercomplete("ÇëÊäÈëÊ¹ÓÃÕßÕÊºÅ:", genbuf);
        strncpy(userid, genbuf, IDLEN + 1);
        if (userid[0] == '\0') {
            clear();
            return 0;
        }

        if (!(getuser(userid, &lookupuser))) {
            move(3, 0);
            prints("²»ÕýÈ·µÄÊ¹ÓÃÕß´úºÅ\n");
            clrtoeol();
            pressreturn();
            clear();
            continue;
        }
        lcount = get_giveupinfo(lookupuser->userid, &basicperm, s);
        move(3, 0);
        clrtobot();

        for (i = 0; level[i] != -1; i++)
            if ((lookupuser->userlevel & level[i]) != normal_level[i]) {
                move(6 + i, 40);
                if (level[i] & basicperm)
                    prints("½äÍøÖÐ");
                else
                    prints("·â½ûÖÐ");
            }
        sel = simple_select_loop(level_conf, SIF_NUMBERKEY | SIF_SINGLE, 0, 6, NULL);
        if (sel == i + 2)
            break;
        if (sel > 0 && sel <= i) {
            /*char buf[40];  commented by binxun*/
            /*---------*/
            char buf[STRLEN]; /*buf is too small...changed by binxun .2003/05/11 */
            /*---------*/
            char reportbuf[120];

            move(40, 0);
            if ((lookupuser->userlevel & level[sel - 1]) == normal_level[sel - 1]) {
                sprintf(buf, "ÕæµÄÒª·â½û%sµÄ%s", lookupuser->userid, (char *) level_conf[sel - 1].data + 2);
                if (askyn(buf, 0) != 0) {
                    sprintf(reportbuf, "·â½û%sµÄ%s ", lookupuser->userid, (char *) level_conf[sel - 1].data + 2);
                    lookupuser->userlevel ^= level[sel - 1];
                    securityreport(reportbuf, lookupuser, NULL);
                    break;
                }
            } else {
                if (!(basicperm & level[sel - 1])) {
                    sprintf(buf, "ÕæµÄÒª½â¿ª%sµÄ%s ·â½û", lookupuser->userid, (char *) level_conf[sel - 1].data + 2);
                    sprintf(reportbuf, "½â¿ª%sµÄ%s ·â½û", lookupuser->userid, (char *) level_conf[sel - 1].data + 2);
                } else {
                    sprintf(buf, "ÕæµÄÒª½â¿ª%sµÄ%s ½äÍø", lookupuser->userid, (char *) level_conf[sel - 1].data + 2);
                    sprintf(reportbuf, "½â¿ª%sµÄ%s ½äÍø", lookupuser->userid, (char *) level_conf[sel - 1].data + 2);
                }
                if (askyn(buf, 0) != 0) {
                    lookupuser->userlevel ^= level[sel - 1];
                    securityreport(reportbuf, lookupuser, NULL);
                    save_giveupinfo(lookupuser, lcount, s);
                    break;
                }
            }
        }
    }
    return 0;
}

int set_BM()
/* Ö±½ÓÈÎÃâ°æÖ÷ Bigman:2002.9.1 */
{

    char bname[STRLEN], oldtitle[STRLEN], vbuf[256], *p;
    int pos, flag = 0, id, m, n, brd_num;
    unsigned int newlevel;
    struct boardheader fh, newfh;
    struct userec *lookupuser, uinfo;
    struct boardheader *bptr;
    char lookgrp[30];

    if (!HAS_PERM(currentuser, PERM_ADMIN) || !HAS_PERM(currentuser, PERM_SYSOP)) {
        move(3, 0);
        clrtobot();
        prints("±§Ç¸, Ö»ÓÐADMINÈ¨ÏÞµÄ¹ÜÀíÔ±²ÅÄÜÐÞ¸ÄÆäËûÓÃ»§È¨ÏÞ");
        pressreturn();
        return 0;
    }

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    clear();
    stand_title("ÈÎÃâ°æÖ÷");
    move(1, 0);
    make_blist();
    namecomplete("ÊäÈëÌÖÂÛÇøÃû³Æ: ", bname);
    if (*bname == '\0') {
        move(2, 0);
        prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
        pressreturn();
        clear();
        return -1;
    }
    pos = getboardnum(bname, &fh);
    memcpy(&newfh, &fh, sizeof(newfh));
    if (!pos) {
        move(2, 0);
        prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
        pressreturn();
        clear();
        return -1;
    }
    while (1) {
        clear();
        stand_title("ÈÎÃâ°æÖ÷");
        move(1, 0);
        prints("ÌÖÂÛÇøÃû³Æ:   %s\n", fh.filename);
        prints("ÌÖÂÛÇøËµÃ÷:   %s\n", fh.title);
        prints("ÌÖÂÛÇø¹ÜÀíÔ±: %s\n", fh.BM);
        strcpy(oldtitle, fh.title);


        getdata(6, 0, "(A)Ôö¼Ó°æÖ÷ (D)É¾³ý°æÖ÷ (Q)ÍË³ö?: [Q]", genbuf, 2, DOECHO, NULL, true);
        if (*genbuf == 'a' || *genbuf == 'A') {
            flag = 1;
        } else if (*genbuf == 'd' || *genbuf == 'D') {
            flag = 2;
            if (newfh.BM[0] == '\0') {
                flag = 0;
                prints("Ã»ÓÐ°æÖ÷£¬²»ÄÜÉ¾³ý°æÖ÷!");
                pressreturn();
            }
        } else {
            clear();
            return 0;
        }

        if (flag > 0) {
            usercomplete("ÇëÊäÈë" NAME_USER_SHORT " ID: ", genbuf);
            if (genbuf[0] == '\0') {
                clear();
                flag = 0;
                /*
                 * return 0;
                 */
            } else if (!(id = getuser(genbuf, &lookupuser))) {
                move(3, 0);
                prints("·Ç·¨ ID");
                clrtoeol();
                pressreturn();
                clear();
                flag = 0;
                /*
                 * return 0;
                 */
            }

            if (flag > 0) {
                uinfo = *lookupuser;
                disply_userinfo(&uinfo, 1);
                brd_num = 0;

                move(18, 0);

                if (!(lookupuser->userlevel & PERM_BOARDS)) {
                    prints("ÓÃ»§%s²»ÊÇ°æÖ÷!\n", lookupuser->userid);
                } else {
                    prints("ÓÃ»§ %s ÎªÒÔÏÂ°æµÄ°æÖ÷\n", lookupuser->userid);

                    for (n = 0; n < get_boardcount(); n++) {
                        bptr = (struct boardheader *) getboard(n + 1);
                        if (chk_BM_instr(bptr->BM, lookupuser->userid) == true) {
                            prints("%-32s%-32s\n", bptr->filename, bptr->title + 12);
                            brd_num++;
                        }
                    }

                }

                getdata(23, 0, "È·ÈÏÈÎÃâ¸ÃÓÃ»§£¨Y/N)?: [N]", genbuf, 2, DOECHO, NULL, true);

                if (*genbuf == 'Y' || *genbuf == 'y') {
                    newlevel = lookupuser->userlevel;


                    if (flag == 1) {
                        if (newfh.BM[0] != '\0')
                            strcat(newfh.BM, " ");
                        strcat(newfh.BM, lookupuser->userid);
                        newlevel |= PERM_BOARDS;
                        mail_file(currentuser->userid, "etc/forbm", lookupuser->userid, "ÐÂÈÎ" NAME_BM "±Ø¶Á", BBSPOST_LINK, NULL);
                    } else if (flag == 2) {
                        m = 0;
                        newfh.BM[0] = '\0';

                        p = strtok(fh.BM, " ");
                        if (p) {
                            if (strcmp(p, lookupuser->userid)) {
                                strncpy(newfh.BM, p, IDLEN + 2);
                                m++;
                            } else if (brd_num == 1) {
                                newlevel &= ~PERM_BOARDS;
                                newlevel &= ~PERM_CLOAK;
                            }
                        }
                        /*
                         * Èç¹ûÔö¼Ó°æÖ÷ÊýÄ¿ÇëÐÞ¸ÄÕâÀï 
                         */
                        for (n = 1; n < (BM_LEN - 1) / (IDLEN + 2); n++) {
                            p = strtok(NULL, " ");
                            if (p == NULL)
                                break;
                            else if (strcmp(p, lookupuser->userid)) {
                                if (m > 0) {
                                    strcat(newfh.BM, " ");
                                    strcat(newfh.BM, p);
                                } else
                                    strncpy(newfh.BM, p, IDLEN + 2);
                                m++;
                            } else if (brd_num == 1) {
                                newlevel &= ~PERM_BOARDS;
                                newlevel &= ~PERM_CLOAK;
                            }
                        }



                    }

                    if (newfh.BM[0] != '\0') {
                        if (strlen(newfh.BM)<=30)
                            sprintf(vbuf, "%-38.38s(BM: %s)", newfh.title + 13, newfh.BM);
                        else
                            sprintf(vbuf, "%-28.28s(BM: %s)", newfh.title + 13, newfh.BM);
                    }
                    else
                        sprintf(vbuf, "%-38.38s", newfh.title + 13);

                    if (flag == 1)
                        sprintf(genbuf, "ÈÎÃü %s µÄ°æÖ÷Îª %s", newfh.filename, lookupuser->userid);
                    else if (flag == 2)
                        sprintf(genbuf, "ÃâÈ¥ %s µÄ°æÖ÷ %s ", newfh.filename, lookupuser->userid);
                    securityreport(genbuf, lookupuser, NULL);
                    lookupuser->userlevel = newlevel;

                    edit_grp(fh.filename, oldtitle + 13, vbuf);
                    set_board(pos, &newfh, NULL);

                    sprintf(genbuf, "¸ü¸ÄÌÖÂÛÇø %s µÄ×ÊÁÏ --> %s", fh.filename, newfh.filename);
                    bbslog("user", "%s", genbuf);
                    strncpy(fh.BM, newfh.BM, BM_LEN - 1);
                }

            }
        }
    }
}
