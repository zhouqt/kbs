<?php
define("ATTACHTMPPATH","boards/_attach");
function getattachtmppath($userid,$utmpnum)
{
	return ATTACHTMPPATH . "/" . $userid . "_" . $utmpnum;
}

define("ATTACHMAXSIZE","1048576");
define("ATTACHMAXCOUNT","20");
$section_nums = array("0", "1", "3", "4", "5", "6", "7", "8", "9");
$section_names = array(
    array("BBS 系统", "[本站]"),
    array("快乐聚会", "[院系][协会][团体]"),
    array("电脑技术", "[电脑][系统][网路]"),
    array("电脑游戏", "[游戏]"),
    array("艺术文化", "[文艺][艺术][学术]"),
    array("转信专区", "[转信]"),
    array("体育休闲", "[体育][星座][音乐]"),
    array("知性感性", "[闲聊][感性]"),
    array("新闻时事", "[新闻][特快][信息]")
);

$order_articles = TRUE;
$default_dir_mode = $dir_modes["ORIGIN"];
?>
