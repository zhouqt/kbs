<?php
	require("www2-funcs.php");
	login_init();

	$usernum = $currentuser["index"];
	if (isset($_GET["board"]))
		$board = $_GET["board"];
	else 
		html_error_quit("错误的讨论区");

	$brdarr = array();
	$brdnum = bbs_getboard($board, $brdarr);
	if ($brdnum == 0)
		html_error_quit("错误的讨论区");
	if (bbs_checkreadperm($usernum,$brdnum)==0)
		html_error_quit("错误的讨论区");
	$top_file= bbs_get_vote_filename($brdarr["NAME"], "notes");
	if (cache_header("public",@filemtime($top_file),300))
		return;

	bbs_board_nav_header($brdarr, "备忘录");
	$brd_encode = urlencode($brdarr["NAME"]);
?>
<link rel="stylesheet" type="text/css" href="ansi.css"/>
<div class="article smaller">
<?php
	if (!file_exists($top_file)) {
		echo "<br/><br/><br/>&nbsp; &nbsp; &nbsp; &nbsp; 此讨论区尚无「备忘录」。";
	} else {
		echo bbs_printansifile($top_file);
	}
?>
</div>
<div class="oper">
[<a href="bbsdoc.php?board=<?php echo $brd_encode; ?>">本讨论区</a>]
<?php
	if (bbs_is_bm($brdnum,$usernum)) {
?>
[<a href="bbsmnote.php?board=<?php echo $brd_encode; ?>">编辑进版画面</a>]
<?php
	}
?> 
[<?php bbs_add_super_fav ('[备忘录] '.$brdarr['DESC'], 'bbsnot.php?board='.$brd_encode); ?>]
</div>
<?php
	page_footer();
?>
