<?php
/* automatically generated site configuration file */

/* 分类讨论区 */
$section_nums = array("1", "2", "3", "4", "0");
$section_names = array(
	array("人在美国", "[校园][创业]"),
	array("大话西游", "[留学][学校][团体]"),
	array("学术研讨", "[学术][科学][电脑]"),
	array("感性生活", "[休闲][生活][感性]"),
	array("本站系统", "[本站]")
);

/* 特殊的个人自定义参数 */
$user_define1 = array(
	array(1, "隐藏 IP", "是否发文和被查询的时候隐藏自己的 IP 信息", "部分隐藏", "完全隐藏")
);

/* 网站基本配置 */
$SiteURL = "http://bbs.stanford.edu/wForum/";
$SiteName = "牧场物语";
$Banner = "/mcwy/bm3_08.jpg' height='60";
define("MAINTITLE", "本站 wForum 永远可能处于调试状态。请小心使用<img src=\"emot/em17.gif\" border=\"0\"/>");
define("OLD_REPLY_STYLE", true);
define("ENABLE_UBB", true);
$AnnounceBoard = "Announcement";
define("SERVERTIMEZONE", "美西时间");

/* 页面参数 */
define("TREEVIEW_MAXITEM", 201);

/* 附件相关 */
define("ATTACHMAXSIZE", "10485760");
define("ATTACHMAXTOTALSIZE", "10485760");
define("ATTACHMAXCOUNT", "20");

/* 自定义头像相关 */

/* 数据库相关 */
$dbuser = "wForum";
$dbpasswd = "fuckatp";
$dbname = "wForum";

/* 其他附加功能 */
define("SHOWTELNETPARAM", false);
define("ALLOW_SYSOP_MULTIQUERY", true);
define('ALLOW_SELF_MULTIQUERY', true);
define("SUPPORT_TEX", true);

/* 默认配置 */
define('COOKIE_PREFIX', '');
define('COOKIE_PATH', '/');
require "default.php";
?>
