#include "screen.h"

//##ModelId=3CFE32F70347
BBSWindow::Screen::Screen(int slns, int scols)
{
}

//##ModelId=3CFE32F70383
void BBSWindow::Screen::redoscr()
{
}

//##ModelId=3CFE32F70397
void BBSWindow::Screen::refresh()
{
}

//##ModelId=3CFE32F703AB
void BBSWindow::Screen::move(int y, int x)
{
}

//##ModelId=3CFE32F703E7
void BBSWindow::Screen::getyx(int *y, int *x)
{
}

//##ModelId=3CFE32F80031
void BBSWindow::Screen::clear()
{
}

//##ModelId=3CFE32F80045
void BBSWindow::Screen::clear_whole_line(int i)
{
}

//##ModelId=3CFE32F8006D
void BBSWindow::Screen::clrtoeol()
{
}

//##ModelId=3CFE32F80081
void BBSWindow::Screen::clrtobot()
{
}

//##ModelId=3CFE32F8009F
void BBSWindow::Screen::outc(unsigned char c)
{
}

//##ModelId=3CFE32F800C7
void BBSWindow::Screen::outns(const char *str, int n)
{
}

//##ModelId=3CFE32F80103
void BBSWindow::Screen::outs(const char *str)
{
}

//##ModelId=3CFE32F8012B
void BBSWindow::Screen::prints(char *format, ...)
{
}

//##ModelId=3CFE32F80167
void BBSWindow::Screen::scroll()
{
}

//##ModelId=3CFE32F8017B
void BBSWindow::Screen::rscroll()
{
}

//##ModelId=3CFE32F8018F
void BBSWindow::Screen::standout()
{
}

//##ModelId=3CFE32F801A3
void BBSWindow::Screen::standend()
{
}

//##ModelId=3CFE32F801B8
void BBSWindow::Screen::saveline(int line, int mode, char *buffer)
{
}

//##ModelId=3D001A7E025A
WORD BBSWindow::Screen::get_cur_attr() const
{
    return cur_attr;
}


//##ModelId=3D001A7E02D2
void BBSWindow::Screen::set_cur_attr(WORD left)
{
    cur_attr = left;
}


//##ModelId=3D001D2D0031
void BBSWindow::Screen::set_fg_color(int color)
{
}

//##ModelId=3D001DF7026C
void BBSWindow::Screen::set_bg_color(int color)
{
}

//##ModelId=3D001E8500C2
void BBSWindow::Screen::set_highlight(bool hi)
{
}

//##ModelId=3D001EBA001E
void BBSWindow::Screen::set_underline(bool underline)
{
}

//##ModelId=3D001F410180
void BBSWindow::Screen::set_twinkling(bool twink)
{
}

//##ModelId=3D00192E015C
const int BBSWindow::Screen::BLACK = 0;

//##ModelId=3D00192E01FD
const int BBSWindow::Screen::RED = 1;

//##ModelId=3D00192E0257
const int BBSWindow::Screen::GREEN = 2;

//##ModelId=3D00192E02B1
const int BBSWindow::Screen::YELLOW = 3;

//##ModelId=3D00192E031F
const int BBSWindow::Screen::BLUE = 4;

//##ModelId=3D00192E0379
const int BBSWindow::Screen::LT_RED = 5;

//##ModelId=3D00192E03DD
const int BBSWindow::Screen::LT_BLUE = 6;

//##ModelId=3D00192F005A
const int BBSWindow::Screen::WHITE = 7;

//##ModelId=3D001F830009
const int BBSWindow::Screen::ATTR_STAND = 0x40;

//##ModelId=3D001FCC01EF
const int BBSWindow::Screen::ATTR_UNDERLINE = 0x80;

//##ModelId=3D0020050164
const int BBSWindow::Screen::ATTR_HIGHLIGHT = 0x100;

//##ModelId=3D00202E0091
const int BBSWindow::Screen::ATTR_TWINKLING = 0x200;

//##ModelId=3D00205C02FA
const int BBSWindow::Screen::ATTR_DEFAULT = 0x400;

