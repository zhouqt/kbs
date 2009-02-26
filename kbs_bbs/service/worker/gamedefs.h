#ifndef WorkerGameDefinitionHeader
#define WorkerGameDefinitionHeader

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifdef ENABLE_DL
#include "service.h"
#include "bbs.h"
#endif

typedef char Boolean;
#if 0
const int MinWidth = 5;
const int MinHeight = 5;
const int MaxXPos = 80;
const int MaxYPos = 25;
const int MaxSteps = 256;
const int MaxDiff = 4;
#else
#define MinWidth 5
#define MinHeight 5
#define MaxXPos  80
#define MaxYPos  25
#define MaxSteps       9999
#define MaxDiff  4
#endif

#define PadSpace 0x00
#define PadTarget 0x04
#define PadBox  0x08
#define PadWall  0x10
#define PadWorker 0x20

enum {
    NullSection,
    PadSection,
    BoxSection,
    TargetSection,
    WorkerSection,

    Horizontal = 100,
    Vertical = 200
};

enum Direction { NullDir, East, North, West, South };

#endif
