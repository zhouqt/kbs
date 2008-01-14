#include <iconv.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

/* ----------------------------------------------------- */
/* QP code : "0123456789ABCDEF"                          */
/* ----------------------------------------------------- */

static int qp_code(x)
register int x;
{
    if (x >= '0' && x <= '9')
        return x - '0';
    if (x >= 'a' && x <= 'f')
        return x - 'a' + 10;
    if (x >= 'A' && x <= 'F')
        return x - 'A' + 10;
    return -1;
}

/* ------------------------------------------------------------------ */
/* BASE64 :                                                           */
/* "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" */
/* ------------------------------------------------------------------ */

static int base64_code(x)
register int x;
{
    if (x >= 'A' && x <= 'Z')
        return x - 'A';
    if (x >= 'a' && x <= 'z')
        return x - 'a' + 26;
    if (x >= '0' && x <= '9')
        return x - '0' + 52;
    if (x == '+')
        return 62;
    if (x == '/')
        return 63;
    return -1;
}

int ignorestr(char *str)
{
    char *s;

    for (s = str; *s != '\0'; s++) {
        if (!isalnum(*s) && !ispunct(*s) && !isspace(*s))
            return 1;
    }
    return 0;
}

/* ----------------------------------------------------- */
/* judge & decode QP / BASE64                            */
/* ----------------------------------------------------- */
/* 这个函式会将一个字串 (src) 从 charset=fromcode 转成 charset=tocode,
 * srclen 是 src 的长度, dst 是输出的buffer, dstlen 则指定了
 * dst 的大小, 最后会补 '\0', 所以要留一个byte给'\0'.
 * 如果遇到 src 中有非字集的字, 或是 src 中有未完整的 byte,
 * 都会砍掉.
 * */

int str_iconv(fromcode, tocode, src, srclen, dst, dstlen)
char *fromcode;                 /* charset of source string */
char *tocode;                   /* charset of destination string */
char *src;                      /* source string */
size_t srclen;                     /* source string length */
char *dst;                      /* destination string */
size_t dstlen;                     /* destination string length */
{
    iconv_t iconv_descriptor;
    int iconv_ret, dstlen_old;

    dstlen--;                   /* keep space for '\0' */
    dstlen_old = dstlen;

/* Open a descriptor for iconv */
    iconv_descriptor = iconv_open(tocode, fromcode);

    if (iconv_descriptor == ((iconv_t) (-1))) { /* if open fail */
        strncpy(dst, src, dstlen);
        return dstlen;
    }

/* Start translation */
    while (srclen > 0 && dstlen > 0) {
        iconv_ret = iconv(iconv_descriptor, (void *) &src, &srclen, &dst, &dstlen);
        if (iconv_ret != 0) {
            switch (errno) {
/* invalid multibyte happened */
            case EILSEQ:
/* delete that byte */
                *dst = *src;
                src++;
                srclen--;
                dst++;
                dstlen--;
                break;
/* incomplete multibyte happened */
            case EINVAL:
/* delete that byte (maybe wrong) */
                *dst = *src;
                src++;
                srclen--;
                dst++;
                dstlen--;
                break;
/* dst no rooms */
            case E2BIG:
/* break out the while loop */
                srclen = 0;
                break;
            }
        }
    }
    *dst = '\0';
/* close descriptor of iconv */

    iconv_close(iconv_descriptor);
    return (dstlen_old - dstlen);
}

void str_decode(unsigned char *dst, unsigned char *src)
{
    register int is_qp, is_base64, is_done;
    register int c1, c2, c3, c4;

    char *dsttmp;
    char *srctmp;
    char srcbuf[256];
    long pos;

    if (ignorestr((char*)src)) {
        strcpy((char*)dst, (char*)src);
        return;
    }
    memcpy(srcbuf, src, 256);
    dsttmp = (char *)dst;
    srctmp = (char *)src;
    pos = (long) src - (long) srctmp;

    for (is_done = is_qp = is_base64 = 0; (c1 = *src) != 0; src++) {
        pos = (long) src - (long) srctmp;
        if (pos > 254) {
            memcpy(dsttmp, srcbuf, 256);
            return;
        }
        if (c1 == '?' && src[1] == '=') {
            src++;
            continue;
        } else if (c1 == '\n') {        /* chuan: multi line encoding */
            src++;
            is_done = is_qp = is_base64 = 0;
            continue;
        } else if (is_qp && c1 == '=') {
            pos = (long) src - (long) srctmp;
            if (pos > 254) {
                memcpy(dsttmp, srcbuf, 256);
                return;
            }
            c1 = *++src;

            pos = (long) src - (long) srctmp;
            if (pos > 254) {
                memcpy(dsttmp, srcbuf, 256);
                return;
            }
            c2 = *++src;
            *dst++ = (qp_code(c1) << 4) | qp_code(c2);
        } else if (is_base64 && !is_done) {
            while (isspace(c1)) {
                pos = (long) src - (long) srctmp;
                if (pos > 254) {
                    memcpy(dsttmp, srcbuf, 256);
                    return;
                }
                c1 = *++src;
            }
            if (!c1)
                break;
            do {
                pos = (long) src - (long) srctmp;
                if (pos > 254) {
                    memcpy(dsttmp, srcbuf, 256);
                    return;
                }
                c2 = *++src;
            } while (isspace(c2));
            if (!c2)
                break;
            do {
                pos = (long) src - (long) srctmp;
                if (pos > 254) {
                    memcpy(dsttmp, srcbuf, 256);
                    return;
                }
                c3 = *++src;
            } while (isspace(c3));
            if (!c3)
                break;
            do {
                pos = (long) src - (long) srctmp;
                if (pos > 254) {
                    memcpy(dsttmp, srcbuf, 256);
                    return;
                }
                c4 = *++src;
            } while (isspace(c4));
            if (!c4)
                break;
            if (c1 == '=' || c2 == '=') {
                is_done = 1;
                continue;
            }
            c2 = base64_code(c2);
            *dst++ = (base64_code(c1) << 2) | ((c2 & 0x30) >> 4);
            if (c3 == '=')
                is_done = 1;
            else {
                c3 = base64_code(c3);
                *dst++ = ((c2 & 0xF) << 4) | ((c3 & 0x3c) >> 2);
                if (c4 == '=')
                    is_done = 1;
                else {
                    *dst++ = ((c3 & 0x03) << 6) | base64_code(c4);
                }
            }
        } else if ((c1 == '=') && (src[1] == '?')) {
            /*
             * c2 : qmarks, c3 : code_kind 
             */

            c2 = c3 = 0;

            for (;;) {
                pos = (long) src - (long) srctmp;
                if (pos > 254) {
                    memcpy(dsttmp, srcbuf, 256);
                    return;
                }
                c1 = *++src;
                if (c1 != '?') {
                    if (c2 == 2)
                        c3 = c1 | 0x20;
                } else {
                    if (++c2 >= 3)
                        break;
                }
            }

            if (c3 == 'q')
                is_qp = 1;
            else if (c3 == 'b')
                is_base64 = 1;
        } else
            *dst++ = c1;
    }
    *dst = '\0';
}
