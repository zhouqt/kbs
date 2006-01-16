<?php
	require("www2-funcs.php");
	require("www2-board.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_READING);
	assert_login();
	
	if (isset($_GET["board"]))
		$board = $_GET["board"];
	else
		html_error_quit("错误的讨论区");
	
	$id = intval($_GET["id"]);
	if(!$id)
		html_error_quit("错误的文章");
	
	// 检查用户能否阅读该版
	$brdarr = array();
	$brdnum = bbs_getboard($board, $brdarr);
	if ($brdnum == 0)
		html_error_quit("错误的讨论区");
	
	$board=$brdarr["NAME"];
	$brd_encode = urlencode($board);
	bbs_set_onboard($brdnum,1);
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $brdnum) == 0)
		html_error_quit("错误的讨论区");

	if(isset($_POST["oldprop"]))
	{
		$oldprop = $_POST["oldprop"];
		$prop = $oldprop & 070;
		if(isset($_POST["mailback"]) && $_POST["mailback"]=="on")
			$prop |= 004;
		if(isset($_POST["innflag"]) && $_POST["innflag"]=="on")
			$prop |= 002;
		if(isset($_POST["cancelo"]) && $_POST["cancelo"]=="on")
			$prop |= 001;
		if($oldprop != $prop)
		{
			$ret = bbs_article_property_write($board, $id, $prop);
			switch($ret)
			{
			case 0:
				html_success_quit("属性修改成功");
			case -1:
				html_error_quit("错误的讨论区");
			case -2:
				html_error_quit("错误的文章");
			case -3:
				html_error_quit("系统错误");
			default:
				html_error_quit("系统错误 ".$ret);
			}
		}
	}
	
	$prop = bbs_article_property_read($board, $id);
	
	page_header("修改文章属性", "<a href=\"bbsdoc.php?board={$board}\">".htmlspecialchars($brdarr["DESC"])."</a>");
?>
<form action="<?php echo $_SERVER["PHP_SELF"]; ?>?action=do&board=<?php echo $brd_encode; ?>&id=<?php echo $id; ?>" method="post" class="large"/>
	<fieldset><legend>修改文章属性</legend><div align="center">
	<input type="hidden" name="oldprop" value="<?php echo $prop; ?>"><table cellspacing="10">
<?php
	print("<tr><td>回文转寄到信箱</td><td>");
	if($prop & 040)
	{
		print("<input type=\"checkbox\" name=\"mailback\" value=\"on\"");
		if($prop & 004)
			print(" checked");
		print(">");
	}
	else
		print("DENY");
	print("</td></tr><tr><td>转信发表</td><td>");
	if($prop & 020)
	{
		print("<input type=\"checkbox\" name=\"innflag\" value=\"on\"");
		if($prop & 002)
			print(" checked");
		print(">");
	}
	else
		print("DENY");
	print("</td></tr><tr><td>收精华标记</td><td>");
	if($prop & 010)
	{
		print("<input type=\"checkbox\" name=\"cancelo\" value=\"on\"");
		if($prop & 001)
			print(" checked");
		print(">");
	}
	else
		print("DENY");
	print("</td></tr>");
?>
	</table></div></fieldset>
	<div class="oper"><input type="submit" value="修改" /></div>
</form>

<?php
	page_footer();
?>