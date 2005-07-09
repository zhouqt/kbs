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
char *sysconf_str();

void disply_userinfo(u, real)
	struct userec *u;
    int real;
{
    struct stat st;
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
#ifdef HAVE_CUSTOM_USER_TITLE
    prints("您的职务: %s\n",get_user_title(u->title));
#endif

    if (real) {
        prints("原始注册资料 : %s\n", ud.realemail);
    }
    prints("注册日期     : %s", ctime(&u->firstlogin));
    prints("最近光临日期 : %s", ctime(&u->lastlogin));
    if (real) {
        prints("最近光临机器 : %s\n", u->lasthost);
    }

    /*---	added by period		hide posts/logins	2000-11-02	---*/
/*    if(HAS_PERM(getCurrentUser(),PERM_ADMINMENU)) { *//* removed to let user can see his own data */
        prints("上站次数     : %d 次\n", u->numlogins);
    if (real)
        prints("文章数目     : %d 篇\n", u->numposts);

        /*       if( real ) {
           prints("文章数目     : %d / %d (Board/1Discuss)\n",
           u->numposts, post_in_tin( u->userid ));
           }  removed by stephen 2000-11-02 */
        /*    } */
#ifdef NINE_BUILD
        {
           int exp;
           exp=countexp(u);
           prints("经验值       : %d(%s)\n",exp,c_exp(exp));
           exp=countperf(u);
           prints("表现值       : %d(%s)\n",exp,c_perf(exp));
        }
#endif
        prints("上站总时数   : %d 小时 %d 分钟\n", u->stay / 3600, (u->stay / 60) % 60);
    setmailfile(genbuf, u->userid, DOT_DIR);
    if (stat(genbuf, &st) >= 0)
        num = st.st_size / (sizeof(struct fileheader));
    
    else
        num = 0;
    prints("私人信箱     : %d 封\n", num);
    if (real) {
        strcpy(genbuf, "bTCPRp#@XWBA$VS!DEM1234567890%");
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
    char ans[3], buf[STRLEN], *emailfile, genbuf[STRLEN];
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
#ifdef HAVE_CUSTOM_USER_TITLE
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
#endif
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
                update_user(&newinfo, usernum, 1);
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
			end_mmapfile(um, sizeof(struct usermemo), -1);
			write_userdata(newinfo.userid, &ud);
            if (real)
                 {
                char secu[STRLEN];

                if (strcmp(u->userid, newinfo.userid))
                    sprintf(secu, "%s 的 ID 被 %s 改为 %s", u->userid, getCurrentUser()->userid, newinfo.userid);   /*Haohmaru.99.5.6 */
                
                else
                    sprintf(secu, "修改 %s 的基本资料或密码。", u->userid);
                securityreport(secu, &newinfo, NULL);
            }
            break;             /*Haohmaru.98.01.10.faint...Luzi加个for循环也不break! */
        }
	} /* for(;;) */
    clear();
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
    uinfo_query(getCurrentUser(), 0, usernum);
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
        fprintf(fn, "usernum: %d, %s", usernum, ctime(&now));
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

