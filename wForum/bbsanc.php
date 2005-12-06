<?php
	/* telnet link */
	require("inc/funcs.php");
	
	if (isset($_GET["path"])) {
		$filename = $_GET["path"];
	} else if (isset($_GET["p"])) {
		$numpath = $_GET["p"];
	
		$brdarr = array();
		$bid = $numpath;
		settype($bid,"integer");
		if( $bid == 0 ) die;
	
		$board = bbs_getbname($bid);
		if( !$board ) die;

		if( $bid != bbs_getboard($board, $brdarr) ) die;
	
		$board = $brdarr['NAME'];
	
		$filename = bbs_ann_num2path($numpath,$currentuser["userid"]);
		if($filename==false) die;
	
		if(! file_exists($filename) || is_dir($filename) ) die;
		
		$filename = substr($filename, strlen("0Announce/"));
	} else die;
	$url = "elite.php?file=".$filename;
	if (isset($_GET["ap"])) $url .= "&ap=".$_GET["ap"];
	header("Location: " . $url);
?>