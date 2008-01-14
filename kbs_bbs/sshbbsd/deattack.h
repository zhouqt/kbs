/* $Id$
 * Cryptographic attack detector for ssh - Header file
 * (C)1998 CORE-SDI, Buenos Aires Argentina
 * Ariel Futoransky(futo@core-sdi.com)
 */

#ifndef _DEATTACK_H
#define _DEATTACK_H

/* Return codes */
#define DEATTACK_OK		0
#define DEATTACK_DETECTED	1

int detect_attack(unsigned char *buf, word32 len, unsigned char IV[8]);
#endif
