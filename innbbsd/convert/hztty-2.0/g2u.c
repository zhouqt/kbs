/* $Id$ */

#ifndef lint
static char *rcs_id = "$Id$";
#endif                          /* lint */

#include "config.h"
#include "io.h"

#define	GtoU_bad1 0xa1
#define	GtoU_bad2 0xbc

extern unsigned char GtoU[], UtoG[];
extern int GtoU_count, UtoG_count;

#define	c1	(unsigned char)(s[0])
#define	c2	(unsigned char)(s[1])

static void g2u(s, t)
register char *s, *t;
{
    register unsigned int i;

    if ((c2 >= 0xa1) && (c2 <= 0xfe)) {
        if ((c1 >= 0xa1) && (c1 <= 0xa9)) {
            i = ((c1 - 0xa1) * 94 + (c2 - 0xa1)) * 2;
            t[0] = GtoU[i++];
            t[1] = GtoU[i];
            return;
        } else if ((c1 >= 0xb0) && (c1 <= 0xf7)) {
            i = ((c1 - 0xb0 + 9) * 94 + (c2 - 0xa1)) * 2;
            t[0] = GtoU[i++];
            t[1] = GtoU[i];
            return;
        }
    }
    t[0] = GtoU_bad1;
    t[1] = GtoU_bad2;
}

static void u2g(s, t)
register char *s, *t;
{
    register long int i = ((c1 << 8) + c2) * 2;

    t[0] = UtoG[i++];
    t[1] = UtoG[i];
}

#undef c1
#undef c2

static char gb2uni_savec[MAX_MODULE];
static char uni2gb_savec[MAX_MODULE];

int gb2uni_init(arg)
char *arg;
{
    static int gb2uni_inst = 0;

    gb2uni_savec[gb2uni_inst] = '\0';
    return (gb2uni_inst++);
}

int uni2gb_init(arg)
char *arg;
{
    static int uni2gb_inst = 0;

    uni2gb_savec[uni2gb_inst] = '\0';
    return (uni2gb_inst++);
}

char *gb2uni(s, plen, inst)
char *s;
int *plen;
int inst;
{
    char buf[4096];             /* big enough */
    char *s_start = s;
    register int len = *plen;
    register char *p = buf;

    if (*plen == 0)
        return (s);
    if (gb2uni_savec[inst]) {
        *(--s) = gb2uni_savec[inst];
        len++;
        gb2uni_savec[inst] = 0;
    }
    bcopy(s, p, len);

    s = s_start;
    while (len > 0) {
        if (((*p) & 0x80) == 0) {       /* ASCII */
            *s++ = 0;
            *s++ = *p++;
            len--;
            continue;
        }
        if (len == 1) {         /* dangling */
            gb2uni_savec[inst] = *p;
            break;
        }
        g2u(p, s);
        s++;
        s++;
        p++;
        p++;
        len--;
        len--;
    }
    *plen = s - s_start;
    return s_start;
}

char *uni2gb(s, plen, inst)
char *s;
int *plen;
int inst;
{
    char buf[4096];             /* big enough */
    char *s_start = s;
    register int len = *plen;
    register char *p = buf;

    if (len == 0)
        return (s);
    if (uni2gb_savec[inst]) {
        *(--s) = uni2gb_savec[inst];
        len++;
        uni2gb_savec[inst] = 0;
    }
    bcopy(s, p, len);

    s = s_start;
    while (len >= 2) {
        if ((p[0] == 0) && ((p[1] & 0x80) == 0)) {      /* ASCII */
            *s++ = p[1] & 0x7f;
        } else {
            u2g(p, s);
            s++;
            s++;
        }
        p++;
        p++;
        len--;
        len--;
    }
    if (len == 1)               /* dangling */
        uni2gb_savec[inst] = *p;
    *plen = s - s_start;
    return s_start;
}
