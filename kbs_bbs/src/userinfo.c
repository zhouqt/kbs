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

extern time_t   login_start_time;
char    *sysconf_str();

void
disply_userinfo( u, real )
struct userec *u ;
int     real;
{
    struct stat st;
    int         num, diff;
    int         exp;

    move(real==1?2:3,0);
    clrtobot();
    prints("ÄúµÄ´úºÅ     : %s\n", u->userid);
    prints("ÄúµÄêÇ³Æ     : %s\n", u->username);
    prints("ÕæÊµĞÕÃû     : %s\n", u->realname);
    prints("¾Ó×¡×¡Ö·     : %s\n", u->address);
    prints("µç×ÓÓÊ¼şĞÅÏä : %s\n", u->email);
    if( real ) {
        prints("ÕæÊµ E-mail  : %s\n", u->realemail );
        prints("Ident ×ÊÁÏ   : %s\n", u->ident );
    }
    prints("×¢²áÈÕÆÚ     : %s", ctime( &u->firstlogin));
    prints("×î½ü¹âÁÙÈÕÆÚ : %s", ctime( &u->lastlogin));
    if( real ) {
        prints("×î½ü¹âÁÙ»úÆ÷ : %s\n", u->lasthost );
    }
    /*---	added by period		hide posts/logins	2000-11-02	---*/
    /*    if(HAS_PERM(currentuser,PERM_ADMINMENU)) {*/ /* removed to let user can see his own data */
    prints("ÉÏÕ¾´ÎÊı     : %d ´Î\n", u->numlogins);
    if(real) prints("ÎÄÕÂÊıÄ¿     : %d Æª\n", u->numposts);
    /*       if( real ) {
               prints("ÎÄÕÂÊıÄ¿     : %d / %d (Board/1Discuss)\n",
                  u->numposts, post_in_tin( u->userid ));
           }  removed by stephen 2000-11-02*/
    /*    }*/
    /* move these things, alex , 97.6
       exp=countexp(u);
       prints("¾­ÑéÖµ       : %d(%s)\n",exp,cexp(exp));
       exp=countperf(u);
       prints("±íÏÖÖµ       : %d(%s)\n",exp,cperf(exp));  */
    prints("ÉÏÕ¾×ÜÊ±Êı   : %d Ğ¡Ê± %d ·ÖÖÓ\n",u->stay/3600,(u->stay/60)%60);
    setmailfile(genbuf, u->userid, DOT_DIR);
    if( stat( genbuf, &st ) >= 0 )
        num = st.st_size / (sizeof( struct fileheader ));
    else
        num = 0;
    prints("Ë½ÈËĞÅÏä     : %d ·â\n", num );

    if( real ) {
        strcpy( genbuf, "bTCPRp#@XWBA$VS!DEM1234567890%" );
        for( num = 0; num < (int)strlen(genbuf); num++ )
            if( !(u->userlevel & (1 << num)) )
                genbuf[num] = '-';
        genbuf[num] = '\0';
        prints("Ê¹ÓÃÕßÈ¨ÏŞ   : %s\n", genbuf );
    } else {
        diff = (time(0) - login_start_time) / 60;
        prints("Í£ÁôÆÚ¼ä     : %d Ğ¡Ê± %02d ·Ö\n", diff / 60, diff % 60 );
        prints("ÆÁÄ»´óĞ¡     : %dx%d\n", t_lines, t_columns );
    }
    prints("\n");
    if( u->userlevel & PERM_LOGINOK ) {
        prints("  ÄúµÄ×¢²á³ÌĞòÒÑ¾­Íê³É, »¶Ó­¼ÓÈë±¾Õ¾.\n");
    } else if( u->lastlogin - u->firstlogin < 3 * 86400 ) {
        prints("  ĞÂÊÖÉÏÂ·, ÇëÔÄ¶Á Announce ÌÖÂÛÇø.\n" );
    } else {
        prints("  ×¢²áÉĞÎ´³É¹¦, Çë²Î¿¼±¾Õ¾½øÕ¾»­ÃæËµÃ÷.\n");
    }
}


int
uinfo_query( u, real, unum )
struct userec *u ;
int     real, unum;
{
    struct userec       newinfo;
    char        ans[3], buf[ STRLEN ],*emailfile,genbuf[STRLEN];
    int         i, fail = 0 ,netty_check  = 0;
    FILE        *fin, *fout,*dp;
    time_t      code;

    memcpy( &newinfo, u, sizeof(struct userec));
    getdata( t_lines-1, 0, real ?
             "ÇëÑ¡Ôñ (0)½áÊø (1)ĞŞ¸Ä×ÊÁÏ (2)Éè¶¨ÃÜÂë (3) ¸Ä ID ==> [0]" :
             "ÇëÑ¡Ôñ (0)½áÊø (1)ĞŞ¸Ä×ÊÁÏ (2)Éè¶¨ÃÜÂë ==> [0]",
             ans, 2, DOECHO, NULL,YEA);
    clear();
    refresh();

    i = 3;
    move( i++, 0 );
    if(ans[0]!='3'||real)
        prints("Ê¹ÓÃÕß´úºÅ: %s\n", u->userid );

    switch( ans[0] ) {
    case '1':
        move( 1, 0 );
        prints("ÇëÖğÏîĞŞ¸Ä,Ö±½Ó°´ <ENTER> ´ú±íÊ¹ÓÃ [] ÄÚµÄ×ÊÁÏ¡£\n");

        sprintf( genbuf, "êÇ³Æ [%s]: ", u->username );
        getdata( i++, 0, genbuf, buf, NAMELEN, DOECHO, NULL ,YEA);
        if( buf[0] ) strncpy( newinfo.username, buf, NAMELEN );

        sprintf( genbuf, "ÕæÊµĞÕÃû [%s]: ", u->realname );
        getdata( i++, 0, genbuf, buf, NAMELEN, DOECHO, NULL,YEA);
        if( buf[0] ) strncpy( newinfo.realname, buf, NAMELEN );

        sprintf( genbuf, "¾Ó×¡µØÖ· [%s]: ", u->address );
        getdata( i++, 0, genbuf, buf, STRLEN, DOECHO, NULL,YEA);
        if( buf[0] ) strncpy( newinfo.address, buf, NAMELEN );

        sprintf( genbuf, "µç×ÓĞÅÏä [%s]: ", u->email );
        getdata( i++, 0, genbuf, buf, STRLEN, DOECHO, NULL,YEA);
        if ( buf[0] )
        {
            /*netty_check = 1;*/
            /* È¡Ïûemail ÈÏÖ¤, alex , 97.7 */
            strncpy( newinfo.email, buf, STRLEN );

        }
        if( real ) {
            sprintf( genbuf, "ÕæÊµEmail[%s]: ", u->realemail );
            getdata( i++, 0, genbuf, buf, STRLEN, DOECHO, NULL ,YEA);
            if( buf[0] ) strncpy( newinfo.realemail, buf, STRLEN-16 );

            sprintf( genbuf, "ÉÏÏß´ÎÊı [%d]: ", u->numlogins );
            getdata( i++, 0, genbuf, buf, 16, DOECHO, NULL ,YEA);
            if( atoi( buf ) > 0 ) newinfo.numlogins = atoi( buf );

            sprintf( genbuf, "ÎÄÕÂÊıÄ¿ [%d]: ", u->numposts );
            getdata( i++, 0, genbuf, buf, 16, DOECHO, NULL ,YEA);
            {
                int lres;
                lres = atoi(buf);
                if(lres > 0 || ('\0' == buf[1] && '0' == *buf))
                    newinfo.numposts = lres;
            }
            /*            if( atoi( buf ) > 0 ) newinfo.numposts = atoi( buf );*/

            sprintf( genbuf, "½«×¢²áÈÕÆÚÌáÇ°"REGISTER_WAIT_TIME_NAME" [Y/N]");
            getdata( i++, 0, genbuf, buf, 16, DOECHO, NULL, YEA);
            if(buf[0]=='y'||buf[0]=='Y') newinfo.firstlogin-=REGISTER_WAIT_TIME;

            sprintf( genbuf, "½«×î½ü¹âÁÙÈÕÆÚÉèÖÃÎª½ñÌìÂğ£¿[Y/N]");
            getdata( i++, 0, genbuf, buf, 16, DOECHO, NULL, YEA);
            if(buf[0]=='y'||buf[0]=='Y') newinfo.lastlogin=time(0);

        }

        break;
    case '3':
        if( ! real ) {
            clear();
            return 0;
        }
        /* Bigman 2000.10.2 ĞŞ¸ÄÊ¹ÓÃÕßIDÎ»Êı²»¹» */
        getdata(i++,0,"ĞÂµÄÊ¹ÓÃÕß´úºÅ: ",genbuf,IDLEN+1,DOECHO,NULL,YEA);
        if(*genbuf != '\0') {
            if(searchuser(genbuf)) {
                prints("\n´íÎó! ÒÑ¾­ÓĞÍ¬Ñù ID µÄÊ¹ÓÃÕß\n") ;
                fail++;
                break;
            } else {
                strncpy(newinfo.userid, genbuf,IDLEN+2) ;
            }
        } else {
            break;
        }
        /* fall throw, must change passwd for newid, by wwj 2001/5/7 */
        
    case '2':
        if( ! real ) {
            getdata(i++,0,"ÇëÊäÈëÔ­ÃÜÂë: ",buf,39,NOECHO,NULL,YEA);
            if( *buf == '\0' || !checkpasswd2( buf,u)) {
                prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
                fail++;
                break;
            }
        }
        getdata(i++,0,"ÇëÉè¶¨ĞÂÃÜÂë: ",buf,39,NOECHO,NULL,YEA);
        if( buf[0] == '\0' ) {
            prints("\n\nÃÜÂëÉè¶¨È¡Ïû, ¼ÌĞøÊ¹ÓÃ¾ÉÃÜÂë\n");
            fail++;
            break;
        }
        getdata(i++,0,"ÇëÖØĞÂÊäÈëĞÂÃÜÂë: ",genbuf,39,NOECHO,NULL,YEA);
        if(strcmp(buf,genbuf)) {
            prints("\n\nÁ½¸öÃÜÂë²»Ò»ÖÂ, ÎŞ·¨Éè¶¨ĞÂÃÜÂë¡£\n");
            fail++;
            break;
        }
        /*	Added by cityhunter to deny others to modify SYSOP's passwd */
        if( real && (strcmp(u->userid,"SYSOP") ==0) )
        {
            prints("\n\n´íÎó!ÏµÍ³½ûÖ¹ĞŞ¸ÄSYSOPµÄÃÜÂë,"NAME_POLICE"ÕıÔÚÀ´µÄÂ·ÉÏ :)");
            pressreturn();
            clear();
            return 0;
        }
        /* end of this addin */
        setpasswd(buf,&newinfo);
        break;
        
    default:
        clear();
        return 0;
    }
    if( fail != 0 ) {
        pressreturn();
        clear();
        return 0;
    }
    for(;;)
    {
        getdata(t_lines-1,0,"È·¶¨Òª¸Ä±äÂğ?  (Yes or No): ",ans,2,DOECHO,NULL,YEA);
        if (*ans=='n'||*ans=='N') break;
        if( *ans == 'y' || *ans == 'Y' ) {
            if(real)
            {
                char        secu[STRLEN];
                if(strcmp(u->userid, newinfo.userid ))
                    sprintf(secu,"%s µÄ ID ±» %s ¸ÄÎª %s",u->userid,currentuser->userid,newinfo.userid);/*Haohmaru.99.5.6*/
                else
	                sprintf(secu,"ĞŞ¸Ä %s µÄ»ù±¾×ÊÁÏ»òÃÜÂë¡£",u->userid);
                securityreport(secu,&newinfo);
            } else {
                update_user(&newinfo,usernum,1); 
                if ( strcmp(newinfo.username,uinfo.username)) {
                    strcpy(uinfo.username,newinfo.username);
                    UPDATE_UTMP_STR(username,uinfo);
                }
            }
            if( strcmp( u->userid, newinfo.userid ) ) {
                char src[ STRLEN ], dst[ STRLEN ];

                setmailpath( src, u->userid );
                setmailpath( dst, newinfo.userid );
		/*
                sprintf(genbuf,"mv %s %s",src, dst);
		*/
		f_mv(src,dst);
                sethomepath( src, u->userid );
                sethomepath( dst, newinfo.userid );
		/*
                sprintf(genbuf,"mv %s %s",src ,dst);
		*/
		f_mv(src,dst);
                sprintf(src,"tmp/email/%s",u->userid);
                unlink(src);
                setuserid( unum, newinfo.userid );
            }
            /* added by netty to automatically send a mail to new user. */

            if ((netty_check == 1))
            {
                if((strchr( newinfo.email, '@' ) != NULL ) &&
                        (!strstr( newinfo.email, "@firebird.cs") ) &&
                        (!strstr( newinfo.email, "@bbs.") ) &&
                        (!invalidaddr(newinfo.email) ) &&
                        (!strstr( newinfo.email, ".bbs@") )) {
                    if( (emailfile = sysconf_str( "EMAILFILE" )) != NULL )
                    {
                        code=(time(0)/2)+(rand()/10);
                        sethomefile(genbuf, u->userid, "mailcheck");
                        if((dp=fopen(genbuf,"w"))==NULL)
                        {
                            fclose(dp);
                            return -1;
                        }
                        fprintf(dp,"%9.9d\n",code);
                        fclose(dp);
                        sprintf( genbuf, "/usr/lib/sendmail -f SYSOP@%s %s ",
                                 email_domain(), newinfo.email );
                        fout = popen( genbuf, "w" );
                        fin  = fopen( emailfile, "r" );
                        if (fin == NULL || fout == NULL) return -1;
                        fprintf( fout, "Reply-To: SYSOP@%s\n", email_domain());
                        fprintf( fout, "From: SYSOP@%s\n",  email_domain() );
                        fprintf( fout, "To: %s\n", newinfo.email);
                        fprintf( fout, "Subject: @%s@[-%9.9d-]firebird mail check.\n", u->userid,code );
                        fprintf( fout, "X-Forwarded-By: SYSOP \n" );
                        fprintf( fout, "X-Disclaimer: None\n");
                        fprintf( fout, "\n");
                        fprintf(fout,"ÄúµÄ»ù±¾×ÊÁÏÈçÏÂ£º\n",u->userid);
                        fprintf(fout,"Ê¹ÓÃÕß´úºÅ£º%s (%s)\n",u->userid,u->username);
                        fprintf(fout,"ĞÕ      Ãû£º%s\n",u->realname);
                        fprintf(fout,"ÉÏÕ¾Î»ÖÃ  £º%s\n",u->lasthost);
                        fprintf(fout,"µç×ÓÓÊ¼ş  £º%s\n\n",u->email);
                        fprintf(fout,"Ç×°®µÄ %s(%s):\n",u->userid,u->username);

                        while (fgets( genbuf, 255, fin ) != NULL ) {
                            if (genbuf[0] == '.' && genbuf[ 1 ] == '\n')
                                fputs( ". \n", fout );
                            else fputs( genbuf, fout );
                        }
                        fprintf(fout, ".\n");
                        fclose( fin );
                        pclose( fout );
                    }
                }else
                {
                    if(sysconf_str( "EMAILFILE" )!=NULL)
                    {
                        move(t_lines-5,0);
                        prints("\nÄãµÄµç×ÓÓÊ¼şµØÖ· ¡¾[33m%s[m¡¿\n",newinfo.email);
                        prints("²¢·Ç Unix ÕÊºÅ£¬ÏµÍ³²»»áÍ¶µİÉí·İÈ·ÈÏĞÅ£¬Çëµ½[32m¹¤¾ßÏä[mÖĞĞŞ¸Ä..\n");
                        pressanykey();
                    }
                }
            }
            update_user(&newinfo,unum,1);
            break;/*Haohmaru.98.01.10.faint...Luzi¼Ó¸öforÑ­»·Ò²²»break!*/
        }
    }clear();
    return 0;
}

void
x_info()
{
    modify_user_mode( GMENU );
    disply_userinfo( currentuser, 1 );
    if (!strcmp("guest", currentuser->userid)) {
        pressreturn();
        return;
    }
    uinfo_query( currentuser, 0, usernum );
}

void
getfield( line, info, desc, buf, len )
int     line, len;
char    *info, *desc, *buf;
{
    char        prompt[ STRLEN ];

    /*    sprintf( genbuf, "  Ô­ÏÈÉè¶¨: %-46.46s (%s)", buf, info ); */
    sprintf( genbuf, "  Ô­ÏÈÉè¶¨: %-20.20s (%s)", buf, info );
    move( line, 0 );
    prints( genbuf );
    sprintf( prompt, "  %s: ", desc );
    getdata( line+1, 0, prompt, genbuf, len, DOECHO, NULL ,YEA);
    if( genbuf[0] != '\0' ) {
        strncpy( buf, genbuf, len );
    }
    move( line, 0 );
    clrtoeol();
    prints( "  %s: %s\n", desc, buf );
    clrtoeol();
}

void
x_fillform()
{
    char        rname[ NAMELEN ], addr[ STRLEN ];
    char        phone[ STRLEN ], career[ STRLEN ],birth[ STRLEN];
    char        ans[5], *mesg, *ptr;
    FILE        *fn;
    time_t      now;

    modify_user_mode(NEW);
    move( 3, 0 );
    clrtobot();

    if (!strcmp("guest", currentuser->userid)) {
        prints( "±§Ç¸, ÇëÓÃ new ÉêÇëÒ»¸öĞÂÕÊºÅááÔÙÌîÉêÇë±í." );
        pressreturn();
        return;
    }
    if( currentuser->userlevel & PERM_LOGINOK ) {
        prints( "ÄúµÄÉí·İÈ·ÈÏÒÑ¾­³É¹¦, »¶Ó­¼ÓÈë±¾Õ¾µÄĞĞÁĞ." );
        pressreturn();
        return;
    }
    if ((time(0)-currentuser->firstlogin) < REGISTER_WAIT_TIME)
    {
        prints( "ÄúÊ×´ÎµÇÈë±¾Õ¾Î´Âú"REGISTER_WAIT_TIME_NAME"..." );
        prints( "ÇëÏÈËÄ´¦ÊìÏ¤Ò»ÏÂ£¬ÔÚÂú"REGISTER_WAIT_TIME_NAME"ÒÔºóÔÙÌîĞ´×¢²áµ¥¡£");
        pressreturn();
        return;
    }

    /*prints("Òò×ªÕ¾²»½ÓÊÕĞÂ×¢²áÓÃ»§ÌîĞ´×¢²áµ¥; 8 ºÅ»Ö¸´±¾·şÎñ¡£ÇëµÈºò¡£");
    pressreturn();
    return;*/


    if( (fn = fopen( "new_register", "r" )) != NULL ) {
        while( fgets( genbuf, STRLEN, fn ) != NULL ) {
            if( (ptr = strchr( genbuf, '\n' )) != NULL )
                *ptr = '\0';
            if( strncmp( genbuf, "userid: ", 8 ) == 0 &&
                    strcmp( genbuf + 8, currentuser->userid ) == 0 ) {
                fclose( fn );
                prints( "Õ¾³¤ÉĞÎ´´¦ÀíÄúµÄ×¢²áÉêÇëµ¥, ÇëÄÍĞÄµÈºò." );
                pressreturn();
                return;
            }
        }
        fclose( fn );
    }
    /* added by KCN 1999.10.25 */
    ansimore("etc/register.note",NA);
    getdata(t_lines-1,0,"ÄúÈ·¶¨ÒªÌîĞ´×¢²áµ¥Âğ (Y/N)? [N]: ",ans,3,DOECHO,NULL,YEA);
    if( ans[0] != 'Y' && ans[0] != 'y' )
        return;
    strncpy( rname, currentuser->realname, NAMELEN );
    strncpy( addr,  currentuser->address,  STRLEN  );
    career[0] = phone[0] = birth[0]='\0';
    clear();
    while( 1 ) {
        move( 3, 0 );
        clrtoeol();
        prints( "%s ÄúºÃ, Çë¾İÊµÌîĞ´ÒÔÏÂµÄ×ÊÁÏ(ÇëÊ¹ÓÃÖĞÎÄ):\n", currentuser->userid );
        genbuf[0] = '\0';/*Haohmaru.99.09.17.ÒÔÏÂÄÚÈİ²»µÃ¹ı¶Ì*/
        while ( strlen( genbuf ) < 3 ) {
            getfield(  6, "ÇëÓÃÖĞÎÄ,²»ÄÜÊäÈëµÄºº×ÖÇëÓÃÆ´Òô", "ÕæÊµĞÕÃû", rname, NAMELEN ); }
        genbuf[0] = '\0';
        while ( strlen( genbuf ) < 2 ) {
            getfield(  8, "Ñ§Ğ£Ïµ¼¶»òµ¥Î»È«³Æ", "·şÎñµ¥Î»", career,STRLEN ); }
        genbuf[0] = '\0';
        while ( strlen( genbuf ) < 6 ) {
            getfield( 10, "Çë¾ßÌåµ½ÇŞÊÒ»òÃÅÅÆºÅÂë", "Ä¿Ç°×¡Ö·", addr,  STRLEN ); }
        genbuf[0] = '\0';
        while ( strlen( genbuf ) < 2 ) {
            getfield( 12, "°üÀ¨¿ÉÁ¬ÂçÊ±¼ä,ÈôÎŞ¿ÉÓÃºô»ú»òEmailµØÖ·´úÌæ",     "Á¬Âçµç»°", phone, STRLEN ); }
        getfield( 14, "Äê.ÔÂ.ÈÕ(¹«Ôª)",     "³öÉúÄêÔÂ", birth, STRLEN );
        mesg = "ÒÔÉÏ×ÊÁÏÊÇ·ñÕıÈ·, °´ Q ·ÅÆú×¢²á (Y/N/Quit)? [N]: ";
        getdata(t_lines-1,0,mesg,ans,3,DOECHO,NULL,YEA);
        if( ans[0] == 'Q' || ans[0] == 'q' )
            return;
        if( ans[0] == 'Y' || ans[0] == 'y' )
            break;
    }
    strncpy( currentuser->realname, rname,  NAMELEN );
    strncpy( currentuser->address,  addr,   STRLEN  );
    if( (fn = fopen( "new_register", "a" )) != NULL ) {
        now = time( NULL );
        fprintf( fn, "usernum: %d, %s", usernum, ctime( &now ) );
        fprintf( fn, "userid: %s\n",    currentuser->userid );
        fprintf( fn, "realname: %s\n",  rname );
        fprintf( fn, "career: %s\n",    career );
        fprintf( fn, "addr: %s\n",      addr );
        fprintf( fn, "phone: %s\n",     phone );
        fprintf( fn, "birth: %s\n",     birth);
        fprintf( fn, "----\n" );
        fclose( fn );
    }
}

