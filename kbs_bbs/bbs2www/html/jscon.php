<?php
	/**
	 * This file displays article as Javascript to user.
	 * $Id$
	 */
	require("funcs.php");
login_init();

	if( isset( $_GET["bid"] ) )
	{
		$brdnum = $_GET["bid"] ;
		if( $brdnum == 0 ){
			exit;
		}
		$board = bbs_getbname($brdnum);
		if( !$board ){
			exit;
		}
	}
	else {
		exit;
	}
	$isnormalboard=bbs_normalboard($board);
	if (($loginok != 1) && !$isnormalboard) {
		exit;
	}

	if($loginok == 1)
		$usernum = $currentuser["index"];

	if (!$isnormalboard && bbs_checkreadperm($usernum, $brdnum) == 0) {
		exit;
	}
	if (isset($_GET["id"]))
		$id = $_GET["id"];
	else {
		exit;
	}
	settype($id, "integer");

	if (isset($_GET["ftype"])){
		$ftype = $_GET["ftype"];
		if($ftype != $dir_modes["ZHIDING"])
			$ftype = $dir_modes["NORMAL"];
	}
	else
		$ftype = $dir_modes["NORMAL"];
	$total = bbs_countarticles($brdnum, $ftype);
	if ($total <= 0) {
		exit;
	}
	$articles = array ();
	$num = bbs_get_records_from_id($board, $id, $ftype, $articles);
	if ($num == 0)
	{
		exit;
	}
	else
	{
		$filename=bbs_get_board_filename($board, $articles[1]["FILENAME"]);
		if ($isnormalboard) {
       			if (cache_header("public",filemtime($filename),300))
               			return;
               	}
		bbs_print_article_js($filename,1, "/bbscon.php?" . $_SERVER["QUERY_STRING"]);
	}
?>
