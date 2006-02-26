<?php
function bbs_ann_bm_display_articles($articles, $isBoard) {
	global $show_none, $has_perm_boards, $path;
	$pathstr = substr($path, 9, strlen($path) - 9);
?>
<script type="text/javascript"><!--
var an = new annWriter('<?php echo rawurlencode($pathstr); ?>',<?php echo $has_perm_boards?"1":"0"; ?>);
<?php
	if($show_none)
	{
?>
document.write('<tr><td align="center" colspan="7">该精华区目录没有文章。</td></tr>');
<?php
	}
	else foreach ($articles as $article) {
		$title = htmlspecialchars(rtrim($article['TITLE']), ENT_QUOTES);
		echo 'an.i(' . $article['FLAG'] . ',\'' . $title . '\',\'' . trim($article['BM']) . '\',\'' . rawurlencode($article['FNAME']) . '\',\'' . date('Y-m-d',$article['TIME']) . '\');';
	}
?>
//-->
an.f();
</script>
<?php
	if ($isBoard) echo "</div>"; /* </div>: dirty way ... for closing <div class="doc"> */
}

require('bbs0anbm_pre.php');
	
$board = '';
$articles = array();
$path_tmp = '';
$ret = bbs_read_ann_dir($path,$board,$path_tmp,$articles,1);

$show_none = 0;
switch ($ret) {
	case -1:
		html_error_quit('精华区目录不存在');
		break;
	case -2:
		html_error_quit('无法加载目录文件');
		break;
	case -3:
		$show_none = 1;
		break;
	case -9:
		html_error_quit('系统错误');
		break;
	default;
}

$path = $path_tmp;
$isBoard = false;
$up_cnt = bbs_ann_updirs($path,$board,$up_dirs);
if ($board) {
	$brdarr = array();
	/*if (defined ('USE_ROAM')) {
		$bid = bbs_roam_getboard ($board, $brdarr);
		if ($bid < 0)
			html_error_quit('系统错误');
	}
	else */
		$bid = bbs_getboard($board,$brdarr);
	if ($bid) {
		$board = $brdarr['NAME'];
		$usernum = $currentuser['index'];
		/*if (defined ('USE_ROAM')) {
			$ret = bbs_roam_checkreadperm($usernum, $bid);
			if ( $ret <= 0)
				html_error_quit('不存在该目录');
			$ret = bbs_roam_normalboard($board);
			if ( $ret < 0 )
				html_error_quit('系统错误');
			if ( $ret == 1 ) {
				$dotnames = BBS_HOME . '/' . $path . '/.Names';
				if (cache_header('public',filemtime($dotnames),300))
					return;
			}
		}
		else { */
			if (bbs_checkreadperm($usernum, $bid) == 0)
				html_error_quit('不存在该目录');
			bbs_set_onboard($bid,1);
		/*} */
		bbs_board_header($brdarr,-1,0);
		$isBoard = true;
	}
	else {
		$board = '';
		bbs_ann_header($board);
	}
	
}
else {
	$bid = 0;
	bbs_ann_header();
}
?>
<script src="www2-addons.js" type="text/javascript"></script>
<?php
bbs_ann_bm_display_articles($articles, $isBoard);
	
if ($up_cnt >= 2)
	bbs_ann_bm_foot($up_dirs[$up_cnt - 2]);
else
	bbs_ann_bm_foot('');

page_footer();
?>
