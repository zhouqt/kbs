/* $Id$ */

#ifndef lint
static char *rcs_id = "$Id$";
#endif                          /* lint */

#include "config.h"
#include "io.h"

/*** UTF8 => UNICODE ***/

static int u8toun(p, len, pu1, pu2)
char *p;
int len;
unsigned char *pu1, *pu2;
{
    register unsigned char c1 = p[0], c2 = p[1], c3 = p[2];

    if ((c1 & 0x80) == 0) {     /* ASCII character */
        *pu1 = 0;
        *pu2 = c1;
        return 1;
    }
    if ((c1 & 0xc0) == 0x80) {  /* unexpected tail character */
        return -1;
    }
    if ((c1 & 0xe0) == 0xc0) {  /* 110vvvvv 10vvvvvv */
        if (len < 2)            /* at least two characters are expected */
            return 0;           /* dangling */
        if ((c2 & 0xc0) != 0x80)        /* unexpected head character */
            return -2;
        *pu1 = (c1 & 0x1c) >> 2;
        *pu2 = ((c1 & 0x03) << 6) | (c2 & 0x3f);
        return 2;
    }
    if ((c1 & 0xf0) == 0xe0) {  /* 1110vvvv 10vvvvvv 10vvvvvv */
        if (len < 3)            /* at least three characters are expected */
            return 0;           /* dangling */
        if ((c2 & 0xc0) != 0x80)        /* unexpected head character */
            return -2;
        if ((c3 & 0xc0) != 0x80)        /* unexpected head character */
            return -3;
        *pu1 = ((c1 & 0x0f) << 4) | ((c2 & 0x3c) >> 2);
        *pu2 = ((c2 & 0x03) << 6) | (c3 & 0x3f);
        return 3;
    }
}

static char utf8_uni_savec[MAX_MODULE][2];

int utf8_uni_init(arg)
char *arg;
{
    static int utf8_uni_inst = 0;

    utf8_uni_savec[utf8_uni_inst][0] = '\0';
    utf8_uni_savec[utf8_uni_inst][1] = '\0';
    return (utf8_uni_inst++);
}

#define CODE_ERROR	0x80

char *utf8_uni(s, plen, inst)
char *s;
int *plen;
int inst;
{
    char buf[4096];
    char *s_start = s;
    register int len = *plen;
    register char *p = buf;

    if (len == 0)
        return (s);
    if (utf8_uni_savec[inst][0]) {
        if (utf8_uni_savec[inst][1]) {
            *(--s) = utf8_uni_savec[inst][1];
            len++;
            utf8_uni_savec[inst][1] = 0;
        }
        *(--s) = utf8_uni_savec[inst][0];
        len++;
        utf8_uni_savec[inst][0] = 0;
    }
    bcopy(s, p, len);

    s = s_start;
    while (len > 0) {
        int nc = u8toun(p, len, s, s + 1);

        if (nc > 0) {
            s++;
            s++;
            p += nc;
            len -= nc;
        } else if (nc < 0) {    /* bad character */
            *s++ = 0;
            *s++ = CODE_ERROR;
            p += (-nc);
            len -= (-nc);
        } else {                /* dangling */
            if (len <= 2)
                utf8_uni_savec[inst][0] = *p++;
            if (len == 1)
                utf8_uni_savec[inst][1] = *p++;
            break;
        }
    }
    *plen = s - s_start;
    return s_start;
}

/*** UNICODE => UTF8 ***/

static char uni_utf8_savec[MAX_MODULE];
static char uni_utf8_saved[MAX_MODULE];

int uni_utf8_init(arg)
char *arg;
{
    static int uni_utf8_inst = 0;

    uni_utf8_saved[uni_utf8_inst] = 0;
    return (uni_utf8_inst++);
}

char *uni_utf8(s, plen, inst)
char *s;
int *plen;
int inst;
{
    char buf[4096];
    char *s_start = s;
    register int len = *plen;
    register char *p = buf;

    if (len == 0)
        return (s);
    if (uni_utf8_saved[inst]) {
        *(--s) = uni_utf8_savec[inst];
        len++;
        uni_utf8_saved[inst] = 0;
    }
    bcopy(s, p, len);

    s = s_start;
    while (len >= 2) {
        if ((p[0] == 0) && ((p[1] & 0x80) == 0)) {      /* ASCII */
            *s++ = p[1];
        } else if ((p[0] & 0xf8) == 0) {        /* 0x0080 - 0x07ff */
            *s++ = 0xc0 | (p[0] << 2) | ((p[1] >> 6) & 0x03);
            *s++ = 0x80 | (p[1] & 0x3f);
        } else {                /* 0x0800 - 0xffff */
            *s++ = 0xe0 | ((p[0] >> 4) & 0x0f);
            *s++ = 0x80 | ((p[0] & 0x0f) << 2) | ((p[1] >> 6) & 0x03);
            *s++ = 0x80 | (p[1] & 0x3f);
        }
        p++;
        p++;
        len--;
        len--;
    }
    if (len == 1) {             /* dangling */
        uni_utf8_saved[inst] = 1;
        uni_utf8_savec[inst] = *p;
    }
    *plen = s - s_start;
    return s_start;
}

/*********** UTF 7 **************/

/* ASCII subsets */
static unsigned char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define	SAFE_CHARS	"'(),-.:?"
#define	SPACE_CHARS	" \t\n\r"

/* character classes */
#define	BASE64		0x01
#define	SAFE		0x02
#define	SPACE		0x04

/* base64 state */
#define	IN_ASCII	0
#define	IN_BASE64	1
#define AFTER_PLUS	2

static char inv_base64[128];
static char char_type[256];

static void init_utf7_tables()
{
    register unsigned char *s;
    static first_time = 1;

    if (!first_time)
        return;

    for (s = base64; *s != '\0'; s++) {
        char_type[*s] |= BASE64;
        inv_base64[*s] = s - base64;
    }
    for (s = SAFE_CHARS; *s != '\0'; s++)
        char_type[*s] |= SAFE;
    for (s = SPACE_CHARS; *s != '\0'; s++)
        char_type[*s] |= SPACE;

    first_time = 0;
}

struct utf7_context {
    short int state;            /* state in the base64 */
    short int nbits;            /* number of bits in the bit buffer */
    unsigned long bit_buffer;
};

static int u7toun(c, pu1, pu2, ctx)     /* return 1 if a unicode is produced */
unsigned char c;                /* a char in utf7 stream */
unsigned char *pu1, *pu2;       /* the unicode */
struct utf7_context *ctx;       /* the context */
{
    if (ctx->state == IN_ASCII) {
        if (c == '+') {
            ctx->state = AFTER_PLUS;
            return 0;
        } else {
            *pu1 = 0;
            *pu2 = c;
            return 1;
        }
    }
    if (ctx->state == AFTER_PLUS) {
        if (c == '-') {
            *pu1 = 0;
            *pu2 = '+';
            return 1;
        } else {
            ctx->state = IN_BASE64;
            ctx->nbits = 0;
            /*
             * don't return yet, continue to the IN_BASE64 mode 
             */
        }
    }

    /*
     * now we're in Base64 mode 
     */
    if (char_type[c] & BASE64) {
        ctx->bit_buffer <<= 6;
        ctx->bit_buffer |= inv_base64[c];
        ctx->nbits += 6;
        if (ctx->nbits >= 16) {
            ctx->nbits -= 16;
            *pu1 = (char) ((ctx->bit_buffer >> (ctx->nbits + 8))
                           & 0x00ff);
            *pu2 = (char) ((ctx->bit_buffer >> ctx->nbits)
                           & 0x00ff);
            return 1;
        }
        return 0;
    }
    ctx->state = IN_ASCII;
    if (c != '-') {
        *pu1 = 0;
        *pu2 = c;
        return 1;
    }
    return 0;
}

static int untou7(u1, u2, s, ctx)       /* return the # of char written to s */
unsigned char u1, u2;           /* the unicode */
unsigned char *s;               /* store here */
struct utf7_context *ctx;       /* the context */
{
    unsigned char *prev_s = s;

    if (((u1 == 0) && ((u2 & 0x80) == 0)) && (char_type[u2] & (BASE64 | SAFE | SPACE))) {       /* safe ASCII */
        if (ctx->state == IN_BASE64) {
            if (ctx->nbits > 0)
                *s++ = base64[(ctx->bit_buffer << (6 - ctx->nbits)) & 0x3f];
            if ((char_type[u2] & BASE64) || u2 == '-')
                *s++ = '-';
            ctx->state = IN_ASCII;
        }
        *s++ = u2;
        if (u2 == '+')
            *s++ = '-';
    } else {
        if (ctx->state == IN_ASCII) {
            *s++ = '+';
            ctx->state = IN_BASE64;
            ctx->nbits = 0;
        }
        ctx->bit_buffer <<= 8;
        ctx->bit_buffer |= u1;
        ctx->bit_buffer <<= 8;
        ctx->bit_buffer |= u2;
        ctx->nbits += 16;
        while (ctx->nbits >= 6) {
            ctx->nbits -= 6;
            *s++ = base64[(ctx->bit_buffer >> ctx->nbits) & 0x3f];
        }
    }
    return (s - prev_s);        /* return the length */
}

/***** UTF7 => UNICODE *****/

static struct utf7_context utf7_uni_context[MAX_MODULE];

int utf7_uni_init(arg)
char *arg;
{
    static int utf7_uni_inst = 0;

    init_utf7_tables();
    utf7_uni_context[utf7_uni_inst].state = IN_ASCII;
    utf7_uni_context[utf7_uni_inst].nbits = 0;
    utf7_uni_context[utf7_uni_inst].bit_buffer = 0L;
    return (utf7_uni_inst++);
}

char *utf7_uni(s, plen, inst)
char *s;
int *plen;
int inst;
{
    char buf[4096];
    char *s_start = s;
    register int len = *plen;
    register char *p = buf;

    if (len == 0)
        return (s);
    bcopy(s, p, len);

    while (len-- > 0) {
        if (u7toun(*p++, s, s + 1, &(utf7_uni_context[inst]))) {
            s++;
            s++;
        }
    }
    *plen = s - s_start;
    return s_start;
}

/***** UNICODE => UTF7 *****/

static char uni_utf7_savec[MAX_MODULE];
static char uni_utf7_saved[MAX_MODULE];
static struct utf7_context uni_utf7_context[MAX_MODULE];

int uni_utf7_init(arg)
char *arg;
{
    static int uni_utf7_inst = 0;

    init_utf7_tables();
    uni_utf7_saved[uni_utf7_inst] = 0;
    uni_utf7_context[uni_utf7_inst].state = IN_ASCII;
    uni_utf7_context[uni_utf7_inst].nbits = 0;
    uni_utf7_context[uni_utf7_inst].bit_buffer = 0L;
    return (uni_utf7_inst++);
}

char *uni_utf7(s, plen, inst)
char *s;
int *plen;
int inst;
{
    char buf[4096];
    char *s_start = s;
    register int len = *plen;
    register char *p = buf;

    if (len == 0)
        return (s);
    if (uni_utf7_saved[inst]) {
        *(--s) = uni_utf7_savec[inst];
        len++;
        uni_utf7_saved[inst] = 0;
    }
    bcopy(s, p, len);

    s = s_start;
    while (len >= 2) {
        s += untou7(p[0], p[1], s, &(uni_utf7_context[inst]));
        len--;
        len--;
        p++;
        p++;
    }
    if (len == 1) {             /* dangling */
        uni_utf7_saved[inst] = 1;
        uni_utf7_savec[inst] = *p;
    }
    *plen = s - s_start;
    return s_start;
}
