<?php
	require("www2-funcs.php");
	login_init();

	if( !isset($_GET["board"]) && !isset($_POST["board"]))
		html_error_quit("错误的讨论区");
	if( isset($_GET["board"]) )
		$board = $_GET["board"];
	else
		$board = $_POST["board"];

	$brdarr = array();
	$brdnum = bbs_getboard($board, $brdarr);
	if ($brdnum == 0){
		html_error_quit("错误的讨论区");
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $brdnum) == 0){
		html_error_quit("错误的讨论区");
	}
	$board = $brdarr["NAME"];
	$isnormalboard = bbs_normalboard($board);

	if ($isnormalboard) {
		$dotdirname = bbs_get_board_index($board, $dir_modes["NORMAL"]);
		if (cache_header("public",@filemtime($dotdirname),300))
			return;
	}
	bbs_board_nav_header($brdarr, "版内文章搜索");
	if(!isset($_GET["q"])){
?>
<script type="text/javascript">
<!--
	document.write(getFindBox('<?php echo $brdarr["NAME"]; ?>'));
//-->
</script>
<?php
		page_footer();
		exit;
	}

	if( isset( $_GET["title"] ) ){
		$title = $_GET["title"];
	}else
		$title="";

	if( isset( $_GET["title2"] ) ){
		$title2 = $_GET["title2"];
	}else
		$title2="";

	if( isset( $_GET["title3"] ) ){
		$title3 = $_GET["title3"];
	}else
		$title3="";

	if( isset( $_GET["userid"] ) ){
		$userid = $_GET["userid"];
	}else
		$userid="";

	if( isset( $_GET["dt"] ) ){
		$dt = $_GET["dt"];
	}else
		$dt=0;
	settype($dt, "integer");
	if($dt <= 0)
		$dt = 7;
	else if($dt > 9999)
		$dt = 9999;

	if( isset( $_GET["mg"] ) ){
		$mg = $_GET["mg"];
	}else
		$mg = "";
	if($mg != "")
		$mgon=1;
	else
		$mgon=0;

	if( isset( $_GET["og"] ) ){
		$og = $_GET["og"];
	}else
		$og = "";
	if($og != "")
		$ogon=1;
	else
		$ogon=0;

	if( isset( $_GET["ag"] ) ){
		$ag = $_GET["ag"];
	}else
		$ag = "";
	if($ag != "")
		$agon=1;
	else
		$agon=0;


function log_it($id, $ip, $board, $title, $title2, $title3, $userid, $dt, $mgon, $agon, $ogon, $num) {
	$action = $board . " | " . $title . " | " . $title2 . " | " . $title3 . " | " . $userid . " | " . $dt . " | " . $mgon . " | " . $agon . " | " . $ogon . " | " . $num;
    $action = "[".date("Y-m-d H:i:s")."] $id($ip) | ".$action."\n";
    $logs = BBS_HOME . "/bbsbfind.log";
    if(!($fn = fopen($logs,"a")))
        return FALSE;
    if (!flock ($fn, LOCK_EX))
    {
        fclose ($fh);
        return FALSE;
    }

    fputs($fn,$action);
    flock ($fn, LOCK_UN);
    fclose($fn);
    return TRUE;
}

	$articles = bbs_search_articles($board, $title, $title2, $title3, $userid, $dt, $mgon, $agon, $ogon);

	if( $articles <= 0 ){
		html_error_quit("系统错误:".$articles);
	}
//	log_it($currentuser["userid"], $_SERVER["REMOTE_ADDR"], $board, $title, $title2, $title3, $userid, $dt, $mgon, $agon, $ogon, count($articles));
?>
<script>
var ta = new tabWriter(0,'main wide','查找讨论区"<?php echo $brdarr["NAME"];?>"内, 标题含: "<?php echo htmlspecialchars($title,ENT_QUOTES);?>"<?php if($title2!="") echo ' 和"'.htmlspecialchars($title2,ENT_QUOTES).'"';?><?php if($title3!="") echo ',不含"'.htmlspecialchars($title3,ENT_QUOTES).'"';?> 作者为: "<?php if($userid!="") echo $userid; else echo "所有者";?>", "<?php echo $dt;?>"天以内的 <?php if($mgon) echo "精华"; if($agon) echo "附件"; if($ogon) echo "主题";?>文章',
[['编号','6%','center'],['标记','6%','center'],['作者','12%','center'],['日期','8%','center'],['标题',0,0]]);
<?php
	foreach ($articles as $article)
	{
		$flags = $article["FLAGS"];
		$col1 = $article["NUM"];
		$col2 = $flags[0] . $flags[3];
		$col3 = '<a href="bbsqry.php?userid=' . $article["OWNER"] . '">' . $article["OWNER"] . '</a>';
		$col4 = strftime("%b&nbsp;%e", $article["POSTTIME"]);
		$col5 = '<a href="bbscon.php?board=' . $brdarr["NAME"] . '&id=' . $article["ID"] . '">' . htmlspecialchars($article["TITLE"], ENT_QUOTES) . ' </a>';
		echo "ta.r('$col1','$col2','$col3','$col4','$col5');\n";
	}
	$i = count($articles);
?>
ta.t();
</script>
<div class="oper">
共找到 <?php echo $i;?> 篇文章符合条件 <?php if($i>=999) echo "(匹配结果过多, 省略第1000以后的查询结果)";?>
[<a href="bbsdoc.php?board=<?php echo $brdarr["NAME"];?>">返回本讨论区</a>] [<a href="javascript:history.go(-1)">返回上一页</a>]
</div>
<?php
	page_footer();
?>
