#ifndef GameScreenProcHeader
#define GameScreenProcHeader

void gcharbox(int x, int y, int wid, int hei);
void gcharrect(int x, int y, int wid, int hei);
void gcharline(int pos, int start, int end, int orit);

void target(int x, int y);
void worker(int x, int y);
void rectbox(int x, int y);
void rectwall(int x, int y);
void background(int x, int y);
void FoolWorker(void);
void rectboxtarget(int x, int y);

#endif
