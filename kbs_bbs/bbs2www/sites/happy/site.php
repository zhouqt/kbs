<?php
define("ATTACHTMPPATH","boards/_attach");
function getattachtmppath($userid,$utmpnum)
{
	return ATTACHTMPPATH . "/" . $userid . "_" . $utmpnum;
}

define("ATTACHMAXSIZE","1048576");
define("ATTACHMAXCOUNT","20");
define("HAVE_PC", 1); // 支持个人文集
define("MAINPAGE_FILE","mainpage.php"); // 不使用静态 mainpage 页面
$section_nums = array("0", "1", "3", "4", "5", "6", "7", "8", "9");
$section_names = array(
	array("本站系统", "[HAPPYBBS系统区]"),
	array("网络服务", "[WWW][FTP][校园网]"),
	array("群体组织", "[院系][协会][团体]"),
	array("个人空间", "[你] [我] [他] [她]"),
	array("艺术文化", "[文艺][艺术][学术]"),
	array("电脑技术", "[电脑][系统][网路]"),
	array("休闲娱乐", "[流行][星座][游戏]"),
	array("知性感性", "[闲聊][感性]"),
	array("其他版面", "[未分类]")
);

$order_articles = TRUE;
$default_dir_mode = $dir_modes["ORIGIN"];
?>
