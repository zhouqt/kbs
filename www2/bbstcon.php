<?php
	require("www2-funcs.php");
	login_init();
	bbs_session_modify_user_mode(BBS_MODE_READING);
	$gid = $_GET["gid"];
	settype($gid, "integer");
	$start = isset($_GET["start"])?$_GET["start"]:0;
	settype($start, "integer");
	$pno = isset($_GET["pno"])?$_GET["pno"]:0;
	settype($pno, "integer");
	
	if( $gid < 0 ) $gid = 0 ; 
	if($start <= 0 ) $start = $gid;
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
	$num = bbs_get_threads_from_gid($bid, $gid, $start , $articles , $haveprev );
	if( $num == 0 )
	{
		html_error_quit("错误的参数");
	}
	
	$pagesize = 20;
	$totalpage = ( $num - 1 ) / $pagesize + 1;
	$totalpage = intval( $totalpage );
	if( $pno < 1 || $pno > $totalpage )
		$pno = 1;
	
	$startnum = ( $pno - 1 ) * $pagesize;
	$endnum = $startnum + $pagesize;
	if( $endnum > $num )
		$endnum = $num;

	bbs_checkorigin($board);
	$origins = array();
	$origin_num = bbs_get_records_from_id($board, $gid, $dir_modes["ORIGIN"], $origins);

	if($origin_num == 0)
		$origins[0]["ID"] = $origins[2]["ID"] = $gid;
	
	if ($isnormalboard) { /* cache 判断索引修改时间 - atppp */
		$dotdirname = bbs_get_board_index($board, $dir_modes["NORMAL"]);
		if (cache_header("public",@filemtime($dotdirname),300))
			return;
	}
	page_header("同主题-" . htmlspecialchars($brdarr["DESC"]). "-" .htmlspecialchars($articles[0]["TITLE"]), "<a href=\"bbsdoc.php?board=".$brd_encode."\">".htmlspecialchars($brdarr["DESC"])."</a>");
?>
<a name="top"></a>
<h1 class="ttit">同主题阅读：<?php echo htmlspecialchars($articles[0]["TITLE"]); ?> </h1>
<script type="text/javascript"><!--
var o = new tconWriter('<?php echo addslashes($board); ?>',<?php echo $bid; ?>,<?php echo $gid; ?>,<?php echo $start; ?>,<?php echo $totalpage; ?>,<?php echo $pno; ?>,<?php echo $startnum; ?>,<?php echo $origins[0]["ID"]; ?>,<?php echo $origins[2]["ID"]; ?>);
o.h();
<?php
	$strs = array();
	for( $i = $startnum ; $i < $endnum ; $i ++ )
	{
		$article = $articles[$i];
		$strs[] = "[" . $article["ID"] . ",'" . $article["OWNER"] . "']";
		bbs_brcaddread($board , $article["ID"]);
	}
	$arts = "[" . implode(",", $strs) . "]";
?>
o.o(<?php echo $arts; ?>);o.h();
//-->
</script>
<div class="oper">
<?php bbs_add_super_fav ('[同主题] '.$articles[0]['TITLE'], 'bbstcon.php?board='.$board.'&gid='.$gid); ?>
| <a href="javascript:history.go(-1)">后退</a>
| <a href="bbsdoc.php?board=<?php echo $board; ?>">进入 <b><?php echo $board_desc; ?></b> 讨论区</a>
<a href="#top">返回顶部</a>
</div>
<?php
	page_footer();
?>
