#include "bbs.h"

struct boardheader *bcache;
struct BCACHE *brdshm;
struct UTMPFILE *utmpshm;
struct userec* currentuser;
char fromhost[ IPLEN+1 ] ;
