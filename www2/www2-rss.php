<?php
if (!defined('_BBS_WWW2_RSS_PHP_'))
{
define('_BBS_WWW2_RSS_PHP_', 1);

define('RSS_ENCODING', "UTF-8");
// comment the following three lines if you set RSS_ENCODING gb18030
iconv_set_encoding("internal_encoding", "gb18030");
iconv_set_encoding("output_encoding", "UTF-8//IGNORE");
ob_start("ob_iconv_handler");

define('ARTICLE_PER_RSS', 20);
define('DESC_CHAR_PER_RSS_ITEM', 1000);

/* TODO: consider path */
/*
$domain_name = explode(":",trim($_SERVER["HTTP_HOST"]));
$thispath = dirname($_SERVER['PHP_SELF']);
if (substr($thispath, -1) != "/") $thispath .= "/";
*/
define('SiteURL', "http://" . BBS_WEBDOMAIN . "/");


function generate_rss($channel, $items) {
	$str = "<?xml version=\"1.0\" encoding=\"" . RSS_ENCODING . "\" ?>\n<rss version=\"2.0\">\n\t<channel>\n";
	foreach ($channel as $key => $value) {
		$str .= "\t\t<$key>$value</$key>\n";
	}
	foreach ($items as $item) {
		$str .= "\t\t<item>\n";
		foreach ($item as $key => $value) {
			$str .= "\t\t\t<$key>$value</$key>\n";
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