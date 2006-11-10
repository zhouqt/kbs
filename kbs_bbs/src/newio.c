/*
底层的I/O库。
		KCN重写
*/

#include "bbs.h"
#include <arpa/telnet.h>

#ifdef AIX
#include <sys/select.h>
#endif

/*输入输出缓冲区的大小
  输出缓冲区给个半屏大小足以，
  输入缓冲区一般都利用不上，给
  个128字节吧
*/
#define OBUFSIZE  (1024*2)
#define IBUFSIZE  (128)

#define INPUT_ACTIVE 0
#define INPUT_IDLE 1

#ifdef SSHBBS
#include "ssh_funcs.h"
#endif /* SSHBBS */

extern int temp_numposts;

char outbuffer[OBUFSIZE + 1];
char *outbuf = outbuffer + 1;
int obufsize = 0;

char inbuffer[IBUFSIZE + 1];
char *inbuf = inbuffer + 1;
int ibufsize = 0;

int icurrchar = 0;
int KEY_ESC_arg;

int idle_count = 0;

static time_t old;
static time_t lasttime = 0;

extern int convcode;
extern int ssh_sock;
void oflush()
{
    if (obufsize) {
        if (convcode) {
            char *out;

            out = gb2big(outbuf, &obufsize, 0, getSession());
#ifdef SSHBBS
            if (ssh_write(0, out, obufsize) < 0)
#else
            if (write(0, out, obufsize) < 0)
#endif
                abort_bbs(0);
        } else
#ifdef SSHBBS
        if (ssh_write(0, outbuf, obufsize) < 0)
#else
        if (write(0, outbuf, obufsize) < 0)
#endif
            abort_bbs(0);
    }
    obufsize = 0;
}

void ochar(char c)
{
    if (obufsize > OBUFSIZE - 1) {      /* doin a oflush */
        oflush();
    }
    outbuf[obufsize++] = c;
    /*
     * need to change IAC to IAC IAC 
     */
    if (((unsigned char) c) == IAC) {
        if (obufsize > OBUFSIZE - 1) {  /* doin a oflush */
            oflush();
        }
        outbuf[obufsize++] = c;
    }
}

int raw_write(int fd,const char *buf, int len)
{
    static int lastcounter = 0;
    int nowcounter;
    static int bufcounter;
#ifndef SSHBBS
    int i, retlen=0;
#endif

#ifdef ZMODEM_RATE
        nowcounter = time(0);
        if (lastcounter == nowcounter) {
            if (bufcounter >= ZMODEM_RATE) {
                sleep(1);
                nowcounter = time(0);
                bufcounter = len;
            } else
                bufcounter += len;
        } else {
            /*
             * time clocked, clear bufcounter 
             */
            bufcounter = len;
        }
        lastcounter = nowcounter;
#endif    
#ifdef SSHBBS
    return ssh_write(fd, buf, len);
#else
    for (i = 0; i < len; i++) {
        int mylen;

        if ((unsigned char) buf[i] == 0xff)
            mylen = write(fd, "\xff\xff", 2);
        else if (buf[i] == 13)
            mylen = write(fd, "\x0d\x00", 2);
        else
            mylen = write(fd, &buf[i], 1);
        if (mylen < 0)
            break;
        retlen += mylen;
    }
    return retlen;
#endif
}

void raw_ochar(char c)
{
    raw_write(0, &c, 1);
}

int raw_read(int fd, char *buf, int len)
{
#ifdef SSHBBS
    return ssh_read(fd, buf, len);
#else
    int i,j,retlen=0;
    retlen = read(fd,buf,len);
    for(i=0;i<retlen;i++) {
        if(i>0&&((unsigned char)buf[i-1]==0xff)&&((unsigned char)buf[i]==0xff)) {
            retlen--;
            for(j=i;j<retlen;j++)
                buf[j]=buf[j+1];
            continue;
        }
        if(i>0&&buf[i-1]==0x0d&&buf[i]==0x00) {
            retlen--;
            for(j=i;j<retlen;j++)
                buf[j]=buf[j+1];
            continue;
        }
    }
    return retlen;
#endif
}

void output(const char *s, int len)
{
    /*
     * need to change IAC to IAC IAC
     * if(obufsize+len > OBUFSIZE) {
     * #ifdef SSHBBS
     * ssh_write(0,outbuf,obufsize) ;
     * #else
     * write(0,outbuf,obufsize) ;
     * #endif
     * obufsize = 0 ;
     * }
     * memcpy(outbuf+obufsize, s, len) ;
     * obufsize+=len ; 
     */
    int i;

    while(obufsize+len>OBUFSIZE - 1) {
        i=OBUFSIZE-obufsize;
        memcpy(outbuf+obufsize, s, i);
        obufsize += i;
        s+=i;
        len-=i;
        oflush();
    }
    i=len;
    memcpy(outbuf+obufsize, s, i);
    obufsize += i;
}


int i_newfd = 0;
static void (*flushf) () = NULL;

int i_timeout = 0;
static int i_timeoutusec = 0;
static time_t i_begintimeout;
static void (*i_timeout_func) (void *);
static struct timeval i_to, *i_top = NULL;
static void *timeout_data;

void add_io(int fd, int timeout)
{
    i_newfd = fd;
    if (timeout) {
        i_to.tv_sec = timeout;
        i_to.tv_usec = 0;
        i_top = &i_to;
    } else
        i_top = NULL;
}

void add_flush(void (*flushfunc) ())
{
    flushf = flushfunc;
}

void set_alarm(int set_timeout, int set_timeoutusec,void (*timeout_func) (void *), void *data)
{
    i_timeout = set_timeout;
    i_timeoutusec=set_timeoutusec;
    i_begintimeout = time(0);
    i_timeout_func = timeout_func;
    timeout_data = data;
}

int num_in_buf()
{
    return icurrchar - ibufsize ; 
}

int telnet_state = 0;
char lastch;
int naw_col, naw_ln, naw_changed=0;

static int telnet_machine(unsigned char ch)
{
    switch (telnet_state) {
    case 255:                  /* after the first IAC */
        switch (ch) {
        case DO:
        case DONT:
        case WILL:
        case WONT:
            telnet_state = 1;
            break;
        case SB:               /* loop forever looking for the SE */
            telnet_state = 2;
            break;
        case IAC:
            return IAC;
        default:
            telnet_state = 0;   /* other telnet command */
        }
        break;
    case 1:                    /* Get the DO,DONT,WILL,WONT */
        telnet_state = 0;       /* the ch is the telnet option */
        break;
    case 2:                    /* the telnet suboption */
        if (ch == 31)
            telnet_state = 5;   /* wait for windows size */
        else if (ch == IAC)
            telnet_state = 3;   /* wait for SE */
        else
            telnet_state = 4;   /* filter telnet SB data */
        break;
    case 3:                    /* wait for se */
        if (ch == SE) {
            telnet_state = 0;
            if(naw_changed) {
                naw_changed = 0;
                do_naws(naw_ln, naw_col);
            }
        }
        else
            telnet_state = 4;
        break;
    case 4:                    /* telnet SB data */
        if (ch == IAC)
            telnet_state = 3;   /* wait for SE */
        break;
    case 5:
        naw_changed = 1;
        telnet_state = 6;
        break;
    case 6:
        naw_col = ch;
        if (ch == IAC)
            telnet_state = 4;
        else
            telnet_state = 7;
        break;
    case 7:
        if (ch == IAC)
            telnet_state = 4;
        else
            telnet_state = 8;
        break;
    case 8:
        naw_ln = ch;
        if (ch == IAC)
            telnet_state = 4;
        else
            telnet_state = 2;
        break;
    }
    return 0;
}

int filter_telnet(char *s, int *len)
{
    unsigned char *p1, *p2, *pend;
    int newlen;

    newlen = 0;
    for (p1 = (unsigned char *) s, p2 = (unsigned char *) s, pend = (unsigned char *) s + (*len); p1 != pend; p1++) {
        if (telnet_state) {
            int ch = 0;

            ch = telnet_machine(*p1);
            if (ch == IAC) {    /* 两个IAC */
                *p2 = IAC;
                p2++;
                newlen++;
            }
        } else {
            if (*p1 == IAC)
                telnet_state = 255;
            else {
                *p2 = *p1;
                p2++;
                newlen++;
            }
        }
    }
    return (*len = newlen);
}

bool inremsg = false;

struct key_struct *keymem=NULL;
int keymem_total;
int kicked=0;
int incalendar=0;

void ktimeout(void *data)
{
    kicked = 1;
}

int igetch()
{
    time_t now;
    char c;
    int hasaddio = 1;
    extern int RMSG;


    if ((uinfo.mode == CHAT1 || uinfo.mode == TALK || uinfo.mode == PAGE) && RMSG == true)
        hasaddio = 0;

  igetagain:
    if (ibufsize == icurrchar) {
        fd_set readfds, xds;
        struct timeval to;
        int sr, hifd;

        to.tv_sec = 0;
        to.tv_usec = 0;
        hifd = 1;
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        if ((hasaddio && (i_newfd))&&(!inremsg)) {
            FD_SET(i_newfd, &readfds);
            if (hifd <= i_newfd)
                hifd = i_newfd + 1;
        }
	//TODO: igetkey重入问题
        if ((uinfo.mode != POSTING && uinfo.mode != SMAIL && uinfo.mode != EDIT) || DEFINE(getCurrentUser(), DEF_LOGININFORM))
        if (scrint&&!inremsg) {
            while (msg_count) {
                inremsg = true;
                msg_count--;
                r_msg();
                refresh();
                inremsg = false;
            }
        }
        if(kicked) return KEY_TIMEOUT;
#ifdef SSHBBS
	sr = ssh_select(hifd, &readfds, NULL, NULL, &to);
#else
        sr = select(hifd, &readfds, NULL, NULL, &to);
#endif
        if (sr < 0 && errno == EINTR) {
            if (talkrequest)
                return KEY_TALK;
            if ((uinfo.mode != POSTING && uinfo.mode != SMAIL && uinfo.mode != EDIT) || DEFINE(getCurrentUser(), DEF_LOGININFORM))
            if (scrint&&!inremsg) {
                while (msg_count) {
                    inremsg = true;
                    msg_count--;
                    r_msg();
                    inremsg = false;
                }
                goto igetagain;
            }
        }
        if (sr < 0 && errno != EINTR)
            abort_bbs(0);
        if (sr == 0) {
            refresh();
            if (flushf)
                (*flushf) ();

            while (1) {
                int alarm_timeout;

                hifd = 1;
                FD_ZERO(&xds);
                FD_SET(0, &xds);
                FD_ZERO(&readfds);
                FD_SET(0, &readfds);
                if ((hasaddio && (i_newfd))&&(!inremsg)) {
                    FD_SET(i_newfd, &readfds);
                    if (hifd <= i_newfd)
                        hifd = i_newfd + 1;
                }
                alarm_timeout = 0;
                if (i_top)
                    to = *i_top;
                else {
                    while ((i_timeout != 0)||(i_timeoutusec!=0)) {
                        to.tv_sec = i_timeout - (time(0) - i_begintimeout);
                        to.tv_usec = i_timeoutusec;
                        if ((to.tv_sec < 0) ||((to.tv_sec==0)&&(i_timeoutusec==0))){
                            i_timeout = 0;
                            i_timeoutusec=0;
                            if (i_timeout_func)
                            	(*i_timeout_func) (timeout_data);
                            else
                            	return KEY_TIMEOUT;
                            if(kicked) return KEY_TIMEOUT;
                            continue;
                        };
                        alarm_timeout = 1;
                        break;
                    };
                    if (!alarm_timeout)
                        to.tv_sec = IDLE_TIMEOUT;
                }
#ifdef SSHBBS
		sr = ssh_select(hifd, &readfds, NULL, &xds, &to);
#else
                sr = select(hifd, &readfds, NULL, &xds, &to);
#endif
                if (sr < 0 && errno == EINTR) {
                    if (talkrequest)
                        return KEY_TALK;
                }
                if(kicked) return KEY_TIMEOUT;
                if ((uinfo.mode != POSTING && uinfo.mode != SMAIL && uinfo.mode != EDIT) || DEFINE(getCurrentUser(), DEF_LOGININFORM))
                if (!inremsg) {
		      int saveerrno=errno;
                    while (msg_count) {
                        inremsg = true;
                        msg_count--;
                        r_msg();
                        refresh();
                        inremsg = false;
                    }
                    if (sr<0&&saveerrno==EINTR)continue;
                }
                if (sr == 0 && alarm_timeout) {
                    i_timeout = 0;
                    i_timeoutusec=0;
                    if (i_timeout_func)
                    	(*i_timeout_func) (timeout_data);
                    else
                    	return KEY_TIMEOUT;
                    continue;
                }
                if (sr >= 0)
                    break;
                if (errno == EINTR)
                    continue;
                else
                    abort_bbs(0);

            }
            if ((sr == 0) && (!i_top))
                abort_bbs(0);
            if (sr == 0)
                return I_TIMEOUT;
            if (FD_ISSET(0, &xds))
                abort_bbs(0);
        }
        if (hasaddio && (i_newfd && FD_ISSET(i_newfd, &readfds)))
            return I_OTHERDATA;
#ifdef SSHBBS
        while ((ibufsize = ssh_read(0, inbuffer + 1, IBUFSIZE)) <= 0) {
#else
        while ((ibufsize = read(0, inbuffer + 1, IBUFSIZE)) <= 0) {
#endif
            if (ibufsize == 0)
                longjmp(byebye, -1);
            if (ibufsize < 0 && errno != EINTR)
                longjmp(byebye, -1);
        }
        if (!filter_telnet(inbuffer + 1, &ibufsize)) {
            icurrchar = 0;
            ibufsize = 0;
            goto igetagain;
        }

        /*
         * add by KCN for GB/BIG5 encode 
         */
        if (convcode) {
            inbuf = big2gb(inbuffer + 1, &ibufsize, 0, getSession());
            if (ibufsize == 0) {
                icurrchar = 0;
                goto igetagain;
            }
        } else
            inbuf = inbuffer + 1;
        /*
         * end 
         */
        icurrchar = 0;
        if (ibufsize > IBUFSIZE) {
            ibufsize = 0;
            goto igetagain;
        }
    }

    if (icurrchar >= ibufsize) {
        ibufsize = icurrchar;
        goto igetagain;
    }
    if (((inbuf[icurrchar] == '\n') && (lastch == '\r'))
        || ((inbuf[icurrchar] == '\r') && (lastch == '\n'))) {
        lastch = 0;
        goto igetagain;
    }

    else if (icurrchar != ibufsize) {
        if (((inbuf[icurrchar] == '\n') && (inbuf[icurrchar + 1] == '\r'))
            || ((inbuf[icurrchar] == '\r') && (inbuf[icurrchar + 1] == '\n'))) {
            icurrchar++;
            lastch = 0;
        }
    } else
        lastch = inbuf[icurrchar];

    idle_count = 0;
    c = inbuf[icurrchar];

    switch (c) {
    case Ctrl('@'):
    case Ctrl('L'):
        redoscr();
        icurrchar++;
    	now = time(0);
        uinfo.freshtime = now;
        if (now - old > 60) {
           UPDATE_UTMP(freshtime, uinfo);
           old = now;
        }
        goto igetagain;
    case Ctrl('Z'):
        if(scrint&&uinfo.mode!=LOCKSCREEN&&uinfo.mode!=NEW&&uinfo.mode!=LOGIN&&uinfo.mode!=BBSNET &&uinfo.mode!=WINMINE&&!inremsg) {
            icurrchar++;
            inremsg = true;
            r_msg();
            inremsg = false;
            goto igetagain;
        }
        break;
    default:
        break;
    }
    icurrchar++;
    while ((icurrchar != ibufsize) && (inbuf[icurrchar] == 0))
        icurrchar++;
    now = time(0);
    if (Ctrl('T') != c)
        uinfo.freshtime = now;
    /*
     * add by KCN , decrease temp_numposts 
     */
    if (lasttime + 60 * 60 * 8 < now) {
        lasttime = now;
        if (temp_numposts > 0)
            temp_numposts--;
    }
    if (now - old > 60) {
        UPDATE_UTMP(freshtime, uinfo);
        old = now;
    }
    return c;
}

int* keybuffer;
int keybuffer_count=0;
int skip_key=0;
static int dicting=0;
#ifdef NEW_HELP
static int f1ing=0;
#endif
int ingetdata=false;

extern void mailscr();

int igetkey()
{
    int mode;
    int ch, last, llast;
    int ret=0;

    if(keybuffer_count) {
        ret = *keybuffer;
        keybuffer++;
        keybuffer_count--;
        return ret;
    }
    mode = last = llast = 0;
//    if (ibufsize == icurrchar)
//        refresh();
    while (1) {
        ch = igetch();
        if(kicked) return KEY_TIMEOUT;

        if(check_calltime()){
			mode = 0;
			continue;
		}

#if defined(SMTH) || defined(FREE)
	if (scrint&&ch==Ctrl('V')) {
            if (getCurrentUser()&&!HAS_PERM(getCurrentUser(),PERM_DENYRELAX)&&uinfo.mode!=LOCKSCREEN&&!dicting){
				dicting=1;
	            exec_mbem("@mod:service/libdict.so#dict_main");
				dicting=0;
			}
            continue;
        }
#endif
        if (scrint&&(ch == KEY_TALK) && talkrequest) {
            if (uinfo.mode != CHAT1 && uinfo.mode != CHAT2 && uinfo.mode != CHAT3 && uinfo.mode != CHAT4 && uinfo.mode != TALK && uinfo.mode != PAGE) {
                talkreply();
                return KEY_REFRESH;
            } else
                return KEY_TALK;
        }
        if (mode == 0) {
            if (ch == KEY_ESC) {
                if(ibufsize==icurrchar) {
                    switch(uinfo.mode){
                        case POSTING:
                        case SMAIL:
                        case EDITUFILE:
                        case EDITSFILE:
                        case NOTEPAD:
                        case EDIT:
                        case EDITANN:
                        case RMAIL:
                        case CALENEDIT:
                        case CSIE_ANNOUNCE:
                        case POSTCROSS:
                            break;
                        default:
                            return KEY_ESC;
                    }
                }
                mode = 1;
            }
            else {
                ret = ch;
                break;      /* Normal Key */
            }
        } else if (mode == 1) { /* Escape sequence */
            if (ch == '[' || ch == 'O')
                mode = 2;
            else if (ch == '1' || ch == '4')
                mode = 3;
            else {
                KEY_ESC_arg = ch;
                return KEY_ESC;
            }
        } else if (mode == 2) { /* Cursor key */
            if (ch >= 'A' && ch <= 'D') {
               ret = KEY_UP + (ch - 'A');
               break;
            }
            else if (ch >= 'P' && ch <= 'S') {
                ret = KEY_F1+ch-'P';
                break;
            }
            else if (ch >= '1' && ch <= '6')
                mode = 3;
            else {
                ret = ch;
                break;
            }
        } else if (mode == 3) { /* Ins Del Home End PgUp PgDn */
            if (ch == '~') {
                ret = KEY_HOME + (last - '1');
                break;
            }
            else if (ch >= '0' && ch <= '9')
                mode = 4;
            else {
                ret = ch;
                break;
            }
        } else if (mode == 4) {
            if (ch == '~') {
                int k=(llast-'1')*10+(last-'1');
                if(k<=3) ret = KEY_F1+k;
                else ret = KEY_F1+k-1;
        	if (scrint&&ret==KEY_F10&&!incalendar) {
        	      mode=0;
                    if (getCurrentUser()&&!HAS_PERM(getCurrentUser(),PERM_DENYRELAX)&&uinfo.mode!=LOCKSCREEN)
                    exec_mbem("@mod:service/libcalendar.so#calendar_main");
                    continue;
                }
        	else if(scrint&&ret==KEY_F6) {
        	    mode=0;
        	    auto_chinese();
        	    continue;
        	}
            else if (scrint&&ret==KEY_F9){
                mode=0;
                if (getCurrentUser()&& (check_mail_perm(getCurrentUser(),NULL) == 0))
                    mailscr();
                continue;
            }

                break;
            }
            else {
                ret = ch;
                break;
            }
        }
        llast = last;
        last = ch;
    }

#ifdef NEW_HELP
	if(scrint && ret == KEY_F1 && uinfo.mode != LOCKSCREEN && !f1ing){
		int oldmode = uinfo.mode;

		modify_user_mode(HELP);
		f1ing=1;
		newhelp(helpmode);
		f1ing=0;
		modify_user_mode(oldmode);

		return igetkey();
	}
#endif

    if(scrint&&keymem_total&&!skip_key&&!ingetdata) {
        int i,j,p;
        for(i=0;i<keymem_total;i++) {
            p=!keymem[i].status[0];
            if(keymem[i].status[0]==-1) continue;
            j=0;
            while(keymem[i].status[j]&&j<10) {
                if(keymem[i].status[j]==uinfo.mode) p=1;
                j++;
            }
            if(p&&ret==keymem[i].key) {
                j=0;
                while(keymem[i].mapped[j]&&j<10) j++;
                if(j==0) continue;
                ret = keymem[i].mapped[0];
                keybuffer_count = j-1;
                keybuffer = keymem[i].mapped+1;
                break;
            }
        }
    }
    
    return ret;
}

bool enableESC=false;

/*
 * ret:  -1: user cancel input
 */
int getdata(int line, int col, char *prompt, char *buf, int len, int echo, void *nouse, int clearlabel)
{
    int ch, clen = 0, curr = 0, x, y;
    bool init=true;
    char tmp[STRLEN],save[STRLEN];
    extern int scr_cols;
    extern int RMSG;

    if (clearlabel == true) {
        buf[0] = 0;
    }
    if (scrint) move(line, col);
    if (prompt) prints("%s", prompt);
    if (scrint) getyx(&y, &x);
    clen = strlen(buf);
    if(clen>=len) clen = len - 1;
    curr = clen;
    buf[curr] = '\0';
    strncpy(save, buf, STRLEN);
    save[STRLEN-1]=0;

    if (!scrint) {
        prints("%s", buf);
        while ((ch = igetkey()) != '\r') {
            /*
             * TODO: add KEY_REFRESH support 
             */
	    if (ch == '\n')
                break;
            if (ch == '\177' || ch == Ctrl('H')) {
                if (clen == 0)
                    continue;
                clen--;
                ochar(Ctrl('H'));
                ochar(' ');
                ochar(Ctrl('H'));
                oflush();
                continue;
            }
            if (!isprint2(ch))
                continue;
            if (clen >= len - 1)
                continue;
            buf[clen++] = ch;
            if (echo) ochar(ch);
            else ochar('*');
        }
        buf[clen] = '\0';
        prints("\n");
        oflush();
        return clen;
    }
    ingetdata = true;
    clrtoeol();
    while (1) {
        int i;
        move(y, x);
        if(init) prints("\x1b[4m");
        for(i=0;i<clen;i++)
            if(!echo||buf[i]==KEY_ESC||!isprint2(buf[i])) outc('*');
            else outc(buf[i]);
        resetcolor();
        clrtoeol();
        move(y, x+curr);

        ch = igetkey();

        if(kicked) {
            ingetdata = false;
            return 0;
        }
        if (true == RMSG && (KEY_UP == ch || KEY_DOWN == ch)) {
            ingetdata = false;
            return -ch;         /* Leeward 98.07.30 supporting msgX */
        }
        if ((uinfo.mode == KILLER && (!buf[0]) && (ch==KEY_UP||ch==KEY_DOWN||ch==KEY_PGUP||ch==KEY_PGDN))||(ch>=Ctrl('S')&&ch<=Ctrl('W'))) {
            ingetdata = false;
            return -ch;
        }
#if 0 //#ifdef NINE_BUILD
	if (true == RMSG && ch == Ctrl('Z') && clen == 0) break;
#endif
        if (ch == '\n' || ch == '\r')
            break;
#ifdef CHINESE_CHARACTER
        if (ch == Ctrl('R')) {
            SET_CHANGEDEFINE(getCurrentUser(), DEF_CHCHAR);
            init=false;
            continue;
        }
#endif        	
        if (ch == '\177' || ch == Ctrl('H')) {
            if(init) {
                init=false;
                buf[0]=0;
                curr=0;
                clen=0;
            }
            if (curr == 0) {
                continue;
            }
            strcpy(tmp, &buf[curr]);
            buf[--curr] = '\0';
#ifdef CHINESE_CHARACTER
            if (DEFINE(getCurrentUser(), DEF_CHCHAR)) {
                int i,j=0;
                for(i=0;i<curr;i++)
                    if(j) j=0;
                    else if(buf[i]<0) j=1;
                if(j) {
                    buf[--curr] = '\0';
                    clen--;
                }
            }
#endif
            (void) strcat(buf, tmp);
            clen--;
            continue;
        }

        if(ch == KEY_ESC&&!enableESC) {
            strncpy(buf, save, Min(len,STRLEN));
            buf[Min(len,STRLEN-1)]=0;
            curr = strlen(buf);
            clen = curr;
            init=true;
            continue;
        }
		if(ch == Ctrl('C')){
			buf[0]='\0';
			outc('\n');
			ingetdata = false;
			return -1;
		}
        
        if (ch == KEY_DEL) {
            if(init) {
                init=false;
                buf[0]=0;
                curr=0;
                clen=0;
            }
            if (curr >= clen) {
                curr = clen;
                continue;
            }
            strcpy(tmp, &buf[curr + 1]);
#ifdef CHINESE_CHARACTER
            if (DEFINE(getCurrentUser(), DEF_CHCHAR)) {
                int i,j=0;
                for(i=0;i<curr+1;i++)
                    if(j) j=0;
                    else if(buf[i]<0) j=1;
                if(j) {
                    strcpy(tmp, &buf[curr + 2]);
                    clen--;
                }
            }
#endif
            buf[curr] = '\0';
            (void) strcat(buf, tmp);
            clen--;
            continue;
        }
        if (ch == KEY_LEFT) {
            init=false;
            if (curr == 0) {
                continue;
            }
            curr--;
#ifdef CHINESE_CHARACTER
            if (DEFINE(getCurrentUser(), DEF_CHCHAR)) {
                int i,j=0;
                for(i=0;i<curr;i++)
                    if(j) j=0;
                    else if(buf[i]<0) j=1;
                if(j) curr--;
            }
#endif
            move(y, x + curr);
            continue;
        }
        if (ch == Ctrl('E') || ch == KEY_END) {
            init=false;
            curr = clen;
            move(y, x + curr);
            continue;
        }
        if (ch == Ctrl('A') || ch == KEY_HOME) {
            init=false;
            curr = 0;
            move(y, x + curr);
            continue;
        }
        if (ch == KEY_RIGHT) {
            init=false;
            if (curr >= clen) {
                curr = clen;
                continue;
            }
            curr++;
#ifdef CHINESE_CHARACTER
            if (DEFINE(getCurrentUser(), DEF_CHCHAR)) {
                int i,j=0;
                for(i=0;i<curr;i++)
                    if(j) j=0;
                    else if(buf[i]<0) j=1;
                if(j) curr++;
            }
#endif
            move(y, x + curr);
            continue;
        }
        if (!isprint2(ch)&&(ch!=KEY_ESC||!enableESC)) {
            init=false;
            continue;
        }
        if (x + clen >= scr_cols || clen >= len - 1) {
            init=false;
            continue;
        }

        if(init) {
            init=false;
            buf[0]=0;
            curr=0;
            clen=0;
        }
        if (!buf[curr]) {
            buf[curr + 1] = '\0';
            buf[curr] = ch;
        } else {
            int i;
            for (i = len - 2; i >= curr; i--)
                buf[i + 1] = buf[i];
            buf[curr] = ch;
        }
        curr++;
        clen++;
    }
    buf[clen] = '\0';
    outc('\n');
    ingetdata = false;
    return clen;
}

bool UPDOWN=false;

int multi_getdata(int line, int col, int maxcol, char *prompt, char *buf, int len, int maxline, int clearlabel, int textmode)
{
    int ch, x, y, startx, starty, now, i, j, k, i0, chk, cursorx, cursory;
    char savebuffer[25][LINELEN*3];
    bool init=true;
    char tmp[1024];
    extern int RMSG;

	/*************stiger************
	 * textmode 0 :  ctrl+q换行，enter发表
	 * textmode 1 :  enter换行，ctrl+w发送
	 *********************************/

    if(uinfo.mode!=MSGING && uinfo.mode != POSTTMPL )
        ingetdata = true;
    if (clearlabel == true) {
        buf[0] = 0;
    }
    move(line, col);
    if (prompt)
        prints("%s", prompt);
    getyx(&starty, &startx);
    now = strlen(buf);
    for(i=0;i<=24;i++)
        saveline(i, 0, savebuffer[i]);
    strncpy(tmp, buf, 1024);
    tmp[1023]=0;
    cursory = starty;
    cursorx = startx;

    while (1) {
        y = starty; x = startx;
        move(y, x);
        chk = 0;
        if(now==0) {
            cursory = y;
            cursorx = x;
        }
        for(i=0; i<strlen(buf); i++) {
            if(chk) chk=0;
            else if(buf[i]<0) chk=1;
            if(chk&&x>=maxcol) x++;
            if(buf[i]!=13&&buf[i]!=10) {
                if(x>maxcol) {
                    clrtoeol();
                    x = 0;
                    y++;
                    if(y>=scr_lns) {
                        scroll();
                        starty--;
                        cursory--;
                        y--;
                    }
                    move(y, x);
                }
                if(init) prints("\x1b[4m");
                prints("%c", buf[i]);
                resetcolor();
                x++;
            }
            else {
                clrtoeol();
                x = 0;
                y++;
                if(y>=scr_lns) {
                    scroll();
                    starty--;
                    cursory--;
                    y--;
                }
                move(y, x);
            }
            if(i==now-1) {
                cursory = y;
                cursorx = x;
            }
        }
        clrtoeol();
        move(cursory, cursorx);
        ch = igetkey();
		if(textmode == 0){
        	if ((ch == '\n' || ch == '\r'))
            	break;
		}else{
			if (ch == Ctrl('W'))
				break;
		}
        for(i=starty;i<=y;i++)
            saveline(i, 1, savebuffer[i]);
        if (true == RMSG && (KEY_UP == ch || KEY_DOWN == ch) && (!buf[0])) {
            ingetdata = false;
            return -ch;
        }
        if (true == UPDOWN && (KEY_UP == ch || KEY_DOWN == ch)) {
            ingetdata = false;
            return -ch;
        }
#if 0 //#ifdef NINE_BUILD
        if (RMSG && (ch == Ctrl('Z')) && (!buf[0])) {
            ingetdata = false;
            return -ch;
        }
#endif
#ifdef CHINESE_CHARACTER
        if (ch == Ctrl('R')) {
            init=false;
            SET_CHANGEDEFINE(getCurrentUser(), DEF_CHCHAR);
            continue;
        }
#endif        	
        switch(ch) {
            case KEY_ESC:
                init=true;
                strncpy(buf, tmp, Min(len,STRLEN));
                buf[Min(len,1024-1)]=0;
                now=strlen(buf);
                break;
			case Ctrl('W'):
            case Ctrl('Q'):
            case '\n':
            case '\r':
                if(init) {
                    init=false;
                    buf[0]=0;
                    now=0;
                }
                if(UPDOWN) break;
                if(y-starty+1<maxline) {
                    for(i=strlen(buf)+1;i>now;i--)
                        buf[i]=buf[i-1];
                    buf[now++]='\n';
                }
                break;
            case KEY_UP:
                init=false;
                if(cursory>starty) {
                    y = starty; x = startx;
                    chk = 0;
                    if(y==cursory-1&&x<=cursorx)
                        now=0;
                    for(i=0; i<strlen(buf); i++) {
                        if(chk) chk=0;
                        else if(buf[i]<0) chk=1;
                        if(chk&&x>=maxcol) x++;
                        if(buf[i]!=13&&buf[i]!=10) {
                            if(x>maxcol) {
                                x = col;
                                y++;
                            }
                            x++;
                        }
                        else {
                            x = col;
                            y++;
                        }
#ifdef CHINESE_CHARACTER
                        if (!DEFINE(getCurrentUser(), DEF_CHCHAR)||!chk)
#endif
                        if(y==cursory-1&&x<=cursorx)
                            now=i+1;
                    }
                }
                break;
            case KEY_DOWN:
                init=false;
                if(cursory<y) {
                    y = starty; x = startx;
                    chk = 0;
                    if(y==cursory+1&&x<=cursorx)
                        now=0;
                    for(i=0; i<strlen(buf); i++) {
                        if(chk) chk=0;
                        else if(buf[i]<0) chk=1;
                        if(chk&&x>=maxcol) x++;
                        if(buf[i]!=13&&buf[i]!=10) {
                            if(x>maxcol) {
                                x = col;
                                y++;
                            }
                            x++;
                        }
                        else {
                            x = col;
                            y++;
                        }
#ifdef CHINESE_CHARACTER
                        if (!DEFINE(getCurrentUser(), DEF_CHCHAR)||!chk)
#endif
                        if(y==cursory+1&&x<=cursorx)
                            now=i+1;
                    }
                }
                break;
            case '\177':
            case Ctrl('H'):
                if(init) {
                    init=false;
                    buf[0]=0;
                    now=0;
                }
                if(now>0) {
                    for(i=now-1;i<strlen(buf);i++)
                        buf[i]=buf[i+1];
                    now--;
#ifdef CHINESE_CHARACTER
                    if (DEFINE(getCurrentUser(), DEF_CHCHAR)) {
                        chk = 0;
                        for(i=0;i<now;i++) {
                            if(chk) chk=0;
                            else if(buf[i]<0) chk=1;
                        }
                        if(chk) {
                            for(i=now-1;i<strlen(buf);i++)
                                buf[i]=buf[i+1];
                            now--;
                        }
                    }
#endif
                }
                break;
            case KEY_DEL:
                if(init) {
                    init=false;
                    buf[0]=0;
                    now=0;
                }
                if(now<strlen(buf)) {
#ifdef CHINESE_CHARACTER
                    if (DEFINE(getCurrentUser(), DEF_CHCHAR)) {
                        chk = 0;
                        for(i=0;i<now+1;i++) {
                            if(chk) chk=0;
                            else if(buf[i]<0) chk=1;
                        }
                        if(chk)
                            for(i=now;i<strlen(buf);i++)
                                buf[i]=buf[i+1];
                    }
#endif
                    for(i=now;i<strlen(buf);i++)
                        buf[i]=buf[i+1];
                }
                break;
            case KEY_LEFT:
                init=false;
                if(now>0) {
                    now--;
#ifdef CHINESE_CHARACTER
                    if (DEFINE(getCurrentUser(), DEF_CHCHAR)) {
                        chk = 0;
                        for(i=0;i<now;i++) {
                            if(chk) chk=0;
                            else if(buf[i]<0) chk=1;
                        }
                        if(chk) now--;
                    }
#endif
                }
                break;
            case KEY_RIGHT:
                init=false;
                if(now<strlen(buf)) {
                    now++;
#ifdef CHINESE_CHARACTER
                    if (DEFINE(getCurrentUser(), DEF_CHCHAR)) {
                        chk = 0;
                        for(i=0;i<now;i++) {
                            if(chk) chk=0;
                            else if(buf[i]<0) chk=1;
                        }
                        if(chk) now++;
                    }
#endif
                }
                break;
            case KEY_HOME:
            case Ctrl('A'):
                init=false;
                now--;
                while(now>=0&&buf[now]!='\n'&&buf[now]!='\r') now--;
                now++;
                break;
            case KEY_END:
            case Ctrl('E'):
                init=false;
                while(now<strlen(buf)&&buf[now]!='\n'&&buf[now]!='\r') now++;
                break;
            case KEY_PGUP:
                init=false;
                now=0;
                break;
            case KEY_PGDN:
                init=false;
                now = strlen(buf);
                break;
            case Ctrl('Y'):
                if(init) {
                    init=false;
                    buf[0]=0;
                    now=0;
                }
                i0 = strlen(buf);
                i=now-1;
                while(i>=0&&buf[i]!='\n'&&buf[i]!='\r') i--;
                i++;
                if(!buf[i]) break;
                j=now;
                while(j<i0-1&&buf[j]!='\n'&&buf[j]!='\r') j++;
                if(j>=i0-1) j=i0-1;
                j=j-i+1;
                if(j<0) j=0;
                for(k=0;k<i0-i-j+1;k++)
                    buf[i+k]=buf[i+j+k];

                y = starty; x = startx;
                chk = 0;
                if(y==cursory&&x<=cursorx)
                    now=0;
                for(i=0; i<strlen(buf); i++) {
                    if(chk) chk=0;
                    else if(buf[i]<0) chk=1;
                    if(chk&&x>=maxcol) x++;
                    if(buf[i]!=13&&buf[i]!=10) {
                        if(x>maxcol) {
                            x = col;
                            y++;
                        }
                        x++;
                    }
                    else {
                        x = col;
                        y++;
                    }
#ifdef CHINESE_CHARACTER
                    if (!DEFINE(getCurrentUser(), DEF_CHCHAR)||!chk)
#endif
                    if(y==cursory&&x<=cursorx)
                        now=i+1;
                }

                if(now>strlen(buf)) now=strlen(buf);
                break;
            default:
                if(isprint2(ch)&&strlen(buf)<len-1) {
                    if(init) {
                        init=false;
                        buf[0]=0;
                        now=0;
                    }
                    for(i=strlen(buf)+1;i>now;i--)
                        buf[i]=buf[i-1];
                    buf[now++]=ch;
                    y = starty; x = startx;
                    chk = 0;
                    for(i=0; i<strlen(buf); i++) {
                        if(chk) chk=0;
                        else if(buf[i]<0) chk=1;
                        if(chk&&x>=maxcol) x++;
                        if(buf[i]!=13&&buf[i]!=10) {
                            if(x>maxcol) {
                                x = col;
                                y++;
                            }
                            x++;
                        }
                        else {
                            x = col;
                            y++;
                        }
                    }
                    if(y-starty+1>maxline) {
                        for(i=now-1;i<strlen(buf);i++)
                            buf[i]=buf[i+1];
                        now--;
                    }
                }
                init=false;
                break;
        }
    }

    ingetdata = false;
    return y-starty+1;
}

int lock_scr(void){                             /* Leeward 98.02.22 */
    char passbuf[STRLEN];

    if (!strcmp(getCurrentUser()->userid, "guest"))
        return 1;

    modify_user_mode(LOCKSCREEN);
    clear();
    /*
     * lock_monitor(); 
     */
    while (1) {
        move(19, 32);
        clrtobot();
        prints("\033[1m\033[32m%s\033[m", BBS_FULL_NAME);
        move(21, 0);
        clrtobot();
        getdata(21, 0, "屏幕现在已经锁定，要解除锁定，请输入密码：", passbuf, 39, NOECHO, NULL, true);
        move(22, 32);
        if (!checkpasswd2(passbuf, getCurrentUser())) {
            prints("\033[1m\033[31m密码输入错误...\033[m\n");
            pressanykey();
        } else {
            prints("\033[1m\033[31m屏幕现在已经解除锁定\033[m\n");
            /*
             * pressanykey(); 
             */
            break;
        }
    }
    return 0;
}

void printdash(char *mesg)
{
    char buf[80], *ptr;
    int len;

    memset(buf, '=', 79);
    buf[79] = '\0';
    if (mesg != NULL) {
        len = strlen(mesg);
        if (len > 76)
            len = 76;
        ptr = &buf[40 - len / 2];
        ptr[-1] = ' ';
        ptr[len] = ' ';
        strncpy(ptr, mesg, len);
    }
    prints("%s\n", buf);
}

void bell()
{
    /*
     * change by KCN 1999.09.08    fprintf(stderr,"%c",Ctrl('G')) ; 
     */
    char sound;

    sound = Ctrl('G');
    output(&sound, 1);

}

int pressreturn()
{
    extern int showansi;
    char buf[3];

    showansi = 1;
    move(t_lines - 1, 0);
    prints("\x1b[m");
    clrtoeol();
    getdata(t_lines - 1, 0, "                              \x1b[33m请按 ◆\x1b[36mEnter\x1b[33m◆ 继续\x1b[m", buf, 2, NOECHO, NULL, true);
    move(t_lines - 1, 0);
    clrtoeol();
    return 0;
}

int askyn(str, defa)
char str[STRLEN];
int defa;
{
    int x, y;
    char realstr[STRLEN * 2];
    char ans[6];

    sprintf(realstr, "%s (Y/N)? [%c]: ", str, (defa) ? 'Y' : 'N');
    getyx(&x, &y);
    getdata(x, y, realstr, ans, 3, DOECHO, NULL, true);
    if (ans[0] == 'Y' || ans[0] == 'y')
        return 1;
    else if (ans[0] == 'N' || ans[0] == 'n')
        return 0;
    return defa;
}

int pressanykey()
{
    extern int showansi;

    showansi = 1;
    move(t_lines - 1, 0);
    prints("\x1b[m");
    clrtoeol();
    prints("                                \x1b[5;1;33m按任何键继续 ..\x1b[m");
    igetkey();
    move(t_lines - 1, 0);
    clrtoeol();
    return 0;
}
