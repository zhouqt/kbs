/*
µ×²ãµÄI/O¿â¡£
		KCNÖØÐ´
*/

#include "bbs.h"
#include <arpa/telnet.h>

#ifdef AIX
#include <sys/select.h>
#endif

/*ÊäÈëÊä³ö»º³åÇøµÄ´óÐ¡
  Êä³ö»º³åÇø¸ø¸ö°ëÆÁ´óÐ¡×ãÒÔ£¬
  ÊäÈë»º³åÇøÒ»°ã¶¼ÀûÓÃ²»ÉÏ£¬¸ø
  ¸ö128×Ö½Ú°É
*/
#define OBUFSIZE  (1024)
#define IBUFSIZE  (128)

#define INPUT_ACTIVE 0
#define INPUT_IDLE 1

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
extern char *big2gb(char *, int *, int);
extern char *gb2big(char *, int *, int);
extern int ssh_sock;
void oflush()
{
    if (obufsize) {
        if (convcode) {
            char *out;

            out = gb2big(outbuf, &obufsize, 0);
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
    /* need to change IAC to IAC IAC */
    if (((unsigned char) c) == IAC) {
        if (obufsize > OBUFSIZE - 1) {  /* doin a oflush */
            oflush();
        }
        outbuf[obufsize++] = c;
    }
}

#define ZMODEM_RATE 5000
int ZmodemRateLimit = 1;
int raw_write(int fd, char *buf, int len)
{
    static int lastcounter = 0;
    int nowcounter, i;
    static int bufcounter;
    int retlen;

    if (ZmodemRateLimit) {
        nowcounter = time(0);
        if (lastcounter == nowcounter) {
            if (bufcounter >= ZMODEM_RATE) {
                sleep(1);
                nowcounter = time(0);
                bufcounter = len;
            } else
                bufcounter += len;
        } else {
            /* time clocked, clear bufcounter */
            bufcounter = len;
        }
        lastcounter = nowcounter;
    }
#ifdef SSHBBS
    return ssh_write(fd, buf, len);
#else
    for (i = 0; i < len; i++) {
        int mylen;

        if ((unsigned char) buf[i] == 0xff)
            mylen = write(fd, "\0xff,0xff", 2);
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
    return read(fd, buf, len);
#endif
}

void output(s, len)
    char *s;
    int len;
{
    /* need to change IAC to IAC IAC
       if(obufsize+len > OBUFSIZE) {
       #ifdef SSHBBS
       ssh_write(0,outbuf,obufsize) ;
       #else
       write(0,outbuf,obufsize) ;
       #endif
       obufsize = 0 ;
       }
       memcpy(outbuf+obufsize, s, len) ;
       obufsize+=len ; */
    int i;

    for (i = 0; i < len; i++)
        ochar(s[i]);
}


int i_newfd = 0;
static void (*flushf) () = NULL;

static int i_timeout = 0;
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

void set_alarm(int set_timeout, void (*timeout_func) (void *), void *data)
{
    i_timeout = set_timeout;
    i_begintimeout = time(0);
    i_timeout_func = timeout_func;
    timeout_data = data;
}

int num_in_buf()
{
    /*---   Modified according to zhch@dii	period	2000-11-21	---*/
    /*    return icurrchar - ibufsize ; */
    int n;

    if ((n = icurrchar - ibufsize) < 0)
        n = 0;
    return n;
}

int telnet_state = 0;
char lastch;

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
        if (ch == SE)
            telnet_state = 0;
        else
            telnet_state = 4;
        break;
    case 4:                    /* telnet SB data */
        if (ch == IAC)
            telnet_state = 3;   /* wait for SE */
        break;
    case 5:
        telnet_state = 6;
        break;
    case 6:
        /*
           if (ch<120&&ch>=80)
           t_columns=ch;
           else
           t_columns=80;
         */
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
        /*
           if (ch<35&&ch>=24)
           t_lines=ch;
           else
           t_lines=24;
         */
        if (ch == IAC)
            telnet_state = 4;
        else
            telnet_state = 4;
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
            if (ch == IAC) {    /* Á½¸öIAC */
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
        if (hasaddio && (i_newfd)) {
            FD_SET(i_newfd, &readfds);
            if (hifd <= i_newfd)
                hifd = i_newfd + 1;
        }
        sr = select(hifd, &readfds, NULL, NULL, &to);
	if (sr < 0 && errno == EINTR && talkrequest )
	    return KEY_TALK;
        if (sr < 0 && errno != EINTR)
            abort_bbs(0);
        if (sr == 0) {
            if (flushf)
                (*flushf) ();
            refresh();

            while (1) {
                int alarm_timeout;

                hifd = 1;
                FD_ZERO(&xds);
                FD_SET(0, &xds);
                FD_ZERO(&readfds);
                FD_SET(0, &readfds);
                if (hasaddio && (i_newfd)) {
                    FD_SET(i_newfd, &readfds);
                    if (hifd <= i_newfd)
                        hifd = i_newfd + 1;
                }
                alarm_timeout = 0;
                if (i_top)
                    to = *i_top;
                else {
                    while (i_timeout != 0) {
                        to.tv_sec = i_timeout - (time(0) - i_begintimeout);
                        if (to.tv_sec <= 0) {
                            i_timeout = 0;
                            (*i_timeout_func) (timeout_data);
                            continue;
                        };
                        alarm_timeout = 1;
                        break;
                    };
                    if (!alarm_timeout)
                        to.tv_sec = IDLE_TIMEOUT;
                }
                sr = select(hifd, &readfds, NULL, &xds, &to);
		if (sr < 0 && errno == EINTR && talkrequest )
	    		return KEY_TALK;
                if (sr == 0 && alarm_timeout) {
                    i_timeout = 0;
                    (*i_timeout_func) (timeout_data);
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

        /* add by KCN for GB/BIG5 encode */
        if (convcode) {
            inbuf = big2gb(inbuffer + 1, &ibufsize, 0);
            if (ibufsize == 0) {
                icurrchar = 0;
                goto igetagain;
            }
        } else
            inbuf = inbuffer + 1;
        /* end */
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
    case Ctrl('L'):
        redoscr();
        icurrchar++;
        goto igetagain;
    default:
        break;
    }
    icurrchar++;
    while ((icurrchar != ibufsize) && (inbuf[icurrchar] == 0))
        icurrchar++;
    now = time(0);
    /*---	Ctrl-T disabled as anti-idle key	period	2000-12-05	---*/
    if (Ctrl('T') != c)
        uinfo.freshtime = now;
    /* add by KCN , decrease temp_numposts */
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

int igetkey()
{
    int mode;
    int ch, last;

    mode = last = 0;
    while (1) {
        ch = igetch();

        check_calltime();

        if ((ch==KEY_TALK)&&talkrequest) {
        	if(uinfo.mode!=CHAT1&&uinfo.mode!=CHAT2&&uinfo.mode!=CHAT3&&uinfo.mode!=CHAT4
        		&&uinfo.mode!=TALK&&uinfo.mode!=PAGE) {
		        talkreply();
		        return KEY_REFRESH;
		    } else
		    	return KEY_TALK;
    	}
        if (mode == 0) {
            if (ch == KEY_ESC)
                mode = 1;
            else
                return ch;      /* Normal Key */
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
            if (ch >= 'A' && ch <= 'D')
                return KEY_UP + (ch - 'A');
            else if (ch >= '1' && ch <= '6')
                mode = 3;
            else
                return ch;
        } else if (mode == 3) { /* Ins Del Home End PgUp PgDn */
            if (ch == '~')
                return KEY_HOME + (last - '1');
            else
                return ch;
        }
        last = ch;
    }
}

int getdata(int line, int col, char *prompt, char *buf, int len, int echo, void *nouse, int clearlabel)
{
    int ch, clen = 0, curr = 0, x, y;
    char tmp[STRLEN];
    extern unsigned char scr_cols;
    extern int RMSG;

    if (clearlabel == true) {
        memset(buf, 0, sizeof(buf));
    }
    move(line, col);
    if (prompt)
        prints("%s", prompt);
    y = line;
//    getyx(&y,&x);
    col += (prompt == NULL) ? 0 : num_noans_chr(prompt);
    x = col;
    clen = strlen(buf);
    curr = (clen >= len) ? len - 1 : clen;
    buf[curr] = '\0';
    prints("%s", buf);

    if (!scrint || (echo == false)) {
        while ((ch = igetkey()) != '\r') {
        	/* TODO: add KEY_REFRESH support */
            if (ch == '\n')
                break;
            if (ch == '\177' || ch == Ctrl('H')) {
                if (clen == 0) {
                    continue;
                }
                clen--;
                ochar(Ctrl('H'));
                ochar(' ');
                ochar(Ctrl('H'));
                oflush();
                continue;
            }
            if (!isprint2(ch)) {
                continue;
            }
            if (clen >= len - 1) {
                continue;
            }
            buf[clen++] = ch;
            /* move(line, col + clen);  Leeward 98.02.23  -- removed by wwj 2001/5/8 */
            if (echo)
                ochar(ch);
            else
                ochar('*');
        }
        buf[clen] = '\0';
        prints("\n");
        oflush();
        return clen;
    }
    clrtoeol();
    while (1) {
        if ((uinfo.mode == CHAT1 || uinfo.mode == TALK) && RMSG == true) {
            refresh();
        }
        ch = igetkey();
        /* TODO: add KEY_REFRESH support ???*/

        if (true == RMSG && (KEY_UP == ch || KEY_DOWN == ch))
            return -ch;         /* Leeward 98.07.30 supporting msgX */

        if (ch == '\n' || ch == '\r')
            break;
        if (ch == '\177' || ch == Ctrl('H')) {
            if (curr == 0) {
                continue;
            }
            strcpy(tmp, &buf[curr]);
            buf[--curr] = '\0';
            (void) strcat(buf, tmp);
            clen--;
            move(y, x);
            prints("%s", buf);
            clrtoeol();
            move(y, x + curr);
            continue;
        }
        if (ch == KEY_DEL) {
            if (curr >= clen) {
                curr = clen;
                continue;
            }
            strcpy(tmp, &buf[curr + 1]);
            buf[curr] = '\0';
            (void) strcat(buf, tmp);
            clen--;
            move(y, x);
            prints("%s", buf);
            clrtoeol();
            move(y, x + curr);
            continue;
        }
        if (ch == KEY_LEFT) {
            if (curr == 0) {
                continue;
            }
            curr--;
            move(y, x + curr);
            continue;
        }
        if (ch == Ctrl('E') || ch == KEY_END) {
            curr = clen;
            move(y, x + curr);
            continue;
        }
        if (ch == Ctrl('A') || ch == KEY_HOME) {
            curr = 0;
            move(y, x + curr);
            continue;
        }
        if (ch == KEY_RIGHT) {
            if (curr >= clen) {
                curr = clen;
                continue;
            }
            curr++;
            move(y, x + curr);
            continue;
        }
        if (!isprint2(ch)) {
            continue;
        }

        if (x + clen >= scr_cols || clen >= len - 1) {
            continue;
        }

        if (!buf[curr]) {
            buf[curr + 1] = '\0';
            buf[curr] = ch;
            outc(ch);
        } else {
            /*
               strncpy(tmp, &buf[curr], len);
               buf[curr] = ch;
               buf[curr + 1] = '\0';
               strncat(buf, tmp, len - curr);
             */
            int i;

            for (i = len - 2; i >= curr; i--)
                buf[i + 1] = buf[i];
            buf[curr] = ch;
            move(y, x + curr);
            outs(buf + curr);
            move(y, x + curr + 1);
        }
        curr++;
        clen++;
        /*
           move(y, x);
           prints("%s", buf);
           move(y, x + curr);
         */
    }
    buf[clen] = '\0';
    if (echo) {
        move(y, x);
        prints("%s", buf);
    }
    prints("\n");
    refresh();
    return clen;
}

int lock_scr()
{                               /* Leeward 98.02.22 */
    char passbuf[STRLEN];

    if (!strcmp(currentuser->userid, "guest"))
        return 1;

    modify_user_mode(LOCKSCREEN);
    clear();
    /*lock_monitor(); */
    while (1) {
        move(19, 32);
        clrtobot();
        prints("[1m[32mBBS " NAME_BBS_CHINESE "Õ¾[m");
        move(21, 0);
        clrtobot();
        getdata(21, 0, "ÆÁÄ»ÏÖÔÚÒÑ¾­Ëø¶¨£¬Òª½â³ýËø¶¨£¬ÇëÊäÈëÃÜÂë£º", passbuf, 39, NOECHO, NULL, true);
        move(22, 32);
        if (!checkpasswd2(passbuf, currentuser)) {
            prints("[1m[31mÃÜÂëÊäÈë´íÎó...[m\n");
            pressanykey();
        } else {
            prints("[1m[31mÆÁÄ»ÏÖÔÚÒÑ¾­½â³ýËø¶¨[m\n");
            /*pressanykey(); */
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
    /* change by KCN 1999.09.08    fprintf(stderr,"%c",Ctrl('G')) ; */
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
    clrtoeol();
    getdata(t_lines - 1, 0, "                              \x1b[33mÇë°´ ¡ô\x1b[36mEnter\x1b[33m¡ô ¼ÌÐø\x1b[m", buf, 2, NOECHO, NULL, true);
    move(t_lines - 1, 0);
    clrtoeol();
    refresh();
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
    clrtoeol();
    prints("\x1b[m                                \x1b[5;1;33m°´ÈÎºÎ¼ü¼ÌÐø ..\x1b[m");
    igetkey();
    move(t_lines - 1, 0);
    clrtoeol();
    return 0;
}
