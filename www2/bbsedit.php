<?php
	require("www2-funcs.php");
	require("www2-board.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_EDIT);
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
	bbs_set_onboard($brdnum,1);
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $brdnum) == 0)
		html_error_quit("错误的讨论区");
	$board = $brdarr['NAME'];
	if(bbs_checkpostperm($usernum, $brdnum) == 0) {
		html_error_quit("错误的讨论区或者您无权在此讨论区发表文章");
	}
	if (bbs_is_readonly_board($brdarr))
		html_error_quit("不能在只读讨论区发表文章");
	if (isset($_GET["ftype"]))
		if ($_GET["ftype"] == $dir_modes["ZHIDING"])
			$ftype = $dir_modes["ZHIDING"];
		else
			$ftype = $dir_modes["NORMAL"];
	else
		$ftype = $dir_modes["NORMAL"];

	bbs_board_nav_header($brdarr, "修改文章");
	
	if (isset($_GET['id']))
		$id = intval($_GET['id']);
	else
		html_error_quit("错误的文编号");
	$articles = array();
	$num = bbs_get_records_from_id($brdarr["NAME"], $id, $ftype, $articles);
	if ($num == 0)
		html_error_quit("错误的文编号");
	/*$ret = bbs_caneditfile($board,$articles[1]['FILENAME']);
	switch ($ret) {
		case -1:
			html_error_quit("讨论区名称错误");
			break;
		case -2:
			html_error_quit("本版不能修改文章");
			break;
		case -3:
			html_error_quit("本版已被设置只读");
			break;
		case -4:
			html_error_quit("无法取得文件记录");
			break;
		case -5:
			html_error_quit("不能修改他人文章!");
			break;
		case -6:
			html_error_quit("同名ID不能修改老ID的文章");
			break;
		case -7:
			html_error_quit("您的POST权被封");
			break;
		default:
	}*/

	$brd_encode = urlencode($brdarr["NAME"]);
	
	if (isset($_GET['do'])) {

		$ret = bbs_edittitle($board, $id, $_POST["title"], $ftype);
		if($ret != 0)
		{
			switch($ret)
			{
				case -1:
					html_error_quit("错误的讨论区");
					break;
				case -2:
					html_error_quit("对不起，该讨论区不能修改标题");
					break;
				case -3:
					html_error_quit("对不起，该讨论区为只读讨论区");
					break;
				case -4:
					html_error_quit("错误的文章号");
					break;
				case -5:
					html_error_quit("对不起，您已被停止在".$board."版的发文权限");
					break;
				case -6:
					html_error_quit("对不起，您无权修改本文");
					break;
				case -7:
					html_error_quit("标题含有不雅用字");
					break;
				case -8:
					html_error_quit("对不起，当前模式无法修改标题");
					break;
				case -9:
					html_error_quit("标题过长");
					break;
				default:
					html_error_quit("系统错误，请联系管理员");
			}
		}

		$ret=bbs_updatearticle($board,$articles[1]['FILENAME'],$_POST['text']);
		switch ($ret) {
			case -1:
				html_error_quit("修改文章失败，文章可能含有不恰当内容");
				break;
			case -10:
				html_error_quit("找不到文件!");
				break;
			case 0:
				html_success_quit("文章修改成功",
				array("<a href='bbsdoc.php?board=" . $brd_encode . "'>返回 " . $brdarr['DESC'] . "</a>",
				"<a href='bbscon.php?board=" . $brd_encode . "&id=" . $id . "&ftype=" . $ftype . "'>返回《" . htmlspecialchars($_POST["title"]) . "》</a>"));
				break;
			default:
		}
		html_error_quit("系统错误");
	}
?>
<link rel="stylesheet" type="text/css" href="ansi.css"/>
<form name="postform" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>?board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>&ftype=<?php echo $ftype; ?>&do" class="large">
<div class="article">
<?php
	$notes_file = bbs_get_vote_filename($brdarr["NAME"], "notes");
	$fp = FALSE;
	if(file_exists($notes_file))
	{
	    $fp = fopen($notes_file, "r");
	    if ($fp == FALSE)
	    {
	    	$notes_file = "vote/notes";
            if(file_exists($notes_file))
    		    $fp = fopen($notes_file, "r");
		}
	}
	if ($fp == FALSE)
	{
?>
<div class="green">发文注意事项: <br/>
发文时应慎重考虑文章内容是否适合公开场合发表，请勿肆意灌水。谢谢您的合作。</div>
<?php
	}
    else
	{
	    fclose($fp);
		echo bbs_printansifile($notes_file);
	}
?>
</div>
<fieldset><legend>修改文章</legend>
发信人: <?php echo $articles[1]['OWNER']; ?>, 信区: <?php echo $brd_encode; ?> [<a href="bbsdoc.php?board=<?php echo $brd_encode; ?>">本讨论区</a>]<br/>
标&nbsp;&nbsp;题: <input type="text" name="title" size="40" maxlength="100" value="<?php echo $articles[1]['TITLE']; ?>" />
<textarea name="text" onkeydown='return textarea_okd(dosubmit, event);' wrap="physical" id="sfocus">
<?php
	bbs_printoriginfile($board,$articles[1]['FILENAME']);
?>
</textarea>
<div class="oper">
<input type="button" onclick="dosubmit();" name="post" value="修改" />
<input type="reset" value="复原" />
<input type="button" value="返回" onclick="history.go(-1)" />
</div>
</form>
<?php
page_footer();
?>
