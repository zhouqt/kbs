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
char buf2[STRLEN];
struct user_info *t_search();
extern struct screenline *big_picture;
char MsgDesUid[14]; /* 保存所发msg的目的uid 1998.7.5 by dong */

int
get_msg(uid,msg,line)
char *msg,*uid;
int line;
{
        char genbuf[3];

        move(line,0);
        clrtoeol();
        prints("送音信给：%s",uid);
        memset(msg,0,sizeof(msg));
        while(1)
        {
                getdata( line+1, 0, "音信 : ", msg, 59, DOECHO, NULL,NA);
                if(msg[0]=='\0')
                        return NA;
                getdata( line+2, 0, "确定要送出吗(Y)是的 (N)不要 (E)再编辑? [Y]: ",
                    genbuf, 2, DOECHO, NULL ,1);
                if(genbuf[0]=='e'||genbuf[0]=='E')
                        continue;
                if(genbuf[0]=='n'||genbuf[0]=='N')
                        return NA;
		if(genbuf[0]=='G')
		{
			if(HAS_PERM(PERM_SYSOP))
				return 2;
			else
				return YEA;
		}
                else
                        return YEA;
        }
}

char
msgchar(uin)
struct user_info *uin;
{
    if ((uin->pager&ALLMSG_PAGER)) return ' ';
    if ((can_override(uin->userid,currentuser.userid)))
    {
        if((uin->pager&FRIENDMSG_PAGER))
                return 'O';
        else
                return '#';
    }
    return '*';
}

int
canmsg(uin)
struct user_info *uin;
{
    if ((uin->pager&ALLMSG_PAGER) || HAS_PERM(PERM_SYSOP)) return YEA;
    if ((uin->pager&FRIENDMSG_PAGER))
    {
        if(can_override(uin->userid,currentuser.userid))
                return YEA;
    }
    return NA;
}

s_msg()
{
      do_sendmsg(NULL,NULL,0);
}

int
show_allmsgs()
{
        char fname[STRLEN];
	int  oldmode;

        setuserfile(fname,"msgfile");
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
                prints("没有任何的讯息存在！！");
                pressanykey();
                clear();
        }
	uinfo.mode = oldmode;
}

/* snow change at 10.24 */
/* 得到文件的一行 */
void getoneline(fp,line)
FILE *fp;
char line[200];
{
        int i;
        char ch;

        for (i=0;i<200;i++)
                line[i] = 0;
        i = 0;

        while (!feof(fp)) {
                fscanf(fp,"%c",&ch);
                if (ch=='\n') break;
                line[i++] = ch ;
        }
        line[i]=0;
}

/* snow change at 10.24 */
/* 聊天室看msg的函数 */
void chat_show_allmsgs()
{
        char fname[STRLEN];
        FILE *fp;
        char buf[200];

        setuserfile(fname,"msgfile");
        if(dashf(fname))
        {
                printchatline("你的消息有:");
                fp =fopen(fname,"r");
                while (!feof(fp)) {
                        getoneline(fp,buf);
                        printchatline(buf);
                }
                fclose(fp);       
        }
        else
        {
                printchatline("没有任何的讯息存在！！");
        }
}

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

	*msgbak = 0;	/* period 2000-11-20 may be used without init */
	if(mode==0)
	{
		modify_user_mode( MSG );
		move(2,0) ; clrtobot();
	}
	if(uentp==NULL)
	{
		prints("<输入使用者代号>\n") ;
		move(1,0) ;
		clrtoeol() ;
		prints("送讯息给: ") ;
		creat_list() ;
		namecomplete(NULL,uident) ;
		if(uident[0] == '\0') 
		{
			clear() ;
			return 0 ;
		}
		/*    if(searchuser(uident)==0 || tuid == usernum) 
		      {
		      if(uentp==NULL)
		      {
		      move(2,0) ;
		      prints("错误的使用者 ID\n") ;
		      pressreturn() ;
		      move(2,0) ;
		      clrtoeol() ;
		      }
		      return -1 ;
		      }*/
		uin=t_search(uident,NA);
		if(uin==NULL)
		{  
			move(2,0) ;
			prints("对方目前不在线上，或是使用者代号输入错误...\n");
			pressreturn() ;
			move(2,0) ;
			clrtoeol() ;
			return -1 ;
		} 
		if(!canmsg(uin))
		{
			move(2,0) ;
			prints("对方已经关闭接受讯息的呼叫器...\n");
			pressreturn() ;
			move(2,0) ;
			clrtoeol() ;
			return -1;
		}
		/* 保存所发msg的目的uid 1998.7.5 by dong*/
		strcpy(MsgDesUid, uident);

	}else
	{
		/*  if(!ci_strcmp(uentp->userid,currentuser.userid))	rem by Haohmaru,这样才可以自己给自己发msg
		    return 0;    
		 */   uin=uentp;
		strcpy(uident,uin->userid);
		/*   strcpy(MsgDesUid, uin->userid); change by KCN,is wrong*/
	}

	if(LOCKSCREEN == uin->mode) /* Leeward 98.02.28 */
	{
		move(2,0) ;
		prints("对方已经锁定屏幕，请稍候再发或给他(她)写信...\n");
		pressreturn() ;
		move(2,0) ;
		clrtoeol() ;
		return -1 ;
	}
	if((uin->mode==POSTING || uin->mode==SMAIL) && !lookupuser.userdefine&DEF_LOGININFORM)/*Haohmaru.2000.01.16*/
	{/*
	    move(2,0) ;
	    prints("对方正在发文章或写信，请稍候再发或给他(她)写信...\n");
	    pressreturn() ;
	    move(2,0) ;
	    clrtoeol() ;
	    return -1 ;*/
	}
	if (NA==canIsend2(uin->userid))/*Haohmaru.06.06.99.检查自己是否被ignore*/
	{
		move(2,0) ;
		prints("对方拒绝接受你的讯息...\n");
		pressreturn() ;
		move(2,0) ;
		clrtoeol() ;
		return -1;
	}


	sethomefile(buf,uident,"msgcount");
	fp=fopen(buf, "rb");
	if (fp!=NULL)
	{
		fread(&msg_count,sizeof(int),1,fp);
		fclose(fp);

		if(msg_count>MAXMESSAGE)
		{
			move(2,0) ;
			prints("对方尚有一些讯息未处理，请稍候再发或给他(她)写信...\n");
			pressreturn() ;
			move(2,0) ;
			clrtoeol() ;
			return -1 ;
		}
	}
	if(msgstr==NULL)
	{
		Gmode=get_msg(uident,buf,1);
		if (!Gmode){
			move(1,0); clrtoeol();
			move(2,0); clrtoeol(); 
			return 0;
		}
	}

	now=time(0);
	timestr=ctime(&now)+11;
	*(timestr+8)='\0';
	strcpy(ret_str,"R 回讯息");
	if(msgstr==NULL||mode==2)
	{
		sprintf(msgbuf,"[44m[36m%-12.12s[33m(%-5.5s):[37m%-59.59s[m[%dm\033[%dm\n", currentuser.userid, 
				timestr, (msgstr==NULL)?buf:msgstr,uinfo.pid+100,uin->pid+100);
		sprintf(msgbak,"[44m[0;1;32m=>[37m%-10.10s[33m(%-5.5s):[36m%-59.59s[m[%dm\033[%dm\n", uident,timestr, (msgstr==NULL)?buf:msgstr,uinfo.pid+100,uin->pid+100);
	}else
	{
		if(mode==0) {
			sprintf(msgbuf,"[44m[33m站长於 %8.8s 时广播："
				"[37m%-55.55s[m\033[%dm\n",
/*				"[37m%-59.59s[m\033[%dm\n",*/
					timestr,msgstr,uin->pid+100);
		}
		else if(mode==1)
		{
			sprintf(msgbuf,"[44m[36m%-12.12s(%-5.5s) 邀请你[37m%-43.43s(%s)[m[%dm\033[%dm\n", 
					currentuser.userid, timestr, msgstr,ret_str,uinfo.pid+100,uin->pid+100);
			sprintf(msgbak,"[44m[37m你(%-5.5s) 邀请%-12.12s[36m%-43.43s(%s)[m[%dm\033[%dm\n", timestr,uident,msgstr,ret_str,uinfo.pid+100,uin->pid+100);
		}else if(mode==3)
		{
			sprintf(msgbuf,"[44m[32mBBS 系统通告(%-5.5s):[37m%-59.59s[31m(%s)[m\033[%dm\n",
					timestr, (msgstr==NULL)?buf:msgstr,ret_str,uin->pid+100);
		}
	}
	if (Gmode == 2)
		sprintf(msgbuf,"[44m[33m站长於 %8.8s 时广播：[37m%-59.59s[m\033[%dm\n",timestr,buf,uin->pid+100);
/* 检查所发msg的目的uid是否已经改变  1998.7.5 by dong*/
	uin=t_search(MsgDesUid,uin->pid);
       
	if ((uin == NULL) || (uin->active == 0) || (uin->pid == 0) || (kill(uin->pid, 0) !=0))
        { /*
	    uin=t_search(MsgDesUid, NA);
            if ((uin == NULL) || (uin->active == 0) || (uin->pid == 0) || (kill(uin->pid, 0) !=0)){ */
            if (mode != 0)
               return -2;
            move(2,0) ;
            prints("对方已经离线....\n");
            pressreturn() ;
            move(2,0) ;
            clrtoeol() ;
 
            return -2; /* 对方已经离线 */
            /*} */
        }

        sethomefile(buf,uident,"msgfile");
        if((fp=fopen(buf,"a"))==NULL)
                return -1;
        fputs(msgbuf,fp);
        fclose(fp);

/*Haohmaru.99.6.03.回的msg也记录*/
	if(strcmp(currentuser.userid,uident)){
        	sethomefile(buf,currentuser.userid,"msgfile");
        	if((fp=fopen(buf,"a"))==NULL)
                	return -1;
        	fputs(msgbak,fp);
        	fclose(fp);
	}
        if(kill(uin->pid,SIGUSR2)==-1&&msgstr==NULL)
        {
            prints("\n对方已经离线.....\n") ; pressreturn();
            clear();
            return -1;
        }
        if(msgstr==NULL)
        { 
         prints("\n已送出讯息....\n") ; pressreturn();
         clear() ;
        }
        sethomefile(buf,uident,"msgcount");
        fp=fopen(buf, "wb");
        if (fp!=NULL)
          {
            msg_count++;
            fwrite(&msg_count,sizeof(int),1,fp);
            fclose(fp);
           }
    return 1 ;
}

int
dowall(uin)
struct user_info *uin;
{
        if (!uin->active || !uin->pid) return -1;
	/*---	不给当前窗口发消息了	period	2000-11-13	---*/
	if(getpid() == uin->pid) return -1;

        move(1,0);
        clrtoeol();
        prints("[32m正对 %s 广播.... Ctrl-D 停止对此位 User 广播。[m",uin->userid); 
        refresh();
        if (strcmp(uin->userid, "guest")) /* Leeward 98.06.19 */
        {
          /* 保存所发msg的目的uid 1998.7.5 by dong*/
          strcpy(MsgDesUid, uin->userid);

          do_sendmsg(uin,buf2,0); /* 广播时避免被过多的 guest 打断 */
        }
}


int
wall()
{
    modify_user_mode( MSG );
    move(2,0) ; clrtobot();
    if (!get_msg("所有使用者",buf2,1) ){
         move(1,0); clrtoeol();
         move(2,0); clrtoeol();
         return 0;
    }
    if( apply_ulist( dowall ) == -1 ) {
        move(2,0);
        prints( "没有任何使用者上线\n" );
        pressanykey();
    }
    prints("\n已经广播完毕....\n");
    pressanykey();
}

long f_offset=0;

void
r_msg()
{
    FILE *fp;
    char buf[256] ;
    char msg[256] ;
    char msgX[256];              /* Leeward 98.07.30 supporting msgX */
    int  msgXch = 0;             /* Leeward 98.07.30 supporting msgX */
    int  XOK = 0;                /* Leeward 98.07.30 supporting msgX */
    static int RMSGCount = 0;    /* Leeward 98.07.30 supporting msgX */
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

    if((uinfo.mode == POSTING || uinfo.mode == SMAIL) && !DEFINE(DEF_LOGININFORM))/*Haohmaru.99.12.16.发文章时不回msg*/
    { 
      setuserfile(buf,"msgcount");
      fp = fopen(buf,"rb");
      if( fp != NULL )
	{
		fread(&msg_count,sizeof(int),1,fp);
		fclose(fp);
	}
      saveline(line, 0, savebuffer);
      move(line,0); clrtoeol(); refresh();
	if(msg_count){
      prints("[1m[33m你有新的讯息，请发表完文章后按 Ctrl+Z 回讯息[0m");
      refresh(); sleep(1);
      }
	else{
	prints("[1m没有任何新的讯息存在![0m");
	refresh();sleep(1);
	}
      saveline(line, 1, savebuffer);
      return;
    }
    setuserfile(fname,"msgfile");
    if(!dashf(fname)) { /* Leeward 98.07.30 */
      saveline(line, 0, savebuffer);
      move(line,0); clrtoeol(); refresh();
      prints("[1m没有任何的讯息存在！！[0m");
      refresh(); sleep(1);
      saveline(line, 1, savebuffer); /* restore line */
      return;
    }
    setuserfile(fname2,"msgcount");
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
    if(DEFINE(DEF_SOUNDMSG))
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
       if(!DEFINE(DEF_IGNOREMSG))/*Haohmaru.98.12.23*/
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
          ptr=strtok(msg+10," ["); /* 和msg中 userid的位置关系密切*/
          if(ptr==NULL)/*|| !ci_strcmp(ptr,currentuser.userid))*/
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
                  sprintf(msgbuf,"回讯息给 %s: ",usid);

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
               sprintf(msgbuf,"[1m结束回这条讯息[m");
             else if (0 == XOK && (- KEY_UP == msgXch || - KEY_DOWN == msgXch))
               sprintf(msgbuf, "[1m系统错误：sendmsg.c/r_msg/msgx/fopen|locate[m");
             else {
                  if(buf[0]!='\0')
                  {
                        /* 保存所发msg的目的uid 1998.7.5 by dong*/
                        strcpy(MsgDesUid, usid);  
                        retcode = do_sendmsg(uin,buf,2);
                        if (retcode  == 1)
                           sprintf(msgbuf,"[1m帮你送出讯息了[m");
                        else if (retcode == -2)
                           sprintf(msgbuf,"[1m对方已经离线了...[m");
                  }else
                        sprintf(msgbuf,"[1m空讯息, 所以不送出.[m");
               }

          }else
          { /* Leeward 98.07.30 add below 2 lines to fix bug */
            if (- KEY_UP != msgXch && - KEY_DOWN != msgXch) {
                  strncpy(tmp, big_picture[line + 1].data, 256/*LINELEN*/) ;
                  tmp[big_picture[line + 1].len]='\0';
            }
                  sprintf(msgbuf,"[1m找不出发讯息的人[m");
                  /* Leeward 98.07.30 enable reply other messages */
                  move(line,0); clrtoeol(); prints("%s",msgbuf);
                  refresh(); oflush() ;
                  sprintf(msgbuf, "[1m请按↑或↓切换讯息，或按 Enter 结束：[m");
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

        if( !pageinfo->active || !pageinfo->pid )
                return 0;
        if (can_override(pageinfo->userid,currentuser.userid)) {
                if(getuser(pageinfo->userid)<=0)
                        return 0;
                if(!(lookupuser.userdefine&DEF_LOGININFORM))
                        return 0;
                if(!ci_strcmp(pageinfo->userid,currentuser.userid))
                        return 0;
                sprintf(msg,"你的好朋友 %s 已经上站罗！",currentuser.userid);

                /* 保存所发msg的目的uid 1998.7.5 by dong*/
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
