/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


/* Maximum Screen width in chars */
/*#define LINELEN (220) */
#define LINELEN LENGTH_SCREEN_LINE

/* Line buffer modes             */
#define MODIFIED (1)   /* if line has been modifed, output to screen   */
#define STANDOUT (2)   /* if this line has a standout region */

namespace BBSWindow {

//##ModelId=3CFDD0B8039B
//##Documentation
//## 客户终端的屏幕缓冲区
class Screen
{
  public:
    //##ModelId=3CFE32F70347
    //##Documentation
    //## 构造函数，传入初始化的屏幕行数和列数
    Screen(int slns = 25, int scols = 80);

    //##ModelId=3CFE32F70383
    //##Documentation
    //## 全部重绘屏幕
    void redoscr();

    //##ModelId=3CFE32F70397
    //##Documentation
    //## 刷新修改过的屏幕
    void refresh();

    //##ModelId=3CFE32F703AB
    //##Documentation
    //## 光标移动到(x,y)点
    void move(int y, int x);

    //##ModelId=3CFE32F703E7
    //##Documentation
    //## 获得当前的光标位置
    void getyx(int *y, int *x);

    //##ModelId=3CFE32F80031
    //##Documentation
    //## 清屏
    void clear();

    //##ModelId=3CFE32F80045
    //##Documentation
    //## 清除第i行
    void clear_whole_line(int i);

    //##ModelId=3CFE32F8006D
    //##Documentation
    //## 清到行末
    void clrtoeol();

    //##ModelId=3CFE32F80081
    //##Documentation
    //## 清到页末
    void clrtobot();

    //##ModelId=3CFE32F8009F
    //##Documentation
    //## 输出一个字符
    void outc(unsigned char c);

    //##ModelId=3CFE32F800C7
    //##Documentation
    //## 输出n个字符的字符串
    void outns(const char *str, int n);

    //##ModelId=3CFE32F80103
    //##Documentation
    //## 输出字符串
    void outs(const char *str);

    //##ModelId=3CFE32F8012B
    //##Documentation
    //## 类似printf的屏幕输出
    void prints(char *format, ...);

    //##ModelId=3CFE32F80167
    //##Documentation
    //## 滚行
    void scroll();

    //##ModelId=3CFE32F8017B
    //##Documentation
    //## 上滚一行
    void rscroll();

    //##ModelId=3CFE32F8018F
    //##Documentation
    //## 开始反白输出，直接设置当前画笔属性加上反白
    void standout();

    //##ModelId=3CFE32F801A3
    //##Documentation
    //## 反白输出结束，去掉当前画笔反白属性
    void standend();

    //##ModelId=3CFE32F801B8
    //##Documentation
    //## 保存第i行
    void saveline(int line, int mode, char *buffer);

    //##ModelId=3D001A7E025A
    WORD get_cur_attr() const;

    //##ModelId=3D001A7E02D2
    void set_cur_attr(WORD left);

    //##ModelId=3D001D2D0031
    void set_fg_color(int color);

    //##ModelId=3D001DF7026C
    void set_bg_color(int color);

    //##ModelId=3D001E8500C2
    void set_highlight(bool hi);

    //##ModelId=3D001EBA001E
    void set_underline(bool underline);

    //##ModelId=3D001F410180
    void set_twinkling(bool twink);

    //##ModelId=3D00192E015C
    //##Documentation
    //## 黑色
    static const int BLACK;

    //##ModelId=3D00192E01FD
    //##Documentation
    //## 红色
    static const int RED;

    //##ModelId=3D00192E0257
    //##Documentation
    //## 绿色
    static const int GREEN;

    //##ModelId=3D00192E02B1
    //##Documentation
    //## 黄色
    static const int YELLOW;

    //##ModelId=3D00192E031F
    //##Documentation
    //## 深蓝色
    static const int BLUE;

    //##ModelId=3D00192E0379
    //##Documentation
    //## 粉红色
    static const int LT_RED;

    //##ModelId=3D00192E03DD
    //##Documentation
    //## 淡蓝色
    static const int LT_BLUE;

    //##ModelId=3D00192F005A
    //##Documentation
    //## 白色
    static const int WHITE;

  private:
    //##ModelId=3CFDE54B00F6
    //##Documentation
    //## 屏幕的总行数，至少应该 >=25
    unsigned char scr_lns;

    //##ModelId=3CFDE54B010A
    //##Documentation
    //## 屏幕的总列数，应该 =80
    unsigned char scr_cols;

    //##ModelId=3CFDE54B0114
    //##Documentation
    //## 当前光标所在行
    unsigned char cur_ln;

    //##ModelId=3CFDE54B011E
    //##Documentation
    //## 当前光标所在列
    unsigned char cur_col;

    //##ModelId=3CFDE54B0128
    //##Documentation
    //## 当前屏幕在big_picture数组中的起始下标
    unsigned char roll;

    //##ModelId=3CFDE54B0132
    //##Documentation
    //## 当前屏幕已经滚动了多少行
    unsigned char scrollcnt;

    //##ModelId=3CFDE54B013C
    //##Documentation
    //## 已经清屏幕，下次需要调用redoscr来重绘
    unsigned char docls;

    //##ModelId=3CFDE54B015A
    //##Documentation
    //## 当前正在反白输出
    unsigned char standing;

    //##ModelId=3CFDE54B015B
    //##Documentation
    //## 是否正在过滤esc后面的字符
    int inansi;

    //##ModelId=3CFE2F4F02BF
    //##Documentation
    //## 是否过滤ansi换码序列
    int fitler_color;

    //##ModelId=3CFE48B600AD
    //##Documentation
    //## 当前画笔属性
    WORD cur_attr;

    //##ModelId=3D001F830009
    static const int ATTR_STAND;

    //##ModelId=3D001FCC01EF
    static const int ATTR_UNDERLINE;

    //##ModelId=3D0020050164
    static const int ATTR_HIGHLIGHT;

    //##ModelId=3D00202E0091
    static const int ATTR_TWINKLING;

    //##ModelId=3D00205C02FA
    static const int ATTR_DEFAULT;

    //##ModelId=3D00207D038E
    const int ATTR_MODIFIED;

    //##ModelId=3CFE303F03DD
    //##Documentation
    //## 屏幕输出的数组。是一个scr_lns行的union screenline
    screenline *big_picture;

};

} // namespace BBSWindow

namespace BBSWindow {

//##ModelId=3CFDE56501F8
union screenline
{
    //##ModelId=3CFDE58C0077
    char rawlinebuf[LINELEN];

    //##ModelId=3CFE4D10000F
    struct _line linebuf;

};

} // namespace BBSWindow

namespace BBSWindow {

//##ModelId=3CFE4CED019F
struct _line
{
    //##ModelId=3CFE4D07030E
    //##Documentation
    //## 每行的显示数据
    char linebuf[80];

    //##ModelId=3CFE4D07030F
    //##Documentation
    //## 每一个对应的属性
    DWORD attr[80];

    //##ModelId=3CFE4D070318
    //##Documentation
    //## 被修改过的起始位置
    unsigned char mod_start;

    //##ModelId=3CFE4D070319
    //##Documentation
    //## 行的数据长度
    unsigned char line_len;

};

} // namespace BBSWindow

