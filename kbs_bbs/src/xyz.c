/*
	This file has been checked global varible.
	一些杂项的功能
*/

#include "bbs.h"
extern int switch_code();       /* KCN,99.09.05 */
extern int convcode;            /* KCN,99.09.05 */
int modify_user_mode(mode)
int mode;
{
    if (uinfo.mode == mode)
        return 0;               /* 必须减少update_ulist的次数. ylsdd 2001.4.27 */
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
        prints("%c. 退出 ", 'A' + i);
    } else if ((arg->pbits & (1 << i)) != (arg->oldbits & (1 << i))) {
        prints("%c. %-27s \033[31;1m%3s\033[m", 'A' + i, permstrings[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
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
        prints("%c. 退出 ", 'A' + i);
    } else if ((arg->pbits & (1 << i)) != (arg->oldbits & (1 << i))) {
        prints("%c. %-40s \033[31;1m%3s\033[m", 'A' + i, user_smsdefstr[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    } else {
        prints("%c. %-40s \x1b[37;0m%3s\x1b[m", 'A' + i, user_smsdefstr[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    }
    return SHOW_CONTINUE;
}
#endif

int showuserdefine1(struct _select_def *conf, int i)
{
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    i = i - 1;
    if (i == conf->item_count - 1) {
        prints("%c. 退出 ", 'A' + i);
    } else if ((arg->pbits & (1 << i)) != (arg->oldbits & (1 << i))) {
        prints("%c. %-27s \033[31;1m%3s\033[m", 'A' + i, user_definestr[i+32], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    } else {
        prints("%c. %-27s \x1b[37;0m%3s\x1b[m", 'A' + i, user_definestr[i+32], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    }
    return SHOW_CONTINUE;
}

int showuserdefine(struct _select_def *conf, int i)
{
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    i = i - 1;
    if (i == conf->item_count - 1) {
        prints("%c. 退出 ", 'A' + i);
    } else if ((arg->pbits & (1 << i)) != (arg->oldbits & (1 << i))) {
        prints("%c. %-27s \033[31;1m%3s\033[m", 'A' + i, user_definestr[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
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
        prints("%c. 退出 ", 'A' + i);
    } else {
        if ((arg->pbits & (1 << i)) != (arg->oldbits & (1 << i))) {
            prints("%c. %-27s \033[31;1m%3s\033[m", 'A' + i, permstrings[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
        } else if ((1 << i == PERM_BASIC || 1 << i == PERM_POST || 1 << i == PERM_CHAT || 1 << i == PERM_PAGE || 1 << i == PERM_DENYMAIL|| 1 << i == PERM_DENYRELAX) && (arg->basic & (1 << i)))
            prints("%c. %-27s \033[32;1m%3s\033[m", 'A' + i, permstrings[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
        else
            prints("%c. %-27s \033[37;0m%3s\033[m", 'A' + i, permstrings[i], ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
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
    prints("请按下你要的代码来设定%s, Ctrl+Q退出，Ctrl+A放弃修改退出.\n", prompt);
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
    perm_conf.prompt = "◆";
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

/* 删除过期的帐号 */
/* 算是给奥运的献礼 */
/* Bigman 2001.7.14 */
int confirm_delete_id()
{
    char buff[STRLEN];

    if (!HAS_PERM(currentuser, PERM_ADMIN)) {
        move(3, 0);
        clrtobot();
        prints("抱歉, 只有总管理员才能修改");
        pressreturn();
        return 1;
    }

    modify_user_mode(ADMIN);
    clear();
    move(8, 0);
    prints("\n");
    clrtoeol();
    getdata(9,0,"请输入要确认清除的用户ID: ", genbuf, IDLEN+1,1,NULL,true);
    if (genbuf[0] == '\0') {
        clear();
        return 1;
    }

    if ((usernum = searchuser(genbuf)) != 0) {
        prints("此帐号有人使用\n");
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
    int lcount = 0, basicperm;
    int s[10][2];

    /*
     * add by alex, 97.7 , strict the power of sysop 
     */
    if (!HAS_PERM(currentuser, PERM_ADMIN) || !HAS_PERM(currentuser, PERM_SYSOP)) {
        move(3, 0);
        clrtobot();
        prints("抱歉, 只有ADMIN权限的管理员才能修改其他用户权限");
        pressreturn();
        return 0;
    }

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return 0;
    }
    clear();
    move(0, 0);
    prints("更改" NAME_USER_SHORT "的权限(\x1b[m注意：如果是封禁解封，请使用封禁选单！\x1b[m\n");
    clrtoeol();
    move(1, 0);
    usercomplete("请输入" NAME_USER_SHORT " ID: ", genbuf);
    if (genbuf[0] == '\0') {
        clear();
        return 0;
    }
    if (!(id = getuser(genbuf, &lookupuser))) {
        move(3, 0);
        prints("非法 ID");
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

/*Bad 2002.7.6 受限与戒网问题*/
    lcount=get_giveupinfo(lookupuser->userid,&basicperm,s);
    move(1, 0);
    clrtobot();
    move(2, 0);
    prints("请设定" NAME_USER_SHORT " '%s' 的权限\n", genbuf);
    newlevel = setperms(lookupuser->userlevel, basicperm, "权限", NUMPERMS, setperm_show, NULL);
    move(2, 0);
    if (newlevel == lookupuser->userlevel)
        prints(NAME_USER_SHORT " '%s' 的权限没有更改\n", lookupuser->userid);
    else {                      /* Leeward: 1997.12.02 : Modification starts */
        char secu[STRLEN];

        sprintf(secu, "修改 %s 的权限XPERM%d %d", lookupuser->userid, lookupuser->userlevel, newlevel);
        securityreport(secu, lookupuser, NULL);
        lookupuser->userlevel = newlevel;
        /*
         * Leeward: 1997.12.02 : Modification stops 
         */

        prints(NAME_USER_SHORT " '%s' 的权限已更改,注意：如果是封禁解封，\n", lookupuser->userid);
        sprintf(genbuf, "changed permissions for %s", lookupuser->userid);
        bbslog("user","%s",genbuf);
        /*
         * Haohmaru.98.10.03.给新任版主自动发信 
         */
//        if ((lookupuser->userlevel & PERM_BOARDS) && flag == 0)
//            mail_file(currentuser->userid, "etc/forbm", lookupuser->userid, "新任" NAME_BM "必读", BBSPOST_LINK, NULL);
        /*
         * Bigman 2000.1.5 修改权限自动发信 
         */
        if ((lookupuser->userlevel & PERM_CLOAK) && flag1 == 0)
            mail_file(currentuser->userid, "etc/forcloak", lookupuser->userid, NAME_SYSOP_GROUP "授予您隐身权限", BBSPOST_LINK, NULL);
        if ((lookupuser->userlevel & PERM_XEMPT) && flag2 == 0)
            mail_file(currentuser->userid, "etc/forlongid", lookupuser->userid, NAME_SYSOP_GROUP "授予您长期帐号权限", BBSPOST_LINK, NULL);
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
        prints("抱歉, 您没有此权限");
        pressreturn();
        return -1;
    }

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    clear();
    move(0, 0);
    prints("列示具有特定权限的" NAME_USER_SHORT "的资料\n");
    clrtoeol();
    move(2, 0);
    prints("请设定需要检查的权限\n");
    scanuser.userlevel = 0;
    /*
     * change showperminfoX to showperminfo 
     */
    newlevel = setperms(scanuser.userlevel, 0, "权限", NUMPERMS, showperminfo, NULL);
    move(2, 0);
    if (newlevel == scanuser.userlevel)
        prints("你没有设定任何权限\n");
    else {
        char secu[STRLEN];
        char buffer[256];
        int fhp;
        FILE *fpx;
        long count = 0L;

		gettmpfilename( buffer, "XCL" );
        //sprintf(buffer, "tmp/XCL.%s%d", currentuser->userid, getpid());
        if (-1 == (fhp = open(".PASSWDS", O_RDONLY))) {
            prints("系统错误: 无法打开口令文件\n");
        } else if (NULL == (fpx = fopen(buffer, "w"))) {
            close(fhp);
            prints("系统错误: 无法打开临时文件\n");
        } else {
            prints("列示操作可能需要较长时间才能完成, 请耐心等待. ");
            clrtoeol();
            if (askyn("你确定要进行列示吗", 0)) {
                while (read(fhp, &scanuser, sizeof(struct userec)) > 0) {
                    if ((scanuser.userlevel & newlevel) == newlevel && strcmp("SYSOP", scanuser.userid)) {
						struct userdata ud;

						read_userdata(scanuser.userid, &ud);
                        count++;
                        fprintf(fpx, "\033[1m\033[33m请保持这一行位于屏幕第一行，此时按 X 键可给下列用户发信要求其补齐个人注册资料\033[m\n\n");
                        fprintf(fpx, "用户代号(昵称) : %s(%s)\n\n", scanuser.userid, scanuser.username);
                        fprintf(fpx, "真  实  姓  名 : %s\n\n", ud.realname);
                        fprintf(fpx, "居  住  住  址 : %s\n\n", ud.address);
                        fprintf(fpx, "电  子  邮  件 : %s\n\n", ud.email);
                        fprintf(fpx, "单位$电话@认证 : %s\n\n", ud.realemail);
                        fprintf(fpx, "注  册  日  期 : %s\n", ctime(&scanuser.firstlogin));
                        fprintf(fpx, "最后的登录日期 : %s\n", ctime(&scanuser.lastlogin));
                        fprintf(fpx, "最后的登录机器 : %s\n\n", scanuser.lasthost);
                        fprintf(fpx, "上  站  次  数 : %d 次\n\n", scanuser.numlogins);
                        fprintf(fpx, "文  章  数  目 : %d 篇\n\n", scanuser.numposts);
                    }
                }
                fprintf(fpx, "\033[1m\033[33m一共列出了 %ld 项具有此权限的用户资料\033[m\n\n*** 这是列示结果的最后一行．如果检查完毕，请按 q 键结束 *** (以下均为空行)", count);
                {
                    int dummy;  /* process the situation of a too high screen :PP */

                    for (dummy = 0; dummy < t_lines * 4; dummy++)
                        fputs("\n", fpx);
                }
                close(fhp);
                fclose(fpx);

                sprintf(secu, "\033[1m\033[33m一共列出了 %ld 项具有此权限的用户资料\033[m", count);
                move(2, 0);
                prints(secu);
                clrtoeol();
                sprintf(genbuf, "listed %ld userlevel of %d", count, newlevel);
                bbslog("user","%s",genbuf);
                pressanykey();

                /*
                 * sprintf(secu, "列示具有特定权限的 %ld 个用户的资料", count); 
                 */
                clear();
                ansimore(buffer, false);
                clear();
                move(2, 0);
                prints("列示操作完成");
                clrtoeol();

                unlink(buffer);
            } else {
                move(2, 0);
                prints("取消列示操作");
                clrtoeol();
            }
        }
    }
    pressreturn();
    clear();
    return 0;
}

int Xdelipacl()
{
    int id;
    struct userec *lookupuser;

    if (!HAS_PERM(currentuser, PERM_SYSOP)) {
        move(3, 0);
        clrtobot();
        prints("抱歉, 只有SYSOP权限的管理员才能修改其他用户权限");
        pressreturn();
        return 0;
    }

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return 0;
    }
    clear();
    move(0, 0);
    prints("删除" NAME_USER_SHORT "的IP控制\n");
    clrtoeol();
    move(1, 0);
    usercomplete("请输入" NAME_USER_SHORT " ID: ", genbuf);
    if (genbuf[0] == '\0') {
        clear();
        return 0;
    }
    if (!(id = getuser(genbuf, &lookupuser))) {
        move(3, 0);
        prints("非法 ID");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    sprintf(genbuf, "home/%c/%s/ipacl", toupper(lookupuser->userid[0]), lookupuser->userid);
    unlink(genbuf);
    clear();
    return 0;
}

int Xdeljunk()
{
    char buf[256];

    if (!HAS_PERM(currentuser, PERM_SYSOP)) {
        move(3, 0);
        clrtobot();
        prints("抱歉, 只有SYSOP权限的管理员才能修改其他用户权限");
        pressreturn();
        return 0;
    }

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return 0;
    }
    clear();
    move(0, 0);
    prints("删除版面垃圾箱\n");
    clrtoeol();
    move(1, 0);
    make_blist(0);
    namecomplete("输入讨论区名称: ", genbuf);
    if (genbuf[0] == '\0') {
        clear();
        return 0;
    }
    sprintf(buf, "boards/%s/.DELETED", genbuf);
    unlink(buf);
    sprintf(buf, "boards/%s/.JUNK", genbuf);
    unlink(buf);
    clear();
    return 0;
}

#ifdef SMS_SUPPORT
int x_usersmsdef()
{
    unsigned int newlevel;

    modify_user_mode(USERDEF);
	clear();
    if (!strcmp(currentuser->userid, "guest"))
        return 0;

	if( ! currentmemo->ud.mobileregistered ){
        move(3, 0);
		prints("您没有注册手机号码");
        pressreturn();
        return 0;
    }
    move(2, 0);
    newlevel = setperms(currentmemo->ud.smsdef, 0, "短信参数", NUMSMSDEF, showsmsdef, NULL);
    move(2, 0);
    if (newlevel == currentmemo->ud.smsdef)
        prints("参数没有修改...\n");
    else {
        currentmemo->ud.smsdef = newlevel;
		write_userdata(currentuser->userid, &(currentmemo->ud) );
        prints("新的参数设定完成...\n\n");
    }
    pressreturn();
    clear();
    return 0;
}
#endif

int x_userdefine1()
{
    int id;
    unsigned int newlevel;
    struct userec *lookupuser;

    modify_user_mode(USERDEF);
    if (!(id = getuser(currentuser->userid, &lookupuser))) {
        move(3, 0);
        prints("错误的 " NAME_USER_SHORT " ID...");
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
    newlevel = setperms(lookupuser->userdefine[1], 0, "参数", NUMDEFINES-32, showuserdefine1, NULL);
    move(2, 0);
    if (newlevel == lookupuser->userdefine[1])
        prints("参数没有修改...\n");
    else {
        lookupuser->userdefine[1] = newlevel;
        currentuser->userdefine[1] = newlevel;
        prints("新的参数设定完成...\n\n");
    }
    pressreturn();
    clear();
    return 0;
}

int x_userdefine()
{
    int id;
    unsigned int newlevel;
    extern int nettyNN;
    struct userec *lookupuser;

    modify_user_mode(USERDEF);
    if (!(id = getuser(currentuser->userid, &lookupuser))) {
        move(3, 0);
        prints("错误的 " NAME_USER_SHORT " ID...");
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
    newlevel = setperms(lookupuser->userdefine[0], 0, "参数", 31, showuserdefine, NULL);
    move(2, 0);
    if (newlevel == lookupuser->userdefine[0])
        prints("参数没有修改...\n");
    else {
        lookupuser->userdefine[0] = newlevel;
        currentuser->userdefine[0] = newlevel;
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
        prints("新的参数设定完成...\n\n");
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
        prints("隐身术 (cloak) 已经%s了!", (uinfo.invisible) ? "启动" : "停止");
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
    prints("目前系统日期与时间: %s", ctime(&t));
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
    char *explain_file[] = { "个人说明档", "签名档", "自己的备忘录", "离站的画面", NULL };

    modify_user_mode(GMENU);
    clear();
    move(1, 0);
    prints("编修个人档案\n\n");
    for (num = 0; e_file[num] != NULL && explain_file[num] != NULL; num++) {
        prints("[\033[32m%d\033[m] %s\n", num + 1, explain_file[num]);
    }
    prints("[\033[32m%d\033[m] 都不想改\n", num + 1);

    getdata(num + 5, 0, "你要编修哪一项个人档案: ", ans, 2, DOECHO, NULL, true);
    if (ans[0] - '0' <= 0 || ans[0] - '0' > num || ans[0] == '\n' || ans[0] == '\0')
        return;

    ch = ans[0] - '0' - 1;

    sethomefile(genbuf, currentuser->userid, e_file[ch]);
    move(3, 0);
    clrtobot();
    sprintf(buf, "(E)编辑 (D)删除 %s? [E]: ", explain_file[ch]);
    getdata(3, 0, buf, ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'D' || ans[0] == 'd') {
        my_unlink(genbuf);
        move(5, 0);
        prints("%s 已删除\n", explain_file[ch]);
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
        prints("%s 更新过\n", explain_file[ch]);
        sprintf(buf, "edit %s", explain_file[ch]);
        if (!strcmp(e_file[ch], "signatures")) {

			currentmemo->ud.signum = calc_numofsig(currentuser->userid);
			write_userdata( currentuser->userid, &(currentmemo->ud) );

            if (currentmemo->ud.signum&&(currentuser->signature==0))
            	currentuser->signature=1;
            prints("系统重新设定以及读入你的签名档...");
        }
        bbslog("user","%s",buf);
    } else
        prints("%s 取消修改\n", explain_file[ch]);
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
        #ifdef FLOWBANNER
        "banner",
        #endif
        NULL
    };

    /*
     * "/usr/share/apache/htdocs/script/menucontext.js", NULL};
     */
    static const char *explain_file[] =
        { "Welcome", "公用备忘录", "进站欢迎档", "活动看版", "离站画面", "menu.ini", "穿梭IP", "身份确认档", "注册单完成档", "注册单失败档(真实姓名)", "注册单失败档(服务单位)",
        "注册单失败档(居住地址)", "注册单失败档(联络电话)", "注册单失败档(真实资料)", "注册单失败档(中文填写)", "注册单失败档(过多的ID)", "注册单失败档(不能穿梭注册)",
        "身份确认完成档        ",
        "身份确认失败档",
        "不可注册的 ID         ", "不可登录的 IP", "系统自动过滤的词语    ",
        "sysconf.ini", "WWW主菜单             ", "近期热点", "系统热点",
        "给新任版主的信        ", "给隐身用户的信",
        "给长期用户的信        ",
        "转信版和新闻组对应",
        "封禁理由列表", "新用户个人定制区","给新注册用户的信", 
        #ifdef HAVE_CUSTOM_USER_TITLE
        "用户职务表",
        #endif
        #ifdef FLOWBANNER
        "全站流动信息",
        #endif
        NULL
    };

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return;
    }
    clear();
    move(0, 0);
    prints("编修系统档案\n\n");
    for (num = 0; e_file[num] != NULL && explain_file[num] != NULL; num++) {
        prints("[\033[32m%2d\033[m] %s%s", num + 1, explain_file[num], ((num + 1) % 2) ? "      " : "\n");
        /*
         * Leeward 98.03.29 调整显示布局，以便加入“系统自动过滤的词语”一项
         */
        /*
         * Leeward 98.07.31 调整显示布局，以便加入“不可登录的 IP”一项
         */
        /*
         * Bigman 2001.6.23 调整布局， 增加其他几项内容
         */
    }

    prints("[\033[32m%2d\033[m] 都不想改\n", num + 1);

    /*
     * 下面的21是行号，以后添加，可以相应调整
     */
    getdata(21, 0, "你要编修哪一项系统档案: ", ans, 3, DOECHO, NULL, true);
    ch = atoi(ans);
    if (!isdigit(ans[0]) || ch <= 0 || ch > num || ans[0] == '\n' || ans[0] == '\0')
        return;
    ch -= 1;
    sprintf(genbuf, "etc/%s", e_file[ch]);
    move(2, 0);
    clrtobot();
    sprintf(buf, "(E)编辑 (D)删除 %s? [E]: ", explain_file[ch]);
    getdata(3, 0, buf, ans, 2, DOECHO, NULL, true);
    if (ans[0] == 'D' || ans[0] == 'd') {
        {
            char secu[STRLEN];

            sprintf(secu, "删除系统档案：%s", explain_file[ch]);
            securityreport(secu, NULL, NULL);
        }
        my_unlink(genbuf);
        move(5, 0);
        prints("%s 已删除\n", explain_file[ch]);
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
        prints("%s 更新过", explain_file[ch]);
        sprintf(buf, "edit %s", explain_file[ch]);
        bbslog("user","%s",buf);
        {
            char secu[STRLEN];

            sprintf(secu, "修改系统档案：%s", explain_file[ch]);
            securityreport(secu, NULL, NULL);
        }

        if (!strcmp(e_file[ch], "../Welcome")) {
            my_unlink("Welcome.rec");
            prints("\nWelcome 记录档更新");
        }
#ifdef FILTER
	if (!strcmp(e_file[ch], "badword")) {
            my_unlink(BADWORD_IMG_FILE);
            prints("\n过滤词表更新");
	}
#endif
#ifdef HAVE_CUSTOM_USER_TITLE
       if (!strcmp(e_file[ch],"../" USER_TITLE_FILE)) {
          load_user_title();
       }
#endif
#ifdef FLOWBANNER
       if (!strcmp(e_file[ch],"banner")) {
          load_site_banner(0);
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
    char buf[100], buf1[100];

    ansimore("etc/zmodem", 0);
    move(t_lines - 1, 0);
    clrtoeol();
    strcpy(buf, "N");
    getdata(t_lines - 1, 0, "您确定要使用Zmodem传输文件么?[y/N]", buf, 2, DOECHO, NULL, true);
    if (toupper(buf[0]) != 'Y')
        return 1;
    strncpy(buf, title, 76);
    buf[80] = '\0';
    escape_filename(buf);
    strcat(buf, ".TXT");
    move(t_lines - 2, 0);
    clrtoeol();
    prints("请输入文件名，为空则放弃");
    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, "", buf, 78, DOECHO, NULL, 0);
    if (buf[0] == '\0')
        return 0;
    buf[78] = '\0';
    escape_filename(buf);
    sprintf(buf1, "SMTH-%s-", currboard->filename);
    strcat(buf1, buf);
    bbs_zsendfile(filename, buf1);
    return 0;
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

int find_ip( const char *ip, int flag, char *result)
/* 查找IP的主程序，flag：1	普通调用，打印所有信息 */
/*			 0	在审批时调用，只返回结果到result */
/* 返回：	0 正常 */
/* 		1 数据文件无法打开 */
/*		2 错误IP */
/*		3 数据库无该项内容 */
/* commented by Bigman: 2002.8.20 */
{
    FILE *fn; 
    int num=0;

    unsigned long temp_num;
    struct in_addr queryip;
    char linebuf[256];

    fn = fopen("etc/ip_arrange.txt", "rt");

    if (fn == NULL) {
        strncpy(result,"没找到ip_arrange.txt",255);
        return 1;
    }

	temp_num=0;

        if (my_inet_aton(ip, &queryip) == 0) {
            strncpy(result,"错误的ip",254);
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
		strncpy(result,"数据库暂无",254);
		fclose(fn);
		return(3);
	}

    fclose(fn);
    return 0;
}

int search_ip()
/* 从管理菜单调用查询IP程序 */
/* Bigman: 2002.8.20 */
/* 返回：0 正常 */
/*       1 数据文件无法打开 */
{
	char ip[17];
	char result[256];
	int back_flag;

	clear();
	while (1) {
            
		getdata(0, 0, "输入查询的IP(直接回车退出):", ip, 16, DOECHO, NULL, true);

		if (ip[0] == 0) {
			return 0;
		}
		prints("%s 查询结果:\n",ip);
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
    struct user_info *uin;
	char ans[3];
	int i;

    if (!check_systempasswd()) {
        return -1;
    }
	clear();
	move(3,0);
	prints("友情提醒:\n");
	prints("    请先关闭 bbsd, sshbbsd, httpd");
	getdata(7, 0, "确定要踢掉所有在线用户? (y/N) [N]:", ans, 3, DOECHO, NULL, true);
	if(ans[0] != 'y' && ans[0] != 'Y')
		return 0;

	for(i=0; i < USHM_SIZE ; i++){
		uin = get_utmpent(i);
		if( !uin || !uin->uid )
			continue;
		move(10,0);
		clrtoeol();
		prints("正在踢  %s\n",uin->userid);
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
	prints("清除完毕\n");
	pressanykey();
    return 1;
}
