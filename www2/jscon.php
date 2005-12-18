<?php
	require("www2-funcs.php");
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
		$ftype = intval($_GET["ftype"]);
	} else {
		$ftype = $dir_modes["NORMAL"];
	}
	$dir_perm = bbs_is_permit_mode($ftype, 2);
	if (!$dir_perm) {
		exit;
	}

	if(($ftype == $dir_modes["DELETED"]) && (!bbs_is_bm($brdnum, $usernum)))
	{
		exit;
	}
	
	$total = bbs_countarticles($brdnum, $ftype);
	if ($total <= 0) {
		exit;
	}
	$articles = array ();

    if ($dir_perm == 1) { //sorted
        $articles = array ();
        $num = bbs_get_records_from_id($board, $id, $ftype, $articles);
        if ($num <= 0) exit;
        $article = $articles[1]["FILENAME"];
    } else {
        $num = @intval($_GET["num"]);
        if (($num <= 0) || ($num > $total)) exit;
        if (($articles = bbs_getarticles($board, $num, 1, $ftype)) === false) exit;
        if ($id != $articles[0]["ID"]) exit;
        $article = $articles[0]["FILENAME"];
    }
	$filename = bbs_get_board_filename($board, $article);
	if ($isnormalboard && ($ftype != $dir_modes["DELETED"])) {
		if (cache_header("public",@filemtime($filename),300))
			return;
	} else {
		cache_header("nocache");
	}
	
	header("Content-Type: text/javascript; charset=gb2312");
	bbs_print_article_js($filename,1, "bbscon.php?" . $_SERVER["QUERY_STRING"]);
?>
