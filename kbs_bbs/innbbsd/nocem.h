/*
  NoCeM-INNBBSD
  Yen-Ming Lee <leeym@cae.ce.ntu.edu.tw>
*/
/*
$Id$
*/

#ifndef NOCEM_H
#define NOCEM_H

#include "innbbsconf.h"
#include "bbslib.h"
#include "inntobbs.h"
/*#include "version.h"            for FB3 */
#include "bbs.h"

#include <stdarg.h>             /* for va_start() problem */

typedef struct ncmperm_t {
    char *issuer;
    char *type;
    int perm;
} ncmperm_t;

#define TEXT    0
#define NCMHDR  1
#define NCMBDY  2

#define NOPGP   -1
#define PGPGOOD 0
#define PGPBAD  1
#define PGPUN   2

#define P_OKAY  0
#define P_FAIL  -1
#define P_UNKNOWN       -2
#define P_DISALLOW      -3

#define STRLEN          80
#define MAXSPAMMID      10000
#undef LINELEN
#define	LINELEN		1024

#define LeeymBBS        BBSHOST
#define LeeymEMAIL      ADMINUSER
#ifndef ADMINUSER
#define ADMINUSER "sysop@##BBSHOST"
#endif
#ifndef BBSHOST
#define BBSHOST "localhost"
#endif
#define NCMINNBBSVER    VERSION

#undef	DONT_REGISTER
#undef COUNT

extern char NCMVER[];
extern char ISSUER[];
extern char TYPE[];
extern char ACTION[];
extern char NCMID[];
extern char COUNT[];
extern char THRESHOLD[];
extern char KEYID[];
extern char SPAMMID_NOW[];

//extern char SPAMMID[][];
extern char SPAMMID[MAXSPAMMID][STRLEN];

extern ncmperm_t *NCMPERM, **NCMPERM_BYTYPE;
extern int NCMCOUNT;

ncmperm_t *search_issuer(char *issuer);
int receive_nocem();

#endif                          /* NOCEM_H */
