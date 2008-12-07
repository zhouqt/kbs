#include "system.h"

static const char uu_conv[64] = {
    '`', '!', '"', '#', '$', '%', '&', '\'',
    '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', ':', ';', '<', '=', '>', '?',
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', '[', '\\', ']', '^', '_'
};

#define ENCODE(ch) (uu_conv[(ch) & 077])

void uuencode(const char *inbuf, int size, const char *filename, OUTC_FUNC fn)
{
    register int ch, left;
    register const char *p = NULL, *ptr;
    int n;
    char buf[80];

    sprintf(buf, "begin 644 %s\n", filename);
    for (n = 0; n < strlen(buf); n++)
        (*fn)(buf[n]);

    left = size;
    ptr = inbuf;
    while (1) {
        n = 0;
        if (!left)
            break;
        n = left > 45 ? 45 : left;
        left -= n;
        if ((*fn)(ENCODE(n)) == EOF)
            break;
        for (p = ptr, ptr+=n; n > 2; n -= 3, p += 3) {
            ch = *p >> 2;
            ch = ENCODE(ch);
            if ((*fn)(ch) == EOF)
                break;
            ch = ENCODE(((*p << 4) & 060) | ((p[1] >> 4) & 017));
            if ((*fn)(ch) == EOF)
                break;
            ch = ENCODE(((p[1] << 2) & 074) | ((p[2] >> 6) & 03));
            if ((*fn)(ch) == EOF)
                break;
            ch = ENCODE((p[2] & 077));
            if ((*fn)(ch) == EOF)
                break;
        }

        if (n != 0)
            break;

        if ((*fn)('\n') == EOF)
            break;
    }

    while (n != 0) {
        char ch1 = p[0];
        char ch2 = (n == 1) ? 0 : p[1];

        ch = ENCODE(ch1 >> 2);
        if ((*fn)(ch) == EOF)
            break;

        ch = ENCODE(((ch1 << 4) & 060) | ((ch2 >> 4) & 017));
        if ((*fn)(ch) == EOF)
            break;

        if (n == 1)
            ch = ENCODE('\0');
        else {
            ch = ENCODE((ch2 << 2) & 074);
        }
        if ((*fn)(ch) == EOF)
            break;
        ch = ENCODE('\0');
        if ((*fn)(ch) == EOF)
            break;
        (*fn)('\n');
        break;
    }
    (*fn)(ENCODE('\0'));
    (*fn)('\n');
    (*fn)('e');
    (*fn)('n');
    (*fn)('d');
    (*fn)('\n');
    return;
}
