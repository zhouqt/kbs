#include"gamedraw.h"
#include"gameinit.h"
#include"screens.h"

extern int PadWidth, PadHeight;
extern char * GamePad;
extern BoxPoint WorkerPoint;

void DrawElement(int x, int y, int type)
{
    switch (type) {
        case PadSpace :
            background(x, y);
            break;
        case PadWall :
            rectwall(x, y);
            break;
        case PadBox :
            rectbox(x, y);
            break;
        case PadTarget :
            target(x, y);
            break;
        case PadWorker :
            worker(x, y);
            break;
        default :
            if (type & PadTarget && type & PadBox)
                rectboxtarget(x, y);
            break;
    }
}

void DrawCell(int x, int y)
{
    if (y < 0 || x < 0 || y >= PadHeight || x >= PadWidth) return;
    DrawElement(x, y, GamePad[y*PadWidth + x ]);
}

void DrawPad(void)
{
    int i,j, val, ptr;
    ptr = 0;
    for (j=0;j<PadHeight;j++)
        for (i=0;i<PadWidth;i++) {
            val = GamePad[ptr++];
            if (i%2) continue;
            DrawElement(i, j, val);
        }
    worker(WorkerPoint.x, WorkerPoint.y);
}

#if 0
void DrawData(void)
{
    int j, val, ptr;
    PadPoint * pptr;
    ptr = 0;
    for (j=0;j<PadPointNum;j++) {
        pptr = GamePadData + ptr++;
        if (pptr->orientation == Horizontal) {
            val = pptr->start;
            pptr->start -= val % 2;
            val = pptr->end;
            pptr->end -= (1 - val % 2);
        } else {
            val = pptr->pos;
            pptr->pos -= val%2;
        }
        gcharline(pptr->pos, pptr->start, pptr->end, pptr->orientation);
    }
}
#endif
