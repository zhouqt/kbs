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
int use_define=0;
extern int iscolor;
extern int switch_code(); /* KCN,99.09.05 */
extern int convcode; /* KCN,99.09.05 */
int
modify_user_mode( mode )
int     mode;
{
    if(uinfo.mode==mode) return 0; /* ±ØĞë¼õÉÙupdate_ulistµÄ´ÎÊı. ylsdd 2001.4.27 */
    uinfo.mode = mode;
    UPDATE_UTMP(mode,uinfo);
    return 0;
}
/*
int
x_csh()
{
    int save_pager;
    clear() ;
    refresh() ;
    reset_tty() ;
    save_pager = uinfo.pager;
    uinfo.pager = 0 ;
    UPDATE_UTMP(pager,uinfo);
    report("shell out");
#ifdef SYSV
    do_exec("sh", NULL) ;
#else
    do_exec("csh", NULL);
#endif
    restore_tty() ;
    uinfo.pager = save_pager;
    UPDATE_UTMP(pager,uinfo);
    clear() ;
    return 0 ;
}
*/

int showperminfoX(unsigned int pbits,int i,int flag ) /* Leeward 98.06.05 */
{
    char        buf[ STRLEN ];

    if ( 16 == i || 11 == i || 20==i || 10 == i || 14 == i || 17 == i || 21 == i || 29==i || 28==i || 26==i || 24==i)
    {
        sprintf( buf, "%c. %-30s %3s", 'A' + i, (use_define)?user_definestr[i]:permstrings[i], ((pbits >> i) & 1 ? "ON" : "OFF"));
        move(6 + (i >= 16) + (i >= 10) + (i >=11) + (i >= 14) + (i >= 17) + (i >=20) + (i >= 21) + (i>=29) + (i>=28) + (i>=26), 0);
        prints( buf );
        refresh();
        return YEA;
    }
    else
    {
        if (pbits) bell();
        return NA;
    }
}

int
showperminfo( unsigned int pbits,int i,int flag)
{
    char        buf[ STRLEN ];

    sprintf( buf, "%c. %-30s %3s", 'A' + i, (use_define)?user_definestr[i]:permstrings[i],
             ((pbits >> i) & 1 ? "ON" : "OFF"));
    move( i+6-(( i>15)? 16:0) , 0+(( i>15)? 40:0) );
    prints( buf );
    refresh();
    return YEA;
}
/* bad 2002.7.6 */
int
showperminfo2( unsigned int pbits,unsigned int basic,int i,int flag)
{
    char        buf[ STRLEN ];

    if((1<<i==PERM_BASIC||1<<i==PERM_POST||1<<i==PERM_CHAT||1<<i==PERM_PAGE||1<<i==PERM_DENYMAIL)&&(basic&(1<<i)))
        sprintf( buf, "%c. %-30s [32;1m%3s[m", 'A' + i, (use_define)?user_definestr[i]:permstrings[i],
                 ((pbits >> i) & 1 ? "ON" : "OFF"));
    else
        sprintf( buf, "%c. %-30s [37;0m%3s[m", 'A' + i, (use_define)?user_definestr[i]:permstrings[i],
                 ((pbits >> i) & 1 ? "ON" : "OFF"));
    move( i+6-(( i>15)? 16:0) , 0+(( i>15)? 50:0) );
    prints( buf );
    refresh();
    return YEA;
}

unsigned int setperms(unsigned int pbits,char *prompt,int numbers,int (*showfunc)(unsigned int ,int ,int))
{
    int lastperm = numbers - 1;
    int i, done = NA;
    char choice[3];

    move(4,0);
    prints("Çë°´ÏÂÄãÒªµÄ´úÂëÀ´Éè¶¨%s£¬°´ Enter ½áÊø.\n",prompt);
    move(6,0);
    clrtobot();
    /*    pbits &= (1 << numbers) - 1;*/
    for (i=0; i<=lastperm; i++) {
        (*showfunc)( pbits, i,NA);
    }
    while (!done) {
        getdata(t_lines-1, 0, "Ñ¡Ôñ(ENTER ½áÊø): ",choice,2,DOECHO,NULL,YEA);
        *choice = toupper(*choice);
        if (*choice == '\n' || *choice == '\0') done = YEA;
        else if (*choice < 'A' || *choice > 'A' + lastperm) bell();
        else {
            i = *choice - 'A';
            pbits ^= (1 << i);
            if((*showfunc)( pbits, i ,YEA)==NA)
            {
                pbits ^= (1 << i);
            }
        }
    }
    return( pbits );
}
/* bad 2002.7.6*/
unsigned int setperms2(unsigned int pbits,unsigned int basic,char *prompt,int numbers,int (*showfunc)(unsigned int ,unsigned int,int ,int))
{
    int lastperm = numbers - 1;
    int i, done = NA;
    char choice[3];

    move(4,0);
    prints("Çë°´ÏÂÄãÒªµÄ´úÂëÀ´Éè¶¨%s£¬°´ Enter ½áÊø.\n",prompt);
    move(6,0);
    clrtobot();
    /*    pbits &= (1 << numbers) - 1;*/
    for (i=0; i<=lastperm; i++) {
        (*showfunc)( pbits, basic, i,NA);
    }
    while (!done) {
        getdata(t_lines-1, 0, "Ñ¡Ôñ(ENTER ½áÊø): ",choice,2,DOECHO,NULL,YEA);
        *choice = toupper(*choice);
        if (*choice == '\n' || *choice == '\0') done = YEA;
        else if (*choice < 'A' || *choice > 'A' + lastperm) bell();
        else {
            i = *choice - 'A';
            pbits ^= (1 << i);
            if((*showfunc)( pbits, basic,i ,YEA)==NA)
            {
                pbits ^= (1 << i);
            }
        }
    }
    return( pbits );
}
/* É¾³ı¹ıÆÚµÄÕÊºÅ */
/* ËãÊÇ¸ø°ÂÔËµÄÏ×Àñ */
/* Bigman 2001.7.14 */
int confirm_delete_id()
{	
   char buff[STRLEN];

   if (!HAS_PERM(currentuser,PERM_ADMIN) )
    {
        move( 3, 0 );
        clrtobot();
        prints( "±§Ç¸, Ö»ÓĞ×Ü¹ÜÀíÔ±²ÅÄÜĞŞ¸Ä");
        pressreturn();
        return 1;
    }

    modify_user_mode( ADMIN );
    clear();
    move(8,0) ;
    prints("\n") ;
    clrtoeol() ;
    move(9,0) ;
    usercomplete("ÇëÊäÈëÒªÈ·ÈÏÇå³ıµÄÓÃ»§ID: ",genbuf) ;
    if(genbuf[0] == '\0') {
        clear() ;
        return 1 ;}

    if((usernum = searchuser(genbuf )) != 0)
	{
	prints("´ËÕÊºÅÓĞÈËÊ¹ÓÃ\n") ;
	pressreturn();
        return 1;
	}

    sethomepath( buff,genbuf);
    /*
    sprintf(commd,"rm -rf %s",buff);
    */
    f_rm(buff);
    setmailpath( buff,genbuf);
    f_rm(buff);
    /*
    sprintf(commd,"rm -rf %s",buff);
    */

    report("delete confirmly dead id's directory");

    clrtoeol() ;
    pressreturn() ;
    clear() ;
    return 0 ;
}
/* inserted by cityhunter to let OBOARDS to change a users basic level */
int
p_level()
{
    int id ;
    char secu[STRLEN];
    struct userec* lookupuser;

    if (!HAS_PERM(currentuser,PERM_OBOARDS) )
    {
        move( 3, 0 );
        clrtobot();
        prints( "±§Ç¸, Ö»ÓĞ°æÎñ¹ÜÀíÈ¨ÏŞµÄ¹ÜÀíÔ±²ÅÄÜĞŞ¸Ä");
        pressreturn();
        return 0;
    }

    modify_user_mode( ADMIN );
    clear();
    move(8,0) ;
    prints("·â½û»ò½â³ıÓÃ»§·¢ÎÄÈ¨ÏŞ\n") ;
    clrtoeol() ;
    move(9,0) ;
    usercomplete("ÇëÊäÈëÒª¸ü¸ÄÈ¨ÏŞµÄÓÃ»§ID: ",genbuf) ;
    if(genbuf[0] == '\0') {
        clear() ;
        return 0 ;
    }
    if(!(id = getuser(genbuf,&lookupuser))) {
        move(11,24) ;
        prints("ÎŞĞ§µÄÓÃ»§ID!!!") ;
        clrtoeol() ;
        pressreturn() ;
        clear() ;
        return 0 ;
    }
    if(strcmp(genbuf,"SYSOP")==0 && strcmp(currentuser->userid, "SYSOP")){
        move(11,0) ;
        prints("²»¿ÉÒÔĞŞ¸ÄSYSOPµÄÈ¨ÏŞ!!!") ;
        clrtoeol() ;
        pressreturn() ;
        clear() ;
        return 0 ;
    }

    move(11,0);
    if(lookupuser->userlevel & PERM_POST)
    {
        prints("ÓÃ»§ '%s' ÏÖÔÚ¾ßÓĞ·¢ÎÄÈ¨ÏŞ\n",lookupuser->userid) ;
    }
    else
    {
        prints("ÓÃ»§ '%s' ÏÖÔÚÃ»ÓĞ·¢ÎÄÈ¨ÏŞ\n",lookupuser->userid) ;
    }
    getdata(12,0,"È·¶¨ÒªĞŞ¸Ä¸ÃÓÃ»§µÄ·¢ÎÄÈ¨ÏŞ (Y/N)? [N]: ",genbuf,4,DOECHO,NULL,YEA);

    if(*genbuf=='y'||*genbuf=='Y'){
        lookupuser->userlevel ^= PERM_POST;/* ¸Ä±ä¸ÃÓÃ»§È¨ÏŞ */
        sprintf(secu,"ĞŞ¸Ä %s µÄ·¢ÎÄÈ¨ÏŞ",lookupuser->userid);
        securityreport(secu,lookupuser,NULL);
        move(13,0);
        if(lookupuser->userlevel & PERM_POST)
        {
            prints("ÓÃ»§ '%s' ÒÑ±»»Ö¸´·¢ÎÄÈ¨ÏŞ\n",lookupuser->userid) ;
        }
        else
        {
            prints("ÒÑÈ¡ÏûÓÃ»§ '%s' µÄ·¢ÎÄÈ¨ÏŞ\n",lookupuser->userid) ;
        }
    }
    clrtoeol() ;
    pressreturn() ;
    clear() ;
    return 0 ;
}
/* end of this insertion */
int
x_level()
{
    int id ;
    unsigned int newlevel;
    int flag=0;/*Haohmaru,98.10.05*/
    int flag1=0,flag2=0; /* bigman 2000.1.5 */
    struct userec* lookupuser;
    char genbuf2[255];
    int lcount=0,tcount=0,i,j,kcount=0,basicperm;
    int s[10][2];
    FILE* fn;

    /* add by alex, 97.7 , strict the power of sysop */
    if (!HAS_PERM(currentuser,PERM_ADMIN) || !HAS_PERM(currentuser,PERM_SYSOP))
    {
        move( 3, 0 );
        clrtobot();
        prints( "±§Ç¸, Ö»ÓĞADMINÈ¨ÏŞµÄ¹ÜÀíÔ±²ÅÄÜĞŞ¸ÄÆäËûÓÃ»§È¨ÏŞ");
        pressreturn();
        return 0;
    }

    modify_user_mode( ADMIN );
    if(!check_systempasswd())
    {
        return 0;
    }
    clear();
    move(0,0) ;
    prints("¸ü¸Ä"NAME_USER_SHORT"µÄÈ¨ÏŞ\n") ;
    clrtoeol() ;
    move(1,0) ;
    usercomplete("ÇëÊäÈë"NAME_USER_SHORT" ID: ",genbuf) ;
    if(genbuf[0] == '\0') {
        clear() ;
        return 0 ;
    }
    if(!(id = getuser(genbuf,&lookupuser))) {
        move(3,0) ;
        prints("·Ç·¨ ID") ;
        clrtoeol() ;
        pressreturn() ;
        clear() ;
        return 0 ;
    }
    if ((lookupuser->userlevel & PERM_BOARDS ))/*Haohmaru.98.10.05*/
        flag=1;
    if ((lookupuser->userlevel & PERM_CLOAK ))/* Bigman 2000.1.5 */
        flag1=1;
    if ((lookupuser->userlevel & PERM_XEMPT ))
        flag2=1;

/*Bad 2002.7.6 ÊÜÏŞÓë½äÍøÎÊÌâ*/
	basicperm=0;
	sethomefile( genbuf2, lookupuser->userid,"giveup" );
    	fn = fopen(genbuf2, "rt");
    	if (fn) {
	    while(!feof(fn)){
    	        if(fscanf(fn, "%d %d",&i,&j)<=0)break;
    	        s[lcount][0]=i;
    	        s[lcount][1]=j;
    	        switch(i){
    	        	case 1:basicperm|=PERM_BASIC; break;
    	        	case 2:basicperm|=PERM_POST; break;
    	        	case 3:basicperm|=PERM_CHAT; break;
    	        	case 4:basicperm|=PERM_PAGE; break;
    	        	case 5:basicperm|=PERM_DENYMAIL; break;
    	        }
    	        lcount++;
            }
    	    fclose(fn);
    	}

    move(1,0);
    clrtobot();
    move(2,0);
    prints("ÇëÉè¶¨"NAME_USER_SHORT" '%s' µÄÈ¨ÏŞ\n", genbuf);
    newlevel = setperms2(lookupuser->userlevel,basicperm,"È¨ÏŞ",NUMPERMS,showperminfo2);
    move(2,0);
    if (newlevel == lookupuser->userlevel)
        prints(NAME_USER_SHORT" '%s' µÄÈ¨ÏŞÃ»ÓĞ¸ü¸Ä\n", lookupuser->userid);
    else { /* Leeward: 1997.12.02 : Modification starts */
        char        secu[STRLEN];

        sprintf(secu,"ĞŞ¸Ä %s µÄÈ¨ÏŞXPERM%d %d",
                lookupuser->userid, lookupuser->userlevel, newlevel);
        securityreport(secu,lookupuser,NULL);
        lookupuser->userlevel = newlevel;
        /* Leeward: 1997.12.02 : Modification stops */

        prints(NAME_USER_SHORT" '%s' µÄÈ¨ÏŞÒÑ¸ü¸Ä\n",lookupuser->userid) ;
        sprintf(genbuf, "changed permissions for %s", lookupuser->userid);
        report(genbuf);
        /*Haohmaru.98.10.03.¸øĞÂÈÎ°åÖ÷×Ô¶¯·¢ĞÅ*/
        if ((lookupuser->userlevel & PERM_BOARDS ) && flag==0 )
            mail_file(currentuser->userid,"etc/forbm",lookupuser->userid,"ĞÂÈÎ"NAME_BM"±Ø¶Á",0);
        /* Bigman 2000.1.5 ĞŞ¸ÄÈ¨ÏŞ×Ô¶¯·¢ĞÅ */
        if ((lookupuser->userlevel & PERM_CLOAK ) && flag1==0 )
            mail_file(currentuser->userid,"etc/forcloak",lookupuser->userid,NAME_SYSOP_GROUP"ÊÚÓèÄúÒşÉíÈ¨ÏŞ",0);
        if ((lookupuser->userlevel & PERM_XEMPT ) && flag2==0 )
            mail_file(currentuser->userid,"etc/forlongid",lookupuser->userid,NAME_SYSOP_GROUP"ÊÚÓèÄú³¤ÆÚÕÊºÅÈ¨ÏŞ",0);
/*Bad 2002.7.6 ÊÜÏŞÓë½äÍøÎÊÌâ*/
    	kcount=lcount;
    	for(i=0;i<lcount;i++){
    		j=0;
    		switch(s[i][0]){
    			case 1:j=lookupuser->userlevel&PERM_BASIC;break;
    			case 2:j=lookupuser->userlevel&PERM_POST;break;
    			case 3:j=lookupuser->userlevel&PERM_CHAT;break;
    			case 4:j=lookupuser->userlevel&PERM_PAGE;break;
    			case 5:j=!(lookupuser->userlevel&PERM_DENYMAIL);break;
    		}
    		if(j){
    			kcount--;
    			s[i][1]=0;
    		}
    	}
    	if(kcount!=lcount){
    		if(kcount==0) unlink(genbuf2);
    		else{
			fn=fopen(genbuf2, "wt");
            		for(i=0;i<lcount;i++)
            		if(s[i][1]>0) fprintf(fn,"%d %d\n",s[i][0],s[i][1]);
            		fclose(fn);
    		}
    	}
        if(lookupuser->userlevel&PERM_BASIC) tcount++;
        if(lookupuser->userlevel&PERM_POST) tcount++;
        if(lookupuser->userlevel&PERM_CHAT) tcount++;
        if(lookupuser->userlevel&PERM_PAGE) tcount++;
        if(!(lookupuser->userlevel&PERM_DENYMAIL)) tcount++;

        if(kcount+tcount==5&&kcount>0)lookupuser->flags[0]|=GIVEUP_FLAG;
        else lookupuser->flags[0]&=~GIVEUP_FLAG;
    }
    pressreturn() ;
    clear() ;
    return 0 ;
}

int
XCheckLevel() /* Leeward 98.06.05 */
{
    unsigned int newlevel;
    struct userec scanuser;

    if (!HAS_PERM(currentuser,PERM_ADMIN) || !HAS_PERM(currentuser,PERM_SYSOP))
    {
        move( 3, 0 );
        clrtobot();
        prints( "±§Ç¸, ÄúÃ»ÓĞ´ËÈ¨ÏŞ");
        pressreturn();
        return -1;
    }

    modify_user_mode( ADMIN );
    if(!check_systempasswd())
    {
        return -1;
    }
    clear();
    move(0,0) ;
    prints("ÁĞÊ¾¾ßÓĞÌØ¶¨È¨ÏŞµÄ"NAME_USER_SHORT"µÄ×ÊÁÏ\n") ;
    clrtoeol() ;
    move(2,0);
    prints("ÇëÉè¶¨ĞèÒª¼ì²éµÄÈ¨ÏŞ\n");
    scanuser.userlevel = 0;
    newlevel = setperms(scanuser.userlevel,"È¨ÏŞ",NUMPERMS,showperminfoX);
    move(2,0);
    if (newlevel == scanuser.userlevel)
        prints("ÄãÃ»ÓĞÉè¶¨ÈÎºÎÈ¨ÏŞ\n");
    else
    {
        char        secu[STRLEN];
        char        buffer[256];
        int         fhp;
        FILE        *fpx;
        long        count = 0L;

        sprintf(buffer, "tmp/XCL.%s%d", currentuser->userid, getpid());
        if (- 1 == (fhp = open(".PASSWDS", O_RDONLY)))
        {
            prints("ÏµÍ³´íÎó: ÎŞ·¨´ò¿ª¿ÚÁîÎÄ¼ş\n");
        }
        else if (NULL == (fpx = fopen(buffer, "w")))
        {
            close(fhp);
            prints("ÏµÍ³´íÎó: ÎŞ·¨´ò¿ªÁÙÊ±ÎÄ¼ş\n");
        }
        else
        {
            prints("ÁĞÊ¾²Ù×÷¿ÉÄÜĞèÒª½Ï³¤Ê±¼ä²ÅÄÜÍê³É, ÇëÄÍĞÄµÈ´ı. ");
            clrtoeol();
            if (askyn("ÄãÈ·¶¨Òª½øĞĞÁĞÊ¾Âğ", 0))
            {
                while (read(fhp, &scanuser, sizeof(struct userec)) > 0)
                {
                    if ((scanuser.userlevel & newlevel) == newlevel
                            &&  strcmp("SYSOP", scanuser.userid))
                    {
                        count ++;
                        fprintf(fpx,"[1m[33mÇë±£³ÖÕâÒ»ĞĞÎ»ÓÚÆÁÄ»µÚÒ»ĞĞ£¬´ËÊ±°´ X ¼ü¿É¸øÏÂÁĞÓÃ»§·¢ĞÅÒªÇóÆä²¹Æë¸öÈË×¢²á×ÊÁÏ[m\n\n");
                        fprintf(fpx,"ÓÃ»§´úºÅ(êÇ³Æ) : %s(%s)\n\n", scanuser.userid, scanuser.username);
                        fprintf(fpx,"Õæ  Êµ  ĞÕ  Ãû : %s\n\n", scanuser.realname);
                        fprintf(fpx,"¾Ó  ×¡  ×¡  Ö· : %s\n\n", scanuser.address);
                        fprintf(fpx,"µç  ×Ó  ÓÊ  ¼ş : %s\n\n", scanuser.email);
                        fprintf(fpx,"µ¥Î»$µç»°@ÈÏÖ¤ : %s\n\n", scanuser.realemail);
                        fprintf(fpx,"×¢  ²á  ÈÕ  ÆÚ : %s\n", ctime(&scanuser.firstlogin));
                        fprintf(fpx,"×îºóµÄµÇÂ¼ÈÕÆÚ : %s\n", ctime(&scanuser.lastlogin));
                        fprintf(fpx,"×îºóµÄµÇÂ¼»úÆ÷ : %s\n\n", scanuser.lasthost );
                        fprintf(fpx,"ÉÏ  Õ¾  ´Î  Êı : %d ´Î\n\n", scanuser.numlogins);
                        fprintf(fpx,"ÎÄ  ÕÂ  Êı  Ä¿ : %d Æª\n\n", scanuser.numposts);
                    }
                }
                fprintf(fpx, "[1m[33mÒ»¹²ÁĞ³öÁË %ld Ïî¾ßÓĞ´ËÈ¨ÏŞµÄÓÃ»§×ÊÁÏ[m\n\n*** ÕâÊÇÁĞÊ¾½á¹ûµÄ×îºóÒ»ĞĞ£®Èç¹û¼ì²éÍê±Ï£¬Çë°´ q ¼ü½áÊø *** (ÒÔÏÂ¾ùÎª¿ÕĞĞ)", count);
                { int dummy; /* process the situation of a too high screen :PP */
                    for (dummy = 0; dummy < t_lines * 4; dummy ++) fputs("\n", fpx);
                }
                close(fhp);
                fclose(fpx);

                sprintf(secu, "[1m[33mÒ»¹²ÁĞ³öÁË %ld Ïî¾ßÓĞ´ËÈ¨ÏŞµÄÓÃ»§×ÊÁÏ[m", count);
                move(2, 0);
                prints(secu);
                clrtoeol();
                sprintf(genbuf, "listed %ld userlevel of %d", count, newlevel);
                report(genbuf);
                pressanykey();

                /*sprintf(secu, "ÁĞÊ¾¾ßÓĞÌØ¶¨È¨ÏŞµÄ %ld ¸öÓÃ»§µÄ×ÊÁÏ", count);*/
                clear();
                ansimore(buffer, NA);
                clear();
                move(2, 0);
                prints("ÁĞÊ¾²Ù×÷Íê³É");
                clrtoeol();

                unlink(buffer);
            }
            else
            {
                move(2, 0);
                prints("È¡ÏûÁĞÊ¾²Ù×÷");
                clrtoeol();
            }
        }
    }
    pressreturn() ;
    clear() ;
    return 0 ;
}

int
x_userdefine()
{
    int id ;
    unsigned int newlevel;
    extern int nettyNN;
    struct userec* lookupuser;

    modify_user_mode( USERDEF );
    if(!(id = getuser(currentuser->userid,&lookupuser))) {
        move(3,0) ;
        prints("´íÎóµÄ "NAME_USER_SHORT" ID...") ;
        clrtoeol() ;
        pressreturn() ;
        clear() ;
        return 0 ;
    }
    if( !strcmp(currentuser->userid,"guest") )
	return 0;
    move(1,0);
    clrtobot();
    move(2,0);
    use_define=1;
    newlevel = setperms(lookupuser->userdefine,"²ÎÊı",NUMDEFINES,showperminfo);
    move(2,0);
    if (newlevel == lookupuser->userdefine)
        prints("²ÎÊıÃ»ÓĞĞŞ¸Ä...\n");
    else {
        lookupuser->userdefine = newlevel;
        currentuser->userdefine=newlevel;
        if (((convcode)&&(newlevel&DEF_USEGB))  /* KCN,99.09.05 */
                ||((!convcode)&&!(newlevel&DEF_USEGB)))
            switch_code();
        uinfo.pager|=FRIEND_PAGER;
        if(!(uinfo.pager&ALL_PAGER))
        {
            if(!DEFINE(currentuser,DEF_FRIENDCALL))
                uinfo.pager&=~FRIEND_PAGER;
        }
        uinfo.pager&=~ALLMSG_PAGER;
        uinfo.pager&=~FRIENDMSG_PAGER;
        if(DEFINE(currentuser,DEF_FRIENDMSG))
        {
            uinfo.pager|=FRIENDMSG_PAGER;
        }
        if(DEFINE(currentuser,DEF_ALLMSG))
        {
            uinfo.pager|=ALLMSG_PAGER;
            uinfo.pager|=FRIENDMSG_PAGER;
        }
		UPDATE_UTMP(pager,uinfo);
        if(DEFINE(currentuser,DEF_ACBOARD))
            nettyNN=NNread_init();
        prints("ĞÂµÄ²ÎÊıÉè¶¨Íê³É...\n\n") ;
    }
    iscolor=(DEFINE(currentuser,DEF_COLOR))?1:0;
    pressreturn() ;
    clear() ;
    use_define=0;
    return 0 ;
}




int
x_cloak()
{
    modify_user_mode( GMENU );
    report("toggle cloak");
    uinfo.invisible = (uinfo.invisible)?NA:YEA ;
	UPDATE_UTMP(invisible,uinfo);
    if (!uinfo.in_chat) {
        move(1,0) ;
        clrtoeol();
        prints( "ÒşÉíÊõ (cloak) ÒÑ¾­%sÁË!",
                (uinfo.invisible) ? "Æô¶¯" : "Í£Ö¹" ) ;
        pressreturn();
    }
    return 0 ;
}

int
x_date()
{
    time_t t;

    modify_user_mode( XMENU );
    clear();
    move(8,0);
    time(&t);
    prints("Ä¿Ç°ÏµÍ³ÈÕÆÚÓëÊ±¼ä: %s", ctime(&t));
    clrtoeol();
    pressreturn();
    return 0;
}

void
x_edits()
{
    int aborted;
    char ans[7],buf[STRLEN];
    int ch,num;
    char *e_file[]={"plans","signatures","notes","logout",NULL};
    char *explain_file[]={"¸öÈËËµÃ÷µµ","Ç©Ãûµµ","×Ô¼ºµÄ±¸ÍüÂ¼","ÀëÕ¾µÄ»­Ãæ",NULL};

    modify_user_mode( GMENU );
    clear();
    move(1,0);
    prints("±àĞŞ¸öÈËµµ°¸\n\n");
    for(num=0;e_file[num]!=NULL&&explain_file[num]!=NULL;num++)
    {
        prints("[[32m%d[m] %s\n",num+1,explain_file[num]);
    }
    prints("[[32m%d[m] ¶¼²»Ïë¸Ä\n",num+1);

    getdata(num+5,0,"ÄãÒª±àĞŞÄÄÒ»Ïî¸öÈËµµ°¸: ",ans,2,DOECHO,NULL,YEA);
    if(ans[0]-'0'<=0 || ans[0]-'0'>num|| ans[0]=='\n'|| ans[0]=='\0')
        return;

    ch=ans[0]-'0'-1;
    switch( ch ){
    	case 0:
        	modify_user_mode( EDITPLAN );
	case 1:
	    	modify_user_mode( EDITSIG );
    }

    sethomefile(genbuf,currentuser->userid,e_file[ch]);
    move(3,0);
    clrtobot();
    sprintf(buf,"(E)±à¼­ (D)É¾³ı %s? [E]: ",explain_file[ch]);
    getdata(3,0,buf,ans,2,DOECHO,NULL,YEA);
    if (ans[0] == 'D' || ans[0] == 'd') {
        unlink(genbuf);
        move(5,0);
        prints("%s ÒÑÉ¾³ı\n",explain_file[ch]);
        sprintf(buf,"delete %s",explain_file[ch]);
        report(buf);
        pressreturn();
        clear();
        return;
    }
    modify_user_mode( EDITUFILE);
    aborted = vedit(genbuf, NA);
    clear();
    if (!aborted) {
        prints("%s ¸üĞÂ¹ı\n",explain_file[ch]);
        sprintf(buf,"edit %s",explain_file[ch]);
        if(!strcmp(e_file[ch],"signatures"))
        {
            set_numofsig();
            prints("ÏµÍ³ÖØĞÂÉè¶¨ÒÔ¼°¶ÁÈëÄãµÄÇ©Ãûµµ...");
        }
        report(buf);
    }else
        prints("%s È¡ÏûĞŞ¸Ä\n",explain_file[ch]);
    pressreturn();
}

void
a_edits()
{
    int aborted;
    char ans[7],buf[STRLEN];
    int ch,num;

    /* Leeward 98.04.01 added: sysconf.ini */
    /* Leeward 98.07.31 added: .badIP */
    /* stephen 2000.10.17 added: /usr/share/apache/htdocs/script/menucontext.js */
    /* period  2000.10.17 link /backup/www/htdocs/script/menucontext.js --> /home0/bbs/etc/www_menu.js */
    static const char *e_file[]={"../Welcome","../vote/notes","issue","movie","logout","menu.ini", "proxyIP", "mailcheck","s_fill","f_fill.realname","f_fill.unit","f_fill.address","f_fill.telephone","f_fill.real","f_fill.chinese","f_fill.toomany","f_fill.proxy","smail","f_fill","../.badname", "../.badIP", "../.badword", "sysconf.ini", "www_menu.js", "../0Announce/hotinfo", "forbm", "forcloak", "forlongid", "../innd/newsfeeds.bbs", NULL};
    /* "/usr/share/apache/htdocs/script/menucontext.js", NULL};    */
	static const char *explain_file[]={"Welcome","¹«ÓÃ±¸ÍüÂ¼","½øÕ¾»¶Ó­µµ","»î¶¯¿´°æ","ÀëÕ¾»­Ãæ", "menu.ini","´©ËóIP","Éí·İÈ·ÈÏµµ","×¢²áµ¥Íê³Éµµ","×¢²áµ¥Ê§°Üµµ(ÕæÊµĞÕÃû)","×¢²áµ¥Ê§°Üµµ(·şÎñµ¥Î»)","×¢²áµ¥Ê§°Üµµ(¾Ó×¡µØÖ·)","×¢²áµ¥Ê§°Üµµ(ÁªÂçµç»°)","×¢²áµ¥Ê§°Üµµ(ÕæÊµ×ÊÁÏ)","×¢²áµ¥Ê§°Üµµ(ÖĞÎÄÌîĞ´)","×¢²áµ¥Ê§°Üµµ(¹ı¶àµÄID)","×¢²áµ¥Ê§°Üµµ(²»ÄÜ´©Ëó×¢²á)" ,"Éí·İÈ·ÈÏÍê³Éµµ        ","Éí·İÈ·ÈÏÊ§°Üµµ","²»¿É×¢²áµÄ ID         ", "²»¿ÉµÇÂ¼µÄ IP", "ÏµÍ³×Ô¶¯¹ıÂËµÄ´ÊÓï    ", "sysconf.ini", "WWWÖ÷²Ëµ¥             ", "½üÆÚÈÈµã","¸øĞÂÈÎ°æÖ÷µÄĞÅ        ", "¸øÒşÉíÓÃ»§µÄĞÅ","¸ø³¤ÆÚÓÃ»§µÄĞÅ        ", "×ªĞÅ°æºÍĞÂÎÅ×é¶ÔÓ¦", NULL};

    modify_user_mode( ADMIN );
    if(!check_systempasswd())
    {
        return;
    }
    clear();
    move(0,0);
    prints("±àĞŞÏµÍ³µµ°¸\n\n");
    for(num=0;e_file[num]!=NULL&&explain_file[num]!=NULL;num++)
    {
        prints("[[32m%2d[m] %s%s",num+1,explain_file[num],
               (num + 1 >= 1 && num + 1 <= 28 && (num + 1) % 2) ? "      " : "\n");
        /* Leeward 98.03.29 µ÷ÕûÏÔÊ¾²¼¾Ö£¬ÒÔ±ã¼ÓÈë¡°ÏµÍ³×Ô¶¯¹ıÂËµÄ´ÊÓï¡±Ò»Ïî */
        /* Leeward 98.07.31 µ÷ÕûÏÔÊ¾²¼¾Ö£¬ÒÔ±ã¼ÓÈë¡°²»¿ÉµÇÂ¼µÄ IP¡±Ò»Ïî */
	/* Bigman 2001.6.23 µ÷Õû²¼¾Ö£¬ Ôö¼ÓÆäËû¼¸ÏîÄÚÈİ */
    }
	
    prints("[[32m%2d[m] ¶¼²»Ïë¸Ä\n",num+1);

	/* ÏÂÃæµÄ21ÊÇĞĞºÅ£¬ÒÔºóÌí¼Ó£¬¿ÉÒÔÏàÓ¦µ÷Õû */
    getdata(21,0,"ÄãÒª±àĞŞÄÄÒ»ÏîÏµÍ³µµ°¸: ",ans,3,DOECHO,NULL,YEA);
    ch=atoi(ans);
    if(!isdigit(ans[0])||ch<=0 || ch>num|| ans[0]=='\n'|| ans[0]=='\0')
        return;
    ch-=1;
    sprintf(genbuf,"etc/%s",e_file[ch]);
    move(2,0);
    clrtobot();
    sprintf(buf,"(E)±à¼­ (D)É¾³ı %s? [E]: ",explain_file[ch]);
    getdata(3,0,buf,ans,2,DOECHO,NULL,YEA);
    if (ans[0] == 'D' || ans[0] == 'd') {
        {
            char        secu[STRLEN];
            sprintf(secu,"É¾³ıÏµÍ³µµ°¸£º%s",explain_file[ch]);
            securityreport(secu,NULL,NULL);
        }
        unlink(genbuf);
        move(5,0);
        prints("%s ÒÑÉ¾³ı\n",explain_file[ch]);
        sprintf(buf,"delete %s",explain_file[ch]);
        report(buf);
        pressreturn();
        clear();
        return;
    }
    modify_user_mode( EDITSFILE);
    aborted = vedit(genbuf, NA);
    clear();
    if (aborted!=-1) {
        prints("%s ¸üĞÂ¹ı",explain_file[ch]);
        sprintf(buf,"edit %s",explain_file[ch]);
        report(buf);
        {
            char        secu[STRLEN];
            sprintf(secu,"ĞŞ¸ÄÏµÍ³µµ°¸£º%s",explain_file[ch]);
            securityreport(secu,NULL,NULL);
        }

        if(!strcmp(e_file[ch],"../Welcome"))
        {
            unlink("Welcome.rec");
            prints("\nWelcome ¼ÇÂ¼µµ¸üĞÂ");
        }
    }
    pressreturn();
}

#ifdef BBSDOORS

void
ent_bnet()  /* Bill Schwartz */
{
    int save_pager = uinfo.pager;
    uinfo.pager = -1;
    report("BBSNet Enter") ;
    modify_user_mode( BBSNET );
    /* bbsnet.sh is a shell script that can be customized without */
    /* having to recompile anything.  If you edit it while someone */
    /* is in bbsnet they will be sent back to the xyz menu when they */
    /* leave the system they are currently in. */

    do_exec("bbsnet.sh",NULL) ;
    uinfo.pager = save_pager;
    report("BBSNet Exit");
    clear() ;
}

#endif
static void escape_filename(char *fn)
{
     static const  char invalid[] = {'/','\\','!','&','|','*','?','`','\'','\"',';','<','>',':'};
     int i,j;
     for (i=0;i<strlen(fn);i++)
     	  for (j=0;j<sizeof(invalid);j++)
     	  	  if (fn[i] == invalid[j]) fn[i] = '_';
}
int zsend_file( char *filename, char *title )
{
	char *t;
	char buf[100],buf1[100];
	int i;
	ansimore("etc/zmodem",0);
       move(t_lines-1, 0);
       clrtoeol();
       strcpy(buf,"N");
       getdata( t_lines-1, 0, "ÄúÈ·¶¨ÒªÊ¹ÓÃZmodem´«ÊäÎÄ¼şÃ´?[y/N]", buf, 2, DOECHO, NULL ,YEA);
       if (toupper(buf[0])  != 'Y') return FULLUPDATE;
       strncpy(buf,title,76) ;
       buf[80] = '\0';
       escape_filename(buf);
       strcat(buf,".TXT");
       move(t_lines-2, 0);
       clrtoeol();
       prints("ÇëÊäÈëÎÄ¼şÃû£¬Îª¿ÕÔò·ÅÆú");
       move(t_lines-1, 0);
       clrtoeol();
       getdata( t_lines-1, 0, "", buf, 78, DOECHO, NULL ,0);
       if (buf[0] == '\0') return FULLUPDATE;
       buf[78] = '\0';
       escape_filename(buf);
       sprintf(buf1,"SMTH-%s-",currboard);
       strcat(buf1,buf);
	bbs_zsendfile(filename,buf1);
	return FULLUPDATE;
}

