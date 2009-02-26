#include"gamedefs.h"
#include"gamedraw.h"
#include"gameinit.h"
#include"gameplay.h"
#include"screens.h"

extern BoxPoint * GameTargetData;
extern int TargetPointNum;    //  Target Point Number
extern int BoxPointNum;     //  Target Point Number
extern int PadBaseX, PadBaseY;   //  Pad Base X, Y

extern BoxPoint WorkerPoint;   //  Worker Current Position

extern char * GamePad;     //  Game Pad Map
extern int PadWidth, PadHeight;   //  Game Pad Width, Height

BoxPoint PadDiff[MaxDiff];
int PadDiffNum = 0;

Boolean addiff(int x, int y)
{
    if (PadDiffNum >= MaxDiff) return FALSE;
    PadDiff[PadDiffNum].x = x;
    PadDiff[PadDiffNum].y = y;
    PadDiffNum ++;
    return TRUE;
}

void showdiff(void)
{
    int i;
    for (i=0; i< PadDiffNum; i++)
        DrawCell(PadDiff[i].x, PadDiff[i].y);
    worker(WorkerPoint.x, WorkerPoint.y);
}

void cleardiff(void)
{
    PadDiffNum = 0;
}

void movebox(int x, int y, int xoff, int yoff)
{
    char * ptr, * target;
    ptr = GamePad + y * PadWidth + x;
    target = ptr + xoff + yoff*PadWidth;
    if (PadSpace != *target && PadTarget != *target) return;
    if (0 == (PadBox & *ptr)) return;
    *target++ |=  PadBox;
    *target  |=  PadBox;
    *ptr++  &= ~PadBox;
    *ptr  &= ~PadBox;
}

/*Boolean workermove(Direction dir)*/
int workermove(int dir)
{
    Boolean b_rtn;
    char other, target;
    char * ptr;
    int x, y, xoff, yoff;
    b_rtn = 0;
    x = WorkerPoint.x;
    y = WorkerPoint.y;
    ptr = GamePad + y * PadWidth + x;
    switch (dir) {
        case East:
            xoff = 2; yoff = 0; break; // 2 space for each cell
        case West:
            xoff = -2; yoff = 0; break;
        case North:
            xoff = 0; yoff = -1; break;
        case South:
            xoff = 0; yoff = 1; break;
        default :
            return 0; // break;
    }
    target = *(ptr + xoff + yoff*PadWidth);
    other = *(ptr + ((xoff + yoff*PadWidth)<<1));
    if (PadSpace == target || PadTarget == target) b_rtn = 1;
    else if (PadBox & target) {
        if (PadSpace == other || PadTarget == other) {
            addiff(x+xoff*2, y+yoff*2);
            movebox(x + xoff, y + yoff, xoff, yoff);
            b_rtn = 2;
        } else b_rtn = 0;
    } else b_rtn = 0;
    if (b_rtn) {
        addiff(x, y);
        addiff(x+xoff, y+yoff);
        WorkerPoint.x = x + xoff;
        WorkerPoint.y = y + yoff;
    }
    return b_rtn;
}

void regretmove(int dir)
{
    int x, y, xoff, yoff;
    int bmovebox=0;
    x = WorkerPoint.x;
    y = WorkerPoint.y;
    if (dir>8) {
        dir-=8;
        bmovebox=1;
    }
    switch (dir) {
        case East:
            xoff = 2; yoff = 0; break; // 2 space for each cell
        case West:
            xoff = -2; yoff = 0; break;
        case North:
            xoff = 0; yoff = -1; break;
        case South:
            xoff = 0; yoff = 1; break;
        default :
            return ; // break;
    }
    addiff(x+xoff, y+yoff);
    addiff(x, y);
    addiff(x-xoff, y-yoff);
    WorkerPoint.x = x - xoff;
    WorkerPoint.y = y - yoff;
    if (bmovebox)
        movebox(x + xoff, y + yoff, -xoff, -yoff);
}

Boolean wingame(void)
{
    int i, x, y, num;
    char * ptr;
    num = 0;
    for (i=0;i<TargetPointNum;i++) {
        x = GameTargetData[i].x;
        y = GameTargetData[i].y;
        ptr = GamePad + x + y * PadWidth;
        if ((*ptr & PadTarget) && (*ptr & PadBox)) num ++;
        else break;
    }
    if (num != TargetPointNum) return FALSE;
    else return TRUE;
}
