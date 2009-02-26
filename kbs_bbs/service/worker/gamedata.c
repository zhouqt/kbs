/*#include<iostream.h>*/
/*#include<fstream.h>*/
#include<stdio.h>
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"gamedefs.h"

/*int procpoint(char * buf, int & x, int & y, char sep = ',',*/
/*  int basex = 0, int basey = 0)*/
int procpoint(char * buf, int * x, int * y, char sep, int basex, int basey)
{
    char * ptr, * beg;
    int len;

    if (NULL == buf) return 0;
    len = strlen(buf);
    ptr = strchr(buf, sep);
    if (len < 1 || NULL == ptr) return 0;
    beg = buf;
    *ptr++ = 0;
    if ('(' == *beg) beg++;
    *x = atoi(beg);
    beg = buf + len-1;
    if (')' == *beg) *beg = 0;
    *y = atoi(ptr);
    if (*x < 0 || *y < 0) return 0;
    *x += basex;
    *y += basey;
    if (*x >= MaxXPos || *y >= MaxYPos) return 0;
    return 2;
}

/*int procline(char * buf, int & x1, int & y1, int & x2, int & y2,*/
/* int basex = 0, int basey = 0)*/
int procline(char * buf, int * x1, int * y1, int * x2, int * y2, int basex, int basey)
{
    char buf1[16], buf2[16];
    char * ptr;
    int len, pos;
    if (NULL == buf) return 0;
    len = strlen(buf);
    ptr = strchr(buf, '-');
    if (len < 1 || NULL == ptr) return 0;
    pos = ptr - buf;
    if (pos >= 16 || len - pos >= 16) return 0;
    strncpy(buf1, buf, pos);
    ptr ++;
    strncpy(buf2, ptr, len - pos - 1);
    buf1[pos] = 0;
    buf2[len-pos-1] = 0;
    pos = procpoint(buf1, x1, y1, ',', basex, basey);
    len = procpoint(buf2, x2, y2, ',', basex, basey);
    if (len != 2 || pos != 2) return 0;
    else return 4;
}

char * trim(char * buf, char tch)
{
    char * ptr, * pptr;
    int len;

    if (NULL == buf) return NULL;

    len = strlen(buf);
    if (0 == len) return buf;
    ptr = buf;
    while (*ptr) {
        if (tch == *ptr) {
            pptr = ptr;
            do {
                *pptr = *(pptr+1);
                pptr ++;
            } while (*pptr);
        } else ptr ++;
    }
    return buf;
}

char * stoupper(char * str)
{
    int len;
    char * ptr;
    if (NULL == str) return NULL;
    ptr = str;
    len = strlen(str);
    if (len < 1 || len > 1000) return NULL;
    while (*ptr && len) {
        len--;
        *ptr = toupper(*ptr);
        ptr++;
    }
    return str;
}

#if 0
char * stolower(char * str)
{
    int len;
    char * ptr;
    if (NULL == str) return NULL;
    ptr = str;
    len = strlen(str);
    if (len < 1 || len > 1000) return NULL;
    while (*ptr && len) {
        len--;
        *ptr = tolower(*ptr);
        ptr++;
    }
    return str;
}
#endif
