#ifndef _GAME_SCREEN_HEADER_
#define _GAME_SCREEN_HEADER_

#ifdef ENABLE_DL
#  define _mvaddnstr(x, y, ptr, len) _mvaddstr(x, y, ptr)
#  define cleartoeol() _cleartoeol()
#  define clear() _clear()
#  define move(x, y) _move(x, y)
int getch (void);

void _prints(const char * s);
void println(const char * s);
void _move(int y1, int x1);
void _clear (void);
void _cleartoeol(void);
void _mvaddstr(int x, int y, char * ptr);
#else
#ifndef _USE_NCURSE_

# ifndef KEY_UP
#  define KEY_LEFT  260
#  define KEY_RIGHT 259
#  define KEY_UP    257
#  define KEY_DOWN  258
#endif

#  define _mvaddnstr(x, y, ptr, len) _mvaddstr(x, y, ptr)
#  define cleartoeol() _cleartoeol()
#  define clear() _clear()
#  define move(x, y) _move(x, y)

int getch (void);
int kbhit(void);
int get_tty (void);
void init_tty (void);
void reset_tty (void);

void oflush (void);
void outs(char * msg);
void prints(const char * s);
void println(const char * s);
void _move(int y1, int x1);
void _clear (void);
void _cleartoeol(void);
void _mvaddstr(int x, int y, char * ptr);

#else

#  define _mvaddnstr(a,b,c,d) mvaddnstr(a,b,c,d)
#  define _move(x,y) move(x,y)

#endif

void initialize(void);
void finalize(void);

#endif
#endif
