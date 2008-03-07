<?php
require("www2-funcs.php");
require("www2-rss.php");
login_init(FALSE, TRUE);

$query = $_SERVER["QUERY_STRING"];
settype($query, "string");
if (strlen($query) < 2) die;
$type = substr($query, 0, 1);
$board = substr($query, 1);

// 检查用户能否阅读该版
$brdarr = array();
$isnormalboard = bbs_safe_getboard(0, $board, $brdarr);
if (is_null($isnormalboard)) {
	die;
}
if (strcmp($board, $brdarr["NAME"])) die; //cache consideration
$brdnum = $brdarr["BID"];
if ($brdarr["FLAG"]&BBS_BOARD_GROUP) {
	die;
}

/*
 * 内部版面不提供 rss 的理由：
 * 1. rss 软件或 rss 网站无法正确实现 session-based 登录，或者实现了也无意义
 * 2. 内部版面内容流入 rss 软件或 rss 网站并被保存可能造成信息泄露
 * 3. 内部版面 rss 无法在 squid 前端缓存，可能造成潜在负荷问题
 * 4. 内部版面有啥好 rss 的？直接上站看不就完了！
 */
if (!$isnormalboard) die;


if ($type == "g") {
	$ftype = $dir_modes["DIGEST"];
	$title = $desc = "文摘区";
} else if ($type == "m") {
	$ftype = $dir_modes["MARK"];
	$title = $desc = "保留区";
	bbs_checkmark($board);
} else if ($type == "o") {
	$ftype = $dir_modes["ORIGIN"];
	$title = "";
	$desc = "主题";
} else {
	die; //TODO?
}
$dotdirname = bbs_get_board_index($board, $ftype);
$modifytime = @filemtime($dotdirname);

if (cache_header("public",$modifytime,1800))
	return;

$channel = array();
$htmlboardDesc = BBS_FULL_NAME . " " . $board . "/" . htmlspecialchars($brdarr["DESC"], ENT_QUOTES) . " ";
$channel["title"] = $htmlboardDesc . $title;
$channel["description"] = $htmlboardDesc . " 版面" . $desc . "索引";

$channel["link"] = SiteURL."frames.html?mainurl=".urlencode("bbsdoc.php?board=".$board); /* TODO: ftype? */
$channel["language"] = "zh-cn";
$channel["generator"] = "KBS RSS Generator";
$channel["lastBuildDate"] = gmt_date_format($modifytime);

$total = bbs_countarticles($brdnum, $ftype);
$items = array();
if ($total > 0) {
	$artcnt = ARTICLE_PER_RSS;
	if ($total <= $artcnt) {
		$start = 1;
		$artcnt = $total;
	} else {
		$start = ($total - $artcnt + 1);
	}
	($articles = bbs_getarticles($board, $start, $artcnt, $ftype)) or die;
	$cc = count($articles);
	for ($i = $cc - 1; $i >= 0; $i--) {
		$origin = $articles[$i];
		$item = array();
		$item["title"] = htmlspecialchars($origin['TITLE'], ENT_QUOTES) . " ";
		$conurl = "bbscon.php?bid=".$brdnum."&amp;id=".$origin['ID']; 
		//you want the link to always work, so you don't want to add num
		//if ($ftype == $dir_modes["DIGEST"]) $conurl .= "&amp;ftype=".$ftype."&amp;num=".($start+$i);
		$item["link"] = SiteURL.$conurl;
		$item["author"] = $origin['OWNER'];
		$item["pubDate"] = gmt_date_format($origin['POSTTIME']);
		$item["guid"] = $item["link"]; //TODO
		//$item["comments"] = ?? //TODO
		
		$filename = bbs_get_board_filename($board, $origin["FILENAME"]);
		$contents = bbs2_readfile_text($filename, DESC_CHAR_PER_RSS_ITEM, 0);
		if (is_string($contents)) {
			$item["description"] = "<![CDATA[" . $contents . "]]>";
		}
		$items[] = $item;
	}
}

output_rss($channel, $items);
?>
