<?php

$SiteName="大话西游 紫霞BBS";

$SiteURL = "http://wforum.zixia.net/";

$Banner="bar/bar.jpg";

define('OLD_REPLY_STYLE', true); //是否使用传统 telnet re 文方式

define('SMS_SUPPORT', true);

define('AUDIO_CHAT', true);

define ("MAINTITLE","<img src=\"bar/title.jpg\"/>");

define("ATTACHMAXSIZE","512000");
define ("ATTACHMAXTOTALSIZE","1024000");
define("ATTACHMAXCOUNT","20");

define('MYFACEMAXSIZE','102400');

define('ALLOW_SELF_MULTIQUERY', true);
define('SUPPORT_TEX', true);
define("ONBOARD_USERS", true);

$section_nums = array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9");
$section_names = array(
    array("斧 头 帮", "[黑帮/系统]"),
    array("大话西游", "[大话/技术]"),
    array("大小分舵", "[地盘/校班]"),
    array("吃喝玩乐", "[美食/休闲]"),
    array("游戏人生", "[游戏/娱乐]"),
    array("吟风弄月", "[感性/文化]"),
    array("西游之路", "[出国/聚会]"),
    array("豆腐西施", "[店面/培训]"),
    array("东城西就", "[省份/城市]"),
    array("大圣取经", "[专业/申请]") 
);

define("DB_ENABLED", true);
require "dbconn.php";
require "default.php";

function init_zixia_boards()
{
        define ( ZIXIA_BOARD_FILE, "/bbs/www/zixia/wForum/zixia.boards" );

        global $zixia_boards_list;
        $zixia_boards_list = array();

        if (($fd = fopen(ZIXIA_BOARD_FILE, "r")) === false) {
                fclose($fout);
                return false;
        }
        $total = 0;

        while (!feof($fd)) {
                $line = trim(fgets($fd, 4096));
//echo "$line<br>\n";
                if (strlen($line) == 0 || $line{0} == "#") continue;
                $bn_info = preg_split("/\s+/", $line);
//print_r($bn_info);
                $bn_e = strtolower($bn_info[0]);
                $bn_c = $bn_info[1];
                $zixia_boards_list[$bn_e] = $bn_c;
//print_r($zixia_boards_list);
//echo "<hr>\n";
        }
        fclose($fd);

//print_r($zixia_boards_list);
	return true;
}

function get_zixia_board_info( $bn )
{
	global $zixia_boards_list;

	$bn = strtolower($bn);
	if ( array_key_exists($bn, $zixia_boards_list) ){
		if ( strlen($zixia_boards_list[$bn])){
			return $zixia_boards_list[$bn];
		}else{
			return true;
		}
	}

	return false;
}



?>
