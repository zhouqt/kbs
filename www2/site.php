<?php
define("www2dev", true);
define("ATTACHMAXSIZE",BBS_MAXATTACHMENTSIZE);        //附件总字节数的上限，单位 bytes
define("ATTACHMAXCOUNT",BBS_MAXATTACHMENTCOUNT);      //附件数目的上限
define("MAINPAGE_FILE","mainpage.php");              //首页导读的 URL
define("QUOTED_LINES", BBS_QUOTED_LINES);             //web 回文保留的引文行数
define("SITE_NEWSMTH", 1);
define("RUNNINGTIME", 1);                             //底部显示页面运行时间
define("AUTO_BMP2PNG_THRESHOLD", 100000); // requires ImageMagick and safe_mode off
define("HAVE_PC", 1);

if (!defined("BBS_HAVE_BLOG"))
	define("BBS_HAVE_BLOG", 1); // pig2532 ymsw

// web 前端有 squid 或者 apache 的 mod_proxy 等代理的时候请打开这个选项
//define("CHECK_X_FORWARDED_FOR", 1);


// 界面方案的名称
$style_names = array(
	"蓝色经典",
	"冬雪皑皑",
	"猪猪猪的泡泡"
);

// 这个暂时这样.............
define("BBS_NICKNAME", "水木");

// 权限名称
$permstrings = array(
    "基本权力",
    "进入聊天室",
    "呼叫他人聊天",
    "发表文章",
    "使用者资料正确",
    "实习站务",
    "可隐身",
    "可见隐身",
    "长期帐号",
    "编辑系统档案",
    "板主",
    "帐号管理员",
    "水木清华智囊团",
    "封禁娱乐权限",
    "系统维护管理员",
    "Read/Post 限制",
    "精华区总管",
    "讨论区总管",
    "活动看版总管",
    "不能 ZAP(讨论区专用)",
    "聊天室OP(元老院专用)",
    "系统总管理员",
    "荣誉帐号",
    "特殊权限 5",
    "仲裁委员",
    "特殊权限 7",
    "自杀进行中",
    "集体专用帐号",
    "看系统讨论版",
    "封禁Mail"
);

?>
