/*

strerror.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Mar 22 18:18:21 1995 ylo

Replacement for strerror for systems that don't have it.

*/

#include <stdio.h>
#include <errno.h>

extern int sys_nerr;
extern char *sys_errlist[];

char *strerror(int error_number)
{
    if (error_number >= 0 && error_number < sys_nerr)
        return sys_errlist[error_number];
    else
        return "Bad error code";
}
