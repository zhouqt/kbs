<?php

function getattachtmppath($userid,$utmpnum)
{
  $attachdir="cache/home/" . strtoupper(substr($userid,0,1)) . "/" . $userid . "/" . $utmpnum . "/upload";
  return $attachdir;

}

define("ANNOUNCENUMBER",5);

define("ARTICLESPERPAGE",30); //目录列表下每页显示的主题数

define("THREADSPERPAGE",10); //文章阅读时每页显示的文章数

$SiteURL="http://172.16.50.79";

$SiteName="大运村BBS";

$HTMLTitle="大运村BBS";

$HTMLCharset="GB2312";

$DEFAULTStyle="defaultstyle";

$Banner="pic/ws.jpg";

$BannerURL="http://172.16.50.79";

define ("MAINTITLE","<IMG SRC=\"bar/title.jpg\">");
define("ATTACHMAXSIZE","5242880");
define ("ATTACHMAXTOTALSIZE","20971520");
define("ATTACHMAXCOUNT","20");

$section_nums = array("0", "1", "2", "3", "4");
$section_names = array(
    array("BBS 系统", "[系统]"),
    array("校园信息", "[信息]"),
    array("电脑技术", "[电脑]"),
    array("学术科学", "[学习]"),
    array("休闲娱乐", "[娱乐]")
);
$sectionCount=count($section_names);

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

$user_define_num=count($user_define);
require "default.php";
?>
