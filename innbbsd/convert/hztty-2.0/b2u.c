/* $Id$ */

#ifndef lint
static char *rcs_id = "$Id$";
#endif                          /* lint */

#include "config.h"
#include "io.h"

#define	BtoU_bad1 0xa1
#define	BtoU_bad2 0xf5

extern unsigned char BtoU[], UtoB[];
extern int BtoU_count, UtoB_count;

#define	c1	(unsigned char)(s[0])
#define	c2	(unsigned char)(s[1])

static void b2u(s, t)
register char *s, *t;
{
    register unsigned int i;

    if ((c2 >= 0xa1) && (c2 <= 0xfe)) {
        if ((c1 >= 0xa1) && (c1 <= 0xa9)) {
            i = ((c1 - 0xa1) * 94 + (c2 - 0xa1)) * 2;
            t[0] = BtoU[i++];
            t[1] = BtoU[i];
            return;
        } else if ((c1 >= 0xb0) && (c1 <= 0xf7)) {
            i = ((c1 - 0xb0 + 9) * 94 + (c2 - 0xa1)) * 2;
            t[0] = BtoU[i++];
            t[1] = BtoU[i];
            return;
        }
    }
    t[0] = BtoU_bad1;
    t[1] = BtoU_bad2;
}

static void u2b(s, t)
register char *s, *t;
{
    register long int i = ((c1 << 8) + c2) * 2;

    t[0] = UtoB[i++];
    t[1] = UtoB[i];
}

#undef c1
#undef c2

static char big2uni_savec[MAX_MODULE];
static char uni2big_savec[MAX_MODULE];

int big2uni_init(arg)
char *arg;
{
    static int big2uni_inst = 0;

    big2uni_savec[big2uni_inst] = '\0';
    return (big2uni_inst++);
}

int uni2big_init(arg)
char *arg;
{
    static int uni2big_inst = 0;

    uni2big_savec[uni2big_inst] = '\0';
    return (uni2big_inst++);
}

char *big2uni(s, plen, inst)
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
    if (big2uni_savec[inst]) {
        *(--s) = big2uni_savec[inst];
        len++;
        big2uni_savec[inst] = 0;
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
            big2uni_savec[inst] = *p;
            break;
        }
        b2u(p, s);
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

char *uni2big(s, plen, inst)
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
    if (uni2big_savec[inst]) {
        *(--s) = uni2big_savec[inst];
        len++;
        uni2big_savec[inst] = 0;
    }
    bcopy(s, p, len);

    s = s_start;
    while (len >= 2) {
        if ((p[0] == 0) && ((p[1] & 0x80) == 0)) {      /* ASCII */
            *s++ = p[1] & 0x7f;
        } else {
            u2b(p, s);
            s++;
            s++;
        }
        p++;
        p++;
        len--;
        len--;
    }
    if (len == 1)               /* dangling */
        uni2big_savec[inst] = *p;
    *plen = s - s_start;
    return s_start;
}
