<?php
	/**
	 *  This file delete board article.
	 */
	require("funcs.php");
	if ($loginok !=1 )
		html_nologin();
	else
	{
		html_init("gb2312");
		$board = $_GET["board"];
		$file = $_GET["file"];

		$ret = bbs_delfile($board,$file); // 0 success -1 no perm  -2 wrong parameter
		switch($ret)
		{
		case -1:
			html_error_quit("你无权删除该文!");
			break;

		case -2:
			html_error_quit("错误的版名或者文件名!");
			break;
		default:
?>
删除成功.<br><a href="/bbsdoc.php?board=<?php echo $board;?>">返回本讨论区</a>
<?php
			break;
		}
	}
?>
