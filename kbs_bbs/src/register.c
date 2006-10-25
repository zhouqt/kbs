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

/*
    checked Global variable
*/
#include "bbs.h"

#define  EMAIL          0x0001
#define  NICK           0x0002
#define  REALNAME       0x0004
#define  ADDR           0x0008
#define  REALEMAIL      0x0010
#define  BADEMAIL       0x0020
#define  NEWREG         0x0040

char *Ctime();

extern struct user_info uinfo;
extern time_t login_start_time;
extern int convcode;            /* KCN,99.09.05 */

void new_register()
{
    struct userec newuser;
    int allocid, do_try, flag, usernum;
    char buf[STRLEN];

/* temp !!!!!*/
/*    prints("Sorry, we don't accept newusers due to system problem, we'll fixit ASAP\n");
    oflush();
    sleep(2);
    exit(-1);
*/
    memset(&newuser, 0, sizeof(newuser));
    getdata(0, 0, "使用GB编码阅读?(\xa8\xcf\xa5\xce BIG5\xbd\x58\xbe\x5c\xc5\xaa\xbd\xd0\xbf\xefN)(Y/N)? [Y]: ", buf, 4, DOECHO, NULL, true);
    if (*buf == 'n' || *buf == 'N')
        if (!convcode)
            switch_code();

    ansimore("etc/register", false);
    do_try = 0;
    while (1) {
        if (++do_try >= 10) {
            prints("\n掰掰，按太多下  <Enter> 了...\n");
            refresh();
            longjmp(byebye, -1);
        }
        getdata(0, 0, "请输入代号: ", newuser.userid, IDLEN + 1, DOECHO, NULL, true);
        flag = 1;
        if (id_invalid(newuser.userid) == 1) {
            prints("帐号必须由英文字母或数字组成，并且第一个字符必须是英文字母!\n");
            /*                prints("帐号必须由英文字母或数字，而且帐号第一个字是英文字母!\n"); */
            flag = 0;
        }
        if (flag) {
            if (strlen(newuser.userid) < 2) {
                prints("代号至少需有两个英文字母!\n");
            } else if ((*newuser.userid == '\0') || bad_user_id(newuser.userid) || (strcasecmp(newuser.userid, "SYSOPS") == 0) || (strcasecmp(newuser.userid, "BMS") == 0)) {
                prints("抱歉，本站暂不提供此帐号注册。\n");
            } else if ((usernum = searchuser(newuser.userid)) != 0) {   /*( dosearchuser( newuser.userid ) ) midified by dong , 1998.12.2, change getuser -> searchuser , 1999.10.26 */
                prints("此帐号已经有人使用\n");
            } else {
                /*---	---*/
                struct stat lst;
                time_t lnow;

                lnow = time(NULL);
                sethomepath(genbuf, newuser.userid);
                if (!stat(genbuf, &lst) && S_ISDIR(lst.st_mode)
                    && (lnow - lst.st_ctime < SEC_DELETED_OLDHOME /* 3600*24*30 */ )) {
                    prints("目前无法注册帐号%s，请与系统管理人员联系。\n", newuser.userid);
                    sprintf(genbuf, "IP %s new id %s failed[home changed in past 30 days]", getSession()->fromhost, newuser.userid);
                    bbslog("user","%s",genbuf);
                }
                else{
                    /* etnlegend, 2006.10.14, 新用户可能继承原有同名用户信件... */
                    sethomepath(genbuf,newuser.userid);
                    my_f_rm(genbuf);
                    setmailpath(genbuf,newuser.userid);
                    my_f_rm(genbuf);
                    break;
                }
            }
        }
    }

    newuser.firstlogin = newuser.lastlogin = time(NULL) - 13 * 60 * 24;
    do_try = 0;
    while (1) {
        char passbuf[STRLEN], passbuf2[STRLEN];

        if (++do_try >= 10) {
            prints("\n掰掰，按太多下  <Enter> 了...\n");
            refresh();
            longjmp(byebye, -1);
        }
        getdata(0, 0, "请设定您的密码: ", passbuf, 39, NOECHO, NULL, true);
        if (strlen(passbuf) < 4 || !strcmp(passbuf, newuser.userid)) {
            prints("密码太短或与使用者代号相同, 请重新输入\n");
            continue;
        }
        getdata(0, 0, "请再输入一次你的密码: ", passbuf2, 39, NOECHO, NULL, true);
        if (strcmp(passbuf, passbuf2) != 0) {
            prints("密码输入错误, 请重新输入密码.\n");
            continue;
        }

        setpasswd(passbuf, &newuser);
        break;
    }

    if ( searchuser(newuser.userid) != 0) {   
        prints("此帐号已经有人使用\n");
        refresh();
        longjmp(byebye, -1);
    }

    newuser.userlevel = PERM_BASIC;
    newuser.userdefine[0] = -1;
    newuser.userdefine[1] = -1;
/*   newuser.userdefine&=~DEF_MAILMSG;
    newuser.userdefine&=~DEF_EDITMSG; */
    SET_UNDEFINE(&newuser, DEF_NOTMSGFRIEND);
    if (convcode)
        SET_UNDEFINE(&newuser, DEF_USEGB);

	SET_UNDEFINE(&newuser, DEF_SHOWREALUSERDATA);

    newuser.exittime = time(NULL) - 100;
    /*newuser.unuse2 = -1;*/
    newuser.flags |= PAGER_FLAG;
    newuser.title = 0;
    newuser.firstlogin = newuser.lastlogin = time(NULL);

    allocid = getnewuserid2(newuser.userid);
    if (allocid > MAXUSERS || allocid <= 0) {
        prints("抱歉, 由于某些系统原因, 无法注册新的帐号.\n\r");
        oflush();
        sleep(2);
        exit(1);
    }
    newbbslog(BBSLOG_USIES, "APPLY: %s uid %d from %s", newuser.userid, allocid, getSession()->fromhost);

    update_user(&newuser, allocid, 1);

    if (!dosearchuser(newuser.userid)) {
        /* change by KCN 1999.09.08
           fprintf(stderr,"User failed to create\n") ;
         */
        prints("User failed to create %d-%s\n", allocid, newuser.userid);
        oflush();
        exit(1);
    }
    bbslog("user","%s","new account");
}

/*加入对 #TH 结尾的realemail的帐号自动通过注册的功能  by binxun
*/
int invalid_realmail(userid, email, msize)
    char *userid, *email;
    int msize;
{
    FILE *fn;
    char ans[4], fname[STRLEN];
    char genbuf[STRLEN];

/*
    if ((emailfile = sysconf_str("EMAILFILE")) == NULL)
        return 0;
*/

    if (strchr(email, '@') && valid_ident(email))
        return 0;
    /*
       ansimore( emailfile, false );
       getdata(t_lines-1,0,"您要现在 email-post 吗? (Y/N) [Y]: ",
       ans,2,DOECHO,NULL,true);
       while( *ans != 'n' && *ans != 'N' ) {
     */
    sprintf(fname, "tmp/email/%s", userid);
    if ((fn = fopen(fname, "r")) != NULL) {
        fgets(genbuf, STRLEN, fn);
        fclose(fn);
        strtok(genbuf, "\n");
        if (!valid_ident(genbuf)) {
        } else if (strchr(genbuf, '@') != NULL) {
            unlink(fname);
            strncpy(email, genbuf, msize);
            move(10, 0);
            prints("恭贺您!! 您已通过身份验证, 成为本站公民. \n");
            prints("         本站为您所提供的额外服务, \n");
            prints("         包括Mail,Post,Message,Talk 等. \n");
            prints("  \n");
            prints("建议您,  先四处浏览一下, \n");
            prints("         不懂的地方, 请在 sysop 版留言, \n");
            prints("         本站会派专人为您解答. \n");
            getdata(18, 0, "请按 <Enter>  <<  ", ans, 2, DOECHO, NULL, true);
            return 0;
        }
    }
    return 1;
}

void check_register_info()
{
    const char *newregfile;
    //int perm;
    char buf[STRLEN];
	char career[STRLEN];
	char phone[40];

    clear();
    sprintf(buf, "%s", email_domain());
    if (!(getCurrentUser()->userlevel & PERM_BASIC)) {
        getCurrentUser()->userlevel = PERM_DENYMAIL|PERM_DENYRELAX;
        return;
    }
    /*urec->userlevel |= PERM_DEFAULT; */
    //perm = PERM_DEFAULT & sysconf_eval("AUTOSET_PERM",PERM_DEFAULT);

//    invalid_realmail(getCurrentUser()->userid,curruserdata.realemail,STRLEN - 16);
    invalid_realmail(getCurrentUser()->userid,getSession()->currentmemo->ud.realemail,STRLEN - 16);

    do_after_login(getCurrentUser(),getSession()->utmpent,0);

    /*    if( sysconf_str( "IDENTFILE" ) != NULL ) {  commented out by netty to save time */
    while (strlen(getCurrentUser()->username) < 2) {
        getdata(2, 0, "请输入您的昵称:(例如," DEFAULT_NICK ") << ", buf, NAMELEN, DOECHO, NULL, true);
        strcpy(getCurrentUser()->username, buf);
        strcpy(uinfo.username, buf);
        UPDATE_UTMP_STR(username, uinfo);
    }
//    if (strlen(curruserdata.realname) < 2) {
    if (strlen(getSession()->currentmemo->ud.realname) < 2) {
        move(3, 0);
        prints("请输入您的真实姓名: (站长会帮您保密的 !)\n");
        getdata(4, 0, "> ", buf, NAMELEN, DOECHO, NULL, true);
//        strcpy(curruserdata.realname, buf);
        strcpy(getSession()->currentmemo->ud.realname, buf);
    }
//    if (strlen(curruserdata.address) < 6) {
    if (strlen(getSession()->currentmemo->ud.address) < 6) {
        move(5, 0);
//        prints("您目前填写的地址是‘%s’，长度小于 \033[1m\033[37m6\033[m，系统认为其过于简短。\n", curruserdata.address[0] ? curruserdata.address : "空地址");  /* Leeward 98.04.26 */
        prints("您目前填写的地址是‘%s’，长度小于 \033[1m\033[37m6\033[m，系统认为其过于简短。\n", getSession()->currentmemo->ud.address[0] ? getSession()->currentmemo->ud.address : "空地址");  /* Leeward 98.04.26 */
        getdata(6, 0, "请详细填写您的住址：", buf, NAMELEN, DOECHO, NULL, true);
//        strcpy(curruserdata.address, buf);
        strcpy(getSession()->currentmemo->ud.address, buf);
    }

	/* 加入转让ID后的代码   by binxun 2003-5-23 */
	sethomefile(buf,getCurrentUser()->userid,"conveyID");
	if(dashf(buf))
	{
	
        move(6,0);
		prints("此ID由您的朋友转让给您,恭喜您获得此ID,请填写以下资料.");
		getCurrentUser()->firstlogin = getCurrentUser()->lastlogin; /* atppp 20050312 */
		do{
		    getdata(7,0,"学校系级或单位全称(具体到部门):",career,STRLEN,DOECHO,NULL,true);
		}while(strlen(career) < 4);
        do{
		    getdata(8,0,"您的联系电话或者Email:",phone,40,DOECHO,NULL,true);
		}while(strlen(phone) < 6);

             unlink(buf);   
        sprintf(buf,"%s$%s@转让", career,phone);
		if(strlen(buf) >= STRLEN - 16)sprintf(buf,"%s@转让",phone);
//		strncpy(curruserdata.realemail,buf,STRLEN-16);
		strncpy(getSession()->currentmemo->ud.realemail,buf,STRLEN-16);
//		curruserdata.realemail[STRLEN-16-1]='\0';
		getSession()->currentmemo->ud.realemail[STRLEN-16-1]='\0';
//		write_userdata(getCurrentUser()->userid,&curruserdata);
		write_userdata(getCurrentUser()->userid,&(getSession()->currentmemo->ud));
	}

#ifdef HAVE_BIRTHDAY
	if (!is_valid_date(getSession()->currentmemo->ud.birthyear+1900, 
				getSession()->currentmemo->ud.birthmonth,
				getSession()->currentmemo->ud.birthday))
	{
		time_t now;
		struct tm *tmnow;

		now = time(0);
		tmnow = localtime(&now);
		clear();
		buf[0] = '\0';
		move(0, 0);
		prints("我们检查到你的部分注册资料不够完全，为了更好的为您提供个性化的服务，");
		move(1, 0);
		prints("希望您填写以下资料。");
		while (buf[0] < '1' || buf[0] > '2')
		{
			getdata(2, 0, "请输入你的性别: [1]男的 [2]女的 (1 or 2): ",
					buf, 2, DOECHO, NULL, true);
		}
		switch (buf[0])
		{
		case '1':
			getSession()->currentmemo->ud.gender = 'M';
			break;
		case '2':
			getSession()->currentmemo->ud.gender = 'F';
			break;
		}
		move(4, 0);
		prints("请输入您的出生日期");
		while (getSession()->currentmemo->ud.birthyear < tmnow->tm_year - 98
			   || getSession()->currentmemo->ud.birthyear > tmnow->tm_year - 3)
		{
			buf[0] = '\0';
			getdata(5, 0, "四位数公元年: ", buf, 5, DOECHO, NULL, true);
			if (atoi(buf) < 1900)
				continue;
			getSession()->currentmemo->ud.birthyear = atoi(buf) - 1900;
		}
		while (getSession()->currentmemo->ud.birthmonth < 1 
				|| getSession()->currentmemo->ud.birthmonth > 12)
		{
			buf[0] = '\0';
			getdata(6, 0, "出生月: (1-12) ", buf, 3, DOECHO, NULL, true);
			getSession()->currentmemo->ud.birthmonth = atoi(buf);
		}
		do
		{
			buf[0] = '\0';
			getdata(7, 0, "出生日: (1-31) ", buf, 3, DOECHO, NULL, true);
			getSession()->currentmemo->ud.birthday = atoi(buf);
		} while (!is_valid_date(getSession()->currentmemo->ud.birthyear + 1900,
					getSession()->currentmemo->ud.birthmonth,
					getSession()->currentmemo->ud.birthday));
		write_userdata(getCurrentUser()->userid, &(getSession()->currentmemo->ud));
	}
#endif
#ifdef NEW_COMERS
	if (getCurrentUser()->numlogins == 1)
	{
		FILE *fout;
		char buf2[STRLEN];

		gettmpfilename( buf, "newcomer" );
		//sprintf(buf, "tmp/newcomer.%s", getCurrentUser()->userid);
		if ((fout = fopen(buf, "w")) != NULL)
		{
			fprintf(fout, "大家好,\n\n");
			fprintf(fout, "我是 %s (%s), 来自 %s\n", getCurrentUser()->userid,
					getCurrentUser()->username, SHOW_USERIP(getCurrentUser(), getSession()->fromhost));
			fprintf(fout, "今天%s初来此站报到, 请大家多多指教。\n",
#ifdef HAVE_BIRTHDAY
//					(curruserdata.gender == 'M') ? "小弟" : "小女子");
					(getSession()->currentmemo->ud.gender == 'M') ? "小弟" : "小女子");
#else
                                        "小弟");
#endif
#ifndef NEWSMTH
			move(9, 0);
			prints("请作个简短的个人简介, 向本站其他使用者打个招呼\n");
			prints("(最多三行, 写完可直接按 <Enter> 跳离)....");
			getdata(11, 0, ":", buf2, 75, DOECHO, NULL, true);
			if (buf2[0] != '\0')
			{
				fprintf(fout, "\n\n自我介绍:\n\n");
				fprintf(fout, "%s\n", buf2);
				getdata(12, 0, ":", buf2, 75, DOECHO, NULL, true);
				if (buf2[0] != '\0')
				{
					fprintf(fout, "%s\n", buf2);
					getdata(13, 0, ":", buf2, 75, DOECHO, NULL, true);
					if (buf2[0] != '\0')
					{
						fprintf(fout, "%s\n", buf2);
					}
				}
			}
#endif
			fclose(fout);
			sprintf(buf2, "新手上路: %s", getCurrentUser()->username);
			post_file(getCurrentUser(), "", buf, "newcomers", buf2, 0, 2, getSession());
			unlink(buf);
		}
		pressanykey();
	}
#endif
    if (!strcmp(getCurrentUser()->userid, "SYSOP")) {
        getCurrentUser()->userlevel = ~0;
        getCurrentUser()->userlevel &= ~PERM_SUICIDE;        /* Leeward 98.10.13 */
        getCurrentUser()->userlevel &= ~(PERM_DENYMAIL|PERM_DENYRELAX);       /* Bigman 2000.9.22 */
        getCurrentUser()->userlevel &= ~PERM_JURY;       /* 不能是仲裁 */
    }
    if (!(getCurrentUser()->userlevel & PERM_LOGINOK)) {
        if (HAS_PERM(getCurrentUser(), PERM_SYSOP))
            return;
//        if (!invalid_realmail(getCurrentUser()->userid, curruserdata.realemail, STRLEN - 16)) {
        if (!invalid_realmail(getCurrentUser()->userid, getSession()->currentmemo->ud.realemail, STRLEN - 16)) {
            getCurrentUser()->userlevel |= PERM_DEFAULT;
            /*
            if (HAS_PERM(getCurrentUser(), PERM_DENYPOST) && !HAS_PERM(getCurrentUser(), PERM_SYSOP))
                getCurrentUser()->userlevel &= ~PERM_POST;
            */
        } else {
            /* added by netty to automatically send a mail to new user. */
            /* begin of check if local email-addr  */
            /*       if (
               (!strstr( urec->email, "@bbs.") ) &&
               (!strstr( urec->email, ".bbs@") )&&
               (!invalidaddr(urec->email))&&
               sysconf_str( "EMAILFILE" )!=NULL) 
               {
               move( 15, 0 );
               prints( "您的电子信箱  尚须通过回信验证...  \n" );
               prints( "      SYSOP 将寄一封验证信给您,\n" );
               prints( "      您只要回信, 就可以成为本站合格公民.\n" );
               getdata( 19 ,0, "您要 SYSOP 寄这一封信吗?(Y/N) [Y] << ", ans,2,DOECHO,NULL,true);
               if ( *ans != 'n' && *ans != 'N' ) {
               code=(time(0)/2)+(rand()/10);
               sethomefile(genbuf,urec->userid,"mailcheck");
               if((dp=fopen(genbuf,"w"))==NULL)
               {
               fclose(dp);
               return;
               }
               fprintf(dp,"%9.9d\n",code);
               fclose(dp);
               sprintf( genbuf, "/usr/lib/sendmail -f SYSOP.bbs@%s %s ", 
               email_domain(), urec->email );
               fout = popen( genbuf, "w" );
               fin  = fopen( sysconf_str( "EMAILFILE" ), "r" );
               if ((fin != NULL) && (fout != NULL)) {
               fprintf( fout, "Reply-To: SYSOP.bbs@%s\n", email_domain());
               fprintf( fout, "From: SYSOP.bbs@%s\n",  email_domain() ); 
               fprintf( fout, "To: %s\n", urec->email);
               fprintf( fout, "Subject: @%s@[-%9.9d-]firebird mail check.\n", urec->userid ,code);
               fprintf( fout, "X-Forwarded-By: SYSOP \n" );
               fprintf( fout, "X-Disclaimer: None\n");
               fprintf( fout, "\n");
               fprintf(fout,"您的基本资料如下：\n",urec->userid);
               fprintf(fout,"使用者代号：%s (%s)\n",urec->userid,urec->username);
               fprintf(fout,"姓      名：%s\n",urec->realname);
               fprintf(fout,"上站位置  ：%s\n",urec->lasthost);
               fprintf(fout,"电子邮件  ：%s\n\n",urec->email);
               fprintf(fout,"亲爱的 %s(%s):\n",urec->userid,urec->username);
               while (fgets( genbuf, 255, fin ) != NULL ) {
               if (genbuf[0] == '.' && genbuf[ 1 ] == '\n')
               fputs( ". \n", fout );
               else fputs( genbuf, fout );
               }
               fprintf(fout, ".\n");                                    
               fclose( fin );
               fclose( fout );                                     
               }
               getdata( 20 ,0, "信已寄出, SYSOP 将等您回信哦!! 请按 <Enter> << ", ans,2,DOECHO,NULL ,true);
               }
               }else
               {
               showansi=1;
               if(sysconf_str( "EMAILFILE" )!=NULL)
               {
               prints("\n你的电子邮件地址 【\033[33m%s\033[m】\n",urec->email);
               prints("并非 Unix 帐号，系统不会投递身份确认信，请到\033[32m工具箱\033[m中修改..\n");
               pressanykey();
               }
               }
               deleted by alex, remove email certify */

            clear();            /* Leeward 98.05.14 */
            move(12, 0);
            prints("您现在还没有通过身份认证，没有talk,mail,message,post等权限。\n");
            prints("如果你要成为%s的注册" NAME_USER_SHORT "，", BBS_FULL_NAME);
            prints("请到\033[33;1m个人工具箱\033[m中详细真实地填写\033[32m 注册单,\033[m\n");
            prints("经" NAME_SYSOP_GROUP "审核通过以后，就可以成为本站的注册" NAME_USER_SHORT ".\n");
            prints("\n主菜单  -->  I) 个人工具箱  -->  F) 填写注册单\n");
            pressreturn();
        }
        /* end of check if local email-addr */
        /*  above lines added by netty...  */
    }
//    	curruserdata.realemail[STRLEN -16 - 1] = '\0';  //纠错代码
    	getSession()->currentmemo->ud.realemail[STRLEN -16 - 1] = '\0';  //纠错代码
//	write_userdata(getCurrentUser()->userid, &curruserdata);
	write_userdata(getCurrentUser()->userid, &(getSession()->currentmemo->ud));
    newregfile = sysconf_str("NEWREGFILE");
    /*if (getCurrentUser()->lastlogin - getCurrentUser()->firstlogin < REGISTER_WAIT_TIME && !HAS_PERM(getCurrentUser(), PERM_SYSOP) && newregfile != NULL) {
        getCurrentUser()->userlevel &= ~(perm);
        ansimore(newregfile, true);
    }先注释掉*/
}

/* 转让ID     by binxun  ... 2003.5 */
int ConveyID(void){
    FILE* fn = NULL;
	long now;
	char buf[STRLEN],filename[STRLEN],systembuf[STRLEN];
	int i;

    //检查权限
        if (HAS_PERM(getCurrentUser(), PERM_SYSOP) || HAS_PERM(getCurrentUser(), PERM_BOARDS) || HAS_PERM(getCurrentUser(), PERM_OBOARDS) || HAS_PERM(getCurrentUser(), PERM_ACCOUNTS)
        || HAS_PERM(getCurrentUser(), PERM_ANNOUNCE)
        || HAS_PERM(getCurrentUser(), PERM_JURY) || HAS_PERM(getCurrentUser(), PERM_SUICIDE) || HAS_PERM(getCurrentUser(), PERM_CHATOP) || (!HAS_PERM(getCurrentUser(), PERM_POST))
        || HAS_PERM(getCurrentUser(), PERM_DENYMAIL)
        || HAS_PERM(getCurrentUser(), PERM_DENYRELAX)) {
        clear();
        move(11, 28);
		prints("\033[1;33m你有重任在身,不能转让ID!\033[m");
        pressanykey();
        return -1;
    }

	//给出提示信息

    clear();
    move(1, 0);
    prints("选择转让ID后,新用户上站填写完资料,即可获得用户权限!\n\n");
    prints("\033[1;31m特别提醒! !转让ID后原来的一切个人资料都不复存在!!\033[m\n\n");
    prints("\033[1;31m该ID的上站次数将下调到不超过10，文章数将调整为0。\033[m\n");
    prints("\033[1;31m该ID生命力的计算时间将从新用户接受转让时刻开始重新计算。\033[m\n");
    prints("\033[1;31m接受转让后的ID，将不能修改转让前所发表的文章，不能管理转让前所开设的Blog。\033[m\n");
    prints("\033[1;31m同时，不排除转让ID将会给您带来其他未声明的不便之处。\033[m\n\n");
    prints("\033[1;31m一旦确认转让，将不可恢复，请您慎重考虑。\033[m\n");
    move(12,0);

    if (askyn("你确定要转让这个 ID 吗？", 0) == 1) {
        clear();
        getdata(0, 0, "请输入原密码(输入正确的话会立刻断线): ", buf, 39, NOECHO, NULL, true);   /*Haohmaru,98.10.12,check the passwds */
        if (*buf == '\0' || !checkpasswd2(buf, getCurrentUser())) {
            prints("\n\n很抱歉, 您输入的密码不正确。\n");
            pressanykey();
            return -1;
        }

        //记录备份信息
        now = time(0);
		gettmpfilename( filename, "convey" );
        //sprintf(filename, "tmp/%s.tmp", getCurrentUser()->userid);
        fn = fopen(filename, "w");
		if(fn){
			fprintf(fn,"\033[1m %s \033[m 在 \033[1m%24.24s\033[m 转让ID了,以下是他的资料，请保留...",getCurrentUser()->userid,ctime(&now));
			getuinfo(fn, getCurrentUser());
			fprintf(fn, "\n                     \033[1m 系统自动发信系统留\033[m\n");
			fclose(fn);
			sprintf(buf, "%s 转让ID的备份资料", getCurrentUser()->userid);
			post_file(getCurrentUser(), "", filename, "Registry", buf, 0, 1,getSession());
			unlink(filename);
		}
		else{
		    move(15,0);
			prints("不能生成临时文件!转让ID失败,请与SYSOP联系.");
			return -1;
		}

		//清空所有存在的配置文件,信箱
		setmailpath(buf,getCurrentUser()->userid);
		sprintf(systembuf,"/bin/rm -fr %s",buf);
		system(systembuf);
		sethomepath(buf,getCurrentUser()->userid);
		sprintf(systembuf,"/bin/rm %s/*",buf);
		system(systembuf);
		sprintf(systembuf,"/bin/rm %s/.*",buf);
        system(systembuf);

		//生成转让ID文件
        sethomefile(filename,getCurrentUser()->userid,"conveyID");
		if((fn=fopen(filename,"w")) != NULL){
		    fprintf(fn,"Convey ID at %s",ctime(&now));
			fclose(fn);
		}
		else{
		    move(15,0);
		    prints("不能生成转让ID文件!转让ID失败,请与SYSOP联系.");
			return -1;
		}
		getCurrentUser()->userlevel = 0;
		getCurrentUser()->userlevel |= PERM_BASIC;

		getCurrentUser()->numposts = 0;
		if(getCurrentUser()->numlogins > 10)getCurrentUser()->numlogins = 10;
		getCurrentUser()->stay = 0;
		strncpy(getCurrentUser()->username,getCurrentUser()->userid,IDLEN);
		SET_UNDEFINE(getCurrentUser(),DEF_NOTMSGFRIEND);
		SET_UNDEFINE(getCurrentUser(),DEF_SHOWREALUSERDATA);

        if (convcode)
            SET_UNDEFINE(getCurrentUser(),DEF_USEGB);

        getCurrentUser()->flags |= PAGER_FLAG;
        getCurrentUser()->title = 0;
		for(i = 0; i < MAXCLUB>>5 ; i++){
		    getCurrentUser()->club_read_rights[i] = 0;
			getCurrentUser()->club_write_rights[i] = 0;
		}
		getCurrentUser()->signature = 0;
		getCurrentUser()->usedspace = 0;

		//clear 用户信息
//		bzero(&curruserdata,sizeof(struct userdata));
		bzero(&(getSession()->currentmemo->ud),sizeof(struct userdata));
//		strcpy(curruserdata.userid,getCurrentUser()->userid);
		strcpy(getSession()->currentmemo->ud.userid,getCurrentUser()->userid);
//		write_userdata(getCurrentUser()->userid,&curruserdata);
		write_userdata(getCurrentUser()->userid,&(getSession()->currentmemo->ud));

        move(12,0);
		prints("转让ID成功,马上断线了,告别这个ID吧.");
        pressanykey();
		//断线
        abort_bbs(0);
    }
    return 0;
}

/*设定ID密码保护 by binxun 2003.10 */
int ProtectID(void){
	char buf[STRLEN],print_buf[STRLEN];
	struct protect_id_passwd protect;
	FILE* fp;
	
	clear();
	if(!HAS_PERM(getCurrentUser(),PERM_LOGINOK)) {
        	move(11, 28);
		prints("\033[1;33m你尚未通过身份认证,不能设定密码保护!\033[m");
        	pressanykey();
		return -1;
        }
	
	sethomefile(buf,getCurrentUser()->userid,"protectID");
	if(dashf(buf)) {
        	move(11, 28);
		prints("\033[1;33m你已经设定密码保护功能,不能再更改设定!\033[m");
        	pressanykey();
		return -1;
	}
	    
	move(1, 0);
    	prints("选择密码保护功能后,可以在遗忘密码或者密码被盗用的情况下,根据事先设定的信息");
	move(2, 0);
	prints("重新找回自己的密码.");
    	move(4, 0);
    	prints("这些设定的信息包括:\033[1;31m 姓名/生日/Email/密码提示问题/问题答案\033[m");
    	move(6,0);
	//prints("上面的信息设置完成, 密码保护功能一旦成功打开后, 将不能再更改这些信息,切记,切记!");
	//move(7,0);
	prints("找回密码时,需要 姓名/生日/问题答案 和设定完全一致,才会将新密码发往 Email .");

	move(8,0);
    	if (askyn("你确定要打开密码保护功能吗？", 0) == 0) {
		return -1;
	}
	
	clear();
	memset(&protect, 0 , sizeof(struct protect_id_passwd));
	//输入相关设置信息

	move(1,0);
	prints("请逐项修改,直接按 <ENTER> 代表使用 [] 内的资料。\n");
	
	sprintf(print_buf,"请输入您的真实姓名: [%s]",getSession()->currentmemo->ud.realname);
	getdata(3, 0, print_buf, buf, NAMELEN, DOECHO, NULL, true);
	if(buf[0])
		strncpy(protect.name,buf,NAMELEN);
	else
		strncpy(protect.name,getSession()->currentmemo->ud.realname,NAMELEN);
	
#ifdef HAVE_BIRTHDAY
	move(4,0);
	prints("请输入您的出生日期: ");
	
	sprintf(print_buf,"四位数公元年: [%d]",getSession()->currentmemo->ud.birthyear);
	while (protect.birthyear > 2010 || protect.birthyear < 1900) {
		getdata(5, 0, print_buf, buf, 5, DOECHO, NULL, true);
		if(buf[0]) protect.birthyear = atoi(buf);
		else
			protect.birthyear = getSession()->currentmemo->ud.birthyear;
	}

	sprintf(print_buf,"出生月: [%d]",getSession()->currentmemo->ud.birthmonth);
	while (protect.birthmonth < 1 || protect.birthmonth > 12) {
		getdata(6, 0, print_buf, buf, 3, DOECHO, NULL, true);
		if(buf[0]) protect.birthmonth = atoi(buf);
		else
			protect.birthmonth = getSession()->currentmemo->ud.birthmonth;
	}
	
	sprintf(print_buf,"出生日: [%d]",getSession()->currentmemo->ud.birthday);
	while (protect.birthday < 1 || protect.birthday > 31) {
		getdata(7, 0, print_buf, buf, 3, DOECHO, NULL, true);
		if(buf[0]) protect.birthday = atoi(buf);
		else
			protect.birthday = getSession()->currentmemo->ud.birthday;
	}
#endif

	sprintf(print_buf,"您的Email: ");
	do {
		getdata(8, 0, print_buf, buf, STRLEN, DOECHO, NULL, true);
	} while(!strchr(buf,'@'));
	strncpy(protect.email, buf, STRLEN);
	
	sprintf(print_buf,"密码提示问题: ");
	do {
		getdata(9, 0, print_buf, buf, STRLEN, DOECHO, NULL, true);
	} while(!buf[0]);
	strncpy(protect.question, buf, STRLEN);
	
	sprintf(print_buf,"问题答案(至少四个字符): ");
	do {
		getdata(10, 0, print_buf, buf, STRLEN, DOECHO, NULL, true);
	} while(strlen(buf) < 4);
	strncpy(protect.answer, buf, STRLEN);

	if (askyn("你确定要设定吗？", 0) == 1) {	
		move(12,0);
		sethomefile(buf,getCurrentUser()->userid,"protectID");	
		
		fp = fopen(buf,"w");
		if(!fp) {
			prints("不能打开文件,请与SYSOP联系.");	
			return 0;
		}
		fwrite(&protect,sizeof(struct protect_id_passwd),1,fp);
		fclose(fp);
		
		prints("密码保护已经设定");
		pressanykey();
	}
    return 0;
}
