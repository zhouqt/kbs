#ifndef GameDataProcHeader
#define GameDataProcHeader

#include"gamedefs.h"

char * stoupper(char * str);
char * stolower(char * str);
char * trim(char * buf, char tch);
int procpoint(char * buf, int * x, int * y, char sep, int basex, int basey);
int procline(char * buf, int * x1, int * y1, int * x2, int * y2, int basex, int basey);
/* 4cxx
int procpoint(char * buf, int & x, int & y,
 char sep = ',', int basex = 0, int basey = 0);
int procline(char * buf, int & x1, int & y1, int & x2, int & y2,
 int basex = 0, int basey = 0);
*/
#endif
