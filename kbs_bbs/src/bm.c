
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
#include <time.h>
#include <ctype.h>

extern cmpbnames();

/*Add by SmallPig*/
char filebuf[STRLEN];
char deadline[STRLEN];/*²¹³äËµÃ÷*/
char denymsg[STRLEN];
int  denyday;

int
listdeny(int page)  /* Haohmaru.12.18.98.ÎªÄÇÐ©±äÌ¬µÃ·âÈË³¬¹ýÒ»ÆÁµÄ°åÖ÷¶øÐ´ */
{
    FILE *fp;
    int x = 0, y = 3, cnt = 0, max = 0, len;
    int i;
    char u_buf[STRLEN*2], line[STRLEN*2], *nick;

    clear();
    prints("Éè¶¨ÎÞ·¨ Post µÄÃûµ¥\n");
    move(y,x);
    CreateNameList();
    setbfile( genbuf, currboard,"deny_users" );
    if ((fp = fopen(genbuf, "r")) == NULL) {
        prints("(none)\n");
        return 0;
    }
    for(i=1;i<=page*20;i++)
    {
        if(fgets(genbuf, 2*STRLEN, fp)==NULL)
            break;
    }
    while(fgets(genbuf, 2*STRLEN, fp) != NULL)
    {
        strtok( genbuf, " \n\r\t" );
        strcpy( u_buf, genbuf );
        AddNameList( u_buf );
        nick = (char *) strtok( NULL, "\n\r\t" );
        if( nick != NULL )
        {
            while( *nick == ' ' )  nick++;
            if( *nick == '\0' )  nick = NULL;
        }
        if( nick == NULL )
        {
            strcpy( line, u_buf );
        } else
        {
            if (cnt<20)
                sprintf( line, "%-12s%s", u_buf, nick );
        }
        if( (len = strlen( line )) > max )
            max = len;
/*        if( x + len > 90 )
            line[ 90 - x ] = '\0';*-P-*/
        if( x + len > 79 ) line[ 79 ] = '\0';
        if (cnt<20)/*haohmaru.12.19.98*/
            prints( "%s", line );
        cnt++;
        if ((++y) >= t_lines-1)
        {
            y = 3;
            x += max + 2;
            max = 0;
            /*            if( x > 90 )  break;*/
        }
        move(y,x);
    }
    fclose(fp);
    if (cnt == 0) prints("(none)\n");
    return cnt;
}

int
addtodeny(uident) /* Ìí¼Ó ½ûÖ¹POSTÓÃ»§ */
char *uident;
{
    char buf2[50],strtosave[256],date[STRLEN]="0";
    int maxdeny;
    /*Haohmaru.99.4.1.auto notify*/
    time_t  now;
    int  id;
    char buffer[STRLEN];
    FILE *fn;
    char filename[STRLEN];
    int day, autofree = 0;

    now=time(0);
    strncpy(date,ctime(&now)+4,7);
    setbfile( genbuf, currboard,"deny_users" );
    if( seek_in_file(genbuf, uident) || !strcmp(currboard, "denypost"))
        return -1;
    if (HAS_PERM(currentuser,PERM_SYSOP)||HAS_PERM(currentuser,PERM_OBOARDS))
        maxdeny=70;
    else
        maxdeny=14;

/*MUST:*/
    *denymsg = 0;
    while(0 == strlen(denymsg)) {
	    getdata(2,0,"ÊäÈëËµÃ÷(°´*È¡Ïû): ", denymsg,30,DOECHO,NULL,YEA);
    }
/*	  if (0==strlen(denymsg)) goto MUST;*/
    if (denymsg[0]=='*')
        return 0;
#ifdef DEBUG
    autofree = askyn("¸Ã·â½ûÊÇ·ñ×Ô¶¯½â·â£¿", YEA);
#else
    if (HAS_PERM(currentuser,PERM_SYSOP)||HAS_PERM(currentuser,PERM_OBOARDS))
        sprintf(filebuf,"ÊäÈëÌìÊý(0-ÊÖ¶¯½â·â£¬×î³¤%dÌì)",maxdeny);
    else
#endif /*DEBUG*/
        sprintf(filebuf,"ÊäÈëÌìÊý(×î³¤%dÌì)",maxdeny);
/*MUST1:*/
    denyday = 0;
    while(!denyday) {
        getdata(3,0,filebuf, buf2,4,DOECHO,NULL,YEA);
        if ((buf2[0]<'0')||(buf2[0]>'9')) continue; /*goto MUST1;*/
        denyday=atoi(buf2);
        if ((denyday<0)||(denyday>maxdeny)) denyday = 0; /*goto MUST1;*/
        else if (!(HAS_PERM(currentuser,PERM_SYSOP)||HAS_PERM(currentuser,PERM_OBOARDS))&&!denyday)
            denyday = 0; /*goto MUST1;*/
        else if ((HAS_PERM(currentuser,PERM_SYSOP)||HAS_PERM(currentuser,PERM_OBOARDS)) && !denyday && !autofree)
            break;
    }

#ifdef DEBUG
    if(denyday && autofree) {
#else
    if (denyday) {
#endif /*DEBUG*/
        struct tm* tmtime;
        time_t undenytime=now+denyday*24*60*60;
/*        time_t daytime=now+(day+1)*24*60*60;
        now=time(0);*/
        tmtime=gmtime(&undenytime);

        sprintf( strtosave, "%-12.12s %-30.30s%-12.12s %2dÔÂ%2dÈÕ½â\x1b[%um",
                uident, denymsg ,currentuser->userid,
                tmtime->tm_mon+1,tmtime->tm_mday, undenytime);/*Haohmaru 98,09,25,ÏÔÊ¾ÊÇË­Ê²Ã´Ê±ºò·âµÄ */
    } else {
        struct tm* tmtime;
#ifdef DEBUG
        time_t undenytime = now+denyday*24*60*60;
        tmtime=gmtime(&undenytime);
        sprintf( strtosave, "%-12.12s %-30.30s%-12.12s %2dÔÂ%2dÈÕºóÊÖ¶¯½â·â\x1b[%um",
                uident, denymsg, currentuser->userid,
                tmtime->tm_mon+1,tmtime->tm_mday, undenytime);
#else
        now=time(0);
        tmtime=gmtime(&now);
        sprintf( strtosave, "%-12.12s %-30.30s%-12.12s at %2dÔÂ%2dÈÕ ÊÖ¶¯½â·â",
                uident, denymsg, currentuser->userid,
                tmtime->tm_mon+1,tmtime->tm_mday);
#endif
    }

/*
MUST2:
    getdata(4,0,"ÊäÈë²¹³äËµÃ÷: ", buf,60,DOECHO,NULL,YEA);
    if (0==strlen(buf))
	goto MUST2;
    sprintf(deadline,"%-60s",buf);
*/
    return addtofile(genbuf,strtosave);
}

int
deldeny(uident)  /* É¾³ý ½ûÖ¹POSTÓÃ»§ */
char *uident;
{
    char fn[STRLEN];
    FILE* fn1;
    char filename[STRLEN];
    char buffer[STRLEN];
    time_t now;
    struct userec* lookupuser;

    now=time(0);
    setbfile( fn,currboard, "deny_users" );
    /*Haohmaru.4.1.×Ô¶¯·¢ÐÅÍ¨Öª*/
    sprintf(filename,"etc/%s.dny",currentuser->userid);
    fn1=fopen(filename,"w");
    if (HAS_PERM(currentuser,PERM_SYSOP)||HAS_PERM(currentuser,PERM_OBOARDS))
    {sprintf(buffer,"[Í¨Öª]");
        fprintf(fn1,"¼ÄÐÅÈË: %s \n",currentuser->userid) ;
        fprintf(fn1,"±ê  Ìâ: %s\n",buffer) ;
        fprintf(fn1,"·¢ÐÅÕ¾: %s (%24.24s)\n","BBS "NAME_BBS_CHINESE"Õ¾",ctime(&now)) ;
        fprintf(fn1,"À´  Ô´: %s \n",currentuser->lasthost) ;
        fprintf(fn1,"\n");
        fprintf(fn1,"Äú±»Õ¾ÎñÈËÔ± %s ½â³ýÔÚ %s °åµÄ·â½û\n",currentuser->userid,currboard);
    }
    else
    {
        sprintf(buffer,"[Í¨Öª]",currboard,currentuser->userid);
        fprintf(fn1,"¼ÄÐÅÈË: %s \n",currentuser->userid) ;
        fprintf(fn1,"±ê  Ìâ: %s\n",buffer) ;
        fprintf(fn1,"·¢ÐÅÕ¾: %s (%24.24s)\n","BBS "NAME_BBS_CHINESE"Õ¾",ctime(&now)) ;
        fprintf(fn1,"À´  Ô´: %s \n",currentuser->lasthost) ;
        fprintf(fn1,"\n");
        fprintf(fn1,"Äú±» %s °å°åÖ÷ %s ½â³ý·â½û\n",currboard,currentuser->userid);
    }
    fclose(fn1);

    /*½â·âÍ¬Ñù·¢ÎÄµ½undenypost°æ  Bigman:2000.6.30*/ 
    getuser(uident,&lookupuser); 
    if (lookupuser==NULL) 
    	sprintf(buffer,"%s ½â·âËÀµôµÄÕÊºÅ %s ÔÚ %s ",currentuser->userid,uident,currboard); 
    else {
      if (PERM_BOARDS & lookupuser->userlevel)
    	sprintf(buffer,"%s ½â·âÄ³°å°åÖ÷ %s ÔÚ %s ",currentuser->userid,lookupuser->userid,currboard); 
      else
        sprintf(buffer,"%s ½â·â %s ÔÚ %s",currentuser->userid,lookupuser->userid,currboard);
      mail_file(currentuser->userid,filename,uident,buffer,0);
    }
    postfile(filename,"undenypost",buffer,1);
    unlink(filename);
    return del_from_file(fn,lookupuser?lookupuser->userid:uident);
}

int
deny_user(ent, fileinfo, direct)  /* ½ûÖ¹POSTÓÃ»§Ãûµ¥ Î¬»¤Ö÷º¯Êý*/
int ent ;
struct fileheader *fileinfo ;
char *direct ;
{
    char uident[STRLEN];
    int page=0;
    char ans[10],repbuf[STRLEN];
    int count;
    /*Haohmaru.99.4.1.auto notify*/
    time_t  now;
    int  id;
    char buffer[STRLEN];
    FILE *fn,*fp;
    char filename[STRLEN];
    int  find;/*Haohmaru.99.12.09*/
    char *idindex;/*Haohmaru.99.12.09*/
    int my_flag=0;	/* Bigman. 2001.2.19 */

    /*   static page=0;*//*Haohmaru.12.18*/
    now=time(0);
    if(!HAS_PERM(currentuser,PERM_SYSOP))
        if(!chk_currBM(currBM,currentuser))
        {
            return DONOTHING;
        }

    while (1) {
    	char querybuf[0xff];
Here:
	clear();
        count = listdeny(0);
        if (count>0 && count<20)/*Haohmaru.12.18,¿´ÏÂÒ»ÆÁ*/
	    snprintf(querybuf,0xff,"(O)Ôö¼Ó%s (A)Ôö¼Ó (D)É¾³ý or (E)Àë¿ª [E]: ",fileinfo->owner);
	else if (count>20)
	    snprintf(querybuf, 0xff, "(O)Ôö¼Ó%s (A)Ôö¼Ó (D)É¾³ý (N)ºóÃæµÚNÆÁ or (E)Àë¿ª [E]: ",fileinfo->owner);
	else 
	    snprintf(querybuf, 0xff, "(O)Ôö¼Ó%s (A)Ôö¼Ó or (E)Àë¿ª [E]: ",fileinfo->owner);

        getdata(1,0, querybuf, ans,7,DOECHO,NULL,YEA);
    *ans=(char)toupper((int)*ans);
    
	if (*ans == 'A' || *ans == 'O' ) {
            struct userec saveuser;
            move(1,0);
            if (*ans=='A')
		    	usercomplete("Ôö¼ÓÎÞ·¨ POST µÄÊ¹ÓÃÕß: ", uident);
		    else strncpy(uident, fileinfo->owner, STRLEN-4);
            /*Haohmaru.99.4.1,Ôö¼Ó±»·âIDÕýÈ·ÐÔ¼ì²é*/
            if(!(id = searchuser(uident)))  /* change getuser -> searchuser , by dong, 1999.10.26 */
            {
                move(3,0) ;
                prints("·Ç·¨ ID") ;
                clrtoeol() ;
                pressreturn() ;
                goto Here;
            }
            if( *uident != '\0' )
            {   sprintf(filebuf,"%-12s","Î¥·´Ä³ÌõÕ¾¹æ");
                if(addtodeny(uident)==1)
                {
                	struct userec* lookupuser,*saveptr;
                    sprintf(repbuf,"%s È¡Ïû %s ÔÚ %s µÄ POST È¨Á¦",
                            currentuser->userid,uident,currboard);
                    report(repbuf);

                    /*Haohmaru.4.1.×Ô¶¯·¢ÐÅÍ¨Öª²¢·¢ÎÄÕÂÓÚ°åÉÏ*/
                    sprintf(filename,"etc/%s.deny",currentuser->userid);
                    fn=fopen(filename,"w+");
                    memcpy(&saveuser,currentuser,sizeof(struct userec));
                    saveptr = currentuser;
                    currentuser = &saveuser;
                    sprintf(buffer,"%s±»È¡ÏûÔÚ%s°æµÄ·¢ÎÄÈ¨ÏÞ",uident,currboard);

                    if ((HAS_PERM(currentuser,PERM_SYSOP)||HAS_PERM(currentuser,PERM_OBOARDS)) && !chk_BM_instr(currBM,currentuser->userid))
                    {	   my_flag=0;
                        fprintf(fn,"¼ÄÐÅÈË: SYSOP (System Operator) \n") ;
                        fprintf(fn,"±ê  Ìâ: %s\n",buffer) ;
                        fprintf(fn,"·¢ÐÅÕ¾: %s (%24.24s)\n","BBS "NAME_BBS_CHINESE"Õ¾",ctime(&now)) ;
                        fprintf(fn,"À´  Ô´: smth.org\n") ;
                        fprintf(fn,"\n");
                        fprintf(fn,"ÓÉÓÚÄúÔÚ \x1b[4m%s\x1b[0m °æ \x1b[4m%s\x1b[0m£¬ÎÒºÜÒÅº¶µØÍ¨ÖªÄú£¬ \n",currboard,denymsg);
                        if (denyday)
                            fprintf(fn,"Äú±»ÔÝÊ±È¡ÏûÔÚ¸Ã°æµÄ·¢ÎÄÈ¨Á¦ \x1b[4m%d\x1b[0m Ìì£¬µ½ÆÚºóÇë»Ø¸´\n",denyday);
                        else
                            fprintf(fn,"Äú±»ÔÝÊ±È¡ÏûÔÚ¸Ã°æµÄ·¢ÎÄÈ¨Á¦£¬µ½ÆÚºóÇë»Ø¸´\n");
                        fprintf(fn,"´ËÐÅÉêÇë»Ö¸´È¨ÏÞ¡£\n");
                        fprintf(fn,"\n");
                        fprintf(fn,"                            "NAME_BBS_CHINESE NAME_SYSOP_GROUP"Öµ°àÕ¾Îñ£º\x1b[4m%s\x1b[0m\n",currentuser->userid);
                        fprintf(fn,"                              %s\n",ctime(&now));
                        strcpy(currentuser->userid,"SYSOP");
                        strcpy(currentuser->username,NAME_SYSOP);
                        strcpy(currentuser->realname,NAME_SYSOP);
                    }
                    else
                    {		my_flag=1;
                        fprintf(fn,"¼ÄÐÅÈË: %s \n",currentuser->userid) ;
                        fprintf(fn,"±ê  Ìâ: %s\n",buffer) ;
                        fprintf(fn,"·¢ÐÅÕ¾: %s (%24.24s)\n","BBS "NAME_BBS_CHINESE"Õ¾",ctime(&now)) ;
                        fprintf(fn,"À´  Ô´: %s \n",currentuser->lasthost) ;
                        fprintf(fn,"\n");
                        fprintf(fn,"ÓÉÓÚÄúÔÚ \x1b[4m%s\x1b[0m °æ \x1b[4m%s\x1b[0m£¬ÎÒºÜÒÅº¶µØÍ¨ÖªÄú£¬ \n",currboard,denymsg);
                        if (denyday)
                            fprintf(fn,"Äú±»ÔÝÊ±È¡ÏûÔÚ¸Ã°æµÄ·¢ÎÄÈ¨Á¦ \x1b[4m%d\x1b[0m Ìì£¬µ½ÆÚºóÇë»Ø¸´\n",denyday);
                        else
                            fprintf(fn,"Äú±»ÔÝÊ±È¡ÏûÔÚ¸Ã°æµÄ·¢ÎÄÈ¨Á¦£¬µ½ÆÚºóÇë»Ø¸´\n");
                        fprintf(fn,"´ËÐÅÉêÇë»Ö¸´È¨ÏÞ¡£\n");
                        fprintf(fn,"\n");
                        fprintf(fn,"                              "NAME_BM":\x1b[4m%s\x1b[0m\n",currentuser->userid);
                        fprintf(fn,"                              %s\n",ctime(&now));
                    }
                    fclose(fn);
                    mail_file(currentuser->userid,filename,uident,buffer,0);
                    fn=fopen(filename,"w+");
                    fprintf(fn,"ÓÉÓÚ \x1b[4m%s\x1b[0m ÔÚ \x1b[4m%s\x1b[0m °æµÄ \x1b[4m%s\x1b[0m ÐÐÎª£¬\n",uident,currboard,denymsg);
                    if (denyday)
                        fprintf(fn,"±»ÔÝÊ±È¡ÏûÔÚ±¾°æµÄ·¢ÎÄÈ¨Á¦ \x1b[4m%d\x1b[0m Ìì¡£\n",denyday);
                    else
                        fprintf(fn,"Äú±»ÔÝÊ±È¡ÏûÔÚ¸Ã°æµÄ·¢ÎÄÈ¨Á¦£¬µ½ÆÚºóÇë»Ø¸´\n");

                    if (my_flag==0)
                    {
                        fprintf(fn,"                            "NAME_BBS_CHINESE NAME_SYSOP_GROUP"Öµ°àÕ¾Îñ£º\x1b[4m%s\x1b[0m\n",saveptr->userid);
                    }
                    else
                    {
                        fprintf(fn,"                              "NAME_BM":\x1b[4m%s\x1b[0m\n",currentuser->userid);
                    }
                    fprintf(fn,"                              %s\n",ctime(&now));
                    fclose(fn);
                    postfile(filename,currboard,buffer,2);
                    /*	unlink(filename); */
                    currentuser = saveptr;

                    sprintf(buffer,"%s ±» %s ·â½û±¾°åPOSTÈ¨",uident,currentuser->userid);
                    getuser(uident,&lookupuser);

                    if(PERM_BOARDS & lookupuser->userlevel)
                        sprintf(buffer,"%s ·âÄ³°å"NAME_BM" %s ÔÚ %s",currentuser->userid,uident,currboard);
                    else
                        sprintf(buffer,"%s ·â %s ÔÚ %s",currentuser->userid,uident,currboard);
                    postfile(filename,"denypost",buffer,8);
                    unlink(filename);
                }
            }
        } else if ((*ans == 'D' ) && count) {
		int len;
            move(1,0);
            /*           namecomplete("É¾³ýÎÞ·¨ POST µÄÊ¹ÓÃÕß: ", uident);by Haohmaru.99.4.1.ÕâÖÖÉ¾³ý·¨»áÎóÉ¾Í¬×ÖÄ¸¿ªÍ·µÄID
            	     usercomplete("É¾³ýÎÞ·¨ POST µÄÊ¹ÓÃÕß: ", uident);Haohmaru.faint...Õâ¸öº¯ÊýÒ²»á³ö´í£,±ÈÈç·âµÄÊ±ºòID½ÐUSAleader,ºóÀ´ID±»É¾£¬ÓÐÈËÓÖ×¢²áÁË¸öusaleader,ÓÚÊÇ¾Í½â²»ÁË,´óÐ¡Ð´ÒýÆðµÄ£¬Boy°å¾Í³öÏÖ¹ýÕâÖÖÇé¿ö£¬ËùÒÔ¸Ä³ÉÏÂÃæµÄ¡£
            	     Haohmaru.99.4.1,Ôö¼Ó±»½âIDÕýÈ·ÐÔ¼ì²é*/
            /*          if(!(id = searchuser(uident)))  change getuser -> searchuser, by dong, 1999.10.26 */
            sprintf(genbuf,"É¾³ýÎÞ·¨ POST µÄÊ¹ÓÃÕß: ");
            getdata(1, 0, genbuf, uident, 13, DOECHO, NULL,YEA);
            find = 0;/*Haohmaru.99.12.09.Ô­À´µÄ´úÂëÈç¹û±»·âÕßÒÑ×ÔÉ±¾ÍÉ¾²»µôÁË*/
            setbfile( genbuf, currboard,"deny_users" );
            if ((fp = fopen(genbuf, "r")) == NULL)
            {
                prints("(none)\n");
                return 0;
            }
	    len = strlen(uident);
            while(fgets(genbuf, 256/*STRLEN*/, fp) != NULL)
            {
		if (!strncasecmp(genbuf,uident,len)) {
                  if (genbuf[len] == 32) {
	            strncpy(uident,genbuf,len);
		    uident[len]=0;
		    find=1;
		    break;
		  }
		}
            }
            fclose(fp);
            if(!find)
            {
                move(3,0) ;
                prints("¸ÃID²»ÔÚ·â½ûÃûµ¥ÄÚ!");
                clrtoeol() ;
                pressreturn() ;
                goto Here;
            }
            /*---	add to check if undeny time reached.	by period 2000-09-11	---*/
            {
                char lbuf[256], * lptr;
                time_t ldenytime;
                /* now the corresponding line in genbuf */
                if( NULL != (lptr = strrchr(genbuf, '[')) )
                    sscanf(lptr + 1, "%lu", &ldenytime);
                else ldenytime = now + 1;
                if(ldenytime > now) {
                    move(3, 0);
                    prints(genbuf);
                    if(NA == askyn("¸ÃÓÃ»§·â½ûÊ±ÏÞÎ´µ½£¬È·ÈÏÒª½â·â£¿", NA/*, NA*/))
                        goto Here;	/* I hate Goto!!! */
                }
            }
            /*---		---*/
            move(1,0);
            clrtoeol();
            if (uident[0] != '\0')
            {
                if(deldeny(uident))
                {
                    sprintf(repbuf,"%s »Ö¸´ %s ÔÚ %s µÄ POST È¨Á¦",
                            currentuser->userid,uident,currboard);
                    report(repbuf);
                }
            }
        }
        else if (count>20 )
        {
            page=*ans-'0';
            if(page<0 || page>10) break;/*²»»á·âÈË³¬¹ý10ÆÁ°É?ÄÇ¿ÉÊÇ200ÈË°¡!*/
            listdeny(page);
            pressanykey();
        }
        else  break;
    } /*end of while*/
    clear();
    return FULLUPDATE;
}

#ifndef LEEWARD_X_FILTER
/* Leeward added below 98.04.02 */
int
BoardFilter()
{
    int aborted;
    char ans[7],buf[STRLEN],bufX[STRLEN];
    int ch,num;

    char *e_file[]={".badword", NULL};
    char *explain_file[]={bufX, NULL};

    if(!HAS_PERM(currentuser,PERM_SYSOP) && !HAS_PERM(currentuser,PERM_OBOARDS))
        return DONOTHING;
    if(!check_systempasswd())
        return DONOTHING;

    modify_user_mode( ADMIN );
    clear();
    move(0,0);
    sprintf(bufX, "%s °å×Ô¶¯¹ýÂËµÄ´ÊÓï",currboard);
    prints("±àÐÞ %s °æµÄÏµÍ³µµ°¸\n\n",currboard);
    for(num=0;e_file[num]!=NULL&&explain_file[num]!=NULL;num++)
    {
        prints("[[32m%2d[m] %s%s",num+1,explain_file[num],
               (num + 1 >= 9 && num + 1 <= 16 && (num + 1) % 2) ? "      " : "\n");
        /* Leeward 98.03.29 µ÷ÕûÏÔÊ¾²¼¾Ö£¬ÒÔ±ã¼ÓÈë¡°ÏµÍ³×Ô¶¯¹ýÂËµÄ´ÊÓï¡±Ò»Ïî */
    }

    /* ºÄ×Ó£ºÇå³ýÎÞ·¨Çø¶ÎÉ¾³ýµÄ´íÎóµÄ´úÂë²»Ó¦¸Ã·ÅÔÚ´¦Àí¹ýÂË´Ê»ãµÄº¯ÊýÖÐ£¬
             ·ñÔò½á¹¹¾Í»ìÂÒÁË(Leeward 99.07.25) */
    /*   prints("[[32m%2d[m] Çå³ýÎÞ·¨Çø¶ÎÉ¾³ýµÄ´íÎó(ÇëÈ·ÈÏÄ¿Ç°Ã»ÓÐÈËÔÚ±¾°åÖ´ÐÐÇø¶ÎÉ¾³ý)\n",num+1);*/
    prints("[[32m%2d[m] ¶¼²»Ïë¸Ä\n",num+1);

    getdata(num+4,0,"ÄãÒª±àÐÞÄÄÒ»ÏîÏµÍ³µµ°¸: ",ans,3,DOECHO,NULL,YEA);
    ch=atoi(ans);
    if(!isdigit(ans[0])||ch<=0 || ch>num|| ans[0]=='\n'|| ans[0]=='\0')
        return FULLUPDATE;
    ch-=1;
    sprintf(genbuf,"boards/%s/%s",currboard,e_file[ch]);
    move(2,0);
    clrtobot();
    sprintf(buf,"(E)±à¼­ (D)É¾³ý %s? [E]: ",explain_file[ch]);
    getdata(3,0,buf,ans,2,DOECHO,NULL,YEA);
    if (ans[0] == 'D' || ans[0] == 'd') {
        {
            char        secu[STRLEN];
            sprintf(secu,"É¾³ýÏµÍ³µµ°¸£º%s",explain_file[ch]);
            securityreport(secu,NULL);
        }
        unlink(genbuf);
        move(5,0);
        prints("%s ÒÑÉ¾³ý\n",explain_file[ch]);
        sprintf(buf,"delete %s",explain_file[ch]);
        report(buf);
        pressreturn();
        return FULLUPDATE;
    }
    modify_user_mode( EDITSFILE);
    aborted = vedit(genbuf, NA);
    clear();
    if (aborted!=-1) {
        prints("%s ¸üÐÂ¹ý",explain_file[ch]);
        sprintf(buf,"edit %s",explain_file[ch]);
        report(buf);
        {
            char        secu[STRLEN];
            sprintf(secu,"ÐÞ¸ÄÏµÍ³µµ°¸£º%s",explain_file[ch]);
            securityreport(secu,NULL);
        }
    }
    pressreturn();
    return FULLUPDATE;
}
#endif /* LEEWARD_X_FILTER */
