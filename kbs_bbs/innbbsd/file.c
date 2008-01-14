#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <string.h>
#include "inn_funcs.h"

#define MAXARGS     100

/*   isfile is called by
 *   isfile(filenamecomp1, filecomp2,  filecomp3, ..., (char *)0);
 *   extern "C" int isfile(const char *, const char *[]) ;
*/

char FILEBUF[4096];

static char DOLLAR_[8192];
char *getstream(fp)
FILE *fp;
{
    return fgets(DOLLAR_, sizeof(DOLLAR_) - 1, fp);
}

/*
    The same as sprintf, but return the new string
    fileglue("%s/%s",home,".newsrc");
*/

char *fileglue(char* fmt,...)
{
    va_list ap;
    static char *newstring;
    static char gluebuffer[8192];

    va_start(ap,fmt);

    vsprintf(gluebuffer, fmt, ap);
    newstring = gluebuffer;
    va_end(ap);
    return newstring;
}

size_t filesize(const char *filename){
    struct stat st;
    return (stat(filename,&st)==-1?0:st.st_size);
}

int iszerofile(const char *filename){
    struct stat st;
    return (!stat(filename,&st)&&!st.st_size);
}

int isfile(const char *filename){
    struct stat st;
    return !(stat(filename,&st)==-1||!S_ISREG(st.st_mode));
}

int isfilev(char* fmt,...)
{
    va_list ap;
    char *p;

    va_start(ap,fmt);

    FILEBUF[0] = '\0';
    strcat(FILEBUF, fmt);
    while ((p = va_arg(ap, char *)) != (char *) 0) {
        strcat(FILEBUF, p);
    }
    printf("file %s\n", FILEBUF);

    va_end(ap);
    return isfile(FILEBUF);
}

int isdir(const char *filename){
    struct stat st;
    return !(stat(filename,&st)==-1||!S_ISDIR(st.st_mode));
}

int isdirv(char *fmt,...)
{
    va_list ap;
    char *p;

    va_start(ap,fmt);

    FILEBUF[0] = '\0';
    strcpy(FILEBUF,fmt);
    while ((p = va_arg(ap, char *)) != (char *) 0) {
        strcat(FILEBUF, p);
    }

    va_end(ap);
    return isdir(FILEBUF);
}

unsigned long mtime(filename)
char *filename;
{
    struct stat st;

    if (stat(filename, &st))
        return 0;
    return st.st_mtime;
}

unsigned long mtimev(char *fmt,...)
{
    va_list ap;
    char *p;

    va_start(ap,fmt);

    FILEBUF[0] = '\0';
    strcpy(FILEBUF,fmt);
    while ((p = va_arg(ap, char *)) != (char *) 0) {
        strcat(FILEBUF, p);
    }

    va_end(ap);
    return mtime(FILEBUF);
}

unsigned long atime(filename)
char *filename;
{
    struct stat st;

    if (stat(filename, &st))
        return 0;
    return st.st_atime;
}

unsigned long atimev(char* fmt,...)
{
    va_list ap;
    char *p;

    va_start(ap,fmt);

    FILEBUF[0] = '\0';
    strcpy(FILEBUF,fmt);
    while ((p = va_arg(ap, char *)) != (char *) 0) {
        strcat(FILEBUF, p);
    }

    va_end(ap);
    return atime(FILEBUF);
}

/*#undef TEST*/
#ifdef TEST
main(argc, argv)
int argc;
char **argv;
{
    int i;

    if (argc > 3) {
        if (isfilev(argv[1], argv[2], (char *) 0))
            printf("%s %s %s is file\n", argv[1], argv[2], argv[3]);
        if (isdirv(argv[1], argv[2], (char *) 0))
            printf("%s %s %s is dir\n", argv[1], argv[2], argv[3]);
        printf("mtime %d\n", mtimev(argv[1], argv[2], (char *) 0));
        printf("atime %d\n", atimev(argv[1], argv[2], (char *) 0));
    }
    printf("fileglue %s\n", fileglue("%s/%s", "home", ".test"));
}
#endif
