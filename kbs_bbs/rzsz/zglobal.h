#ifndef ZMODEM_GLOBAL_H
#define ZMODEM_GLOBAL_H

/* zglobal.h - prototypes etcetera for lrzsz

  Copyright (C) until 1998 Chuck Forsberg (OMEN Technology Inc)
  Copyright (C) 1994 Matt Porter
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
*/

#include "bbs.h"

extern jmp_buf zmodemjmp;
extern int raw_read(int fd, char *buf, int len);
extern int raw_write(int fd, const char *buf, int len);
extern void raw_ochar(char c);

#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX 256
#endif

#if !defined(PATH_MAX) && defined(_PC_PATH_MAX)
#define PATH_MAX (pathconf ("/", _PC_PATH_MAX) < 1 ? 1024 : pathconf ("/", _PC_PATH_MAX))
#endif

#if !defined(PATH_MAX) && defined(MAXPATHLEN)
#define PATH_MAX MAXPATHLEN
#endif

#ifndef PATH_MAX
#define PATH_MAX _POSIX_PATH_MAX
#endif

#define OK      0
#define ERROR   (-1)

#ifndef FALSE
#define FALSE   0
#endif /* FALSE */

#ifndef TRUE
#define TRUE    1
#endif /* TRUE */

/* Ward Christensen / CP/M parameters - Don't change these! */
#define ENQ         005
#define CAN         ('X'&037)
#define XOFF        ('s'&037)
#define XON         ('q'&037)
#define SOH         1
#define STX         2
#define EOT         4
#define ACK         6
#define NAK         025
#define CPMEOF      032
#define WANTCRC     0103            /* send C not NAK to get crc not checksum */
#define WANTG       0107            /* Send G not NAK to get nonstop batch xmsn */
#define TIMEOUT     (-2)
#define RCDO        (-3)
#define WCEOT       (-10)
#define RETRYMAX    10
#define UNIXFILE    0xF000          /* The S_IFMT file mask bit for stat */
#define DEFBYTL     2000000000L     /* default rx file size */

enum zm_type_enum {
    ZM_XMODEM,
    ZM_YMODEM,
    ZM_ZMODEM
};

struct zm_fileinfo {
    char *fname;
    time_t modtime;
    mode_t mode;
    size_t bytes_total;
    size_t bytes_sent;
    size_t bytes_received;
    size_t bytes_skipped;           /* crash recovery */
    int eof_seen;
};

#define R_BYTESLEFT(x)  ((x)->bytes_total-(x)->bytes_received)

extern enum zm_type_enum protocol;
extern const char *program_name;    /* the name by which we were called */
extern int errors;
extern int no_timeout;
extern int Zctlesc;                 /* Encode control characters */
void bibi(int n);

#define sendline(c)     raw_ochar((c) & 0377)
#define xsendline(c)    raw_ochar(c)

/* zreadline.c */
extern char *readline_ptr;          /* pointer for removing chars from linbuf */
extern int readline_left;           /* number of buffered chars left to read */

#define READLINE_PF(timeout) \
    (--readline_left >= 0? (*readline_ptr++ & 0377) : readline_internal(timeout))

int readline_internal(unsigned int timeout);
void readline_purge(void);
void readline_setup(int fd, size_t readnum, size_t buffer_size);

/* rbsb.c */
extern int Twostop;

#ifdef READCHECK_READS
extern unsigned char checked;
#endif
extern int iofd;
extern unsigned Baudrate;

/* rbsb.c */
int rdchk(int fd);
int io_mode(int fd, int n);
void sendbrk(int fd);

#define flushmo() oflush()
void purgeline(int fd);
void canit(int fd);

/* error.c */

extern void zmodem_error(int status, int errnum, const char *format, ...);

/* crctab.c */
extern unsigned short crctab[256];

#define updcrc(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)
extern long cr3tab[];

#define UPDC32(b, c) (cr3tab[((int)c ^ b) & 0xff] ^ ((c >> 8) & 0x00FFFFFF))

/* zm.c */
#include "zmodem.h"
extern unsigned int Rxtimeout;  /* Tenths of seconds to wait for something */
extern int bytes_per_error;     /* generate one error around every x bytes */

/* Globals used by ZMODEM functions */
extern int Rxframeind;          /* ZBIN ZBIN32, or ZHEX type of frame received */
extern int Rxtype;              /* Type of header received */
extern int Zrwindow;            /* RX window size (controls garbage count) */

/* extern int Rxcount; *//* Count of data bytes received */
extern char Rxhdr[4];           /* Received header */
extern char Txhdr[4];           /* Transmitted header */
extern long Txpos;              /* Transmitted file position */
extern int Txfcs32;             /* TURE means send binary frames with 32 bit FCS */
extern int Crc32t;              /* Display flag indicating 32 bit CRC being sent */
extern int Crc32;               /* Display flag indicating 32 bit CRC being received */
extern int Znulls;              /* Number of nulls to send at beginning of ZDATA hdr */
extern char Attn[ZATTNLEN + 1]; /* Attention string rx sends to tx on err */

extern void zsendline(int c);
extern void zsendline_init(void);
void zsbhdr(int type, char *hdr);
void zshhdr(int type, char *hdr);
void zsdata(const char *buf, size_t length, int frameend);
void zsda32(const char *buf, size_t length, int frameend);
int zrdata(char *buf, int length, size_t * received);
int zgethdr(char *hdr, int eflag, size_t *);
void stohdr(size_t pos);
long rclhdr(char *hdr);
void readline_clean(void);
#endif

