<?php

define("ANNOUNCENUMBER",5);

define("ARTICLESPERPAGE",30); //目录列表下每页显示的主题数

define("THREADSPERPAGE",10); //文章阅读时每页显示的文章数

$SiteURL="http://bbs.bjsing.net:8081/forum/";

$SiteName="北极星BBS";

$Banner="pic/ws.jpg";

define ("MAINTITLE","<IMG SRC=\"bar/title.jpg\">");
define("ATTACHMAXSIZE","5242880");
define ("ATTACHMAXTOTALSIZE","20971520");
define("ATTACHMAXCOUNT","20");

$section_nums = array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9");
$section_names = array(
    array("本站系统", "[站内]"),
    array("石油学院", "[本校]"),
    array("兄弟院校", "[高校][组织]"),
    array("电脑技术", "[电脑][网路]"),
    array("休闲娱乐", "[休闲][娱乐]"),
    array("学术科学", "[文理][科学]"),
    array("文艺社交", "[文学][艺术]"),
    array("体育健身", "[运动][健身]"),
    array("谈天聊地", "[谈天][新闻]"),
    array("技术转信", "[电脑][系统]"),
);

require "default.php";
?>
