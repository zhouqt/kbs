<?php

$setboard=1;

require("inc/funcs.php");
require("inc/user.inc.php");
require("inc/xml.inc.php");

setStat("十大热门话题");

show_nav();

showUserMailBoxOrBR();
head_var();

showTopTen("/xml/day.xml", "今日十大热门话题");
/* 分区变换糊里糊涂的，各站可能都不一样，要用的自己调整 - atppp
for ($i = 0; $i < $sectionCount; $i++) {

	// 分区号变换
	$secid = get_secname_index($section_nums[$i]) - 1;
	if ($secid < 0) $secid = $sectionCount - 1;

	showTopTen("/xml/day_sec" . $secid . ".xml", $section_names[$i][0] . " 分区十大热门话题");
}
*/

show_footer();

function showTopTen($filename, $caption) {
	
	if (!($doc = domxml_open_file(get_bbsfile($filename)))) return;

	$root = $doc->document_element();
	$boards = $root->child_nodes();

	$boardArr = array();
?>
<table cellspacing=1 cellpadding=0 align=center width="97%" class=TableBorder1>
<thead><tr><th align="center" colspan=5 height=25><?php echo $caption; ?></th></tr></thead>
</table>
<br>
<table cellspacing=1 cellpadding=0 align=center width="97%" class=TableBorder1>
<thead><tr><th height="25" width=50>名次</td><th>讨论区</td><th>标题</td><th>作者</td><th>人数</td></tr></thead>
<?php
	$i = 0;
	while($board = array_shift($boards))
	{
		if ($board->node_type() == XML_TEXT_NODE)
			continue;
		
		$r_title = find_content($board, "title");
		$r_author = find_content($board, "author");
		$r_board = find_content($board, "board");
		$r_time = find_content($board, "time");
		$r_number = find_content($board, "number");
		$r_groupid = find_content($board, "groupid");
		
		$brdnum = bbs_getboard($r_board, $boardArr);
		if ($brdnum == 0)
			continue;
		$r_board = urlencode($boardArr["NAME"]);
		$i++;
		$class = 'TableBody'.($i%2 +1);
?>
<tr>
<td height="25" class=<?php echo $class; ?>  align=center><?php echo $i; ?></td>
<td class=<?php echo $class; ?> >&nbsp;<a href="board.php?name=<?php echo $r_board; ?>"><?php echo $r_board; ?></a></td>
<td class=<?php echo $class; ?> >&nbsp;<a href="disparticle.php?boardName=<?php echo $r_board; ?>&ID=<?php echo $r_groupid; ?>"><?php echo htmlspecialchars($r_title); ?></a></td>
<td class=<?php echo $class; ?> align="center"><a href="dispuser.php?id=<?php echo $r_author; ?>"><?php echo $r_author; ?></a></td>
<td class=<?php echo $class; ?> align="center">&nbsp;<?php echo $r_number; ?></td>
</tr>
<?php
	}
?>
</table>
<br><br>
<?php
}
?>
