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

extern time_t   login_start_time;
char *genpasswd() ;
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
    prints("ƒ˙µƒ¥˙∫≈     : %s\n", u->userid);
    prints("ƒ˙µƒÍ«≥∆     : %s\n", u->username);
    prints("’Ê µ–’√˚     : %s\n", u->realname);
    prints("æ”◊°◊°÷∑     : %s\n", u->address);
    prints("µÁ◊”” º˛–≈œ‰ : %s\n", u->email);
    if( real ) {
        prints("’Ê µ E-mail  : %s\n", u->termtype + 16 );
        prints("Ident ◊ ¡œ   : %s\n", u->ident );
    }
    prints("÷’∂Àª˙–ŒÃ¨   : %s\n", u->termtype );
    prints("◊¢≤·»’∆⁄     : %s", ctime( &u->firstlogin));
    prints("◊ÓΩ¸π‚¡Ÿ»’∆⁄ : %s", ctime( &u->lastlogin));
    if( real ) {
        prints("◊ÓΩ¸π‚¡Ÿª˙∆˜ : %s\n", u->lasthost );
    }
    /*---	added by period		hide posts/logins	2000-11-02	---*/
    /*    if(HAS_PERM(PERM_ADMINMENU)) {*/ /* removed to let user can see his own data */
    prints("…œ’æ¥Œ ˝     : %d ¥Œ\n", u->numlogins);
    if(real) prints("Œƒ’¬ ˝ƒø     : %d ∆™\n", u->numposts);
    /*       if( real ) {
               prints("Œƒ’¬ ˝ƒø     : %d / %d (Board/1Discuss)\n",
                  u->numposts, post_in_tin( u->userid ));
           }  removed by stephen 2000-11-02*/
    /*    }*/
    /* move these things, alex , 97.6
       exp=countexp(u);
       prints("æ≠—È÷µ       : %d(%s)\n",exp,cexp(exp));
       exp=countperf(u);
       prints("±Ìœ÷÷µ       : %d(%s)\n",exp,cperf(exp));  */
    prints("…œ’æ◊‹ ± ˝   : %d –° ± %d ∑÷÷”\n",u->stay/3600,(u->stay/60)%60);
    setmailfile(genbuf, u->userid, DOT_DIR);
    if( stat( genbuf, &st ) >= 0 )
        num = st.st_size / (sizeof( struct fileheader ));
    else
        num = 0;
    prints("ÀΩ»À–≈œ‰     : %d ∑‚\n", num );

    if( real ) {
        strcpy( genbuf, "bTCPRp#@XWBA$VS!DEM1234567890%" );
        for( num = 0; num < strlen(genbuf); num++ )
            if( !(u->userlevel & (1 << num)) )
                genbuf[num] = '-';
        genbuf[num] = '\0';
        prints(" π”√’ﬂ»®œﬁ   : %s\n", genbuf );
    } else {
        diff = (time(0) - login_start_time) / 60;
        prints("Õ£¡Ù∆⁄º‰     : %d –° ± %02d ∑÷\n", diff / 60, diff % 60 );
        prints("∆¡ƒª¥Û–°     : %dx%d\n", t_lines, t_columns );
    }
    prints("\n");
    if( u->userlevel & PERM_LOGINOK ) {
        prints("  ƒ˙µƒ◊¢≤·≥Ã–Ú“—æ≠ÕÍ≥…, ª∂”≠º”»Î±æ’æ.\n");
    } else if( u->lastlogin - u->firstlogin < 3 * 86400 ) {
        prints("  –¬ ÷…œ¬∑, «Î‘ƒ∂¡ Announce Ã÷¬€«¯.\n" );
    } else {
        prints("  ◊¢≤·…–Œ¥≥…π¶, «Î≤Œøº±æ’æΩ¯’æª≠√ÊÀµ√˜.\n");
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

    memcpy( &newinfo, u, sizeof(currentuser));
    getdata( t_lines-1, 0, real ?
             "«Î—°‘Ò (0)Ω· ¯ (1)–ﬁ∏ƒ◊ ¡œ (2)…Ë∂®√‹¬Î (3) ∏ƒ ID ==> [0]" :
             "«Î—°‘Ò (0)Ω· ¯ (1)–ﬁ∏ƒ◊ ¡œ (2)…Ë∂®√‹¬Î ==> [0]",
             ans, 2, DOECHO, NULL,YEA);
    clear();
    refresh();

    i = 3;
    move( i++, 0 );
    if(ans[0]!='3'||real)
        prints(" π”√’ﬂ¥˙∫≈: %s\n", u->userid );

    switch( ans[0] ) {
    case '1':
        move( 1, 0 );
        prints("«Î÷œÓ–ﬁ∏ƒ,÷±Ω”∞¥ <ENTER> ¥˙±Ì π”√ [] ƒ⁄µƒ◊ ¡œ°£\n");

        sprintf( genbuf, "Í«≥∆ [%s]: ", u->username );
        getdata( i++, 0, genbuf, buf, NAMELEN, DOECHO, NULL ,YEA);
        if( buf[0] ) strncpy( newinfo.username, buf, NAMELEN );
        if(!real && buf[0]) strncpy(uinfo.username,buf,40);

        sprintf( genbuf, "’Ê µ–’√˚ [%s]: ", u->realname );
        getdata( i++, 0, genbuf, buf, NAMELEN, DOECHO, NULL,YEA);
        if( buf[0] ) strncpy( newinfo.realname, buf, NAMELEN );

        sprintf( genbuf, "æ”◊°µÿ÷∑ [%s]: ", u->address );
        getdata( i++, 0, genbuf, buf, STRLEN, DOECHO, NULL,YEA);
        if( buf[0] ) strncpy( newinfo.address, buf, NAMELEN );

        sprintf( genbuf, "µÁ◊”–≈œ‰ [%s]: ", u->email );
        getdata( i++, 0, genbuf, buf, STRLEN, DOECHO, NULL,YEA);
        if ( buf[0] )
        {
            /*netty_check = 1;*/
            /* »°œ˚email »œ÷§, alex , 97.7 */
            strncpy( newinfo.email, buf, STRLEN );

        }
        sprintf( genbuf, "÷’∂Àª˙–ŒÃ¨ [%s]: ", u->termtype );
        getdata( i++, 0, genbuf, buf, 16, DOECHO, NULL ,YEA);
        if( buf[0] ) strncpy( newinfo.termtype, buf, 16 );

        if( real ) {
            sprintf( genbuf, "’Ê µEmail[%s]: ", u->termtype+16 );
            getdata( i++, 0, genbuf, buf, STRLEN, DOECHO, NULL ,YEA);
            if( buf[0] ) strncpy( newinfo.termtype+16, buf, STRLEN-16 );

            sprintf( genbuf, "…œœﬂ¥Œ ˝ [%d]: ", u->numlogins );
            getdata( i++, 0, genbuf, buf, 16, DOECHO, NULL ,YEA);
            if( atoi( buf ) > 0 ) newinfo.numlogins = atoi( buf );

            sprintf( genbuf, "Œƒ’¬ ˝ƒø [%d]: ", u->numposts );
            getdata( i++, 0, genbuf, buf, 16, DOECHO, NULL ,YEA);
            {
                int lres;
                lres = atoi(buf);
                if(lres > 0 || ('\0' == buf[1] && '0' == *buf))
                    newinfo.numposts = lres;
            }
            /*            if( atoi( buf ) > 0 ) newinfo.numposts = atoi( buf );*/

            sprintf( genbuf, "Ω´◊¢≤·»’∆⁄Ã·«∞»˝ÃÏ [Y/N]");
            getdata( i++, 0, genbuf, buf, 16, DOECHO, NULL, YEA);
            if(buf[0]=='y'||buf[0]=='Y') newinfo.firstlogin-=3*86400;

            sprintf( genbuf, "Ω´◊ÓΩ¸π‚¡Ÿ»’∆⁄…Ë÷√Œ™ΩÒÃÏ¬£ø[Y/N]");
            getdata( i++, 0, genbuf, buf, 16, DOECHO, NULL, YEA);
            if(buf[0]=='y'||buf[0]=='Y') newinfo.lastlogin=time(0);

        }

        break;
    case '2':
        if( ! real ) {
            getdata(i++,0,"«Î ‰»Î‘≠√‹¬Î: ",buf,PASSLEN,NOECHO,NULL,YEA);
            if( *buf == '\0' || !checkpasswd( u->passwd, buf )) {
                prints("\n\n∫‹±ß«∏, ƒ˙ ‰»Îµƒ√‹¬Î≤ª’˝»∑°£\n");
                fail++;
                break;
            }
        }
        getdata(i++,0,"«Î…Ë∂®–¬√‹¬Î: ",buf,PASSLEN,NOECHO,NULL,YEA);
        if( buf[0] == '\0' ) {
            prints("\n\n√‹¬Î…Ë∂®»°œ˚, ºÃ–¯ π”√æ…√‹¬Î\n");
            fail++;
            break;
        }
        strncpy(genbuf,buf,PASSLEN) ;

        getdata(i++,0,"«Î÷ÿ–¬ ‰»Î–¬√‹¬Î: ",buf,PASSLEN,NOECHO,NULL,YEA);
        if(strncmp(buf,genbuf,PASSLEN)) {
            prints("\n\n–¬√‹¬Î»∑»œ ß∞‹, Œﬁ∑®…Ë∂®–¬√‹¬Î°£\n");
            fail++;
            break;
        }
        buf[8] = '\0';
        /*	Added by cityhunter to deny others to modify SYSOP's passwd */
        if( real && (strcmp(u->userid,"SYSOP") ==0) )
        {
            prints("\n\n¥ÌŒÛ!œµÕ≥Ω˚÷π–ﬁ∏ƒSYSOPµƒ√‹¬Î,æØ≤Ï’˝‘⁄¿¥µƒ¬∑…œ :)");
            pressreturn();
            clear();
            return 0;
        }
        /* end of this addin */
        strncpy( newinfo.passwd, genpasswd( buf ), PASSLEN );
        break;
    case '3':
        if( ! real ) {
            clear();
            return 0;
        }
        /* Bigman 2000.10.2 –ﬁ∏ƒ π”√’ﬂIDŒª ˝≤ªπª */
        getdata(i++,0,"–¬µƒ π”√’ﬂ¥˙∫≈: ",genbuf,IDLEN+1,DOECHO,NULL,YEA);
        if(*genbuf != '\0') {
            if(getuser(genbuf)) {
                prints("\n¥ÌŒÛ! “—æ≠”–Õ¨—˘ ID µƒ π”√’ﬂ\n") ;
                fail++;
            } else {
                strncpy(newinfo.userid, genbuf,IDLEN+2) ;
            }
        }
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
        getdata(t_lines-1,0,"»∑∂®“™∏ƒ±‰¬?  (Yes or No): ",ans,2,DOECHO,NULL,YEA);
        if (*ans=='n'||*ans=='N') break;
        if( *ans == 'y' || *ans == 'Y' ) {
            if(real)
            {
                char        secu[STRLEN];
                sprintf(secu,"–ﬁ∏ƒ %s µƒª˘±æ◊ ¡œªÚ√‹¬Î°£",u->userid);
                if(strcmp(u->userid, newinfo.userid ))
                    sprintf(secu,"%s µƒ ID ±ª %s ∏ƒŒ™ %s",u->userid,currentuser.userid,newinfo.userid);/*Haohmaru.99.5.6*/
                securityreport(secu);
            }
            if( strcmp( u->userid, newinfo.userid ) ) {
                char src[ STRLEN ], dst[ STRLEN ];

                setmailpath( src, u->userid );
                setmailpath( dst, newinfo.userid );
                sprintf(genbuf,"mv %s %s",src, dst);
                system(genbuf);/*
                       rename( src, dst );*/
                sethomepath( src, u->userid );
                sethomepath( dst, newinfo.userid );
                sprintf(genbuf,"mv %s %s",src ,dst);
                system(genbuf);/*
                       rename( src, dst );*/
                sprintf(src,"tmp/email_%s",u->userid);
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
                            return;
                        }
                        fprintf(dp,"%9.9d\n",code);
                        fclose(dp);
                        sprintf( genbuf, "/usr/lib/sendmail -f SYSOP.bbs@%s %s ",
                                 email_domain(), newinfo.email );
                        fout = popen( genbuf, "w" );
                        fin  = fopen( emailfile, "r" );
                        if (fin == NULL || fout == NULL) return -1;
                        fprintf( fout, "Reply-To: SYSOP.bbs@%s\n", email_domain());
                        fprintf( fout, "From: SYSOP.bbs@%s\n",  email_domain() );
                        fprintf( fout, "To: %s\n", newinfo.email);
                        fprintf( fout, "Subject: @%s@[-%9.9d-]firebird mail check.\n", u->userid,code );
                        fprintf( fout, "X-Forwarded-By: SYSOP \n" );
                        fprintf( fout, "X-Disclaimer: None\n");
                        fprintf( fout, "\n");
                        fprintf(fout,"ƒ˙µƒª˘±æ◊ ¡œ»Áœ¬£∫\n",u->userid);
                        fprintf(fout," π”√’ﬂ¥˙∫≈£∫%s (%s)\n",u->userid,u->username);
                        fprintf(fout,"–’      √˚£∫%s\n",u->realname);
                        fprintf(fout,"…œ’æŒª÷√  £∫%s\n",u->lasthost);
                        fprintf(fout,"µÁ◊”” º˛  £∫%s\n\n",u->email);
                        fprintf(fout,"«◊∞Æµƒ %s(%s):\n",u->userid,u->username);

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
                        prints("\nƒ„µƒµÁ◊”” º˛µÿ÷∑ °æ[33m%s[m°ø\n",newinfo.email);
                        prints("≤¢∑« Unix ’ ∫≈£¨œµÕ≥≤ªª·Õ∂µ›…Ì∑›»∑»œ–≈£¨«ÎµΩ[32mπ§æﬂœ‰[m÷––ﬁ∏ƒ..\n");
                        pressanykey();
                    }
                }
            }
            memcpy( u, &newinfo, sizeof(newinfo) );
            set_safe_record();
            substitute_record( PASSFILE, &newinfo, sizeof(newinfo), unum );
            break;/*Haohmaru.98.01.10.faint...Luziº”∏ˆfor—≠ª∑“≤≤ªbreak!*/
        }
    }clear();
    return 0;
}

void
x_info()
{
    modify_user_mode( GMENU );
    disply_userinfo( &currentuser, 1 );
    if (!strcmp("guest", currentuser.userid)) {
        pressreturn();
        return;
    }
    uinfo_query( &currentuser, 0, usernum );
}

void
getfield( line, info, desc, buf, len )
int     line, len;
char    *info, *desc, *buf;
{
    char        prompt[ STRLEN ];

    /*    sprintf( genbuf, "  ‘≠œ»…Ë∂®: %-46.46s (%s)", buf, info ); */
    sprintf( genbuf, "  ‘≠œ»…Ë∂®: %-20.20s (%s)", buf, info );
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

    if (!strcmp("guest", currentuser.userid)) {
        prints( "±ß«∏, «Î”√ new …Í«Î“ª∏ˆ–¬’ ∫≈··‘ŸÃÓ…Í«Î±Ì." );
        pressreturn();
        return;
    }
    if( currentuser.userlevel & PERM_LOGINOK ) {
        prints( "ƒ˙µƒ…Ì∑›»∑»œ“—æ≠≥…π¶, ª∂”≠º”»Î±æ’æµƒ––¡–." );
        pressreturn();
        return;
    }
    if ((time(0)-currentuser.firstlogin) < 3*86400)
    {
        prints( "ƒ˙ ◊¥Œµ«»Î±æ’æŒ¥¬˙3ÃÏ(72∏ˆ–° ±)..." );
        prints( "«Îœ»Àƒ¥¶ Ïœ§“ªœ¬£¨‘⁄¬˙3ÃÏ“‘∫Û‘ŸÃÓ–¥◊¢≤·µ•°£");
        pressreturn();
        return;
    }

    /*prints("“Ú◊™’æ≤ªΩ” ’–¬◊¢≤·”√ªßÃÓ–¥◊¢≤·µ•; 8 ∫≈ª÷∏¥±æ∑˛ŒÒ°£«Îµ»∫Ú°£");
    pressreturn();
    return;*/


    if( (fn = fopen( "new_register", "r" )) != NULL ) {
        while( fgets( genbuf, STRLEN, fn ) != NULL ) {
            if( (ptr = strchr( genbuf, '\n' )) != NULL )
                *ptr = '\0';
            if( strncmp( genbuf, "userid: ", 8 ) == 0 &&
                    strcmp( genbuf + 8, currentuser.userid ) == 0 ) {
                fclose( fn );
                prints( "’æ≥§…–Œ¥¥¶¿Ìƒ˙µƒ◊¢≤·…Í«Îµ•, «ÎƒÕ–ƒµ»∫Ú." );
                pressreturn();
                return;
            }
        }
        fclose( fn );
    }
    /* added by KCN 1999.10.25 */
    ansimore("etc/register.note",NA);
    getdata(t_lines-1,8,"ƒ˙»∑∂®“™ÃÓ–¥◊¢≤·µ•¬ (Y/N)? [N]: ",ans,3,DOECHO,NULL,YEA);
    if( ans[0] != 'Y' && ans[0] != 'y' )
        return;
    strncpy( rname, currentuser.realname, NAMELEN );
    strncpy( addr,  currentuser.address,  STRLEN  );
    career[0] = phone[0] = birth[0]='\0';
    clear();
    while( 1 ) {
        move( 3, 0 );
        clrtoeol();
        prints( "%s ƒ˙∫√, «Îæ› µÃÓ–¥“‘œ¬µƒ◊ ¡œ(«Î π”√÷–Œƒ):\n", currentuser.userid );
        genbuf[0] = '\0';/*Haohmaru.99.09.17.“‘œ¬ƒ⁄»›≤ªµ√π˝∂Ã*/
        while ( strlen( genbuf ) < 3 ) {
            getfield(  6, "«Î”√÷–Œƒ,≤ªƒ‹ ‰»Îµƒ∫∫◊÷«Î”√∆¥“Ù", "’Ê µ–’√˚", rname, NAMELEN ); }
        genbuf[0] = '\0';
        while ( strlen( genbuf ) < 2 ) {
            getfield(  8, "—ß–£œµº∂ªÚµ•Œª»´≥∆", "∑˛ŒÒµ•Œª", career,STRLEN ); }
        genbuf[0] = '\0';
        while ( strlen( genbuf ) < 6 ) {
            getfield( 10, "«ÎæﬂÃÂµΩ«ﬁ “ªÚ√≈≈∆∫≈¬Î", "ƒø«∞◊°÷∑", addr,  STRLEN ); }
        genbuf[0] = '\0';
        while ( strlen( genbuf ) < 2 ) {
            getfield( 12, "∞¸¿®ø…¡¨¬Á ±º‰,»ÙŒﬁø…”√∫Ùª˙ªÚEmailµÿ÷∑¥˙ÃÊ",     "¡¨¬ÁµÁª∞", phone, STRLEN ); }
        getfield( 14, "ƒÍ.‘¬.»’(π´‘™)(—°‘ÒÃÓ–¥)",     "≥ˆ…˙ƒÍ‘¬", birth, STRLEN );
        mesg = "“‘…œ◊ ¡œ «∑Ò’˝»∑, ∞¥ Q ∑≈∆˙◊¢≤· (Y/N/Quit)? [N]: ";
        getdata(t_lines-1,0,mesg,ans,3,DOECHO,NULL,YEA);
        if( ans[0] == 'Q' || ans[0] == 'q' )
            return;
        if( ans[0] == 'Y' || ans[0] == 'y' )
            break;
    }
    strncpy( currentuser.realname, rname,  NAMELEN );
    strncpy( currentuser.address,  addr,   STRLEN  );
    if( (fn = fopen( "new_register", "a" )) != NULL ) {
        now = time( NULL );
        fprintf( fn, "usernum: %d, %s", usernum, ctime( &now ) );
        fprintf( fn, "userid: %s\n",    currentuser.userid );
        fprintf( fn, "realname: %s\n",  rname );
        fprintf( fn, "career: %s\n",    career );
        fprintf( fn, "addr: %s\n",      addr );
        fprintf( fn, "phone: %s\n",     phone );
        fprintf( fn, "birth: %s\n",     birth);
        fprintf( fn, "----\n" );
        fclose( fn );
    }
}

