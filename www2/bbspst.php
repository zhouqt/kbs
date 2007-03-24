<?php
	require("www2-funcs.php");
	require("www2-board.php");
	$htmlErrorNotify = "divReplyForm";
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_POSTING);
	assert_login();

	if (isset($_GET["board"]))
		$board = $_GET["board"];
	else
		html_error_quit("错误的讨论区");
	// 检查用户能否阅读该版
	$brdarr = array();
	$brdnum = bbs_getboard($board, $brdarr);
	if ($brdnum == 0)
		html_error_quit("错误的讨论区");
	$board = $brdarr["NAME"];
	bbs_set_onboard($brdnum,1);
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $brdnum) == 0)
		html_error_quit("错误的讨论区");
	if(bbs_checkpostperm($usernum, $brdnum) == 0) {
		html_error_quit("错误的讨论区或者您无权在此讨论区发表文章");
	}
	if (bbs_is_readonly_board($brdarr))
		html_error_quit("不能在只读讨论区发表文章");
	if (isset($_GET["reid"]))
	{
		$reid = $_GET["reid"];
		if(bbs_is_noreply_board($brdarr))
			html_error_quit("本版只可发表文章,不可回复文章!");
	}
	else {
		$reid = 0;
	}
	settype($reid, "integer");
	$articles = array();
	if ($reid > 0)
	{
		$num = bbs_get_records_from_id($board, $reid,$dir_modes["NORMAL"],$articles);
		if ($num == 0)
		{
			html_error_quit("错误的 Re 文编号");
		}
		if ($articles[1]["FLAGS"][2] == 'y')
			html_error_quit("该文不可回复!");
	}
	$brd_encode = urlencode($board);
	
	bbs_board_nav_header($brdarr, $reid ? "回复文章" : "发表文章");
?>
<script type="text/javascript" src="static/www2-addons.js"></script>
<script type="text/javascript"><!--
	var o = new replyForm('<?php echo $brd_encode; ?>',<?php echo $reid; ?>,'<?php if ($reid) echo addslashes($articles[1]["TITLE"]); ?> ',<?php
	echo bbs_is_attach_board($brdarr)?"1":"0"; ?>,<?php echo $currentuser["signum"]; ?>,<?php echo $currentuser["signature"]; ?>,<?php
	echo bbs_is_anony_board($brdarr)?"1":"0"; ?>,<?php echo bbs_is_outgo_board($brdarr)?"1":"0"; ?>,<?php
		$local_save = 0;
		if ($reid > 0) $local_save = !strncmp($articles[1]["INNFLAG"], "LL", 2);
	echo $local_save?"1":"0"; ?>);w(o.f());
//-->
</script>
<?php
	if($reid > 0){
		$filename = $articles[1]["FILENAME"];
		$filename = "boards/" . $board . "/" . $filename;
		echo bbs_get_quote($filename);
	}
?>
</textarea>
<script type="text/javascript">w(o.t());</script>
<?php
page_footer();
?>
