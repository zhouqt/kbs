<?php
$Version="Powered by wForum Version 0.9";
$Copyright="<a href='http://www.aka.cn/' target=_blank>阿卡信息技术(北京)有限公司</a> & <a href='http://www.smth.cn'>水木清华BBS</a> 版权所有 1994 - 2008 <br><font face=Verdana, Arial, Helvetica, sans-serif size=1><b>Roy<font color=#CC0000>@zixia.net</font></b></font> ";

if (!isset($AnnounceBoard)) { //公告版面
	$AnnounceBoard="Announce";
}

$HTMLCharset = "GB2312"; //这个我觉得肯定是 gb2312 吧，以后要变的话再做成可自定义的参数

if (!isset($HTMLTitle)) {
	$HTMLTitle = $SiteName;
}

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
	define('USEBROWSCAP',0); //默认 OFF. 设置成 ON 虽然会准确判断出非 IE 浏览器，但是会大大降低出首页的速度，慎用！//ToDo: 这个没有放入 site_defines.php
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

if (!defined('SHOWREPLYTREE')) { //是否用树图显示回复结构
	define('SHOWREPLYTREE', 1);  //默认用树图显示
}

if (!defined('ALLOWMULTIQUERY')) { //是否允许全站/多版面查询
	define('ALLOWMULTIQUERY', 0); //默认关闭
}

if (!defined('SMS_SUPPORT')) { //是否允许手机短信，ToDo: 其实最好是 phplib 里面输出 #define SMS_SUPPORT 的参数
	define('SMS_SUPPORT', 0); //默认关闭
}

if (!defined('USER_FACE')) { //是否允许自定义头像
	define('USER_FACE', 1); //默认开启
}

if (!defined('AUDIO_CHAT')) { //是否显示语音聊天室的 link
	define('AUDIO_CHAT', 0); //默认关闭
}

if (!defined('OLD_REPLY_STYLE')) { //是否使用传统 telnet re 文方式
	define('OLD_REPLY_STYLE', true); //默认关闭
}

if (!defined('SHOW_REGISTER_TIME')) { //是否显示用户注册时间
	define('SHOW_REGISTER_TIME', false); //默认关闭
}

/* 格式：每项五个参数，第一个 0 表示 telnet 下专用参数；第二个是参数名称；第三个是参数具体解释；第四五是参数 ON 和 OFF 所代表的具体含义 */
if (!isset($user_define)) {
	$user_define=array(array(0,"显示活动看版", "在telnet方式下是否显示活动看板","显示","不显示"), /* DEF_ACBOARD */
	    array(0,"使用彩色", "在telnet方式下是否使用彩色显示", "使用", "不使用"),                /* DEF_COLOR */
	    array(0, "编辑时显示状态栏","在telnet方式下编辑文章时是否显示状态栏", "显示","不显示"),         /* DEF_EDITMSG */
	    array(0,"分类讨论区以 New 显示", "在telnet方式下是否以未读方式阅读分类讨论区", "是", "否"),    /* DEF_NEWPOST */
	    array(0,"选单的讯息栏","在telnet方式下是否显示选单讯息栏","显示","不显示"),             /* DEF_ENDLINE */
	    array(0,"上站时显示好友名单","在telnet方式下上站时是否显示好友在线名单","显示","不显示"),       /* DEF_LOGFRIEND */
	    array(1,"让好友呼叫","当呼叫器关闭时是否允许好友呼叫","是", "否"),               /* DEF_FRIENDCALL */
	    array(0, "使用自己的离站画面", "telnet方式下是否使用自己的离站画面","是", "否"),      /* DEF_LOGOUT */
	    array(0, "进站时显示备忘录", "telnet方式下进站时是否显示备忘录", "是", "否"),        /* DEF_INNOTE */
	    array(0, "离站时显示备忘录", "telnet方式下离站时是否显示备忘录", "是", "否"),        /* DEF_OUTNOTE */
	    array(0, "讯息栏模式", "telnet方式下讯息栏的显示内容",  "呼叫器状态", "在线人数"), /* DEF_NOTMSGFRIEND */
	    array(0, "菜单模式选择", "telnet方式下的菜单模式", "缺省模式", "精简模式"), /* DEF_NORMALSCR */
	    array(0, "阅读文章是否使用绕卷选择", "telnet方式下阅读文章是否绕卷选择", "是","否"),/* DEF_CIRCLE */
	    array(0, "阅读文章游标停於第一篇未读","telnet方式下文章列表时光标自动定位的位置", "第一篇未读文章", "最新一篇文章"),       /* DEF_FIRSTNEW */
	    array(0, "屏幕标题色彩", "telnet方式下屏幕标题色彩显示模式", "标准", "自动变换"), /* DEF_TITLECOLOR */
	    array(1, "接受所有人的讯息", "是否允许所有人给您发短消息？","是","否"),         /* DEF_ALLMSG */
	    array(1, "接受好友的讯息", "是否好友给您发短消息？", "是", "否"),          /* DEF_FRIENDMSG */
	    array(1, "收到讯息发出声音","收到短信后是否以声音提醒您？","是","否"),         /* DEF_SOUNDMSG */
	    array(0, "离站後寄回所有讯息","您退出登陆后是否把短信息记录发回您的信箱？", "是", "否"),       /* DEF_MAILMSG */
	    array(0, "发文章时实时显示讯息","telnet方式下编辑文章时是否实时显示短消息？","是", "否"),     /*"所有好友上站均通知",    DEF_LOGININFORM */
	    array(0,"菜单上显示帮助信息","telnet方式下是否在菜单上显示帮助信息？", "是", "否"),       /* DEF_SHOWSCREEN */
	    array(0, "进站时显示十大新闻","telnet方式进站时是否显示十大热门话题？", "显示", "不显示"),       /* DEF_SHOWHOT */
	    array(0, "进站时观看留言版","telnet方式下进站时是否显示留言板","显示","不显示"),         /* DEF_NOTEPAD */
	    array(0, "忽略讯息功能键", "telnet方式下用哪个键忽略短信？", "Enter键","Esc键"),       /* DEF_IGNOREMSG */
	    array(0, "使用高亮界面","telnet方式下是否使用高亮界面？", "使用", "不使用"),         /* DEF_HIGHCOLOR */
	    array(0, "进站时观看上站人数统计图", "telnet方式下进站时是否显示上站人数统计图？", "显示", "不显示"), /* DEF_SHOWSTATISTIC Haohmaru 98.09.24 */
	    array(0, "未读标记字符","telnet方式下用哪个字符作为未读标记", "*","N"),           /* DEF_UNREADMARK Luzi 99.01.12 */
	    array(0, "使用GB码阅读","telnet方式下用GB码阅读？", "是", "否"),             /* DEF_USEGB KCN 99.09.03 */
	    array(0, "对汉字进行整字处理", "telnet方式下是否对汉字进行整字处理","是", "否"),  /* DEF_CHCHAR 2002.9.1 */
	    array(1,"显示详细用户信息", "是否允许他人看到您的性别、生日、联系方式等资料", "允许", "不允许"),  /*DEF_SHOWDETAILUSERDATA 2003.7.31 */
	    array(1,"显示真实用户信息",  "是否允许他人看到您的真实姓名、地址等资料", "允许", "不允许") /*DEF_REALDETAILUSERDATA 2003.7.31 */
	);
}

if (!isset($user_define1)) {
	$user_define1=array(array(1,"隐藏 IP", "是否发文和被查询的时候隐藏自己的 IP 信息","不隐藏","隐藏") /* DEF_HIDEIP */
	);
}

if (!isset($mailbox_prop)) {
	$mailbox_prop=array(array(1,"发信时保存信件到发件箱", "是否发信时自动选择保存到发件箱","保存","不保存"),
		array(1,"删除信件时不保存到垃圾箱", "是否删除信件时不保存到垃圾箱","不保存","保存"),
		array(0,"快捷邮箱键", "telnet方式下版面按 'v' 时进入什么界面？","信箱主界面","收件箱")
	);
}
?>
