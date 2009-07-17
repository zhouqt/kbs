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
	//$isnormalboard=bbs_normalboard($board);
	$lookupuser = array();
	$guestUID = bbs_getuser("guest", $lookupuser);
	$isnormalboard = bbs_checkreadperm($guestUID, $brdnum);
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

	$use_tex = 0;
	if(@$_GET["tex"] == "yes")
		$use_tex = 1;

	$indexModify = @filemtime(bbs_get_board_index($board, $dir_modes["NORMAL"]));

	// 获取上一篇或下一篇，同主题上一篇或下一篇的指示
	@$ptr=$_GET["p"];
	// 同主题的指示在这里处理
	if ($ptr == "tn")
	{
		if ($isnormalboard && cache_header("public",$indexModify,10)) exit;
		
		$articles = bbs_get_threads_from_id($brdnum, $id, $dir_modes["NORMAL"],1);
		if ($articles == FALSE)
			$redirt_id = $id;
		else
			$redirt_id = $articles[0]["ID"];
		bbs_brcaddread($brdarr["NAME"], $redirt_id);
		$url = "Location: " . "bbscon.php?bid=" . $brdnum . "&id=" . $redirt_id;
		if(defined("ENABLE_JSMATH") && $use_tex && $articles[0]["IS_TEX"])
			$url .= "&tex=yes";
		header($url);
		exit;
	}
	elseif ($ptr == "tp")
	{
		if ($isnormalboard && cache_header("public",$indexModify,10)) exit;
		
		$articles = bbs_get_threads_from_id($brdnum, $id, $dir_modes["NORMAL"],-1);
		if ($articles == FALSE)
			$redirt_id = $id;
		else
			$redirt_id = $articles[0]["ID"];
		bbs_brcaddread($brdarr["NAME"], $redirt_id);
		$url = "Location: " . "bbscon.php?bid=" . $brdnum . "&id=" . $redirt_id;
		if(defined("ENABLE_JSMATH") && $use_tex && $articles[0]["IS_TEX"])
			$url .= "&tex=yes";
		header($url);
		exit;
	}

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

	if (!$ftype && $ptr == 'p' && $articles[0]["ID"] != 0) {
		if ($isnormalboard && cache_header("public",$indexModify,10)) exit;
		
		bbs_brcaddread($brdarr["NAME"], $articles[0]["ID"]);
		$url = "Location: " . "bbscon.php?bid=" . $brdnum . "&id=" . $articles[0]["ID"];
		if(defined("ENABLE_JSMATH") && $use_tex && $articles[0]["IS_TEX"])
			$url .= "&tex=yes";
		header($url);
		exit;
	}
	if (!$ftype && $ptr == 'n' && $articles[2]["ID"] != 0)
	{
		if ($isnormalboard && cache_header("public",$indexModify,10)) exit;
		
		bbs_brcaddread($brdarr["NAME"], $articles[2]["ID"]);
		$url = "Location: " . "bbscon.php?bid=" . $brdnum . "&id=" . $articles[2]["ID"];
		if(defined("ENABLE_JSMATH") && $use_tex && $articles[2]["IS_TEX"])
			$url .= "&tex=yes";
		header($url);
		exit;
	}

	$filename = bbs_get_board_filename($board, $article["FILENAME"]);
	if ($isnormalboard && ($ftype != $dir_modes["DELETED"])) {
		if (cache_header("public",@filemtime($filename),300)) return;
		$cacheable = true;
	} else {
		$cacheable = false;
	}

	@$attachpos=$_GET["ap"];//pointer to the size after ATTACHMENT PAD
	if ($attachpos!=0) {
		die; // atppp 20060223
		bbs_file_output_attachment($filename, $attachpos);
		exit;
	}

	$page_title = "<a href=\"bbsdoc.php?board=".$brdarr["NAME"]."\">".htmlspecialchars($brdarr["DESC"])."</a>";
	page_header("阅读文章".$dir_name[$ftype], $page_title);
?>
<script type="text/javascript" src="static/www2-addons.js"></script>
<h1><?php echo $brdarr["NAME"]; ?> 版 <?php echo $dir_name[$ftype]; ?></h1>
<?php
	if(defined("ENABLE_JSMATH") && $article["IS_TEX"] && $use_tex) {
		print("<script src=\"/jsMath/easy/load.js\"></script>");
	}
?>
<script type="text/javascript"><!--
var o = new conWriter(<?php echo $ftype; ?>, '<?php echo addslashes($brdarr["NAME"]); ?>', <?php echo $brdnum; ?>, <?php
echo $article["ID"];?>, <?php echo $article["GROUPID"];?>, <?php echo $article["REID"];?>, '<?php
echo addslashes(bbs_get_super_fav($article['TITLE'], "bbscon.php?bid=" . $brdnum . "&id=" . $article["ID"]));?>', <?php echo $num; ?>, <?php echo $article["IS_TEX"] ? ($use_tex ? "2" : "1") : "0"; ?>);
o.h(1);
att = new attWriter(<?php echo $brdnum; ?>,<?php echo $id; ?>,<?php echo $ftype; ?>,<?php echo $num; ?>,<?php echo ($cacheable?"1":"0"); ?>);
<?php $s = bbs2_readfile($filename); if (is_string($s)) echo $s; ?>
o.h(0);o.t();
//-->
</script>
<?php
	if ($ftype==0) bbs_brcaddread($brdarr["NAME"], $articles[1]["ID"]);
	page_footer();
?>
