<?php

$needlogin = 0;

require("inc/funcs.php");
require("inc/rss.inc.php");
require("inc/xml.inc.php");

header("Content-Type: text/xml; charset=$HTMLCharset");
if (!RSS_SUPPORT) exit;

$filename = get_bbsfile("/xml/day.xml");

$modifytime = @filemtime($filename);
if (cache_header("public",$modifytime,300)) {
	exit();
}

$channel = get_channel($modifytime);
$items = get_items($filename);
header("Content-Disposition: inline;filename=rsstopten.xml");
echo generate_rss($channel, $items);

function get_channel() {
	global $SiteURL;
	global $SiteName;
	$re = array();
	$re["title"] = "$SiteName 热门话题";
	$re["link"] = $SiteURL."topten.php";
	$re["description"] = "$SiteName 热门话题";
	$re["language"] = "zh-cn";
	$re["generator"] = "wForum RSS Generator";
	if ($modifytime > 0) {
		$re["lastBuildDate"] = gmdate("D, d M Y H:i:s", $modifytime) . " GMT";
	}
	return $re;
}

function get_items($filename) {
	global $SiteURL;
	$re = array();
	if (!($doc = domxml_open_file($filename))) return $re;

	$root = $doc->document_element();
	$boards = $root->child_nodes();

	$boardArr = array();
	while($board = array_shift($boards))
	{
		if ($board->node_type() == XML_TEXT_NODE)
			continue;
		
		$r_title = find_content($board, "title");
		$r_author = find_content($board, "author");
		$r_board = find_content($board, "board");
		$r_time = find_content($board, "time");
		$r_number = find_content($board, "number");
		$r_groupid = find_content($board, "groupid");
		
		$boardID = bbs_getboard($r_board, $boardArr);
		if ($boardID == 0)
			continue;
		$r_board = $boardArr["NAME"];
		
		$item = array();
		$item["title"] = htmlspecialchars($r_title, ENT_QUOTES)." ";
		$item["link"] = $SiteURL."bbscon.php?bid=".$boardID."&amp;id=".$r_groupid."&amp;lw=1";
		$item["author"] = $r_author;
		$item["pubDate"] = gmdate("D, d M Y H:i:s", $r_time) . " GMT";
		$item["guid"] = $SiteURL."bbscon.php?bid=".$boardID."&amp;id=".$r_groupid;
		$item["comments"] = $SiteURL."disparticle.php?boardName=".$r_board."&amp;ID=".$r_groupid;
		
		$re[] = $item;
	}
	return $re;
}
?>
