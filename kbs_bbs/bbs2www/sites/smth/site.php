<?php

define("ATTACHMAXSIZE","2097152");
define("ATTACHMAXCOUNT","10");
define("HAVE_PC", 1); // 支持个人文集
define("MAINPAGE_FILE","mainpage.html"); // 使用静态 mainpage 页面
define("STATIC_FRAME",1); //使用静态 frame
define("HAVE_BRDENV", 1); //支持版面导读
define("QUOTED_LINES","3");
define("ACTIVATIONLEN",15); //激活码长度
define("ENABLE_ABOARDS" , 1);//web首页使用活动看板
define("SERVICE_QUIZ" , 1); //www开心辞典
define("SITE_SMTH",1);
define('BBS_NEWPOSTSTAT',1);
define('MIN_REG_TIME', 48);

$domain_name = explode(":",trim($_SERVER["HTTP_HOST"]));
define("BBS_DOMAIN_NAME" , $domain_name[0]); //域名

$section_nums = array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9","A","B","C");
$section_names = array(
    array("BBS 系统", "[站内]"),
    array("青春校园", "[清华/院校]"),
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
?>
