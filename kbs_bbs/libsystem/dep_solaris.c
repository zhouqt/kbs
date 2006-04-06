/* etnlegend, 2006.04.05, solaris dependences */

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include "config.h"

#endif /* CONFIG_H_INCLUDED */


#ifndef HAVE_STRSEP

#include <string.h>

char* strsep(char **strptr,const char *delim){
    char *ptr;
    if(!(ptr=*strptr))
        return NULL;
    if(!*delim){
        *strptr=NULL;
        return ptr;
    }
    if(!(*strptr=(!*(delim+1)?strchr(*strptr,*delim):strpbrk(*strptr,delim))))
        return ptr;
    *((*strptr)++)=0;
    return ptr;
}

#endif /* HAVE_STRSEP */

