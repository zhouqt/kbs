/*
	This file has been checked global varible.
	Ò»Ğ©ÔÓÏîµÄ¹¦ÄÜ
*/

#include "bbs.h"
extern int switch_code();       /* KCN,99.09.05 */
extern int convcode;            /* KCN,99.09.05 */
int modify_user_mode(mode)
int mode;
{
    if (uinfo.mode == mode)
        return 0;               /* ±ØĞë¼õÉÙupdate_ulistµÄ´ÎÊı. ylsdd 2001.4.27 */
    uinfo.mode = mode;
    UPDATE_UTMP(mode, uinfo);
    return 0;
}

/*
int
x_csh()
{
    int save_pager;
    clear() ;
    refresh() ;
    reset_tty() ;
    save_pager = uinfo.pager;
    uinfo.pager = 0 ;
    UPDATE_UTMP(pager,uinfo);
    bbslog("user","%s","shell out");
#ifdef SYSV
    do_exec("sh", NULL) ;
#else
    do_exec("csh", NULL);
#endif
    restore_tty() ;
    uinfo.pager = save_pager;
    UPDATE_UTMP(pager,uinfo);
    clear() ;
    return 0 ;
}

*/

struct _setperm_select {
    unsigned int pbits;
    unsigned int basic;
    unsigned int oldbits;
};
int showperminfo(struct _select_def *conf, int i)
{
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    i = i - 1;
    if (i == conf->item_count - 1) {
        prints("%c. ÍË³ö ", 'A' + i);
    } else if ((arg->pbits & (1 << i)) != (arg->oldbits & (1 << i))) {
        prints("%c. %-27s [31;1m%3s[m", 'A' + i, permstrings[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    } else {
        prints("%c. %-27s \x1b[37;0m%3s\x1b[m", 'A' + i, permstrings[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    }
    return SHOW_CONTINUE;
}

#ifdef SMS_SUPPORT
int showsmsdef(struct _select_def *conf, int i)
{
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    i = i - 1;
    if (i == conf->item_count - 1) {
        prints("%c. ÍË³ö ", 'A' + i);
    } else if ((arg->pbits & (1 << i)) != (arg->oldbits & (1 << i))) {
        prints("%c. %-40s [31;1m%3s[m", 'A' + i, user_smsdefstr[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    } else {
        prints("%c. %-40s \x1b[37;0m%3s\x1b[m", 'A' + i, user_smsdefstr[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    }
    return SHOW_CONTINUE;
}
#endif

int showuserdefine(struct _select_def *conf, int i)
{
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    i = i - 1;
    if (i == conf->item_count - 1) {
        prints("%c. ÍË³ö ", 'A' + i);
    } else if ((arg->pbits & (1 << i)) != (arg->oldbits & (1 << i))) {
        prints("%c. %-27s [31;1m%3s[m", 'A' + i, user_definestr[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    } else {
        prints("%c. %-27s \x1b[37;0m%3s\x1b[m", 'A' + i, user_definestr[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    }
    return SHOW_CONTINUE;
}

int setperm_select(struct _select_def *conf)
{
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    if (conf->pos == conf->item_count)
        return SHOW_QUIT;
    arg->pbits ^= (1 << (conf->pos - 1));
    return SHOW_REFRESHSELECT;
}

int setperm_show(struct _select_def *conf, int i)
{
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    i = i - 1;
    if (i == conf->item_count - 1) {
        prints("%c. ÍË³ö ", 'A' + i);
    } else {
        if ((arg->pbits & (1 << i)) != (arg->oldbits & (1 << i))) {
            prints("%c. %-27s [31;1m%3s[m", 'A' + i, permstrings[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
        } else if ((1 << i == PERM_BASIC || 1 << i == PERM_POST || 1 << i == PERM_CHAT || 1 << i == PERM_PAGE || 1 << i == PERM_DENYMAIL|| 1 << i == PERM_DENYRELAX) && (arg->basic & (1 << i)))
            prints("%c. %-27s [32;1m%3s[m", 'A' + i, permstrings[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
        else
            prints("%c. %-27s [37;0m%3s[m", 'A' + i, permstrings[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    }
    return SHOW_CONTINUE;
}

int setperm_key(struct _select_def *conf, int key)
{
    int sel;

    if (key == Ctrl('Q'))
        return SHOW_QUIT;
    if (key == Ctrl('A')) {
        struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

        arg->pbits = arg->oldbits;
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

unsigned int setperms(unsigned int pbits, unsigned int basic, char *prompt, int numbers, int (*show) (struct _select_def *, int), int (*select) (struct _select_def *))
{
    struct _select_def perm_conf;
    struct _setperm_select arg;
    POINT *pts;
    int i;

    pts = (POINT *) malloc(sizeof(POINT) * (numbers + 1));

    move(4, 0);
    prints("Çë°´ÏÂÄãÒªµÄ´úÂëÀ´Éè¶¨%s, Ctrl+QÍË³ö£¬Ctrl+A·ÅÆúĞŞ¸ÄÍË³ö.\n", prompt);
    move(6, 0);
    clrtobot();

    for (i = 0; i < numbers + 1; i++) {
        pts[i].x = 0 + ((i > 15) ? 42 : 2);
        pts[i].y = i + 6 - ((i > 15) ? 16 : 0);
    }
    arg.pbits = pbits;
    arg.basic = basic;
    arg.oldbits = pbits;
    bzero((char *) &perm_conf, sizeof(struct _select_def));
    perm_conf.item_count = numbers + 1;
    perm_conf.item_per_page = numbers + 1;
    perm_conf.flag = LF_BELL | LF_LOOP; /*|LF_HILIGHTSEL;*/
    perm_conf.prompt = "¡ô";
    perm_conf.item_pos = pts;
    perm_conf.arg = &arg;
    perm_conf.title_pos.x = 1;
    perm_conf.title_pos.y = 6;
    perm_conf.pos = numbers + 1;

    if (select)
        perm_conf.on_select = select;
    else
        perm_conf.on_select = setperm_select;
    perm_conf.show_data = show;
    perm_conf.key_command = setperm_key;

    list_select_loop(&perm_conf);
    free(pts);
    return arg.pbits;
}

/* É¾³ı¹ıÆÚµÄÕÊºÅ */
/* ËãÊÇ¸ø°ÂÔËµÄÏ×Àñ */
/* Bigman 2001.7.14 */
int confirm_delete_id()
{
    char buff[STRLEN];

    if (!HAS_PERM(currentuser, PERM_ADMIN)) {
        move(3, 0);
        clrtobot();
        prints("±§Ç¸, Ö»ÓĞ×Ü¹ÜÀíÔ±²ÅÄÜĞŞ¸Ä");
        pressreturn();
        return 1;
    }

    modify_user_mode(ADMIN);
    clear();
    move(8, 0);
    prints("\n");
    clrtoeol();
    getdata(9,0,"ÇëÊäÈëÒªÈ·ÈÏÇå³ıµÄÓÃ»§ID: ", genbuf, IDLEN+1,1,NULL,true);
    if (genbuf[0] == '\0') {
        clear();
        return 1;
    }

    if ((usernum = searchuser(genbuf)) != 0) {
        prints("´ËÕÊºÅÓĞÈËÊ¹ÓÃ\n");
        pressreturn();
        return 1;
    }

    sethomepath(buff, genbuf);
    /*
     * sprintf(commd,"rm -rf %s",buff);
     */
    f_rm(buff);
    setmailpath(buff, genbuf);
    f_rm(buff);
    /*
     * sprintf(commd,"rm -rf %s",buff);
     */

    bbslog("user","%s","delete confirmly dead id's directory");

    clrtoeol();
    pressreturn();
    clear();
    return 0;
}

int x_level()
{
    int id;
    unsigned int newlevel;
    int flag = 0;               /*Haohmaru,98.10.05 */
    int flag1 = 0, flag2 = 0;   /* bigman 2000.1.5 */
    struct userec *lookupuser;
    char genbuf2[255];
    int lcount = 0, i, j, kcount = 0, basicperm;
    int s[10][2];
    FILE *fn;

    /*
     * add by alex, 97.7 , strict the power of sysop 
     */
    if (!HAS_PERM(currentuser, PERM_ADMIN) || !HAS_PERM(currentuser, PERM_SYSOP)) {
        move(3, 0);
        clrtobot();
        prints("±§Ç¸, Ö»ÓĞADMINÈ¨ÏŞµÄ¹ÜÀíÔ±²ÅÄÜĞŞ¸ÄÆäËûÓÃ»§È¨ÏŞ");
        pressreturn();
        return 0;
    }

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return 0;
    }
    clear();
    move(0, 0);
    prints("¸ü¸Ä" NAME_USER_SHORT "µÄÈ¨ÏŞ(\x1b[m×¢Òâ£ºÈç¹ûÊÇ·â½û½â·â£¬ÇëÊ¹ÓÃ·â½ûÑ¡µ¥£¡\x1b[m\n");
    clrtoeol();
    move(1, 0);
    usercomplete("ÇëÊäÈë" NAME_USER_SHORT " ID: ", genbuf);
    if (genbuf[0] == '\0') {
        clear();
        return 0;
    }
    if (!(id = getuser(genbuf, &lookupuser))) {
        move(3, 0);
        prints("·Ç·¨ ID");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    if ((lookupuser->userlevel & PERM_BOARDS))  /*Haohmaru.98.10.05 */
        flag = 1;
    if ((lookupuser->userlevel & PERM_CLOAK))   /* Bigman 2000.1.5 */
        flag1 = 1;
    if ((lookupuser->userlevel & PERM_XEMPT))
        flag2 = 1;

/*Bad 2002.7.6 ÊÜÏŞÓë½äÍøÎÊÌâ*/
    lcount=get_giveupinfo(lookupuser->userid,&basicperm,s);
    move(1, 0);
    clrtobot();
    move(2, 0);
    prints("ÇëÉè¶¨" NAME_USER_SHORT " '%s' µÄÈ¨ÏŞ\n", genbuf);
    newlevel = setperms(lookupuser->userlevel, basicperm, "È¨ÏŞ", NUMPERMS, setperm_show, NULL);
    move(2, 0);
    if (newlevel == lookupuser->userlevel)
        prints(NAME_USER_SHORT " '%s' µÄÈ¨ÏŞÃ»ÓĞ¸ü¸Ä\n", lookupuser->userid);
    else {                      /* Leeward: 1997.12.02 : Modification starts */
        char secu[STRLEN];

        sprintf(secu, "ĞŞ¸Ä %s µÄÈ¨ÏŞXPERM%d %d", lookupuser->userid, lookupuser->userlevel, newlevel);
        securityreport(secu, lookupuser, NULL);
        lookupuser->userlevel = newlevel;
        /*
         * Leeward: 1997.12.02 : Modification stops 
         */

        prints(NAME_USER_SHORT " '%s' µÄÈ¨ÏŞÒÑ¸ü¸Ä,×¢Òâ£ºÈç¹ûÊÇ·â½û½â·â£¬\n", lookupuser->userid);
        sprintf(genbuf, "changed permissions for %s", lookupuser->userid);
        bbslog("user","%s",genbuf);
        /*
         * Haohmaru.98.10.03.¸øĞÂÈÎ°æÖ÷×Ô¶¯·¢ĞÅ 
         */
//        if ((lookupuser->userlevel & PERM_BOARDS) && flag == 0)
//            mail_file(currentuser->userid, "etc/forbm", lookupuser->userid, "ĞÂÈÎ" NAME_BM "±Ø¶Á", BBSPOST_LINK, NULL);
        /*
         * Bigman 2000.1.5 ĞŞ¸ÄÈ¨ÏŞ×Ô¶¯·¢ĞÅ 
         */
        if ((lookupuser->userlevel & PERM_CLOAK) && flag1 == 0)
            mail_file(currentuser->userid, "etc/forcloak", lookupuser->userid, NAME_SYSOP_GROUP "ÊÚÓèÄúÒşÉíÈ¨ÏŞ", BBSPOST_LINK, NULL);
        if ((lookupuser->userlevel & PERM_XEMPT) && flag2 == 0)
            mail_file(currentuser->userid, "etc/forlongid", lookupuser->userid, NAME_SYSOP_GROUP "ÊÚÓèÄú³¤ÆÚÕÊºÅÈ¨ÏŞ", BBSPOST_LINK, NULL);
		save_giveupinfo(lookupuser,lcount,s);
    }
    pressreturn();
    clear();
    return 0;
}

int XCheckLevel()
{                               /* Leeward 98.06.05 */
    unsigned int newlevel;
    struct userec scanuser;

    if (!HAS_PERM(currentuser, PERM_ADMIN) || !HAS_PERM(currentuser, PERM_SYSOP)) {
        move(3, 0);
        clrtobot();
        prints("±§Ç¸, ÄúÃ»ÓĞ´ËÈ¨ÏŞ");
        pressreturn();
        return -1;
    }

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    clear();
    move(0, 0);
    prints("ÁĞÊ¾¾ßÓĞÌØ¶¨È¨ÏŞµÄ" NAME_USER_SHORT "µÄ×ÊÁÏ\n");
    clrtoeol();
    move(2, 0);
    prints("ÇëÉè¶¨ĞèÒª¼ì²éµÄÈ¨ÏŞ\n");
    scanuser.userlevel = 0;
    /*
     * change showperminfoX to showperminfo 
     */
    newlevel = setperms(scanuser.userlevel, 0, "È¨ÏŞ", NUMPERMS, showperminfo, NULL);
    move(2, 0);
    if (newlevel == scanuser.userlevel)
        prints("ÄãÃ»ÓĞÉè¶¨ÈÎºÎÈ¨ÏŞ\n");
    else {
        char secu[STRLEN];
        char buffer[256];
        int fhp;
        FILE *fpx;
        long count = 0L;

		gettmpfilename( buffer, "XCL" );
        //sprintf(buffer, "tmp/XCL.%s%d", currentuser->userid, getpid());
        if (-1 == (fhp = open(".PASSWDS", O_RDONLY))) {
            prints("ÏµÍ³´íÎó: ÎŞ·¨´ò¿ª¿ÚÁîÎÄ¼ş\n");
        } else if (NULL == (fpx = fopen(buffer, "w"))) {
            close(fhp);
            prints("ÏµÍ³´íÎó: ÎŞ·¨´ò¿ªÁÙÊ±ÎÄ¼ş\n");
        } else {
            prints("ÁĞÊ¾²Ù×÷¿ÉÄÜĞèÒª½Ï³¤Ê±¼ä²ÅÄÜÍê³É, ÇëÄÍĞÄµÈ´ı. ");
            clrtoeol();
            if (askyn("ÄãÈ·¶¨Òª½øĞĞÁĞÊ¾Âğ", 0)) {
                while (read(fhp, &scanuser, sizeof(struct userec)) > 0) {
                    if ((scanuser.userlevel & newlevel) == newlevel && strcmp("SYSOP", scanuser.userid)) {
						struct userdata ud;

						read_userdata(scanuser.userid, &ud);
                        count++;
                        fprintf(fpx, "[1m[33mÇë±£³ÖÕâÒ»ĞĞÎ»ÓÚÆÁÄ»µÚÒ»ĞĞ£¬´ËÊ±°´ X ¼ü¿É¸øÏÂÁĞÓÃ»§·¢ĞÅÒªÇóÆä²¹Æë¸öÈË×¢²á×ÊÁÏ[m\n\n");
                        fprintf(fpx, "ÓÃ»§´úºÅ(êÇ³Æ) : %s(%s)\n\n", scanuser.userid, scanuser.username);
                        fprintf(fpx, "Õæ  Êµ  ĞÕ  Ãû : %s\n\n", ud.realname);
                        fprintf(fpx, "¾Ó  ×¡  ×¡  Ö· : %s\n\n", ud.address);
                        fprintf(fpx, "µç  ×Ó  ÓÊ  ¼ş : %s\n\n", ud.email);
                        fprintf(fpx, "µ¥Î»$µç»°@ÈÏÖ¤ : %s\n\n", ud.realemail);
                        fprintf(fpx, "×¢  ²á  ÈÕ  ÆÚ : %s\n", ctime(&scanuser.firstlogin));
                        fprintf(fpx, "×îºóµÄµÇÂ¼ÈÕÆÚ : %s\n", ctime(&scanuser.lastlogin));
                        fprintf(fpx, "×îºóµÄµÇÂ¼»úÆ÷ : %s\n\n", scanuser.lasthost);
                        fprintf(fpx, "ÉÏ  Õ¾  ´Î  Êı : %d ´Î\n\n", scanuser.numlogins);
                        fprintf(fpx, "ÎÄ  ÕÂ  Êı  Ä¿ : %d Æª\n\n", scanuser.numposts);
                    }
                }
                fprintf(fpx, "[1m[33mÒ»¹²ÁĞ³öÁË %ld Ïî¾ßÓĞ´ËÈ¨ÏŞµÄÓÃ»§×ÊÁÏ[m\n\n*** ÕâÊÇÁĞÊ¾½á¹ûµÄ×îºóÒ»ĞĞ£®Èç¹û¼ì²éÍê±Ï£¬Çë°´ q ¼ü½áÊø *** (ÒÔÏÂ¾ùÎª¿ÕĞĞ)", count);
                {
                    int dummy;  /* process the situation of a too high screen :PP */

                    for (dummy = 0; dummy < t_lines * 4; dummy++)
                        fputs("\n", fpx);
                }
                close(fhp);
                fclose(fpx);

                sprintf(secu, "[1m[33mÒ»¹²ÁĞ³öÁË %ld Ïî¾ßÓĞ´ËÈ¨ÏŞµÄÓÃ»§×ÊÁÏ[m", count);
                move(2, 0);
                prints(secu);
                clrtoeol();
                sprintf(genbuf, "listed %ld userlevel of %d", count, newlevel);
                bbslog("user","%s",genbuf);
                pressanykey();

                /*
                 * sprintf(secu, "ÁĞÊ¾¾ßÓĞÌØ¶¨È¨ÏŞµÄ %ld ¸öÓÃ»§µÄ×ÊÁÏ", count); 
                 */
                clear();
                ansimore(buffer, false);
                clear();
                move(2, 0);
                prints("ÁĞÊ¾²Ù×÷Íê³É");
                clrtoeol();

                unlink(buffer);
            } else {
                move(2, 0);
                prints("È¡ÏûÁĞÊ¾²Ù×÷");
                clrtoeol();
            }
        }
    }
    pressreturn();
    clear();
    return 0;
}

#ifdef SMS_SUPPORT
int x_usersmsdef()
{
    unsigned int newlevel;
    struct userec *lookupuser;

    modify_user_mode(USERDEF);
	clear();
    if (!strcmp(currentuser->userid, "guest"))
        return 0;

	if( ! currentmemo->ud.mobileregistered ){
        move(3, 0);
		prints("ÄúÃ»ÓĞ×¢²áÊÖ»úºÅÂë");
        pressreturn();
        return 0;
    }
    move(2, 0);
    newlevel = setperms(currentmemo->ud.smsdef, 0, "¶ÌĞÅ²ÎÊı", NUMSMSDEF, showsmsdef, NULL);
    move(2, 0);
    if (newlevel == currentmemo->ud.smsdef)
        prints("²ÎÊıÃ»ÓĞĞŞ¸Ä...\n");
    else {
        currentmemo->ud.smsdef = newlevel;
		write_userdata(currentuser->userid, &(currentmemo->ud) );
        prints("ĞÂµÄ²ÎÊıÉè¶¨Íê³É...\n\n");
    }
    pressreturn();
    clear();
    return 0;
}
#endif

int x_userdefine()
{
    int id;
    unsigned int newlevel;
    extern int nettyNN;
    struct userec *lookupuser;

    modify_user_mode(USERDEF);
    if (!(id = getuser(currentuser->userid, &lookupuser))) {
        move(3, 0);
        prints("´íÎóµÄ " NAME_USER_SHORT " ID...");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    if (!strcmp(currentuser->userid, "guest"))
        return 0;
    move(1, 0);
    clrtobot();
    move(2, 0);
    newlevel = setperms(lookupuser->userdefine, 0, "²ÎÊı", NUMDEFINES, showuserdefine, NULL);
    move(2, 0);
    if (newlevel == lookupuser->userdefine)
        prints("²ÎÊıÃ»ÓĞĞŞ¸Ä...\n");
    else {
        lookupuser->userdefine = newlevel;
        currentuser->userdefine = newlevel;
        if (((convcode) && (newlevel & DEF_USEGB))      /* KCN,99.09.05 */
            ||((!convcode) && !(newlevel & DEF_USEGB)))
            switch_code();
        uinfo.pager |= FRIEND_PAGER;
        if (!(uinfo.pager & ALL_PAGER)) {
            if (!DEFINE(currentuser, DEF_FRIENDCALL))
                uinfo.pager &= ~FRIEND_PAGER;
        }
        uinfo.pager &= ~ALLMSG_PAGER;
        uinfo.pager &= ~FRIENDMSG_PAGER;
        if (DEFINE(currentuser, DEF_FRIENDMSG)) {
            uinfo.pager |= FRIENDMSG_PAGER;
        }
        if (DEFINE(currentuser, DEF_ALLMSG)) {
            uinfo.pager |= ALLMSG_PAGER;
            uinfo.pager |= FRIENDMSG_PAGER;
        }
        UPDATE_UTMP(pager, uinfo);
        if (DEFINE(currentuser, DEF_ACBOARD))
            nettyNN = NNread_init();
        prints("ĞÂµÄ²ÎÊıÉè¶¨Íê³É...\n\n");
    }
    pressreturn();
    clear();
    return 0;
}

int x_cloak()
{
    modify_user_mode(GMENU);
    bbslog("user","%s","toggle cloak");
    uinfo.invisible = (uinfo.invisible) ? false : true;
    UPDATE_UTMP(invisible, uinfo);
    if (!uinfo.in_chat) {
        move(1, 0);
        clrtoeol();
        prints("ÒşÉíÊõ (cloak) ÒÑ¾­%sÁË!", (uinfo.invisible) ? "Æô¶¯" : "Í£Ö¹");
        pressreturn();
    }
    return 0;
}

int x_date()
{
    time_t t;

    modify_user_mode(XMENU);
    clear();
    move(8, 0);
    time(&t);
    prints("Ä¿Ç°ÏµÍ³ÈÕÆÚÓëÊ±¼ä: %s", ctime(&t));
    clrtoeol();
    pressreturn();
    return 0;
}

void x_edits()
{
    int aborted;
    char ans[7], buf[STRLEN];
    int ch, num;
    char *e_file[] = { "plans", "signatures", "notes", "logout", NULL };
    char *explain_file[] = { "¸öÈËËµÃ÷µµ", "Ç©Ãûµµ", "×Ô¼ºµÄ±¸ÍüÂ¼", "ÀëÕ¾µÄ»­Ãæ", NULL };

    modify_user_mode(GMENU);
    clear();
    move(1, 0);
    prints("±àĞŞ¸öÈËµµ°¸\n\n");
    for (num = 0; e_file[num] != NULL && explain_file[num] != NULL; num++) {
        prints("[[32m%d[m] %s\n", num + 1, explain_file[num]);
    }
    prints("[[32m%d[m] ¶¼²»Ïë¸Ä\n", num + 1);

    getdata(num + 5, 0, "ÄãÒª±àĞŞÄÄÒ»Ïî¸öÈËµµ°¸: ", ans, 2, DOECHO, NULL, true);
    if (ans[0] - '0' <= 0 || ans[0] - '0' > num || ans[0] == '\n' || ans[0] == '\0')
        return;

    ch = ans[0] - '0' - 1;

    sethomefile(genbuf, currentuser->userid, e_file[ch]);
    move(3, 0);
    clrtobot();
    sprintf(buf, "(E)±à¼­ (D)É¾³ı %s? [E]: ", explain_file[ch]);
    getdata(3, 0, buf, ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'D' || ans[0] == 'd') {
        my_unlink(genbuf);
        move(5, 0);
        prints("%s ÒÑÉ¾³ı\n", explain_file[ch]);
        sprintf(buf, "delete %s", explain_file[ch]);
        bbslog("user","%s",buf);
        pressreturn();
        clear();
        return;
    }
    modify_user_mode(EDITUFILE);
    aborted = vedit(genbuf, false, NULL, NULL);
    clear();
    if (!aborted) {
        prints("%s ¸üĞÂ¹ı\n", explain_file[ch]);
        sprintf(buf, "edit %s", explain_file[ch]);
        if (!strcmp(e_file[ch], "signatures")) {
            set_numofsig();
            if (numofsig&&(currentuser->signature==0))
            	currentuser->signature=1;
            prints("ÏµÍ³ÖØĞÂÉè¶¨ÒÔ¼°¶ÁÈëÄãµÄÇ©Ãûµµ...");
        }
        bbslog("user","%s",buf);
    } else
        prints("%s È¡ÏûĞŞ¸Ä\n", explain_file[ch]);
    pressreturn();
}

void a_edits()
{
    int aborted;
    char ans[7], buf[STRLEN];
    int ch, num;

    /*
     * Leeward 98.04.01 added: sysconf.ini
     */
    /*
     * Leeward 98.07.31 added: .badIP
     */
    /*
     * stephen 2000.10.17 added: /usr/share/apache/htdocs/script/menucontext.js
     */
    /*
     * period  2000.10.17 link /backup/www/htdocs/script/menucontext.js --> /home0/bbs/etc/www_menu.js
     */
    static const char *e_file[] =
        { "../Welcome", "../vote/notes", "issue", "movie", "logout", "menu.ini", "proxyIP", "mailcheck", "s_fill", "f_fill.realname", "f_fill.unit", "f_fill.address", "f_fill.telephone",
        "f_fill.real", "f_fill.chinese", "f_fill.toomany", "f_fill.proxy", "smail", "f_fill", "../.badname", "../.badIP", "badword", "sysconf.ini", "www_menu.js", "../0Announce/hotinfo",
        "../0Announce/systeminfo",
        "forbm",
        "forcloak",
        "forlongid", "../innd/newsfeeds.bbs", "deny_reason", "initial_favboard","tonewuser", 
        #ifdef HAVE_CUSTOM_USER_TITLE
        "../" USER_TITLE_FILE,
        #endif
        NULL
    };

    /*
     * "/usr/share/apache/htdocs/script/menucontext.js", NULL};
     */
    static const char *explain_file[] =
        { "Welcome", "¹«ÓÃ±¸ÍüÂ¼", "½øÕ¾»¶Ó­µµ", "»î¶¯¿´°æ", "ÀëÕ¾»­Ãæ", "menu.ini", "´©ËóIP", "Éí·İÈ·ÈÏµµ", "×¢²áµ¥Íê³Éµµ", "×¢²áµ¥Ê§°Üµµ(ÕæÊµĞÕÃû)", "×¢²áµ¥Ê§°Üµµ(·şÎñµ¥Î»)",
        "×¢²áµ¥Ê§°Üµµ(¾Ó×¡µØÖ·)", "×¢²áµ¥Ê§°Üµµ(ÁªÂçµç»°)", "×¢²áµ¥Ê§°Üµµ(ÕæÊµ×ÊÁÏ)", "×¢²áµ¥Ê§°Üµµ(ÖĞÎÄÌîĞ´)", "×¢²áµ¥Ê§°Üµµ(¹ı¶àµÄID)", "×¢²áµ¥Ê§°Üµµ(²»ÄÜ´©Ëó×¢²á)",
        "Éí·İÈ·ÈÏÍê³Éµµ        ",
        "Éí·İÈ·ÈÏÊ§°Üµµ",
        "²»¿É×¢²áµÄ ID         ", "²»¿ÉµÇÂ¼µÄ IP", "ÏµÍ³×Ô¶¯¹ıÂËµÄ´ÊÓï    ",
        "sysconf.ini", "WWWÖ÷²Ëµ¥             ", "½üÆÚÈÈµã", "ÏµÍ³ÈÈµã",
        "¸øĞÂÈÎ°æÖ÷µÄĞÅ        ", "¸øÒşÉíÓÃ»§µÄĞÅ",
        "¸ø³¤ÆÚÓÃ»§µÄĞÅ        ",
        "×ªĞÅ°æºÍĞÂÎÅ×é¶ÔÓ¦",
        "·â½ûÀíÓÉÁĞ±í", "ĞÂÓÃ»§¸öÈË¶¨ÖÆÇø","¸øĞÂ×¢²áÓÃ»§µÄĞÅ", 
        #ifdef HAVE_CUSTOM_USER_TITLE
        "ÓÃ»§Ö°Îñ±í",
        #endif
        NULL
    };

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return;
    }
    clear();
    move(0, 0);
    prints("±àĞŞÏµÍ³µµ°¸\n\n");
    for (num = 0; e_file[num] != NULL && explain_file[num] != NULL; num++) {
        prints("[[32m%2d[m] %s%s", num + 1, explain_file[num], ((num + 1) % 2) ? "      " : "\n");
        /*
         * Leeward 98.03.29 µ÷ÕûÏÔÊ¾²¼¾Ö£¬ÒÔ±ã¼ÓÈë¡°ÏµÍ³×Ô¶¯¹ıÂËµÄ´ÊÓï¡±Ò»Ïî
         */
        /*
         * Leeward 98.07.31 µ÷ÕûÏÔÊ¾²¼¾Ö£¬ÒÔ±ã¼ÓÈë¡°²»¿ÉµÇÂ¼µÄ IP¡±Ò»Ïî
         */
        /*
         * Bigman 2001.6.23 µ÷Õû²¼¾Ö£¬ Ôö¼ÓÆäËû¼¸ÏîÄÚÈİ
         */
    }

    prints("[[32m%2d[m] ¶¼²»Ïë¸Ä\n", num + 1);

    /*
     * ÏÂÃæµÄ21ÊÇĞĞºÅ£¬ÒÔºóÌí¼Ó£¬¿ÉÒÔÏàÓ¦µ÷Õû
     */
    getdata(21, 0, "ÄãÒª±àĞŞÄÄÒ»ÏîÏµÍ³µµ°¸: ", ans, 3, DOECHO, NULL, true);
    ch = atoi(ans);
    if (!isdigit(ans[0]) || ch <= 0 || ch > num || ans[0] == '\n' || ans[0] == '\0')
        return;
    ch -= 1;
    sprintf(genbuf, "etc/%s", e_file[ch]);
    move(2, 0);
    clrtobot();
    sprintf(buf, "(E)±à¼­ (D)É¾³ı %s? [E]: ", explain_file[ch]);
    getdata(3, 0, buf, ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'D' || ans[0] == 'd') {
        {
            char secu[STRLEN];

            sprintf(secu, "É¾³ıÏµÍ³µµ°¸£º%s", explain_file[ch]);
            securityreport(secu, NULL, NULL);
        }
        my_unlink(genbuf);
        move(5, 0);
        prints("%s ÒÑÉ¾³ı\n", explain_file[ch]);
        sprintf(buf, "delete %s", explain_file[ch]);
        bbslog("user","%s",buf);
        pressreturn();
        clear();
        return;
    }
    modify_user_mode(EDITSFILE);
    aborted = vedit(genbuf, false, NULL, NULL);
    clear();
    if (aborted != -1) {
        prints("%s ¸üĞÂ¹ı", explain_file[ch]);
        sprintf(buf, "edit %s", explain_file[ch]);
        bbslog("user","%s",buf);
        {
            char secu[STRLEN];

            sprintf(secu, "ĞŞ¸ÄÏµÍ³µµ°¸£º%s", explain_file[ch]);
            securityreport(secu, NULL, NULL);
        }

        if (!strcmp(e_file[ch], "../Welcome")) {
            my_unlink("Welcome.rec");
            prints("\nWelcome ¼ÇÂ¼µµ¸üĞÂ");
        }
#ifdef FILTER
	if (!strcmp(e_file[ch], "badword")) {
            my_unlink(BADWORD_IMG_FILE);
            prints("\n¹ıÂË´Ê±í¸üĞÂ");
	}
#endif
#ifdef HAVE_CUSTOM_USER_TITLE
       if (!strcmp(e_file[ch],"../" USER_TITLE_FILE)) {
          load_user_title();
       }
#endif
    }
    pressreturn();
}

#ifdef CAN_EXEC

#ifdef BBSDOORS

void ent_bnet()
{                               /* Bill Schwartz */
    int save_pager = uinfo.pager;

    uinfo.pager = -1;
    bbslog("user","%s","BBSNet Enter");
    modify_user_mode(BBSNET);
    /*
     * bbsnet.sh is a shell script that can be customized without 
     */
    /*
     * having to recompile anything.  If you edit it while someone 
     */
    /*
     * is in bbsnet they will be sent back to the xyz menu when they 
     */
    /*
     * leave the system they are currently in. 
     */

    do_exec("bbsnet.sh", NULL);
    uinfo.pager = save_pager;
    bbslog("user","%s","BBSNet Exit");
    clear();
}

#endif
#endif
static void escape_filename(char *fn)
{
    static const char invalid[] = { '/', '\\', '!', '&', '|', '*', '?', '`', '\'', '\"', ';', '<', '>', ':' };
    int i, j;

    for (i = 0; i < strlen(fn); i++)
        for (j = 0; j < sizeof(invalid); j++)
            if (fn[i] == invalid[j])
                fn[i] = '_';
}
int zsend_file(char *filename, char *title)
{
    char *t;
    char buf[100], buf1[100];
    int i;

    ansimore("etc/zmodem", 0);
    move(t_lines - 1, 0);
    clrtoeol();
    strcpy(buf, "N");
    getdata(t_lines - 1, 0, "ÄúÈ·¶¨ÒªÊ¹ÓÃZmodem´«ÊäÎÄ¼şÃ´?[y/N]", buf, 2, DOECHO, NULL, true);
    if (toupper(buf[0]) != 'Y')
        return FULLUPDATE;
    strncpy(buf, title, 76);
    buf[80] = '\0';
    escape_filename(buf);
    strcat(buf, ".TXT");
    move(t_lines - 2, 0);
    clrtoeol();
    prints("ÇëÊäÈëÎÄ¼şÃû£¬Îª¿ÕÔò·ÅÆú");
    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, "", buf, 78, DOECHO, NULL, 0);
    if (buf[0] == '\0')
        return FULLUPDATE;
    buf[78] = '\0';
    escape_filename(buf);
    sprintf(buf1, "SMTH-%s-", currboard->filename);
    strcat(buf1, buf);
    bbs_zsendfile(filename, buf1);
    return FULLUPDATE;
}

int my_inet_aton(const char * ip, struct in_addr* queryip)
{
	char* p1;
	unsigned int i1,i2,i3,i4;
	unsigned char i[4];

	p1=(char*)ip;
	i1=atoi(p1);
	if (i1>255) return 0;
	i[0]=i1;
	while (*p1&&*p1!='.') {
		if (!isdigit(*p1))
			return 0;
		p1++;
	}
	if (!*p1) return 0;
	p1++;

	i2=atoi(p1);
	i[1]=i2;
	if (i2>255) return 0;
	while (*p1&&*p1!='.') {
		if (!isdigit(*p1))
			return 0;
		p1++;
	}
	if (!*p1) return 0;
	p1++;

	i3=atoi(p1);
	if (i3>255) return 0;
	i[2]=i3;
	while (*p1&&*p1!='.') {
		if (!isdigit(*p1))
			return 0;
		p1++;
	}
	if (!*p1) return 0;
	p1++;

	i4=atoi(p1);
	if (i4>255) return 0;
	i[3]=i4;
	while (*p1) {
		if (!isdigit(*p1))
			return 0;
		p1++;
	}
	if (!*p1) {
		memcpy(queryip,i,4);
		return 1;
	}

	return 0;
}

int find_ip(ip,flag,result)
const char *ip;
int flag;
char result[];
/* ²éÕÒIPµÄÖ÷³ÌĞò£¬flag£º1	ÆÕÍ¨µ÷ÓÃ£¬´òÓ¡ËùÓĞĞÅÏ¢ */
/*			 0	ÔÚÉóÅúÊ±µ÷ÓÃ£¬Ö»·µ»Ø½á¹ûµ½result */
/* ·µ»Ø£º	0 Õı³£ */
/* 		1 Êı¾İÎÄ¼şÎŞ·¨´ò¿ª */
/*		2 ´íÎóIP */
/*		3 Êı¾İ¿âÎŞ¸ÃÏîÄÚÈİ */
/* commented by Bigman: 2002.8.20 */
{
    FILE *fn; 
    int num=0;

    unsigned long temp_num;
    struct in_addr queryip;
    char linebuf[256];

    fn = fopen("etc/ip_arrange.txt", "rt");

    if (fn == NULL) {
        strncpy(result,"Ã»ÕÒµ½ip_arrange.txt",255);
        return 1;
    }

	temp_num=0;

        if (my_inet_aton(ip, &queryip) == 0) {
            strncpy(result,"´íÎóµÄip",254);
	    fclose(fn);
            return 2;
	}

	queryip.s_addr=ntohl(queryip.s_addr);
	fseek(fn,0,SEEK_SET);

	linebuf[255] = 0;

        while (fgets(linebuf, 254, fn)) {
            char *p1, *p2;
            struct in_addr from, to;
	    char* tostr;

            p2 = p1 = linebuf;
            while ((*p2) && (*p2 != ' ') && *p2 != '\t')
                p2++;
            if (!(*p2))
                continue;
            *p2 = 0;
            if (my_inet_aton(p1, &from) == 0)
                continue;
	    from.s_addr=ntohl(from.s_addr);
            p1 = p2 + 1;
            while ((*p1) && (*p1 == ' ') && *p1 == '\t')
                p1++;
            if (!(*p1))
                continue;
            p2 = p1;

            while ((*p2) && (*p2 != ' ') && *p2 != '\t')
                p2++;
            if (!(*p2))
                continue;
            *p2 = 0;
            if (my_inet_aton(p1, &to) == 0)
                continue;
	    to.s_addr=ntohl(to.s_addr);
	    tostr=p1;

            p1 = p2 + 1;
            while ((*p1) && (*p1 == ' ') && *p1 == '\t')
                p1++;
            if (!(*p1))
                continue;

	    if (from.s_addr==0) continue;
            if (((queryip.s_addr >= from.s_addr) && (queryip.s_addr <= to.s_addr) && (from.s_addr<=to.s_addr)) || 
                ((queryip.s_addr >= to.s_addr) && (queryip.s_addr <= from.s_addr) && (from.s_addr>=to.s_addr)))
               	{
			num++;
			if (flag==1) prints("%s %s %s", linebuf,tostr,p1);
			if ((temp_num == 0) || (temp_num >= (abs(to.s_addr-from.s_addr))))
			{
				temp_num=abs(to.s_addr-from.s_addr);
				strncpy(result,p1,254);
			}
		}
        }

	if (num==0) {
		strncpy(result,"Êı¾İ¿âÔİÎŞ",254);
		fclose(fn);
		return(3);
	}

    fclose(fn);
    return 0;
}

int search_ip()
/* ´Ó¹ÜÀí²Ëµ¥µ÷ÓÃ²éÑ¯IP³ÌĞò */
/* Bigman: 2002.8.20 */
/* ·µ»Ø£º0 Õı³£ */
/*       1 Êı¾İÎÄ¼şÎŞ·¨´ò¿ª */
{
	char ip[17];
	char result[256];
	int back_flag;

	clear();
	while (1) {
            
		getdata(0, 0, "ÊäÈë²éÑ¯µÄIP(Ö±½Ó»Ø³µÍË³ö):", ip, 16, DOECHO, NULL, true);

		if (ip[0] == 0) {
			return 0;
		}
		prints("%s ²éÑ¯½á¹û:\n",ip);
		clrtobot();

		back_flag=find_ip(ip,1,result);

		prints("\033[33m%s\033[m\n",result);

		if (back_flag == 1) {
		pressreturn();
		return(1);
		}

	}
	return(0);
}

int kick_all_user()
{
    int id, ind;
    struct user_info *uin;
	char ans[3];
	int i;

    if (!check_systempasswd()) {
        return;
    }
	clear();
	move(3,0);
	prints("ÓÑÇéÌáĞÑ:\n");
	prints("    ÇëÏÈ¹Ø±Õ bbsd, sshbbsd, httpd");
	getdata(7, 0, "È·¶¨ÒªÌßµôËùÓĞÔÚÏßÓÃ»§? (y/N) [N]:", ans, 3, DOECHO, NULL, true);
	if(ans[0] != 'y' && ans[0] != 'Y')
		return 0;

	for(i=0; i < USHM_SIZE ; i++){
		uin = get_utmpent(i);
		if( !uin || !uin->uid )
			continue;
		move(10,0);
		clrtoeol();
		prints("ÕıÔÚÌß  %s\n",uin->userid);
		if( uin->pid == uinfo.pid )
			continue;

	    if (uin->mode == WEBEXPLORE)
	        clear_utmp(i, uin->uid, uin->pid);

	    if ( !uin->active || (kill(uin->pid, 0) == -1)) {
	        continue;
	    }
	    if (kill(uin->pid, SIGHUP) == -1) {
	        clear_utmp(i, uin->uid, uin->pid);
	    }
	}

	move(13,0);
	prints("Çå³ıÍê±Ï\n");
	pressanykey();
    return 1;
}
