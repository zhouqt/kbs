<?php
	/**
	 * This file displays article to user.
	 * $Id$
	 */
	require("funcs.php");
	if ($loginok != 1)
		html_nologin();
	else
	{
		if (isset($_GET["board"]))
			$board = $_GET["board"];
		else {
			html_init("gb2312");
			html_error_quit("错误的讨论区");
		}
		// 检查用户能否阅读该版
		$brdarr = array();
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0) {
			html_init("gb2312");
			html_error_quit("错误的讨论区");
		}
		$usernum = $currentuser["index"];
		if (bbs_checkreadperm($usernum, $brdnum) == 0) {
			html_init("gb2312");
			html_error_quit("错误的讨论区");
		}
		$total = bbs_countarticles($brdnum, $dir_modes["NORMAL"]);
		if ($total <= 0) {
			html_init("gb2312");
			html_error_quit("本讨论区目前没有文章");
		}
		if (isset($_GET["num"]))
			$num = $_GET["num"];
		else {
			html_init("gb2312");
			html_error_quit("错误的参数");
		}
		if (isset($_GET["id"]))
			$id = $_GET["id"];
		else {
			html_init("gb2312");
			html_error_quit("错误的文章号");
		}
		$articles = bbs_get_records_from_id($brdarr["NAME"], $id, 
				$dir_modes["NORMAL"]);
		if ($articles == FALSE)
		{
			html_init("gb2312");
			html_error_quit("bbs_get_records_from_id failed.");
		}
		else
		{
			$filename=bbs_get_board_filename($brdarr["NAME"], $articles[1]["FILENAME"]);
                	if (cache_header("public",filemtime($filename),300))
                		return;
			@$attachpos=$_GET["attachpos"];
			@$attachname=$_GET["filename"];
			if ($attachpos!=0) {
				Header("Content-type: " . mime_content_type($attachname));
				Header("Accept-Ranges: bytes");
				Header("Accept-Length: " . filesize($filename));
				Header("Content-Disposition: attachment; filename=" . $attachname);
				$file = fopen($filename, "r");
				fseek($file,$attachpos);
				echo fread($file,filesize($filename) - $attachpos);
				fclose($file);
				exit;
			} else
?>
<body>
<?php
			html_init("gb2312");
			bbs_printansifile($filename,1,$_SERVER['PHP_SELF']);
		}
		html_normal_quit();
	}
?>
