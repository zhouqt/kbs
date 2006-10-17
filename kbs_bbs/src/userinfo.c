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
*/  
/* KCN modified 2001.5.10 */ 

#include "bbs.h"
extern time_t login_start_time;

void disply_userinfo(struct userec *u, int real)
{
    int num, diff;
	struct userdata ud;

	read_userdata(u->userid, &ud);
    move(real >= 1 ? 2 : 3, 0);
    clrtobot();
    prints("您的代号     : %s\n", u->userid);
    prints("您的昵称     : %s\n", u->username);
    if (real != 2)  //注册单不显示真实姓名
        prints("真实姓名     : %s\n", ud.realname);
    prints("居住住址     : %s\n", ud.address);
    if(real)
	prints("注册E-mail   : %s\n",ud.reg_email);
    else
    	prints("电子邮件信箱 : %s\n", ud.email);

	/*加入生日等显示 added by binxun 2003.5.20*/
#ifdef HAVE_BIRTHDAY
    prints("您的性别     : %s\n",(ud.gender=='M')?"男":"女");
	prints("您的生日     : %d-%d-%d\n",ud.birthyear+1900,ud.birthmonth,ud.birthday);
#endif
#ifdef ZIXIA
    prints("您的修炼道行 : %d\n",u->altar);
#endif
    prints("您的职务     : %s\n",get_user_title(u->title));

    if (real) {
        prints("原始注册资料 : %s\n", ud.realemail);
    }
    prints("注册日期     : %s", ctime(&u->firstlogin));
    prints("最近光临日期 : %s", ctime(&u->lastlogin));
    if (real) {
        prints("最近光临机器 : %s\n", u->lasthost);
    }

    prints("上站次数     : %d 次\n", u->numlogins);
    if (real)
        prints("文章数目     : %d 篇\n", u->numposts);

        /*       if( real ) {
           prints("文章数目     : %d / %d (Board/1Discuss)\n",
           u->numposts, post_in_tin( u->userid ));
           }  removed by stephen 2000-11-02 */
        /*    } */
#if 0
        {
           int exp;
           exp=countexp(u);
           prints("经验值       : %d(%s)\n",exp,c_exp(exp));
           exp=countperf(u);
           prints("表现值       : %d(%s)\n",exp,c_perf(exp));
        }
#endif
        prints("上站总时数   : %d 小时 %d 分钟\n", u->stay / 3600, (u->stay / 60) % 60);
#ifdef NEWSMTH
    if(u->score_user>publicshm->us_sample[1])
        sprintf(genbuf,"%d -- RANKING %.2lf%%",u->score_user,100*us_ranking(u->score_user));
    else
        sprintf(genbuf,"%d -- RANKING %.1lf%%",u->score_user,100*us_ranking(u->score_user));
    prints("积分         : %s\n",genbuf);
#else
    prints("\n");
#endif
    if (real) {
        strcpy(genbuf, XPERMSTR);
        for (num = 0; num < (int) strlen(genbuf); num++)
            if (!(u->userlevel & (1 << num)))
                genbuf[num] = '-';
        genbuf[num] = '\0';
        prints("使用者权限   : %s\n", genbuf);
    } else {
        diff = (time(0) - login_start_time) / 60;
        prints("停留期间     : %d 小时 %02d 分\n", diff / 60, diff % 60);
        prints("屏幕大小     : %dx%d\n", t_lines, t_columns);
    }
    prints("\n");
    if (u->userlevel & PERM_LOGINOK) {
        prints("  您的注册程序已经完成, 欢迎加入本站.\n");
    } else if (u->lastlogin - u->firstlogin < 3 * 86400) {
        prints("  新手上路, 请阅读 Announce 讨论区.\n");
    } else {
        prints("  注册尚未成功, 请参考本站进站画面说明.\n");
    }
}
int uinfo_query(struct userec *u, int real, int unum) 
{
    struct userec newinfo;
	const char *emailfile;
    char ans[3], buf[STRLEN], genbuf[STRLEN];
    int i, fail = 0, netty_check = 0, tmpchange = 0;
    FILE * fin, *fout, *dp;
    time_t code;
	struct userdata ud;
	struct usermemo *um;
	
	time_t now;
	struct tm *tmnow;

	now = time(0);
	tmnow = localtime(&now);

    memcpy(&newinfo, u, sizeof(struct userec));
	read_userdata(u->userid, &ud);
	read_user_memo(u->userid, &um);
	//memcpy(&ud, &(getSession()->currentmemo->ud), sizeof(ud));
    getdata(t_lines - 1, 0, real ? "请选择 (0)结束 (1)修改资料 (2)设定密码 (3) 改 ID ==> [0]" : "请选择 (0)结束 (1)修改资料 (2)设定密码 ==> [0]", ans, 2, DOECHO, NULL, true);
    clear();
    i = 3;
    move(i++, 0);
    if (ans[0] != '3' || real)
        prints("使用者代号: %s\n", u->userid);
    switch (ans[0]) {
    case '1':
        move(1, 0);
        prints("请逐项修改,直接按 <ENTER> 代表使用 [] 内的资料。\n");
        sprintf(genbuf, "昵称 [%s]: ", u->username);
        getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, NULL, true);
        if (buf[0])
            strncpy(newinfo.username, buf, NAMELEN);
        sprintf(genbuf, "真实姓名 [%s]: ", ud.realname);
        getdata(i++, 0, genbuf, buf, NAMELEN, DOECHO, NULL, true);
        if (buf[0])
            strncpy(ud.realname, buf, NAMELEN);
        sprintf(genbuf, "居住地址 [%s]: ", ud.address);
        getdata(i++, 0, genbuf, buf, STRLEN, DOECHO, NULL, true);
        if (buf[0])
            strncpy(ud.address, buf, NAMELEN);
        sprintf(genbuf, "电子信箱 [%s]: ", ud.email);
        getdata(i++, 0, genbuf, buf, STRLEN, DOECHO, NULL, true);
        if (buf[0])
             {

                /*netty_check = 1; */
                /* 取消email 认证, alex , 97.7 */
                strncpy(ud.email, buf, STRLEN);
            }
		/* 加入修改生日等 added by binxun 2003.5.20 */
#ifdef  HAVE_BIRTHDAY
        sprintf(genbuf, "您的性别: [1]男 [2]女 (1 or 2)[%d]",(ud.gender=='M')?1:2);
	    do{
            getdata(i, 0, genbuf,buf, 2, DOECHO, NULL, true);
			if(buf[0]==0)break;
        }while (buf[0] < '1' || buf[0] > '2');
        i++;
		switch (buf[0])
		{
		case '1':
			ud.gender = 'M';
			break;
		case '2':
			ud.gender = 'F';
			break;
		default:
		    break;
		}

		prints("请输入您的出生日期");
		i++;
        do{
			buf[0] = '\0';
			sprintf(genbuf, "四位数公元年: [%d]: ", ud.birthyear+1900);
			getdata(i, 0, genbuf, buf, 5, DOECHO, NULL, true);
			if(buf[0]) {
				if (atoi(buf) < 1900)continue;
				ud.birthyear = atoi(buf) - 1900;
			}
		}while (ud.birthyear < tmnow->tm_year - 98 || ud.birthyear > tmnow->tm_year - 3);
		i++;

        do{
			buf[0] = '\0';
			sprintf(genbuf, "出生月: (1-12) [%d]: ", ud.birthmonth);
			getdata(i, 0, genbuf, buf, 3, DOECHO, NULL, true);
			if(buf[0]) {
				ud.birthmonth = atoi(buf);
			}
		}while (ud.birthmonth < 1 || ud.birthmonth > 12);
		i++;

        do
		{
			buf[0] = '\0';
			sprintf(genbuf, "出生日: (1-31) [%d]: ", ud.birthday);
			getdata(i, 0, genbuf, buf, 3, DOECHO, NULL, true);
			if((buf[0] && atoi(buf) >= 1 && atoi(buf) <= 31))
				ud.birthday = atoi(buf);
		}while (!is_valid_date(ud.birthyear + 1900,
					ud.birthmonth,
					ud.birthday));
		i++;
#endif

        if (real) {
	usertitle:
	     sprintf(genbuf, "当前职务: %s[%d](可以直接输入职务，输入0除去职务): ", get_user_title(u->title),u->title);
         getdata(i++, 0, genbuf, buf, STRLEN, DOECHO, NULL, true);
         if (buf[0]) {
            if (!strcmp(buf, "0")) {
                newinfo.title = 0;
            } else {
                unsigned char ititle,tflag;
                ititle = atoi(buf);
                if (ititle > 0) {
                    newinfo.title = ititle;
                } else {
                    ititle=0;tflag=0;
                    do{
                        ititle++;
                        if(!strcmp(buf,get_user_title(ititle))){
                            newinfo.title=ititle;
                            tflag=1;
                            break;
                        }
                    }while(ititle<255);
                    if(!tflag){
                        prints("职务表内没有此职务，请先修改用户职务表\n");
                        pressreturn();
                        i--;
                        goto usertitle;
                    }
                }
            }
        }
            sprintf(genbuf, "真实Email[%s]: ", ud.realemail);
            getdata(i++, 0, genbuf, buf, STRLEN, DOECHO, NULL, true);
            if (buf[0])
                strncpy(ud.realemail, buf, STRLEN - 16);
            sprintf(genbuf, "上线次数 [%d]: ", u->numlogins);
            getdata(i++, 0, genbuf, buf, 16, DOECHO, NULL, true);
            if (atoi(buf) > 0)
                newinfo.numlogins = atoi(buf);
            sprintf(genbuf, "文章数目 [%d]: ", u->numposts);
            getdata(i++, 0, genbuf, buf, 16, DOECHO, NULL, true);
             {
                int lres;

                lres = atoi(buf);
                if (lres > 0 || ('\0' == buf[1] && '0' == *buf))
                    newinfo.numposts = lres;
            }
            
                /*            if( atoi( buf ) > 0 ) newinfo.numposts = atoi( buf ); */ 
                sprintf(genbuf, "将注册日期提前" REGISTER_WAIT_TIME_NAME " [Y/N]");
            getdata(i++, 0, genbuf, buf, 16, DOECHO, NULL, true);
            if (buf[0] == 'y' || buf[0] == 'Y')
                newinfo.firstlogin -= REGISTER_WAIT_TIME;
            sprintf(genbuf, "将最近光临日期设置为今天吗？[Y/N]");
            getdata(i++, 0, genbuf, buf, 16, DOECHO, NULL, true);
            if (buf[0] == 'y' || buf[0] == 'Y')
                newinfo.lastlogin = time(0);
        }
        break;
    case '3':
        if (!real) {
			end_mmapfile(um, sizeof(struct usermemo), -1);
            clear();
            return 0;
        }
        
            /* Bigman 2000.10.2 修改使用者ID位数不够 */ 
            getdata(i++, 0, "新的使用者代号: ", genbuf, IDLEN + 1, DOECHO, NULL, true);
	 if (strncmp(genbuf,u->userid, IDLEN+2)&&!strncasecmp(genbuf,u->userid, IDLEN+2)) tmpchange = 1;
        if (*genbuf != '\0') {
            if (searchuser(genbuf) && !tmpchange) {
                prints("\n错误! 已经有同样 ID 的使用者\n");
                fail++;
                break;
            } else {
                strncpy(newinfo.userid, genbuf, IDLEN + 2);
				strncpy(ud.userid, genbuf, IDLEN + 2);
            }
        } else {
            break;
        }
        
            /* fall throw, must change passwd for newid, by wwj 2001/5/7 */ 
    case '2':
        if (!real) {
            getdata(i++, 0, "请输入原密码: ", buf, 39, NOECHO, NULL, true);
            if (*buf == '\0' || !checkpasswd2(buf, u)) {
                prints("\n\n很抱歉, 您输入的密码不正确。\n");
                fail++;
                break;
            }
        }
        getdata(i++, 0, "请设定新密码: ", buf, 39, NOECHO, NULL, true);
        if (buf[0] == '\0') {
            prints("\n\n密码设定取消, 继续使用旧密码\n");
            if(ans[0]!='3')
                fail++;
            break;
        }
        getdata(i++, 0, "请重新输入新密码: ", genbuf, 39, NOECHO, NULL, true);
        if (strcmp(buf, genbuf)) {
            prints("\n\n两个密码不一致, 无法设定新密码。\n");
            fail++;
            break;
        }
        
            /*      Added by cityhunter to deny others to modify SYSOP's passwd */ 
            if (real && (strcmp(u->userid, "SYSOP") == 0))
             {
            prints("\n\n错误!系统禁止修改SYSOP的密码," NAME_POLICE "正在来的路上 :)");
            pressreturn();
            clear();
			end_mmapfile(um, sizeof(struct usermemo), -1);
            return 0;
            }
        
            /* end of this addin */ 
            setpasswd(buf, &newinfo);
        break;
    default:
        clear();
		end_mmapfile(um, sizeof(struct usermemo), -1);
        return 0;
    }
    if (fail != 0) {
        pressreturn();
        clear();
		end_mmapfile(um, sizeof(struct usermemo), -1);
        return 0;
    }
    for (;;)
	{
        getdata(t_lines - 1, 0, "确定要改变吗?  (Yes or No): ", ans, 2, DOECHO, NULL, true);
        if (*ans == 'n' || *ans == 'N')
            break;
        if (*ans == 'y' || *ans == 'Y') {
			if (!real) {
                update_user(&newinfo, getSession()->currentuid, 1);
                if (strcmp(newinfo.username, uinfo.username)) {
                    strcpy(uinfo.username, newinfo.username);
                    UPDATE_UTMP_STR(username, uinfo);
                }
            }
	     if (strcmp(u->userid, newinfo.userid)) {
                char src[STRLEN], dst[STRLEN];
                if (id_invalid(newinfo.userid) == 1) {
                    prints("帐号必须由英文字母或数字组成，并且第一个字符必须是英文字母!\n");
                    pressanykey();
                } else {
                    setmailpath(src, u->userid);
                    setmailpath(dst, newinfo.userid);
                
                    /*
                       sprintf(genbuf,"mv %s %s",src, dst);
                     */ 
                    f_mv(src, dst);
                    sethomepath(src, u->userid);
                    sethomepath(dst, newinfo.userid);
                
                    /*
                       sprintf(genbuf,"mv %s %s",src ,dst);
                     */ 
                    f_mv(src, dst);
                    sprintf(src, "tmp/email/%s", u->userid);
                    unlink(src);
                    setuserid(unum, newinfo.userid);
                }
            }
            
                /* added by netty to automatically send a mail to new user. */ 
                if ((netty_check == 1))
                 {
                if ((strchr(ud.email, '@') != NULL) && 
                     (!strstr(ud.email, "@firebird.cs")) && (!strstr(ud.email, "@bbs.")) && (!invalidaddr(ud.email)) && (!strstr(ud.email, ".bbs@"))) {
                    if ((emailfile = sysconf_str("EMAILFILE")) != NULL)
                         {
                        code = (time(0) / 2) + (rand() / 10);
                        sethomefile(genbuf, u->userid, "mailcheck");
                        if ((dp = fopen(genbuf, "w")) == NULL)
                             {
                            fclose(dp);
                            return -1;
                            }
                        fprintf(dp, "%9.9lu\n", code);
                        fclose(dp);
                        sprintf(genbuf, "/usr/lib/sendmail -f SYSOP@%s %s ", email_domain(), ud.email);
                        fout = popen(genbuf, "w");
                        fin = fopen(emailfile, "r");
                        if (fin == NULL || fout == NULL)
                            return -1;
                        fprintf(fout, "Reply-To: SYSOP@%s\n", email_domain());
                        fprintf(fout, "From: SYSOP@%s\n", email_domain());
                        fprintf(fout, "To: %s\n", ud.email);
                        fprintf(fout, "Subject: @%s@[-%9.9lu-]firebird mail check.\n", u->userid, code);
                        fprintf(fout, "X-Forwarded-By: SYSOP \n");
                        fprintf(fout, "X-Disclaimer: None\n");
                        fprintf(fout, "\n");
                        fprintf(fout, "您的基本资料如下：\n");
                        fprintf(fout, "使用者代号：%s (%s)\n", u->userid, u->username);
                        fprintf(fout, "姓      名：%s\n", ud.realname);
                        fprintf(fout, "上站位置  ：%s\n", u->lasthost);
                        fprintf(fout, "电子邮件  ：%s\n\n", ud.email);
                        fprintf(fout, "亲爱的 %s(%s):\n", u->userid, u->username);
                        while (fgets(genbuf, 255, fin) != NULL) {
                            if (genbuf[0] == '.' && genbuf[1] == '\n')
                                fputs(". \n", fout);
                            
                            else
                                fputs(genbuf, fout);
                        }
                        fprintf(fout, ".\n");
                        fclose(fin);
                        pclose(fout);
                        }
                } else
                     {
                    if (sysconf_str("EMAILFILE") != NULL)
                         {
                        move(t_lines - 5, 0);
                        prints("\n你的电子邮件地址 【\033[33m%s\033[m】\n", ud.email);
                        prints("并非 Unix 帐号，系统不会投递身份确认信，请到\033[32m工具箱\033[m中修改..\n");
                        pressanykey();
                        }
                    }
                }
            update_user(&newinfo, unum, 1);
			memcpy(&(um->ud), &ud, sizeof(ud));
			write_userdata(newinfo.userid, &ud);
            if (real)
                 {
                char secu[STRLEN];

                if (strcmp(u->userid, newinfo.userid))
                    sprintf(secu, "%s 的 ID 被 %s 改为 %s", u->userid, getCurrentUser()->userid, newinfo.userid);   /*Haohmaru.99.5.6 */
                
                else
                    sprintf(secu, "修改 %s 的基本资料或密码。", u->userid);
                securityreport(secu, &newinfo, NULL, getSession());
            }
            break;             /*Haohmaru.98.01.10.faint...Luzi加个for循环也不break! */
        }
	} /* for(;;) */
    clear();
	end_mmapfile(um, sizeof(struct usermemo), -1);
    return 0;
}
void x_info() 
{
    modify_user_mode(GMENU);
    disply_userinfo(getCurrentUser(), 1);
    if (!strcmp("guest", getCurrentUser()->userid)) {
        pressreturn();
        return;
    }
    uinfo_query(getCurrentUser(), 0, getSession()->currentuid);
}
    void getfield(line, info, desc, buf, len)  int line, len;
    char *info, *desc, *buf;


{
    char prompt[STRLEN];

    
        /*    sprintf( genbuf, "  原先设定: %-46.46s (%s)", buf, info ); */ 
        sprintf(genbuf, "  原先设定: %-20.20s (%s)", buf, info);
    move(line, 0);
    prints(genbuf);
    sprintf(prompt, "  %s: ", desc);
    getdata(line + 1, 0, prompt, genbuf, len, DOECHO, NULL, true);
    if (genbuf[0] != '\0') {
        strncpy(buf, genbuf, len);
    }
    move(line, 0);
    clrtoeol();
    prints("  %s: %s\n", desc, buf);
    clrtoeol();
}
void x_fillform() 
{
    char rname[NAMELEN], addr[STRLEN];
    char phone[STRLEN], career[STRLEN], birth[STRLEN];
    char ans[5], *mesg, *ptr;
    FILE * fn;
    time_t now;
    struct userdata ud;

    modify_user_mode(NEW);
    move(3, 0);
    clrtobot();
    if (!strcmp("guest", getCurrentUser()->userid)) {
        prints("抱歉, 请用 new 申请一个新帐号后再填申请表.");
        pressreturn();
        return;
    }
    if (getCurrentUser()->userlevel & PERM_LOGINOK) {
        prints("您的身份确认已经成功, 欢迎加入本站的行列.");
        pressreturn();
        return;
    }
    if ((time(0) - getCurrentUser()->firstlogin) < REGISTER_WAIT_TIME)
	{
        prints("您首次登入本站未满" REGISTER_WAIT_TIME_NAME "...");
        prints("请先四处熟悉一下，在满" REGISTER_WAIT_TIME_NAME "以后再填写注册单。");
        pressreturn();
        return;
	}
    
	if ((fn = fopen("new_register", "r")) != NULL) {
        while (fgets(genbuf, STRLEN, fn) != NULL) {
            if ((ptr = strchr(genbuf, '\n')) != NULL)
                *ptr = '\0';
            if (strncmp(genbuf, "userid: ", 8) == 0 && strcmp(genbuf + 8, getCurrentUser()->userid) == 0) {
                fclose(fn);
                prints("站长尚未处理您的注册申请单, 请耐心等候.");
                pressreturn();
                return;
            }
        }
        fclose(fn);
    }
    
	/* added by KCN 1999.10.25 */ 
	ansimore("etc/register.note", false);
    getdata(t_lines - 1, 0, "您确定要填写注册单吗 (Y/N)? [N]: ", ans, 3, DOECHO, NULL, true);
    if (ans[0] != 'Y' && ans[0] != 'y')
        return;
//    memcpy(&ud,&curruserdata,sizeof(ud));
    memcpy(&ud,&(getSession()->currentmemo->ud),sizeof(ud));
    strncpy(rname, ud.realname, NAMELEN);
    strncpy(addr, ud.address, STRLEN);
    career[0] = phone[0] = birth[0] = '\0';
    clear();
    while (1) {
        move(3, 0);
        clrtoeol();
        prints("%s 您好, 请据实填写以下的资料(请使用中文):\n", getCurrentUser()->userid);
        genbuf[0] = '\0';      /*Haohmaru.99.09.17.以下内容不得过短 */
        while (strlen(genbuf) < 3) {
            getfield(6, "请用中文,不能输入的汉字请用拼音", "真实姓名", rname, NAMELEN);
        }
        genbuf[0] = '\0';
        while (strlen(genbuf) < 2) {
            getfield(8, "学校系级或单位全称及所属部门", "服务单位", career, STRLEN);
        }
        genbuf[0] = '\0';
        while (strlen(genbuf) < 6) {
            getfield(10, "请具体到寝室或门牌号码", "目前住址", addr, STRLEN);
        }
        genbuf[0] = '\0';
        while (strlen(genbuf) < 2) {
            getfield(12, "包括可连络时间,若无可用呼机或Email地址代替", "连络电话", phone, STRLEN);
        }
#ifndef HAVE_BIRTHDAY
        getfield(14, "年.月.日(公元)", "出生年月", birth, STRLEN);
#endif
        mesg = "以上资料是否正确, 按 Q 放弃注册 (Y/N/Quit)? [N]: ";
        getdata(t_lines - 1, 0, mesg, ans, 3, DOECHO, NULL, true);
        if (ans[0] == 'Q' || ans[0] == 'q')
            return;
        if (ans[0] == 'Y' || ans[0] == 'y')
            break;
    }
    strncpy(ud.realname, rname, NAMELEN);
    strncpy(ud.address, addr, STRLEN);
	write_userdata(getCurrentUser()->userid, &ud);
//	memcpy(&curruserdata,&ud,sizeof(ud));
	memcpy(&(getSession()->currentmemo->ud),&ud,sizeof(ud));
    if ((fn = fopen("new_register", "a")) != NULL) {
        now = time(NULL);
        fprintf(fn, "usernum: %d, %s", getSession()->currentuid, ctime(&now));
        fprintf(fn, "userid: %s\n", getCurrentUser()->userid);
        fprintf(fn, "realname: %s\n", rname);
        fprintf(fn, "career: %s\n", career);
        fprintf(fn, "addr: %s\n", addr);
        fprintf(fn, "phone: %s\n", phone);
#ifndef HAVE_BIRTHDAY
        fprintf(fn, "birth: %s\n", birth);
#else
        fprintf(fn, "birth: %d-%d-%d\n", ud.birthyear, ud.birthmonth, ud.birthday);
#endif
        fprintf(fn, "----\n");
        fclose(fn);
    }
}

/* etnlegend, 2006.09.21, 修改用户资料接口... */

#define MU_LENGTH                       256
#define MU_ITEM                         20
#define MU_KEY_RESET                    '~'
#define MU_GEN_N(s)                     (s)
#define MU_GEN_R(s)                     ("\033[1;31m" s "\033[m")
#define MU_GEN_Y(s)                     ("\033[1;33m" s "\033[m")
#define MU_GEN_G(s)                     ("\033[1;32m" s "\033[m")
#define MU_GEN_C(s)                     ("\033[1;36m" s "\033[m")
#define MU_MSG(f,s)                     (MU_GEN_##f(s))
#define MU_PUT(r,s)                                                                             \
    do{                                                                                         \
        move((r),0);                                                                            \
        clrtoeol();                                                                             \
        move((r),2);                                                                            \
        prints("%s\033[0;33m<Enter>\033[m",(s));                                                \
        WAIT_RETURN;                                                                            \
    }                                                                                           \
    while(0)
#define MU_GET_CORE(r,p,b,l,t)                                                                  \
    do{                                                                                         \
        if(r==(MU_ITEM+2)){                                                                     \
            move((MU_ITEM+1),0);                                                                \
            clrtobot();                                                                         \
        }                                                                                       \
        else{                                                                                   \
            move((r),0);                                                                        \
            clrtoeol();                                                                         \
        }                                                                                       \
        getdata((r),2,(p),(b),((l)+1),(t),NULL,true);                                           \
    }                                                                                           \
    while(0)
#define MU_GET(r,p,b,l)                 MU_GET_CORE(r,p,b,l,DOECHO)
#define MU_GETPWD(r,p,b,l)              MU_GET_CORE(r,p,b,l,NOECHO)

struct mu_item{
    int x;
    int y;
    int key;
    int access;
    const char *prefix;
    const char *menu;
};

struct mu_arg{
    enum {MU_MENU_INIT,MU_MENU_SELECT,MU_MENU_RESET,MU_MENU_QUIT} type;
    const unsigned int *access;
    const struct mu_item *item;
};

struct mu_so_arg{
    char from[18][IPLEN];
    int count;
};

struct mu_so_key{
    const char *value;
    int count;
};

static inline int mu_digit_string(const char *s){
    while(isdigit(*s++))
        continue;
    return (!*--s);
}

static inline int mu_show_help(int n){
    static const char *help[MU_ITEM]={
        "选择此项可进行用户名称修改相关操作...",
        "选择此项可进行用户密码修改相关操作...",
        "选择此项可进行用户昵称修改相关操作...",
        "选择此项可进行用户真实姓名修改相关操作...",
        "选择此项可进行用户性别修改相关操作...",
        "选择此项可进行用户出生日期修改相关操作...",
        "选择此项可进行用户通讯地址修改相关操作...",
        "选择此项可进行用户电子邮件地址修改相关操作...",
        "选择此项可进行用户联系电话修改相关操作...",
        "选择此项可进行用户身份修改相关操作...",
        "选择此项可进行用户原始注册资料修改相关操作...",
        "选择此项可进行用户注册时间调整相关操作...",
        "选择此项可进行用户最近访问时间更新相关操作...",
        "选择此项可进行用户当前登录来源查看相关操作...",
        "选择此项可进行用户登录统计修改相关操作...",
        "选择此项可进行用户发表文章统计修改相关操作...",
        "选择此项可进行用户日均在线时间查看相关操作...",
        "选择此项可进行用户积分修改相关操作...",
        "选择此项可进行用户权限修改相关操作...",
        "<Enter> 选择, <Esc> 退出, <~> 恢复原设定..."
    };
    char buf[MU_LENGTH];
    snprintf(buf,MU_LENGTH,"  [帮助] %-.64s",help[n]);
    move((t_lines-1),0);
    clrtoeol();
    prints("\033[1;34;47m%s\033[K\033[m",buf);
    return 0;
}

static inline int mu_show_hint(const char *h){
    char buf[MU_LENGTH];
    snprintf(buf,MU_LENGTH,"  原始设定值 [%-.64s]",h);
    move((t_lines-1),0);
    clrtoeol();
    prints("\033[0;32;47m%s\033[K\033[m",buf);
    return 0;
}

static inline int mu_get_time_desc(time_t *timestamp,char *buf,int from,int length){
    char *p;
    if(!(p=ctime(timestamp))){
        buf[0]=0;
        return -1;
    }
    snprintf(buf,length,"%s",&p[from]);
    if((p=strchr(buf,'\n')))
        *p=0;
    return 0;
}

static int mu_show_online_list(struct user_info *info,void *varg,int pos){
    struct mu_so_arg *arg=(struct mu_so_arg*)varg;
    char buf[MU_LENGTH];
#ifndef HAVE_IPV6_SMTH
    char desc[32];
#endif /* ! HAVE_IPV6_SMTH */
    if(!(info->active))
        return 0;
    if(!(arg->count<18))
        return 0;
    move((arg->count+4),0);
#ifndef HAVE_IPV6_SMTH
    mu_get_time_desc(&(info->logintime),desc,0,32);
    snprintf(buf,MU_LENGTH,"[%02d]  %-24.24s%-32.32s%s",(arg->count+1),info->from,desc,ModeType(info->mode));
#else /* HAVE_IPV6_SMTH */
    snprintf(buf,MU_LENGTH,"[%02d]  %-50.50s%s",(arg->count+1),info->from,ModeType(info->mode));
#endif /* ! HAVE_IPV6_SMTH */
    prints((info->invisible?"\033[1;33m%s\033[m":"\033[1;37m%s\033[m"),buf);
    snprintf(arg->from[arg->count++],IPLEN,"%s",info->from);
    return 0;
}

static int mu_show_online_generate_user(struct userec *user,void *varg){
    struct mu_so_key *key=(struct mu_so_key*)varg;
    if(!(user->userid[0]))
        return 0;
    if(!strcmp(key->value,user->lasthost)&&(key->count<100)){
        move((3+(key->count%20)),(2+((key->count/20)<<4)));
        prints("\033[1;37m%s\033[m",user->userid);
        key->count++;
    }
    return 0;
}

static int mu_show_online_generate_utmp(struct user_info *info,void *varg,int pos){
    struct mu_so_key *key=(struct mu_so_key*)varg;
    if(!(info->active))
        return 0;
    if(!strcmp(key->value,info->from)&&(key->count<100)){
        move((3+(key->count%20)),(2+((key->count/20)<<4)));
        prints((info->invisible?"\033[1;32m%s\033[m":"\033[1;37m%s\033[m"),info->userid);
        key->count++;
    }
    return 0;
}

static inline int mu_show_online(int uid,int mode){
    struct mu_so_arg arg;
    struct mu_so_key key;
    const struct userec *urec;
    char buf[MU_LENGTH];
    int use_utmp_method;
    move(2,0);
    clrtobot();
    if(!(urec=getuserbynum(uid))||!(urec->userid[0])){
        MU_PUT(2,MU_MSG(R,"无法获取所请求用户的信息..."));
        return -1;
    }
#ifndef HAVE_IPV6_SMTH
    snprintf(buf,MU_LENGTH,"%-6.6s%-24.24s%-32.32s%s","序号","来源地址","登录时间","当前状态");
    prints("\033[0;32;47m%s\033[K\033[m",buf);
    snprintf(buf,MU_LENGTH,"%-6.6s%-24.24s%-32.32s%s","[00]",urec->lasthost,"[无法确定]","[最近访问]");
    move(3,0);
    prints("\033[1;33;46m%s\033[K\033[m",buf);
#else /* HAVE_IPV6_SMTH */
    snprintf(buf,MU_LENGTH,"%-6.6s%-50.50s%s","序号","来源地址","登录时间");
    prints("\033[0;32;47m%s\033[K\033[m",buf);
    snprintf(buf,MU_LENGTH,"%-6.6s%-50.50s%s","[00]",urec->lasthost,"[最近访问来源]");
    move(3,0);
    prints("\033[1;33;46m%s\033[K\033[m",buf);
#endif /* ! HAVE_IPV6_SMTH */
    memset(&arg,0,sizeof(struct mu_so_arg));
    apply_utmp(mu_show_online_list,0,urec->userid,&arg);
    move((t_lines-1),0);
    if(!mode){
        prints("\033[0;32;47m%s\033[K\033[m","列示完成, 按 <Enter> 键退出...");
        WAIT_RETURN;
        return 0;
    }
    prints("\033[0;32;47m%s\033[K\033[m","列示完成, 按 <Enter> 键退出, 按 <A>/<L> 键进行来源分析...");
    use_utmp_method=0;
    do{
        switch(igetkey()){
            case '\n':
            case '\r':
                return 1;
            case 'A':
            case 'a':
                break;
            case 'L':
            case 'l':
                use_utmp_method=1;
                break;
            default:
                continue;
        }
        break;
    }
    while(1);
    move((t_lines-1),0);
    clrtoeol();
    getdata((t_lines-1),0,MU_MSG(Y,"请输入需要进行分析的项目序号 [0]: "),buf,3,DOECHO,NULL,true);
    trimstr(buf);
    if(isdigit(buf[0])){
        buf[0]=atoi(buf);
        buf[0]=((buf[0]>arg.count)?0:buf[0]);
    }
    else
        buf[0]=0;
    key.value=(buf[0]?arg.from[buf[0]-1]:urec->lasthost);
    key.count=0;
    move(2,0);
    clrtobot();
    snprintf(buf,MU_LENGTH,"以下为%s登录来源为 %s 的用户...",(use_utmp_method?"当前在线":"最近访问"),key.value);
    prints("\033[0;32;47m%s\033[K\033[m",buf);
    if(use_utmp_method)
        apply_ulist(mu_show_online_generate_utmp,&key);
    else
        apply_users(mu_show_online_generate_user,&key);
    move((t_lines-1),0);
    prints("\033[0;32;47m%s\033[K\033[m","列示完成, 按 <Enter> 键退出...");
    WAIT_RETURN;
    return 2;
}

static inline int mu_generate_level(int row,int col,unsigned int *level,unsigned int init){
#define MU_GL_N                         "\033[1;33m%c\033[m"
#define MU_GL_A                         "\033[1;32m%c\033[m"
#define MU_GL_D                         "\033[1;31m%c\033[m"
#define MU_GL_U                         "\033[1;34m%c\033[m"
#define MU_GL_B                         "\033[1;37m%c\033[m"
#define MU_GL_C(t)                      ("\033[4m" MU_GL_##t)
    int i,key,next;
    unsigned int perm,mask;
    *level=(perm=init);
    move(row,col);
    prints(MU_GL_B,'[');
    for(i=0,mask=0x01;i<NUMPERMS;i++,mask<<=1)
        prints(((perm&mask)?MU_GL_N:MU_GL_U),XPERMSTR[i]);
    prints(MU_GL_B,']');
    next=0;
    do{
        i=next;
        mask=(1<<i);
        move(row,(col+(i+1)));
        if((perm^init)&mask)
            prints(((perm&mask)?MU_GL_C(A):MU_GL_C(D)),XPERMSTR[i]);
        else
            prints(((perm&mask)?MU_GL_C(N):MU_GL_C(U)),XPERMSTR[i]);
        move(row,(col+(NUMPERMS+2)));
        clrtoeol();
        prints(" \033[1;36m%s\033[m",permstrings[i]);
        move(row,2);
        do{
            switch((key=igetkey())){
                case KEY_ESC:
                    return -1;
                case '\n':
                case '\r':
                    *level=perm;
                    return 0;
                case ' ':
                    perm^=mask;
                    break;
                case KEY_LEFT:
                    next=((i==0)?(NUMPERMS-1):(i-1));
                    break;
                case KEY_RIGHT:
                    next=((i==(NUMPERMS-1))?0:(i+1));
                    break;
                case KEY_HOME:
                    next=0;
                    if(next==i)
                        continue;
                    break;
                case KEY_END:
                    next=(NUMPERMS-1);
                    if(next==i)
                        continue;
                    break;
                default:
                    for(next=(NUMPERMS-1);!(next<0);next--)
                        if(key==XPERMSTR[next])
                            break;
                    if(next==-1&&isalpha(key)){
                        for(next=(NUMPERMS-1);!(next<0);next--)
                            if(toupper(key)==toupper(XPERMSTR[next]))
                                break;
                    }
                    if(next==-1){
                        next=i;
                        continue;
                    }
                    if(next==i)
                        continue;
                    break;
            }
            break;
        }
        while(1);
        if(next!=i){
            move(row,(col+(i+1)));
            if((perm^init)&mask)
                prints(((perm&mask)?MU_GL_A:MU_GL_D),XPERMSTR[i]);
            else
                prints(((perm&mask)?MU_GL_N:MU_GL_U),XPERMSTR[i]);
        }
    }
    while(1);
    return 1;
#undef MU_GL_N
#undef MU_GL_A
#undef MU_GL_D
#undef MU_GL_U
#undef MU_GL_B
#undef MU_GL_C
}

static inline int mu_process_directories(const char *ouserid,const char *nuserid){
    char src[MU_LENGTH],dst[MU_LENGTH];
    setmailpath(dst,nuserid);
    setmailpath(src,ouserid);
    my_f_rm(dst);
    if(f_mv(src,dst)==-1)
        return -1;
    sethomepath(dst,nuserid);
    sethomepath(src,ouserid);
    my_f_rm(dst);
    if(f_mv(src,dst)==-1){
        return -1;
    }
    snprintf(dst,MU_LENGTH,"tmp/email/%s",ouserid);
    f_rm(dst);
    return 0;
}

static int mu_menu_on_select(struct _select_def *conf){
    struct mu_arg *arg=(struct mu_arg*)conf->arg;
    const struct mu_item *item=&(arg->item[conf->pos-1]);
    if(item->access){
        arg->type=MU_MENU_SELECT;
        return SHOW_SELECT;
    }
    return SHOW_CONTINUE;
}

static int mu_menu_show(struct _select_def *conf,int pos){
    struct mu_arg *arg=(struct mu_arg*)conf->arg;
    const struct mu_item *item=&(arg->item[pos-1]);
    move(item->y,item->x);
    prints((item->access?"[\033[1;31m%c\033[m]%s: %s":"[\033[0;33m%c\033[m]%s: %s"),item->key,item->prefix,item->menu);
    return SHOW_CONTINUE;
}

static int mu_menu_pre_key(struct _select_def *conf,int *key){
    struct mu_arg *arg=(struct mu_arg*)conf->arg;
    int pos=conf->pos;
    unsigned int current=(1<<(pos-1));
    unsigned int access=((*(arg->access))&(~current));
    switch(*key){
        case KEY_UP:
            if(!(access&(current-1))){
                pos=MU_ITEM;
                current=(1<<(pos-1));
            }
            while(!(access&current)){
                pos--;
                current>>=1;
            }
            break;
        case KEY_DOWN:
            if(!(access&~(current-1))){
                pos=1;
                current=(1<<(pos-1));
            }
            while(!(access&current)){
                pos++;
                current<<=1;
            }
            break;
        default:
            return SHOW_CONTINUE;
    }
    conf->new_pos=pos;
    return SHOW_SELCHANGE;
}

static int mu_menu_key(struct _select_def *conf,int key){
    struct mu_arg *arg=(struct mu_arg*)conf->arg;
    const struct mu_item *item=arg->item;
    int value,index;
    if(key==KEY_ESC){
        arg->type=MU_MENU_QUIT;
        return SHOW_QUIT;
    }
    if(key==MU_KEY_RESET&&item[conf->pos-1].access){
        arg->type=MU_MENU_RESET;
        return SHOW_QUIT;
    }
    for(value=toupper(key),index=0;index<MU_ITEM;index++)
        if(item[index].access&&item[index].key==value){
            conf->new_pos=(index+1);
            return SHOW_SELCHANGE;
        }
    return SHOW_CONTINUE;
}

static int mu_menu_on_change(struct _select_def *conf,int new_pos){
    mu_show_help(new_pos-1);
    return SHOW_CONTINUE;
}

int modify_userinfo(int uid,int mode){
#define MU_ACCESS_USER                  0x000921FE
#define MU_ACCESS_ADMIN                 0x000FFFFF
#define MU_ACCESS_READ                  0x00092000
#define MU_ACCESS(d)                    (access&(1<<(d)))
#define MU_SIZE(t,f)                    (sizeof(((const struct t*)0)->f))
#define MU_SIZE_user(f)                 MU_SIZE(userec,f)
#define MU_SIZE_data(f)                 MU_SIZE(userdata,f)
#define MU_DIFF_str(x,y,l)              (strncmp((const void*)(x),(const void*)(y),(l)))
#define MU_DIFF_val(x,y,l)              ((x)!=(y))
#define MU_MENUFORM(d,c,f,s)                                                                    \
    do{                                                                                         \
        snprintf(menu[d],MU_LENGTH,MU_MSG(c,f),(s));                                            \
    }                                                                                           \
    while(0)
#define MU_CAST_0(d,i,m,f,s)                                                                    \
    do{                                                                                         \
        MU_MENUFORM(d,G,f,n##i.m);                                                              \
    }                                                                                           \
    while(0)
#define MU_CAST_buf(d,i,m,f,s)                                                                  \
    do{                                                                                         \
        MU_MENUFORM(d,G,f,s);                                                                   \
    }                                                                                           \
    while(0)
#define MU_SET(d,i,m,t,f,s)                                                                     \
    do{                                                                                         \
        if(MU_DIFF_##t(n##i.m,o##i.m,MU_SIZE_##i(m))){                                          \
            MU_CAST_##s(d,i,m,f,s);                                                             \
            change|=(1<<(d));                                                                   \
        }                                                                                       \
        else{                                                                                   \
            snprintf(menu[d],MU_LENGTH,"%s",omenu[d]);                                          \
            change&=(~(1<<(d)));                                                                \
        }                                                                                       \
    }                                                                                           \
    while(0)
#define MU_RESET(d,i,m)                                                                         \
    do{                                                                                         \
        if(change&(1<<(d))){                                                                    \
            memcpy(&(n##i.m),&(o##i.m),MU_SIZE_##i(m));                                         \
            snprintf(menu[d],MU_LENGTH,"%s",omenu[d]);                                          \
            change&=(~(1<<(d)));                                                                \
        }                                                                                       \
    }                                                                                           \
    while(0)
#define MU_SHOW_HINT(d)                                                                         \
    do{                                                                                         \
        mu_show_hint(omenu[d]);                                                                 \
    }                                                                                           \
    while(0)
#define MU_GET_TIME(e)                                                                          \
    do{                                                                                         \
        mu_get_time_desc(&(e),buf,0,MU_LENGTH);                                                 \
    }                                                                                           \
    while(0)
#define MU_VERIFY(d,i,m,t)                                                                      \
    do{                                                                                         \
        if(!(change&(1<<(d))))                                                                  \
            break;                                                                              \
        if(MU_DIFF_##t(v##i.m,o##i.m,MU_SIZE_##i(m)))                                           \
            verify|=(1<<(d));                                                                   \
    }                                                                                           \
    while(0)
#define MU_EXEC(d,i,m)                                                                          \
    do{                                                                                         \
        if(!(change&(1<<(d))))                                                                  \
            break;                                                                              \
        memcpy(&(v##i.m),&(n##i.m),MU_SIZE_##i(m));                                             \
    }                                                                                           \
    while(0)
    static const char *prefix[MU_ITEM]={
        "用户名称","用户密码","用户昵称","真实姓名","用户性别",
        "出生日期","通信地址","电子邮件","联系电话","用户身份",
        "注册资料","注册时间","最近访问","登录来源","登录统计",
        "发文统计","在线统计","用户积分","用户权限","结束操作"
    };
    static const char *title="\033[1;32m[设定用户资料]\033[m";
#if !defined(HAVE_BIRTHDAY)||!defined(NEWSMTH)
    static const char *invalid="\033[0;33m<当前站点设定不支持该项功能>\033[m";
#endif /* !HAVE_BIRTHDAY||!NEWSMTH */
    static const char *md5_mask="<MD5 加密过的密码>";
    FILE *fp;
    struct mu_item item[MU_ITEM+1];
    struct _select_def conf;
    struct mu_arg arg;
    struct userec ouser,nuser,vuser,*urec;
    struct userdata odata,ndata,vdata;
    struct usermemo *memo;
    POINT loc[MU_ITEM];
    char omenu[MU_ITEM][MU_LENGTH],menu[MU_ITEM][MU_LENGTH],buf[MU_LENGTH],name[MU_LENGTH];
    unsigned char uc,uf;
    int i,j,k,loop,pos,mail;
    unsigned int access,change,verify,level;
    time_t current;
    switch(mode){
        case 0:
            access=MU_ACCESS_USER;
            break;
        case 1:
            access=MU_ACCESS_ADMIN;
            break;
        case 2:
            access=MU_ACCESS_READ;
            break;
        default:
            return -2;
    }
#ifndef HAVE_BIRTHDAY
    access&=(~0x00000030);
#endif /* ! HAVE_BIRTHDAY */
#ifndef NEWSMTH
    access&=(~0x00020000);
#endif /* ! NEWSMTH */
    modify_user_mode(!mode?GMENU:ADMIN);
    clear();
    move(0,0);
    prints("%s",title);
    if(!(urec=getuserbynum(uid))||!(urec->userid[0])){
        MU_PUT(2,MU_MSG(R,"所请求的用户信息不正确..."));
        return -3;
    }
    if(!mode&&strcmp(urec->userid,getCurrentUser()->userid)){
        MU_PUT(2,MU_MSG(R,"当前的调用类型错误..."));
        return -4;
    }
    memcpy(&ouser,urec,sizeof(struct userec));
    if(read_userdata(urec->userid,&odata)==-1){
        MU_PUT(2,MU_MSG(R,"读取用户数据时发生错误..."));
        return -5;
    }
    memcpy(&nuser,&ouser,sizeof(struct userec));
    memcpy(&ndata,&odata,sizeof(struct userdata));
    for(i=0;i<MU_ITEM;i++){
        loc[i].x=2;
        loc[i].y=(2+i);
        item[i].x=loc[i].x;
        item[i].y=loc[i].y;
        item[i].key=('A'+i);
        item[i].access=MU_ACCESS(i);
        item[i].prefix=prefix[i];
        item[i].menu=menu[i];
    }
    item[i].x=-1;
    item[i].y=-1;
    item[i].key=-1;
    item[i].access=0;
    item[i].prefix=NULL;
    item[i].menu=NULL;
    MU_MENUFORM(0,N,"%s",nuser.userid);
    MU_MENUFORM(1,N,"%s",md5_mask);
    MU_MENUFORM(2,N,"%s",nuser.username);
    MU_MENUFORM(3,N,"%s",ndata.realname);
#ifndef HAVE_BIRTHDAY
    MU_MENUFORM(4,N,"%s",invalid);
    MU_MENUFORM(5,N,"%s",invalid);
#else /* HAVE_BIRTHDAY */
    MU_MENUFORM(4,N,"%s",(ndata.gender=='M'?"男":"女"));
    snprintf(buf,MU_LENGTH,"%04d 年 %02d 月 %02d 日",(ndata.birthyear+1900),
        ndata.birthmonth,ndata.birthday);
    MU_MENUFORM(5,N,"%s",buf);
#endif /* ! HAVE_BIRTHDAY */
    MU_MENUFORM(6,N,"%-.64s",ndata.address);
    MU_MENUFORM(7,N,"%-.64s",ndata.email);
    MU_MENUFORM(8,N,"%-.64s",ndata.telephone);
    snprintf(buf,MU_LENGTH,"[%s] <%u>",(nuser.title?get_user_title(nuser.title):"无"),nuser.title);
    MU_MENUFORM(9,N,"%s",buf);
    MU_MENUFORM(10,N,"%-.64s",ndata.realemail);
    MU_GET_TIME(nuser.firstlogin);
    MU_MENUFORM(11,N,"%s",buf);
    MU_GET_TIME(nuser.lastlogin);
    MU_MENUFORM(12,N,"%s",buf);
    MU_MENUFORM(13,N,"%s",nuser.lasthost);
    MU_MENUFORM(14,N,"%u",nuser.numlogins);
    MU_MENUFORM(15,N,"%u",nuser.numposts);
    if(nuser.stay>3600)
        snprintf(buf,MU_LENGTH,"%ld 小时 %2ld 分钟",(nuser.stay/3600),((nuser.stay%3600)/60));
    else
        snprintf(buf,MU_LENGTH,"%ld 分钟",(nuser.stay/60));
    MU_MENUFORM(16,N,"%s",buf);
#ifndef NEWSMTH
    MU_MENUFORM(17,N,"%s",invalid);
#else /* NEWSMTH */
    snprintf(buf,MU_LENGTH,((nuser.score_user>publicshm->us_sample[1])?"%d <RANKING %.2lf%%>":
        "%d <RANKING %.1lf%%>"),nuser.score_user,(100*us_ranking(nuser.score_user)));
    MU_MENUFORM(17,N,"%s",buf);
#endif /* ! NEWSMTH */
    MU_MENUFORM(18,N,"<%s>",gen_permstr(nuser.userlevel,buf));
    memcpy(omenu,menu,(MU_ITEM*MU_LENGTH*sizeof(char)));
    arg.type=MU_MENU_INIT;
    arg.access=&access;
    arg.item=item;
    memset(&conf,0,sizeof(struct _select_def));
    conf.item_count=MU_ITEM;
    conf.item_per_page=MU_ITEM;
    conf.prompt="◆";
    conf.item_pos=loc;
    conf.arg=&arg;
    conf.title_pos.x=-1;
    conf.title_pos.y=-1;
    conf.on_select=mu_menu_on_select;
    conf.show_data=mu_menu_show;
    conf.pre_key_command=mu_menu_pre_key;
    conf.key_command=mu_menu_key;
    conf.on_selchange=mu_menu_on_change;
    loop=1;
    pos=MU_ITEM;
    mail=0;
    change=0;
    do{
        move(1,0);
        clrtobot();
        conf.pos=pos;
        conf.flag=LF_LOOP;
        if(change)
            MU_MENUFORM((MU_ITEM-1),R,"%s","<已修改>");
        else
            MU_MENUFORM((MU_ITEM-1),N,"%s","<未修改>");
        mu_show_help(conf.pos-1);
        list_select_loop(&conf);
        pos=conf.pos;
        if(arg.type==MU_MENU_QUIT){
            if(change){
                MU_GET((MU_ITEM+2),MU_MSG(Y,"放弃修改并退出? [N]: "),buf,1);
                if(toupper(buf[0])!='Y')
                    continue;
            }
            return -1;
        }
        else if(arg.type==MU_MENU_SELECT){
            switch((i=(pos-1))){
#define MU_CURR_ROW                     (i+2)
                case 0:
                    MU_SHOW_HINT(i);
                    if(!strcmp(nuser.userid,getCurrentUser()->userid)){
                        MU_PUT(MU_CURR_ROW,MU_MSG(C,"无法修改当前登录的用户名..."));
                        break;
                    }
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,"请输入新的用户名: "),buf,IDLEN);
                    trimstr(buf);
                    if(!buf[0]||!strcmp(buf,nuser.userid))
                        break;
                    if(id_invalid(buf)){
                        MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的用户名不合法..."));
                        break;
                    }
                    if(strcasecmp(buf,ouser.userid)&&searchuser(buf)){
                        MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的用户名已经存在..."));
                        break;
                    }
                case 1:
                    if(!mode){
                        MU_GETPWD(MU_CURR_ROW,MU_MSG(Y,"请输入原密码: "),&buf[40],38);
                        if(!buf[40])
                            break;
                        if(!checkpasswd2(&buf[40],&ouser)){
                            MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的密码不正确..."));
                            break;
                        }
                    }
                    j=0;
                    while(!j){
                        MU_GETPWD(MU_CURR_ROW,MU_MSG(Y,"请输入新的密码: "),&buf[40],38);
                        if(!buf[40]){
                            j++;
                            continue;
                        }
                        MU_GETPWD(MU_CURR_ROW,MU_MSG(Y,"请重复输入新的密码: "),&buf[80],38);
                        if(!buf[80]){
                            j++;
                            continue;
                        }
                        if(strcmp(&buf[40],&buf[80])){
                            MU_PUT(MU_CURR_ROW,MU_MSG(R,"两次输入的密码不匹配, 请重新输入..."));
                            continue;
                        }
                        if(!mode&&(strlen(&buf[40])<6)){
                            MU_PUT(MU_CURR_ROW,MU_MSG(C,"新的密码过于简短, 请至少设置 6 位..."));
                            continue;
                        }
                        break;
                    }
                    if(j)
                        break;
                    if(!i){
                        snprintf(nuser.userid,(IDLEN+2),"%s",buf);
                        snprintf(ndata.userid,(IDLEN+2),"%s",buf);
                        snprintf(buf,80,"%s \033[0;33m[将会踢出该用户全部登录]\033[m",nuser.userid);
                        MU_SET(0,user,userid,str,"%s",buf);
                    }
                    setpasswd(&buf[80],&nuser);
                    snprintf(buf,MU_LENGTH,"%s",md5_mask);
                    MU_SET(1,user,md5passwd,str,"%s",buf);
                    break;
                case 2:
                    MU_SHOW_HINT(i);
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,"请输入新的昵称: "),buf,(NAMELEN-1));
                    if(!buf[0])
                        break;
                    snprintf(nuser.username,NAMELEN,"%s",buf);
                    MU_SET(i,user,username,str,"%s",0);
                    break;
                case 3:
                    MU_SHOW_HINT(i);
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,"请输入新的真实姓名: "),buf,(NAMELEN-1));
                    trimstr(buf);
                    if(!buf[0])
                        break;
                    snprintf(ndata.realname,NAMELEN,"%s",buf);
                    MU_SET(i,data,realname,str,"%s",0);
                    break;
#ifdef HAVE_BIRTHDAY
                case 4:
                    ndata.gender=(ndata.gender=='M'?'F':'M');
                    snprintf(buf,MU_LENGTH,"%s",(ndata.gender=='M'?"男":"女"));
                    MU_SET(i,data,gender,val,"%s",buf);
                    break;
                case 5:
                    /* 就这里事多! 等我有空非把这三个 birth 捏一起不可... */
#define MU_PARSE2(p)                    ((((p)[0]*10)+((p)[1]*1))-('0'*11))
#define MU_PARSE4(p)                    ((MU_PARSE2(p)*100)+(MU_PARSE2(&(p)[2])*1))
#define MU_MON_CORR(m)                  (((abs(abs(((m)<<1)-15)-5))>>1)&0x01)
#define MU_FEB_CORR(y)                  (2-((!(y%4)?1:0)-(!(y%100)?1:0)))
                    MU_SHOW_HINT(i);
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,"请输入新的出生日期(YYYYMMDD): "),buf,8);
                    trimstr(buf);
                    if(!buf[0])
                        break;
                    if(!mu_digit_string(buf)){
                        MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的出生日期不符合预定格式..."));
                        break;
                    }
                    j=MU_PARSE4(buf);
                    buf[0]=MU_PARSE2(&buf[4]);
                    buf[1]=MU_PARSE2(&buf[6]);
                    if((j<1910||j>2010)||(buf[0]<1||buf[0]>12)||(buf[1]<1
                        ||buf[1]>(31-(MU_MON_CORR(buf[0])+(buf[0]==2?MU_FEB_CORR(j):0))))){
                        MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的出生日期不合法..."));
                        break;
                    }
                    ndata.birthyear=(j-1900);
                    ndata.birthmonth=buf[0];
                    ndata.birthday=buf[1];
                    snprintf(buf,MU_LENGTH,"%04d 年 %02d 月 %02d 日",(ndata.birthyear+1900),
                        ndata.birthmonth,ndata.birthday);
                    MU_SET(i,data,birthyear,val,"%s",buf);
                    if(change&(1<<5))
                        break;
                    MU_SET(i,data,birthmonth,val,"%s",buf);
                    if(change&(1<<5))
                        break;
                    MU_SET(i,data,birthday,val,"%s",buf);
#undef MU_PARSE2
#undef MU_PARSE4
#undef MU_MON_CORR
#undef MU_FEB_CORR
                    break;
#endif /* HAVE_BIRTHDAY */
                case 6:
                    MU_SHOW_HINT(i);
                    MU_PUT(MU_CURR_ROW,MU_MSG(Y,"请输入新的通信地址..."));
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,": "),buf,64);
                    trimstr(buf);
                    if(!buf[0])
                        break;
                    snprintf(ndata.address,STRLEN,"%s",buf);
                    MU_SET(i,data,address,str,"%s",0);
                    break;
                case 7:
                    MU_SHOW_HINT(i);
                    MU_PUT(MU_CURR_ROW,MU_MSG(Y,"请输入新的电子邮件地址..."));
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,": "),buf,64);
                    trimstr(buf);
                    if(!buf[0])
                        break;
                    snprintf(ndata.email,STRLEN,"%s",buf);
                    MU_SET(i,data,email,str,"%s",0);
                    break;
                case 8:
                    MU_SHOW_HINT(i);
                    MU_PUT(MU_CURR_ROW,MU_MSG(Y,"请输入新的联系电话..."));
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,": "),buf,64);
                    trimstr(buf);
                    if(!buf[0])
                        break;
                    snprintf(ndata.telephone,STRLEN,"%s",buf);
                    MU_SET(i,data,telephone,str,"%s",0);
                    break;
                case 9:
                    MU_SHOW_HINT(i);
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,"请输入新的职务{<名称>|<#序号>|<@>}: "),buf,(USER_TITLE_LEN-1));
                    trimstr(buf);
                    if(!buf[0])
                        break;
                    if(buf[0]=='@'&&!buf[1]){
                        j=select_user_title(NULL);
                        if(j==-1)
                            break;
                    }
                    else if(buf[0]=='#'){
                        if(!mu_digit_string(&buf[1])||(j=atoi(&buf[1]))>255){
                            MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的职务序号不合法..."));
                            break;
                        }
                        if(j&&!*get_user_title(j)){
                            MU_GET(MU_CURR_ROW,MU_MSG(Y,"当前序号对应的职务不存在, 是否继续? [N]: "),buf,1);
                            if(toupper(buf[0])!='Y')
                                break;
                        }
                    }
                    else{
                        for(uc=0,uf=0,j=1;j<256;j++)
                            if(!strcmp(get_user_title(j),buf)&&!uc++)
                                uf=j;
                        if(!uc){
                            MU_PUT(MU_CURR_ROW,MU_MSG(C,"当前输入的职务名称尚未定义..."));
                            break;
                        }
                        j=(uc==1?uf:select_user_title(buf));
                        if(j==-1)
                            break;
                    }
                    nuser.title=j;
                    snprintf(buf,MU_LENGTH,"[%s] <%u>",(nuser.title?get_user_title(nuser.title):"无"),nuser.title);
                    MU_SET(i,user,title,val,"%s",buf);
                    break;
                case 10:
                    MU_SHOW_HINT(i);
                    MU_PUT(MU_CURR_ROW,MU_MSG(Y,"请输入新的原始注册资料..."));
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,": "),buf,60);
                    trimstr(buf);
                    if(!buf[0])
                        break;
                    snprintf(ndata.realemail,(STRLEN-16),"%s",buf);
                    MU_SET(i,data,realemail,str,"%s",0);
                    break;
                case 11:
                    MU_SHOW_HINT(i);
                    current=time(NULL);
                    if(!(nuser.firstlogin)){
                        MU_GET(MU_CURR_ROW,MU_MSG(Y,"该用户注册时间错误, 是否重新设置? [Y]: "),buf,1);
                        if(toupper(buf[0])=='N')
                            break;
                    }
                    else if((nuser.userlevel&PERM_LOGINOK)||(current-nuser.firstlogin)>REGISTER_WAIT_TIME){
                        MU_PUT(MU_CURR_ROW,MU_MSG(C,"该用户无重新设置注册时间的必要..."));
                        break;
                    }
                    else{
                        MU_GET(MU_CURR_ROW,MU_MSG(Y,"是否重新设置该用户的注册时间(以便其填写注册单)? [N]: "),buf,1);
                        if(toupper(buf[0])!='Y')
                            break;
                    }
                    nuser.firstlogin=(current-REGISTER_WAIT_TIME);
                    MU_GET_TIME(nuser.firstlogin);
                    MU_SET(i,user,firstlogin,val,"%s",buf);
                    break;
                case 12:
                    MU_SHOW_HINT(i);
                    current=time(NULL);
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,"是否重新设置该用户的最近访问时间? [N]: "),buf,1);
                    if(toupper(buf[0])!='Y')
                        break;
                    nuser.lastlogin=current;
                    MU_GET_TIME(nuser.lastlogin);
                    MU_SET(i,user,lastlogin,val,"%s",buf);
                    break;
                case 13:
                    mu_show_online(uid,mode);
                    break;
                case 14:
                    MU_SHOW_HINT(i);
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,"请输入新的登录数量{<N>|<+N>|<-N>}: "),buf,9);
                    trimstr(buf);
                    if(!buf[0])
                        break;
                    if(buf[0]=='+'||buf[0]=='-'){
                        if(!mu_digit_string(&buf[1])){
                            MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的数字形式不合法..."));
                            break;
                        }
                        j=atoi(buf);
                        if((nuser.numlogins+j)<0)
                            nuser.numlogins=0;
                        nuser.numlogins+=j;
                    }
                    else{
                        if(!mu_digit_string(buf)){
                            MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的数字形式不合法..."));
                            break;
                        }
                        nuser.numlogins=atoi(buf);
                    }
                    MU_SET(i,user,numlogins,val,"%u",0);
                    break;
                case 15:
                    MU_SHOW_HINT(i);
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,"请输入新的文章数量{<N>|<+N>|<-N>}: "),buf,9);
                    trimstr(buf);
                    if(!buf[0])
                        break;
                    if(buf[0]=='+'||buf[0]=='-'){
                        if(!mu_digit_string(&buf[1])){
                            MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的数字形式不合法..."));
                            break;
                        }
                        j=atoi(buf);
                        if((nuser.numposts+j)<0)
                            nuser.numposts=0;
                        nuser.numposts+=j;
                    }
                    else{
                        if(!mu_digit_string(buf)){
                            MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的数字形式不合法..."));
                            break;
                        }
                        nuser.numposts=atoi(buf);
                    }
                    MU_SET(i,user,numposts,val,"%u",0);
                    break;
                case 16:
                    current=time(NULL);
                    j=(int)((((double)nuser.stay)/((double)(current-nuser.firstlogin)))*1440);
                    if(j<60)
                        snprintf(buf,MU_LENGTH,"\033[1;33m日均在线时间为 \033[1;31m%d\033[1;33m 分钟...\033[m",j);
                    else
                        snprintf(buf,MU_LENGTH,"\033[1;33m日均在线时间为 \033[1;31m%d\033[1;33m 小时 "
                            "\033[1;31m%2d\033[1;33m 分钟...\033[m",(j/60),(j%60));
                    MU_PUT(MU_CURR_ROW,buf);
                    break;
#ifdef NEWSMTH
                case 17:
                    MU_SHOW_HINT(i);
                    MU_GET(MU_CURR_ROW,MU_MSG(Y,"请输入新的用户积分数量{<N>|<+N>|<-N>}: "),buf,9);
                    trimstr(buf);
                    if(!buf[0])
                        break;
                    if(buf[0]=='+'||buf[0]=='-'){
                        if(!mu_digit_string(&buf[1])){
                            MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的数字形式不合法..."));
                            break;
                        }
                        j=atoi(buf);
                        if((nuser.score_user+j)<0)
                            nuser.score_user=0;
                        nuser.score_user+=j;
                    }
                    else{
                        if(!mu_digit_string(buf)){
                            MU_PUT(MU_CURR_ROW,MU_MSG(C,"输入的数字形式不合法..."));
                            break;
                        }
                        nuser.score_user=atoi(buf);
                    }
                    snprintf(buf,MU_LENGTH,((nuser.score_user>publicshm->us_sample[1])?"%d <RANKING %.2lf%%>":
                        "%d <RANKING %.1lf%%>"),nuser.score_user,(100*us_ranking(nuser.score_user)));
                    MU_SET(i,user,score_user,val,"%s",buf);
                    break;
#endif /* NEWSMTH */
                case 18:
                    MU_SHOW_HINT(i);
                    if(mu_generate_level(MU_CURR_ROW,15,&level,nuser.userlevel)==-1)
                        break;
                    if(level!=ouser.userlevel&&level!=nuser.userlevel){
                        MU_GET(MU_CURR_ROW,MU_MSG(Y,"是否向该用户发送权限修改通知? [Y]: "),buf,1);
                        mail=(toupper(buf[0])!='N');
                    }
                    nuser.userlevel=level;
                    buf[0]='<';
                    gen_permstr(nuser.userlevel,&buf[1]);
                    snprintf(&buf[NUMPERMS+1],(MU_LENGTH-(NUMPERMS+1)),"> \033[0;33m[%s通知]\033[m",(mail?"启用":"禁用"));
                    MU_SET(i,user,userlevel,val,"%s",buf);
                    break;
#undef MU_CURR_ROW
                case (MU_ITEM-1):
                    if(!change)
                        return 0;
                    MU_GET((MU_ITEM+2),MU_MSG(Y,"执行修改并退出? [N]: "),buf,1);
                    loop=(!(toupper(buf[0])=='Y'));
                    continue;
                default:
                    break;
            }
        }
        else if(arg.type==MU_MENU_RESET){
            switch((i=(pos-1))){
                case 0:
                case 1:
                    MU_RESET(i,user,userid);
                    MU_RESET(i,user,md5passwd);
                    break;
                case 2:
                    MU_RESET(i,user,username);
                    break;
                case 3:
                    MU_RESET(i,data,realname);
                    break;
#ifdef HAVE_BIRTHDAY
                case 4:
                    MU_RESET(i,data,gender);
                case 5:
                    MU_RESET(i,data,birthyear);
                    MU_RESET(i,data,birthmonth);
                    MU_RESET(i,data,birthday);
                    break;
#endif /* HAVE_BIRTHDAY */
                case 6:
                    MU_RESET(i,data,address);
                    break;
                case 7:
                    MU_RESET(i,data,email);
                    break;
                case 8:
                    MU_RESET(i,data,telephone);
                    break;
                case 9:
                    MU_RESET(i,user,title);
                    break;
                case 10:
                    MU_RESET(i,data,realemail);
                    break;
                case 11:
                    MU_RESET(i,user,firstlogin);
                    break;
                case 12:
                    MU_RESET(i,user,lastlogin);
                    break;
                case 13:
                    MU_RESET(i,user,lasthost);
                    break;
                case 14:
                    MU_RESET(i,user,numlogins);
                    break;
                case 15:
                    MU_RESET(i,user,numposts);
                    break;
                case 16:
                    MU_RESET(i,user,stay);
                    break;
#ifdef NEWSMTH
                case 17:
                    MU_RESET(i,user,score_user);
                    break;
#endif /* NEWSMTH */
                case 18:
                    MU_RESET(i,user,userlevel);
                    break;
                case (MU_ITEM-1):
                    if(change){
                        memcpy(&nuser,&ouser,sizeof(struct userec));
                        memcpy(&ndata,&odata,sizeof(struct userdata));
                        memcpy(menu,omenu,(MU_ITEM*MU_LENGTH*sizeof(char)));
                        change=0;
                    }
                    break;
                default:
                    break;
            }
        }
        else
            continue;
    }
    while(loop);
    memcpy(&vuser,urec,sizeof(struct userec));
    if(read_userdata(urec->userid,&vdata)==-1){
        MU_PUT((MU_ITEM+2),MU_MSG(R,"校验用户数据时发生致命错误..."));
        return -6;
    }
    verify=0;
    MU_VERIFY(0,user,userid,str);
    MU_VERIFY(1,user,md5passwd,str);
    MU_VERIFY(2,user,username,str);
    MU_VERIFY(3,data,realname,str);
#ifdef HAVE_BIRTHDAY
    MU_VERIFY(4,data,gender,val);
    MU_VERIFY(5,data,birthyear,val);
    MU_VERIFY(5,data,birthmonth,val);
    MU_VERIFY(5,data,birthday,val);
#endif /* HAVE_BIRTHDAY */
    MU_VERIFY(6,data,address,str);
    MU_VERIFY(7,data,email,str);
    MU_VERIFY(8,data,telephone,str);
    MU_VERIFY(9,user,title,val);
    MU_VERIFY(10,data,realemail,str);
    MU_VERIFY(11,user,firstlogin,val);
    MU_VERIFY(12,user,lastlogin,val);
    MU_VERIFY(13,user,lasthost,str);
    MU_VERIFY(14,user,numlogins,val);
    MU_VERIFY(15,user,numposts,val);
    MU_VERIFY(16,user,stay,val);
#ifdef NEWSMTH
    MU_VERIFY(17,user,score_user,val);
#endif /* NEWSMTH */
    MU_VERIFY(18,user,userlevel,val);
    if(verify){
        MU_GET((MU_ITEM+2),MU_MSG(Y,"部分用户数据已经发生变化, 是否强制修改? [N]: "),buf,1);
        if(toupper(buf[0])!='Y'){
            MU_PUT((MU_ITEM+2),MU_MSG(Y,"取消修改操作..."));
            return -7;
        }
        if(verify&0x01)
            change&=(~0x03);
    }
    memcpy(&vuser,urec,sizeof(struct userec));
    if(read_userdata(urec->userid,&vdata)==-1){
        MU_PUT((MU_ITEM+2),MU_MSG(R,"修改用户数据时发生致命错误..."));
        return -8;
    }
    MU_EXEC(2,user,username);
    MU_EXEC(3,data,realname);
#ifdef HAVE_BIRTHDAY
    MU_EXEC(4,data,gender);
    MU_EXEC(5,data,birthyear);
    MU_EXEC(5,data,birthmonth);
    MU_EXEC(5,data,birthday);
#endif /* HAVE_BIRTHDAY */
    MU_EXEC(6,data,address);
    MU_EXEC(7,data,email);
    MU_EXEC(8,data,telephone);
    MU_EXEC(9,user,title);
    MU_EXEC(10,data,realemail);
    MU_EXEC(11,user,firstlogin);
    MU_EXEC(12,user,lastlogin);
    MU_EXEC(13,user,lasthost);
    MU_EXEC(14,user,numlogins);
    MU_EXEC(15,user,numposts);
    MU_EXEC(16,user,stay);
#ifdef NEWSMTH
    MU_EXEC(17,user,score_user);
#endif /* NEWSMTH */
    MU_EXEC(18,user,userlevel);
    i=(verify&0x01);
    do{
        if(change&0x01){
            if(strcasecmp(vuser.userid,nuser.userid)&&searchuser(nuser.userid)){
                i=2;
                break;
            }
            for(j=0;(k=kick_user_utmp(uid,NULL,0))==10;j+=k)
                continue;
            if(j+=k)
                newbbslog(BBSLOG_USER,"modify_userinfo_kick: %s <%d> [ %d logins ]",urec->userid,uid,j);
            setuserid(uid,nuser.userid);
            if(searchuser(nuser.userid)!=uid){
                i=3;
                break;
            }
            if(mu_process_directories(vuser.userid,nuser.userid)==-1){
                i=4;
                break;
            }
            MU_EXEC(0,user,userid);
            MU_EXEC(0,data,userid);
        }
    }
    while(0);
    if(!i)
        MU_EXEC(1,user,md5passwd);
    else
        change&=(~0x03);
    memcpy(urec,&vuser,sizeof(struct userec));
    if(write_userdata(urec->userid,&vdata)==-1){
        MU_PUT((MU_ITEM+2),MU_MSG(R,"回写用户数据时发生致命错误..."));
        return -9;
    }
    j=read_user_memo(urec->userid,&memo);
    if(!(j<0)&&(j!=sizeof(struct usermemo)))
        end_mmapfile(memo,sizeof(struct usermemo),-1);
    if(j==sizeof(struct usermemo)){
        memcpy(&(memo->ud),&vdata,sizeof(struct userdata));
        end_mmapfile(memo,sizeof(struct usermemo),-1);
    }
    else{
        sethomefile(buf,urec->userid,"usermemo");
        unlink(buf);
    }
    if(mail&&(change&(1<<18))){
        snprintf(name,MU_LENGTH,"tmp/modify_userinfo_%lu_%d.mail",time(NULL),(int)getpid());
        if((fp=fopen(name,"w"))){
            write_header(fp,getCurrentUser(),1,NULL,"[系统] 用户权限修改通知",0,0,getSession());
            fprintf(fp,"\033[1;37m[%s]\033[m\n\n","用户权限修改明细");
            fprintf(fp,"  \033[1;33m原用户权限状态: \033[0;33m<%s>\033[m\n",gen_permstr(ouser.userlevel,buf));
            fprintf(fp,"  \033[1;33m现用户权限状态: \033[1;32m<%s>\033[m\n\n",gen_permstr(vuser.userlevel,buf));
            fprintf(fp,"\033[1;37m[%s]\033[m\n\n","涉及修改的权限位说明");
            for(level=(ouser.userlevel^vuser.userlevel),j=0;j<NUMPERMS;j++)
                if(level&(1<<j))
                    fprintf(fp,"  %s<%c> \033[1;33m%s\033[m\n",((vuser.userlevel&(1<<j))?"\033[1;32m+":"\033[1;31m-"),
                        XPERMSTR[j],permstrings[j]);
            fprintf(fp,"\n\033[1;37m[%s]\033[m\n","如有异常情况请及时回复本信联系站务人员设法解决!");
            fclose(fp);
            mail_file(getCurrentUser()->userid,name,urec->userid,"[系统] 用户权限修改通知",BBSPOST_MOVE,NULL);
            unlink(name);
        }
    }
    if(mode){
        snprintf(name,MU_LENGTH,"tmp/modify_userinfo_%lu_%d.log",time(NULL),(int)getpid());
        if(!(fp=fopen(name,"w"))){
            snprintf(buf,MU_LENGTH,"设定用户数据: <%d,%#6.6x> %s%c-> %s",uid,change,ouser.userid,((change&0x01)?' ':0),nuser.userid);
            securityreport(buf,NULL,NULL,getSession());
        }
        else{
            sprintf(buf,"设定用户数据: %s%c-> %s",ouser.userid,((change&0x01)?' ':0),nuser.userid);
            write_header(fp,getCurrentUser(),0,"syssecurity",buf,0,0,getSession());
            fprintf(fp,"\033[1;37m[用户 <uid=\033[1;31m%d\033[1;37m> 数据修改明细]\033[m\n\n",uid);
            for(j=0;j<(MU_ITEM-1);j++){
                if(change&(1<<j))
                    fprintf(fp," \033[1;33m[%-8.8s]: \033[0;33m%s\033[m\n%-13.13s\033[1;32m%s\033[m\n\n",prefix[j],omenu[j],"",menu[j]);
            }
            fclose(fp);
            post_file(getCurrentUser(),"",name,"syssecurity",buf,0,3,getSession());
            unlink(name);
        }
        newbbslog(BBSLOG_USER,"modify_userinfo: %s <%d,%#6.6x>",urec->userid,uid,change);
    }
    switch(i){
        case 1:
            MU_PUT((MU_ITEM+2),MU_MSG(Y,"操作完成! 已取消名称和密码的修改(用户数据变化)..."));
            return 2;
        case 2:
            MU_PUT((MU_ITEM+2),MU_MSG(Y,"操作完成! 已取消名称和密码的修改(已存在同名用户)..."));
            return 3;
        case 3:
            MU_PUT((MU_ITEM+2),MU_MSG(R,"操作完成! 已取消名称和密码的修改(系统错误)..."));
            return 4;
        case 4:
            MU_PUT((MU_ITEM+2),MU_MSG(R,"操作完成! 已取消名称和密码的修改(文件错误)..."));
            return 5;
        case 0:
            MU_PUT((MU_ITEM+2),MU_MSG(G,"操作完成! 所有操作均已成功执行!"));
            break;
        default:
            MU_PUT((MU_ITEM+2),MU_MSG(Y,"操作完成!"));
            return 6;
    }
    return 1;
#undef MU_ACCESS_USER
#undef MU_ACCESS_ADMIN
#undef MU_ACCESS_READ
#undef MU_ACCESS
#undef MU_SIZE
#undef MU_SIZE_user
#undef MU_SIZE_data
#undef MU_DIFF_str
#undef MU_DIFF_val
#undef MU_MENUFORM
#undef MU_CAST_0
#undef MU_CAST_buf
#undef MU_SET
#undef MU_RESET
#undef MU_SHOW_HINT
#undef MU_GET_TIME
#undef MU_VERIFY
#undef MU_EXEC
}

int modify_userinfo_current(void){
    return modify_userinfo(uinfo.uid,0);
}

int modify_userinfo_admin(void){
#ifdef HAVE_STRICT_USERINFO
#define MUA_PERM    (PERM_ADMIN)
#else /* HAVE_STRICT_USERINFO */
#define MUA_PERM    (PERM_SYSOP|PERM_ADMIN)
#endif /* HAVE_STRICT_USERINFO */
    char name[STRLEN];
    int uid,mode;
    clear();
#ifndef SOURCE_PERM_CHECK
    mode=1;
#else /* SOURCE_PERM_CHECK */
    if(!HAS_PERM(getCurrentUser(),MUA_PERM)){
        MU_PUT(2,MU_MSG(R,"当前用户不具有查阅用户资料的权限..."));
        return -0xFF;
    }
    mode=(HAS_PERM(getCurrentUser(),PERM_ADMIN)?1:2);
#endif /* ! SOURCE_PERM_CHECK */
    if(!check_systempasswd())
        return -0xFE;
    clear();
    move(0,0);
    prints("\033[1;32m[%s用户资料]\033[m",((mode==1)?"修改":"查阅"));
    move(1,0);
    usercomplete("请输入用户名: ",name);
    move(1,0);
    clrtobot();
    if(!name[0]){
        MU_PUT(2,MU_MSG(Y,"取消..."));
        return -0xFD;
    }
    if(!(uid=searchuser(name))){
        MU_PUT(2,MU_MSG(R,"错误的用户名..."));
        return -0xFC;
    }
    return modify_userinfo(uid,mode);
#undef MUA_PERM
}

#undef MU_LENGTH
#undef MU_ITEM
#undef MU_KEY_RESET
#undef MU_GEN_N
#undef MU_GEN_R
#undef MU_GEN_Y
#undef MU_GEN_G
#undef MU_GEN_C
#undef MU_MSG
#undef MU_PUT
#undef MU_GET_CORE
#undef MU_GET
#undef MU_GETPWD

/* END -- etnlegend, 2006.09.21, 修改用户资料接口... */

