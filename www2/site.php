<?php
//define("www2dev", "DEFINED");
define("MAINPAGE_FILE","mainpage.php");              //首页导读的 URL
define("RUNNINGTIME", 1);                             //底部显示页面运行时间
define("AUTO_BMP2PNG_THRESHOLD", 100000); // requires ImageMagick and safe_mode off
//define("ENABLE_JSMATH", 1);

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

?>
