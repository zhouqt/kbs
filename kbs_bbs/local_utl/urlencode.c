char bin2hex(int val)
{
    int i;

    i = val & 0x0F;
    if (i >= 0 && i < 10)
        return '0' + i;
    else
        return 'A' + (i - 10);
}

char *encode_url(char *buf, const char *str, size_t buflen)
{
    int i, j;
    int len;
    unsigned char c;
    int buflenm1;

    len = strlen(str);
    buf[buflen - 1] = '\0';
    buflenm1 = buflen - 1;
    for (i = 0, j = 0; i < len && j < buflenm1; i++) {
        c = (unsigned char) str[i];
        if (!isalnum(c)) {
            buf[j++] = '%';
            if (j < buflenm1)
                buf[j++] = bin2hex((c >> 4) & 0x0F);
            if (j < buflenm1)
                buf[j++] = bin2hex(c & 0x0F);
        } else {
            buf[j] = str[i];
            j++;
        }
    }
    buf[j] = '\0';

    return buf;
}

