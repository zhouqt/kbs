<?php
function getattachtmppath($userid,$utmpnum)
{
  $attachdir="cache/home/" . strtoupper(substr($userid,0,1)) . "/" . $userid . "/" . $utmpnum . "/upload";
  return $attachdir;
}

define("ATTACHMAXSIZE","2097152");
define("ATTACHMAXCOUNT","3");
$section_nums = array("0", "1", "2", "3", "4", "5", "6", "7", "8");
$section_names = array(
    array("温馨小屋", "[本站][系统]"),
    array("校园生活", "[校园][协会]"),
    array("BBS 技术", "[版本][管理][转信]"),
    array("电脑技术", "[编程][电脑][系统]"),
    array("学术科学", "[学习][语言]"),
    array("文化人文", "[文学]"),
    array("休闲娱乐", "[休闲][音乐][娱乐]"),
    array("知性感性", "[感性][闲聊]"),
    array("体育新闻", "[体育][新闻][信息]")
);
?>
