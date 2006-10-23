<?php
	require("www2-funcs.php");
	require("www2-board.php");
	require("www2-rss.php");
	
	login_init(FALSE, TRUE);

	if(isset($_GET["h"])) 
		$h = $_GET["h"];
	else
		exit;
	
	$channel = array();
	$items = array();

	$channel["link"] = SiteURL . "frames.html";
	$channel["language"] = "zh-cn";
	$channel["generator"] = "KBS RSS Generator";

		if($h == 2) {
			if(isset($_GET["s"]))
				$s = $_GET["s"];
			else
				exit;
			$xmlfile = BBS_HOME . "/xml/day_sec" . $s . ".xml"; // 分区十大热门话题
		} else if ($h == 1) {
			$xmlfile = BBS_HOME . "/xml/day.xml";               // 今日十大热门话题
		} else {
			exit;
		}
		$modifytime = @filemtime($xmlfile);

		if(cache_header("public", $modifytime, 1800))
			return;
		
		$channel["title"] = BBS_FULL_NAME . " " . (($h == 2)?(constant("BBS_SECNAME" . $s . "_0") . "区"):"今日") . "十大热门话题";
		$channel["description"] = BBS_FULL_NAME . date("Y年m月d日", $modifytime);
		if($h == 2)
			$channel["description"] .= constant("BBS_SECNAME" . $s . "_0") . "区";
		$channel["description"] .= "十大热门话题";
		$channel["lastBuildDate"] = gmt_date_format(time());
		$doc = domxml_open_file($xmlfile);
		if(!$doc)
			return;
		$root = $doc->document_element();
		$boards = $root->child_nodes();
		$brdarr = array();
		$item = array();
		while($board = array_shift($boards)) {
			if($board->node_type() == XML_TEXT_NODE)
				continue;
			$brdname = find_content($board, "board");
			$brdnum = bbs_getboard($brdname, $brdarr);
			if($brdnum == 0)
				continue;
			$brdname = urlencode($brdarr["NAME"]);
			$gid = find_content($board, "groupid");
			$item["title"] = htmlspecialchars("[" . $brdarr["DESC"] . "] " . find_content($board, "title"));
			$item["link"] = SiteURL . "bbstcon.php?board=" . $brdname . "&amp;gid=" . $gid;
			$item["author"] = find_content($board, "author");
			$item["pubDate"] = gmt_date_format(find_content($board, "time"));
			$item["guid"] = $item["link"];
			
			$articles = array ();
			$num = bbs_get_records_from_id($brdname, $gid, $dir_modes["NORMAL"], $articles);
			if ($num <= 0)
				continue;
			$article = $articles[1];
			if(!$article)
				continue;
			$contentfile = bbs_get_board_filename($brdname, $article["FILENAME"]);
			$content = bbs2_readfile_text($contentfile, DESC_CHAR_PER_RSS_ITEM, 0);
			if(is_string($content))
				$item["description"] = "<![CDATA[" . $content . "]]>";
			else
				continue;
			$items[] = $item;
		}

	$channel["lastBuildDate"] = gmt_date_format($modifytime);

	output_rss($channel, $items);

?>
