<?php
/*
 * 可配置参数总列表
 *
 * 格式
 * 0. integer: 是常数(0)还是变量(1)，一般站点定义参数都应该是常数
 * 1. string:  常量或变量名称
 * 2. string:  常量或变量类型：integer(i), string(s), boolean(b), enum 待加
 * 3. mixed:   默认值
 * 4. string:  参数说明
 */
$site_defines = array(
"网站基本配置",
array(1, "SiteURL", 's', "http://localhost/", "站点根地址，注意最后一个字符必须是 /"),
array(1, "SiteName", 's', "测试站", "站点名称"),
array(1, "Banner", 's', "pic/ws.jpg", "页面左上角显示的站点标题图片"),
array(0, "MAINTITLE", 's', "<img src=\"bar/title.jpg\" />", "页面正上方显示的站点标题"),
array(0, "OLD_REPLY_STYLE", 'b', false, "是否使用传统 telnet re 文方式"),
array(0, "ENABLE_UBB", 'b', true, "是否支持 UBB"),
array(0, "SHOW_REGISTER_TIME", 'b', false, "是否显示用户注册时间"),
array(1, "AnnounceBoard", 's', "Announce", "系统公告版面英文名称"),
array(0, "SERVERTIMEZONE", 's', '北京时间', "服务器时区"),
array(0, "SHOW_SERVER_TIME", 'b', true, "显示服务器时间"),
"页面参数",
array(0, "ANNOUNCENUMBER", 'i', 5, "首页滚动显示的公告数量"),
array(0, "ARTICLESPERPAGE", 'i', 30, "目录列表下每页显示的主题数"),
array(0, "USERSPERPAGE", 'i', 20, "在线用户/好友等列表每页显示的人数"),
array(0, "THREADSPERPAGE", 'i', 10, "同主题阅读每页文章数"),
array(0, "BOARDS_PER_ROW", 'i', 3, "折叠版面列表每行版面数目"),
array(0, "SHOWREPLYTREE", 'b', true, "是否用树图显示回复结构"),
array(0, "TREEVIEW_MAXITEM", 'i', 51, "树形回复结构最多显示的回复数"),
array(0, "SHOW_POST_UNREAD", 'b', true, "版面帖子列表是否显示 new 的未读标志"),
array(0, "SECTION_DEF_CLOSE", 'b', false, "首页分区版面列表在默认情况下是否关闭"),
"附件相关",
array(0, "ATTACHMAXSIZE", 's', '524288', "最大附件尺寸，需要同时修改php.ini里的upload_max_size ,单位byte"),
array(0, "ATTACHMAXTOTALSIZE", 's', '2097152', "一篇文章中所有附件的总大小限制, 单位byte"),
array(0, "ATTACHMAXCOUNT", 's', '5', "一篇文章中最多可上载的附件数量"),
"自定义头像相关",
array(0, "USER_FACE", 'b', true, "是否允许自定义头像"),
array(0, "MYFACEDIR", 's', "uploadFace/", "自定义头像上传保存位置"),
array(0, "MYFACEMAXSIZE", 's', "524288", "自定义头像最大文件大小, 单位byte"),
"数据库相关",
array(0, "DB_ENABLED", 'b', false, "是否启用数据库支持，目前仅用于小字报"),
array(1, "dbhost", 's', "localhost", "数据库服务器地址"),
array(1, "dbuser", 's', "", "数据库服务器用户"),
array(1, "dbpasswd", 's', "", "数据库服务器密码"),
array(1, "dbname", 's', "", "数据库表"),
"其他附加功能",
array(0, "SUPPORT_TEX", 'b', false, "是否支持动态 tex2mathml 转换"),
array(0, "AUTO_KICK", 'b', true, "一个用户登录过多时是自动（是）还是提示（否）踢出以前的登录"),
array(0, "SHOWTELNETPARAM", 'b', true, "是否允许配置 telnet 下专用的个人参数"),
array(0, "ALLOWMULTIQUERY", 'b', false, "是否允许一般用户进行全站/多版面查询"),
array(0, "ALLOW_SYSOP_MULTIQUERY", 'b', false, "是否允许管理员进行全站/多版面查询"),
array(0, "ALLOW_SELF_MULTIQUERY", 'b', false, "是否允许全站查询自己发表的文章"),
array(0, "RSS_SUPPORT", 'b', true, "是否允许 RSS"),
array(0, "ONBOARD_USERS", 'b', false, "是否允许显示版面在线用户"),
array(0, "SMS_SUPPORT", 'b', false, "是否允许手机短信"),
array(0, "AUDIO_CHAT", 'b', false, "是否显示语音聊天室的 link"),
false
);

/* site.php 没有定义过的参数都定义一下 */
$ccc = count($site_defines);
for ($i = 0; $i < $ccc; $i++) {
	if ($site_defines[$i] === false) break;
	if (is_string($site_defines[$i])) continue;
	
	$default = $site_defines[$i][3];
	$cur_var = $site_defines[$i][1];
	if ($site_defines[$i][0] == 0) {
		if (!defined($cur_var)) define($cur_var, $default);
	} else {
		if (!isset($$cur_var)) $$cur_var = $default;
	}
}




/* 还有一些遗留下来的可配置参数，以后看情况要不要放入 site_defines */
$HTMLCharset = "GB2312"; //这个我觉得肯定是 gb2312 吧，以后要变的话再做成可自定义的参数

if (!defined('COOKIE_PREFIX')) { //cookie名称的前缀
    define('COOKIE_PREFIX', "W_");
}
if (!defined('COOKIE_PATH')) {
    define('COOKIE_PATH', "");
}
if (!defined('COOKIE_DOMAIN')) {
    define('COOKIE_DOMAIN', "");
}
if (!defined('QUOTE_LEV')) {
	define('QUOTE_LEV', BBS_QUOTE_LEV);
}
if (!defined('QUOTED_LINES')) {
	define('QUOTED_LINES', BBS_QUOTED_LINES);
}

if (!isset($HTMLTitle)) {
	$HTMLTitle = $SiteName;
}

if (!defined('USEBROWSCAP')) { //是否使用 browscap 函数来更准确地判断浏览器和操作系统类型，需要配置 PHP - atppp
	define('USEBROWSCAP',0); //默认 OFF. 设置成 ON 虽然会准确判断出非 IE 浏览器，但是会大大降低出首页的速度，慎用！
}

$sectionCount=count($section_names);

if (!isset($DEFAULTStyle)) {
	$DEFAULTStyle="defaultstyle";  //默认CSS风格，如果要改动千万要确认该CSS存在。
}


/*
 * 用户自定义参数
 *
 * 格式：每项五个参数，第一个 0 表示 telnet 下专用参数；第二个是参数名称；
 *                     第三个是参数具体解释；第四五是参数 ON 和 OFF 所代表的具体含义
 */
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
	    array(0, "离站後寄回所有讯息","您退出登录后是否把短信息记录发回您的信箱？", "是", "否"),       /* DEF_MAILMSG */
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
} else $user_define_default = 0;

if (!isset($user_define1)) {
	$user_define1=array(array(1,"隐藏 IP", "是否发文和被查询的时候隐藏自己的 IP 信息","隐藏","不隐藏") /* DEF_HIDEIP */
	);
} else $user_define1_default = 0;

if (!isset($mailbox_prop)) {
	$mailbox_prop=array(array(1,"发信时保存信件到发件箱", "是否发信时自动选择保存到发件箱","保存","不保存"),
		array(1,"删除信件时不保存到垃圾箱", "是否删除信件时不保存到垃圾箱","不保存","保存"),
		array(0,"快捷邮箱键", "telnet方式下版面按 'v' 时进入什么界面？","信箱主界面","收件箱")
	);
} else $mailbox_prop_default = 0;
?>
