<?php
define("ATTACHTMPPATH","boards/_attach");
function getattachtmppath($userid,$utmpnum)
{
  return ATTACHTMPPATH . "/" . $userid . "_" . $utmpnum;
}

define("ATTACHMAXSIZE","1048576");
define("ATTACHMAXCOUNT","20");
define("MAINPAGE_FILE","mainpage.html");

$section_nums = array(
		"ab", "cd", "ef", "gh", "ij", "kl", "mn", "opq"
				);
$section_names = array(
    array("本站系统", "[站内]"),
    array("我们的家", "[本校]"),
    array("人文艺术", "[学科/语言]"),
    array("电脑技术", "[休闲/音乐]"),
    array("学术科学", "[文化/人文]"),
    array("休闲娱乐", "[社会/信息]"),
    array("知性感性", "[谈天/感性]"),
    array("体育健身", "[运动/健身]")
);
?>
