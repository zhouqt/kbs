<?php
	/**
	 * This file displays article to user in digest .
	 * $Id$
	 */
	require("funcs.php");
login_init();

function display_navigation_bar($brdarr, $short_filename, $num)
{
	global $currentuser;

	$brd_encode = urlencode($brdarr["NAME"]);
	$PAGE_SIZE = 20;
?>
[<a href="/bbssec.php">分类讨论区</a>]
[<a href="/bbsdoc.php?board=<?php echo $brd_encode; ?>">返回版面</a>]
[<a href="/bbsgdoc.php?board=<?php echo $brd_encode; ?>&page=<?php echo intval(($num + $PAGE_SIZE - 1) / $PAGE_SIZE); ?>">返回文摘区</a>]
[<a href="javascript:history.go(-1)">快速返回</a>]
<?php
}

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
		$total = bbs_countarticles($brdnum, $dir_modes["THREAD"]);
		if ($total <= 0) {
			html_init("gb2312");
			html_error_quit("本文摘区目前没有文章");
		}

#		if (isset($_GET["file"]))
#			$short_filename = $_GET["file"];
#		else {
			if(! isset($_GET["num"]) ){
				html_init("gb2312");
				html_error_quit("错误的文章号");
			}
			$num = $_GET["num"];
			settype($num, "integer");
			if ($num == 0)
			{
				html_init("gb2312");
				html_error_quit("错误的文章号.");
			}
			$short_filename = bbs_get_filename_from_num( $brdarr["NAME"], $num, $dir_modes["THREAD"] );
			if(! $short_filename ){
				html_init("gb2312");
				html_error_quit("错误的文章号.....");
			}
#		}
                bbs_set_onboard($brcnum,1);

		$filename=bbs_get_board_filename($brdarr["NAME"], $short_filename);
		$test_file = @fopen($filename,"r");
		if(! $test_file ){
			html_init("gb2312");
			html_error_quit("错误的文章号...");
		}
		fclose($test_file);

			if (bbs_normalboard($board)) {
            			if (cache_header("public",filemtime($filename),300))
                			return;
                	}
//			Header("Cache-control: nocache");
			@$attachpos=$_GET["ap"];//pointer to the size after ATTACHMENT PAD
			if ($attachpos!=0) {
				require_once("attachment.php");
				output_attachment($filename, $attachpos);
				exit;
			} else
			{
				html_init("gb2312");
?>
<body>
<center><p><?php echo BBS_FULL_NAME; ?> -- 文章阅读 [讨论区: <?php echo $brdarr["NAME"]; ?>]</a></p></center>
<?php
				display_navigation_bar($brdarr, $short_filename, $num);
?>
<hr class="default" />
<table width="610" border="0">
<tr><td>
<?php
				bbs_print_article($filename,1,$_SERVER['PHP_SELF'] . '?' . $_SERVER['QUERY_STRING']);
?>
</td></tr></table>
<hr class="default" />
<?php
				display_navigation_bar($brdarr, $articles, $num);
			}

//		if ($currentuser["userid"] != "guest")
//			bbs_brcaddread($brdarr["NAME"], $articles[1]["ID"]);
		html_normal_quit();
	}
?>
