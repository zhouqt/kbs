/*

strerror.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Mar 22 18:18:21 1995 ylo

Replacement for strerror for systems that don't have it.

*/

/*
 * $Id$
 * $Log$
 * Revision 1.1  2002/04/27 05:47:26  kxn
 * Initial revision
 *
 * Revision 1.1  2001/07/04 06:07:13  bbsdev
 * bbs sshd
 *
 * Revision 1.2  1998/05/12 22:14:46  ylo
 * 	Fixed strerror to never return NULL.
 *
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.2  1995/07/13  01:40:55  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
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
