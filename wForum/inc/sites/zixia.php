<?php
require "default.php";
//define("ATTACHTMPPATH","boards/_attach");
function getattachtmppath($userid,$utmpnum)
{
  $attachdir="cache/home/" . strtoupper(substr($userid,0,1)) . "/" . $userid . "/" . $utmpnum . "/upload";
  return $attachdir;
//  return ATTACHTMPPATH;
}

define("ANNOUNCENUMBER",5);

define("ARTICLESPERPAGE",20);

$SiteName="测试BBS";

$SiteURL="http://news.zixia.net:8080";

$HTMLTitle="测试BBS";

$HTMLCharset="GB2312";

$DEFAULTStyle="defaultstyle";

$Banner="pic/ws.jpg";

$BannerURL="http://aka.com.cn";

define("ATTACHMAXSIZE","1048576");
define("ATTACHMAXCOUNT","20");

$section_nums = array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9");
$section_names = array(
    array("斧 头 帮", "[黑帮/系统]"),
    array("大话西游", "[大话/技术]"),
    array("大小分舵", "[地盘/校班]"),
    array("吃喝玩乐", "[美食/休闲]"),
    array("游戏人生", "[游戏/娱乐]"),
    array("吟风弄月", "[感性/文化]"),
    array("西游之路", "[出国/聚会]"),
    array("豆腐西施", "[店面/培训]"),
    array("月光宝盒", "[转信/列表]"),
    array("大圣取经", "[专业/申请]") 
);
$sectionCount=count($section_names);
?>
