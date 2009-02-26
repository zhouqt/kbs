#include "pip.h"

void clrnlines(int n);

enum { STRIP_ALL, ONLY_COLOR, NO_RELOAD };

int    //patch for MagicBBS by Magi
clrchyiuan(int start, int end)
{
    move(start, 0);
    clrnlines(end - start);
    return 0;
}

int
strip_ansi(buf, str, mode)
char *buf, *str;
int mode;
{
    register int ansi, count = 0;

    for (ansi = 0; *str /*&& *str != '\n' */ ; str++) {
        if (*str == 27) {
            if (mode) {
                if (buf)
                    *buf++ = *str;
                count++;
            }
            ansi = 1;
        } else if (ansi && strchr("[;1234567890mfHABCDnsuJKc=n", *str)) {
            if ((mode == NO_RELOAD && !strchr("c=n", *str))
                    || (mode == ONLY_COLOR
                        && strchr("[;1234567890m", *str))) {
                if (buf)
                    *buf++ = *str;
                count++;
            }
            if (strchr("mHn ", *str))
                ansi = 0;
        } else {
            ansi = 0;
            if (buf)
                *buf++ = *str;
            count++;
        }
    }
    if (buf)
        *buf = '\0';
    return count;
}

char *
Ptt_prints(char *str, int mode)
{
    strip_ansi(str, str, mode);
    return str;
}

int
show_file(char *filename, int y, int lines, int mode)
{
    FILE *fp;
    char buf[256];

    if (y >= 0)
        move(y, 0);

    if ((fp = fopen(filename, "r"))) {
        while (fgets(buf, 256, fp) && lines--)
            outs(Ptt_prints(buf, mode));
        fclose(fp);
    } else
        return 0;
    return 1;
}
