#include "bbs.h"

struct boardheader *bcache;
struct BCACHE *brdshm;
struct userec *currentuser;
char fromhost[IPLEN + 1];
