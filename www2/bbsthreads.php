<?php
	require("www2-funcs.php");
	login_init();
	assert_login();

	if(isset($_GET["board"]))
		$board = $_GET["board"];
	else
		html_error_quit("错误的参数");
	$brdarr = array();
	$bid = bbs_getboard($board, $brdarr);
	if($bid == 0)
		html_error_quit("错误的讨论区");
	$board = $brdarr["NAME"];
	$brd_encode = urlencode( $board );
	if(isset($_GET["gid"]))
		$gid = $_GET["gid"];
	else
		html_error_quit("错误的参数");
	if(isset($_GET["start"]))
		$start = $_GET["start"];
	else
		html_error_quit("错误的参数");
	
	if(isset($_POST["operate"]))
	{
		$operate = $_POST["operate"];
		$ret = bbs_threads_bmfunc($bid, $gid, $start, $operate);
		if($ret >= 0)
			html_success_quit("操作成功， {$ret} 条记录被修改。");
		else if($ret == -1)
			html_error_quit("错误的讨论区域");
		else if($ret == -2)
			html_error_quit("没有权限");
		else
			html_error_quit("系统内部错误");
	}
	
	bbs_board_nav_header($brdarr, "同主题操作");
?>
<form action="bbsthreads.php?board=<?php echo $board; ?>&gid=<?php echo $gid; ?>&start=<?php echo $start; ?>" method="post" class="medium">
	<fieldset>
		<legend>同主题操作</legend>
		<div class="inputs">请选择要执行的操作：<br><blockquote>
			<input type="radio" name="operate" value="1">删除<br>
			<input type="radio" name="operate" value="2">M标记<br>
			<input type="radio" name="operate" value="3">取消M标记<br>
			<input type="radio" name="operate" value="4">删除拟删文章<br>
			<input type="radio" name="operate" value="6">设定拟删标记<br>
			<input type="radio" name="operate" value="7">取消拟删标记<br>
			<input type="radio" name="operate" value="8">设定不可回复<br>
			<input type="radio" name="operate" value="9">取消不可回复<br>
		</blockquote>
			<div align="center"><input type="submit" value="确定"><br></div>
		</div>
	</fieldset>
</form><br>
<?php
	page_footer();
?>