<?php

$needlogin=0;
$nocookie = 1;

require("inc/funcs.php");
require("inc/board.inc.php");

global $boardArr;
global $boardID;
global $boardName;

header("Content-Type: text/xml; charset=$HTMLCharset");
if (!RSS_SUPPORT) exit;

preprocess();
main($boardID, $boardName, $boardArr);

function preprocess(){
	global $boardID;
	global $boardName;
	global $boardArr;
	global $loginok;
	global $currentuser;
	if (!isset($_GET['board'])) {
		exit;
	}
	$boardName = $_GET['board'];
	$brdArr = array();
	$boardID = bbs_getboard($boardName, $brdArr);
	$boardArr = $brdArr;
	$boardName = $brdArr['NAME'];
	if ($boardID == 0) {
		exit;
	}
	if (!bbs_normalboard($boardName)) {
		if($loginok == 1) {
			$usernum = $currentuser["index"];
			if (bbs_checkreadperm($usernum, $boardID) == 0) {
				exit;
			}
		} else {
			exit;
		}
	}
	if ($boardArr['FLAG'] & BBS_BOARD_GROUP ) {
		exit;
	}
	return true;
}

function main($boardID, $boardName, $boardArr) {
	$includeDesc = isset($_GET["includeContents"]);
	$rssfile = 	bbs_get_board_filename($boardName, $includeDesc ? ".RSS_FULL" : ".RSS");
	$filename = bbs_get_board_filename($boardName, ".ORIGIN");
	$modifytime = @filemtime($filename);
	$cmt = @filemtime($rssfile);
	if ($modifytime < $cmt) {
		@readfile($rssfile);
	} else {
		$fp = @fopen($rssfile, "w");
		generate_rss_header($boardName, htmlspecialchars($boardArr["DESC"],ENT_QUOTES), $modifytime, $fp);
		generate_rss_contents($boardID, $boardName, $includeDesc, $fp);
		generate_rss_footer($fp);
		if ($fp !== false) {
			fclose($fp);
			@readfile($rssfile);
		}
	}
}

function outputs($fp, $str) {
	if ($fp === false) echo $str;
	else fwrite($fp, $str);
}

function generate_rss_header($boardName, $htmlboardDesc, $modifytime, $fp) {
	global $SiteURL;
	global $HTMLCharset;
	outputs($fp, "<?xml version=\"1.0\" encoding=\"$HTMLCharset\" ?>\n<rss version=\"2.0\">\n");
	outputs($fp, "\t<channel>\n");
	outputs($fp, "\t\t<title>$htmlboardDesc</title>\n");
	outputs($fp, "\t\t<link>".$SiteURL."board.php?name=".$boardName."</link>\n");
	outputs($fp, "\t\t<description>$htmlboardDesc 版面主题索引</description>\n");
	outputs($fp, "\t\t<language>zh-cn</language>\n");
	outputs($fp, "\t\t<generator>wForum RSS Generator</generator>\n");
	if ($modifytime > 0) {
		outputs($fp, "\t\t<lastBuildDate>".gmdate("D, d M Y H:i:s", $modifytime) . " GMT</lastBuildDate>\n");
	}
}

function generate_rss_footer($fp) {
	outputs($fp, "\t</channel>\n</rss>\n");
}

function generate_rss_contents($boardID, $boardName, $includeDesc, $fp) {
	global $SiteURL;
	global $dir_modes;
	$contents = "";
	$maxArticles = 20;
	$dir_mode = $dir_modes["ORIGIN"];
	$total = bbs_countarticles($boardID, $dir_mode);
	if ($total > 0) {
		if ($total < $maxArticles) $maxArticles = $total;
		$articles = bbs_getarticles($boardName, $total - $maxArticles + 1, $maxArticles, $dir_mode);
		$cc = count($articles);
		for ($i = count($articles) - 1; $i >= 0; $i--) {
			$origin = $articles[$i];
			outputs($fp, "\t\t<item>\n");
			outputs($fp, "\t\t\t<title>".htmlspecialchars($origin['TITLE'],ENT_QUOTES)." </title>\n");
			outputs($fp, "\t\t\t<link>".$SiteURL."disparticle.php?boardName=".$boardName."&amp;ID=".$origin['ID']."</link>\n");
			outputs($fp, "\t\t\t<author>".$origin['OWNER']."</author>\n");
			outputs($fp, "\t\t\t<pubDate>".gmdate("D, d M Y H:i:s", $origin['POSTTIME']) . " GMT</pubDate>\n");
			outputs($fp, "\t\t\t<guid>".$SiteURL."bbscon.php?bid=".$boardID."&amp;id=".$origin['ID']."</guid>\n");
			outputs($fp, "\t\t\t<comments>".$SiteURL."disparticle.php?boardName=".$boardname."&amp;ID=".$origin['ID']."</comments>\n");
			if ($includeDesc) {
				$filename = bbs_get_board_filename($boardName, $origin["FILENAME"]);
				$contents = bbs_printansifile($filename,1,'bbscon.php?bid='.$boardID.'&amp;id='.$origin['ID'], 0, 0);
				outputs($fp, "\t\t\t<description>".htmlspecialchars($contents, ENT_QUOTES)." </description>\n");
			}
			outputs($fp, "\t\t</item>\n");
		}
	}
}
?>
