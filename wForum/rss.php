<?php
/* 这个只支持文摘区和原作区！ */
$needlogin = 0;

require("inc/funcs.php");
require("inc/board.inc.php");
require("inc/rss.inc.php");

global $boardArr;
global $boardID;
global $boardName;
global $ftype;

if (!RSS_SUPPORT) exit;

preprocess();
main($boardID, $boardName, $boardArr, $modifytime);

function preprocess(){
	global $boardID;
	global $boardName;
	global $boardArr;
	global $loginok;
	global $guestloginok;
	global $currentuser;
	global $dir_modes;
	global $modifytime;
	global $ftype;
	if (isset($_GET['ftype'])) {
		$ftype = @intval($_GET['ftype']);
	} else {
		$ftype = $dir_modes["ORIGIN"];
	}
	if ($ftype == $dir_modes["DIGEST"]) {
		$indexFile = ".DIGEST";
	} else if ($ftype == $dir_modes["ORIGIN"]) {
		$indexFile = ".ORIGIN";
	} else {
		exit;
	}

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
	if (!$loginok && !$guestloginok) bbs_setguest_nologin();
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
	$filename = bbs_get_board_filename($boardName, $indexFile);
	$modifytime = @filemtime($filename);
	if ($isnormalboard) {
		if (cache_header("public",$modifytime,300)) {
			exit;
		}
	}
	return true;
}

function main($boardID, $boardName, $boardArr, $modifytime) {
	global $HTMLCharset;
	$includeDesc = isset($_GET["ic"]);
	$lw = isset($_GET["lw"]);
	$channel = generate_rss_header($boardName, htmlspecialchars($boardArr["DESC"], ENT_QUOTES), $modifytime);
	$items = generate_rss_contents($boardID, $boardName, $includeDesc, $lw);
	header("Content-Type: text/xml; charset=$HTMLCharset");
	header("Content-Disposition: inline;filename=rss.xml");
	echo generate_rss($channel, $items);
}

function generate_rss_header($boardName, $htmlboardDesc, $modifytime) {
	global $SiteURL;
	global $ftype;
	global $dir_modes;
	$re = array();
	if ($ftype == $dir_modes["ORIGIN"]) {
		$re["title"] = $htmlboardDesc;
		$re["description"] = "$htmlboardDesc 版面主题索引";
	} else {
		$re["title"] = "$htmlboardDesc 文摘区";
		$re["description"] = "$htmlboardDesc 文摘区索引";
	}
	$re["link"] = $SiteURL."board.php?name=".$boardName;
	$re["language"] = "zh-cn";
	$re["generator"] = "wForum RSS Generator";
	if ($modifytime > 0) {
		$re["lastBuildDate"] = gmdate("D, d M Y H:i:s", $modifytime) . " GMT";
	}
	return $re;
}

function generate_rss_contents($boardID, $boardName, $includeDesc, $lw) {
	global $SiteURL;
	global $ftype;
	global $dir_modes;
	$contents = "";
	$maxArticles = 20;
	$total = bbs_countarticles($boardID, $ftype);
	$re = array();
	if ($total > 0) {
		if ($total < $maxArticles) $maxArticles = $total;
		$start = $total - $maxArticles + 1;
		$articles = bbs_getarticles($boardName, $start, $maxArticles, $ftype);
		$cc = count($articles);
		for ($i = count($articles) - 1; $i >= 0; $i--) {
			$origin = $articles[$i];
			$item = array();
			$item["title"] = htmlspecialchars($origin['TITLE'], ENT_QUOTES)." ";
			if ($ftype != $dir_modes["ORIGIN"]) {
				$conurl .= "boardcon.php?bid=".$boardID."&amp;id=".$origin['ID']."&amp;num=".($i+$start)."&amp;ftype=".$ftype;
			} else {
				$conurl = "bbscon.php?bid=".$boardID."&amp;id=".$origin['ID'];
			}
			if ($lw) {
				$item["link"] = $SiteURL.$conurl."&amp;lw=1";
			} else {
				if ($ftype != $dir_modes["ORIGIN"]) {
					$item["link"] = $SiteURL.$conurl;
				} else {
					$item["link"] = $SiteURL."disparticle.php?boardName=".$boardName."&amp;ID=".$origin['ID'];
				}
			}
			$item["author"] = $origin['OWNER'];
			$item["pubDate"] = gmdate("D, d M Y H:i:s", $origin['POSTTIME']) . " GMT";
			$item["guid"] = $SiteURL."bbscon.php?bid=".$boardID."&amp;id=".$origin['ID'];
			$item["comments"] = $SiteURL."disparticle.php?boardName=".$boardname."&amp;ID=".$origin['ID'];
			if ($includeDesc) {
				$filename = bbs_get_board_filename($boardName, $origin["FILENAME"]);
				$contents = bbs_printansifile($filename,1,$conurl, 0, 0);
				$item["description"] = htmlspecialchars($contents, ENT_QUOTES);
			}
			$re[] = $item;
		}
	}
	return $re;
}
?>
