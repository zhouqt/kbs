/*

  Author: Tomi Salo <ttsalo@ssh.fi>

  Copyright (C) 1996 SSH Communications Security Oy, Espoo, Finland
  See COPYING for distribution conditions.

  Header file for snprintf.c

  */

/*
 * $Id:
 * $Log$
 * Revision 1.1  2002/04/27 05:47:26  kxn
 * Initial revision
 *
 * Revision 1.1  2001/07/04 06:07:12  bbsdev
 * bbs sshd
 *
 * Revision 1.1  1999/02/21 19:52:38  ylo
 * 	Intermediate commit of ssh1.2.27 stuff.
 * 	Main change is sprintf -> snprintf; however, there are also
 * 	many other changes.
 *
 * Revision 1.7  1998/10/04 02:54:00  ylo
 *      Removed #include "sshincludes.h"
 *
 * Revision 1.6  1998/06/24 13:46:46  kivinen
 *      Fixed Log entry.
 *
 * $EndLog$
 */

#ifndef SNPRINTF_H
#define SNPRINTF_H

/* Write formatted text to buffer 'str', using format string 'format'.
   Returns number of characters written, or negative if error
   occurred. SshBuffer's size is given in 'size'. Format string is
   understood as defined in ANSI C.

   NOTE: This does NOT work identically with BDS's snprintf.

   Integers: Ansi C says that precision specifies the minimun
   number of digits to print. BSD's version however counts the
   prefixes (+, -, ' ', '0x', '0X', octal prefix '0'...) as
   'digits'.

   Also, BSD implementation does not permit padding integers
   to specified width with zeros on left (in front of the prefixes),
   it uses spaces instead, even when Ansi C only forbids padding
   with zeros on the right side of numbers.
   
   */

int snprintf(char *str, size_t size, const char *format, ...);

int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#endif /* SNPRINTF_H */
