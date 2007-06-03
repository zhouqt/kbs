<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_READING);
	$brdarr = array();
	if( isset( $_GET["bid"] ) ){
		$brdnum = $_GET["bid"] ;
		settype($brdnum,"integer");
		if( $brdnum == 0 ){
			html_error_quit("错误的讨论区!");
		}
		$board = bbs_getbname($brdnum);
		if( !$board ){
			html_error_quit("错误的讨论区");
		}
		if( $brdnum != bbs_getboard($board, $brdarr) ){
			html_error_quit("错误的讨论区");
		}
	} else {
		html_error_quit("错误的讨论区");
	}
	$isnormalboard=bbs_normalboard($board);
	bbs_set_onboard($brdnum,1);

	$usernum = $currentuser["index"];

	if (!$isnormalboard && bbs_checkreadperm($usernum, $brdnum) == 0) {
		html_error_quit("错误的讨论区");
	}
	if (isset($_GET["id"]))
		$id = $_GET["id"];
	else {
		html_error_quit("错误的文章号");
	}
	settype($id, "integer");
	
	$indexModify = @filemtime(bbs_get_board_index($board, $dir_modes["NORMAL"]));

	if (isset($_GET["ftype"])){
		$ftype = intval($_GET["ftype"]);
	} else {
		$ftype = $dir_modes["NORMAL"];
	}
	$dir_perm = bbs_is_permit_mode($ftype, 1);
	if (!$dir_perm) {
		html_error_quit("错误的模式");
	}

	if(($ftype == $dir_modes["DELETED"]) && (!bbs_is_bm($brdnum, $usernum)))
	{
		html_error_quit("你不能看这个东西哟。");
	}
	
	$total = bbs_countarticles($brdnum, $ftype);
	if ($total <= 0) {
		html_error_quit("错误的文章号,原文可能已经被删除");
	}
	$articles = array ();

    if ($dir_perm == 1) { //sorted
        $articles = array ();
        $num = bbs_get_records_from_id($brdarr["NAME"], $id, $ftype, $articles);
        if ($num <= 0) html_error_quit("错误的文章号,原文可能已经被删除<script>clearArticleDiv(".$id.");</script>");
        if ($ftype == $dir_modes["ZHIDING"]) $num = 0; // for caching the same url
        $article = $articles[1];
    } else {
        $num = @intval($_GET["num"]);
        if (($num <= 0) || ($num > $total)) html_error_quit("错误的文章号,原文可能已经被删除");
        if (($articles = bbs_getarticles($brdarr["NAME"], $num, 1, $ftype)) === false) html_error_quit("错误的文章号,原文可能已经被删除");
        if ($id != $articles[0]["ID"]) html_error_quit("错误的文章号,原文可能已经被删除");
        $article = $articles[0];
    }

	$filename = bbs_get_board_filename($board, $article["FILENAME"]);
	if ($isnormalboard && ($ftype != $dir_modes["DELETED"])) {
		if (cache_header("public",@filemtime($filename),300)) return;
		$cacheable = true;
	} else {
		$cacheable = false;
	}

    $fp = fopen($filename, "r");
    if(!$fp)
        exit;
    for($i=0; $i<4; $i++)
        $line = fgets($fp, 1024);
    while(!feof($fp))
        print(fread($fp, 1024));
    fclose($fp);

	if ($ftype==0)
	    bbs_brcaddread($brdarr["NAME"], $articles[1]["ID"]);
?>
