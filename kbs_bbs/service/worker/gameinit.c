/*#include<iostream.h>*/
/*#include<fstream.h>*/
#include<stdio.h>
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"gamedata.h"
#include"gameinit.h"
#include"screens.h"

static char CommentChar = '#';

PadPoint * GamePadData = NULL;
BoxPoint * GameBoxData = NULL;
BoxPoint * GameTargetData = NULL;
int PadPointNum = 0;   // Pad Point Number
int BoxPointNum = 0;   // Box Point Number
int TargetPointNum = 0;   // Target Point Number
int DataWidth, DataHeight;  // Pad Initial Width, Pad Initial Height
int PadBaseX = 0, PadBaseY = 0; // Pad Base X, Y

BoxPoint GameWorkerData;  // Worker Initial Position
BoxPoint WorkerPoint;   // Worker Current Position

char * GamePad = NULL;   // Game Pad Map
int PadWidth, PadHeight;  // Game Pad Width, Height

void DestroyPad(void)
{
    /* delete [] GamePad;*/
    free(GamePad);
    GamePad = NULL;
    PadWidth = PadHeight = 0;
}

Boolean InitPad(void)
{
    int i, j;
    int start, end;
    unsigned int pos, orit;
    unsigned int step;
    char * cptr;
    PadPoint * pptr;
    BoxPoint * bptr;

    DestroyPad();
    PadWidth = DataWidth;
    PadHeight = DataHeight;
    /* GamePad = new char[PadWidth * PadHeight];*/
    GamePad = malloc(PadWidth * PadHeight);
    if (NULL == GamePad) return FALSE;
    cptr = GamePad;
    for (j=0;j<PadHeight;j++)
        for (i=0;i<PadWidth;i++)
            *cptr++ = PadSpace;
    for (i=0;i<PadPointNum;i++) {
        cptr = GamePad;
        pptr = &(GamePadData[i]);
        start = pptr->start;
        end = pptr->end;
        pos = pptr->pos;
        orit = pptr->orientation;
        if (Horizontal == orit) {
            cptr += pos * PadWidth + start;
            step = 1;
        } else {
            cptr += start * PadWidth + pos;
            step = PadWidth;
        }
        for (j=start;j<=end;j++) {
            *cptr = PadWall;
            cptr += step;
        }
    }
    for (i=0;i<TargetPointNum;i++) {
        cptr = GamePad;
        bptr = GameTargetData + i;
        cptr += bptr->y * PadWidth + bptr->x;
        *cptr |= PadTarget;
    }
    for (i=0;i<BoxPointNum;i++) {
        cptr = GamePad;
        bptr = GameBoxData + i;
        cptr += bptr->y * PadWidth + bptr->x;
        *cptr |= PadBox;
    }
    WorkerPoint.x = GameWorkerData.x;
    WorkerPoint.y = GameWorkerData.y;
    return TRUE;
}

void DestroyData(void)
{
    /* delete [] GameTargetData;*/
    free(GameTargetData);
    GameTargetData = NULL;
    TargetPointNum = 0;

    /* delete [] GameBoxData;*/
    free(GameBoxData);
    GameBoxData = NULL;
    BoxPointNum = 0;

    /* delete [] GamePadData;*/
    free(GamePadData);
    GamePadData = NULL;
    PadPointNum = 0;
}

Boolean InitData(char * fname)
{
    char buf[1024];
    Boolean b_err;
    int cursect, sectnum;
    int lilen, pos, line;
    int wid, hei, num;
    int x1, y1, x2, y2;
    /* ifstream * ifp;*/
    FILE * ifp;

    if (NULL == fname) return FALSE;
    b_err = FALSE;

    DestroyData();

    cursect = NullSection;
    sectnum = 0;
    line = 0;

    /* ifp = new ifstream(fname);*/
    ifp = fopen(fname, "r");
    if (NULL == ifp) return FALSE;
    /* while(!ifp->eof() && ifp->good()) {*/
    /*  ifp->getline(buf, 1024);*/
    while (!feof(ifp)) {
        /*  ifs->getline(buf, 1024);*/
        char * nptr;
        if (NULL == fgets(buf, 1024, ifp)) break;;
        if ((nptr = strchr(buf, '\n'))) *nptr = 0;

        line ++;
        /*  if(!ifp->good()) { b_err = FALSE; break; }*/
        if (CommentChar == *buf) continue;
        lilen = strlen(buf);
        if (lilen<1) continue;
        trim(buf, ' ');
        if (SectionStart == *buf || SectionEnd == buf[lilen-1]) {
            sectnum = 0;
            num = wid = hei = 0;
            lilen -= 2;
            stoupper(buf+1);
            if (!strncmp("PAD", buf+1, lilen)) cursect = PadSection;
            else if (!strncmp("BOX", buf+1, lilen)) cursect = BoxSection;
            else if (!strncmp("WORKER", buf+1, lilen)) cursect = WorkerSection;
            else if (!strncmp("TARGET", buf+1, lilen)) cursect = TargetSection;
            else cursect = NullSection;
#ifdef DEBUG_OUTPUT
            /*   cout << "Section : " << buf << endl;*/
            fprintf(stderr, "Section : %s\n", buf);
#endif
            continue;
        }
        if (!strncmp("Number=", buf, 7)) {
            num = atoi(buf+7);
            if (num < 1 || 0 != sectnum) { b_err = TRUE; break; }
#ifdef DEBUG_OUTPUT
            /*   cout << "Number = " << num << endl;*/
            fprintf(stderr, "Number = %d\n", num);
#endif
            switch (cursect) {
                case PadSection :
                    /*     if(NULL != GamePadData) delete [] GamePadData;*/
                    /*     GamePadData = new PadPoint[num];*/
                    if (NULL != GamePadData) free(GamePadData);
                    GamePadData = (PadPoint *)malloc(sizeof(PadPoint) * num);
                    PadPointNum = num;
                    break;
                case BoxSection :
                    /*     if(NULL != GameBoxData) delete [] GameBoxData;*/
                    /*     GameBoxData = new BoxPoint[num];*/
                    if (NULL != GameBoxData) free(GameBoxData);
                    GameBoxData = (BoxPoint *)malloc(sizeof(BoxPoint) * num);
                    BoxPointNum = num;
                    break;
                case TargetSection :
                    /*     if(NULL != GameTargetData) delete [] GameTargetData;*/
                    /*     GameTargetData = new BoxPoint[num];*/
                    if (NULL != GameTargetData) free(GameTargetData);
                    GameTargetData = (BoxPoint *)malloc(sizeof(BoxPoint) * num);
                    TargetPointNum = num;
                    break;
                case WorkerSection :
                    if (1 != num) b_err = TRUE;
                    break;
            }
            if (TRUE == b_err) break;
            continue;
        }
        if (!strncmp("Size=", buf, 5)) {
            pos = procpoint(buf+5, &wid, &hei, 'x', 0, 0);
#ifdef DEBUG_OUTPUT
            fprintf(stderr, "Size = %d * %d\n", wid, hei);
#endif
            if (2 != pos || 0 != sectnum) { b_err = TRUE; break; }
            switch (cursect) {
                case PadSection :
                    if (wid < MinWidth || hei < MinHeight) b_err = TRUE;
                    else if (wid + PadBaseX >= MaxXPos
                             || hei + PadBaseY >= MaxYPos) b_err = TRUE;
                    else {
                        DataWidth = wid + PadBaseX;
                        DataHeight = hei + PadBaseY;
                    }
                    break;
                case BoxSection :
                case TargetSection :
                case WorkerSection :
                    if (wid != 1 ||  hei != 1) b_err = TRUE;
                    break;
            }
            continue;
        }
        if (!strncmp("Base=", buf, 5)) {
            if (PadSection != cursect || sectnum > 0) { b_err = TRUE; break; }
            pos = procpoint(buf + 5, &x1, &y1, ',', 0, 0);
            if (2 != pos || 0 != sectnum) { b_err = TRUE; break; }
            if (x1 + DataWidth >= MaxXPos) x1 = (MaxXPos - DataWidth)/2;
            if (y1 + DataHeight >= MaxYPos) y1 = (MaxYPos - DataHeight)/2;
            if (x1 < 0) x1 = 0;
            if (y1 < 0) y1 = 0;
            PadBaseX = x1; PadBaseY = y1;
            DataWidth  += x1;
            DataHeight += y1;
            continue;
        }
        /*--- Below must be line data.... ---*/
        switch (cursect) {
                BoxPoint * bpt;
                PadPoint * ppt;
            case PadSection :
                pos = procline(buf, &x1, &y1, &x2, &y2, PadBaseX, PadBaseY);
                if (sectnum >= PadPointNum || 4 != pos) { b_err = TRUE; break; }
                ppt = &(GamePadData[sectnum ++]);
                x1 = getodd(x1);
                x2 = getodd(x2);
                if (x1 == x2) {
                    if (y1 == y2) { //{ b_err = TRUE; break; }
                        ppt->orientation = Horizontal;
                        ppt->pos = y1;
                        ppt->start = ppt->end = x1;
                    } else {
                        ppt->orientation = Vertical;
                        ppt->pos = x1;
                        ppt->start = y1>y2 ? y2 : y1;
                        ppt->end = y1>y2 ? y1 : y2;
                    }
                } else if (y1 == y2) {
                    ppt->orientation = Horizontal;
                    ppt->pos = y1;
                    ppt->start = x1>x2 ? x2 : x1;
                    ppt->end = x1>x2 ? x1 :x2;
                } else b_err = TRUE;
                break;
            case BoxSection :
                pos = procpoint(buf, &x1, &y1, ',', PadBaseX, PadBaseY);
                x1 = getodd(x1);
                if (sectnum >= BoxPointNum || 2 != pos) { b_err = TRUE; break; }
                bpt = &(GameBoxData[sectnum++]);
                bpt->x = x1;
                bpt->y = y1;
                break;
            case TargetSection :
                pos = procpoint(buf, &x1, &y1, ',', PadBaseX, PadBaseY);
                x1 = getodd(x1);
                if (sectnum >= TargetPointNum || 2 != pos) { b_err = TRUE; break; }
                bpt = &(GameTargetData[sectnum++]);
                bpt->x = x1;
                bpt->y = y1;
                break;
            case WorkerSection :
                pos = procpoint(buf, &x1, &y1, ',', PadBaseX, PadBaseY);
                x1 = getodd(x1);
                if (2 != pos) { b_err = TRUE; break; }
                GameWorkerData.x = x1;
                GameWorkerData.y = y1;
                sectnum ++;
                break;
        }
        if (TRUE == b_err) break;
    }
    /* delete ifp;*/
    fclose(ifp);
    if (TargetPointNum != BoxPointNum) b_err = TRUE;
    if (TRUE == b_err) {
        DestroyData();
#if DEBUG_OUT
        fprintf(stderr, "Error encounted at line %s\n", line);
#endif
    }
    return (!b_err);
}

