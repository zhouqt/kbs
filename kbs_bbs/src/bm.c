
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
#include <time.h>
#include <ctype.h>

/*Add by SmallPig*/

int listdeny(int page)
{                               /* Haohmaru.12.18.98.Œ™ƒ«–©±‰Ã¨µ√∑‚»À≥¨π˝“ª∆¡µƒ∞Ê÷˜∂¯–¥ */
    FILE *fp;
    int x = 0, y = 3, cnt = 0, max = 0, len;
    int i;
    char u_buf[STRLEN * 2], line[STRLEN * 2], *nick;

    clear();
    prints("…Ë∂®Œﬁ∑® Post µƒ√˚µ•\n");
    move(y, x);
    CreateNameList();
    setbfile(genbuf, currboard->filename, "deny_users");
    if ((fp = fopen(genbuf, "r")) == NULL) {
        prints("(none)\n");
        return 0;
    }
    for (i = 1; i <= page * 20; i++) {
        if (fgets(genbuf, 2 * STRLEN, fp) == NULL)
            break;
    }
    while (fgets(genbuf, 2 * STRLEN, fp) != NULL) {
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
            if (cnt < 20)
                sprintf(line, "%-12s%s", u_buf, nick);
        }
        if ((len = strlen(line)) > max)
            max = len;
/*        if( x + len > 90 )
            line[ 90 - x ] = '\0';*-P-*/
        if (x + len > 79)
            line[79] = '\0';
        if (cnt < 20)           /*haohmaru.12.19.98 */
            prints("%s", line);
        cnt++;
        if ((++y) >= t_lines - 1) {
            y = 3;
            x += max + 2;
            max = 0;
            /*
             * if( x > 90 )  break; 
             */
        }
        move(y, x);
    }
    fclose(fp);
    if (cnt == 0)
        prints("(none)\n");
    return cnt;
}

int addtodeny(char *uident)
{                               /* ÃÌº” Ω˚÷πPOST”√ªß */
    char buf2[50], strtosave[256], date[STRLEN] = "0";
    int maxdeny;

    /*
     * Haohmaru.99.4.1.auto notify 
     */
    time_t now;
    char buffer[STRLEN];
    FILE *fn;
    char filename[STRLEN];
    int autofree = 0;
    char filebuf[STRLEN];
    char denymsg[STRLEN];
    int denyday;
    int reasonfile;

    now = time(0);
    strncpy(date, ctime(&now) + 4, 7);
    setbfile(genbuf, currboard->filename, "deny_users");
    if (seek_in_file(genbuf, uident) || !strcmp(currboard->filename, "denypost"))
        return -1;
    if (HAS_PERM(currentuser, PERM_SYSOP) || HAS_PERM(currentuser, PERM_OBOARDS))
        maxdeny = 70;
    else
        maxdeny = 14;

    *denymsg = 0;
    if ((reasonfile = open("etc/deny_reason", O_RDONLY)) != -1) {
        int reason = -1;
        int maxreason;
        char *file_buf;
        char *denymsglist[50];
        struct stat st;

        move(3, 0);
        clrtobot();
        if (fstat(reasonfile, &st) == 0) {
            int i;

            file_buf = (char *) malloc(st.st_size);
            read(reasonfile, file_buf, st.st_size);
            maxreason = 1;
            denymsglist[0] = file_buf;
            prints("%s", "∑‚Ω˚¿Ì”…¡–±Ì.\n");
            for (i = 1; i < st.st_size; i++) {
                if (file_buf[i] == '\n') {
                    file_buf[i] = 0;
                    prints("%d.%s\n", maxreason, denymsglist[maxreason - 1]);
                    if (i == st.st_size - 1)
                        break;
                    if (file_buf[i + 1] == '\r') {
                        if (i + 1 == st.st_size - 1)
                            break;
                        denymsglist[maxreason] = &file_buf[i + 2];
                        maxreason++;
                        i += 2;
                    } else {
                        denymsglist[maxreason] = &file_buf[i + 1];
                        maxreason++;
                        i++;
                    }
                }
            }
            prints("%s", "0. ÷∂Ø ‰»Î∑‚Ω˚¿Ì”…");
            while (1) {
                getdata(2, 0, "«Î¥”¡–±Ì—°‘Ò∑‚Ω˚¿Ì”…(0Œ™ ÷π§ ‰»Î,*ÕÀ≥ˆ):", denymsg, 2, DOECHO, NULL, true);
                if (denymsg[0] == '*') {
                    free(file_buf);
                    close(reasonfile);
                    return 0;
                }
                if (isdigit(denymsg[0])) {
                    reason = atoi(denymsg);
                    if (reason == 0) {
                        denymsg[0] = 0;
                        move(2, 0);
                        clrtobot();
                        break;
                    }
                    if (reason <= maxreason) {
                        strncpy(denymsg, denymsglist[reason - 1], STRLEN - 1);
                        denymsg[STRLEN - 1] = 0;
                        move(2, 0);
                        clrtobot();
                        prints("∑‚Ω˚¿Ì”…: %s\n", denymsg);
                        break;
                    }
                }
                move(3, 0);
                prints("%s", " ‰»Î¥ÌŒÛ!");
            }
            free(file_buf);
        }
        close(reasonfile);
    }

    while (0 == strlen(denymsg)) {
        getdata(2, 0, " ‰»ÎÀµ√˜(∞¥*»°œ˚): ", denymsg, 30, DOECHO, NULL, true);
    }
    if (denymsg[0] == '*')
        return 0;
#ifdef MANUAL_DENY
    autofree = askyn("∏√∑‚Ω˚ «∑Ò◊‘∂ØΩ‚∑‚£ø(—° [1;31mY[m ±Ì æΩ¯––◊‘∂ØΩ‚∑‚)", true);
#else
    autofree = true;
#endif
    sprintf(filebuf, " ‰»ÎÃÏ ˝(◊Ó≥§%dÃÏ)(∞¥*»°œ˚∑‚Ω˚)", maxdeny);
    denyday = 0;
    while (!denyday) {
        getdata(3, 0, filebuf, buf2, 4, DOECHO, NULL, true);
	if (buf2[0] == '*')return 0; 
        if ((buf2[0] < '0') || (buf2[0] > '9'))
            continue;           /*goto MUST1; */
        denyday = atoi(buf2);
        if ((denyday < 0) || (denyday > maxdeny))
            denyday = 0;        /*goto MUST1; */
        else if (!(HAS_PERM(currentuser, PERM_SYSOP) || HAS_PERM(currentuser, PERM_OBOARDS)) && !denyday)
            denyday = 0;        /*goto MUST1; */
        else if ((HAS_PERM(currentuser, PERM_SYSOP) || HAS_PERM(currentuser, PERM_OBOARDS)) && !denyday && !autofree)
            break;
    }

    if (denyday && autofree) {
        struct tm *tmtime;
        time_t undenytime = now + denyday * 24 * 60 * 60;

        tmtime = gmtime(&undenytime);

        sprintf(strtosave, "%-12.12s %-30.30s%-12.12s %2d‘¬%2d»’Ω‚\x1b[%lum", uident, denymsg, currentuser->userid, tmtime->tm_mon + 1, tmtime->tm_mday, undenytime);   /*Haohmaru 98,09,25,œ‘ æ «À≠ ≤√¥ ±∫Ú∑‚µƒ */
    } else {
        struct tm *tmtime;
        time_t undenytime = now + denyday * 24 * 60 * 60;

        tmtime = gmtime(&undenytime);
        sprintf(strtosave, "%-12.12s %-30.30s%-12.12s %2d‘¬%2d»’∫Û\x1b[%lum", uident, denymsg, currentuser->userid, tmtime->tm_mon + 1, tmtime->tm_mday, undenytime);
    }

    if (addtofile(genbuf, strtosave) == 1) {
        struct userec *lookupuser, *saveptr;
        int my_flag = 0;        /* Bigman. 2001.2.19 */
        struct userec saveuser;

        /*
         * Haohmaru.4.1.◊‘∂Ø∑¢–≈Õ®÷™≤¢∑¢Œƒ’¬”⁄∞Ê…œ 
         */
		gettmpfilename( filename, "deny" );
        //sprintf(filename, "tmp/%s.deny", currentuser->userid);
        fn = fopen(filename, "w+");
        memcpy(&saveuser, currentuser, sizeof(struct userec));
        saveptr = currentuser;
        currentuser = &saveuser;
        sprintf(buffer, "%s±ª»°œ˚‘⁄%s∞Êµƒ∑¢Œƒ»®œﬁ", uident, currboard->filename);

        if ((HAS_PERM(currentuser, PERM_SYSOP) || HAS_PERM(currentuser, PERM_OBOARDS)) && !chk_BM_instr(currBM, currentuser->userid)) {
            my_flag = 0;
            fprintf(fn, "ºƒ–≈»À: SYSOP (System Operator) \n");
            fprintf(fn, "±Í  Ã‚: %s\n", buffer);
            fprintf(fn, "∑¢–≈’æ: %s (%24.24s)\n", BBS_FULL_NAME, ctime(&now));
            fprintf(fn, "¿¥  ‘¥: %s\n", NAME_BBS_ENGLISH);
            fprintf(fn, "\n");
            fprintf(fn, "”…”⁄ƒ˙‘⁄ \x1b[4m%s\x1b[m ∞Ê \x1b[4m%s\x1b[m£¨Œ“∫‹“≈∫∂µÿÕ®÷™ƒ˙£¨ \n", currboard->filename, denymsg);
            if (denyday)
                fprintf(fn, "ƒ˙±ª‘› ±»°œ˚‘⁄∏√∞Êµƒ∑¢Œƒ»®¡¶ \x1b[4m%d\x1b[m ÃÏ", denyday);
            else
                fprintf(fn, "ƒ˙±ª‘› ±»°œ˚‘⁄∏√∞Êµƒ∑¢Œƒ»®¡¶");
            if (!autofree)
                fprintf(fn, "£¨µΩ∆⁄∫Û«Îªÿ∏¥\n¥À–≈…Í«Îª÷∏¥»®œﬁ°£\n");
            fprintf(fn, "\n");
            fprintf(fn, "                            " NAME_BBS_CHINESE NAME_SYSOP_GROUP "÷µ∞‡’æŒÒ£∫\x1b[4m%s\x1b[m\n", currentuser->userid);
            fprintf(fn, "                              %s\n", ctime(&now));
            strcpy(currentuser->userid, "SYSOP");
            strcpy(currentuser->username, NAME_SYSOP);
            /*strcpy(currentuser->realname, NAME_SYSOP);*/
        } else {
            my_flag = 1;
            fprintf(fn, "ºƒ–≈»À: %s \n", currentuser->userid);
            fprintf(fn, "±Í  Ã‚: %s\n", buffer);
            fprintf(fn, "∑¢–≈’æ: %s (%24.24s)\n", "BBS " NAME_BBS_CHINESE "’æ", ctime(&now));
            fprintf(fn, "¿¥  ‘¥: %s \n", fromhost);
            fprintf(fn, "\n");
            fprintf(fn, "”…”⁄ƒ˙‘⁄ \x1b[4m%s\x1b[m ∞Ê \x1b[4m%s\x1b[m£¨Œ“∫‹“≈∫∂µÿÕ®÷™ƒ˙£¨ \n", currboard->filename, denymsg);
            if (denyday)
                fprintf(fn, "ƒ˙±ª‘› ±»°œ˚‘⁄∏√∞Êµƒ∑¢Œƒ»®¡¶ \x1b[4m%d\x1b[m ÃÏ", denyday);
            else
                fprintf(fn, "ƒ˙±ª‘› ±»°œ˚‘⁄∏√∞Êµƒ∑¢Œƒ»®¡¶");
            if (!autofree)
                fprintf(fn, "£¨µΩ∆⁄∫Û«Îªÿ∏¥\n¥À–≈…Í«Îª÷∏¥»®œﬁ°£\n");
            fprintf(fn, "\n");
            fprintf(fn, "                              " NAME_BM ":\x1b[4m%s\x1b[m\n", currentuser->userid);
            fprintf(fn, "                              %s\n", ctime(&now));
        }
        fclose(fn);
        mail_file(currentuser->userid, filename, uident, buffer, 0, NULL);
        fn = fopen(filename, "w+");
        fprintf(fn, "”…”⁄ \x1b[4m%s\x1b[m ‘⁄ \x1b[4m%s\x1b[m ∞Êµƒ \x1b[4m%s\x1b[m ––Œ™£¨\n", uident, currboard->filename, denymsg);
        if (denyday)
            fprintf(fn, "±ª‘› ±»°œ˚‘⁄±æ∞Êµƒ∑¢Œƒ»®¡¶ \x1b[4m%d\x1b[m ÃÏ°£\n", denyday);
        else
            fprintf(fn, "ƒ˙±ª‘› ±»°œ˚‘⁄∏√∞Êµƒ∑¢Œƒ»®¡¶£¨µΩ∆⁄∫Û«Îªÿ∏¥\n");

        if (my_flag == 0) {
            fprintf(fn, "                            " NAME_BBS_CHINESE NAME_SYSOP_GROUP "÷µ∞‡’æŒÒ£∫\x1b[4m%s\x1b[m\n", saveptr->userid);
        } else {
            fprintf(fn, "                              " NAME_BM ":\x1b[4m%s\x1b[m\n", currentuser->userid);
        }
        fprintf(fn, "                              %s\n", ctime(&now));
        fclose(fn);
        post_file(currentuser, "", filename, currboard->filename, buffer, 0, 2);
        /*
         * unlink(filename); 
         */
        currentuser = saveptr;

        sprintf(buffer, "%s ±ª %s ∑‚Ω˚±æ∞ÊPOST»®", uident, currentuser->userid);
        getuser(uident, &lookupuser);

        if (PERM_BOARDS & lookupuser->userlevel)
            sprintf(buffer, "%s ∑‚ƒ≥∞Ê" NAME_BM " %s ‘⁄ %s", currentuser->userid, uident, currboard->filename);
        else
            sprintf(buffer, "%s ∑‚ %s ‘⁄ %s", currentuser->userid, uident, currboard->filename);
        post_file(currentuser, "", filename, "denypost", buffer, 0, 8);
        unlink(filename);
        bmlog(currentuser->userid, currboard->filename, 10, 1);
    }
    return 0;
}


int deny_user(int ent, struct fileheader *fileinfo, char *direct)
{                               /* Ω˚÷πPOST”√ªß√˚µ• Œ¨ª§÷˜∫Ø ˝ */
    char uident[STRLEN];
    int page = 0;
    char ans[10], repbuf[STRLEN];
    int count;

    /*
     * Haohmaru.99.4.1.auto notify 
     */
    time_t now;
    int id;
    FILE *fp;
    int find;                   /*Haohmaru.99.12.09 */

/*   static page=0; *//*
 * * Haohmaru.12.18 
 */
    now = time(0);
    if (!HAS_PERM(currentuser, PERM_SYSOP))
        if (!chk_currBM(currBM, currentuser)) {
            return DONOTHING;
        }

    while (1) {
        char querybuf[0xff];

      Here:
        clear();
        count = listdeny(0);
        if (count > 0 && count < 20)    /*Haohmaru.12.18,ø¥œ¬“ª∆¡ */
            snprintf(querybuf, 0xff, "(O)‘ˆº”%s (A)‘ˆº” (D)…æ≥˝ or (E)¿Îø™ [E]: ", fileinfo->owner);
        else if (count > 20)
            snprintf(querybuf, 0xff, "(O)‘ˆº”%s (A)‘ˆº” (D)…æ≥˝ (N)∫Û√Êµ⁄N∆¡ or (E)¿Îø™ [E]: ", fileinfo->owner);
        else
            snprintf(querybuf, 0xff, "(O)‘ˆº”%s (A)‘ˆº” or (E)¿Îø™ [E]: ", fileinfo->owner);

        getdata(1, 0, querybuf, ans, 7, DOECHO, NULL, true);
        *ans = (char) toupper((int) *ans);

        if (*ans == 'A' || *ans == 'O') {
            struct userec *denyuser;

            move(1, 0);
            if (*ans == 'A')
                usercomplete("‘ˆº”Œﬁ∑® POST µƒ π”√’ﬂ: ", uident);
            else
                strncpy(uident, fileinfo->owner, STRLEN - 4);
            /*
             * Haohmaru.99.4.1,‘ˆº”±ª∑‚ID’˝»∑–‘ºÏ≤È 
             */
            if (!(id = getuser(uident, &denyuser))) {   /* change getuser -> searchuser , by dong, 1999.10.26 */
                move(3, 0);
                prints("∑«∑® ID");
                clrtoeol();
                pressreturn();
                goto Here;
            }
            strncpy(uident, denyuser->userid, IDLEN);
            uident[IDLEN] = 0;

            if (*uident != '\0') {
                addtodeny(uident);
            }
        } else if ((*ans == 'D') && count) {
            int len;

            move(1, 0);
            sprintf(genbuf, "…æ≥˝Œﬁ∑® POST µƒ π”√’ﬂ: ");
            getdata(1, 0, genbuf, uident, 13, DOECHO, NULL, true);
            find = 0;           /*Haohmaru.99.12.09.‘≠¿¥µƒ¥˙¬Î»Áπ˚±ª∑‚’ﬂ“—◊‘…±æÕ…æ≤ªµÙ¡À */
            setbfile(genbuf, currboard->filename, "deny_users");
            if ((fp = fopen(genbuf, "r")) == NULL) {
                prints("(none)\n");
                return 0;
            }
            len = strlen(uident);
            while (fgets(genbuf, 256 /*STRLEN*/, fp) != NULL) {
                if (!strncasecmp(genbuf, uident, len)) {
                    if (genbuf[len] == 32) {
                        strncpy(uident, genbuf, len);
                        uident[len] = 0;
                        find = 1;
                        break;
                    }
                }
            }
            fclose(fp);
            if (!find) {
                move(3, 0);
                prints("∏√ID≤ª‘⁄∑‚Ω˚√˚µ•ƒ⁄!");
                clrtoeol();
                pressreturn();
                goto Here;
            }
            /*---	add to check if undeny time reached.	by period 2000-09-11	---*/
            {
                char *lptr;
                time_t ldenytime;

                /*
                 * now the corresponding line in genbuf 
                 */
                if (NULL != (lptr = strrchr(genbuf, '[')))
                    sscanf(lptr + 1, "%lu", &ldenytime);
                else
                    ldenytime = now + 1;
                if (ldenytime > now) {
                    move(3, 0);
                    prints(genbuf);
                    if (false == askyn("∏√”√ªß∑‚Ω˚ ±œﬁŒ¥µΩ£¨»∑»œ“™Ω‚∑‚£ø", false /*, false */ ))
                        goto Here;      /* I hate Goto!!! */
                }
            }
            /*---		---*/
            move(1, 0);
            clrtoeol();
            if (uident[0] != '\0') {
                if (deldeny(currentuser, currboard->filename, uident, 0)) {
                }
            }
        } else if (count > 20 && isdigit(ans[0])) {
            if (ans[1] == 0)
                page = *ans - '0';
            else
                page = atoi(ans);
            if (page < 0)
                break;          /*≤ªª·∑‚»À≥¨π˝10∆¡∞…?ƒ«ø… «200»À∞°!  ª·µƒ£° */
            listdeny(page);
            pressanykey();
        } else
            break;
    }                           /*end of while */
    clear();
    return FULLUPDATE;
}

int addclubmember(char *uident, int readperm)
{
    char genbuf1[80], genbuf2[80];
    int id;
    int i;
    char ans[8];
    int seek;
    struct userec *lookupuser;

    if (!(id = getuser(uident, &lookupuser))) {
        move(3, 0);
        prints("Invalid User Id");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    strcpy(uident, lookupuser->userid);
    if (readperm)
        setbfile(genbuf, currboard->filename, "read_club_users");
    else
        setbfile(genbuf, currboard->filename, "write_club_users");

	seek = seek_in_file(genbuf, uident);
	if (seek) {
		move(2, 0);
		prints(" ‰»ÎµƒID “—æ≠¥Ê‘⁄!");
		pressreturn();
		return -1;
	}

	getdata(4, 0, "’Êµƒ“™ÃÌº”√¥?[Y/N]: ", ans, 7, 1, NULL, true);
	if ((*ans != 'Y') && (*ans != 'y'))
		return -1;
		
//    if ((i = getboardnum(currboard->filename, &bh)) == 0)
//        return DONOTHING;
    i=currboardent;
    seek = addtofile(genbuf, uident);;
    if (seek == 1) {
        if (readperm == 1)      /*∂¡»®œﬁ*/
            lookupuser->club_read_rights[(currboard->clubnum - 1) >> 5] |= 1 << ((currboard->clubnum - 1) & 0x1f);
        else
            lookupuser->club_write_rights[(currboard->clubnum - 1) >> 5] |= 1 << ((currboard->clubnum - 1) & 0x1f);
        return FULLUPDATE;
    }
    return DONOTHING;

}

int delclubmember(char *uident, int readperm)
{
    char genbuf1[80], genbuf2[80];
    char fn[STRLEN];
    int id;
    int i;
    int ret;
    struct userec *lookupuser;

    if (!(id = getuser(uident, &lookupuser))) {
        move(3, 0);
        prints("Invalid User Id");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    strcpy(uident, lookupuser->userid);
    i=currboardent;
    if (readperm)
        setbfile(fn, currboard->filename, "read_club_users");
    else
        setbfile(fn, currboard->filename, "write_club_users");
    ret = del_from_file(fn, uident);
    if (ret == 0) {
        if (readperm == 1)      /*∂¡»®œﬁ*/
            lookupuser->club_read_rights[(currboard->clubnum - 1) >> 5] &= ~(1 << ((currboard->clubnum - 1) & 0x1f));
        else
            lookupuser->club_write_rights[(currboard->clubnum - 1) >> 5] &= ~(1 << ((currboard->clubnum - 1) & 0x1f));
        return FULLUPDATE;
    }
    return DONOTHING;
}

int clubmember(int ent, struct fileheader *fh, char *direct)
{
    char uident[STRLEN];
    char ans[8], buf[STRLEN];
    int count, i;
    int readperm;
    static char comment[STRLEN]={0};    /* add an additional comment for club .added by binxun . 2003.7.7*/
    char tempbuf[256];

    if (!(chk_currBM(currBM, currentuser))) {
        return DONOTHING;
    }
    i=currboardent;
    if ((!(currboard->flag & BOARD_CLUB_READ) && !(currboard->flag & BOARD_CLUB_WRITE)) || currboard->clubnum <= 0 || currboard->clubnum >= MAXCLUB)
        return DONOTHING;
    if ((currboard->flag & BOARD_CLUB_READ) && (currboard->flag & BOARD_CLUB_WRITE)) {
        int choose;
        int left = (80 - 24) / 2;
        int top = (scr_lns - 11) / 2;
        struct _select_item menu_conf[] = {
            {left, top + 2, 'R', SIT_SELECT, (void *) "[R] …Ë∂®ø…∂¡»°”√ªß√˚µ•"},
            {left, top + 3, 'P', SIT_SELECT, (void *) "[P] …Ë∂®ø…∑¢±Ì”√ªß√˚µ•"},
            {-1, -1, -1, 0, NULL}
        };

        clear();
        choose = simple_select_loop(menu_conf, SIF_SINGLE | SIF_NUMBERKEY, t_columns, t_lines, NULL);
        if (choose == 1)
            readperm = 1;
        else
            readperm = 0;
    } else if (currboard->flag & BOARD_CLUB_READ)
        readperm = 1;
    else
        readperm = 0;
    if (readperm)
        setbfile(buf, currboard->filename, "read_club_users");
    else
        setbfile(buf, currboard->filename, "write_club_users");
    ansimore(buf, true);
    while (1) {
        clear();
        prints("…Ë∂®æ„¿÷≤ø√˚µ•\n");
        count = listfilecontent(buf);
        if (count)
            getdata(1, 0, "(A)‘ˆº” (D)…æ≥˝or (E)¿Îø™[E]",
                    ans, 7, DOECHO, NULL, true);
        else
            getdata(1, 0, "(A)‘ˆº” or (E)¿Îø™ [E]: ", ans, 7, DOECHO, NULL, true);
        if (*ans == 'A' || *ans == 'a') {
            move(1, 0);
            usercomplete("‘ˆº”æ„¿÷≤ø≥…‘±: ", uident);
            if (*uident != '\0') {
                if (addclubmember(uident, readperm) == 1) {
			 /* add an additional comment for club .added by binxun . 2003.7.7*/                	
                    sprintf(genbuf,"∏Ωº”Àµ√˜:[%s]\n",comment);
                    getdata(2,0,genbuf,tempbuf,STRLEN,DOECHO,NULL,true);
                    if(tempbuf[0]){
	                    strncpy(comment,tempbuf,STRLEN-1);
       	             comment[STRLEN-1] = 0;
                    	}
                    sprintf(tempbuf,"∏Ωº”Àµ√˜:%s",comment);
                    sprintf(genbuf, "%s ”… %s  ⁄”Ë %s æ„¿÷≤ø»®¡¶", uident, currentuser->userid, currboard->filename);
                    /*securityreport(genbuf, NULL, NULL);*/
                    mail_buf(currentuser, tempbuf, uident, genbuf);
                    deliverreport(genbuf, tempbuf);
                }
            }
        } else if ((*ans == 'D' || *ans == 'd') && count) {
            move(1, 0);
            namecomplete("…æ≥˝æ„¿÷≤ø π”√’ﬂ: ", uident);
            move(1, 0);
            clrtoeol();
            if (uident[0] != '\0') {
                sprintf(genbuf, "’Êµƒ“™»°œ˚%sµƒæ„¿÷≤ø»®¡¶√¥£ø", uident);
                if (askyn(genbuf, true))
                    if (delclubmember(uident, readperm)) {
                    		 /* add an additional comment for club .added by binxun . 2003.7.7*/                	
	                    sprintf(genbuf,"∏Ωº”Àµ√˜:[%s]\n",comment);
	   	             getdata(2,0,genbuf,tempbuf,STRLEN,DOECHO,NULL,true);
	          	      if(tempbuf[0]){
	              	      strncpy(comment,tempbuf,STRLEN-1);
	       	             comment[STRLEN-1] = 0;
	   	          	}
	              	sprintf(tempbuf,"∏Ωº”Àµ√˜:%s",comment);

	                     sprintf(genbuf, " %s ±ª %s »°œ˚ %s æ„¿÷≤ø»®¡¶", uident, currentuser->userid, currboard->filename);
	                     /*securityreport(genbuf, NULL, NULL);*/
	                    	mail_buf(currentuser, tempbuf, uident, genbuf);
	                    	deliverreport(genbuf, tempbuf);
                    }
            }
        }
        /*
         * else if ((*ans == 'M' || *ans == 'm') && count) {
         * club_send();
         * }
         */
        else
            break;

    }
    clear();
    return FULLUPDATE;
}
