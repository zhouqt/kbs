<?php
	require_once("www2-funcs.php");
	require_once("www2-board.php");
	login_init();
bbs_session_modify_user_mode(BBS_MODE_CSIE_ANNOUNCE);
if (isset($_GET["p"])) {
	$numpath = $_GET["p"];

	$brdarr = array();
	$bid = $numpath;
	settype($bid,"integer");
	if( $bid == 0 ){
		html_error_quit("错误的版面");
	}

	$board = bbs_getbname($bid);
	if( !$board ){
		html_error_quit("错误的讨论区");
	}
	if( $bid != bbs_getboard($board, $brdarr) ){
		html_error_quit("错误的讨论区");
	}

	$board = $brdarr['NAME'];

	$filename = bbs_ann_num2path($numpath,$currentuser["userid"]);
	if($filename==false){
		html_error_quit("错误的文章");
	}

	if(! file_exists($filename) || is_dir($filename) ){
		html_error_quit("错误的文章...");
	}
	
	bbs_set_onboard($bid,1);
	if (bbs_normalboard($board)) {
		if (cache_header('public',filemtime($filename),300))
				return;
	}
	
	$up_dirs = array();
	$up_cnt = bbs_ann_updirs($filename,$board,$up_dirs);

} else {

	if (isset($_GET["path"]))
		$path = $_GET["path"];
	else {
		html_error_quit("错误的文章");
	}

	if (strstr($path, ".Names") || strstr($path, "..") || strstr($path, "SYSHome")){
		html_error_quit("错误的文章");
	}

	if($path[0]=='/')
		$filename="0Announce".$path;
	else
		$filename="0Announce/".$path;

	if( bbs_ann_traverse_check($filename,$currentuser["userid"]) < 0 ) {
		html_error_quit("错误的目录");
	}
	
	if(! file_exists($filename)){
		html_error_quit("错误的文章号...");
	}
	
	$board = '';
	$up_dirs = array();
	$up_cnt = bbs_ann_updirs($filename,$board,$up_dirs);
		
	if ($board) {
		$brdarr = array();
		$bid = bbs_getboard($board,$brdarr);
		if ($bid) {
			$board = $brdarr['NAME'];
			$usernum = $currentuser['index'];
			if (bbs_checkreadperm($usernum, $bid) == 0) 
				html_error_quit('不存在该目录');
			bbs_set_onboard($bid,1);
			if (bbs_normalboard($board)) {
				if (cache_header('public',filemtime($filename),300))
					return;
			}
		}
		else {
			$board = '';
			if (cache_header('public',filemtime($filename),300))
				return;
		}
	}
	else {
		if (cache_header('public',filemtime($filename),300))
			return;
		$bid = 0;
	}
}

	@$attachpos=intval($_GET["ap"]);//pointer to the size after ATTACHMENT PAD
	if ($attachpos) {
		bbs_file_output_attachment($filename, $attachpos);
		exit;
	}
	
	if ($bid) {
		page_header("精华区文章阅读", "<a href=\"bbsdoc.php?board=".$brdarr["NAME"]."\">".htmlspecialchars($brdarr["DESC"])."</a>");
		echo '<h1>' . $brdarr["NAME"] . ' 版 (精华区)</h1>';
	} else {
		bbs_ann_header($board);
	}
?>
<div class="article">
<?php
	bbs_print_article($filename,1,$_SERVER['PHP_SELF'] . '?' . $_SERVER['QUERY_STRING']);
?>
</div>
<?php
			
	bbs_ann_xsearch($board);

	($up_cnt >= 2)?bbs_ann_foot($up_dirs[$up_cnt - 2]):bbs_ann_foot('');

	page_footer();
?>
