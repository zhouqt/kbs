#ifndef GamePlayFunctionHeader
#define GamePlayFunctionHeader

#include"gamedefs.h"

Boolean addiff(int x, int y);
void showdiff(void);
void cleardiff(void);

//Boolean workermove(Direction dir);
int workermove(int dir);
Boolean wingame(void);
void regretmove(int dir);

#endif
