/*

cipher.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Apr 19 16:50:42 1995 ylo

*/

#ifndef CIPHER_H
#define CIPHER_H

#ifndef WITHOUT_IDEA
#include "idea.h"
#endif                          /* WITHOUT_IDEA */
#include "des.h"
#ifdef WITH_ARCFOUR
#include "arcfour.h"
#endif                          /* WITH_ARCFOUR */
#ifdef WITH_BLOWFISH
#include "blowfish.h"
#endif                          /* WITH_BLOWFISH */

/* Cipher types.  New types can be added, but old types should not be removed
   for compatibility.  The maximum allowed value is 31. */
#define SSH_CIPHER_NOT_SET	-1      /* None selected (invalid number). */
#define SSH_CIPHER_NONE		0       /* no encryption */
#define SSH_CIPHER_IDEA		1       /* IDEA CFB */
#define SSH_CIPHER_DES		2       /* DES CBC */
#define SSH_CIPHER_3DES		3       /* 3DES CBC */
#define SSH_CIPHER_ARCFOUR	5       /* Arcfour */
#define SSH_CIPHER_BLOWFISH     6       /* Bruce Schneier's Blowfish */
#define SSH_CIPHER_RESERVED	7       /* Reserved for 40 bit crippled encryption,
                                           Bernard Perrot <perrot@lal.in2p3.fr> */

typedef struct {
    unsigned int type;
    union {
#ifndef WITHOUT_IDEA
        struct {
            IDEAContext key;
            unsigned char iv[8];
        } idea;
#endif                          /* WITHOUT_IDEA */
#ifdef WITH_DES
        struct {
            DESContext key;
            unsigned char iv[8];
        } des;
#endif                          /* WITH_DES */
        struct {
            DESContext key1;
            unsigned char iv1[8];
            DESContext key2;
            unsigned char iv2[8];
            DESContext key3;
            unsigned char iv3[8];
        } des3;
#ifdef WITH_ARCFOUR
        ArcfourContext arcfour;
#endif
#ifdef WITH_BLOWFISH
        BlowfishContext blowfish;
#endif                          /* WITH_BLOWFISH */
    } u;
} CipherContext;

/* Returns a bit mask indicating which ciphers are supported by this
   implementation.  The bit mask has the corresponding bit set of each
   supported cipher. */
unsigned int cipher_mask(void);

/* Returns the name of the cipher. */
const char *cipher_name(int cipher);

/* Parses the name of the cipher.  Returns the number of the corresponding
   cipher, or -1 on error. */
int cipher_number(const char *name);

/* Selects the cipher to use and sets the key.  If for_encryption is true,
   the key is setup for encryption; otherwise it is setup for decryption. */
void cipher_set_key(CipherContext * context, int cipher, const unsigned char *key, int keylen, int for_encryption);

/* Sets key for the cipher by computing the MD5 checksum of the passphrase,
   and using the resulting 16 bytes as the key. */
void cipher_set_key_string(CipherContext * context, int cipher, const char *passphrase, int for_encryption);

/* Encrypts data using the cipher.  For most ciphers, len should be a
   multiple of 8. */
void cipher_encrypt(CipherContext * context, unsigned char *dest, const unsigned char *src, unsigned int len);

/* Decrypts data using the cipher.  For most ciphers, len should be a
   multiple of 8. */
void cipher_decrypt(CipherContext * context, unsigned char *dest, const unsigned char *src, unsigned int len);

#endif                          /* CIPHER_H */
