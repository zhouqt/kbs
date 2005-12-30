<?php
define("ATTACHMAXSIZE",BBS_MAXATTACHMENTSIZE);        //附件总字节数的上限，单位 bytes
define("ATTACHMAXCOUNT",BBS_MAXATTACHMENTCOUNT);      //附件数目的上限
define("MAINPAGE_FILE","mainpage.html");              //首页导读的 URL
define("QUOTED_LINES", BBS_QUOTED_LINES);             //web 回文保留的引文行数
define("SITE_NEWSMTH", 1);

$section_nums = array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9");
$section_names = array(
    array("社区管理", "[社区/系统]"),
    array("国内院校", "[院校/地域]"),
    array("休闲娱乐", "[休闲/音乐]"),
    array("游戏天地", "[游戏/对战]"),
    array("体育健身", "[运动/健身]"),
    array("社会信息", "[社会/信息]"),
    array("知性感性", "[谈天/感性]"),
    array("文化人文", "[文化/人文]"),
    array("学术科学", "[学科/语言]"),
    array("电脑技术", "[电脑/技术]")
);

// 界面方案的名称
$style_names = array(
	"默认方案（蓝色经典）",
	"白的（猪猪猪测试用）"
);

?>
