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

int
d_board()
{
    int bid ;
    char bname[STRLEN];
    char title[STRLEN];
    extern char lookgrp[];

    modify_user_mode( ADMIN );
    if(!check_systempasswd())
    {
        return -1;
    }
    clear();
    stand_title( "…æ≥˝Ã÷¬€«¯" );
    make_blist() ;
    move(1,0) ;
    namecomplete( "«Î ‰»ÎÃ÷¬€«¯: ",genbuf) ;
    if( genbuf[0] == '\0' )
        return 0;
    strcpy(bname,genbuf);
    if (delete_board(bname,title)!=0) return 0;
    if(seek_in_file("0Announce/.Search",bname))
    {
#ifdef BBSMAIN
        getdata(3,0,"“∆≥˝æ´ª™«¯ (Yes, or No) [Y]: ",genbuf,4,DOECHO,NULL,YEA) ;
        if( genbuf[0] != 'N' && genbuf[0] != 'n')
        {
#endif
            get_grp(bname);
            del_grp(lookgrp,bname,title+13);

        }
    }
            if( !bname[0] ) {
	    if(seek_in_file("etc/anonymous",bname))
	        del_from_file("etc/anonymous",bname);
	    if(seek_in_file("0Announce/.Search",bname))
	        del_from_file("0Announce/.Search",bname);
        sprintf(genbuf, "deleted board %s", bname);
        report(genbuf);
        sprintf(genbuf,"/bin/rm -fr boards/%s",bname) ;
        system(genbuf) ;
        sprintf(genbuf,"/bin/rm -fr vote/%s",bname) ;
        system(genbuf) ;
    }

    move(4,0) ;
    prints("±æÃ÷¬€«¯“—æ≠…æ≥˝...\n") ;
    pressreturn() ;
    clear() ;
    return 0 ;
}

suicide()
{
    char buf[STRLEN];
    FILE *fn;
    time_t now;
    char filename[STRLEN];
#define XPERMSTR "bTCPRp#@XWBA$VS!DEM1234567890%"
    char XPERM[48];
    int  oldXPERM;
    int  num;

    modify_user_mode( OFFLINE );
    if(HAS_PERM(PERM_SYSOP)||HAS_PERM(PERM_BOARDS)||HAS_PERM(PERM_OBOARDS)||HAS_PERM(PERM_ACCOUNTS)||HAS_PERM(PERM_ANNOUNCE)||HAS_PERM(PERM_JURY) ||HAS_PERM(PERM_SUICIDE)||HAS_PERM(PERM_CHATOP)||HAS_PERM(PERM_DENYPOST)||HAS_PERM(PERM_DENYMAIL))
    {
        clear();
        move(11,28);
        prints("[1m[33mƒ„”–÷ÿ»Œ‘⁄…Ì£¨≤ªƒ‹◊‘…±£°[m");
        pressanykey();
        return;
    }

    clear();
    move(1,0);
    prints("—°‘Ò◊‘…±Ω´ πƒ˙µƒ…˙√¸¡¶ºı…ŸµΩ14ÃÏ£¨14ÃÏ∫Ûƒ˙µƒ’ ∫≈◊‘∂Øœ˚ ß°£");
    move(3,0);
    prints("‘⁄’‚14ÃÏƒ⁄»Ù∏ƒ±‰÷˜“‚µƒª∞£¨‘Úø…“‘Õ®π˝µ«¬º±æ’æ“ª¥Œª÷∏¥‘≠…˙√¸¡¶");
    move(5,0);
    prints("◊‘…±”√ªßΩ´∂™ ßÀ˘”–[33mÃÿ ‚»®œﬁ[0m£°£°£°");
    move(7,0);
    /*
        clear();
        move(1,0);
        prints("—°‘Ò◊‘…±Ω´ πƒ˙µƒ’ ∫≈◊‘∂Ø¡¢º¥œ˚ ß!");
        move(3,0);
        prints("ƒ˙µƒ’ ∫≈Ω´¬Ì…œ¥”œµÕ≥÷–…æ≥˝");
        */

    if(askyn("ƒ„»∑∂®“™◊‘…±¬£ø",0)==1)
    {
        clear();
        getdata(0,0,"«Î ‰»Î‘≠√‹¬Î( ‰»Î’˝»∑µƒª∞ª·¡¢øÃ∂œœﬂ): ",buf,39,NOECHO,NULL,YEA);/*Haohmaru,98.10.12,check the passwds*/
        if( *buf == '\0' || !checkpasswd2( buf,currentuser )) {
            prints("\n\n∫‹±ß«∏, ƒ˙ ‰»Îµƒ√‹¬Î≤ª’˝»∑°£\n");
            pressanykey();
            return;
        }

        oldXPERM=currentuser->userlevel;
        strcpy(XPERM, XPERMSTR);
        for ( num = 0; num < (int)strlen(XPERM); num++ )
            if ( !(oldXPERM & (1 << num)) )
                XPERM[num] = ' ';
        XPERM[num] = '\0';
        currentuser->userlevel&=0x3F;/*Haohmaru,99.3.20.◊‘…±’ﬂ÷ª±£¡Ùª˘±æ»®œﬁ*/
        currentuser->userlevel^=PERM_SUICIDE;

        /*Haohmaru.99.3.20.◊‘…±Õ®÷™*/
        now=time(0);
        sprintf(filename,"etc/%s.tmp",currentuser->userid);
        fn=fopen(filename,"w");
        fprintf(fn,"[1m%s[m “—æ≠‘⁄ [1m%24.24s[m ◊‘…±¡À£¨“‘œ¬ «À˚µƒ◊ ¡œ£¨«Î±£¡Ù...",currentuser->userid,ctime(&now));
        fprintf(fn,"\n\n“‘œ¬ «◊‘…±’ﬂ‘≠¿¥µƒ»®œﬁ\n\033[1m\033[33m%s\n[0m", XPERM);
        getuinfo(fn, currentuser);
        fprintf(fn,"\n                      [1m œµÕ≥◊‘∂Ø∑¢–≈œµÕ≥¡Ù[m\n");
        fclose(fn);
        sprintf(buf,"%s µƒ◊‘…±Õ®÷™",currentuser->userid);
        postfile(filename,"Goodbye",buf,1);
        unlink(filename);

        /*kick_user(&uinfo);
        exit(0);*/
        abort_bbs();
    }
}


offline()
{
    char buf[STRLEN];

    modify_user_mode( OFFLINE );

    if(HAS_PERM(PERM_SYSOP))
        return;
    clear();
    move(1,0);
    prints("[32m∫√ƒ—π˝‡∏.....[m");
    move(3,0);
    if(askyn("ƒ„»∑∂®“™¿Îø™’‚∏ˆ¥Ûº“Õ•",0)==1)
    {
        clear();
        if(d_user(currentuser->userid)==1)
        {
            mail_info();
            kick_user(&uinfo);
            exit(0);
        }
    }
}

getuinfo(fn, ptr_urec)
FILE *fn;
struct userec *ptr_urec;
{
    fprintf(fn,"\n\nƒ˙µƒ¥˙∫≈     : %s\n", ptr_urec->userid);
    fprintf(fn,"ƒ˙µƒÍ«≥∆     : %s\n", ptr_urec->username);
    fprintf(fn,"’Ê µ–’√˚     : %s\n", ptr_urec->realname);
    fprintf(fn,"æ”◊°◊°÷∑     : %s\n", ptr_urec->address);
    fprintf(fn,"µÁ◊”” º˛–≈œ‰ : %s\n", ptr_urec->email);
    fprintf(fn,"’Ê µ E-mail  : %s\n", ptr_urec->realemail);
    fprintf(fn,"Ident ◊ ¡œ   : %s\n", ptr_urec->ident);
    fprintf(fn,"◊¢≤·»’∆⁄     : %s", ctime( &ptr_urec->firstlogin));
    fprintf(fn,"◊ÓΩ¸π‚¡Ÿ»’∆⁄ : %s", ctime( &ptr_urec->lastlogin));
    fprintf(fn,"◊ÓΩ¸π‚¡Ÿª˙∆˜ : %s\n", ptr_urec->lasthost );
    fprintf(fn,"…œ’æ¥Œ ˝     : %d ¥Œ\n", ptr_urec->numlogins);
    fprintf(fn,"Œƒ’¬ ˝ƒø     : %d / %d (Board/1Discuss)\n",
            ptr_urec->numposts, post_in_tin( ptr_urec->userid ));
}

mail_info()
{
    FILE *fn;
    time_t now;
    char filename[STRLEN];

    now=time(0);
    sprintf(filename,"etc/%s.tmp",currentuser->userid);
    fn=fopen(filename,"w");
    fprintf(fn,"[1m%s[m “—æ≠‘⁄ [1m%24.24s[m ◊‘…±¡À£¨“‘œ¬ «À˚(À˝)µƒ◊ ¡œ£¨«Î±£¡Ù...",currentuser->userid
            ,ctime(&now));
    getuinfo(fn, currentuser);
    fprintf(fn,"\n                      [1m œµÕ≥◊‘∂Ø∑¢–≈œµÕ≥¡Ù[m\n");
    fclose(fn);
    mail_file(filename,"acmanager","◊‘…±Õ®÷™....");
    unlink(filename);
}


int kickuser(struct user_info* uentp,char* arg,int count)
{
	kill(uentp->pid,SIGHUP);
	clear_utmp2(uentp);
	UNUSED_ARG(arg);
	UNUSED_ARG(count);
	return 0;
}

int
d_user(cid)
char cid[IDLEN];
{
    int id,fd ;
    char tmpbuf [30];
    char userid[IDLEN+2];
    struct userec* lookupuser;

    if(uinfo.mode!=OFFLINE)
    {
        modify_user_mode( ADMIN );
        if(!check_systempasswd())
        {
            return 0;
        }
        clear();
        stand_title( "…æ≥˝ π”√’ﬂ’ ∫≈" );
        move(1,0) ;
        usercomplete("«Î ‰»Î”˚…æ≥˝µƒ π”√’ﬂ¥˙∫≈: ",userid);
        if(userid[0] == '\0') {
            clear() ;
            return 0 ;
        }
    }else
        strcpy(userid,cid);
    if(!(id = getuser(userid,&lookupuser))) {
        move(3,0) ;
        prints("¥ÌŒÛµƒ π”√’ﬂ¥˙∫≈...") ;
        clrtoeol() ;
        pressreturn() ;
        clear() ;
        return 0 ;
    }
    /*    if (!isalpha(lookupuser->userid[0])) return 0;*/
    /* rrr - don't know how...*/
    move(1,0) ;
    if(uinfo.mode!=OFFLINE)
        prints("…æ≥˝ π”√’ﬂ '%s'.",userid) ;
    else
        prints(" %s Ω´¿Îø™’‚¿Ô",cid);
    clrtoeol();
    getdata(2,0,"(Yes, or No) [N]: ",genbuf,4,DOECHO,NULL,YEA) ;
    if(genbuf[0] != 'Y' && genbuf[0] != 'y') { /* if not yes quit */
        move(2,0) ;
        if(uinfo.mode!=OFFLINE)
            prints("»°œ˚…æ≥˝ π”√’ﬂ...\n") ;
        else
            prints("ƒ„÷’Ï∂ªÿ–ƒ◊™“‚¡À£¨∫√∏ﬂ–À‡∏...");
        pressreturn() ;
        clear() ;
        return 0 ;
    }
    if(uinfo.mode!=OFFLINE)
    {
        char        secu[STRLEN];
        sprintf(secu,"…æ≥˝ π”√’ﬂ£∫%s",lookupuser->userid);
        securityreport(secu,lookupuser);
    }
    sprintf(genbuf, "%s deleted user %s", currentuser->userid,lookupuser->userid);
    report(genbuf);
    /*Haohmaru.99.12.23.±ª…æID“ª∏ˆ‘¬ƒ⁄≤ªµ√◊¢≤·*/
    if((fd = open(".badname",O_WRONLY|O_CREAT,0644)) != -1 ) {
        char buf[STRLEN] ;
        char thtime[40];
        time_t dtime;
        dtime = time(0);
        sprintf(thtime,"%d",dtime);
        flock(fd,LOCK_EX) ;
        lseek(fd,0,SEEK_END) ;
        sprintf(buf,"%-12.12s %-66.66s\n",lookupuser->userid, thtime) ;
        write(fd,buf,strlen(buf)) ;
        flock(fd,LOCK_UN) ;
        close(fd) ;
    }
    else{
        printf("¥ÌŒÛ£¨«Î±®∏ÊSYSOP");
        pressanykey();}
    setmailpath(tmpbuf, lookupuser->userid);
    sprintf(genbuf,"/bin/rm -fr %s", tmpbuf) ;
    system(genbuf) ;
    sethomepath(tmpbuf, lookupuser->userid);
    sprintf(genbuf,"/bin/rm -fr %s", tmpbuf) ;
    system(genbuf) ;
    sprintf(genbuf,"/bin/rm -fr tmp/email/%s", lookupuser->userid) ;
    system(genbuf) ;
    apply_utmp(kickuser,0,userid,0);
    setuserid( id, "" );
    lookupuser->userlevel = 0;
    strcpy(lookupuser->address, "");
    strcpy(lookupuser->username, "");
    strcpy(lookupuser->realname, "");
/*    lookupuser->userid[0] = '\0' ; */
    move(2,0) ;
    prints("%s “—æ≠“—æ≠∫Õ±æº“Õ• ß»•¡™¬Á....\n",userid) ;
    pressreturn() ;

    clear() ;
    return 1 ;
}

/* to be Continue fix kick user problem */
int kick_user(struct user_info *userinfo)
{
    int id, ind ;
    struct user_info uin;
    struct userec kuinfo;
    char buffer [40];
   	char userid[40];

    if(uinfo.mode!=LUSERS&&uinfo.mode!=OFFLINE&&uinfo.mode!=FRIEND)
    {
        modify_user_mode( ADMIN );
        stand_title( "Kick User" );
        move(1,0) ;
        usercomplete("Enter userid to be kicked: ",userid) ;
        if(*userid == '\0') {
            clear() ;
            return 0 ;
        }
        if(!(id = searchuser(userid))) { /* change getuser -> searchuser, by dong, 1999.10.26 */
            move(3,0) ;
            prints("Invalid User Id") ;
            clrtoeol() ;
            pressreturn() ;
            clear() ;
            return 0 ;
        }
        move(1,0) ;
        prints("Kick User '%s'.",userid) ;
        clrtoeol();
        getdata(2,0,"(Yes, or No) [N]: ",genbuf,4,DOECHO,NULL,YEA) ;
        if(genbuf[0] != 'Y' && genbuf[0] != 'y') { /* if not yes quit */
            move(2,0) ;
            prints("Aborting Kick User\n") ;
            pressreturn() ;
            clear() ;
            return 0 ;
        }
    	return apply_utmp(kickuser,0,userid,0);
    }else
    {
        uin=*userinfo;
        strcpy(userid,uin.userid);
        ind=YEA;
    }
    if (!ind || !uin.active || (kill(uin.pid,0) == -1)) {
        if(uinfo.mode!=LUSERS&&uinfo.mode!=OFFLINE&&uinfo.mode!=FRIEND)
        {
            move(3,0) ;
            prints("User Has Logged Out") ;
            clrtoeol() ;
            pressreturn() ;
            clear() ;
        }
        return 0 ;
    }
    if (kill(uin.pid,SIGHUP) == -1)
    {
    	clear_utmp2(userinfo);
    }
    sprintf(buffer, "kicked %s", userid);
    report(buffer);
    /*sprintf( genbuf, "%s (%s)", kuinfo.userid, kuinfo.username );modified by dong, 1998.11.2 */
    /*log( "1system", "KICK %s (%s)", uin.userid, uin.username );*/
    /*    uin.active = NA;
        uin.pid = 0;
        uin.invisible = YEA;
        uin.sockactive = 0;
        uin.sockaddr = 0;
        uin.destuid = 0;
        update_ulist( &uin, ind ); Œﬁ“‚“Â∂¯«“≤Œ ˝”–¥Ì£¨À˘“‘◊¢ ÕµÙ dong 1998.7.7*/
    move(2,0) ;
    if(uinfo.mode!=LUSERS&&uinfo.mode!=OFFLINE&&uinfo.mode!=FRIEND)
    {
        prints("User has been Kicked\n") ;
        pressreturn() ;
        clear() ;
    }
    return 1 ;
}

