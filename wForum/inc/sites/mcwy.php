<?php

define('SERVERTIMEZONE','美西时间');

$SiteURL="http://bbs.stanford.edu/wForum/";

$SiteName="牧场物语";

$Banner="/mcwy/bm3_08.jpg";

$AnnounceBoard = "Announcement";

define ("MAINTITLE","&nbsp;");
define("ATTACHMAXSIZE","8388608");
define ("ATTACHMAXTOTALSIZE","8388608");
define("ATTACHMAXCOUNT","20");

$section_nums = array("0", "1", "2", "3", "4", "5", "6");
$section_names = array(
    array("本站系统", "[本站]"),
    array("人在美国", "[校园][创业]"),
    array("大话西游", "[留学][学校][团体]"),
    array("学术研讨", "[学术][科学][电脑]"),
    array("感性生活", "[休闲][生活][感性]")
);

$user_define1=array(array(1,"隐藏 IP", "是否发文和被查询的时候隐藏自己的 IP 信息","部分隐藏","完全隐藏") /* DEF_HIDEIP */
);

define('SHOWTELNETPARAM',0);

define('ALLOWMULTIQUERY', 1);

//$dbhost='localhost';
$dbuser='wForum';
$dbpasswd='fuckatp';
$dbname='wForum';

require "default.php";
?>
