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

#ifdef lint
#include <sys/uio.h>
#endif

/********************snow*/
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAXMESSAGE 5
#define MAXLASTCMD 6
#include "chat.h"
char chatroom[IDLEN];           /* Chat-Room Name */
int chatline;                   /* Where to display message now */
int stop_line;                  /* next line of bottom of message window area */
FILE *rec;
int recflag=0;
char buftopic[STRLEN];

int cfd; /* ÁÄÌìÊÒµÄsocket */

extern char MsgDesUid[14]; /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong */
extern int ibufsize; /* KCN added for debug 1999.09.7 */

/* snow ¸ÄÓÚ 10.24 */
/* ¶¨ÒåÁÄÌìÊÒ¿´msgµÄº¯ÊıÍ· */
void chat_show_allmsgs();

extern char page_requestor[];
extern int talkrequest;
extern char *modestring();
extern char pagerchar();
extern struct UTMPFILE *utmpshm;
extern void t_pager();
/*
extern char *Cdate();
*/
void set_rec();
struct user_info *t_search();

#define b_lines t_lines-1
#define cuser currentuser
#define CHAT_LOGIN_OK       "OK"
#define CHAT_LOGIN_EXISTS   "EX"
#define CHAT_LOGIN_INVALID  "IN"
#define CHAT_LOGIN_BOGUS    "BG"
char *msg_seperator = "¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª";
char *msg_shortulist = "\033[33m\033[44m Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬  ©¦ Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬  ©¦ Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬ \033[m";
extern char BoardName[];

char *
Cdate(clock) /* Ê±¼ä --> Ó¢ÎÄ */
  time_t *clock;
{
/* Leeward 2000.01.01 Adjust year display for 20** */
  static char foo[24/*22*/];
  struct tm *mytm = localtime(clock);

  strftime(foo, 24/*22*/, "%Y-%m-%d %T %a"/*"%D %T %a"*/, mytm);
  return (foo);
}

void
printchatline(str) /*ÏÔÊ¾Ò»ĞĞ£¬²¢ÏÂÒÆÖ¸Ê¾·û*/
  char *str;
{
  char tmpstr[256];
  char *p;
  int i;
  int len; /* add by KCN for disable long line */
  int inesc;

/* snow add at 10.25 */
  p = str;
  i = 0;
  len=0;
  inesc=0;

  while ( *p != 0 ) {
	if (len>=79) {
	 tmpstr[strlen(tmpstr)-(len-79)]=0; 
	 break;
	}
        if (*p=='%') {
          if ( *(p+1) == 0 ) {
                tmpstr[i] = '%';
                i++;
                p++;
		len++;
          }
          else if ( *(p+1) == '%' ) {
                tmpstr[i] = '%';
                i++;
                p++;
                p++;
		len++;
          }
          else if ( *(p+1)>'0' && *(p+1)<='7' ) {
                tmpstr[i++] = '\033';
                tmpstr[i++] = '[';
                tmpstr[i++] = '3';
                tmpstr[i++] = *(p+1);
                tmpstr[i++] = 'm';
                p++;
                p++;
          }
          else if ( *(p+1) == '0' )
          {
                tmpstr[i++] = '\033';
                tmpstr[i++] = '[';
                tmpstr[i++] = '0';
                tmpstr[i++] = 'm';
                p++;
                p++;
          }
          else {
                tmpstr[i] = '%';
                i++;
                p++;
                tmpstr[i] = *p;
                i++;
                p++;
		len+=2;
          }
        }
        else {
		if (inesc) {
		   if (isalpha(*p))
			inesc=0;
		   /* ÀÁµÃ´¦ÀíÁ½¸öESCµÄÇé¿öÁË KCN*/
		} else {
		  if (*p=='\033')
			inesc=1;
                  else
			len++;
		}
		tmpstr[i] = *p;
                i++;
                p++;
        }
  }
  tmpstr[i++] = '\033';
  tmpstr[i++] = '[';
  tmpstr[i++] = '0';
  tmpstr[i++] = 'm';
  tmpstr[i] = 0;

  move(chatline, 0);
  clrtoeol();
  outs(tmpstr); /* snow change at 10.25 */
  outc('\n');
  if(recflag==1)
          fprintf(rec,"%s\n",str);
  if (++chatline == stop_line)
    chatline = 2;
  move(chatline, 0);
  clrtoeol();
  outs("==>");
}


void
chat_clear() /* clear chat */
{
  for (chatline = 2; chatline < stop_line; chatline++)
  {
    move(chatline, 0);
    clrtoeol();
  }
  chatline = stop_line - 1;
  printchatline("");
}


void
print_chatid(chatid)
  char *chatid;
{
  move(b_lines, 0);
  outs(chatid);
  outc(':');
}


chat_send(fd, buf)
  int fd;
  char *buf;
{
  int len;

  sprintf(genbuf, "%s\n", buf);
  len = strlen(genbuf);
  return (send(fd, genbuf, len, 0) == len);/* Í¨¹ısocket send */
}

int
chat_recv(fd, chatid)
  int fd;
  char *chatid;
{
  static char buf[512];
  static int bufstart = 0;
  int c, len, x, y;
  char filename[STRLEN];
  char *bptr, *xptr;

  len = sizeof(buf) - bufstart - 1;
  if ((c = recv(fd, buf + bufstart, len, 0)) <= 0)
    return -1;
  c += bufstart;
  bptr = buf;
  while (c > 0)
  {
    len = strlen(bptr) + 1;
    if (len > c && len < (sizeof buf / 2))
      break;

    if (*bptr == '/') /* ´¦Àíserver´«À´µÄÃüÁî */
    {
      switch (bptr[1])
      {
      case 'p':
/* add by KCN for list long emote */
        outs("[5;31m              ....°´ÈÎÒâ¼ü¼ÌĞø....[m");
  	add_io(0, 0);
        igetkey();
  	add_io(cfd, 0);
	break;
      case 'c':
        chat_clear();
        break;
      case 'n':
        strncpy(chatid, bptr + 2, 8);
        print_chatid(chatid);
        clrtoeol();
        break;
      case 'r':
        strncpy(chatroom, bptr + 2, IDLEN - 1);
        break;
      case 'z':
        strncpy(chatroom, bptr + 2, IDLEN - 1);
      case 't':
        move(0, 0);
        clrtoeol();
        sprintf(genbuf, "·¿¼ä£º [36m%s", chatroom);
        if (bptr[1]=='t') strcpy(buftopic,bptr+2);
        prints("[44m[33m %-21s  [33m»°Ìâ£º[36m%-51s[31m%2s[m", genbuf, buftopic ,(recflag==1)?"Â¼":"  ");
      }
    }
    else{
      printchatline(bptr);

      /* Leeward: 97.12.18: /n ÃüÁî³É¹¦ºó, Í¬²½¸üĞÂÁÄÌìÊÒÄÚÍâÁÄÌìÕßµÄ×´Ì¬ 
         ¼´ÔÚÁÄÌìÊÒÄÚÍâ¾ùÄÜÏÔÊ¾ÁÄÌìÕßµ±Ç°ÕıÔÚÊ¹ÓÃµÄÁÄÌì´úºÅ */
      if (xptr = strstr(bptr, "\033[m ½«ÁÄÌì´úºÅ¸ÄÎª \033[1m"))
      {
        if (strstr(bptr, "¡ò \033[1m"))
          if (strstr(bptr + 7, "\033[m ¡ò"))
            if (!strncmp(bptr + 7, chatid, x = strlen(chatid)))
              if (*(bptr+7+x)=='\033')
            { /* update uinfo */
              uinfo.in_chat = YEA;
              strncpy(chatid, xptr + (x = strlen("\033[m ½«ÁÄÌì´úºÅ¸ÄÎª \033[1m")), y = strstr(bptr + 7, "\033[m ¡ò") - (xptr + x));
              chatid[y] = 0;
              strcpy(uinfo.chatid, chatid);
              update_utmp();
            }
      }

      /* Leeward: 98.04.30: /k ºó£¬¸æÖª±»ÌßÕßÊÇË­ÌßµÄ */
      if (xptr = strstr(bptr, "*** Äã±»ÁÄÌìÊÒop(\033[1m\033[33m"))
      {
        if (xptr = strstr(bptr, "\033[m)Ìß³öÀ´ÁË ***"))
        {
               /* sprintf(buf,"kick %s out of chat room  by %s ", uin.userid, currentuser.userid);
               report(buf); add by dong, 1999.1.9 */

          /* ¼Æ»®ÔÚÕâÀï¸ø±»ÌßÕß·¢ msg */
        }
      }
    }

    c -= len;
    bptr += len;
  }

  if (c > 0)
  {
    strcpy(genbuf, bptr);
    strcpy(buf, genbuf);
    bufstart = len - 1;
  }
  else
    bufstart = 0;
  return 0;
}


/* KCN add chat 2 */
void ent_chat2()  /* ½øÈëÁÄÌìÊÒ*/
{
   ent_chat(2);
}

void ent_chat1()  /* ½øÈëÁÄÌìÊÒ*/
{
   ent_chat(1);
}

int
ent_chat(chatnum)  /* ½øÈëÁÄÌìÊÒ*/
int chatnum;
{
  char inbuf[128], chatid[20], lastcmd[MAXLASTCMD][128], *ptr;
  struct sockaddr_in sin;
  struct hostent *h;
  int cmdpos, ch;
  int currchar;
  int newmail;
  extern int dumb_term;
  int page_pending = NA;
  int chatting = YEA;
  FILE * fp;

  if (!strcmp(currentuser.userid,"guest"))
    return -1;
  modify_user_mode(CHAT1);
/*
  gethostname(inbuf, STRLEN);
  if (!(h = gethostbyname(inbuf)))
  {
    perror("gethostbyname");
    return -1;
  }
*/
  memset(&sin, 0, sizeof sin);
  sin.sin_family = PF_INET;
  /* sin.sin_family = h->h_addrtype; */
/*
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);
*/
  sin.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  if (chatnum==1)
    sin.sin_port = htons(CHATPORT3);
  else
    sin.sin_port = htons(CHATPORT2);

  cfd = socket(sin.sin_family, SOCK_STREAM, 0);

  if (connect(cfd, (struct sockaddr *) & sin, sizeof sin)) /*Èç¹ûÁ¬½Ó chatd Ê§°Ü£¬ÔòÆô¶¯chatd */
  { 
    close(cfd);

    switch (ch = fork())
    {
    case -1/*fork failure*/:
      fp = fopen("error.log", "wt");
      fprintf(fp, "fork error\n");
        fclose(fp);
      break;
    case 0/*fork success*/:
      fp = fopen("error.log", "wt");
      fprintf(fp, "fork success, sub-proc\n");
      fclose(fp);
      prints("¿ªÆôÁÄÌìÊÒ...");
      if (chatnum==1)
          system("bin/chatd");
      else
	  system("bin/chatd 2");
      exit(1);
    default:
      fp = fopen("error.log", "wt");
      fprintf(fp, "fork success, par-proc\n");
      fclose(fp);
      /* The chat daemon forks so we can wait on it here. */
      waitpid(ch, NULL, 0);
    }

    cfd = socket(sin.sin_family, SOCK_STREAM, 0);
    if ((connect(cfd, (struct sockaddr *) & sin, sizeof sin)))
    {
      perror("connect failed");
      return -1;
    }
  }

  /* Leeward 98.04.26 */
  move(3, 0);
  clrtoeol();
  move(4, 0); /* Leave line 3 for error message while entering chat room */
  prints("ÊäÈë×Ö·û [1m[37m*[m ÔÙ°´ [1m[37mEnter[m ¿ÉÒÔÈ¡Ïû½øÈëÁÄÌìÊÒ¡£                                    ");
  clrtoeol();
  move(5, 0);
  clrtoeol(); /* Clear line 5 for good looking */

  while (1)
  {
    getdata(2, 0, "ÇëÊäÈëÁÄÌì´úºÅ£º", inbuf, 9, DOECHO, NULL,YEA);
    sprintf(chatid, "%s", ((inbuf[0]!='\0'&&inbuf[0]!='\n'&&inbuf[0]!='/') ? inbuf : cuser.userid));
    chatid[8] = '\0';
    if ('*' == inbuf[0]) /* Leeward 98.04.26 */
    {
       close(cfd); 
       return 0;                        
    }

    sprintf(inbuf, "/! %d %d %s %s", uinfo.uid,
      cuser.userlevel, cuser.userid, chatid);
    chat_send(cfd, inbuf); /* send user info to chatd , and chatd will check it*/
    if (recv(cfd, inbuf, 3, 0) != 3)
    {
      return 0;
    }
    if (!strcmp(inbuf, CHAT_LOGIN_OK))
      break;
    else if (!strcmp(inbuf, CHAT_LOGIN_EXISTS))
      ptr = "Õâ¸ö´úºÅÒÑ¾­ÓĞÈËÓÃÁË";
    else if (!strcmp(inbuf, CHAT_LOGIN_INVALID))
      ptr = "Õâ¸ö´úºÅÊÇ´íÎóµÄ";
    else 
      ptr = "ÒÑ¾­ÓĞÒ»¸ö´°¿ÚÔÚÁÄÌìÊÒÀïÁË£¨Èô·ÇÈç´ËÇëÍË³öBBSÖØĞÂµÇÂ¼£»Èô»¹²»ĞĞÔÙÕÒÔÚÏßÕ¾³¤£©";
    move(3, 0);
    prints(ptr);
    clrtoeol();
    bell();
  }

  add_io(cfd, 0);

  newmail = cmdpos = currchar = 0;
  memset(lastcmd, 0, MAXLASTCMD * 80);

  /* update uinfo */
  uinfo.in_chat = YEA;
  strcpy(uinfo.chatid, chatid);
  update_utmp();

  /* initiate screen */
  clear();
  chatline = 2;
  strcpy(inbuf, chatid);
  stop_line = t_lines - 2;
  move(stop_line, 0);
  outs(msg_seperator);
  move(1, 0);
  outs(msg_seperator);
  print_chatid(chatid);
  memset(inbuf, 0, 80);

  /* chat begin */
  while (chatting) 
  {
    move(b_lines, currchar + 10);
    ch = igetkey();

    if (talkrequest)
      page_pending = YEA;
    if (page_pending)
      page_pending = servicepage(0, NULL);
   
    switch (ch)
    {
    case KEY_DOWN:
      cmdpos += MAXLASTCMD - 2;

    case KEY_UP:
      { int i;
      cmdpos++;
      for (i=0;i<MAXLASTCMD-1;i++,cmdpos++) {
        cmdpos %= MAXLASTCMD;
	if (lastcmd[cmdpos][0]) break;
      }
      strcpy(inbuf, lastcmd[cmdpos]);
      
      move(b_lines, 10);
      clrtoeol();
      ch=inbuf[69];
      inbuf[69]=0;
      outs(inbuf);
      inbuf[69]=ch;
      currchar = strlen(inbuf);
      continue;
      }

    case KEY_LEFT:
      if (currchar)
        --currchar;
      continue;

    case KEY_RIGHT:
      if (inbuf[currchar])
        ++currchar;
      continue;
    }

    if (!newmail && chkmail(0)) /* check mail */
    {
      newmail = 1;
      printchatline("[32m*** [31mµ±£¡ÄãÓĞĞÂĞÅÀ´À²...[m");
    }

    if (ch == I_OTHERDATA)      /* incoming */
    {
      if (chat_recv(cfd, chatid) == -1)
        break;
      continue;
    }

#ifdef BIT8
    if (isprint2(ch))
#else
    if (isprint(ch))
#endif

    {
      if (currchar < 126) /* Î´ÂúÒ»ĞĞ,print it*/
      {
        if (inbuf[currchar])
        {                       /* insert */
          int i;
          for (i = currchar; inbuf[i] && i < 127; i++);
          inbuf[i + 1] = '\0';
          for (; i > currchar; i--)
            inbuf[i] = inbuf[i - 1];
        }
        else
        {                       /* append */
          inbuf[currchar + 1] = '\0';
        }
        inbuf[currchar] = ch;
	ch = inbuf[69];    /* save the end of line */
	inbuf[69]=0;
	
        move(b_lines, currchar + 10);
        outs(&inbuf[currchar++]);
	inbuf[69]=ch;
      }
      continue;
    }

    if (ch == '\n' || ch == '\r')
    {
      if (currchar)
      {
                /* add to command history */
        for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
          strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
        strcpy(lastcmd[0], inbuf);

        chatting = chat_cmd(inbuf, cfd); /*localÃüÁî´¦Àí*/
                /* Èç¹ûÊÇÍË³öchatroomÖ¸Áî£¬·¢ÏûÏ¢¸øchatd */
        if (chatting == 0)
          chatting = chat_send(cfd, inbuf);

/*---	add by period	2000-10-29	fix '/ b' problem	---*/
	if(*inbuf == '/' && ' ' == *(inbuf+1)) {
	    char *ptr, *pnt;
	    ptr = inbuf + 1;
	    while(*ptr) {
		    if(' ' != *ptr) break;
		    pnt = ptr + 1;
		    do {
			    *(pnt-1) = *pnt;
		    } while(*pnt++);
	    }
	}
/*---		---*/
        chatting = 0;
        while (inbuf[chatting]!='\0' && inbuf[chatting]!=' ')
          chatting ++;
        if (chatting > 1)
          {       
             if (!ci_strncmp(inbuf, "/bye", chatting)) break;
             if (!ci_strncmp(inbuf, "/exit", chatting)) /*added by alex, 96.9.5*/
                break;                     /* modified by Luzi 1997.11.18 */
           } else chatting = 1; /* added by Luzi 97.11.18 11:40AM */

        inbuf[0] = '\0';
        currchar = cmdpos = 0;
        move(b_lines, 10);
        clrtoeol();
      }
      continue;
    }

    if (ch == Ctrl('H') || ch == '\177') /*Backspace */
    {
  /*    if (currchar > 79 ) ÂúÒ»ĞĞ,print it,added by Haohmaru*
      {  currchar=79;
        inbuf[currchar] = ch;
        move(b_lines, currchar + 10);
        outs(&inbuf[currchar++]);
      }*/
      if (currchar)
      {
        currchar--;
        inbuf[127] = '\0';
        memcpy(&inbuf[currchar], &inbuf[currchar + 1], 127 - currchar);
        move(b_lines, currchar + 10);
        clrtoeol();
	ch = inbuf[69];    /* save the end of line */
	inbuf[69]=0;

        outs(&inbuf[currchar]);
	inbuf[69]=ch;
      }
      continue;
    }
    if (ch == Ctrl('Z'))
    {
      r_lastmsg(); /* Leeward 98.07.30 support msgX */
      inbuf[0] = '\0';
      currchar = 0;
      move(b_lines, 10);
      clrtoeol();
      continue;
    }

    if (ch == Ctrl('C') /*|| ch == Ctrl('D')*/)/* ^C ÍË³ö*/
    {
      chat_send(cfd, "/b");
      if(recflag==1)
      {
        set_rec();
      }
      break;
    }
  }
  /* chat end */
  close(cfd);
  add_io(0, 0);
  uinfo.in_chat = NA;
  uinfo.chatid[0] = '\0';
  update_utmp();
  clear();
  refresh();
  return 0;
}


int
printuserent(uentp) /* print one user & status */
  struct user_info *uentp;
{
  static char uline[256];
  static int cnt;
  char pline[50];

  if (!uentp)
  {
    if (cnt)
      printchatline(uline);
    bzero(uline, 256);
    cnt = 0;
    return 0;
  }
  if (!uentp->active || !uentp->pid)
    return 0;
  if (!HAS_PERM(PERM_SEECLOAK) && uentp->invisible)
    return 0;

#if 0
  if (kill(uentp->pid, 0) == -1)
    return 0;
#endif
  
  sprintf(pline, " %s%-13s[m%c%-10s",myfriend(uentp->userid)?"[32m":"", uentp->userid, uentp->invisible ? '#' : ' ',
    modestring(uentp->mode, uentp->destuid, 0, NULL));
  if (cnt < 2)
    strcat(pline, "©¦");
  strcat(uline, pline);
  if (++cnt == 3)
  {
    printchatline(uline);
    memset(uline, 0, 256);
    cnt = 0;
  }
  return 0;
}

int
print_friend_ent(uentp) /* print one user & status if he is a friend*/
  struct user_info *uentp;
{
  static char uline[256];
  static int cnt;
  char pline[50];

  if (!uentp)
  {
    if (cnt)
      printchatline(uline);
    bzero(uline, 256);
    cnt = 0;
    return 0;
  }
  if (!uentp->active || !uentp->pid)
    return 0;
  if (!HAS_PERM(PERM_SEECLOAK) && uentp->invisible)
    return 0;

#if 0
  if (kill(uentp->pid, 0) == -1)
    return 0;
#endif
  
  if (!myfriend(uentp->userid)) 
    return 0;
  sprintf(pline, " %-13s%c%-10s",uentp->userid, uentp->invisible ? '#' : ' ',
      modestring(uentp->mode, uentp->destuid, 0, NULL));
  if (cnt < 2)
    strcat(pline, "©¦");
  strcat(uline, pline);
  if (++cnt == 3)
  {
    printchatline(uline);
    memset(uline, 0, 256);
    cnt = 0;
  }
  return 0;
}

void
chat_help(arg)
char *arg;
{
    char buf[256];
    FILE *fp;

  if (strstr(arg, " op")) /* op's help*/
  {
    if((fp=fopen("help/chatophelp","r"))==NULL)
        return;
    while(fgets(buf, 255, fp) != NULL)
    {
        printchatline(buf);
    }
    fclose(fp);
  }
  else
  {
    if((fp=fopen("help/chathelp","r"))==NULL) /* user's help */
        return;
    while(fgets(buf, 255, fp) != NULL)
    {
        char *ptr;

        ptr=strstr(buf,"\n");
        *ptr='\0';
        printchatline(buf);
    }
    fclose(fp);
  }
}

void
call_user(arg) /* invite user online to chatroom*/
char *arg;
{
  char *userid,msg[STRLEN*2];
  struct user_info *uin ;
  int state;

  userid=strrchr(arg,' ');
  if(userid==NULL)
  {
        printchatline("[37m*** [32mÇëÊäÈëÄãÒªÑûÇëµÄ ID[37m ***[m");
        return;
  }else
        userid+=1;
  if(!strcasecmp(userid,currentuser.userid)) 
        sprintf(msg,"[32mÄã²»ÓÃÑûÇë×Ô¼º°¡[m");
  else if (!HAS_PERM(PERM_PAGE))  /* Leeward 98.07.30 */
    sprintf(msg,"[32mÄãÃ»ÓĞ·¢ĞÅÏ¢µÄÈ¨ÏŞ[m");
  else
  {
        uin=t_search(userid,NA); 
        if(uin==NULL)
            sprintf(msg,"[32m%s[37m ²¢Ã»ÓĞÉÏÕ¾[m",userid);
        else if(LOCKSCREEN == uin->mode) /* Leeward 98.02.28 */
          sprintf(msg,"[32m%s[37mÒÑ¾­Ëø¶¨ÆÁÄ»£¬ÇëÉÔºòÔÙÑûÇë»ò¸øËû(Ëı)Ğ´ĞÅ.\n", uin->userid);
        else
          if(!canmsg(uin))
                sprintf(msg,"%s ÒÑ¾­¹Ø±Õ½ÓÊÜÑ¶Ï¢µÄºô½ĞÆ÷.\n", uin->userid);
         else {  FILE *fp;
                 int msg_count=0;
                 char buf[STRLEN];
                 sethomefile(buf,uin->userid,"msgcount");
                 fp=fopen(buf, "rb");
  if (fp!=NULL)
   {
    fread(&msg_count,sizeof(int),1,fp);
    fclose(fp);
   }
    if(msg_count>MAXMESSAGE)
        sprintf(msg,"¶Ô·½ÉĞÓĞÒ»Ğ©Ñ¶Ï¢Î´´¦Àí£¬ÇëÉÔºòÔÙÑûÇë»ò¸øËû(Ëı)Ğ´ĞÅ...\n");

          else {
                  sprintf(msg,"µ½ÁÄÌì¹ã³¡µÄ %s ÁÄÌìÊÒ ÁÄÁÄÌì",chatroom);
                  /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong*/
                  strcpy(MsgDesUid, uin->userid);
                  state = do_sendmsg(uin,msg,1);
                  if (state == 1)
                     sprintf(msg,"[37mÒÑ¾­°ïÄãÑûÇë %s ÁË[m",uin->userid);
                  else if(state == -2)
                     sprintf(msg,"[37m¶Ô·½ÒÑ¾­ÀíÏßÁË...[m");
              }
   }
  }
  printchatline(msg);
}

int
Isspace(ch)
  char ch;
{
  return (int) strchr(" \t\n\r", ch);
}


char *
nextword(str)
  char **str;
{
  char *p;
  while (Isspace(**str))
    (*str)++;
  p = *str;
  while (**str && !Isspace(**str))
    (*str)++;
  if (**str)
  {
    **str = '\0';
    (*str)++;
  }
  return p;
}

void
chat_sendmsg(arg) /* send msg in chatroom , by alex, 96.9.5*/
char *arg;
{
  char *userid,msg[STRLEN*2], *msg1;
  struct user_info *uin ;
  int state;

  userid = nextword(&arg);
  if (strchr("\n\r",*arg)) userid=NULL;
  else userid = nextword(&arg);            

 /*  userid=strrchr(arg,' '); */
  if(userid==NULL)
  {
        printchatline("[37m*** [32mÇëÊäÈëÄãÒª·¢ÏûÏ¢µÄ ID[37m ***[m");
        return;
  }else
/*  if(!strcasecmp(userid,currentuser.userid)) 
        sprintf(msg,"[32mÄã²»ÓÃ¶Ô×Ô¼º·¢message°¡[m");
  else*/ if (!HAS_PERM(PERM_PAGE))  /* Leeward 98.07.30 */
    sprintf(msg,"[32mÄãÃ»ÓĞ·¢ĞÅÏ¢µÄÈ¨ÏŞ[m");
  else
  {
        uin=t_search(userid,NA); 
        if(uin==NULL)
        {
              sprintf(msg,"[32m%s[37m ²¢Ã»ÓĞÉÏÕ¾[m",userid);
        }
        else if(LOCKSCREEN == uin->mode) /* Leeward 98.02.28 */
          sprintf(msg,"[32m%s[37mÒÑ¾­Ëø¶¨ÆÁÄ»£¬ÇëÉÔºòÔÙ·¢»ò¸øËû(Ëı)Ğ´ĞÅ.\n", uin->userid);
        else
        {
        if(!canmsg(uin))
         sprintf(msg,"[32m%s[37mÒÑ¾­¹Ø±Õ½ÓÊÜÑ¶Ï¢µÄºô½ĞÆ÷.\n", uin->userid);
        else if(NA==canIsend2(uin->userid))
	 sprintf(msg,"[32m%s[37m¾Ü¾ø½ÓÊÜÄãµÄÑ¶Ï¢.\n",uin->userid);/*Haohmaru.99.6.6,¼ì²éÊÇ·ñ±»ignore*/
 else {  FILE *fp;
         int msg_count=0;
         char buf[STRLEN];
         sethomefile(buf,uin->userid,"msgcount");
         fp=fopen(buf, "rb");
  if (fp!=NULL)
   {
    fread(&msg_count,sizeof(int),1,fp);
    fclose(fp);
   }
    if(msg_count>MAXMESSAGE)
        sprintf(msg,"¶Ô·½ÉĞÓĞÒ»Ğ©Ñ¶Ï¢Î´´¦Àí£¬ÇëÉÔºòÔÙ·¢»ò¸øËû(Ëı)Ğ´ĞÅ...\n");
    else
       {
              /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong*/
              strcpy(MsgDesUid, uin->userid);
              state = do_sendmsg(uin,arg,2);

              if (state == 1)
                 sprintf(msg,"[37mÒÑ¾­·¢ÏûÏ¢¸ø [32m%s[37m ÁË[m",uin->userid);
              else if (state == -2)
                 sprintf(msg,"[37m¶Ô·½ÒÑ¾­ÀëÏßÁË...[m");
        }
     }
  }
}
  printchatline(msg);
}

int
c_cmpuids(uid,up)
int uid ;
struct user_info *up ;
{
    return (uid == up->uid) ;
}

void
chat_status( uentp, fptr, farg ,fd) /* modified by Luzi 1997.11.18 */ 
struct user_info *uentp;
int (*fptr)();
int farg,fd;
{
    int         i,num;
        char            tmpstr[31];
    char *lpTmp;
    resolve_utmp();
    num=0;
    for( i = 0; i < USHM_SIZE; i++ ) {
        *uentp = utmpshm->uinfo[ i ];
      if( (*fptr)( farg, uentp ) )
      {
        if( !uentp->active || !uentp->pid )
           continue;
        if(uentp->invisible==1)
        { 
          if(HAS_PERM(PERM_SEECLOAK))
          {
                if(++num==1)
                   sprintf(genbuf,"µ±Ç°×´Ì¬£º");
                sprintf(genbuf+strlen(genbuf),"[32m#[m");     
          }
          else continue;
        }
        lpTmp=(char*)idle_str(uentp);
        if(++num==1)
          sprintf(genbuf,"µ±Ç°×´Ì¬£º");
  /*      sprintf(tmpstr,"[1m%-8s[m ", modestring(uentp->mode, 
           uentp->destuid, 1,(uentp->in_chat ? uentp->chatid : NULL)));
                strcat(genbuf, tmpstr); */
        if (uentp->in_chat) /* add by Luzi 1997.11.18*/
           {
              sprintf(tmpstr,"/q %s\n",uentp->userid);
              send(fd, tmpstr, strlen(tmpstr), 0); 
              tmpstr[recv(fd, tmpstr, 30, 0)]='\0';
              if (tmpstr[0]=='1')
                {  sprintf(genbuf+strlen(genbuf),"'%s' room as '%s'",
                   tmpstr+1,uentp->chatid);
                   if (lpTmp[0]!=' ') 
                      sprintf(genbuf+strlen(genbuf),"[%s];",lpTmp);
                   else strcat(genbuf," ;");
                   continue;
                 }
             }
          sprintf(genbuf,"%s%-8s",genbuf,modestring(uentp->mode, 
           uentp->destuid, 0,/* 1->0 ²»ÏÔÊ¾ÁÄÌì¶ÔÏóµÈ modified by dong 1996.10.26 */
           (uentp->in_chat ? uentp->chatid : NULL)));
        if (lpTmp[0]!=' ') 
           sprintf(genbuf+strlen(genbuf),"[%s];",lpTmp);
        else strcat(genbuf," ;");
      }     
    }
        printchatline(genbuf);
    return;
}

void
call_query(arg,fd) /* by alex, 1996.9.5*/ /* by Luzi, 1997.11.30 */
char *arg;
int fd;           
{
    char        uident[STRLEN], inbuf[STRLEN*2], *newline, *userid;
    int         tuid=0, i, bFlag=0;
    struct user_info uin,uout;
    char qry_mail_dir[STRLEN];
    time_t exit_time,temp;

    userid = nextword(&arg); 
    if (strchr("\n\r",*arg))
    {
      printchatline("*** ÇëÊäÈëÒª²éÑ¯µÄID ***");
      return;
    }
    else 
    {
      userid = nextword(&arg);  
    }
    strcpy(uident,strtok(userid," "));
    if(!(tuid = getuser(uident))) {
	    sprintf(genbuf,"[32mÕâ¸öID²»´æÔÚ£¡[m");
	    printchatline(genbuf);
	    return;
    }
    uinfo.destuid = tuid ;
    uin.destuid = tuid ;
    update_utmp();
   
    search_ulist( &uin, c_cmpuids, tuid );

    setmailfile(qry_mail_dir, lookupuser.userid, DOT_DIR);
 
/*---	modified by period	2000-11-02	hide posts/logins	---*/
#ifdef _DETAIL_UINFO_
    sprintf(genbuf, "%s (%s):      %s", lookupuser.userid, lookupuser.username,
                   (check_query_mail(qry_mail_dir)==1)? "ÓĞĞÂĞÅ":"    ");
    printchatline(genbuf);
      sprintf(genbuf,"¹²ÉÏÕ¾ %d ´Î£¬·¢±í¹ı %d ÆªÎÄÕÂ£¬ÉúÃüÁ¦[%d]%s",
              lookupuser.numlogins, lookupuser.numposts, 
              compute_user_value(&lookupuser),
              (lookupuser.userlevel & PERM_SUICIDE)?" (×ÔÉ±ÖĞ)":" ");
          printchatline(genbuf);
#else
    sprintf(genbuf, "%s (%s):   ÉúÃüÁ¦[%d]%s   %s",
		    lookupuser.userid, lookupuser.username,
		    compute_user_value(&lookupuser),
		    (lookupuser.userlevel & PERM_SUICIDE)?" (×ÔÉ±ÖĞ)":" ",
                   (check_query_mail(qry_mail_dir)==1)? "ÓĞĞÂĞÅ":"    ");
    printchatline(genbuf);
#endif /*_DETAIL_UINFO_*/
    strcpy(inbuf, ctime(&(lookupuser.lastlogin)));
    if( (newline = strchr(genbuf, '\n')) != NULL )
        *newline = '\0';

        /*    modified by Luzi 1997.11.18      */
       for( i = 0; i < USHM_SIZE; i++ ) {
          uout = utmpshm->uinfo[ i ];
          if( c_cmpuids( tuid, &uout ) )
           {
             if( !uout.active || !uout.pid )
                continue;
             if(uout.invisible==0 || HAS_PERM(PERM_SEECLOAK))
               {
                 bFlag=1;break;}
           }
         }
     if (bFlag==0)
     {
        sprintf(genbuf, "ÉÏ´ÎÉÏÏßÀ´×Ô  %s Ê±¼äÎª %s "/*\n"*/ ,  
          (lookupuser.lasthost[0] == '\0'/* || DEFINE(DEF_HIDEIP)*/ ? "(²»Ïê)" : lookupuser.lasthost), inbuf);/* Haohmaru.99.12.18*/
        printchatline(genbuf);

/* »ñµÃÀëÏßÊ±¼ä Luzi 1998/10/23 */
        exit_time = get_exit_time(lookupuser.userid,genbuf);
        if( (newline = strchr(genbuf, '\n')) != NULL )
            *newline = '\0';
        if (exit_time > lookupuser.lastlogin) strcpy(inbuf,genbuf);
/*Haohmaru.98.12.04.ºÍ²Ëµ¥²éÑ¯½á¹ûÒ»ÖÂ*/
    if (exit_time <= lookupuser.lastlogin
     || (uin.active && uin.pid
         && (!uin.invisible || (uin.invisible && HAS_PERM(PERM_SEECLOAK)))))
      strcpy(inbuf,"ÒòÔÚÏßÉÏ»ò·Ç³£¶ÏÏß²»Ïê");
    if (exit_time <= lookupuser.lastlogin && (uin.invisible&& !HAS_PERM(PERM_SEECLOAK)))
        {
        temp=lookupuser.lastlogin+(lookupuser.numlogins%7)+5;
        strcpy(inbuf,ctime(&temp));/*Haohmaru.98.12.04.ÈÃÒşÉíÓÃ»§¿´ÉÏÈ¥ÀëÏßÊ±¼ä±ÈÉÏÏßÊ±¼äÍí5ÃëÖÓ*/
        if( (newline = strchr(inbuf, '\n')) != NULL )
        *newline = '\0';
        }
 /*       else strcpy(inbuf,"[Òò·Ç³£¶ÏÏß²»Ïê]");*/
        sprintf(genbuf, "ÀëÏßÊ±¼äÎª %s "/*\n"*/ , inbuf);
        printchatline(genbuf);
        }
    else 
        {
      sprintf(genbuf, "Ä¿Ç°ÕıÔÚÏßÉÏ: À´×Ô %s ÉÏÏßÊ±¼ä %s"/*\n"*/,
          (lookupuser.lasthost[0] == '\0'/* || DEFINE(DEF_HIDEIP)*/ ? "(²»Ïê)" : lookupuser.lasthost), inbuf);/*Haohmaru.99.12.18*/
          printchatline(genbuf);
      chat_status( &uin, c_cmpuids, tuid ,fd);  
        }

}               

void
call_query_ByChatid(arg,fd) /* add by dong, 1998.9.12*/
char *arg;
int fd;
{
    char        uident[STRLEN], inbuf[STRLEN*2], *newline, *chatid;
    int         tuid=0, i, bFlag=0;
    struct user_info uin,uout;
    char qry_mail_dir[STRLEN];
    char            tmpstr[40];
    time_t exit_time;

    chatid = nextword(&arg);

    if (strchr("\n\r",*arg))
    {
      printchatline("*** ÇëÊäÈëÒª²éÑ¯µÄchat ID ***");
      return;
    }
    else
    {
      chatid = nextword(&arg);
    }
    strcpy(uident,strtok(chatid," "));
    if (strlen(uident) > 32)
       uident[32] = '\0';

    /* get user id from the chat id */
    sprintf(tmpstr,"/qc %s\n", uident);
    send(fd, tmpstr, strlen(tmpstr), 0);
    tmpstr[recv(fd, tmpstr, 40, 0)]='\0';
    if (tmpstr[0]=='1')
    {  
          sprintf(uident, "%s", tmpstr+1);
    }
    else{
          sprintf(genbuf,"[32mÕâ¸öchat ID²»´æÔÚ£¡[m");
          printchatline(genbuf);
          return;
    }

    if(!(tuid = getuser(uident))) {
          sprintf(genbuf,"[32mÕâ¸öchat ID²»´æÔÚ£¡[m");
          printchatline(genbuf);
          return;
    }
    uinfo.destuid = tuid ;
    uin.destuid = tuid ;
    update_utmp();

    search_ulist( &uin, c_cmpuids, tuid );

    setmailfile(qry_mail_dir, lookupuser.userid, DOT_DIR);

/*---	modified by period	2000-11-02	hide posts/logins	---*/
#ifdef _DETAIL_UINFO_
    sprintf(genbuf, "%s (%s):      %s", lookupuser.userid, lookupuser.username,
                   (check_query_mail(qry_mail_dir)==1)? "ÓĞĞÂĞÅ":"    ");
    printchatline(genbuf);
    sprintf(genbuf,"¹²ÉÏÕ¾ %d ´Î£¬·¢±í¹ı %d ÆªÎÄÕÂ£¬ÉúÃüÁ¦[%d]%s",
              lookupuser.numlogins, lookupuser.numposts,
              compute_user_value(&lookupuser),
              (lookupuser.userlevel & PERM_SUICIDE)?" (×ÔÉ±ÖĞ)":" ");
    printchatline(genbuf);
#else
    sprintf(genbuf, "%s (%s):   ÉúÃüÁ¦[%d]%s   %s",
		    lookupuser.userid, lookupuser.username,
		    compute_user_value(&lookupuser),
		    (lookupuser.userlevel & PERM_SUICIDE)?" (×ÔÉ±ÖĞ)":" ",
                   (check_query_mail(qry_mail_dir)==1)? "ÓĞĞÂĞÅ":"    ");
    printchatline(genbuf);
#endif
    strcpy(inbuf, ctime(&(lookupuser.lastlogin)));
    if( (newline = strchr(genbuf, '\n')) != NULL )
        *newline = '\0';

        /*    modified by Luzi 1997.11.18      */
       for( i = 0; i < USHM_SIZE; i++ ) {
          uout = utmpshm->uinfo[ i ];
          if( c_cmpuids( tuid, &uout ) )
           {
             if( !uout.active || !uout.pid )
                continue;
             if(uout.invisible==0 || HAS_PERM(PERM_SEECLOAK))
               {
                 bFlag=1;break;}
           }
         }
     /*------*/
     if (bFlag==0)
     {
        sprintf(genbuf, "ÉÏ´ÎÉÏÏßÀ´×Ô  %s Ê±¼äÎª %s\n",
          (lookupuser.lasthost[0] == '\0' /*|| DEFINE(DEF_HIDEIP)*/ ? "(²»Ïê)" : lookupuser.lasthost), inbuf);/*Haohmaru.99.12.18*/
          printchatline(genbuf);
/* »ñµÃÀëÏßÊ±¼ä Luzi 1998/10/23 */
        exit_time = get_exit_time(lookupuser.userid,genbuf);
        if( (newline = strchr(genbuf, '\n')) != NULL )
            *newline = '\0';
        if (exit_time > lookupuser.lastlogin) strcpy(inbuf,genbuf);
        else strcpy(inbuf,"[Òò·Ç³£¶ÏÏß²»Ïê]");
        sprintf(genbuf, "ÀëÏßÊ±¼äÎª %s \n",inbuf);
        printchatline(genbuf);

        }
    else
        {
      sprintf(genbuf, "Ä¿Ç°ÕıÔÚÏßÉÏ: À´×Ô %s ÉÏÏßÊ±¼ä %s\n",
          (lookupuser.lasthost[0] == '\0' ? "(²»Ïê)" : lookupuser.lasthost), inbuf);
          printchatline(genbuf);
      chat_status( &uin, c_cmpuids, tuid ,fd);
        }

}

void
chat_date()
{
  time_t thetime;

  time(&thetime);
  sprintf(genbuf, " %s±ê×¼Ê±¼ä: [32m%s[m", BoardName, Cdate(&thetime));
  printchatline(genbuf);
}

/*remed by Haohmaru.99.12.02.Õâ¸ö¹¦ÄÜÒÑÎŞÒâÒå
void
chat_users()
{
  printchatline("");
  sprintf(genbuf,"[1m¡¾ %s µ±Ç°ÏßÉÏÓÃ»§ÁĞ±í ¡¿[m", BoardName);
  printchatline(genbuf);
  printchatline(msg_shortulist);

  if (apply_ulist(printuserent) == -1)
  {
    printchatline("[1m¿ÕÎŞÒ»ÈË[m");
  }
  printuserent(NULL);
}
*/
void
chat_friends()
{
  printchatline("");
  sprintf(genbuf,"[1m¡¾ µ±Ç°ÏßÉÏµÄºÃÓÑÁĞ±í ¡¿[m");
  printchatline(genbuf);
  printchatline(msg_shortulist);

  if (apply_ulist(print_friend_ent) == -1)
  {
    printchatline("[1mÃ»ÓĞÅóÓÑÔÚÏßÉÏ[m");
  }
  print_friend_ent(NULL);
}

void
set_rec() /* set recorder */
{
        char fname[STRLEN];
        time_t now;
        
        now=time(0);
/*        if(!HAS_PERM(PERM_SYSOP))
                return;*/

        sprintf(fname,"etc/%s.chat",currentuser.userid);
        if(recflag==0)
        {
                if((rec=fopen(fname,"w"))==NULL)
                        return;

                printchatline("[5m[32mRecord Start ...[m");
                recflag=1;
                move(0, 0);
                clrtoeol();
                sprintf(genbuf, "·¿¼ä£º [36m%s", chatroom);
                prints("[44m[33m %-21s  [33m»°Ìâ£º[36m%-51s[31m%2s[m", genbuf, buftopic ,(recflag==1)?"Â¼":"  ");

                fprintf(rec,"±¾¶ÎÓÉ %s",currentuser.userid);
                fprintf(rec,"ËùÂ¼ÏÂ£¬Ê±¼ä£º %s",ctime(&now));
        }else
        {
                recflag=0;
                move(0, 0);
                clrtoeol();
                sprintf(genbuf, "·¿¼ä£º [36m%s", chatroom);
                prints("[44m[33m %-21s  [33m»°Ìâ£º[36m%-51s[31m%2s[m", genbuf, buftopic ,(recflag==1)?"Â¼":"  ");

                printchatline("[5m[32mRecord Stop ...[m");
                fprintf(rec,"½áÊøÊ±¼ä£º%s\n",ctime(&now));
                fclose(rec);
                mail_file(fname,currentuser.userid,"Â¼Òô½á¹û");
/*                postfile(fname,"syssecurity","Â¼Òô½á¹û",2); 
 Luzi 1997.10.29     postfile(fname,"sysop","Â¼Òô½á¹û",2); */
/* post it on sysop board */
                unlink(fname);
        }
}


void
setpager()
{
        char buf[STRLEN];

        t_pager();/*toggle pager*/
        sprintf(buf,"[32m*** [31mÄã %s ÁËºô½ĞÆ÷[m",(uinfo.pager&ALL_PAGER)?"´ò¿ª":"¹Ø±Õ");
        printchatline(buf);

}
void
call_kickoff(arg) /* kick ID off BBS, by Luzi, 97.11.1*/
char *arg;
{
  char *userid,msg[STRLEN*2], *msg1;
  struct user_info *uin ;
  int state;
  char buf[80]; 

  if(!HAS_PERM(PERM_SYSOP))
  {
     printchatline("*** Äã²»ÊÇÕ¾³¤ ***");
     return;
  }

  userid = nextword(&arg);
  if (strchr("\n\r",*arg)) userid=NULL;
  else userid = nextword(&arg);   

  if(userid==NULL)
  {
        printchatline("*** ÇëÊäÈëÄãÒªÌßÏÂÕ¾µÄ ID ***");
        return;
  }else
  if(!strcasecmp(userid,currentuser.userid)) 
        sprintf(msg,"*** Faint!ÄãÒª°Ñ×Ô¼ºÌßÏÂÕ¾°¡£¿***");
  else
  {
        uin=t_search(userid,NA); 
        if(uin==NULL)
        {
              sprintf(msg,"%s ²¢Ã»ÓĞÉÏÕ¾",userid);
        }
        else
        {
               sprintf(buf,"kick %s out of bbs by %s", uin->userid, currentuser.userid);
	       report(buf); /* add by dong, 1999.1.9 */
	       kill(uin->pid,SIGHUP);
               /*uin->active = NA;   by dong 1998.7.7
               uin->pid = 0;
               uin->invisible = YEA;
               uin->sockactive = 0;
               uin->sockaddr = 0;
               uin->destuid = 0;
               update_ulist( uin, YEA ); ÎŞÒâÒå²¢ÇÒ²ÎÊı´íÎó£¬ËùÒÔ×¢ÊÍµô*/
               return;
	}
   }
  printchatline(msg);
}
int
cmpinames(userid, uv)       /* added by Luzi 1997.11.28 */
char    *userid;
char *uv;
{
 return !ci_strcmp(userid, uv);
}                   

void
call_listen(arg,fd) /* added by Luzi 1997.11.28 */
char *arg;
int fd;
{
   char uident[STRLEN];
   char path[40];
   char *userid;
   char ignoreuser[IDLEN+1];
   int  nIdx;
   userid = nextword(&arg);
   if (strchr("\n\r",*arg)) userid=NULL;
   else userid = nextword(&arg);

   if(userid==NULL)
     {
        printchatline("*** ÇëÊäÈëÓÃ»§µÄID ***");
        }
  else
   if(!strcasecmp(userid,currentuser.userid))
        printchatline("*** ÕâÊÇÄã×Ô¼ºµÄID ***");
   else   
     {
       strcpy(uident,strtok(userid," "));
       if(!searchuser(uident)) /* change getuser -> searchuser, by dong, 1999.10.26 */
        printchatline("*** Ã»ÓĞÕâ¸öID ***");
       else {
              sethomefile( path, currentuser.userid , "/ignores");
              nIdx=search_record( path,ignoreuser, IDLEN+1, cmpinames, userid );
              if (nIdx <= 0)
                 printchatline("*** ¸ÃÓÃ»§µÄÁÄÌìÑ¶Ï¢Ã»ÓĞ±»ºöÂÔ°¡ ***");
              else if (delete_record( path, IDLEN+1, nIdx)==0)
                     {
                       printchatline("*** ÒÑ»Ö¸´¶Ô¸ÃÓÃ»§ÁÄÌìÑ¶Ï¢µÄ½ÓÊÕ ***");
                       sprintf(uident,"/listen %s\n",userid);
                       send(fd, uident, strlen(uident), 0);
                      }
                 else printchatline("*** system error ***");
             }
       }
}                       

void
call_ignore(arg,fd)             /* added by Luzi 1997.11.28 */
char  *arg;
int fd;
{
   char buf[IDLEN+1],buf2[76];
   FILE *fp;
   char uident[STRLEN];
   char path[40];
   char *userid;
   char ignoreuser[IDLEN+1];
   int  nIdx;
   userid = nextword(&arg);
   if (strchr("\n\r",*arg)) userid=NULL;
   else userid = nextword(&arg);

   sethomefile( path, currentuser.userid , "/ignores");
   if(userid==NULL)
     {
       if((fp=fopen(path,"r"))==NULL)
           {
              printchatline("*** ÇëÊäÈëÒªºöÂÔµÄID ***");
              return;}     
       nIdx=0;
       strcpy(buf2,"¡¼ºöÂÔÆäÑ¶Ï¢µÄÓÃ»§IDÁĞ±í¡½");
       while(fread(buf, IDLEN+1, 1,fp)>-0 )
        {
          if (nIdx%4==0)
             {
                printchatline(buf2);
                *buf2='\0';
              }
          nIdx++;
          sprintf(buf2+strlen(buf2),"  %-13s",buf);
          }
        if (nIdx>0) printchatline(buf2);
        else printchatline("*** ÉĞÎ´Éè¶¨ºöÂÔÓÃ»§µÄÃûµ¥ ***");
        fclose(fp);
      }
  else
   if(!strcasecmp(userid,currentuser.userid))
        printchatline("*** ÎŞ·¨ºöÂÔ×Ô¼ºµÄĞÅÏ¢ ***");
   else
     {
       strcpy(uident,strtok(userid," "));
       if(!searchuser(uident))    /* change getuser -> searchuser, by dong, 1999.10.26 */
        printchatline("*** Ã»ÓĞÕâ¸ö ID ***");
       else {
              nIdx=search_record( path,ignoreuser, IDLEN+1, cmpinames, userid );
              if (nIdx > 0)
                 printchatline("*** ¸ÃIDÒÑ¾­±»ºöÂÔÁË ***");
              else
               {
                fp=fopen(path,"r");
                if (fp!=NULL)
                  {
                    fseek(fp,0,SEEK_END);
                    if (ftell(fp)>=(IDLEN+1)*MAX_IGNORE)
                      {
                        fclose(fp);
                        printchatline("*** ºöÂÔÓÃ»§Ãûµ¥ÒÑÂú ***");
                        return;
                       }
                    fclose(fp);
                   }
                if (append_record( path, userid, IDLEN+1)==0)
                 {
                    printchatline("*** ºöÂÔÒÑ¾­Éè¶¨ ***");     
                    sprintf(uident,"/ignore %s\n",userid);
                    send(fd, uident, strlen(uident), 0);
                   }
                else printchatline("*** ÏµÍ³´íÎó ***");
               }
            }
       }
}

void
call_alias(arg,fd)             /* added by Luzi 1998.01.25 */
char  *arg;
int fd;
{
   char buf[128],buf2[129];
   FILE *fp;
   char uident[128];
   char path[40];
   char *emoteid,*emotestr, *tmpemote, *tmpbuf;
   int  nIdx;
   emoteid = nextword(&arg);
   if (strchr("\n\r",*arg)) emoteid=NULL;
   else emoteid = nextword(&arg);

   sethomefile( path, currentuser.userid , "/emotes");
   if(emoteid==NULL)
     {
       if((fp=fopen(path,"r"))==NULL)
         {
           printchatline("*** ÇëÊäÈëÒª¶¨ÒåµÄemoteÃû³Æ ***");
           return;}
       nIdx=0;chat_clear();
       printchatline("¡¼ÓÃ»§×Ô¶¨ÒåemoteÁĞ±í¡½");
       while(fread(buf, 128, 1,fp)>-0 )
          {
             printchatline(buf);
             nIdx++;
             if (nIdx%18==0)
               {
                 outs("[5;31m              ....°´ÈÎÒâ¼ü¼ÌĞø....[m");
                 igetkey();
                }
           }
       fclose(fp);
      }
  else
     {
       if (strchr("\n\r",*arg)) emotestr=NULL;
       else emotestr=arg;
       nIdx=0;
       if((fp=fopen(path,"r"))!=NULL)
        {
           while(1)
             {
                if (fread(buf, 128, 1,fp)==0)
                 { nIdx=0;break; }
                tmpbuf=buf;
                tmpemote=nextword(&tmpbuf);
                nIdx++;
                if (ci_strcmp(emoteid,tmpemote)==0) break;
              }
           fclose(fp);
          }
       if (nIdx > 0)
        {
          if (emotestr!=NULL)
            {
                printchatline("*** ¸ÃemoteÒÑ¾­±»¶¨Òå¹ıÁË ***");
                return;
             }
          if (delete_record( path, 128, nIdx)==0)
             {
                printchatline("*** ¸Ã×Ô¶¨ÒåemoteÒÑ¾­±»É¾³ıÁË ***");
                sprintf(uident,"/alias_del %s\n",emoteid);
                send(fd, uident, strlen(uident), 0);
              }
          else printchatline("*** system error ***");
        }
       else if (emotestr==NULL)
              printchatline("*** ÇëÖ¸¶¨emote¶ÔÓ¦µÄ×Ö´® ***"); 
            else 
               {
                fp=fopen(path,"r");
                if (fp!=NULL)
                  {
                    fseek(fp,0,SEEK_END);
                    if (ftell(fp)>=128*MAX_EMOTES)
                      {
                        fclose(fp);
                        printchatline("*** ÓÃ»§×Ô¶¨ÒåemoteµÄÁĞ±íÒÑÂú ***");
                        return;
                       }
                    fclose(fp);
                   }
                sprintf(buf,"%s %s",emoteid,emotestr);
                if (append_record( path, buf, 128)==0)
                 {
                    printchatline("*** ×Ô¶¨ÒåemoteÒÑ¾­Éè¶¨ ***");
                    sprintf(uident,"/alias_add %s\n",buf);
                    send(fd, uident, strlen(uident), 0);
                   }
                else printchatline("*** ÏµÍ³´íÎó ***");
            }
       }
}

void call_mail() /* added by Luzi, 1997/12/22 */
{
  fileheader mailheader;
  extern char currmaildir[ STRLEN ] ;
  FILE *fpin;
  static char buf[512] ;
  char b2[512] ;
  char *t ;
  if (chkmail()==0) /* check mail */
    {
      printchatline("\033[32m*** Ã»ÓĞĞÂµÄĞÅ¼ş ***\033[m");
      return;
    }
  setmailfile(currmaildir, currentuser.userid, DOT_DIR);
  fpin = fopen(currmaildir, "rb");
  if (fpin == NULL) return;
  printchatline("\033[32m*** µ±Ç°ĞÂµÄĞÅ¼şÈçÏÂ ***\033[m");
  while(fread(&mailheader, sizeof (fileheader), 1, fpin))
    {
      if ((mailheader.accessed[0] & FILE_READ) == 0)
        {
          strncpy(b2,mailheader.owner,STRLEN) ;
          if( (t = strchr(b2,' ')) != NULL )
          *t = '\0' ;
          sprintf(buf,"\033[31m %-20.20s ¡ï %.46s \033[m",b2, mailheader.title);
          printchatline(buf);
         }
     }
  fclose(fpin);
}

void
build_ver()
{
  sprintf(genbuf, " µ±Ç°°æ±¾±àÒëÊ±¼ä: [32m%s %s[m", __DATE__, __TIME__ );
  printchatline(genbuf);
}

struct chat_command chat_cmdtbl[] = {
  {"pager",setpager,1,0},
  {"help", chat_help,1,0},
  {"clear", chat_clear,1,0},
  {"date", chat_date,1,0},
  {"g", chat_friends,1,0},/*
  {"users", chat_users,1,0},remed by Haohmaru.99.12.02.ÎŞÒâÒåÁË*/
  {"send", chat_sendmsg,1,0}, /*by alex , 96.9.5*/
  {"seemsg",chat_show_allmsgs,1,0}, /* snow change at 10.24 */
  {"set",set_rec,1,0},
  {"call",call_user,1,0},
  {"query",call_query,1,1},   /* modify by Luzi 1997.11.18 */
  {"qc", call_query_ByChatid, 2,1}, /* added by dong 1998.9.12 */
  {"x",call_kickoff,1,0},
  {"ignore",call_ignore,6,1}, /* added by Luzi 1997.11.28 */
  {"listen",call_listen,6,1}, /* added by Luzi 1997.11.28 */
  {"mail",call_mail,4,0},     /* added by Luzi 1997.12.22 */
  {"alias",call_alias,5,1},   /* added by Luzi 1998.01.25 */
  {"version",build_ver,1,0},  /* added by period 2000.10.29 */
  {NULL, NULL,1,0}
};       

int
chat_cmd_match(buf, str, nLenth)
  char *buf;
  char *str;
  short nLenth;
{
  short i=0;
  while (*str && *buf && !isspace(*buf))
  {
    if (tolower(*buf++) != *str++)
      return 0;
    i++;
  }
  return (i >= nLenth);
}                          

int
chat_cmd(buf, fd)
  char *buf;
  int fd;
{
  int i;

  if (*buf++ != '/')
    return 0;

  for (i = 0; chat_cmdtbl[i].cmdname; i++)
  {
    if (chat_cmd_match(buf, chat_cmdtbl[i].cmdname, chat_cmdtbl[i].nCmdLenth))
                                                    /* ^^^^Ôö¼Óµ÷ÓÃ²ÎÊı */
    {
      if (chat_cmdtbl[i].bNeed)    /*ĞÂÔö¸ù¾İchat_command.bNeed·ÖÖ§µ÷ÓÃ*/
         chat_cmdtbl[i].cmdfunc(buf,fd);
      else chat_cmdtbl[i].cmdfunc(buf);
      return 1;
    }
  }          
  return 0;
}    


