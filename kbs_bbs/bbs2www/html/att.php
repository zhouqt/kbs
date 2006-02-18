<?php
	require("www2-funcs.php");
	login_init();
	$query = $_SERVER["QUERY_STRING"];
	settype($query, "string");
	$av = explode(".", $query);
	$ac = count($av);
	if ($ac == 4 || $ac == 5) {
		$ftype = 0;
		$num = 0;
		$ap = intval($av[3]);
	} else if ($ac == 6 || $ac == 7) {
		$ftype = intval($av[3]);
		$num = intval($av[4]);
		$ap = intval($av[5]);
	} else {
		die;
	}
	if ($ap <= 0) die;
	$bid = intval($av[1]);
	$id = intval($av[2]);
	$brdarr = array();
	$isnormalboard = bbs_safe_getboard($bid, "", $brdarr);
	if (is_null($isnormalboard)) {
		die;
	}
	$board = $brdarr["NAME"];
	$brdnum = $brdarr["BID"];

	$dir_perm = bbs_is_permit_mode($ftype, 1);
	if (!$dir_perm) {
		die;
	}

	$usernum = $currentuser["index"];
	if(($ftype == $dir_modes["DELETED"]) && (!bbs_is_bm($brdnum, $usernum))) {
		die;
	}
	
	$total = bbs_countarticles($brdnum, $ftype);
	if ($total <= 0) {
		die;
	}

	$articles = array ();
    if ($dir_perm == 1) { //sorted
        $articles = array ();
        $num = bbs_get_records_from_id($board, $id, $ftype, $articles);
        if ($num <= 0) die;
        $article = $articles[1];
    } else {
        if (($num <= 0) || ($num > $total)) die;
        if (($articles = bbs_getarticles($board, $num, 1, $ftype)) === false) die;
        if ($id != $articles[0]["ID"]) die;
        $article = $articles[0];
    }

	$filename = bbs_get_board_filename($board, $article["FILENAME"]);
	if ($isnormalboard && ($ftype != $dir_modes["DELETED"])) {
		if (cache_header("public",@filemtime($filename),300)) return;
	}

	bbs_file_output_attachment($filename, $ap);
?>