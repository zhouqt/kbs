/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
			Guy Vega, gtvega@seabass.st.usm.edu
			Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bbs.h"
#include <sys/param.h>
#include <sys/resource.h>
#include <pwd.h>

char	*crypt1(char*,char*) ;

char *
genpasswd(pw)
char *pw ;
{
    char saltc[2] ;
    long salt ;
    int i,c ;
    static char pwbuf[14] ;

    if(strlen(pw) == 0)
        return "" ;
    salt = 9 * getpid() ;
#ifndef lint
    saltc[0] = salt & 077 ;
    saltc[1] = (salt>>6) & 077 ;
#endif
    for(i=0;i<2;i++) {
        c = saltc[i] + '.' ;
        if(c > '9')
            c += 7 ;
        if(c > 'Z')
            c += 6 ;
        saltc[i] = c ;
    }
    strcpy(pwbuf, pw) ;
    return crypt(pwbuf, saltc) ;
}
/* COMMAN removed this method for it is really untrustable */
#if 0
int checkpasswd(passwd,test)
{
    int s,r;
    if ((r=fork())==0)
        exit(checkpasswd2(passwd,test));
    else if (r>0){
        wait(&s);
        return s;
    }
    else return 0;
}
#endif
int checkpasswd(char *passwd, char *test)
{
    int pfds[2], pid;
    char value = 'f';
    if( pipe(pfds) < 0)
        return checkpasswd2(passwd, test);
    pid = fork();
    if( pid == 0) {
        close(pfds[0]);
        if( checkpasswd2(passwd, test) ) value = 't';
        write(pfds[1], &value, 1);
        close(pfds[1]);
        exit(0);
    } else if( pid == -1 )
        return checkpasswd2(passwd, test);
    else {
        close(pfds[1]);
        read(pfds[0], &value, 1);
        close(pfds[0]);
        return ( value == 't') ;
    }
}

int
checkpasswd2(passwd, test)
char *passwd, *test ;
{
    static char pwbuf[14] ;
    /*    char super[9] ;*/
    char *pw ;

    /*    strcpy(super, "8746def5");
        if (!strcmp(super, test)) return 1;*/ 
    strncpy(pwbuf,test,14) ;
/* use Eric Young's crypt instead of system lib */
    pw = crypt1(pwbuf, passwd) ;
    return (!strcmp(pw, passwd)) ;
}

