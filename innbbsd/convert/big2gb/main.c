#include <stdio.h>

typedef unsigned int (conv_func_t) (unsigned int);

extern conv_func_t fromgb;
extern conv_func_t togb;
extern conv_func_t frombig;
extern conv_func_t tobig;
extern conv_func_t s2t;
extern conv_func_t t2s;
extern conv_func_t v2z;

static unsigned int nochar;

int isgb2312(unsigned int gbchar)
{
    unsigned char lead = (gbchar >> 8) & 0xff;
    unsigned char trail = (gbchar) & 0xff;

    return (lead >= 0xa1 && lead <= 0xfe && trail >= 0xa1 && trail <= 0xfe);
}

int main(int argc, char *argv[])
{
    unsigned char ch;
    unsigned char lead = 0;
    int c;
    conv_func_t *fromfunc;
    conv_func_t *tofunc;
    conv_func_t *stfunc;

    fromfunc = frombig;
    tofunc = togb;
    stfunc = t2s;

    while ((c = getopt(argc, argv, "r")) != -1) {
        switch (c) {
        case 'r':
            fromfunc = fromgb;
            tofunc = tobig;
            stfunc = s2t;
            break;
        default:
            fprintf(stderr, "usage: %s -r\n", argv[0]);
            fprintf(stderr, "\t-r: From GB to BIG5 (default: from BIG5 to GB)\n");
            return -1;
        }
    }

    nochar = tofunc(0);
    ch = getchar();
    while (!feof(stdin)) {
        if (lead) {
            unsigned int tmpchar = fromfunc((lead << 8) | ch);
            unsigned int mb = tofunc(stfunc(tmpchar));
            unsigned int mbvz = tofunc(stfunc(v2z(tmpchar)));

            if (mb == nochar)
                mb = mbvz;
            else if (tofunc == togb && mbvz != nochar && !isgb2312(mb))
                mb = mbvz;
            putchar((mb >> 8) & 0xff);
            putchar(mb & 0xff);
            lead = 0;
        } else if ((ch & 0x80)) {
            lead = ch;
        } else
            putchar(ch);
        ch = getchar();
    }
    if (lead) {
        putchar((nochar >> 8) & 0xff);
        putchar(nochar & 0xff);
    }
    return 0;
}
