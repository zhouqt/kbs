<?php

//$AnnounceBoard="Announce"; //公告版面

define("ANNOUNCENUMBER",5);  //首页滚动显示的公告数量

define("ARTICLESPERPAGE",30); //目录列表下每页显示的主题数

define("THREADSPERPAGE",10); //文章阅读时每页显示的文章数

define("BOARDS_PER_ROW", 3); //折叠版面列表每行版面数目

//define("SECTION_DEF_CLOSE", false); //首页分区版面列表在默认情况下是否关闭

define ('MAINTITLE','<img src="bar/title.jpg" />'); //页面正上方显示的站点标题

define('OLD_REPLY_STYLE', true); //使用传统 telnet re 文方式

define("ENABLE_UBB", true); //是否支持 UBB

define('SHOW_REGISTER_TIME', false); //是否显示用户注册时间

/* 附件：每个最大尺寸，总最大尺寸，总最大数量 */
define("ATTACHMAXSIZE","5242880");
define ("ATTACHMAXTOTALSIZE","20971520");
define("ATTACHMAXCOUNT","20");

define('SERVERTIMEZONE','北京时间'); //服务器时区
//define("SHOW_SERVER_TIME", true); //显示服务器时间

define('SHOWTELNETPARAM', 0); //是否允许配置 telnet 下专用的个人参数

//define('MYFACEDIR','uploadFace/'); //自定义头像上传保存位置

//define('MYFACEMAXSIZE','524288'); //自定义头像最大文件大小, 单位byte

define('SHOWREPLYTREE', 1);  //是否用树图显示回复结构

define('ALLOWMULTIQUERY', false); //是否允许一般用户进行全站/多版面查询
define('ALLOW_SYSOP_MULTIQUERY', false); //是否允许管理员进行全站/多版面查询
define('ALLOW_SELF_MULTIQUERY', false); //是否允许全站查询自己发表的文章

//define('AUTO_KICK', false); //一个用户登录过多时是自动（是）还是提示（否）踢出以前的登录

//define('SHOW_POST_UNREAD', true); //版面帖子列表是否显示 new 的未读标志

//define('SMS_SUPPORT', 0); //是否允许手机短信

//define('USER_FACE', 1); //是否允许自定义头像

//define('AUDIO_CHAT', 0); //是否显示语音聊天室的 link
	
$SiteURL="http://172.16.50.79"; //站点 URL，也就是本站首页地址

$SiteName="大运村BBS";   //站点名称

$Banner="pic/ws.jpg"; //页面左上角显示的图片

/* 分区代号和分区名称 */
$section_nums = array("0", "1", "2", "3", "4");
$section_names = array(
    array("BBS 系统", "[系统]"),
    array("校园信息", "[信息]"),
    array("电脑技术", "[电脑]"),
    array("学术科学", "[学习]"),
    array("休闲娱乐", "[娱乐]")
);


/* 数据库配置 */
$dbhost='localhost';
$dbuser='';
$dbpasswd='';
$dbname='';

require "default.php";
?>
