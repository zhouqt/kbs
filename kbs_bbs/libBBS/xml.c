#include "bbs.h"

/*
 * buflen is a value-result variable. When it is passed to the function,
 * its value is the buffer length (including the trailing '\0' character).
 * When the function returned, its value is the number of characters 
 * actually copied to buf (excluding the trailing '\0' character).
*/
char *string_copy(char *buf, const char *str, size_t * buflen)
{
    size_t i;
    size_t len;

    if (*buflen == 0)
        return buf;
    len = *buflen - 1;
    for (i = 0; i < len; i++) {
        if (str[i] == '\0') {
            buf[i] = str[i];
            break;
        }
        buf[i] = str[i];
    }
    *buflen = i;
    buf[i] = '\0';

    return buf;
}

char *encode_xml(char *buf, const char *str, size_t buflen)
{
    size_t i, j, k;
    size_t len;

    bzero(buf, buflen);
    len = strlen(str);
    for (i = 0, j = 0; i < len && j < buflen; i++) {
        switch (str[i]) {
        case '\"':
            k = buflen - j;
            string_copy(&buf[j], "&quot;", &k);
            j += k;
            break;
        case '\'':
            k = buflen - j;
            string_copy(&buf[j], "&apos;", &k);
            j += k;
            break;
        case '&':
            k = buflen - j;
            string_copy(&buf[j], "&amp;", &k);
            j += k;
            break;
        case '>':
            k = buflen - j;
            string_copy(&buf[j], "&gt;", &k);
            j += k;
            break;
        case '<':
            k = buflen - j;
            string_copy(&buf[j], "&lt;", &k);
            j += k;
            break;
        default:
            buf[j] = str[i];
            j++;
        }
    }
    buf[buflen - 1] = '\0';

    return buf;
}

