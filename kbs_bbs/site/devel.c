#include "bbs.h"

const char seccode[SECNUM][5] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C",
};
const char  * const groups[] = {
    "system.faq",
    "thu.faq",
    "sci.faq",
    "rec.faq",
    "literal.faq",
    "social.faq",
    "game.faq",
    "sport.faq",
    "talk.faq",
    "info.faq",
    "develop.faq",
    "os.faq",
    "comp.faq",
    NULL
};
const char secname[SECNUM][2][20] = {
    {"BBS 系统", "[站内]"},
    {"清华大学", "[本校]"},
    {"学术科学", "[学科/语言]"},
    {"休闲娱乐", "[休闲/音乐]"},
    {"文化人文", "[文化/人文]"},
    {"社会信息", "[社会/信息]"},
    {"游戏天地", "[游戏/对战]"},
    {"体育健身", "[运动/健身]"},
    {"知性感性", "[谈天/感性]"},
    {"电脑信息", "[电脑/信息]"},
    {"软件开发", "[语言/工具]"},
    {"操作系统", "[系统/内核]"},
    {"电脑技术", "[专项技术]"},
};
