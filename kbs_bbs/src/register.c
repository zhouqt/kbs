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
char    *genpasswd();
char    *Ctime();

extern char     fromhost[ 60 ];
extern time_t   login_start_time;
time_t          system_time;
extern int convcode;  /* KCN,99.09.05 */
extern int switch_code(); /* KCN,99.09.05 */

int
bad_user_id( userid )
char    *userid;
{
    FILE        *fp;
    char        buf[STRLEN];
    char        *ptr, ch;
    int         i;

    i = 0;
    ptr = userid;
    while( (ch = *ptr++) != '\0' ) {
        i++;
        if( !isalnum( ch ) && ch != '_' )
            return 1;
    }
    if (i<2) return 1;
    if( (fp = fopen( ".badname", "r" )) != NULL ) {
        while( fgets( buf, STRLEN, fp ) != NULL ) {
            ptr = strtok( buf, " \n\t\r" );
            if( ptr != NULL && *ptr != '#')
            {
                if( strcasecmp( ptr, userid ) == 0 ) {
                    if(ptr[13]>47 && ptr[13]<58)/*Haohmaru.99.12.24*/
                    {
                        char timebuf[9];
                        time_t	t,now;
                        strncpy(timebuf,ptr+13,9);
                        t = atol(timebuf);
                        now = time(0);
                        if(now - t > 24*30*3600)
                        {
                            fclose( fp );
                            return 0;
                        }
                    }
                    fclose(fp);
                    return 1;
                }
            }
        }
        fclose(fp);
    }
    return 0;
}

int
compute_user_value( urec )
struct userec *urec;
{
    int         value;

    /* if (urec) has CHATCLOAK permission, don't kick it */
    if( (urec->userlevel & PERM_CHATCLOAK ) && (!(urec->userlevel & PERM_SUICIDE)))
        return 999;


    value = (time(0) - urec->lastlogin) / 60;    /* min */
    if (0 == value) value = 1; /* Leeward 98.03.30 */

    /* ĞŞ¸Ä: ½«ÓÀ¾ÃÕÊºÅ×ªÎª³¤ÆÚÕÊºÅ, Bigman 2000.8.11 */
    if ((urec->userlevel & PERM_XEMPT) && (!(urec->userlevel & PERM_SUICIDE)) )
    {	if (urec->lastlogin < 988610030)
        return 666; /* Èç¹ûÃ»ÓĞµÇÂ¼¹ıµÄ */
        else
            return (667 * 24 * 60 - value)/(60*24);
    }
    /* new user should register in 30 mins */
    if( strcmp( urec->userid, "new" ) == 0 ) {
        return (30 - value) / 60; /* *->/ modified by dong, 1998.12.3 */
    }

    /* ×ÔÉ±¹¦ÄÜ,Luzi 1998.10.10 */
    if (urec->userlevel & PERM_SUICIDE)
        return (15 * 24 * 60 - value)/(60*24);
    /**********************/
    if(urec->numlogins <= 3)
        return (15 * 24 * 60 - value)/(60*24);
    if( !(urec->userlevel & PERM_LOGINOK) )
        return (30 * 24 * 60 - value)/(60*24);
    /* if (urec->userlevel & PERM_LONGID)
         return (667 * 24 * 60 - value)/(60*24); */
    return (120 * 24 * 60 - value)/(60*24);
}

int
getnewuserid()
{
    struct userec utmp, zerorec;
    struct stat st;
    int         fd, size, val, i;
    char    tmpstr[30];

    system_time = time( NULL );
    /*
    if( stat( "tmp/killuser", &st )== -1 || st.st_mtime < system_time-3600 ) {
        if( (fd = open( "tmp/killuser", O_RDWR|O_CREAT, 0600 )) == -1 )
            return -1;
        write( fd, ctime( &system_time ), 25 );
        close( fd );
        log_usies( "CLEAN", "dated users." );
        prints( "Ñ°ÕÒĞÂÕÊºÅÖĞ, ÇëÉÔ´ıÆ¬¿Ì...\n\r" );
        oflush();
        memset( &zerorec, 0, sizeof( zerorec ) );
        if( (fd = open( PASSFILE, O_RDWR|O_CREAT, 0600 )) == -1 )
            return -1;
        size = sizeof( utmp );
        for( i = 0; i < MAXUSERS; i++ ) {
            if( read( fd, &utmp, size ) != size )
                break;
            val = compute_user_value( &utmp );
            if( utmp.userid[0] != '\0' && val <= 0 ) {
                sprintf( genbuf, "#%d %-12s %15.15s %d %d %d",
                         i+1, utmp.userid, ctime( &(utmp.lastlogin) )+4,
                         utmp.numlogins, utmp.numposts, val );
                log_usies( "KILL ", genbuf );
                if( !bad_user_id( utmp.userid ) ) {
                    setmailpath(tmpstr, utmp.userid);
                    sprintf( genbuf, "/bin/rm -fr %s", tmpstr);
                    system( genbuf );
                    sethomepath(tmpstr, utmp.userid);
                    sprintf( genbuf, "/bin/rm -fr %s", tmpstr);
                    system( genbuf );
                    sprintf( genbuf, "/bin/rm -f tmp/email_%s", utmp.userid );
                    system( genbuf );
                }
                lseek( fd, -size, SEEK_CUR );
                write( fd, &zerorec, sizeof( utmp ) );
            }
        }
        close( fd );
    }
*/
    if( (fd = open( PASSFILE, O_RDWR|O_CREAT, 0600 )) == -1 )
        return -1;
    flock( fd, LOCK_EX );

    i = searchnewuser();
    sprintf( genbuf, "uid %d from %s", i, fromhost );
    log_usies( "APPLY", genbuf );

    if( i <= 0 || i > MAXUSERS ) {
        flock(fd,LOCK_UN) ;
        close(fd) ;
        if( dashf( "etc/user_full" ) ) {
            ansimore( "etc/user_full", NA );
            oflush();
        } else {
            prints( "±§Ç¸, Ê¹ÓÃÕßÕÊºÅÒÑ¾­ÂúÁË, ÎŞ·¨×¢²áĞÂµÄÕÊºÅ.\n\r" );
            oflush();
        }
        val = (st.st_mtime - system_time + 3660) / 60;
        prints( "ÇëµÈ´ı %d ·ÖÖÓááÔÙÊÔÒ»´Î, ×£ÄãºÃÔË.\n\r", val );
        oflush();
        sleep( 2 );
        exit( 1 );
    }
    memset( &utmp, 0, sizeof( utmp ) );
    strcpy( utmp.userid, "new" );
    utmp.lastlogin = time( NULL );
    if( lseek( fd, sizeof(utmp) * (i-1), SEEK_SET ) == -1 ) {
        flock( fd, LOCK_UN );
        close( fd );
        return -1;
    }
    write( fd, &utmp, sizeof(utmp) );
    setuserid( i, utmp.userid );
    flock( fd, LOCK_UN );
    close( fd );
    return i;
}

int
id_invalid(userid)
char userid[IDLEN];
{
    char *s;
    if (!isalpha(userid[0]))
        return 1;
    for(s=userid;*s != '\0'; s++) {
        if(*s<1 || !isalnum(*s)) {
            return 1;
        }
    }
    return 0;
}
void
new_register()
{
    struct userec       newuser;
    char        passbuf[ STRLEN ];
    int         allocid, try,flag;


    if( 1 ) {
        time_t  now;

        now = time( 0 );
        sprintf( genbuf, "etc/no_register_%3.3s", ctime( &now ) );
        if( dashf( genbuf ) ) {
            ansimore( genbuf, NA );
            pressreturn();
            exit( 1 );
        }
    }
    memset( &newuser, 0, sizeof(newuser) );
    allocid = getnewuserid()  ;
    if(allocid > MAXUSERS || allocid <= 0) {
        printf("No space for new users on the system!\n\r") ;
        exit(1) ;
    }

    getdata(0, 0, "Ê¹ÓÃGB±àÂëÔÄ¶Á?(\xa8\xcf\xa5\xce BIG5\xbd\x58\xbe\x5c\xc5\xaa\xbd\xd0\xbf\xefN)(Y/N)? [Y]: ", passbuf, 4, DOECHO, NULL, YEA);
    if (*passbuf == 'n' || *passbuf == 'N')
        if (!convcode)
            switch_code();

    ansimore("etc/register", NA);
    try = 0;
    while( 1 ) {
        if( ++try >= 10 ) {
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
			&& (lnow - lst.st_ctime < 2592000/* 3600*24*30 */) ) {
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
    setuserid( allocid, newuser.userid ); /* added by dong, 1998.12.2 */
    newuser.firstlogin = newuser.lastlogin = time(NULL) - 13 * 60 * 24 ;
    substitute_record(PASSFILE,&newuser,sizeof(newuser),allocid);
    while( 1 ) {
        getdata(0,0,"ÇëÉè¶¨ÄúµÄÃÜÂë: ",passbuf,PASSLEN,NOECHO,NULL,YEA) ;
        if( strlen( passbuf ) < 4 || !strcmp( passbuf, newuser.userid ) ) {
            prints("ÃÜÂëÌ«¶Ì»òÓëÊ¹ÓÃÕß´úºÅÏàÍ¬, ÇëÖØĞÂÊäÈë\n") ;
            continue;
        }
        strncpy( newuser.passwd, passbuf, PASSLEN );
        getdata(0,0,"ÇëÔÙÊäÈëÒ»´ÎÄãµÄÃÜÂë: ",passbuf,PASSLEN,NOECHO,NULL,YEA);
        if( strncmp( passbuf, newuser.passwd, PASSLEN ) != 0 ) {
            prints("ÃÜÂëÊäÈë´íÎó, ÇëÖØĞÂÊäÈëÃÜÂë.\n") ;
            continue;
        }
        passbuf[8] = '\0' ;
        strncpy( newuser.passwd, genpasswd( passbuf ), PASSLEN );
        break;
    }
    getdata(0,0,"ÇëÊäÈëÖÕ¶Ë»úĞÎÌ¬: [vt100] ",newuser.termtype,16,DOECHO,NULL,YEA);
    if( newuser.termtype[0] == '\0' ) {
        strcpy(newuser.termtype, "vt100");
    }
    newuser.userlevel = PERM_BASIC;
    newuser.userdefine=-1;
    newuser.userdefine&=~DEF_MAILMSG;
    newuser.userdefine&=~DEF_EDITMSG;
    if (convcode)
        newuser.userdefine&=~DEF_USEGB;

    newuser.notemode=-1;
    newuser.unuse1=-1;
    newuser.unuse2=-1;
    newuser.flags[0] = CURSOR_FLAG;
    newuser.flags[0] |= PAGER_FLAG;
    newuser.flags[1] = 0;
    newuser.firstlogin = newuser.lastlogin = time(NULL) ;

    if( substitute_record(PASSFILE,&newuser,sizeof(newuser),allocid) == -1 ) {
        /* change by KCN 1999.09.08
                fprintf(stderr,"too much, good bye!\n") ;
        */
        prints("too much, good bye!\n") ;
        oflush();
        exit(1) ;
    }
    setuserid( allocid, newuser.userid );
    if( !dosearchuser(newuser.userid) ) {
        /* change by KCN 1999.09.08
                fprintf(stderr,"User failed to create\n") ;
        */
        prints("User failed to create %d-%s\n",allocid,newuser.userid) ;
        oflush();
        exit(1) ;
    }
    touchnew();
    report( "new account" );
}

char *
trim( s )
char *s;
{
    static char buf[ 256 ];
    char *l, *r;

    buf[ 0 ] = '\0' ;
    r = s + strlen( s ) - 1;

    for (l = s ; strchr(" \t\r\n", *l) && *l; l++);

    /* if all space, *l is null here, we just return null */
    if (*l != '\0') {
        for ( ; strchr(" \t\r\n", *r) && r >= l ; r-- );
        strncpy( buf, l, r - l + 1 );
    }
    return buf;
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
    sprintf( fname, "tmp/email_%s", userid );
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
    struct userec *urec = &currentuser;
    char        *newregfile;
    int         perm;
    time_t      code;
    FILE        *fin, *fout,*dp;
    char        ans[4],buf[STRLEN];
    extern int showansi;

    clear();
    sprintf(buf,"%s",email_domain());
    if( !(urec->userlevel & PERM_BASIC) ) {
        urec->userlevel = 0;
        return;
    }
    /*urec->userlevel |= PERM_DEFAULT;*/
    perm = PERM_DEFAULT & sysconf_eval( "AUTOSET_PERM" );

    /*    if( sysconf_str( "IDENTFILE" ) != NULL ) {  commented out by netty to save time */
    while ( strlen( urec->username ) < 2 ) {
        getdata( 2, 0, "ÇëÊäÈëÄúµÄêÇ³Æ:(ÀıÈç,´òµ¹±±Ô¼) << ", urec->username, NAMELEN,DOECHO,NULL ,YEA);
        strcpy(uinfo.username,urec->username);
        update_utmp();
    }
    if ( strlen( urec->realname ) < 2 ) {
        move( 3, 0 );
        prints( "ÇëÊäÈëÄúµÄÕæÊµĞÕÃû: (Õ¾³¤»á°ïÄú±£ÃÜµÄ !)\n" );
        getdata( 4, 0, "> ", urec->realname, NAMELEN,DOECHO,NULL,YEA);
    }
    if ( strlen( urec->address ) < 6 ) {
        move( 5, 0 );
        prints( "ÄúÄ¿Ç°ÌîĞ´µÄµØÖ·ÊÇ¡®%s¡¯£¬³¤¶ÈĞ¡ÓÚ [1m[37m6[m£¬ÏµÍ³ÈÏÎªÆä¹ıÓÚ¼ò¶Ì¡£\n", strlen( urec->address ) > 0 ? urec->address : "¿ÕµØÖ·" ); /* Leeward 98.04.26 */
        getdata( 6, 0, "ÇëÏêÏ¸ÌîĞ´ÄúµÄ×¡Ö·£º", urec->address, NAMELEN,DOECHO,NULL,YEA);
    }
    if ( strchr( urec->email, '@' ) == NULL ) {
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
        prints( "³ÉÎª±¾Õ¾ºÏ·¨¹«ÃñµÄ·½·¨ÈçÏÂ£º\n\n" );
        prints( "ÄãµÄÕÊºÅµÄµÚÒ»´ÎµÇÂ¼ºóµÄ 72 Ğ¡Ê±ÄÚ£¨[1m[33m²»ÊÇÖ¸ÉÏ BBS 72Ğ¡Ê±[m£©£¬\n");
        prints( "    Äã´¦ÓÚĞÂÊÖÉÏÂ·ÆÚ¼ä, ÇëËÄ´¦²Î¹ÛÑ§Ï°ÍøÂçÊ¹ÓÃ·½·¨ºÍ¸÷ÖÖÀñÒÇ¡£\n");
        prints( "    ÔÚ´ËÆÚ¼ä£¬²»ÄÜ×¢²á³ÉÎªºÏ·¨¹«Ãñ¡£\n\n");
        prints( "¹ıÁËÕâ¿ªÊ¼µÄ 72 Ğ¡Ê±, Äã¾Í¿ÉÒÔÔÚ½øÈë±¾Õ¾ÒÔºó£¬\n" );
        prints( "    ÔÚ¡®¸öÈË¹¤¾ßÏä¡¯ÄÚÏêÏ¸×¢²áÕæÊµÉí·İ£¬\n" );
        prints( "    SYSOPs »á¾¡¿ì¼ì²é²¢È·ÈÏÄãµÄ×¢²áµ¥¡£\n\n" );
        /* Leeward adds below 98.04.26 */
        prints( "[1m[33mÈç¹ûÄãÒÑ¾­Í¨¹ıÁË×¢²á£¬³ÉÎªÁËºÏ·¨¹«Ãñ£¬È´ÒÀÈ»»¹ÊÇ¿´µ½ÁË±¾ĞÅÏ¢£¬\nÄÇÊÇÒòÎªÄãÃ»ÓĞÔÚ¡®¸öÈË¹¤¾ßÏä¡¯ÄÚÉè¶¨¡®µç×ÓÓÊ¼şĞÅÏä¡¯¡£[m\nÇë´Ó¡®Ö÷Ñ¡µ¥¡¯½øÈë¡®¸öÈË¹¤¾ßÏä¡¯ÄÚ£¬ÔÙ½øÈë¡®Éè¶¨¸öÈË×ÊÁÏ¡¯Ò»Ïî½øĞĞÉè¶¨¡£\nÈç¹ûÄãÊµÔÚÃ»ÓĞÈÎºÎ¿ÉÓÃµÄ¡®µç×ÓÓÊ¼şĞÅÏä¡¯¿ÉÒÔÉè¶¨£¬ÓÖ²»Ô¸Òâ¿´µ½±¾ĞÅÏ¢£¬\n¿ÉÒÔÊ¹ÓÃ [1m[33m%s.bbs@smth.org[m ½øĞĞÉè¶¨¡£\n×¢Òâ¡ÃÉÏÃæ¸ø³öµÄµç×ÓÓÊ¼şĞÅÏä²»ÄÜ½ÓÊÕµç×ÓÓÊ¼ş£¬½ö½öÊÇÓÃÀ´Ê¹ÏµÍ³²»ÔÙÏÔÊ¾±¾ĞÅÏ¢¡£", currentuser.userid);
        pressreturn();
    }
    if(!strcmp(currentuser.userid,"SYSOP"))
    {
        currentuser.userlevel=~0;
        currentuser.userlevel&=~PERM_SUICIDE; /* Leeward 98.10.13 */
        currentuser.userlevel&=~PERM_DENYMAIL; /* Bigman 2000.9.22 */
        substitute_record(PASSFILE,&currentuser,sizeof(struct userec),usernum);
    }
    if(!(currentuser.userlevel&PERM_LOGINOK))
    {
        if( HAS_PERM( PERM_SYSOP ))
            return;
        if(!invalid_realmail( urec->userid, urec->termtype+16, STRLEN-16 ))
        {
            set_safe_record();
            urec->userlevel |= PERM_DEFAULT;
            if( HAS_PERM( PERM_DENYPOST ) && !HAS_PERM( PERM_SYSOP ) )
                urec->userlevel &= ~PERM_POST;
            substitute_record(PASSFILE,urec,sizeof(struct userec),usernum) ;
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
            prints( "Èç¹ûÄãÒª³ÉÎª±¾Õ¾µÄ×¢²áÓÃ»§£¬\n\n");
            prints( "ÇëÔÚ[31m¸öÈË¹¤¾ßÏä[0mÄÚ[31mÏêÏ¸×¢²áÉí·İ[0m\n" );
            prints( "ÄúÖ»ÒªÈ·ÊµÏêÏ¸ÌîĞ´ÁËÄãµÄÕæÊµÉí·İ×ÊÁÏ, \n");
            prints( "ÔÚÕ¾³¤ÌæÄãÊÖ¹¤ÈÏÖ¤ÁËÒÔºó£¬¾Í¿ÉÒÔ³ÉÎª±¾Õ¾ºÏ¸ñ¹«Ãñ.\n" );
            pressreturn();
        }
        /* end of check if local email-addr */
        /*  above lines added by netty...  */
    }
    newregfile = sysconf_str( "NEWREGFILE" );
    if( urec->lastlogin - urec->firstlogin < 3*86400 &&
            !HAS_PERM( PERM_SYSOP) && newregfile != NULL ) {
        set_safe_record();
        urec->userlevel &= ~(perm);
        substitute_record(PASSFILE,urec,sizeof(struct userec),usernum) ;
        ansimore( newregfile, YEA );
    }
    set_safe_record();
    if( HAS_PERM( PERM_DENYPOST ) && !HAS_PERM( PERM_SYSOP ) )
    {
        currentuser.userlevel &= ~PERM_POST;
        substitute_record(PASSFILE,urec,sizeof(struct userec),usernum) ;
    }
}

