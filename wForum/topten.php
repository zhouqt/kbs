<?php


$setboard=1;

require("inc/funcs.php");
require("inc/user.inc.php");

show_nav();

setStat("十大热门话题");
?>
<br>
<TABLE cellSpacing=0 cellPadding=0 width=97% border=0 align=center>
<?php

if ($loginok==1) {
	showUserMailbox();
}

head_var();

if (isErrFounded()) {
	html_error_quit();
} else {
	showTopTen();
}
show_footer();

/*--------------- function defines ------------------*/

function showTopTen(){
	$top_file = get_bbsfile("/etc/posts/day");
	$fp = fopen($top_file, "r");
	if ($fp == FALSE) {
		foundErr("无法读取十大话题数据");
		return false;
	}
	$modifytime=filemtime($top_file);
?>
<table cellspacing=1 cellpadding=0 align=center width="97%" class=tableBorder1>
<thead><tr><th align="center" colspan=5 height=25>今日十大热门话题</th></tr></thead>
</table>
<br>
<table cellspacing=1 cellpadding=0 align=center width="97%" class=tableBorder1>
<thead><tr><th height="25" width=50>名次</td><th>讨论区</td><th>标题</td><th>作者</td><th>人数</td></tr></thead>
<?php
	fgets($fp, 256);
	fgets($fp, 256);
	for ($i = 0; $i < 10 && !feof($fp); $i++)
	{
		$one_line = fgets($fp, 256);
		if ($one_line == FALSE)
			break;
		$r_board = trim(substr($one_line, 41, 16));
		$r_num = trim(substr($one_line, 97, 4));
		$r_id = trim(substr($one_line, 118, 12));
		$one_line = fgets($fp, 256);
		if ($one_line == FALSE)
			break;
		$r_title = trim(substr($one_line, 27, 60));
		$class='tablebody'.($i%2 +1);
?>
<tr>
<td class=<?php echo $class; ?>  align=center><?php echo $i+1; ?></td>
<td class=<?php echo $class; ?> >&nbsp;<a href="board.php?name=<?php echo $r_board; ?>"><?php echo $r_board; ?></a></td>
<td class=<?php echo $class; ?> >&nbsp;<a href="queryresult.php?boardName=<?php echo $r_board; ?>&title=<?php echo urlencode($r_title); ?>"><?php echo $r_title; ?></a></td>
<td class=<?php echo $class; ?> >&nbsp;<a href="dispuser?id=<?php echo $r_id; ?>"><?php echo $r_id; ?></a></td>
<td class=<?php echo $class; ?> >&nbsp;<?php echo $r_num; ?></td>
</tr>
<?php
		}
		fclose($fp);
?>
</table>
<?php
}
?>