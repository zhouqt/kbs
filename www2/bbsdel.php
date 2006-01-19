<?php
	require("www2-funcs.php");
	login_init();
	assert_login();
	page_header("删除文章");
	$board = $_GET["board"];
	$id = $_GET["id"];

	$ret = bbs_delpost($board,$id); // 0 success -1 no perm  -2 wrong parameter
	switch($ret)
	{
	case -1:
		html_error_quit("你无权删除该文!");
		break;
	case -2:
		html_error_quit("错误的版名或者文章号!");
		break;
	default:
		html_success_quit("删除成功.<br><a href=\"bbsdoc.php?board=" . $board . "\">返回本讨论区</a>");
	}
?>
