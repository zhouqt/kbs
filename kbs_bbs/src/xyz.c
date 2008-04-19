/*
	This file has been checked global varible.
	一些杂项的功能
*/

#include "bbs.h"

#ifdef SECONDSITE
void myexec_cmd(int umode,const char *cmdfile, const char *param);
#endif
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

#ifdef SECONDSITE
int x_mj(void){
    myexec_cmd(BBSNET,"bin/qkmj",NULL);
    clear();
    pressreturn();
    clear();
    return 0;
}
#endif /* SECONDSITE */

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
    prints("更改" NAME_USER_SHORT "的权限(\x1b[m注意：如果是封禁解封，请使用封禁选单！\x1b[m)\n");
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

#ifdef HAVE_ACTIVATION
int x_manageactivation(void){
    int id;
    struct userec *lookupuser;
    struct activation_info ai;
    char title[STRLEN];

    if (!HAS_PERM(getCurrentUser(), PERM_SYSOP)) {
        move(3, 0);
        clrtobot();
        prints("抱歉, 只有SYSOP权限的管理员才能管理用户激活");
        pressreturn();
        return 0;
    }

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return 0;
    }
    clear();
    move(0, 0);
    prints(NAME_USER_SHORT "激活控制\n");
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
    getactivation(&ai, lookupuser);
    move(4, 0);
    if (lookupuser->flags & ACTIVATED_FLAG) {
        prints("用户已经激活. 激活 email: %s\n\n", ai.reg_email);
        if (askyn("按 Y 取消用户激活", false)) {
            ai.activated = 0;
            setactivation(&ai, lookupuser);
            lookupuser->flags &= ~ACTIVATED_FLAG;
            sprintf(title, "%s 取消 %s 用户激活", getCurrentUser()->userid, lookupuser->userid);
        } else {
            clear();
            return 0;
        }
    } else {
        prints("\033[1;31m用户还没有激活\033[0m\n\n");
        if (askyn("按 Y 手动激活用户", false)) {
            ai.activated = 1;
            setactivation(&ai, lookupuser);
            lookupuser->flags |= ACTIVATED_FLAG;
            sprintf(title, "%s 手动激活 %s 用户", getCurrentUser()->userid, lookupuser->userid);
        } else {
            clear();
            return 0;
        }
    }
    {
    	FILE *fout;
    	char buf[STRLEN];

    	sprintf(buf, "tmp/mactivation.%s",getCurrentUser()->userid);
        if ((fout = fopen(buf, "w")) != NULL)
    	{
            fprintf(fout, "%s\n", title);
            fprintf(fout, "以下是个人资料");
            getuinfo(fout, lookupuser);
            fclose(fout);
    		post_file(getCurrentUser(), "", buf, "Activation", title, 0, 2, getSession());
    		unlink(buf);
    	}
    }
    clear();
    return 0;
}
#endif /* HAVE_ACTIVATION */

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
    make_blist(0, 1);
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
	prints("同步他人收藏夹,好友名单,未读标记和自定义功能键到本ID.\n会导致本ID原来的收藏夹,好友名单,未读标记和自定义功能键丢失，慎用\n");
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
	prints("\033[31m本次操作会覆盖本id原收藏夹、好友名单、未读标记和自定义功能键，无法恢复\033[m");
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

    getdata(15, 0, "同步自定义功能键? [Y]: ", passwd, 2, DOECHO, NULL, true);
    if (toupper(passwd[0]) != 'N')
    {
        sethomefile(dpath, destuser -> userid, "definekey");
        sethomefile(mypath, getCurrentUser() -> userid, "definekey");
        f_cp(dpath, mypath, 0);
        load_key(NULL);
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

#ifdef SECONDSITE
/*得到主站的收藏夹和未度标记*/
int get_mainsite(void){
	char passwd[PASSLEN+1];
	char dpath[PATHLEN];
	char mypath[PATHLEN];
	char cmdtmp[256];
	int count=0;
	clear();
	move(1,0);
	prints("同步主站资料到本站本ID.\n会导致本站本ID被同步的原始资料丢失，慎用\n");
	prints("\033[32m为了保证数据同步性，操作前请先退出本id其他登录\033[m\n");
	prints("\033[31m本次操作会覆盖本id原好友名单/自定义键，无法恢复\033[m");
	getdata(10,0,"确信要进行此操作吗? [y/N] ", passwd, 2, DOECHO, NULL, true);
	if (passwd[0] != 'y' && passwd[0] != 'Y'){
		clear();
		return 0;
	}
    getdata(14,0,"同步好友名单? [Y]: ",passwd,2,DOECHO,NULL,true);
    if(toupper(passwd[0])!='N'){
        sprintf(dpath, "tmp/second.%s.friends", getCurrentUser()->userid);
		unlink(dpath);
		sprintf(cmdtmp, "/usr/bin/wget -O %s http://10.0.4.238:5257/home/%c/%s/friends", dpath, toupper(getCurrentUser()->userid[0]), getCurrentUser()->userid);
		system(cmdtmp);
		if(!dashf(dpath)){
			move(14,30);
			prints("失败");
			goto outfriend;
		}
        sethomefile(mypath,getCurrentUser()->userid,"friends");
        f_cp(dpath,mypath,0);
		unlink(dpath);
        getfriendstr(getCurrentUser(),get_utmpent(getSession()->utmpent),getSession());
        count++;
    }
outfriend:
    getdata(16,0,"同步自定义按键? [Y]: ",passwd,2,DOECHO,NULL,true);
    if(toupper(passwd[0])!='N'){
        sprintf(dpath, "tmp/second.%s.definekey", getCurrentUser()->userid);
		unlink(dpath);
		sprintf(cmdtmp, "/usr/bin/wget -O %s http://10.0.4.238:5257/home/%c/%s/definekey", dpath, toupper(getCurrentUser()->userid[0]), getCurrentUser()->userid);
		system(cmdtmp);
		if(!dashf(dpath)){
			move(16,30);
			prints("失败");
			goto outkey;
		}
        sethomefile(mypath,getCurrentUser()->userid,"definekey");
        f_cp(dpath,mypath,0);
		unlink(dpath);
		load_key(NULL);
        count++;
    }
outkey:
    if(count){
	    move(20,0);
	    prints("操作成功, 您无需重新登录即可使用新数据!");
    }
    pressanykey();
	clear();
	return 0;
}
#endif /* SECONDSITE */


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

#ifdef SECONDSITE
static time_t old=0;

static void
datapipefd(int fds, int fdn)
{
	fd_set rs;
	int retv, max;
	char buf[1024];
	time_t now;

	max = 1 + ((fdn > fds) ? fdn : fds);
	FD_ZERO(&rs);
	while (1) {
		FD_SET(fds, &rs);
		FD_SET(fdn, &rs);
		retv = select(max, &rs, NULL, NULL, NULL);
		if (retv < 0) {
			if (errno != EINTR)
				break;
			continue;
		}
		if (FD_ISSET(fds, &rs)) {
#ifdef SSHBBS
			retv = ssh_read(fds, buf, sizeof (buf));
#else
			retv = read(fds, buf, sizeof (buf));
#endif
			if (retv > 0) {

        	now = time(0);
            uinfo.freshtime = now;
            if (now - old > 60) {
               UPDATE_UTMP(freshtime, uinfo);
               old = now;
            }

				write(fdn, buf, retv);
			} else if (retv == 0 || (retv < 0 && errno != EINTR))
				break;
			FD_CLR(fds, &rs);
		}
		if (FD_ISSET(fdn, &rs)) {
			retv = read(fdn, buf, sizeof (buf));
			if (retv > 0) {
#ifdef SSHBBS
				ssh_write(fds, buf, retv);
#else
				write(fds, buf, retv);
#endif
			} else if (retv == 0 || (retv < 0 && errno != EINTR))
				break;
			FD_CLR(fdn, &rs);
        }
    }
}

void myexec_cmd(int umode,const char *cmdfile,const char *param){
	char param1[256];
	int save_pager;
	pid_t childpid;
	int p[2];
	param1[0] = 0;
	if (param != NULL) {
		char *avoid = "&;!`'\"|?~<>^()[]{}$\n\r\\", *ptr;
		int n = strlen(avoid);
		strncpy(param1, param, sizeof (param1));
		param1[255]='\0';
		while (n > 0) {
			n--;
			ptr = strchr(param1, avoid[n]);
			if (ptr != NULL)
				*ptr = 0;
		}
	}

	if (!dashf(cmdfile)) {
		move(2, 0);
		prints("no %s\n", cmdfile);
		pressreturn();
		return;
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, p) < 0)
		return;

	modify_user_mode(umode);
	clear();
	signal(SIGALRM, SIG_IGN);
	signal(SIGCHLD, SIG_DFL);
	childpid = fork();
	if (childpid == 0) {
		close(p[0]);
		if (p[1] != 0)
			dup2(p[1], 0);
		dup2(0, 1);
		dup2(0, 2);
		if (param1[0]) {
			execl(cmdfile, cmdfile, param1, getCurrentUser()->userid, NULL);
		} else {
			execl(cmdfile, cmdfile, getCurrentUser()->userid, NULL);
		}
		exit(0);
	} else if (childpid > 0) {
		close(p[1]);
		datapipefd(0, p[0]);
		close(p[0]);
		while (wait(NULL) != childpid)
			sleep(1);
	} else {
		close(p[0]);
		close(p[1]);
	}
	signal(SIGCHLD, SIG_IGN);

	return;
}
#endif /* SECONDSITE */

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
			get_my_webdomain(-1);
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
			get_my_webdomain(-1);
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

/* etnlegend, 2006.12.13, 查询 IP 地址的地理位置更新, 直接使用 QQWry.Dat 信息库... */
static inline int qqwry_read_int(int fd,uint32_t *data,size_t count,off_t offset){
    uint8_t buf[4]={0,0,0,0};
    if(pread(fd,buf,count,offset)!=count)
        return -1;
    *data=(buf[0]|(buf[1]<<8)|(buf[2]<<16)|(buf[3]<<24));
    return 0;
}

static inline int qqwry_read_str(int fd,char *data,size_t count,off_t offset){
    if(!(pread(fd,data,count,offset)>0))
        return -1;
    data[count-1]=0;
    return 0;
}

int qqwry_search(char *description,const char *ip_address){
#define QQWRY_DAT           "etc/QQWry.Dat"
#define QQWRY_QUIT(r)       do{close(fd);return (r);}while(0)
#define QQWRY_READ_INT(s,o) do{if(qqwry_read_int(fd,&data,(s),(o))==-1)QQWRY_QUIT(-1);}while(0)
#define QQWRY_READ_STR(s,o) do{if(qqwry_read_str(fd,buf,(s),(o))==-1)QQWRY_QUIT(-1);size=(strlen(buf)+1);}while(0)
    struct in_addr sin;
    char buf[STRLEN],*separator;
    int fd,match,bound_b,bound_e,current;
    uint32_t data,ip,ip_b,ip_e;
    off_t index_b,index_e,index_offset,record_offset,country_offset,location_offset;
    size_t size;
    if((fd=open(QQWRY_DAT,O_RDONLY,0644))==-1)
        QQWRY_QUIT(-1);
    if(!inet_aton(ip_address,&sin))
        QQWRY_QUIT(-1);
    ip=ntohl(sin.s_addr);
    QQWRY_READ_INT(4,0);
    index_b=(off_t)data;
    QQWRY_READ_INT(4,4);
    index_e=(off_t)data;
    match=0;
    bound_b=0;
    bound_e=(index_e-index_b)/7;
    while(bound_b<bound_e){
        current=(bound_b+bound_e)/2;
        index_offset=(index_b+current*7);
        QQWRY_READ_INT(4,index_offset);
        ip_b=(uint32_t)data;
        QQWRY_READ_INT(3,index_offset+4);
        record_offset=(off_t)data;
        QQWRY_READ_INT(4,record_offset);
        ip_e=(uint32_t)data;
        if(ip<ip_b)
            bound_e=current;
        else if(ip>ip_e)
            bound_b=current+1;
        else{
            match=1;
            break;
        }
    }
    if(!match)
        QQWRY_QUIT(0);
    QQWRY_READ_INT(1,record_offset+4);
    if(data==0x01){
        QQWRY_READ_INT(3,record_offset+5);
        country_offset=(off_t)data;
        QQWRY_READ_INT(1,country_offset);
        if(data==0x02){
            location_offset=country_offset+4;
            QQWRY_READ_INT(3,country_offset+1);
            country_offset=(off_t)data;
            QQWRY_READ_STR(STRLEN,country_offset);
        }
        else{
            QQWRY_READ_STR(STRLEN,country_offset);
            location_offset=country_offset+size;
        }
    }
    else if(data==0x02){
        location_offset=record_offset+8;
        QQWRY_READ_INT(3,record_offset+5);
        country_offset=(off_t)data;
        QQWRY_READ_STR(STRLEN,country_offset);
    }
    else{
        QQWRY_READ_STR(STRLEN,record_offset+4);
        location_offset=(record_offset+4)+size;
    }
    strcpy(description,buf);
    if(size>(STRLEN-4))
        QQWRY_QUIT(1);
    separator=&description[size];
    separator[-1]=' ';
    QQWRY_READ_INT(1,location_offset);
    if(data==0x01||data==0x02){
        QQWRY_READ_INT(3,location_offset+1);
        location_offset=(off_t)data;
        if(!location_offset)
            buf[0]=0;
        else
            QQWRY_READ_STR(STRLEN-size,location_offset);
    }
    else
        QQWRY_READ_STR(STRLEN-size,location_offset);
    strcpy(separator,buf);
    QQWRY_QUIT(1);
#undef QQWRY_DAT
#undef QQWRY_QUIT
#undef QQWRY_READ_INT
#undef QQWRY_READ_STR
}

int search_ip(void){
	char ip_address[16],description[STRLEN];
    int ret;
	clear();
    move(0,0);
    prints("\033[1;32m%s\033[m","[查询 IP 地址位置]");
    do{
        move(1,0);
        clrtobot();
        getdata(2,0,"请输入需要查询的 IP 地址: ",ip_address,16,DOECHO,NULL,true);
        if(!ip_address[0])
            break;
        ret=qqwry_search(description,ip_address);
        move(4,0);
        if(ret==-1)
            prints("\033[1;31m%s\033[0;33m<Enter>\033[m","查询过程中发生错误...");
        else if(!ret)
            prints("\033[1;36m%s\033[0;33m<Enter>\033[m","当前数据库中没有与该 IP 地址相关的位置信息...");
        else
            prints("\033[1;33m<%s> [%s]\033[m\n\n\033[1;32m查询成功!\033[0;33m<Enter>\033[m",ip_address,description);
        WAIT_RETURN;
    }
    while(1);
    return 0;
}
/* END - etnlegend, 2006.12.13, 查询 IP 地址的地理位置更新, 直接使用 QQWry.Dat 信息库... */

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

static int perm_tex(struct fileheader *fh)
{
	if(isowner(getCurrentUser(), fh)) return 1;
    return chk_currBM(currboard->BM,getCurrentUser());
}
static int set_tex(struct fileheader *fh, int i)
{
	if(i==0){
		fh->accessed[1] &= ~FILE_TEX;
	}else{
		fh->accessed[1] |= FILE_TEX;
	}
	return 1;
}
static int get_tex(struct fileheader *fh)
{
	if(fh->accessed[1] & FILE_TEX)
		return 1;
	else
		return 0;
}

#define FH_SELECT_NUM 4
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
	{"收精华标记", perm_cancelo, set_cancelo, get_cancelo},
    {"TEX标记", perm_tex, set_tex, get_tex}
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
                if(i == 3) {
                    char bfile[PATHLEN];
                    setbfile(bfile, currboard->filename, fh->filename);
                    if(dashf(bfile))
                        f_touch(bfile);
                }
        	}
        }
        
    init_write_dir_arg(&dirarg);
    dirarg.fd=arg->fd;
    dirarg.ent = conf->pos;
    if (prepare_write_dir(&dirarg, fh, arg->mode) == 0){
    	originFh = dirarg.fileptr + (dirarg.ent - 1);
    	memcpy(originFh, fh, sizeof(struct fileheader));

    	if (dirarg.needlock)
            un_lock(dirarg.fd, 0, SEEK_SET, 0);

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

/*
 *  Term下管理推荐版面, jiangjun, 2007.11.02
 */

#define MAXRCMDBRD 15
/* 注意, 这个 show_rcmd_brd() 没有翻页功能, 这数别给太大了 ... fancy Nov 5 2007 */
#define RCMDBOARD  "xml/rcmdbrd.xml"

void show_rcmd_brd(struct boardheader bh[], int total)
{
    int i=0;
    char buf[STRLEN];

    move(3, 0);
    clrtobot();
    prints("目前已添加 \033[1;32m%d\033[m 个推荐版面:", total);
    while(i<total){
        if(bh[i].filename[0]==0)
            break;
        move(i+4, 0);
        sprintf(buf, "\033[1;3%dm%-20s\t%-20s\033[m", i%2?2:3, bh[i].filename, bh[i].title+13);
        prints(buf);
        i++;
    }
}

int set_rcmdbrd()
{
    char buf[256],buf1[256], *ptr, ans[2];
    FILE *fp;
    int i, total, bid;
    struct boardheader bh[MAXRCMDBRD], *newbh;

    total = 0;

    if(!HAS_PERM(getCurrentUser(), PERM_SYSOP))
        return DONOTHING;

    newbh = (struct boardheader *) malloc(sizeof(struct boardheader));
    memset(&bh, 0, MAXRCMDBRD * sizeof(struct boardheader));
/* get old recommend board data */
    if((fp=fopen(RCMDBOARD, "r"))!=NULL){
        while(fgets(buf, 256, fp)!=NULL){
            if((ptr=strstr(buf, "EnglishName"))!=NULL){
                strcpy(buf1, ptr+strlen("EnglishName>"));
                if((ptr=strchr(buf1, '<'))!=NULL)
                    *ptr = '\0';
                if(getboardnum(buf1, &(bh[total]))==0)
                    continue;
                total++;
            }
            if(total>=MAXRCMDBRD)
                break;
        }
        fclose(fp);
    }

    while(1){
        clear();
        move(0, 0);
        prints("推荐讨论区选单");
/*Here:  // board exists while adding or deleteing, should goto here*/
        show_rcmd_brd(bh, total);
        getdata(1, 0, "(A)添加讨论区 (D)删除讨论区 (E)退出 [E]: ", ans, 2, DOECHO, NULL, true);
        if(ans[0]=='a' || ans[0]=='A'){
            if(total>=MAXRCMDBRD){
                move(2, 0);
                prints("已经达到最大推荐讨论区数目!");
                WAIT_RETURN;
                continue;
            }
            make_blist(0, 1);
            move(1, 0);
            namecomplete("请输入欲添加的讨论区英文名: ", buf);
            if(!buf[0])
                continue;
            bid=getboardnum(buf, newbh);
            if(bid==0){
                move(2, 0);
                prints("错误讨论区!");
                WAIT_RETURN;
                continue;
            }
            if(!public_board(newbh)){
                move(2, 0);
                prints("请勿推荐内部讨论区!");
                WAIT_RETURN;
                continue;
            }
            for(i=0;i<total;i++){
                if(!strcasecmp(buf, bh[i].filename)){
                    move(2, 0);
                    prints("已经添加该讨论区!");
                    WAIT_RETURN;
                    /*goto Here;*/
                    break;
                }
            }
            if (i < total) continue;
            memcpy(&(bh[total]), newbh, sizeof(struct boardheader));
            total ++;
        }else if(ans[0]=='d' || ans[0]=='D'){
            getdata(1, 0, "请输入欲删除的讨论区英文名: ", buf, STRLEN, DOECHO, NULL, true);
            if(buf[0]){
                for(i=0;i<total;i++){
                    if(!strcasecmp(buf, bh[i].filename)){
                        /*bh[i].filename[0]='\0';
                        memmove(&(bh[i]), &(bh[i+1]), (total - i - 1) * sizeof(struct boardheader));
                        memset(&(bh[total-1]), 0, sizeof(struct boardheader));
                        total --;
                        move(2, 0);
                        prints("已经删除讨论区");
                        WAIT_RETURN;
                        goto Here;*/
                        break;
                    }
                }
                if (i < total)
                {
                    bh[i].filename[0]='\0';
                    memmove(&(bh[i]), &bh[i+1], (total - i - 1) * sizeof(struct boardheader));
                    memset(&(bh[total-1]), 0, sizeof(struct boardheader));
                    total --;
                    move(2, 0);
                    prints("已经删除讨论区");
                    WAIT_RETURN;
                    continue;
                }
                move(2, 0);
                prints("讨论区不在列表中");
                WAIT_RETURN;
            }
        }else
            break;
    }

    free(newbh);
    if((fp=fopen(RCMDBOARD, "w"))==NULL){
        move(2, 0);
        prints("系统错误!");
        WAIT_RETURN;
        return FULLUPDATE;
    }
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"GBK\"?>\n"
                "<RecommendBoards>\n");
    for(i=0;i<total;i++){
        fprintf(fp, "<Board><EnglishName>%s</EnglishName></Board>\n", bh[i].filename);
    }
    fprintf(fp, "</RecommendBoards>\n");
    fclose(fp);
    return FULLUPDATE;
}
