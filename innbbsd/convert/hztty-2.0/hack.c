/* $Id$ */

#ifndef lint
static char *rcs_id = "$Id$";
#endif                          /* lint */

#include "config.h"
#include "io.h"

#include <ctype.h>

int rot13_init()
{
    return (0);
}

char *rot13(s, plen, inst)
char *s;
int *plen;
int inst;
{
    register int i;

    for (i = 0; i < *plen; i++)
        s[i] = (('A' <= s[i]) && (s[i] <= 'M') || ('a' <= s[i]) && (s[i] <= 'm')) ? (s[i] + 13) : (('N' <= s[i]) && (s[i] <= 'Z') || ('n' <= s[i]) && (s[i] <= 'z')) ? (s[i] - 13) : s[i];
    return (s);
}

int toascii_init()
{
    return (0);
}

char *to_ascii(s, plen, inst)
char *s;
int *plen;
int inst;
{
    register int i;

    for (i = 0; i < *plen; i++)
        s[i] &= 0x7f;
    return (s);
}

int toupper_init()
{
    return (0);
}

char *to_upper(s, plen, inst)
char *s;
int *plen;
int inst;
{
    register int i;

    for (i = 0; i < *plen; i++)
        s[i] = toupper(s[i]);
    return (s);
}

int tolower_init()
{
    return (0);
}

char *to_lower(s, plen, inst)
char *s;
int *plen;
int inst;
{
    register int i;

    for (i = 0; i < *plen; i++)
        s[i] = tolower(s[i]);
    return (s);
}
