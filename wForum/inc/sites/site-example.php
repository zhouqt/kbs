<?php

define("ANNOUNCENUMBER",5);

define("ARTICLESPERPAGE",30); //目录列表下每页显示的主题数

define("THREADSPERPAGE",10); //文章阅读时每页显示的文章数

define('SERVERTIMEZONE','北京时间');

$SiteURL="http://172.16.50.79";

$SiteName="大运村BBS";

$HTMLTitle="大运村BBS";

$HTMLCharset="GB2312";

$DEFAULTStyle="defaultstyle";

$Banner="pic/ws.jpg";

$BannerURL="http://172.16.50.79";

define ("MAINTITLE","<IMG SRC=\"bar/title.jpg\">");
define("ATTACHMAXSIZE","5242880");
define ("ATTACHMAXTOTALSIZE","20971520");
define("ATTACHMAXCOUNT","20");

$section_nums = array("0", "1", "2", "3", "4");
$section_names = array(
    array("BBS 系统", "[系统]"),
    array("校园信息", "[信息]"),
    array("电脑技术", "[电脑]"),
    array("学术科学", "[学习]"),
    array("休闲娱乐", "[娱乐]")
);
$sectionCount=count($section_names);

define('SHOWTELNETPARAM', 0); //不允许配置 telnet 下专用的个人参数
define('SHOWREPLYTREE', 1);  //用树图显示回复结构
define('ALLOWMULTIQUERY', 1); //允许全站/多版面查询

require "default.php";
?>
