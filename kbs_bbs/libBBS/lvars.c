#include "bbs.h"

struct boardheader *bcache;
struct BCACHE *brdshm;
struct UTMPFILE *utmpshm;
struct userec *currentuser;
struct usermemo *currentmemo;
//struct userdata curruserdata;
struct _mail_list user_mail_list;


char fromhost[IPLEN + 1];
