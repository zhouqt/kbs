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

char            cexplain[STRLEN];
char           *Ctime();
char            lookgrp[30];
static	int sysoppassed=0;

/* modified by wwj, 2001/5/7, for new md5 passwd */
void igenpass(const char *passwd,const char *userid,unsigned char md5passwd[]);

int check_systempasswd()
{
    FILE           *pass;
    char            passbuf[40], prepass[STRLEN];

    if ((sysoppassed)&&(time(NULL)-sysoppassed<60*60)) return true;
    clear();
    if ((pass = fopen("etc/systempassword", "rb")) != NULL)
    {
        fgets(prepass, STRLEN, pass);
        prepass[strlen(prepass) - 1] = '\0';
        if(!strcmp(prepass,"md5")){
            fread(&prepass[16],1,16,pass);
        }
        fclose(pass);
        
        getdata(1, 0, "ÇëÊäÈëÏµÍ³ÃÜÂë: ", passbuf, 39, NOECHO, NULL, true);
        if (passbuf[0] == '\0' || passbuf[0] == '\n')
            return false;
            
        
        if(!strcmp(prepass,"md5")){
            igenpass(passbuf,"[system]",(unsigned char *)prepass);
            passbuf[0]=(char)!memcmp(prepass,&prepass[16],16);
        } else {
            passbuf[0]=(char)checkpasswd(prepass, passbuf);
        }
        if (!passbuf[0]) {
            move(2, 0);
            prints( MSG_ERR_USERID );
            securityreport("ÏµÍ³ÃÜÂëÊäÈë´íÎó...",NULL,NULL);
            pressanykey();
            return false;
        }
    }
    sysoppassed=time(NULL);
    return true;
}

int setsystempasswd()
{
    FILE           *pass;
    char            passbuf[40], prepass[40];

    modify_user_mode(ADMIN);
    if (strcmp(currentuser->userid, "SYSOP"))
        return -1;
    if (!check_systempasswd())
        return -1;
    getdata(2, 0, "ÇëÊäÈëĞÂµÄÏµÍ³ÃÜÂë: ", passbuf, 39, NOECHO, NULL, true);
    getdata(3, 0, "È·ÈÏĞÂµÄÏµÍ³ÃÜÂë: ", prepass, 39, NOECHO, NULL, true);
    if (strcmp(passbuf, prepass))
        return -1;
    if ((pass = fopen("etc/systempassword", "w")) == NULL)
    {
        move(4, 0);
        prints("ÏµÍ³ÃÜÂëÎŞ·¨Éè¶¨....");
        pressanykey();
        return -1;
    }
    fwrite("md5\n",4,1,pass);
    
    igenpass(passbuf,"[system]",(unsigned char *)prepass);
    fwrite(prepass,16,1,pass);
    
    fclose(pass);
    move(4, 0);
    prints("ÏµÍ³ÃÜÂëÉè¶¨Íê³É....");
    pressanykey();
    return 0;
}



void securityreport(char *str,struct userec* lookupuser,char fdata[ 7 ][ STRLEN ])		/* Leeward: 1997.12.02 */
{
    FILE           *se;
    char            fname[STRLEN];
    int             savemode;
    char           *ptr;

    savemode = uinfo.mode;
    sprintf(fname, "tmp/security.%d", getpid());
    if ((se = fopen(fname, "w")) != NULL)
    {
    	if (lookupuser) {
	        if (strstr(str, "ÈÃ") && strstr(str, "Í¨¹ıÉí·İÈ·ÈÏ"))
	        {
	            fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\n[32mÔ­Òò£º%s[m\n", str);
	            fprintf(se, "ÒÔÏÂÊÇÍ¨¹ıÕß¸öÈË×ÊÁÏ");
	            /*    getuinfo(se, lookupuser);*/
	            /*Haohmaru.99.4.15.°Ñ±»×¢²áµÄ×ÊÁÏÁĞµÃ¸üÏêÏ¸,Í¬Ê±È¥µô×¢²áÕßµÄ×ÊÁÏ*/
	            fprintf(se,"\n\nÄúµÄ´úºÅ     : %s\n",fdata[1]);
	            fprintf(se,"ÄúµÄêÇ³Æ     : %s\n",lookupuser->username);
	            fprintf(se,"ÕæÊµĞÕÃû     : %s\n",fdata[2]);
	            fprintf(se,"µç×ÓÓÊ¼şĞÅÏä : %s\n",lookupuser->email);
	            fprintf(se,"ÕæÊµ E-mail  : %s$%s@%s\n",fdata[3],fdata[5],currentuser->userid);
	            fprintf(se,"·şÎñµ¥Î»     : %s\n",fdata[3]);
	            fprintf(se,"Ä¿Ç°×¡Ö·     : %s\n",fdata[4]);
	            fprintf(se,"Á¬Âçµç»°     : %s\n",fdata[5]);
	            fprintf(se,"×¢²áÈÕÆÚ     : %s", ctime( &lookupuser->firstlogin));
	            fprintf(se,"×î½ü¹âÁÙÈÕÆÚ : %s", ctime( &lookupuser->lastlogin));
	            fprintf(se,"×î½ü¹âÁÙ»úÆ÷ : %s\n", lookupuser->lasthost );
	            fprintf(se,"ÉÏÕ¾´ÎÊı     : %d ´Î\n", lookupuser->numlogins);
	            fprintf(se,"ÎÄÕÂÊıÄ¿     : %d(Board)\n",lookupuser->numposts);
	            fprintf(se,"Éú    ÈÕ     : %s\n",fdata[6]);
	            /*    fprintf(se, "\n[33mÒÔÏÂÊÇÈÏÖ¤Õß¸öÈË×ÊÁÏ[35m");
	                getuinfo(se, currentuser);rem by Haohmaru.99.4.16*/
	            fclose(se);
	            post_file(currentuser,"",fname, "Registry", str, 0, 2);
	        }
	        else if (strstr(str, "É¾³ıÊ¹ÓÃÕß£º"))
	        {
	            fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\n[32mÔ­Òò£º%s[m\n", str);
	            fprintf(se, "ÒÔÏÂÊÇ±»É¾Õß¸öÈË×ÊÁÏ");
	            getuinfo(se, lookupuser);
	            fprintf(se, "\nÒÔÏÂÊÇÉ¾³ıÕß¸öÈË×ÊÁÏ");
	            getuinfo(se, currentuser);
	            fclose(se);
	            post_file(currentuser,"", fname, "syssecurity", str, 0, 2);
	        }
	        else if ((ptr = strstr(str, "µÄÈ¨ÏŞXPERM"))!=NULL)
	        {
	            int             oldXPERM, newXPERM;
	            int             num;
	            char            XPERM[48];

	            sscanf(ptr + strlen("µÄÈ¨ÏŞXPERM"), "%d %d",
	                   &oldXPERM, &newXPERM);
	            *(ptr + strlen("µÄÈ¨ÏŞ")) = 0;

	            fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\n[32mÔ­Òò£º%s[m\n", str);

	            strcpy(XPERM, XPERMSTR);
	            for (num = 0; num < (int)strlen(XPERM); num++)
	                if (!(oldXPERM & (1 << num)))
	                    XPERM[num] = ' ';
	            XPERM[num] = '\0';
	            fprintf(se, "ÒÔÏÂÊÇ±»¸ÄÕßÔ­À´µÄÈ¨ÏŞ\n\033[1m\033[33m%s", XPERM);

	            strcpy(XPERM, XPERMSTR);
	            for (num = 0; num < (int)strlen(XPERM); num++)
	                if (!(newXPERM & (1 << num)))
	                    XPERM[num] = ' ';
	            XPERM[num] = '\0';
	            fprintf(se, "\n%s\033[0m\nÒÔÉÏÊÇ±»¸ÄÕßÏÖÔÚµÄÈ¨ÏŞ\n", XPERM);

	            fprintf(se, "\n"
	                    "\033[1m\033[33mb\033[0m»ù±¾È¨Á¦ \033[1m\033[33mT\033[0m½øÁÄÌìÊÒ \033[1m\033[33mC\033[0mºô½ĞÁÄÌì \033[1m\033[33mP\033[0m·¢ÎÄÕÂ \033[1m\033[33mR\033[0m×ÊÁÏÕıÈ· \033[1m\033[33mp\033[0m±»½û·¢ÎÄ \033[1m\033[33m#\033[0m¿ÉÒşÉí \033[1m\033[33m@\033[0m¿É¼ûÒşÉí\n"
	                    "\033[1m\033[33mX\033[0m³¤ÆÚÕÊºÅ \033[1m\033[33mW\033[0m±à¼­ÏµÍ³µµ°¸ \033[1m\033[33mB\033[0m°åÖ÷ \033[1m\033[33mA\033[0mÕÊºÅ¹ÜÀí \033[1m\033[33m$\033[0mÖÇÄÒÍÅ \033[1m\033[33mV\033[0mÍ¶Æ±¹ÜÀí \033[1m\033[33mS\033[0mÏµÍ³Î¬»¤\n"
	                    "\033[1m\033[33m!\033[0mRead/PostÏŞÖÆ \033[1m\033[33mD\033[0m¾«»ªÇø×Ü¹Ü \033[1m\033[33mE\033[0mÌÖÂÛÇø×Ü¹Ü \033[1m\033[33mM\033[0m»î¶¯¿´°æ×Ü¹Ü \033[1m\033[33m1\033[0m²»ÄÜZAP \033[1m\033[33m2\033[0mÁÄÌìÊÒOP\n"
	                    "\033[1m\033[33m3\033[0mÏµÍ³×Ü¹ÜÀíÔ± \033[1m\033[33m4\033[0mÈÙÓşÕÊºÅ \033[1m\033[33m5->9\033[0m ÌØÊâÈ¨ÏŞ5->9 \033[1m\033[33m0\033[0m¿´ÏµÍ³ÌÖÂÛ°æ \033[1m\033[33m%%\033[0m·â½ûMail"
	                    "\n");

	            fprintf(se, "\nÒÔÏÂÊÇ±»¸ÄÕß¸öÈË×ÊÁÏ");
	            getuinfo(se, lookupuser);
	            fprintf(se, "\nÒÔÏÂÊÇĞŞ¸ÄÕß¸öÈË×ÊÁÏ");
	            getuinfo(se, currentuser);
	            fclose(se);
	            post_file(currentuser,"",fname, "syssecurity", str, 0, 2);
	        }
                else    /* Modified for change id by Bigman 2001.5.25 */                
                {                                                                       
                      fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\n^[[32mÔ­Òò£º%s^[[m\n", str);     
                      fprintf(se, "ÒÔÏÂÊÇ¸öÈË×ÊÁÏ");                                  
                      getuinfo(se, currentuser);                                      
                      fclose(se);                                                     
                      post_file(currentuser,"",fname, "syssecurity", str, 0, 2);
                }                                                                       
    	}
        else
        {
            fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\n[32mÔ­Òò£º%s[m\n", str);
            fprintf(se, "ÒÔÏÂÊÇ¸öÈË×ÊÁÏ");
            getuinfo(se, currentuser);
            fclose(se);
            if (strstr(str, "Éè¶¨Ê¹ÓÃÕß×¢²á×ÊÁÏ"))	/* Leeward 98.03.29 */
                post_file(currentuser,"" ,fname, "Registry", str, 0, 2);
            else
                post_file(currentuser,"",fname, "syssecurity", str, 0, 2);
        }
        unlink(fname);
        modify_user_mode(savemode);
    }
}

int get_grp(seekstr)
char            seekstr[STRLEN];
{
    FILE           *fp;
    char            buf[STRLEN];
    char           *namep;

    if ((fp = fopen("0Announce/.Search", "r")) == NULL)
        return 0;
    while (fgets(buf, STRLEN, fp) != NULL)
    {
        namep = strtok(buf, ": \n\r\t");
        if (namep != NULL && strcasecmp(namep, seekstr) == 0)
        {
            fclose(fp);
            strtok(NULL, "/");
            namep = strtok(NULL, "/");
            strcpy(lookgrp, namep);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void stand_title(title)
char           *title;
{
    clear();
    standout();
    prints(title);
    standend();
}

int m_info()
{
    struct userec   uinfo;
    int             id;
    struct userec* lookupuser;


    modify_user_mode(ADMIN);
    if (!check_systempasswd())	/* Haohmaru.98.12.19 */
    {
        return -1;
    }
    clear();
    stand_title("ĞŞ¸ÄÊ¹ÓÃÕß´úºÅ");
    move(1, 0);
    usercomplete("ÇëÊäÈëÊ¹ÓÃÕß´úºÅ: ", genbuf);
    if (*genbuf == '\0')
    {
        clear();
        return -1;
    }
    if (!(id = getuser(genbuf,&lookupuser)))
    {
        move(3, 0);
        prints( MSG_ERR_USERID );
        clrtoeol();
        pressreturn();
        clear();
        return -1;
    }
    uinfo=*lookupuser;

    move(1, 0);
    clrtobot();
    disply_userinfo(&uinfo, 1);
    uinfo_query(&uinfo, 1, id);
    return 0;
}

extern int      cmpbnames();

int valid_brdname(brd)
char           *brd;
{
    char            ch;

    ch = *brd++;
    if (!isalnum(ch) && ch != '_')
        return 0;
    while ((ch = *brd++) != '\0')
    {
        if (!isalnum(ch) && ch != '_' && ch != '.')
            return 0;
    }
    return 1;
}

char           *
chgrp()
{
    int             i, ch;
    char            buf[STRLEN], ans[6];

    /*
static char    *explain[] = {
    "±¾Õ¾ÏµÍ³",
    "ĞİÏĞÓéÀÖ",
    "µçÄÔ¼¼Êõ",
    "Ñ§Êõ¿ÆÑ§",
    "ÌåÓı½¡Éí",
    "Ì¸ÌìËµµØ",
    "Ğ£Ô°ĞÅÏ¢",
    "ÒÕÊõÎÄ»¯",
    "ÈËÎÄÉç»á",
    "ÍøÂçĞÅÏ¢",
    "Çå»ª´óÑ§",
    "ĞÖµÜÔºĞ£",
    "Æä  Ëû",
    NULL
};

static char    *groups[] = {
    "system.faq",
    "rec.faq",
    "comp.faq",
    "sci.faq",
    "sport.faq",
    "talk.faq",
    "campus.faq",
    "literal.faq",
    "soc.faq",
    "network.faq",
    "thu.faq",
    "univ.faq",
    "other.faq",
    NULL
};
*/

    clear();
    move(2, 0);
    prints("Ñ¡Ôñ¾«»ªÇøµÄÄ¿Â¼\n");
    oflush();

    for (i = 0;; i++)
    {
        if (explain[i] == NULL || groups[i] == NULL)
            break;
        prints("[32m%2d[m. %-20s%-20s\n", i, explain[i], groups[i]);
    }
    sprintf(buf, "ÇëÊäÈëÄãµÄÑ¡Ôñ(0~%d): ", i-1);
    while (1)
    {
        getdata(i + 3, 0, buf, ans, 4, DOECHO, NULL, true);
        if (!isdigit(ans[0]))
            continue;
        ch = atoi(ans);
        if (ch < 0 || ch >= i || ans[0] == '\r' || ans[0] == '\0')
            continue;
        else
            break;
    }
    sprintf(cexplain, "%s", explain[ch]);

    return groups[ch];
}


int m_newbrd()
{
    struct boardheader newboard;
    char            ans[5];
    char            vbuf[100];
    char           *group;


    modify_user_mode(ADMIN);
    if (!check_systempasswd())
    {
        return -1;
    }
    clear();
    memset(&newboard, 0, sizeof(newboard));
    prints("¿ªÆôĞÂÌÖÂÛÇø:");
    while (1)
    {
        getdata(3, 0, "ÌÖÂÛÇøÃû³Æ:   ", newboard.filename, 18, DOECHO, NULL, true);
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
    if (getbnum(newboard.filename) > 0 || mkdir(genbuf, 0755) == -1
            || mkdir(vbuf, 0755) == -1)
    {
        prints("\n´íÎó£º´íÎóµÄÌÖÂÛÇøÃû³Æ\n");
        pressreturn();
        clear();
        return -1;
    }
    newboard.flag = 0;
    getdata(5, 0, "ÌÖÂÛÇø¹ÜÀíÔ±: ", newboard.BM, BM_LEN - 1, DOECHO, NULL, true);
    getdata(6, 0, "ÊÇ·ñÏŞÖÆ´æÈ¡È¨Á¦ (Y/N)? [N]: ", ans, 4, DOECHO, NULL, true);
    if (*ans == 'y' || *ans == 'Y')
    {
        getdata(6, 0, "ÏŞÖÆ Read/Post? [R]: ", ans, 4, DOECHO, NULL, true);
        if (*ans == 'P' || *ans == 'p')
            newboard.level = PERM_POSTMASK;
        else
            newboard.level = 0;
        move(1, 0);
        clrtobot();
        move(2, 0);
        prints("Éè¶¨ %s È¨Á¦. ÌÖÂÛÇø: '%s'\n",
               (newboard.level & PERM_POSTMASK ? "POST" : "READ"),
               newboard.filename);
        newboard.level = setperms(newboard.level, 0, "È¨ÏŞ", NUMPERMS, showperminfo);
        clear();
    }
    else
        newboard.level = 0;
    getdata(7, 0, "ÊÇ·ñ¼ÓÈëÄäÃû°å (Y/N)? [N]: ", ans, 4, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y') {
    	newboard.flag|=BOARD_ANNONY;
        addtofile("etc/anonymous", newboard.filename);
    }
    getdata(8, 0, "ÊÇ·ñ²»¼ÇÎÄÕÂÊı(Y/N)? [N]: ", ans, 4, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y')
        newboard.flag|=BOARD_JUNK;
    getdata(9, 0, "ÊÇ·ñ¿ÉÏòÍâ×ªĞÅ(Y/N)? [N]: ", ans, 4, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y')
        newboard.flag|=BOARD_OUTFLAG;
    if (add_board(&newboard)==-1) {
    	move(t_lines-1,0);
    	outs("¼ÓÈëÌÖÂÛÇøÊ§°Ü!\n");
        pressreturn();
        clear();
        return -1;
    }
    group = chgrp();
    if (group != NULL)
    {
        if (newboard.BM[0] != '\0')
            sprintf(vbuf, "%-38.38s(BM: %s)", newboard.title + 13, newboard.BM);
        else
            sprintf(vbuf, "%-38.38s", newboard.title + 13);

        if (add_grp(group, newboard.filename, vbuf, cexplain) == -1)
            prints("\n³ÉÁ¢¾«»ªÇøÊ§°Ü....\n");
        else
            prints("ÒÑ¾­ÖÃÈë¾«»ªÇø...\n");
    }
    prints("\nĞÂÌÖÂÛÇø³ÉÁ¢\n");
    sprintf(genbuf, "add brd %s", newboard.filename);
    report(genbuf);
    {
        char            secu[STRLEN];
        sprintf(secu, "³ÉÁ¢ĞÂ°å£º%s", newboard.filename);
        securityreport(secu,NULL,NULL);
    }
    pressreturn();
    clear();
    return 0;
}

int m_editbrd()
{
    char   bname[STRLEN], buf[STRLEN], oldtitle[STRLEN], vbuf[256], *group;
    char   oldpath[STRLEN], newpath[STRLEN], tmp_grp[30];
    int    pos, noidboard, a_mv;
    struct boardheader fh, newfh;

    modify_user_mode(ADMIN);
    if (!check_systempasswd())
    {
        return -1;
    }
    clear();
    stand_title("ĞŞ¸ÄÌÖÂÛÇø×ÊÑ¶");
    move(1, 0);
    make_blist();
    namecomplete("ÊäÈëÌÖÂÛÇøÃû³Æ: ", bname);
    if (*bname == '\0')
    {
        move(2, 0);
        prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
        pressreturn();
        clear();
        return -1;
    }
    pos = getboardnum(bname,&fh);
    if (!pos)
    {
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
    prints("ÄäÃûÌÖÂÛÇø:   %s\n", (noidboard) ? "Yes" : "No");
    prints("²»¼ÇÎÄÕÂÊı:   %s\n", (fh.flag&BOARD_JUNK) ? "Yes" : "No");
    prints("¿ÉÏòÍâ×ªĞÅ:   %s\n", (fh.flag&BOARD_OUTFLAG) ? "Yes" : "No");
    strcpy(oldtitle, fh.title);
    prints("ÏŞÖÆ %s È¨Á¦: %s", (fh.level & PERM_POSTMASK) ? "POST" : "READ",
           (fh.level & ~PERM_POSTMASK) == 0 ? "²»ÉèÏŞ" : "ÓĞÉèÏŞ");
    getdata(9, 0, "ÊÇ·ñ¸ü¸ÄÒÔÉÏ×ÊÑ¶? (Yes or No) [N]: ", genbuf, 4, DOECHO, NULL, true);
    if (*genbuf == 'y' || *genbuf == 'Y')
    {
        move(8, 0);
        prints("Ö±½Ó°´ <Return> ²»ĞŞ¸Ä´ËÀ¸×ÊÑ¶\n");
enterbname:
        getdata(9, 0, "ĞÂÌÖÂÛÇøÃû³Æ: ", genbuf, 18, DOECHO, NULL, true);
        if (*genbuf != 0)
        {
            if (getboardnum(genbuf,NULL)>0)
            {
                move(3, 0);
                prints("´íÎó! ´ËÌÖÂÛÇøÒÑ¾­´æÔÚ\n");
                move(10, 0);
                clrtobot();
                goto enterbname;
            }
            strncpy(newfh.filename, genbuf, sizeof(newfh.filename));
            strcpy(bname, genbuf);
        }
        getdata(10, 0, "ĞÂÌÖÂÛÇøËµÃ÷: ", genbuf, 60, DOECHO, NULL, true);
        if (*genbuf != 0)
            strncpy(newfh.title, genbuf, sizeof(newfh.title));
        getdata(11, 0, "ÌÖÂÛÇø¹ÜÀíÔ±: ", genbuf, 60, DOECHO, NULL, true);
        if (*genbuf != 0)
            strncpy(newfh.BM, genbuf, sizeof(newfh.BM));
        if (*genbuf == ' ')
            strncpy(newfh.BM, "\0", sizeof(newfh.BM));
        /* newfh.BM[ BM_LEN - 1 ]=fh.BM[ BM_LEN - 1 ]; */
        sprintf(buf, "ÄäÃû°æ (Y/N)? [%c]: ", (noidboard) ? 'Y' : 'N');
        getdata(12, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'y' || *genbuf == 'Y' || *genbuf == 'N' || *genbuf == 'n')
        {
            if (*genbuf == 'y' || *genbuf == 'Y')
                noidboard = 1;
            else
                noidboard = 0;
        }
        sprintf(buf, "²»¼ÇÎÄÕÂÊı (Y/N)? [%c]: ", (newfh.flag&BOARD_JUNK) ? 'Y' : 'N');
        getdata(13, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'y' || *genbuf == 'Y' || *genbuf == 'N' || *genbuf == 'n')
        {
            if (*genbuf == 'y' || *genbuf == 'Y')
        		newfh.flag|=BOARD_JUNK;
            else
        		newfh.flag&=~BOARD_JUNK;
        };
        sprintf(buf, "¿ÉÏòÍâ×ªĞÅ (Y/N)? [%c]: ", (newfh.flag&BOARD_OUTFLAG) ? 'Y' : 'N');
        getdata(14, 0, buf, genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'y' || *genbuf == 'Y' || *genbuf == 'N' || *genbuf == 'n')
        {
            if (*genbuf == 'y' || *genbuf == 'Y')
        		newfh.flag|=BOARD_OUTFLAG;
            else
        		newfh.flag&=~BOARD_OUTFLAG;
        };
        getdata(15, 0, "ÊÇ·ñÒÆ¶¯¾«»ªÇøµÄÎ»ÖÃ (Y/N)? [N]: ", genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'Y' || *genbuf == 'y')
            a_mv = 2;
        else
            a_mv = 0;
        getdata(16, 0, "ÊÇ·ñ¸ü¸Ä´æÈ¡È¨ÏŞ (Y/N)? [N]: ", genbuf, 4, DOECHO, NULL, true);
        if (*genbuf == 'Y' || *genbuf == 'y')
        {
            char            ans[5];
            sprintf(genbuf, "ÏŞÖÆ (R)ÔÄ¶Á »ò (P)ÕÅÌù ÎÄÕÂ [%c]: ",
                    (newfh.level & PERM_POSTMASK ? 'P' : 'R'));
            getdata(17, 0, genbuf, ans, 4, DOECHO, NULL, true);
            if ((newfh.level & PERM_POSTMASK) && (*ans == 'R' || *ans == 'r'))
                newfh.level &= ~PERM_POSTMASK;
            else
                if (!(newfh.level & PERM_POSTMASK) && (*ans == 'P' || *ans == 'p'))
                    newfh.level |= PERM_POSTMASK;
            move(1, 0);
            clrtobot();
            move(2, 0);
            prints("Éè¶¨ %s '%s' ÌÖÂÛÇøµÄÈ¨ÏŞ\n",
                   newfh.level & PERM_POSTMASK ? "ÕÅÌù" : "ÔÄ¶Á", newfh.filename);
            newfh.level = setperms(newfh.level, 0, "È¨ÏŞ", NUMPERMS, showperminfo);
            clear();
            getdata(0, 0, "È·¶¨Òª¸ü¸ÄÂğ? (Y/N) [N]: ", genbuf, 4, DOECHO, NULL, true);
        } else
        {
            getdata(17, 0, "È·¶¨Òª¸ü¸ÄÂğ? (Y/N) [N]: ", genbuf, 4, DOECHO, NULL, true);
        }
        if (*genbuf == 'Y' || *genbuf == 'y')
        {
            {
                char            secu[STRLEN];
                sprintf(secu, "ĞŞ¸ÄÌÖÂÛÇø£º%s(%s)", fh.filename, newfh.filename);
                securityreport(secu,NULL,NULL);
            }
            if (strcmp(fh.filename, newfh.filename))
            {
                char            old[256], tar[256];
                a_mv = 1;
                setbpath(old, fh.filename);
                setbpath(tar, newfh.filename);
                f_mv(old, tar);
                sprintf(old, "vote/%s", fh.filename);
                sprintf(tar, "vote/%s", newfh.filename);
                f_mv(old, tar);
            }
            if (newfh.BM[0] != '\0')
                sprintf(vbuf, "%-38.38s(BM: %s)", newfh.title + 13, newfh.BM);
            else
                sprintf(vbuf, "%-38.38s", newfh.title + 13);
            get_grp(fh.filename);
            edit_grp(fh.filename, lookgrp, oldtitle + 13, vbuf);
            if (a_mv >= 1)
            {
                group = chgrp();
                get_grp(fh.filename);
                strcpy(tmp_grp, lookgrp);
                if (strcmp(tmp_grp, group) || a_mv != 2)
                {
                    del_from_file("0Announce/.Search", fh.filename);
                    if (group != NULL)
                    {
                        if (newfh.BM[0] != '\0')
                            sprintf(vbuf, "%-38.38s(BM: %s)", newfh.title + 13, newfh.BM);
                        else
                            sprintf(vbuf, "%-38.38s", newfh.title + 13);

                        if (add_grp(group, newfh.filename, vbuf, cexplain) == -1)
                            prints("\n³ÉÁ¢¾«»ªÇøÊ§°Ü....\n");
                        else
                            prints("ÒÑ¾­ÖÃÈë¾«»ªÇø...\n");
                        sprintf(newpath, "0Announce/groups/%s/%s", group, newfh.filename);
                        sprintf(oldpath, "0Announce/groups/%s/%s", tmp_grp, fh.filename);
                        if (dashd(oldpath))
                        {
				/*
                            sprintf(genbuf, "/bin/rm -fr %s", newpath);
			    */
                            f_rm(newpath);
                        }
                        f_mv(oldpath, newpath);
                        del_grp(tmp_grp, fh.filename, fh.title + 13);
                    }
                }
            }
            if (noidboard == 1 && !anonymousboard(newfh.filename)) {
            	newfh.flag|=BOARD_ANNONY;
                addtofile("etc/anonymous", newfh.filename);
            }
            else
                if (noidboard == 0) {
            		newfh.flag&=~BOARD_ANNONY;
                    del_from_file("etc/anonymous", newfh.filename);
                }
			set_board(pos, &newfh);
            sprintf(genbuf, "¸ü¸ÄÌÖÂÛÇø %s µÄ×ÊÁÏ --> %s",
                    fh.filename, newfh.filename);
            report(genbuf);
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

    if (check_systempasswd()==false) return -1;
    modify_user_mode(ADMIN);
    clear();
    stand_title("²éÑ¯Ê¹ÓÃÕß·¢ÎÄ¼ÇÂ¼");
    move(1,0);
    usercomplete("ÇëÊäÈëÊ¹ÓÃÕßÕÊºÅ:", genbuf);
    strcpy(tmp_id, genbuf);
    if(  tmp_id[0] == '\0') {
        clear();
        return -1;
    }

    if(!(id = getuser(genbuf,NULL))) {
        move(3,0) ;
        prints("²»ÕıÈ·µÄÊ¹ÓÃÕß´úºÅ\n") ;
        clrtoeol();
        pressreturn();
        clear();
        return -1;
    }

    sprintf(tmp_command, "grep -a -w %s user.log | grep posted > tmp/searchresult.%d", tmp_id,getpid());
    system(tmp_command);
    sprintf(tmp_command,"tmp/searchresult.%d",getpid());
    mail_file(currentuser->userid,tmp_command, currentuser->userid, "ÏµÍ³²éÑ¯½á¹û",1);

    sprintf(buf, "²éÑ¯ÓÃ»§ %s µÄ·¢ÎÄÇé¿ö", tmp_id);
    securityreport(buf,NULL,NULL);  /*Ğ´Èësyssecurity°å, stephen 2000.12.21*/
    sprintf(buf, "Search the posts by %s in the trace", tmp_id);
    report(buf);   /*Ğ´Èëtrace, stephen 2000.12.21*/

    move(3,0);
    prints("²éÑ¯½á¹ûÒÑ¾­¼Äµ½ÄúµÄĞÅÏä£¡ \n");
    pressreturn();
    clear();
    return 0;
}           /* stephen 2000.12.15 let sysop search in trace */


char            curruser[IDLEN + 2];
extern int      delmsgs[];
extern int      delcnt;

void domailclean(struct fileheader *fhdrp,char* arg)
{
    static int      newcnt, savecnt, deleted, idc;
    char            buf[STRLEN];

    if (fhdrp == NULL)
    {
        bbslog("clean", "new = %d, saved = %d, deleted = %d", newcnt, savecnt, deleted);
        newcnt = savecnt = deleted = idc = 0;
        if (delcnt)
        {
            setmailfile(buf, curruser, DOT_DIR);
            while (delcnt--)
                delete_record(buf, sizeof(struct fileheader), delmsgs[delcnt],NULL,NULL);
        }
        delcnt = 0;
        return;
    }
    idc++;
    if (!(fhdrp->accessed[0] & FILE_READ))
        newcnt++;
    else
        if (fhdrp->accessed[0] & FILE_MARKED)
            savecnt++;
        else
        {
            deleted++;
            setmailfile(buf, curruser, fhdrp->filename);
            unlink(buf);
            delmsgs[delcnt++] = idc;
        }
}

int cleanmail(struct userec  *urec,char* arg)
{
    struct stat     statb;
    if (urec->userid[0] == '\0' || !strcmp(urec->userid, "new"))
        return 0;
    setmailfile(genbuf, urec->userid, DOT_DIR);
    if (stat(genbuf, &statb) == -1) {
        bbslog("clean","%s no mail",urec->userid);
    } else {
        if (statb.st_size == 0) {
            bbslog("clean","%s no mail",urec->userid);
        } else {
            strcpy(curruser, urec->userid);
            delcnt = 0;
            apply_record(genbuf, (RECORD_FUNC_ARG)domailclean, sizeof(struct fileheader),0,1);
            domailclean(NULL,0);
        }
    }
    return 0;
}

int m_mclean()
{
    char            ans[5];

    modify_user_mode(ADMIN);
    if (!check_systempasswd())
    {
        return -1;
    }
    clear();
    stand_title("Çå³ıË½ÈËĞÅ¼ş");
    move(1, 0);
    prints("Çå³ıËùÓĞÒÑ¶ÁÇÒÎ´ mark µÄĞÅ¼ş\n");
    getdata(2, 0, "È·¶¨Âğ (Y/N)? [N]: ", ans, 3, DOECHO, NULL, true);
    if (ans[0] != 'Y' && ans[0] != 'y')
    {
        clear();
        return 0;
    }
    {
        char            secu[STRLEN];
        sprintf(secu, "Çå³ıËùÓĞÊ¹ÓÃÕßÒÑ¶ÁĞÅ¼ş¡£");
        securityreport(secu,NULL,NULL);
    }

    move(3, 0);
    prints("ÇëÄÍĞÄµÈºò.\n");
    refresh();
    apply_users(cleanmail,0);
    move(4, 0);
    prints("Çå³ıÍê³É! Çë²é¿´ÈÕÖ¾ÎÄ¼ş.\n");
    report("Mail Clean");
    pressreturn();
    clear();
    return 0;
}

void trace_state(flag, name, size)
int             flag, size;
char           *name;
{
    char            buf[STRLEN];

    if (flag != -1)
    {
        sprintf(buf, "ON (size = %d)", size);
    } else
    {
        strcpy(buf, "OFF");
    }
    prints("%s¼ÇÂ¼ %s\n", name, buf);
}

int touchfile(filename)
char           *filename;
{
    int             fd;

    if ((fd = open(filename, O_RDWR | O_CREAT, 0600)) > 0)
    {
        close(fd);
    }
    return fd;
}

int m_trace()
{
    struct stat     ostatb, cstatb;
    int             otflag, ctflag, done = 0;
    char            ans[3];
    char           *msg;

    modify_user_mode(ADMIN);
    if (!check_systempasswd())
    {
        return -1;
    }
    clear();
    stand_title("Set Trace Options");
    while (!done)
    {
        move(2, 0);
        otflag = stat("trace", &ostatb);
        ctflag = stat("trace.chatd", &cstatb);
        prints("Ä¿Ç°Éè¶¨:\n");
        trace_state(otflag, "Ò»°ã", ostatb.st_size);
        trace_state(ctflag, "ÁÄÌì", cstatb.st_size);
        move(9, 0);
        prints("<1> ÇĞ»»Ò»°ã¼ÇÂ¼\n");
        prints("<2> ÇĞ»»ÁÄÌì¼ÇÂ¼\n");
        getdata(12, 0, "ÇëÑ¡Ôñ (1/2/Exit) [E]: ", ans, 2, DOECHO, NULL, true);

        switch (ans[0])
        {
        case '1':
            if (otflag)
            {
                touchfile("trace");
                msg = "Ò»°ã¼ÇÂ¼ ON";
            } else
            {
                f_mv("trace", "trace.old");
                msg = "Ò»°ã¼ÇÂ¼ OFF";
            }
            break;
        case '2':
            if (ctflag)
            {
                touchfile("trace.chatd");
                msg = "ÁÄÌì¼ÇÂ¼ ON";
            } else
            {
                f_mv("trace.chatd", "trace.chatd.old");
                msg = "ÁÄÌì¼ÇÂ¼ OFF";
            }
            break;
        default:
            msg = NULL;
            done = 1;
        }
        move(t_lines - 2, 0);
        if (msg)
        {
            prints("%s\n", msg);
            report(msg);
        }
    }
    clear();
    return 0;
}
int
valid_userid(ident ) /* check the user has registed, added by dong, 1999.4.18 */
char    *ident;
{
    if( strchr( ident, '@' ) && valid_ident( ident ) )
        return 1;
    return 0;
}
int check_proxy_IP(ip, buf) /* added for rejection of register from proxy,
			Bigman, 2001.11.9 */
	/* Óëbbsd_singleÀïÃæµÃlocal_check_ban_IP»ù±¾Ò»Ñù£¬¿ÉÒÔ¿¼ÂÇ¹²ÓÃ*/
char *ip;
char *buf;
{ /* Leeward 98.07.31
      RETURN:
                     - 1: No any banned IP is defined now
                       0: The checked IP is not banned
      other value over 0: The checked IP is banned, the reason is put in buf
      */
    FILE *Ban = fopen("etc/proxyIP", "r");
    char IPBan[64];
    int  IPX = - 1;
    char *ptr;

    if (!Ban)
        return IPX;
    else
        IPX ++;

    while (fgets(IPBan, 64, Ban))
       {
        if ((ptr = strchr(IPBan, '\n'))!=NULL)
            *ptr = 0;
        if ((ptr = strchr(IPBan, ' '))!=NULL)
        {
            *ptr ++ = 0;
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
int apply_reg(regfile,fname,pid,num)
/* added by Bigman, 2002.5.31 */
/* ÉêÇëÖ¸¶¨ÌõÊı×¢²áµ¥ */
char *regfile,*fname;
long pid;
int num;
{
	FILE		*in_fn,*out_fn,*tmp_fn;
	char	fname1[STRLEN],fname2[STRLEN];
	int sum,fd;	
	char *ptr;

	strcpy(fname1,"reg.ctrl");
	
    if ((in_fn = fopen(regfile, "r+")) == NULL)
    {
        move(2, 0);
        prints("ÏµÍ³´íÎó, ÎŞ·¨¶ÁÈ¡×¢²á×ÊÁÏµµ: %s\n", regfile);
        pressreturn();
        return -1;
    }
    
    fd=fileno(in_fn);
    flock(fd,LOCK_EX);
 
    if ((out_fn = fopen(fname, "w")) == NULL)
    {
        move(2, 0);
	flock(fd,LOCK_UN);
	fclose(in_fn);
        prints("ÏµÍ³´íÎó, ÎŞ·¨Ğ´ÁÙÊ±×¢²á×ÊÁÏµµ: %s\n", fname);
        pressreturn();
        return -1;
    } 
    sum=0;
    
    while (fgets(genbuf, STRLEN, in_fn) != NULL)
    {
        if ((ptr = (char *) strstr(genbuf, "----")) != NULL)
            sum++;
            
            fputs(genbuf, out_fn);
            
        if (sum >= num) break;
    }	
    fclose(out_fn);
    
    if (sum>=num) 
    {
    		sum=0;
    		
    		sprintf(fname2,"tmp/reg.%ld",pid);
        
        	if ((tmp_fn = fopen(fname2, "w")) == NULL)
		{
     			prints("²»ÄÜ½¨Á¢ÁÙÊ±ÎÄ¼ş:%s\n",fname2);
			flock(fd,LOCK_UN);
			fclose(in_fn);
        		pressreturn();	
        		return -1;
        	}		
    		
		while (fgets(genbuf, STRLEN, in_fn) != NULL)
    		{
        		if ((ptr = (char *) strstr(genbuf, "userid")) != NULL)
            			sum++;
            		fputs(genbuf, tmp_fn);
            		
             	}

		flock(fd,LOCK_UN);

    		fclose(in_fn);
    		fclose(tmp_fn);
    		        		
		if (sum>0) 
		{	f_rm(regfile);
			f_mv(fname2,regfile);      
        	}
        	else f_rm(regfile);
        	
        	f_rm(fname2);
    	
    }
    
    else f_rm(regfile);
    
    if ((out_fn = fopen(fname1, "a")) == NULL)
    {
        move(2, 0);
        prints("ÏµÍ³´íÎó, ÎŞ·¨¸ü¸Ä×¢²á¿ØÖÆÎÄ¼ş: %s\n", fname1);
        pressreturn();
        return -1;
    }

    fd=fileno(out_fn); 
 
    flock(fd,LOCK_UN); 
    fprintf(out_fn,"%ld\n",pid);
    flock(fd,LOCK_UN);
    fclose(out_fn);
	
    return(0);
}
int check_reg(mod)
int mod;
/* added by Bigman, 2002.5.31 */
/* mod=0 ¼ì²éreg_controlÎÄ¼ş */
/* mod=1 Õı³£ÍË³öÉ¾³ı¸ÃÎÄ¼ş */
{
	FILE		*fn1,*fn2;
	char	fname1[STRLEN]; 
	char	fname2[STRLEN];
	long	myid;
	int	flag=0,fd;

	strcpy(fname1,"reg.ctrl");
	
	if ((fn1 = fopen(fname1, "r")) != NULL)
        {	
 
		fd=fileno(fn1);
		flock(fd,LOCK_EX);

		sprintf(fname2,"tmp/reg.c%ld",getpid());
        
        	if ((fn2 = fopen(fname2, "w")) == NULL)
		{
     			prints("²»ÄÜ½¨Á¢ÁÙÊ±ÎÄ¼ş:%s\n",fname2);
			flock(fd,LOCK_UN);
			fclose(fn1);
        		pressreturn();	
        		return -1;
        	}
        	else
        	{        	
        		while (fgets(genbuf, STRLEN, fn1) != NULL)
   			{

   				myid=atol(genbuf); 
  
				if (mod ==0) {
/*    					if (myid==getpid())
					{
					prints("ÄãÖ»ÄÜÒ»¸ö½ø³Ì½øĞĞÉóÅúÕÊºÅ");
					pressreturn();
					return -1;
					}
*/
	
   					if (kill(myid,0)==-1)	/*×¢²áÖĞ¼ä¶ÏÏßÁË£¬»Ö¸´*/
   					{	flag=1;
   						restore_reg(myid);
   					}
   					else
   					{
   						fprintf(fn2,"%ld\n",myid);
   					}
   				}
   				else
   				{		flag=1;
   					if (myid!=getpid())
   						fprintf(fn2,"%ld\n",myid);
   					
   				
   				}
   				 				
   			}
   		fclose(fn2);
   		} 
		flock(fd,LOCK_UN);      
        	fclose(fn1);
        	
        	if (flag ==1) {
			f_rm(fname1);
			f_mv(fname2,fname1);
			}
        	f_rm(fname2);		
        	
        }
	
	return(0);
}

int restore_reg(pid)
long pid;
/* added by Bigman, 2002.5.31 */
/* »Ö¸´¶ÏÏßµÄ×¢²áÎÄ¼ş */
{
	FILE           *fn,*freg;
    	char            *regfile,buf[STRLEN];	
	int	fd1,fd2;
	
    	regfile = "new_register";    	
    	
    	sprintf(buf,"register.%ld",pid);

    	if ((fn = fopen(buf, "r")) != NULL)
    	{	fd1=fileno(fn);
		flock(fd1,LOCK_EX);

    		if ((freg = fopen(regfile, "a")) != NULL) 
    		{	fd2=fileno(freg);
			flock(fd2,LOCK_EX);
        		while (fgets(genbuf, STRLEN, fn) != NULL)
        			fputs(genbuf, freg);
                   	flock(fd2,LOCK_UN); 
        		fclose(freg);   
        
        	}
		flock(fd1,LOCK_UN);
        	fclose(fn);
        	
        	f_rm(buf);
        }	
        
return(0);
}
static const char    *field[] = {"usernum", "userid", "realname", "career",
                           "addr", "phone", "birth", NULL};
static const char    *reason[] = {
    "ÇëÊäÈëÕæÊµĞÕÃû(¹úÍâ¿ÉÓÃÆ´Òô).", "ÇëÏêÌîÑ§Ğ£¿ÆÏµ»ò¹¤×÷µ¥Î».",
    "ÇëÌîĞ´ÍêÕûµÄ×¡Ö·×ÊÁÏ.", "ÇëÏêÌîÁ¬Âçµç»°(ÈôÎŞ¿ÉÓÃºô»ú»òEmailµØÖ·´úÌæ).",
    "ÇëÈ·Êµ¶øÏêÏ¸µÄÌîĞ´×¢²áÉêÇë±í.", "ÇëÓÃÖĞÎÄÌîĞ´ÉêÇëµ¥.",
    "²»ÔÊĞí´Ó´©Ëó×¢²á", "Í¬Ò»¸öÓÃ»§×¢²áÁË¹ı¶àID",
    NULL};

int scan_register_form(logfile, regfile)
char           *logfile, *regfile;
{
    static const char    *finfo[] = {"ÕÊºÅÎ»ÖÃ", "ÉêÇë´úºÅ", "ÕæÊµĞÕÃû", "·şÎñµ¥Î»",
                           "Ä¿Ç°×¡Ö·", "Á¬Âçµç»°", "Éú    ÈÕ", NULL};
    struct userec   uinfo;
    FILE           *fn, *fout, *freg;
    char            fdata[7][STRLEN];
    char            fname[STRLEN], buf[STRLEN], buff;
    /* ^^^^^ Added by Marco */
    char            ans[5], *ptr, *uid;
    int             n, unum, fd;
    int		    count,sum,total_num;/*Haohmaru.2000.3.9.¼ÆËã»¹ÓĞ¶àÉÙµ¥×ÓÃ»´¦Àí*/

    long	pid;	/* Added by Bigman: 2002.5.31 */

    uid = currentuser->userid;
    stand_title("ÒÀĞòÉè¶¨ËùÓĞĞÂ×¢²á×ÊÁÏ");
/*    sprintf(fname, "%s.tmp", regfile);*/

    pid=getpid();
    sprintf(fname, "register.%ld",pid);

    move(2, 0);
    if (dashf(fname))
    {
/*        prints("[1mÆäËû SYSOP ÕıÔÚÊ¹ÓÃ telnet »ò WWW ²é¿´×¢²áÉêÇëµ¥£¬Çë¼ì²éÊ¹ÓÃÕß×´Ì¬¡£\n\n");
        prints("[33mÈç¹ûÃ»ÓĞÆäËû SYSOP ÕıÔÚ²é¿´×¢²áÉêÇëµ¥£¬ÔòÊÇÓÉÓÚ¶ÏÏßÔì³ÉµÄÎŞ·¨×¢²á¡£\n");
        prints("Çë½ø bbsroot ÕÊ»§ÔËĞĞÒ»´ÎÒÔÏÂÃüÁî£º\n");
        prints("                                   [32mcat new_register.tmp >> new_register[33m\n");
        prints("È·ÈÏÉÏÊöÃüÁîÔËĞĞ³É¹¦ºó£¬ÔÙÔËĞĞÒ»´ÎÒÔÏÂÃüÁî£º\n");
        prints("                                            [32mrm new_register.tmp\n[0m");
        pressreturn();
        return -1;*/

        restore_reg(pid);	/* Bigman,2002.5.31:»Ö¸´¸ÃÎÄ¼ş */
    }
/*    f_mv(regfile, fname);*/
/*ÉêÇë×¢²áµ¥ added by Bigman, 2002.5.31*/

/*Í³¼Æ×ÜµÄ×¢²áµ¥Êı Bigman, 2002.6.2 */
    if ((fn = fopen(regfile, "r")) == NULL)
    {
        move(2, 0);
        prints("ÏµÍ³´íÎó, ÎŞ·¨¶ÁÈ¡×¢²á×ÊÁÏµµ: %s\n", fname);
        pressreturn();
        return -1;
    }

    fd=fileno(fn);
    flock(fd,LOCK_EX);

    total_num=0;
    while (fgets(genbuf, STRLEN, fn) != NULL)
    {
        if ((ptr = (char *) strstr(genbuf, "userid")) != NULL)
            total_num++;
    }
    flock(fd,LOCK_UN);
    fclose(fn);

    apply_reg(regfile,fname,pid,50);

    if ((fn = fopen(fname, "r")) == NULL)
    {
        move(2, 0);
        prints("ÏµÍ³´íÎó, ÎŞ·¨¶ÁÈ¡×¢²á×ÊÁÏµµ: %s\n", fname);
        pressreturn();
        return -1;
    }
    memset(fdata, 0, sizeof(fdata));
    /*Haohmaru.2000.3.9.¼ÆËã¹²ÓĞ¶àÉÙµ¥×Ó*/
    sum=0;
    while (fgets(genbuf, STRLEN, fn) != NULL)
    {
        if ((ptr = (char *) strstr(genbuf, "userid")) != NULL)
            sum++;
    }
    fseek(fn, 0, SEEK_SET);
    count=1;
    while (fgets(genbuf, STRLEN, fn) != NULL)
    {
    	struct userec* lookupuser;
        if ((ptr = (char *) strstr(genbuf, ": ")) != NULL)
        {
            *ptr = '\0';
            for (n = 0; field[n] != NULL; n++)
            {
                if (strcmp(genbuf, field[n]) == 0)
                {
                    strcpy(fdata[n], ptr + 2);
                    if ((ptr = (char *) strchr(fdata[n], '\n')) != NULL)
                        *ptr = '\0';
                }
            }
        }
        else if ((unum = getuser(fdata[1],&lookupuser)) == 0)
        {
            move(2, 0);
            clrtobot();
            prints("ÏµÍ³´íÎó, ²éÎŞ´ËÕÊºÅ.\n\n");
            for (n = 0; field[n] != NULL; n++)
                prints("%s     : %s\n", finfo[n], fdata[n]);
            pressreturn();
            memset(fdata, 0, sizeof(fdata));
        }
        else
        {
            uinfo=*lookupuser;
            move(1, 0);
            prints("ÕÊºÅÎ»ÖÃ     : %d   ¹²ÓĞ %d ÕÅ×¢²áµ¥£¬µ±Ç°ÎªµÚ %d ÕÅ£¬»¹Ê£ %d ÕÅ\n", unum,total_num,count++,sum-count+1);/*Haohmaru.2000.3.9.¼ÆËã»¹ÓĞ¶àÉÙµ¥×ÓÃ»´¦Àí*/
            disply_userinfo(&uinfo, 1);
            move(15, 0);
            printdash(NULL);
            for (n = 0; field[n] != NULL; n++)
		/* added for rejection of register from proxy */
		/* Bigman, 2001.11.9 */
		if (n==1)	{
			if (check_proxy_IP(uinfo.lasthost,buf) > 0)
				prints("%s     : %s \033[33m%s\033[0m\n", finfo[n], fdata[n], buf);
			else prints("%s     : %s\n", finfo[n], fdata[n]);
		} else prints("%s     : %s\n", finfo[n], fdata[n]);
            /* if (uinfo.userlevel & PERM_LOGINOK) modified by dong, 1999.4.18*/
            if ((uinfo.userlevel & PERM_LOGINOK) || valid_userid(uinfo.realemail))
            {
                move(t_lines - 1, 0);
                prints("´ËÕÊºÅ²»ĞèÔÙÌîĞ´×¢²áµ¥.\n");
                igetkey();
                ans[0] = 'D';
            } else
            {
                getdata(t_lines - 1, 0, "ÊÇ·ñ½ÓÊÜ´Ë×ÊÁÏ (Y/N/Q/Del/Skip)? [S]: ",
                        ans, 3, DOECHO, NULL, true);
            }
            move(2, 0);
            clrtobot();
            switch (ans[0])
            {
            case 'D':
            case 'd':
                break;
            case 'Y':
            case 'y':
                prints("ÒÔÏÂÊ¹ÓÃÕß×ÊÁÏÒÑ¾­¸üĞÂ:\n");
                n = strlen(fdata[5]);
                if (n + strlen(fdata[3]) > 60)
                {
                    if (n > 40)
                        fdata[5][n = 40] = '\0';
                    fdata[3][60 - n] = '\0';
                }
                strncpy(uinfo.realname, fdata[2], NAMELEN);
                strncpy(uinfo.address, fdata[4], NAMELEN);
                sprintf(genbuf, "%s$%s@%s", fdata[3], fdata[5], uid);
                strncpy(uinfo.realemail, genbuf, STRLEN - 16);
                sprintf(buf, "tmp/email/%s", uinfo.userid);
                if ((fout = fopen(buf, "w")) != NULL)
                {
                    fprintf(fout, "%s\n", genbuf);
                    fclose(fout);
                }

                update_user(&uinfo,unum,0);
                mail_file(currentuser->userid,"etc/s_fill", uinfo.userid, "¹§ìûÄã£¬ÄãÒÑ¾­Íê³É×¢²á¡£",0);
                sprintf(genbuf, "%s ÈÃ %s Í¨¹ıÉí·İÈ·ÈÏ.", uid, uinfo.userid);
                securityreport(genbuf,lookupuser,fdata);
                if ((fout = fopen(logfile, "a")) != NULL)
                {
                	time_t now;
                    for (n = 0; field[n] != NULL; n++)
                        fprintf(fout, "%s: %s\n", field[n], fdata[n]);
                    now = time(NULL);
                    fprintf(fout, "Date: %s\n", Ctime(now));
                    fprintf(fout, "Approved: %s\n", uid);
                    fprintf(fout, "----\n");
                    fclose(fout);
                }
                /* user_display( &uinfo, 1 ); */
                /* pressreturn(); */

               /* Ôö¼Ó×¢²áĞÅÏ¢¼ÇÂ¼ 2001.11.11 Bigman */
                sethomefile( buf, uinfo.userid, "/register");
               if ((fout = fopen(buf, "w")) != NULL)
                {
			for (n = 0; field[n] != NULL; n++)
				fprintf(fout, "%s     : %s\n", finfo[n], fdata[n]);
			fprintf(fout,"ÄúµÄêÇ³Æ     : %s\n", uinfo.username);
			fprintf(fout,"µç×ÓÓÊ¼şĞÅÏä : %s\n", uinfo.email);
			fprintf(fout,"ÕæÊµ E-mail  : %s\n", uinfo.realemail );
			fprintf(fout,"Ident ×ÊÁÏ   : %s\n", uinfo.ident );
			fprintf(fout,"×¢²áÈÕÆÚ     : %s\n", ctime( &uinfo.firstlogin));
			fprintf(fout,"×¢²áÊ±µÄ»úÆ÷ : %s\n", uinfo.lasthost );
			fprintf(fout, "Approved: %s\n",uid);
			fclose(fout);
                }

                break;
            case 'Q':
            case 'q':
                if ((freg = fopen(regfile, "a")) != NULL)
                {	
			fd=fileno(freg);
			flock(fd,LOCK_EX);

                    for (n = 0; field[n] != NULL; n++)
                        fprintf(freg, "%s: %s\n", field[n], fdata[n]);
                    fprintf(freg, "----\n");
                    while (fgets(genbuf, STRLEN, fn) != NULL)
                        fputs(genbuf, freg);

			flock(fd,LOCK_UN);
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
                buff = buf[0];	/* Added by Marco */
                if (buf[0] != '\0')
                {
                    if (buf[0] >= '0' && buf[0] < '0' + n)
                    {
                        strcpy(buf, reason[buf[0] - '0']);
                    }
                    sprintf(genbuf, "<×¢²áÊ§°Ü> - %s", buf);
                    strncpy(uinfo.address, genbuf, NAMELEN);
                    update_user(&uinfo,unum,0);

                    /* ------------------- Added by Marco */
                    switch (buff)
                    {
                    case '0':
                        mail_file(currentuser->userid,"etc/f_fill.realname", uinfo.userid, uinfo.address,0);
                        break;
                    case '1':
                        mail_file(currentuser->userid,"etc/f_fill.unit", uinfo.userid, uinfo.address,0);
                        break;
                    case '2':
                        mail_file(currentuser->userid,"etc/f_fill.address", uinfo.userid, uinfo.address,0);
                        break;
                    case '3':
                        mail_file(currentuser->userid,"etc/f_fill.telephone", uinfo.userid, uinfo.address,0);
                        break;
                    case '4':
                        mail_file(currentuser->userid,"etc/f_fill.real", uinfo.userid, uinfo.address,0);
                        break;
                    case '5':
                        mail_file(currentuser->userid,"etc/f_fill.chinese", uinfo.userid, uinfo.address,0);
                        break;
                    case '6':
                        mail_file(currentuser->userid,"etc/f_fill.proxy", uinfo.userid, uinfo.address,0);
                        break;
                    case '7':
                        mail_file(currentuser->userid,"etc/f_fill.toomany", uinfo.userid, uinfo.address,0);
                        break;
                    default:
                        mail_file(currentuser->userid,"etc/f_fill.real", uinfo.userid,uinfo.address,0);
                        break;
                    }
                    /*
                     * -------------------------------------------------------
                     */
                    /* user_display( &uinfo, 1 ); */
                    /* pressreturn(); */
                    break;
                }
                move(10, 0);
                clrtobot();
                prints("È¡ÏûÍË»Ø´Ë×¢²áÉêÇë±í.\n");
                /* run default -- put back to regfile */
            default:
                if ((freg = fopen(regfile, "a")) != NULL)
                {
			fd=fileno(freg);
			flock(fd,LOCK_EX);

			for (n = 0; field[n] != NULL; n++)
                        fprintf(freg, "%s: %s\n", field[n], fdata[n]);
			fprintf(freg, "----\n");

			flock(fd,LOCK_UN);
			fclose(freg);
                }
            }
            memset(fdata, 0, sizeof(fdata));
        }
    }

    check_reg(1);   /* Bigman:2002.5.31 */

    fclose(fn);
    unlink(fname);
    return (0);
}

int m_register()
{
    FILE           *fn;
    char            ans[3], *fname;
    int             x, y, wid, len;

    modify_user_mode(ADMIN);
    if (!check_systempasswd())
    {
        return -1;
    }
    clear();

   if (check_reg(0)!=0) return(-1);	/* added by Bigman, 2002.5.31 */

    stand_title("Éè¶¨Ê¹ÓÃÕß×¢²á×ÊÁÏ");
    move(2, 0);

    fname = "new_register"; 

    if ((fn = fopen(fname, "r")) == NULL)
    {
        prints("Ä¿Ç°²¢ÎŞĞÂ×¢²á×ÊÁÏ.");
        pressreturn();
    } else
    {
        y = 2, x = wid = 0;
        while (fgets(genbuf, STRLEN, fn) != NULL && x < 65)
        {
            if (strncmp(genbuf, "userid: ", 8) == 0)
            {
                move(y++, x);
                prints(genbuf + 8);
                len = strlen(genbuf + 8);
                if (len > wid)
                    wid = len;
                if (y >= t_lines - 2)
                {
                    y = 2;
                    x += wid + 2;
                }
            }
        }
        fclose(fn);
        getdata(t_lines - 1, 0, "Éè¶¨×ÊÁÏÂğ (Y/N)? [N]: ", ans, 2, DOECHO, NULL, true);
        if (ans[0] == 'Y' || ans[0] == 'y')
        {
            {
                char            secu[STRLEN];
                sprintf(secu, "Éè¶¨Ê¹ÓÃÕß×¢²á×ÊÁÏ");
                securityreport(secu,NULL,NULL);
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
    if (!HAS_PERM(currentuser,PERM_ADMIN)) return -1;
    getdata(t_lines - 1, 0, "½ûÖ¹µÇÂ½Âğ (Y/N)? [N]: ", ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y')
    {
        if( vedit("NOLOGIN",false)==-1)
            unlink("NOLOGIN");
    }
    return 0;
}

/* czz added 2002.01.15 */
int inn_start()
{
	char ans[4], tmp_command[80];
	getdata(t_lines - 1, 0, "Æô¶¯×ªĞÅÂğ (Y/N)? [N]: ", ans, 2, DOECHO, NULL, true);
	if (ans[0] == 'Y' || ans[0] == 'y')
	{
		sprintf(tmp_command, "~bbs/innd/innbbsd");
		system(tmp_command);
	}
	return 0;
}

int inn_reload()
{
	char ans[4], tmp_command[80];
	getdata(t_lines - 1, 0, "ÖØ¶ÁÅäÖÃÂğ (Y/N)? [N]: ", ans, 2, DOECHO, NULL, true);
	if (ans[0] == 'Y' || ans[0] == 'y')
	{
		sprintf(tmp_command, "~bbs/innd/ctlinnbbsd reload");
		system(tmp_command);
	}
	return 0;
}

int inn_stop()
{
	char ans[4], tmp_command[80];
	getdata(t_lines - 1, 0, "Í£Ö¹×ªĞÅÂğ (Y/N)? [N]: ", ans, 2, DOECHO, NULL, true);
	if (ans[0] == 'Y' || ans[0] == 'y')
	{
		sprintf(tmp_command, "~bbs/innd/ctlinnbbsd shutdown");
		system(tmp_command);
	}
	return 0;
}
/* added end */
