<?php
$Version="Powered by wForum Version 0.9";
$Copyright="<a href='http://www.aka.cn/' target=_blank>阿卡信息技术(北京)有限公司</a> & <a href='http://www.smth.cn'>水木清华BBS</a> 版权所有 1994 - 2008 <br><font face=Verdana, Arial, Helvetica, sans-serif size=1><b>Roy<font color=#CC0000>@zixia.net</font></b></font> ";

$AnnounceBoard="Announce";
if (!defined('BOARDLISTSTYLE')) { 
/* 版面列表方式
 * default 缺省
 * simplest 不显示列表
 */
	define('BOARDLISTSTYLE','default');
}

if (!defined('ANNOUNCENUMBER')) {   //首页滚动显示的公告数量
	define('ANNOUNCENUMBER',5);
}

if (!defined('ARTICLESPERPAGE')) {
	define('ARTICLESPERPAGE',30); //目录列表下每页显示的主题数
}

if (!defined('THREADSPERPAGE')) { //同主题阅读每页文章数
	define('THREADSPERPAGE',10);
}

if (!defined('MAINTITLE')) { //标题图片
	define ('MAINTITLE','<IMG SRC="bar/title.jpg">');
}

if (!defined('ATTACHMAXSIZE')) { //最大附件尺寸，需要同时修改php.ini里的upload_max_size ,单位byte
	define('ATTACHMAXSIZE','524288');
}

if (!defined('ATTACHMAXTOTALSIZE')) { //一篇文章中所有附件的总大小限制, 单位byte
	define('ATTACHMAXTOTALSIZE','2097152');
}

if (!defined('ATTACHMAXCOUNT')) { //一篇文章中最多可上载的附件数量
	define('ATTACHMAXCOUNT','5');
}


?>
