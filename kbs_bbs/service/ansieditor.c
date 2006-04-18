/* 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include "service.h"
#include "bbs.h"

SMTH_API int scr_lns, scr_cols;

#define EDITOR_NAME "Belle AnsiEditor"
#define NUM_COLS LINELEN
#define NUM_ROWS LINEHEIGHT
#define MAX_PENS 10
#define MAX_UNDO 200
#define NUM_SKEYS 10
#define MAX_HELP_PAGES 20
#define MAX_PATH_LEN 1024

#define MAIL_ADDR_LEN 31
#define MAIL_TITLE_LEN 31

#define GBK_PAGE_RECS 10

#define ESC 27
#define SPACE 32
#define BRK 91 /* '[' */
#define SEMICOLON 59 /* ';' */
#define RETURN 13
#define DEL 127

/* SENDMAIL and BBSHOST are supposed to be defined in bbs.h, comment them when deploying */
#if defined(CYGWIN)
#define DEFAULT_DIR_PREFIX "."
#define DEFAULT_USER_ID ""
#else
#define DEFAULT_DIR_PREFIX BBSHOME "/service"
#define DEFAULT_USER_ID "public"
#endif
#define CMDFILE "ansihelp.txt"

#define NEXT_FCOLOR(x) ((x)? ((x)+1)%38: 30)
#define NEXT_BCOLOR(x) ((x)? ((x)+1)%48: 40)
#define SAME_COLOR(c1, c2) (!(c1) && !(c2)? 1: c1+10 == c2 || c2+10 == c1)
#define COLOR_F2B(x) (!(x)? (x): x+10)
#define COLOR_B2F(x) (!(x)? (x): x-10)

enum {CTRL_A=1, CTRL_B, CTRL_C, CTRL_D, CTRL_E, CTRL_F, CTRL_G,
	CTRL_H, CTRL_I, CTRL_J, CTRL_K, CTRL_L, CTRL_M, CTRL_N, 
	CTRL_O, CTRL_P, CTRL_Q, CTRL_R, CTRL_S, CTRL_T, CTRL_U, 
	CTRL_V, CTRL_W, CTRL_X, CTRL_Y, CTRL_Z
};
enum cmds { CMD_DATA=0, /* normal data */	
	CMD_UP, CMD_DOWN, CMD_RIGHT, CMD_LEFT, CMD_HOME, CMD_END, CMD_PGUP, CMD_PGDN, /*  arrows */	
	CMD_ABSORB, CMD_RESET, CMD_FCOLOR, CMD_BCOLOR, CMD_BLINK, CMD_BRIGHT, CMD_UNDERLINE,  /* setting attributes */
	CMD_APPLYFCOLOR, CMD_APPLYBCOLOR, CMD_APPLYBLINK, CMD_APPLYBRIGHT, CMD_APPLYUNDERLINE, CMD_APPLYATTRIB, /* apply attributes */
	CMD_LINEMODE,/* line mode */	
	CMD_ERASE, CMD_ADDPEN, CMD_DELPEN, CMD_LISTPEN, CMD_NEXTPEN, CMD_PREVPEN, /* pen functions */	
	CMD_INSROW, CMD_DELROW, CMD_ERASEROW, CMD_INSCOL, CMD_DELCOL, CMD_ERASECOL, /* row/col functions */
	CMD_SKEY, CMD_SETSKEY, CMD_LISTSKEY,/* shortcut keys */
	CMD_REFRESH, /* refresh screen */
	CMD_ERASEALL, CMD_ERASEUP, CMD_ERASEDOWN, CMD_ERASELEFT, CMD_ERASERIGHT,/* clear screen */
	CMD_G_FCOLOR, CMD_G_FCOLOREXT, CMD_G_BCOLOR, CMD_G_BCOLOREXT, CMD_G_BLINK, CMD_G_BRIGHT, CMD_G_UNDERLINE, CMD_G_ATTRIB, CMD_G_CHAR, CMD_G_ALL,
	CMD_A4_FCOLOR, CMD_A4_FCOLOREXT, CMD_A4_BCOLOR, CMD_A4_BCOLOREXT, CMD_A4_BLINK, CMD_A4_BRIGHT, CMD_A4_UNDERLINE, CMD_A4_ATTRIB, 
	CMD_A8_FCOLOR, CMD_A8_FCOLOREXT, CMD_A8_BCOLOR, CMD_A8_BCOLOREXT, CMD_A8_BLINK, CMD_A8_BRIGHT, CMD_A8_UNDERLINE, CMD_A8_ATTRIB, 
	CMD_HELPCMD, CMD_HELPCODE, /* help */	
	CMD_LOAD, CMD_SAVE, /* save/load */	
	CMD_EXPORT, /* export to file */
	CMD_REDO, CMD_UNDO, /* redo/undo */	
	CMD_COPY, CMD_PASTE, CMD_DEL, /* copy/paste/del a char*/	
	CMD_ROWCOPY, CMD_ROWPASTE, CMD_COLCOPY, CMD_COLPASTE,/* copy/paste row/col */
	CMD_DRAW, /* drawing */	
	CMD_ANSIPST, /* paste ansi text */	
	CMD_ANSIMODE, /* ansi text mode string */
	CMD_MAIL, /* mail */	
	CMD_BLOCK, /* block operations */	
	CMD_CONFIRM, /* confirm */	
	CMD_GBK, /* gbk code input */
	CMD_QUIT, /* quit */	
	NUM_COMMANDS
};
enum states { S_EDIT=0, /* editing */	
	S_SELBLK, /* block operation:step 1 */ 	
	S_EDITBLK, /* block operation:step 2 */ 	
	S_APST, /* paste ansi text */
	S_HLPCMD, /* command help */
	S_HLPCODE, /* code help */
	S_MAIL, /* mail */
	NUM_STATES
};
enum {NONE, DRAW, UPDATE, NUM_DRAW_METHODS};
enum {HOVW, HINS, VOVW, VINS, NUM_LINEMODES};
enum {CP_CHAR, CP_BLOCK};
enum {ADJACENT_4, ADJACENT_8};
struct ansiattr {
	char fcolor;
	char bcolor;
	char blink;
	char bright;
	char underline;
	char reverse; /* only used for block editing */
};
enum {A_FCOLOR, A_FCOLOREXT, A_BCOLOR, A_BCOLOREXT, A_BLINK, A_BRIGHT, A_UNDERLINE, A_REVERSE, A_ATTRIB, A_CHAR, A_ALL};
struct charinfo {
	unsigned char code;
	struct ansiattr attr;
	char chinese; /* if this is a chinese char */
	char half;  /* valid if chinese=1, half=0 for first byte, half=1 for second byte*/
};
struct undoinfo {
	int dirt;
	char fields;
	void** address;
	void** olddata;
	void** newdata;
	int* len;
	char row1; /* row1-row2 defines the affected area */
	char row2;
	struct undoinfo *prev;
	struct undoinfo *next;
};
struct fsm_entry {
	int next_state;
	void (*callback)();
};
/***************************************************************************
 *                       data structures                                   *
 ***************************************************************************/
/* chinese chars in use*/
unsigned char char_diamond[3] = {0xa1, 0xf4, 0x00};
unsigned char char_sep[3] = {0xa9, 0xa7, 0x00};
unsigned char char_sep2[3] = {0xa8, 0x55, 0x00};
unsigned char char_space[3] = {0xa1, 0xa1, 0x00};

/* COMMAND LINE PARAMETERS */
char dir_prefix[MAX_PATH_LEN];
char user_id[32];
char save_dir[MAX_PATH_LEN];
/* TTY */
static struct termios save_tty;

/* IO */
int unget_flag; 
int unget_buf;  /* providing capability to ungetch 1 char */

struct user_info uinfo;

/* EDITING */
unsigned char code_first, code_second; /* input data */
int chinese; /* if the input data is a chinese char */
int cur_x, cur_y;  /* current cursor position */
int save_x, save_y; /* save current cursor position temporarily*/
int save_cur_x, save_cur_y; /* save current cursor position*/
struct charinfo graph[NUM_ROWS][NUM_COLS]; /* 80x23 graph, one additonal row for prevention of memory fault*/
struct charinfo save_graph[NUM_ROWS][NUM_COLS]; /* save graph */
int copy_type; /* if the copy is a char or block */
struct charinfo char_copy[2]; /* copy/paste a char */ 
struct charinfo row_copy[NUM_COLS]; /* copy/paste a row */
struct charinfo col_copy[NUM_ROWS-2][2]; /* copy/paste a col */
unsigned int anchor_dirt, dirt; /* used to test if the graph is modified and for limited undo*/
struct ansiattr cur_attr; /* current ansi attributes */
struct ansiattr save_attr; /* saved ansi attributes */
struct charinfo pens[MAX_PENS][2];
int num_pens; /* the number of available pens */
int cur_pen; /* current pen */
int drawing; /* drawing methods */
int linemode; /* line mode */
int prev_state; /* previous state */
int state; /* current state */
char message[300]; /* message showing on the status bar */
static char save_file[20]; /* file for saving progress*/
static char export_file[20]; /* file for exporting graph */

/* BLOCK EDITING */
int blk_x1, blk_y1, blk_x2, blk_y2;
struct undoinfo *blk_undo_ptr;
struct charinfo blk_swap_graph[NUM_ROWS][NUM_COLS]; /* for swap */
struct charinfo blk_copy[NUM_ROWS][NUM_COLS]; /* block copy */
int blk_cp_x1, blk_cp_y1, blk_cp_x2, blk_cp_y2; /* range of the block copy */

/* MAILING */
static char mail_addr[MAIL_ADDR_LEN]; /* mail address */
static char mail_title[MAIL_TITLE_LEN]; /* mail title */
enum {ML_ADDR, ML_TITLE} mail_step; /* steps of mailing */
static char mail_str[40000]; /* mail content for the whole graph */
int mail_sent;

/* UNDO/REDO */
static struct undoinfo undo_list; /* first node of undo list */
static struct undoinfo pending; 
struct undoinfo *undo_ptr; /* current position in undo list */
struct undoinfo *undo_limit; /* limit the undo scope */
int undo_num; /* the length of undo list */
int chkpt_flag; /* if checkpoint the current action */

/* GBK CODE */
static int gbk_recs[] = {
	0xa1a0, 0xa1b0, 0xa1c0, 0xa1d0, 0xa1e0, 0xa1f0,
	0xa2a0, 0xa2b0, 0xa2c0, 0xa2d0, 0xa2e0, 0xa2f0,
	0xa3a0, 0xa3b0, 0xa3c0, 0xa3d0, 0xa3e0, 0xa3f0,
	0xa4a0, 0xa4b0, 0xa4c0, 0xa4d0, 0xa4e0, 0xa4f0,
	0xa5a0, 0xa5b0, 0xa5c0, 0xa5d0, 0xa5e0, 0xa5f0,
	0xa6a0, 0xa6b0, 0xa6c0, 0xa6d0, 0xa6e0, 0xa6f0,
	0xa7a0, 0xa7b0, 0xa7c0, 0xa7d0, 0xa7e0, 0xa7f0,
	0xa840, 0xa850, 0xa860, 0xa870, 0xa880, 0xa890, 0xa8a0, 0xa8b0, 0xa8c0, 0xa8d0, 0xa8e0,
	0xa940, 0xa950, 0xa960, 0xa970, 0xa980, 0xa990, 0xa9a0, 0xa9b0, 0xa9c0, 0xa9d0, 0xa9e0,
	0 /* end */
};
static int gbk_total_pages = (sizeof(gbk_recs)/sizeof(int)-2)/GBK_PAGE_RECS+1;
static int gbk_page;

/* SHORTCUT KEYS */
struct charinfo sc_keys[NUM_SKEYS][2];

/* ANSI PASTE */
enum {AP_DATA, AP_CMD, AP_LB, AP_ATTR, AP_SEP} apst_step; /* for ansi parsing */
enum{AP_ESCBRK, AP_STARBRK, AP_MODES} apst_mode;

char dirt_str[2][2] = {" ", "*"};
char drawing_str[3][7] = {"抬笔", "落笔", "更新"};
char linemode_str[4][7]={"行擦写","行插入","列擦写","列插入"};
char state_str[8][10]={"编辑","块选中","块编辑","带色粘贴", "命令帮助", "GBK码帮助", "寄邮件"};
char apst_mode_str[2][5]={"ESC["," *[ "};

/* HELP COMMAND */
static int hcmd_total_pages; 
static int hcmd_page;
static char* cmd_help_str[MAX_HELP_PAGES][NUM_ROWS-1];
int prev_hcmd_state;

/* ADJACENT REPLACEMENT*/
char a4_dir[4][2] = { {1,0}, {0,-1}, {-1,0}, {0,1} };
char a8_dir[8][2] = { {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}, {-1,1}, {0,1}, {1,1} };
char adj_visited[NUM_ROWS-1][NUM_COLS]; 
/***************************************************************************
 *                            prototypes                                   *
 ***************************************************************************/
void draw_sbar();
/***************************************************************************
 *                       tty functions                                     *
 ***************************************************************************/
void tty_raw(int fd) 
{
	struct termios new_tty;

	tcgetattr(fd, &save_tty);
	new_tty = save_tty;

	new_tty.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	new_tty.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	new_tty.c_cflag &= ~(CSIZE | PARENB);
	new_tty.c_cflag |= CS8;
	new_tty.c_cc[VMIN] = 1;
	new_tty.c_cc[VTIME] = 0;

	tcsetattr(fd, TCSANOW, &new_tty);
}

void tty_reset(int fd)
{
	tcsetattr(fd, TCSANOW, &save_tty);
}
/***************************************************************************
 *                       i/o functions                                     *
 ***************************************************************************/
char getch() /* block call */
{
    if (unget_flag) {
	    unget_flag = 0;
	    return unget_buf;
    }
    return igetch(); 
}
char getch1() /* non-block call */
{
	return getch();
}
void ungetch(int ch)
{
	unget_flag = 1;
	unget_buf = ch;
}
int get_cmd()
{
	unsigned char ch,ch1,ch2;

	while (1) {
		ch = getch();
		switch (state) {
		case S_EDIT: switch (ch) {
CMDAGAIN1:
			case ESC: switch (ch1=getch()) {
				case BRK:
					switch (getch()) {
					case 'A': return CMD_UP;
					case 'B': return CMD_DOWN;
					case 'C': return CMD_RIGHT;
					case 'D': return CMD_LEFT;
					case '1': if (getch() == '~') return CMD_HOME;
					case '2': if (getch() == '~') return CMD_LINEMODE;
					case '3': if (getch() == '~') return CMD_DEL;
					case '4': if (getch() == '~') return CMD_END;
					case '5': if (getch() == '~') return CMD_PGUP;
					case '6': if (getch() == '~') return CMD_PGDN;
					}
					goto BADCMD;
				case '`': return CMD_LISTSKEY;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9': code_first = ch1; return CMD_SKEY;
				case 'a': return CMD_ABSORB;
				case 'A': return CMD_ADDPEN;
				case 'b': return CMD_BCOLOR;
				case 'c': return CMD_ROWCOPY;
				case 'C': return CMD_COLCOPY;
				case 'd': return CMD_DELROW;
				case 'D': return CMD_DELCOL;
				case 'e': return CMD_ERASEROW;
				case 'E': return CMD_ERASECOL;
				case 'f': return CMD_FCOLOR;
				case 'g': switch (getch()) {
					case 'a': return CMD_G_ATTRIB;
					case 'b': return CMD_G_BCOLOR;
					case 'B': return CMD_G_BCOLOREXT;
					case 'c': return CMD_G_CHAR;
					case 'f': return CMD_G_FCOLOR;
					case 'F': return CMD_G_FCOLOREXT;
					case 'g': return CMD_G_ALL;
					case 'n': return CMD_G_BLINK;
					case 't': return CMD_G_BRIGHT;
					case 'u': return CMD_G_UNDERLINE;
					}
					goto BADCMD;
				case 'h': return CMD_HELPCMD;
				case 'H': return CMD_HELPCODE;
				case 'i': return CMD_INSROW;
				case 'I': return CMD_INSCOL;
				case 'j': return CMD_PREVPEN;
				case 'l': return CMD_LOAD;
				case 'L': return CMD_DELPEN;
				case 'm': return CMD_LINEMODE;
#if !defined(NOMAIL)
				case 'M': return CMD_MAIL;
#endif
				case 'n': return CMD_BLINK;
				case 'o': switch (ch2=getch()) {
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9': code_first = ch2; return CMD_SETSKEY;
					}
					goto BADCMD;
				case 'O': switch (getch()) {
					case 'A': return CMD_UP;
					case 'B': return CMD_DOWN;
					case 'C': return CMD_RIGHT;
					case 'D': return CMD_LEFT;
					}
					goto BADCMD;
				case 'p': return CMD_LISTPEN;
				case 's': return CMD_SAVE;
				case 't': return CMD_BRIGHT;
				case 'u': return CMD_UNDERLINE;
				case 'v': return CMD_ROWPASTE;
				case 'V': return CMD_COLPASTE;
				case 'X': return CMD_ERASEALL;
				case 'y': return CMD_ERASEUP;
				case 'Y': return CMD_ERASELEFT;
				case 'z': return CMD_ERASEDOWN;
				case 'Z': return CMD_ERASERIGHT;
				case ',': switch (getch()) {
					case 'a': return CMD_A4_ATTRIB;
					case 'b': return CMD_A4_BCOLOR;
					case 'B': return CMD_A4_BCOLOREXT;
					case 'f': return CMD_A4_FCOLOR;
					case 'F': return CMD_A4_FCOLOREXT;
					case 'n': return CMD_A4_BLINK;
					case 't': return CMD_A4_BRIGHT;
					case 'u': return CMD_A4_UNDERLINE;
					}
					goto BADCMD;
				case '.': switch (getch()) {
					case 'a': return CMD_A8_ATTRIB;
					case 'b': return CMD_A8_BCOLOR;
					case 'B': return CMD_A8_BCOLOREXT;
					case 'f': return CMD_A8_FCOLOR;
					case 'F': return CMD_A8_FCOLOREXT;
					case 'n': return CMD_A8_BLINK;
					case 't': return CMD_A8_BRIGHT;
					case 'u': return CMD_A8_UNDERLINE;
					}
					goto BADCMD;
				case ESC: goto CMDAGAIN1;
				default: goto BADCMD;
				}
			case CTRL_A: return CMD_APPLYATTRIB;
			case CTRL_B: return CMD_APPLYBCOLOR;
			case CTRL_C: return CMD_COPY;
			case CTRL_E: return CMD_ERASE;
			case CTRL_F: return CMD_APPLYFCOLOR;
			case CTRL_G: return CMD_GBK;
			case CTRL_H: 
			case DEL:return CMD_DEL;
			case CTRL_J: return CMD_NEXTPEN;
			case CTRL_K: return CMD_BLOCK;
			case CTRL_L: return CMD_REFRESH;
			case RETURN: return CMD_CONFIRM;
			case CTRL_N: return CMD_APPLYBLINK;
			case CTRL_P: return CMD_DRAW;
			case CTRL_Q: return CMD_QUIT;
			case CTRL_R: return CMD_RESET;
			case CTRL_T: return CMD_APPLYBRIGHT;
			case CTRL_U: return CMD_APPLYUNDERLINE;
			case CTRL_V: return CMD_PASTE;
#if defined(NOMAIL)
			case CTRL_W: return CMD_EXPORT;
#endif
			case CTRL_X: return CMD_ANSIPST;
			case CTRL_Y: return CMD_REDO;
			case CTRL_Z: return CMD_UNDO;
			}
			if (ch > 127) {
				chinese = 1;
				code_first = ch;
				code_second = getch();
				if (code_first == char_space[0] && code_second == char_space[1]) { /* convert chinese SPACE to english SPACE */
					chinese = 0;
					code_first = SPACE;
				}
				return CMD_DATA;
			}
			if (ch >= 32 && ch <=126) {
				chinese = 0;
				code_first = ch;
				return CMD_DATA;
			}
			break;
		case S_SELBLK: switch (ch) {
			case ESC: switch (ch1=getch()) {
				case BRK: switch (getch()) {
					case 'A': return CMD_UP;
					case 'B': return CMD_DOWN;
					case 'C': return CMD_RIGHT;
					case 'D': return CMD_LEFT;
					}
					continue;
				case 'O': switch (getch()) {
					case 'A': return CMD_UP;
					case 'B': return CMD_DOWN;
					case 'C': return CMD_RIGHT;
					case 'D': return CMD_LEFT;
					}
					continue;
				}
				continue;
			case CTRL_K: return CMD_BLOCK;
			case CTRL_L: return CMD_REFRESH;
			case CTRL_Q: return CMD_QUIT;
			}
			continue;
		case S_EDITBLK: switch (ch) {
			case ESC: switch (getch()){
				case BRK:
					switch (getch()) {
					case 'A': return CMD_UP;
					case 'B': return CMD_DOWN;
					case 'C': return CMD_RIGHT;
					case 'D': return CMD_LEFT;
					case '3': if (getch() == '~') return CMD_DEL;
					}
					continue;
				case 'O': switch (getch()) {
					case 'A': return CMD_UP;
					case 'B': return CMD_DOWN;
					case 'C': return CMD_RIGHT;
					case 'D': return CMD_LEFT;
					}
					continue;
				case 'a': return CMD_ABSORB;
				case 'b': return CMD_BCOLOR;
				case 'f': return CMD_FCOLOR;
				case 'g': switch (getch()) {
					case 'a': return CMD_G_ATTRIB;
					case 'b': return CMD_G_BCOLOR;
					case 'B': return CMD_G_BCOLOREXT;
					case 'c': return CMD_G_CHAR;
					case 'f': return CMD_G_FCOLOR;
					case 'F': return CMD_G_FCOLOREXT;
					case 'g': return CMD_G_ALL;
					case 'n': return CMD_G_BLINK;
					case 't': return CMD_G_BRIGHT;
					case 'u': return CMD_G_UNDERLINE;
					}
				case 'h': return CMD_HELPCMD;
				case 'n': return CMD_BLINK;
				case 't': return CMD_BRIGHT;
				case 'u': return CMD_UNDERLINE;
				case ',': switch (getch()) {
					case 'a': return CMD_A4_ATTRIB;
					case 'b': return CMD_A4_BCOLOR;
					case 'B': return CMD_A4_BCOLOREXT;
					case 'f': return CMD_A4_FCOLOR;
					case 'F': return CMD_A4_FCOLOREXT;
					case 'n': return CMD_A4_BLINK;
					case 't': return CMD_A4_BRIGHT;
					case 'u': return CMD_A4_UNDERLINE;
					}
					continue;
				case '.': switch (getch()) {
					case 'a': return CMD_A8_ATTRIB;
					case 'b': return CMD_A8_BCOLOR;
					case 'B': return CMD_A8_BCOLOREXT;
					case 'f': return CMD_A8_FCOLOR;
					case 'F': return CMD_A8_FCOLOREXT;
					case 'n': return CMD_A8_BLINK;
					case 't': return CMD_A8_BRIGHT;
					case 'u': return CMD_A8_UNDERLINE;
					}
					continue;
				}
				continue;
			case CTRL_A: return CMD_APPLYATTRIB;
			case CTRL_B: return CMD_APPLYBCOLOR;
			case CTRL_C: return CMD_COPY;
			case CTRL_F: return CMD_APPLYFCOLOR;
			case CTRL_H: 
			case DEL:return CMD_DEL;
			case CTRL_L: return CMD_REFRESH;
			case CTRL_N: return CMD_APPLYBLINK;
			case CTRL_R: return CMD_RESET;
			case CTRL_Q: return CMD_QUIT;
			case CTRL_T: return CMD_APPLYBRIGHT;
			case CTRL_U: return CMD_APPLYUNDERLINE;
			case CTRL_Y: return CMD_REDO;
			case CTRL_Z: return CMD_UNDO;
			}
			continue;
		case S_APST: switch (ch) {
			case CTRL_O: return CMD_ANSIMODE;
			case CTRL_Q: return CMD_QUIT;
			case RETURN: return CMD_CONFIRM;
			}
			if (ch > 127) {
				chinese = 1;
				code_first = ch;
				code_second = getch();
				return CMD_DATA;
			}
			if ((ch >= 32 && ch <=126) || ch == ESC) {
				chinese = 0;
				code_first = ch;
				return CMD_DATA;
			}
			continue;
		case S_HLPCMD: switch (ch) {
			case ESC: switch (ch1=getch1()) {
				case BRK: switch (getch1()) {
					case 'C': return CMD_RIGHT;
					case 'D': return CMD_LEFT;
					}
					continue;
				case 'O': switch (getch1()) {
					case 'C': return CMD_RIGHT;
					case 'D': return CMD_LEFT;
					}
					continue;
				}
				continue;
			case CTRL_Q: return CMD_QUIT;
			}
			continue;
		case S_HLPCODE: switch (ch) {
			case ESC: switch (ch1=getch1()) {
				case BRK: switch (getch1()) {
					case 'C': return CMD_RIGHT;
					case 'D': return CMD_LEFT;
					}
					continue;
				case 'O': switch (getch1()) {
					case 'C': return CMD_RIGHT;
					case 'D': return CMD_LEFT;
					}
					continue;
				}
				continue;
			case CTRL_Q: return CMD_QUIT;
			}
			continue;
		case S_MAIL: 
			switch (ch) {
			case ESC: switch (getch1()) {
				case BRK: switch (getch1()) {
					case '1':
					case '2':
					case '4':
					case '5':
					case '6': getch1(); continue;
					case '3': if (getch1() == '~') return CMD_DEL;
					}
					continue;
				case 'O': getch1(); continue;
				}
				continue;
			case CTRL_H:
			case DEL: return CMD_DEL;
			case CTRL_Q: return CMD_QUIT;
			case RETURN: return CMD_CONFIRM;
			}
			if (ch > 127) {
				chinese = 1;
				code_first = ch;
				code_second = getch();
				return CMD_DATA;
			}
			if (ch >= 32 && ch <=126) {
				chinese = 0;
				code_first = ch;
				return CMD_DATA;
			}
			continue;
		}
BADCMD:
		strcpy(message, "\033[31m未知命令,用<ESC>h参阅帮助.\033[0m");
		draw_sbar();
	}

}



/***************************************************************************
 *                       ansi functions                                    *
 ***************************************************************************/

void clr_scr()
{
	//prints("\033[2J");
	clear();
}

void clr2tail()
{
//	prints("\033[K");
	clrtoeol();
}

void gotoxy(int x, int y)
{
	struct charinfo *ci;
	if (x<0) {
		if (y>0) {
			x = scr_cols-1;
			y--;
		}
		else 
			x = y = 0;
	}
	else if (x>scr_cols-1) {
		if (y<scr_lns-2) {
			x = 0;
			y++;
		}
		else {
			x = scr_cols-1;
			y = scr_lns-2;
		}
	}
	y = (y<0? 0: y);
	y = (y>scr_lns-2? scr_lns-2: y);
	cur_x = x;
	cur_y = y;
	ci = &graph[cur_y][cur_x];
	if (ci->chinese && ci->half) {
		assert(cur_x);
		cur_x--;
	}
//	prints("\033[%d;%dH", cur_y+1, cur_x+1);
	move(cur_y, cur_x);
}
void gotoxy2(int x, int y)
{
	x = (x<0? 0: x);
	x = (x>scr_cols-1? scr_cols-1: x);
	y = (y<0? 0: y);
	y = (y>scr_lns-1? scr_lns-1: y);
	cur_x = x;
	cur_y = y;
//	prints("\033[%d;%dH", cur_y+1, cur_x+1);
	move(cur_y, cur_x);
}
/***************************************************************************
 *                       undo/redo functions                               *
 ***************************************************************************/
int redo(int* row1, int* row2) {
	int i;

	assert(undo_ptr);
	if (!undo_ptr->next) return 0;
	undo_ptr = undo_ptr->next;
	for (i=0; i<undo_ptr->fields; i++)
		memcpy(undo_ptr->address[i], undo_ptr->newdata[i], undo_ptr->len[i]);
	*row1 = undo_ptr->row1;
	*row2 = undo_ptr->row2;
	return 1;
}
int undo(int* row1, int* row2) {
	int i;

	if (undo_ptr == &undo_list || undo_ptr == undo_limit) return 0;
	for (i=0; i<undo_ptr->fields; i++)
		memcpy(undo_ptr->address[i], undo_ptr->olddata[i], undo_ptr->len[i]);
	*row1 = undo_ptr->row1;
	*row2 = undo_ptr->row2;
	undo_ptr = undo_ptr->prev;
	return 1;
}
void free_undoinfo(struct undoinfo *ui)
{
	int i;
	if (!ui) return;
	if (ui->fields) {
		for (i=0; i<ui->fields; i++) {
			free(ui->olddata[i]);
			free(ui->newdata[i]);
		}
		free(ui->address);
		free(ui->olddata);
		free(ui->newdata);
		free(ui->len);
		memset(ui, 0, sizeof(struct undoinfo));
	}
}
void undo_free(struct undoinfo *from) {
	struct undoinfo *p, *q;

	assert(from != &undo_list);
	p = from;
	while (p) {
		q = p->next;
		free_undoinfo(p);
		free(p);
		p = q;
		undo_num--;
	}
}
void chkpt_begin(int r1, int r2, int fds, ...)
{
	int i;
	va_list args;
	if (!chkpt_flag) return;
	if (fds <= 0) return;
	va_start(args, fds);

	free_undoinfo(&pending);
	pending.row1 = r1;
	pending.row2 = r2;
	pending.fields = fds;
	pending.address = (void**)malloc(sizeof(void*)*fds);
	pending.olddata = (void**)malloc(sizeof(void*)*fds);
	pending.newdata = (void**)malloc(sizeof(void*)*fds);
	pending.len = (int*)malloc(sizeof(int)*fds);

	for (i=0; i<fds; i++)  {
		void* addr = va_arg(args, void*);
		int ln = va_arg(args, int);
		pending.address[i] = addr;
		pending.len[i] = ln;
		pending.olddata[i] = malloc(ln);
		memcpy(pending.olddata[i], addr, ln);
		pending.newdata[i] = malloc(ln);
	}
}
void chkpt_commit()
{
	int i;
	struct undoinfo *ui, *p;
	if (!chkpt_flag) return;
	if (!pending.fields) return;
	assert(undo_ptr);
	undo_free(undo_ptr->next);
	undo_ptr->next = NULL;
	ui = (struct undoinfo*)malloc(sizeof(struct undoinfo));
	*ui = pending;
	for (i=0; i<ui->fields; i++)
		memcpy(ui->newdata[i], ui->address[i], ui->len[i]);
	memset(&pending, 0, sizeof(struct undoinfo));
	ui->dirt = dirt;

	undo_ptr->next = ui;
	ui->prev = undo_ptr;
	ui->next = NULL;
	undo_ptr = ui;
	undo_num++;
	if (undo_num > MAX_UNDO) {
		p = undo_list.next;
		undo_list.next = p->next;
		p->next->prev = &undo_list;
		free_undoinfo(p);
		free(p);
		undo_num--;
	}
}
/***************************************************************************
 *                       helper functions                                  *
 ***************************************************************************/
int createdir() 
{
	struct stat buf;
	sprintf(save_dir, "%s/%s", dir_prefix, user_id);
	if (!stat(save_dir, &buf) && S_ISDIR(buf.st_mode))
		return 1;
	if (!mkdir(save_dir, S_IRWXU))
		return 1;
	return 0;
}
void str2hex(char* s, unsigned char* hi, unsigned char* lo)
{
	if (strlen(s)!=4) return;
	*hi = (isdigit(s[0])? (s[0]-'0')*16: (s[0]-'a'+10)*16) + (isdigit(s[1])? s[1]-'0': s[1]-'a'+10);
	*lo = (isdigit(s[2])? (s[2]-'0')*16: (s[2]-'a'+10)*16) + (isdigit(s[3])? s[3]-'0': s[3]-'a'+10);
}
void clrchr(struct charinfo *c, struct ansiattr* attr) /* clear ONLY 1 char */
{
    memset(c, 0, sizeof(struct charinfo));
    c->code = SPACE;
	if (attr)
		c->attr = *attr;
}
void clrchr2(struct charinfo *c, struct ansiattr* attr) /* clear chinese char if possible */
{
	if (c->chinese && !c->half)
		clrchr(c++, attr);
	clrchr(c,attr);
}
int isdefault(struct ansiattr *attr)
{
	assert(attr);
	return (!attr->fcolor && !attr->bcolor && !attr->blink && !attr->bright && !attr->underline && !attr->reverse);
}
char* get_ansi_str(struct ansiattr *attr)
{
	static char s[100];
	char ch[10];

	assert(attr);
	if (isdefault(attr))
		strcpy(s, "\033[0m");
	else {
		strcpy(s, "\033[0m\033[");
		if (attr->fcolor) {
			sprintf(ch, "%d;", attr->fcolor);
			strcat(s, ch); 
		}
		if (attr->bcolor) {
			sprintf(ch, "%d;", attr->bcolor);
			strcat(s, ch); 
		}
		if (attr->blink)
			strcat(s, "5;"); 
		if (attr->bright) 
			strcat(s, "1;");
		if (attr->underline)
			strcat(s, "4;"); 
		if (attr->reverse)
			strcat(s, "7;");
		strcpy(s+strlen(s)-1, "m");
	}
	return s;
}
void set_cur_attr(int code)
{
	if (!code)
		memset(&cur_attr, 0, sizeof(struct ansiattr));
	else if (code>=30 && code<=37)
		cur_attr.fcolor = code;
	else if (code>=40 && code<=47)
		cur_attr.bcolor = code;
	else if (code == 5)
		cur_attr.blink = 1;
	else if (code == 1)
		cur_attr.bright = 1;
	else if (code == 4)
		cur_attr.underline = 1;
	else if (code == 7)
		cur_attr.reverse = 1;
}
void set_cur_char(struct ansiattr *attr)
{
	struct charinfo *ci;
	struct ansiattr a={0};

	if (attr)
		a = *attr;
	ci = &graph[cur_y][cur_x];
	ci->code = code_first;
	ci->attr = a;
	if ((ci->chinese = chinese)!=0) {
		if (cur_x == scr_cols-1) 
			clrchr(ci, &ci->attr);
		else {
			ci->half = 0;
			ci++;
			ci->code = code_second;
			ci->attr = a;
			ci->chinese = 1;
			ci->half = 1;
		}
	}
}
void set_cur_char2(struct ansiattr *attr)
{
	struct charinfo *ci;
	set_cur_char(attr);
	ci = &graph[cur_y][cur_x];
	assert(!ci->chinese || (ci->chinese && !ci->half));
	if (ci->chinese)
		ci+=2;
	else
		ci++;
	if (ci->chinese && ci->half) 
		clrchr(ci, &ci->attr);
}
void upd_cur_char(struct ansiattr *attr)
{
	struct charinfo *ci;
	struct ansiattr a={0};

	if (attr)
		a = *attr;
	ci = &graph[cur_y][cur_x];
	ci->attr = a;
	if (ci->chinese && !ci->half) {
		ci++;
		ci->attr = *attr;
	}
}
void opt_graph()
{
	int i, j;
	struct charinfo *ci;
	struct ansiattr a;

	for (i=0; i<scr_lns-1; i++) {
		memset(&a, 0, sizeof(struct ansiattr));
		for (j=0; j<scr_cols; j++) {
			ci = &graph[i][j];
			if (!(!ci->chinese && ci->code == SPACE)) {
				a = ci->attr;
				if (ci->chinese)
					j++;
				continue;
			}
			if (ci->attr.bcolor == a.bcolor && ci->attr.underline == a.underline && ci->attr.blink == a.blink)
				ci->attr = a;
		}
	}
}
char* get_char_str(struct charinfo *ci, int withansi)
{
	static char s[200];
	char ch[10];
	char *str=NULL;

	if (withansi) {
		str = get_ansi_str(&ci->attr);
		strcpy(s, str);
	}
	else
		*s = '\0';
	sprintf(ch, "%c", ci->code);
	strcat(s, ch);
	if (ci->chinese && !ci->half) {
		ci++;
		sprintf(ch,"%c",ci->code);
		strcat(s,ch);
	}
	if (withansi && strcmp(str, "\033[0m"))
		strcat(s, "\033[0m");
	return s;
}
char* get_row_str(int x, int y, int dup)
{
	static char s[1000];
	char ch[10];
	struct charinfo *ci;
	int fc, bc, bl, br, ul, re;
	int len;
	int open;
	int i;

	if (!dup)
		strcpy(s, "\033[0;");
	else 
		strcpy(s, "\033\033[0;");
	fc = bc = bl = br = ul = re = 0;
	open = 1;
	for (i=x; i<scr_cols; i++) {
		ci = &graph[y][i];
		assert(!ci->chinese || (ci->chinese && !ci->half));
		if ((fc != ci->attr.fcolor && !ci->attr.fcolor) || 
            (bc != ci->attr.bcolor && !ci->attr.bcolor) ||
            (bl != ci->attr.blink && !ci->attr.blink) ||
            (br != ci->attr.bright && !ci->attr.bright) ||
            (ul != ci->attr.underline && !ci->attr.underline) ||
            (re != ci->attr.reverse && !ci->attr.reverse)) {
			if (!dup)
				strcat(s, "\033[0;");
			else 
				strcat(s, "\033\033[0;");
			fc = bc = bl = br = ul = re = 0;
			open = 1;
		}
		if (fc != ci->attr.fcolor || bc != ci->attr.bcolor ||  bl != ci->attr.blink  ||  br != ci->attr.bright || ul != ci->attr.underline || re != ci->attr.reverse) {
			if (!open) {
				if (!dup)
					strcat(s, "\033[");
				else
					strcat(s, "\033\033[");
			}
			if (fc != ci->attr.fcolor) {
				sprintf(ch, "%d;", ci->attr.fcolor);
				strcat(s, ch);
			}
			if (bc != ci->attr.bcolor) {
				sprintf(ch, "%d;", ci->attr.bcolor);
				strcat(s, ch);
			}
			if (bl != ci->attr.blink)
				strcat(s, "5;");
			if (br != ci->attr.bright)
				strcat(s, "1;");
			if (ul != ci->attr.underline)
				strcat(s, "4;");
			if (re != ci->attr.reverse)
				strcat(s, "7;");
			fc = ci->attr.fcolor; bc = ci->attr.bcolor; bl = ci->attr.blink; br = ci->attr.bright; ul = ci->attr.underline; re = ci->attr.reverse;
			open = 1;
		}
		len = strlen(s);
		if (open)
			s[len-1] = 'm';
		s[len++] = ci->code;
		if (ci->chinese) {
			s[len++] = (ci+1)->code;
			i++;
		}
		s[len]='\0';
		open = 0;
	}
	if (fc || bc || bl || br || ul || re){
		if (dup)
			strcat(s, "\033\033[0m");
		else
			strcat(s, "\033[0m");
    }
	return s;
}
char* get_mail_str()
{
	int i;
	memcpy(save_graph, graph, sizeof(graph));
	opt_graph();
	for (i = 0; i<scr_lns-1; i++) {
		strcat(mail_str, get_row_str(0, i, 0));
		strcat(mail_str, "\n");
	}
	memcpy(graph, save_graph, sizeof(graph));
	return mail_str;
}
void load_cmdhelp()
{
	FILE* fp;
	char s[1024];
	int pages, lines;

	hcmd_total_pages = 0;
	sprintf(s, "%s/%s", dir_prefix, CMDFILE);
	if (!(fp=fopen(s, "r")))
		return;
	pages = lines = 0;
	while (!feof(fp)) {
		if (pages == MAX_HELP_PAGES)
			break;
		fgets(s, 1024, fp);
		if (!strncmp(s, "#END", 4)) {
			pages++;
			break;
		}
		if (!strncmp(s, "#PAGE", 5)) {
			pages++;
			lines = 0;
			continue;
		}
		if (lines > scr_lns-2)
			continue;
		s[strlen(s)-1] = '\0'; /* get rid of \n */
		cmd_help_str[pages][lines] = (char*)malloc(strlen(s)+1);
		strcpy(cmd_help_str[pages][lines], s);
		lines++;
	}
	hcmd_total_pages = pages;
	fclose(fp);
}
void unload_cmdhelp()
{
	int pages, lines;
	for (pages = 0; pages < hcmd_total_pages; pages++) {
		lines=0;
		while (lines < scr_lns-1 && cmd_help_str[pages][lines]) {
			free(cmd_help_str[pages][lines]);
			cmd_help_str[pages][lines] = NULL;
			lines++;
		}
	}
}
void save_cur() {
	save_x = cur_x; save_y = cur_y;
}
void load_cur() {
	gotoxy(save_x, save_y);
}
void init()
{
	int i,j;

	unget_flag = 0;
	cur_x = cur_y = 0;

	memset(graph, 0, sizeof(graph));
	for (i=0; i<scr_lns-1; i++)
		for (j=0; j<scr_cols; j++)
			graph[i][j].code = SPACE;

	copy_type = CP_CHAR;
	memset(char_copy, 0, sizeof(char_copy));
	char_copy[0].code = SPACE;

	memset(row_copy, 0, sizeof(row_copy));
	for (i=0; i<scr_cols; i++) 
		row_copy[i].code = SPACE;

	memset(col_copy, 0, sizeof(col_copy));
	for (i=0; i<scr_lns-1; i++)
		col_copy[i][0].code = SPACE;

	anchor_dirt = dirt = 0;
	memset(&cur_attr, 0, sizeof(struct ansiattr));
	memset(pens, 0, sizeof(pens));
	pens[0][0].code = SPACE;
	pens[1][0].code = 0xa8; pens[1][0].chinese = 1; pens[1][1].code = 0x80; pens[1][1].chinese = 1; pens[1][1].half = 1;
	pens[2][0].code = 0xa9; pens[2][0].chinese = 1; pens[2][1].code = 0xa4; pens[2][1].chinese = 1; pens[2][1].half = 1;
	pens[3][0].code = 0xa9; pens[3][0].chinese = 1; pens[3][1].code = 0xa6; pens[3][1].chinese = 1; pens[3][1].half = 1;
	pens[4][0].code = 0xa9; pens[4][0].chinese = 1; pens[4][1].code = 0xa5; pens[4][1].chinese = 1; pens[4][1].half = 1;
	pens[5][0].code = 0xa9; pens[5][0].chinese = 1; pens[5][1].code = 0xa7; pens[5][1].chinese = 1; pens[5][1].half = 1;
	cur_pen = 0;
	num_pens = 6;
	memset(sc_keys, 0, sizeof(sc_keys));
	for (i=0; i<NUM_SKEYS; i++) 
		sc_keys[i][0].code = SPACE;
	sc_keys[0][0].code = 0xa8; sc_keys[0][0].chinese = 1; sc_keys[0][1].code = 0x80; sc_keys[0][1].chinese = 1; sc_keys[0][1].half = 1;
	sc_keys[1][0].code = 0xa9; sc_keys[1][0].chinese = 1; sc_keys[1][1].code = 0xb0; sc_keys[1][1].chinese = 1; sc_keys[1][1].half = 1;
	sc_keys[2][0].code = 0xa9; sc_keys[2][0].chinese = 1; sc_keys[2][1].code = 0xb4; sc_keys[2][1].chinese = 1; sc_keys[2][1].half = 1;
	sc_keys[3][0].code = 0xa9; sc_keys[3][0].chinese = 1; sc_keys[3][1].code = 0xb8; sc_keys[3][1].chinese = 1; sc_keys[3][1].half = 1;
	sc_keys[4][0].code = 0xa9; sc_keys[4][0].chinese = 1; sc_keys[4][1].code = 0xbc; sc_keys[4][1].chinese = 1; sc_keys[4][1].half = 1;
	sc_keys[5][0].code = 0xa9; sc_keys[5][0].chinese = 1; sc_keys[5][1].code = 0xb3; sc_keys[5][1].chinese = 1; sc_keys[5][1].half = 1;
	sc_keys[6][0].code = 0xa9; sc_keys[6][0].chinese = 1; sc_keys[6][1].code = 0xb7; sc_keys[6][1].chinese = 1; sc_keys[5][1].half = 1;
	sc_keys[7][0].code = 0xa9; sc_keys[7][0].chinese = 1; sc_keys[7][1].code = 0xbb; sc_keys[7][1].chinese = 1; sc_keys[5][1].half = 1;
	sc_keys[8][0].code = 0xa9; sc_keys[8][0].chinese = 1; sc_keys[8][1].code = 0xbf; sc_keys[8][1].chinese = 1; sc_keys[5][1].half = 1;
	drawing = 0;
	linemode = 0;
	state = S_EDIT;
	*message='\0';
	undo_limit = undo_ptr = &undo_list;
	undo_num = 0;
	chkpt_flag = 1;
	apst_mode = 0;

	blk_x1 = 0; blk_y1 = 0;
	blk_x2 = scr_cols-1; blk_y2 = scr_lns-2;
	
	load_cmdhelp();
}
void draw_sbar()
{
	char s[200];
	char tmp1[20]={0};
	char x,y;

	switch (state) {
	case S_EDIT:
		sprintf(tmp1, "%s%s\033[0m", get_ansi_str(&cur_attr), char_diamond);
	
		sprintf(s, "\033[0m%s%s%02d,%02d%s%s%s%s%s%s%s%s%s%s%s", dirt_str[(anchor_dirt != undo_ptr->dirt)], char_sep, cur_x+1, cur_y+1, char_sep, tmp1, char_sep, get_char_str(&pens[cur_pen][0], 0), drawing_str[drawing], char_sep, linemode_str[linemode], char_sep, state_str[state], char_sep2, message);
		break;
	case S_SELBLK:
		sprintf(s, "\033[0m%02d,%02d-%02d,%02d%s%s%s%s", blk_x1+1, blk_y1+1, blk_x2+1, blk_y2+1, char_sep, state_str[state], char_sep2, message);
		break;
	case S_EDITBLK:
		sprintf(tmp1, "%s%s\033[0m", get_ansi_str(&cur_attr), char_diamond);
		sprintf(s, "\033[0m%s%s%02d,%02d-%02d,%02d%s%s%s%s%s%s", dirt_str[(anchor_dirt != undo_ptr->dirt)], char_sep, blk_x1+1, blk_y1+1, blk_x2+1, blk_y2+1, char_sep, tmp1, char_sep, state_str[state], char_sep2, message);
		break;
	case S_APST:
		sprintf(s, "\033[0m命令格式:%s%s%s%s%s", apst_mode_str[apst_mode], char_sep, state_str[state], char_sep2, message);
		break;
	case S_HLPCMD:
		sprintf(s, "\033[0m%s%s%s", state_str[state], char_sep2, message);
		break;
	case S_HLPCODE:
		sprintf(s, "\033[0m%s%s%s", state_str[state], char_sep2, message);
		break;
	case S_MAIL:
		sprintf(s, "\033[0m%s%s%s", state_str[state], char_sep2, message);
		break;
	default: assert(0);
	}
	x = cur_x; y = cur_y;
	gotoxy2(0,scr_lns-1);
	clr2tail();
	prints(s);
	gotoxy2(x, y);
}
void draw_graph()
{
	int i;

	save_cur();
	clr_scr();
	for (i=0; i<scr_lns-1; i++) {
		gotoxy(0,i);
		prints(get_row_str(0,i,0));
	}
	load_cur();
}
int confirm_quit()
{
	char ch;
	if (anchor_dirt == undo_ptr->dirt)
		return 1;
	clr_scr();
	draw_sbar();
	save_cur();
	gotoxy(20, 10);
	prints("\033[0m你所作的改动未保存,确定要退出吗(Y/N)?");	
	while (1) {
		ch = getch();
		if (toupper(ch) == 'Y')
			return 1;
		if (toupper(ch) == 'N') {
			load_cur();
			draw_graph();
			draw_sbar();
			return 0;
		}
	}
}
int read_progress()
{
	char fname[200];
	FILE* fp;
	if (!createdir()) return 0;
	sprintf(fname, "%s/%s", save_dir, save_file);
	if (!(fp=fopen(fname, "r"))) return 0;
	fread(graph, sizeof(graph), 1, fp);
	fread(&cur_x, sizeof(char), 1, fp);
	fread(&cur_y, sizeof(char), 1, fp);
	fread(&cur_attr, sizeof(struct ansiattr), 1, fp);
	fread(sc_keys, sizeof(sc_keys), 1, fp);
	fread(pens, sizeof(pens), 1, fp);
	fread(&num_pens, sizeof(char), 1, fp);
	fread(&cur_pen, sizeof(char), 1, fp);
	fread(&drawing, sizeof(char), 1, fp);
	fread(&linemode, sizeof(char), 1, fp);
	fread(message, sizeof(message), 1, fp);
	fread(mail_addr, sizeof(mail_addr), 1, fp);
	fread(mail_title, sizeof(mail_title), 1, fp);
	fread(&gbk_page, sizeof(int), 1, fp);
	fread(&hcmd_page, sizeof(int), 1, fp);
	fread(&apst_mode, sizeof(char), 1, fp);
	fclose(fp);
	draw_graph();
	return 1;
}
int write_progress()
{
	char fname[200];
	FILE* fp;
	if (!createdir()) return 0;
	sprintf(fname, "%s/%s", save_dir, save_file);
	if (!(fp=fopen(fname, "w"))) return 0;
	fwrite(graph, sizeof(graph), 1, fp);
	fwrite(&cur_x, sizeof(char), 1, fp);
	fwrite(&cur_y, sizeof(char), 1, fp);
	fwrite(&cur_attr, sizeof(struct ansiattr), 1, fp);
	fwrite(sc_keys, sizeof(sc_keys), 1, fp);
	fwrite(pens, sizeof(pens), 1, fp);
	fwrite(&num_pens, sizeof(char), 1, fp);
	fwrite(&cur_pen, sizeof(char), 1, fp);
	fwrite(&drawing, sizeof(char), 1, fp);
	fwrite(&linemode, sizeof(char), 1, fp);
	fwrite(message, sizeof(message), 1, fp);
	fwrite(mail_addr, sizeof(mail_addr), 1, fp);
	fwrite(mail_title, sizeof(mail_title), 1, fp);
	fwrite(&gbk_page, sizeof(int), 1, fp);
	fwrite(&hcmd_page, sizeof(int), 1, fp);
	fwrite(&apst_mode, sizeof(char), 1, fp);
	fclose(fp);
	return 1;
}
void send_mail()
{
	char s[100];
	FILE *fp;
	sprintf(s, DEFAULT_DIR_PREFIX "/ansieditor.%s", user_id);
	if (!(fp=fopen(s, "w"))) {
		mail_sent = -1;
		return;
	}
	fputs(get_mail_str(), fp);
	fprintf(fp, ".\n");
	fclose(fp);
	if (!strchr(mail_addr, '@')) {
		if (!mail_file(user_id, s, mail_addr, mail_title, 1,NULL)){
			mail_sent=1;
		} else {
			mail_sent=-1;
		}

	} else {
		if (!bbs_sendmail(s, mail_title, mail_addr, 0, 0, getSession())){
			mail_sent=1;
		} else {
			mail_sent=-1;
		}
	}
/*
	sprintf(s, "%s %s", SENDMAIL, mail_addr);
	if (!(fp=popen(s, "w"))) {
		mail_sent = -1;
		return;
	}
	fprintf(fp, "From: %s@%s\n", user_id, NAME_BBS_ENGLISH);
	fprintf(fp, "To: %s\n", mail_addr);
	fprintf(fp, "Subject: %s\n", mail_title);
	fprintf(fp, "MIME-Version: 1.0\n");
	fprintf(fp, "Content-Type: text/plain; charset=GB2312\n");
	fprintf(fp, "Content-Transfer-Encoding: 8bit\n");
	fputs(get_mail_str(), fp);
	fprintf(fp, ".\n");
	fclose(fp);
*/

}
int write_file()
{
	char fname[200];
	FILE* fp;
	if (!createdir()) return 0;
	sprintf(fname, "%s/%s", save_dir, export_file);
	if (!(fp=fopen(fname, "w"))) return 0;
	fputs(get_mail_str(), fp);
	fclose(fp);

	return 1;
}
void help_code(int page)
{
	int code;
	int i,j;
	char* url = "更多请参见:http://www.khngai.com/chinese/charmap/tblgbk.php?page=0";

	assert(page>=0 && page<gbk_total_pages);
	clr_scr();
	gotoxy2(34,0);
	prints("\033[0m部分GBK码");
	gotoxy2(17,1);
	prints("0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
	for (i=0; i<GBK_PAGE_RECS; i++) {
		code = gbk_recs[page*10+i];
		if (!code)
			break;
		gotoxy2(10, 2*i+2);
		prints("%4X: ", code);
		gotoxy2(17, 2*i+2);
		for (j=0; j<16; j++) {
			gotoxy2(17+3*j, cur_y);
			prints("%c%c",  ((code+j)>>8)&0x00FF, (code+j)&0x00FF);
		}
		gotoxy2(cur_x+3, cur_y);
		prints(":%X", code);
	}
	gotoxy2(17,cur_y+1);
	prints("0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
	gotoxy2((scr_cols-strlen(url))/2, cur_y+1);
	prints(url);
	sprintf(message, "\033[0m左右键翻页,CTRL_Q退出,第%d/%d页", page+1, gbk_total_pages);
	gotoxy2(scr_cols-1, scr_lns-1);
	draw_sbar();
}
void help_cmd(int page)
{
	int i;

	i = 0;
	clr_scr();
	while (i < scr_lns-1 && cmd_help_str[page][i]) {
		gotoxy2(0, i);
		prints(cmd_help_str[page][i++]);
	}
	sprintf(message, "\033[0m左右键翻页,CTRL_Q退出,第%d/%d页", page+1, hcmd_total_pages);
	gotoxy2(scr_cols-1, scr_lns-1);
	draw_sbar();
}
void clr_brokenchars(struct charinfo g[][NUM_COLS], int row1, int row2, int col, int which)
{
	int i;
	struct charinfo *ci;
	assert(row1<=row2);
	for (i=row1; i<=row2; i++) {
		ci = &g[i][col];
		if (!which) { /* clear first byte */
			if (ci->chinese && !ci->half) {
				if (col == scr_cols-1)
					clrchr(ci, &ci->attr);
				else
					clrchr2(ci, &ci->attr);
			}
		}
		else { /* clear second byte */
			if (ci->chinese && ci->half) {
				assert(col>0);
				ci--;
				clrchr2(ci, &ci->attr);
			}
		}
	}
}
int clr_reverse()
{
	int i, j;
	for (i=0; i<scr_lns-1; i++)
		for (j=0; j<scr_cols; j++) 
			graph[i][j].attr.reverse = 0;
    return 0;
}
void set_blk_attr(int type, int value)
{
	int i,j;
	struct charinfo *ci;
	char *f1, *f2, *f3;
	int x1, y1, x2, y2, swap;

	x1 = blk_x1; y1 = blk_y1; x2 = blk_x2; y2 = blk_y2;
	if (x1 > x2) {
		swap = x2; x2 = x1; x1 = swap;
	}
	if (y1 > y2) {
		swap = y2; y2 = y1; y1 = swap;
	}
	for (i=y1; i<=y2; i++) {
		for (j=x1; j<=x2; j++) {
			ci = &graph[i][j];
			switch (type) {
				case A_FCOLOR: f1 = &(ci-1)->attr.fcolor; f2 = &ci->attr.fcolor; f3 = &(ci+1)->attr.fcolor; 
					break;
				case A_BCOLOR: f1 = &(ci-1)->attr.bcolor; f2 = &ci->attr.bcolor; f3 = &(ci+1)->attr.bcolor; 
					break;
				case A_BLINK: f1 = &(ci-1)->attr.blink; f2 = &ci->attr.blink; f3 = &(ci+1)->attr.blink; 
					break;
				case A_BRIGHT: f1 = &(ci-1)->attr.bright; f2 = &ci->attr.bright; f3 = &(ci+1)->attr.bright; 
					break;
				case A_UNDERLINE: f1 = &(ci-1)->attr.underline; f2 = &ci->attr.underline; f3 = &(ci+1)->attr.underline; 
					break;
				case A_REVERSE: f1 = &(ci-1)->attr.reverse; f2 = &ci->attr.reverse; f3 = &(ci+1)->attr.reverse; 
					break;
				default: assert(0);

			}
			*f2 = value;
			if (ci->chinese && ci->half)
				*f1 = value;
			else if (ci->chinese && !ci->half) {
				*f3 = value;
				j++;
			}
		}
	}
	save_cur();
	for (i=y1; i<=y2; i++) {
		gotoxy(0, i);
		clr2tail();
		prints(get_row_str(0, i, 0));
	}
	load_cur();
}
void clr_blk()
{
	int i, j;
	struct charinfo *ci, *ci1;

	ci = &graph[blk_y1][blk_x2];
	ci1 = &graph[blk_y2][blk_x2];

	if ((ci->chinese && !ci->half) || (ci1->chinese && !ci1->half))
		blk_x2++;
	clr_brokenchars(graph, blk_y1, blk_y2, blk_x1, 1);
	clr_brokenchars(graph, blk_y1, blk_y2, blk_x2, 0);
	for (i=blk_y1; i<=blk_y2; i++)
		for (j=blk_x1; j<=blk_x2; j++)
			clrchr(&graph[i][j], NULL);
	save_cur();
	for (i=blk_y1; i<=blk_y2; i++) {
		gotoxy(0, i);
		clr2tail();
		prints(get_row_str(0, i, 0));
	}
	load_cur();
}
void global_replace(int type)
{
	struct charinfo *ci;
	struct charinfo c1, c2;
	int i, j;
    memset(&c2,0,sizeof(struct charinfo));
	c1 = graph[cur_y][cur_x];
	if (c1.chinese)
		c2 = graph[cur_y][cur_x+1];
	for (i=blk_y1; i<=blk_y2; i++) {
		for (j=blk_x1; j<=blk_x2; j++) {
			ci = &graph[i][j];
			switch (type) {
				case A_FCOLOR:
					if (ci->attr.fcolor == c1.attr.fcolor) 
						ci->attr.fcolor = cur_attr.fcolor;
					break;
				case A_FCOLOREXT:
					if (ci->attr.fcolor == c1.attr.fcolor)
						ci->attr.fcolor = cur_attr.fcolor;
					if (SAME_COLOR(ci->attr.bcolor, c1.attr.fcolor))
						ci->attr.bcolor = COLOR_F2B(cur_attr.fcolor);
					break;
				case A_BCOLOR:
					if (ci->attr.bcolor == c1.attr.bcolor)
						ci->attr.bcolor = cur_attr.bcolor;
					break;
				case A_BCOLOREXT:
					if (SAME_COLOR(ci->attr.fcolor, c1.attr.bcolor))
						ci->attr.fcolor = COLOR_B2F(cur_attr.bcolor);
					if (ci->attr.bcolor == c1.attr.bcolor)
						ci->attr.bcolor = cur_attr.bcolor;
					break;
				case A_BLINK:
					if (ci->attr.blink == c1.attr.blink)
						ci->attr.blink= cur_attr.blink;
					break;
				case A_BRIGHT:
					if (ci->attr.bright == c1.attr.bright)
						ci->attr.bright= cur_attr.bright;
					break;
				case A_UNDERLINE:
					if (ci->attr.underline== c1.attr.underline)
						ci->attr.underline= cur_attr.underline;
					break;
				case A_ATTRIB:
					if (!memcmp(&ci->attr, &c1.attr, sizeof(struct ansiattr)))
						ci->attr = cur_attr;
					break;
				case A_CHAR:
					if ((!c1.chinese && !ci->chinese && ci->code == c1.code) || (c1.chinese && ci->chinese && ci->code == c1.code && (ci+1)->code == c2.code)) {
						ci->attr = cur_attr;
						if (ci->chinese) {
							(ci+1)->attr = cur_attr;
							j++;
						}
					}
					break;
				case A_ALL:
                    if (((!c1.chinese && !ci->chinese && ci->code == c1.code) || (c1.chinese && ci->chinese && ci->code == c1.code && (ci+1)->code == c2.code)) && !memcmp(&ci->attr, &c1.attr, sizeof(struct ansiattr))) {
						ci->attr = cur_attr;
						if (ci->chinese) {
							(ci+1)->attr = cur_attr;
							j++;
						}
					}
					break;
			}
		}
	}
	for (i=blk_y1; i<=blk_y2; i++) {
		ci = &graph[i][blk_x1];
		if (ci->chinese && ci->half)
			(ci-1)->attr = ci->attr;
		ci = &graph[i][blk_x2];
		if (ci->chinese && !ci->half)
			(ci+1)->attr = ci->attr;
	}
}
void adj_replace(int method, int x, int y, int type, struct ansiattr* attr)
{
	int i;
	struct charinfo *ci;
	int cx, cy;

	if (adj_visited[y][x])
		return;
	adj_visited[y][x] = 1;
	ci = &graph[y][x];
	switch (type) {
		case A_FCOLOR: 
			if (ci->attr.fcolor != attr->fcolor) 
				return;
			ci->attr.fcolor = cur_attr.fcolor;
			break;
		case A_FCOLOREXT:
			if (ci->attr.fcolor != attr->fcolor && !SAME_COLOR(ci->attr.bcolor, attr->fcolor))
				return;
			if (ci->attr.fcolor == attr->fcolor)
				ci->attr.fcolor = cur_attr.fcolor;
			if (SAME_COLOR(ci->attr.bcolor, attr->fcolor))
				ci->attr.bcolor = COLOR_F2B(cur_attr.fcolor);
			break;
		case A_BCOLOR:
			if (ci->attr.bcolor != attr->bcolor) 
				return;
			ci->attr.bcolor = cur_attr.bcolor;
			break;
		case A_BCOLOREXT:
			if (ci->attr.bcolor != attr->bcolor && !SAME_COLOR(ci->attr.fcolor, attr->bcolor))
				return;
			if (ci->attr.bcolor == attr->bcolor)
				ci->attr.bcolor = cur_attr.bcolor;
			if (SAME_COLOR(ci->attr.fcolor, attr->bcolor))
				ci->attr.fcolor = COLOR_B2F(cur_attr.bcolor);
			break;
		case A_BLINK:
			if (ci->attr.blink!= attr->blink) 
				return;
			ci->attr.blink= cur_attr.blink;
			break;
		case A_BRIGHT:
			if (ci->attr.bright!= attr->bright) 
				return;
			ci->attr.bright= cur_attr.bright;
			break;
		case A_UNDERLINE:
			if (ci->attr.underline!= attr->underline) 
				return;
			ci->attr.underline= cur_attr.underline;
			break;
		case A_ATTRIB:
			if (memcmp(&ci->attr, attr, sizeof(struct ansiattr)))
				return;
			ci->attr = cur_attr;
			break;
	}
	if (ci->chinese && ci->half && x == blk_x1)
		(ci-1)->attr = ci->attr;
	if (ci->chinese && !ci->half && x == blk_x2)
		(ci+1)->attr = ci->attr;
	if (method == ADJACENT_4) {
		for (i=0; i<4; i++) {
			cx = x+a4_dir[i][0];
			cy = y+a4_dir[i][1];
			if (cx<blk_x1 || cx > blk_x2 || cy<blk_y1 || cy>blk_y2)
				continue;
			adj_replace(method, cx, cy, type, attr);
		}
	}
	else if (method == ADJACENT_8) {
		for (i=0; i<8; i++) {
			cx = x+a8_dir[i][0];
			cy = y+a8_dir[i][1];
			if (cx<blk_x1 || cx > blk_x2 || cy<blk_y1 || cy>blk_y2)
				continue;
			adj_replace(method, cx, cy, type, attr);
		}
	}
}
void adjacent_replace(int method, int type)
{
	static struct ansiattr a;

	a = graph[cur_y][cur_x].attr;
	memset(adj_visited, 0, sizeof(adj_visited));
	adj_replace(method, cur_x, cur_y, type, &a);
}
/***************************************************************************
 *                       callback functions                                *
 ***************************************************************************/
void _data()
{
	struct charinfo *ci, *ci1;
	static struct charinfo s_row[NUM_COLS]; 
	static struct charinfo s_col[NUM_ROWS-2][2];
	int i;

	if (linemode == HINS) {
		chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][0], sizeof(struct charinfo)*scr_cols);
		memcpy(s_row, graph[cur_y], sizeof(struct charinfo)*scr_cols);
		set_cur_char(&cur_attr);
		for (i = cur_x+1+chinese; i < scr_cols; i++) 
			graph[cur_y][i] = s_row[i-1-chinese];
		ci = &graph[cur_y][scr_cols-1];
		if (ci->chinese && !ci->half)
			clrchr(ci, &ci->attr);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));
		gotoxy(cur_x+1+chinese, cur_y);
		dirt++;
		chkpt_commit();
		draw_sbar();
	}
	else if (linemode == HOVW) {
		chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3);
		set_cur_char2(&cur_attr);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));
		gotoxy(cur_x+1+chinese, cur_y);
		dirt++;
		chkpt_commit();
		draw_sbar();
	}
	else if (linemode == VINS) {
		chkpt_begin(cur_y, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][0], sizeof(struct charinfo)*scr_cols*(scr_lns-cur_y-1));
		clr_brokenchars(graph, cur_y, scr_lns-2, cur_x, 1);
		for (i = cur_y; i<scr_lns-1; i++)
			memcpy(&s_col[i][0], &graph[i][cur_x], sizeof(struct charinfo)*2);
		clr_brokenchars(graph, cur_y, scr_lns-2, cur_x, 0);
		for (i = cur_y; i<scr_lns-2; i++) {
			ci = &graph[i+1][cur_x];
			ci1 = &s_col[i][0];
			*ci = *ci1;
			if (ci->chinese) {
				ci++; ci1++;
				*ci = *ci1;
				ci++;
				if (ci->chinese && ci->half) 
					clrchr(ci, &ci->attr);
			}
		}
		set_cur_char2(&cur_attr);
		save_cur();
		for (i = cur_y; i<scr_lns-1; i++) {
			gotoxy(0, i);
			clr2tail();
			prints(get_row_str(0, i, 0));
		}
		load_cur();
		gotoxy(cur_x, cur_y+1);
		dirt++;
		chkpt_commit();
		draw_sbar();

	}
	else if (linemode == VOVW) {
		if (cur_y == scr_lns-2) 
			chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3);
		else
			chkpt_begin(cur_y, cur_y+1, 4, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3, &graph[cur_y+1][cur_x-1], sizeof(struct charinfo)*4);
		set_cur_char2(&cur_attr);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));
		if (cur_y < scr_lns-2) {
			clr_brokenchars(graph, cur_y+1, cur_y+1, cur_x, 1);
			save_cur();
			gotoxy(0, cur_y+1);
			clr2tail();
			prints(get_row_str(0, cur_y, 0));
			load_cur();
			gotoxy(cur_x, cur_y+1);
		}
		dirt++;
		chkpt_commit();
		draw_sbar();
	}
	else
		assert(0);
}
void _curup()
{
	struct charinfo *ci;
	if (drawing == DRAW) {
		chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3);
		ci = &pens[cur_pen][0];
		code_first = ci->code;
		if ((chinese = ci->chinese)!=0)
			code_second = (ci+1)->code;
		if (!cur_pen)
			clrchr2(&graph[cur_y][cur_x], NULL);
		else
			set_cur_char2(&cur_attr);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));
		gotoxy(cur_x, cur_y-1);
		dirt++;
		chkpt_commit();
	}
	else if (drawing == UPDATE) {
		chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3);
		upd_cur_char(&cur_attr);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));
		gotoxy(cur_x, cur_y-1);
		dirt++;
		chkpt_commit();
	}
	else
		gotoxy(cur_x, cur_y-1);
	draw_sbar();
}
void _curdn()
{
	struct charinfo *ci;
	if (drawing == DRAW) {
		chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3);
		ci = &pens[cur_pen][0];
		code_first = ci->code;
		if ((chinese = ci->chinese)!=0)
			code_second = (ci+1)->code;
		if (!cur_pen)
			clrchr2(&graph[cur_y][cur_x], NULL);
		else
			set_cur_char2(&cur_attr);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));
		gotoxy(cur_x, cur_y+1);
		dirt++;
		chkpt_commit();
	}
	else if (drawing == UPDATE) {
		chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3);
		upd_cur_char(&cur_attr);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));
		gotoxy(cur_x, cur_y+1);
		dirt++;
		chkpt_commit();
	}
	else
		gotoxy(cur_x, cur_y+1);
	draw_sbar();
}
void _currt()
{
	struct charinfo *ci, *ci1;
	ci = &graph[cur_y][cur_x];
	if (drawing == DRAW) {
		chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3);
		ci1 = &pens[cur_pen][0];
		code_first = ci1->code;
		if ((chinese = ci1->chinese)!=0)
			code_second = (ci1+1)->code;
		if (!cur_pen)
			clrchr2(&graph[cur_y][cur_x], NULL);
		else
			set_cur_char2(&cur_attr);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));
		assert(!ci->chinese || (ci->chinese && !ci->half));
		if (ci->chinese)
			gotoxy(cur_x+2, cur_y);
		else
			gotoxy(cur_x+1, cur_y);
		dirt++;
		chkpt_commit();
	}
	else if (drawing == UPDATE) {
		chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3);
		upd_cur_char(&cur_attr);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));
		assert(!ci->chinese || (ci->chinese && !ci->half));
		if (ci->chinese)
			gotoxy(cur_x+2, cur_y);
		else
			gotoxy(cur_x+1, cur_y);
		dirt++;
		chkpt_commit();
	}
	else {
		assert(!ci->chinese || (ci->chinese && !ci->half));
		if (ci->chinese)
			gotoxy(cur_x+2, cur_y);
		else
			gotoxy(cur_x+1, cur_y);
	}
	draw_sbar();
}
void _curlf()
{
	struct charinfo *ci, *ci1;

	ci = &graph[cur_y][cur_x];
	if (drawing == DRAW) {
		chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3);
		ci1 = &pens[cur_pen][0];
		code_first = ci1->code;
		if ((chinese = ci1->chinese)!=0)
			code_second = (ci1+1)->code;
		if (!cur_pen)
			clrchr2(&graph[cur_y][cur_x], NULL);
		else
			set_cur_char2(&cur_attr);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));

		assert(!ci->chinese || (ci->chinese && !ci->half));
		if (ci->chinese)
			gotoxy(cur_x-2, cur_y);
		else
			gotoxy(cur_x-1, cur_y);
		dirt++;
		chkpt_commit();
	}
	else if (drawing == UPDATE) {
		chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3);
		upd_cur_char(&cur_attr);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));
		assert(!ci->chinese || (ci->chinese && !ci->half));
		if (cur_x>0 && (ci-1)->chinese)
			gotoxy(cur_x-2, cur_y);
		else
			gotoxy(cur_x-1, cur_y);
		dirt++;
		chkpt_commit();
	}
	else {
		assert(!ci->chinese || (ci->chinese && !ci->half));
		if (cur_x>0 && (ci-1)->chinese)
			gotoxy(cur_x-2, cur_y);
		else
			gotoxy(cur_x-1, cur_y);
	}
	draw_sbar();
}
void _home()
{
	gotoxy(0,cur_y);
	draw_sbar();
}
void _endl()
{
	gotoxy(scr_cols-1, cur_y);
	draw_sbar();
}
void _pgup()
{
	gotoxy(0,0);
	draw_sbar();
}
void _pgdn()
{
	gotoxy(scr_cols-1, scr_lns-2);
	draw_sbar();
}
void _absob()
{
	chkpt_begin(-1, -1, 1, &cur_attr, sizeof(struct ansiattr));
	cur_attr = graph[cur_y][cur_x].attr;
	chkpt_commit();
	strcpy(message, "\033[0m当前文本的属性被采用.");
	draw_sbar();
}
void _reset()
{
	chkpt_begin(-1, -1, 1, &cur_attr, sizeof(struct ansiattr));
	memset(&cur_attr, 0, sizeof(struct ansiattr));
	chkpt_commit();
	draw_sbar();
}
void _aplfc()
{
	struct charinfo *ci;

	chkpt_begin(cur_y, cur_y, 1, &graph[cur_y][cur_x], sizeof(struct charinfo)*2);
	ci = &graph[cur_y][cur_x];
	assert(!ci->chinese || (ci->chinese && !ci->half));
	ci->attr.fcolor = cur_attr.fcolor;
	prints(get_char_str(ci,1));
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _aplbc()
{
	struct charinfo *ci;

	chkpt_begin(cur_y, cur_y, 1, &graph[cur_y][cur_x], sizeof(struct charinfo)*2);
	ci = &graph[cur_y][cur_x];
	assert(!ci->chinese || (ci->chinese && !ci->half));
	ci->attr.bcolor = cur_attr.bcolor;
	prints(get_char_str(ci,1));
	dirt++;
	chkpt_commit();
	draw_sbar();

}
void _aplbl()
{
	struct charinfo *ci;

	chkpt_begin(cur_y, cur_y, 1, &graph[cur_y][cur_x], sizeof(struct charinfo)*2);
	ci = &graph[cur_y][cur_x];
	assert(!ci->chinese || (ci->chinese && !ci->half));
	ci->attr.blink = cur_attr.blink;
	prints(get_char_str(ci,1));
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _aplbr()
{ 
	struct charinfo *ci;

	chkpt_begin(cur_y, cur_y, 1, &graph[cur_y][cur_x], sizeof(struct charinfo)*2);
	ci = &graph[cur_y][cur_x];
	assert(!ci->chinese || (ci->chinese && !ci->half));
	ci->attr.bright = cur_attr.bright;
	prints(get_char_str(ci,1));
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _aplul()
{
	struct charinfo *ci;

	chkpt_begin(cur_y, cur_y, 1, &graph[cur_y][cur_x], sizeof(struct charinfo)*2);
	ci = &graph[cur_y][cur_x];
	assert(!ci->chinese || (ci->chinese && !ci->half));
	ci->attr.underline = cur_attr.underline;
	prints(get_char_str(ci,1));
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _aplattr()
{
	struct charinfo *ci;

	chkpt_begin(cur_y, cur_y, 1, &graph[cur_y][cur_x], sizeof(struct charinfo)*2);
	ci = &graph[cur_y][cur_x];
	assert(!ci->chinese || (ci->chinese && !ci->half));
	ci->attr = cur_attr;
	prints(get_char_str(ci,1));
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _setfc()
{
	chkpt_begin(-1, -1, 1, &cur_attr, sizeof(struct ansiattr));
	cur_attr.fcolor = NEXT_FCOLOR(cur_attr.fcolor);
	chkpt_commit();
	draw_sbar();
}
void _setbc()
{
	chkpt_begin(-1, -1, 1, &cur_attr, sizeof(struct ansiattr));
	cur_attr.bcolor = NEXT_BCOLOR(cur_attr.bcolor);
	chkpt_commit();
	draw_sbar();
}
void _setbl()
{
	chkpt_begin(-1, -1, 1, &cur_attr, sizeof(struct ansiattr));
	cur_attr.blink = !(cur_attr.blink);
	chkpt_commit();
	draw_sbar();
}
void _setbr()
{
	chkpt_begin(-1, -1, 1, &cur_attr, sizeof(struct ansiattr));
	cur_attr.bright = !(cur_attr.bright);
	chkpt_commit();
	draw_sbar();
}
void _setul()
{
	chkpt_begin(-1, -1, 1, &cur_attr, sizeof(struct ansiattr));
	cur_attr.underline = !(cur_attr.underline);
	chkpt_commit();
	draw_sbar();
}
void _setlm()
{
	chkpt_begin(-1, -1, 1, &linemode, sizeof(char));
	linemode = (linemode+1)%NUM_LINEMODES;
	draw_sbar();
	chkpt_commit();
} 
void _setp0()
{
	chkpt_begin(-1, -1, 2, &cur_pen, sizeof(char), &drawing, sizeof(char));
	cur_pen = 0;
	drawing = 1;
	chkpt_commit();
	draw_sbar();

}
void _apen()
{
	struct charinfo *ci;
	
	if (num_pens == MAX_PENS)
		sprintf(message, "\033[31m10支笔已满,无法再添加.\033[0m");
	else {
		ci = &graph[cur_y][cur_x];
		assert(!ci->chinese || (ci->chinese && !ci->half));
		chkpt_begin(-1, -1, 3, &cur_pen, sizeof(char), &num_pens, sizeof(char), pens, sizeof(pens));
		pens[num_pens][0] = *ci;
		if (ci->chinese)
			pens[num_pens][1] = *(ci+1);		
		cur_pen = num_pens;
		num_pens++;
		sprintf(message, "\033[0m画笔%s被设为当前画笔.", get_char_str(&pens[cur_pen][0],0));
		chkpt_commit();
	}
	draw_sbar();
}
void _dpen()
{
	int i;
	char s[200];

	if (!cur_pen) 
		strcpy(message, "\033[31m此0号笔是橡皮擦,不能被删除.\033[0m");
	else {
		chkpt_begin(-1, -1, 3, &cur_pen, sizeof(char), &num_pens, sizeof(char), pens, sizeof(pens));
		strcpy(s, get_char_str(&pens[cur_pen][0],0));
		for (i=cur_pen; i<num_pens-1; i++) {
			pens[i][0] = pens[i+1][0];
			pens[i][1] = pens[i+1][1];
		}
		num_pens--;
		if (cur_pen >= num_pens)
			cur_pen = num_pens-1;
		sprintf(message, "\033[0m画笔%s已被删除.",s);
		chkpt_commit();
	}
	draw_sbar();
}
void _lpen()
{
	int i;
	char s[200];

	strcpy(s, "\033[0m笔:橡皮擦");
	for (i=1; i<num_pens; i++) {
		strcat(s, " ");
		strcat(s, get_char_str(&pens[i][0],0));
	}
	strcpy(message, s);
	draw_sbar();
}
void _npen()
{
	chkpt_begin(-1, -1, 1, &cur_pen, sizeof(char));
	cur_pen = (cur_pen+1)%num_pens;
	draw_sbar();
	chkpt_commit();
}
void _ppen()
{
	chkpt_begin(-1, -1, 1, &cur_pen, sizeof(char));
	cur_pen = (cur_pen-1+num_pens)%num_pens;
	draw_sbar();
	chkpt_commit();
}
void _irow()
{
	int i;

	chkpt_begin(cur_y, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][0], sizeof(struct charinfo)*scr_cols*(scr_lns-1-cur_y));
	for (i=scr_lns-2; i>cur_y; i--)
		memcpy(&graph[i][0], &graph[i-1][0], sizeof(struct charinfo)*scr_cols);
	for (i=0; i<scr_cols; i++)
		clrchr(&graph[cur_y][i], NULL);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _drow()
{
	int i;

	chkpt_begin(cur_y, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][0], sizeof(struct charinfo)*scr_cols*(scr_lns-1-cur_y));
	for (i=cur_y; i<scr_lns-2;i++)
		memcpy(&graph[i][0], &graph[i+1][0], sizeof(struct charinfo)*scr_cols);
	for (i=0; i<scr_cols; i++)
		clrchr(&graph[scr_lns-2][i], NULL);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _erow()
{
	int i;

	chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][0], sizeof(struct charinfo)*scr_cols);
	for (i=0; i<scr_cols; i++)
		clrchr(&graph[cur_y][i], NULL);
	dirt++;
	chkpt_commit();
	save_cur();
	gotoxy(0, cur_y);
	clr2tail();
	prints(get_row_str(0, cur_y, 0));
	load_cur();
	draw_sbar();
}
void _icol()
{
	int i,j;

	chkpt_begin(0, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	clr_brokenchars(graph, 0, scr_lns-2, cur_x, 1);
	for (i=0; i<scr_lns-1; i++) {
		for (j=scr_cols-1; j>cur_x; j--)
			graph[i][j] = graph[i][j-1];
		clrchr(&graph[i][cur_x], NULL);
	}
	clr_brokenchars(graph, 0, scr_lns-2, scr_cols-1, 0);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _dcol()
{
	int i,j;

	chkpt_begin(0, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	clr_brokenchars(graph, 0, scr_lns-2, cur_x, 1);
	clr_brokenchars(graph, 0, scr_lns-2, cur_x, 0);
	for (i=0; i<scr_lns-1; i++) {
		for (j=cur_x; j<scr_cols-1; j++)
			graph[i][j] = graph[i][j+1];
		clrchr(&graph[i][scr_cols-1], NULL);
	}
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _ecol()
{
	int i;

	chkpt_begin(0, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	clr_brokenchars(graph, 0, scr_lns-2, cur_x, 1);
	clr_brokenchars(graph, 0, scr_lns-2, cur_x, 0);
	for (i=0; i<scr_lns-1; i++)
		clrchr(&graph[i][cur_x], NULL);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _hcmd()
{
	if (!hcmd_total_pages) {
		strcpy(message, "\033[31m无法装载命令帮助文件.\033[0m");
		state = prev_state;
		draw_sbar();
		return;
	}
	prev_hcmd_state = prev_state;
	save_cur_x = cur_x; save_cur_y = cur_y;
	help_cmd(hcmd_page);
	draw_sbar();
}
void _hcode()
{
	save_cur_x = cur_x; save_cur_y = cur_y;
	help_code(gbk_page);
}
void _load()
{
	unsigned char ch;
	int n;

	while (1) {
		sprintf(message, "\033[0m读取进度(CTRL_Q取消):%s", save_file);
		draw_sbar();
		n = strlen(save_file);
		ch = getch();
		if (ch == ESC && getch1() == BRK && getch1()=='3' && getch1()=='~')
			ch = DEL;
		if (n < 15 && (isalnum(ch) || ch=='_' || ch=='-' || ch=='.')) {
			save_file[n++]=ch;
			save_file[n]='\0';
		}
		else if (ch & 0x80)
			getch();
		else if ((ch == DEL || ch == CTRL_H) && n) 
			save_file[n-1] = '\0';
		else if (ch == CTRL_Q) {
			strcpy(message, "\033[0m放弃读取.");
			break;
		}
		else if (ch == RETURN) {
			if (!n) 
				strcpy(message, "\033[0m放弃读取.");
			else if (read_progress()) {
				anchor_dirt = dirt = 0;
				sprintf(message, "\033[0m进度文件%s读取成功.", save_file);
				free_undoinfo(undo_list.next);
				undo_list.dirt = 0;
				undo_num = 0;
				chkpt_flag = 1;
				undo_ptr = &undo_list;
			}
			else
				strcpy(message, "\033[31m读取失败.\033[0m");
			break;
		}
	}
	draw_sbar();
}
void _save()
{
	unsigned char ch;
	int n;

	while (1) {
		sprintf(message, "\033[0m保存进度(CTRL_Q取消):%s", save_file);
		draw_sbar();
		n = strlen(save_file);
		ch = getch();
		if (ch == ESC && getch1() == BRK && getch1()=='3' && getch1()=='~')
			ch = DEL;
		if (n < 15 && (isalnum(ch) || ch=='_' || ch=='-' || ch=='.')) {
			save_file[n++]=ch;
			save_file[n]='\0';
		}
		else if (ch & 0x80)
			getch();
		else if ((ch == DEL || ch == CTRL_H) && n) 
			save_file[n-1] = '\0';
		else if (ch == CTRL_Q) {
			strcpy(message, "\033[0m取消保存.");
			break;
		}
		else if (ch == RETURN) {
			if (!n) 
				strcpy(message, "\033[0m取消保存.");
			else if (write_progress()) {
				anchor_dirt = dirt;
				sprintf(message, "\033[0m进度文件%s保存成功.", save_file);
			}
			else
				strcpy(message, "\033[31m保存失败.\033[0m");
			break;
		}
	}
	draw_sbar();
}
void _export()
{
	unsigned char ch;
	int n;

	while (1) {
		sprintf(message, "\033[0m输出到文件(CTRL_Q取消):%s", export_file);
		draw_sbar();
		n = strlen(export_file);
		ch = getch();
		if (ch == ESC && getch1() == BRK && getch1()=='3' && getch1()=='~')
			ch = DEL;
		if (n < 15 && (isalnum(ch) || ch=='_' || ch=='-' || ch=='.')) {
			export_file[n++]=ch;
			export_file[n]='\0';
		}
		else if (ch & 0x80)
			getch();
		else if ((ch == DEL || ch == CTRL_H) && n) 
			export_file[n-1] = '\0';
		else if (ch == CTRL_Q) {
			strcpy(message, "\033[0m取消输出.");
			break;
		}
		else if (ch == RETURN) {
			if (!n) 
				strcpy(message, "\033[0m取消输出.");
			else if (write_file())
				sprintf(message, "\033[0m文件%s输出成功.", export_file);
			else
				strcpy(message, "\033[31m输出失败.\033[0m");
			break;
		}
	}
	draw_sbar();
}
void _redo()
{
	int i;
	int r1, r2;
	if (!redo(&r1, &r2))
		strcpy(message, "\033[31m无法进行REDO.\033[0m");
	else {
		if (r1!=-1 && r2!=-1) {
			save_cur();
			for (i=r1; i<=r2; i++) {
				gotoxy(0, i);
				clr2tail();
				prints(get_row_str(0, i, 0));
			}
			load_cur();
		}
		strcpy(message, "\033[0mREDO成功.");
	}
	draw_sbar();
}
void _undo()
{
	int i;
	int r1, r2;
	if (!undo(&r1, &r2))
		strcpy(message, "\033[31m无法进行UNDO.\033[0m");
	else {
		if (r1!=-1 && r2!=-1) {
			save_cur();
			for (i=r1; i<=r2; i++) {
				gotoxy(0, i);
				clr2tail();
				prints(get_row_str(0, i, 0));
			}
			load_cur();
		}
		strcpy(message, "\033[0mUNDO成功.");
	}
	draw_sbar();
}
void _skey()
{
	int n = code_first-'0';
	struct charinfo *key;

	key = &sc_keys[n][0];
	code_first = key->code;
	if ((chinese = key->chinese)!=0)
		code_second = (key+1)->code;
	_data();
}
void _sskey()
{
	int n = code_first-'0';
	struct charinfo *ci,*ci1;

	chkpt_begin(-1, -1, 1, sc_keys, sizeof(sc_keys));
	ci1 = &sc_keys[n][0];
	ci = &graph[cur_y][cur_x];
	*ci1 = *ci;
	if (ci1->chinese) {
		ci1++; ci++;
		*ci1 = *ci;
	}
	sprintf(message, "\033[0m%s被设为第%d号快捷键.", get_char_str(&sc_keys[n][0],0), n);
	chkpt_commit();
	draw_sbar();
}
void _lskey()
{
	int i;
	char s[200];
	char ch[10];

	strcpy(s, "\033[0m快捷:");
	for (i=0; i<NUM_SKEYS; i++) {
		sprintf(ch, "%d", i);
		strcat(s, ch);
		strcat(s, get_char_str(&sc_keys[i][0],0));
		if (!sc_keys[i][0].chinese)
			strcat(s, " ");
	}
	strcpy(message, s);
	draw_sbar();
}
void _refrh()
{
	save_cur();
	clr_scr();
	load_cur();
	draw_graph();
	draw_sbar();
}
void _clra()
{
	int i, j;

	chkpt_begin(0, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	memset(graph, 0, sizeof(graph));
	for (i=0; i<scr_lns-1; i++)
		for (j=0; j<scr_cols; j++)
			graph[i][j].code = SPACE;
	save_cur();
	clr_scr();
	load_cur();
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _clrup()
{
	int i, j;
	int y;

	chkpt_begin(0, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(struct charinfo)*scr_cols*(cur_y+1));
	memset(graph, 0, sizeof(struct charinfo)*scr_cols*(cur_y+1));
	y = cur_y;
	for (i=0; i<=y; i++)
		for (j=0; j<scr_cols; j++)
			graph[i][j].code = SPACE;
	save_cur();
	for (i=0; i<=y; i++) {
		gotoxy(0, i);
		clr2tail();
		prints(get_row_str(0, i, 0));
	}
	load_cur();
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _clrdn()
{
	int i, j;

	chkpt_begin(cur_y, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][0], sizeof(struct charinfo)*scr_cols*(scr_lns-cur_y-1));
	memset(&graph[cur_y][0], 0, sizeof(struct charinfo)*scr_cols*(scr_lns-cur_y-1));
	for (i=cur_y; i<scr_lns-1; i++)
		for (j=0; j<scr_cols; j++) 
			graph[i][j].code = SPACE;
	save_cur();
	for (i=cur_y; i<scr_lns-1; i++) {
		gotoxy(0, i);
		clr2tail();
		prints(get_row_str(0, i, 0));
	}
	load_cur();
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _clrlf()
{
	int i,j;

	chkpt_begin(0, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	clr_brokenchars(graph, 0, scr_lns-2, cur_x, 0);
	for (i=0; i<scr_lns-1; i++) {
		memset(&graph[i][0], 0, sizeof(struct charinfo)*(cur_x+1));
		for (j=0; j<=cur_x; j++)
			graph[i][j].code = SPACE;
	}
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _clrrt()
{
	int i,j;

	chkpt_begin(0, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	clr_brokenchars(graph, 0, scr_lns-2, cur_x, 1);
	for (i=0; i<scr_lns-1; i++) {
		memset(&graph[i][cur_x], 0, sizeof(struct charinfo)*(scr_cols-cur_x));
		for (j=cur_x; j<scr_cols; j++)
			graph[i][j].code = SPACE;
	}
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _gfc()
{
	if (graph[cur_y][cur_x].attr.fcolor == cur_attr.fcolor)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	global_replace(A_FCOLOR);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _gfce()
{
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	global_replace(A_FCOLOREXT);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _gbc()
{
	if (graph[cur_y][cur_x].attr.bcolor == cur_attr.bcolor)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	global_replace(A_BCOLOR);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _gbce()
{
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	global_replace(A_BCOLOREXT);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _gbl()
{
	if (graph[cur_y][cur_x].attr.blink== cur_attr.blink)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	global_replace(A_BLINK);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _gbr()
{
	if (graph[cur_y][cur_x].attr.bright== cur_attr.bright)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	global_replace(A_BRIGHT);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _gul()
{
	if (graph[cur_y][cur_x].attr.underline== cur_attr.underline)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	global_replace(A_UNDERLINE);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _gattr()
{
	if (!memcmp(&graph[cur_y][cur_x].attr, &cur_attr, sizeof(struct ansiattr)))
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	global_replace(A_ATTRIB);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _gchr()
{
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	global_replace(A_CHAR);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _gall()
{
	if (!memcmp(&graph[cur_y][cur_x].attr, &cur_attr, sizeof(struct ansiattr)))
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	global_replace(A_ALL);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a4fc()
{
	if (graph[cur_y][cur_x].attr.fcolor == cur_attr.fcolor)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_4, A_FCOLOR);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a4fce()
{
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_4, A_FCOLOREXT);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a4bc()
{
	if (graph[cur_y][cur_x].attr.bcolor == cur_attr.bcolor)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_4, A_BCOLOR);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a4bce()
{
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_4, A_BCOLOREXT);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a4bl()
{
	if (graph[cur_y][cur_x].attr.blink == cur_attr.blink)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_4, A_BLINK);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a4br()
{
	if (graph[cur_y][cur_x].attr.bright== cur_attr.bright)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_4, A_BRIGHT);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a4ul()
{
	if (graph[cur_y][cur_x].attr.underline == cur_attr.underline)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_4, A_UNDERLINE);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a4attr()
{
	if (!memcmp(&graph[cur_y][cur_x].attr, &cur_attr, sizeof(struct ansiattr)))
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_4, A_ATTRIB);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a8fc()
{
	if (graph[cur_y][cur_x].attr.fcolor == cur_attr.fcolor)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_8, A_FCOLOR);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a8fce()
{
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_8, A_FCOLOREXT);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a8bc()
{
	if (graph[cur_y][cur_x].attr.bcolor == cur_attr.bcolor)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_8, A_BCOLOR);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a8bce()
{
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_8, A_BCOLOREXT);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a8bl()
{
	if (graph[cur_y][cur_x].attr.blink == cur_attr.blink)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_8, A_BLINK);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a8br()
{
	if (graph[cur_y][cur_x].attr.bright== cur_attr.bright)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_8, A_BRIGHT);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a8ul()
{
	if (graph[cur_y][cur_x].attr.underline == cur_attr.underline)
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_8, A_UNDERLINE);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _a8attr()
{
	if (!memcmp(&graph[cur_y][cur_x].attr, &cur_attr, sizeof(struct ansiattr)))
		return;
	chkpt_begin(blk_y1, blk_y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	adjacent_replace(ADJACENT_8, A_ATTRIB);
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _copy()
{
	struct charinfo *ci;

	copy_type = CP_CHAR;
	ci = &graph[cur_y][cur_x];
	assert(!ci->chinese || (ci->chinese && !ci->half));
	char_copy[0] = *ci;
	if (ci->chinese)
		char_copy[1] = *(ci+1);
	sprintf(message, "\033[0m字符%s被复制.", get_char_str(ci,1));
	draw_sbar();
}
void _pst()
{
	struct charinfo *ci;
	int i, j;
	int x1, y1, x2, y2;

	switch (copy_type) {
		case CP_CHAR:
			chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][cur_x], sizeof(struct charinfo)*3);
			ci = &char_copy[0];
			code_first = ci->code;
			if ((chinese = ci->chinese)!=0)
				code_second = (ci+1)->code;
			set_cur_char2(&ci->attr);
			save_cur();
			clr2tail();
			prints(get_row_str(cur_x, cur_y, 0));
			load_cur();
			sprintf(message, "\033[0m字符%s被粘贴.", get_char_str(ci,1));
			dirt++;
			chkpt_commit();
			break;
		case CP_BLOCK:
			x1 = cur_x; y1 = cur_y; x2 = x1+(blk_cp_x2-blk_cp_x1); y2 = y1+(blk_cp_y2-blk_cp_y1);
			x2 = x2 > scr_cols-1? scr_cols-1: x2;
			y2 = y2 > scr_lns-2? scr_lns-2: y2;
			assert(x1<=x2 && y1<=y2);
			chkpt_begin(y1, y2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[y1][0], sizeof(struct charinfo)*scr_cols*(y2-y1+1));
			clr_brokenchars(graph, y1, y2, x1, 1);
			clr_brokenchars(graph, y1, y2, x2, 0);
			for (i=y1; i<=y2; i++) {
				for (j=x1; j<=x2; j++)
					graph[i][j] = blk_copy[blk_cp_y1+(i-y1)][blk_cp_x1+(j-x1)];
				if (x2 == scr_cols-1) {
					ci = &graph[i][scr_cols-1];
					if (ci->chinese && !ci->half)
						clrchr(ci, NULL);
				}
			}
			save_cur();
			for (i=y1; i<=y2; i++) {
				gotoxy(0, i);
				clr2tail();
				prints(get_row_str(0, i, 0));
			}
			load_cur();
			sprintf(message, "\033[0m块被粘贴.");
			dirt++;
			chkpt_commit();
			break;
	}
	draw_sbar();
}
void _del()
{
	int i;
	struct charinfo *ci, *ci1;
	static struct charinfo s_row[NUM_COLS]; 
	static struct charinfo s_col[NUM_ROWS-2][2];
	int off;

	ci = &graph[cur_y][cur_x];

	if (linemode == HINS) {
		if (cur_x>0) {
			chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][0], sizeof(struct charinfo)*scr_cols);
			memcpy(s_row, graph[cur_y], sizeof(struct charinfo)*scr_cols);
			off = 1+(ci-1)->chinese;
			for (i = cur_x; i < scr_cols; i++)
				graph[cur_y][i-off] = s_row[i];
			for (i = scr_cols-off; i<scr_cols; i++)
				clrchr(&graph[cur_y][i], NULL);
			gotoxy(cur_x-off, cur_y);
			clr2tail();
			prints(get_row_str(cur_x, cur_y, 0));
			dirt++;
			chkpt_commit();
			draw_sbar();
		}
	}
	else if (linemode == HOVW) {
		struct charinfo *p = (cur_x<2 && !cur_y ? (struct charinfo*)graph: &graph[cur_y][cur_x]-2);
		chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), p, sizeof(struct charinfo)*2);
		if (cur_x>0 && (ci-1)->chinese)
			gotoxy(cur_x-2, cur_y);
		else
			gotoxy(cur_x-1, cur_y);
		clrchr2(&graph[cur_y][cur_x], NULL);
		clr2tail();
		prints(get_row_str(cur_x, cur_y, 0));
		dirt++;
		chkpt_commit();
		draw_sbar();
	}
	else if (linemode == VINS) {
		if (cur_y>0) {
			chkpt_begin(cur_y-1, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y-1][0], sizeof(struct charinfo)*scr_cols*(scr_lns-cur_y));
			clr_brokenchars(graph, cur_y-1, scr_lns-2, cur_x, 1);
			for (i = cur_y; i<scr_lns-1; i++)
				memcpy(&s_col[i][0], &graph[i][cur_x], sizeof(struct charinfo)*2);
			clr_brokenchars(graph, cur_y-1, scr_lns-2, cur_x, 0);
			for (i = cur_y-1; i<scr_lns-2; i++) {
				ci = &graph[i][cur_x];
				ci1 = &s_col[i+1][0];
				*ci = *ci1;
				if (ci->chinese) {
					ci++; ci1++;
					*ci = *ci1;
					ci++;
					if (ci->chinese && ci->half) 
						clrchr(ci, &ci->attr);
				}
			}
			clrchr(&graph[scr_lns-2][cur_x], NULL);
			save_cur();
			for (i = cur_y-1; i<scr_lns-1; i++) {
				gotoxy(0, i);
				clr2tail();
				prints(get_row_str(0, i, 0));
			}
			load_cur();
			gotoxy(cur_x, cur_y-1);
			dirt++;
			chkpt_commit();
			draw_sbar();
		}
	}
	else if (linemode == VOVW) {
		struct charinfo *p;
		int y = !cur_y? cur_y: cur_y-1;
		if (!cur_y) {
			if (cur_x<1)
				p = (struct charinfo*)graph;
			else
				p = &graph[cur_y][cur_x-1];
		}
		else
			p = &graph[cur_y-1][cur_x-1];
		chkpt_begin(y, y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), p, sizeof(struct charinfo)*3);
		clr_brokenchars(graph, y, y, cur_x, 1);
		gotoxy(cur_x, cur_y-1);
		clrchr2(&graph[cur_y][cur_x], NULL);
		save_cur();
		gotoxy(0, cur_y);
		clr2tail();
		prints(get_row_str(0, cur_y, 0));
		load_cur();
		dirt++;
		chkpt_commit();
		draw_sbar();
	}

}
void _rcopy()
{
	memcpy(row_copy, &graph[cur_y][0], sizeof(struct charinfo)*scr_cols);
	sprintf(message, "\033[0m本行文本被复制.");
	draw_sbar();
}
void _rpst()
{
	chkpt_begin(cur_y, cur_y, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), &graph[cur_y][0], sizeof(struct charinfo)*scr_cols);
	memcpy(&graph[cur_y][0], row_copy, sizeof(struct charinfo)*scr_cols);
	save_cur();
	gotoxy(0, cur_y);
	clr2tail();
	prints(get_row_str(0, cur_y, 0));
	load_cur();
	sprintf(message, "\033[0m复制文本已粘贴至本行.");
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _ccopy()
{
	int i;
	struct charinfo *ci, *ci1;

	memset(col_copy, 0, sizeof(col_copy));
	for (i=0; i<scr_lns-1; i++) {
		ci = &graph[i][cur_x];
		ci1 = &col_copy[i][0];
		*ci1 = *ci;
		if (ci->chinese && ci->half)
			clrchr(ci1, &ci1->attr);
		else {
			if (ci1->chinese) {
				ci1++; ci++;
				*ci1 = *ci;
			}
		}
	}
	sprintf(message, "\033[0m本列文本被复制.");
	draw_sbar();
}
void _cpst()
{
	int i;
	struct charinfo *ci, *ci1;

	chkpt_begin(0, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	clr_brokenchars(graph, 0, scr_lns-2, cur_x, 1);
	clr_brokenchars(graph, 0, scr_lns-2, cur_x, 0);
	for (i=0; i<scr_lns-1; i++) {
		ci = &graph[i][cur_x];
		ci1 = &col_copy[i][0];
		*ci = *ci1;
		if (ci->chinese) {
			if (cur_x == scr_cols-1)
				clrchr(ci, &ci->attr);
			else {
				ci++; ci1++;
				*ci = *ci1;
				ci++;
				if (ci->chinese && ci->half)
					clrchr(ci, &ci->attr);
			}
		}
	}
	dirt++;
	chkpt_commit();
	draw_graph();
	draw_sbar();
}
void _draw()
{
	chkpt_begin(-1, -1, 1, &drawing, sizeof(char));
	drawing = (drawing+1)%NUM_DRAW_METHODS;
	draw_sbar();
	chkpt_commit();
}
void _mail()
{
	char *str1 = "请输入邮件地址:";
	save_cur_x = cur_x; save_cur_y = cur_y;
	
	clr_scr();
	gotoxy2(0,0);
	prints("\033[0m");
	prints(str1); 
	gotoxy2(cur_x+strlen(str1), cur_y);
	if (strlen(mail_addr)) {
		prints(mail_addr);
		gotoxy2(cur_x+strlen(mail_addr),cur_y);
	}
	mail_step = ML_ADDR;
	mail_sent = 0;
	strcpy(message, "\033[0m取消寄信用CTRL_Q.");
	draw_sbar();
}
void _ret()
{
	if (cur_y < scr_lns-2) {
		chkpt_begin(-1, -1, 2, &cur_x, sizeof(char), &cur_y, sizeof(char)); 
		gotoxy(0, cur_y+1);
		draw_sbar();
		chkpt_commit();
	}
}
void _igbk()
{
	unsigned char ch;
	char *str1 = "\033[0mGBK码输入(回车取消):";
	char gbk[5]={0};
	int n=0;

	strcpy(message, str1);
	draw_sbar();
	while (1) {
		ch = getch();
		if ((ch>='0' && ch<='9') || (ch>='a' && ch<='f') || (ch>='A' && ch<='F')) {
			if ((!n && ch>='8') || n) {
				gbk[n++] = tolower(ch);
				sprintf(message, "%s%s", str1, gbk);
				draw_sbar();
				if (n==4) {
					sprintf(message, "\033[0mGBK码%s输入完成.", gbk);
					str2hex(gbk, &code_first, &code_second);
					chinese = 1;
					_data();
					return;
				}
				continue;
			}
		}
		if (ch == RETURN || ch == CTRL_Q) {
			strcpy(message, "\033[0m取消GBK码输入");
			draw_sbar();
			return;
		}
	}
}
void _blk1()
{
	save_cur_x = cur_x; save_cur_y = cur_y;
	memcpy(save_graph, graph, sizeof(graph));
	blk_x1 = blk_x2 = cur_x;
	blk_y1 = blk_y2 = cur_y;
	strcpy(message, "\033[0m光标键设定块,CTRL_K确定,CTRL_Q退出.");
	set_blk_attr(A_REVERSE, 1);
	draw_sbar();
}
void _blk2()
{
	char swap;

	if (blk_x1 > blk_x2) {
		swap = blk_x1; blk_x1 = blk_x2; blk_x2 = swap;
	}
	if (blk_y1 > blk_y2) {
		swap = blk_y1; blk_y1 = blk_y2; blk_y2 = swap;
	}
	save_attr = cur_attr;
	undo_limit = undo_ptr;
	blk_undo_ptr = undo_ptr;
	strcpy(message, "\033[0m请设置块的属性,CTRL_Q结束,<ESC>h帮助.");
	clr_reverse();
	draw_graph();
	draw_sbar();
}
void _quit()
{
	if (!confirm_quit())
		return;
	undo_free(undo_list.next);
	undo_list.next = NULL;
	tty_reset(0);
	unload_cmdhelp();
	exit(0);
}
void _adata()
{
	static char n;
	char ch;
	switch (apst_step) {
		case AP_DATA:
			switch (apst_mode) {
			case AP_STARBRK:
				if (!chinese && code_first == '*') {
					if ((ch = getch1())==BRK) {
						n = 0;
						apst_step = AP_CMD;
						ungetch(ch);
						return;
					}
					ungetch(ch);
				}
				/* don't break, also accept ESC[ */
			case AP_ESCBRK:
				if (!chinese && code_first == ESC) {
					n = 0;
					apst_step = AP_CMD;
					return;
				}
				break;
            default:
                break;
			}
			set_cur_char2(&cur_attr);
			clr2tail();
			prints(get_row_str(cur_x, cur_y, 0));
			if (cur_x+1+chinese <= scr_cols-1) {
				gotoxy(cur_x+1+chinese, cur_y);
				draw_sbar();
			}
			break;
		case AP_CMD:
			if (!chinese && code_first == ESC)
				apst_step = AP_CMD;
			else if (!chinese && code_first == BRK)
				apst_step = AP_LB;
			else {
				if (!chinese && code_first == 'O')
					getch1();
				apst_step = AP_DATA;
			}
			break;
		case AP_LB:
			if (!chinese && isdigit(code_first)) {
				n = code_first - '0';
				apst_step = AP_ATTR;
			}
			else if (!chinese && code_first == 'm') {
				set_cur_attr(n);
				apst_step = AP_DATA;
			}
			else 
				apst_step = AP_DATA;
			break;
		case AP_ATTR:
			if (!chinese && isdigit(code_first)) {
				n = n*10 + (code_first - '0');
				apst_step = AP_ATTR;
			}
			else if (!chinese && code_first == SEMICOLON) {
				set_cur_attr(n);
				apst_step = AP_SEP;
			}
			else if (!chinese && code_first == 'm') {
				set_cur_attr(n);
				apst_step = AP_DATA;
			}
			else
				apst_step = AP_DATA;
			break;
		case AP_SEP:
			if (!chinese && isdigit(code_first)) {
				n = code_first - '0';
				apst_step = AP_ATTR;
			}
			else 
				apst_step = AP_DATA;
			break;
        default:
            break;
	}
}
void _apst()
{
	save_attr = cur_attr;
	save_cur_x = cur_x; save_cur_y = cur_y;

	memset(&cur_attr, 0, sizeof(cur_attr));
	chkpt_begin(0, scr_lns-2, 3, &cur_x, sizeof(char), &cur_y, sizeof(char), graph, sizeof(graph));
	apst_step = AP_DATA;
	strcpy(message, "\033[0m粘贴带色文本,CTRL_O改变命令格式,CTRL_Q结束.");
	draw_sbar();
}
void _amode()
{
	apst_mode = (apst_mode+1)%AP_MODES;
	draw_sbar();
}
void _aret()
{
	struct charinfo* ci;
	if (cur_y < scr_lns-2) {
		ci = &graph[cur_y+1][save_cur_x];
		clr_brokenchars(graph, cur_y+1, cur_y+1, save_cur_x, 1);
		gotoxy(save_cur_x, cur_y+1);
		save_cur();
		gotoxy(0, cur_y);
		clr2tail();
		prints(get_row_str(0, cur_y, 0));
		load_cur();
		draw_sbar();
	}
}
void _aquit()
{
	cur_attr = save_attr;
	strcpy(message, "\033[0m带色粘贴结束.");
	if (cur_x != save_cur_x || cur_y != save_cur_y) {
		dirt++;
		chkpt_commit();
	}
	draw_sbar();
}
void _minfo()
{
	int len;

	switch (mail_step) {
		case ML_ADDR:
			if ((len=strlen(mail_addr))<MAIL_ADDR_LEN-1 && !chinese && code_first != SPACE) {
				len++;
				mail_addr[len-1] = code_first;
				mail_addr[len] = '\0';
				prints("%c", code_first);
				gotoxy2(cur_x+1, cur_y);
				draw_sbar();
			}
			break;
		case ML_TITLE:
			if ((len=strlen(mail_title))<MAIL_TITLE_LEN-2) {
				len++;
				mail_title[len-1] = code_first;
				prints("%c", code_first);
				if (chinese) {
					len++;
					mail_title[len-1] = code_second;
					prints("%c", code_second);
				}
				mail_title[len] = '\0';
				gotoxy2(cur_x+1+chinese, cur_y);
				draw_sbar();
			}
			break;
	}
}
void _mdel()
{
	int len;

	switch (mail_step) {
		case ML_ADDR:
			if ((len=strlen(mail_addr))!=0) {
				mail_addr[--len]='\0';
				gotoxy2(cur_x-1, cur_y);
				clr2tail();
				draw_sbar();
			}
			break;
		case ML_TITLE:
			if ((len=strlen(mail_title))!=0) {
				mail_title[--len]='\0';
				gotoxy2(cur_x-1, cur_y);
				clr2tail();
				draw_sbar();
			}
			break;
	}
}
void _mquit()
{
	if (!mail_sent)
		strcpy(message, "\033[0m取消寄信.");
	else if (mail_sent<0)
		strcpy(message, "\033[31m发信失败.\033[0m");
	else
		strcpy(message, "\033[0m信已寄出.");
	draw_graph();
	draw_sbar();
	gotoxy(save_cur_x, save_cur_y);
}
void _mconf()
{
	char *str1 = "请输入邮件标题:";
    struct userec *u = NULL;
	switch (mail_step) {
		case ML_ADDR: 
			 if (!strchr(mail_addr, '@')) {
			    if (HAS_PERM(getCurrentUser(),PERM_DENYMAIL)) {
					strcpy(message, "\033[31m你无权给本站用户寄信.\033[0m");
					draw_sbar();
					break;
				}
		         if (getuser(mail_addr, &u) == 0) {
					strcpy(message, "\033[31m错误的使用者帐号.\033[0m");
					draw_sbar();
					break;
				 }
			}
			gotoxy2(0,1);
			prints("\033[0m");
			prints(str1); 
			gotoxy2(cur_x+strlen(str1), cur_y);
			if (strlen(mail_title)) {
				prints(mail_title);
				gotoxy2(cur_x+strlen(mail_title),cur_y);
			}
			mail_step = ML_TITLE;
			draw_sbar();
			break;
		case ML_TITLE:
			send_mail();
			state = S_EDIT;
			_mquit();
			break;
	}
}
void _h2np()
{
	gbk_page = (gbk_page+1)%gbk_total_pages;
	help_code(gbk_page);
}
void _h2pp()
{
	gbk_page = (gbk_page+gbk_total_pages-1)%gbk_total_pages;
	help_code(gbk_page);
}
void _h2quit()
{
	strcpy(message, "\033[0mGBK码检索结束.");
	gotoxy(save_cur_x, save_cur_y);
	draw_graph();
	draw_sbar();
}
void _h1np()
{
	hcmd_page = (hcmd_page+1)%hcmd_total_pages;
	help_cmd(hcmd_page);
}
void _h1pp()
{
	hcmd_page = (hcmd_page+hcmd_total_pages-1)%hcmd_total_pages;
	help_cmd(hcmd_page);
}
void _h1quit()
{
	strcpy(message, "\033[0m命令帮助结束.");
	gotoxy(save_cur_x, save_cur_y);
	state = prev_hcmd_state;
	draw_graph();
	draw_sbar();
}
void _b1curup()
{
	if (cur_y > 0) {
		clr_reverse();
		if (cur_y > blk_y1) {
			save_cur();
			gotoxy(0, cur_y);
			clr2tail();
			prints(get_row_str(0, cur_y, 0));
			load_cur();
		}
		gotoxy(cur_x, cur_y-1);
		blk_x2 = cur_x; blk_y2 = cur_y;
		set_blk_attr(A_REVERSE, 1);
		draw_sbar();
	}
}
void _b1curdn()
{
	if (cur_y < scr_lns-2) {
		clr_reverse();
		if (cur_y < blk_y1) {
			save_cur();
			gotoxy(0, cur_y);
			clr2tail();
			prints(get_row_str(0, cur_y, 0));
			load_cur();
		}
		gotoxy(cur_x, cur_y+1);
		blk_x2 = cur_x; blk_y2 = cur_y;
		set_blk_attr(A_REVERSE, 1);
		draw_sbar();
	}
}
void _b1currt()
{
	struct charinfo *ci;
	int x;

	ci = &graph[cur_y][cur_x];
	x = cur_x+1+ci->chinese;
	if (x < scr_cols) {
		clr_reverse();
		gotoxy(x, cur_y);
		blk_x2 = cur_x; blk_y2 = cur_y;
		set_blk_attr(A_REVERSE, 1);
		draw_sbar();
	}
}
void _b1curlf()
{
	struct charinfo *ci;
	int x;

	ci = &graph[cur_y][cur_x];
	x = cur_x-1-ci->chinese;
	if (x >=0) {
		clr_reverse();
		gotoxy(x, cur_y);
		blk_x2 = cur_x; blk_y2 = cur_y;
		set_blk_attr(A_REVERSE, 1);
		draw_sbar();
	}
}
void _b2curup()
{
	if (cur_y == blk_y1) return;
	if (cur_x < blk_x1)
		gotoxy(cur_x+1, cur_y-1);
	else
		gotoxy(cur_x, cur_y-1);
	draw_sbar();
}
void _b2curdn()
{
	if (cur_y == blk_y2) return;
	if (cur_x < blk_x1)
		gotoxy(cur_x+1, cur_y+1);
	else
		gotoxy(cur_x, cur_y+1);
	draw_sbar();
}
void _b2currt()
{
	struct charinfo *ci;

	ci = &graph[cur_y][cur_x];
	if (cur_x+1+ci->chinese > blk_x2)
		return;
	gotoxy(cur_x+1+ci->chinese, cur_y);
	draw_sbar();
}
void _b2curlf()
{
	if (cur_x <= blk_x1) return;
	gotoxy(cur_x-1, cur_y);
	draw_sbar();
}
void _bdel()
{
	chkpt_begin(blk_y1, blk_y2, 1, &graph[blk_y1][0], sizeof(struct charinfo)*scr_cols*(blk_y2-blk_y1+1));
	clr_blk();
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _bcopy()
{
	struct charinfo *ci, *ci1;

	copy_type = CP_BLOCK;
	blk_cp_x1 = blk_x1; blk_cp_y1 = blk_y1; blk_cp_x2 = blk_x2; blk_cp_y2 = blk_y2;
	ci = &graph[blk_cp_y1][blk_cp_x2];
	ci1 = &graph[blk_cp_y2][blk_cp_x2];
	if ((ci->chinese && !ci->half) || (ci1->chinese && !ci1->half))
		blk_cp_x2++;
	memcpy(blk_copy, graph, sizeof(graph));
	clr_brokenchars(blk_copy, blk_cp_y1, blk_cp_y2, blk_cp_x1, 1);
	clr_brokenchars(blk_copy, blk_cp_y1, blk_cp_y2, blk_cp_x2, 0);
	sprintf(message, "\033[0m块被复制.");
	draw_sbar();
}
void _baplfc()
{
	chkpt_begin(blk_y1, blk_y2, 1, &graph[blk_y1][0], sizeof(struct charinfo)*scr_cols*(blk_y2-blk_y1+1));
	set_blk_attr(A_FCOLOR, cur_attr.fcolor);
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _baplbc()
{
	chkpt_begin(blk_y1, blk_y2, 1, &graph[blk_y1][0], sizeof(struct charinfo)*scr_cols*(blk_y2-blk_y1+1));
	set_blk_attr(A_BCOLOR, cur_attr.bcolor);
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _baplbl()
{
	chkpt_begin(blk_y1, blk_y2, 1, &graph[blk_y1][0], sizeof(struct charinfo)*scr_cols*(blk_y2-blk_y1+1));
	set_blk_attr(A_BLINK, cur_attr.blink);
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _baplbr()
{
	chkpt_begin(blk_y1, blk_y2, 1, &graph[blk_y1][0], sizeof(struct charinfo)*scr_cols*(blk_y2-blk_y1+1));
	set_blk_attr(A_BRIGHT, cur_attr.bright);
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _baplul()
{
	chkpt_begin(blk_y1, blk_y2, 1, &graph[blk_y1][0], sizeof(struct charinfo)*scr_cols*(blk_y2-blk_y1+1));
	set_blk_attr(A_UNDERLINE, cur_attr.underline);
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _baplattr()
{
	chkpt_begin(blk_y1, blk_y2, 1, &graph[blk_y1][0], sizeof(struct charinfo)*scr_cols*(blk_y2-blk_y1+1));
	set_blk_attr(A_FCOLOR, cur_attr.fcolor);
	set_blk_attr(A_BCOLOR, cur_attr.bcolor);
	set_blk_attr(A_BLINK, cur_attr.blink);
	set_blk_attr(A_BRIGHT, cur_attr.bright);
	set_blk_attr(A_UNDERLINE, cur_attr.underline);
	dirt++;
	chkpt_commit();
	draw_sbar();
}
void _b1quit()
{
	blk_x1 = 0; blk_y1 = 0;
	blk_x2 = scr_cols-1; blk_y2 = scr_lns-2;
	strcpy(message, "\033[0m块操作已取消.");
	memcpy(graph, save_graph, sizeof(graph));
	gotoxy(save_cur_x, save_cur_y);
	draw_graph();
	draw_sbar();
}
void _b2quit()
{
	cur_attr = save_attr;
	blk_x1 = 0; blk_y1 = 0;
	blk_x2 = scr_cols-1; blk_y2 = scr_lns-2;
	undo_limit = &undo_list;
	undo_free(blk_undo_ptr->next);
	blk_undo_ptr->next = NULL;
	undo_ptr = blk_undo_ptr;
	dirt = undo_ptr->dirt;
	if (memcmp(graph, save_graph, sizeof(graph))) { /* if the graph was modified, record it into undo list */
		memcpy(blk_swap_graph, graph, sizeof(graph));
		memcpy(graph, save_graph, sizeof(graph));
		chkpt_begin(blk_y1, blk_y2, 1, &graph[blk_y1][0], sizeof(struct charinfo)*scr_cols*(blk_y2-blk_y1+1));
		memcpy(graph, blk_swap_graph, sizeof(graph));
		dirt++;
		chkpt_commit();
	}
	strcpy(message, "\033[0m块操作结束.");
	gotoxy(save_cur_x, save_cur_y);
	draw_graph();
	draw_sbar();
}
/* finite state machine */
struct fsm_entry fsm[NUM_COMMANDS][NUM_STATES] = { /* define finite state machine */
/* S_EDIT          S_SELBLK         S_EDITBLK          S_APST           S_HLPCMD          S_HLPCODE          S_MAIL*/
{{S_EDIT, _data}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST,_adata}, {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,_minfo}}, /*CMD_DATA*/
{{S_EDIT, _curup},{S_SELBLK,_b1curup},{S_EDITBLK,_b2curup},{S_APST,NULL},{S_HLPCMD,NULL}, {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_UP*/
{{S_EDIT, _curdn},{S_SELBLK,_b1curdn},{S_EDITBLK,_b2curdn},{S_APST,NULL},{S_HLPCMD,NULL}, {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_DOWN*/
{{S_EDIT, _currt},{S_SELBLK,_b1currt},{S_EDITBLK,_b2currt},{S_APST,NULL},{S_HLPCMD,_h1np},{S_HLPCODE,_h2np}, {S_MAIL,NULL}}, /*CMD_RIGHT*/
{{S_EDIT, _curlf},{S_SELBLK,_b1curlf},{S_EDITBLK,_b2curlf},{S_APST,NULL},{S_HLPCMD,_h1pp},{S_HLPCODE,_h2pp}, {S_MAIL,NULL}}, /*CMD_LEFT*/
{{S_EDIT, _home}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_HOME*/
{{S_EDIT, _endl}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_END*/
{{S_EDIT, _pgup}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_PGUP*/
{{S_EDIT, _pgdn}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_PGDN*/
{{S_EDIT, _absob},{S_SELBLK,NULL},  {S_EDITBLK,_absob},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ABSORB*/
{{S_EDIT, _reset},{S_SELBLK,NULL},  {S_EDITBLK,_reset},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_RESET*/
{{S_EDIT, _setfc},{S_SELBLK,NULL},  {S_EDITBLK,_setfc},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_FCOLOR*/
{{S_EDIT, _setbc},{S_SELBLK,NULL},  {S_EDITBLK,_setbc},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_BCOLOR*/
{{S_EDIT, _setbl},{S_SELBLK,NULL},  {S_EDITBLK,_setbl},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_BLINK*/
{{S_EDIT, _setbr},{S_SELBLK,NULL},  {S_EDITBLK,_setbr},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_BRIGHT*/
{{S_EDIT, _setul},{S_SELBLK,NULL},  {S_EDITBLK,_setul},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_UNDERLINE*/
{{S_EDIT, _aplfc},{S_SELBLK,NULL}, {S_EDITBLK,_baplfc},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_APPLYFCOLOR*/
{{S_EDIT, _aplbc},{S_SELBLK,NULL}, {S_EDITBLK,_baplbc},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_APPLYBCOLOR*/
{{S_EDIT, _aplbl},{S_SELBLK,NULL}, {S_EDITBLK,_baplbl},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_APPLYBLINK*/
{{S_EDIT, _aplbr},{S_SELBLK,NULL}, {S_EDITBLK,_baplbr},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_APPLYBRIGHT*/
{{S_EDIT, _aplul},{S_SELBLK,NULL}, {S_EDITBLK,_baplul},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_APPLYUNDERLINE*/
{{S_EDIT,_aplattr},{S_SELBLK,NULL}, {S_EDITBLK,_baplattr},{S_APST, NULL},{S_HLPCMD,NULL}, {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_APPLYATTRIB*/
{{S_EDIT, _setlm},{S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_LINEMODE*/
{{S_EDIT, _setp0},{S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ERASE*/
{{S_EDIT, _apen}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ADDPEN*/
{{S_EDIT, _dpen}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_DELPEN*/
{{S_EDIT, _lpen}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_LISTPEN*/
{{S_EDIT, _npen}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_NEXTPEN*/
{{S_EDIT, _ppen}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_PREVPEN*/
{{S_EDIT, _irow}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_INSROW*/
{{S_EDIT, _drow}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_DELROW*/
{{S_EDIT, _erow}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ERASEROW*/
{{S_EDIT, _icol}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_INSCOL*/
{{S_EDIT, _dcol}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_DELCOL*/
{{S_EDIT, _ecol}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ERASECOL*/
{{S_EDIT, _skey}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_SKEY*/
{{S_EDIT, _sskey},{S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_SETSKEY*/
{{S_EDIT, _lskey},{S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_LISTSKEY*/
{{S_EDIT, _refrh},{S_SELBLK,_refrh},{S_EDITBLK,_refrh},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_REFRESH*/
{{S_EDIT, _clra}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ERASEALL*/
{{S_EDIT, _clrup},{S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ERASEUP*/
{{S_EDIT, _clrdn},{S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ERASEDOWN*/
{{S_EDIT, _clrlf},{S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ERASELEFT*/
{{S_EDIT, _clrrt},{S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ERASERIGHT*/
{{S_EDIT, _gfc},  {S_SELBLK,NULL},  {S_EDITBLK,_gfc},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_G_FCOLOR*/
{{S_EDIT, _gfce}, {S_SELBLK,NULL},  {S_EDITBLK,_gfce}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_G_FCOLOREXT*/
{{S_EDIT, _gbc},  {S_SELBLK,NULL},  {S_EDITBLK,_gbc},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_G_BCOLOR*/
{{S_EDIT, _gbce}, {S_SELBLK,NULL},  {S_EDITBLK,_gbce}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_G_BCOLOREXT*/
{{S_EDIT, _gbl},  {S_SELBLK,NULL},  {S_EDITBLK,_gbl},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_G_BLINK*/
{{S_EDIT, _gbr},  {S_SELBLK,NULL},  {S_EDITBLK,_gbr},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_G_BRIGHT*/
{{S_EDIT, _gul},  {S_SELBLK,NULL},  {S_EDITBLK,_gul},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_G_UNDERLINE*/
{{S_EDIT, _gattr},{S_SELBLK,NULL},  {S_EDITBLK,_gattr},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_G_ATTRIB*/
{{S_EDIT, _gchr}, {S_SELBLK,NULL},  {S_EDITBLK,_gchr}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_G_CHAR*/
{{S_EDIT, _gall}, {S_SELBLK,NULL},  {S_EDITBLK,_gall}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_G_ALL*/
{{S_EDIT, _a4fc}, {S_SELBLK,NULL},  {S_EDITBLK,_a4fc}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A4_FCOLOR*/
{{S_EDIT, _a4fce},{S_SELBLK,NULL},  {S_EDITBLK,_a4fce},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A4_FCOLOREXT*/
{{S_EDIT, _a4bc}, {S_SELBLK,NULL},  {S_EDITBLK,_a4bc}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A4_BCOLOR*/
{{S_EDIT, _a4bce},{S_SELBLK,NULL},  {S_EDITBLK,_a4bce},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A4_BCOLOREXT*/
{{S_EDIT, _a4bl}, {S_SELBLK,NULL},  {S_EDITBLK,_a4bl}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A4_BLINk*/
{{S_EDIT, _a4br}, {S_SELBLK,NULL},  {S_EDITBLK,_a4br}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A4_BRIGHT*/
{{S_EDIT, _a4ul}, {S_SELBLK,NULL},  {S_EDITBLK,_a4ul}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A4_UNDERLINE*/
{{S_EDIT,_a4attr},{S_SELBLK,NULL},  {S_EDITBLK,_a4attr},{S_APST, NULL}, {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A4_ATTRIB*/
{{S_EDIT, _a8fc}, {S_SELBLK,NULL},  {S_EDITBLK,_a8fc}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A8_FCOLOR*/
{{S_EDIT, _a8fce},{S_SELBLK,NULL},  {S_EDITBLK,_a8fce},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A8_FCOLOREXT*/
{{S_EDIT, _a8bc}, {S_SELBLK,NULL},  {S_EDITBLK,_a8bc}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A8_BCOLOR*/
{{S_EDIT, _a8bce},{S_SELBLK,NULL},  {S_EDITBLK,_a8bce},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A8_BCOLOREXT*/
{{S_EDIT, _a8bl}, {S_SELBLK,NULL},  {S_EDITBLK,_a8bl}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A8_BLINk*/
{{S_EDIT, _a8br}, {S_SELBLK,NULL},  {S_EDITBLK,_a8br}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A8_BRIGHT*/
{{S_EDIT, _a8ul}, {S_SELBLK,NULL},  {S_EDITBLK,_a8ul}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A8_UNDERLINE*/
{{S_EDIT,_a8attr},{S_SELBLK,NULL},  {S_EDITBLK,_a8attr},{S_APST, NULL}, {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_A8_ATTRIB*/
{{S_HLPCMD,_hcmd},{S_SELBLK,NULL},  {S_HLPCMD,_hcmd},  {S_APST, _hcmd}, {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_HELPCMD*/
{{S_HLPCODE,_hcode},{S_SELBLK,NULL},{S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_HELPCODE*/
{{S_EDIT, _load}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_LOAD*/
{{S_EDIT, _save}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_SAVE*/
{{S_EDIT, _export},{S_SELBLK,NULL}, {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_EXPORT*/
{{S_EDIT, _redo}, {S_SELBLK,NULL},  {S_EDITBLK,_redo}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_REDO*/
{{S_EDIT, _undo}, {S_SELBLK,NULL},  {S_EDITBLK,_undo}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_UNDO*/
{{S_EDIT, _copy}, {S_SELBLK,NULL},  {S_EDITBLK,_bcopy},{S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_COPY*/
{{S_EDIT, _pst},  {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_PASTE*/
{{S_EDIT, _del},  {S_SELBLK,NULL},  {S_EDITBLK,_bdel}, {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,_mdel}},/*CMD_DEL*/
{{S_EDIT, _rcopy},{S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ROWCOPY*/
{{S_EDIT, _rpst}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ROWPASTE*/
{{S_EDIT, _ccopy},{S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_COLCOPY*/
{{S_EDIT, _cpst}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_COLPASTE*/
{{S_EDIT, _draw}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_DRAW*/
{{S_APST, _apst}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ANSIPST*/
{{S_EDIT, NULL},  {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, _amode},{S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_ANSIMODE*/
{{S_MAIL, _mail}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_MAIL*/
{{S_SELBLK,_blk1},{S_EDITBLK,_blk2},{S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_BLOCK*/
{{S_EDIT, _ret},  {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, _aret}, {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,_mconf}}, /*CMD_CONFIRM*/
{{S_EDIT, _igbk}, {S_SELBLK,NULL},  {S_EDITBLK,NULL},  {S_APST, NULL},  {S_HLPCMD,NULL},  {S_HLPCODE,NULL},  {S_MAIL,NULL}}, /*CMD_GBK*/
{{S_EDIT, _quit}, {S_EDIT, _b1quit},{S_EDIT, _b2quit}, {S_EDIT,_aquit}, {S_HLPCMD,_h1quit},{S_EDIT,_h2quit}, {S_EDIT,_mquit}}, /*CMD_QUIT*/
};
/***************************************************************************
 *                       main function                                     *
 ***************************************************************************/


int editor_main()
{
	int cmd;
	struct fsm_entry *entry;
	int oldmode;

	oldmode=uinfo.mode;

    strncpy(user_id, getCurrentUser()->userid, 20); 

	modify_user_mode(EDITOR);

	strcpy(dir_prefix, DEFAULT_DIR_PREFIX);
	init();
	//tty_raw(0);
	clr_scr();
	sprintf(message, "欢迎使用%s,<ESC>h帮助.", EDITOR_NAME);
	draw_sbar();
	gotoxy(0,0);
	state=S_EDIT;
	while (1) {
//		fflush(stdout);
		refresh();
		cmd = get_cmd();
		if ( (cmd==CMD_QUIT) && (state==S_EDIT) ){
			break;
		}
		entry = &fsm[cmd][state];
		prev_state = state;
		state = entry->next_state;
		if (entry->callback)
			entry->callback();
	}
	unload_cmdhelp();
	modify_user_mode(oldmode);
	redoscr();
	return 0;
} 
