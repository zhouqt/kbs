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
    
    
    2001/5/6 modified by wwj
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

#include "chat.h"


extern char MsgDesUid[14]; /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong */
extern char BoardName[];

extern char page_requestor[];
extern int talkrequest;
extern char *modestring();
extern char pagerchar();
extern struct UTMPFILE *utmpshm;
extern void t_pager();



#define s_lines         (t_lines-2)
#define b_lines         (t_lines-1)
#define screen_lines    (t_lines-4)

int chat_cmd(chatcontext *pthis,char *buf);

void set_rec();
struct user_info *t_search();

#define CHAT_LOGIN_OK       "OK"
#define CHAT_LOGIN_EXISTS   "EX"
#define CHAT_LOGIN_INVALID  "IN"
#define CHAT_LOGIN_BOGUS    "BG"
const char *msg_seperator = "¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª";
const char *msg_shortulist = "\033[33m\033[44m Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬  ©¦ Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬  ©¦ Ê¹ÓÃÕß´úºÅ    Ä¿Ç°×´Ì¬ \033[m";

void printchatline(chatcontext * pthis, const char *str) /*ÏÔÊ¾Ò»ÐÐ£¬²¢ÏÂÒÆÖ¸Ê¾·û*/
{
    char tmpstr[256];
    const char *p;
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

    move(pthis->chatline, 0);
    clrtoeol();
    outs(tmpstr); /* snow change at 10.25 */
    outc('\n');
    
    if(pthis->rec)fprintf(pthis->rec,"%s\n",str);
    
    if (++pthis->chatline == s_lines)    pthis->chatline = 2;
    
    move(pthis->chatline, 0);
    clrtoeol();
    outs("==>");
}


void chat_clear(chatcontext * pthis) /* clear chat */
{
    for (pthis->chatline = 2; pthis->chatline < s_lines; pthis->chatline++)
    {
        move(pthis->chatline, 0);
        clrtoeol();
    }
    pthis->chatline = s_lines - 1;
    printchatline(pthis,"");
}


void print_chatid(chatcontext * pthis)
{
    move(b_lines, 0);
    outs(pthis->chatid);
    outc(':');
}


int chat_send(chatcontext * pthis, const char *buf)
{
    int  len;
    char gbuf[256];
    for(len=0;len<sizeof(gbuf)-1 && buf[len];len++)gbuf[len]=buf[len];
    gbuf[len++]='\n';
    return (send(pthis->cfd, gbuf, len, 0) == len);  /* Í¨¹ýsocket send */
}

int chat_waitkey(chatcontext *pthis)
{
	char ch;
    outs("             [5;31m*** °´¿Õ¸ñ¼ü¼ÌÐø ***[m");
    add_io(0, 0);
    ch=igetkey();
    add_io(pthis->cfd, 0);
    return strchr(" \r\n",ch)!=NULL;
}


/* buffered recv  2001/5/6 -- wwj */
int chat_recv(chatcontext * pthis, char * buf, int sz)
{
    int len;
    
    do {
        if(buf && pthis->bufptr){ /* Èç¹û»º³åÇøÀïÃæÓÐ£¬ÏÈ·µ»Ø¸øuser */
            len=sz;
            if(len>pthis->bufptr)len=pthis->bufptr;
            memcpy(buf,pthis->buf,len);
            if(len<pthis->bufptr){
                memcpy(pthis->buf,&pthis->buf[len],pthis->bufptr-len);
            }
            pthis->bufptr-=len;
            return len;
        }
        len = recv(pthis->cfd, &pthis->buf[pthis->bufptr], sizeof(pthis->buf)-pthis->bufptr, 0);
        if(len <= 0) return -1;
        pthis->bufptr+=len;
    } while(buf);
    return 0;
}

int chat_parse(chatcontext * pthis,int drecv)
{
    int  len;
    char *bptr;

    if(drecv)chat_recv(pthis,NULL,0);
    
    len=0;

    bptr = pthis->buf;
    while (bptr<pthis->buf+pthis->bufptr)  {
        for(len=0;bptr[len];len++){
            if(bptr+len==pthis->buf+pthis->bufptr){
                len=-1;
                break;
            }
        }
        if(len<0)break;
        len++; /* skip 0 */

        if (*bptr == '/') /* ´¦Àíserver´«À´µÄÃüÁî */
        {
            switch (bptr[1])
            {
            case 'p':
                /* add by KCN for list long emote */
                chat_waitkey(pthis);
                break;
                
            case 'c':
                chat_clear(pthis);
                break;
                
            case 'n':
                strncpy(pthis->chatid, bptr + 2, 8);
                pthis->chatid[8] = 0;
                print_chatid(pthis);
                clrtoeol();
                
                uinfo.in_chat = YEA;
                strcpy(uinfo.chatid, pthis->chatid);
                UPDATE_UTMP_STR(chatid,uinfo);
                break;
                
            case 'r':
                strncpy(pthis->chatroom, bptr + 2, IDLEN - 1);
                break;
                
            case 'z':
                strncpy(pthis->chatroom, bptr + 2, IDLEN - 1);
                /* fall throw */
            case 't':
                move(0, 0);
                clrtoeol();
                sprintf(genbuf, "·¿¼ä£º [36m%s", pthis->chatroom);
                if (bptr[1]=='t') strcpy(pthis->topic,bptr+2);
                prints("[44m[33m %-21s  [33m»°Ìâ£º[36m%-51s[31m%2s[m", genbuf, pthis->topic ,(pthis->rec)?"Â¼":"  ");
                break;
            }
        } else {
            printchatline(pthis,bptr);
        }
        bptr += len;
    }

    if (len< 0)
    {
        strcpy(genbuf, bptr);
        strcpy(pthis->buf, genbuf);
        pthis->bufptr-=bptr-pthis->buf;
    } else {
        pthis->bufptr = 0;
    }
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

/* 2001/5/6 --wwj, ÐÞ¸Ä ent_chat º¯Êý */

int ent_chat_conn(chatcontext * pthis, int chatnum)
{
    struct sockaddr_in sin;
    int    ch;
    char   inbuf[128],*ptr;
    
    memset(&sin, 0, sizeof sin);
    sin.sin_family = PF_INET;
    sin.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    if (chatnum==1)
        sin.sin_port = htons(CHATPORT3);
    else
        sin.sin_port = htons(CHATPORT2);

    pthis->cfd = socket(sin.sin_family, SOCK_STREAM, 0);
    if (connect(pthis->cfd, (struct sockaddr *) & sin, sizeof sin)) /*Èç¹ûÁ¬½Ó chatd Ê§°Ü£¬ÔòÆô¶¯chatd */
    {
        close(pthis->cfd);

        switch (ch = fork())
        {
        case -1/*fork failure*/:
            log("3chatd","fork error");
            break;
        case 0/*fork success*/:
            log("chatd","fork success");
            prints("¿ªÆôÁÄÌìÊÒ...");
            if (chatnum==1)
                system("bin/chatd");
            else
                system("bin/chatd 2");
            exit(1);
        default:
            log("chatd","fork par-proc");
            /* The chat daemon forks so we can wait on it here. */
            waitpid(ch, NULL, 0);
        }

        pthis->cfd = socket(sin.sin_family, SOCK_STREAM, 0);
        if ((connect(pthis->cfd, (struct sockaddr *) & sin, sizeof sin)))
        {
            close(pthis->cfd);
            log("3chatd","connect2 failed %d", errno);
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

        if ('*' == inbuf[0]) /* Leeward 98.04.26 */
        {
            close(pthis->cfd);
            return 0;
        }
        
        if( inbuf[0]!='\0'&&inbuf[0]!='\n'&&inbuf[0]!='/' ){
            strncpy(pthis->chatid,inbuf,8);
        } else {
            strncpy(pthis->chatid,currentuser.userid,8);
        }
        pthis->chatid[8] = '\0';

        sprintf(inbuf, "/! %d %d %s %s", uinfo.uid, currentuser.userlevel, currentuser.userid, pthis->chatid);
        chat_send(pthis, inbuf); /* send user info to chatd , and chatd will check it*/
        
        if (chat_recv(pthis, inbuf, 3) != 3) {
            close(pthis->cfd);
            return 0;
        }
        if (!strcmp(inbuf, CHAT_LOGIN_OK))
            break;
        else if (!strcmp(inbuf, CHAT_LOGIN_EXISTS))
            ptr = "Õâ¸ö´úºÅÒÑ¾­ÓÐÈËÓÃÁË";
        else if (!strcmp(inbuf, CHAT_LOGIN_INVALID))
            ptr = "Õâ¸ö´úºÅÊÇ´íÎóµÄ";
        else
            ptr = "ÒÑ¾­ÓÐÒ»¸ö´°¿ÚÔÚÁÄÌìÊÒÀïÁË£¨Èô·ÇÈç´ËÇëÍË³öBBSÖØÐÂµÇÂ¼£»Èô»¹²»ÐÐÔÙÕÒÔÚÏßÕ¾³¤£©";
        move(3, 0);
        prints(ptr);
        clrtoeol();
        bell();
    }
    return 1;
}

int ent_chat(int chatnum)  /* ½øÈëÁÄÌìÊÒ*/
{
    chatcontext *pthis;
    
    char inbuf[128];
    int  ch,cmdpos;
    int  currchar;
    int  modified; /* the line is modified? -- wwj */
    int  newmail;
    int  page_pending = NA;
    int  chatting = YEA;

    if (!strcmp(currentuser.userid,"guest")) return -1;
    
    pthis=(chatcontext*)malloc(sizeof(chatcontext));
    bzero(pthis,sizeof(chatcontext));
    if(!pthis)return -1;
    
    modify_user_mode(CHAT1);
    ch=ent_chat_conn(pthis,chatnum);
    if(ch!=1){
        free(pthis);
        return ch;
    }
    
    add_io(pthis->cfd, 0);

    modified = newmail = cmdpos = currchar = 0;

    /* update uinfo */
    uinfo.in_chat = YEA;
    strcpy(uinfo.chatid, pthis->chatid);
    UPDATE_UTMP(in_chat,uinfo);
    UPDATE_UTMP_STR(chatid,uinfo);
    
    /* initiate screen */
    clear();
    pthis->chatline = 2;
    move(s_lines, 0);
    outs(msg_seperator);
    move(1, 0);
    outs(msg_seperator);
    print_chatid(pthis);
    
    
    memset(inbuf, 0, 80);

    /* chat begin */
    while (chatting)
    {
        if(pthis->bufptr && chat_parse(pthis,0) == -1) break; /* have more data? */
        
        move(b_lines, currchar + 10);
        ch = igetkey();

        if (talkrequest)
            page_pending = YEA;
        if (page_pending)
            page_pending = servicepage(0, NULL);

        switch (ch)
        {
        case KEY_UP:
        case KEY_DOWN:
            if(cmdpos==pthis->cmdpos){
            	strcpy(pthis->lastcmd[cmdpos],inbuf);
            	modified=0;
            }
            if(ch==KEY_UP) {
            	if(cmdpos!=(pthis->cmdpos+1)%MAXLASTCMD){
            		int i=(cmdpos+MAXLASTCMD-1)%MAXLASTCMD;
            		if(pthis->lastcmd[i][0])cmdpos=i;
                }
            }
            if(ch==KEY_DOWN){
            	if(cmdpos!=pthis->cmdpos)cmdpos=(cmdpos+1)%MAXLASTCMD;
            }
       	    strcpy(inbuf, pthis->lastcmd[cmdpos]);
       	    
       	    if(cmdpos==pthis->cmdpos){
       	        modified=1;
       	    }

            move(b_lines, 10);
            clrtoeol();
            ch=inbuf[69];
            inbuf[69]=0;
            outs(inbuf);
            inbuf[69]=ch;
            currchar = strlen(inbuf);
            continue;

        case KEY_LEFT:
            if (currchar)
                --currchar;
            continue;

        case KEY_RIGHT:
            if (inbuf[currchar])
                ++currchar;
            continue;
        
        case KEY_ESC:    
        case Ctrl('X'):
            inbuf[0]=0;
            currchar=0;
            move(b_lines, currchar + 10);
            clrtoeol();
            modified=1;
            continue;
        
        case Ctrl('A'):
            currchar=0;
            continue;
        
        case Ctrl('E'):
            currchar=strlen(inbuf);
            continue;
        }

        if (!newmail && chkmail(0)) /* check mail */
        {
            newmail = 1;
            printchatline(pthis,"[32m*** [31mµ±£¡ÄãÓÐÐÂÐÅÀ´À²...[m");
        }

        if (ch == I_OTHERDATA)      /* incoming */
        {
            if (chat_parse(pthis,1) == -1) break;
            continue;
        }

#ifdef BIT8
        if (isprint2(ch))
#else
        if (isprint(ch))
#endif

        {
            if (currchar < 126) /* Î´ÂúÒ»ÐÐ,print it*/
            {
                modified=1;
                
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
                if(modified){
                    /* add to command history */
                    ch=sizeof(pthis->lastcmd[pthis->cmdpos])-1;
                    strncpy(pthis->lastcmd[pthis->cmdpos], inbuf,ch);
                    pthis->lastcmd[pthis->cmdpos][ch]=0;
                    
                    pthis->cmdpos=(pthis->cmdpos+1)%MAXLASTCMD;
                    cmdpos=pthis->cmdpos;
                } else { /* use history, so can +1 */
                    cmdpos=(cmdpos+1)%MAXLASTCMD;
                }
                
                if(inbuf[0]=='/' && Isspace(inbuf[1]) ){ /* discard / b */
                    printchatline(pthis,"\x1b[37m*** \x1b[32mÇëÊäÈëÕýÈ·µÄÖ¸Áî£¬Ê¹ÓÃ/hÑ°Çó°ïÖú\x1b[37m ***\x1b[m");
                } else {
                    chatting = chat_cmd(pthis,inbuf); /*localÃüÁî´¦Àí*/
                    if (chatting == 0) chatting = chat_send(pthis, inbuf);

                    if(inbuf[0]=='/'){
                        ch = 1;
                        while (inbuf[ch]!='\0' && inbuf[ch]!=' ') ch ++;
                        if (ch > 1)
                        {
                            if (!strncasecmp(inbuf, "/bye", ch)) break;
                            if (!strncasecmp(inbuf, "/exit", ch)) break; /*added by alex, 96.9.5*/
                        }
                    }
                }
                
                modified=0; 
                inbuf[0] = '\0';
                currchar = 0;
                move(b_lines, 10);
                clrtoeol();
            }
            continue;
        }

        if (ch == Ctrl('H') || ch == '\177') /*Backspace */
        {
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
            chat_send(pthis, "/b");
            if(pthis->rec) set_rec();
            break;
        }
    }
    /* chat end */
    close(pthis->cfd);
    
    add_io(0, 0);
    
    uinfo.in_chat = NA;
    uinfo.chatid[0] = '\0';
    UPDATE_UTMP(in_chat,uinfo);
    UPDATE_UTMP(chatid[0],uinfo);
    clear();
    refresh();
  
    free(pthis);
    
    return 0;
}


int printuserent(chatcontext *pthis,struct user_info *uentp) /* print one user & status */
{
    static char uline[256];
    static int cnt;
    char pline[50];

    if (!uentp)
    {
        if (cnt)
            printchatline(pthis,uline);
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

    sprintf(pline, " %s%-13s[m%c%-10s",myfriend(uentp->uid,NULL)?"[32m":"", uentp->userid, uentp->invisible ? '#' : ' ',
            modestring(uentp->mode, uentp->destuid, 0, NULL));
    if (cnt < 2)
        strcat(pline, "©¦");
    strcat(uline, pline);
    if (++cnt == 3)
    {
        printchatline(pthis,uline);
        memset(uline, 0, 256);
        cnt = 0;
    }
    return 0;
}

int
print_friend_ent(struct user_info *uentp,chatcontext *pthis,int pos) /* print one user & status if he is a friend*/
{
    char pline[50];

    if (!uentp->active || !uentp->pid)
        return 0;
    if (!HAS_PERM(PERM_SEECLOAK) && uentp->invisible)
        return 0;

#if 0
    if (kill(uentp->pid, 0) == -1)
        return 0;
#endif

    if (!myfriend(uentp->uid,NULL))
        return 0;
    sprintf(pline, " %-13s%c%-10s",uentp->userid, uentp->invisible ? '#' : ' ',
            modestring(uentp->mode, uentp->destuid, 0, NULL));
    if (pthis->apply_count < 2)
        strcat(pline, "©¦");
    strcat(pthis->apply_buf, pline);
    if (pthis->apply_count == 2)
    {
        printchatline(pthis,pthis->apply_buf);
        memset(pthis->apply_buf, 0, 256);
        pthis->apply_count = 0;
    } else
		pthis->apply_count++;
    return COUNT;
}

void chat_help(chatcontext *pthis,const char *arg)
{
    char buf[256];
    FILE *fp;

    if (strstr(arg, "op")) /* op's help*/
    {
        if((fp=fopen("help/chatophelp","r"))==NULL)
            return;
        while(fgets(buf, 255, fp) != NULL)
        {
            printchatline(pthis,buf);
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
            printchatline(pthis,buf);
        }
        fclose(fp);
    }
}

void call_user(chatcontext *pthis,const char *arg) /* invite user online to chatroom*/
{
    char  msg[STRLEN*2];
    struct user_info *uin ;
    int state;

    if(!*arg) {
        printchatline(pthis,"[37m*** [32mÇëÊäÈëÄãÒªÑûÇëµÄ ID[37m ***[m");
        return;
    }
    if(!strcasecmp(arg,currentuser.userid))
        sprintf(msg,"[32mÄã²»ÓÃÑûÇë×Ô¼º°¡[m");
    else if (!HAS_PERM(PERM_PAGE))  /* Leeward 98.07.30 */
        sprintf(msg,"[32mÄãÃ»ÓÐ·¢ÐÅÏ¢µÄÈ¨ÏÞ[m");
    else
    {
        uin=t_search(arg,NA);
        if(uin==NULL)
            sprintf(msg,"[32m%s[37m ²¢Ã»ÓÐÉÏÕ¾[m",arg);
        else if(LOCKSCREEN == uin->mode) /* Leeward 98.02.28 */
            sprintf(msg,"[32m%s[37mÒÑ¾­Ëø¶¨ÆÁÄ»£¬ÇëÉÔºòÔÙÑûÇë»ò¸øËû(Ëý)Ð´ÐÅ.\n", uin->userid);
        else
            if(!canmsg(uin))
                sprintf(msg,"%s ÒÑ¾­¹Ø±Õ½ÓÊÜÑ¶Ï¢µÄºô½ÐÆ÷.\n", uin->userid);
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
                    sprintf(msg,"¶Ô·½ÉÐÓÐÒ»Ð©Ñ¶Ï¢Î´´¦Àí£¬ÇëÉÔºòÔÙÑûÇë»ò¸øËû(Ëý)Ð´ÐÅ...\n");

                else {
                    sprintf(msg,"µ½ÁÄÌì¹ã³¡µÄ %s ÁÄÌìÊÒ ÁÄÁÄÌì",pthis->chatroom);
                    /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong*/
                    strcpy(MsgDesUid, uin->userid);
                    state = do_sendmsg(uin,msg,1);
                    if (state == 1)
                        sprintf(msg,"[37mÒÑ¾­°ïÄãÑûÇë %s ÁË[m",uin->userid);
                    else if(state == -2)
                        sprintf(msg,"[37m¶Ô·½ÒÑ¾­ÀëÏßÁË...[m");
                }
            }
    }
    printchatline(pthis,msg);
}


void chat_sendmsg(chatcontext *pthis,const char *arg) /* send msg in chatroom , by alex, 96.9.5*/
{
    char   userid[IDLEN+1];
    char   msg[STRLEN*2];
    struct user_info *uin ;
    int    state;
    
    nextword(&arg,userid,sizeof(userid));
    
    if(!userid[0]) {
        printchatline(pthis,"\x1b[37m*** \x1b[32mÇëÊäÈëÄãÒª·¢ÏûÏ¢µÄ ID\x1b[37m ***\x1b[m");
        return;
    } else if (!*arg) {
        printchatline(pthis,"\x1b[37m*** \x1b[32mÇëÊäÈëÄãÒª·¢µÄÏûÏ¢\x1b[37m ***\x1b[m");
        return;
    } else if(!HAS_PERM(PERM_PAGE)) {  /* Leeward 98.07.30 */
		sprintf(msg,"\x1b[32mÄãÃ»ÓÐ·¢ÐÅÏ¢µÄÈ¨ÏÞ\x1b[m");
	} else {
		uin=t_search(userid,NA);
		if(uin==NULL)
		{
			sprintf(msg,"\x1b[32m%s\x1b[37m ²¢Ã»ÓÐÉÏÕ¾\x1b[m",userid);
		}
		else if(LOCKSCREEN == uin->mode) /* Leeward 98.02.28 */
			sprintf(msg,"\x1b[32m%s\x1b[37mÒÑ¾­Ëø¶¨ÆÁÄ»£¬ÇëÉÔºòÔÙ·¢»ò¸øËû(Ëý)Ð´ÐÅ.\n", uin->userid);
		else
		{
			if(!canmsg(uin))
				sprintf(msg,"\x1b[32m%s\x1b[37mÒÑ¾­¹Ø±Õ½ÓÊÜÑ¶Ï¢µÄºô½ÐÆ÷.\n", uin->userid);
			else if(NA==canIsend2(uin->userid))
				sprintf(msg,"\x1b[32m%s\x1b[37m¾Ü¾ø½ÓÊÜÄãµÄÑ¶Ï¢.\n",uin->userid);/*Haohmaru.99.6.6,¼ì²éÊÇ·ñ±»ignore*/
			else {  
			    FILE *fp;
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
				    sprintf(msg,"¶Ô·½ÉÐÓÐÒ»Ð©Ñ¶Ï¢Î´´¦Àí£¬ÇëÉÔºòÔÙ·¢»ò¸øËû(Ëý)Ð´ÐÅ...\n");
			    else
			    {
				    /* ±£´æËù·¢msgµÄÄ¿µÄuid 1998.7.5 by dong*/
				    strcpy(MsgDesUid, uin->userid);
				    state = do_sendmsg(uin,arg,2);
				
				    if (state == 1)
					    sprintf(msg,"\x1b[37mÒÑ¾­·¢ÏûÏ¢¸ø \x1b[32m%s\x1b[37m ÁË\x1b[m",uin->userid);
				    else if (state == -2)
					    sprintf(msg,"\x1b[37m¶Ô·½ÒÑ¾­ÀëÏßÁË...\x1b[m");
			    }
			}
		}
	}
    printchatline(pthis,msg);
}

int c_cmpuids(int uid,struct user_info *up)
{
    return (uid == up->uid) ;
}

int chat_status(struct user_info *uentp,chatcontext *pthis) 
{
    char  tmpstr[31];
    char *lpTmp;
    
	if(uentp->invisible==1)
	{
		if(HAS_PERM(PERM_SEECLOAK))
		{
			sprintf(genbuf+strlen(genbuf),"\x1b[32m#\x1b[m");
		}
		else return 0;
	}
	lpTmp=(char*)idle_str(uentp);
	if (uentp->in_chat) /* add by Luzi 1997.11.18*/
	{
		sprintf(tmpstr,"/q %s",uentp->userid);
		chat_send(pthis,tmpstr);
		
		tmpstr[chat_recv(pthis, tmpstr, 30)]='\0';
		
		if (tmpstr[0]=='1')
		{  
		    sprintf(genbuf+strlen(genbuf),"'%s' room as '%s'", tmpstr+1,uentp->chatid);
		    if (lpTmp[0]!=' ')
			    sprintf(genbuf+strlen(genbuf),"[%s];",lpTmp);
		    else strcat(genbuf," ;");
		    return COUNT;
		}
	}
	sprintf(genbuf,"%s%-8s",genbuf,modestring(uentp->mode,
		uentp->destuid, 0,/* 1->0 ²»ÏÔÊ¾ÁÄÌì¶ÔÏóµÈ modified by dong 1996.10.26 */
		(uentp->in_chat ? uentp->chatid : NULL)));
	if (lpTmp[0]!=' ')
		sprintf(genbuf+strlen(genbuf),"[%s];",lpTmp);
	else strcat(genbuf," ;");
    return COUNT;
}

static void query_user(chatcontext *pthis,const char* userid)
{
    int  tuid=0;
    char qry_mail_dir[STRLEN],inbuf[STRLEN*2];
    char* newline;
    time_t exit_time,temp;

    if(!(tuid = getuser(userid))) {
        printchatline(pthis,"[32mÕâ¸öID²»´æÔÚ£¡[m");
        return;
    }

    setmailfile(qry_mail_dir, lookupuser.userid, DOT_DIR);

    /*---	modified by period	2000-11-02	hide posts/logins	---*/
#ifdef _DETAIL_UINFO_
    sprintf(genbuf, "%s (%s):      %s", lookupuser.userid, lookupuser.username,
            (check_query_mail(qry_mail_dir)==1)? "ÓÐÐÂÐÅ":"    ");
    printchatline(pthis,genbuf);
    sprintf(genbuf,"¹²ÉÏÕ¾ %d ´Î£¬·¢±í¹ý %d ÆªÎÄÕÂ£¬ÉúÃüÁ¦[%d]%s",
            lookupuser.numlogins, lookupuser.numposts,
            compute_user_value(&lookupuser),
            (lookupuser.userlevel & PERM_SUICIDE)?" (×ÔÉ±ÖÐ)":" ");
    printchatline(pthis,genbuf);
#else
    sprintf(genbuf, "%s (%s):   ÉúÃüÁ¦[%d]%s   %s",
            lookupuser.userid, lookupuser.username,
            compute_user_value(&lookupuser),
            (lookupuser.userlevel & PERM_SUICIDE)?" (×ÔÉ±ÖÐ)":" ",
            (check_query_mail(qry_mail_dir)==1)? "ÓÐÐÂÐÅ":"    ");
    printchatline(pthis,genbuf);
#endif /*_DETAIL_UINFO_*/
    strcpy(inbuf, ctime(&(lookupuser.lastlogin)));
    if( (newline = strchr(genbuf, '\n')) != NULL )
        *newline = '\0';

    strcpy(genbuf,"µ±Ç°×´Ì¬£º");
    if (apply_utmpuid((APPLY_UTMP_FUNC) chat_status,tuid,(char*)pthis)) {
    	char buf[1024];
        sprintf(buf, "Ä¿Ç°ÕýÔÚÏßÉÏ: À´×Ô %s ÉÏÏßÊ±¼ä %s"/*\n"*/,
                (lookupuser.lasthost[0] == '\0'/* || DEFINE(DEF_HIDEIP)*/ ? "(²»Ïê)" : lookupuser.lasthost), inbuf);/*Haohmaru.99.12.18*/
        printchatline(pthis,buf);
        
	    printchatline(pthis,genbuf);
    } else {
        sprintf(genbuf, "ÉÏ´ÎÉÏÏßÀ´×Ô  %s Ê±¼äÎª %s "/*\n"*/ ,
                (lookupuser.lasthost[0] == '\0'/* || DEFINE(DEF_HIDEIP)*/ ? "(²»Ïê)" : lookupuser.lasthost), inbuf);/* Haohmaru.99.12.18*/
        printchatline(pthis,genbuf);

        /* »ñµÃÀëÏßÊ±¼ä Luzi 1998/10/23 */
        exit_time = get_exit_time(lookupuser.userid,genbuf);
        if( (newline = strchr(genbuf, '\n')) != NULL )
            *newline = '\0';
        if (exit_time > lookupuser.lastlogin) strcpy(inbuf,genbuf);
        /*Haohmaru.98.12.04.ºÍ²Ëµ¥²éÑ¯½á¹ûÒ»ÖÂ*/
        if (exit_time <= lookupuser.lastlogin )
        	/*
                || (uin.active && uin.pid
                    && (!uin.invisible || (uin.invisible && HAS_PERM(PERM_SEECLOAK)))))
            */
            strcpy(inbuf,"ÒòÔÚÏßÉÏ»ò·Ç³£¶ÏÏß²»Ïê");
        if (exit_time <= lookupuser.lastlogin) /* && (uin.invisible&& !HAS_PERM(PERM_SEECLOAK)))*/
        {
            temp=lookupuser.lastlogin+(lookupuser.numlogins%7)+5;
            strcpy(inbuf,ctime(&temp));/*Haohmaru.98.12.04.ÈÃÒþÉíÓÃ»§¿´ÉÏÈ¥ÀëÏßÊ±¼ä±ÈÉÏÏßÊ±¼äÍí5ÃëÖÓ*/
            if( (newline = strchr(inbuf, '\n')) != NULL )
                *newline = '\0';
        }
        /*       else strcpy(inbuf,"[Òò·Ç³£¶ÏÏß²»Ïê]");*/
        sprintf(genbuf, "ÀëÏßÊ±¼äÎª %s "/*\n"*/ , inbuf);
        printchatline(pthis,genbuf);
    }
#ifdef DEBUG
    if(HAS_PERM(PERM_SYSOP)) {
	sprintf(genbuf, "%d", tuid);
	printchatline(pthis,genbuf);
    }
#endif
}

void call_query(chatcontext *pthis,const char *arg) /* by alex, 1996.9.5*/ /* by Luzi, 1997.11.30 */
{
    if (!*arg) {
        printchatline(pthis,"*** ÇëÊäÈëÒª²éÑ¯µÄID ***");
    } else {
        query_user(pthis,arg);
    }
}

void call_query_ByChatid(chatcontext *pthis, const char *arg) /* add by dong, 1998.9.12*/
{
    char        uident[32];
    char        tmpstr[40];

    if (!*arg) {
        printchatline(pthis,"*** ÇëÊäÈëÒª²éÑ¯µÄchat ID ***");
        return;
    }
    strncpy(uident,arg,32);
    uident[31] = '\0';

    /* get user id from the chat id */
    sprintf(tmpstr,"/qc %s", uident);
    chat_send(pthis, tmpstr);
    tmpstr[chat_recv(pthis, tmpstr, 40)]='\0';
    if (tmpstr[0]=='1')
    {
        sprintf(uident, "%s", tmpstr+1);
    }
    else{
        sprintf(genbuf,"[32mÕâ¸öchat ID²»´æÔÚ£¡[m");
        printchatline(pthis,genbuf);
        return;
    }

    query_user(pthis,uident);
}

void chat_date(chatcontext *pthis, const char *arg)
{
    time_t thetime;

    time(&thetime);
    sprintf(genbuf, " %s±ê×¼Ê±¼ä: [32m%s[m", BoardName, Cdate(&thetime));
    printchatline(pthis,genbuf);
}

void chat_friends(chatcontext *pthis, const char *arg)
{
    /* printchatline(pthis,"");   moved by wwj */
    int num,i;

    pthis->apply_count=0;
    sprintf(genbuf,"[1m¡¾ µ±Ç°ÏßÉÏµÄºÃÓÑÁÐ±í ¡¿[m");
    printchatline(pthis,genbuf);
    printchatline(pthis,msg_shortulist);

	pthis->apply_buf=genbuf;
	pthis->apply_buf[0]=0;
	num = 0;
   	for (i=0;i<nf;i++) {
/*        sort_user_record(0,i2-1);*/
	    num+=apply_utmpuid((APPLY_UTMP_FUNC)print_friend_ent,topfriend[i].uid,(char*)pthis);
   	}
   	if (pthis->apply_count)
   		printchatline(pthis,pthis->apply_buf);
    if (num == 0)
    {
        printchatline(pthis,"[1mÃ»ÓÐÅóÓÑÔÚÏßÉÏ[m");
    }
}

void set_rec(chatcontext *pthis, const char *arg) /* set recorder */
{
    char fname[STRLEN];
    time_t now;

    now=time(0);
    /*        if(!HAS_PERM(PERM_SYSOP))
                    return;*/

    sprintf(fname,"etc/%s.chat",currentuser.userid);
    if(!pthis->rec)
    {
        if((pthis->rec=fopen(fname,"w"))==NULL)return;

        printchatline(pthis,"[5m[32mRecord Start ...[m");
        move(0, 0);
        clrtoeol();
        sprintf(genbuf, "·¿¼ä£º [36m%s", pthis->chatroom);
        prints("[44m[33m %-21s  [33m»°Ìâ£º[36m%-51s[31m%2s[m", genbuf, pthis->topic ,(pthis->rec)?"Â¼":"  ");

        fprintf(pthis->rec,"·¢ÐÅÈË: %s (%s) ·¿¼ä: %s\n»°  Ìâ: %s\x1b[m\n\n",
            currentuser.userid, currentuser.username, pthis->chatroom, pthis->topic);
	    fprintf(pthis->rec, "±¾¶ÎÓÉ %s",currentuser.userid);
        fprintf(pthis->rec,"ËùÂ¼ÏÂ£¬Ê±¼ä£º %s",ctime(&now));
        
        log("user","start record room %s", pthis->chatroom);
    } else {
        move(0, 0);
        clrtoeol();
        sprintf(genbuf, "·¿¼ä£º [36m%s", pthis->chatroom);
        prints("[44m[33m %-21s  [33m»°Ìâ£º[36m%-51s[31m%2s[m", genbuf, pthis->topic ,(pthis->rec)?"Â¼":"  ");

        printchatline(pthis,"[5m[32mRecord Stop ...[m");
        fprintf(pthis->rec,"½áÊøÊ±¼ä£º%s\n",ctime(&now));
        fclose(pthis->rec);
        pthis->rec=NULL;
        
        mail_file(fname,currentuser.userid,"Â¼Òô½á¹û");
        unlink(fname);

        log("user","stop record room %s", pthis->chatroom);
    }
}


void setpager(chatcontext *pthis, const char *arg)
{
    char buf[STRLEN];

    t_pager();/*toggle pager*/
    sprintf(buf,"[32m*** [31mÄã %s ÁËºô½ÐÆ÷[m",(uinfo.pager&ALL_PAGER)?"´ò¿ª":"¹Ø±Õ");
    printchatline(pthis,buf);

}

void call_kickoff(chatcontext *pthis, const char *arg) /* kick ID off BBS, by Luzi, 97.11.1*/
{
    char msg[STRLEN];
    struct user_info *uin ;

    if(!HAS_PERM(PERM_SYSOP))
    {
        printchatline(pthis,"*** Äã²»ÊÇÕ¾³¤ ***");
        return;
    }

    if(!*arg)
    {
        printchatline(pthis,"*** ÇëÊäÈëÄãÒªÌßÏÂÕ¾µÄ ID ***");
        return;
    } else
        if(!strcasecmp(arg,currentuser.userid))
            sprintf(msg,"*** Faint!ÄãÒª°Ñ×Ô¼ºÌßÏÂÕ¾°¡£¿***");
        else
        {
            uin=t_search(arg,NA);
            if(uin==NULL)
            {
                sprintf(msg,"%s ²¢Ã»ÓÐÉÏÕ¾",arg);
            } else {
                log("user","kick %s out of bbs", uin->userid);
                kill(uin->pid,SIGHUP);
                return;
            }
        }
    printchatline(pthis,msg);
}

int cmpinames(const char *userid,const  char *uv)       /* added by Luzi 1997.11.28 */
{
    return !strcasecmp(userid, uv);
}

void call_listen(chatcontext *pthis,const char *arg) /* added by Luzi 1997.11.28 */
{
    char path[40];
    char uident[IDLEN+1];
    char ignoreuser[IDLEN+1];
    int  nIdx;
    
    if(!*arg) {
        printchatline(pthis,"*** ÇëÊäÈëÓÃ»§µÄID ***");
    } else
        if(!strcasecmp(arg,currentuser.userid))
            printchatline(pthis,"*** ÕâÊÇÄã×Ô¼ºµÄID ***");
        else
        {
            strncpy(uident,arg,IDLEN+1);
            uident[IDLEN+1]=0;
            if(!searchuser(uident)) /* change getuser -> searchuser, by dong, 1999.10.26 */
                printchatline(pthis,"*** Ã»ÓÐÕâ¸öID ***");
            else {
                sethomefile( path, currentuser.userid , "/ignores");
                nIdx=search_record( path,ignoreuser, IDLEN+1, cmpinames, uident );
                if (nIdx <= 0)
                    printchatline(pthis,"*** ¸ÃÓÃ»§µÄÁÄÌìÑ¶Ï¢Ã»ÓÐ±»ºöÂÔ°¡ ***");
                else if (delete_record( path, IDLEN+1, nIdx)==0)
                {
                    log("user","listen %s", uident);
                    printchatline(pthis,"*** ÒÑ»Ö¸´¶Ô¸ÃÓÃ»§ÁÄÌìÑ¶Ï¢µÄ½ÓÊÕ ***");
                    sprintf(uident,"/listen %s\n",uident);
                    chat_send(pthis,uident);
                } else {
                    log("3user","listen %s failed", uident);
                    printchatline(pthis,"*** system error ***");
                }
            }
        }
}

void call_ignore(chatcontext *pthis,const char *arg)             /* added by Luzi 1997.11.28 */
{
    char buf[STRLEN],buf2[76];
    FILE *fp;
    char uident[IDLEN+1];
    char path[40];
    char ignoreuser[IDLEN+1];
    int  nIdx;

    sethomefile( path, currentuser.userid , "/ignores");
    if(!*arg)
    {
        nIdx=0;
        if((fp=fopen(path,"r"))!=NULL)
        {
            strcpy(buf2,"¡¼ºöÂÔÆäÑ¶Ï¢µÄÓÃ»§IDÁÐ±í¡½");
            while(fread(buf, IDLEN+1, 1,fp)>-0 )
            {
                if (nIdx%4==0)
                {
                    printchatline(pthis,buf2);
                    *buf2='\0';
                }
                nIdx++;
                sprintf(buf2+strlen(buf2),"  %-13s",buf);
            }
            fclose(fp);
        }
        if (nIdx>0) 
            printchatline(pthis,buf2);
        else 
            printchatline(pthis,"*** ÉÐÎ´Éè¶¨ºöÂÔÓÃ»§µÄÃûµ¥ ***");
    } else
        if(!strcasecmp(arg,currentuser.userid))
            printchatline(pthis,"*** ÎÞ·¨ºöÂÔ×Ô¼ºµÄÐÅÏ¢ ***");
        else
        {
            strncpy(uident,arg,IDLEN+1);
            uident[IDLEN+1]=0;
            if(!searchuser(uident))    /* change getuser -> searchuser, by dong, 1999.10.26 */
                printchatline(pthis,"*** Ã»ÓÐÕâ¸ö ID ***");
            else {
                nIdx=search_record( path,ignoreuser, IDLEN+1, cmpinames, uident );
                if (nIdx > 0)
                    printchatline(pthis,"*** ¸ÃIDÒÑ¾­±»ºöÂÔÁË ***");
                else
                {
                    fp=fopen(path,"r");
                    if (fp!=NULL)
                    {
                        fseek(fp,0,SEEK_END);
                        if (ftell(fp)>=(IDLEN+1)*MAX_IGNORE)
                        {
                            fclose(fp);
                            printchatline(pthis,"*** ºöÂÔÓÃ»§Ãûµ¥ÒÑÂú ***");
                            return;
                        }
                        fclose(fp);
                    }
                    if (append_record( path, uident, IDLEN+1)==0)
                    {
                        printchatline(pthis,"*** ºöÂÔÒÑ¾­Éè¶¨ ***");
                        
                        log("user","ignore %s", uident);
                        
                        sprintf(buf,"/ignore %s",uident);
                        chat_send(pthis,buf);
                    } else {
                        printchatline(pthis,"*** ÏµÍ³´íÎó ***");
                        
                        log("3user","ignore %s failed", uident);
                    }
                }
            }
        }
}

void call_alias(chatcontext *pthis,const char *arg)             /* added by Luzi 1998.01.25 */
{
    char buf[128],buf2[200];
    FILE *fp;
    char path[40];
    
    char emoteid[40];
    int  nIdx;
    
    
    nextword(&arg,emoteid,sizeof(emoteid));
    
    sethomefile( path, currentuser.userid , "/emotes");
    if(!emoteid[0])
    {
        if((fp=fopen(path,"r"))==NULL)
        {
            printchatline(pthis,"*** »¹Ã»ÓÐ×Ô¶¨ÒåµÄemote ***");
            return;
        }
        nIdx=0;
        chat_clear(pthis);
        printchatline(pthis,"¡¼ÓÃ»§×Ô¶¨ÒåemoteÁÐ±í¡½");
        while(fread(buf, 128, 1,fp)>-0 )
        {
            printchatline(pthis,buf);
            nIdx++;
            if (nIdx%(screen_lines)==0)
            {
            	if(!chat_waitkey(pthis))break;
            }
        }
        fclose(fp);
    } else {
        nIdx=0;
        if((fp=fopen(path,"r"))!=NULL)
        {
            while(1)
            {
                const char * tmpbuf;
                char tmpemote[40];
                
                if (fread(buf, 128, 1,fp)==0){ nIdx=0;break; }
                tmpbuf=buf;
                nextword(&tmpbuf,tmpemote,sizeof(tmpemote));
                nIdx++;
                if (strcasecmp(emoteid,tmpemote)==0) break;
            }
            fclose(fp);
        }
        if (nIdx > 0)
        {
            if (*arg)
            {
                printchatline(pthis,"*** ¸ÃemoteÒÑ¾­±»¶¨Òå¹ýÁË ***");
                return;
            }
            if (delete_record( path, 128, nIdx)==0)
            {
                printchatline(pthis,"*** ¸Ã×Ô¶¨ÒåemoteÒÑ¾­±»É¾³ýÁË ***");
                
                sprintf(buf,"/alias_del %s",emoteid);
                chat_send(pthis,buf);
            } else {
                log("3user","delete alias %s fail",emoteid);
                printchatline(pthis,"*** system error ***");
            }
        }
        else if (!*arg)
            printchatline(pthis,"*** ÇëÖ¸¶¨emote¶ÔÓ¦µÄ×Ö´® ***");
        else
        {
            fp=fopen(path,"r");
            if (fp!=NULL)
            {
                fseek(fp,0,SEEK_END);
                if (ftell(fp)>=128*MAX_EMOTES)
                {
                    fclose(fp);
                    printchatline(pthis,"*** ÓÃ»§×Ô¶¨ÒåemoteµÄÁÐ±íÒÑÂú ***");
                    return;
                }
                fclose(fp);
            }
            sprintf(buf,"%s %s",emoteid,arg);
            if (append_record( path, buf, 128)==0)
            {
                printchatline(pthis,"*** ×Ô¶¨ÒåemoteÒÑ¾­Éè¶¨ ***");
                sprintf(buf2,"/alias_add %s",buf);
                chat_send(pthis,buf2);
            } else {
                log("3user","add alias %s fail",emoteid);
                printchatline(pthis,"*** ÏµÍ³´íÎó ***");
            }
        }
    }
}

void call_mail(chatcontext *pthis,const char *arg) /* added by Luzi, 1997/12/22 */
{
    extern char currmaildir[ STRLEN ] ;
    fileheader mailheader;
    FILE *fpin;
    char b2[ STRLEN ] ;
    char *t ;
    
    if (chkmail()==0) /* check mail */
    {
        printchatline(pthis,"\033[32m*** Ã»ÓÐÐÂµÄÐÅ¼þ ***\033[m");
        return;
    }
    setmailfile(currmaildir, currentuser.userid, DOT_DIR);
    fpin = fopen(currmaildir, "rb");
    if (fpin == NULL) return;
    printchatline(pthis,"\033[32m*** µ±Ç°ÐÂµÄÐÅ¼þÈçÏÂ ***\033[m");
    while(fread(&mailheader, sizeof (fileheader), 1, fpin))
    {
        if ((mailheader.accessed[0] & FILE_READ) == 0)
        {
            strncpy(b2,mailheader.owner,STRLEN) ;
            if( (t = strchr(b2,' ')) != NULL )  *t = '\0' ;
            sprintf(genbuf,"\033[31m %-20.20s ¡ï %.46s \033[m",b2, mailheader.title);
            printchatline(pthis,genbuf);
        }
    }
    fclose(fpin);
}

void build_ver(chatcontext *pthis,const char *arg)
{
    printchatline(pthis," µ±Ç°°æ±¾±àÒëÊ±¼ä: [32m" __DATE__ " " __TIME__ "[m");
}

/* 
 2001/5/6  modified by wwj
 ÁÄÌìÊÒ¿´msgµÄº¯Êý 
 */
void chat_show_allmsgs(chatcontext *pthis,const char *arg)
{
    char fname[STRLEN];
    FILE *fp;
    char buf[400];
    int  line,cnt,i;
    long pos;
    
    line=atoi(arg);
    
    if(line<screen_lines-1)line=screen_lines-1;
    if(line>300)line=300;

    setuserfile(fname,"msgfile");
    if( (fp=fopen(fname,"rb")) !=NULL )
    {
        fseek(fp,0,SEEK_END);
        pos=ftell(fp);
        
        cnt=0;
        while(cnt<line){
        	if(pos<sizeof(buf)){/* begin of file */
        		fseek(fp,0,SEEK_SET);
        		break;
            }
            pos-=sizeof(buf);
            fseek(fp,pos,SEEK_SET);
            i=fread(buf,1,sizeof(buf),fp);
            while(i>0){
            	i--;
                if(buf[i]=='\n')cnt++;
                if(cnt==line){
                    fseek(fp,pos+i+1,SEEK_SET);
                    break;
                }
            }
        }
        
        sprintf(buf,"***** ×î½ü %d ÌõÏûÏ¢ *****",line);
        printchatline(pthis,buf);
        
        line=0;
        while (!feof(fp)) {
            bzero(buf,sizeof(buf));
            fgets(buf,sizeof(buf),fp);
            if(!buf[0])break;
            cnt=strlen(buf)-1;
            buf[cnt]=0;  /* delete \n */
            printchatline(pthis,buf);

            line++;
            if(line%(screen_lines)==0){
            	if(!chat_waitkey(pthis))break;
            }
        }
        fclose(fp);
    } else {
        printchatline(pthis,"***** Ã»ÓÐÈÎºÎµÄÑ¶Ï¢´æÔÚ£¡£¡*****");
    }
}


struct chat_command chat_cmdtbl[] = {
    {"pager",setpager,1},
    {"help", chat_help,1},
    {"clear", (void (*) (chatcontext *,const char *) )chat_clear,1},
    {"date", chat_date,1},
    {"g", chat_friends,1},
    {"send", chat_sendmsg,1},       /* by alex , 96.9.5 */
    {"seemsg",chat_show_allmsgs,1}, /* added by snow at 1998.10.24 */
    {"set",set_rec,1},
    {"call",call_user,1},
    {"query",call_query,1},         /* modify by Luzi 1997.11.18 */
    {"qc", call_query_ByChatid, 2}, /* added by dong 1998.9.12 */
    {"x",call_kickoff,1},
    {"ignore",call_ignore,6}, /* added by Luzi 1997.11.28 */
    {"listen",call_listen,6}, /* added by Luzi 1997.11.28 */
    {"mail",call_mail,4},     /* added by Luzi 1997.12.22 */
    {"alias",call_alias,5},   /* added by Luzi 1998.01.25 */
    {"version",build_ver,1},  /* added by period 2000.10.29 */
    {NULL, NULL,1}
};

int chat_cmd_match(const char *buf,const char * str,short nLenth)
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

int chat_cmd(chatcontext *pthis,char *buf)
{
    int i,j;

    if (*buf++ != '/') return 0;

    for (i = 0; chat_cmdtbl[i].cmdname; i++)
    {
        if (chat_cmd_match(buf, chat_cmdtbl[i].cmdname, chat_cmdtbl[i].nCmdLenth)) break;
    }
    if(!chat_cmdtbl[i].cmdname)return 0;
    
    for(j=0;buf[j];j++){
        if(buf[j]=='\t' || buf[j]=='\r' || buf[j]=='\n'){
            buf[j]=' ';
        }
    }
    while(*buf && *buf!=' ')buf++;
    while(*buf && *buf==' ')buf++; /* chop head */
    
    /* chop tail */
    j=strlen(buf)-1;
    while(j>0 && buf[j]==' ')j--;
    buf[j+1]=0;
    
    chat_cmdtbl[i].cmdfunc(pthis,buf);
    return 1;
}


