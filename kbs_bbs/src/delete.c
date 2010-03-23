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

int d_board(void)
{
    char bname[STRLEN+1];
    char buf[STRLEN];
    const struct boardheader *bh;
    int bid;
    struct boardheader oldbh;

    modify_user_mode(ADMIN);
    if (!check_systempasswd()) {
        return -1;
    }
    clear();
    stand_title("删除讨论区");
    make_blist(0, 1);
    move(1, 0);
    namecomplete("请输入讨论区: ", bname);
    if (bname[0] == '\0')
        return 0;

    if ((bid = getbid(bname, &bh)) == 0) {
        move(2, 0);
        prints("不正确的讨论区");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }

    memcpy(&oldbh, bh, sizeof(struct boardheader));
    move(1, 0);
    prints("删除讨论区 '%s'.", bh->filename);
    clrtoeol();
    getdata(2, 0, "(Yes, or No) [N]: ", genbuf, 4, DOECHO, NULL, true);
    if (genbuf[0] != 'Y' && genbuf[0] != 'y') {
        move(2, 0);
        prints("取消删除....\n");
        pressreturn();
        clear();
        return 0;
    }

    getdata(3, 0, "移除精华区 [y/N]: ", genbuf, 4, DOECHO, NULL, true);
    if (genbuf[0] == 'Y' || genbuf[0] == 'y') {
        edit_group(&oldbh, NULL);
    }

    if (delete_board(bid, getSession()) != 0)
        return 0;
#if 0
    sprintf(genbuf, "boards/%s", bname);
    f_rm(genbuf);
    sprintf(genbuf, "vote/%s", bname);
    f_rm(genbuf);
#endif

    /* this should be in delete_board function ? */
    sprintf(buf, "删除讨论区：%s", oldbh.filename);
    securityreport(buf, NULL, NULL, getSession());

    move(4, 0);
    prints("本讨论区已经删除...\n");
    pressreturn();
    clear();
    return 0;
}

#ifndef SECONDSITE
int suicide(void)
{
    char buf[STRLEN];
    FILE *fn;
    time_t now;
    char filename[STRLEN];

    char XPERM[48];
    int oldXPERM;
    int num;

    modify_user_mode(OFFLINE);
    if (HAS_PERM(getCurrentUser(), PERM_SYSOP) || HAS_PERM(getCurrentUser(), PERM_BOARDS) || HAS_PERM(getCurrentUser(), PERM_OBOARDS) || HAS_PERM(getCurrentUser(), PERM_ACCOUNTS)
            || HAS_PERM(getCurrentUser(), PERM_ANNOUNCE)
            || HAS_PERM(getCurrentUser(), PERM_JURY) || HAS_PERM(getCurrentUser(), PERM_SUICIDE) || HAS_PERM(getCurrentUser(), PERM_CHATOP) || (!HAS_PERM(getCurrentUser(), PERM_POST))
            || HAS_PERM(getCurrentUser(), PERM_DENYMAIL)
            || HAS_PERM(getCurrentUser(), PERM_DENYRELAX)) {
        clear();
        move(11, 28);
        prints("\033[1m\033[33m你有重任在身，不能自杀！\033[m");
        pressanykey();
        return -1;
    }

    clear();
    move(1, 0);
    prints("选择自杀将使您的生命力减少到14天，14天后您的帐号自动消失。");
    move(3, 0);
    prints("在这14天内若改变主意的话，则可以通过登录本站一次恢复原生命力");
    move(5, 0);
    prints("自杀用户将丢失所有\033[33m特殊权限\033[m！！！");
    move(7, 0);
    /*
       clear();
       move(1,0);
       prints("选择自杀将使您的帐号自动立即消失!");
       move(3,0);
       prints("您的帐号将马上从系统中删除");
     */

    if (askyn("你确定要自杀吗？", 0) == 1) {
        clear();
        getdata(0, 0, "请输入原密码(输入正确的话会立刻断线): ", buf, 39, NOECHO, NULL, true);   /*Haohmaru,98.10.12,check the passwds */
        if (*buf == '\0' || !checkpasswd2(buf, getCurrentUser())) {
            prints("\n\n很抱歉, 您输入的密码不正确。\n");
            pressanykey();
            return -1;
        }

        oldXPERM = getCurrentUser()->userlevel;
        strcpy(XPERM, XPERMSTR);
        for (num = 0; num < (int) strlen(XPERM); num++)
            if (!(oldXPERM & (1 << num)))
                XPERM[num] = ' ';
        XPERM[num] = '\0';
        getCurrentUser()->userlevel &= 0x3F; /*Haohmaru,99.3.20.自杀者只保留基本权限 */
        getCurrentUser()->userlevel ^= PERM_SUICIDE;

        /*Haohmaru.99.3.20.自杀通知 */
        now = time(0);
        sprintf(filename, "etc/%s.tmp", getCurrentUser()->userid);
        fn = fopen(filename, "w");
        fprintf(fn, "\033[1m%s\033[m 已经在 \033[1m%24.24s\033[m 自杀了，以下是他的资料，请保留...", getCurrentUser()->userid, ctime(&now));
        fprintf(fn, "\n\n以下是自杀者原来的权限\n\033[1m\033[33m%s\n\033[m", XPERM);
        getuinfo(fn, getCurrentUser());
        fprintf(fn, "\n                      \033[1m 系统自动发信系统留\033[m\n");
        fclose(fn);
        sprintf(buf, "%s 的自杀通知", getCurrentUser()->userid);
        post_file(getCurrentUser(), "", filename, "Goodbye", buf, 0, 1,getSession());
        unlink(filename);

        /*kick_user(&uinfo);
           exit(0); */
        /*abort_bbs(0);*/
        kick_user_utmp(getSession() -> currentuid, NULL, 0);
    }
    return 0;
}
#endif /* SECONDSITE */

int giveupnet(void)
{
    /*
        PERM_BASIC      上站
        PERM_POST       发表
        PERM_CHAT       聊天
        PERM_PAGE       呼叫
        PERM_DENYMAIL   发信
        PERM_DENYRELAX  娱乐
    */
    static const char *desc[GIVEUPINFO_PERM_COUNT]={
        "登录上站权限","发表文章权限","聊天广场权限","站内信息权限", "站内信件权限","休闲娱乐权限"
    };
    FILE *fn;
    char buf[STRLEN], genbuf[PATHLEN];
    char ans[3], day[10];
    int i, j, k, iDays;
    int s[GIVEUPINFO_PERM_COUNT];

    modify_user_mode(GIVEUPNET);
    if (!HAS_PERM(getCurrentUser(), PERM_LOGINOK)) {
        clear();
        move(11, 28);
        prints("\033[1m\033[33m你有还没有注册通过，不能戒网！\033[m");
        pressanykey();
        return -1;
    }

    if (HAS_PERM(getCurrentUser(), PERM_SYSOP) || HAS_PERM(getCurrentUser(), PERM_BOARDS) || HAS_PERM(getCurrentUser(), PERM_OBOARDS) || HAS_PERM(getCurrentUser(), PERM_ACCOUNTS)
            || HAS_PERM(getCurrentUser(), PERM_ANNOUNCE)
            || HAS_PERM(getCurrentUser(), PERM_JURY) || HAS_PERM(getCurrentUser(), PERM_SUICIDE) || HAS_PERM(getCurrentUser(), PERM_CHATOP)) {
        clear();
        move(11, 28);
        prints("\033[1m\033[33m你有重任在身，不能戒网！\033[m");
        pressanykey();
        return -1;
    }

    if (get_giveupinfo(getCurrentUser(),s)) {
        for (i=0; i<GIVEUPINFO_PERM_COUNT; i++)
            if (s[i])
                break;
        if (i<GIVEUPINFO_PERM_COUNT) {
            clear();
            move(0,0);
            prints("\033[1;33m[当前戒网及有期封禁状态]\033[m\n\n");
            for (i=0; i<GIVEUPINFO_PERM_COUNT; i++)
                if (!s[i])
                    continue;
                else {
                    sprintf(genbuf,"%-16.16s%-8.8s目前剩余 %4d 天\n",desc[i],(s[i]>0?"戒网":"封禁"),
                            (int)((s[i]>0?s[i]:(-s[i]))-(time(NULL)/86400)));
                    prints("%s",genbuf);
                }
            pressanykey();
        }
    }

    clear();
    move(1, 0);
    prints("请选择戒网种类:");
    move(3, 0);
    prints("(0) - 结束");
    for (i=0; i<6; i++) {
        move(i + 4, 0);
        prints("(%d) - %s", i + 1, desc[i]);
    }

    getdata(12, 0, "请选择 [0]", ans, 2, DOECHO, NULL, true);
    if (ans[0] < '1' || ans[0] > '6') {
        return -1;
    }
    k = 1;
    switch (ans[0]) {
        case '1':
            k = k && (getCurrentUser()->userlevel & PERM_BASIC);
            break;
        case '2':
            k = k && (getCurrentUser()->userlevel & PERM_POST);
            break;
        case '3':
            k = k && (getCurrentUser()->userlevel & PERM_CHAT);
            break;
        case '4':
            k = k && (getCurrentUser()->userlevel & PERM_PAGE);
            break;
        case '5':
            k = k && !(getCurrentUser()->userlevel & PERM_DENYMAIL);
            break;
        case '6':
            k = k && !(getCurrentUser()->userlevel & PERM_DENYRELAX);
            break;
    }

    if (!k) {
        prints("\n\n你已经没有了该权限");
        pressanykey();
        return -1;
    }

    getdata(11, 0, "请输入戒网天数 [0]", day, 4, DOECHO, NULL, true);
    i = 0;
    while (day[i]) {
        if (!isdigit(day[i]))
            return -1;
        i++;
    }
    j = atoi(day);
    if (j <= 0)
        return -1;

    if (compute_user_value(getCurrentUser()) <= j) {
        prints("\n\n对不起，天数不可以大于生命力...");
        pressanykey();
        return -1;
    }
    iDays = j;
    j = time(0) / 3600 / 24 + j;

    move(13, 0);

    if (askyn("你确定要戒网吗\x1b[1;31m(注意:戒网之后不能提前解开!\x1b[m)？", 0) == 1) {
        getdata(15, 0, "请输入密码: ", buf, 39, NOECHO, NULL, true);
        if (*buf == '\0' || !checkpasswd2(buf, getCurrentUser())) {
            prints("\n\n很抱歉, 您输入的密码不正确。\n");
            pressanykey();
            return -1;
        }
        switch (ans[0]) {
            case '1':
                getCurrentUser()->userlevel &= ~PERM_BASIC;
                break;
            case '2':
                getCurrentUser()->userlevel &= ~PERM_POST;
                break;
            case '3':
                getCurrentUser()->userlevel &= ~PERM_CHAT;
                break;
            case '4':
                getCurrentUser()->userlevel &= ~PERM_PAGE;
                break;
            case '5':
                getCurrentUser()->userlevel |= PERM_DENYMAIL;
                break;
            case '6':
                getCurrentUser()->userlevel |= PERM_DENYRELAX;
                break;
        }

        s[ans[0]-49]=j;
        save_giveupinfo(getCurrentUser(),s);

        gettmpfilename(genbuf, "giveup");
        if ((fn = fopen(genbuf, "w")) != NULL) {
            fprintf(fn, "\033[1m%s\033[m 戒 %s %d 天", getCurrentUser()->userid, desc[ans[0] - '1'], iDays);
            getuinfo(fn, getCurrentUser());
            fclose(fn);
            sprintf(buf, "%s 的戒网通知", getCurrentUser()->userid);
            post_file(getCurrentUser(), "", genbuf, "GiveupNotice", buf, 0,  2, getSession());
            unlink(genbuf);
        }

        prints("\n\n你已经开始戒网了");
        pressanykey();
        if (ans[0] == '1') {
            //abort_bbs(0);
            kick_user_utmp(getSession() -> currentuid, NULL, 0);
        }
    }
    return 0;
}

#if 0
static void mail_info()
{
    FILE *fn;
    time_t now;
    char filename[STRLEN];

    now = time(0);
    sprintf(filename, "etc/%s.tmp", getCurrentUser()->userid);
    fn = fopen(filename, "w");
    fprintf(fn, "\033[1m%s\033[m 已经在 \033[1m%24.24s\033[m 自杀了，以下是他(她)的资料，请保留...", getCurrentUser()->userid, ctime(&now));
    getuinfo(fn, getCurrentUser());
    fprintf(fn, "\n                      \033[1m 系统自动发信系统留\033[m\n");
    fclose(fn);
    mail_file(getCurrentUser()->userid, filename, "acmanager", "自杀通知....", BBSPOST_MOVE, NULL);
}
void offline()
{
    modify_user_mode(OFFLINE);

    if (HAS_PERM(getCurrentUser(), PERM_SYSOP))
        return;
    clear();
    move(1, 0);
    prints("\033[32m好难过喔.....\033[m");
    move(3, 0);
    if (askyn("你确定要离开这个大家庭", 0) == 1) {
        clear();
        if (d_user(getCurrentUser()->userid) == 1) {
            mail_info();
            kick_user(&uinfo);
            exit(0);
        }
    }
}
#endif

int d_user(cid)
char cid[IDLEN];
{
    int uid, fd;
    char tmpbuf[30];
    char userid[IDLEN + 2];
    struct userec *lookupuser;
    /* 增加显示用户信息 Bigman:2003.5.11*/
    struct userec uinfo1;
    /*struct userdata ud;*/

    if (uinfo.mode != OFFLINE) {
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
        stand_title("删除使用者帐号");
        move(1, 0);
        usercomplete("请输入欲删除的使用者代号: ", userid);
        if (userid[0] == '\0') {
            clear();
            return 0;
        }
    } else
        strcpy(userid, cid);
    if (!(uid = getuser(userid, &lookupuser))) {
        move(3, 0);
        prints(MSG_ERR_USERID);
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    /* 增加显示用户信息 Bigman:2003.5.11*/
    uinfo1 = *lookupuser;
    clrtobot();

    disply_userinfo(&uinfo1, 1);

    /*    if (!isalpha(lookupuser->userid[0])) return 0; */
    /* rrr - don't know how... */
    move(22, 0);
    if (uinfo.mode != OFFLINE)
        prints("删除使用者 '%s'.", userid);
    else
        prints(" %s 将离开这里", cid);
    /*    clrtoeol(); */

    getdata(24, 0, "(Yes, or No) [No](注意要输入全单词和大小写): ", genbuf, 5, DOECHO, NULL, true);
    if (strcmp(genbuf,"Yes")) { /* if not yes quit */
        move(24, 0);
        if (uinfo.mode != OFFLINE)
            prints("取消删除使用者...\n");
        else
            prints("你终于回心转意了，好高兴喔...");
        pressreturn();
        clear();
        return 0;
    }
    if (uinfo.mode != OFFLINE) {
        char secu[STRLEN];

        sprintf(secu, "删除使用者：%s", lookupuser->userid);
        securityreport(secu, lookupuser, NULL, getSession());
    }
    newbbslog(BBSLOG_USER,"%s deleted user %s", getCurrentUser()->userid, lookupuser->userid);
    /*Haohmaru.99.12.23.被删ID一个月内不得注册 */
    if ((fd = open(".badname", O_WRONLY | O_CREAT, 0644)) != -1) {
        char buf[STRLEN];
        char thtime[40];
        time_t dtime;

        dtime = time(0);
        sprintf(thtime, "%lu", dtime);
        writew_lock(fd, 0, SEEK_SET, 0);
        lseek(fd, 0, SEEK_END);
        sprintf(buf, "%-12.12s %-66.66s\n", lookupuser->userid, thtime);
        write(fd, buf, strlen(buf));
        un_lock(fd, 0, SEEK_SET, 0);
        close(fd);
    } else {
        printf("错误，请报告SYSOP");
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
    kick_user_utmp(uid, NULL, SIGKILL);
    setuserid(uid, "");
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
    prints("%s 已经已经和本家庭失去联络....\n", userid);
    pressreturn();

    clear();
    return 1;
}

int kick_user(int uid, char *userid, struct user_info *userinfo)
{
    newbbslog(BBSLOG_USER, "kicked %s", userid);
    if (strcmp(getCurrentUser()->userid, userid)) {
        char buf[STRLEN];
        sprintf(buf, "%s 踢出使用者 %s", getCurrentUser()->userid, userid);
        securityreport(buf, NULL, NULL, getSession());
    }
    return kick_user_utmp(uid, userinfo, 0);
}

int kick_user_menu(void)
{
    char userid[STRLEN], ans[10];
    int uid;
    struct userec *u;

    modify_user_mode(ADMIN);
    stand_title("踢用户");
    move(1, 0);
    usercomplete("输入要踢出的用户名: ", userid);
    if (*userid == '\0') {
        clear();
        return 0;
    }
    if (!(uid = getuser(userid, &u))) {       /* change getuser -> searchuser, by dong, 1999.10.26 */
        move(3, 0);
        prints("用户名错误");
        clrtoeol();
        pressreturn();
        clear();
        return 0;
    }
    strcpy(userid, u->userid);
    move(1, 0);
    prints("踢出用户 '%s'.", userid);
    clrtoeol();
    getdata(2, 0, "(Yes, or No) [N]: ", ans, 4, DOECHO, NULL, true);
    if (ans[0] != 'Y' && ans[0] != 'y') {     /* if not yes quit */
        move(2, 0);
        prints("取消\n");
        pressreturn();
        clear();
        return 0;
    }
    kick_user(uid, userid, NULL);
    move(4, 0);
    prints("用户已被踢\n");
    pressreturn();
    clear();
    return 1;
}

