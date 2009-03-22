<?php
function bbs_ann_display_articles($articles, $isBoard) {
	global $show_none;
?>
<table class="main wide"><col width="5%"><col width="7%"><col><col width="12%"><col width="12%">
<tbody><tr><th>#</th><th>类型</th><th>标题</th><th>整理</th><th>编辑日期</th></tr></tbody>
<tbody>
<?php
	$num = 0;
	if($show_none)
	{
?>
<tr><td align="center" colspan="5">该精华区目录没有文章。</td></tr>
<?php
	}
	else foreach ($articles as $article) {
		switch($article['FLAG']) {
			case 0:
				$img = 'oldgroup.gif';
				$alt = '错误';
				$url = '';
				break;
			case 1:
				$img = 'groupgroup.gif';
				$alt = '目录';
				$url = 'bbs0an.php?path='.rawurlencode($article['PATH']);
				break;
			case 2:
			case 3:
			default:
				$img = 'newgroup.gif';
				$alt = '文件';
				$url = 'bbsanc.php?path='.rawurlencode($article['PATH']);
		}
		$col1 = 'putImage(\''.$img.'\',\'alt="'.$alt.'"\')';
		if ($article['FLAG']==3)
			$col2 = '<font color="red">@</font>';
		else
			$col2 = '&nbsp;';
		if ($url)
			$col2 .= '<a href="'.$url.'">'.htmlspecialchars($article['TITLE']).' </a>';
		else
			$col2 .= htmlspecialchars($article['TITLE']).' ';
		$bm = explode(' ',trim($article['BM']));
		$bm = $bm[0];
		$col3 = $bm?'<a href="bbsqry.php?userid='.$bm.'">'.$bm.'</a>':'&nbsp;';
		$col4 = date('Y-m-d',$article['TIME']);
		$num++;
		echo '<tr class="'.($num%2?'odd':'even').'"><td class="center">'.$num.'</td><td class="center"><script>'.$col1
		    .'</script></td><td>'.$col2.'</td><td class="center">'.$col3.'</td><td class="center">'.$col4.'</td></tr>';
	}
?>
</tbody></table>
<?php
	if ($isBoard) echo "</div>"; /* </div>: dirty way ... for closing <div class="doc"> */
}

require_once('www2-funcs.php');
require_once('www2-board.php');
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

	$path = bbs_ann_num2path($numpath,$currentuser["userid"]);
	if($path==false){
		html_error_quit("错误的文章");
	}
	$path = substr($path, 10, strlen($path) - 9);
}
else if (isset($_GET['path']))
	$path = trim($_GET['path']);
else 
	$path = "";

if (strstr($path, '.Names') || strstr($path, '..') || strstr($path, 'SYSHome'))
	html_error_quit('不存在该目录');

$board = '';
$articles = array();
$path_tmp = '';
$ret = bbs_read_ann_dir($path,$board,$path_tmp,$articles);

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
$title = bbs_ann_get_title($path);
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
			$dotnames = BBS_HOME . '/' . $path . '/.Names';
			if (cache_header('public',filemtime($dotnames),300))
				return;
		}
		//bbs_board_header($brdarr,-1,0);
		bbs_ann_header($board);
		print("<h1 class=\"bt\">{$title}</h1>");
		$isBoard = true;
?>
<script>
var c = new docWriter('<?php echo addslashes($brdarr["NAME"]); ?>',<?php echo $bid; ?>,0,0,-1,0,0,0,0);
</script>
<?php
}
else {
	$board = '';
	bbs_ann_header($board);
	print("<h1 class=\"bt\">{$title}</h1>");
}		

}
else {
$dotnames = BBS_HOME . '/' . $path . '/.Names';
if (cache_header('public',filemtime($dotnames),300))
	return;
$bid = 0;
bbs_ann_header();
print("<h1 class=\"bt\">{$title}</h1>");
}

bbs_ann_display_articles($articles, $isBoard);
bbs_ann_xsearch($board);

if ($up_cnt >= 2)
	bbs_ann_foot($up_dirs[$up_cnt - 2]);
else
	bbs_ann_foot('');

page_footer();
?>
