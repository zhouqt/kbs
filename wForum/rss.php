<?php

$needlogin = 0;
$nocookie = 1;

require("inc/funcs.php");
require("inc/board.inc.php");
require("inc/rss.inc.php");

global $boardArr;
global $boardID;
global $boardName;

header("Content-Type: text/xml; charset=$HTMLCharset");
if (!RSS_SUPPORT) exit;

preprocess();
main($boardID, $boardName, $boardArr, $modifytime);

function preprocess(){
	global $boardID;
	global $boardName;
	global $boardArr;
	global $loginok;
	global $currentuser;
	global $modifytime;
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
	if (!$loginok) bbs_setguest_nologin();
	if (!($isnormalboard = bbs_normalboard($boardName))) {
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
	$filename = bbs_get_board_filename($boardName, ".ORIGIN");
	$modifytime = @filemtime($filename);
	if ($isnormalboard) {
		if (cache_header("public",$modifytime,300)) {
			exit();
		}
	}
	return true;
}

function main($boardID, $boardName, $boardArr, $modifytime) {
	$includeDesc = isset($_GET["includeContents"]);
	$lw = isset($_GET["lw"]);
	$channel = generate_rss_header($boardName, htmlspecialchars($boardArr["DESC"], ENT_QUOTES), $modifytime);
	$items = generate_rss_contents($boardID, $boardName, $includeDesc, $lw);
	echo generate_rss($channel, $items);
}

function generate_rss_header($boardName, $htmlboardDesc, $modifytime) {
	global $SiteURL;
	$re = array();
	$re["title"] = $htmlboardDesc;
	$re["link"] = $SiteURL."board.php?name=".$boardName;
	$re["description"] = "$htmlboardDesc 版面主题索引";
	$re["language"] = "zh-cn";
	$re["generator"] = "wForum RSS Generator";
	if ($modifytime > 0) {
		$re["lastBuildDate"] = gmdate("D, d M Y H:i:s", $modifytime) . " GMT";
	}
	return $re;
}

function generate_rss_contents($boardID, $boardName, $includeDesc, $lw) {
	global $SiteURL;
	global $dir_modes;
	$contents = "";
	$maxArticles = 20;
	$dir_mode = $dir_modes["ORIGIN"];
	$total = bbs_countarticles($boardID, $dir_mode);
	$re = array();
	if ($total > 0) {
		if ($total < $maxArticles) $maxArticles = $total;
		$articles = bbs_getarticles($boardName, $total - $maxArticles + 1, $maxArticles, $dir_mode);
		$cc = count($articles);
		for ($i = count($articles) - 1; $i >= 0; $i--) {
			$origin = $articles[$i];
			$item = array();
			$item["title"] = htmlspecialchars($origin['TITLE'], ENT_QUOTES);
			if ($lw) {
				$item["link"] = $SiteURL."bbscon.php?bid=".$boardID."&amp;id=".$origin['ID']."&amp;lw=1";
			} else {
				$item["link"] = $SiteURL."disparticle.php?boardName=".$boardName."&amp;ID=".$origin['ID'];
			}
			$item["author"] = $origin['OWNER'];
			$item["pubDate"] = gmdate("D, d M Y H:i:s", $origin['POSTTIME']) . " GMT";
			$item["guid"] = $SiteURL."bbscon.php?bid=".$boardID."&amp;id=".$origin['ID'];
			$item["comments"] = $SiteURL."disparticle.php?boardName=".$boardname."&amp;ID=".$origin['ID'];
			if ($includeDesc) {
				$filename = bbs_get_board_filename($boardName, $origin["FILENAME"]);
				$contents = bbs_printansifile($filename,1,'bbscon.php?bid='.$boardID.'&amp;id='.$origin['ID'], 0, 0);
				$item["description"] = htmlspecialchars($contents, ENT_QUOTES);
			}
			$re[] = $item;
		}
	}
	return $re;
}
?>
