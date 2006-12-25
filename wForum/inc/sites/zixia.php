<?php

$SiteName="大话西游 紫霞BBS";

$SiteURL = "http://bbs.zixia.net/";

$Banner=$SiteURL . "bar/bar.jpg";

define('OLD_REPLY_STYLE', true); //是否使用传统 telnet re 文方式

define('SMS_SUPPORT', true);

define('AUDIO_CHAT', true);

// by zixia 2006-12-24 mod_rewrite url support
define('URL_REWRITE', false);

//define ("MAINTITLE","<a href='http://www.ecallchina.com/free.asp' target='_blank'><img src=\"bar/free-phone-card.gif\" border=0/></a>");

define ("MAINTITLE","<script type='text/javascript'><!--
google_ad_client = 'pub-8383497624729613';
google_ad_width = 728;
google_ad_height = 90;
google_ad_format = '728x90_as';
google_ad_type = 'image';
google_ad_channel = '9403601813';
//--></script>
<script type='text/javascript'
  src='http://pagead2.googlesyndication.com/pagead/show_ads.js'>
</script>
");
//define ("MAINTITLE","<img src=\"bar/title.jpg\"/>");

define("ATTACHMAXSIZE","512000");
define ("ATTACHMAXTOTALSIZE","1024000");
define("ATTACHMAXCOUNT","20");

define('COOKIE_PREFIX', "");

define('MYFACEMAXSIZE','102400');

define('ALLOW_SELF_MULTIQUERY', true);
define('SUPPORT_TEX', true);
define("ONBOARD_USERS", true);

define('SHOW_POST_UNREAD', false);
define('ARTICLE_USE_JS', false);

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

$AdSenseSquare = 
'
<div style="float:right">
<script type="text/javascript"><!--
google_ad_client = "pub-8383497624729613";
google_ad_width = 250;
google_ad_height = 250;
google_ad_format = "250x250_as";
google_ad_type = "text_image";
//2006-12-08: wForum Square
google_ad_channel = "1680473580";
//--></script>
<script type="text/javascript"
  src="http://pagead2.googlesyndication.com/pagead/show_ads.js">
</script>
</div>
';

$AdSenseTextLink = 
'
<p>
<script type="text/javascript"><!--
google_ad_client = "pub-8383497624729613";
google_ad_width = 728;
google_ad_height = 15;
google_ad_format = "728x15_0ads_al";
//2006-12-08: wForum Article
google_ad_channel = "5432286314";
//--></script>
<script type="text/javascript"
  src="http://pagead2.googlesyndication.com/pagead/show_ads.js">
</script>
';


$AdSenseSearchForm = 
'
<!-- SiteSearch Google -->
<form method="get" action="http://www.google.com/custom" target="_blank" style="display:inline">
<input type="hidden" name="domains" value="bbs.zixia.net;zixia.net"></input>
<label for="sbi" style="display: none">输入您的搜索字词</label>
<label for="sbb" style="display: none">提交搜索表单</label>
<input type="radio" name="sitesearch" value="" checked id="ss0"></input>
<label for="ss0" title="搜索网络"><font size="-1" color="#000000">Web</font></label>
<input type="radio" name="sitesearch" value="bbs.zixia.net" id="ss1"></input>
<label for="ss1" title="搜索 bbs.zixia.net"><font size="-1" color="#000000">bbs.zixia.net</font></label>
<input type="radio" name="sitesearch" value="zixia.net" id="ss2"></input>
<label for="ss2" title="搜索 zixia.net"><font size="-1" color="#000000">zixia.net</font></label>
<input type="text" name="q" size="31" maxlength="255" value="" id="sbi" style="line-height:15px" ></input>
<input type="submit" name="sa" value="Google 搜索" id="sbb"></input>
<input type="hidden" name="client" value="pub-8383497624729613"></input>
<input type="hidden" name="forid" value="1"></input>
<input type="hidden" name="channel" value="2588366270"></input>
<input type="hidden" name="ie" value="GB2312"></input>
<input type="hidden" name="oe" value="GB2312"></input>
<input type="hidden" name="flav" value="0000"></input>
<input type="hidden" name="sig" value="xg0zd23UfL8iXnDA"></input>
<input type="hidden" name="cof" value="GALT:#008000;GL:1;DIV:#336699;VLC:663399;AH:center;BGC:FFFFFF;LBGC:ffffff;ALC:0000FF;LC:0000FF;T:000000;GFNT:0000FF;GIMP:0000FF;LH:50;LW:135;L:http://bbs.zixia.net/images/zixia_logo.gif;S:http://zixia.net;FORID:1"></input>
<input type="hidden" name="hl" value="zh-CN"></input>
</form>
<!-- SiteSearch Google -->
';

$AdSenseFooterBanner = 
'
<table border=0>
<tr>
<td>
<!-- firefox start -->
<script type="text/javascript"><!--
google_ad_client = "pub-8383497624729613";
google_ad_width = 120;
google_ad_height = 60;
google_ad_format = "120x60_as_rimg";
google_cpa_choice = "CAAQve6fnAIaCOVxK8eoGA18KL-_93M";
google_ad_channel = "1238122555";
//--></script>
<script type="text/javascript" src="http://pagead2.googlesyndication.com/pagead/show_ads.js">
</script>
<!-- firefox end -->
<br>
<!-- picasa start -->
<script type="text/javascript"><!--
google_ad_client = "pub-8383497624729613";
google_ad_width = 120;
google_ad_height = 60;
google_ad_format = "120x60_as_rimg";
google_cpa_choice = "CAAQpJ7UlAIaCCGWP6yzH3JiKODy9IMB";
google_ad_channel = "1238122555";
//--></script>
<script type="text/javascript" src="http://pagead2.googlesyndication.com/pagead/show_ads.js">
</script>
<!-- picasa end -->
</td>
<td>

<!-- google analytics start -->
<script type="text/javascript"><!--
google_ad_client = "pub-8383497624729613";
google_ad_width = 728;
google_ad_height = 90;
google_ad_format = "728x90_as";
google_ad_type = "text_image";
//2006-11-25: wForum Bottom Banner
google_ad_channel = "2699445787";
//alert("zixia");
//--></script>
<script type="text/javascript"
  src="http://pagead2.googlesyndication.com/pagead/show_ads.js">
</script>
<!-- google analytics end -->

</td>
<td>

<!-- adwords start -->
<script type="text/javascript"><!--
google_ad_client = "pub-8383497624729613";
google_ad_width = 120;
google_ad_height = 60;
google_ad_format = "120x60_as_rimg";
google_cpa_choice = "CAAQyP38hwIaCGteZD4nAZc6KITT6n4";
google_ad_channel = "1238122555";
//--></script>
<script type="text/javascript" src="http://pagead2.googlesyndication.com/pagead/show_ads.js">
</script>
<!-- adwords end -->

<br>

<!-- adsense sart -->
<script type="text/javascript"><!--
google_ad_client = "pub-8383497624729613";
google_ad_width = 120;
google_ad_height = 60;
google_ad_format = "120x60_as_rimg";
google_cpa_choice = "CAAQ1e-ujAIaCG0-GTONE76KKLHM93M";
google_ad_channel = "1238122555";
//--></script>
<script type="text/javascript" src="http://pagead2.googlesyndication.com/pagead/show_ads.js">
</script>
<!-- adsense end -->
</td>
</tr>
</table>

';


$FooterBan = 

'<script src="http://www.google-analytics.com/urchin.js" type="text/javascript"> </script>
 <script type="text/javascript"> _uacct = "UA-287835-3"; urchinTracker(); </script>
'

. 

$AdSenseFooterBanner

;

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
