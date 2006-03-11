<?php
	require("www2-funcs.php");
	login_init();

function display_navigation_bar($brdarr, $num, $article)
{
	$brd_encode = urlencode($brdarr["NAME"]);
	$PAGE_SIZE = 20;
?>
<a href="bbsdoc.php?board=<?php echo $article[1]["O_BOARD"];?>"><?php echo $article[1]["O_BOARD"];?>版</a> 
<a href="bbscon.php?bid=<?php echo $article[1]["O_BID"];?>&id=<?php echo $article[1]["O_ID"];?>">原始文章</a>
<?php
}

	$board = "Recommend";
	$brdarr = array();
	$brdnum = bbs_getboard($board, $brdarr);
	if ($brdnum == 0) {
		html_error_quit("错误的讨论区");
	}
	$total = bbs_countarticles($brdnum, $dir_modes["DIGEST"]);
	if ($total <= 0) {
		html_error_quit("目前没有文章");
	}

	if(! isset($_GET["id"]) ){
		html_error_quit("错误的文章号");
	}
	$id = $_GET["id"];
	settype($id, "integer");
	if ($id == 0)
	{
		html_error_quit("错误的文章号.");
	}
	$articles = array ();
	$ftype = $dir_modes["NORMAL"];
	$num = bbs_get_records_from_id($brdarr["NAME"], $id, $ftype, $articles);
	if ($num == 0)
	{
		html_error_quit("错误的文章号.");
	}
	$filename=bbs_get_board_filename($brdarr["NAME"], $articles[1]["FILENAME"]);

	if(!file_exists($filename)){
		html_error_quit("错误的文章号...");
	}

	if (cache_header("public",@filemtime($filename),300))
		return;

	@$attachpos=$_GET["ap"];//pointer to the size after ATTACHMENT PAD
	if ($attachpos!=0) {
		die;
		bbs_file_output_attachment($filename, $attachpos);
		exit;
	}
	page_header("推荐文章阅读", "<a href='bbsrecommend.php'>推荐文章</a>");
?>
<h1>推荐文章阅读</h1>
<div class="article">
<div class="recnav">
<?php
	display_navigation_bar($brdarr, $num, $articles);
?>
</div>
<div>
<script type="text/javascript"><!--
att = new attWriter('r',<?php echo $id; ?>,0,0,1);
<?php
/* TODO: this article is put in an extra div because without it, IE6 will show no padding-left for the first line */
	$s = bbs2_readfile($filename);
	if (is_string($s)) echo $s;
?>
//-->
</script>
</div></div>
<div class="oper">[
<?php
	display_navigation_bar($brdarr, $num, $articles);
?>
] [<a href="javascript:history.go(-1)">快速返回</a>]
</div>
<?php
	page_footer();
?>
