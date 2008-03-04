<?php
if (!defined('_BBS_WWW2_RSS_PHP_'))
{
define('_BBS_WWW2_RSS_PHP_', 1);

define('RSS_ENCODING', "UTF-8");

define('ARTICLE_PER_RSS', 20);
define('DESC_CHAR_PER_RSS_ITEM', 2048);

function cv($c) {
	return iconv("GB18030", "UTF-8//IGNORE", $c);
}

if (!defined("BBS_WEBDOMAIN")) { // 这个可以在 site.php 里面定义以提高效率
	define("BBS_WEBDOMAIN", bbs_get_webdomain());
}
define('SiteURL', "http://" . BBS_WEBDOMAIN . "/");


function generate_rss($channel, $items) {
	$str = "<?xml version=\"1.0\" encoding=\"" . RSS_ENCODING . "\" ?>\n<rss version=\"2.0\">\n\t<channel>\n";
	foreach ($channel as $key => $value) {
		$str .= "\t\t<$key>".cv($value)."</$key>\n";
	}
	foreach ($items as $item) {
		$str .= "\t\t<item>\n";
		foreach ($item as $key => $value) {
			$str .= "\t\t\t<$key>".cv($value)."</$key>\n";
		}
		$str .= "\t\t</item>\n";
	}
	$str .= "\t</channel>\n</rss>\n";
	return $str;
}

function output_rss($channel, $items) {
	header("Content-Type: text/xml; charset=" . RSS_ENCODING);
	echo generate_rss($channel, $items);
}

} // !define ('_BBS_WWW2_RSS_PHP_')
?>
