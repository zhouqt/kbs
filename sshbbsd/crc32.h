/*

crc32.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1992 Tatu Ylonen, Espoo, Finland
                   All rights reserved

Created: Tue Feb 11 14:37:27 1992 ylo

Functions for computing 32-bit CRC.

*/

/*
 * $Id$
 * $Log$
 * Revision 1.4  2002/08/22 15:42:52  kcn
 * fix bug
 *
 * Revision 1.3  2002/08/04 11:39:41  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:46  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:25  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:09  bbsdev
 * bbs sshd
 *
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.2  1995/07/13  01:21:45  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */

#ifndef CRC32_H
#define CRC32_H

/* This computes a 32 bit CRC of the data in the buffer, and returns the
   CRC.  The polynomial used is 0xedb88320. */
unsigned long ssh_crc32(const unsigned char *buf, unsigned int len);

#endif                          /* CRC32_H */
