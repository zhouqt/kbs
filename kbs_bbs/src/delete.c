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
    fprintf(fn, "\033[1m%s\033[m 已经在 \033[1m%24.24s\033[m 自杀了，以下是他(她)的资料，请保留...", currentuser->userid, ctime(&now));
    getuinfo(fn, currentuser);
    fprintf(fn, "\n                      \033[1m 系统自动发信系统留\033[m\n");
    fclose(fn);
    mail_file(currentuser->userid, filename, "acmanager", "自杀通知....", BBSPOST_MOVE, NULL);
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
    stand_title("删除讨论区");
    make_blist();
    move(1, 0);
    namecomplete("请输入讨论区: ", genbuf);
    if (genbuf[0] == '\0')
        return 0;
    strcpy(bname, genbuf);
    if (delete_board(bname, title) != 0)
        return 0;
    if (seek_in_file("0Announce/.Search", bname)) {
#ifdef BBSMAIN
        getdata(3, 0, "移除精华区 (Yes, or No) [Y]: ", genbuf, 4, DOECHO, NULL, true);
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
    prints("本讨论区已经删除...\n");
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
        prints("\033[1m\033[33m你有重任在身，不能自杀！\033[m");
        pressanykey();
        return;
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
        if (*buf == '\0' || !checkpasswd2(buf, currentuser)) {
            prints("\n\n很抱歉, 您输入的密码不正确。\n");
            pressanykey();
            return;
        }

        oldXPERM = currentuser->userlevel;
        strcpy(XPERM, XPERMSTR);
        for (num = 0; num < (int) strlen(XPERM); num++)
            if (!(oldXPERM & (1 << num)))
                XPERM[num] = ' ';
        XPERM[num] = '\0';
        currentuser->userlevel &= 0x3F; /*Haohmaru,99.3.20.自杀者只保留基本权限 */
        currentuser->userlevel ^= PERM_SUICIDE;

        /*Haohmaru.99.3.20.自杀通知 */
        now = time(0);
        sprintf(filename, "etc/%s.tmp", currentuser->userid);
        fn = fopen(filename, "w");
        fprintf(fn, "\033[1m%s\033[m 已经在 \033[1m%24.24s\033[m 自杀了，以下是他的资料，请保留...", currentuser->userid, ctime(&now));
        fprintf(fn, "\n\n以下是自杀者原来的权限\n\033[1m\033[33m%s\n\033[m", XPERM);
        getuinfo(fn, currentuser);
        fprintf(fn, "\n                      \033[1m 系统自动发信系统留\033[m\n");
        fclose(fn);
        sprintf(buf, "%s 的自杀通知", currentuser->userid);
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
        prints("\033[1m\033[33m你有重任在身，不能自杀！\033[m");
        pressanykey();
        return;
    }

    clear();
    move(1, 0);
    prints("一旦自杀，就无法挽回");
    move(3, 0);
    prints("真的想要一死了之吗? ");
    move(5, 0);

    if (askyn("你确定要自杀吗？", 0) == 1) {
        char buf2[STRLEN], tmpbuf[PATHLEN], genbuf[PATHLEN];
	 int id;
        clear();
	 getdata(0, 0, "请输入一句简短的自杀留言: ", buf2, 75, DOECHO, NULL, true);
        getdata(0, 0, "请输入原密码(输入正确的话会立刻断线并且无法挽回): ", buf, 39, NOECHO, NULL, true);   /*Haohmaru,98.10.12,check the passwds */
        if (*buf == '\0' || !checkpasswd2(buf, currentuser)) {
            prints("\n\n很抱歉, 您输入的密码不正确。\n");
            pressanykey();
            return;
        }

        now = time(0);
        sprintf(filename, "etc/%s.tmp", currentuser->userid);
        fn = fopen(filename, "w");
        fprintf(fn, "大家好,\n\n我是 %s (%s)。 我已经离开这里了。", currentuser->userid, currentuser->username);
        fprintf(fn, "\n\n我不会更不可能忘记自 %s", ctime(&(currentuser->firstlogin)));
        fprintf(fn, "以来我在本站 %d 次 login 中总共 %d 分钟逗留期间的点点滴滴。", currentuser->numlogins, currentuser->stay/60);
        fprintf(fn, "\n请我的好友把 %s 从你们的好友名单中拿掉吧。", currentuser->userid);
        fprintf(fn, "\n\n或许有朝一日我会回来的。 珍重!! 再见!!");
        fprintf(fn, "\n\n自杀者的简短留言: %s", buf2);
        fclose(fn);
        sprintf(buf, "%s 的自杀通知", currentuser->userid);
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
PERM_BASIC   上站
PERM_POST    发表
PERM_CHAT    聊天
PERM_PAGE    呼叫
PERM_DENYMAIL发信
PERM_DENYRELAX娱乐
*/
    char buf[STRLEN], genbuf[PATHLEN];
    FILE *fn;
    char ans[3], day[10];
    int i, j, k, lcount, tcount;

    modify_user_mode(GIVEUPNET);
    if (!HAS_PERM(currentuser, PERM_LOGINOK)) {
        clear();
        move(11, 28);
        prints("\033[1m\033[33m你有还没有注册通过，不能戒网！\033[m");
        pressanykey();
        return;
    }

    if (HAS_PERM(currentuser, PERM_SYSOP) || HAS_PERM(currentuser, PERM_BOARDS) || HAS_PERM(currentuser, PERM_OBOARDS) || HAS_PERM(currentuser, PERM_ACCOUNTS)
        || HAS_PERM(currentuser, PERM_ANNOUNCE)
        || HAS_PERM(currentuser, PERM_JURY) || HAS_PERM(currentuser, PERM_SUICIDE) || HAS_PERM(currentuser, PERM_CHATOP)) {
        clear();
        move(11, 28);
        prints("\033[1m\033[33m你有重任在身，不能戒网！\033[m");
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
        prints("你现在的戒网情况：\n\n");
        while (!feof(fn)) {
            if (fscanf(fn, "%d %d", &i, &j) <= 0)
                break;
            switch (i) {
            case 1:
                prints("上站权限");
                break;
            case 2:
                prints("发表权限");
                break;
            case 3:
                prints("聊天权限");
                break;
            case 4:
                prints("呼叫权限");
                break;
            case 5:
                prints("发信权限");
                break;
            case 6:
                prints("休闲娱乐权限");
                break;
            }
            sprintf(buf, "        还有%d天\n", j - time(0) / 3600 / 24);
            prints(buf);
            lcount++;
        }
        fclose(fn);
        pressanykey();
    }


    clear();
    move(1, 0);
    prints("请选择戒网种类:");
    move(3, 0);
    prints("(0) - 结束");
    move(4, 0);
    prints("(1) - 上站权限");
    move(5, 0);
    prints("(2) - 发表权限");
    move(6, 0);
    prints("(3) - 聊天室权限");
    move(7, 0);
    prints("(4) - 发送消息和呼叫聊天权限");
    move(8, 0);
    prints("(5) - 发信权限");
    move(9, 0);
    prints("(6) - 休闲娱乐权限");

    getdata(12, 0, "请选择 [0]", ans, 2, DOECHO, NULL, true);
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
        prints("\n\n你已经没有了该权限");
        pressanykey();
        return;
    }

    getdata(11, 0, "请输入戒网天数 [0]", day, 4, DOECHO, NULL, true);
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
        prints("\n\n对不起，天数不可以大于生命力...");
        pressanykey();
        return;
    }
    j = time(0) / 3600 / 24 + j;

    move(13, 0);

    if (askyn("你确定要戒网吗\x1b[1;31m(注意:戒网之后不能提前解开!\x1b[m)？", 0) == 1) {
        getdata(15, 0, "请输入密码: ", buf, 39, NOECHO, NULL, true);
        if (*buf == '\0' || !checkpasswd2(buf, currentuser)) {
            prints("\n\n很抱歉, 您输入的密码不正确。\n");
            pressanykey();
            return;
        }

        sethomefile(genbuf, currentuser->userid, "giveup");
        fn = fopen(genbuf, "at");
        if (!fn) {
            prints("\n\n由于系统问题，现在你不能戒网");
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

        prints("\n\n你已经开始戒网了");
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
    prints("\033[32m好难过喔.....\033[m");
    move(3, 0);
    if (askyn("你确定要离开这个大家庭", 0) == 1) {
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
       /* 增加显示用户信息 Bigman:2003.5.11*/
        struct userec uinfo1;
	/*struct userdata ud;*/

    if (uinfo.mode != OFFLINE) {
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
    if (!(id = getuser(userid, &lookupuser))) {
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
        securityreport(secu, lookupuser, NULL);
    }
    sprintf(genbuf, "%s deleted user %s", currentuser->userid, lookupuser->userid);
    bbslog("user","%s",genbuf);
    /*Haohmaru.99.12.23.被删ID一个月内不得注册 */
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
    prints("%s 已经已经和本家庭失去联络....\n", userid);
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
       update_ulist( &uin, ind ); 无意义而且参数有错，所以注释掉 dong 1998.7.7 */
    move(2, 0);
    if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE && uinfo.mode != FRIEND) {
        prints("User has been Kicked\n");
        pressreturn();
        clear();
    }
    return 1;
}
