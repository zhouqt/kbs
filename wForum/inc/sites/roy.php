<?php

define('ANNOUNCENUMBER',5);

define('ARTICLESPERPAGE',30); //目录列表下每页显示的主题数

define('THREADSPERPAGE',10); //文章阅读时每页显示的文章数

$SiteURL="http://172.16.50.79/";

$SiteName="大运村BBS";

$Banner="pic/ws.jpg";

define ('MAINTITLE','<img src="bar/title.jpg" />');
define('ATTACHMAXSIZE','5242880');
define ('ATTACHMAXTOTALSIZE','20971520');
define('ATTACHMAXCOUNT','20');

$section_nums = array("0", "1", "2", "3", "4");
$section_names = array(
    array("BBS 系统", "[系统]"),
    array("校园信息", "[信息]"),
    array("电脑技术", "[电脑]"),
    array("学术科学", "[学习]"),
    array("休闲娱乐", "[娱乐]")
);

$FooterBan='<table cellSpacing=0 cellPadding=0 border=0 align=center><tr><td width=100 align=center><a href="http://www.turckware.ru/en/e_mmc.htm#download" target="_blank"><img border="0" src="/images/copyright/mmcache02.gif"></a></td><td width=100 align=center><a href="http://www.php.net/" target="_blank"><img border="0" src="/images/copyright/php-small-trans-light.gif"></a></td><td width=100 align=center><a href="http://www.apache.org/" target="_blank"><img border="0" src="/images/copyright/apache_pb.gif"></a></td><td width=100 align=center><a href="http://www.redhat.com/" target="_blank"><img border="0" src="/images/copyright/poweredby.png"></a></td><td width=100 align=center><a href="http://www.mysql.com/" target="_blank"><img border="0" src="/images/copyright/mysql-17.gif"></a></td></tr></table>';

require "default.php";
?>
