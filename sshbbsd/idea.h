/*

idea.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sun Jun 25 04:44:30 1995 ylo

The IDEA encryption algorithm.

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
