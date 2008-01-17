/*
  lrz - receive files with x/y/zmodem
  Copyright (C) until 1988 Chuck Forsberg (Omen Technology INC)
  Copyright (C) 1994 Matt Porter, Michael D. Black
  Copyright (C) 1996, 1997 Uwe Ohse

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA.

  originally written by Chuck Forsberg
*/

#include "zglobal.h"

#define SS_NORMAL 0
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif



#ifndef HAVE_ERRNO_DECLARATION
extern int errno;
#endif

#define MAX_BLOCK 20480

/*
 * Max value for HOWMANY is 255 if NFGVMIN is not defined.
 *   A larger value reduces system overhead but may evoke kernel bugs.
 *   133 corresponds to an XMODEM/CRC sector
 */
#ifndef HOWMANY
#ifdef NFGVMIN
#define HOWMANY MAX_BLOCK
#else
#define HOWMANY 255
#endif
#endif

extern unsigned Baudrate;

FILE *fout;


static int Lastrx;
static int Crcflg;
static int Firstsec;
extern int errors;
extern int Restricted;             /* restricted; no /.. or ../ in filenames */
int Readnum = HOWMANY;          /* Number of bytes to ask for in read() from modem */
int skip_if_not_found;

char *Pathname;
const char *program_name;       /* the name by which we were called */

int MakeLCPathname = TRUE;      /* make received pathname lower case */
extern int Quiet;                  /* overrides logic that would otherwise set verbose */
int Nflag = 0;                  /* Don't really transfer files */
int Rxclob = FALSE;             /* Clobber existing file */
int Rxbinary = FALSE;           /* receive all files in bin mode */
int Rxascii = FALSE;            /* receive files in ascii (translate) mode */
int Thisbinary;                 /* current file is to be received in bin mode */
int try_resume = FALSE;
int junk_path = FALSE;
extern int no_timeout;
extern enum zm_type_enum protocol;
int zmodem_requested = FALSE;

#ifdef SEGMENTS
int chinseg = 0;                /* Number of characters received in this data seg */
char secbuf[1 + (SEGMENTS + 1) * MAX_BLOCK];
#else
char secbuf[MAX_BLOCK + 1];
#endif


#if defined(F_GETFD) && defined(F_SETFD) && defined(O_SYNC)
static int o_sync = 0;
#endif
static int rzfiles(struct zm_fileinfo *);
static int tryz(void);
static void checkpath(const char *name);
static void report(int sct);
static void uncaps(char *s);
static int IsAnyLower(const char *s);
static int putsec(struct zm_fileinfo *zi, char *buf, size_t n);
static int procheader(char *name, struct zm_fileinfo *);
static int wcgetsec(size_t * Blklen, char *rxbuf, unsigned int maxtime);
static int wcrx(struct zm_fileinfo *);
static int wcrxpn(struct zm_fileinfo *, char *rpn);
static int wcreceive(int argc, const char **argp);
static int rzfile(struct zm_fileinfo *);
static int closeit(struct zm_fileinfo *);
static void ackbibi(void);
static void zmputs(const char *s);
static size_t getfree(void);

static long buffersize = 32768;

extern char Lzmanag;                   /* Local file management request */
extern char zconv;                     /* ZMODEM file conversion request */
extern char zmanag;                    /* ZMODEM file management request */
extern char ztrans;                    /* ZMODEM file transport request */
extern int Zctlesc;                    /* Encode control characters */
extern int Zrwindow;            /* RX window size (controls garbage count) */

int tryzhdrtype = ZRINIT;       /* Header type to send corresponding to Last rx close */
extern time_t stop_time;


/* called by signal interrupt or terminate to clean things up */
extern void bibi(int n);

static char fname[1024];

char * bbs_zrecvfile()
{
    register int npats = 0;
    const char **patts = NULL;        /* keep compiler quiet */
    int exitcode = 0;
    const char* paths="tmp/";
    char* fn;

    Rxtimeout = 100;
    Restricted = 2;
    no_timeout = TRUE;

    protocol = ZM_ZMODEM;


    /* initialize zsendline tab */
    zsendline_init();

    io_mode(1, 1);
    readline_setup(1, 128, 256);
    purgeline(1);
    patts = &paths;
    fn = fname;
    if (wcreceive(npats, patts) == ERROR) {
        exitcode = 0200;
        canit(0);
        fn=NULL;
    }
    io_mode(0, 0);
    if (exitcode && !zmodem_requested)  /* bellow again with all thy might. */
        canit(0);
    return fn;
}

/*
 * Let's receive something already.
 */

static int wcreceive(int argc, const char **argp)
{
    int c;
    struct zm_fileinfo zi;

    zi.fname = NULL;
    zi.modtime = 0;
    zi.mode = 0;
    zi.bytes_total = 0;
    zi.bytes_sent = 0;
    zi.bytes_received = 0;
    zi.bytes_skipped = 0;
    zi.eof_seen = 0;

    if (protocol != ZM_XMODEM || argc == 0) {
        Crcflg = 1;
        if ((c = tryz()) != 0) {
            if (c == ZCOMPL)
                return OK;
            if (c == ERROR)
                goto fubar;
            c = rzfiles(&zi);

            if (c)
                goto fubar;
        } else {
            for (;;) {
                if (wcrxpn(&zi, secbuf) == ERROR)
                    goto fubar;
                if (secbuf[0] == 0)
                    return OK;
                if (procheader(secbuf, &zi) == ERROR)
                    goto fubar;
                if (wcrx(&zi) == ERROR)
                    goto fubar;

            }
        }
    } else {
        char dummy[128];

        dummy[0] = '\0';        /* pre-ANSI HPUX cc demands this */
        dummy[1] = '\0';        /* procheader uses name + 1 + strlen(name) */
        zi.bytes_total = DEFBYTL;

        procheader(dummy, &zi);

        if (Pathname)
            free(Pathname);
        errno = 0;
        Pathname = malloc(PATH_MAX + 1);
        if (!Pathname)
            zmodem_error(1, 0, "out of memory");

        strcpy(Pathname, *argp);
        checkpath(Pathname);

        if ((fout = fopen(Pathname, "w")) == NULL) {
            return ERROR;
        }
        if (wcrx(&zi) == ERROR) {
            goto fubar;
        }
    }
    return OK;
  fubar:
    canit(0);
    if (fout)
        fclose(fout);

    if (Restricted && Pathname) {
        unlink(Pathname);
    }
    return ERROR;
}


/*
 * Fetch a pathname from the other end as a C ctyle ASCIZ string.
 * Length is indeterminate as long as less than Blklen
 * A null string represents no more files (YMODEM)
 */
static int wcrxpn(struct zm_fileinfo *zi, char *rpn)
{
    register int c;
    size_t Blklen = 0;          /* record length of received packets */

#ifdef NFGVMIN
    READLINE_PF(1);
#else
    purgeline(0);
#endif

  et_tu:
    Firstsec = TRUE;
    zi->eof_seen = FALSE;
    sendline(Crcflg ? WANTCRC : NAK);
    flushmo();
    purgeline(0);               /* Do read next time ... */
    while ((c = wcgetsec(&Blklen, rpn, 100)) != 0) {
        if (c == WCEOT) {
            sendline(ACK);
            flushmo();
            purgeline(0);       /* Do read next time ... */
            READLINE_PF(1);
            goto et_tu;
        }
        return ERROR;
    }
    sendline(ACK);
    flushmo();
    return OK;
}

/*
 * Adapted from CMODEM13.C, written by
 * Jack M. Wierda and Roderick W. Hart
 */
static int wcrx(struct zm_fileinfo *zi)
{
    register int sectnum, sectcurr;
    register char sendchar;
    size_t Blklen;

    Firstsec = TRUE;
    sectnum = 0;
    zi->eof_seen = FALSE;
    sendchar = Crcflg ? WANTCRC : NAK;

    for (;;) {
        sendline(sendchar);     /* send it now, we're ready! */
        flushmo();
        purgeline(0);           /* Do read next time ... */
        sectcurr = wcgetsec(&Blklen, secbuf, (unsigned int) ((sectnum & 0177) ? 50 : 130));
        report(sectcurr);
        if (sectcurr == ((sectnum + 1) & 0377)) {
            sectnum++;
            /* if using xmodem we don't know how long a file is */
            if (zi->bytes_total && R_BYTESLEFT(zi) < Blklen)
                Blklen = R_BYTESLEFT(zi);
            zi->bytes_received += Blklen;
            if (putsec(zi, secbuf, Blklen) == ERROR)
                return ERROR;
            sendchar = ACK;
        } else if (sectcurr == (sectnum & 0377)) {
            sendchar = ACK;
        } else if (sectcurr == WCEOT) {
            if (closeit(zi))
                return ERROR;
            sendline(ACK);
            flushmo();
            purgeline(0);       /* Do read next time ... */
            return OK;
        } else if (sectcurr == ERROR)
            return ERROR;
        else {
            return ERROR;
        }
    }
}

/*
 * Wcgetsec fetches a Ward Christensen type sector.
 * Returns sector number encountered or ERROR if valid sector not received,
 * or CAN CAN received
 * or WCEOT if eot sector
 * time is timeout for first char, set to 4 seconds thereafter
 ***************** NO ACK IS SENT IF SECTOR IS RECEIVED OK **************
 *    (Caller must do that when he is good and ready to get next sector)
 */
static int wcgetsec(size_t * Blklen, char *rxbuf, unsigned int maxtime)
{
    register int checksum, wcj, firstch;
    register unsigned short oldcrc;
    register char *p;
    int sectcurr;

    for (Lastrx = errors = 0; errors < RETRYMAX; errors++) {

        if ((firstch = READLINE_PF(maxtime)) == STX) {
            *Blklen = 1024;
            goto get2;
        }
        if (firstch == SOH) {
            *Blklen = 128;
          get2:
            sectcurr = READLINE_PF(1);
            if ((sectcurr + (oldcrc = READLINE_PF(1))) == 0377) {
                oldcrc = checksum = 0;
                for (p = rxbuf, wcj = *Blklen; --wcj >= 0;) {
                    if ((firstch = READLINE_PF(1)) < 0)
                        goto bilge;
                    oldcrc = updcrc(firstch, oldcrc);
                    checksum += (*p++ = firstch);
                }
                if ((firstch = READLINE_PF(1)) < 0)
                    goto bilge;
                if (Crcflg) {
                    oldcrc = updcrc(firstch, oldcrc);
                    if ((firstch = READLINE_PF(1)) < 0)
                        goto bilge;
                    oldcrc = updcrc(firstch, oldcrc);
                    if (oldcrc & 0xFFFF) {
                    } else {
                        Firstsec = FALSE;
                        return sectcurr;
                    }
                } else if (((checksum - firstch) & 0377) == 0) {
                    Firstsec = FALSE;
                    return sectcurr;
                }
            }
        }
        /* make sure eot really is eot and not just mixmash */
#ifdef NFGVMIN
        else if (firstch == EOT && READLINE_PF(1) == TIMEOUT)
            return WCEOT;
#else
        else if (firstch == EOT && READLINE_PF > 0)
            return WCEOT;
#endif
        else if (firstch == CAN) {
            if (Lastrx == CAN) {
                return ERROR;
            } else {
                Lastrx = CAN;
                continue;
            }
        } else if (firstch == TIMEOUT) {
            if (Firstsec)
                goto humbug;
          bilge:
			;
        }

      humbug:
        Lastrx = 0;
        {
            int cnt = 1000;

            while (cnt-- && READLINE_PF(1) != TIMEOUT);
        }
        if (Firstsec) {
            sendline(Crcflg ? WANTCRC : NAK);
            flushmo();
            purgeline(0);       /* Do read next time ... */
        } else {
            maxtime = 40;
            sendline(NAK);
            flushmo();
            purgeline(0);       /* Do read next time ... */
        }
    }
    /* try to stop the bubble machine. */
    canit(0);
    return ERROR;
}

#define ZCRC_DIFFERS (ERROR+1)
#define ZCRC_EQUAL (ERROR+2)
/*
 * do ZCRC-Check for open file f.
 * check at most check_bytes bytes (crash recovery). if 0 -> whole file.
 * remote file size is remote_bytes.
 */
static int do_crc_check(FILE * f, size_t remote_bytes, size_t check_bytes)
{
    struct stat st;
    unsigned long crc;
    unsigned long rcrc;
    size_t n;
    int c;
    int t1 = 0, t2 = 0;

    if (-1 == fstat(fileno(f), &st)) {
        return ERROR;
    }
    if (check_bytes == 0 && ((size_t) st.st_size) != remote_bytes)
        return ZCRC_DIFFERS;    /* shortcut */

    crc = 0xFFFFFFFFL;
    n = check_bytes;
    if (n == 0)
        n = st.st_size;
    while (n-- && ((c = getc(f)) != EOF))
        crc = UPDC32(c, crc);
    crc = ~crc;
    clearerr(f);                /* Clear EOF */
    fseek(f, 0L, 0);

    while (t1 < 3) {
        stohdr(check_bytes);
        zshhdr(ZCRC, Txhdr);
        while (t2 < 3) {
            size_t tmp;

            c = zgethdr(Rxhdr, 0, &tmp);
            rcrc = (unsigned long) tmp;
            switch (c) {
            default:           /* ignore */
                break;
            case ZFIN:
                return ERROR;
            case ZRINIT:
                return ERROR;
            case ZCAN:
                return ERROR;
                break;
            case ZCRC:
                if (crc != rcrc)
                    return ZCRC_DIFFERS;
                return ZCRC_EQUAL;
                break;
            }
        }
    }
    return ERROR;
}

/*
 * Process incoming file information header
 */
static int procheader(char *name, struct zm_fileinfo *zi)
{
    const char *openmode;
    char *p;
    static char *name_static = NULL;
    char *nameend;

    if (name_static)
        free(name_static);
    if (junk_path) {
        p = strrchr(name, '/');
        if (p) {
            p++;
            if (!*p) {
                /* alert - file name ended in with a / */
                return ERROR;
            }
            name = p;
        }
    }
    name_static = malloc(strlen(name) + 1);
    if (!name_static)
        zmodem_error(1, 0, "out of memory");
    strcpy(name_static, name);
    zi->fname = name_static;


    /* set default parameters and overrides */
    openmode = "w";
    Thisbinary = (!Rxascii) || Rxbinary;
    if (Lzmanag)
        zmanag = Lzmanag;

    /*
     *  Process ZMODEM remote file management requests
     */
    if (!Rxbinary && zconv == ZCNL)     /* Remote ASCII override */
        Thisbinary = 0;
    if (zconv == ZCBIN)         /* Remote Binary override */
        Thisbinary = TRUE;
    if (Thisbinary && zconv == ZCBIN && try_resume)
        zconv = ZCRESUM;
    if (zmanag == ZF1_ZMAPND && zconv != ZCRESUM)
        openmode = "a";
    if (skip_if_not_found)
        openmode = "r+";

    zi->bytes_total = DEFBYTL;
    zi->mode = 0;
    zi->eof_seen = 0;
    zi->modtime = 0;

    nameend = name + 1 + strlen(name);
    if (*nameend) {             /* file coming from Unix or DOS system */
        long modtime;
        long bytes_total;
        int mode;

        sscanf(nameend, "%ld%lo%o", &bytes_total, &modtime, &mode);
        zi->modtime = modtime;
        zi->bytes_total = bytes_total;
        zi->mode = mode;
        if (zi->mode & UNIXFILE)
            ++Thisbinary;
    }

    /* Check for existing file */
    if (zconv != ZCRESUM && !Rxclob && (zmanag & ZF1_ZMMASK) != ZF1_ZMCLOB && (zmanag & ZF1_ZMMASK) != ZF1_ZMAPND && (fout = fopen(name, "r"))) {
        struct stat sta;
        char *tmpname;
        char *ptr;
        int i;

        if (zmanag == ZF1_ZMNEW || zmanag == ZF1_ZMNEWL) {
            if (-1 == fstat(fileno(fout), &sta)) {
                return ERROR;
            }
            if (zmanag == ZF1_ZMNEW) {
                if (sta.st_mtime > zi->modtime) {
                    return ERROR;       /* skips file */
                }
            } else {
                /* newer-or-longer */
                if (((size_t) sta.st_size) >= zi->bytes_total && sta.st_mtime > zi->modtime) {
                    return ERROR;       /* skips file */
                }
            }
            fclose(fout);
        } else if (zmanag == ZF1_ZMCRC) {
            int r = do_crc_check(fout, zi->bytes_total, 0);

            if (r == ERROR) {
                fclose(fout);
                return ERROR;
            }
            if (r != ZCRC_DIFFERS) {
                return ERROR;   /* skips */
            }
            fclose(fout);
        } else {
            size_t namelen;

            fclose(fout);
            if ((zmanag & ZF1_ZMMASK) != ZF1_ZMCHNG) {
                return ERROR;
            }
            /* try to rename */
            namelen = strlen(name);
            tmpname = malloc(namelen + 5);
            memcpy(tmpname, name, namelen);
            ptr = tmpname + namelen;
            *ptr++ = '.';
            i = 0;
            do {
                sprintf(ptr, "%d", i++);
            } while (i < 1000 && stat(tmpname, &sta) == 0);
            if (i == 1000) {
                free(tmpname);
                return ERROR;
            }
            free(name_static);
            name_static = malloc(strlen(tmpname) + 1);
            if (!name_static)
                zmodem_error(1, 0, "out of memory");
            strcpy(name_static, tmpname);
            zi->fname = name_static;
            free(tmpname);
        }
    }

    if (!*nameend) {            /* File coming from CP/M system */
        for (p = name_static; *p; ++p)  /* change / to _ */
            if (*p == '/')
                *p = '_';

        if (*--p == '.')        /* zap trailing period */
            *p = 0;
    }



    if (!zmodem_requested && MakeLCPathname && !IsAnyLower(name_static)
        && !(zi->mode & UNIXFILE))
        uncaps(name_static);
    {
        if (protocol == ZM_XMODEM)
            /* we don't have the filename yet */
            return OK;          /* dummy */
        if (Pathname)
            free(Pathname);
        Pathname = malloc((PATH_MAX) * 2);
        if (!Pathname)
            zmodem_error(1, 0, "out of memory");
        strcpy(Pathname, name_static);
        checkpath(name_static);
        if (Nflag) {
            /* cast because we might not have a prototyp for strdup :-/ */
            free(name_static);
            name_static = (char *) strdup("/dev/null");
            if (!name_static) {
                exit(1);
            }
        }
#ifdef OMEN
        /* looks like a security hole -- uwe */
        if (name_static[0] == '!' || name_static[0] == '|') {
            if (!(fout = popen(name_static + 1, "w"))) {
                return ERROR;
            }
            return (OK);
        }
#endif
/*
        if (Thisbinary && zconv == ZCRESUM) {
            struct stat st;

            fout = fopen(name_static, "r+");
            if (fout && 0 == fstat(fileno(fout), &st)) {
                int can_resume = TRUE;

                if (zmanag == ZF1_ZMCRC) {
                    int r = do_crc_check(fout, zi->bytes_total, st.st_size);

                    if (r == ERROR) {
                        fclose(fout);
                        return ZFERR;
                    }
                    if (r == ZCRC_DIFFERS) {
                        can_resume = FALSE;
                    }
                }
                if ((unsigned long) st.st_size > zi->bytes_total) {
                    can_resume = FALSE;
                }

                zi->bytes_skipped = st.st_size & ~(1023);
                if (can_resume) {
                    if (fseek(fout, (long) zi->bytes_skipped, SEEK_SET)) {
                        fclose(fout);
                        return ZFERR;
                    }
                } else
                    zi->bytes_skipped = 0;
                goto buffer_it;
            }
            zi->bytes_skipped = 0;
            if (fout)
                fclose(fout);
        }*/
        zi->bytes_skipped = 0;
        strcpy(fname, name_static);
        fout = fopen(name_static, openmode);
        if (!fout) {
            return ERROR;
        }
    }
    {
        static char *s = NULL;
        static size_t last_length = 0;

#if defined(F_GETFD) && defined(F_SETFD) && defined(O_SYNC)
        if (o_sync) {
            int oldflags;

            oldflags = fcntl(fileno(fout), F_GETFD, 0);
            if (oldflags >= 0 && !(oldflags & O_SYNC)) {
                oldflags |= O_SYNC;
                fcntl(fileno(fout), F_SETFD, oldflags); /* errors don't matter */
            }
        }
#endif

        if (buffersize == -1 && s) {
            if (zi->bytes_total > last_length) {
                free(s);
                s = NULL;
                last_length = 0;
            }
        }
        if (!s && buffersize) {
            last_length = 32768;
            if (buffersize == -1) {
                if (zi->bytes_total > 0)
                    last_length = zi->bytes_total;
            } else
                last_length = buffersize;
            /* buffer `4096' bytes pages */
            last_length = (last_length + 4095) & 0xfffff000;
            s = malloc(last_length);
            if (!s) {
                exit(1);
            }
        }
        if (s) {
#ifdef SETVBUF_REVERSED
            setvbuf(fout, _IOFBF, s, last_length);
#else
            setvbuf(fout, s, _IOFBF, last_length);
#endif
        }
    }
    zi->bytes_received = zi->bytes_skipped;

    return OK;
}


/*
 * Putsec writes the n characters of buf to receive file fout.
 *  If not in binary mode, carriage returns, and all characters
 *  starting with CPMEOF are discarded.
 */
static int putsec(struct zm_fileinfo *zi, char *buf, size_t n)
{
    register char *p;

    if (n == 0)
        return OK;
    if (Thisbinary) {
        if (fwrite(buf, n, 1, fout) != 1)
            return ERROR;
    } else {
        if (zi->eof_seen)
            return OK;
        for (p = buf; n > 0; ++p, n--) {
            if (*p == '\r')
                continue;
            if (*p == CPMEOF) {
                zi->eof_seen = TRUE;
                return OK;
            }
            putc(*p, fout);
        }
    }
    return OK;
}

/* make string s lower case */
static void uncaps(char *s)
{
    for (; *s; ++s)
        if (isupper((unsigned char) (*s)))
            *s = tolower(*s);
}

/*
 * IsAnyLower returns TRUE if string s has lower case letters.
 */
static int IsAnyLower(const char *s)
{
    for (; *s; ++s)
        if (islower((unsigned char) (*s)))
            return TRUE;
    return FALSE;
}

static void report(int sct)
{
}


/*
 * Totalitarian Communist pathname processing
 */
static void checkpath(const char *name)
{
    if (Restricted) {
        const char *p;

        p = strrchr(name, '/');
        if (p)
            p++;
        else
            p = name;
        /* don't overwrite any file in very restricted mode.
         * don't overwrite hidden files in restricted mode */
        if ((Restricted == 2 || *name == '.') && fopen(name, "r") != NULL) {
            canit(0);
            bibi(-1);
        }
        /* restrict pathnames to current tree or uucppublic */
        if (strstr(name, "../")
#ifdef PUBDIR
            || (name[0] == '/' && strncmp(name, PUBDIR, strlen(PUBDIR)))
#endif
            ) {
            canit(0);
            bibi(-1);
        }
        if (Restricted > 1) {
            if (name[0] == '.' || strstr(name, "/.")) {
                canit(0);
                bibi(-1);
            }
        }
    }
}

/*
 * Initialize for Zmodem receive attempt, try to activate Zmodem sender
 *  Handles ZSINIT frame
 *  Return ZFILE if Zmodem filename received, -1 on error,
 *   ZCOMPL if transaction finished,  else 0
 */
static int tryz(void)
{
    register int c, n;
    register int cmdzack1flg;
    int zrqinits_received = 0;
    size_t bytes_in_block = 0;

    if (protocol != ZM_ZMODEM)  /* Check for "rb" program name */
        return 0;

    for (n = zmodem_requested ? 15 : 5; (--n + zrqinits_received) >= 0 && zrqinits_received < 10;) {
        /* Set buffer length (0) and capability flags */
#ifdef SEGMENTS
        stohdr(SEGMENTS * MAX_BLOCK);
#else
        stohdr(0L);
#endif
#ifdef CANBREAK
        Txhdr[ZF0] = CANFC32 | CANFDX | CANOVIO | CANBRK;
#else
        Txhdr[ZF0] = CANFC32 | CANFDX | CANOVIO;
#endif
        if (Zctlesc)
            Txhdr[ZF0] |= TESCCTL;      /* TESCCTL == ESCCTL */
        zshhdr(tryzhdrtype, Txhdr);

        if (tryzhdrtype == ZSKIP)       /* Don't skip too far */
            tryzhdrtype = ZRINIT;       /* CAF 8-21-87 */
      again:
        switch (zgethdr(Rxhdr, 0, NULL)) {
        case ZRQINIT:
            /* getting one ZRQINIT is totally ok. Normally a ZFILE follows 
             * (and might be in our buffer, so don't purge it). But if we
             * get more ZRQINITs than the sender has started up before us
             * and sent ZRQINITs while waiting. 
             */
            zrqinits_received++;
            continue;

        case ZEOF:
            continue;
        case TIMEOUT:
            continue;
        case ZFILE:
            zconv = Rxhdr[ZF0];
            if (!zconv)
                /* resume with sz -r is impossible (at least with unix sz)
                 * if this is not set */
                zconv = ZCBIN;
            if (Rxhdr[ZF1] & ZF1_ZMSKNOLOC) {
                Rxhdr[ZF1] &= ~(ZF1_ZMSKNOLOC);
                skip_if_not_found = TRUE;
            }
            zmanag = Rxhdr[ZF1];
            ztrans = Rxhdr[ZF2];
            tryzhdrtype = ZRINIT;
            c = zrdata(secbuf, MAX_BLOCK, &bytes_in_block);
            io_mode(0, 3);
            if (c == GOTCRCW)
                return ZFILE;
            zshhdr(ZNAK, Txhdr);
            goto again;
        case ZSINIT:
            /* this once was:
             * Zctlesc = TESCCTL & Rxhdr[ZF0];
             * trouble: if rz get --escape flag:
             * - it sends TESCCTL to sz, 
             *   get a ZSINIT _without_ TESCCTL (yeah - sender didn't know), 
             *   overwrites Zctlesc flag ...
             * - sender receives TESCCTL and uses "|=..."
             * so: sz escapes, but rz doesn't unescape ... not good.
             */
            Zctlesc |= TESCCTL & Rxhdr[ZF0];
            if (zrdata(Attn, ZATTNLEN, &bytes_in_block) == GOTCRCW) {
                stohdr(1L);
                zshhdr(ZACK, Txhdr);
                goto again;
            }
            zshhdr(ZNAK, Txhdr);
            goto again;
        case ZFREECNT:
            stohdr(getfree());
            zshhdr(ZACK, Txhdr);
            goto again;
        case ZCOMMAND:
            cmdzack1flg = Rxhdr[ZF0];
            if (zrdata(secbuf, MAX_BLOCK, &bytes_in_block) == GOTCRCW) {
                zshhdr(ZCOMPL, Txhdr);
                return ZCOMPL;
            }
            zshhdr(ZNAK, Txhdr);
            goto again;
        case ZCOMPL:
            goto again;
        default:
            continue;
        case ZFIN:
            ackbibi();
            return ZCOMPL;
        case ZRINIT:
            return ERROR;
        case ZCAN:
            return ERROR;
        }
    }
    return 0;
}


/*
 * Receive 1 or more files with ZMODEM protocol
 */
static int rzfiles(struct zm_fileinfo *zi)
{
    register int c;

    for (;;) {
        c = rzfile(zi);
        switch (c) {
        case ZEOF:
            /* FALL THROUGH */
        case ZSKIP:
            if (c == ZSKIP) {
            }
            switch (tryz()) {
            case ZCOMPL:
                return OK;
            default:
                return ERROR;
            case ZFILE:
                break;
            }
            continue;
        default:
            return c;
        case ERROR:
            return ERROR;
        }
    }
}

/* "OOSB" means Out Of Sync Block. I once thought that if sz sents
 * blocks a,b,c,d, of which a is ok, b fails, we might want to save 
 * c and d. But, alas, i never saw c and d.
 */
#define SAVE_OOSB
#ifdef SAVE_OOSB
typedef struct oosb_t {
    size_t pos;
    size_t len;
    char *data;
    struct oosb_t *next;
} oosb_t;
struct oosb_t *anker = NULL;
#endif

/*
 * Receive a file with ZMODEM protocol
 *  Assumes file name frame is in secbuf
 */
static int rzfile(struct zm_fileinfo *zi)
{
    register int c, n;
    size_t bytes_in_block = 0;

    zi->eof_seen = FALSE;

    n = 20;

    if (procheader(secbuf, zi) == ERROR) {
        return (tryzhdrtype = ZSKIP);
    }

    for (;;) {
#ifdef SEGMENTS
        chinseg = 0;
#endif
        stohdr(zi->bytes_received);
        zshhdr(ZRPOS, Txhdr);
        goto skip_oosb;
      nxthdr:
#ifdef SAVE_OOSB
        if (anker) {
            oosb_t *akt, *last, *next;

            for (akt = anker, last = NULL; akt; last = akt ? akt : last, akt = next) {
                if (akt->pos == zi->bytes_received) {
                    putsec(zi, akt->data, akt->len);
                    zi->bytes_received += akt->len;
                    goto nxthdr;
                }
                next = akt->next;
                if (akt->pos < zi->bytes_received) {
                    if (last)
                        last->next = akt->next;
                    else
                        anker = akt->next;
                    free(akt->data);
                    free(akt);
                    akt = NULL;
                }
            }
        }
#endif
      skip_oosb:
        c = zgethdr(Rxhdr, 0, NULL);
        switch (c) {
        default:
            return ERROR;
        case ZNAK:
        case TIMEOUT:
#ifdef SEGMENTS
            putsec(secbuf, chinseg);
            chinseg = 0;
#endif
            if (--n < 0) {
                return ERROR;
            }
        case ZFILE:
            zrdata(secbuf, MAX_BLOCK, &bytes_in_block);
            continue;
        case ZEOF:
#ifdef SEGMENTS
            putsec(secbuf, chinseg);
            chinseg = 0;
#endif
            if (rclhdr(Rxhdr) != (long) zi->bytes_received) {
                /*
                 * Ignore eof if it's at wrong place - force
                 *  a timeout because the eof might have gone
                 *  out before we sent our zrpos.
                 */
                errors = 0;
                goto nxthdr;
            }
            if (closeit(zi)) {
                tryzhdrtype = ZFERR;
                return ERROR;
            }
            return c;
        case ERROR:            /* Too much garbage in header search error */
#ifdef SEGMENTS
            putsec(secbuf, chinseg);
            chinseg = 0;
#endif
            if (--n < 0) {
                return ERROR;
            }
            zmputs(Attn);
            continue;
        case ZSKIP:
#ifdef SEGMENTS
            putsec(secbuf, chinseg);
            chinseg = 0;
#endif
            closeit(zi);
            return c;
        case ZDATA:
            if (rclhdr(Rxhdr) != (long) zi->bytes_received) {
#if defined(SAVE_OOSB)
                oosb_t *neu;
                size_t pos = rclhdr(Rxhdr);
#endif
                if (--n < 0) {
                    return ERROR;
                }
#if defined(SAVE_OOSB)
                switch (c = zrdata(secbuf, MAX_BLOCK, &bytes_in_block)) {
                case GOTCRCW:
                case GOTCRCG:
                case GOTCRCE:
                case GOTCRCQ:
                    if (pos > zi->bytes_received) {
                        neu = malloc(sizeof(oosb_t));
                        if (neu)
                            neu->data = malloc(bytes_in_block);
                        if (neu && neu->data) {
                            memcpy(neu->data, secbuf, bytes_in_block);
                            neu->pos = pos;
                            neu->len = bytes_in_block;
                            neu->next = anker;
                            anker = neu;
                        } else if (neu)
                            free(neu);
                    }
                }
#endif
#ifdef SEGMENTS
                putsec(secbuf, chinseg);
                chinseg = 0;
#endif
                zmputs(Attn);
                continue;
            }
          moredata:
#ifdef SEGMENTS
            if (chinseg >= (MAX_BLOCK * SEGMENTS)) {
                putsec(secbuf, chinseg);
                chinseg = 0;
            }
            switch (c = zrdata(secbuf + chinseg, MAX_BLOCK, &bytes_in_block))
#else
            switch (c = zrdata(secbuf, MAX_BLOCK, &bytes_in_block))
#endif
            {
            case ZCAN:
#ifdef SEGMENTS
                putsec(secbuf, chinseg);
                chinseg = 0;
#endif
                return ERROR;
            case ERROR:        /* CRC error */
#ifdef SEGMENTS
                putsec(secbuf, chinseg);
                chinseg = 0;
#endif
                if (--n < 0) {
                    return ERROR;
                }
                zmputs(Attn);
                continue;
            case TIMEOUT:
#ifdef SEGMENTS
                putsec(secbuf, chinseg);
                chinseg = 0;
#endif
                if (--n < 0) {
                    return ERROR;
                }
                continue;
            case GOTCRCW:
                n = 20;
#ifdef SEGMENTS
                chinseg += bytes_in_block;
                putsec(zi, secbuf, chinseg);
                chinseg = 0;
#else
                putsec(zi, secbuf, bytes_in_block);
#endif
                zi->bytes_received += bytes_in_block;
                stohdr(zi->bytes_received);
                zshhdr(ZACK | 0x80, Txhdr);
                goto nxthdr;
            case GOTCRCQ:
                n = 20;
#ifdef SEGMENTS
                chinseg += bytes_in_block;
#else
                putsec(zi, secbuf, bytes_in_block);
#endif
                zi->bytes_received += bytes_in_block;
                stohdr(zi->bytes_received);
                zshhdr(ZACK, Txhdr);
                goto moredata;
            case GOTCRCG:
                n = 20;
#ifdef SEGMENTS
                chinseg += bytes_in_block;
#else
                putsec(zi, secbuf, bytes_in_block);
#endif
                zi->bytes_received += bytes_in_block;
                goto moredata;
            case GOTCRCE:
                n = 20;
#ifdef SEGMENTS
                chinseg += bytes_in_block;
#else
                putsec(zi, secbuf, bytes_in_block);
#endif
                zi->bytes_received += bytes_in_block;
                goto nxthdr;
            }
        }
    }
}

/*
 * Send a string to the modem, processing for \336 (sleep 1 sec)
 *   and \335 (break signal)
 */
static void zmputs(const char *s)
{
    const char *p;

    while (s && *s) {
        p = strpbrk(s, "\335\336");
        if (!p) {
            raw_write(0, s, strlen(s));
            return;
        }
        if (p != s) {
            raw_write(0, s, (size_t) (p - s));
            s = p;
        }
        if (*p == '\336')
            sleep(1);
        else
            sendbrk(0);
        p++;
    }
}

/*
 * Close the receive dataset, return OK or ERROR
 */
static int closeit(struct zm_fileinfo *zi)
{
    int ret;

    ret = fclose(fout);
    if (ret) {
        /* this may be any sort of error, including random data corruption */

        unlink(Pathname);
        return ERROR;
    }
    if (zi->modtime) {
#ifdef HAVE_STRUCT_UTIMBUF
        struct utimbuf timep;

        timep.actime = time(NULL);
        timep.modtime = zi->modtime;
        utime(Pathname, &timep);
#else
        time_t timep[2];

        timep[0] = time(NULL);
        timep[1] = zi->modtime;
        utime(Pathname, timep);
#endif
    }
#ifdef S_ISREG
    if (S_ISREG(zi->mode)) {
#else
    if ((zi->mode & S_IFMT) == S_IFREG) {
#endif
        /* we must not make this program executable if running 
         * under rsh, because the user might have uploaded an
         * unrestricted shell.
         */
        chmod(Pathname, (00666 & zi->mode));
    }
    return OK;
}

/*
 * Ack a ZFIN packet, let byegones be byegones
 */
static void ackbibi(void)
{
    int n;

    Readnum = 1;
    stohdr(0L);
    for (n = 3; --n >= 0;) {
        purgeline(0);
        zshhdr(ZFIN, Txhdr);
        switch (READLINE_PF(100)) {
        case 'O':
            READLINE_PF(1);     /* Discard 2nd 'O' */
            return;
        case RCDO:
            return;
        case TIMEOUT:
        default:
            break;
        }
    }
}



/*
 * Routine to calculate the free bytes on the current file system
 *  ~0 means many free bytes (unknown)
 */
static size_t getfree(void)
{
    return ((size_t) (~0L));    /* many free bytes ... */
}

/* End of lrz.c */
