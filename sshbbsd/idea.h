/*

idea.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sun Jun 25 04:44:30 1995 ylo

The IDEA encryption algorithm.

*/

/*
 * $Id$
 * $Log$
 * Revision 1.3  2002/08/04 11:39:42  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:47  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:26  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:10  bbsdev
 * bbs sshd
 *
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.2  1995/07/13  01:25:00  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */

#ifndef IDEA_H
#define IDEA_H

typedef struct {
    word16 key_schedule[52];
} IDEAContext;

/* Sets idea key for encryption. */
void idea_set_key(IDEAContext * c, const unsigned char key[16]);

/* Destroys any sensitive data in the context. */
void idea_destroy_context(IDEAContext * c);

/* Performs the IDEA cipher transform on a block of data. */
void idea_transform(IDEAContext * c, word32 l, word32 r, word32 * output);

/* Encrypts len bytes from src to dest in CFB mode.  Len need not be a multiple
   of 8; if it is not, iv at return will contain garbage.  
   Otherwise, iv will be modified at end to a value suitable for continuing
   encryption. */
void idea_cfb_encrypt(IDEAContext * c, unsigned char *iv, unsigned char *dest, const unsigned char *src, unsigned int len);


/* Decrypts len bytes from src to dest in CFB mode.  Len need not be a multiple
   of 8; if it is not, iv at return will contain garbage.  
   Otherwise, iv will be modified at end to a value suitable for continuing
   decryption. */
void idea_cfb_decrypt(IDEAContext * c, unsigned char *iv, unsigned char *dest, const unsigned char *src, unsigned int len);

#endif                          /* IDEA_H */
