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

char *sysconf_str();
char *Ctime();

extern struct user_info uinfo;
extern time_t login_start_time;
extern int convcode;            /* KCN,99.09.05 */
extern int switch_code();       /* KCN,99.09.05 */

void new_register()
{
    struct userec newuser;
    int allocid, do_try, flag;
    FILE* fn;
    char buf[STRLEN], fname[PATHLEN], title[STRLEN];

/* temp !!!!!*/
/*    prints("Sorry, we don't accept newusers due to system problem, we'll fixit ASAP\n");
    oflush();
    sleep(2);
    exit(-1);
*/
    memset(&newuser, 0, sizeof(newuser));
    getdata(0, 0, "Ê¹ÓÃGB±àÂëÔÄ¶Á?(\xa8\xcf\xa5\xce BIG5\xbd\x58\xbe\x5c\xc5\xaa\xbd\xd0\xbf\xefN)(Y/N)? [Y]: ", buf, 4, DOECHO, NULL, true);
    if (*buf == 'n' || *buf == 'N')
        if (!convcode)
            switch_code();

    ansimore("etc/register", false);
    do_try = 0;
    while (1) {
        if (++do_try >= 10) {
            prints("\nêşêş£¬°´Ì«¶àÏÂ  <Enter> ÁË...\n");
            refresh();
            longjmp(byebye, -1);
        }
        getdata(0, 0, "ÇëÊäÈë´úºÅ: ", newuser.userid, IDLEN + 1, DOECHO, NULL, true);
        flag = 1;
        if (id_invalid(newuser.userid) == 1) {
            prints("ÕÊºÅ±ØĞëÓÉÓ¢ÎÄ×ÖÄ¸»òÊı×Ö×é³É£¬²¢ÇÒµÚÒ»¸ö×Ö·û±ØĞëÊÇÓ¢ÎÄ×ÖÄ¸!\n");
            /*                prints("ÕÊºÅ±ØĞëÓÉÓ¢ÎÄ×ÖÄ¸»òÊı×Ö£¬¶øÇÒÕÊºÅµÚÒ»¸ö×ÖÊÇÓ¢ÎÄ×ÖÄ¸!\n"); */
            flag = 0;
        }
        if (flag) {
            if (strlen(newuser.userid) < 2) {
                prints("´úºÅÖÁÉÙĞèÓĞÁ½¸öÓ¢ÎÄ×ÖÄ¸!\n");
            } else if ((*newuser.userid == '\0') || bad_user_id(newuser.userid)) {
                prints("ÏµÍ³ÓÃ×Ö»òÊÇ²»ÑÅµÄ´úºÅ¡£\n");
            } else if ((usernum = searchuser(newuser.userid)) != 0) {   /*( dosearchuser( newuser.userid ) ) midified by dong , 1998.12.2, change getuser -> searchuser , 1999.10.26 */
                prints("´ËÕÊºÅÒÑ¾­ÓĞÈËÊ¹ÓÃ\n");
            } else {
                /*---	---*/
                struct stat lst;
                time_t lnow;

                lnow = time(NULL);
                sethomepath(genbuf, newuser.userid);
                if (!stat(genbuf, &lst) && S_ISDIR(lst.st_mode)
                    && (lnow - lst.st_ctime < SEC_DELETED_OLDHOME /* 3600*24*30 */ )) {
                    prints("Ä¿Ç°ÎŞ·¨×¢²áÕÊºÅ%s£¬ÇëÓëÏµÍ³¹ÜÀíÈËÔ±ÁªÏµ¡£\n", newuser.userid);
                    sprintf(genbuf, "IP %s new id %s failed[home changed in past 30 days]", fromhost, newuser.userid);
                    bbslog("user","%s",genbuf);
                } else
                /*---	---*/
                    break;
            }
        }
    }

    newuser.firstlogin = newuser.lastlogin = time(NULL) - 13 * 60 * 24;
    do_try = 0;
    while (1) {
        char passbuf[STRLEN], passbuf2[STRLEN];

        if (++do_try >= 10) {
            prints("\nêşêş£¬°´Ì«¶àÏÂ  <Enter> ÁË...\n");
            refresh();
            longjmp(byebye, -1);
        }
        getdata(0, 0, "ÇëÉè¶¨ÄúµÄÃÜÂë: ", passbuf, 39, NOECHO, NULL, true);
        if (strlen(passbuf) < 4 || !strcmp(passbuf, newuser.userid)) {
            prints("ÃÜÂëÌ«¶Ì»òÓëÊ¹ÓÃÕß´úºÅÏàÍ¬, ÇëÖØĞÂÊäÈë\n");
            continue;
        }
        getdata(0, 0, "ÇëÔÙÊäÈëÒ»´ÎÄãµÄÃÜÂë: ", passbuf2, 39, NOECHO, NULL, true);
        if (strcmp(passbuf, passbuf2) != 0) {
            prints("ÃÜÂëÊäÈë´íÎó, ÇëÖØĞÂÊäÈëÃÜÂë.\n");
            continue;
        }

        setpasswd(passbuf, &newuser);
        break;
    }
    newuser.userlevel = PERM_BASIC;
    newuser.userdefine = -1;
/*   newuser.userdefine&=~DEF_MAILMSG;
    newuser.userdefine&=~DEF_EDITMSG; */
    newuser.userdefine &= ~DEF_NOTMSGFRIEND;
    if (convcode)
        newuser.userdefine &= ~DEF_USEGB;

    newuser.notemode = -1;
    newuser.exittime = time(NULL) - 100;
    /*newuser.unuse2 = -1;*/
    newuser.flags[0] = CURSOR_FLAG;
    newuser.flags[0] |= PAGER_FLAG;
    newuser.flags[1] = 0;
    newuser.firstlogin = newuser.lastlogin = time(NULL);

    allocid = getnewuserid2(newuser.userid);
    if (allocid > MAXUSERS || allocid <= 0) {
        prints("±§Ç¸, ÓÉÓÚÄ³Ğ©ÏµÍ³Ô­Òò, ÎŞ·¨×¢²áĞÂµÄÕÊºÅ.\n\r");
        oflush();
        sleep(2);
        exit(1);
    }
    newbbslog(BBSLOG_USIES, "APPLY: uid %d from %s", allocid, fromhost);

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

/*¼ÓÈë¶Ô #TH ½áÎ²µÄrealemailµÄÕÊºÅ×Ô¶¯Í¨¹ı×¢²áµÄ¹¦ÄÜ  by binxun
*/
int invalid_realmail(userid, email, msize)
    char *userid, *email;
    int msize;
{
    FILE *fn;
    char *emailfile, ans[4], fname[STRLEN];
    char genbuf[STRLEN];
    struct userec* uc;
    time_t now;
    int len = strlen(email);

    if ((emailfile = sysconf_str("EMAILFILE")) == NULL)
        return 0;

    if (strchr(email, '@') && valid_ident(email))
        return 0;
    /*
       ansimore( emailfile, false );
       getdata(t_lines-1,0,"ÄúÒªÏÖÔÚ email-post Âğ? (Y/N) [Y]: ",
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
            prints("¹§ºØÄú!! ÄúÒÑÍ¨¹ıÉí·İÑéÖ¤, ³ÉÎª±¾Õ¾¹«Ãñ. \n");
            prints("         ±¾Õ¾ÎªÄúËùÌá¹©µÄ¶îÍâ·şÎñ, \n");
            prints("         °üÀ¨Mail,Post,Message,Talk µÈ. \n");
            prints("  \n");
            prints("½¨ÒéÄú,  ÏÈËÄ´¦ä¯ÀÀÒ»ÏÂ, \n");
            prints("         ²»¶®µÄµØ·½, ÇëÔÚ sysop °æÁôÑÔ, \n");
            prints("         ±¾Õ¾»áÅÉ×¨ÈËÎªÄú½â´ğ. \n");
            getdata(18, 0, "Çë°´ <Enter>  <<  ", ans, 2, DOECHO, NULL, true);
            return 0;
        }
    }

#ifdef HAVE_TSINGHUA_INFO_REGISTER

    if(len >= 3)
    {
    	strncpy(genbuf,email+strlen(email)-3,3);
	if(!strncasecmp(genbuf,"#TH",3))
	{
		getuser(userid,&uc);
		// > 3 days
		now = time(NULL);
		if(now - uc->firstlogin >= REGISTER_TSINGHUA_WAIT_TIME)
		{
	 		if(auto_register(userid,email,msize) < 0) // Íê³É×Ô¶¯×¢²á
				return 1;
			else
				return 0;     //success
		}
	}
    }
#endif
    return 1;
}

void check_register_info()
{
    char *newregfile;
    int perm;
    char buf[STRLEN];
	char career[STRLEN];
	char phone[40];

    clear();
    sprintf(buf, "%s", email_domain());
    if (!(currentuser->userlevel & PERM_BASIC)) {
        currentuser->userlevel = PERM_DENYMAIL|PERM_DENYRELAX;
        return;
    }
    /*urec->userlevel |= PERM_DEFAULT; */
    perm = PERM_DEFAULT & sysconf_eval("AUTOSET_PERM",PERM_DEFAULT);

    invalid_realmail(currentuser->userid,curruserdata.realemail,STRLEN - 16);

    do_after_login(currentuser,utmpent,0);

    /*    if( sysconf_str( "IDENTFILE" ) != NULL ) {  commented out by netty to save time */
    while (strlen(currentuser->username) < 2) {
        getdata(2, 0, "ÇëÊäÈëÄúµÄêÇ³Æ:(ÀıÈç," DEFAULT_NICK ") << ", buf, NAMELEN, DOECHO, NULL, true);
        strcpy(currentuser->username, buf);
        strcpy(uinfo.username, buf);
        UPDATE_UTMP_STR(username, uinfo);
    }
    if (strlen(curruserdata.realname) < 2) {
        move(3, 0);
        prints("ÇëÊäÈëÄúµÄÕæÊµĞÕÃû: (Õ¾³¤»á°ïÄú±£ÃÜµÄ !)\n");
        getdata(4, 0, "> ", buf, NAMELEN, DOECHO, NULL, true);
        strcpy(curruserdata.realname, buf);
    }
    if (strlen(curruserdata.address) < 6) {
        move(5, 0);
        prints("ÄúÄ¿Ç°ÌîĞ´µÄµØÖ·ÊÇ¡®%s¡¯£¬³¤¶ÈĞ¡ÓÚ [1m[37m6[m£¬ÏµÍ³ÈÏÎªÆä¹ıÓÚ¼ò¶Ì¡£\n", curruserdata.address[0] ? curruserdata.address : "¿ÕµØÖ·");  /* Leeward 98.04.26 */
        getdata(6, 0, "ÇëÏêÏ¸ÌîĞ´ÄúµÄ×¡Ö·£º", buf, NAMELEN, DOECHO, NULL, true);
        strcpy(curruserdata.address, buf);
    }

	/* ¼ÓÈë×ªÈÃIDºóµÄ´úÂë   by binxun 2003-5-23 */
	sethomefile(buf,currentuser->userid,"conveyID");
	if(dashf(buf))
	{
	
        move(6,0);
		prints("´ËIDÓÉÄúµÄÅóÓÑ×ªÈÃ¸øÄú,¹§Ï²Äú»ñµÃ´ËID,ÇëÌîĞ´ÒÔÏÂ×ÊÁÏ.");
		do{
		    getdata(7,0,"Ñ§Ğ£Ïµ¼¶»òµ¥Î»È«³Æ(¾ßÌåµ½²¿ÃÅ):",career,STRLEN,DOECHO,NULL,true);
		}while(strlen(career) < 4);
        do{
		    getdata(8,0,"ÄúµÄÁªÏµµç»°»òÕßEmail:",phone,40,DOECHO,NULL,true);
		}while(strlen(phone) < 6);

             unlink(buf);   
        sprintf(buf,"%s$%s@SYSOP", career,phone);
		if(strlen(buf) >= STRLEN - 16)sprintf(buf,"%s@SYSOP",phone);
		strncpy(curruserdata.realemail,buf,STRLEN-16);
		curruserdata.realemail[STRLEN-16-1]='\0';
		write_userdata(currentuser->userid,&curruserdata);
		
	}


    if (strchr(curruserdata.email, '@') == NULL) {
        clear();
        move(3, 0);
        prints("Ö»ÓĞ±¾Õ¾µÄºÏ·¨¹«Ãñ²ÅÄÜ¹»ÍêÈ«ÏíÓĞ¸÷ÖÖ¹¦ÄÜ£¬");
        /* alex           prints( "³ÉÎª±¾Õ¾ºÏ·¨¹«ÃñÓĞÁ½ÖÖ°ì·¨£º\n\n" );
           prints( "1. Èç¹ûÄãÓĞºÏ·¨µÄemailĞÅÏä(·ÇBBS), \n");
           prints( "       Äã¿ÉÒÔÓÃ»ØÈÏÖ¤ĞÅµÄ·½Ê½À´Í¨¹ıÈÏÖ¤¡£ \n\n" );
           prints( "2. Èç¹ûÄãÃ»ÓĞemailĞÅÏä(·ÇBBS)£¬Äã¿ÉÒÔÔÚ½øÈë±¾Õ¾ÒÔºó£¬\n" );
           prints( "       ÔÚ'¸öÈË¹¤¾ßÏä'ÄÚ ÏêÏ¸×¢²áÕæÊµÉí·İ£¬( Ö÷²Ëµ¥  -->  I) ¸öÈË¹¤¾ßÏä  -->  F) ÌîĞ´×¢²áµ¥ )\n" );
           prints( "       SYSOPs »á¾¡¿ì ¼ì²é²¢È·ÈÏÄãµÄ×¢²áµ¥¡£\n" );
           move( 17, 0 );
           prints( "µç×ÓĞÅÏä¸ñÊ½Îª: xxx@xxx.xxx.edu.cn \n" );
           getdata( 18, 0, "ÇëÊäÈëµç×ÓĞÅÏä: (²»ÄÜÌá¹©Õß°´ <Enter>) << "
           , urec->email, STRLEN,DOECHO,NULL,true);
           if ((strchr( urec->email, '@' ) == NULL )) {
           sprintf( genbuf, "%s.bbs@%s", urec->userid,buf );
           strncpy( urec->email, genbuf, STRLEN);
           }
           alex, ÒòÎªÈ¡ÏûÁËemail¹¦ÄÜ , 97.7 */
        prints("³ÉÎª" NAME_BBS_NICK"ºÏ·¨" NAME_USER_SHORT "µÄ·½·¨ÈçÏÂ£º\n\n");
        prints("ÄúµÄÕÊºÅÔÚµÚÒ»´ÎµÇÂ¼ºóµÄ " REGISTER_WAIT_TIME_NAME "ÄÚ£¨[1m[33m²»ÊÇÖ¸ÉÏ BBS " REGISTER_WAIT_TIME_NAME "[m£©£¬\n");
        prints("´¦ÓÚĞÂÊÖÉÏÂ·ÆÚ¼ä, ²»ÄÜ×¢²á³ÉÎªºÏ·¨" NAME_USER_SHORT "£¬ÇëËÄ´¦²Î¹ÛÑ§Ï°£¬ÍÆ¼öÔÄ¶Á BBSHELP°æ£¬\nÑ§Ï°±¾Õ¾Ê¹ÓÃ·½·¨ºÍ¸÷ÖÖÀñÒÇ¡£\n");
        prints("\n" REGISTER_WAIT_TIME_NAME "ºó, Äú¾Í¿ÉÒÔ\033[33;1mÌîĞ´×¢²áµ¥\033[mÁË£¬×¢²áµ¥Í¨¹ıÕ¾ÎñÈÏÖ¤ÒÔºó£¬Äú¾ÍÓµÓĞ±¾Õ¾ºÏ·¨ÓÃ»§\nµÄ»ù±¾È¨ÏŞ¡£×¢²áµ¥ÌîĞ´Â·¾¶ÈçÏÂ: \n\n");
        prints("I) ¸öÈË¹¤¾ßÏä --> F) ÌîĞ´×¢²áµ¥\n\n");
        //prints("    " NAME_SYSOP_GROUP "»á¾¡¿ì¼ì²é²¢È·ÈÏÄãµÄ×¢²áµ¥¡£\n\n");
        /* Leeward adds below 98.04.26 */
        prints("[1m[33mÈç¹ûÄúÒÑ¾­Í¨¹ı×¢²á£¬³ÉÎªºÏ·¨" NAME_USER_SHORT "£¬È´ÒÀÈ»¿´µ½±¾ĞÅÏ¢£¬ÄÇ¿ÉÄÜÊÇÓÉÓÚÄúÃ»ÓĞÔÚ\n¡®¸öÈË¹¤¾ßÏä¡¯ÄÚÉè¶¨¡®µç×ÓÓÊ¼şĞÅÏä¡¯¡£[m\n");
	prints("\nI) ¸öÈË¹¤¾ßÏä --> I) Éè¶¨¸öÈË×ÊÁÏ\n");

	prints("\nÈç¹ûÄúÊµÔÚÃ»ÓĞÈÎºÎ¿ÉÓÃµÄ'µç×ÓÓÊ¼şĞÅÏä'¿ÉÒÔÉè¶¨£¬ÓÖ²»Ô¸Òâ¿´µ½±¾ĞÅÏ¢£¬¿ÉÒÔÊ¹ÓÃ\n%s.bbs@%s½øĞĞÉè¶¨¡£\n\033[33;1m×¢Òâ: ÉÏÃæµÄµç×ÓÓÊ¼şĞÅÏä²»ÄÜ½ÓÊÕµç×ÓÓÊ¼ş£¬½öÓÃÀ´Ê¹ÏµÍ³²»ÔÙÏÔÊ¾±¾ĞÅÏ¢¡£\033[m", currentuser->userid, NAME_BBS_ENGLISH);
        pressreturn();
    }
#ifdef HAVE_BIRTHDAY
	if (!is_valid_date(curruserdata.birthyear+1900, curruserdata.birthmonth,
				curruserdata.birthday))
	{
		time_t now;
		struct tm *tmnow;

		now = time(0);
		tmnow = localtime(&now);
		clear();
		buf[0] = '\0';
		move(0, 0);
		prints("ÎÒÃÇ¼ì²éµ½ÄãµÄ²¿·Ö×¢²á×ÊÁÏ²»¹»ÍêÈ«£¬ÎªÁË¸üºÃµÄÎªÄúÌá¹©¸öĞÔ»¯µÄ·şÎñ£¬");
		move(1, 0);
		prints("Ï£ÍûÄúÌîĞ´ÒÔÏÂ×ÊÁÏ¡£");
		while (buf[0] < '1' || buf[0] > '2')
		{
			getdata(2, 0, "ÇëÊäÈëÄãµÄĞÔ±ğ: [1]ÄĞµÄ [2]Å®µÄ (1 or 2): ",
					buf, 2, DOECHO, NULL, true);
		}
		switch (buf[0])
		{
		case '1':
			curruserdata.gender = 'M';
			break;
		case '2':
			curruserdata.gender = 'F';
			break;
		}
		move(4, 0);
		prints("ÇëÊäÈëÄúµÄ³öÉúÈÕÆÚ");
		while (curruserdata.birthyear < tmnow->tm_year - 98
			   || curruserdata.birthyear > tmnow->tm_year - 3)
		{
			buf[0] = '\0';
			getdata(5, 0, "ËÄÎ»Êı¹«ÔªÄê: ", buf, 5, DOECHO, NULL, true);
			if (atoi(buf) < 1900)
				continue;
			curruserdata.birthyear = atoi(buf) - 1900;
		}
		while (curruserdata.birthmonth < 1 || curruserdata.birthmonth > 12)
		{
			buf[0] = '\0';
			getdata(6, 0, "³öÉúÔÂ: (1-12) ", buf, 3, DOECHO, NULL, true);
			curruserdata.birthmonth = atoi(buf);
		}
		do
		{
			buf[0] = '\0';
			getdata(7, 0, "³öÉúÈÕ: (1-31) ", buf, 3, DOECHO, NULL, true);
			curruserdata.birthday = atoi(buf);
		} while (!is_valid_date(curruserdata.birthyear + 1900,
					curruserdata.birthmonth,
					curruserdata.birthday));
		write_userdata(currentuser->userid, &curruserdata);
	}
#endif
#ifdef NEW_COMERS
	if (currentuser->numlogins == 1)
	{
		FILE *fout;
		char buf2[STRLEN];

		gettmpfilename( buf, "newcomer" );
		//sprintf(buf, "tmp/newcomer.%s", currentuser->userid);
		if ((fout = fopen(buf, "w")) != NULL)
		{
			fprintf(fout, "´ó¼ÒºÃ,\n\n");
			fprintf(fout, "ÎÒÊÇ %s (%s), À´×Ô %s\n", currentuser->userid,
					currentuser->username, fromhost);
			fprintf(fout, "½ñÌì%s³õÀ´´ËÕ¾±¨µ½, Çë´ó¼Ò¶à¶àÖ¸½Ì¡£\n",
#ifdef HAVE_BIRTHDAY
					(curruserdata.gender == 'M') ? "Ğ¡µÜ" : "Ğ¡Å®×Ó");
#else
                                        "Ğ¡µÜ");
#endif
			move(9, 0);
			prints("Çë×÷¸ö¼ò¶ÌµÄ¸öÈË¼ò½é, Ïò±¾Õ¾ÆäËûÊ¹ÓÃÕß´ò¸öÕĞºô\n");
			prints("(×î¶àÈıĞĞ, Ğ´Íê¿ÉÖ±½Ó°´ <Enter> ÌøÀë)....");
			getdata(11, 0, ":", buf2, 75, DOECHO, NULL, true);
			if (buf2[0] != '\0')
			{
				fprintf(fout, "\n\n×ÔÎÒ½éÉÜ:\n\n");
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
			fclose(fout);
			sprintf(buf2, "ĞÂÊÖÉÏÂ·: %s", currentuser->username);
			post_file(currentuser, "", buf, "newcomers", buf2, 0, 2);
			unlink(buf);
		}
		pressanykey();
	}
#endif
    if (!strcmp(currentuser->userid, "SYSOP")) {
        currentuser->userlevel = ~0;
        currentuser->userlevel &= ~PERM_SUICIDE;        /* Leeward 98.10.13 */
        currentuser->userlevel &= ~(PERM_DENYMAIL|PERM_DENYRELAX);       /* Bigman 2000.9.22 */
        currentuser->userlevel &= ~PERM_JURY;       /* ²»ÄÜÊÇÖÙ²Ã */
    }
    if (!(currentuser->userlevel & PERM_LOGINOK)) {
        if (HAS_PERM(currentuser, PERM_SYSOP))
            return;
        if (!invalid_realmail(currentuser->userid, curruserdata.realemail, STRLEN - 16)) {
            currentuser->userlevel |= PERM_DEFAULT;
            /*
            if (HAS_PERM(currentuser, PERM_DENYPOST) && !HAS_PERM(currentuser, PERM_SYSOP))
                currentuser->userlevel &= ~PERM_POST;
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
               prints( "ÄúµÄµç×ÓĞÅÏä  ÉĞĞëÍ¨¹ı»ØĞÅÑéÖ¤...  \n" );
               prints( "      SYSOP ½«¼ÄÒ»·âÑéÖ¤ĞÅ¸øÄú,\n" );
               prints( "      ÄúÖ»Òª»ØĞÅ, ¾Í¿ÉÒÔ³ÉÎª±¾Õ¾ºÏ¸ñ¹«Ãñ.\n" );
               getdata( 19 ,0, "ÄúÒª SYSOP ¼ÄÕâÒ»·âĞÅÂğ?(Y/N) [Y] << ", ans,2,DOECHO,NULL,true);
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
               fprintf(fout,"ÄúµÄ»ù±¾×ÊÁÏÈçÏÂ£º\n",urec->userid);
               fprintf(fout,"Ê¹ÓÃÕß´úºÅ£º%s (%s)\n",urec->userid,urec->username);
               fprintf(fout,"ĞÕ      Ãû£º%s\n",urec->realname);
               fprintf(fout,"ÉÏÕ¾Î»ÖÃ  £º%s\n",urec->lasthost);
               fprintf(fout,"µç×ÓÓÊ¼ş  £º%s\n\n",urec->email);
               fprintf(fout,"Ç×°®µÄ %s(%s):\n",urec->userid,urec->username);
               while (fgets( genbuf, 255, fin ) != NULL ) {
               if (genbuf[0] == '.' && genbuf[ 1 ] == '\n')
               fputs( ". \n", fout );
               else fputs( genbuf, fout );
               }
               fprintf(fout, ".\n");                                    
               fclose( fin );
               fclose( fout );                                     
               }
               getdata( 20 ,0, "ĞÅÒÑ¼Ä³ö, SYSOP ½«µÈÄú»ØĞÅÅ¶!! Çë°´ <Enter> << ", ans,2,DOECHO,NULL ,true);
               }
               }else
               {
               showansi=1;
               if(sysconf_str( "EMAILFILE" )!=NULL)
               {
               prints("\nÄãµÄµç×ÓÓÊ¼şµØÖ· ¡¾[33m%s[m¡¿\n",urec->email);
               prints("²¢·Ç Unix ÕÊºÅ£¬ÏµÍ³²»»áÍ¶µİÉí·İÈ·ÈÏĞÅ£¬Çëµ½[32m¹¤¾ßÏä[mÖĞĞŞ¸Ä..\n");
               pressanykey();
               }
               }
               deleted by alex, remove email certify */

            clear();            /* Leeward 98.05.14 */
            move(12, 0);
            prints("ÄúÏÖÔÚ»¹Ã»ÓĞÍ¨¹ıÉí·İÈÏÖ¤£¬Ã»ÓĞtalk,mail,message,postµÈÈ¨ÏŞ¡£\n");
            prints("Èç¹ûÄãÒª³ÉÎª" NAME_BBS_NICK "µÄ×¢²á" NAME_USER_SHORT "£¬");
            prints("Çëµ½[33;1m¸öÈË¹¤¾ßÏä[mÖĞÏêÏ¸ÕæÊµµØÌîĞ´[32m ×¢²áµ¥,[m\n");
            prints("¾­" NAME_SYSOP_GROUP "ÉóºËÍ¨¹ıÒÔºó£¬¾Í¿ÉÒÔ³ÉÎª±¾Õ¾µÄ×¢²á" NAME_USER_SHORT ".\n");
            prints("\nÖ÷²Ëµ¥  -->  I) ¸öÈË¹¤¾ßÏä  -->  F) ÌîĞ´×¢²áµ¥\n");
            pressreturn();
        }
        /* end of check if local email-addr */
        /*  above lines added by netty...  */
    }
    	curruserdata.realemail[STRLEN -16 - 1] = '\0';  //¾À´í´úÂë
	write_userdata(currentuser->userid, &curruserdata);
    newregfile = sysconf_str("NEWREGFILE");
    /*if (currentuser->lastlogin - currentuser->firstlogin < REGISTER_WAIT_TIME && !HAS_PERM(currentuser, PERM_SYSOP) && newregfile != NULL) {
        currentuser->userlevel &= ~(perm);
        ansimore(newregfile, true);
    }ÏÈ×¢ÊÍµô*/
}

/* ×ªÈÃID     by binxun  ... 2003.5 */
void ConveyID()
{
    FILE* fn = NULL;
	long now;
	char buf[STRLEN],filename[STRLEN],systembuf[STRLEN];
	int i;

    //¼ì²éÈ¨ÏŞ
        if (HAS_PERM(currentuser, PERM_SYSOP) || HAS_PERM(currentuser, PERM_BOARDS) || HAS_PERM(currentuser, PERM_OBOARDS) || HAS_PERM(currentuser, PERM_ACCOUNTS)
        || HAS_PERM(currentuser, PERM_ANNOUNCE)
        || HAS_PERM(currentuser, PERM_JURY) || HAS_PERM(currentuser, PERM_SUICIDE) || HAS_PERM(currentuser, PERM_CHATOP) || (!HAS_PERM(currentuser, PERM_POST))
        || HAS_PERM(currentuser, PERM_DENYMAIL)
        || HAS_PERM(currentuser, PERM_DENYRELAX)) {
        clear();
        move(11, 28);
		prints("\033[1;33mÄãÓĞÖØÈÎÔÚÉí,²»ÄÜ×ªÈÃID!\033[m");
        pressanykey();
        return;
    }

	//¸ø³öÌáÊ¾ĞÅÏ¢

    clear();
    move(1, 0);
    prints("Ñ¡Ôñ×ªÈÃIDºó,ĞÂÓÃ»§ÉÏÕ¾ÌîĞ´Íê×ÊÁÏ,¼´¿É»ñµÃÓÃ»§È¨ÏŞ!");
    move(3, 0);
    prints("\033[1;31mÌØ±ğÌáĞÑ! !×ªÈÃIDºóÔ­À´µÄÒ»ÇĞ¸öÈË×ÊÁÏ¶¼²»¸´´æÔÚ!!\033[m");
    move(5,0);

    if (askyn("ÄãÈ·¶¨Òª×ªÈÃÕâ¸ö ID Âğ£¿", 0) == 1) {
        clear();
        getdata(0, 0, "ÇëÊäÈëÔ­ÃÜÂë(ÊäÈëÕıÈ·µÄ»°»áÁ¢¿Ì¶ÏÏß): ", buf, 39, NOECHO, NULL, true);   /*Haohmaru,98.10.12,check the passwds */
        if (*buf == '\0' || !checkpasswd2(buf, currentuser)) {
            prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
            pressanykey();
            return;
        }

        //¼ÇÂ¼±¸·İĞÅÏ¢
        now = time(0);
		gettmpfilename( filename, "convey" );
        //sprintf(filename, "tmp/%s.tmp", currentuser->userid);
        fn = fopen(filename, "w");
		if(fn){
			fprintf(fn,"\033[1m %s \033[m ÔÚ \033[1m%24.24s\033[m ×ªÈÃIDÁË,ÒÔÏÂÊÇËûµÄ×ÊÁÏ£¬Çë±£Áô...",currentuser->userid,ctime(&now));
			getuinfo(fn, currentuser);
			fprintf(fn, "\n                     \033[1m ÏµÍ³×Ô¶¯·¢ĞÅÏµÍ³Áô\033[m\n");
			fclose(fn);
			sprintf(buf, "%s ×ªÈÃIDµÄ±¸·İ×ÊÁÏ", currentuser->userid);
			post_file(currentuser, "", filename, "Registry", buf, 0, 1);
			unlink(filename);
		}
		else{
		    move(12,0);
			prints("²»ÄÜÉú³ÉÁÙÊ±ÎÄ¼ş!×ªÈÃIDÊ§°Ü,ÇëÓëSYSOPÁªÏµ.");
			return;
		}

		//Çå¿ÕËùÓĞ´æÔÚµÄÅäÖÃÎÄ¼ş,ĞÅÏä
		setmailpath(buf,currentuser->userid);
		sprintf(systembuf,"/bin/rm -fr %s",buf);
		system(systembuf);
		sethomepath(buf,currentuser->userid);
		sprintf(systembuf,"/bin/rm %s/*",buf);
		system(systembuf);
		sprintf(systembuf,"/bin/rm %s/.*",buf);
        system(systembuf);

		//Éú³É×ªÈÃIDÎÄ¼ş
        sethomefile(filename,currentuser->userid,"conveyID");
		if((fn=fopen(filename,"w")) != NULL){
		    fprintf(fn,"Convey ID at %s",ctime(&now));
			fclose(fn);
		}
		else{
		    move(12,0);
		    prints("²»ÄÜÉú³É×ªÈÃIDÎÄ¼ş!×ªÈÃIDÊ§°Ü,ÇëÓëSYSOPÁªÏµ.");
			return;
		}
		currentuser->userlevel = 0;
		currentuser->userlevel |= PERM_BASIC;

		currentuser->numposts = 0;
		if(currentuser->numlogins > 10)currentuser->numlogins = 10;
		currentuser->stay = 0;
		strncpy(currentuser->username,currentuser->userid,IDLEN);
		currentuser->userdefine &= ~DEF_NOTMSGFRIEND;
        if (convcode)
            currentuser->userdefine &= ~DEF_USEGB;

        currentuser->notemode = -1;

        currentuser->flags[0] = CURSOR_FLAG;
        currentuser->flags[0] |= PAGER_FLAG;
        currentuser->flags[1] = 0;
		for(i = 0; i < MAXCLUB>>5 ; i++){
		    currentuser->club_read_rights[i] = 0;
			currentuser->club_write_rights[i] = 0;
		}
		currentuser->signature = 0;
		currentuser->usedspace = 0;

		//clear ÓÃ»§ĞÅÏ¢
		bzero(&curruserdata,sizeof(struct userdata));
		strcpy(curruserdata.userid,currentuser->userid);
		write_userdata(currentuser->userid,&curruserdata);

        move(12,0);
		prints("×ªÈÃID³É¹¦,ÂíÉÏ¶ÏÏßÁË,¸æ±ğÕâ¸öID°É.");
        pressanykey();
		//¶ÏÏß
        abort_bbs(0);
    }

    return;
}
