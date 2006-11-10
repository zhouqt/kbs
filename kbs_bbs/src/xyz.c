/*
	This file has been checked global varible.
	一些杂项的功能
*/

#include "bbs.h"
extern int convcode;            /* KCN,99.09.05 */

int modify_user_mode(int mode){
    int ret=uinfo.mode;
    if(uinfo.mode!=mode){
        uinfo.mode=mode;
        UPDATE_UTMP(mode,uinfo);
    }
    return ret;
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
int confirm_delete_id(void){
    char buff[STRLEN];
    int usernum;

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

int x_level(void){
    unsigned int newlevel;
    int flag = 0;               /*Haohmaru,98.10.05 */
    int flag1 = 0, flag2 = 0;   /* bigman 2000.1.5 */
    struct userec *lookupuser;
    int id,basicperm,s[GIVEUPINFO_PERM_COUNT];

    /*
     * add by alex, 97.7 , strict the power of sysop 
     */
#ifdef SOURCE_PERM_CHECK
    if (!HAS_PERM(getCurrentUser(), PERM_ADMIN)) {
        move(3, 0);
        clrtobot();
        prints("抱歉, 您没有 ADMIN 权限!");
        pressreturn();
        return 0;
    }
#endif
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
    basicperm=get_giveupinfo(lookupuser,s);
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
        securityreport(secu, lookupuser, NULL, getSession());
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
//            mail_file(getCurrentUser()->userid, "etc/forbm", lookupuser->userid, "新任" NAME_BM "必读", BBSPOST_LINK, NULL);
        /*
         * Bigman 2000.1.5 修改权限自动发信 
         */
        if ((lookupuser->userlevel & PERM_CLOAK) && flag1 == 0)
            mail_file(getCurrentUser()->userid, "etc/forcloak", lookupuser->userid, NAME_SYSOP_GROUP "授予您隐身权限", BBSPOST_LINK, NULL);
        if ((lookupuser->userlevel & PERM_XEMPT) && flag2 == 0)
            mail_file(getCurrentUser()->userid, "etc/forlongid", lookupuser->userid, NAME_SYSOP_GROUP "授予您长期帐号权限", BBSPOST_LINK, NULL);
		save_giveupinfo(lookupuser,s);
    }
    pressreturn();
    clear();
    return 0;
}



/*etnlegend,2005.07.10*/
struct check_level_arg{
    int check_mode;
    int count;
    unsigned int check_level;
    FILE *log_file;
};
static int check_level_func(struct userec *user,struct check_level_arg* arg){
    char perm[32];
    if(!user->userid[0])
        return 0;
    if(!arg->check_mode){
        if((user->userlevel&arg->check_level)^arg->check_level)
            return 0;
    }
    else{
        if(!(user->userlevel&arg->check_level))
            return 0;
    }
    fprintf(arg->log_file,"%-24.24s%s\n",user->userid,gen_permstr(user->userlevel,perm));
    arg->count++;
    return 0;
}
int XCheckLevel(void){
    struct check_level_arg arg;
    char buf[40],perm[32];
    int i;
    modify_user_mode(ADMIN);
    if(!check_systempasswd())
        return -1;
    clear();
    move(0,0);prints("\033[1;32m查阅具有特定权限的用户\033[m");
    move(2,0);prints("设定需要查阅的权限:");
    arg.check_level=setperms(0,0,"权限",NUMPERMS,showperminfo,NULL);
    for(arg.count=0,i=0;i<NUMPERMS;i++)
        if(arg.check_level&(1<<i))
            arg.count++;
    if(!arg.count){
        move(2,0);clrtoeol();
        prints("未设定需要查阅的权限,放弃操作...");
        pressreturn();
        return -1;
    }
    arg.check_mode=0;
    if(arg.count>1){
        do{
            move(2,0);clrtoeol();
            getdata(2,0,"已设定多个需要查阅的权限,请选择逻辑关系{与(And)|或(Or)} [A]: ",buf,2,DOECHO,NULL,true);
        }
        while(buf[0]&&!(buf[0]=='a'||buf[0]=='o'||buf[0]=='A'||buf[0]=='O'));
        arg.check_mode=(buf[0]=='o'||buf[0]=='O')?1:0;
    }
    sprintf(buf,"tmp/check_level_%ld_%d",time(NULL), (int)getpid());
    if(!(arg.log_file=fopen(buf,"w"))){
        move(2,0);clrtoeol();
        prints("创建临时文件错误,操作中断...");
        pressreturn();
        return -1;
    }
    fprintf(arg.log_file,"\033[1;32m查阅具有特定权限的用户·查询结果\033[m\n\n");
    fprintf(arg.log_file,"权限设定 \033[1;33m%s\033[m <%s>\n\n",gen_permstr(arg.check_level,perm),arg.check_mode?"OR":"AND");
    arg.count=0;
    apply_users((int (*)(struct userec*,void*))check_level_func,&arg);
    fprintf(arg.log_file,"\n共 \033[1;33m%d\033[m 位用户符合查询条件\n",arg.count);
    fclose(arg.log_file);
    move(2,0);clrtoeol();
    prints("\033[1;36m查阅具有特定权限的用户·查询结果 已回寄, 请检查信件...\033[m");
    mail_file(getCurrentUser()->userid,buf,getCurrentUser()->userid,"查阅具有特定权限的用户·查询结果",BBSPOST_MOVE,NULL);
    securityreport("查阅具有特定权限的用户",NULL,NULL, getSession());
    bbslog("user","%s","查阅具有特定权限的用户");
    pressreturn();clear();
    return 0;
}

int Xdelipacl(void){
    int id;
    struct userec *lookupuser;

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

int Xdeljunk(void){
    char buf[256], board[STRLEN];

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
    namecomplete("输入讨论区名称: ", board);
    if (board[0] == '\0') {
        clear();
        return 0;
    }
    sprintf(buf, "确认删除 %s 版的垃圾箱和废纸篓吗", board);
    if (askyn(buf, false) == false) return 0;
    sprintf(buf, "boards/%s/.DELETED", board);
    unlink(buf);
    sprintf(buf, "boards/%s/.JUNK", board);
    unlink(buf);
    sprintf(buf, "%s 清空 %s 版面垃圾箱", getCurrentUser()->userid, board);
    securityreport(buf, NULL, NULL, getSession());
    clear();
    return 0;
}

/*得到别人的收藏夹和未度标记*/
int get_favread(void){
	char destid[IDLEN+1];
	char passwd[PASSLEN+1];
    struct userec *destuser;
	char dpath[PATHLEN];
	char mypath[PATHLEN];
	int count;

	clear();
	move(1,0);
	prints("同步他人收藏夹,好友名单和未读标记到本ID.\n会导致本ID原来的收藏夹,好友名单和未读标记丢失，慎用\n");
    getdata(5, 0, "请输入对方ID:", destid, IDLEN+1, DOECHO, NULL, true);
	if(destid[0] == '\0' || destid[0] == '\n'){
		clear();
		return 0;
	}
    if(!getuser(destid,&destuser)){
		move(7,0);
		prints("没有这个用户\n");
		pressanykey();
		return 0;
	}
    if (!strcmp(destuser->userid,getCurrentUser()->userid)) {
		move(7,0);
		prints("不要自己拷贝自己，不好玩的说\n");
		pressanykey();
		return 0;
	}
    getdata(6, 0, "请输入对方密码:", passwd, PASSLEN+1, NOECHO, NULL, true);
    if(!passwd[0]){
        move(8,0);
        prints("取消...");
        pressanykey();
        return 0;
    }
    if(!checkpasswd2(passwd,destuser)){
        logattempt(destuser->userid, getSession()->fromhost, "sync");
		move(8,0);
		prints("密码错误\n");
		pressanykey();
		return 0;
	}
	move(8,0);
	prints("\033[32m为了保证数据同步性，操作前请先退出本id其他登录\033[m\n");
	prints("\033[31m本次操作会覆盖本id原收藏夹、好友名单和未读标记，无法恢复\033[m");
	getdata(10,0,"确信要进行此操作吗? [y/N] ", passwd, 2, DOECHO, NULL, true);
	if (passwd[0] != 'y' && passwd[0] != 'Y'){
		clear();
		return 0;
	}

    count=0;
    getdata(12,0,"同步个人定制? [Y]: ",passwd,2,DOECHO,NULL,true);
    if(toupper(passwd[0])!='N'){
        sethomefile(dpath,destuser->userid,"favboard");
        sethomefile(mypath,getCurrentUser()->userid,"favboard");
        f_cp(dpath,mypath,0);
        getSession()->mybrd_list_t=0;
        load_favboard(1,getSession());
        count++;
    }

#ifdef HAVE_BRC_CONTROL
    getdata(13,0,"同步未读标记? [Y]: ",passwd,2,DOECHO,NULL,true);
    if(toupper(passwd[0])!='N'){
	    sethomefile(dpath,destuser->userid,BRCFILE);
	    sethomefile(mypath,getCurrentUser()->userid,BRCFILE);
	    f_cp(dpath,mypath,0);

        if(getSession()->brc_cache_entry)
        {
		    memset(getSession()->brc_cache_entry,0,BRC_CACHE_NUM*sizeof(struct _brc_cache_entry));
		    brc_initial(getCurrentUser()->userid,DEFAULTBOARD,getSession());
		    if(currboard)
			    brc_initial(getCurrentUser()->userid,currboard->filename,getSession());
	    }
        count++;
    }
#endif

    getdata(14,0,"同步好友名单? [Y]: ",passwd,2,DOECHO,NULL,true);
    if(toupper(passwd[0])!='N'){
        sethomefile(dpath,destuser->userid,"friends");
        sethomefile(mypath,getCurrentUser()->userid,"friends");
        f_cp(dpath,mypath,0);
        getfriendstr(getCurrentUser(),get_utmpent(getSession()->utmpent),getSession());
        count++;
    }

    if(count){
	    move(16,0);
	    prints("操作成功, 您无需重新登录即可使用新数据!");
    }

    pressanykey();
	clear();
	return 0;
}

#ifdef SMS_SUPPORT
int x_usersmsdef(void){
    unsigned int newlevel;
    modify_user_mode(USERDEF);
	clear();
    if (!strcmp(getCurrentUser()->userid, "guest"))
        return 0;

	if( ! getSession()->currentmemo->ud.mobileregistered ){
        move(3, 0);
		prints("您没有注册手机号码");
        pressreturn();
        return 0;
    }
    move(2, 0);
    newlevel = setperms(getSession()->currentmemo->ud.smsdef, 0, "短信参数", NUMSMSDEF, showsmsdef, NULL);
    move(2, 0);
    if (newlevel == getSession()->currentmemo->ud.smsdef)
        prints("参数没有修改...\n");
    else {
        getSession()->currentmemo->ud.smsdef = newlevel;
		write_userdata(getCurrentUser()->userid, &(getSession()->currentmemo->ud) );
        prints("新的参数设定完成...\n\n");
    }
    pressreturn();
    clear();
    return 0;
}
#endif

int x_userdefine1(void){
    int id;
    unsigned int newlevel;
    struct userec *lookupuser;

    modify_user_mode(USERDEF);
    if (!(id = getuser(getCurrentUser()->userid, &lookupuser))) {
        move(3, 0);
        prints("错误的 " NAME_USER_SHORT " ID...");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    if (!strcmp(getCurrentUser()->userid, "guest"))
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
        getCurrentUser()->userdefine[1] = newlevel;
        prints("新的参数设定完成...\n\n");
    }
    pressreturn();
    clear();
    return 0;
}

int x_userdefine(void){
    int id;
    unsigned int newlevel;
    extern int nettyNN;
    struct userec *lookupuser;

    modify_user_mode(USERDEF);
    if (!(id = getuser(getCurrentUser()->userid, &lookupuser))) {
        move(3, 0);
        prints("错误的 " NAME_USER_SHORT " ID...");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    if (!strcmp(getCurrentUser()->userid, "guest"))
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
        getCurrentUser()->userdefine[0] = newlevel;
        if (((convcode) && (newlevel & DEF_USEGB))      /* KCN,99.09.05 */
            ||((!convcode) && !(newlevel & DEF_USEGB)))
            switch_code();
        uinfo.pager |= FRIEND_PAGER;
        if (!(uinfo.pager & ALL_PAGER)) {
            if (!DEFINE(getCurrentUser(), DEF_FRIENDCALL))
                uinfo.pager &= ~FRIEND_PAGER;
        }
        uinfo.pager &= ~ALLMSG_PAGER;
        uinfo.pager &= ~FRIENDMSG_PAGER;
        if (DEFINE(getCurrentUser(), DEF_FRIENDMSG)) {
            uinfo.pager |= FRIENDMSG_PAGER;
        }
        if (DEFINE(getCurrentUser(), DEF_ALLMSG)) {
            uinfo.pager |= ALLMSG_PAGER;
            uinfo.pager |= FRIENDMSG_PAGER;
        }
        UPDATE_UTMP(pager, uinfo);
        if (DEFINE(getCurrentUser(), DEF_ACBOARD))
            nettyNN = NNread_init();
        prints("新的参数设定完成...\n\n");
    }
    pressreturn();
    clear();
    return 0;
}

int x_cloak(void){
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

int x_date(void){
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

int x_edits(void){
    int aborted;
	int gdataret;
    char ans[7], buf[STRLEN];
    int ch, num;
    char *e_file[] = { "plans", "signatures", "notes", "logout", "myurl",
#ifdef AUTOREMAIL
			"autoremail",
#endif
			NULL };
    char *explain_file[] = { "个人说明档", "签名档", "自己的备忘录", "离站的画面", "附件链接所使用的域名",
#ifdef AUTOREMAIL
			"站内信件自动回复",
#endif
		   	NULL };

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
        return 0;

    ch = ans[0] - '0' - 1;

    sethomefile(genbuf, getCurrentUser()->userid, e_file[ch]);
    move(3, 0);
    clrtobot();
    sprintf(buf, "(E)编辑 (D)删除 %s? [E]: ", explain_file[ch]);
    gdataret = getdata(3, 0, buf, ans, 2, DOECHO, NULL, true);
	if(gdataret == -1) return 0;
    if (ans[0] == 'D' || ans[0] == 'd') {
        my_unlink(genbuf);
        move(5, 0);
        prints("%s 已删除\n", explain_file[ch]);
        sprintf(buf, "delete %s", explain_file[ch]);
		if (!strcmp(e_file[ch], "signatures")) {
			getSession()->currentmemo->ud.signum = calc_numofsig(getCurrentUser()->userid);
			write_userdata( getCurrentUser()->userid, &(getSession()->currentmemo->ud) );

            if (getSession()->currentmemo->ud.signum&&(getCurrentUser()->signature==0))
            	getCurrentUser()->signature=1;
        }else if(!strcmp(e_file[ch], "myurl")){
			get_my_webdomain(1);
		}
        bbslog("user","%s",buf);
        pressreturn();
        clear();
        return 0;
    }
    modify_user_mode(EDITUFILE);
    aborted = vedit(genbuf, false, NULL, NULL, 0);
    clear();
    if (!aborted) {
        prints("%s 更新过\n", explain_file[ch]);
        sprintf(buf, "edit %s", explain_file[ch]);
        if (!strcmp(e_file[ch], "signatures")) {

			getSession()->currentmemo->ud.signum = calc_numofsig(getCurrentUser()->userid);
			write_userdata( getCurrentUser()->userid, &(getSession()->currentmemo->ud) );

            if (getSession()->currentmemo->ud.signum&&(getCurrentUser()->signature==0))
            	getCurrentUser()->signature=1;
            prints("系统重新设定以及读入你的签名档...");
        }else if(!strcmp(e_file[ch], "myurl")){
			get_my_webdomain(1);
		}
        bbslog("user","%s",buf);
    } else
        prints("%s 取消修改\n", explain_file[ch]);
    pressreturn();
    return 0;
}

int a_edits(void){
    int aborted;
	int gdataret;
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
        "../" USER_TITLE_FILE,
        #ifdef FLOWBANNER
        "banner",
        #endif
#ifdef FB2KENDLINE
		"whatdate",
#endif
#ifdef ZIXIA
		"flinks.wForum",
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
        "用户职务表",
        #ifdef FLOWBANNER
        "全站流动信息",
        #endif
#ifdef FB2KENDLINE
		"节日信息",
#endif
#ifdef ZIXIA
		"web 友情链接",
#endif
        NULL
    };
#ifdef SOURCE_PERM_CHECK
    if (!HAS_PERM(getCurrentUser(), PERM_ADMIN)) {
        move(3, 0);
        clrtobot();
        prints("抱歉, 您没有 ADMIN 权限!");
        pressreturn();
        return 0;
    }
#endif
    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return 0;
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
        return 0;
    ch -= 1;
    sprintf(genbuf, "etc/%s", e_file[ch]);
    move(2, 0);
    clrtobot();
    sprintf(buf, "(E)编辑 (D)删除 %s? [E]: ", explain_file[ch]);
    gdataret = getdata(3, 0, buf, ans, 2, DOECHO, NULL, true);
	if(gdataret == -1) return 0;
    if (ans[0] == 'D' || ans[0] == 'd') {
        {
            char secu[STRLEN];

            sprintf(secu, "删除系统档案：%s", explain_file[ch]);
            securityreport(secu, NULL, NULL, getSession());
        }
        my_unlink(genbuf);
        move(5, 0);
        prints("%s 已删除\n", explain_file[ch]);
        sprintf(buf, "delete %s", explain_file[ch]);
        bbslog("user","%s",buf);
        pressreturn();
        clear();
        return 0;
    }
    modify_user_mode(EDITSFILE);
    aborted = vedit(genbuf, false, NULL, NULL, 0);
    clear();
    if (aborted != -1) {
        prints("%s 更新过", explain_file[ch]);
        sprintf(buf, "edit %s", explain_file[ch]);
        bbslog("user","%s",buf);
        {
            char secu[STRLEN];

            sprintf(secu, "修改系统档案：%s", explain_file[ch]);
            securityreport(secu, NULL, NULL, getSession());
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
       if (!strcmp(e_file[ch],"../" USER_TITLE_FILE)) {
          load_user_title();
       }
#ifdef FLOWBANNER
       if (!strcmp(e_file[ch],"banner")) {
          load_site_banner(0);
       }
#endif
#ifdef FB2KENDLINE
	   if (!strcmp(e_file[ch],"whatdate")){
			setpublicshmreadonly(0);
			publicshm->nextfreshdatetime = time(0);
			setpublicshmreadonly(0);
	   }
#endif
#ifdef ZIXIA
	   if (!strcmp(e_file[ch],"flinks.wForum")){
            my_unlink("etc/flinks.wForum.html");
            prints("\n友情链接更新");
	   }
#endif
    }
    pressreturn();
    return 0;
}

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

int search_ip(void){
/* 从管理菜单调用查询IP程序 */
/* Bigman: 2002.8.20 */
/* 返回：0 正常 */
/*       1 数据文件无法打开 */
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
}

int kick_all_user(void){
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
        kick_user_utmp(uin->uid, uin, 0);
	}

	move(13,0);
	prints("清除完毕\n");
	pressanykey();
    return 1;
}

static int perm_mailback(struct fileheader *fh)
{
	if (!isowner(getCurrentUser(), fh)) {
		return 0;
	}else
		return 1;
}
static int set_mailback(struct fileheader *fh, int i)
{
	if(i==0){
		fh->accessed[1] &= ~FILE_MAILBACK;
	}else{
		fh->accessed[1] |= FILE_MAILBACK;
	}
	return 1;
}
static int get_mailback(struct fileheader *fh)
{
	if(fh->accessed[1] & FILE_MAILBACK)
		return 1;
	else
		return 0;
}

static int perm_innflag(struct fileheader *fh){
	if(!(currboard->flag & BOARD_OUTFLAG)) return 0;
	if(isowner(getCurrentUser(), fh)) return 1;
    return chk_currBM(currboard->BM,getCurrentUser());
}
static int set_innflag(struct fileheader *fh,int arg){
    if(!arg){
        fh->innflag[0]='L';
        fh->innflag[1]='L';
    }
    else{
        fh->innflag[0]='S';
        fh->innflag[1]='S';
    }
    return 1;
}
static int get_innflag(struct fileheader *fh){
    return (fh->innflag[0]==fh->innflag[1]&&fh->innflag[0]=='S')?1:0;
}

static int perm_cancelo(struct fileheader *fh)
{
	if(!(fh->accessed[0] & FILE_IMPORTED)) return 0;
    return chk_currBM(currboard->BM,getCurrentUser());
}
static int set_cancelo(struct fileheader *fh, int i)
{
	if(i==0){
		fh->accessed[0] &= ~FILE_IMPORTED;
	}else{
		fh->accessed[0] |= FILE_IMPORTED;
	}
	return 1;
}
static int get_cancelo(struct fileheader *fh)
{
	if(fh->accessed[0] & FILE_IMPORTED)
		return 1;
	else
		return 0;
}

#define FH_SELECT_NUM 3
static struct _fh_select
{
	char *desc;
	int (*have_perm) (struct fileheader *);
	int (*set) (struct fileheader *, int);
	int (*get) (struct fileheader *);
} fh_select[FH_SELECT_NUM] = 
{
	{"回文转寄到信箱", perm_mailback, set_mailback, get_mailback},
	{"转信发表", perm_innflag, set_innflag, get_innflag},
	{"收精华标记", perm_cancelo, set_cancelo, get_cancelo}
};

int show_fhselect(struct _select_def *conf, int i)
{
	struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    i = i - 1;
    if (i == conf->item_count - 1) {
        prints("%c. 退出 ", 'A' + i);
    } else if (!(arg->basic & (1 << i))) {
    	prints("%c. \033[30;1m%-27s:DENY\033[m", 'A' + i, fh_select[i].desc);
    } else if ((arg->pbits & (1 << i)) != (arg->oldbits & (1 << i))) {
        prints("%c. %-27s \033[31;1m%3s\033[m", 'A' + i, fh_select[i].desc, ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    } else {
        prints("%c. %-27s \x1b[37;0m%3s\x1b[m", 'A' + i, fh_select[i].desc, ((arg->pbits >> i) & 1 ? "ON" : "OFF"));
    }
    return SHOW_CONTINUE;
}

int fhselect_select(struct _select_def *conf)
{
    struct _setperm_select *arg = (struct _setperm_select *) conf->arg;

    if (conf->pos == conf->item_count)
        return SHOW_QUIT;
    if (!(arg->basic & ( 1 << (conf->pos - 1)))){
    	return SHOW_CONTINUE;
    }
    arg->pbits ^= (1 << (conf->pos - 1));
    return SHOW_REFRESHSELECT;
}

#include "read.h"

int fhselect(struct _select_def* conf,struct fileheader *fh,long flag)
{
    int i;
    unsigned int oldlevel=0;
    unsigned int perms=0;
    unsigned int newlevel;
    int oldmode;
    struct write_dir_arg dirarg;
    struct read_arg* arg=(struct read_arg*)conf->arg;
    struct fileheader *originFh;
    
	if (arg->mode!=DIR_MODE_NORMAL) {
        return DONOTHING;
    }
	/*
	if (!HAS_PERM(getCurrentUser(), PERM_SYSOP))
        return DONOTHING;
    */
	oldmode = uinfo.mode;
    modify_user_mode(USERDEF);
    
    for(i=0; i<FH_SELECT_NUM; i++){
    	if(fh_select[i].have_perm==NULL || fh_select[i].have_perm(fh)){
    		perms |= (1<<i);
    		if(fh_select[i].get(fh)){
    			oldlevel |= (1<<i);
    		}
    	}
    }
    
    move(1, 0);
    clrtobot();
    move(2, 0);
	prints("\033[1;31m修改\033[1;32m%s\033[1;31m的文章属性:\033[m \033[1;33m%s\033[m", fh->owner, fh->title);
    newlevel = setperms(oldlevel, perms, "参数", FH_SELECT_NUM, show_fhselect, fhselect_select);
    move(22, 0);
    if ((newlevel & perms) == (oldlevel & perms))
        prints("参数没有修改...\n");
    else {
        for(i=0; i<FH_SELECT_NUM; i++){
        	if((perms & (1<<i)) && ( (oldlevel & (1<<i)) != (newlevel & (1<<i)) ) ){
        		fh_select[i].set(fh, newlevel & (1<<i));
        	}
        }
        
    init_write_dir_arg(&dirarg);
    dirarg.fd=arg->fd;
    dirarg.ent = conf->pos;
    if (prepare_write_dir(&dirarg, fh, arg->mode) == 0){
    	originFh = dirarg.fileptr + (dirarg.ent - 1);
    	memcpy(originFh, fh, sizeof(struct fileheader));

    	if (dirarg.needlock)
        	flock(dirarg.fd, LOCK_UN);

    	free_write_dir_arg(&dirarg);
        prints("新的参数设定完成...\n\n");
    }else{
		free_write_dir_arg(&dirarg);
		prints("系统错误，失败...\n");
	}

    }
    pressreturn();
    modify_user_mode(oldmode);
    return FULLUPDATE;
}

