/*-
 * Copyright (c) 2001 Andrew L. Neporada <andr@dgap.mipt.ru>
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "bbs.h"
#ifndef SOLARIS
#include <sys/cdefs.h>
#endif

#include <sys/types.h>
#include <limits.h>
#include <string.h>

#ifndef HAVE_MEMMEM
/*
 * Find the first occurrence of byte string p in byte string s.
 *
 * This implementation uses simplified Boyer-Moore algorithm (only
 * bad-character shift table is used).
 * See:
 * Boyer R.S., Moore J.S. 1977, "A fast string searching algorithm",
 * Communications of ACM. 20:762-772.
 */
void *memmem(s, slen, p, plen)
register const void *s, *p;
size_t slen, plen;
{
    register const u_char *str, *substr;
    register size_t i, max_shift, curr_shift;

    size_t shift[UCHAR_MAX + 1];

    if (!plen)
        return ((void *) s);
    if (plen > slen)
        return (NULL);

    str = (const u_char *) s;
    substr = (const u_char *) p;

    for (i = 0; i <= UCHAR_MAX; i++)
        shift[i] = plen + 1;
    for (i = 0; i < plen; i++)
        shift[substr[i]] = plen - i;

    i = 0;
    max_shift = slen - plen;
    while (i <= max_shift) {
        if (*str == *substr && !memcmp(str + 1, substr + 1, plen - 1))
            return ((void *) str);
        curr_shift = shift[str[plen]];
        str += curr_shift;
        i += curr_shift;
    }
    return (NULL);
}
#endif /* ! HAVE_MEMMEM */
