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

#include "bbs.h"

#define  EMAIL          0x0001 
#define  NICK           0x0002 
#define  REALNAME       0x0004 
#define  ADDR           0x0008
#define  REALEMAIL      0x0010
#define  BADEMAIL       0x0020
#define  NEWREG         0x0040

char    *sysconf_str();
char    *Ctime();

extern time_t   login_start_time;
time_t          system_time;
extern int convcode;  /* KCN,99.09.05 */
extern int switch_code(); /* KCN,99.09.05 */

void
new_register()
{
    struct userec       newuser;
    int    allocid, do_try,flag,lockfd;
    char   buf[STRLEN];
/* temp !!!!!*/
/*    prints("Sorry, we don't accept newusers due to system problem, we'll fixit ASAP\n");
    oflush();
    sleep(2);
    exit(-1);
*/
    memset( &newuser, 0, sizeof(newuser) );
    getdata(0, 0, "Ê¹ÓÃGB±àÂëÔÄ¶Á?(\xa8\xcf\xa5\xce BIG5\xbd\x58\xbe\x5c\xc5\xaa\xbd\xd0\xbf\xefN)(Y/N)? [Y]: ", buf, 4, DOECHO, NULL, YEA);
    if (*buf == 'n' || *buf == 'N')
        if (!convcode)
            switch_code();

    ansimore("etc/register", NA);
    do_try = 0;
    while( 1 ) {
        if( ++do_try >= 10 ) {
            prints("\nêşêş£¬°´Ì«¶àÏÂ  <Enter> ÁË...\n");
            refresh();
            longjmp( byebye, -1 );
        }
        getdata(0,0,"ÇëÊäÈë´úºÅ: ",newuser.userid,IDLEN+1,DOECHO,NULL,YEA);
        flag = 1;
        if(id_invalid(newuser.userid)==1)
        {
            prints("ÕÊºÅ±ØĞëÓÉÓ¢ÎÄ×ÖÄ¸»òÊı×Ö×é³É£¬²¢ÇÒµÚÒ»¸ö×Ö·û±ØĞëÊÇÓ¢ÎÄ×ÖÄ¸!\n");
            /*                prints("ÕÊºÅ±ØĞëÓÉÓ¢ÎÄ×ÖÄ¸»òÊı×Ö£¬¶øÇÒÕÊºÅµÚÒ»¸ö×ÖÊÇÓ¢ÎÄ×ÖÄ¸!\n");*/
            flag=0;
        }
        if (flag) {
	    if(strlen(newuser.userid) < 2) {
                prints("´úºÅÖÁÉÙĞèÓĞÁ½¸öÓ¢ÎÄ×ÖÄ¸!\n");
            } else if ( (*newuser.userid == '\0') || bad_user_id( newuser.userid )){
                prints( "ÏµÍ³ÓÃ×Ö»òÊÇ²»ÑÅµÄ´úºÅ¡£\n" );
            } else if((usernum = searchuser( newuser.userid )) != 0) /*( dosearchuser( newuser.userid ) ) midified by dong , 1998.12.2, change getuser -> searchuser , 1999.10.26*/
	    {
                prints("´ËÕÊºÅÒÑ¾­ÓĞÈËÊ¹ÓÃ\n") ;
            } else {
		/*---	---*/
	      	struct stat lst;
		time_t lnow;
		lnow = time(NULL);
		sethomepath( genbuf, newuser.userid );
		if(!stat(genbuf, &lst) && S_ISDIR(lst.st_mode)
			&& (lnow - lst.st_ctime < SEC_DELETED_OLDHOME /* 3600*24*30 */) ) {
			prints("Ä¿Ç°ÎŞ·¨×¢²áÕÊºÅ%s£¬ÇëÓëÏµÍ³¹ÜÀíÈËÔ±ÁªÏµ¡£\n", newuser.userid);
			sprintf(genbuf, "IP %s new id %s failed[home changed in past 30 days]",
					fromhost, newuser.userid);
			report(genbuf);
		} else
		/*---	---*/
		break;
            }
	}
    }

    newuser.firstlogin = newuser.lastlogin = time(NULL) - 13 * 60 * 24 ;
    while( 1 ) {
        char  passbuf[ STRLEN ], passbuf2[ STRLEN ];
        getdata(0,0,"ÇëÉè¶¨ÄúµÄÃÜÂë: ",passbuf,39,NOECHO,NULL,YEA) ;
        if( strlen( passbuf ) < 4 || !strcmp( passbuf, newuser.userid ) ) {
            prints("ÃÜÂëÌ«¶Ì»òÓëÊ¹ÓÃÕß´úºÅÏàÍ¬, ÇëÖØĞÂÊäÈë\n") ;
            continue;
        }
        getdata(0,0,"ÇëÔÙÊäÈëÒ»´ÎÄãµÄÃÜÂë: ",passbuf2,39,NOECHO,NULL,YEA);
        if( strcmp( passbuf, passbuf2) != 0 ) {
            prints("ÃÜÂëÊäÈë´íÎó, ÇëÖØĞÂÊäÈëÃÜÂë.\n") ;
            continue;
        }
        
        setpasswd(passbuf,&newuser);
        break;
    }
    newuser.userlevel = PERM_BASIC;
    newuser.userdefine=-1;
/*   newuser.userdefine&=~DEF_MAILMSG;
    newuser.userdefine&=~DEF_EDITMSG; */
    newuser.userdefine &= ~DEF_NOTMSGFRIEND;
    if (convcode)
        newuser.userdefine&=~DEF_USEGB;

    newuser.notemode=-1;
    newuser.unuse1=-1;
    newuser.unuse2=-1;
    newuser.flags[0] = CURSOR_FLAG;
    newuser.flags[0] |= PAGER_FLAG;
    newuser.flags[1] = 0;
    newuser.firstlogin = newuser.lastlogin = time(NULL) ;

    allocid = getnewuserid2(newuser.userid)  ;
    if(allocid > MAXUSERS || allocid <= 0) {
        prints("No space for new users on the system!\n\r") ;
        oflush();
	    exit(1) ;
    }

	update_user(&newuser,allocid,1);

    if( !dosearchuser(newuser.userid) ) {
        /* change by KCN 1999.09.08
                fprintf(stderr,"User failed to create\n") ;
        */
        prints("User failed to create %d-%s\n",allocid,newuser.userid) ;
        oflush();
        exit(1) ;
    }
    report( "new account" );
}

int
invalid_realmail( userid, email, msize )
char    *userid, *email;
int     msize;
{
    FILE        *fn;
    char        *emailfile, ans[4],fname[STRLEN];

    if( (emailfile = sysconf_str( "EMAILFILE" )) == NULL )
        return 0;

    if( strchr( email, '@' ) && valid_ident( email ) )
        return 0;
    /*
        ansimore( emailfile, NA );
        getdata(t_lines-1,0,"ÄúÒªÏÖÔÚ email-post Âğ? (Y/N) [Y]: ",
            ans,2,DOECHO,NULL,YEA);
        while( *ans != 'n' && *ans != 'N' ) {
    */
    sprintf( fname, "tmp/email/%s", userid );
    if( (fn = fopen( fname, "r" )) != NULL ) {
        fgets( genbuf, STRLEN, fn );
        fclose( fn );
        strtok( genbuf, "\n" );
        if (!valid_ident( genbuf )) {
        } else if( strchr( genbuf, '@' ) != NULL ) {
            unlink(fname);
            strncpy( email, genbuf, msize );
            move( 10, 0 );
            prints( "¹§ºØÄú!! ÄúÒÑÍ¨¹ıÉí·İÑéÖ¤, ³ÉÎª±¾Õ¾¹«Ãñ. \n" );
            prints( "         ±¾Õ¾ÎªÄúËùÌá¹©µÄ¶îÍâ·şÎñ, \n" );
            prints( "         °üÀ¨Mail,Post,Message,Talk µÈ. \n" );
            prints( "  \n" );
            prints( "½¨ÒéÄú,  ÏÈËÄ´¦ä¯ÀÀÒ»ÏÂ, \n" );
            prints( "         ²»¶®µÄµØ·½, ÇëÔÚ sysop °åÁôÑÔ, \n" );
            prints( "         ±¾Õ¾»áÅÉ×¨ÈËÎªÄú½â´ğ. \n" );
            getdata( 18 ,0, "Çë°´ <Enter>  <<  ", ans,2,DOECHO,NULL ,YEA);
            return 0;
        }
    }
    return 1;
}

void
check_register_info()
{
    char        *newregfile;
    int         perm;
    time_t      code;
    FILE        *fin, *fout,*dp;
    char        ans[4],buf[STRLEN];
    extern int showansi;

    clear();
    sprintf(buf,"%s",email_domain());
    if( !(currentuser->userlevel & PERM_BASIC) ) {
        currentuser->userlevel = 0;
        return;
    }
    /*urec->userlevel |= PERM_DEFAULT;*/
    perm = PERM_DEFAULT & sysconf_eval( "AUTOSET_PERM" );

    /*    if( sysconf_str( "IDENTFILE" ) != NULL ) {  commented out by netty to save time */
    while ( strlen( currentuser->username ) < 2 ) {
        getdata( 2, 0, "ÇëÊäÈëÄúµÄêÇ³Æ:(ÀıÈç," DEFAULT_NICK ") << ", buf, NAMELEN,DOECHO,NULL ,YEA);
        strcpy(currentuser->username,buf);
        strcpy(uinfo.username,buf);
        UPDATE_UTMP_STR(username,uinfo);
    }
    if ( strlen( currentuser->realname ) < 2 ) {
        move( 3, 0 );
        prints( "ÇëÊäÈëÄúµÄÕæÊµĞÕÃû: (Õ¾³¤»á°ïÄú±£ÃÜµÄ !)\n" );
        getdata( 4, 0, "> ", buf, NAMELEN,DOECHO,NULL,YEA);
        strcpy(currentuser->realname,buf);
    }
    if ( strlen( currentuser->address ) < 6 ) {
        move( 5, 0 );
        prints( "ÄúÄ¿Ç°ÌîĞ´µÄµØÖ·ÊÇ¡®%s¡¯£¬³¤¶ÈĞ¡ÓÚ [1m[37m6[m£¬ÏµÍ³ÈÏÎªÆä¹ıÓÚ¼ò¶Ì¡£\n", currentuser->address[0] ? currentuser->address : "¿ÕµØÖ·" ); /* Leeward 98.04.26 */
        getdata( 6, 0, "ÇëÏêÏ¸ÌîĞ´ÄúµÄ×¡Ö·£º", buf, NAMELEN,DOECHO,NULL,YEA);
        strcpy(currentuser->address,buf);
    }
    if ( strchr( currentuser->email, '@' ) == NULL ) {
        move( 3, 0 );
        prints( "Ö»ÓĞ±¾Õ¾µÄºÏ·¨¹«Ãñ²ÅÄÜ¹»ÍêÈ«ÏíÓĞ¸÷ÖÖ¹¦ÄÜ£¬ \n" );
        /* alex           prints( "³ÉÎª±¾Õ¾ºÏ·¨¹«ÃñÓĞÁ½ÖÖ°ì·¨£º\n\n" );
                    prints( "1. Èç¹ûÄãÓĞºÏ·¨µÄemailĞÅÏä(·ÇBBS), \n");
                                prints( "       Äã¿ÉÒÔÓÃ»ØÈÏÖ¤ĞÅµÄ·½Ê½À´Í¨¹ıÈÏÖ¤¡£ \n\n" );
                    prints( "2. Èç¹ûÄãÃ»ÓĞemailĞÅÏä(·ÇBBS)£¬Äã¿ÉÒÔÔÚ½øÈë±¾Õ¾ÒÔºó£¬\n" );
                    prints( "       ÔÚ'¸öÈË¹¤¾ßÏä'ÄÚ ÏêÏ¸×¢²áÕæÊµÉí·İ£¬\n" );
                    prints( "       SYSOPs »á¾¡¿ì ¼ì²é²¢È·ÈÏÄãµÄ×¢²áµ¥¡£\n" );
                    move( 17, 0 );
                    prints( "µç×ÓĞÅÏä¸ñÊ½Îª: xxx@xxx.xxx.edu.cn \n" );
                    getdata( 18, 0, "ÇëÊäÈëµç×ÓĞÅÏä: (²»ÄÜÌá¹©Õß°´ <Enter>) << "
                                    , urec->email, STRLEN,DOECHO,NULL,YEA);
                    if ((strchr( urec->email, '@' ) == NULL )) { 
                        sprintf( genbuf, "%s.bbs@%s", urec->userid,buf );
                        strncpy( urec->email, genbuf, STRLEN);
                    }
        alex, ÒòÎªÈ¡ÏûÁËemail¹¦ÄÜ , 97.7 */
        prints( "³ÉÎª"NAME_BBS_NICK"ºÏ·¨"NAME_USER_SHORT"µÄ·½·¨ÈçÏÂ£º\n\n" );
        prints( "ÄãµÄÕÊºÅµÄµÚÒ»´ÎµÇÂ¼ºóµÄ "REGISTER_WAIT_TIME_NAME"ÄÚ£¨[1m[33m²»ÊÇÖ¸ÉÏ BBS "REGISTER_WAIT_TIME_NAME"[m£©£¬\n");
        prints( "    Äã´¦ÓÚĞÂÊÖÉÏÂ·ÆÚ¼ä, ÇëËÄ´¦²Î¹ÛÑ§Ï°ÍøÂçÊ¹ÓÃ·½·¨ºÍ¸÷ÖÖÀñÒÇ¡£\n");
        prints( "    ÔÚ´ËÆÚ¼ä£¬²»ÄÜ×¢²á³ÉÎªºÏ·¨"NAME_USER_SHORT"¡£\n\n");
        prints( "¹ıÁËÕâ¿ªÊ¼µÄ "REGISTER_WAIT_TIME_NAME", Äã¾Í¿ÉÒÔÔÚ½øÈë"NAME_BBS_NICK"ÒÔºó£¬\n" );
        prints( "    ÔÚ¡®¸öÈË¹¤¾ßÏä¡¯ÄÚÏêÏ¸×¢²áÕæÊµÉí·İ£¬\n" );
        prints( "    "NAME_SYSOP_GROUP"»á¾¡¿ì¼ì²é²¢È·ÈÏÄãµÄ×¢²áµ¥¡£\n\n" );
        /* Leeward adds below 98.04.26 */
        prints( "[1m[33mÈç¹ûÄãÒÑ¾­Í¨¹ıÁË×¢²á£¬³ÉÎªÁËºÏ·¨"NAME_USER_SHORT"£¬È´ÒÀÈ»»¹ÊÇ¿´µ½ÁË±¾ĞÅÏ¢£¬\nÄÇÊÇÒòÎªÄãÃ»ÓĞÔÚ¡®¸öÈË¹¤¾ßÏä¡¯ÄÚÉè¶¨¡®µç×ÓÓÊ¼şĞÅÏä¡¯¡£[m\nÇë´Ó¡®Ö÷Ñ¡µ¥¡¯½øÈë¡®¸öÈË¹¤¾ßÏä¡¯ÄÚ£¬ÔÙ½øÈë¡®Éè¶¨¸öÈË×ÊÁÏ¡¯Ò»Ïî½øĞĞÉè¶¨¡£\nÈç¹ûÄãÊµÔÚÃ»ÓĞÈÎºÎ¿ÉÓÃµÄ¡®µç×ÓÓÊ¼şĞÅÏä¡¯¿ÉÒÔÉè¶¨£¬ÓÖ²»Ô¸Òâ¿´µ½±¾ĞÅÏ¢£¬\n¿ÉÒÔÊ¹ÓÃ [1m[33m%s.bbs@smth.org[m ½øĞĞÉè¶¨¡£\n×¢Òâ¡ÃÉÏÃæ¸ø³öµÄµç×ÓÓÊ¼şĞÅÏä²»ÄÜ½ÓÊÕµç×ÓÓÊ¼ş£¬½ö½öÊÇÓÃÀ´Ê¹ÏµÍ³²»ÔÙÏÔÊ¾±¾ĞÅÏ¢¡£", currentuser->userid);
        pressreturn();
    }
    if(!strcmp(currentuser->userid,"SYSOP"))
    {
        currentuser->userlevel=~0;
        currentuser->userlevel&=~PERM_SUICIDE; /* Leeward 98.10.13 */
        currentuser->userlevel&=~PERM_DENYMAIL; /* Bigman 2000.9.22 */
    }
    if(!(currentuser->userlevel&PERM_LOGINOK))
    {
        if( HAS_PERM(currentuser, PERM_SYSOP ))
            return;
        if(!invalid_realmail( currentuser->userid, currentuser->realemail, STRLEN-16 ))
        {
            currentuser->userlevel |= PERM_DEFAULT;
            if( HAS_PERM(currentuser, PERM_DENYPOST ) && !HAS_PERM(currentuser, PERM_SYSOP ) )
                currentuser->userlevel &= ~PERM_POST;
        }else {
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
                       getdata( 19 ,0, "ÄúÒª SYSOP ¼ÄÕâÒ»·âĞÅÂğ?(Y/N) [Y] << ", ans,2,DOECHO,NULL,YEA);
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
                       getdata( 20 ,0, "ĞÅÒÑ¼Ä³ö, SYSOP ½«µÈÄú»ØĞÅÅ¶!! Çë°´ <Enter> << ", ans,2,DOECHO,NULL ,YEA);
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

            clear(); /* Leeward 98.05.14 */
            move( 12, 0 );
            prints( "Äã»¹Ã»ÓĞÍ¨¹ıÉí·İÈÏÖ¤£¬½«»áÃ»ÓĞtalk,mail,message,postÈ¨...  \n" );
            prints( "Èç¹ûÄãÒª³ÉÎª"NAME_BBS_NICK"µÄ×¢²á"NAME_USER_SHORT"£¬\n\n");
            prints( "ÇëÔÚ[31m¸öÈË¹¤¾ßÏä[0mÄÚ[31mÏêÏ¸×¢²áÉí·İ[0m\n" );
            prints( "ÄúÖ»ÒªÈ·ÊµÏêÏ¸ÌîĞ´ÁËÄãµÄÕæÊµÉí·İ×ÊÁÏ, \n");
            prints( "ÔÚ"NAME_SYSOP_GROUP"ÌæÄãÊÖ¹¤ÈÏÖ¤ÁËÒÔºó£¬¾Í¿ÉÒÔ³ÉÎª±¾Õ¾ºÏ¸ñ"NAME_USER_SHORT".\n" );
            pressreturn();
        }
        /* end of check if local email-addr */
        /*  above lines added by netty...  */
    }
    newregfile = sysconf_str( "NEWREGFILE" );
    if( currentuser->lastlogin - currentuser->firstlogin < REGISTER_WAIT_TIME &&
            !HAS_PERM(currentuser, PERM_SYSOP) && newregfile != NULL ) {
        currentuser->userlevel &= ~(perm);
        ansimore( newregfile, YEA );
    }
    if( HAS_PERM(currentuser, PERM_DENYPOST ) && !HAS_PERM(currentuser, PERM_SYSOP ) )
    {
        currentuser->userlevel &= ~PERM_POST;
    }
}

