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
    
    2002/08/05 checked global variable.
    2001/5/6 modified by wwj
*/
#include "bbs.h"
#ifdef lint
#include <sys/uio.h>
#endif                          /* 
                                 */
/********************snow*/
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef SOLARIS
#include <sys/filio.h>
#endif

#include "chat.h"
extern char MsgDesUid[14];      /* ±£¥ÊÀ˘∑¢msgµƒƒøµƒuid 1998.7.5 by dong */
extern char page_requestor[];
extern char *modestring();
extern struct UTMPFILE *utmpshm;
extern void t_pager();

#define s_lines         (t_lines-2)
#define b_lines         (t_lines-1)
#define screen_lines    (t_lines-4)
int chat_cmd(chatcontext * pthis, char *buf);
void set_rec();
struct user_info *t_search();

#define CHAT_LOGIN_OK       "OK"
#define CHAT_LOGIN_EXISTS   "EX"
#define CHAT_LOGIN_INVALID  "IN"
#define CHAT_LOGIN_BOGUS    "BG"
static const char *msg_seperator =
    "°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™°™";
static const char *msg_shortulist =
    "\033[33m\033[44m  π”√’ﬂ¥˙∫≈    ƒø«∞◊¥Ã¨  ©¶  π”√’ﬂ¥˙∫≈    ƒø«∞◊¥Ã¨  ©¶  π”√’ﬂ¥˙∫≈    ƒø«∞◊¥Ã¨ \033[m";
static int ent_chat(int chatnum);       /* Ω¯»Î¡ƒÃÏ “ */
int chat_waitkey(chatcontext * pthis)
{
    char ch;

    outs("           [5;31m°Ù ∞¥ø’∏Òº¸ºÃ–¯£¨Qº¸»°œ˚ °Ù[m");
    add_io(0, 0);
    ch = igetkey();
    add_io(pthis->cfd, 0);
    return strchr(" \r\n", ch) != NULL;
}
static void printchatline(chatcontext * pthis, const char *str)
{                               /*œ‘ æ“ª––£¨≤¢œ¬“∆÷∏ æ∑˚ */
    char tmpstr[256];
    const char *p;
    int i;
    int len;                    /* add by KCN for disable long line */
    int inesc;

    if (pthis->outputcount < 0)
        return;                 /* by wwj 2001/5/9 */
    p = str;
    i = 0;
    len = 0;
    inesc = 0;
    while (*p != 0) {
        if (len >= 79) {
            tmpstr[strlen(tmpstr) - (len - 79)] = 0;
            break;
        }
        if (*p == '%') {
            if (*(p + 1) == 0) {
                tmpstr[i] = '%';
                i++;
                p++;
                len++;
            } else if (*(p + 1) == '%') {
                tmpstr[i] = '%';
                i++;
                p++;
                p++;
                len++;
            } else if (*(p + 1) > '0' && *(p + 1) <= '7') {
                tmpstr[i++] = '\033';
                tmpstr[i++] = '[';
                tmpstr[i++] = '3';
                tmpstr[i++] = *(p + 1);
                tmpstr[i++] = 'm';
                p++;
                p++;
            } else if (*(p + 1) == '0') {
                tmpstr[i++] = '\033';
                tmpstr[i++] = '[';
                tmpstr[i++] = '0';
                tmpstr[i++] = 'm';
                p++;
                p++;
            } else {
                tmpstr[i] = '%';
                i++;
                p++;
                tmpstr[i] = *p;
                i++;
                p++;
                len += 2;
            }
        } else {
            if (inesc) {
                if (isalpha(*p))
                    inesc = 0;
                /* ¿¡µ√¥¶¿Ì¡Ω∏ˆESCµƒ«Èøˆ¡À KCN */
            } else {
                if (*p == '\033')
                    inesc = 1;
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
    if (pthis->outputcount++ == screen_lines - 1) {
        if (!chat_waitkey(pthis))
            pthis->outputcount = -1;
        else
            pthis->outputcount = 1;
        move(pthis->chatline, 0);
        clrtoeol();
    }
    outs(tmpstr);               /* snow change at 10.25 */
    outc('\n');
    if (pthis->rec)
        fprintf(pthis->rec, "%s\n", str);
    if (++pthis->chatline == s_lines)
        pthis->chatline = 2;
    move(pthis->chatline, 0);
    clrtoeol();
    outs("==>");
}

void chat_clear(chatcontext * pthis)
{                               /* clear chat */
    for (pthis->chatline = 2; pthis->chatline < s_lines; pthis->chatline++) {
        move(pthis->chatline, 0);
        clrtoeol();
    }
    pthis->chatline = s_lines - 1;
    printchatline(pthis, "");
    pthis->outputcount = 0;
}

void print_chatid(chatcontext * pthis)
{
    move(b_lines, 0);
    outs(pthis->chatid);
    outc(':');
}
int chat_send(chatcontext * pthis, const char *buf)
{
    int len;
    char gbuf[256];

    for (len = 0; len < (int) sizeof(gbuf) - 1 && buf[len]; len++)
        gbuf[len] = buf[len];
    gbuf[len++] = '\n';
    return (send(pthis->cfd, gbuf, len, 0) == len);     /* Õ®π˝socket send */
}

/* buffered recv  2001/5/6 -- wwj */
int chat_recv(chatcontext * pthis, char *buf, int sz)
{
    int len;

    do {
        if (buf && pthis->bufptr) {     /* »Áπ˚ª∫≥Â«¯¿Ô√Ê”–£¨œ»∑µªÿ∏¯user */
            len = sz;
            if (len > pthis->bufptr)
                len = pthis->bufptr;
            memcpy(buf, pthis->buf, len);
            if (len < pthis->bufptr) {
                memcpy(pthis->buf, &pthis->buf[len], pthis->bufptr - len);
            }
            pthis->bufptr -= len;
            return len;
        }
        len =
            recv(pthis->cfd, &pthis->buf[pthis->bufptr],
                 sizeof(pthis->buf) - pthis->bufptr, 0);
        if (len <= 0)
            return -1;
        pthis->bufptr += len;
    } while (buf);
    return 0;
}

int chat_parse(chatcontext * pthis)
{
    int len;
    char *bptr;

    if (!pthis->bufptr || pthis->buf[pthis->bufptr - 1] != 0) {
        if (chat_recv(pthis, NULL, 0) < 0)
            return -1;
    }
    len = 0;
    bptr = pthis->buf;
    while (bptr < pthis->buf + pthis->bufptr) {
        for (len = 0; bptr + len < pthis->buf + pthis->bufptr; len++) {
            if (!bptr[len]) {
                len = -len;
                break;
            }
        }
        if (len > 0)
            break;
        len = -len + 1;         /* skip 0 */
        if (*bptr == '/') {     /* ¥¶¿Ìserver¥´¿¥µƒ√¸¡Ó */
            switch (bptr[1]) {
            case 'p':
                /* add by KCN for list long emote */
                /* chat_waitkey(pthis);  removed by wwj */
                break;
            case 'c':
                chat_clear(pthis);
                break;
            case 'n':
                strncpy(pthis->chatid, bptr + 2, 8);
                pthis->chatid[8] = 0;
                print_chatid(pthis);
                clrtoeol();
                uinfo.in_chat = true;
                strcpy(uinfo.chatid, pthis->chatid);
                UPDATE_UTMP_STR(chatid, uinfo);
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
                if (bptr[1] == 't')
                    strcpy(pthis->topic, bptr + 2);
                prints
                    ("[44m[33m ∑øº‰£∫ [36m%-10s  [33mª∞Ã‚£∫[36m%-51s[31m%2s[m",
                     pthis->chatroom, pthis->topic,
                     (pthis->rec) ? "¬º" : "  ");
                break;
            }
        } else {
            printchatline(pthis, bptr);
        }
        bptr += len;
        len = 0;
    }
    if (len > 0) {
        memcpy(pthis->buf, bptr, pthis->bufptr - (bptr - pthis->buf));
        pthis->bufptr -= bptr - pthis->buf;
    } else {
        pthis->bufptr = 0;
    }
    return 0;
}

int chat_checkparse(chatcontext * pthis)
{
    long cnt;
    int result = 0;

    cnt = 0;
    if (pthis->bufptr
        || ((result = ioctl(pthis->cfd, FIONREAD, &cnt)) == 0 && cnt)) {
        do {
            if (chat_parse(pthis) == -1)
                return 0;
        } while (pthis->bufptr);        /* read all data from server */
    }
    if (result != 0)
        return 0;
    return 1;
}

/* KCN add chat 2 */
int ent_chat2()
{                               /* Ω¯»Î¡ƒÃÏ “ */
    return ent_chat(2);
}

int ent_chat1()
{                               /* Ω¯»Î¡ƒÃÏ “ */
    return ent_chat(1);
}

/* 2001/5/6 --wwj, –ﬁ∏ƒ ent_chat ∫Ø ˝ */
int ent_chat_conn(chatcontext * pthis, int chatnum)
{
    struct sockaddr_in sin;
    int ch;
    char inbuf[128], *ptr;

    memset(&sin, 0, sizeof sin);
    sin.sin_family = PF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
/*
    sin.sin_addr.s_addr = inet_addr("166.111.8.237");
*/
    if (chatnum == 1)
        sin.sin_port = htons(CHATPORT3);
    else
        sin.sin_port = htons(CHATPORT2);
    pthis->cfd = socket(sin.sin_family, SOCK_STREAM, 0);
    if (connect(pthis->cfd, (struct sockaddr *) &sin, sizeof sin)) {    /*»Áπ˚¡¨Ω” chatd  ß∞‹£¨‘Ú∆Ù∂Øchatd */
        close(pthis->cfd);
        switch (ch = fork()) {
        case -1 /*fork failure */ :
            bbslog("3chatd", "fork error");
            break;
        case 0 /*fork success */ :
            bbslog("chatd", "fork success");
            prints("ø™∆Ù¡ƒÃÏ “...");
            if (chatnum == 1)
                system("bin/chatd");
            else
                system("bin/chatd 2");
            exit(1);
        default:
            bbslog("chatd", "fork par-proc");
            /* The chat daemon forks so we can wait on it here. */
            waitpid(ch, NULL, 0);
        }
        pthis->cfd = socket(sin.sin_family, SOCK_STREAM, 0);
        if ((connect(pthis->cfd, (struct sockaddr *) &sin, sizeof sin))) {
            close(pthis->cfd);
            bbslog("3chatd", "connect2 failed %d", errno);
            return -1;
        }
    }
    /* Leeward 98.04.26 */
    move(3, 0);
    clrtoeol();
    move(4, 0);                 /* Leave line 3 for error message while entering chat room */
    prints
        (" ‰»Î◊÷∑˚ [1m[37m*[m ‘Ÿ∞¥ [1m[37mEnter[m ø…“‘»°œ˚Ω¯»Î¡ƒÃÏ “°£                                    ");
    clrtoeol();
    move(5, 0);
    clrtoeol();                 /* Clear line 5 for good looking */
    while (1) {
        getdata(2, 0, "«Î ‰»Î¡ƒÃÏ¥˙∫≈£∫", inbuf, 9, DOECHO, NULL, true);
        if ('*' == inbuf[0]) {  /* Leeward 98.04.26 */
            close(pthis->cfd);
            return 0;
        }
        if (inbuf[0] != '\0' && inbuf[0] != '\n' && inbuf[0] != '/') {
            strncpy(pthis->chatid, inbuf, 8);
        } else {
            strncpy(pthis->chatid, currentuser->userid, 8);
        }
        pthis->chatid[8] = '\0';
        sprintf(inbuf, "/! %d %d %s %s", uinfo.uid, currentuser->userlevel,
                currentuser->userid, pthis->chatid);
        chat_send(pthis, inbuf);        /* send user info to chatd , and chatd will check it */
        if (chat_recv(pthis, inbuf, 3) != 3) {
            close(pthis->cfd);
            return 0;
        }
        if (!strcmp(inbuf, CHAT_LOGIN_OK))
            break;
        else if (!strcmp(inbuf, CHAT_LOGIN_EXISTS))
            ptr = "’‚∏ˆ¥˙∫≈“—æ≠”–»À”√¡À";
        else if (!strcmp(inbuf, CHAT_LOGIN_INVALID))
            ptr = "’‚∏ˆ¥˙∫≈ «¥ÌŒÛµƒ";
        else
            ptr =
                "“—æ≠”–“ª∏ˆ¥∞ø⁄‘⁄¡ƒÃÏ “¿Ô¡À£®»Ù∑«»Á¥À«ÎÕÀ≥ˆBBS÷ÿ–¬µ«¬º£ª»Ùªπ≤ª––‘Ÿ’“‘⁄œﬂ’æ≥§£©";
        move(3, 0);
        prints(ptr);
        clrtoeol();
        bell();
    }
    return 1;
}
static int ent_chat(int chatnum)
{                               /* Ω¯»Î¡ƒÃÏ “ */
    chatcontext *pthis;
    char inbuf[128];
    int ch, cmdpos;
    int currchar;
    int modified;               /* the line is modified? -- wwj */
    int newmail;
    int page_pending = false;
    int chatting = true;

    if (!strcmp(currentuser->userid, "guest"))
        return -1;
    pthis = (chatcontext *) malloc(sizeof(chatcontext));
    bzero(pthis, sizeof(chatcontext));
    if (!pthis)
        return -1;
    modify_user_mode(CHAT1);
    ch = ent_chat_conn(pthis, chatnum);
    if (ch != 1) {
        free(pthis);
        return ch;
    }
    add_io(pthis->cfd, 0);
    modified = newmail = cmdpos = currchar = 0;
    /* update uinfo */
    uinfo.in_chat = true;
    strcpy(uinfo.chatid, pthis->chatid);
    UPDATE_UTMP(in_chat, uinfo);
    UPDATE_UTMP_STR(chatid, uinfo);
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
    while (chatting) {
        if (chat_checkparse(pthis) == 0)
            break;
        move(b_lines, currchar + 10);
        pthis->outputcount = 0;
        ch = igetkey();
        if (ch==KEY_TALK) {
            int talkpage = servicepage(0, pthis->buf);

            if (talkpage != page_pending) {
            	bell();
            	oflush();
                printchatline(pthis, pthis->buf);
                page_pending = talkpage;
            }
        }
        if (chat_checkparse(pthis) == 0)
            break;
        if (ch == I_OTHERDATA)
            continue;
        switch (ch) {
        case KEY_UP:
        case KEY_DOWN:
            if (cmdpos == pthis->cmdpos) {
                strcpy(pthis->lastcmd[cmdpos], inbuf);
                modified = 0;
            }
            if (ch == KEY_UP) {
                if (cmdpos != (pthis->cmdpos + 1) % MAXLASTCMD) {
                    int i = (cmdpos + MAXLASTCMD - 1) % MAXLASTCMD;

                    if (pthis->lastcmd[i][0])
                        cmdpos = i;
                }
            }
            if (ch == KEY_DOWN) {
                if (cmdpos != pthis->cmdpos)
                    cmdpos = (cmdpos + 1) % MAXLASTCMD;
            }
            strcpy(inbuf, pthis->lastcmd[cmdpos]);
            if (cmdpos == pthis->cmdpos) {
                modified = 1;
            }
            move(b_lines, 10);
            clrtoeol();
            ch = inbuf[69];
            inbuf[69] = 0;
            outs(inbuf);
            inbuf[69] = ch;
            currchar = strlen(inbuf);
            continue;
#ifdef CHINESE_CHARACTER
        case Ctrl('R'):
			currentuser->userdefine = currentuser->userdefine ^ DEF_CHCHAR;
        	continue;
#endif        	
        case KEY_LEFT:
            if (currchar)
                --currchar;
#ifdef CHINESE_CHARACTER
			if (DEFINE(currentuser, DEF_CHCHAR)) {
				int i,j=0;
				for(i=0;i<currchar;i++)
					if(j) j=0;
					else if(inbuf[i]<0) j=1;
				if(j) {
					currchar--;
				}
			}
#endif
            continue;
        case KEY_RIGHT:
            if (inbuf[currchar])
                ++currchar;
#ifdef CHINESE_CHARACTER
			if (DEFINE(currentuser, DEF_CHCHAR)) {
				int i,j=0;
				for(i=0;i<currchar;i++)
					if(j) j=0;
					else if(inbuf[i]<0) j=1;
				if(j) {
		            if (inbuf[currchar])
        		        ++currchar;
				}
			}
#endif
            continue;
        case KEY_ESC:
        case Ctrl('X'):
            inbuf[0] = 0;
            currchar = 0;
            move(b_lines, currchar + 10);
            clrtoeol();
            modified = 1;
            continue;
        case Ctrl('A'):
            currchar = 0;
            continue;
        case Ctrl('E'):
            currchar = strlen(inbuf);
            continue;
        }
        if (!newmail && chkmail(0)) {   /* check mail */
            newmail = 1;
            printchatline(pthis, "[32m*** [31mµ±£°ƒ„”––¬–≈¿¥¿≤...[m");
        }
#ifdef BIT8
        if (isprint2(ch))
#else                           /* 
                                 */
        if (isprint(ch))
#endif                          /* 
                                 */
        {
            if (currchar < 126) {       /* Œ¥¬˙“ª––,print it */
                modified = 1;
                if (inbuf[currchar]) {  /* insert */
                    int i;

                    for (i = currchar; inbuf[i] && i < 127; i++);
                    inbuf[i + 1] = '\0';
                    for (; i > currchar; i--)
                        inbuf[i] = inbuf[i - 1];
                } else {        /* append */
                    inbuf[currchar + 1] = '\0';
                }
                inbuf[currchar] = ch;
                ch = inbuf[69]; /* save the end of line */
                inbuf[69] = 0;
                move(b_lines, currchar + 10);
                outs(&inbuf[currchar++]);
                inbuf[69] = ch;
            }
            continue;
        }
        if (ch == '\n' || ch == '\r') {
            if (currchar) {
                if (modified) {
                    /* add to command history */
                    ch = sizeof(pthis->lastcmd[pthis->cmdpos]) - 1;
                    strncpy(pthis->lastcmd[pthis->cmdpos], inbuf, ch);
                    pthis->lastcmd[pthis->cmdpos][ch] = 0;
                    pthis->cmdpos = (pthis->cmdpos + 1) % MAXLASTCMD;
                    cmdpos = pthis->cmdpos;
                } else {        /* use history, so can +1 */
                    cmdpos = (cmdpos + 1) % MAXLASTCMD;
                }
                if (inbuf[0] == '/' && Isspace(inbuf[1])) {     /* discard / b */
                    printchatline(pthis,
                                  "\x1b[37m*** \x1b[32m«Î ‰»Î’˝»∑µƒ÷∏¡Ó£¨ π”√/h—∞«Û∞Ô÷˙\x1b[37m ***\x1b[m");
                } else {
                    chatting = chat_cmd(pthis, inbuf);  /*local√¸¡Ó¥¶¿Ì */
                    if (chatting == 0)
                        chatting = chat_send(pthis, inbuf);
                    if (inbuf[0] == '/') {
                        ch = 1;
                        while (inbuf[ch] != '\0' && inbuf[ch] != ' ')
                            ch++;
                        if (ch > 1) {
                            if (!strncasecmp(inbuf, "/bye", ch))
                                break;
                            if (!strncasecmp(inbuf, "/exit", ch))
                                break;  /*added by alex, 96.9.5 */
                        }
                    }
                }
                modified = 0;
                inbuf[0] = '\0';
                currchar = 0;
                move(b_lines, 10);
                clrtoeol();
            }
            continue;
        }
        if (ch == Ctrl('H') || ch == '\177') {  /*Backspace */
            if (currchar) {
                currchar--;
                inbuf[127] = '\0';
                memcpy(&inbuf[currchar], &inbuf[currchar + 1],
                       127 - currchar);
                move(b_lines, currchar + 10);
                clrtoeol();
                ch = inbuf[69]; /* save the end of line */
                inbuf[69] = 0;
                outs(&inbuf[currchar]);
                inbuf[69] = ch;
            }
#ifdef CHINESE_CHARACTER
			if (DEFINE(currentuser, DEF_CHCHAR)) {
				int i,j=0;
				for(i=0;i<currchar;i++)
					if(j) j=0;
					else if(inbuf[i]<0) j=1;
				if(j) {
	                currchar--;
	                inbuf[127] = '\0';
	                memcpy(&inbuf[currchar], &inbuf[currchar + 1],
	                       127 - currchar);
	                move(b_lines, currchar + 10);
	                clrtoeol();
	                ch = inbuf[69]; /* save the end of line */
	                inbuf[69] = 0;
	                outs(&inbuf[currchar]);
	                inbuf[69] = ch;
				}
			}
#endif
            continue;
        }
        if (ch == Ctrl('Z')) {
            r_lastmsg();        /* Leeward 98.07.30 support msgX */
            inbuf[0] = '\0';
            currchar = 0;
            move(b_lines, 10);
            clrtoeol();
            continue;
        }
        if (ch == Ctrl('C') /*|| ch == Ctrl('D') */ ) { /* ^C ÕÀ≥ˆ */
            chat_send(pthis, "/b");
            if (pthis->rec)
                set_rec();
            break;
        }
    }
    /* chat end */
    if(pthis->rec)set_rec(pthis,NULL);
    close(pthis->cfd);
    add_io(0, 0);
    uinfo.in_chat = false;
    uinfo.chatid[0] = '\0';
    UPDATE_UTMP(in_chat, uinfo);
    UPDATE_UTMP(chatid[0], uinfo);
    clear();
    free(pthis);
    return 0;
}

/*
int printuserent(chatcontext * pthis, struct user_info *uentp)
{
    static char uline[256];
    static int cnt;
    char pline[50];

    if (!uentp) {
        if (cnt)
            printchatline(pthis, uline);
        bzero(uline, 256);
        cnt = 0;
        return 0;
    }
    if (!uentp->active || !uentp->pid)
        return 0;
    if (!HAS_PERM(currentuser, PERM_SEECLOAK) && uentp->invisible)
        return 0;
#if 0
    if (kill(uentp->pid, 0) == -1)
        return 0;
#endif
    sprintf(pline, " %s%-13s[m%c%-10s",
            myfriend(uentp->uid, NULL) ? "[32m" : "", uentp->userid,
            uentp->invisible ? '#' : ' ', modestring(uentp->mode,
                                                     uentp->destuid, 0,
                                                     NULL));
    if (cnt < 2)
        strcat(pline, "©¶");
    strcat(uline, pline);
    if (++cnt == 3) {
        printchatline(pthis, uline);
        memset(uline, 0, 256);
        cnt = 0;
    }
    return 0;
}
*/
int print_friend_ent(struct user_info *uentp, chatcontext * pthis, int pos)
{                               /* print one user & status if he is a friend */
    char pline[50];

    if (!uentp->active || !uentp->pid)
        return 0;
    if (!HAS_PERM(currentuser, PERM_SEECLOAK) && uentp->invisible)
        return 0;
#if 0
    if (kill(uentp->pid, 0) == -1)
        return 0;
#endif                          /* 
                                 */
    if (!myfriend(uentp->uid, NULL))
        return 0;
    sprintf(pline, " %-13s%c%-10s", uentp->userid,
            uentp->invisible ? '#' : ' ', modestring(uentp->mode,
                                                     uentp->destuid, 0,
                                                     NULL));
    if (pthis->apply_count < 2)
        strcat(pline, "©¶");
    strcat(pthis->apply_buf, pline);
    if (pthis->apply_count == 2) {
        printchatline(pthis, pthis->apply_buf);
        memset(pthis->apply_buf, 0, 256);
        pthis->apply_count = 0;
    } else
        pthis->apply_count++;
    return COUNT;
}
void chat_help(chatcontext * pthis, const char *arg)
{
    char buf[256];
    FILE *fp;

    if (strstr(arg, "op")) {    /* op's help */
        if ((fp = fopen("help/chatophelp", "r")) == NULL)
            return;
        while (fgets(buf, 255, fp) != NULL) {
            printchatline(pthis, buf);
        }
        fclose(fp);
    } else {
        if ((fp = fopen("help/chathelp", "r")) == NULL)
            /* user's help */
            return;
        while (fgets(buf, 255, fp) != NULL) {
            char *ptr;

            ptr = strstr(buf, "\n");
            *ptr = '\0';
            printchatline(pthis, buf);
        }
        fclose(fp);
    }
}
void call_user(chatcontext * pthis, const char *arg)
{                               /* invite user online to chatroom */
    char msg[STRLEN * 2];
    struct user_info *uin;
    int state;

    if (!*arg) {
        printchatline(pthis,
                      "[37m*** [32m«Î ‰»Îƒ„“™—˚«Îµƒ ID[37m ***[m");
        return;
    }
    if (!strcasecmp(arg, currentuser->userid))
        sprintf(msg, "[32mƒ„≤ª”√—˚«Î◊‘º∫∞°[m");
    else if (!HAS_PERM(currentuser, PERM_PAGE))
        /* Leeward 98.07.30 */
        sprintf(msg, "[32mƒ„√ª”–∑¢–≈œ¢µƒ»®œﬁ[m");
    else {
        uin = t_search(arg, false);
        if (uin == NULL)
            sprintf(msg, "[32m%s[37m ≤¢√ª”–…œ’æ[m", arg);
        else if (LOCKSCREEN == uin->mode)
            /* Leeward 98.02.28 */
            sprintf(msg,
                    "[32m%s[37m“—æ≠À¯∂®∆¡ƒª£¨«Î…‘∫Ú‘Ÿ—˚«ÎªÚ∏¯À˚(À˝)–¥–≈.\n",
                    uin->userid);
        else if (!canmsg(currentuser, uin))
            sprintf(msg, "%s “—æ≠πÿ±’Ω” ‹—∂œ¢µƒ∫ÙΩ–∆˜.\n", uin->userid);
        else {
            FILE *fp;
            int msg_count = 0;
            char buf[STRLEN];

            sethomefile(buf, uin->userid, "msgcount");
            fp = fopen(buf, "rb");
            if (fp != NULL) {
                fread(&msg_count, sizeof(int), 1, fp);
                fclose(fp);
            }
            if (msg_count > MAXMESSAGE)
                sprintf(msg,
                        "∂‘∑Ω…–”–“ª–©—∂œ¢Œ¥¥¶¿Ì£¨«Î…‘∫Ú‘Ÿ—˚«ÎªÚ∏¯À˚(À˝)–¥–≈...\n");
            else {
                sprintf(msg, "µΩ¡ƒÃÏπ„≥°µƒ %s ¡ƒÃÏ “ ¡ƒ¡ƒÃÏ",
                        pthis->chatroom);
                /* ±£¥ÊÀ˘∑¢msgµƒƒøµƒuid 1998.7.5 by dong */
                strcpy(MsgDesUid, uin->userid);
                state = do_sendmsg(uin, msg, 1);
                if (state == 1)
                    sprintf(msg, "[37m“—æ≠∞Ôƒ„—˚«Î %s ¡À[m",
                            uin->userid);
                else if (state == -2)
                    sprintf(msg, "[37m∂‘∑Ω“—æ≠¿Îœﬂ¡À...[m");
                else
                    sprintf(msg, "[37∂‘∑Ω≤ªΩ” ‹ƒ„µƒœ˚œ¢...[m");
            }
        }
    }
    printchatline(pthis, msg);
}
void chat_sendmsg(chatcontext * pthis, const char *arg)
{                               /* send msg in chatroom , by alex, 96.9.5 */
    char userid[IDLEN + 1];
    char msg[STRLEN * 2];
    struct user_info *uin;
    int state;

    nextword(&arg, userid, sizeof(userid));
    if (!userid[0]) {
        printchatline(pthis,
                      "\x1b[37m*** \x1b[32m«Î ‰»Îƒ„“™∑¢œ˚œ¢µƒ ID\x1b[37m ***\x1b[m");
        return;
    } else if (!*arg) {
        printchatline(pthis,
                      "\x1b[37m*** \x1b[32m«Î ‰»Îƒ„“™∑¢µƒœ˚œ¢\x1b[37m ***\x1b[m");
        return;
    } else if (!HAS_PERM(currentuser, PERM_PAGE)) {     /* Leeward 98.07.30 */
        sprintf(msg, "\x1b[32mƒ„√ª”–∑¢–≈œ¢µƒ»®œﬁ\x1b[m");
    } else {
        uin = t_search(userid, false);
        if (uin == NULL) {
            sprintf(msg, "\x1b[32m%s\x1b[37m ≤¢√ª”–…œ’æ\x1b[m", userid);
        } else if (LOCKSCREEN == uin->mode)
            /* Leeward 98.02.28 */
            sprintf(msg,
                    "\x1b[32m%s\x1b[37m“—æ≠À¯∂®∆¡ƒª£¨«Î…‘∫Ú‘Ÿ∑¢ªÚ∏¯À˚(À˝)–¥–≈.\n",
                    uin->userid);
        else {
            if (!canmsg(currentuser, uin))
                sprintf(msg,
                        "\x1b[32m%s\x1b[37m“—æ≠πÿ±’Ω” ‹—∂œ¢µƒ∫ÙΩ–∆˜.\n",
                        uin->userid);
            else if (false == canIsend2(currentuser,uin->userid))
                sprintf(msg, "\x1b[32m%s\x1b[37mæ‹æ¯Ω” ‹ƒ„µƒ—∂œ¢.\n", uin->userid);     /*Haohmaru.99.6.6,ºÏ≤È «∑Ò±ªignore */
            else {
                FILE *fp;
                int msg_count = 0;
                char buf[STRLEN];

                sethomefile(buf, uin->userid, "msgcount");
                fp = fopen(buf, "rb");
                if (fp != NULL) {
                    fread(&msg_count, sizeof(int), 1, fp);
                    fclose(fp);
                }
                if (msg_count > MAXMESSAGE)
                    sprintf(msg,
                            "∂‘∑Ω…–”–“ª–©—∂œ¢Œ¥¥¶¿Ì£¨«Î…‘∫Ú‘Ÿ∑¢ªÚ∏¯À˚(À˝)–¥–≈...\n");
                else {
                    /* ±£¥ÊÀ˘∑¢msgµƒƒøµƒuid 1998.7.5 by dong */
                    strcpy(MsgDesUid, uin->userid);
                    state = do_sendmsg(uin, arg, 2);
                    if (state == 1)
                        sprintf(msg,
                                "\x1b[37m“—æ≠∑¢œ˚œ¢∏¯ \x1b[32m%s\x1b[37m ¡À\x1b[m",
                                uin->userid);
                    else if (state == -2)
                        sprintf(msg, "\x1b[37m∂‘∑Ω“—æ≠¿Îœﬂ¡À...\x1b[m");
                    else
                        sprintf(msg, "[37∂‘∑Ω≤ªΩ” ‹ƒ„µƒœ˚œ¢...[m");
                }
            }
        }
    }
    printchatline(pthis, msg);
}
int c_cmpuids(int uid, struct user_info *up)
{
    return (uid == up->uid);
}
int chat_status(struct user_info *uentp, chatcontext * pthis)
{
    char tmpstr[31];
    char *lpTmp;

    if (strlen(genbuf)>t_columns) 
    	return QUIT;
    if (uentp->invisible == 1) {
        if (HAS_PERM(currentuser, PERM_SEECLOAK)) {
            sprintf(genbuf + strlen(genbuf), "\x1b[32m#\x1b[m");
        } else
            return 0;
    }
    lpTmp = (char *) idle_str(uentp);
    if (uentp->in_chat) {       /* add by Luzi 1997.11.18 */
        int res;

        sprintf(tmpstr, "/q %s", uentp->userid);
        chat_send(pthis, tmpstr);
        res = chat_recv(pthis, tmpstr, 30);
        if (res <= 0)
            return -1;
        tmpstr[res] = '\0';
        if (tmpstr[0] == '1') {
            sprintf(genbuf + strlen(genbuf), "'%s' room as '%s'",
                    tmpstr + 1, uentp->chatid);
            if (lpTmp[0] != ' ')
                sprintf(genbuf + strlen(genbuf), "[%s];", lpTmp);
            else
                strcat(genbuf, " ;");
            return COUNT;
        }
    }
    sprintf(genbuf, "%s%-8s", genbuf, modestring(uentp->mode, uentp->destuid, 0,        /* 1->0 ≤ªœ‘ æ¡ƒÃÏ∂‘œÛµ» modified by dong 1996.10.26 */
                                                 (uentp->in_chat ? uentp->
                                                  chatid : NULL)));
    if (lpTmp[0] != ' ')
        sprintf(genbuf + strlen(genbuf), "[%s];", lpTmp);
    else
        strcat(genbuf, " ;");
    return COUNT;
}
static void query_user(chatcontext * pthis, const char *userid)
{
    int tuid = 0;
    char qry_mail_dir[STRLEN], inbuf[STRLEN * 2];
    char *newline;
    time_t exit_time, temp;
    struct userec *lookupuser;

    if (!(tuid = getuser(userid, &lookupuser))) {
        printchatline(pthis, "[32m’‚∏ˆID≤ª¥Ê‘⁄£°[m");
        return;
    }
    setmailfile(qry_mail_dir, lookupuser->userid, DOT_DIR);
    /*---	modified by period	2000-11-02	hide posts/logins	---*/
#ifdef _DETAIL_UINFO_
    sprintf(genbuf, "%s (%s):      %s", lookupuser->userid,
            lookupuser->username,
            (check_query_mail(qry_mail_dir) == 1) ? "”––¬–≈" : "    ");
    printchatline(pthis, genbuf);
    sprintf(genbuf, "π≤…œ’æ %d ¥Œ£¨∑¢±Ìπ˝ %d ∆™Œƒ’¬£¨…˙√¸¡¶[%d]%s",
            lookupuser->numlogins, lookupuser->numposts,
            compute_user_value(lookupuser),
            (lookupuser->userlevel & PERM_SUICIDE) ? " (◊‘…±÷–)" : " ");
    printchatline(pthis, genbuf);
#else                           /*
                                 */
    sprintf(genbuf, "%s (%s):   …˙√¸¡¶[%d]%s   %s",
            lookupuser->userid, lookupuser->username,
            compute_user_value(lookupuser),
            (lookupuser->userlevel & PERM_SUICIDE) ? " (◊‘…±÷–)" : " ",
            (check_query_mail(qry_mail_dir) == 1) ? "”––¬–≈" : "    ");
    printchatline(pthis, genbuf);
#endif  /*_DETAIL_UINFO_*/
    strcpy(inbuf, ctime(&(lookupuser->lastlogin)));
    if ((newline = strchr(genbuf, '\n')) != NULL)
        *newline = '\0';
    strcpy(genbuf, "µ±«∞◊¥Ã¨£∫");
    if (apply_utmpuid((APPLY_UTMP_FUNC) chat_status, tuid, (char *) pthis)) {
        char buf[1024];

	lookupuser->lasthost[IPLEN-1] = '\0';
        sprintf(buf, "ƒø«∞’˝‘⁄œﬂ…œ: ¿¥◊‘ %s …œœﬂ ±º‰ %s" /*\n" */ ,
                (lookupuser->lasthost[0] == '\0' /* || DEFINE(currentuser,DEF_HIDEIP) */ ? "(≤ªœÍ)" : lookupuser->lasthost), inbuf);    /*Haohmaru.99.12.18 */
        printchatline(pthis, buf);
        printchatline(pthis, genbuf);
    } else {
        lookupuser->lasthost[IPLEN-1] = '\0';  
        sprintf(genbuf, "…œ¥Œ…œœﬂ¿¥◊‘  %s  ±º‰Œ™ %s " /*\n" */ ,
                (lookupuser->lasthost[0] == '\0' /* || DEFINE(currentuser,DEF_HIDEIP) */ ? "(≤ªœÍ)" : lookupuser->lasthost), inbuf);    /* Haohmaru.99.12.18 */
        printchatline(pthis, genbuf);
        /* ªÒµ√¿Îœﬂ ±º‰ Luzi 1998/10/23 */
        exit_time = get_exit_time(lookupuser->userid, genbuf);
        if ((newline = strchr(genbuf, '\n')) != NULL)
            *newline = '\0';
        if (exit_time > lookupuser->lastlogin)
            strcpy(inbuf, genbuf);
        /*Haohmaru.98.12.04.∫Õ≤Àµ•≤È—ØΩ·π˚“ª÷¬ */
        if (exit_time <= lookupuser->lastlogin)
            /*
               || (uin.active && uin.pid
               && (!uin.invisible || (uin.invisible && HAS_PERM(currentuser,PERM_SEECLOAK)))))
             */
            strcpy(inbuf, "“Ú‘⁄œﬂ…œªÚ∑«≥£∂œœﬂ≤ªœÍ");
        if (exit_time <= lookupuser->lastlogin) {       /* && (uin.invisible&& !HAS_PERM(currentuser,PERM_SEECLOAK))) */
            temp = lookupuser->lastlogin + (lookupuser->numlogins % 7) + 5;
            strcpy(inbuf, ctime(&temp));        /*Haohmaru.98.12.04.»√“˛…Ì”√ªßø¥…œ»•¿Îœﬂ ±º‰±»…œœﬂ ±º‰ÕÌ5√Î÷” */
            if ((newline = strchr(inbuf, '\n')) != NULL)
                *newline = '\0';
        }
        /*       else strcpy(inbuf,"[“Ú∑«≥£∂œœﬂ≤ªœÍ]"); */
        sprintf(genbuf, "¿Îœﬂ ±º‰Œ™ %s " /*\n" */ , inbuf);
        printchatline(pthis, genbuf);
    }
#ifdef DEBUG
    if (HAS_PERM(currentuser, PERM_SYSOP)) {
        sprintf(genbuf, "%d", tuid);
        printchatline(pthis, genbuf);
    }
#endif                          /* 
                                 */
}
void call_query(chatcontext * pthis, const char *arg)
                                                            /* by alex, 1996.9.5 */
 /* by Luzi, 1997.11.30 */
{
    if (!*arg) {
        printchatline(pthis, "*** «Î ‰»Î“™≤È—ØµƒID ***");
    } else {
        query_user(pthis, arg);
    }
}
void call_query_ByChatid(chatcontext * pthis, const char *arg)
{                               /* add by dong, 1998.9.12 */
    char uident[32];
    char tmpstr[40];
    int res;

    if (!*arg) {
        printchatline(pthis, "*** «Î ‰»Î“™≤È—Øµƒchat ID ***");
        return;
    }
    strncpy(uident, arg, 32);
    uident[31] = '\0';
    /* get user id from the chat id */
    sprintf(tmpstr, "/qc %s", uident);
    chat_send(pthis, tmpstr);
    res = chat_recv(pthis, tmpstr, 40);
    if (res <= 0)
        return;
    tmpstr[res] = '\0';
    if (tmpstr[0] == '1') {
        sprintf(uident, "%s", tmpstr + 1);
    } else {
        sprintf(genbuf, "[32m’‚∏ˆchat ID≤ª¥Ê‘⁄£°[m");
        printchatline(pthis, genbuf);
        return;
    }
    query_user(pthis, uident);
}
void chat_date(chatcontext * pthis, const char *arg)
{
    time_t thetime;

    time(&thetime);
    sprintf(genbuf, " %s±Í◊º ±º‰: [32m%s[m", BBS_FULL_NAME,
            Cdate(thetime));
    printchatline(pthis, genbuf);
}
void chat_friends(chatcontext * pthis, const char *arg)
{
    /* printchatline(pthis,"");   moved by wwj */
    int num, i;
    struct user_info* u;

    pthis->apply_count = 0;
    sprintf(genbuf, "[1m°æ µ±«∞œﬂ…œµƒ∫√”—¡–±Ì °ø[m");
    printchatline(pthis, genbuf);
    printchatline(pthis, msg_shortulist);
    pthis->apply_buf = genbuf;
    pthis->apply_buf[0] = 0;
    num = 0;
    u=get_utmpent(utmpent);
    for (i = 0; i < u->friendsnum; i++) {
        num +=
            apply_utmpuid((APPLY_UTMP_FUNC) print_friend_ent,
                          u->friends_uid[i], (char *) pthis);
    }
    if (pthis->apply_count)
        printchatline(pthis, pthis->apply_buf);
    if (num == 0) {
        printchatline(pthis, "[1m√ª”–≈Û”—‘⁄œﬂ…œ[m");
    }
}
void set_rec(chatcontext * pthis, const char *arg)
{                               /* set recorder */
    char fname[STRLEN];
    time_t now;

    now = time(0);
    /*        if(!HAS_PERM(currentuser,PERM_SYSOP))
       return; */

    /*sprintf(fname, "tmp/%s.chat", currentuser->userid);*/
    sethomefile(fname,currentuser->userid,"chatrec");

    if (!pthis->rec) {
        if ((pthis->rec = fopen(fname, "a")) == NULL)
            return;
        printchatline(pthis, "[5m[32mRecord Start ...[m");
        move(0, 0);
        clrtoeol();
        prints
            ("[44m[33m ∑øº‰£∫ [36m%-10s  [33mª∞Ã‚£∫[36m%-51s[31m%2s[m",
             pthis->chatroom, pthis->topic, (pthis->rec) ? "¬º" : "  ");
        fprintf(pthis->rec,
                "∑¢–≈»À: %s (%s) ∑øº‰: %s\nª∞  Ã‚: %s\x1b[m\n\n",
                currentuser->userid, currentuser->username,
                pthis->chatroom, pthis->topic);
        fprintf(pthis->rec, "±æ∂Œ”… %s", currentuser->userid);
        fprintf(pthis->rec, "À˘¬ºœ¬£¨ ±º‰£∫ %s", ctime(&now));
        bbslog("user", "start record room %s", pthis->chatroom);
    } else {
        move(0, 0);
        clrtoeol();
        prints
            ("[44m[33m ∑øº‰£∫ [36m%-10s  [33mª∞Ã‚£∫[36m%-51s[31m%2s[m",
             pthis->chatroom, pthis->topic, (pthis->rec) ? "¬º" : "  ");
        fprintf(pthis->rec,
                "∑¢–≈»À: %s (%s) ∑øº‰: %s\nª∞  Ã‚: %s\x1b[m\n\n",
                currentuser->userid, currentuser->username,
                pthis->chatroom, pthis->topic);
        printchatline(pthis, "[5m[32mRecord Stop ...[m");
        fprintf(pthis->rec, "Ω· ¯ ±º‰£∫%s\n", ctime(&now));
        fclose(pthis->rec);
        pthis->rec = NULL;
        mail_file(currentuser->userid, fname, currentuser->userid,
                  "¬º“ÙΩ·π˚", 1, NULL);
	my_unlink(fname);
        bbslog("user", "stop record room %s", pthis->chatroom);
    }
}
void setpager(chatcontext * pthis, const char *arg)
{
    char buf[STRLEN];

    t_pager();                  /*toggle pager */
    sprintf(buf, "[32m*** [31mƒ„ %s ¡À∫ÙΩ–∆˜[m",
            (uinfo.pager & ALL_PAGER) ? "¥Úø™" : "πÿ±’");
    printchatline(pthis, buf);
}
void call_kickoff(chatcontext * pthis, const char *arg)
{                               /* kick ID off BBS, by Luzi, 97.11.1 */
    char msg[STRLEN];
    struct user_info *uin;

    if (!HAS_PERM(currentuser, PERM_SYSOP)) {
        printchatline(pthis, "*** ƒ„≤ª «’æ≥§ ***");
        return;
    }
    if (!*arg) {
        printchatline(pthis, "*** «Î ‰»Îƒ„“™Ãﬂœ¬’æµƒ ID ***");
        return;
    } else if (!strcasecmp(arg, currentuser->userid))
        sprintf(msg, "*** Faint!ƒ„“™∞—◊‘º∫Ãﬂœ¬’æ∞°£ø***");
    else {
        uin = t_search(arg, false);
        if (uin == NULL) {
            sprintf(msg, "%s ≤¢√ª”–…œ’æ", arg);
        } else {
            bbslog("user", "kick %s out of bbs", uin->userid);
            kill(uin->pid, SIGHUP);
            return;
        }
    }
    printchatline(pthis, msg);
}
void call_listen(chatcontext * pthis, const char *arg)
{                               /* added by Luzi 1997.11.28 */
    char path[40];
    char uident[IDLEN + 1];
    char ignoreuser[IDLEN + 1];
    int nIdx;

    if (!*arg) {
        printchatline(pthis, "*** «Î ‰»Î”√ªßµƒID ***");
    } else if (!strcasecmp(arg, currentuser->userid))
        printchatline(pthis, "*** ’‚ «ƒ„◊‘º∫µƒID ***");
    else {
        strncpy(uident, arg, IDLEN + 1);
        uident[IDLEN] = 0;
        if (!searchuser(uident))
            /* change getuser -> searchuser, by dong, 1999.10.26 */
            printchatline(pthis, "*** √ª”–’‚∏ˆID ***");
        else {
            sethomefile(path, currentuser->userid, "/ignores");
            nIdx =
                search_record(path, ignoreuser, IDLEN + 1,
                              (RECORD_FUNC_ARG) cmpinames, uident);
            if (nIdx <= 0)
                printchatline(pthis,
                              "*** ∏√”√ªßµƒ¡ƒÃÏ—∂œ¢√ª”–±ª∫ˆ¬‘∞° ***");
            else if (delete_record(path, IDLEN + 1, nIdx, NULL, NULL) == 0) {
                bbslog("user", "listen %s", uident);
                printchatline(pthis,
                              "*** “—ª÷∏¥∂‘∏√”√ªß¡ƒÃÏ—∂œ¢µƒΩ” ’ ***");
                sprintf(uident, "/listen %s\n", uident);
                chat_send(pthis, uident);
            } else {
                bbslog("3error", "listen %s failed", uident);
                printchatline(pthis, "*** system error ***");
            }
        }
    }
}
void call_ignore(chatcontext * pthis, const char *arg)
{                               /* added by Luzi 1997.11.28 */
    char buf[STRLEN], buf2[76];
    FILE *fp;
    char uident[IDLEN + 1];
    char path[40];
    char ignoreuser[IDLEN + 1];
    int nIdx;

    sethomefile(path, currentuser->userid, "/ignores");
    if (!*arg) {
        nIdx = 0;
        if ((fp = fopen(path, "r")) != NULL) {
            strcpy(buf2, "°æ∫ˆ¬‘∆‰—∂œ¢µƒ”√ªßID¡–±Ì°ø");
            while (fread(buf, IDLEN + 1, 1, fp) > -0) {
                if (nIdx % 4 == 0) {
                    printchatline(pthis, buf2);
                    *buf2 = '\0';
                }
                nIdx++;
                sprintf(buf2 + strlen(buf2), "  %-13s", buf);
            }
            fclose(fp);
        }
        if (nIdx > 0)
            printchatline(pthis, buf2);
        else
            printchatline(pthis, "*** …–Œ¥…Ë∂®∫ˆ¬‘”√ªßµƒ√˚µ• ***");
    } else if (!strcasecmp(arg, currentuser->userid))
        printchatline(pthis, "*** Œﬁ∑®∫ˆ¬‘◊‘º∫µƒ–≈œ¢ ***");
    else {
        strncpy(uident, arg, IDLEN + 1);
        uident[IDLEN] = 0;
        if (!searchuser(uident))
            /* change getuser -> searchuser, by dong, 1999.10.26 */
            printchatline(pthis, "*** √ª”–’‚∏ˆ ID ***");
        else {
            nIdx =
                search_record(path, ignoreuser, IDLEN + 1,
                              (RECORD_FUNC_ARG) cmpinames, uident);
            if (nIdx > 0)
                printchatline(pthis, "*** ∏√ID“—æ≠±ª∫ˆ¬‘¡À ***");
            else {
                fp = fopen(path, "r");
                if (fp != NULL) {
                    fseek(fp, 0, SEEK_END);
                    if (ftell(fp) >= (IDLEN + 1) * MAX_IGNORE) {
                        fclose(fp);
                        printchatline(pthis, "*** ∫ˆ¬‘”√ªß√˚µ•“—¬˙ ***");
                        return;
                    }
                    fclose(fp);
                }
                if (append_record(path, uident, IDLEN + 1) == 0) {
                    printchatline(pthis, "*** ∫ˆ¬‘“—æ≠…Ë∂® ***");
                    bbslog("user", "ignore %s", uident);
                    sprintf(buf, "/ignore %s", uident);
                    chat_send(pthis, buf);
                } else {
                    printchatline(pthis, "*** œµÕ≥¥ÌŒÛ ***");
                    bbslog("3error", "ignore %s failed", uident);
                }
            }
        }
    }
}
void call_alias(chatcontext * pthis, const char *arg)
{                               /* added by Luzi 1998.01.25 */
    char buf[128], buf2[200];
    FILE *fp;
    char path[40];
    char emoteid[40];
    int nIdx;

    nextword(&arg, emoteid, sizeof(emoteid));
    sethomefile(path, currentuser->userid, "/emotes");
    if (!emoteid[0]) {
        if ((fp = fopen(path, "r")) == NULL) {
            printchatline(pthis, "*** ªπ√ª”–◊‘∂®“Âµƒemote ***");
            return;
        }
        nIdx = 0;
        printchatline(pthis, "°º”√ªß◊‘∂®“Âemote¡–±Ì°Ω");
        while (fread(buf, 128, 1, fp) > 0) {
            printchatline(pthis, buf);
        }
        fclose(fp);
    } else {
        nIdx = 0;
        if ((fp = fopen(path, "r")) != NULL) {
            while (1) {
                const char *tmpbuf;
                char tmpemote[40];

                if (fread(buf, 128, 1, fp) == 0) {
                    nIdx = 0;
                    break;
                }
                tmpbuf = buf;
                nextword(&tmpbuf, tmpemote, sizeof(tmpemote));
                nIdx++;
                if (strcasecmp(emoteid, tmpemote) == 0)
                    break;
            }
            fclose(fp);
        }
        if (nIdx > 0) {
            if (*arg) {
                printchatline(pthis, "*** ∏√emote“—æ≠±ª∂®“Âπ˝¡À ***");
                return;
            }
            if (delete_record(path, 128, nIdx, NULL, NULL) == 0) {
                printchatline(pthis, "*** ∏√◊‘∂®“Âemote“—æ≠±ª…æ≥˝¡À ***");
                sprintf(buf, "/alias_del %s", emoteid);
                chat_send(pthis, buf);
            } else {
                bbslog("3error", "delete alias %s fail", emoteid);
                printchatline(pthis, "*** system error ***");
            }
        } else if (!*arg)
            printchatline(pthis, "*** «Î÷∏∂®emote∂‘”¶µƒ◊÷¥Æ ***");
        else {
            fp = fopen(path, "r");
            if (fp != NULL) {
                fseek(fp, 0, SEEK_END);
                if (ftell(fp) >= 128 * MAX_EMOTES) {
                    fclose(fp);
                    printchatline(pthis,
                                  "*** ”√ªß◊‘∂®“Âemoteµƒ¡–±Ì“—¬˙ ***");
                    return;
                }
                fclose(fp);
            }
            sprintf(buf, "%s %s", emoteid, arg);
            if (append_record(path, buf, 128) == 0) {
                printchatline(pthis, "*** ◊‘∂®“Âemote“—æ≠…Ë∂® ***");
                sprintf(buf2, "/alias_add %s", buf);
                chat_send(pthis, buf2);
            } else {
                bbslog("3error", "add alias %s fail", emoteid);
                printchatline(pthis, "*** œµÕ≥¥ÌŒÛ ***");
            }
        }
    }
}
void call_mail(chatcontext * pthis, const char *arg)
{                               /* added by Luzi, 1997/12/22 */
    extern char currdirect[255];
    fileheader mailheader;
    FILE *fpin;
    char b2[STRLEN];
    char *t;

    if (chkmail() == 0) {       /* check mail */
        printchatline(pthis, "\033[32m*** √ª”––¬µƒ–≈º˛ ***\033[m");
        return;
    }
    setmailfile(currdirect, currentuser->userid, DOT_DIR);
    fpin = fopen(currdirect, "rb");
    if (fpin == NULL)
        return;
    printchatline(pthis, "\033[32m°æµ±«∞–¬µƒ–≈º˛»Áœ¬°ø\033[m");
    while (fread(&mailheader, sizeof(fileheader), 1, fpin)) {
        if ((mailheader.accessed[0] & FILE_READ) == 0) {
            strcpy(b2, mailheader.owner);
            if ((t = strchr(b2, ' ')) != NULL)
                *t = '\0';
            sprintf(genbuf, "\033[31m %-20.20s °Ô %.46s \033[m", b2,
                    mailheader.title);
            printchatline(pthis, genbuf);
        }
    }
    fclose(fpin);
}

/* 
 2001/5/6  modified by wwj
 ¡ƒÃÏ “ø¥msgµƒ∫Ø ˝ 
 */
void chat_show_allmsgs(chatcontext * pthis, const char *arg)
{
    char fname[STRLEN];
    FILE *fp;
    char buf[MAX_MSG_SIZE], showmsg[MAX_MSG_SIZE*2];
    int line, cnt, i, j=0, count;
    long pos;
    struct msghead head;

    line = atoi(arg);
    if (line < screen_lines - 1)
        line = screen_lines - 1;
    if (line > 300)
        line = 300;
	gettmpfilename( fname, "chatmsg" );
    //sprintf(fname, "tmp/%s.msg", currentuser->userid);
    fp = fopen(fname, "w");
    count = get_msgcount(0, currentuser->userid);
    for(i=0;i<count;i++) 
    if(i>=count-line)
    {
        j++;
        load_msghead(0, currentuser->userid, i, &head);
        load_msgtext(currentuser->userid, &head, buf);
        translate_msg(buf, &head, showmsg);
        fprintf(fp, "%s", showmsg);
    }
    fclose(fp);
    if (count) {
        fp = fopen(fname, "rb");
        sprintf(buf, "°æ◊ÓΩ¸ %d Ãıœ˚œ¢°ø", j);
        printchatline(pthis, buf);
        while (!feof(fp)) {
            bzero(buf, sizeof(buf));
            fgets(buf, sizeof(buf), fp);
            if (!buf[0])
                break;
            cnt = strlen(buf) - 1;
            buf[cnt] = 0;       /* delete \n */
            printchatline(pthis, buf);
        }
        fclose(fp);
    } else {
        printchatline(pthis, "***** √ª”–»Œ∫Œµƒ—∂œ¢¥Ê‘⁄£°£°*****");
    }
    unlink(fname);
}
static const struct chat_command chat_cmdtbl[] = {
    {"pager", setpager, 1},
    {"help", chat_help, 1},
    {"clear", (void (*)(chatcontext *, const char *)) chat_clear, 1},
    {"date", chat_date, 1},
    {"g", chat_friends, 1},
    {"send", chat_sendmsg, 1},  /* by alex , 96.9.5 */
    {"seemsg", chat_show_allmsgs, 1},   /* added by snow at 1998.10.24 */
    {"set", set_rec, 1},
    {"call", call_user, 1},
    {"query", call_query, 1},   /* modify by Luzi 1997.11.18 */
    {"qc", call_query_ByChatid, 2},     /* added by dong 1998.9.12 */
    {"x", call_kickoff, 1},
    {"ignore", call_ignore, 6}, /* added by Luzi 1997.11.28 */
    {"listen", call_listen, 6}, /* added by Luzi 1997.11.28 */
    {"mail", call_mail, 4},     /* added by Luzi 1997.12.22 */
/*    {"alias",call_alias,5},    added by Luzi 1998.01.25 */
    {NULL, NULL, 1}
};
int chat_cmd_match(const char *buf, const char *str, short nLenth)
{
    short i = 0;

    while (*str && *buf && !isspace(*buf)) {
        if (tolower(*buf++) != *str++)
            return 0;
        i++;
    }
    return (i >= nLenth);
}
int chat_cmd(chatcontext * pthis, char *buf)
{
    int i, j;

    if (*buf++ != '/')
        return 0;
    for (i = 0; chat_cmdtbl[i].cmdname; i++) {
        if (chat_cmd_match
            (buf, chat_cmdtbl[i].cmdname, chat_cmdtbl[i].nCmdLenth))
            break;
    }
    if (!chat_cmdtbl[i].cmdname)
        return 0;
    for (j = 0; buf[j]; j++) {
        if (buf[j] == '\t' || buf[j] == '\r' || buf[j] == '\n') {
            buf[j] = ' ';
        }
    }
    while (*buf && *buf != ' ')
        buf++;
    while (*buf && *buf == ' ')
        buf++;                  /* chop head */
    /* chop tail */
    j = strlen(buf) - 1;
    while (j > 0 && buf[j] == ' ')
        j--;
    buf[j + 1] = 0;
    chat_cmdtbl[i].cmdfunc(pthis, buf);
    return 1;
}
