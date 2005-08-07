<?php

define("ANNOUNCENUMBER",5);

define("ARTICLESPERPAGE",30); //目录列表下每页显示的主题数

define("THREADSPERPAGE",10); //文章阅读时每页显示的文章数

$SiteURL="http://192.168.0.11:8080/wForum/";

$SiteName="测试站";

$HTMLTitle="BBS 测试站站";

$Banner="bar/bar.jpg";

define('SMS_SUPPORT', 0);

define ("MAINTITLE","<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0\" height=\"84\" width=\"600\"> <param name=\"MOVIE\" value=\"bar/smth2.swf\" /><embed SRC=\"bar/smth2.swf\" height=\"84\" width=\"600\"></embed></object>");

define("ATTACHMAXSIZE","2097152");
define("ATTACHMAXCOUNT","3");
$section_nums = array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9","A","B","C");
$section_names = array(
    array("BBS 系统", "[站内]"),
    array("清华大学", "[本校]"),
    array("学术科学", "[学科/语言]"),
    array("休闲娱乐", "[休闲/音乐]"),
    array("文化人文", "[文化/人文]"),
    array("社会信息", "[社会/信息]"),
    array("游戏天地", "[游戏/娱乐]"),
    array("体育健身", "[运动/健身]"),
    array("知性感性", "[谈天/感性]"),
    array("电脑信息", "[电脑/信息]"),
    array("软件开发", "[语言/工具]"),
    array("操作系统", "[系统/内核]"),
    array("电脑技术", "[专项技术]")
);

define('COOKIE_PREFIX', '');
define('COOKIE_PATH', '/');

require "default.php";
?>
