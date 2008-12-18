#ifndef __ANSI_H__
#define __ANSI_H__

#define STATE_ESC_SET 0x01
#define STATE_FONT_SET    0x02
#define STATE_NEW_LINE 0x04
#define STATE_QUOTE_LINE 0x08
#define STATE_NONE   0x00
#define STATE_UBB_START 0x10
#define STATE_UBB_MIDDLE 0x20
#define STATE_UBB_END 0x40
#define STATE_TEX_SET   0x80

enum UBBTYPE {UBB_TYPE_IMG,
              UBB_TYPE_ITALICIZE,
              UBB_TYPE_UNDERLINE,
              UBB_TYPE_BOLD,
              UBB_TYPE_FLY,
              UBB_TYPE_RM,
              UBB_TYPE_FLASH,
              UBB_TYPE_CENTER,
              UBB_TYPE_EMAIL,
              UBB_TYPE_HTTPLINK,
              UBB_TYPE_QUOTE,
              UBB_TYPE_QUICKTIME,
              UBB_TYPE_SHOCKWAVE,
              UBB_TYPE_MOVE,
              UBB_TYPE_GLOW,
              UBB_TYPE_SHADOW,
              UBB_TYPE_FACE,
              UBB_TYPE_SOUND,
              UBB_TYPE_ATTACH
             };

enum ATTACHMENTTYPE {
    ATTACH_IMG,
    ATTACH_FLASH,
    ATTACH_OTHERS
};

#define STATE_SET(s, b) ((s) |= (b))
#define STATE_CLR(s, b) ((s) &= ~(b))
#define STATE_ISSET(s, b) ((s) & (b))
#define STATE_ZERO(s) (s = 0)

#define STYLE_SET_FG(s, c) (s = (s & ~0x07) | (c & 0x07))
#define STYLE_SET_BG(s, c) (s = (s & ~0x70) | ((c & 0x07) << 4))
#define STYLE_GET_FG(s) (s & 0x0F)
#define STYLE_GET_BG(s) ((s & 0x70) >> 4)
#define STYLE_CLR_FG(s) (s &= ~0x0F)
#define STYLE_CLR_BG(s) (s &= ~0xF0)
#define STYLE_ZERO(s) (s = 0)
#define STYLE_SET(s, b) (s |= b)
#define STYLE_CLR(s, b) (s &= ~b)
#define STYLE_ISSET(s, b) (s & b)

#define FONT_STYLE_UL     0x0100
#define FONT_STYLE_BLINK  0x0200
#define FONT_STYLE_ITALIC 0x0400

#define FONT_FG_BOLD      0x08
#define FONT_COLOR_BLACK     0x00
#define FONT_COLOR_RED       0x01
#define FONT_COLOR_GREEN     0x02
#define FONT_COLOR_YELLOW    0x03
#define FONT_COLOR_BULE      0x04
#define FONT_COLOR_MAGENTA   0x05
#define FONT_COLOR_CYAN      0x06
#define FONT_COLOR_WHITE     0x07

//#define FONT_STYLE_QUOTE FONT_STYLE_ITALIC
#define FONT_STYLE_QUOTE 0x0000
#define FONT_COLOR_QUOTE FONT_COLOR_CYAN

#define FONT_BG_SET       0x80

#endif /* __ANSI_H__ */
