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
	}
	elseif (isset($_GET["board"])){
		$board = $_GET["board"];
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0) {
			html_error_quit("错误的讨论区");
		}
	}
	elseif (isset($_SERVER['argv'])){
		$board = $_SERVER['argv'][1];
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0) {
			html_error_quit("错误的讨论区");
		}
	}
	else {
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
	elseif (isset($_SERVER['argv'][2]))
		$id = $_SERVER['argv'][2];
	else {
		html_error_quit("错误的文章号");
	}
	settype($id, "integer");

	if (isset($_GET["ftype"])){
		$ftype = intval($_GET["ftype"]);
	} else {
		$ftype = $dir_modes["NORMAL"];
	}
	
	$previd = $nextid = $tprevid = $tnextid = $id;
	// 获取同主题上下篇的ID号
	if($ftype == $dir_modes["NORMAL"])
	{
		$articles = bbs_get_threads_from_id($brdnum, $id, $dir_modes["NORMAL"],1);
		if ($articles == FALSE)
			$tnextid = $id;
		else
			$tnextid = $articles[0]["ID"];
		$articles = bbs_get_threads_from_id($brdnum, $id, $dir_modes["NORMAL"],-1);
		if ($articles == FALSE)
			$tprevid = $id;
		else
			$tprevid = $articles[0]["ID"];
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
        $article = $articles[1];
	
	// 获取上下篇文章的ID号
	if($ftype == $dir_modes["NORMAL"])
	{
		$previd = ($articles[0]["ID"]==0) ? $id : $articles[0]["ID"];
		$nextid = ($articles[2]["ID"]==0) ? $id : $articles[2]["ID"];
	}

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
	}

	@$attachpos=$_GET["ap"];//pointer to the size after ATTACHMENT PAD
	if ($attachpos!=0) {
		require_once("attachment.php");
		output_attachment($filename, $attachpos);
		exit;
	}

	page_header("阅读文章".$dir_name[$ftype], "<a href=\"bbsdoc.php?board=".$brdarr["NAME"]."\">".htmlspecialchars($brdarr["DESC"])."</a>");
?>
<h1><?php echo $brdarr["NAME"]; ?> 版 <?php echo $dir_name[$ftype]; ?></h1>
<script type="text/javascript"><!--
<?php
	$ret = "";
	$ret .= "var o = new conWriter({$ftype},'" . addslashes($brdarr["NAME"]) . "',{$brdnum},";
	$ret .= "{$article["ID"]},{$article["GROUPID"]},{$article["REID"]},'{$article["FILENAME"]}','";
	$ret .= addslashes(bbs_get_super_fav($article["TITLE"], "bbscon.php?bid={$brdnum}&id={$article["ID"]}"));
	$ret .= "',{$num},{$previd},{$nextid},{$tprevid},{$tnextid});";
	echo $ret;
?>
o.h(1);
attachURL = 'bbscon.php?<?php echo $_SERVER["QUERY_STRING"]; ?>';
<?php $s = bbs2_readfile($filename); if (is_string($s)) echo $s; ?>
o.h(0);o.t();
//-->
</script>
<?php
	if (($ftype==0) && ($loginok==1) && ($currentuser["userid"] != "guest"))
		bbs_brcaddread($brdarr["NAME"], $articles[1]["ID"]);
	page_footer();
?>
