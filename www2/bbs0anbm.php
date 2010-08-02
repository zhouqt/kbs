<?php

require('bbs0anbm_pre.php');
	
// 执行各种精华区文件批量操作
$text = "";
if(isset($_POST["annAction"]))
{
	$action = $_POST["annAction"];
	$total = $_POST["annCount"];
	if($action == "delete")
	{
		for($i=1; $i<=$total; $i++)
		{
			if(isset($_POST["ann{$i}"]))
			{
				$fname = $_POST["ann{$i}"];
				if(bbs_ann_delete($filename, $fname) == -1)
				{
					$text = "错误：精华区目录不存在。";
					exit;
				}
			}
		}
	}
	else if(($action == "cut") || ($action == "copy"))
	{
		$fnames = "";
		for($i=1; $i<=$total; $i++)
		{
			if(isset($_POST["ann{$i}"]))
			{
				$fnames .= $_POST["ann{$i}"] . ",";
			}
		}
		$ret = bbs_ann_copy($filename, $fnames, ($action == "cut") ? 1 : 0);
		switch($ret)
		{
			case -1:
					$text = "系统错误：无法修改剪贴板。";
				break;
		}
	}
	else if($action == "paste")
	{
		$ret = bbs_ann_paste($filename);
		switch($ret)
		{
			case -1:
				$text = "错误：精华区目录不存在。";
				break;
			case -2:
				$text = "错误：找不到要粘贴的文件。";
				break;
			case -5:
				$text = "错误：粘贴失败，可能有其他版主正在整理同一目录。";
				break;
		}
	}
	else if($action == "move")
	{
		$oldnum = $_POST["oldnum"];
		$newnum = $_POST["newnum"];
		$ret = bbs_ann_move($filename, $oldnum, $newnum);
		switch($ret)
		{
			case -1:
				$text = "错误：精华区目录不存在。";
				break;
			case -2:
				$text = "错误：找不到要移动的项目。";
				break;
			case -3:
				$text = "错误：移动失败，可能有其他版主正在整理同一目录。";
				break;
		}
	}		
}

function bbs_ann_bm_display_articles($articles, $isBoard) {
	global $show_none, $has_perm_boards, $path, $text, $title;
	$pathstr = substr($path, 9, strlen($path) - 9);
?>
<script type="text/javascript"><!--
var an = new annWriter('<?php echo rawurlencode($pathstr); ?>',<?php echo $has_perm_boards?"1":"0"; ?>,'<?php echo $text; ?>','<?php echo addslashes($title); ?>');
<?php
	if($show_none)
	{
?>
document.write('<tr><td align="center" colspan="7">该精华区目录没有文章。</td></tr>');
<?php
	}
	else foreach ($articles as $article) {
		$title = htmljsformat($article['TITLE']);
        $title_enc = urlencode($article['TITLE']);
		echo 'an.i(' . $article['FLAG'] . ',\'' . $title . ' \',\'' . $title_enc . ' \',\'' . addslashes(trim($article['BM'])) . '\',\'' . rawurlencode($article['FNAME']) . '\',\'' . date('Y-m-d',$article['TIME']) . '\');';
	}
?>
//-->
an.f();
</script>
<?php
	if ($isBoard) echo "</div>"; /* </div>: dirty way ... for closing <div class="doc"> */
}

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
		bbs_board_header($brdarr,-1,0);
		print("<h1 class=\"bt\">{$title}</h1>");
		$isBoard = true;
	}
	else {
		$board = '';
		bbs_ann_header($board);
		print("<h1 class=\"bt\">{$title}</h1>");
	}
	
}
else {
	$bid = 0;
	bbs_ann_header();
	print("<h1 class=\"bt\">{$title}</h1>");
}
?>
<script src="static/www2-addons.js" type="text/javascript"></script>
<?php
bbs_ann_bm_display_articles($articles, $isBoard);
	
if ($up_cnt >= 2)
	bbs_ann_bm_foot($up_dirs[$up_cnt - 2]);
else
	bbs_ann_bm_foot('');

page_footer();
?>
