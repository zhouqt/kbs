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

if (!defined('USERSPERPAGE')) {
    define("USERSPERPAGE", 20); //在线用户/好友等列表每页显示的人数
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

if (!defined('SERVERTIMEZONE')) { //服务器时区 - 暂时这样搞吧 - atppp
	define('SERVERTIMEZONE','北京时间');
}

if (!defined('USEBROWSCAP')) { //是否使用 browscap 函数来更准确地判断浏览器和操作系统类型，需要配置 PHP - atppp
	define('USEBROWSCAP',0); //默认 OFF
}

if (!defined('SHOWTELNETPARAM')) { //是否允许配置 telnet 下专用的个人参数 - atppp
	define('SHOWTELNETPARAM',1); //默认 ON，保持和以前的兼容性
}

if (!defined('MYFACEDIR')) { //自定义头像上传保存位置
	define('MYFACEDIR','uploadFace/');
}

if (!defined('MYFACEMAXSIZE')) { //自定义头像最大文件大小, 单位byte
	define('MYFACEMAXSIZE','524288');
}

?>
