<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_EDITUFILE);
	$usernum = $currentuser["index"];
	if (isset($_GET["board"]))
		$board = $_GET["board"];
	else 
		html_error_quit("错误的讨论区");

	$brdarr = array();
	$brdnum = bbs_getboard($board, $brdarr);
	if ($brdnum == 0)
		html_error_quit("错误的讨论区");
	if (!bbs_is_bm($brdnum,$usernum))
		html_error_quit("你不是版主");
	$top_file="vote/" . $board . "/notes";
	if (isset($_POST["text"])) {
		$fp = @fopen($top_file, "w");
		if ($fp==FALSE) {
			html_error_quit("无法打开文件");
		} else {
			$data=$_POST["text"];
			fwrite($fp,str_replace("\r\n","\n",$data));
			fclose($fp);
			html_success_quit("修改备忘录成功!<br/><a href=\"bbsdoc.php?board=" . $board . "\">本讨论区</a>");
		}
	}
	bbs_board_nav_header($brdarr, "备忘录编辑");
?>
<form method="post" action=<?php echo "\"bbsmnote.php?board=" . $board . "\""; ?> class="large">
<fieldset><legend>修改进版画面</legend>
<textarea name="text" onkeydown='return textarea_okd(this, event);' wrap="physical" id="sfocus">
<?php
    echo @htmlspecialchars(file_get_contents($top_file));
?>
</textarea>
</fieldset>
<div class="oper">
<input type="submit" value="存盘" /> <input type="reset" value="复原" />
</div>
</form>
<?php
	page_footer();
?>
