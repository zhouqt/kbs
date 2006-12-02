/*

  Author: Tomi Salo <ttsalo@ssh.fi>

  Copyright (C) 1996 SSH Communications Security Oy, Espoo, Finland
  See COPYING for distribution conditions.

  Header file for snprintf.c

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

#endif                          /* SNPRINTF_H */
