<?php
	/**
	 * This file displays article to user in digest .
	 * $Id$
	 */
	require("funcs.php");
login_init();

function display_navigation_bar($brdarr, $short_filename, $num, $article)
{
	global $currentuser;

	$brd_encode = urlencode($brdarr["NAME"]);
	$PAGE_SIZE = 20;
?>
[<a href="/bbsdoc.php?board=<?php echo $article[1]["O_BOARD"];?>">文章原始版面:<?php echo $article[1]["O_BOARD"];?></a>]
[<a href="/bbscon.php?board=<?php echo $article[1]["O_BOARD"];?>&id=<?php echo $article[1]["O_ID"];?>">原始文章</a>]
[<a href="javascript:history.go(-1)">快速返回</a>]
<?php
}



	if ($loginok != 1)
		html_nologin();
	else
	{
		$board = "Recommend";
		$brdarr = array();
		$brdnum = bbs_getboard($board, $brdarr);
		if ($brdnum == 0) {
			html_init("gb2312");
			html_error_quit("错误的讨论区");
		}
		$total = bbs_countarticles($brdnum, $dir_modes["DIGEST"]);
		if ($total <= 0) {
			html_init("gb2312");
			html_error_quit("目前没有文章");
		}

			if(! isset($_GET["id"]) ){
				html_init("gb2312");
				html_error_quit("错误的文章号");
			}
			$id = $_GET["id"];
			settype($id, "integer");
			if ($id == 0)
			{
				html_init("gb2312");
				html_error_quit("错误的文章号.");
			}
	$articles = array ();
	$ftype = $dir_modes["NORMAL"];
	$num = bbs_get_records_from_id($brdarr["NAME"], $id, 
			$ftype, $articles);
	if ($num == 0)
	{
		html_init("gb2312","","",1);
		html_error_quit("错误的文章号.");
	}
		$filename=bbs_get_board_filename($brdarr["NAME"], $articles[1]["FILENAME"]);

		$test_file = @fopen($filename,"r");
		if(! $test_file ){
			html_init("gb2312");
			html_error_quit("错误的文章号...");
		}
		fclose($test_file);

#       			if (cache_header("public",filemtime($filename),300))
#               			return;
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
<center><p><?php echo BBS_FULL_NAME; ?> -- 推荐文章阅读 </a></p></center>
<?php
				display_navigation_bar($brdarr, $short_filename, $num, $articles);
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
				display_navigation_bar($brdarr, $articles, $num, $articles);
			}

		html_normal_quit();
	}
?>
