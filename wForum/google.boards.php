<?php
require("inc/funcs.php");

setStat("°æÃæÁÐ±í");

show_nav();
function getAllBoards() {
	global $yank;
	$allBoards = array();
	$boards = bbs_getboards("*", $group, $yank | 2 | 4);
	if ($boards != FALSE) {
		$brd_desc = $boards["DESC"];
		$brd_name = $boards["NAME"];
		$rows = sizeof($brd_desc);
		for ($t = 0; $t < $rows; $t++)	{
			$allBoards[$brd_name[$t]] = $brd_desc[$t];
		}
	}
	return $allBoards;
}

$allBoards = getAllBoards();
uksort($allBoards, "strcasecmp");
$str = "<ul>";
while (list($key, $val) = each($allBoards)) {
    $str .= "<li><a href=\"boarddoc.php?google=1&ftype=0&name=$key\">" . htmlspecialchars($val) . "</a></li>";
}
$str .= "</ul>";
echo $str;

show_footer();
?>
