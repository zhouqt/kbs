<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_READING);
	$gid = $_GET["gid"];
	settype($gid, "integer");
	
	if( $gid < 0 ) $gid = 0 ; 
	$board = $_GET["board"];
	$brdarr = array();
	
	$bid = bbs_getboard($board , $brdarr);
	if($bid == 0)
	{
		html_error_quit("错误的讨论区");
	}
	$board = $brdarr["NAME"];
	$board_desc = $brdarr["DESC"];
	$brd_encode = urlencode( $board );
	
	$isnormalboard = bbs_normalboard($board);
	bbs_set_onboard($bid,1);

	$usernum = $currentuser["index"];
	if (!$isnormalboard && bbs_checkreadperm($usernum, $bid) == 0) 
	{
		html_error_quit("错误的讨论区");
	}
	
	$haveprev = 0;
	$num = bbs_get_threads_from_gid($bid, $gid, 0 , $articles , $haveprev );
	if( $num == 0 )
	{
		html_error_quit("错误的参数");
	}
	
	if ($isnormalboard) { /* cache 判断索引修改时间 - atppp */
		$dotdirname = bbs_get_board_index($board, $dir_modes["NORMAL"]);
		if (cache_header("public",@filemtime($dotdirname),300))
			return;
	}
	page_header("回复树主题阅读", "<a href=\"bbsdoc.php?board=".$brd_encode."\">".htmlspecialchars($brdarr["DESC"])."</a>");
?>
<a name="top"></a>
<h1 class="ttit">回复树主题阅读：<?php echo htmlspecialchars($articles[0]["TITLE"]); ?> </h1>
<link rel="stylesheet" type="text/css" href="static/www2-tree.css" />
<script type="text/javascript" src="static/www2-addons.js"></script>
<script type="text/javascript"><!--
<?php
	$strs = array();
	for( $i = 0 ; $i < $num ; $i ++ )
	{
		$article = $articles[$i];
		$strs[] = "[" . $article["ID"] . "," . $article["REID"] . ",'" . $article["OWNER"] . "']";
	}
	$arts = "[" . implode(",", $strs) . "]";
?>
var o = new treeWriter('<?php echo addslashes($board); ?>',<?php echo $bid; ?>,<?php echo $gid; ?>,<?php echo $arts; ?>);
o.o();
//-->
</script>
<div class="oper">
<a href="bbsdoc.php?board=<?php echo $board; ?>">进入 <b><?php echo $board_desc; ?></b> 讨论区</a>
</div>
<?php
	page_footer();
?>
