#ifndef GameInitializeHeader
#define GameInitializeHeader

#include"gamedefs.h"

#define setodd(val) (val%2 ? val--:val)
#define getodd(val) (val%2 ? val-1:val)

#if 0
const char SectionStart = '[';
const char SectionEnd = ']';
#else
#define SectionStart '['
#define SectionEnd  ']'
#endif

typedef struct _PadPoint {
    unsigned int start, end;
    unsigned int pos;
    int orientation;
} PadPoint;

typedef struct _BoxPoint {
    unsigned int x, y;
} BoxPoint;

void DestroyPad(void);
Boolean InitPad(void);
void DestroyData(void);
Boolean InitData(char * fname);

#endif
