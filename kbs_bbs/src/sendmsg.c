#include "bbs.h"
#ifdef lint
#include <sys/uio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "screen.h"
#define MAXMESSAGE 5

int  RMSG=NA;
extern int RUNSH;
extern struct screenline *big_picture;
extern char MsgDesUid[14]; /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong */
long f_offset=0;
static int RMSGCount = 0;    /* Leeward 98.07.30 supporting msgX */

int
get_msg(uid,msg,line)
char *msg,*uid;
int line;
{
    char genbuf[3];

    move(line,0);
    clrtoeol();
    prints("ËÍÒôÐÅ¸ø£º%s",uid);
    memset(msg,0,sizeof(msg));
    while(1)
    {
        getdata( line+1, 0, "ÒôÐÅ : ", msg, 59, DOECHO, NULL,NA);
        if(msg[0]=='\0')
            return NA;
        getdata( line+2, 0, "È·¶¨ÒªËÍ³öÂð(Y)ÊÇµÄ (N)²»Òª (E)ÔÙ±à¼­? [Y]: ",
                 genbuf, 2, DOECHO, NULL ,1);
        if(genbuf[0]=='e'||genbuf[0]=='E')
            continue;
        if(genbuf[0]=='n'||genbuf[0]=='N')
            return NA;
        if(genbuf[0]=='G')
        {
            if(HAS_PERM(currentuser,PERM_SYSOP))
                return 2;
            else
                return YEA;
        }
        else
            return YEA;
    }
}

int
canmsg(uin)
struct user_info *uin;
{
    if ((uin->pager&ALLMSG_PAGER) || HAS_PERM(currentuser,PERM_SYSOP)) return YEA;
    if ((uin->pager&FRIENDMSG_PAGER))
    {
        if(can_override(uin->userid,currentuser->userid))
            return YEA;
    }
    return NA;
}

s_msg()
{
    do_sendmsg(NULL,NULL,0);
}
extern char msgerr[255];
int
sendmsgfunc(struct user_info *uentp,char msgstr[256],int mode);

int
do_sendmsg(uentp,msgstr,mode)
struct user_info *uentp;
char msgstr[256];
int mode;
{
    char uident[STRLEN] ,ret_str[20];
    FILE *fp;
    time_t now;
    struct user_info *uin ;
    char buf[80],msgbuf[256] ,*timestr,msgbak[256];
    int msg_count=0;
    int Gmode = 0;
    int result;
    *msgbak = 0;	/* period 2000-11-20 may be used without init */
    if((mode==0)||(mode==3))
    {
        modify_user_mode( MSG );
        move(2,0) ; clrtobot();
    }
    if(uentp==NULL)
    {
        prints("<ÊäÈëÊ¹ÓÃÕß´úºÅ>\n") ;
        move(1,0) ;
        clrtoeol() ;
        prints("ËÍÑ¶Ï¢¸ø: ") ;
        creat_list() ;
        namecomplete(NULL,uident) ;
        if(uident[0] == '\0')
        {
            clear() ;
            return 0 ;
        }
        uin=t_search(uident,NA);
        if(uin==NULL)
        {
            move(2,0) ;
            prints("¶Ô·½Ä¿Ç°²»ÔÚÏßÉÏ£¬»òÊÇÊ¹ÓÃÕß´úºÅÊäÈë´íÎó...\n");
            pressreturn() ;
            move(2,0) ;
            clrtoeol() ;
            return -1 ;
        }
        if(!canmsg(uin))
        {
            move(2,0) ;
            prints("¶Ô·½ÒÑ¾­¹Ø±Õ½ÓÊÜÑ¶Ï¢µÄºô½ÐÆ÷...\n");
            pressreturn() ;
            move(2,0) ;
            clrtoeol() ;
            return -1;
        }
        /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong*/
        strcpy(MsgDesUid, uident);
       /* uentp = uin; */

    }else
    {
        /*  if(!strcasecmp(uentp->userid,currentuser->userid))	rem by Haohmaru,ÕâÑù²Å¿ÉÒÔ×Ô¼º¸ø×Ô¼º·¢msg
            return 0;    
         */   uin=uentp;
        strcpy(uident,uin->userid);
        /*   strcpy(MsgDesUid, uin->userid); change by KCN,is wrong*/
    }
    
    /* try to send the msg*/
    result = sendmsgfunc(uin,msgstr,mode);
 
    switch (result) {
    	 case 1: /* success */
            if (mode==0) {
           prints("\nÒÑËÍ³öÑ¶Ï¢....\n") ; pressreturn();
           clear() ;}
           return 1;     	 
    	 break;
    	 case -1: /* failed, reason in msgerr */
             if (mode==0 ) {
    	    move(2,0) ;
    	    prints(msgerr);
    	    pressreturn() ;
    	    move(2,0) ;
    	    clrtoeol() ; }
    	    return -1;
    	 break;
    	 case 0: /* message presending test ok, get the message and resend */
    	     if (mode == 4) return 0;
            Gmode=get_msg(uident,buf,1);
            if (!Gmode){
               move(1,0); clrtoeol();
               move(2,0); clrtoeol();
               return 0;
            }
    	 break;
    	 default: /* unknown reason */
   	   return result;
    	 break;
    }	   
    /* resend the message */
    result = sendmsgfunc(uin,buf,mode);
 
    switch (result) {
    	 case 1: /* success */
          if (mode==0) {
           prints("\nÒÑËÍ³öÑ¶Ï¢....\n") ; pressreturn();
           clear() ;}
           return 1;     	 
    	 break;
    	 case -1: /* failed, reason in msgerr */
           if (mode ==0){
    	    move(2,0) ;
    	    prints(msgerr);
    	    pressreturn() ;
    	    move(2,0) ;
    	    clrtoeol() ;}
    	    return -1;
    	 break;
    	 default: /* unknown reason */
   	   return result;
    	 break;
    }	   
    return 1 ;
}



int
show_allmsgs()
{
    char fname[STRLEN];
    int  oldmode;

    sethomefile(fname,currentuser->userid,"msgfile");
    clear();
    oldmode = uinfo.mode;
    modify_user_mode( LOOKMSGS);
    if(dashf(fname))
    {
        ansimore(fname,YEA);
        clear();
    }
    else
    {
        move(5,30);
        prints("Ã»ÓÐÈÎºÎµÄÑ¶Ï¢´æÔÚ£¡£¡");
        pressanykey();
        clear();
    }
    uinfo.mode = oldmode;
}


int
dowall(struct user_info *uin,char* buf2)
{
    if (!uin->active || !uin->pid) return -1;
    /*---	²»¸øµ±Ç°´°¿Ú·¢ÏûÏ¢ÁË	period	2000-11-13	---*/
    if(getpid() == uin->pid) return -1;

    move(1,0);
    clrtoeol();
    prints("[32mÕý¶Ô %s ¹ã²¥.... Ctrl-D Í£Ö¹¶Ô´ËÎ» User ¹ã²¥¡£[m",uin->userid);
    refresh();
    if (strcmp(uin->userid, "guest")) /* Leeward 98.06.19 */
    {
        /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong*/
        strcpy(MsgDesUid, uin->userid);

        do_sendmsg(uin,buf2,3); /* ¹ã²¥Ê±±ÜÃâ±»¹ý¶àµÄ guest ´ò¶Ï */
    }
    return 0;
}


int
wall()
{
	char buf2[STRLEN];
    modify_user_mode( MSG );
    move(2,0) ; clrtobot();
    if (!get_msg("ËùÓÐÊ¹ÓÃÕß",buf2,1) ){
        move(1,0); clrtoeol();
        move(2,0); clrtoeol();
        return 0;
    }
    if( apply_ulist((APPLY_UTMP_FUNC) dowall,buf2) == -1 ) {
        move(2,0);
        prints( "Ã»ÓÐÈÎºÎÊ¹ÓÃÕßÉÏÏß\n" );
        pressanykey();
    }
    prints("\nÒÑ¾­¹ã²¥Íê±Ï....\n");
    pressanykey();
    return 0;
}

void
r_msg()
{
    FILE *fp;
    char buf[256] ;
    char msg[256] ;
    char msgX[256];              /* Leeward 98.07.30 supporting msgX */
    int  msgXch = 0;             /* Leeward 98.07.30 supporting msgX */
    int  XOK = 0;                /* Leeward 98.07.30 supporting msgX */
    int  msg_count = 0;
    char fname[STRLEN], fname2[STRLEN] ;
    int line,tmpansi;
    int y,x,ch,i;
    char tmp[256];
    char savebuffer[256];
    int send_pid;
    char *ptr;
    int retcode;
    struct user_info *uin ;
    char msgbuf[STRLEN];
    int good_id;
    char usid[STRLEN];


    getyx(&y,&x);
    tmpansi=showansi;
    showansi=1;
    if(uinfo.mode==TALK)
        line=t_lines/2-1;
    else line=0;

    if((uinfo.mode == POSTING || uinfo.mode == SMAIL) && !DEFINE(currentuser,DEF_LOGININFORM))/*Haohmaru.99.12.16.·¢ÎÄÕÂÊ±²»»Ømsg*/
    {
        sethomefile(buf,currentuser->userid,"msgcount");
        fp = fopen(buf,"rb");
        if( fp != NULL )
        {
            fread(&msg_count,sizeof(int),1,fp);
            fclose(fp);
        }
        saveline(line, 0, savebuffer);
        move(line,0); clrtoeol(); refresh();
        if(msg_count){
            prints("[1m[33mÄãÓÐÐÂµÄÑ¶Ï¢£¬Çë·¢±íÍêÎÄÕÂºó°´ Ctrl+Z »ØÑ¶Ï¢[0m");
            refresh(); sleep(1);
        }
        else{
            prints("[1mÃ»ÓÐÈÎºÎÐÂµÄÑ¶Ï¢´æÔÚ![0m");
            refresh();sleep(1);
        }
        saveline(line, 1, savebuffer);
        return;
    }
    sethomefile(fname,currentuser->userid,"msgfile");
    if(!dashf(fname)) { /* Leeward 98.07.30 */
        saveline(line, 0, savebuffer);
        move(line,0); clrtoeol(); refresh();
        prints("[1mÃ»ÓÐÈÎºÎµÄÑ¶Ï¢´æÔÚ£¡£¡[0m");
        refresh(); sleep(1);
        saveline(line, 1, savebuffer); /* restore line */
        return;
    }
    sethomefile(fname2,currentuser->userid,"msgcount");
    RMSG=YEA;
    RMSGCount ++; /* Leeward 98.07.30 supporting msgX */
    saveline(line, 0, savebuffer);
    while(1)             /* modified by Luzi 1997.12.27 */
    {
        if((fp=fopen(fname,"r"))==NULL)
            break;
        i=0;
        if (f_offset==0)
        {
            while( fgets(buf,256,fp)!=NULL)
            {
                ptr=strrchr(buf,'[');
                send_pid=atoi(ptr+1);
                if(send_pid>100)
                    send_pid-=100;
                if (uinfo.pid==send_pid)
                { i=1;
                    strcpy(msg,buf);
                }
            }
        }
        else { fseek(fp, f_offset, SEEK_SET);
            while(fgets( msg, 256, fp)!=NULL)
            {
                ptr=strrchr(msg,'[');
                send_pid=atoi(ptr+1);
                if(send_pid>100)
                    send_pid-=100;
                if (uinfo.pid==send_pid)
                { i=1;break;}
            }
        }
        f_offset=ftell(fp);
        fclose(fp);
        if (i==0) break;
        if(DEFINE(currentuser,DEF_SOUNDMSG))
        {
            bell();
            bell();
        }
        move(line,0); clrtoeol(); prints("%s",msg); refresh();
        oflush() ;
        ch=0;
        while(/*ch!='\n'&&ch!='\r'*/ 1 )
        {

            /*        read(0,&ch,1);*/
END:
            ch=igetkey();
            if(!DEFINE(currentuser,DEF_IGNOREMSG))/*Haohmaru.98.12.23*/
            {
                if (ch== KEY_ESC) break;
                else if(ch==Ctrl('R')||ch=='r'||ch=='R')
                    goto MSGX;
                else
                    goto END;
            }   /* Added by Marco */


MSGX: /* Leeward 98.07.30 supporting msgX */
            strcpy(msgX, msg);


            strcpy(buf,msg);
            ptr=strrchr(buf,'[');
            *ptr='\0';
            ptr=strrchr(buf,'[');
            send_pid=atoi(ptr+1);
            if(send_pid>100)
                send_pid-=100;
            ptr=strtok(msg+10," ["); /* ºÍmsgÖÐ useridµÄÎ»ÖÃ¹ØÏµÃÜÇÐ*/
            if(ptr==NULL)/*|| !strcasecmp(ptr,currentuser->userid))*/
                good_id=NA;
            else
            {
                strcpy(usid,ptr);
                uin=t_search(usid,send_pid);
                if(uin==NULL)
                    good_id=NA;
                else
                    good_id=YEA;
            }
            if(good_id==YEA)
            {
                if (- KEY_UP != msgXch && - KEY_DOWN != msgXch) {
                    strncpy(tmp, big_picture[line + 1].data, 256/*LINELEN*/) ;
                    tmp[big_picture[line + 1].len]='\0';
                } /* Leeward 98.07.30 supporting msgX */
                move(line + 1,0); clrtoeol();
                sprintf(msgbuf,"»ØÑ¶Ï¢¸ø %s: ",usid);

                /* Leeward 98.07.30 supporting msgX */
                /*getdata(line + 1,0,msgbuf,buf,49,DOECHO,NULL,YEA);*/
MSGX2:
                switch (msgXch = getdata(line + 1,0,msgbuf,buf,59,DOECHO,NULL,YEA))
                {
                case - KEY_UP:
                case - KEY_DOWN:
                    {
                        char bufX[256], *ptrX;
                        FILE *fpX;
                        int  send_pidX, Line_1, Line;

                        XOK = Line_1 = Line = 0;
                        if (fpX = fopen(fname,"r")) {
                            while (fgets(bufX,256,fpX)) {
                                Line ++;
                                ptrX = strrchr(bufX,'[');
                                send_pidX = atoi(ptrX+1);
                                if(send_pidX > 100) send_pidX -= 100;
                                if (uinfo.pid == send_pidX) {
                                    if (XOK) { /* KEY_DOWN */
                                        Line_1 = Line;
                                        break;
                                    }
                                    if (!strncmp(msgX, bufX, strlen(msgX))) {
                                        XOK = 1;
                                        if (- KEY_UP == msgXch) break;
                                    }
                                    Line_1 = Line;
                                } /* End if (uinfo.pid == send_pidX) */
                            } /* End while (fgets(bufX,256,fpX)) */
                        } /* if (fpX = fopen(fname,"r")) */

                        if (XOK) {
                            rewind(fpX);
                            for(Line = 0; Line < Line_1; Line ++) fgets(bufX,256,fpX);
                            /* Leeward 98.09.24 enable scroll on both ends when Ctrl+Z */
                            if (!strncmp(msgX, bufX, strlen(msgX))) {
                                if (- KEY_DOWN == msgXch) {
                                    rewind(fpX); fgets(bufX,256,fpX);
                                } else if (- KEY_UP == msgXch)
                                    while (!feof(fpX)) fgets(bufX,256,fpX);
                            }
                            strcpy(msg, bufX);
                            move(line,0); clrtoeol(); prints("%s",msg); refresh();
                            oflush() ;
                            ch = 'R';
                            fclose(fpX);
                            goto MSGX;
                        } else { if (fpX) fclose(fpX);
                            break;
                        }

                    } /* End case */
                    break;

                default: break;
                } /* End switch */ /* Leeward 98.07.30 supporting msgX */

                if (- 1 == XOK)
                    sprintf(msgbuf,"[1m½áÊø»ØÕâÌõÑ¶Ï¢[m");
                else if (0 == XOK && (- KEY_UP == msgXch || - KEY_DOWN == msgXch))
                    sprintf(msgbuf, "[1mÏµÍ³´íÎó£ºsendmsg.c/r_msg/msgx/fopen|locate[m");
                else {
                    if(buf[0]!='\0')
                    {
                        /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong*/
                        strcpy(MsgDesUid, usid);
                        retcode = do_sendmsg(uin,buf,4);
                        if (retcode  == 1)
                            sprintf(msgbuf,"[1m°ïÄãËÍ³öÑ¶Ï¢ÁË[m");
                        else if (retcode == -2)
                            sprintf(msgbuf,"[1m¶Ô·½ÒÑ¾­ÀëÏßÁË...[m");
                    }else
                        sprintf(msgbuf,"[1m¿ÕÑ¶Ï¢, ËùÒÔ²»ËÍ³ö.[m");
                }

            }else
            { /* Leeward 98.07.30 add below 2 lines to fix bug */
                if (- KEY_UP != msgXch && - KEY_DOWN != msgXch) {
                    strncpy(tmp, big_picture[line + 1].data, 256/*LINELEN*/) ;
                    tmp[big_picture[line + 1].len]='\0';
                }
                sprintf(msgbuf,"[1mÕÒ²»³ö·¢Ñ¶Ï¢µÄÈË[m");
                /* Leeward 98.07.30 enable reply other messages */
                move(line,0); clrtoeol(); prints("%s",msgbuf);
                refresh(); oflush() ;
                sprintf(msgbuf, "[1mÇë°´¡ü»ò¡ýÇÐ»»Ñ¶Ï¢£¬»ò°´ Enter ½áÊø£º[m");
                move(line + 1,0); clrtoeol();
                XOK = - 1;
                goto MSGX2;
            }
            move(line,0);
            clrtoeol();
            refresh();
            prints("%s",msgbuf);
            refresh();
            sleep(1);

            move(line + 1,0);
            clrtoeol();
            prints("%s",tmp);

            break;
        }
        fp=fopen(fname2,"rb+");
        if (fp!=NULL)
        { int msg_count;
            fread(&msg_count,sizeof(int),1,fp);
            if (msg_count) msg_count--;
            fseek(fp,0,SEEK_SET);
            fwrite(&msg_count,sizeof(int),1,fp);
            fclose(fp);
        }
    }
    if (count_user()<2) unlink(fname2);
    showansi=tmpansi;
    saveline(line, 1, savebuffer); /* restore line */
    move(y,x);
    refresh();
    /* Leeward 98.07.30 supporting msgX */
    RMSGCount --;
    if (0 == RMSGCount) RMSG=NA;
    signal(SIGUSR2,r_msg);
    return ;
}

int
friend_login_wall(pageinfo)
struct user_info *pageinfo;
{
    char msg[STRLEN];
    struct userec* lookupuser;

    if( !pageinfo->active || !pageinfo->pid )
        return 0;
    if (can_override(pageinfo->userid,currentuser->userid)) {
        if(getuser(pageinfo->userid,&lookupuser)<=0)
            return 0;
        if(!(lookupuser->userdefine&DEF_LOGININFORM))
            return 0;
        if(!strcasecmp(pageinfo->userid,currentuser->userid))
            return 0;
        sprintf(msg,"ÄãµÄºÃÅóÓÑ %s ÒÑ¾­ÉÏÕ¾ÂÞ£¡",currentuser->userid);

        /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong*/
        strcpy(MsgDesUid, pageinfo->userid);
        do_sendmsg(pageinfo,msg,2);
    }
    return 0;
}

void r_lastmsg()
{
    f_offset=0;
    r_msg();
}
