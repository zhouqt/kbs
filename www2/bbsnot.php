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
	$isnormalboard = bbs_normalboard($board);
	if ($isnormalboard) {
		$mt = file_exists($top_file) ? @filemtime($top_file) : time();
		if (cache_header("public",$mt,1800))
			return;
	}

	bbs_board_nav_header($brdarr, "备忘录");
	$brd_encode = urlencode($brdarr["NAME"]);
?>
<link rel="stylesheet" type="text/css" href="ansi.css"/>
<script type="text/javascript">
function writeNote()
{
	var bbsnote,notecontent,divbbsnot;
<?php
	$s = false;
	if (file_exists($top_file)) {
		$s = bbs_printansifile($top_file);
	}
	if (!is_string($s))
	{
		echo "\tnotecontent='".addslashes("<br/><br/><br/>&nbsp; &nbsp; &nbsp; &nbsp; 此讨论区尚无「备忘录」。")."';\n";
		echo "\tbbsnote='".addslashes("<div class=\"green\">发文注意事项: <br/>发文时应慎重考虑文章内容是否适合公开场合发表，请勿肆意灌水。谢谢您的合作。</div>")."';\n";
	} else {
		echo "\tbbsnote=notecontent='".addslashes($s)."';\n";
	}
?>
	if (parent != self && parent && (divbbsnot = parent.document.getElementById('bbsnot')))
	{
		divbbsnot.innerHTML = bbsnote;
	}
	else
	{
		document.getElementById('bbsnot').innerHTML = notecontent;
	}

}
addBootFn(writeNote);
</script>
<div class="article smaller" id="bbsnot">
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
